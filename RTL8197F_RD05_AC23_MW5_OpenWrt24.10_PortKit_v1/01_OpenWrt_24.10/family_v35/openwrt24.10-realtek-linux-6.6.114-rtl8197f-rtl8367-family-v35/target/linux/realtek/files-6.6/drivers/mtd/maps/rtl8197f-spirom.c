// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * MTD driver for the RTL8197F SPI-NOR automatic read window.
 *
 * Array reads use the stable bootloader-programmed auto-map at 0x10000000 and
 * therefore avoid JEDEC/SFDP probing, which wedges early RD05 hardware.  A
 * board may additionally provide a tightly bounded writable window.  Only
 * that window is accepted by the page-program and sector-erase callbacks; the
 * fixed-partitions parser still marks boot/NVRAM/factory/kernel/rootfs read-only.
 *
 * The command path is reconstructed from Realtek's GPL SHEIPA SPI driver.  It
 * preserves the bootloader CTRLR0 setup, uses only single-I/O 3-byte commands,
 * restores the exact automatic-read state after every mutation, and never
 * issues chip erase.  Command-only FIFO width and the sampled status-bit
 * position are calibrated non-destructively with WREN/WRDI before the first
 * page program or sector erase.
 */

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/overflow.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/sizes.h>
#include <linux/slab.h>

#define RTL8197F_SPIC_CTRLR0		0x000
#define RTL8197F_SPIC_CTRLR1		0x004
#define RTL8197F_SPIC_SSIENR		0x008
#define RTL8197F_SPIC_SER		0x010
#define RTL8197F_SPIC_BAUDR		0x014
#define RTL8197F_SPIC_TXFLR		0x020
#define RTL8197F_SPIC_RXFLR		0x024
#define RTL8197F_SPIC_SR		0x028
#define RTL8197F_SPIC_IMR		0x02c
#define RTL8197F_SPIC_ICR		0x048
#define RTL8197F_SPIC_DR		0x060
#define RTL8197F_SPIC_FBAUDR		0x114
#define RTL8197F_SPIC_ADDR_LENGTH	0x118
#define RTL8197F_SPIC_AUTO_LENGTH	0x11c
#define RTL8197F_SPIC_VALID_CMD	0x120
#define RTL8197F_SPIC_FLASH_SIZE	0x124
#define RTL8197F_SPIC_FLUSH_FIFO	0x128

#define RTL8197F_CTRLR0_TMOD		GENMASK(9, 8)
#define RTL8197F_CTRLR0_TMOD_TX	0
#define RTL8197F_CTRLR0_TMOD_RX	3
#define RTL8197F_CTRLR0_USER_MASK	GENMASK(23, 16)

#define RTL8197F_SR_BUSY		BIT(0)
#define RTL8197F_SR_RFNE		BIT(3)
#define RTL8197F_SR_TXE		BIT(5)

#define RTL8197F_SPINOR_WREN		0x06
#define RTL8197F_SPINOR_WRDI		0x04
#define RTL8197F_SPINOR_RDSR		0x05
#define RTL8197F_SPINOR_PP		0x02
#define RTL8197F_SPINOR_SE		0xd8
#define RTL8197F_SPINOR_SR_WIP		BIT(0)
#define RTL8197F_SPINOR_SR_WEL		BIT(1)

#define RTL8197F_FIFO_BYTES		64
#define RTL8197F_CMD_ADDR_BYTES	4
#define RTL8197F_PROGRAM_CHUNK		(RTL8197F_FIFO_BYTES - RTL8197F_CMD_ADDR_BYTES)
#define RTL8197F_PAGE_SIZE		256
#define RTL8197F_CTRL_TIMEOUT_US	200000
#define RTL8197F_PROGRAM_TIMEOUT_MS	2000
#define RTL8197F_ERASE_TIMEOUT_MS	10000
#define RTL8197F_DEFAULT_BAUD_DIV	8

enum rtl8197f_spirom_cmd_mode {
	RTL8197F_CMD_DR8 = 0,
	RTL8197F_CMD_DR16,
	RTL8197F_CMD_DR32_LOW,
	RTL8197F_CMD_DR32_HIGH,
	RTL8197F_CMD_MODE_COUNT,
};

