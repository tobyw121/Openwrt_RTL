/*
 * Based on m25p80.c, by Mike Lavender (mike@steroidmicros.com), with
 * influence from lart.c (Abraham Van Der Merwe) and mtd_dataflash.c
 *
 * Copyright (C) 2005, Intec Automation Inc.
 * Copyright (C) 2014, Freescale Semiconductor, Inc.
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/sizes.h>

#include <linux/mtd/mtd.h>
#include <linux/of_platform.h>
#include <linux/spi/flash.h>
#include <linux/mtd/spi-nor.h>

#include <linux/slab.h>

/* Define max times to check status register before we give up. */

/*
 * For everything but full-chip erase; probably could be much smaller, but kept
 * around for safety for now
 */
#define DEFAULT_READY_WAIT_JIFFIES		(40UL * HZ)

/*
 * For full-chip erase, calibrated to a 2MB flash (M25P16); should be scaled up
 * for larger flash
 */
#define CHIP_ERASE_2MB_READY_WAIT_JIFFIES	(40UL * HZ)

#define SPI_NOR_MAX_ID_LEN	6
#define XMC_NOR_FLASH_SUPPORT

#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
/***********************************************************************/
/* Extend flash opcode and used for Macronix */
#define PPX2_I		0x02		/* 1|2O dual program */
#define PPX2_II		0x02		/* 2 x I/O dual program */
#define PPX4_I		0x02		/* 1|4O quad program */
#define PPX4_II		0x38		/* 4 x I/O quad program */
#define READX2_I	0x03		/* 1|2O dual dual read*/
#define READX2_IO	0xbb		/* 2 x I/O dual read */
#define READX4_I	0x03		/* 1|4O quad read */
#define READX4_IO	0xeb		/* 4 x I/O quad read */

/* Extend flash read dummy cycles and used for Macronix only */
#define MXIC_DUAL_DUMMY_CYCLE    0x4
#define MXIC_QUAD_DUMMY_CYCLE    0x6
#define MXIC_FAST_DUMMY_CYCLE    0x8

/* ESMT */
#define ESMT_DUAL_DUMMY_CYCLE    0x4
#define ESMT_QUAD_DUMMY_CYCLE    0x6
#define ESMT_FAST_DUMMY_CYCLE    0x8

/* Winbond */
#define WINBOND_DUAL_DUMMY_CYCLE    0x4
#define WINBOND_QUAD_DUMMY_CYCLE    0x6
#define WINBOND_FAST_DUMMY_CYCLE    0x8

/* Spansion */
#define SPANSION_DUAL_DUMMY_CYCLE    0x4
#define SPANSION_QUAD_DUMMY_CYCLE    0x6
#define SPANSION_FAST_DUMMY_CYCLE    0x8

/* GigaDevice */
#define GIGADEVICE_DUAL_DUMMY_CYCLE    0x4
#define GIGADEVICE_QUAD_DUMMY_CYCLE    0x6
#define GIGADEVICE_FAST_DUMMY_CYCLE    0x8

/* EON */
#define EON_DUAL_DUMMY_CYCLE    0x4
#define EON_QUAD_DUMMY_CYCLE    0x6
#define EON_FAST_DUMMY_CYCLE    0x8

/* Micron */
#define MICRON_DUAL_DUMMY_CYCLE    0x8
#define MICRON_QUAD_DUMMY_CYCLE    0x10
#define MICRON_FAST_DUMMY_CYCLE    0x8

/* Xmc */
#define XMC_DUAL_DUMMY_CYCLE    0x4
#define XMC_QUAD_DUMMY_CYCLE    0x6
#define XMC_FAST_DUMMY_CYCLE    0x8

#define TP_FLASH_IOCTL
#define TP_IPF_FLASH_IOCTL

struct mtd_info *mtd_spi = NULL;

/****************************************************************************/
/* Extend flash multi-channel read/write type */
enum m25p80_rd_multi_type {
	RD_MULTI_NONE = 0x00,
	RD_DUAL_O     = 0x01,
	RD_DUAL_IO    = 0x02,
	RD_QUAD_O     = 0x03,
	RD_QUAD_IO    = 0x04
};

enum m25p80_wr_multi_type {
	WR_MULTI_NONE = 0x00,
	WR_DUAL_I     = 0x01,
	WR_DUAL_II    = 0x02,
	WR_QUAD_I     = 0x03,
	WR_QUAD_II    = 0x04
};

struct flash_cmd {
	volatile uint8_t ppx2_i;	/* flash_cmd; write dual channels */
	volatile uint8_t ppx2_ii;	/* flash_cmd; write dual channels */
	volatile uint8_t ppx4_i;	/* flash_cmd; wirte quad channels */
	volatile uint8_t ppx4_ii;	/* flash_cmd; wirte quad channels */
	volatile uint8_t readx2_o;	/* flash_cmd; read  dual channels */
	volatile uint8_t readx2_io;	/* flash_cmd; read  dual channels */
	volatile uint8_t readx4_o;	/* flash_cmd; read  quad channels */
	volatile uint8_t readx4_io;	/* flash_cmd; read  quad channels */
};

struct flash_dummy_cycles_info {
	uint32_t  rd_dual_dummy;
	uint32_t  rd_quad_dummy;
	uint32_t  fast_rd_dummy;
};

struct flash_rw_multi_type_info {
	enum m25p80_rd_multi_type rd_dual_type;
	enum m25p80_rd_multi_type rd_quad_type;
	enum m25p80_wr_multi_type wr_dual_type;
	enum m25p80_wr_multi_type wr_quad_type;
};

struct flash_vendor_info {
	uint8_t	flash_id;
	char	vendor_name[16];
	struct	flash_cmd *cmd;
	struct	flash_dummy_cycles_info *dummy;
	struct	flash_rw_multi_type_info *type;
};
/* Flash device cmd,
 * If you want to add new flash vendor
 * Please add new flash vendor command
 */
struct flash_cmd mxic_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

struct flash_cmd esmt_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

struct flash_cmd winbond_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

struct flash_cmd spansion_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

struct flash_cmd gigadevice_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

struct flash_cmd eon_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

struct flash_cmd micron_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

struct flash_cmd xmc_cmd = {
	PPX2_I,
	PPX2_II,
	PPX4_I,
	PPX4_II,
	READX2_I,
	READX2_IO,
	READX4_I,
	READX4_IO,
};

/* Flash device dummy info,
 * If you want to add new flash vendor
 * Please add new flash vendor dummy
 */
struct flash_dummy_cycles_info	mxic_dummy_cycles_info = {
	MXIC_DUAL_DUMMY_CYCLE,
	MXIC_QUAD_DUMMY_CYCLE,
	MXIC_FAST_DUMMY_CYCLE,
};

struct flash_dummy_cycles_info	esmt_dummy_cycles_info = {
	ESMT_DUAL_DUMMY_CYCLE,
	ESMT_QUAD_DUMMY_CYCLE,
	ESMT_FAST_DUMMY_CYCLE,
};

struct flash_dummy_cycles_info	winbond_dummy_cycles_info = {
	WINBOND_DUAL_DUMMY_CYCLE,
	WINBOND_QUAD_DUMMY_CYCLE,
	WINBOND_FAST_DUMMY_CYCLE,
};

struct flash_dummy_cycles_info	spansion_dummy_cycles_info = {
	SPANSION_DUAL_DUMMY_CYCLE,
	SPANSION_QUAD_DUMMY_CYCLE,
	SPANSION_FAST_DUMMY_CYCLE,
};

struct flash_dummy_cycles_info	gigadevice_dummy_cycles_info = {
	GIGADEVICE_DUAL_DUMMY_CYCLE,
	GIGADEVICE_QUAD_DUMMY_CYCLE,
	GIGADEVICE_FAST_DUMMY_CYCLE,
};

struct flash_dummy_cycles_info	eon_dummy_cycles_info = {
	EON_DUAL_DUMMY_CYCLE,
	EON_QUAD_DUMMY_CYCLE,
	EON_FAST_DUMMY_CYCLE,
};

struct flash_dummy_cycles_info	micron_dummy_cycles_info = {
	MICRON_DUAL_DUMMY_CYCLE,
	MICRON_QUAD_DUMMY_CYCLE,
	MICRON_FAST_DUMMY_CYCLE,
};

struct flash_dummy_cycles_info	xmc_dummy_cycles_info = {
	XMC_DUAL_DUMMY_CYCLE,
	XMC_QUAD_DUMMY_CYCLE,
	XMC_FAST_DUMMY_CYCLE,
};

/* Flash device read/write multi type,
 * If you want to add new flash vendor
 * Please add new flash vendor dummy
 */
struct flash_rw_multi_type_info mxic_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

struct flash_rw_multi_type_info esmt_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

struct flash_rw_multi_type_info winbond_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

struct flash_rw_multi_type_info spansion_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

struct flash_rw_multi_type_info gigadevice_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

struct flash_rw_multi_type_info eon_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

struct flash_rw_multi_type_info micron_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

struct flash_rw_multi_type_info xmc_rw_multi_type_info = {
	RD_DUAL_IO,
	RD_QUAD_IO,
	WR_MULTI_NONE,
	WR_QUAD_II
};

/*-----------------------------------------------------------------*/

/* Add new flash vendor
 * format (FlashID, vendor name, flash cmd,
 * flash dummy cycle and read/write multi type),
 * if you want to add new flash
 * vendor, please add here.
 */
struct flash_vendor_info info_mxic = {
	0xC2,
	"MXIC",
	&mxic_cmd,
	&mxic_dummy_cycles_info,
	&mxic_rw_multi_type_info
};

struct flash_vendor_info info_esmt = {
	0x8C,
	"ESMT",
	&esmt_cmd,
	&esmt_dummy_cycles_info,
	&esmt_rw_multi_type_info
};

struct flash_vendor_info info_winbond = {
	0xEF,
	"Winbond",
	&winbond_cmd,
	&winbond_dummy_cycles_info,
	&winbond_rw_multi_type_info
};

struct flash_vendor_info info_spansion = {
	0x01,
	"Spansion",
	&spansion_cmd,
	&spansion_dummy_cycles_info,
	&spansion_rw_multi_type_info
};

struct flash_vendor_info info_gigadevice = {
	0xC8,
	"GigaDevice",
	&gigadevice_cmd,
	&gigadevice_dummy_cycles_info,
	&gigadevice_rw_multi_type_info
};

struct flash_vendor_info info_eon = {
	0x1C,
	"EON",
	&eon_cmd,
	&eon_dummy_cycles_info,
	&eon_rw_multi_type_info
};

#ifndef XMC_NOR_FLASH_SUPPORT
struct flash_vendor_info info_micron = {
	0x20,
	"Micron",
	&micron_cmd,
	&micron_dummy_cycles_info,
	&micron_rw_multi_type_info
};
#else
struct flash_vendor_info info_xmc = {
	0x20,
	"Xmc",
	&xmc_cmd,
	&xmc_dummy_cycles_info,
	&xmc_rw_multi_type_info
};
#endif

/* Flash vendors eg: MXIC.... */
struct flash_device {
	struct flash_vendor_info *vendors[7];
};

#ifdef XMC_NOR_FLASH_SUPPORT
struct flash_device device = {
	{  &info_mxic, &info_esmt, &info_winbond, &info_spansion, &info_gigadevice, &info_eon, &info_xmc}
};
#else
struct flash_device device = {
	{  &info_mxic, &info_esmt, &info_winbond, &info_spansion, &info_gigadevice, &info_eon, &info_micron }
};
#endif

/* chip max clk */
static u32 chip_max_speed_hz = 0;
/***********************************************************************/
#endif /* defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA) */

struct flash_info {
	char		*name;

	/*
	 * This array stores the ID bytes.
	 * The first three bytes are the JEDIC ID.
	 * JEDEC ID zero means "no ID" (mostly older chips).
	 */
	u8		id[SPI_NOR_MAX_ID_LEN];
	u8		id_len;

	/* The size listed here is what works with SPINOR_OP_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	sector_size;
	u16		n_sectors;

	u16		page_size;
	u16		addr_width;

	u16		flags;
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	u32		normal_clk;
	u32		fastrd_clk;
	u32		ex_flags;
#endif
#define	SECT_4K			0x01	/* SPINOR_OP_BE_4K works uniformly */
#define	SPI_NOR_NO_ERASE	0x02	/* No erase command needed */
#define	SST_WRITE		0x04	/* use SST byte programming */
#define	SPI_NOR_NO_FR		0x08	/* Can't do fastread */
#define	SECT_4K_PMC		0x10	/* SPINOR_OP_BE_4K_PMC works uniformly */
#define	SPI_NOR_DUAL_READ	0x20    /* Flash supports Dual Read */
#define	SPI_NOR_QUAD_READ	0x40    /* Flash supports Quad Read */
#define	USE_FSR			0x80	/* use flag status register */
};

