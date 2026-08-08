/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 9021 $
 * $Date: 2010-04-13 15:45:15 +0800 (Tue, 13 Apr 2010) $
 *
 */

/*
 * Include Files
 */
#if defined(__UBOOT__) && defined(__KERNEL__)
/* for U-Boot Loader */
  #include <common.h>
  #include <config.h>
  #ifdef CONFIG_FLASH_SPI
    #include "flash_spi.h"
  #endif /* end of CONFIG_FLASH_SPI */
#elif !defined(__UBOOT__) && defined(__KERNEL__)  /* end of __UBOOT__ & __KERNEL__ */
  #include <linux/init.h>
  #include <linux/module.h>
  #include <linux/device.h>
  #include <linux/interrupt.h>
  #include <linux/mtd/mtd.h>
  #include <linux/mtd/map.h>
  #include <linux/mtd/partitions.h>
  #include <asm/semaphore.h>
  #include <linux/mtd/rtk_spiflash_sio.h>
#else   /* end of elif !__UBOOT__&__KERNEL__ */
  #error Please implement the OS system functions
#endif /* end of else __UBOOT__&__KERNEL__ */


/*
 * Symbol Definition
 */
#if defined(__UBOOT__) && defined(__KERNEL__)
  #ifdef CONFIG_FLASH_SPI
    #define FLASH_INFO_TYPE flash_info_t
    #ifdef CONFIG_FLASH_SRAM_ONLY
      #define PRINT_DEVICE(x...)
      #define OS_PRINTF(x...)
    #else  /* end of CONFIG_FLASH_SRAM_ONLY */
      #define OS_PRINTF printf
      #define PRINT_DEVICE printf
    #endif /* end of else CONFIG_FLASH_SRAM_ONLY */
  #endif  /* end of CONFIG_FLASH_SPI*/
#elif !defined(__UBOOT__) && defined(__KERNEL__)  /* end of __UBOOT__ & __KERNEL__ */
  #define FLASH_INFO_TYPE RTK_SPI_MTD
  #define OS_PRINTF printk
#else   /* end of elif !__UBOOT__&__KERNEL__ */
  #error Please implement the OS system functions
#endif /* end of else __UBOOT__&__KERNEL__ */

#if defined(CONFIG_FLASH_SPI) || !defined(__UBOOT__)
  #ifndef __textflash
    #define __textflash
  #endif /* __textflash */

  #ifndef __dataflash
    #define __dataflash
  #endif /* __dataflash */
#endif /* end of CONFIG_FLASH_SPI || !__UBOOT__*/

/*
 * Data Declaration
 */
#if defined(__UBOOT__) && defined(__KERNEL__)
  #ifdef CONFIG_FLASH_SPI
    FLASH_INFO_TYPE __dataflash flash_info[MAX_SPI_FLASH_CHIPS];
  #endif  /* end of CONFIG_FLASH_SPI*/
#elif !defined(__UBOOT__) && defined(__KERNEL__)  /* end of __UBOOT__ & __KERNEL__ */

#if defined(CONFIG_FLASH_LAYOUT_TYPE1)
static struct mtd_partition rtk_parts[] = {
	[0] = {
		.name = "LOADER",
		.offset = LOADER_START,
		.size = LOADER_SIZE,
		.mask_flags = MTD_WRITEABLE /* force a read-only partition */
	},
	[1] = {
		.name = "BDINFO",
		.offset = LOADER_BDINFO_START,
		.size = LOADER_BDINFO_SIZE,
		.mask_flags = 0
	},
	[2] = {
		.name = "JFFS2",
		.offset = JFFS2_START,
		.size = JFFS2_SIZE,
		.mask_flags = 0
	},
	[3] = {
		.name = "RUNTIME",
		.offset = KERNEL_START,
		.size = KERNEL_SIZE,
		.mask_flags = 0
	},
  #if defined(CONFIG_DUAL_IMAGE)
	[4] = {
		.name = "RUNTIME2",
		.offset = KERNEL2_START,
		.size = KERNEL2_SIZE,
		.mask_flags = 0
	},
	[5] = {
		.name = "SYSINFO",
		.offset = SYSINFO_START,
		.size = SYSINFO_SIZE,
		.mask_flags = 0
	},
  #endif /* CONFIG_DUAL_IMAGE */
#if 0
  #if defined(CONFIG_SQUASHFS_LZMA) | defined(CONFIG_CRAMFS) | defined(CONFIG_SQUASHFS)
	[4] = {
		.name = "Rootfs",
		.offset = ROOTFS_START,
		.size = ROOTFS_SIZE,
		.mask_flags = MTD_WRITEABLE /* force a read-only partition */
	}
  #else /* end of CONFIG_SQUASHFS_LZMA | CONFIG_CRAMFS | CONFIG_SQUASHFS*/
  #endif /* end of else CONFIG_SQUASHFS_LZMA | CONFIG_CRAMFS | CONFIG_SQUASHFS */
#endif
};
#elif defined(CONFIG_FLASH_LAYOUT_TYPE2)
static struct mtd_partition rtk_parts[] = {
	[0] = {
		.name = "LOADER",
		.offset = LOADER_START,
		.size = LOADER_SIZE,
		.mask_flags = MTD_WRITEABLE /* force a read-only partition */
	},
	[1] = {
		.name = "BDINFO",
		.offset = LOADER_BDINFO_START,
		.size = LOADER_BDINFO_SIZE,
		.mask_flags = 0
	},
	[2] = {
		.name = "SYSINFO",
		.offset = SYSINFO_START,
		.size = SYSINFO_SIZE,
		.mask_flags = 0
	},
	[3] = {
		.name = "JFFS2",
		.offset = JFFS2_START,
		.size = JFFS2_SIZE,
		.mask_flags = 0
	},
	[4] = {
		.name = "RUNTIME",
		.offset = KERNEL_START,
		.size = KERNEL_SIZE,
		.mask_flags = 0
	},
  #if defined(CONFIG_DUAL_IMAGE)
	[5] = {
		.name = "RUNTIME2",
		.offset = KERNEL2_START,
		.size = KERNEL2_SIZE,
		.mask_flags = 0
	}
  #endif /* CONFIG_DUAL_IMAGE */
#if 0
  #if defined(CONFIG_SQUASHFS_LZMA) | defined(CONFIG_CRAMFS) | defined(CONFIG_SQUASHFS)
	[5] = {
		.name = "Rootfs",
		.offset = ROOTFS_START,
		.size = ROOTFS_SIZE,
		.mask_flags = MTD_WRITEABLE /* force a read-only partition */
	}
  #else /* end of CONFIG_SQUASHFS_LZMA | CONFIG_CRAMFS | CONFIG_SQUASHFS*/
  #endif /* end of else CONFIG_SQUASHFS_LZMA | CONFIG_CRAMFS | CONFIG_SQUASHFS */
#endif
};
#elif defined(CONFIG_FLASH_LAYOUT_TYPE3) || defined(CONFIG_FLASH_LAYOUT_TYPE4)
static struct mtd_partition rtk_parts[] = {
    [0] = {
        .name = "LOADER",
        .offset = LOADER_START,
        .size = LOADER_SIZE,
        .mask_flags = MTD_WRITEABLE /* force a read-only partition */
    },
    [1] = {
        .name = "BDINFO",
        .offset = LOADER_BDINFO_START,
        .size = LOADER_BDINFO_SIZE,
        .mask_flags = 0
    },
    [2] = {
        .name = "SYSINFO",
        .offset = SYSINFO_START,
        .size = SYSINFO_SIZE,
        .mask_flags = 0
    },
    [3] = {
        .name = "JFFS2 CFG",
        .offset = JFFS2_CFG_START,
        .size = JFFS2_CFG_SIZE,
        .mask_flags = 0
    },
    [4] = {
        .name = "JFFS2 LOG",
        .offset = JFFS2_LOG_START,
        .size = JFFS2_LOG_SIZE,
        .mask_flags = 0
    },
    [5] = {
        .name = "RUNTIME",
        .offset = KERNEL_START,
        .size = KERNEL_SIZE,
        .mask_flags = 0
    },
#if defined(CONFIG_DUAL_IMAGE)
    [6] = {
        .name = "RUNTIME2",
        .offset = KERNEL2_START,
        .size = KERNEL2_SIZE,
        .mask_flags = 0
    }
#endif /* CONFIG_DUAL_IMAGE */

};
#endif /* CONFIG_FLASH_LAYOUT_TYPE3 */


