// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Realtek RTL8197F/RTL8197FH SHEIPA SPI-MEM controller driver.
 *
 * Reconstructed from the Realtek GPL bootcode/BSP spi-sheipa.c.  The
 * important RTL8197F quirk is that ctrlr0 is already programmed by the
 * bootloader and must not be overwritten with DesignWare defaults during
 * Linux bring-up.  Commands are pushed to the FIFO in the Realtek word format
 * used by the SDK, while normal SPI-NOR array reads use the SoC's linear
 * auto-read window at 0x10000000.
 *
 * The write/erase path is intentionally disabled unless the DT node contains
 * realtek,allow-writes.  This keeps early RD05 bring-up safe.
 */

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-mem.h>

#define RTL8197F_SPIC_CTRLR0       0x000
#define RTL8197F_SPIC_CTRLR1       0x004
#define RTL8197F_SPIC_SSIENR       0x008
#define RTL8197F_SPIC_SER          0x010
#define RTL8197F_SPIC_BAUDR        0x014
#define RTL8197F_SPIC_TXFLR        0x020
#define RTL8197F_SPIC_RXFLR        0x024
#define RTL8197F_SPIC_SR           0x028
#define RTL8197F_SPIC_IMR          0x02c
#define RTL8197F_SPIC_ICR          0x048
#define RTL8197F_SPIC_DR           0x060
#define RTL8197F_SPIC_FBAUDR       0x114
#define RTL8197F_SPIC_ADDR_LENGTH  0x118
#define RTL8197F_SPIC_AUTO_LENGTH  0x11c
#define RTL8197F_SPIC_VALID_CMD    0x120
#define RTL8197F_SPIC_FLASH_SIZE   0x124
#define RTL8197F_SPIC_FLUSH_FIFO   0x128

#define RTL8197F_CTRLR0_TMOD       GENMASK(9, 8)
#define RTL8197F_CTRLR0_TMOD_TX    0
#define RTL8197F_CTRLR0_TMOD_RX    3
/* Realtek SDK clears channel/fast-read bits for normal single-I/O user mode. */
#define RTL8197F_CTRLR0_USER_MASK  GENMASK(23, 16)

#define RTL8197F_SR_BUSY           BIT(0)
#define RTL8197F_SR_TFE            BIT(2)
#define RTL8197F_SR_RFNE           BIT(3)
#define RTL8197F_SR_TXE            BIT(5)

#define RTL8197F_FIFO_DEPTH        64
#define RTL8197F_DEFAULT_OCCLK     100000000U /* SDK max_freq; SPIC clock is 200MHz/(2*div) */
#define RTL8197F_DEFAULT_SPEED     15000000U
#define RTL8197F_BOOT_BAUD_DIV     8
#define RTL8197F_TIMEOUT_US        200000

#define SPINOR_OP_WREN             0x06
#define SPINOR_OP_WRDI             0x04
#define SPINOR_OP_RDSR             0x05
#define SPINOR_OP_READ             0x03
#define SPINOR_OP_FAST_READ        0x0b
#define SPINOR_OP_READ_4B          0x13
#define SPINOR_OP_FAST_READ_4B     0x0c
#define SPINOR_OP_RDID             0x9f
#define SPINOR_OP_RDSFDP           0x5a
#define SPINOR_OP_PP               0x02
#define SPINOR_OP_BE_4K            0x20
#define SPINOR_OP_SE               0xd8
#define SPINOR_OP_CHIP_ERASE       0xc7

struct rtl8197f_spic {
	struct device *dev;
	void __iomem *base;
	void __iomem *automap;
	resource_size_t automap_size;
	struct clk *clk;
	u32 oc_hz;
	u32 ctrlr0_boot;
	bool allow_writes;
};

static inline u32 spic_read(struct rtl8197f_spic *spic, u32 reg)
{
	return readl(spic->base + reg);
}

static inline void spic_write(struct rtl8197f_spic *spic, u32 reg, u32 val)
{
	writel(val, spic->base + reg);
}