struct rtl8197f_spirom {
	struct mtd_info mtd;
	struct device *dev;
	void __iomem *base;
	void __iomem *ctrl;
	struct clk *clk;
	resource_size_t size;
	u64 writable_offset;
	u64 writable_size;
	u32 read_shift;
	u32 baud_div;
	u32 ctrlr0_boot;
	u32 ctrlr1_boot;
	u32 ssienr_boot;
	u32 ser_boot;
	u32 baudr_boot;
	u32 fbaudr_boot;
	u32 addr_length_boot;
	u32 auto_length_boot;
	u32 valid_cmd_boot;
	u32 flash_size_boot;
	struct mutex lock;
	u8 status_shift;
	s8 status_bit_shift;
	u8 command_mode;
	bool status_shift_valid;
	bool status_bit_shift_valid;
	bool command_mode_valid;
	bool write_enabled;
};

static inline u32 rtl8197f_spirom_ctrl_read(struct rtl8197f_spirom *rom,
					    u32 reg)
{
	return readl(rom->ctrl + reg);
}

static inline void rtl8197f_spirom_ctrl_write(struct rtl8197f_spirom *rom,
					       u32 reg, u32 val)
{
	writel(val, rom->ctrl + reg);
}

static int rtl8197f_spirom_wait_controller(struct rtl8197f_spirom *rom,
					   const char *where)
{
	u32 val;
	int ret;

	ret = readl_poll_timeout(rom->ctrl + RTL8197F_SPIC_SR, val,
				 !(val & RTL8197F_SR_BUSY), 1,
				 RTL8197F_CTRL_TIMEOUT_US);
	if (ret)
		dev_err(rom->dev, "%s: controller timeout, sr=0x%08x\n",
			where, val);
	else if (val & RTL8197F_SR_TXE)
		dev_warn(rom->dev, "%s: controller TX error, sr=0x%08x\n",
			 where, val);

	return ret;
}

static int rtl8197f_spirom_wait_rx(struct rtl8197f_spirom *rom)
{
	u32 val;
	int ret;

	ret = readl_poll_timeout(rom->ctrl + RTL8197F_SPIC_SR, val,
				 val & RTL8197F_SR_RFNE, 1,
				 RTL8197F_CTRL_TIMEOUT_US);
	if (ret)
		dev_err(rom->dev, "controller RX timeout, sr=0x%08x rxflr=%u\n",
			val, rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_RXFLR));

	return ret;
}

static u32 rtl8197f_spirom_cmd_addr_word(u8 cmd, u32 addr)
{
	/* Realtek GPL SDK CMD_ADDR_FORMAT(cmd, addr). */
	return (cmd & 0x000000ff) |
	       ((addr & 0x000000ff) << 24) |
	       ((addr & 0x0000ff00) << 8) |
	       ((addr & 0x00ff0000) >> 8);
}

static void rtl8197f_spirom_prepare_user(struct rtl8197f_spirom *rom, u32 tmod)
{
	u32 ctrlr0;

	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_SSIENR, 0);

	ctrlr0 = rom->ctrlr0_boot;
	ctrlr0 &= ~(RTL8197F_CTRLR0_TMOD | RTL8197F_CTRLR0_USER_MASK);
	ctrlr0 |= FIELD_PREP(RTL8197F_CTRLR0_TMOD, tmod);

	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_CTRLR0, ctrlr0);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_SER, BIT(0));
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_ADDR_LENGTH, 3);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_BAUDR, rom->baud_div);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_FBAUDR, rom->baud_div);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_IMR, 0);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_ICR, 0xffffffff);
}