FLASH_INFO_TYPE *flash_info[MAX_SPI_FLASH_CHIPS+1];
#else   /* end of elif !__UBOOT__&__KERNEL__ */
  #error Please implement the OS system functions
#endif /* end of else __UBOOT__&__KERNEL__ */

#if defined(CONFIG_FLASH_SPI) || !defined(__UBOOT__)
/*
 *  Set spi_chips as constant can reduce
 *  the data segment size (For flash only)
 */
static const spi_chip_info_t spi_chips[] = {
	{
		.chip_id = MX25L3235D,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "MX25L3235D",
	},
	{
		.chip_id = MX25L3205D,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "MX25L3205D",
	},
	{
		.chip_id = W25Q32,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "W25Q32",
	},
	{
		.chip_id = SST26VF032,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "SST26VF032",
	},
	{
		.chip_id = SST25VF032B,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "SST25VF032B",
	},
	{
		.chip_id = MX25L6405D,
		.chip_size = 0x00800000U,  /*  8MB*/
		.chip_name = "MX25L6405D",
	},
	{
		.chip_id = MX25L6406E,
		.chip_size = 0x00800000U,  /*  8MB*/
		.mrf_id = MFR_ID_MX,
		.chip_name = "MX25L6406E",
	},
	{
		.chip_id = S25FL064A,
		.chip_size = 0x00800000U,  /*  8MB*/
		.chip_name = "S25FL064A",
	},
	{
		.chip_id = MX25L12805D,
		.chip_size = 0x01000000U,  /* 16MB*/
		.chip_name = "MX25L12805D",
	},
	{
		.chip_id = MX25L12845E,
		.chip_size = 0x01000000U,  /* 16MB*/
		.mrf_id = MFR_ID_MX,
		.chip_name = "MX25L12845E",
	},
	{
		.chip_id = S25FL128P,
		.chip_size = 0x01000000U,  /* 16MB*/
		.chip_name = "S25FL128P",
	},
	{
		.chip_id = M25P128,
		.chip_size = 0x01000000U,  /* 16MB*/
		.chip_name = "M25P128",
	},
	{
		.chip_id = MX25L1635D,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "MX25L1635D",
	},
	{
		.chip_id = MX25L1605D,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "MX25L1605D",
	},
	{
		.chip_id = W25Q16,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "W25Q16",
	},
	{
		.chip_id = SST26VF016,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "SST26VF016",
	},
	{
		.chip_id = W25Q80,
		.chip_size = 0x00100000U,  /*  1MB*/
		.chip_name = "W25Q80",
	},
	{
		.chip_id = S25FL008A,
		.chip_size = 0x00100000U , /*  1MB*/
		.chip_name = "S25FL008A",
	},
	{
		.chip_id = MX25L8005,
		.chip_size = 0x00100000U , /*  1MB*/
		.chip_name = "MX25L8005",
	},
	{
		.chip_id = S25FL016A,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "S25FL016A",
	},
	{
		.chip_id = S25FL032A,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "S25FL032A",
	},
	{
		.chip_id = S25FL004A,
		.chip_size = 0x00080000U , /*512KB*/
		.chip_name = "S25FL004A",
	},
	{
		.chip_id = MX25L4005,
		.chip_size = 0x00080000U , /*512KB*/
		.chip_name = "MX25L4005",
	},
}; /* end of spi_chips */

static uint32 spi_flash_total_size = 0;
static uint32 spi_flash_num_of_chips = 0;
#endif /* end of CONFIG_FLASH_SPI || !__UBOOT__*/

/*
 * Macro Definition
 */
#if defined(__UBOOT__) && defined(__KERNEL__)
#elif !defined(__UBOOT__) && defined(__KERNEL__)  /* end of __UBOOT__ & __KERNEL__ */
static inline FLASH_INFO_TYPE *mtd_to_RTKSPI_MTD(struct mtd_info *mtd)
{
	return container_of(mtd, FLASH_INFO_TYPE, mtd);
};
#else   /* end of elif !__UBOOT__&__KERNEL__ */
  #error Please implement the OS system functions
#endif /* end of else __UBOOT__&__KERNEL__ */

/*
 * Function Declaration
 */
#if defined(__UBOOT__) && defined(__KERNEL__)
#elif !defined(__UBOOT__) && defined(__KERNEL__)  /* end of __UBOOT__ & __KERNEL__ */
static int rtk_spi_erase(struct mtd_info *mtd, struct erase_info *instr);
static int rtk_spi_read(struct mtd_info *mtd, loff_t from, size_t len,
	                    size_t *retlen, u_char *buf);
static int rtk_spi_write(struct mtd_info *mtd, loff_t to, size_t len,
		                 size_t * retlen, const u_char * buf);
#else   /* end of elif !__UBOOT__&__KERNEL__ */
  #error Please implement the OS system functions
#endif /* end of else __UBOOT__&__KERNEL__ */


#if defined(CONFIG_FLASH_SPI) || !defined(__UBOOT__)
static void __wait_Ready(FLASH_INFO_TYPE *info);
static int __textflash __spi_commands(FLASH_INFO_TYPE *info, spi_request_t *req);
static int __get_spi_SR(FLASH_INFO_TYPE *info);
static int __wait_WEL(FLASH_INFO_TYPE *info);