static inline void spic_write_dr8(struct rtl8197f_spic *spic, unsigned int dr, u8 val)
{
	/*
	 * The Realtek SDK uses byte/half/word accesses to DRx.  This matters:
	 * a command-only transaction such as RDID/RDSR must enqueue exactly one
	 * command byte, not a 32-bit word with three trailing zero bytes.
	 */
	writeb(val, spic->base + RTL8197F_SPIC_DR + dr * 4);
}

static inline void spic_write_dr16(struct rtl8197f_spic *spic, unsigned int dr, u16 val)
{
	writew(cpu_to_le16(val), spic->base + RTL8197F_SPIC_DR + dr * 4);
}

static inline void spic_write_dr32(struct rtl8197f_spic *spic, unsigned int dr, u32 val)
{
	writel(cpu_to_le32(val), spic->base + RTL8197F_SPIC_DR + dr * 4);
}

static int rtl8197f_spic_wait_not_busy(struct rtl8197f_spic *spic, const char *where)
{
	u32 val;
	int ret;

	ret = readl_poll_timeout(spic->base + RTL8197F_SPIC_SR, val,
				  !(val & RTL8197F_SR_BUSY), 1, RTL8197F_TIMEOUT_US);
	if (ret)
		dev_err(spic->dev, "%s: timeout waiting not-busy, sr=0x%08x\n",
			where, val);
	if (!ret && (val & RTL8197F_SR_TXE))
		dev_warn(spic->dev, "%s: TX error bit set, sr=0x%08x\n", where, val);

	return ret;
}

static int rtl8197f_spic_wait_rx(struct rtl8197f_spic *spic)
{
	u32 val;
	int ret;

	ret = readl_poll_timeout(spic->base + RTL8197F_SPIC_SR, val,
				  val & RTL8197F_SR_RFNE, 1, RTL8197F_TIMEOUT_US);
	if (ret)
		dev_err(spic->dev, "timeout waiting RX data, sr=0x%08x\n", val);

	return ret;
}

static void rtl8197f_spic_disable(struct rtl8197f_spic *spic)
{
	spic_write(spic, RTL8197F_SPIC_SSIENR, 0);
}

static void rtl8197f_spic_enable(struct rtl8197f_spic *spic)
{
	spic_write(spic, RTL8197F_SPIC_SSIENR, 1);
}

static void rtl8197f_spic_set_speed(struct rtl8197f_spic *spic, u32 speed_hz)
{
	u32 div;

	if (!speed_hz)
		speed_hz = RTL8197F_DEFAULT_SPEED;
	if (speed_hz > spic->oc_hz)
		speed_hz = RTL8197F_DEFAULT_SPEED;

	/* Realtek SDK uses div = max_freq / requested_freq, rounded up. */
	div = DIV_ROUND_UP(spic->oc_hz, speed_hz);
	if (div < 2)
		div = 2;
	if (div > 0xffff)
		div = 0xffff;

	spic_write(spic, RTL8197F_SPIC_BAUDR, div);
	spic_write(spic, RTL8197F_SPIC_FBAUDR, div);
}

static void rtl8197f_spic_prepare(struct rtl8197f_spic *spic,
				  struct spi_device *spi, unsigned int tmod)
{
	u32 ctrlr0;

	rtl8197f_spic_disable(spic);

	/*
	 * Preserve DFS/FRF/SCPOL/SCPH from bootloader.  Overwriting ctrlr0 with
	 * a generic 8250/DW-style 8-bit setting can hang the RTL8197F controller.
	 */
	ctrlr0 = spic->ctrlr0_boot ? spic->ctrlr0_boot : spic_read(spic, RTL8197F_SPIC_CTRLR0);
	ctrlr0 &= ~RTL8197F_CTRLR0_TMOD;
	ctrlr0 &= ~RTL8197F_CTRLR0_USER_MASK;
	ctrlr0 |= FIELD_PREP(RTL8197F_CTRLR0_TMOD, tmod);

	spic_write(spic, RTL8197F_SPIC_CTRLR0, ctrlr0);
	spic_write(spic, RTL8197F_SPIC_SER, BIT(spi_get_chipselect(spi, 0)));
	spic_write(spic, RTL8197F_SPIC_IMR, 0);
	spic_write(spic, RTL8197F_SPIC_ICR, 0xffffffff);
	rtl8197f_spic_set_speed(spic, spi->max_speed_hz ?: RTL8197F_DEFAULT_SPEED);
}