static void rtl8197f_spirom_restore_automap(struct rtl8197f_spirom *rom)
{
	if (!rom->ctrl)
		return;

	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_SSIENR, 0);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_CTRLR0, rom->ctrlr0_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_CTRLR1, rom->ctrlr1_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_SER, rom->ser_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_BAUDR, rom->baudr_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_FBAUDR, rom->fbaudr_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_ADDR_LENGTH,
				   rom->addr_length_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_AUTO_LENGTH,
				   rom->auto_length_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_VALID_CMD,
				   rom->valid_cmd_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_FLASH_SIZE,
				   rom->flash_size_boot);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_IMR, 0);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_ICR, 0xffffffff);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_SSIENR,
				   rom->ssienr_boot);
	mb();
	(void)rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_CTRLR0);
}

static void rtl8197f_spirom_flush_fifo(struct rtl8197f_spirom *rom)
{
	u32 pending;
	unsigned int guard = RTL8197F_FIFO_BYTES;

	/*
	 * The v32 hardware log returned status 0x04 after WREN.  That value can be
	 * left in the controller RX FIFO by the bootloader/auto-read engine.  The
	 * SHEIPA block has an explicit FIFO flush register; use it before every
	 * user-mode transaction and drain any residual words as a second guard.
	 */
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_FLUSH_FIFO, 1);
	mb();

	pending = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_RXFLR);
	while (pending && guard--) {
		(void)readl(rom->ctrl + RTL8197F_SPIC_DR);
		pending = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_RXFLR);
	}
}

static const char *rtl8197f_spirom_cmd_mode_name(unsigned int mode)
{
	switch (mode) {
	case RTL8197F_CMD_DR8:
		return "dr8";
	case RTL8197F_CMD_DR16:
		return "dr16";
	case RTL8197F_CMD_DR32_LOW:
		return "dr32-low";
	case RTL8197F_CMD_DR32_HIGH:
		return "dr32-high";
	default:
		return "unknown";
	}
}

static void rtl8197f_spirom_push_cmd_only(struct rtl8197f_spirom *rom,
					 u8 cmd, unsigned int mode)
{
	switch (mode) {
	case RTL8197F_CMD_DR16:
		writew(cpu_to_le16(cmd), rom->ctrl + RTL8197F_SPIC_DR);
		break;
	case RTL8197F_CMD_DR32_LOW:
		writel(cpu_to_le32(cmd), rom->ctrl + RTL8197F_SPIC_DR);
		break;
	case RTL8197F_CMD_DR32_HIGH:
		writel(cpu_to_le32((u32)cmd << 24),
		       rom->ctrl + RTL8197F_SPIC_DR);
		break;
	case RTL8197F_CMD_DR8:
	default:
		writeb(cmd, rom->ctrl + RTL8197F_SPIC_DR);
		break;
	}
}

static int rtl8197f_spirom_tx_mode(struct rtl8197f_spirom *rom, u8 cmd,
				  bool has_addr, u32 addr,
				  const u8 *buf, size_t len,
				  unsigned int command_mode)
{
	size_t i;
	int ret;

	if (len > RTL8197F_PROGRAM_CHUNK)
		return -EINVAL;

	rtl8197f_spirom_prepare_user(rom, RTL8197F_CTRLR0_TMOD_TX);
	rtl8197f_spirom_flush_fifo(rom);

	/* Three-byte-address commands use the GPL SDK packed dword.  Command-only
	 * accesses are calibrated because the Lexra MMIO bridge can expose the
	 * SHEIPA FIFO as an 8-, 16- or 32-bit write port despite identical reads.
	 */
	if (has_addr)
		writel(cpu_to_le32(rtl8197f_spirom_cmd_addr_word(cmd, addr)),
		       rom->ctrl + RTL8197F_SPIC_DR);
	else
		rtl8197f_spirom_push_cmd_only(rom, cmd, command_mode);

	for (i = 0; i < len; i++)
		writeb(buf[i], rom->ctrl + RTL8197F_SPIC_DR);

	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_SSIENR, 1);
	ret = rtl8197f_spirom_wait_controller(rom, "tx");
	return ret;
}

static int rtl8197f_spirom_tx(struct rtl8197f_spirom *rom, u8 cmd,
			     bool has_addr, u32 addr,
			     const u8 *buf, size_t len)
{
	unsigned int mode = rom->command_mode_valid ? rom->command_mode :
			    RTL8197F_CMD_DR8;

	return rtl8197f_spirom_tx_mode(rom, cmd, has_addr, addr, buf, len,
				       mode);
}