/* Function Name:
 *      __read_spi_id
 * Description:
 *      Read SPI chip id
 * Input:
 *      SPI device handler
 * Output:
 *      None
 * Return:
 *      Chip id
 * Note:
 *      The basic flash driver function
 */
static int __textflash __read_spi_id(FLASH_INFO_TYPE *info)
{
	spi_request_t req;

	CHECK_READY();
	/* Set request command&addr */
	req.cmd_t = SPI_C_RDID;
	req.address = -1;

	return(__spi_commands(info, &req));
} /* end of __read_spi_id */

/* Function Name:
 *      __get_spi_SR
 * Description:
 *      Get SPI status
 * Input:
 *      SPI device handler
 * Output:
 *      None
 * Return:
 *      SPI status
 * Note:
 *      The basic flash driver function
 */
static int __textflash __get_spi_SR(FLASH_INFO_TYPE *info)
{
	spi_request_t req;
	uint32 status,i=0;

	/* Set request command&addr */
	req.cmd_t = SPI_C_RDSR;
	req.address = -1;
	while(i<1000)
	{
		i++;
	}
	status = (__spi_commands(info, &req))>>24;

	return(status);
} /* end of __get_spi_SR */

/* Function Name:
 *      __set_spi_SR
 * Description:
 *      Set SPI status
 * Input:
 *      SPI device handler(info), write status data(sr_value)
 * Output:
 *      None
 * Return:
 *      0
 * Note:
 *      The basic flash driver function
 */
static int __textflash __set_spi_SR(FLASH_INFO_TYPE *info, unsigned char sr_value)
{
	spi_request_t req;

	/* Set write enable */
	__wait_WEL(info);

	/* Set request command&addr */
	req.cmd_t = SPI_C_WRSR;
	req.address = sr_value<<16;
	__spi_commands(info, &req);

	return(0);
} /* end of __set_spi_SR */

/* Function Name:
 *      __wait_Ready
 * Description:
 *      Wait Control Register Ready
 *      Wait Writing Progress Over
 * Input:
 *      SPI device handler(info)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      The basic flash driver function
 */
static void __textflash __wait_Ready(FLASH_INFO_TYPE *info)
{
	CHECK_READY();
	while( __get_spi_SR(info)&SPI_WIP );
} /* end of __wait_Ready */

/* Function Name:
 *      __spi_WRDI
 * Description:
 *      Write Disable
 *      All write commands(erase, page program, ...) is not available
 * Input:
 *      SPI device handler(info)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      The basic flash driver function
 */
static void __textflash __spi_WRDI(FLASH_INFO_TYPE *info)
{
	spi_request_t req;
	/* Set request command&addr, addr is X(don't care) */
	req.cmd_t = SPI_C_WRDI;
	__spi_commands(info, &req);
} /* end of __spi_WRDI */

/* Function Name:
 *      __wait_WEL
 * Description:
 *      Set Write Enable
 *      Write commands(erase, page program, ...) is available
 * Input:
 *      SPI device handler(info)
 * Output:
 *      None
 * Return:
 *      SPI status
 * Note:
 *      The basic flash driver function
 */
static int __textflash __wait_WEL(FLASH_INFO_TYPE *info)
{
	spi_request_t req_wel;
	uint32 ret;

	/* Wait Ready */
	CHECK_READY();
	while( __get_spi_SR(info)&SPI_WIP );

	/* Set Write Enable */
	do{
		/* WREN */
		req_wel.cmd_t = SPI_C_WREN;
		req_wel.address = -1;
		__spi_commands(info, &req_wel);
		CHECK_READY();
	}while( !((ret=__get_spi_SR(info))&SPI_WEL) );

	return(ret);
} /* end of __wait_WEL */

/* Function Name:
 *      __spi_commands
 * Description:
 *      All SPI commands
 *      Control the SFCSR and SFDR.
 * Input:
 *      SPI device handler(info), request command(req)
 * Output:
 *      None
 * Return:
 *      None
 *      Device ID
 * Note:
 *      The basic flash driver function
 *      Support Commands:
 *        Write Enable, Write Disable, Chip Erase, Power Down,
 *        Write Status Register, Read Status Register, Read Device ID,
 *        Read, Block Erase, Auto Address Increment,
 *        Page Program
 */