static u32 rtl8197f_spic_cmd_addr_word(u8 cmd, u32 addr)
{
	/* Realtek SDK CMD_ADDR_FORMAT(cmd, addr). */
	return (cmd & 0x000000ff) |
	       ((addr & 0x000000ff) << 24) |
	       ((addr & 0x0000ff00) << 8)  |
	       ((addr & 0x00ff0000) >> 8);
}

static u32 rtl8197f_spic_swab_addr4(u32 addr)
{
	return ((addr & 0x000000ff) << 24) |
	       ((addr & 0x0000ff00) << 8)  |
	       ((addr & 0x00ff0000) >> 8)  |
	       ((addr & 0xff000000) >> 24);
}

static void rtl8197f_spic_push_cmd_addr(struct rtl8197f_spic *spic,
					const struct spi_mem_op *op, u32 addr)
{
	if (op->addr.nbytes == 0) {
		spic_write_dr8(spic, 0, op->cmd.opcode);
		return;
	}

	if (op->addr.nbytes == 3) {
		spic_write_dr32(spic, 0,
			rtl8197f_spic_cmd_addr_word(op->cmd.opcode, addr));
		return;
	}

	/* 4-byte address mode: SDK writes cmd byte then byte-swapped address word. */
	spic_write_dr8(spic, 0, op->cmd.opcode);
	spic_write_dr32(spic, 1, rtl8197f_spic_swab_addr4(addr));
}

static bool rtl8197f_spic_supports_op(struct spi_mem *mem,
				       const struct spi_mem_op *op)
{
	if (op->cmd.buswidth > 1 || op->addr.buswidth > 1 ||
	    op->dummy.buswidth > 1 || op->data.buswidth > 1)
		return false;

	if (op->cmd.dtr || op->addr.dtr || op->dummy.dtr || op->data.dtr)
		return false;

	if (op->addr.nbytes > 4 || op->dummy.nbytes > 8)
		return false;

	return true;
}

static bool rtl8197f_spic_is_write_or_erase(const struct spi_mem_op *op)
{
	switch (op->cmd.opcode) {
	case SPINOR_OP_WREN:
	case SPINOR_OP_WRDI:
		return false;
	case SPINOR_OP_PP:
	case SPINOR_OP_BE_4K:
	case SPINOR_OP_SE:
	case SPINOR_OP_CHIP_ERASE:
		return true;
	default:
		return op->data.dir == SPI_MEM_DATA_OUT;
	}
}

static bool rtl8197f_spic_can_automap(const struct spi_mem_op *op)
{
	return op->data.dir == SPI_MEM_DATA_IN && op->addr.nbytes &&
	       (op->cmd.opcode == SPINOR_OP_READ ||
		op->cmd.opcode == SPINOR_OP_FAST_READ ||
		op->cmd.opcode == SPINOR_OP_READ_4B ||
		op->cmd.opcode == SPINOR_OP_FAST_READ_4B);
}

static int rtl8197f_spic_exec_rx_fifo_once(struct rtl8197f_spic *spic,
					   struct spi_mem *mem,
					   const struct spi_mem_op *op,
					   u32 addr, u8 *buf, unsigned int len)
{
	unsigned int copied = 0;
	int ret;

	rtl8197f_spic_prepare(spic, mem->spi, RTL8197F_CTRLR0_TMOD_RX);
	spic_write(spic, RTL8197F_SPIC_CTRLR1, len);
	rtl8197f_spic_push_cmd_addr(spic, op, addr);

	/* Dummy cycles are not pushed as FIFO bytes in the SDK; auto_length handles them. */
	rtl8197f_spic_enable(spic);
	ret = rtl8197f_spic_wait_not_busy(spic, "rx");
	if (ret)
		return ret;

	while (copied < len) {
		u32 data;
		unsigned int n = min_t(unsigned int, 4, len - copied);

		ret = rtl8197f_spic_wait_rx(spic);
		if (ret)
			return ret;
		data = readl(spic->base + RTL8197F_SPIC_DR);
		memcpy(buf + copied, &data, n);
		copied += n;
	}

	return 0;
}