#define JEDEC_MFR(info)	((info)->id[0])

static const struct flash_info *spi_nor_match_id(const char *name);

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static int read_sr(struct spi_nor *nor)
{
	int ret;
	u8 val;

	ret = nor->read_reg(nor, SPINOR_OP_RDSR, &val, 1);
	if (ret < 0) {
		pr_err("error %d reading SR\n", (int) ret);
		return ret;
	}

	return val;
}

/*
 * Read the flag status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static int read_fsr(struct spi_nor *nor)
{
	int ret;
	u8 val;

	ret = nor->read_reg(nor, SPINOR_OP_RDFSR, &val, 1);
	if (ret < 0) {
		pr_err("error %d reading FSR\n", ret);
		return ret;
	}

	return val;
}

/*
 * Read configuration register, returning its value in the
 * location. Return the configuration register value.
 * Returns negative if error occured.
 */
static int read_cr(struct spi_nor *nor)
{
	int ret;
	u8 val;

	ret = nor->read_reg(nor, SPINOR_OP_RDCR, &val, 1);
	if (ret < 0) {
		dev_err(nor->dev, "error %d reading CR\n", ret);
		return ret;
	}

	return val;
}

/*
 * Dummy Cycle calculation for different type of read.
 * It can be used to support more commands with
 * different dummy cycle requirements.
 */
static inline int spi_nor_read_dummy_cycles(struct spi_nor *nor)
{
	switch (nor->flash_read) {
	case SPI_NOR_FAST:
	case SPI_NOR_DUAL:
	case SPI_NOR_QUAD:
		return 8;
	case SPI_NOR_NORMAL:
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	case SPI_NOR_AUTO:
#endif
		return 0;
	}
	return 0;
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
static inline int write_sr(struct spi_nor *nor, u8 val)
{
	nor->cmd_buf[0] = val;
	return nor->write_reg(nor, SPINOR_OP_WRSR, nor->cmd_buf, 1);
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(struct spi_nor *nor)
{
	return nor->write_reg(nor, SPINOR_OP_WREN, NULL, 0);
}

/*
 * Send write disble instruction to the chip.
 */
static inline int write_disable(struct spi_nor *nor)
{
	return nor->write_reg(nor, SPINOR_OP_WRDI, NULL, 0);
}

static inline struct spi_nor *mtd_to_spi_nor(struct mtd_info *mtd)
{
	return mtd->priv;
}

/* Enable/disable 4-byte addressing mode. */
static inline int set_4byte(struct spi_nor *nor, const struct flash_info *info,
			    int enable)
{
	int status;
	bool need_wren = false;
	u8 cmd;

	switch (JEDEC_MFR(info)) {
	case SNOR_MFR_MICRON:
		/* Some Micron need WREN command; all will accept it */
		need_wren = true;
	case SNOR_MFR_MACRONIX:
	case SNOR_MFR_WINBOND:
		if (need_wren)
			write_enable(nor);

		cmd = enable ? SPINOR_OP_EN4B : SPINOR_OP_EX4B;
		status = nor->write_reg(nor, cmd, NULL, 0);
		if (need_wren)
			write_disable(nor);

		return status;
	default:
		/* Spansion style */
		nor->cmd_buf[0] = enable << 7;
		return nor->write_reg(nor, SPINOR_OP_BRWR, nor->cmd_buf, 1);
	}
}
static inline int spi_nor_sr_ready(struct spi_nor *nor)
{
	int sr = read_sr(nor);
	if (sr < 0)
		return sr;
	else
		return !(sr & SR_WIP);
}

static inline int spi_nor_fsr_ready(struct spi_nor *nor)
{
	int fsr = read_fsr(nor);
	if (fsr < 0)
		return fsr;
	else
		return fsr & FSR_READY;
}

static int spi_nor_ready(struct spi_nor *nor)
{
	int sr, fsr;
	sr = spi_nor_sr_ready(nor);
	if (sr < 0)
		return sr;
	fsr = nor->flags & SNOR_F_USE_FSR ? spi_nor_fsr_ready(nor) : 1;
	if (fsr < 0)
		return fsr;
	return sr && fsr;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int spi_nor_wait_till_ready_with_timeout(struct spi_nor *nor,
						unsigned long timeout_jiffies)
{
	unsigned long deadline;
	int timeout = 0, ret;

	deadline = jiffies + timeout_jiffies;

	while (!timeout) {
		if (time_after_eq(jiffies, deadline))
			timeout = 1;

		ret = spi_nor_ready(nor);
		if (ret < 0)
			return ret;
		if (ret)
			return 0;

		cond_resched();
	}

	dev_err(nor->dev, "flash operation timed out\n");

	return -ETIMEDOUT;
}

static int spi_nor_wait_till_ready(struct spi_nor *nor)
{
	return spi_nor_wait_till_ready_with_timeout(nor,
						    DEFAULT_READY_WAIT_JIFFIES);
}

/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_chip(struct spi_nor *nor)
{
	dev_dbg(nor->dev, " %lldKiB\n", (long long)(nor->mtd.size >> 10));

	return nor->write_reg(nor, SPINOR_OP_CHIP_ERASE, NULL, 0);
}

static int spi_nor_lock_and_prep(struct spi_nor *nor, enum spi_nor_ops ops)
{
	int ret = 0;

	mutex_lock(&nor->lock);

	if (nor->prepare) {
		ret = nor->prepare(nor, ops);
		if (ret) {
			dev_err(nor->dev, "failed in the preparation.\n");
			mutex_unlock(&nor->lock);
			return ret;
		}
	}
	return ret;
}

static void spi_nor_unlock_and_unprep(struct spi_nor *nor, enum spi_nor_ops ops)
{
	if (nor->unprepare)
		nor->unprepare(nor, ops);
	mutex_unlock(&nor->lock);
}

/*
 * Erase an address range on the nor chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int spi_nor_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	u32 addr, len;
	uint32_t rem;
	int ret;

	dev_dbg(nor->dev, "at 0x%llx, len %lld\n", (long long)instr->addr,
			(long long)instr->len);

	div_u64_rem(instr->len, mtd->erasesize, &rem);
	if (rem)
		return -EINVAL;

	addr = instr->addr;
	len = instr->len;

	ret = spi_nor_lock_and_prep(nor, SPI_NOR_OPS_ERASE);
	if (ret)
		return ret;

	/* whole-chip erase? */
	if (len == mtd->size) {
		unsigned long timeout;

		write_enable(nor);

		if (erase_chip(nor)) {
			ret = -EIO;
			goto erase_err;
		}

		/*
		 * Scale the timeout linearly with the size of the flash, with
		 * a minimum calibrated to an old 2MB flash. We could try to
		 * pull these from CFI/SFDP, but these values should be good
		 * enough for now.
		 */
		timeout = max(CHIP_ERASE_2MB_READY_WAIT_JIFFIES,
			      CHIP_ERASE_2MB_READY_WAIT_JIFFIES *
			      (unsigned long)(mtd->size / SZ_2M));
		ret = spi_nor_wait_till_ready_with_timeout(nor, timeout);
		if (ret)
			goto erase_err;

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using SPINOR_OP_SE instead of SPINOR_OP_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} else {
		while (len) {
			write_enable(nor);

			if (nor->erase(nor, addr)) {
				ret = -EIO;
				goto erase_err;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;

			ret = spi_nor_wait_till_ready(nor);
			if (ret)
				goto erase_err;
		}
	}

	write_disable(nor);

	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_ERASE);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return ret;

erase_err:
	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_ERASE);
	instr->state = MTD_ERASE_FAILED;
	return ret;
}

static void stm_get_locked_range(struct spi_nor *nor, u8 sr, loff_t *ofs,
				 uint64_t *len)
{
	struct mtd_info *mtd = &nor->mtd;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	int shift = ffs(mask) - 1;
	int pow;

	if (!(sr & mask)) {
		/* No protection */
		*ofs = 0;
		*len = 0;
	} else {
		pow = ((sr & mask) ^ mask) >> shift;
		*len = mtd->size >> pow;
		*ofs = mtd->size - *len;
	}
}

/*
 * Return 1 if the entire region is locked, 0 otherwise
 */
static int stm_is_locked_sr(struct spi_nor *nor, loff_t ofs, uint64_t len,
			    u8 sr)
{
	loff_t lock_offs;
	uint64_t lock_len;

	stm_get_locked_range(nor, sr, &lock_offs, &lock_len);

	return (ofs + len <= lock_offs + lock_len) && (ofs >= lock_offs);
}

/*
 * Lock a region of the flash. Compatible with ST Micro and similar flash.
 * Supports only the block protection bits BP{0,1,2} in the status register
 * (SR). Does not support these features found in newer SR bitfields:
 *   - TB: top/bottom protect - only handle TB=0 (top protect)
 *   - SEC: sector/block protect - only handle SEC=0 (block protect)
 *   - CMP: complement protect - only support CMP=0 (range is not complemented)
 *
 * Sample table portion for 8MB flash (Winbond w25q64fw):
 *
 *   SEC  |  TB   |  BP2  |  BP1  |  BP0  |  Prot Length  | Protected Portion
 *  --------------------------------------------------------------------------
 *    X   |   X   |   0   |   0   |   0   |  NONE         | NONE
 *    0   |   0   |   0   |   0   |   1   |  128 KB       | Upper 1/64
 *    0   |   0   |   0   |   1   |   0   |  256 KB       | Upper 1/32
 *    0   |   0   |   0   |   1   |   1   |  512 KB       | Upper 1/16
 *    0   |   0   |   1   |   0   |   0   |  1 MB         | Upper 1/8
 *    0   |   0   |   1   |   0   |   1   |  2 MB         | Upper 1/4
 *    0   |   0   |   1   |   1   |   0   |  4 MB         | Upper 1/2
 *    X   |   X   |   1   |   1   |   1   |  8 MB         | ALL
 *
 * Returns negative on errors, 0 on success.
 */
static int stm_lock(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	struct mtd_info *mtd = &nor->mtd;
	u8 status_old, status_new;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	u8 shift = ffs(mask) - 1, pow, val;

	status_old = read_sr(nor);

	/* SPI NOR always locks to the end */
	if (ofs + len != mtd->size) {
		/* Does combined region extend to end? */
		if (!stm_is_locked_sr(nor, ofs + len, mtd->size - ofs - len,
				      status_old))
			return -EINVAL;
		len = mtd->size - ofs;
	}

	/*
	 * Need smallest pow such that:
	 *
	 *   1 / (2^pow) <= (len / size)
	 *
	 * so (assuming power-of-2 size) we do:
	 *
	 *   pow = ceil(log2(size / len)) = log2(size) - floor(log2(len))
	 */
	pow = ilog2(mtd->size) - ilog2(len);
	val = mask - (pow << shift);
	if (val & ~mask)
		return -EINVAL;
	/* Don't "lock" with no region! */
	if (!(val & mask))
		return -EINVAL;

	status_new = (status_old & ~mask) | val;

	/* Only modify protection if it will not unlock other areas */
	if ((status_new & mask) <= (status_old & mask))
		return -EINVAL;

	write_enable(nor);
	return write_sr(nor, status_new);
}

/*
 * Unlock a region of the flash. See stm_lock() for more info
 *
 * Returns negative on errors, 0 on success.
 */
static int stm_unlock(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	struct mtd_info *mtd = &nor->mtd;
	uint8_t status_old, status_new;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	u8 shift = ffs(mask) - 1, pow, val;

	status_old = read_sr(nor);

	/* Cannot unlock; would unlock larger region than requested */
	if (stm_is_locked_sr(nor, ofs - mtd->erasesize, mtd->erasesize,
			     status_old))
		return -EINVAL;

	/*
	 * Need largest pow such that:
	 *
	 *   1 / (2^pow) >= (len / size)
	 *
	 * so (assuming power-of-2 size) we do:
	 *
	 *   pow = floor(log2(size / len)) = log2(size) - ceil(log2(len))
	 */
	pow = ilog2(mtd->size) - order_base_2(mtd->size - (ofs + len));
	if (ofs + len == mtd->size) {
		val = 0; /* fully unlocked */
	} else {
		val = mask - (pow << shift);
		/* Some power-of-two sizes are not supported */
		if (val & ~mask)
			return -EINVAL;
	}

	status_new = (status_old & ~mask) | val;

	/* Only modify protection if it will not lock other areas */
	if ((status_new & mask) >= (status_old & mask))
		return -EINVAL;

	write_enable(nor);
	return write_sr(nor, status_new);
}

/*
 * Check if a region of the flash is (completely) locked. See stm_lock() for
 * more info.
 *
 * Returns 1 if entire region is locked, 0 if any portion is unlocked, and
 * negative on errors.
 */
static int stm_is_locked(struct spi_nor *nor, loff_t ofs, uint64_t len)
{
	int status;

	status = read_sr(nor);
	if (status < 0)
		return status;

	return stm_is_locked_sr(nor, ofs, len, status);
}

static int spi_nor_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	int ret;

	ret = spi_nor_lock_and_prep(nor, SPI_NOR_OPS_LOCK);
	if (ret)
		return ret;

	ret = nor->flash_lock(nor, ofs, len);

	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_UNLOCK);
	return ret;
}