static int __textflash __spi_commands(FLASH_INFO_TYPE *info, spi_request_t *req)
{
	uint32 addr = 0;
	uint32 size = req->size;
	uint8 *buf = (uint8 *)req->buf;
	uint32 sfcsr_value = 0, sfdr_value = 0;
	uint32 ret = 0;

	/* Memory Controller Constrain, deactive, active, deactive SPI Flash */
	CHECK_READY();
	SPI_REG(SFCSR) = SPI_CS_INIT; /* deactive CS0, CS1 */
	CHECK_READY();
	SPI_REG(SFCSR) = 0;           /* active CS0,CS1 */
	CHECK_READY();
	SPI_REG(SFCSR) = SPI_CS_INIT; /* deactive CS0, CS1 */
	CHECK_READY();

	/* sfcar default value */
	sfcsr_value = (CS0 & info->flags) ? SPI_eCS0&SPI_LEN_INIT : SPI_eCS1&SPI_LEN_INIT ;

	/* set command and address */
	sfdr_value = (req->cmd_t)<<24;
	addr = req->address;

	switch( req->cmd_t ){
	/*No Address/Dummy Bytes, data length = 1*/
	case SPI_C_WREN:
	case SPI_C_WRDI:
	case SPI_C_CE:
	case SPI_C_DP:
		/* Write 1 byte command & 0 byte data, read 0 byte data */
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value;
		CHECK_READY();
		SPI_REG(SFDR) = sfdr_value;
		break;

	case SPI_C_WRSR:
		/* Write 1 byte command & 1 byte data, read 0 byte data */
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN2;
		sfdr_value |= addr;
		CHECK_READY();
		SPI_REG(SFDR) = sfdr_value;
		break;
	case SPI_C_RDSR:
	case SPI_C_RDID:
		/* Write 1 byte command & 1 byte data, read 4 byte data */
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value;
		CHECK_READY();
		SPI_REG(SFDR) = sfdr_value;
		CHECK_READY();
		SPI_REG(SFCSR) |= SPI_LEN4;
		CHECK_READY();
		ret = SPI_REG(SFDR);
		break;
	case SPI_C_READ:
		/* Write 1 byte command & 3 byte addr */
		sfcsr_value |=SPI_LEN4;
		sfdr_value |= addr;
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value;
		CHECK_READY();
		SPI_REG(SFDR) = sfdr_value;

		/* Read data, 4 byte/iteration */
		while( size>=4 ){
			CHECK_READY();
			*((uint32*) buf) = SPI_REG(SFDR);
			buf+=4;
			size-=4;
		}

		/* Read data, 1 byte/iteration */
		sfcsr_value &= SPI_LEN_INIT|SPI_LEN1;
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value;
		while( size>0 ){
			CHECK_READY();
			*(buf) = SPI_REG(SFDR)>>24;
			buf++;
			size--;
		}
		break;
	case SPI_C_FREAD:
		/* Fast Read, not Implement */
		sfcsr_value |=SPI_LEN4;
		sfdr_value |= addr;
		break;
	case SPI_C_BE:
		/* Write 1 byte command & 3 byte addr(erased address) */
		sfcsr_value |=SPI_LEN4;
		sfdr_value |= addr;
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value;
		CHECK_READY();
		SPI_REG(SFDR) = sfdr_value;
		break;
	case SPI_C_AAI:
		/* Auto Address Increment */
		if( 0==size ){
			break;
		}
		/* Write 1 byte command & 3 byte addr */
		sfdr_value |= addr;
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
		CHECK_READY();
		SPI_REG(SFDR) = sfdr_value;
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN2;

		/*
		 * Write 1 or 2 byte data at the first time
		 */
		/* only write 1 byte data */
		if( 1==size ){
			CHECK_READY();
			SPI_REG(SFDR) = (((uint32)(*((uint16*) buf)))<<16 | 0x00FF0000);
			buf += 1;
			size = 0;
			__wait_Ready(info);
			req->cmd_t = SPI_C_WRDI;
			__spi_commands(info, req);
			__wait_Ready(info);
			break;
		}
		CHECK_READY();

		/* only write 2 byte data */
		SPI_REG(SFDR) = ((uint32)(*((uint16*) buf)))<<16;
		buf+=2;
		size-=2;

		/* Write data, 2 byte/iteration */
		while( size>=2 ){
			__wait_Ready(info);
			SPI_REG(SFCSR) = SPI_CS_INIT;
			CHECK_READY();
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN3;
			CHECK_READY();
			SPI_REG(SFDR) = (SPI_C_AAI<<24) | ((uint32)(*((uint16*) buf)))<<8;
			buf+=2;
			size-=2;
		}

		/* Write data, 1 byte/iteration */
		while( size>0 ){
			__wait_Ready(info);
			SPI_REG(SFCSR) = SPI_CS_INIT;
			CHECK_READY();
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN3;
			/* or 0x0000FF00 to keep the original data in spi flash (AAI mode only)*/
			CHECK_READY();
			SPI_REG(SFDR) = (SPI_C_AAI<<24) | ((uint32)(*((uint16*) buf)))<<8 | 0x0000FF00;
			buf++;
			size--;
		}

		/* Wait Ready */
		__wait_Ready(info);
		req->cmd_t = SPI_C_WRDI;
		__spi_commands(info, req);
		__wait_Ready(info);
		break;
	case SPI_C_PP:
		/* Write 1 byte command & 3 byte addr */
		sfdr_value |= addr;
		CHECK_READY();
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
		CHECK_READY();
		SPI_REG(SFDR) = sfdr_value;

		/* Write data, 4 byte/iteration */
		if( 0==(((int)buf)%sizeof(uint32)) ){
			while( sizeof(uint32)<=size ){
				CHECK_READY();
				SPI_REG(SFDR) = *((uint32*) buf);
				buf  += sizeof(uint32);
				size -= sizeof(uint32);
			}
		}
		CHECK_READY();

		/* Write data, 1 byte/iteration */
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1;
		while( size>0 ){
			CHECK_READY();
			SPI_REG(SFDR) = ((uint32)(*buf)) << 24;
			buf++;
			size--;
		}

		/* Wait Ready */
		__wait_Ready(info);
		break;
	case SPI_C_RES:
		/* Not implement */
		sfcsr_value |= SPI_LEN4;
		break;
	default:
		break;
	};

	/* Memory Controller Constrain, deactive, active, deactive SPI Flash,
	 * MMIO read Flash data
	 */
	CHECK_READY();
	SPI_REG(SFCSR) = SPI_CS_INIT; /* deactive CS0, CS1 */
	CHECK_READY();
	SPI_REG(SFCSR) = 0;           /* active CS0,CS1 */
	CHECK_READY();
	SPI_REG(SFCSR) = SPI_CS_INIT; /* deactive CS0, CS1 */
	CHECK_READY();
	SPI_REG(FLASHBASE);

	return ret;
} /* end of __spi_commands */


#if defined(ENABLE_SPI_FLASH_READ) || !defined(__UBOOT__)
/* Function Name:
 *      __read_spi_flash
 * Description:
 *      SPI flash read
 * Input:
 *      SPI device handler(info), read buffer(buff), read size(size)
 *      flash start addr(start_addr)
 * Output:
 *      None
 * Return:
 *      0
 * Note:
 *      The basic flash driver function
 */
static int __read_spi_flash(FLASH_INFO_TYPE *info, uint8 *buff, uint32 size, void* start_addr)
{
  #if defined(ENABLE_SPI_FLASH_FORMAL_READ)
	spi_request_t req;
	uint32 offset = 0;

	__wait_Ready(info);

	/* Set request command */
	req.cmd_t = SPI_C_READ;

	/* read SPI_MAX_TRANSFER_SIZE data per iteration */
	while( size >= SPI_MAX_TRANSFER_SIZE ){
		req.address = (uint32) start_addr + offset;
		req.buf = buff+offset;
		req.size = SPI_MAX_TRANSFER_SIZE;
		size -= SPI_MAX_TRANSFER_SIZE;
		__spi_commands(info, &req);
		offset += SPI_MAX_TRANSFER_SIZE;
	}

	/* read data for size < SPI_MAX_TRANSFER_SIZE */
	if( 0!=size ){
		req.address = (uint32) start_addr + offset;
		req.buf = buff+offset;
		req.size = size;
		__spi_commands(info, &req);
	}
  #else  /* end of else ENABLE_SPI_FLASH_FORMAL_READ */
	memcpy(buff, (const void*) (info->BlockBase+start_addr), size);
  #endif /* end of ENABLE_SPI_FLASH_FORMAL_READ */

	return 0;
} /* end of __read_spi_flash */
#endif /* end of ENABLE_SPI_FLASH_READ */

/* Function Name:
 *      __write_spi_flash
 * Description:
 *      Write SPI flash
 * Input:
 *      SPI device handler(info), read data(buff), data size(size)
 *      flash start addr(start_addr, 0~Flash_Size-4 is valid value)
 * Output:
 *      None
 * Return:
 *      0
 * Note:
 *      The basic flash driver function
 */
