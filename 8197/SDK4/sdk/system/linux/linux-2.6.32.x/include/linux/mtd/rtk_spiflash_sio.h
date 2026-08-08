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

#ifndef _RTK_FLASH_SPI_H
#define _RTK_FLASH_SPI_H

/*
 * Include Files
 */
  #ifdef __UBOOT__
  #else
    #include <linux/mtd/rtk_flash_common.h>
  #endif

/* U-Boot/Linux Common Declaration */

/*
 * Symbol Definition
 */
/*   Only support single I/O on all chips    */
/*  Spanson Flash  */
#define S25FL004A   (0x00010212)
#define S25FL008A   (0x00010213)
#define S25FL016A   (0x00010214)
#define S25FL032A   (0x00010215)
#define S25FL064A   (0x00010216)    /*supposed support*/
#define S25FL128P   (0x00012018)    /*only S25FL128P0XMFI001, Uniform  64KB secotr*/
                                /*not support S25FL128P0XMFI011, Uniform 256KB secotr*/
                                /*because #define SPI_BLOCK_SIZE 65536  */

/*  MXIC Flash  */
#define MX25L4005   (0x00C22013)
#define MX25L8005   (0x00C22014)
#define MX25L1605D  (0x00C22015)
#define MX25L3205D  (0x00C22016)  /*supposed support*/
#define MX25L6405D  (0x00C22017)
#define MX25L12805D (0x00C22018)

/*  SST Flash  */
#define SST25VF032B (0x00BF254A)

/*  Windbond  */
#define W25Q32      (0x00EF4016)

/*  Numonyx  */
#define M25P128      (0x00202018)

/*  List of supported Multi I/O chip   */
/*  Spanson Flash   */
/*  MXIC Flash      */
#define MX25L1635D  (0x00C22415)  /*supposed support*/
#define MX25L3235D  (0x00C25E16)  /*supposed support*/
/*  SST Flash       */
#define SST26VF016  (0x00BF2601)
#define SST26VF032  (0x00BF2602)
/*  WindBond Flash  */
#define W25Q80      (0x00EF4014)
#define W25Q16      (0x00EF4015)
#define W25Q32      (0x00EF4016)

#define SPI_FLASH_ID_MASK (0x00FFFFFF)

typedef unsigned int   uint32;
typedef unsigned short uint16;
typedef unsigned char  uint8;
typedef int             int32;

#define SPI_REG(reg)      *((volatile uint32 *)(reg))
#define SPI_BLOCK_SIZE    (65536)  /* 64KB */
#define SPI_SECTOR_SIZE   (4096)  /*  4KB */
//#define ENABLE_SPI_FLASH_FORMAL_READ
#undef ENABLE_SPI_FLASH_FORMAL_READ


#define SFRB                  (0xB8001200)   /*SPI Flash Register Base*/
#define SFCR                  (SFRB)       /*SPI Flash Configuration Register*/
	#define SFCR_CLK_DIV(val) ((val)<<29)
	#define SFCR_EnableRBO    (1<<28)
#define SFCR_EnableWBO        (1<<27)
	#define SFCR_RD_MODE      (1<<26)
	#define SFCR_SFSIZE(val)  ((val)<<23) /*3 bit, 111*/
	#define SFCR_SPI_TCS(val) ((val)<<19) /*4 bit, 1111*/
	#define SFCR_RD_OPT       (1<<18)
#define SFCSR                 (SFRB+0x04)   /*SPI Flash Control&Status Register*/
	#define SFCSR_SPI_CSB0    (1<<31)
	#define SFCSR_SPI_CSB1    (1<<30)
	#define SFCSR_LEN         (3<<28)
	#define SFCSR_SPI_RDY     (1<<27)
	#define SFCSR_IO_WIDTH    (3<<25)
#define SFDR                  (SFRB+0x08)   /*SPI Flash Data Register*/