static int spi_nor_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	int ret;

	ret = spi_nor_lock_and_prep(nor, SPI_NOR_OPS_UNLOCK);
	if (ret)
		return ret;

	ret = nor->flash_unlock(nor, ofs, len);

	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_LOCK);
	return ret;
}

static int spi_nor_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	int ret;

	ret = spi_nor_lock_and_prep(nor, SPI_NOR_OPS_UNLOCK);
	if (ret)
		return ret;

	ret = nor->flash_is_locked(nor, ofs, len);

	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_LOCK);
	return ret;
}

#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
#define FLASH_VERIFIED	0x01	/* Flash has been verified */
#define USE_4B_READ		0x02	/* use 4-byte-address read command */
#endif

/* Used when the "_ext_id" is two bytes at most */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
#define INFO(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags,_normal_clk,_fastrd_clk,_ex_flags)	\
		.id = {							\
			((_jedec_id) >> 16) & 0xff,			\
			((_jedec_id) >> 8) & 0xff,			\
			(_jedec_id) & 0xff,				\
			((_ext_id) >> 8) & 0xff,			\
			(_ext_id) & 0xff,				\
			},						\
		.id_len = (!(_jedec_id) ? 0 : (3 + ((_ext_id) ? 2 : 0))),	\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),					\
		.normal_clk = (_normal_clk),		\
		.fastrd_clk = (_fastrd_clk),		\
		.ex_flags = (_ex_flags),			\

#define INFO6(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags,_normal_clk,_fastrd_clk,_ex_flags)	\
		.id = {							\
			((_jedec_id) >> 16) & 0xff,			\
			((_jedec_id) >> 8) & 0xff,			\
			(_jedec_id) & 0xff,				\
			((_ext_id) >> 16) & 0xff,			\
			((_ext_id) >> 8) & 0xff,			\
			(_ext_id) & 0xff,				\
			},						\
		.id_len = 6,						\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),					\
		.normal_clk = (_normal_clk),		\
		.fastrd_clk = (_fastrd_clk),		\
		.ex_flags = (_ex_flags),			\

#define CAT25_INFO(_sector_size, _n_sectors, _page_size, _addr_width, _flags,_normal_clk,_fastrd_clk,_ex_flags)	\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = (_page_size),				\
		.addr_width = (_addr_width),				\
		.flags = (_flags),					\
		.normal_clk = (_normal_clk),		\
		.fastrd_clk = (_fastrd_clk),		\
		.ex_flags = (_ex_flags),

#else
#define INFO(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)	\
		.id = {							\
			((_jedec_id) >> 16) & 0xff,			\
			((_jedec_id) >> 8) & 0xff,			\
			(_jedec_id) & 0xff,				\
			((_ext_id) >> 8) & 0xff,			\
			(_ext_id) & 0xff,				\
			},						\
		.id_len = (!(_jedec_id) ? 0 : (3 + ((_ext_id) ? 2 : 0))),	\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),

#define INFO6(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)	\
		.id = {							\
			((_jedec_id) >> 16) & 0xff,			\
			((_jedec_id) >> 8) & 0xff,			\
			(_jedec_id) & 0xff,				\
			((_ext_id) >> 16) & 0xff,			\
			((_ext_id) >> 8) & 0xff,			\
			(_ext_id) & 0xff,				\
			},						\
		.id_len = 6,						\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),

#define CAT25_INFO(_sector_size, _n_sectors, _page_size, _addr_width, _flags)	\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = (_page_size),				\
		.addr_width = (_addr_width),				\
		.flags = (_flags),

#endif /* defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA) */

/* NOTE: double check command sets and memory organization when you add
 * more nor chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 *
 * All newly added entries should describe *hardware* and should use SECT_4K
 * (or SECT_4K_PMC) if hardware supports erasing 4 KiB sectors. For usage
 * scenarios excluding small sectors there is config option that can be
 * disabled: CONFIG_MTD_SPI_NOR_USE_4K_SECTORS.
 * For historical (and compatibility) reasons (before we got above config) some
 * old entries may be missing 4K flag.
 */
static const struct flash_info spi_nor_ids[] = {
	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "at25fs010",  INFO(0x1f6601, 0, 32 * 1024,   4, SECT_4K, 41000000, 41000000, 0x0) },
	{ "at25fs040",  INFO(0x1f6604, 0, 64 * 1024,   8, SECT_4K, 41000000, 41000000, 0x0) },

	{ "at25df041a", INFO(0x1f4401, 0, 64 * 1024,   8, SECT_4K, 29000000, 41000000, 0x0) },
	{ "at25df321a", INFO(0x1f4701, 0, 64 * 1024,  64, SECT_4K, 41000000, 75000000, 0x0) },
	{ "at25df641",  INFO(0x1f4800, 0, 64 * 1024, 128, SECT_4K, 41000000, 75000000, 0x0) },

	{ "at26f004",   INFO(0x1f0400, 0, 64 * 1024,  8, SECT_4K, 18000000, 29000000, 0x0) },
	{ "at26df081a", INFO(0x1f4501, 0, 64 * 1024, 16, SECT_4K, 29000000, 41000000, 0x0) },
	{ "at26df161a", INFO(0x1f4601, 0, 64 * 1024, 32, SECT_4K, 29000000, 41000000, 0x0) },
	{ "at26df321",  INFO(0x1f4700, 0, 64 * 1024, 64, SECT_4K, 29000000, 41000000, 0x0) },

	{ "at45db081d", INFO(0x1f2500, 0, 64 * 1024, 16, SECT_4K, 29000000, 41000000, 0x0) },
#else
	{ "at25fs010",  INFO(0x1f6601, 0, 32 * 1024,   4, SECT_4K) },
	{ "at25fs040",  INFO(0x1f6604, 0, 64 * 1024,   8, SECT_4K) },

	{ "at25df041a", INFO(0x1f4401, 0, 64 * 1024,   8, SECT_4K) },
	{ "at25df321a", INFO(0x1f4701, 0, 64 * 1024,  64, SECT_4K) },
	{ "at25df641",  INFO(0x1f4800, 0, 64 * 1024, 128, SECT_4K) },

	{ "at26f004",   INFO(0x1f0400, 0, 64 * 1024,  8, SECT_4K) },
	{ "at26df081a", INFO(0x1f4501, 0, 64 * 1024, 16, SECT_4K) },
	{ "at26df161a", INFO(0x1f4601, 0, 64 * 1024, 32, SECT_4K) },
	{ "at26df321",  INFO(0x1f4700, 0, 64 * 1024, 64, SECT_4K) },

	{ "at45db081d", INFO(0x1f2500, 0, 64 * 1024, 16, SECT_4K) },
#endif

	/* EON -- en25xxx */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "en25f32",    INFO(0x1c3116, 0, 64 * 1024,   64, SECT_4K, 41000000, 75000000, 0x0) },
	{ "en25p32",    INFO(0x1c2016, 0, 64 * 1024,   64, 0, 41000000, 75000000, 0x0) },
	{ "en25q32b",   INFO(0x1c3016, 0, 64 * 1024,   64, 0, 41000000, 100000000, 0x0) },
	{ "en25p64",    INFO(0x1c2017, 0, 64 * 1024,  128, 0, 41000000, 75000000, 0x0) },
	{ "en25q64",    INFO(0x1c3017, 0, 64 * 1024,  128, SECT_4K, 41000000, 100000000, 0x0) },
	{ "en25qh128",  INFO(0x1c7018, 0, 64 * 1024,  256, 0,0, 0, 0x0) },
	{ "en25qh256",  INFO(0x1c7019, 0, 64 * 1024,  512, 0, 41000000, 75000000, 0x0) },
	{ "en25s64",	INFO(0x1c3817, 0, 64 * 1024,  128, SECT_4K,0, 0, 0x0) },
	{ "en25qh64a",  INFO(0x1c7017, 0, 64 * 1024,  128, 0, 75000000, 100000000, FLASH_VERIFIED) },
	{ "en25qh128a", INFO(0x1c7018, 0, 64 * 1024,  256, 0, 75000000, 100000000, FLASH_VERIFIED) },
#else
	{ "en25f32",    INFO(0x1c3116, 0, 64 * 1024,   64, SECT_4K) },
	{ "en25p32",    INFO(0x1c2016, 0, 64 * 1024,   64, 0) },
	{ "en25q32b",   INFO(0x1c3016, 0, 64 * 1024,   64, 0) },
	{ "en25p64",    INFO(0x1c2017, 0, 64 * 1024,  128, 0) },
	{ "en25q64",    INFO(0x1c3017, 0, 64 * 1024,  128, SECT_4K) },
	{ "en25qh128",  INFO(0x1c7018, 0, 64 * 1024,  256, 0) },
	{ "en25qh256",  INFO(0x1c7019, 0, 64 * 1024,  512, 0) },
	{ "en25s64",	INFO(0x1c3817, 0, 64 * 1024,  128, SECT_4K) },
#endif

	/* ESMT */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "f25l32pa", INFO(0x8c2016, 0, 64 * 1024, 64, SECT_4K, 29000000, 41000000, 0x0) },
#else
	{ "f25l32pa", INFO(0x8c2016, 0, 64 * 1024, 64, SECT_4K) },
#endif

	/* Everspin */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "mr25h256", CAT25_INFO( 32 * 1024, 1, 256, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR, 29000000, 29000000, 0x0) },
	{ "mr25h10",  CAT25_INFO(128 * 1024, 1, 256, 3, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR, 29000000, 29000000, 0x0) },
#else
	{ "mr25h256", CAT25_INFO( 32 * 1024, 1, 256, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
	{ "mr25h10",  CAT25_INFO(128 * 1024, 1, 256, 3, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
#endif

	/* Fujitsu */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "mb85rs1mt", INFO(0x047f27, 0, 128 * 1024, 1, SPI_NOR_NO_ERASE,0, 0, 0x0) },
#else
	{ "mb85rs1mt", INFO(0x047f27, 0, 128 * 1024, 1, SPI_NOR_NO_ERASE) },
#endif

	/* GigaDevice */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "gd25q32", INFO(0xc84016, 0, 64 * 1024,  64, SECT_4K, 75000000, 100000000, 0x0) },
	{ "gd25q64", INFO(0xc84017, 0, 64 * 1024, 128, SECT_4K, 41000000, 41000000, FLASH_VERIFIED) },
	{ "gd25q128", INFO(0xc84018, 0, 64 * 1024, 256, SECT_4K, 75000000, 100000000, 0x0) },
#else
	{ "gd25q32", INFO(0xc84016, 0, 64 * 1024,  64, SECT_4K) },
	{ "gd25q64", INFO(0xc84017, 0, 64 * 1024, 128, SECT_4K) },
	{ "gd25q128", INFO(0xc84018, 0, 64 * 1024, 256, SECT_4K) },
#endif

	/* Intel/Numonyx -- xxxs33b */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "160s33b",  INFO(0x898911, 0, 64 * 1024,  32, 0, 29000000, 41000000, 0x0) },
	{ "320s33b",  INFO(0x898912, 0, 64 * 1024,  64, 0, 29000000, 41000000, 0x0) },
	{ "640s33b",  INFO(0x898913, 0, 64 * 1024, 128, 0, 29000000, 41000000, 0x0) },
#else
	{ "160s33b",  INFO(0x898911, 0, 64 * 1024,  32, 0) },
	{ "320s33b",  INFO(0x898912, 0, 64 * 1024,  64, 0) },
	{ "640s33b",  INFO(0x898913, 0, 64 * 1024, 128, 0) },
#endif

	/* ISSI */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "is25cd512", INFO(0x7f9d20, 0, 32 * 1024,   2, SECT_4K,0, 0, 0x0) },