static int __textflash __write_spi_flash(FLASH_INFO_TYPE *info, const uint8 *buff, uint32 size, void *start_addr)
{
	spi_request_t req;
	uint32 offset = 0;
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	uint32 i = 0;
	uint32 org_size = size;
#endif /* SPI_SHOW_PROGRESS */

	/* WREN */
	if( SST25VF032B==info->flash_id ){
		__set_spi_SR(info, 0);
	}
	__wait_WEL(info);

	/*Page Program*/
	if( SST25VF032B == (info->flash_id&SPI_FLASH_ID_MASK) ){
		req.address = (uint32) start_addr;
		req.buf = (uint8 *)buff;
		req.size = size;
		req.cmd_t = SPI_C_AAI;
		__spi_commands(info, &req);
		return 0;
	}else{
		req.cmd_t = SPI_C_PP;
	}

#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	OS_PRINTF("\n");
#endif /* SPI_SHOW_PROGRESS */

	/* start_addr is not aligned on SPI_MAX_TRANSFER_SIZE */
	if( 0 != ( (uint32)start_addr%SPI_MAX_TRANSFER_SIZE) ){
		req.address = (uint32) start_addr;
		req.buf = (uint8 *)buff;
		req.size = SPI_MAX_TRANSFER_SIZE-((uint32)start_addr%SPI_MAX_TRANSFER_SIZE);
		if( size<req.size ){
			req.size = size;
		}
		size -= req.size;
		offset += req.size;
		__spi_commands(info, &req);
	}

	/* start_addr+offset is aligned on SPI_MAX_TRANSFER_SIZE */
	while( size >= SPI_MAX_TRANSFER_SIZE ){
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
		if( 0==i%10 ){
			OS_PRINTF("\rWrite: %3d%%", offset*100/org_size);
		}
		i++;
#endif /* SPI_SHOW_PROGRESS */
		__wait_WEL(info);
		req.address = (uint32) start_addr + offset;
		req.buf = (uint8 *)(buff+offset);
		req.size = SPI_MAX_TRANSFER_SIZE;
		size -= SPI_MAX_TRANSFER_SIZE;
		offset += SPI_MAX_TRANSFER_SIZE;
		__spi_commands(info, &req);
	}
	__wait_WEL(info);

	/* size is less than SPI_MAX_TRANSFER_SIZE and larger than 0*/
	if( 0!=size ){
		req.address = ((uint32) start_addr) + offset;
		req.buf = (uint8 *)(buff+offset);
		req.size = size;
		__spi_commands(info, &req);
	}
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	OS_PRINTF("\rWrite: 100%%\n");
#endif /*SPI_SHOW_PROGRESS*/

	return 0;
} /* end of __write_spi_flash */

/* Function Name:
 *      __erase_spi_flash
 * Description:
 *      Erase SPI flash
 * Input:
 *      SPI device handler(info), erase command(set), erase start addr(addr)
 *      (addr, 0~Flash_Size-4 is valid value)
 * Output:
 *      None
 * Return:
 *      0
 * Note:
 *      The basic flash driver function
 */
static int __textflash __erase_spi_flash(FLASH_INFO_TYPE *info, spi_cmdType_t set, void *addr)
{
	spi_request_t req;
	__wait_WEL(info);
	req.cmd_t = set;
	req.address = (uint32) addr;
	__spi_commands(info, &req);

	return(0);
} /* end of __erase_spi_flash */
#endif /*CONFIG_FLASH_SPI && !__UBOOT__*/

#if defined(__UBOOT__) && defined(CONFIG_FLASH_SPI)
/* Function Name:
 *      flash_init
 * Description:
 *      U-Boot flash init function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Total Flash Size
 * Note:
 *      U-Boot function
 */
__textflash
unsigned long flash_init (void)
{
	int i=0, j=0, offset=0;
	int flashbase = FLASHBASE;
	spi_flash_total_size= 0;

	/* Set flash endian as big-endian */
	SPI_REG(SFCR) |= (SFCR_EnableWBO);
	SPI_REG(SFCR) |= (SFCR_EnableRBO);

	/* Probe SPI flash*/
	for(i=0; i<MAX_SPI_FLASH_CHIPS; i++){
		flash_info[i].flags = (CS0<<i)|R_MODE;
		flash_info[i].size = 0;
		/* Read ID */
		flash_info[i].flash_id = __read_spi_id(&flash_info[i])>>8;

		/* Check the flash_id */
		for(j=0; j<sizeof(spi_chips)/sizeof(spi_chip_info_t); j++){
			if( (flash_info[i].flash_id&SPI_FLASH_ID_MASK) == (spi_chips[j].chip_id) ){
				PRINT_DEVICE("Probe: SPI CS%d Flash Type %s\n", i, spi_chips[j].chip_name);
				flash_info[i].size = spi_chips[j].chip_size;
				if( SST25VF032B==(flash_info[i].flash_id&SPI_FLASH_ID_MASK) ){
					__set_spi_SR(&flash_info[i], 0);
					__spi_WRDI(&flash_info[i]);
				}
				break;
			}
		}
		/* Not a known flash id for this driver */
		if( j==(sizeof(spi_chips)/sizeof(spi_chip_info_t)) ){
			PRINT_DEVICE("Unknown SPI CS%d Flash ID=0x%06x.\n", i, flash_info[i].flash_id);
			flash_info[i].flash_id = FLASH_UNKNOWN;
			continue;
		}
		/* Fill the block's boundary of flash handler */
		for(j=0,offset=0; offset<(flash_info[i].size); j++,offset+=SPI_BLOCK_SIZE){
			flash_info[i].start[j] = flashbase+offset;
		}
		/* Set flash size in SFCR register */
		if( 0==i ){
			SPI_REG(SFCR) &= ~(SFCR_SFSIZE(0x7));
			switch( flash_info[i].size ){
				case 0x00020000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x0);
					break;
				case 0x00040000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x1);
					break;
				case 0x00080000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x2);
					break;
				case 0x00100000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x3);
					break;
				case 0x00200000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x4);
					break;
				case 0x00400000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x5);
					break;
				case 0x00800000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x6);
					break;
				case 0x01000000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x7);
					break;
				default:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x7);
					break;
			}
		}
		/* Set size, sector count, and total size/chip */
		flash_info[i].sector_count = flash_info[i].size/SPI_BLOCK_SIZE;
		flashbase += flash_info[i].size;
		spi_flash_total_size += flash_info[i].size;
		spi_flash_num_of_chips++;
	}
	/* Error check, no SPI flash is detected */
	if( 0 == spi_flash_num_of_chips ){
		OS_PRINTF("No SPI Flash Detected!!!\n");
		while(1);
	}

	/* Check flash size and CFG_ENV_OFFSET */
	if( 0!=(CFG_ENV_OFFSET%SPI_BLOCK_SIZE) || spi_flash_total_size<CFG_ENV_OFFSET ){
		OS_PRINTF("CFG_ENV_OFFSET=0x%08x is defined error!\n", CFG_ENV_OFFSET);
		OS_PRINTF("CFG_ENV_OFFSET must be aligned to 0x%08x, smaller than 0x%08x", SPI_BLOCK_SIZE, spi_flash_total_size);
		while(1);
	}
	/* Software flash protect, the ENV block must be protected */
	if( flash_info[0].size > CFG_ENV_OFFSET ){
		flash_info[0].protect[ CFG_ENV_OFFSET/SPI_BLOCK_SIZE ] = 1;
	}else{
		flash_info[1].protect[(CFG_ENV_OFFSET-flash_info[0].size)/SPI_BLOCK_SIZE] = 1;
	}
	return spi_flash_total_size;
} /* end of flash_init */