static int rtl8197f_spic_exec_rx(struct rtl8197f_spic *spic, struct spi_mem *mem,
				 const struct spi_mem_op *op)
{
	u8 *buf = op->data.buf.in;
	unsigned int len = op->data.nbytes;
	u64 addr = op->addr.val;
	int ret;

	if (!len)
		return 0;

	if (!rtl8197f_spic_can_automap(op))
		dev_info(spic->dev, "spi-mem rx cmd=0x%02x addr_n=%u dummy=%u len=%u\n",
			op->cmd.opcode, op->addr.nbytes, op->dummy.nbytes, len);

	if (spic->automap && rtl8197f_spic_can_automap(op)) {
		if (addr > spic->automap_size || len > spic->automap_size - addr)
			return -EINVAL;
		memcpy_fromio(buf, spic->automap + addr, len);
		return 0;
	}

	/* SDK user mode is FIFO based; reissue long reads in 64-byte chunks. */
	while (len) {
		unsigned int chunk = min_t(unsigned int, len, RTL8197F_FIFO_DEPTH);

		ret = rtl8197f_spic_exec_rx_fifo_once(spic, mem, op, addr, buf, chunk);
		if (ret)
			return ret;

		buf += chunk;
		len -= chunk;
		if (op->addr.nbytes)
			addr += chunk;
	}

	return 0;
}

static int rtl8197f_spic_exec_tx(struct rtl8197f_spic *spic, struct spi_mem *mem,
				 const struct spi_mem_op *op)
{
	const u8 *buf = op->data.buf.out;
	unsigned int len = op->data.nbytes;
	unsigned int done = 0;
	int ret;

	if (rtl8197f_spic_is_write_or_erase(op) && !spic->allow_writes)
		return -EROFS;

	if (len > RTL8197F_FIFO_DEPTH)
		return -EOPNOTSUPP;

	rtl8197f_spic_prepare(spic, mem->spi, RTL8197F_CTRLR0_TMOD_TX);
	rtl8197f_spic_push_cmd_addr(spic, op, op->addr.val);

	while (done < len) {
		u32 data = 0;
		unsigned int n = min_t(unsigned int, 4, len - done);

		memcpy(&data, buf + done, n);
		if (n == 1)
			spic_write_dr8(spic, 0, data);
		else if (n == 2)
			spic_write_dr16(spic, 0, data);
		else if (n == 3) {
			spic_write_dr16(spic, 0, data & 0xffff);
			spic_write_dr8(spic, 0, data >> 16);
		} else
			spic_write_dr32(spic, 0, data);
		done += n;
	}

	rtl8197f_spic_enable(spic);
	ret = rtl8197f_spic_wait_not_busy(spic, "tx");
	return ret;
}

static int rtl8197f_spic_exec_op(struct spi_mem *mem, const struct spi_mem_op *op)
{
	struct rtl8197f_spic *spic = spi_controller_get_devdata(mem->spi->controller);
	int ret;

	if (!rtl8197f_spic_supports_op(mem, op))
		return -EOPNOTSUPP;

	if (op->data.dir != SPI_MEM_DATA_IN || !rtl8197f_spic_can_automap(op))
		dev_info(spic->dev, "spi-mem op cmd=0x%02x dir=%u addr_n=%u dummy=%u len=%u\n",
			op->cmd.opcode, op->data.dir, op->addr.nbytes,
			op->dummy.nbytes, op->data.nbytes);

	switch (op->data.dir) {
	case SPI_MEM_DATA_IN:
		ret = rtl8197f_spic_exec_rx(spic, mem, op);
		break;
	case SPI_MEM_DATA_OUT:
		ret = rtl8197f_spic_exec_tx(spic, mem, op);
		break;
	case SPI_MEM_NO_DATA:
		ret = rtl8197f_spic_exec_tx(spic, mem, op);
		break;
	default:
		ret = -EINVAL;
	}

	if (ret)
		dev_dbg(spic->dev, "op 0x%02x dir %u len %u addr 0x%llx failed: %d\n",
			op->cmd.opcode, op->data.dir, op->data.nbytes,
			op->addr.val, ret);
	return ret;
}

static const struct spi_controller_mem_ops rtl8197f_spic_mem_ops = {
	.supports_op = rtl8197f_spic_supports_op,
	.exec_op = rtl8197f_spic_exec_op,
};