static int rtl8197f_spirom_read_status_raw_mode(struct rtl8197f_spirom *rom,
						u32 *raw,
						unsigned int command_mode)
{
	int ret;

	rtl8197f_spirom_prepare_user(rom, RTL8197F_CTRLR0_TMOD_RX);
	rtl8197f_spirom_flush_fifo(rom);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_CTRLR1, 1);
	rtl8197f_spirom_push_cmd_only(rom, RTL8197F_SPINOR_RDSR,
				      command_mode);
	rtl8197f_spirom_ctrl_write(rom, RTL8197F_SPIC_SSIENR, 1);

	ret = rtl8197f_spirom_wait_controller(rom, "read-status");
	if (ret)
		return ret;
	ret = rtl8197f_spirom_wait_rx(rom);
	if (ret)
		return ret;

	/* Consume a complete FIFO entry.  Both the command write width and the
	 * returned byte lane depend on the SHEIPA/Lexra bridge setup.  WREN/WRDI
	 * calibration below proves the exact combination before erase/program.
	 */
	*raw = readl(rom->ctrl + RTL8197F_SPIC_DR);
	return 0;
}

static int rtl8197f_spirom_read_status_raw(struct rtl8197f_spirom *rom,
					   u32 *raw)
{
	unsigned int mode = rom->command_mode_valid ? rom->command_mode :
			    RTL8197F_CMD_DR32_HIGH;

	return rtl8197f_spirom_read_status_raw_mode(rom, raw, mode);
}

static u8 rtl8197f_spirom_status_normalize(struct rtl8197f_spirom *rom,
					 u32 raw)
{
	u8 status = (raw >> rom->status_shift) & 0xff;

	if (rom->status_bit_shift > 0)
		status >>= rom->status_bit_shift;
	else if (rom->status_bit_shift < 0)
		status <<= -rom->status_bit_shift;

	return status;
}

static int rtl8197f_spirom_read_status(struct rtl8197f_spirom *rom, u8 *status)
{
	u32 raw;
	int ret;

	ret = rtl8197f_spirom_read_status_raw(rom, &raw);
	if (ret)
		return ret;

	*status = rtl8197f_spirom_status_normalize(rom, raw);
	return 0;
}

static bool rtl8197f_spirom_find_wel_transition(u32 before, u32 after,
					      unsigned int *lane,
					      unsigned int *bit)
{
	unsigned int i;

	for (i = 0; i < sizeof(before); i++) {
		u8 old = (before >> (i * 8)) & 0xff;
		u8 now = (after >> (i * 8)) & 0xff;
		u8 delta = now & ~old;
		unsigned int candidate;

		/* Prefer the standard WEL position when it changed. */
		if (delta & RTL8197F_SPINOR_SR_WEL) {
			*lane = i;
			*bit = 1;
			return true;
		}

		/* RD05 auto-map data is shifted by one byte, and the v33 RDSR trace
		 * showed an analogous one-bit sampling shift (0x04 after WREN).
		 * Accept exactly one newly asserted low status bit, then prove it with
		 * WRDI and a second WREN below before trusting it.
		 */
		if (!is_power_of_2(delta))
			continue;
		candidate = __ffs(delta);
		if (candidate < 1 || candidate > 3)
			continue;

		*lane = i;
		*bit = candidate;
		return true;
	}

	return false;
}

static bool rtl8197f_spirom_raw_bit(u32 raw, unsigned int lane,
				   unsigned int bit)
{
	return raw & BIT(lane * 8 + bit);
}