/* Function Name:
 *      flash_print_info
 * Description:
 *      Show flash information
 * Input:
 *      SPI device handler(info)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      U-Boot function
 */
void flash_print_info (FLASH_INFO_TYPE * info)
{
	int i;
	OS_PRINTF("SPI FLASH:\n");
	OS_PRINTF("size:%d KB\n", info->size>>10);
	for(i=0; i<(info->size/SPI_BLOCK_SIZE); i++){
		if( info->protect[i] ){
			OS_PRINTF("[%03d:%08x]   RO   ", i, (info->start[i]));
		}else{
			OS_PRINTF("[%03d:%08x]        ", i, (info->start[i]));
		}
		if( 0==((i+1)%5) && 0!=i ){
			OS_PRINTF("\n");
		}
	}
	OS_PRINTF("\n");
} /* end of flash_print_info */

/* Function Name:
 *      flash_erase
 * Description:
 *      Erase flash
 * Input:
 *      flash handler(info), first sector(s_first), last sector(s_last)
 * Output:
 *      None
 * Return:
 *      0 - OK, always return 0
 *      1 - Timeout
 *      2 - Flash not erase
 * Note:
 *      U-Boot function
 */
int __textflash flash_erase (FLASH_INFO_TYPE * info, int s_first, int s_last)
{
	uint32 offset;
	uint32 index;

	/* Check sector range */
	if( (s_first>=info->sector_count) || (s_last>=info->sector_count) ){
		OS_PRINTF("[%s][%s][%d]Out of Rande, Addr=%08x\n", \
			__FILE__, __FUNCTION__, __LINE__, info->start[0]);
		while(1);
	}

	/* Set flash offset(from address 0) and sector index */
	offset = s_first*SPI_BLOCK_SIZE;
	index = s_first;

	/* Erase sectors/blocks */
	for(; offset<=s_last*SPI_BLOCK_SIZE; offset+=SPI_BLOCK_SIZE, index++){
		/* Check software protect */
		if( 0 != info->protect[index] ){
#if !defined(CONFIG_FLASH_SRAM_ONLY)
			OS_PRINTF("Skip Readonly Block on block[%03d]\n", index);
			continue;
#endif
		}
		/* Call basic erase function */
		__erase_spi_flash(info, SPI_C_BE, (void*) (offset));
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
		if( s_last != s_first ){
			OS_PRINTF("\rErase: %3d%%  ",(index-s_first)*100/(s_last-s_first));
		}
#endif /*SPI_SHOW_PROGRESS*/
	}
	/* Wait all ready */
	__wait_Ready(info);
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	OS_PRINTF("\rErase: 100%%\n");
#endif /*SPI_SHOW_PROGRESS*/
	return 0;
} /* end of flash_erase */

/* Function Name:
 *      write_buff
 * Description:
 *      Write data into flash
 * Input:
 *      flash handler(info), source addr(src), dest address(addr)
 *      data size(cnt)
 *      addr: (FLASHBASE+(0 ~ Flash_Size-4)) is valid value
 * Output:
 *      None
 * Return:
 *      0 - OK, always return 0
 *      1 - Fail
 * Note:
 *      U-Boot function
 */
int __textflash write_buff (FLASH_INFO_TYPE * info, uchar * src, ulong addr, ulong cnt)
{
	uint32 dstAddr = (uint32) addr;
	uint32 srcAddr = (uint32) src;

	/* Check Size */
	if( 0 == cnt ){
		OS_PRINTF("Warning: Flash Write cnt=0\n");
		return 0;
	}

	/* Check flash range */
	if( (dstAddr-info->start[0]+cnt)> info->size ){
		OS_PRINTF("Flash Write Out of range\n");
		return 2;
	}

	/* Call basic write function */
	__write_spi_flash(info,
		(uint8*) (srcAddr),
		(uint32) cnt,
		(void *) (dstAddr-info->start[0]));

	return 0;
} /* end of write_buff */

#if defined(CONFIG_FLASH_SRAM_ONLY)
/* Function Name:
 *      flashonly_memcpy
 * Description:
 *      memory copy in flash only
 * Input:
 *      dst addr(dst), src addr(src), size(size)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      U-Boot function
 */
static void __textflash flashonly_memcpy(void *dst, void* src, ulong size)
{
	int i;
	for(i=0; i<size; i++){
		*(((char*)dst)+i) = *(((char*)src)+i);
	}
} /*end of flashonly_memcpy*/

/*-----------------------------------------------------------------------
 * Update loader when CONFIG_FLASH_SRAM_ONLY is defined
 * This function is never return.
 * This function reboot by jumping to flash start address
 * returns value: 1
 -----------------------------------------------------------------------*/
/* Function Name:
 *      update_loader
 * Description:
 *      update U-Boot via flashonly_memcpy
 * Input:
 *      source address(addr_first), size(size)
 * Output:
 *      None
 * Return:
 *      1 - N/A
 * Note:
 *      U-Boot function
 *      This function is never return.
 */
int __textflash update_loader(ulong addr_first, ulong size)
{
	int i;
	unsigned char buf[128];
	void (*f)(void) = (void *) (0xbfc00000);
	unsigned int offset = 0;

	/* Check size */
	if( size>flash_info[0].size ){
		OS_PRINTF("Outside available Flash\n");
		return (-1);
	}

	/* Unprotect all flash blocks */
	for(i=0; i<CFG_MAX_FLASH_SECT; i++){
		flash_info[0].protect[i] = 0;
	}

	/* Erase flash blocks */
	for(i=0; i<flash_info[0].sector_count; i++){
		if( (flash_info[0].start[i]-CFG_FLASH_BASE) > size ){
			flash_erase( &flash_info[0], 0, i);
			break;
		}
	}

	/* Update U-Boot image of flash */
	for(i=0; i<=size; i+=sizeof(buf)){
		if( (size-i)>= sizeof(buf) ){
			flashonly_memcpy(buf, (void *)(addr_first+i), sizeof(buf));
			write_buff (&flash_info[0], buf, (CFG_FLASH_BASE+i+offset), sizeof(buf));
		}else if( 0 != (size-i) ){
			flashonly_memcpy(buf, (void *)(addr_first+i), size-i);
			write_buff (&flash_info[0], buf, (CFG_FLASH_BASE+i+offset), size-i);
		}
	}

	/* Jump to the new U-Boot, never return */
	f();
	/* Never Do This */
	return 1;
} /* end of update_loader */
#endif /*CONFIG_FLASH_SRAM_ONLY*/


#elif !defined(__UBOOT__) && defined(__KERNEL__)  /*Else __UBOOT__*/
/*-----------------------------------------------------------------------
 -----------------------------------------------------------------------*/