static int rtl8197f_spic_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct spi_controller *ctlr;
	struct rtl8197f_spic *spic;
	u32 automap[2];
	int ret;

	ctlr = devm_spi_alloc_host(dev, sizeof(*spic));
	if (!ctlr)
		return -ENOMEM;

	spic = spi_controller_get_devdata(ctlr);
	spic->dev = dev;
	spic->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(spic->base))
		return PTR_ERR(spic->base);

	spic->clk = devm_clk_get_optional_enabled(dev, NULL);
	if (IS_ERR(spic->clk))
		return PTR_ERR(spic->clk);
	spic->oc_hz = spic->clk ? clk_get_rate(spic->clk) / 2 : RTL8197F_DEFAULT_OCCLK;
	if (!spic->oc_hz)
		spic->oc_hz = RTL8197F_DEFAULT_OCCLK;

	if (!of_property_read_u32_array(dev->of_node, "realtek,auto-map", automap, 2)) {
		spic->automap = devm_ioremap(dev, automap[0], automap[1]);
		if (!spic->automap)
			return -ENOMEM;
		spic->automap_size = automap[1];
	}

	spic->allow_writes = of_property_read_bool(dev->of_node, "realtek,allow-writes");
	if (!spic->allow_writes)
		dev_warn(dev, "flash write/erase path disabled; add realtek,allow-writes after hardware validation\n");

	ctlr->dev.of_node = dev->of_node;
	ctlr->bus_num = -1;
	ctlr->num_chipselect = 2;
	ctlr->mode_bits = SPI_CPOL | SPI_CPHA;
	ctlr->mem_ops = &rtl8197f_spic_mem_ops;
	ctlr->bits_per_word_mask = SPI_BPW_MASK(8);

	platform_set_drvdata(pdev, ctlr);

	rtl8197f_spic_disable(spic);
	spic->ctrlr0_boot = spic_read(spic, RTL8197F_SPIC_CTRLR0);
	spic_write(spic, RTL8197F_SPIC_ADDR_LENGTH, 3);
	spic_write(spic, RTL8197F_SPIC_AUTO_LENGTH,
		   (spic_read(spic, RTL8197F_SPIC_AUTO_LENGTH) & ~GENMASK(17, 16)) |
		   FIELD_PREP(GENMASK(17, 16), 3));
	spic_write(spic, RTL8197F_SPIC_IMR, 0);
	spic_write(spic, RTL8197F_SPIC_ICR, 0xffffffff);
	spic_write(spic, RTL8197F_SPIC_BAUDR, RTL8197F_BOOT_BAUD_DIV);
	spic_write(spic, RTL8197F_SPIC_FBAUDR, RTL8197F_BOOT_BAUD_DIV);
	spic_write(spic, RTL8197F_SPIC_SER, 1);

	dev_info(dev, "RTL8197F SHEIPA registering controller, ctrlr0=0x%08x, oc=%uHz%s\n",
		 spic->ctrlr0_boot, spic->oc_hz,
		 spic->automap ? ", auto-read window enabled" : "");

	ret = devm_spi_register_controller(dev, ctlr);
	if (ret)
		return ret;

	dev_info(dev, "RTL8197F SHEIPA SPI controller registered, ctrlr0=0x%08x, oc=%uHz%s\n",
		 spic->ctrlr0_boot, spic->oc_hz,
		 spic->automap ? ", auto-read window enabled" : "");
	return 0;
}

static const struct of_device_id rtl8197f_spic_of_match[] = {
	{ .compatible = "realtek,rtl8197f-spi-sheipa" },
	{ .compatible = "realtek,rtl8197fh-spi-sheipa" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_spic_of_match);

static struct platform_driver rtl8197f_spic_driver = {
	.probe = rtl8197f_spic_probe,
	.driver = {
		.name = "spi-rtl8197f-sheipa",
		.of_match_table = rtl8197f_spic_of_match,
	},
};
module_platform_driver(rtl8197f_spic_driver);

MODULE_DESCRIPTION("Realtek RTL8197F SHEIPA SPI-MEM controller");
MODULE_AUTHOR("OpenWrt RTL8197F bring-up");
MODULE_LICENSE("GPL");