static int rtl8197f_spirom_calibrate_write_enable(struct rtl8197f_spirom *rom)
{
	static const u8 preferred_modes[] = {
		RTL8197F_CMD_DR32_HIGH, /* Realtek GPL SDK: command in bits 31:24 */
		RTL8197F_CMD_DR8,
		RTL8197F_CMD_DR16,
		RTL8197F_CMD_DR32_LOW,
	};
	u32 before, after, disabled, confirmed;
	unsigned int index, mode, lane, bit;
	int ret;

	for (index = 0; index < ARRAY_SIZE(preferred_modes); index++) {
		mode = preferred_modes[index];
		/* WRDI/WREN are non-destructive.  Requiring a clear-set-clear-set
		 * transition prevents a static BP bit (the v33 raw 0x04 ambiguity)
		 * from being mistaken for WEL.
		 */
		ret = rtl8197f_spirom_tx_mode(rom, RTL8197F_SPINOR_WRDI,
					       false, 0, NULL, 0, mode);
		if (ret)
			continue;
		ret = rtl8197f_spirom_read_status_raw_mode(rom, &before, mode);
		if (ret)
			continue;

		ret = rtl8197f_spirom_tx_mode(rom, RTL8197F_SPINOR_WREN,
					       false, 0, NULL, 0, mode);
		if (ret)
			continue;
		ret = rtl8197f_spirom_read_status_raw_mode(rom, &after, mode);
		if (ret || !rtl8197f_spirom_find_wel_transition(before, after,
							      &lane, &bit))
			continue;

		ret = rtl8197f_spirom_tx_mode(rom, RTL8197F_SPINOR_WRDI,
					       false, 0, NULL, 0, mode);
		if (ret)
			continue;
		ret = rtl8197f_spirom_read_status_raw_mode(rom, &disabled, mode);
		if (ret || rtl8197f_spirom_raw_bit(disabled, lane, bit))
			continue;

		ret = rtl8197f_spirom_tx_mode(rom, RTL8197F_SPINOR_WREN,
					       false, 0, NULL, 0, mode);
		if (ret)
			continue;
		ret = rtl8197f_spirom_read_status_raw_mode(rom, &confirmed, mode);
		if (ret || !rtl8197f_spirom_raw_bit(confirmed, lane, bit))
			continue;

		rom->command_mode = mode;
		rom->command_mode_valid = true;
		rom->status_shift = lane * 8;
		rom->status_shift_valid = true;
		rom->status_bit_shift = (s8)bit - 1;
		rom->status_bit_shift_valid = true;
		dev_info(rom->dev,
			 "calibrated SPI-NOR WEL: cmd=%s lane=%u bit=%u bit-shift=%d before=0x%08x after=0x%08x wrdi=0x%08x confirm=0x%08x\n",
			 rtl8197f_spirom_cmd_mode_name(mode), lane, bit,
			 rom->status_bit_shift, before, after, disabled, confirmed);
		return 0;
	}

	dev_err(rom->dev,
		"unable to calibrate SPI-NOR WEL with DR8/16/32 command modes; txflr=%u rxflr=%u\n",
		rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_TXFLR),
		rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_RXFLR));
	return -EIO;
}

static int rtl8197f_spirom_write_enable(struct rtl8197f_spirom *rom)
{
	u8 status = 0xff;
	unsigned int attempt;
	int ret;

	if (!rom->command_mode_valid || !rom->status_shift_valid ||
	    !rom->status_bit_shift_valid)
		return rtl8197f_spirom_calibrate_write_enable(rom);

	for (attempt = 0; attempt < 4; attempt++) {
		ret = rtl8197f_spirom_tx(rom, RTL8197F_SPINOR_WREN,
					 false, 0, NULL, 0);
		if (ret)
			return ret;
		ret = rtl8197f_spirom_read_status(rom, &status);
		if (ret)
			return ret;
		if (status & RTL8197F_SPINOR_SR_WEL)
			return 0;
		udelay(10);
	}

	dev_err(rom->dev,
		"write-enable latch did not set after calibrated %s transaction, normalized-status=0x%02x txflr=%u rxflr=%u\n",
		rtl8197f_spirom_cmd_mode_name(rom->command_mode), status,
		rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_TXFLR),
		rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_RXFLR));
	return -EIO;
}