/* Function Name:
 *      spidev_init
 * Description:
 *      Linux flash init function
 *      Set mtd structure
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0         - OK
 *      -ENOMEM   - No Memory
 *      -ENODEV   - No Device
 * Note:
 *      1. In Realtek's environment, the max spi flash number is 2.
 *      2. flash_info[0].BlockBase = flashbase of the first chip
 *      3. flash_info[1].BlockBase = flashbase of the first chip
 *      4. flash_info[2].BlockBase = flashbase of the second chip
 *      5. flash_info[0].mtd.size = flash_info[1]->mtd.size+flash_info[2]->mtd.size
 *      6. flash_info[1] => chip1
 *      6. flash_info[2] => chip2
 *      6. flash_info[0] => chip1 + chip2
 *
 *        -----------------------------
 *        |  chip1     |     chip2    |
 *        -----------------------------
 *        ^            ^
 *   flash_info[0]=====^===============
 *   flash_info[1]=====^
 *                 flash_info[2]=======
 */
static int __init spidev_init(void)
{
	int i=0, j=0;
	int flashbase = FLASHBASE;
	RTK_SPI_MTD *flash;
	spi_flash_total_size= 0;

	/* Set flash endian as big-endian */
	SPI_REG(SFCR) |= (SFCR_EnableWBO);
	SPI_REG(SFCR) |= (SFCR_EnableRBO);

	/* Get and set mtd structure */
	flash = kzalloc(sizeof *flash, SLAB_KERNEL);
	if ( !flash ){
		return -ENOMEM;
	}
	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = SPI_MAX_TRANSFER_SIZE;
	flash->mtd.owner = THIS_MODULE;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.erasesize = SPI_BLOCK_SIZE;
	flash->mtd.erase = rtk_spi_erase;
	flash->mtd.read = rtk_spi_read;
	flash->mtd.write = rtk_spi_write;
	flash->flags = 0;

	/* flash_info[0] => group two spi chips */
	flash_info[0] = flash;

	/* Probe SPI flash */
	for(i=1; i<=MAX_SPI_FLASH_CHIPS; i++){
		flash_info[i] = kzalloc(sizeof *flash, SLAB_KERNEL);
		if ( !flash_info[i] ){
			return -ENOMEM;
		}
		flash_info[i]->mtd.type = MTD_NORFLASH;
		flash_info[i]->mtd.writesize = SPI_MAX_TRANSFER_SIZE;
		flash_info[i]->mtd.owner = THIS_MODULE;
		flash_info[i]->mtd.flags = MTD_CAP_NORFLASH;
		flash_info[i]->mtd.erasesize = SPI_BLOCK_SIZE;
		flash_info[i]->mtd.erase = rtk_spi_erase;
		flash_info[i]->mtd.read = rtk_spi_read;
		flash_info[i]->mtd.write = rtk_spi_write;
		flash_info[i]->flags = (CS0<<(i-1))|R_MODE;

		/* Check the flash_id */
		flash_info[i]->flash_id = __read_spi_id(flash_info[i])>>8;
		for(j=0; j<sizeof(spi_chips)/sizeof(spi_chip_info_t); j++){
			if( (flash_info[i]->flash_id&SPI_FLASH_ID_MASK) == (spi_chips[j].chip_id) ){
				OS_PRINTF("Probe: SPI CS%d Flash Type %s\n", i, spi_chips[j].chip_name);
				flash_info[i]->mtd.size = spi_chips[j].chip_size;
				flash_info[i]->mtd.name = spi_chips[j].chip_name;
				if( SST25VF032B==flash_info[i]->flash_id ){
					__set_spi_SR(&flash_info[i], 0);
					__spi_WRDI(flash_info[i]);
				}
				break;
			}
		}
		/* Not a known flash id for this driver */
		if( j==(sizeof(spi_chips)/sizeof(spi_chip_info_t)) ){
			OS_PRINTF("Unknown SPI CS%d Flash ID=0x%06x.\n", i, flash_info[i]->flash_id);
			flash_info[i]->flash_id = FLASH_UNKNOWN;
			flash_info[i] = NULL;
			continue;
		}

		/* Set flags, size, total chips */
		flash->flags |= flash_info[i]->flags;
		flash_info[i]->BlockBase = flashbase;
		flashbase += flash_info[i]->mtd.size;
		spi_flash_total_size += flash_info[i]->mtd.size;
		spi_flash_num_of_chips++;

		/* Set flash size in SFCR register */
		if( 0==i ){
			SPI_REG(SFCR) &= ~(SFCR_SFSIZE(0x7));
			switch( flash_info[i]->mtd.size ){
				case 0x00020000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x0);
					break;
				case 0x00040000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x1);
					break;
				case 0x00080000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x2);
					break;
				case 0x00100000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x3);
					break;
				case 0x00200000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x4);
					break;
				case 0x00400000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x5);
					break;
				case 0x00800000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x6);
					break;
				case 0x01000000U:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x7);
					break;
				default:
					SPI_REG(SFCR) |= SFCR_SFSIZE(0x7);
					break;
			}
		}

		/* Init mutex of per chip */
		init_MUTEX(&flash_info[i]->lock);
		flash_info[i]->mtd.numeraseregions = 0;
	}
	/* Init mutex */
	init_MUTEX(&flash->lock);

	/* Insert MTD device*/
	flash->BlockBase = flash_info[0]->BlockBase;
	flash->mtd.size = spi_flash_total_size;
	flash->mtd.name = "Total SPI FLASH";
	flash->mtd.numeraseregions = 0;
	/* flash size must larger then 4MB for turnkey system */
	if( flash->mtd.size >= 0x00400000U ){
		if ( 1==add_mtd_partitions( &(flash->mtd), rtk_parts, \
					sizeof(rtk_parts)/sizeof(rtk_parts[0])) ){
			OS_PRINTF("[%s:%d]add mtd device fail!\n", __FILE__, __LINE__);
			return -ENODEV;
		}
	}
	else{
		OS_PRINTF("Flash Size must be larger than 4MB!\n");
	}
	return 0;
} /* end of spidev_init */

/*-----------------------------------------------------------------------
 * Erase SPI Flash - On Linux
 -----------------------------------------------------------------------*/
/* Function Name:
 *      rtk_spi_erase
 * Description:
 *      Erase flash
 * Input:
 *      mtd handler(mtd), erase information(instr)
 * Output:
 *      None
 * Return:
 *      0 - OK, always return 0
 *      1 - Timeout
 *      2 - Flash not erase
 * Note:
 *      Linux function
 */