#else
	{ "is25cd512", INFO(0x7f9d20, 0, 32 * 1024,   2, SECT_4K) },
	{ "is25wp032", INFO(0x9d7016, 0, 64 * 1024,  64,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "is25wp064", INFO(0x9d7017, 0, 64 * 1024, 128,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "is25wp128", INFO(0x9d7018, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
#endif

	/* Macronix */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "mx25l512e",   INFO(0xc22010, 0, 64 * 1024,   1, SECT_4K,0, 0, 0x0) },
	{ "mx25l2005a",  INFO(0xc22012, 0, 64 * 1024,   4, SECT_4K, 29000000, 41000000, 0x0) },
	{ "mx25l4005a",  INFO(0xc22013, 0, 64 * 1024,   8, SECT_4K, 29000000, 75000000, 0x0) },
	{ "mx25l8005",   INFO(0xc22014, 0, 64 * 1024,  16, 0, 29000000, 41000000, 0x0) },
	{ "mx25l1606e",  INFO(0xc22015, 0, 64 * 1024,  32, SECT_4K, 29000000, 75000000, 0x0) },
	{ "mx25l3205d",  INFO(0xc22016, 0, 64 * 1024,  64, 0, 29000000, 41000000, FLASH_VERIFIED) },
	{ "mx25l3235d",	 INFO(0xc25e16, 0, 64 * 1024, 64, SECT_4K, 29000000, 100000000, FLASH_VERIFIED)},
	{ "mx25l3255e",  INFO(0xc29e16, 0, 64 * 1024,  64, SECT_4K, 41000000, 100000000, 0x0) },
	{ "mx25l6405d",  INFO(0xc22017, 0, 64 * 1024, 128, 0, 29000000, 41000000, FLASH_VERIFIED) },
	{ "mx25u6435f",  INFO(0xc22537, 0, 64 * 1024, 128, SECT_4K,0, 0, 0x0) },
	{ "mx25l12805d", INFO(0xc22018, 0, 64 * 1024, 256, 0, 29000000, 41000000, FLASH_VERIFIED) },
	{ "mx25l12855e", INFO(0xc22618, 0, 64 * 1024, 256, 0, 41000000, 41000000, 0x0) },
	{ "mx25l25635e", INFO(0xc22019, 0, 64 * 1024, 512, 0, 41000000, 75000000, 0x0) },
	{ "mx25l25655e", INFO(0xc22619, 0, 64 * 1024, 512, 0, 41000000, 41000000, 0x0) },
	{ "mx66l51235l", INFO(0xc2201a, 0, 64 * 1024, 1024, SPI_NOR_QUAD_READ, 41000000, 100000000,FLASH_VERIFIED) },
	{ "mx66l1g55g",  INFO(0xc2261b, 0, 64 * 1024, 2048, SPI_NOR_QUAD_READ,0, 0, 0x0) },
#else
	{ "mx25l512e",   INFO(0xc22010, 0, 64 * 1024,   1, SECT_4K) },
	{ "mx25l2005a",  INFO(0xc22012, 0, 64 * 1024,   4, SECT_4K) },
	{ "mx25l4005a",  INFO(0xc22013, 0, 64 * 1024,   8, SECT_4K) },
	{ "mx25l8005",   INFO(0xc22014, 0, 64 * 1024,  16, 0) },
	{ "mx25l1606e",  INFO(0xc22015, 0, 64 * 1024,  32, SECT_4K) },
	{ "mx25l3205d",  INFO(0xc22016, 0, 64 * 1024,  64, 0) },
	{ "mx25l3255e",  INFO(0xc29e16, 0, 64 * 1024,  64, SECT_4K) },
	{ "mx25l6405d",  INFO(0xc22017, 0, 64 * 1024, 128, 0) },
	{ "mx25u6435f",  INFO(0xc22537, 0, 64 * 1024, 128, SECT_4K) },
	{ "mx25l12805d", INFO(0xc22018, 0, 64 * 1024, 256, 0) },
	{ "mx25l12855e", INFO(0xc22618, 0, 64 * 1024, 256, 0) },
	{ "mx25l25635e", INFO(0xc22019, 0, 64 * 1024, 512, 0) },
	{ "mx25l25655e", INFO(0xc22619, 0, 64 * 1024, 512, 0) },
	{ "mx66l51235l", INFO(0xc2201a, 0, 64 * 1024, 1024, SPI_NOR_QUAD_READ) },
	{ "mx66l1g55g",  INFO(0xc2261b, 0, 64 * 1024, 2048, SPI_NOR_QUAD_READ) },
#endif

	/* Micron */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "n25q032",	 INFO(0x20ba16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ, 41000000, 100000000, 0x0) },
	{ "n25q032a",	 INFO(0x20bb16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ,0, 0, 0x0) },
	{ "n25q064",     INFO(0x20ba17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ, 41000000, 100000000, FLASH_VERIFIED) },
	{ "n25q064a",    INFO(0x20bb17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ,0, 0, 0x0) },
	{ "n25q128a11",  INFO(0x20bb18, 0, 64 * 1024,  256, SPI_NOR_QUAD_READ, 41000000, 100000000, 0x0) },
	{ "n25q128a13",  INFO(0x20ba18, 0, 64 * 1024,  256, SPI_NOR_QUAD_READ, 41000000, 100000000, FLASH_VERIFIED) },
	{ "n25q256a",    INFO(0x20ba19, 0, 64 * 1024,  512, SECT_4K | SPI_NOR_QUAD_READ, 41000000, 100000000, 0x0) },
	{ "n25q512a",    INFO(0x20bb20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ, 41000000, 100000000, 0x0) },
	{ "n25q512ax3",  INFO(0x20ba20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ,0, 0, 0x0) },
	{ "n25q00",      INFO(0x20ba21, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ,0, 0, 0x0) },
#else
	{ "n25q032",	 INFO(0x20ba16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ) },
	{ "n25q032a",	 INFO(0x20bb16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ) },
	{ "n25q064",     INFO(0x20ba17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ) },
	{ "n25q064a",    INFO(0x20bb17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ) },
	{ "n25q128a11",  INFO(0x20bb18, 0, 64 * 1024,  256, SPI_NOR_QUAD_READ) },
	{ "n25q128a13",  INFO(0x20ba18, 0, 64 * 1024,  256, SPI_NOR_QUAD_READ) },
	{ "n25q256a",    INFO(0x20ba19, 0, 64 * 1024,  512, SECT_4K | SPI_NOR_QUAD_READ) },
	{ "n25q512a",    INFO(0x20bb20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ "n25q512ax3",  INFO(0x20ba20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ "n25q00",      INFO(0x20ba21, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
#endif

	/* PMC */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "pm25lv512",   INFO(0,        0, 32 * 1024,    2, SECT_4K_PMC, 18000000, 18000000, 0x0) },
	{ "pm25lv010",   INFO(0,        0, 32 * 1024,    4, SECT_4K_PMC, 18000000, 18000000, 0x0) },
	{ "pm25lq032",   INFO(0x7f9d46, 0, 64 * 1024,   64, SECT_4K, 29000000, 100000000, 0x0) },
#else
	{ "pm25lv512",   INFO(0,        0, 32 * 1024,    2, SECT_4K_PMC) },
	{ "pm25lv010",   INFO(0,        0, 32 * 1024,    4, SECT_4K_PMC) },
	{ "pm25lq032",   INFO(0x7f9d46, 0, 64 * 1024,   64, SECT_4K) },
#endif

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "s25sl032p",  INFO(0x010215, 0x4d00,  64 * 1024,  64, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 0, 0, 0x0) },
	{ "s25sl064p",  INFO(0x010216, 0x4d00,  64 * 1024, 128, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 0, 0, 0x0) },
	{ "s25fl256s0", INFO(0x010219, 0x4d00, 256 * 1024, 128, 0, 41000000, 100000000, 0x0) },
	{ "s25fl256s1", INFO(0x010219, 0x4d01,  64 * 1024, 512, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 41000000, 100000000, 0x0) },
	{ "s25fl512s",  INFO(0x010220, 0x4d00, 256 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 41000000, 100000000, FLASH_VERIFIED) },
	{ "s70fl01gs",  INFO(0x010221, 0x4d00, 256 * 1024, 256, 0, 41000000, 100000000, 0x0) },
	{ "s25sl12800", INFO(0x012018, 0x0300, 256 * 1024,  64, 0, 0, 0, 0x0) },
	{ "s25sl12801", INFO(0x012018, 0x0301,  64 * 1024, 256, 0, 0, 0, 0x0) },
	{ "s25fl128s",	INFO6(0x012018, 0x4d0180, 64 * 1024, 256, SECT_4K | SPI_NOR_QUAD_READ, 29000000, 100000000, 0x0) },
	{ "s25fl129p0", INFO(0x012018, 0x4d00, 256 * 1024,  64, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 29000000, 100000000, 0x0) },
	{ "s25fl129p1", INFO(0x012018, 0x4d01,  64 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 29000000, 100000000, 0x0) },
	{ "s25sl004a",  INFO(0x010212,      0,  64 * 1024,   8, 0, 0, 0, 0x0) },
	{ "s25sl008a",  INFO(0x010213,      0,  64 * 1024,  16, 0, 0, 0, 0x0) },
	{ "s25sl016a",  INFO(0x010214,      0,  64 * 1024,  32, 0, 0, 0, 0x0) },
	{ "s25sl032a",  INFO(0x010215,      0,  64 * 1024,  64, 0, 0, 0, 0x0) },
	{ "s25sl064a",  INFO(0x010216,      0,  64 * 1024, 128, 0, 0, 0, 0x0) },
	{ "s25fl004k",  INFO(0xef4013,      0,  64 * 1024,   8, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 0, 0, 0x0) },
	{ "s25fl008k",  INFO(0xef4014,      0,  64 * 1024,  16, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 0, 0, 0x0) },
	{ "s25fl016k",  INFO(0xef4015,      0,  64 * 1024,  32, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 0, 0, 0x0) },
	{ "s25fl064k",  INFO(0xef4017,      0,  64 * 1024, 128, SECT_4K, 0, 0, 0x0) },
	{ "s25fl132k",  INFO(0x014016,      0,  64 * 1024,  64, SECT_4K,0, 0, 0x0) },
	{ "s25fl164k",  INFO(0x014017,      0,  64 * 1024, 128, SECT_4K,0, 0, 0x0) },
	{ "s25fl204k",  INFO(0x014013,      0,  64 * 1024,   8, SECT_4K | SPI_NOR_DUAL_READ,0, 0, 0x0) },
#else
	{ "s25sl032p",  INFO(0x010215, 0x4d00,  64 * 1024,  64, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25sl064p",  INFO(0x010216, 0x4d00,  64 * 1024, 128, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25fl256s0", INFO(0x010219, 0x4d00, 256 * 1024, 128, 0) },
	{ "s25fl256s1", INFO(0x010219, 0x4d01,  64 * 1024, 512, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25fl512s",  INFO(0x010220, 0x4d00, 256 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s70fl01gs",  INFO(0x010221, 0x4d00, 256 * 1024, 256, 0) },
	{ "s25sl12800", INFO(0x012018, 0x0300, 256 * 1024,  64, 0) },
	{ "s25sl12801", INFO(0x012018, 0x0301,  64 * 1024, 256, 0) },
	{ "s25fl128s",	INFO6(0x012018, 0x4d0180, 64 * 1024, 256, SECT_4K | SPI_NOR_QUAD_READ) },
	{ "s25fl129p0", INFO(0x012018, 0x4d00, 256 * 1024,  64, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25fl129p1", INFO(0x012018, 0x4d01,  64 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25sl004a",  INFO(0x010212,      0,  64 * 1024,   8, 0) },
	{ "s25sl008a",  INFO(0x010213,      0,  64 * 1024,  16, 0) },
	{ "s25sl016a",  INFO(0x010214,      0,  64 * 1024,  32, 0) },
	{ "s25sl032a",  INFO(0x010215,      0,  64 * 1024,  64, 0) },
	{ "s25sl064a",  INFO(0x010216,      0,  64 * 1024, 128, 0) },
	{ "s25fl004k",  INFO(0xef4013,      0,  64 * 1024,   8, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25fl008k",  INFO(0xef4014,      0,  64 * 1024,  16, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25fl016k",  INFO(0xef4015,      0,  64 * 1024,  32, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "s25fl064k",  INFO(0xef4017,      0,  64 * 1024, 128, SECT_4K) },
	{ "s25fl132k",  INFO(0x014016,      0,  64 * 1024,  64, SECT_4K) },
	{ "s25fl164k",  INFO(0x014017,      0,  64 * 1024, 128, SECT_4K) },
	{ "s25fl204k",  INFO(0x014013,      0,  64 * 1024,   8, SECT_4K | SPI_NOR_DUAL_READ) },
#endif

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "sst25vf040b", INFO(0xbf258d, 0, 64 * 1024,  8, SECT_4K | SST_WRITE, 18000000, 41000000, 0x0) },
	{ "sst25vf080b", INFO(0xbf258e, 0, 64 * 1024, 16, SECT_4K | SST_WRITE, 18000000, 41000000, 0x0) },
	{ "sst25vf016b", INFO(0xbf2541, 0, 64 * 1024, 32, SECT_4K | SST_WRITE, 18000000, 41000000, 0x0) },
	{ "sst25vf032b", INFO(0xbf254a, 0, 64 * 1024, 64, SECT_4K | SST_WRITE, 18000000, 41000000, 0x0) },
	{ "sst25vf064c", INFO(0xbf254b, 0, 64 * 1024, 128, SECT_4K, 29000000, 41000000, 0x0) },
	{ "sst25wf512",  INFO(0xbf2501, 0, 64 * 1024,  1, SECT_4K | SST_WRITE, 18000000, 29000000, 0x0) },
	{ "sst25wf010",  INFO(0xbf2502, 0, 64 * 1024,  2, SECT_4K | SST_WRITE, 18000000, 29000000, 0x0) },
	{ "sst25wf020",  INFO(0xbf2503, 0, 64 * 1024,  4, SECT_4K | SST_WRITE, 18000000, 29000000, 0x0) },
	{ "sst25wf020a", INFO(0x621612, 0, 64 * 1024,  4, SECT_4K,0, 0, 0x0) },
	{ "sst25wf040b", INFO(0x621613, 0, 64 * 1024,  8, SECT_4K,0, 0, 0x0) },
	{ "sst25wf040",  INFO(0xbf2504, 0, 64 * 1024,  8, SECT_4K | SST_WRITE, 18000000, 29000000, 0x0) },
	{ "sst25wf080",  INFO(0xbf2505, 0, 64 * 1024, 16, SECT_4K | SST_WRITE,0, 0, 0x0) },
#else
	{ "sst25vf040b", INFO(0xbf258d, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
	{ "sst25vf080b", INFO(0xbf258e, 0, 64 * 1024, 16, SECT_4K | SST_WRITE) },
	{ "sst25vf016b", INFO(0xbf2541, 0, 64 * 1024, 32, SECT_4K | SST_WRITE) },
	{ "sst25vf032b", INFO(0xbf254a, 0, 64 * 1024, 64, SECT_4K | SST_WRITE) },
	{ "sst25vf064c", INFO(0xbf254b, 0, 64 * 1024, 128, SECT_4K) },
	{ "sst25wf512",  INFO(0xbf2501, 0, 64 * 1024,  1, SECT_4K | SST_WRITE) },
	{ "sst25wf010",  INFO(0xbf2502, 0, 64 * 1024,  2, SECT_4K | SST_WRITE) },
	{ "sst25wf020",  INFO(0xbf2503, 0, 64 * 1024,  4, SECT_4K | SST_WRITE) },
	{ "sst25wf020a", INFO(0x621612, 0, 64 * 1024,  4, SECT_4K) },
	{ "sst25wf040b", INFO(0x621613, 0, 64 * 1024,  8, SECT_4K) },
	{ "sst25wf040",  INFO(0xbf2504, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
	{ "sst25wf080",  INFO(0xbf2505, 0, 64 * 1024, 16, SECT_4K | SST_WRITE) },
#endif

	/* ST Microelectronics -- newer production may have feature updates */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "m25p05",  INFO(0x202010,  0,  32 * 1024,   2, 0, 18000000, 18000000, 0x0) },
	{ "m25p10",  INFO(0x202011,  0,  32 * 1024,   4, 0, 18000000, 18000000, 0x0) },
	{ "m25p20",  INFO(0x202012,  0,  64 * 1024,   4, 0, 18000000, 18000000, 0x0) },
	{ "m25p40",  INFO(0x202013,  0,  64 * 1024,   8, 0, 18000000, 18000000, 0x0) },
	{ "m25p80",  INFO(0x202014,  0,  64 * 1024,  16, 0, 18000000, 18000000, 0x0) },
	{ "m25p16",  INFO(0x202015,  0,  64 * 1024,  32, 0, 18000000, 41000000, 0x0) },
	{ "m25p32",  INFO(0x202016,  0,  64 * 1024,  64, 0, 18000000, 41000000, 0x0) },
	{ "m25p64",  INFO(0x202017,  0,  64 * 1024, 128, 0, 18000000, 41000000, 0x0) },
	{ "m25p128", INFO(0x202018,  0, 256 * 1024,  64, 0, 18000000, 41000000, 0x0) },

	{ "m25p05-nonjedec",  INFO(0, 0,  32 * 1024,   2, 0, 18000000, 18000000, 0x0) },
	{ "m25p10-nonjedec",  INFO(0, 0,  32 * 1024,   4, 0, 18000000, 18000000, 0x0) },
	{ "m25p20-nonjedec",  INFO(0, 0,  64 * 1024,   4, 0, 18000000, 18000000, 0x0) },
	{ "m25p40-nonjedec",  INFO(0, 0,  64 * 1024,   8, 0, 18000000, 18000000, 0x0) },
	{ "m25p80-nonjedec",  INFO(0, 0,  64 * 1024,  16, 0, 18000000, 18000000, 0x0) },
	{ "m25p16-nonjedec",  INFO(0, 0,  64 * 1024,  32, 0, 18000000, 41000000, 0x0) },
	{ "m25p32-nonjedec",  INFO(0, 0,  64 * 1024,  64, 0, 18000000, 41000000, 0x0) },
	{ "m25p64-nonjedec",  INFO(0, 0,  64 * 1024, 128, 0, 18000000, 41000000, 0x0) },
	{ "m25p128-nonjedec", INFO(0, 0, 256 * 1024,  64, 0, 18000000, 41000000, 0x0) },

	{ "m45pe10", INFO(0x204011,  0, 64 * 1024,    2, 0, 29000000, 41000000, 0x0) },
	{ "m45pe80", INFO(0x204014,  0, 64 * 1024,   16, 0, 29000000, 41000000, 0x0) },
	{ "m45pe16", INFO(0x204015,  0, 64 * 1024,   32, 0, 29000000, 41000000, 0x0) },

	{ "m25pe20", INFO(0x208012,  0, 64 * 1024,  4,       0, 29000000, 41000000, 0x0) },
	{ "m25pe80", INFO(0x208014,  0, 64 * 1024, 16,       0, 18000000, 41000000, 0x0) },
	{ "m25pe16", INFO(0x208015,  0, 64 * 1024, 32, SECT_4K, 29000000, 41000000, 0x0) },

	{ "m25px16",    INFO(0x207115,  0, 64 * 1024, 32, SECT_4K, 29000000, 75000000, 0x0) },
	{ "m25px32",    INFO(0x207116,  0, 64 * 1024, 64, SECT_4K, 29000000, 75000000, 0x0) },
	{ "m25px32-s0", INFO(0x207316,  0, 64 * 1024, 64, SECT_4K, 29000000, 75000000, 0x0) },
	{ "m25px32-s1", INFO(0x206316,  0, 64 * 1024, 64, SECT_4K, 29000000, 75000000, 0x0) },
	{ "m25px64",    INFO(0x207117,  0, 64 * 1024, 128, 0, 29000000, 75000000, 0x0) },
	{ "m25px80",    INFO(0x207114,  0, 64 * 1024, 16, 0,0, 0, 0x0) },
#else
	{ "m25p05",  INFO(0x202010,  0,  32 * 1024,   2, 0) },
	{ "m25p10",  INFO(0x202011,  0,  32 * 1024,   4, 0) },
	{ "m25p20",  INFO(0x202012,  0,  64 * 1024,   4, 0) },
	{ "m25p40",  INFO(0x202013,  0,  64 * 1024,   8, 0) },
	{ "m25p80",  INFO(0x202014,  0,  64 * 1024,  16, 0) },
	{ "m25p16",  INFO(0x202015,  0,  64 * 1024,  32, 0) },
	{ "m25p32",  INFO(0x202016,  0,  64 * 1024,  64, 0) },
	{ "m25p64",  INFO(0x202017,  0,  64 * 1024, 128, 0) },
	{ "m25p128", INFO(0x202018,  0, 256 * 1024,  64, 0) },

	{ "m25p05-nonjedec",  INFO(0, 0,  32 * 1024,   2, 0) },
	{ "m25p10-nonjedec",  INFO(0, 0,  32 * 1024,   4, 0) },
	{ "m25p20-nonjedec",  INFO(0, 0,  64 * 1024,   4, 0) },
	{ "m25p40-nonjedec",  INFO(0, 0,  64 * 1024,   8, 0) },
	{ "m25p80-nonjedec",  INFO(0, 0,  64 * 1024,  16, 0) },
	{ "m25p16-nonjedec",  INFO(0, 0,  64 * 1024,  32, 0) },
	{ "m25p32-nonjedec",  INFO(0, 0,  64 * 1024,  64, 0) },
	{ "m25p64-nonjedec",  INFO(0, 0,  64 * 1024, 128, 0) },
	{ "m25p128-nonjedec", INFO(0, 0, 256 * 1024,  64, 0) },

	{ "m45pe10", INFO(0x204011,  0, 64 * 1024,    2, 0) },
	{ "m45pe80", INFO(0x204014,  0, 64 * 1024,   16, 0) },
	{ "m45pe16", INFO(0x204015,  0, 64 * 1024,   32, 0) },

	{ "m25pe20", INFO(0x208012,  0, 64 * 1024,  4,       0) },
	{ "m25pe80", INFO(0x208014,  0, 64 * 1024, 16,       0) },
	{ "m25pe16", INFO(0x208015,  0, 64 * 1024, 32, SECT_4K) },

	{ "m25px16",    INFO(0x207115,  0, 64 * 1024, 32, SECT_4K) },
	{ "m25px32",    INFO(0x207116,  0, 64 * 1024, 64, SECT_4K) },
	{ "m25px32-s0", INFO(0x207316,  0, 64 * 1024, 64, SECT_4K) },
	{ "m25px32-s1", INFO(0x206316,  0, 64 * 1024, 64, SECT_4K) },
	{ "m25px64",    INFO(0x207117,  0, 64 * 1024, 128, 0) },
	{ "m25px80",    INFO(0x207114,  0, 64 * 1024, 16, 0) },
#endif

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "w25x05", INFO(0xef3010, 0, 64 * 1024,  1,  SECT_4K, 0, 0, 0x0) },
	{ "w25x10", INFO(0xef3011, 0, 64 * 1024,  2,  SECT_4K, 18000000, 29000000, 0x0) },
	{ "w25x20", INFO(0xef3012, 0, 64 * 1024,  4,  SECT_4K, 18000000, 29000000, 0x0) },
	{ "w25x40", INFO(0xef3013, 0, 64 * 1024,  8,  SECT_4K, 18000000, 29000000, 0x0) },
	{ "w25x80", INFO(0xef3014, 0, 64 * 1024,  16, SECT_4K, 18000000, 29000000, 0x0) },
	{ "w25x16", INFO(0xef3015, 0, 64 * 1024,  32, SECT_4K, 29000000, 41000000, 0x0) },
	{ "w25x32", INFO(0xef3016, 0, 64 * 1024,  64, SECT_4K, 29000000, 41000000, 0x0) },
	{ "w25q32", INFO(0xef4016, 0, 64 * 1024,  64, SECT_4K, 41000000, 75000000, 0x0) },
	{ "w25q32dw", INFO(0xef6016, 0, 64 * 1024,  64, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ, 41000000, 100000000, 0x0) },
	{ "w25x64", INFO(0xef3017, 0, 64 * 1024, 128, SECT_4K, 29000000, 41000000, 0x0) },
	{ "w25q64", INFO(0xef4017, 0, 64 * 1024, 128, SECT_4K, 29000000, 41000000, FLASH_VERIFIED) },
	{ "w25q64dw", INFO(0xef6017, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ,0, 0, 0x0) },
	{ "w25q128fw", INFO(0xef6018, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ,0, 0, 0x0) },
	{ "w25q80", INFO(0xef5014, 0, 64 * 1024,  16, SECT_4K, 41000000, 75000000, 0x0) },
	{ "w25q80bl", INFO(0xef4014, 0, 64 * 1024,  16, SECT_4K, 18000000, 41000000, 0x0) },
	{ "w25q128", INFO(0xef4018, 0, 64 * 1024, 256, SECT_4K, 29000000, 41000000,FLASH_VERIFIED) },
	{ "w25q256", INFO(0xef4019, 0, 64 * 1024, 512, SECT_4K, 41000000, 41000000, USE_4B_READ|FLASH_VERIFIED) },
#else
	{ "w25x05", INFO(0xef3010, 0, 64 * 1024,  1,  SECT_4K) },
	{ "w25x10", INFO(0xef3011, 0, 64 * 1024,  2,  SECT_4K) },
	{ "w25x20", INFO(0xef3012, 0, 64 * 1024,  4,  SECT_4K) },
	{ "w25x40", INFO(0xef3013, 0, 64 * 1024,  8,  SECT_4K) },
	{ "w25x80", INFO(0xef3014, 0, 64 * 1024,  16, SECT_4K) },
	{ "w25x16", INFO(0xef3015, 0, 64 * 1024,  32, SECT_4K) },
	{ "w25x32", INFO(0xef3016, 0, 64 * 1024,  64, SECT_4K) },
	{ "w25q32", INFO(0xef4016, 0, 64 * 1024,  64, SECT_4K) },
	{ "w25q32dw", INFO(0xef6016, 0, 64 * 1024,  64, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "w25x64", INFO(0xef3017, 0, 64 * 1024, 128, SECT_4K) },
	{ "w25q64", INFO(0xef4017, 0, 64 * 1024, 128, SECT_4K) },
	{ "w25q64dw", INFO(0xef6017, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "w25q128fw", INFO(0xef6018, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ "w25q80", INFO(0xef5014, 0, 64 * 1024,  16, SECT_4K) },
	{ "w25q80bl", INFO(0xef4014, 0, 64 * 1024,  16, SECT_4K) },
	{ "w25q128", INFO(0xef4018, 0, 64 * 1024, 256, SECT_4K) },
	{ "w25q256", INFO(0xef4019, 0, 64 * 1024, 512, SECT_4K) },
#endif

	/* Catalyst / On Semiconductor -- non-JEDEC */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	{ "cat25c11", CAT25_INFO(  16, 8, 16, 1, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR, 1000000, 1000000, 0x0) },
	{ "cat25c03", CAT25_INFO(  32, 8, 16, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR, 1000000, 1000000, 0x0) },
	{ "cat25c09", CAT25_INFO( 128, 8, 32, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR, 1000000, 1000000, 0x0) },
	{ "cat25c17", CAT25_INFO( 256, 8, 32, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR, 1000000, 1000000, 0x0) },
	{ "cat25128", CAT25_INFO(2048, 8, 64, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR, 4000000, 4000000, 0x0) },
#else
	{ "cat25c11", CAT25_INFO(  16, 8, 16, 1, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
	{ "cat25c03", CAT25_INFO(  32, 8, 16, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
	{ "cat25c09", CAT25_INFO( 128, 8, 32, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
	{ "cat25c17", CAT25_INFO( 256, 8, 32, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
	{ "cat25128", CAT25_INFO(2048, 8, 64, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
#endif

	/* XMC_NOR_FLASH_SUPPORT */
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
#ifdef XMC_NOR_FLASH_SUPPORT
	{ "XM25QH64A", INFO(0x207017, 0, 64 * 1024, 128, SECT_4K, 75000000, 75000000, 0x0) },
	{ "XM25QH128A", INFO(0x207018, 0, 64 * 1024, 256, SECT_4K, 75000000, 75000000, 0x0) },
	{ "XM25QH128C", INFO(0x204018, 0, 64 * 1024, 256, SECT_4K, 75000000, 75000000, 0x0) },
#else
#endif
#else
#ifdef XMC_NOR_FLASH_SUPPORT
	{ "XM25QH64A", INFO(0x207017, 0, 64 * 1024, 128, SECT_4K) },
	{ "XM25QH128A", INFO(0x207018, 0, 64 * 1024, 256, SECT_4K) },
	{ "XM25QH128C", INFO(0x204018, 0, 64 * 1024, 256, SECT_4K) },
#endif
#endif

	{ },
};

static const struct flash_info *spi_nor_read_id(struct spi_nor *nor)
{
	int			tmp;
	u8			id[SPI_NOR_MAX_ID_LEN];
	const struct flash_info	*info;

	tmp = nor->read_reg(nor, SPINOR_OP_RDID, id, SPI_NOR_MAX_ID_LEN);
	if (tmp < 0) {
		dev_dbg(nor->dev, " error %d reading JEDEC ID\n", tmp);
		return ERR_PTR(tmp);
	}

	for (tmp = 0; tmp < ARRAY_SIZE(spi_nor_ids) - 1; tmp++) {
		info = &spi_nor_ids[tmp];
		if (info->id_len) {
			if (!memcmp(info->id, id, info->id_len))
				return &spi_nor_ids[tmp];
		}
	}
	dev_err(nor->dev, "unrecognized JEDEC id bytes: %02x, %2x, %2x\n",
		id[0], id[1], id[2]);
	return ERR_PTR(-ENODEV);
}

static int spi_nor_read(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	int ret;

	dev_dbg(nor->dev, "from 0x%08x, len %zd\n", (u32)from, len);

	ret = spi_nor_lock_and_prep(nor, SPI_NOR_OPS_READ);
	if (ret)
		return ret;

	ret = nor->read(nor, from, len, retlen, buf);

	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_READ);
	return ret;
}

static int sst_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	size_t actual;
	int ret;

	dev_dbg(nor->dev, "to 0x%08x, len %zd\n", (u32)to, len);

	ret = spi_nor_lock_and_prep(nor, SPI_NOR_OPS_WRITE);
	if (ret)
		return ret;

	write_enable(nor);

	nor->sst_write_second = false;

	actual = to % 2;
	/* Start write from odd address. */
	if (actual) {
		nor->program_opcode = SPINOR_OP_BP;

		/* write one byte. */
		nor->write(nor, to, 1, retlen, buf);
		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto time_out;
	}
	to += actual;

	/* Write out most of the data here. */
	for (; actual < len - 1; actual += 2) {
		nor->program_opcode = SPINOR_OP_AAI_WP;

		/* write two bytes. */
		nor->write(nor, to, 2, retlen, buf + actual);
		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto time_out;
		to += 2;
		nor->sst_write_second = true;
	}
	nor->sst_write_second = false;

	write_disable(nor);
	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		goto time_out;

	/* Write out trailing byte if it exists. */
	if (actual != len) {
		write_enable(nor);

		nor->program_opcode = SPINOR_OP_BP;
		nor->write(nor, to, 1, retlen, buf + actual);

		ret = spi_nor_wait_till_ready(nor);
		if (ret)
			goto time_out;
		write_disable(nor);
	}
time_out:
	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_WRITE);
	return ret;
}

/*
 * Write an address range to the nor chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int spi_nor_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct spi_nor *nor = mtd_to_spi_nor(mtd);
	u32 page_offset, page_size, i;
	int ret;

	dev_dbg(nor->dev, "to 0x%08x, len %zd\n", (u32)to, len);

	ret = spi_nor_lock_and_prep(nor, SPI_NOR_OPS_WRITE);
	if (ret)
		return ret;

	write_enable(nor);

	page_offset = to & (nor->page_size - 1);

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= nor->page_size) {
		nor->write(nor, to, len, retlen, buf);
	} else {
		/* the size of data remaining on the first page */
		page_size = nor->page_size - page_offset;
		nor->write(nor, to, page_size, retlen, buf);

		/* write everything in nor->page_size chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > nor->page_size)
				page_size = nor->page_size;

			ret = spi_nor_wait_till_ready(nor);
			if (ret)
				goto write_err;

			write_enable(nor);

			nor->write(nor, to + i, page_size, retlen, buf + i);
		}
	}

	ret = spi_nor_wait_till_ready(nor);
write_err:
	spi_nor_unlock_and_unprep(nor, SPI_NOR_OPS_WRITE);
	return ret;
}

static int macronix_quad_enable(struct spi_nor *nor)
{
	int ret, val;

	val = read_sr(nor);
	write_enable(nor);

	write_sr(nor, val | SR_QUAD_EN_MX);

	if (spi_nor_wait_till_ready(nor))
		return 1;

	ret = read_sr(nor);
	if (!(ret > 0 && (ret & SR_QUAD_EN_MX))) {
		dev_err(nor->dev, "Macronix Quad bit not set\n");
		return -EINVAL;
	}

	return 0;
}

/*
 * Write status Register and configuration register with 2 bytes
 * The first byte will be written to the status register, while the
 * second byte will be written to the configuration register.
 * Return negative if error occured.
 */
static int write_sr_cr(struct spi_nor *nor, u16 val)
{
	nor->cmd_buf[0] = val & 0xff;
	nor->cmd_buf[1] = (val >> 8);

	return nor->write_reg(nor, SPINOR_OP_WRSR, nor->cmd_buf, 2);
}

static int spansion_quad_enable(struct spi_nor *nor)
{
	int ret;
	int quad_en = CR_QUAD_EN_SPAN << 8;

	write_enable(nor);

	ret = write_sr_cr(nor, quad_en);
	if (ret < 0) {
		dev_err(nor->dev,
			"error while writing configuration register\n");
		return -EINVAL;
	}

	ret = spi_nor_wait_till_ready(nor);
	if (ret) {
		dev_err(nor->dev,
			"timeout while writing configuration register\n");
		return ret;
	}

	/* read back and check it */
	ret = read_cr(nor);
	if (!(ret > 0 && (ret & CR_QUAD_EN_SPAN))) {
		dev_err(nor->dev, "Spansion Quad bit not set\n");
		return -EINVAL;
	}

	return 0;
}

static int set_quad_mode(struct spi_nor *nor, const struct flash_info *info)
{
	int status;

	switch (JEDEC_MFR(info)) {
	case SNOR_MFR_MACRONIX:
		status = macronix_quad_enable(nor);
		if (status) {
			dev_err(nor->dev, "Macronix quad-read not enabled\n");
			return -EINVAL;
		}
		return status;
	case SNOR_MFR_MICRON:
		return 0;
	default:
		status = spansion_quad_enable(nor);
		if (status) {
			dev_err(nor->dev, "Spansion quad-read not enabled\n");
			return -EINVAL;
		}
		return status;
	}
}

static int spi_nor_check(struct spi_nor *nor)
{
	if (!nor->dev || !nor->read || !nor->write ||
		!nor->read_reg || !nor->write_reg || !nor->erase) {
		pr_err("spi-nor: please fill all the necessary fields!\n");
		return -EINVAL;
	}

	return 0;
}

#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
/* Get flash device information such as command, dummy cycle and type */
static int spi_nor_get_flash_device(struct spi_nor *nor, struct flash_info* info)
{
	uint32_t i;
	uint32_t size;

	nor->dev_info.device_id = JEDEC_MFR(info);
	size = sizeof(struct flash_device)/sizeof(struct flash_vendor_info *);
	for (i = 0; i < size; i++) {
		if (nor->dev_info.device_id == device.vendors[i]->flash_id) {
			nor->dev_info.cmd = device.vendors[i]->cmd;
			nor->dev_info.dummy = device.vendors[i]->dummy;
			nor->dev_info.type = device.vendors[i]->type;
			printk(KERN_INFO "flash vendor: %s\n", device.vendors[i]->vendor_name);
			return 0;
		}
	}
	printk(KERN_INFO "\n>>>>>No Flash Vendor support (0x%x)<<<<<\n\n", JEDEC_MFR(info));
#if 1
	printk("use MXIC as flash vendor instead\n");
	nor->dev_info.cmd = device.vendors[0]->cmd;
	nor->dev_info.dummy = device.vendors[0]->dummy;
	nor->dev_info.type = device.vendors[0]->type;
	return 0;
#else
	return -EINVAL;
#endif
}


u32 spi_nor_get_chip_clk(void)
{
	return chip_max_speed_hz;
}
EXPORT_SYMBOL(spi_nor_get_chip_clk);
#endif


#ifdef TP_IPF_FLASH_IOCTL
/*
 *	For IPF Platform
 */
#include <linux/fs.h>

int	flash_major =  239;
#define FLASH_DEVNAME			"flash_chrdev"

#define FLASH_SECTOR_SIZE 		0x10000				/* 64 * 1024 */
#define FLASH_MAX_RW_SIZE 		FLASH_SECTOR_SIZE


#define TP_MAC_OFFSET			0x790000
#define TP_MAC_HEAD_LEN			(8 + 4)
#define TP_MAC_SEC_SIZE			(64)


/*
 * IOCTL Command Codes
 */
#define TP_FLASH_READ              0x01
#define TP_FLASH_WRITE             0x02
#define TP_FLASH_ERASE             0x03
	
#define TP_IO_MAGIC                0xB3
#define TP_IO_FLASH_READ           _IOR(TP_IO_MAGIC, TP_FLASH_READ, char)
#define TP_IO_FLASH_WRITE          _IOW(TP_IO_MAGIC, TP_FLASH_WRITE, char)
#define TP_IO_FLASH_ERASE          _IO (TP_IO_MAGIC, TP_FLASH_ERASE)
	
#define TP_IOC_MAXNR               14

typedef struct 
{
	u_int32_t addr;		/* flash r/w addr	*/
	u_int32_t len;		/* r/w length		*/
	u_int8_t* buf;		/* user-space buffer*/
	u_int32_t buflen;	/* buffer length	*/
	u_int32_t hasHead;	/* hasHead flag 		*/
}ARG;


/*
 * static int spi_nor_read(struct mtd_info *mtd, loff_t from, size_t len,
 *	size_t *retlen, u_char *buf)
 */
int spiflash_ioctl_read(u_char *buf, u32 from, u32 len)
{
	u32 retlen = 0;
	int ret = 0;
	ret = spi_nor_read(mtd_spi, from, len, &retlen, buf);
	return ret;
}

EXPORT_SYMBOL(spiflash_ioctl_read);

/* 
 * spi_nor__write(struct mtd_info *mtd, loff_t to, size_t len,
 *	 size_t *retlen, const u_char *buf)
 */

int spiflash_ioctl_write(u_char *buf, u32 to, u32 len)
{
	u32 retlen = 0;
	int ret = 0;
	ret = spi_nor_write(mtd_spi, to, len, &retlen, buf);
	return ret;
}

EXPORT_SYMBOL(spiflash_ioctl_write);

/*
 * spi_nor_erase(struct mtd_info *mtd, struct erase_info *instr)
 */
int spiflash_ioctl_erase(u32 to, u32 len)
{
	u32 count = 0;
	struct erase_info instr;
	instr.addr = to;
	instr.len = mtd_spi->erasesize;
	instr.callback = NULL;
	instr.mtd = mtd_spi;

	
	for (count = 0; count < (len / mtd_spi->erasesize + 1); count++ )
	{
		if ( (len%mtd_spi->erasesize == 0) && (count == len / mtd_spi->erasesize))
		{
			break;
		}	
		spi_nor_erase(mtd_spi, &instr);
		instr.addr += mtd_spi->erasesize;
		printk(".");
	}	
}

EXPORT_SYMBOL(spiflash_ioctl_erase);


int tp_flash_read(u_int8_t *rwBuf, u_int32_t addr, u_int8_t *usrBuf, u_int32_t usrBufLen)
{
    u_int32_t read_len = usrBufLen;
	
	if(NULL == rwBuf || NULL == usrBuf || read_len <= 0)
	{
		printk("Invalid args!\n");
		return -1;
	}
	
	if(spiflash_ioctl_read(rwBuf, addr, read_len) < 0)
	{
		printk("spiflash_ioctl_read out of scope!\n");
        return -1;
	}
	
	if(copy_to_user(usrBuf, rwBuf, read_len) != 0)
	{
		printk("read copy_to_user failed\n");
        return -1;
	}
	
    return 0;
}

int tp_flash_write(u_int8_t *tempData, 
                      u_int32_t hasHead, u_int32_t offset, u_int8_t *data, u_int32_t len)
{
    u_int32_t address = 0;
    u_int32_t headLen = 0;
    u_int32_t endAddr = 0, startAddr = 0;
    u_int8_t *orignData = NULL;
    u_int32_t headData[2] = {len, 0};
    u_int32_t frontLen = 0, tailLen = 0;
    u_int32_t retLen = 0;

    headData[0] = htonl(len);   

    if (hasHead)
    {
        headLen = 2 * sizeof(u_int32_t);
        len += headLen;
    }

    frontLen = offset % FLASH_SECTOR_SIZE;
    tailLen  = (offset + len) % FLASH_SECTOR_SIZE;
    /* first block address */
    address = offset - frontLen;
    /* last uncomplete block address. if none, next block address instead. */
    endAddr = offset + len - tailLen;

    orignData = tempData + FLASH_SECTOR_SIZE;

    if (frontLen > 0 || headLen > 0)/*first block*/
    {
        spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
        memcpy(tempData, orignData, frontLen);
        
        if (FLASH_SECTOR_SIZE < frontLen + headLen) /* header is in different block */
        {
            headLen = FLASH_SECTOR_SIZE - frontLen;
            /* partition header, first part. */
            memcpy(tempData + frontLen, headData, headLen);

            /***************************************************/
            if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE)) 
            {
                spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
				udelay(20);
                spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
				printk("H#");
            }
            address += FLASH_SECTOR_SIZE;
            /***************************************************/
            spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
            /* partition header, second part. */
            memcpy(tempData, (u_int8_t*)(headData) + headLen, 8 - headLen);

            if (len - headLen < FLASH_SECTOR_SIZE) /* writen length less than one block */
            {
                headLen = 8 - headLen;
                copy_from_user(tempData + headLen, data, tailLen - headLen); /* data to be writen */
                memcpy(tempData + tailLen, orignData + tailLen, FLASH_SECTOR_SIZE - tailLen);
                data += tailLen - headLen;
            }
            else
            {
                headLen = 8 - headLen;
                copy_from_user(tempData + headLen, data, FLASH_SECTOR_SIZE - headLen);
                data += FLASH_SECTOR_SIZE - headLen;
            }
        }
        else /* normal */
        {
            memcpy(tempData + frontLen, headData, headLen); /* header (if exist) */
            
            if (len + frontLen < FLASH_SECTOR_SIZE) /* write less then a block */
            {
                copy_from_user(tempData + frontLen + headLen, data, len - headLen);
                data += len - headLen;
                /* orginal data */
                memcpy(tempData + frontLen + len,
                         orignData + frontLen + len,
                         FLASH_SECTOR_SIZE - (frontLen + len));
            }
            else
            {
                copy_from_user(tempData + frontLen + headLen, data, FLASH_SECTOR_SIZE - frontLen - headLen);
                /* data to be writen */
                data += FLASH_SECTOR_SIZE - frontLen - headLen;
            }
        }

        /***************************************************/
        if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE))/* context changed */
        {            
			spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
			udelay(20);
            spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
			printk("H#");
        }
        address += FLASH_SECTOR_SIZE;
        /***************************************************/
    }

    if (address < endAddr)/* complete blocks in middle */
    {
        startAddr = address;
        while (address < endAddr)
        {
            spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
            copy_from_user(tempData, data, FLASH_SECTOR_SIZE);
            /***************************************************/
            if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE)) /* context changed */
            {
                spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
				udelay(20);
            	spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
				printk("#");
            }
            address += FLASH_SECTOR_SIZE;
            /***************************************************/
            data += FLASH_SECTOR_SIZE;
			udelay(20);
        }
    }

    if (address < offset + len) /* last uncomplete block */
    {
        /*printk("[asuka] block at last start %p\n", address);*/
        spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
        copy_from_user(tempData, data, tailLen); /* firstly, data to be writen */
        memcpy(tempData + tailLen, orignData + tailLen, FLASH_SECTOR_SIZE - tailLen);
        /* secondly, recover orginal data */
        /***************************************************/
        if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE)) /* context changed */
        {            
			spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
			udelay(20);
            spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
			printk("T#");
        }
        address += FLASH_SECTOR_SIZE;
        /***************************************************/
    }
	
    return 0;
}


static long
tp_flash_ioctl(struct file *file,  unsigned int cmd, unsigned long arg)
{
     /* temp buffer for r/w */
    unsigned char *rwBuf = (unsigned char *)kmalloc(FLASH_SECTOR_SIZE * 2, GFP_KERNEL);
    ARG *pArg = (ARG*)arg;

    if (rwBuf == NULL)
    {
        printk("rw_buf error!\n");
        goto wrong;
    }
    if (_IOC_TYPE(cmd) != TP_IO_MAGIC)
    {
        printk("cmd type error!\n");
        goto wrong;
    }
    if (_IOC_NR(cmd) > TP_IOC_MAXNR)
    {
        printk("cmd NR error!\n");
        goto wrong;
    }
    
    switch(cmd)
    {
        case TP_IO_FLASH_READ:
        {
            tp_flash_read(rwBuf, pArg->addr, pArg->buf, pArg->buflen);
            goto good;
            break; 
        }

        case TP_IO_FLASH_WRITE:
        {
            tp_flash_write(rwBuf, pArg->hasHead, pArg->addr, pArg->buf, pArg->buflen);
            goto good;
            break;
        }
        
        case  TP_IO_FLASH_ERASE:
        {
            goto good;
            break;
        }
    }

good:
    kfree(rwBuf);
    return 0;
wrong:
    if (rwBuf)
    {
        kfree(rwBuf);
    }

    return -1;
}
        
static int tp_flash_open (struct inode *inode, struct file *filp)
{
    int minor = iminor(inode);
    
    if ((filp->f_mode & 2) && (minor & 1)) {
        printk("You can't open the RO devices RW!\n");
        return -EACCES;
    }
    return 0;
}

int spi_flash_erase_config(void)
{
	printk("spi_flash_erase_config(): do nothing now\n");
	return 0;
}
EXPORT_SYMBOL(spi_flash_erase_config);

int spi_flash_read_mac(char * mac_addr)
{	
	int mac_idx = 0, num_idx = 0;
	int index = 0, num = 0;
	char mac_num[6] = {0};
	unsigned char rwBuf[TP_MAC_SEC_SIZE] = {0};
	
	if(NULL == mac_addr)
	{
		printk("Invalid args\n");
		return -1;
	}
	
	if(spiflash_ioctl_read(rwBuf, TP_MAC_OFFSET, TP_MAC_SEC_SIZE) < 0)
	{
		printk("spiflash_ioctl_read out of scope!\n");
        return -1;
	}
	printk("spi_flash_read_mac() MAC PTN: %s\n\n", rwBuf);
	for(index = 0; index < TP_MAC_SEC_SIZE; index++)
	{
		printk("%c:%02X: ", rwBuf[index], rwBuf[index]);
	}
	printk("\n\n");
	
	for(mac_idx = 0; mac_idx < 6; mac_idx++)
	{
		num = 0;
		for(num_idx = 0; num_idx < 2; num_idx++)
		{
			index = TP_MAC_HEAD_LEN + mac_idx * 3 + num_idx;
			if(rwBuf[index] >= '0' && rwBuf[index] <= '9')
			{
				num  = num * 16 + rwBuf[index] - 48;
			}
			else if(rwBuf[index] >= 'a' && rwBuf[index] <= 'f')
			{
				num  = num * 16 + rwBuf[index] - 'a' + 10;
			}
			else if(rwBuf[index] >= 'A' && rwBuf[index] <= 'F')
			{
				num  = num * 16 + rwBuf[index] - 'A' + 10;
			}
			else
			{
				printk("spi_flash_read_mac() MAC error: %d %c\n",index, rwBuf[index]);
				return -1;
			}
		}
		mac_num[mac_idx] = num;
	}

	memcpy(mac_addr, mac_num, 6);
	
	return 0;
}
EXPORT_SYMBOL(spi_flash_read_mac);

#endif

struct file_operations flash_fops = {
    unlocked_ioctl:     tp_flash_ioctl,
#ifdef TP_IPF_FLASH_IOCTL
	open:				tp_flash_open,
#endif
};

static int __init spiflash_chrdev_init(void)
{
	int result=0;
	printk(KERN_INFO "Register flash device:%s\n", FLASH_DEVNAME);
	result = register_chrdev(flash_major, FLASH_DEVNAME, &flash_fops);
	if (result < 0) {
		printk("flash: can't get major %d\n",flash_major);
		return result;
	}

	if (flash_major == 0)
	{
		flash_major = result; /* dynamic */
	}
	
	return 0;
}

static void __exit spiflash_chrdev_exit(void)
{
	unregister_chrdev(flash_major, FLASH_DEVNAME);
}

module_init(spiflash_chrdev_init);
module_exit(spiflash_chrdev_exit);

/****************************************************************************/



int spi_nor_scan(struct spi_nor *nor, const char *name, enum read_mode mode)
{
	const struct flash_info *info = NULL;
	struct device *dev = nor->dev;
	struct mtd_info *mtd = &nor->mtd;
	struct device_node *np = nor->flash_node;
	int ret;
	int i;

	mtd_spi = mtd;

	ret = spi_nor_check(nor);
	if (ret)
		return ret;

	if (name)
		info = spi_nor_match_id(name);
	/* Try to auto-detect if chip name wasn't specified or not found */
	if (!info)
		info = spi_nor_read_id(nor);
	if (IS_ERR_OR_NULL(info))
		return -ENOENT;

	/*
	 * If caller has specified name of flash model that can normally be
	 * detected using JEDEC, let's verify it.
	 */
	if (name && info->id_len) {
		const struct flash_info *jinfo;

		jinfo = spi_nor_read_id(nor);
		if (IS_ERR(jinfo)) {
			return PTR_ERR(jinfo);
		} else if (jinfo != info) {
			/*
			 * JEDEC knows better, so overwrite platform ID. We
			 * can't trust partitions any longer, but we'll let
			 * mtd apply them anyway, since some partitions may be
			 * marked read-only, and we don't want to lose that
			 * information, even if it's not 100% accurate.
			 */
			dev_warn(dev, "found %s, expected %s\n",
				 jinfo->name, info->name);
			info = jinfo;
		}
	}

	mutex_init(&nor->lock);

	/*
	 * Atmel, SST, Intel/Numonyx, and others serial NOR tend to power up
	 * with the software protection bits set
	 */

	if (JEDEC_MFR(info) == SNOR_MFR_ATMEL ||
	    JEDEC_MFR(info) == SNOR_MFR_INTEL ||
	    JEDEC_MFR(info) == SNOR_MFR_SST) {
		write_enable(nor);
		write_sr(nor, 0);
	}

	if (!mtd->name)
		mtd->name = dev_name(dev);
	mtd->priv = nor;
	mtd->type = MTD_NORFLASH;
	mtd->writesize = 1;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = info->sector_size * info->n_sectors;
	mtd->_erase = spi_nor_erase;
	mtd->_read = spi_nor_read;

	/* NOR protection support for STmicro/Micron chips and similar */
	if (JEDEC_MFR(info) == SNOR_MFR_MICRON) {
		nor->flash_lock = stm_lock;
		nor->flash_unlock = stm_unlock;
		nor->flash_is_locked = stm_is_locked;
	}

	if (nor->flash_lock && nor->flash_unlock && nor->flash_is_locked) {
		mtd->_lock = spi_nor_lock;
		mtd->_unlock = spi_nor_unlock;
		mtd->_is_locked = spi_nor_is_locked;
	}

	/* sst nor chips use AAI word program */
	if (info->flags & SST_WRITE)
		mtd->_write = sst_write;
	else
		mtd->_write = spi_nor_write;

	if (info->flags & USE_FSR)
		nor->flags |= SNOR_F_USE_FSR;

#ifdef CONFIG_MTD_SPI_NOR_USE_4K_SECTORS
	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		nor->erase_opcode = SPINOR_OP_BE_4K;
		mtd->erasesize = 4096;
	} else if (info->flags & SECT_4K_PMC) {
		nor->erase_opcode = SPINOR_OP_BE_4K_PMC;
		mtd->erasesize = 4096;
	} else
#endif
	{
		nor->erase_opcode = SPINOR_OP_SE;
		mtd->erasesize = info->sector_size;
	}

	if (info->flags & SPI_NOR_NO_ERASE)
		mtd->flags |= MTD_NO_ERASE;

	mtd->dev.parent = dev;
	nor->page_size = info->page_size;
	mtd->writebufsize = nor->page_size;

#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	/* dual io/fast read/auto mode support */
	nor->fast_read = false;
	nor->quad = false;
	nor->dual = false;
	nor->auto_mode = false;
	chip_max_speed_hz = info->normal_clk;
#ifdef CONFIG_M25PXX_USE_MULTI_CHANNEL
#ifdef CONFIG_M25PXX_USE_QUAD
	nor->quad = true;
#endif
#ifdef CONFIG_M25PXX_USE_DUAL
	nor->dual = true;
#endif
#else
#ifdef CONFIG_M25PXX_USE_FAST_READ
	nor->fast_read = true;
	chip_max_speed_hz = info->fastrd_clk;
#endif
#endif
#ifdef CONFIG_M25PXX_USE_AUTO_MODE
	nor->auto_mode = true;
#endif

	ret = spi_nor_get_flash_device(nor,info);
	if (ret) {
		dev_err(dev, "no flash device supported\n");
		return ret;
	}
#endif /* defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA) */

	if (np) {
		/* If we were instantiated by DT, use it */
		if (of_property_read_bool(np, "m25p,fast-read"))
			nor->flash_read = SPI_NOR_FAST;
		else
			nor->flash_read = SPI_NOR_NORMAL;
	} else {
		/* If we weren't instantiated by DT, default to fast-read */
		nor->flash_read = SPI_NOR_FAST;
	}

	/* Some devices cannot do fast-read, no matter what DT tells us */
	if (info->flags & SPI_NOR_NO_FR)
		nor->flash_read = SPI_NOR_NORMAL;

	/* Quad/Dual-read mode takes precedence over fast/normal */
	if (mode == SPI_NOR_QUAD && info->flags & SPI_NOR_QUAD_READ) {
		ret = set_quad_mode(nor, info);
		if (ret) {
			dev_err(dev, "quad mode not supported\n");
			return ret;
		}
		nor->flash_read = SPI_NOR_QUAD;
	} else if (mode == SPI_NOR_DUAL && info->flags & SPI_NOR_DUAL_READ) {
		nor->flash_read = SPI_NOR_DUAL;
	}

#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	/**************************************************/
	if (nor->auto_mode) {
		nor->flash_read = SPI_NOR_AUTO;
		nor->flash_write = SPI_NOR_AUTO_WRITE;
	} else if (nor->quad) {
		nor->flash_read = SPI_NOR_QUAD;
		nor->flash_write = SPI_NOR_QUAD_WRITE;
	} else if (nor->dual) {
		nor->flash_read = SPI_NOR_DUAL;
		nor->flash_write = SPI_NOR_DUAL_WRITE;
	} else if (nor->fast_read) {
		nor->flash_read = SPI_NOR_FAST;
		nor->flash_write = SPI_NOR_NORMAL_WRITE;
	} else {
		nor->flash_read = SPI_NOR_NORMAL;
		nor->flash_write = SPI_NOR_NORMAL_WRITE;
	}
#endif

	/* Default commands */
	switch (nor->flash_read) {
	case SPI_NOR_QUAD:
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
		nor->read_opcode = nor->dev_info.cmd->readx4_io;
		nor->dummy= nor->dev_info.dummy->rd_quad_dummy;
		nor->read_type = nor->dev_info.type->rd_quad_type;
#else
		nor->read_opcode = SPINOR_OP_READ_1_1_4;
#endif
		break;
	case SPI_NOR_DUAL:
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
		nor->read_opcode = SPINOR_OP_READ_1_1_4;
		nor->read_opcode = nor->dev_info.cmd->readx2_io;
		nor->dummy = nor->dev_info.dummy->rd_dual_dummy;
		nor->read_type = nor->dev_info.type->rd_dual_type;
#else
		nor->read_opcode = SPINOR_OP_READ_1_1_2;
#endif
		break;
	case SPI_NOR_FAST:
		nor->read_opcode = SPINOR_OP_READ_FAST;
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
		nor->dummy = nor->dev_info.dummy->fast_rd_dummy;
#endif
		break;
	case SPI_NOR_NORMAL:
		nor->read_opcode = SPINOR_OP_READ;
		break;
#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	case SPI_NOR_AUTO:
		nor->read_opcode = SPINOR_OP_AUTO_MODE;
		break;
#endif
	default:
		dev_err(dev, "No Read opcode defined\n");
		return -EINVAL;
	}

#if defined(CONFIG_RTL819X_SPI_FLASH_SHEIPA)
	switch (nor->flash_write) {
	case SPI_NOR_QUAD_WRITE:
		nor->program_opcode = nor->dev_info.cmd->ppx4_ii;
		nor->write_type = nor->dev_info.type->wr_quad_type;
		break;
	case SPI_NOR_DUAL_WRITE:
		nor->program_opcode = nor->dev_info.cmd->ppx2_ii;
		nor->write_type = nor->dev_info.type->wr_dual_type;
		break;
	case SPI_NOR_NORMAL_WRITE:
	nor->program_opcode = SPINOR_OP_PP;
		break;
	case SPI_NOR_AUTO_WRITE:
		nor->program_opcode = SPINOR_OP_AUTO_MODE;
		break;
	default:
		dev_err(dev, "No write opcode defined\n");
		return -EINVAL;
	}
#else
	nor->program_opcode = SPINOR_OP_PP;
#endif

	if (info->addr_width)
		nor->addr_width = info->addr_width;
	else if (mtd->size > 0x1000000) {
		/* enable 4-byte addressing if the device exceeds 16MiB */
		nor->addr_width = 4;
		if (JEDEC_MFR(info) == SNOR_MFR_SPANSION) {
			/* Dedicated 4-byte command set */
			switch (nor->flash_read) {
			case SPI_NOR_QUAD:
				nor->read_opcode = SPINOR_OP_READ4_1_1_4;
				break;
			case SPI_NOR_DUAL:
				nor->read_opcode = SPINOR_OP_READ4_1_1_2;
				break;
			case SPI_NOR_FAST:
				nor->read_opcode = SPINOR_OP_READ4_FAST;
				break;
			case SPI_NOR_NORMAL:
				nor->read_opcode = SPINOR_OP_READ4;
				break;
			}
			nor->program_opcode = SPINOR_OP_PP_4B;
			/* No small sector erase for 4-byte command set */
			nor->erase_opcode = SPINOR_OP_SE_4B;
			mtd->erasesize = info->sector_size;
		} else
			set_4byte(nor, info, 1);
	} else {
		nor->addr_width = 3;
	}

	nor->read_dummy = spi_nor_read_dummy_cycles(nor);

	dev_info(dev, "%s (%lld Kbytes)\n", info->name,
			(long long)mtd->size >> 10);

	dev_dbg(dev,
		"mtd .name = %s, .size = 0x%llx (%lldMiB), "
		".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		mtd->name, (long long)mtd->size, (long long)(mtd->size >> 20),
		mtd->erasesize, mtd->erasesize / 1024, mtd->numeraseregions);

	if (mtd->numeraseregions)
		for (i = 0; i < mtd->numeraseregions; i++)
			dev_dbg(dev,
				"mtd.eraseregions[%d] = { .offset = 0x%llx, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, (long long)mtd->eraseregions[i].offset,
				mtd->eraseregions[i].erasesize,
				mtd->eraseregions[i].erasesize / 1024,
				mtd->eraseregions[i].numblocks);
	return 0;
}
EXPORT_SYMBOL_GPL(spi_nor_scan);

static const struct flash_info *spi_nor_match_id(const char *name)
{
	const struct flash_info *id = spi_nor_ids;

	while (id->name) {
		if (!strcmp(name, id->name))
			return id;
		id++;
	}
	return NULL;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huang Shijie <shijie8@gmail.com>");
MODULE_AUTHOR("Mike Lavender");
MODULE_DESCRIPTION("framework for SPI NOR");