static int rtl8197f_spirom_wait_flash(struct rtl8197f_spirom *rom,
				     unsigned int timeout_ms,
				     const char *where)
{
	unsigned long deadline = jiffies + msecs_to_jiffies(timeout_ms);
	u8 status = 0xff;
	int ret;

	do {
		ret = rtl8197f_spirom_read_status(rom, &status);
		if (ret)
			return ret;
		if (!(status & RTL8197F_SPINOR_SR_WIP))
			return 0;
		usleep_range(1000, 2000);
	} while (time_before(jiffies, deadline));

	dev_err(rom->dev, "%s: flash busy timeout, status=0x%02x\n",
		where, status);
	return -ETIMEDOUT;
}

static bool rtl8197f_spirom_in_writable_window(struct rtl8197f_spirom *rom,
					       u64 from, u64 len)
{
	u64 end;

	if (!rom->write_enabled || from < rom->writable_offset)
		return false;
	if (check_add_overflow(from, len, &end))
		return false;

	return end <= rom->writable_offset + rom->writable_size;
}

static int rtl8197f_spirom_read(struct mtd_info *mtd, loff_t from,
				 size_t len, size_t *retlen, u_char *buf)
{
	struct rtl8197f_spirom *rom = mtd->priv;
	loff_t mapped_from = from;

	if (from < 0 || from > rom->size || len > rom->size - from)
		return -EINVAL;

	/* RD05 auto-map byte reads are shifted by +1.  Keep partition offsets in
	 * the OEM coordinate system and compensate only for mapped reads.
	 */
	if (rom->read_shift && mapped_from >= rom->read_shift)
		mapped_from -= rom->read_shift;

	mutex_lock(&rom->lock);
	memcpy_fromio(buf, rom->base + mapped_from, len);
	mutex_unlock(&rom->lock);
	*retlen = len;
	return 0;
}

static int rtl8197f_spirom_write(struct mtd_info *mtd, loff_t to,
				  size_t len, size_t *retlen,
				  const u_char *buf)
{
	struct rtl8197f_spirom *rom = mtd->priv;
	u64 pos;
	int ret = 0;

	*retlen = 0;
	if (to < 0 || !rtl8197f_spirom_in_writable_window(rom, to, len))
		return -EROFS;

	pos = to;
	mutex_lock(&rom->lock);
	while (*retlen < len) {
		size_t page_left = RTL8197F_PAGE_SIZE - (pos & (RTL8197F_PAGE_SIZE - 1));
		size_t chunk = min3(len - *retlen, page_left,
				    (size_t)RTL8197F_PROGRAM_CHUNK);

		ret = rtl8197f_spirom_write_enable(rom);
		if (ret)
			break;
		ret = rtl8197f_spirom_tx(rom, RTL8197F_SPINOR_PP, true, pos,
					  buf + *retlen, chunk);
		if (ret)
			break;
		ret = rtl8197f_spirom_wait_flash(rom,
					  RTL8197F_PROGRAM_TIMEOUT_MS,
					  "page-program");
		if (ret)
			break;

		pos += chunk;
		*retlen += chunk;
		cond_resched();
	}
	rtl8197f_spirom_restore_automap(rom);
	mutex_unlock(&rom->lock);

	return ret;
}

static int rtl8197f_spirom_erase(struct mtd_info *mtd,
				  struct erase_info *instr)
{
	struct rtl8197f_spirom *rom = mtd->priv;
	u64 pos, end;
	int ret = 0;

	if (!IS_ALIGNED(instr->addr, mtd->erasesize) ||
	    !IS_ALIGNED(instr->len, mtd->erasesize) ||
	    !rtl8197f_spirom_in_writable_window(rom, instr->addr, instr->len))
		return -EROFS;

	pos = instr->addr;
	end = instr->addr + instr->len;
	instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;

	mutex_lock(&rom->lock);
	while (pos < end) {
		ret = rtl8197f_spirom_write_enable(rom);
		if (ret)
			break;
		ret = rtl8197f_spirom_tx(rom, RTL8197F_SPINOR_SE, true, pos,
					  NULL, 0);
		if (ret)
			break;
		ret = rtl8197f_spirom_wait_flash(rom,
					  RTL8197F_ERASE_TIMEOUT_MS,
					  "sector-erase");
		if (ret)
			break;
		pos += mtd->erasesize;
		cond_resched();
	}
	if (ret)
		instr->fail_addr = pos;
	rtl8197f_spirom_restore_automap(rom);
	mutex_unlock(&rom->lock);