static int rtk_spi_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	uint32 addr,len;
	RTK_SPI_MTD *flash;

	/* Type cast */
	flash = mtd_to_RTKSPI_MTD(mtd);

	/* Sanity checks */
	if ( 0 != (instr->addr % mtd->erasesize) || \
		 0 != (instr->len % mtd->erasesize) ) {
		return -EINVAL;
	}

	/* Check sector range */
	if ( (instr->addr + instr->len) > flash->mtd.size ){
		return -EINVAL;
	}

	/* Set flash addrddress , length, get lock*/
	addr = instr->addr;
	len = instr->len;
	down(&flash->lock);

	/* Erase sectors/blocks */
	while( len ){
		if( addr<=flash_info[1]->mtd.size ){
			__erase_spi_flash(flash_info[1], SPI_C_BE, (void *)addr);
		}else{
			__erase_spi_flash(flash_info[2], SPI_C_BE, (void *)(addr-flash_info[1]->mtd.size));
		}
		addr += mtd->erasesize;
		len -= mtd->erasesize;
	}

	/* Release lock */
	up(&flash->lock);
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);
	return 0;
} /* end of rtk_spi_erase*/

/* Function Name:
 *      rtk_spi_write
 * Description:
 *      Write data into flash
 * Input:
 *      mtd handler(mtd), flash addr(to_addr), size(len)
 *      write data(buf)
 * Output:
 *      wrote size(retlen), always equal to size
 * Return:
 *      0       - OK, always return 0
 *      -EINVAL - Invalid parameter
 * Note:
 *      Linux function
 */
static int rtk_spi_write(struct mtd_info *mtd, loff_t to_addr, size_t len,
		size_t * retlen, const u_char * buf)
{
	RTK_SPI_MTD *flash;
	unsigned int offset = 0;
	unsigned int to = (unsigned int) to_addr;

	/* Type cast */
	flash = mtd_to_RTKSPI_MTD(mtd);

	/* Check len */
	if ( !len ){
		return 0;
	}

/*
 * 1. In Realtek's environment, the max spi flash number is 2.
 * 2. flash_info[0].BlockBase = flashbase of the first chip
 * 3. flash_info[1].BlockBase = flashbase of the first chip
 * 4. flash_info[2].BlockBase = flashbase of the second chip
 * 5. flash_info[0].mtd.size = flash_info[1]->mtd.size+flash_info[2]->mtd.size
 * 6. to >= 0
 *
 *           -----------------------------
 *           |  chip1     |     chip2    |
 *           -----------------------------
 * C1                 |------------------------|
 * C2          |----|
 * C3                 |----------|
 * C4                         |--------|
 * All write conditions (C1, C2, C3, C4)
 */
	/* Check the different conditions */
	if( (to + len) > flash->mtd.size ){                      /*C1*/
		return -EINVAL;
	}else if( ((to + len)<flash_info[1]->mtd.size) )         /*C2*/
	{
		/* Write the first chip */
		down(&flash->lock);
		__write_spi_flash(flash_info[1], buf, len, (void*) to);
		up(&flash->lock);
	}else if( (to<flash_info[1]->mtd.size)        &&         /*C3*/\
	            ((to + len)>flash_info[1]->mtd.size)    ){
		/* Write operation crosses two chips */
		offset = (flash_info[1]->mtd.size)-to;
		down(&flash->lock);

		/* Write the first chip */
		__write_spi_flash(flash_info[1], buf, offset, (void*) to);

		/* Write the second chip */
		__write_spi_flash(flash_info[2], (buf+offset), (len-offset), (void*) 0);
		up(&flash->lock);
	}else{      /*from>[1]BlockBase && (from+len)<total_mtd_size, C4*/
		/* Write the first chip */
		down(&flash->lock);
		__write_spi_flash(flash_info[2], buf, len, (void*) (to-flash_info[1]->mtd.size));
		up(&flash->lock);
	}

	*retlen = len;
	return 0;
} /* end of rtk_spi_write */

/* Function Name:
 *      rtk_spi_read
 * Description:
 *      Read data from flash
 * Input:
 *      mtd handler(mtd), flash addr(from_addr), size(len)
 *      from_addr : (0 ~ Flash_Size-4) is valid
 * Output:
 *      read buffer(buf)
 *      read size(retlen), always equal to size
 * Return:
 *      0       - OK, always return 0
 *      -EINVAL - Invalid parameter
 * Note:
 *      Linux function
 */
static int rtk_spi_read(struct mtd_info *mtd, loff_t from_addr, size_t len,
	size_t *retlen, u_char *buf)
{
	RTK_SPI_MTD *flash = mtd_to_RTKSPI_MTD(mtd);
	unsigned int offset = 0;
	unsigned int from = from_addr;

	/* sanity checks */
	if( !len ){
		return 0;
	}

/*
 * 1. In Realtek's environment, the max spi flash number is 2.
 * 2. flash_info[0].BlockBase = flashbase of the first chip
 * 3. flash_info[1].BlockBase = flashbase of the first chip
 * 4. flash_info[2].BlockBase = flashbase of the second chip
 * 5. flash_info[0].mtd.size = flash_info[1]->mtd.size+flash_info[2]->mtd.size
 * 6. from >= 0
 *
 *           -----------------------------
 *           |  chip1     |     chip2    |
 *           -----------------------------
 * C1                 |------------------------|
 * C2          |----|
 * C3                 |----------|
 * C4                         |--------|
 * All read conditions (C1, C2, C3, C4)
*/
	if( (from + len) > flash->mtd.size ){                  /*C1*/
		return -EINVAL;
	}else if( ((from + len)<flash_info[1]->mtd.size) )     /*C2*/
	{
		/* Read the first chip */
		down(&flash->lock);
		__read_spi_flash(flash_info[1], buf, len, (void*) from);
		up(&flash->lock);
	}else if(  (from<flash_info[1]->mtd.size)          &&  /*C3*/\
	            ((from + len)>flash_info[1]->mtd.size)    ){
		/* Read operation crosses two chips */
		offset = (flash_info[1]->mtd.size)-from;
		down(&flash->lock);

		/* Read the first chip */
		__read_spi_flash(flash_info[1], buf, offset, (void*) from);

		/* Read the second chip */
		__read_spi_flash(flash_info[2], (buf+offset), (len-offset), (void*) 0);
		up(&flash->lock);
	}else{      /*from>[1]mtd.size && (from+len)<total_mtd_size, C4*/
		/* Read the first chip */
		down(&flash->lock);
		__read_spi_flash(flash_info[2], buf, len, (void*) (from-flash_info[1]->mtd.size));
		up(&flash->lock);
	}
	*retlen = len;
	return 0;
} /* end of rtk_spi_read */

/* Function Name:
 *      spidev_exit
 * Description:
 *      Exit and remove spi device/driver module
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      Linux function
 */
static void __exit spidev_exit(void)
{
	int i;
	/* remove mtd device, free flash_info */
	for(i=0; i<MAX_SPI_FLASH_CHIPS; i++){
		if( NULL!=flash_info[i] ){
			del_mtd_device( &(flash_info[i]->mtd));
			kfree(flash_info[i]);
		}
	}
} /* end of spidev_exit */
module_init(spidev_init);
module_exit(spidev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTK SPI Flash Driver");
#else /*End __KERNEL__*/
#endif /*End __UBOOT__&&__KERNEL__*/