#define SPI_CS0               (0x80000000)
#define SPI_CS1               (0x40000000)
#define SPI_eCS0              (0x48000000)   /* and SFCSR to active CS0 */
#define SPI_eCS1              (0x88000000)   /* and SFCSR to active CS1 */
#define SPI_WIP               (0x00000001)   /* Write In Progress */
#define SPI_WEL               (0x00000002)   /* Write Enable Latch */
#define SPI_LEN_INIT          (0xCFFFFFFF) /* and SFCSR to init */
#define SPI_LEN4              (0x30000000)
#define SPI_LEN3              (0x20000000)
#define SPI_LEN2              (0x10000000)
#define SPI_LEN1              (0x00000000)
#define SPI_CS_INIT           (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 |  SPI_LEN1 | SFCSR_SPI_RDY)

#define SPI_MAX_TRANSFER_SIZE (256)

#define FLASHBASE             (CFG_FLASH_BASE)

#ifndef CFG_MAX_FLASH_BANKS
  #define CFG_MAX_FLASH_BANKS (2)
#endif

#define MAX_SPI_FLASH_CHIPS   (CFG_MAX_FLASH_BANKS)

#ifndef CONFIG_SPI_WBO
  #define CONFIG_SPI_WBO      (1)
#endif

#ifndef CONFIG_WBO
  #define CONFIG_SPI_RBO      (1)
#endif

#ifdef __UBOOT__
  #define SPI_SHOW_PROGRESS   (1)
#else
  #undef FLASHBASE
  #define FLASHBASE           (FLASH_BASE)
  #define FLASH_UNKNOWN       (0xFFFF) /* unknown flash type */

  typedef struct RTK_SPI_MTD_s{
	uint32 BlockBase; /* the start address of this block, ex: 0xbfc00000, 0xbfe00000 ... */
	uint32 flags;     /*Chip Select*/
	ulong flash_id;
	struct semaphore lock;
	struct mtd_info mtd;
  }RTK_SPI_MTD;

#endif /*__UBOOT__*/

typedef enum{
/*	RBO=0x01, 
	WBO=0x02, */
	R_MODE=0x04, 
	CS0=0x10, /*CS: bit4 ~ bit7*/
	CS1=0x20,
	CS2=0x40,
	CS3=0x80
}spi_flags;

typedef enum{
	SPI_C_READ = 0x03,  /* Read */
	SPI_C_FREAD = 0x0B, /* Fast Read */
	SPI_C_RDID = 0x9F,  /* Read ID */
	SPI_C_WREN = 0x06,  /* Write Enable */
	SPI_C_WRDI = 0x04,  /* Write Disable */
	SPI_C_BE = 0xD8,    /* Block Erase */
	SPI_C_CE = 0xC7,    /* Chip Erase */
	SPI_C_SE = 0x20,    /* Sector Erase */
	SPI_C_PP = 0x02,    /* Page Program */
	SPI_C_RDSR = 0x05,  /* Read Status Register */
	SPI_C_WRSR = 0x01,  /* Write Status Register */
	SPI_C_DP = 0xB9,    /* Deep Power Down */
	SPI_C_RES = 0xAB,   /* Read Electronic ID */
	SPI_C_AAI = 0xAD    /* Auto Address Increment*/
}spi_cmdType_t;

#if 0
typedef struct rtk_spi_data{
	const char* Type;
	uint32 flags;       /* Chip Select, spi_flags*/
	uint32 ChipSize;    /* total size of this flash chip, ex: 0x100000 for 1MB ... */
	uint32 BlockBase;   /* the start address of this block, ex: 0xbfc00000, 0xbfe00000 ... */
	uint32 NumOfBlock;  /* number of blocks */
}spi_dev_t;
#endif

typedef struct spi_request_s{
	spi_cmdType_t cmd_t;
	uint32 address;        
	uint8   *buf;       /*request buffer*/
	uint32 size;        /*request IO size*/
}spi_request_t;

typedef struct spi_chip_info_s{
	uint32 chip_id;
	uint32 chip_size;
	uint8 *chip_name;
}spi_chip_info_t;


/*
 * Data Declaration
 */
/*
 * Macro Definition
 */

#define SPI_SETLEN(val) do {        \
	SPI_REG(SFCSR) &= 0xCFFFFFFF;   \
	SPI_REG(SFCSR) |= (val-1)<<28;  \
	}while(0)


#define CHECK_READY() do{while( !(SPI_REG(SFCSR)&SFCSR_SPI_RDY) );}while(0)
/*
 * Function Declaration
 */

#endif /*_FLASH_SPI_H*/