	return ret;
}

static int rtl8197f_spirom_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtl8197f_spirom *rom;
	struct resource *res;
	const char *label = NULL;
	u32 erasesize = SZ_64K;
	u32 writable_offset, writable_size;
	int ret;

	rom = devm_kzalloc(dev, sizeof(*rom), GFP_KERNEL);
	if (!rom)
		return -ENOMEM;

	rom->dev = dev;
	mutex_init(&rom->lock);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "automap");
	if (!res)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	rom->size = resource_size(res);
	rom->base = devm_ioremap(dev, res->start, rom->size);
	if (!rom->base)
		return -ENOMEM;

	of_property_read_string(dev->of_node, "label", &label);
	of_property_read_u32(dev->of_node, "erase-size", &erasesize);
	if (of_property_read_u32(dev->of_node, "realtek,read-shift",
				 &rom->read_shift))
		rom->read_shift = 1;
	if (of_property_read_u32(dev->of_node, "realtek,spi-clock-div",
				 &rom->baud_div))
		rom->baud_div = RTL8197F_DEFAULT_BAUD_DIV;
	rom->baud_div = clamp_t(u32, rom->baud_div, 2, 0xffff);

	if (!of_property_read_u32(dev->of_node, "realtek,writable-offset",
				  &writable_offset) &&
	    !of_property_read_u32(dev->of_node, "realtek,writable-size",
				  &writable_size)) {
		rom->writable_offset = writable_offset;
		rom->writable_size = writable_size;
		if (!rom->writable_size ||
		    rom->writable_offset >= rom->size ||
		    rom->writable_size > rom->size - rom->writable_offset ||
		    !IS_ALIGNED(rom->writable_offset, erasesize) ||
		    !IS_ALIGNED(rom->writable_size, erasesize))
			return dev_err_probe(dev, -EINVAL,
				"invalid writable window 0x%llx+0x%llx\n",
				rom->writable_offset, rom->writable_size);

		rom->ctrl = devm_platform_ioremap_resource_byname(pdev,
							      "controller");
		if (IS_ERR(rom->ctrl))
			return dev_err_probe(dev, PTR_ERR(rom->ctrl),
					     "writable window requires controller resource\n");

		rom->clk = devm_clk_get_optional_enabled(dev, NULL);
		if (IS_ERR(rom->clk))
			return dev_err_probe(dev, PTR_ERR(rom->clk),
					     "failed to enable SPI clock\n");

		rom->ctrlr0_boot = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_CTRLR0);
		rom->ctrlr1_boot = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_CTRLR1);
		rom->ssienr_boot = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_SSIENR);
		rom->ser_boot = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_SER);
		rom->baudr_boot = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_BAUDR);
		rom->fbaudr_boot = rtl8197f_spirom_ctrl_read(rom, RTL8197F_SPIC_FBAUDR);
		rom->addr_length_boot = rtl8197f_spirom_ctrl_read(rom,
							 RTL8197F_SPIC_ADDR_LENGTH);
		rom->auto_length_boot = rtl8197f_spirom_ctrl_read(rom,
							 RTL8197F_SPIC_AUTO_LENGTH);
		rom->valid_cmd_boot = rtl8197f_spirom_ctrl_read(rom,
						       RTL8197F_SPIC_VALID_CMD);
		rom->flash_size_boot = rtl8197f_spirom_ctrl_read(rom,
							RTL8197F_SPIC_FLASH_SIZE);
		rom->write_enabled = true;
	}

	rom->mtd.name = label ?: dev_name(dev);
	rom->mtd.type = rom->write_enabled ? MTD_NORFLASH : MTD_ROM;
	rom->mtd.flags = rom->write_enabled ? MTD_CAP_NORFLASH :
						MTD_CAP_ROM | MTD_NO_ERASE;
	rom->mtd.size = rom->size;
	rom->mtd.erasesize = erasesize ?: SZ_64K;
	rom->mtd.writesize = 1;
	rom->mtd.writebufsize = 1;
	rom->mtd.owner = THIS_MODULE;
	rom->mtd.dev.parent = dev;
	rom->mtd.priv = rom;
	rom->mtd._read = rtl8197f_spirom_read;
	if (rom->write_enabled) {
		rom->mtd._write = rtl8197f_spirom_write;
		rom->mtd._erase = rtl8197f_spirom_erase;
	}
	mtd_set_of_node(&rom->mtd, dev->of_node);

	platform_set_drvdata(pdev, rom);

	if (rom->write_enabled)
		dev_info(dev,
			 "RTL8197F SPI auto-map MTD: phys=%pa size=%pa erase=0x%x read_shift=%u writable=0x%llx+0x%llx ctrlr0=0x%08x baud-div=%u\n",
			 &res->start, &rom->size, rom->mtd.erasesize,
			 rom->read_shift, rom->writable_offset,
			 rom->writable_size, rom->ctrlr0_boot, rom->baud_div);
	else
		dev_info(dev,
			 "RTL8197F SPI auto-map MTD: phys=%pa size=%pa erase=0x%x read_shift=%u read-only/no-erase\n",
			 &res->start, &rom->size, rom->mtd.erasesize,
			 rom->read_shift);

	if (rom->size >= 0x360000) {
		u8 kraw[4], rraw[4], kfix[4], rfix[4];
		size_t retlen;

		kraw[0] = readb(rom->base + 0x60000);
		kraw[1] = readb(rom->base + 0x60001);
		kraw[2] = readb(rom->base + 0x60002);
		kraw[3] = readb(rom->base + 0x60003);
		rraw[0] = readb(rom->base + 0x350000);
		rraw[1] = readb(rom->base + 0x350001);
		rraw[2] = readb(rom->base + 0x350002);
		rraw[3] = readb(rom->base + 0x350003);

		rtl8197f_spirom_read(&rom->mtd, 0x60000, sizeof(kfix), &retlen,
				       kfix);
		rtl8197f_spirom_read(&rom->mtd, 0x350000, sizeof(rfix), &retlen,
				       rfix);

		dev_info(dev,
			 "RTL8197F SPI auto-map raw: @0x60000=%*phN @0x350000=%*phN\n",
			 4, kraw, 4, rraw);
		dev_info(dev,
			 "RTL8197F SPI auto-map fixed: @0x60000=%c%c%c%c @0x350000=%c%c%c%c\n",
			 kfix[0], kfix[1], kfix[2], kfix[3],
			 rfix[0], rfix[1], rfix[2], rfix[3]);
	}

	ret = mtd_device_parse_register(&rom->mtd, NULL, NULL, NULL, 0);
	if (ret)
		return ret;

	dev_info(dev, "RTL8197F SPI auto-map MTD registered%s\n",
		 rom->write_enabled ? " with calibrated bounded rootfs_data writes" : "");
	return 0;
}

static int rtl8197f_spirom_remove(struct platform_device *pdev)
{
	struct rtl8197f_spirom *rom = platform_get_drvdata(pdev);

	return mtd_device_unregister(&rom->mtd);
}

static const struct of_device_id rtl8197f_spirom_of_match[] = {
	{ .compatible = "realtek,rtl8197f-spirom" },
	{ .compatible = "realtek,rtl8197fh-spirom" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_spirom_of_match);

static struct platform_driver rtl8197f_spirom_driver = {
	.probe = rtl8197f_spirom_probe,
	.remove = rtl8197f_spirom_remove,
	.driver = {
		.name = "rtl8197f-spirom",
		.of_match_table = rtl8197f_spirom_of_match,
	},
};
module_platform_driver(rtl8197f_spirom_driver);

MODULE_DESCRIPTION("Calibrated bounded writable MTD over RTL8197F/RTL8197FH SPI-NOR auto-map window");
MODULE_AUTHOR("OpenWrt RTL8197F bring-up");
MODULE_LICENSE("GPL");
