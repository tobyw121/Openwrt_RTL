//+FHDR-------------------------------------------------------------------------
// Copyright (c) 2006, Realtek Semiconductor Corporation
// Realtek's Proprietary/Confidential
//
// All rights reserved. No part of this design may be reproduced or stored
// in a retrieval system, or transmitted, in any form or by any means,
// electronic, mechanical, photocopying, recording, or otherwise,
// without prior written permission of the Realtek Semiconductor Corporation.
// Unauthorized reproduction, duplication, use, or disclosure of this
// design will be deemed as infringement.
//------------------------------------------------------------------------------
// Description:
//     Sheipa platform supervisor subsystem initialize code.
//     database.
//-FHDR-------------------------------------------------------------------------

//#include <stdio.h>
//#include "config.h"
//#include "runtime.h"
//#include "platform_defs.h"
//#include "ps_smu_defs.h"
//#include "ps_pmu_defs.h"
//#include <common.h>

//Add for RXI-310 Memory_Controller
//#include <malloc.h>

#include "dram_deliver/ms_rxi310_defs.h"
#include "dram_deliver/dram_param_defs.h"
#include "dram_deliver/dram_device_info.h"

#include <rlxboard.h>
#include <sys_reg.h>
void dram_init_rxi310_ASIC_97G (struct ms_rxi310_portmap *dev_map, const struct dram_device_info *dram_info);
    #define set_rom_progress(...)  
    #define REG_SWR_DDR_0                  (SYSTEM_REGISTER_BASE + 0x280)
    #undef DDR_CALIBRATION

#ifdef CONFIG_SD_CARD_BOOTING
#include <sdcard/sdcard.h>
#include <fs/ff.h>
#define HZ 100
#define printf      dprintf
#ifdef RTL8198
unsigned long loops_per_sec = 2490368 * HZ;	// @CPU 500MHz (this will be update in check_cpu_speed())
#else
unsigned long loops_per_sec = 0x1db000 * HZ;	// @CPU 390MHz, DDR 195 MHz (this will be update in check_cpu_speed())
#endif

#elif defined(CONFIG_IRAM_IMAGE)
#define udelay hal_delay_us
#define printf

#else
//#define udelay hal_delay_us bootcode use
#define printf dprintf//     btprintf bootcode use
#endif
#undef udelay
#undef printf
//#define udelay hal_delay_us bootcode use
    #define udelay hal_delay_us
    //#define printf dprintf//     btprintf bootcode use
    #define printf btprintf//     btprintf bootcode use
//#include "SPIC_deliver/spi_flash/spi_flash_rtk.h"
//#include "spi_flash/DW_common.h"
//#include "spi_flash/spi_flash_public.h"
//#include "spi_flash/spi_flash_private.h"

//#include "rtk_phy_defs.h"

//#define CPU_800
//#define DIRECT_JUMP

#define DRAM_FEQ 1
#ifndef SDRAM_DDR1_SUPPORT
#define ENABLE_DRAM_ODT
#endif

#define CS_2T 0
#define STATIC_CKE 0
#define STATIC_ODT 0
#define tRTW_2T_DIS 0
#define OFFSET_TPIN_CKE 1
#define OFFSET_TPIN_ODT 2
#define CKE_HIGH_ONLY (0x1<<OFFSET_TPIN_CKE)
#define CKE_RST_HIGH ((0x1<<OFFSET_TPIN_CKE) | (0x1<<0))
#define CKE_LOWRST_HIGH ((0x0<<OFFSET_TPIN_CKE) | (0x1<<0))


#define OFFSET_DPIN_ADDR_A0 0
#define OFFSET_DPIN_ADDR_A10 10
#define OFFSET_DPIN_ADDR_A12 12


#define OFFSET_DPIN_BA 17
#define OFFSET_DPIN_WE 21
#define OFFSET_DPIN_CAS 22
#define OFFSET_DPIN_RAS 23
#define OFFSET_DPIN_CS 24
//#define OFFSET_DPIN_ACT 25 //DDR4 only
//#define OFFSET_DPIN_PARITY 26 //DDR4 only

#define CMD_DPIN_PRECHARGE_ALL ((0x1<<OFFSET_DPIN_CAS) | (0x1<<OFFSET_DPIN_ADDR_A10))
#define CMD_DPIN_MR (0x0<<OFFSET_DPIN_BA)
#define CMD_DPIN_MR_ONE_BANK (0x0<<OFFSET_DPIN_ADDR_A12)

#define CMD_DPIN_EMR1 (0x1<<OFFSET_DPIN_BA)
#define CMD_DPIN_EMR1_ONE_BANK (0x1<<OFFSET_DPIN_ADDR_A12)

#define CMD_DPIN_EMR2 (0x2<<OFFSET_DPIN_BA)
#define CMD_DPIN_EMR2_ONE_BANK (0x2<<OFFSET_DPIN_ADDR_A12)

#define CMD_DPIN_EMR3 (0x3<<OFFSET_DPIN_BA)
#define CMD_DPIN_EMR3_ONE_BANK (0x3<<OFFSET_DPIN_ADDR_A12)

#define CMD_DPIN_AUTO_REFRESH (0x1<<OFFSET_DPIN_WE)
#define CMD_DPIN_NOP ((0x1<<OFFSET_DPIN_RAS) | (0x1<<OFFSET_DPIN_CAS) | (0x1<<OFFSET_DPIN_WE))
#define CMD_DPIN_ALL_HIGH ((0x1<<OFFSET_DPIN_CS)) | ((0x1<<OFFSET_DPIN_RAS) | (0x1<<OFFSET_DPIN_CAS) | (0x1<<OFFSET_DPIN_WE))
#define _memctl_debug_printf printf
//#define _memctl_debug_printf(...)

/* Register Macro */
#ifndef REG32
#define REG32(reg)      (*(volatile unsigned int   *)(reg))
//#define REG32(reg)      (*(volatile u32 *)(reg))
#endif
#ifndef REG16
#define REG16(reg)      (*(volatile u16 *)(reg))
#endif
#ifndef REG8
#define REG8(reg)       (*(volatile u8  *)(reg))
#endif

#define REG_READ_U8(register)         		(*(volatile unsigned char *)(register))
#define REG_READ_U16(register)         		(*(volatile unsigned short *)(register))

#ifndef REG_READ_U32
#define REG_READ_U32(register)         		(*(volatile unsigned long *)(register))
#endif

#ifndef REG_WRITE_U32
#define REG_WRITE_U32(register, value)    		(*(volatile unsigned long *)(register) = value)
#endif

#define TURN_ON_CLOCK() \
	REG_WRITE_U32(REG_DDR_PLL_1, REG_READ_U32(REG_DDR_PLL_1) | 0xf); \
	REG_WRITE_U32(REG_DDR_PLL_2, REG_READ_U32(REG_DDR_PLL_2) | (0xf<<16)); 

#define TURN_OFF_CLOCK() \
	REG_WRITE_U32(REG_DDR_PLL_1, REG_READ_U32(REG_DDR_PLL_1) & 0xfffffff2); \
	REG_WRITE_U32(REG_DDR_PLL_2, REG_READ_U32(REG_DDR_PLL_2) & (~(0xD<<16)));	

#define TRUNCATED(a, n) ((a < (1<<n))?a:((1<<n)-1))

#define     BIT_SHIFT_CHIP_ID_CODE              0
#define     BIT_MASK_CHIP_ID_CODE               0xf
#define     IS_FB_BONDING(x)                    (x <= 3)
#define     IS_MCM128_BONDING(x)                    ((x == 0xB) || (x == 0x5))
#define     BIT_GET_CHIP_ID_CODE(x)             (((x) >> BIT_SHIFT_CHIP_ID_CODE) & BIT_MASK_CHIP_ID_CODE)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) < (b) ? (b) : (a))

void dram_init_rxi310_FPGA (struct ms_rxi310_portmap *, struct dram_device_info *);
void dram_init_pll_ASIC (struct ms_rxi310_portmap *,  const struct dram_device_info *, unsigned int *pstrap_pin);
void dram_init_dpi_ip_ASIC (struct ms_rxi310_portmap *,  const struct dram_device_info *);
void dram_init_rxi310_ASIC (struct ms_rxi310_portmap *dev_map,const struct dram_device_info *dram_info);

typedef unsigned int uint32;
#if 0
__attribute__((long_call)) extern int dram_normal_patterns(uint32 dram_start, uint32 dram_size, uint32 area_size);
__attribute__((long_call)) extern int dram_addr_rot(uint32 dram_start, uint32 dram_size, uint32 area_size);
__attribute__((long_call)) extern int dram_com_addr_rot(uint32 dram_start, uint32 dram_size, uint32 area_size);
__attribute__((long_call)) extern int dram_byte_access(uint32 dram_start, uint32 dram_size, uint32 area_size);
__attribute__((long_call)) extern int dram_half_word_access(uint32 dram_start, uint32 dram_size,uint32 area_size);
__attribute__((long_call)) extern int dram_walking_of_1(uint32 dram_start, uint32 dram_size, uint32 area_size);
__attribute__((long_call)) extern int dram_walking_of_0(uint32 dram_start, uint32 dram_size, uint32 area_size);
__attribute__((long_call)) extern int memcpy_test(uint32 dram_start, uint32 dram_size, uint32 area_size);
__attribute__((long_call)) extern void udelay(unsigned long usec);
__attribute__((long_call)) extern int dprintf(char *fmt, ...);
__attribute__((long_call)) extern void * memcpy(void * dest,const void *src,uint32 count);
__attribute__((long_call)) extern void * memset(void *,int,uint32);
#endif

#define UART0_FREQ 100000000
#define UART_LCR_DLS		0x00000003	/* 8 bits mode       */
#define UART_LCR_DLAB		0x00000080	/* DLAB bit          */
#define UART_IOBASE 0xb8147000
#define UART_RBR		0x00
#define UART_THR		0x00
#define UART_IER		0x04
#define UART_IIR		0x08
#define UART_LCR		0x0C
#define UART_LSR		0x14
#define UART_DLL		0x00
#define UART_DLH		0x04
#if 0 // FPGA
const int baud_r_table[8][4] = 
{
        {0x32, 0x0, 0xf, 0x1ff}, // 9600
        {0x19, 0x0, 0xf, 0x1ff}, // 19200
        {0xd,   0x0, 0xf, 0x0}, // 38400
        {0x9,   0x0, 0xe, 0x7}, // 57600
        {0x5,   0x0, 0xc, 0x7},  // 115200
        {0x3,   0x0, 0x9, 0x1f},  // 230400
        {0x2,   0x0, 0x5, 0x1ff},  // 460800
        {0x1,   0x0, 0x5, 0x1ff}  // 921600
};
#else
const int baud_r_table[8][4] = 
{
        {0x32, 0x0, 0xf, 0x1ff}, // 9600
        {0xf9, 0x0, 0xf, 0x3ff}, // 19200
        {0x7d, 0x0, 0xf, 0x1ff}, // 38400
        {0x53, 0x0, 0xf, 0x3ff}, // 57600
        {0x2a, 0x0, 0xf, 0x7f},  // 115200
        {0x15, 0x0, 0xf, 0x7f},  // 230400
        {0xb,   0x0, 0xe, 0xff},  // 460800
        {0x6,   0x0, 0xd, 0x0}  // 921600
};
#endif

unsigned int dwc_serial_init (void)
{
    //ENABLE_UART;

               unsigned int divisor, ovsr, ovsr_adj, baud_rate_index;
               baud_rate_index = 0;//pefuse_data->uart_baud_rate;

               switch(baud_rate_index)
               {
                        case 0:  // 38400 default
                                divisor = baud_r_table[2][0] | (baud_r_table[2][1] << 8);
                                ovsr = baud_r_table[2][2];
                                ovsr_adj = baud_r_table[2][3];
                                break;
                        case 1:  // 19200
                                divisor = baud_r_table[1][0] | (baud_r_table[1][1] << 8);
                                ovsr = baud_r_table[1][2];
                                ovsr_adj = baud_r_table[1][3];
                                break;
                        case 2:  // 38400
                                divisor = baud_r_table[2][0] | (baud_r_table[2][1] << 8);
                                ovsr = baud_r_table[2][2];
                                ovsr_adj = baud_r_table[2][3];
                                break;
                        case 3:  // 57600
                                divisor = baud_r_table[3][0] | (baud_r_table[3][1] << 8);
                                ovsr = baud_r_table[3][2];
                                ovsr_adj = baud_r_table[3][3];
                                break;
                        case 4:  // 115200
                                divisor = baud_r_table[4][0] | (baud_r_table[4][1] << 8);
                                ovsr = baud_r_table[4][2];
                                ovsr_adj = baud_r_table[4][3];
                                break;
                        case 5:  // 230400
                                divisor = baud_r_table[5][0] | (baud_r_table[5][1] << 8);
                                ovsr = baud_r_table[5][2];
                                ovsr_adj = baud_r_table[5][3];
                                break;
                        case 6:  // 460800
                                divisor = baud_r_table[6][0] | (baud_r_table[6][1] << 8);
                                ovsr = baud_r_table[6][2];
                                ovsr_adj = baud_r_table[6][3];
                                break;
                        case 7:  // 921600
                                divisor = baud_r_table[7][0] | (baud_r_table[7][1] << 8);
                                ovsr = baud_r_table[7][2];
                                ovsr_adj = baud_r_table[7][3];
                                break;
                        default: // 38400
                                divisor = baud_r_table[2][0] | (baud_r_table[2][1] << 8);
                                ovsr = baud_r_table[2][2];
                                ovsr_adj = baud_r_table[2][3];
                                break;

               }
	REG32(UART_IOBASE+UART_IER) = 0;

	REG32(UART_IOBASE+UART_LCR) = UART_LCR_DLAB;
	REG32(UART_IOBASE+UART_DLL) = divisor & 0xff;//UART_BAUD_RATEL;
	REG32(UART_IOBASE+UART_DLH) = (divisor & 0xff00) >> 8;//UART_BAUD_RATEH;

               REG32(UART_IOBASE+0x20) = ovsr<<4;
               
               REG32(UART_IOBASE+0x1c) = (REG32(UART_IOBASE+0x1c) & 0xF800FFFF) | (ovsr_adj<<16);

	REG32(UART_IOBASE+UART_LCR) = UART_LCR_DLS;
	return 0;
}

#define EFUSE_BASE_ADDR                     0xB8000700
#define     REG_EFUSE_BASE_ADDR                 EFUSE_BASE_ADDR  // 0x18000700
#define     REG_EFUSE_P0_0                      (REG_EFUSE_BASE_ADDR + 0x00000080)
#define OFFSE_EFUSE_PHYSICAL_OPTIONS_0x78           0x78
#define BIT_SHIFT_EFUSE_DRAM_BIT_SHIFT                   4
#define BIT_MASK_EFUSE_DRAM_BIT_SHIFT                    0xF
#define BIT_GET_EFUSE_DRAM_BIT_SHIFT(x)                  (((x) >> BIT_SHIFT_EFUSE_DRAM_BIT_SHIFT) & BIT_MASK_EFUSE_DRAM_BIT_SHIFT)

#define OFFSE_EFUSE_PHYSICAL_OPTIONS_0x7C           0x7c
#define OFFSE_EFUSE_PHYSICAL_OPTIONS_0x71           0x71

uint32_t read_efuse_byte_iram(uint32_t idx)
{
    // efuse controller only support read 4 bytes operation. 
    uint32_t addr  = (REG_EFUSE_P0_0 + idx) & (~0x3);
    uint32_t value = REG32(addr);
    uint8_t retval = 0;

    switch(idx%4) {
        case 0:
            retval = value & 0xFF;
            break;
        case 1:
            retval = (value >> 8) & 0xFF;
            break;
        case 2:
            retval = (value >> 16) & 0xFF;
            break;
        case 3:
            retval = (value >> 24) & 0xFF;
            break;
        //default:
        //    ;
    }

    return retval;
}

#ifdef CONFIG_SD_CARD_BOOTING
u1Byte load_efuse_data_to_reg(void)
{
    u4Byte cnt = 0;
    u4Byte cnt_max = 5000;

#ifdef NO_CONFIG_RTL8197F
    // patch default REG_EFUSE_TIMING_CTRL value to 0x01040A4F for RTL 8197F
    REG32(REG_EFUSE_TIMING_CTRL) = 0x01040A4F;
#endif

    REG32(REG_EFUSE_CONFIG) = REG32(REG_EFUSE_CONFIG) & (~BIT_EFUSE_CONFIG_PWR_GATE_EN);
    REG32(REG_EFUSE_CMD) = REG32(REG_EFUSE_CMD) & (~BIT_EFUSE_RW_CTRL);

    do {
        if ((REG32(REG_EFUSE_CMD) & BIT_EFUSE_CTRL_STATE) == BIT_EFUSE_CTRL_STATE) {
            break;
        }
        cnt++;
        udelay(1);
    } while(cnt < cnt_max);

    if (cnt < cnt_max) {
        DBG_MSG(COMP_EFUSE, DBG_MESSAGE, ("load efuse ok\n"));
        //set_rom_progress(ROM_PROGRESS_LOAD_EFUSE_TO_REG_OK);
        return STATUS_EFUSE_SUCCESS;
    } else {
        DBG_MSG(COMP_EFUSE, DBG_MESSAGE, ("load efuse fail. 0x%x(0x%x) \n", REG_EFUSE_CMD, REG32(REG_EFUSE_CMD)));
        //set_rom_progress(ROM_PROGRESS_LOAD_EFUSE_TO_REG_FAIL);
        return STATUS_EFUSE_FAIL;
    }
}
#else
extern unsigned char load_efuse_data_to_reg(void);
#endif

#ifdef NO_CONFIG_RTL8197F
const static unsigned short rl6387_pll_frequence_table[7][13]=
{  //{2C8[5:4], 2C8[8], 2C8[18:16], 2CC[5:4], 2CC[9:8], 2CC[1], 2C8[29:28], 2C8[1:0], 2C8[25:24], 2CC[0], 2C8[15:12], 2C8[23], 2C8[22:20]}
	{3          ,1         ,5               ,2            ,2           ,1         ,2               ,2            ,0               ,1        ,11             ,1           ,2},  //1440 MHz
	{3          ,1         ,4               ,2            ,2           ,1         ,2               ,2            ,0               ,1        ,11             ,1           ,2},  //1280MHz
	{3          ,0         ,4               ,2            ,1           ,0         ,2               ,2            ,0               ,1        ,9               ,1           ,2},  //1120MHz
	{3          ,0         ,3               ,2            ,1           ,0         ,2               ,2            ,0               ,1        ,7               ,1           ,2},  //960MHz
	{3          ,0         ,2               ,0            ,0           ,1         ,2               ,2            ,0               ,1        ,7               ,1           ,2},  //800MHz
	{2          ,0         ,2               ,0            ,0           ,1         ,2               ,2            ,0               ,1        ,5               ,1           ,2},  //640MHz
	{2          ,0         ,1               ,0            ,0           ,0         ,2               ,2            ,0               ,1        ,5               ,1           ,2}   //320MHz
};
#else  // NO_CONFIG_RTL8197F
const static unsigned short rl6387_pll_frequence_table[7][13]=
{  //{2C8[5:4], 2C8[8], 2C8[18:16], 2CC[5:4], 2CC[9:8], 2CC[1], 2C8[29:28], 2C8[1:0], 2C8[25:24], 2CC[0], 2C8[15:12], 2C8[23], 2C8[22:20]}
	{3          ,1         ,5               ,2            ,2           ,1         ,2               ,2            ,0               ,1        ,11             ,1           ,2},  //1440 MHz
	{3          ,1         ,4               ,2            ,2           ,1         ,2               ,2            ,0               ,1        ,11             ,1           ,2},  //1280MHz
	{3          ,0         ,3               ,2            ,1           ,0         ,2               ,2            ,0               ,1        ,9               ,1           ,2},  //1095MHz
	{3          ,0         ,3               ,2            ,1           ,0         ,2               ,2            ,0               ,1        ,7               ,1           ,2},  //960MHz
	{3          ,0         ,2               ,0            ,0           ,1         ,2               ,2            ,0               ,1        ,7               ,1           ,2},  //800MHz
	{2          ,0         ,2               ,0            ,0           ,1         ,2               ,2            ,0               ,1        ,5               ,1           ,2},  //640MHz
	{2          ,0         ,1               ,0            ,0           ,0         ,2               ,2            ,0               ,1        ,5               ,1           ,2}   //320MHz
};
#endif // NO_CONFIG_RTL8197F

#ifdef CPU_800
void Setting_CPU_Speed()
{
	#define SYS_STRAP_REG 0xb8000008
	REG32(SYS_STRAP_REG)&= ~(3<<19);   //wei add, bit [20:19]=0   //setting 800MHz
}
#endif
#ifdef DDR_DUMP_ALL_REGISTER
void dram_dump_all_register(void)
{
        unsigned int addr_start, addr_end, i;
        
        // crt phy register
        addr_start = DPI_DLL_BASE;
        addr_end = DPI_DLL_BASE + 0x3d0;
        printf("DDR PHY register =================\n");
        for(i = addr_start; i <= addr_end; i+=4)
        {       
                printf("0x%x = 0x%08x \n", i, (unsigned int)REG_READ_U32(i));
        }

        // system register
        addr_start = SYSTEM_REGISTER_BASE;
        addr_end = SYSTEM_REGISTER_BASE + 0x2ec;
        printf("SYSTEM register =================\n");
        for(i = addr_start; i <= addr_end; i+=4)
        {       
                printf("0x%x = 0x%08x \n", i, (unsigned int)REG_READ_U32(i));
        }

        // pctl register
        addr_start = BSP_MS_I_DRAMC_0_BASE;
        addr_end = BSP_MS_I_DRAMC_0_BASE + 0x2a4;
        printf("PCTL register ==================\n");
        for(i = addr_start; i <= addr_end; i+=4)
        {       
                printf("0x%x = 0x%08x \n", i, (unsigned int)REG_READ_U32(i));
        }

        return;
}
#endif

void dram_auto_size_detect_rxi310(struct ms_rxi310_portmap *dev_map,
		const struct dram_device_info *dram_info)
{
        unsigned int page_size_local, bank_size, cr_bst_len, real_page_size, real_dram_size;
        unsigned int max_page = 0x7, min_bank = 0x0, max_bank = 0x3;
        volatile unsigned int * dram_addr = (unsigned int *)CONFIG_SYS_SDRAM_BASE_UNCACHE;
        unsigned int trfc = DRAM_512M_TRFC, tfaw = DRAM_TFAW;
        uint32_t dfi_rate, dram_period;
#ifdef CONFIG_RTL8197G
        uint8_t temp_val = read_efuse_byte_iram(OFFSE_EFUSE_PHYSICAL_OPTIONS_0x78);
        uint8_t one_bank_col_bit = BIT_GET_EFUSE_DRAM_BIT_SHIFT(temp_val);
#endif

        dfi_rate = 1 << (uint32_t) (dram_info->dfi_rate);
        dram_period = (dram_info-> ddr_period_ps)*(dfi_rate); // according DFI_RATE to setting

        //unsigned int dram_data;
        //_memctl_debug_printf("Enter %s, page_size = %d, bank_size = %d \n", 
        //__FUNCTION__, dram_info->dev->page, dram_info->dev->bank);
#ifdef CONFIG_RTL8197G
        if(one_bank_col_bit == 0x0)
#endif
        {
                if(dram_info->dev->device_type == DDR_2)
                {
                        max_page = 0x5; // DDR2 max page size is 2KB, we set 8KB for software detect
                        bank_size = 0x2; 
                        max_bank = 0x3;
                        page_size_local = 0x3;
                        real_page_size = 0x800;
                }
                else if(dram_info->dev->device_type == DDR_3)
                {
                        max_page = 0x5; // 
                        bank_size = 0x2; 
                        max_bank = 0x3;
                        page_size_local = 0x3;
                        real_page_size = 0x800;
                }
                else 
                {
                        max_page = 0x5; // DDR1 max page size is 2KB, we set 8KB for software detect
                        bank_size = 0x1; //only 4 bank in DDR 1
                        max_bank = 0x1;
                        page_size_local = 0x3;
                        real_page_size = 0x800;
                }

                
                if (dram_info-> mode_reg-> bst_len == BST_LEN_4) {
                        cr_bst_len = 0; //bst_4
                        //_memctl_debug_printf("\nDDR2  dram_info-> mode_reg-> bst_len=%d\n",dram_info-> mode_reg-> bst_len);
                }
                else { // BST_LEN_8
                        cr_bst_len = 1; // bst_8
                        //_memctl_debug_printf("\nDDR2  dram_info-> mode_reg-> bst_len=%d\n",dram_info-> mode_reg-> bst_len);
                }

                //step 1, set largest page_size & smallest bank_size to check correct page_size
                dev_map-> misc = (
                                                (max_page  << WRAP_MISC_PAGE_SIZE_BFO) |  
                                                (min_bank << WRAP_MISC_BANK_SIZE_BFO) |
                                                (cr_bst_len  << WRAP_MISC_BST_SIZE_BFO )  
                                           );
                //write a random value to 0x80000000
                *dram_addr = 0xfe5566ef; // no meaning of fe5566ef, just random, 0x100 /4 because dram_addr is unsigned int, +1 will add 4 byte address
                if((*(dram_addr + (0x100/4))) == 0xfe5566ef)  //page size is 256B, because multiple mapping on 0x80000000 & 0x80000100
                {       
                        page_size_local = 0x0; 
                        real_page_size = 0x100;
                        //tfaw = DRAM_1066_2K_TFAW; //set default         
                        _memctl_debug_printf("\nDetect page_size = 256B (%d)\n",page_size_local);
                }
                else if((*(dram_addr + (0x200/4))) == 0xfe5566ef)  //page size is 512B
                {       
                        page_size_local = 0x1; 
                        real_page_size = 0x200;
                        //tfaw = DRAM_1066_2K_TFAW; //set default
                        _memctl_debug_printf("\nDetect page_size = 512B (%d)\n",page_size_local);
                }
                else if((*(dram_addr + (0x400/4))) == 0xfe5566ef)  //page size is 1KB
                {       
                        page_size_local = 0x2; 
                        real_page_size = 0x400;
                        //tfaw = DRAM_1066_1K_TFAW; //set default
                        _memctl_debug_printf("\nDetect page_size = 1KB (%d)\n",page_size_local);
                }
                else if((*(dram_addr + (0x800/4))) == 0xfe5566ef)  //page size is 2KB
                {       
                        page_size_local = 0x3; 
                        real_page_size = 0x800; 
                        //tfaw = DRAM_1066_2K_TFAW; //set default
                        _memctl_debug_printf("\nDetect page_size = 2KB (%d)\n",page_size_local);
                }
                else if((*(dram_addr + (0x1000/4))) == 0xfe5566ef)     //page size is 4KB
                {       
                        page_size_local = 0x4; 
                        real_page_size = 0x1000;
                        //tfaw = DRAM_1066_2K_TFAW; //set default
                        _memctl_debug_printf("\nDetect page_size = 4KB (%d)\n",page_size_local);
                }
                else if((*(dram_addr + (0x2000/4))) == 0xfe5566ef)     //page size is 8KB
                {       
                        page_size_local = 0x5; 
                        real_page_size = 0x2000;
                        //tfaw = DRAM_1066_2K_TFAW; //set default
                        _memctl_debug_printf("\nDetect page_size = 8KB (%d)\n",page_size_local);
                }
                else if((*(dram_addr + (0x4000/4))) == 0xfe5566ef)     //page size is 16KB
                {       
                        page_size_local = 0x6; 
                        real_page_size = 0x4000;
                        //tfaw = DRAM_1066_2K_TFAW; //set default
                        _memctl_debug_printf("\nDetect page_size = 16KB (%d)\n",page_size_local);
                }
                else if((*(dram_addr + (0x8000/4))) == 0xfe5566ef)     //page size is 32KB
                {       
                        page_size_local = 0x7; 
                        real_page_size = 0x8000;
                        //tfaw = DRAM_1066_2K_TFAW; //set default
                        _memctl_debug_printf("\nDetect page_size = 32KB (%d)\n",page_size_local);
                }
                else
                {
                        _memctl_debug_printf("\nDetect page_size = %d, not detected\n",page_size_local);
                }

                if((dram_info->dev->device_type != DDR_1))
                {
                        //step2, set bank size to max bank size & page size to real page size, detect bank
                        dev_map-> misc = (
                                                        (page_size_local  << WRAP_MISC_PAGE_SIZE_BFO) |  
                                                        (max_bank << WRAP_MISC_BANK_SIZE_BFO) |
                                                        (cr_bst_len  << WRAP_MISC_BST_SIZE_BFO )  
                                                   );        
                        //write a random to mem
                        *dram_addr = 0x77feef88; // no meaning of 77feef88, just random
                        if((*(dram_addr + 2*real_page_size/4)) == 0x77feef88)
                        {
                                bank_size = 0x0;
                                _memctl_debug_printf("\nDetect bank_size = 2 banks(0x%x)\n",bank_size);
                        }
                        else if((*(dram_addr + 4*real_page_size/4)) == 0x77feef88)
                        {
                                bank_size = 0x1;
                                _memctl_debug_printf("\nDetect bank_size = 4 banks(0x%x)\n",bank_size);
                        }
                        else if((*(dram_addr + 8*real_page_size/4)) == 0x77feef88)
                        {
                                bank_size = 0x2;
                                _memctl_debug_printf("\nDetect bank_size = 8 banks(0x%x)\n",bank_size);
                        }
                        else
                        {// if not all of above , set default by DDR type
                                _memctl_debug_printf("\nDetect bank_size = %d , not detected\n",bank_size);
                        }
                }
                // set final page size & bank size 
                dev_map-> misc = (
                                                (page_size_local     << WRAP_MISC_PAGE_SIZE_BFO) |  
                                                (bank_size << WRAP_MISC_BANK_SIZE_BFO) |
                                                (cr_bst_len  << WRAP_MISC_BST_SIZE_BFO )  
                                           );        
        }

        //set tFAW
        //dev_map->tpr1 = ((dev_map->tpr1 & 0xE0FFFFFF) | ((TRUNCATED((tfaw/dram_period + 1),5)) << 24));

        //detect dram size
        *dram_addr = 0x95318CA9;
        if((*(dram_addr + (0x800000/4))) == 0x95318CA9)
        {
                        real_dram_size = 0x800000;
                        trfc = DRAM_32M_TRFC;
                        _memctl_debug_printf("\nDetect dram size = 8MB (0x%x)\n",real_dram_size);
        }
        else if((*(dram_addr + (0x1000000/4))) == 0x95318CA9)
        {                       
                        real_dram_size = 0x1000000;
                        trfc = DRAM_32M_TRFC;
                        _memctl_debug_printf("\nDetect dram size = 16MB (0x%x)\n",real_dram_size);
        }
        else if((*(dram_addr + (0x2000000/4))) == 0x95318CA9)
        {
                        real_dram_size = 0x2000000;
                        trfc = DRAM_32M_TRFC;
                        _memctl_debug_printf("\nDetect dram size = 32MB (0x%x)\n",real_dram_size);
        }       
        else if((*(dram_addr + (0x4000000/4))) == 0x95318CA9)
        {
                        real_dram_size = 0x4000000;
                        trfc = DRAM_64M_TRFC;
                        _memctl_debug_printf("\nDetect dram size = 64MB (0x%x)\n",real_dram_size);
        }
        else if((*(dram_addr + (0x8000000/4))) == 0x95318CA9)
        {
                        real_dram_size = 0x8000000;
                        trfc = DRAM_128M_TRFC;
                        _memctl_debug_printf("\nDetect dram size = 128MB (0x%x)\n",real_dram_size);
        }
        else
        {
                        real_dram_size = 0x10000000;
                        trfc = DRAM_256M_TRFC;
                        _memctl_debug_printf("\nDetect dram size >= 256MB \n");
        }

        //set tRFC for different dram size
          dev_map-> drr = (
                                   (0                                                                                                << PCTL_DRR_REF_DIS_BFO) |
                                   (9                                                                                                 << PCTL_DRR_REF_NUM_BFO) |
                                   ((((dram_info-> timing -> trefi_ps)/dram_period)+1)<< PCTL_DRR_TREF_BFO    ) |
                                   ((((trfc)/dram_period)+1) << PCTL_DRR_TRFC_BFO       )
                                  );          

        return;

}

void dram_calibration_turn_on_odt(const struct dram_device_info *dram_info)
{
        unsigned char bond = BIT_GET_CHIP_ID_CODE(REG32(REG_BOND_OPTION));
        _memctl_calibration_printf("\ndram_init.c dram_K_turn_on_odt\n");
        //REG_WRITE_U32(READ_CTRL_2, (REG_READ_U32(READ_CTRL_2) & 0xffcfffff) | 0x00300000); 
        REG_WRITE_U32(READ_CTRL_2, (REG_READ_U32(READ_CTRL_2) & 0xffcf8000) | dram_info->phy->odt_default); 
#ifdef DDR_K_DQS_EN
        REG_WRITE_U32(PAD_DQS, (REG_READ_U32(PAD_DQS) & 0xfff0fff0) | 0x00090006); 
        REG_WRITE_U32(PAD_DQS_2, (REG_READ_U32(PAD_DQS_2) & 0xfff0fff0) | 0x00090006); 
#else
        REG_WRITE_U32(PAD_DQS, (REG_READ_U32(PAD_DQS) & 0xfff0fff0) | 0x00060009); 
        REG_WRITE_U32(PAD_DQS_2, (REG_READ_U32(PAD_DQS_2) & 0xfff0fff0) | 0x00060009); 
#endif
        //set DQ ODT 79 ohm
        //REG_WRITE_U32(PAD_DQ, (REG_READ_U32(PAD_DQ) & 0xFFFFFFF0) | 0xa);             // DQ 0 ,default 1, set to 2
        //REG_WRITE_U32(PAD_DQ_1, (REG_READ_U32(PAD_DQ_1) & 0xFFFFFFF0) | 0xa);             // DQ 8, default 1, set to 2

#if 1//def DRAM_ADD_OUTPUT_DRIVING
        REG_WRITE_U32(PAD_DCK, (REG_READ_U32(PAD_DCK) & 0xFFFFFF00) | (dram_info->phy->dck_driving));  //ck default 0, set to 1
        REG_WRITE_U32(PAD_CMD, (REG_READ_U32(PAD_CMD) & 0xFFFFFFF0) | (dram_info->phy->cmd_driving));  // CMD default 0, set to 1
        REG_WRITE_U32(PAD_BK_ADR, (REG_READ_U32(PAD_BK_ADR) & 0xFFFFFFF0) | (dram_info->phy->ba_addr_driving)); //bank addr, default 0, set to 1
        REG_WRITE_U32(PAD_DQ, (REG_READ_U32(PAD_DQ) & 0xFFFFF00F) | (dram_info->phy->dq_driving<<4));             // DQ 0 ,default 1, set to 2
        REG_WRITE_U32(PAD_DQ_1, (REG_READ_U32(PAD_DQ_1) & 0xFFFFF00F) | (dram_info->phy->dq_driving<<4));             // DQ 8, default 1, set to 2
        REG_WRITE_U32(PAD_DQS, (REG_READ_U32(PAD_DQS) & 0xFFFF000F) | (dram_info->phy->dqs_driving << 4));
        REG_WRITE_U32(PAD_DQS_2, (REG_READ_U32(PAD_DQS_2) & 0xFFFF000F) | (dram_info->phy->dqs_driving << 4));
#endif
        return;
}

void dram_set_wrlvl_fb(int mck_dqs)
{
	if(mck_dqs <= -58)
	{
		_memctl_calibration_printf("mck_ck = %d, %d\n", mck_dqs, __LINE__);
		REG_WRITE_U32(WRLVL_CTRL, (REG_READ_U32(WRLVL_CTRL) & 0xfffffffc) | 0x00000003); // wrlvl_fb = 1
		REG_WRITE_U32(BIST_2TO1_0, (REG_READ_U32(BIST_2TO1_0) & 0xfcffffff) | 0x00000000); // wrlv_delay = 0
	}
	else if((mck_dqs >= -57) && (mck_dqs <= -1))
	{
		_memctl_calibration_printf("mck_ck = %d, %d\n", mck_dqs, __LINE__);
		REG_WRITE_U32(WRLVL_CTRL, (REG_READ_U32(WRLVL_CTRL) & 0xfffffffc) | 0x00000000); // wrlvl_fb = 0
		REG_WRITE_U32(BIST_2TO1_0, (REG_READ_U32(BIST_2TO1_0) & 0xfcffffff) | 0x00000000); // wrlv_delay = 0

	}
	else if((mck_dqs >= 0) && (mck_dqs <= 26))
	{
		_memctl_calibration_printf("mck_ck = %d, %d\n", mck_dqs, __LINE__);
		REG_WRITE_U32(WRLVL_CTRL, (REG_READ_U32(WRLVL_CTRL) & 0xfffffffc) | 0x00000003); // wrlvl_fb = 1
		REG_WRITE_U32(BIST_2TO1_0, (REG_READ_U32(BIST_2TO1_0) & 0xfcffffff) | 0x00000000); // wrlv_delay = 0

	}
	else if((mck_dqs >= 27) && (mck_dqs <= 57))
	{
		_memctl_calibration_printf("mck_ck = %d, %d\n", mck_dqs, __LINE__);
		REG_WRITE_U32(WRLVL_CTRL, (REG_READ_U32(WRLVL_CTRL) & 0xfffffffc) | 0x00000000); // wrlvl_fb = 0
		REG_WRITE_U32(BIST_2TO1_0, (REG_READ_U32(BIST_2TO1_0) & 0xfcffffff) | 0x03000000); // wrlv_delay = 1

	}
	else if((mck_dqs >= 58) && (mck_dqs <= 63))
	{
		_memctl_calibration_printf("mck_ck = %d, %d\n", mck_dqs, __LINE__);
		REG_WRITE_U32(WRLVL_CTRL, (REG_READ_U32(WRLVL_CTRL) & 0xfffffffc) | 0x00000003); // wrlvl_fb = 1
		REG_WRITE_U32(BIST_2TO1_0, (REG_READ_U32(BIST_2TO1_0) & 0xfcffffff) | 0x03000000); // wrlv_delay = 1
	}

}

__attribute__((long_call)) extern void bl2_calltest1(void);
__attribute__((long_call)) extern void bl2_calltest2(void);
#define TURN_OFF_CLOCK() \
	REG_WRITE_U32(REG_DDR_PLL_1, REG_READ_U32(REG_DDR_PLL_1) & 0xfffffff2); \
	REG_WRITE_U32(REG_DDR_PLL_2, REG_READ_U32(REG_DDR_PLL_2) & (~(0xD<<16)));	

//__attribute__((section(".init_ram_entry")))
int ss_init_rxi310(void)
#ifdef BRANCH_ONLY_IRAM
{
        #if 0
        // simple test dprintf for loading code to SRAM
        dprintf("20161028_1742~~~~~~~~~~~~~~~~~~~~~~~\n");
        bl2_calltest1();
        if(dram_normal_patterns(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("dram_normal_patterns test failed ");        return -1;}//fail}

        if(dram_addr_rot(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("dram_addr_rot test failed ");        return -1;}//fail}

        if(dram_com_addr_rot(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("dram_com_addr_rot test failed ");        return -1;}//fail}

        //comment for code size 
        if(dram_byte_access(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("dram_byte_access test failed ");        return -1;}//fail}

        if(dram_half_word_access(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("dram_half_word_access test failed ");        return -1;}//fail}    

        if(dram_walking_of_1(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("dram_walking_of_1 test failed ");        return -1;}//fail}

        if(dram_walking_of_0(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("dram_walking_of_0 test failed ");        return -1;}//fail}    

        if(memcpy_test(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
        {dprintf("memcpy_test test failed ");        return -1;}//fail}

        bl2_calltest2();
        #else
        void (*jump_func)(void);
        jump_func = (void *)(0xb0000000);
        jump_func();
        #endif
    return 0;
}
#else
{
        //dwc_serial_init();
        struct ms_rxi310_portmap *ms_ctrl_0_map;
        unsigned int strap_pin, pdram_type_idx, pdram_freq_idx, ver_idx, bd_idx, dram_vendor = 0, dram_vendor_invisible = 0;
        unsigned int real_vendor = 0;
        // vendor_map[dram_vendor_invisible][dram_vendor] 
        unsigned char vendor_map[2][6] = 
        {
                {0,1,2,3,4,5}, // 0: Etron/FB bonding, 1: Winbond, 2: ESMT, 3: PieceMakers, 4: NTC, 5: Zentel(APM)
                {2,5,2,5,2,2}
        };
        ms_ctrl_0_map = (struct ms_rxi310_portmap*) BSP_MS_I_DRAMC_0_BASE;

        TURN_OFF_CLOCK();

        // TODO: modify it to DDR1 200,  DDR1 400
        // (dram_type, dram_freq)
        // 0,0 => DDR2 400 MHz
        // 0,1 => DDR2 533 MHz
        // 1,0 => DDR1 200 MHz
        // 1,1 => DDR1 250 MHz
       set_rom_progress(0x35);
        unsigned char bond = BIT_GET_CHIP_ID_CODE(REG32(REG_BOND_OPTION));
        strap_pin = REG_READ_U32(REG_HW_STRAP_DDR);
        pdram_type_idx = BIT_GET_STRAP_PIN_DRAM_TYPE_INV(strap_pin);
        if ((strap_pin & BIT_STRAP_PIN_HW_DBG_DISABLE) == BIT_STRAP_PIN_HW_DBG_DISABLE) {
           pdram_freq_idx = BIT_GET_STRAP_PIN_DRAM_FEQ(strap_pin);
        } else { // HW_DBG_DISABLE == 0
           pdram_freq_idx = 0;
        }

        //get vendor
        dram_vendor_invisible = read_efuse_byte_iram(OFFSE_EFUSE_PHYSICAL_OPTIONS_0x7C);
        dram_vendor = read_efuse_byte_iram(OFFSE_EFUSE_PHYSICAL_OPTIONS_0x71);
        if(dram_vendor_invisible & 0x1)
        {
                real_vendor = vendor_map[1][((dram_vendor_invisible>>1)&0x1f)];
        }
        else
        {
                real_vendor = vendor_map[0][dram_vendor];
        }

        _memctl_debug_printf("bond:0x%x\n", bond);

        struct dram_device_info dram_table[2][2];
#if 1
        if (IS_FB_BONDING(bond)) { // CONFIG_DDR2_DISCRET, for TFBGA
                dram_table[0][0] = ddr2_800_dram_discret_info;
                dram_table[0][1] = ddr2_1066_dram_discret_info;
                dram_table[1][0] = ddr1_400_dram_info;
                dram_table[1][1] = ddr1_500_dram_info;
        } 
        else 
#endif
#ifndef DDR_CALIBRATION
        {
                if(IS_MCM128_BONDING(bond))
                {
                        set_rom_progress(0x36);
                        _memctl_debug_printf("MCM 128MB\n");
                        switch(real_vendor){
                                case 0: 
                                        // Etron, but for 97G, no Etron, use ESMT/default parameter
                                        //_memctl_debug_printf("0 dram vendor esmt \n");
                                        dram_table[0][0] = ddr2_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_1066_dram_M14D5121632A_info;             
                                        break;
                                case 1: 
                                        // winbond
                                        //_memctl_debug_printf("dram vendor winbond \n");
                                        dram_table[0][0] = ddr2_winbond_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_winbond_1066_dram_M14D5121632A_info;                                        
                                        break;
                                case 2: 
                                        //ESMT/default parameter
                                        _memctl_debug_printf("dram vendor esmt \n");
                                        dram_table[0][0] = ddr2_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_1066_dram_M14D5121632A_info;                                        
                                        break; 
                                case 5: 
                                        //APM parameter
                                        _memctl_debug_printf("dram vendor apm \n");
                                        dram_table[0][0] = ddr2_APM32MB_800_dram;
                                        dram_table[0][1] = ddr2_APM32MB_1066_dram_info;                                        
                                        break;       
                                default: 
                                        //ESMT/default parameter
                                        _memctl_debug_printf("default dram vendor esmt \n");
                                        dram_table[0][0] = ddr2_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_1066_dram_M14D5121632A_info;                                        
                                        break;

                        }
                        
                        dram_table[1][0] = ddr1_400_dram_info;
                        dram_table[1][1] = ddr1_500_dram_info;
                        /*
                        memcpy(&(dram_table[0][0]), &ddr2_800_dram_M14D5121632A_info, 24);
                        memcpy(&(dram_table[0][1]), &ddr2_1066_dram_M14D5121632A_info, 24);
                        memcpy(&(dram_table[1][0]), &ddr1_400_dram_info, 24);
                        memcpy(&(dram_table[1][1]), &ddr1_500_dram_info, 24);
                        */
                }
                else
                {
                        set_rom_progress(0x37);
                        _memctl_debug_printf("MCM 64/32MB\n");
                        switch(real_vendor){
                                case 0: 
                                        // Etron, but for 97G, no Etron, use ESMT/default parameter
                                        //_memctl_debug_printf("0 dram vendor esmt \n");
                                        dram_table[0][0] = ddr2_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_1066_dram_M14D5121632A_info;             
                                        break;
                                case 1: 
                                        // winbond
                                        //_memctl_debug_printf("dram vendor winbond \n");
                                        dram_table[0][0] = ddr2_winbond_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_winbond_1066_dram_M14D5121632A_info;                                        
                                        break;
                                case 2: 
                                        //ESMT/default parameter
                                        //_memctl_debug_printf("dram vendor esmt \n");
                                        dram_table[0][0] = ddr2_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_1066_dram_M14D5121632A_info;                                        
                                        break; 
                                case 5: 
                                        //APM parameter
                                        //_memctl_debug_printf("dram vendor apm \n");
                                        dram_table[0][0] = ddr2_APM32MB_800_dram;
                                        dram_table[0][1] = ddr2_APM32MB_1066_dram_info;                                        
                                        break;       
                                default: 
                                        //ESMT/default parameter
                                         //_memctl_debug_printf("default dram vendor esmt \n");
                                        dram_table[0][0] = ddr2_800_dram_M14D5121632A_info;
                                        dram_table[0][1] = ddr2_1066_dram_M14D5121632A_info;                                        
                                        break;

                        }
                        

                        dram_table[1][0] = ddr1_400_dram_info;
                        dram_table[1][1] = ddr1_500_dram_info;
                        /*
                        memcpy(&(dram_table[0][0]), &ddr2_800_dram_M14D5121632A_info, 24);
                        memcpy(&(dram_table[0][1]), &ddr2_1066_dram_M14D5121632A_info, 24);
                        memcpy(&(dram_table[1][0]), &ddr1_400_dram_info, 24);
                        memcpy(&(dram_table[1][1]), &ddr1_500_dram_info, 24);
                        */
                }
        }
#else
        memcpy(&(dram_table[0][0]), &ddr2_800_dram_M14D5121632A_info, 24);
        memcpy(&(dram_table[0][1]), &ddr2_1066_dram_M14D5121632A_info, 24);
#endif
        dram_init_pll_ASIC(ms_ctrl_0_map, &dram_table[pdram_type_idx][pdram_freq_idx], &strap_pin);
        set_rom_progress(0x38);
        dram_init_dpi_ip_ASIC(ms_ctrl_0_map, &dram_table[pdram_type_idx][pdram_freq_idx]);
        set_rom_progress(0x39);
#ifdef NO_CONFIG_RTL8197F
        dram_init_rxi310_ASIC(ms_ctrl_0_map, &dram_table[pdram_type_idx][pdram_freq_idx]);
#else
        dram_init_rxi310_ASIC_97G(ms_ctrl_0_map, &dram_table[pdram_type_idx][pdram_freq_idx]);
        set_rom_progress(0x3a);

#endif
        
        set_rom_progress(0x3b);        
        
        dram_calibration_turn_on_odt(&dram_table[pdram_type_idx][pdram_freq_idx]);
        dram_auto_size_detect_rxi310(ms_ctrl_0_map, &dram_table[pdram_type_idx][pdram_freq_idx]);
        
#ifdef DDR_CALIBRATION
        dram_calibration_entry(&dram_table[pdram_type_idx][pdram_freq_idx]);
#endif

#ifdef DDR_DUMP_ALL_REGISTER
        dram_dump_all_register();
#endif
#if 0
        set_rom_progress(0x3c);
        //_memctl_debug_printf("dram test\n");

        if(dram_normal_patterns(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail
                
        if(dram_addr_rot(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail

        if(dram_com_addr_rot(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail

        //comment for code size	
        if(dram_byte_access(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail

        if(dram_half_word_access(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail


        if(dram_walking_of_1(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail

        if(dram_walking_of_0(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail


        if(memcpy_test(CONFIG_SYS_MEMTEST_START, 0x100, 0x10))
                return -1;//fail
        set_rom_progress(0x3d);
#endif
        printf("\nDDR init OK\n");
        return 0;


//#ifdef DIRECT_JUMP
#ifdef CONFIG_SD_CARD_BOOTING 
        printf("\nDDR init OK\n");
#if 0
	{
	    unsigned int addr;
	    unsigned char sys_boot_type;
	    unsigned int strap_pin;
	    EFUSE_DATA efuse_data;
	    void (*jump_func)(void);

	    dprintf("%s(%d): init ram function 20160408_1\n", __func__, __LINE__);

	    strap_pin = REG32(REG_HW_STRAP);
	    sys_boot_type = BIT_GET_STRAP_PIN_BOOT_SEL(strap_pin);
	    addr = (unsigned int)UBOOT_ADDR;  
	    memset(&efuse_data, 0, sizeof(EFUSE_DATA));

        //globalDebugLevel = DBG_MESSAGE;
	    load_data_from_storage(sys_boot_type, &efuse_data, addr, UBOOT_SIGNATURE);

	    addr += 0x10;

	    dprintf("%s(%d): addr:0x%x \n", __func__, __LINE__, addr);    
	    jump_func = (void *)(addr);
	    jump_func(); 
	}
#else // for sd card booting
    {
        unsigned int ret_val;
        FATFS fatFs;
        FIL fil;
        unsigned int br;
        //char filename[20] = "boot_ok.bin";
         //unsigned int addr = UBOOT_ADDR;
         //char filename[20] = "nfjrom";
         //unsigned int addr = LINUX_ADDR;	

	char filename[20] = "boot.img"; 
	unsigned int addr = BOOTCODE_ADDR;
	
        PIMG_HEADER_TYPE p_header = (PIMG_HEADER_TYPE)addr;
        void (*jump_func)(void);

        printf("%s(%d): 0x%x, 0x%x\n", __func__, __LINE__, &fatFs, &fil);
        //globalDebugLevel = DBG_MESSAGE;

        fatFs.win = (unsigned char *)CACHE_2_NONCACHE_ADDR((unsigned int)fatFs.win1);

        ret_val = f_mount(0, (FATFS *)(&fatFs));
        ret_val = f_open((FIL *)(&fil), filename, FA_READ);

        if (ret_val) {
            printf("%s(%d): open file fail(0x%x) \n", __FUNCTION__, __LINE__, ret_val);
            return -1;
        }

        ret_val = f_read((FIL *)(&fil), (unsigned char *)addr, fil.fsize, &br);
        printf("%s(%d): read 0x%x byte to 0x%x\n", __func__, __LINE__, fil.fsize, addr);
        f_close(&fil);
#if 0
        if (check_image_header(p_header, UBOOT_SIGNATURE) == FALSE) {
            return -1;
        }

	    addr += 0x10;
#endif
	    printf("%s(%d): addr:0x%x \n", __func__, __LINE__, addr);    
	    jump_func = (void *)(addr);
	    jump_func(); 
    }
#endif
#endif
	return 0;
}
#endif

#if 1
void WAIT_DONE_RTK(unsigned int addr, unsigned int mask, unsigned int value)
{
        int timeout = 0;

	while ( (REG_READ_U32(addr) & mask) != value )
	{

                if(timeout++>100000){
                    _memctl_debug_printf("[%s]timeout..%d\n",__FUNCTION__, timeout);
                    return;
                 }
    
		asm("nop");
	}
}

#endif

void dram_set_frequence_register(const struct dram_device_info *dram_info,const unsigned short* pll_table)
{	
	//2c8
	unsigned int ddr_pll_6, ddr_pll_7, mck_dqs,mck_ck;
	ddr_pll_6 = (pll_table[0] << REG_CCO_BAND) | (pll_table[1] << REG_CCO_KVCO) | (pll_table[2] << REG_LOOP_PI_ISEL)  | 
		(pll_table[6] << REG_PLL_LDO_VSEL) | (pll_table[7] << REG_V10_LDO_VSEL) | (pll_table[8] << REG_PDIV) | 
		(pll_table[10] << REG_ICP) |(pll_table[11] << REG_LPF_CP) | (pll_table[12] << REG_LPF_SR);
	ddr_pll_7 = (pll_table[3] << REG_POST_PI_BIAS) | (pll_table[4] << REG_POST_PI_RL) | (pll_table[5] << REG_POST_PI_RS) | 
		(pll_table[9] << REG_PLL_CPMODE);

	//set output enable	
	if(dram_info->phy->mck_default < 0 )
		mck_ck = dram_info->phy->mck_default + 64;
	else
		mck_ck = dram_info->phy->mck_default;
	if(dram_info->phy->mdqs_default <0 )
		mck_dqs = dram_info->phy->mdqs_default + 64;
	else
		mck_dqs = dram_info->phy->mdqs_default;

	if((mck_ck >= 16) && (mck_ck <= 31))
		ddr_pll_7 = (ddr_pll_7 & 0xFFF0FFFF) | (0x1 << 16); 
	else if((mck_ck >= 32) && (mck_ck <= 47))
		ddr_pll_7 = (ddr_pll_7 & 0xFFF0FFFF) | (0x0 << 16); 
	else if((mck_ck >= 48) && (mck_ck <= 63))
		ddr_pll_7 = (ddr_pll_7 & 0xFFF0FFFF) | (0x1 << 16); 

	if((mck_dqs >= 16) && (mck_dqs <= 31))
		ddr_pll_7 = (ddr_pll_7 & 0xFFF3FFFF) | (0xC << 16); 
	else if((mck_dqs >= 32) && (mck_dqs <= 47))
		ddr_pll_7 = (ddr_pll_7 & 0xFFF3FFFF) | (0x0 << 16); 
	else if((mck_dqs >= 48) && (mck_dqs <= 63))
		ddr_pll_7 = (ddr_pll_7 & 0xFFF3FFFF) | (0xC << 16); 
	
	REG_WRITE_U32(REG_DDR_PLL_6, ddr_pll_6);
	REG_WRITE_U32(REG_DDR_PLL_7, ddr_pll_7);
	return;
}

void dram_init_clk_frequency(const struct dram_device_info *dram_info, unsigned int *pstrap_pin)
{
        unsigned int ddr_freq = (1000000/(dram_info -> ddr_period_ps) *2); // unit: MHz/s
        unsigned int dram_ck_n_code;// = (((1000000/(dram_info -> ddr_period_ps))*2) /DDRPLL_reference_clock) -3;
        unsigned int dram_ck_f_code;// =((ddr_freq*1000/DDRPLL_reference_clock - (dram_ck_n_code+3)*1000)*8192)/1000;
        unsigned int freq;

        if ((*pstrap_pin & BIT_STRAP_PIN_SEL_40M) == BIT_STRAP_PIN_SEL_40M) {
        	dram_ck_n_code = ((1000000/(dram_info ->ddr_period_ps))*2 /DDRPLL_reference_clock_40) -3;
        	dram_ck_f_code = ((ddr_freq*1000/DDRPLL_reference_clock_40 - (dram_ck_n_code+3)*1000)*8192)/1000;
        }
        else {
        	dram_ck_n_code = ((1000000/(dram_info ->ddr_period_ps))*2 /DDRPLL_reference_clock_25) -3;
        	dram_ck_f_code = ((ddr_freq*1000/DDRPLL_reference_clock_25 - (dram_ck_n_code+3)*1000)*8192)/1000;
        }
        //Clock frequency control
        //reg_dpi_pdiv , (set  sys_reg 0x1800_02C8[25:24] )
        if(ddr_freq <= 500)
        {
        	//rl6387_pll_frequence_table[6][]
        	freq = 6;
        }
        else if( ddr_freq > 500 && ddr_freq <= 750 )
        {
        	//rl6387_pll_frequence_table[5][]
        	freq = 5;
        }
        else if( ddr_freq > 750 && ddr_freq <= 800)
        {
        	//rl6387_pll_frequence_table[4][]
        	freq = 4;
        }
        else if( ddr_freq > 800 && ddr_freq <= 1100)
        {
        	//rl6387_pll_frequence_table[3][]
        	freq = 2;
        }
        else if( ddr_freq > 1100 && ddr_freq <= 1150)
        {
        	//rl6387_pll_frequence_table[2][]
        	freq = 2;
        }
        else if( ddr_freq > 1150 && ddr_freq <= 1400)
        {
        	//rl6387_pll_frequence_table[1][]
        	freq = 1;
        }
        else if( ddr_freq >= 1400)
        {
        	//rl6387_pll_frequence_table[0][]
        	freq = 0;
        }


        REG_WRITE_U32(REG_DDR_PLL_11, (dram_ck_f_code << BIT_CRT_F_CODE) | (dram_ck_n_code << BIT_CRT_N_CODE));

        dram_set_frequence_register(dram_info,&(rl6387_pll_frequence_table[freq][0]));
        _memctl_debug_printf("\n %s ,ddr_freq=%d (Mbps), %d (MHZ) \n",__FUNCTION__, ddr_freq, ddr_freq/2);

        return;
}

void dram_init_pll_ASIC (struct ms_rxi310_portmap *dev_map,
               const struct dram_device_info *dram_info,
                unsigned int *pstrap_pin)
{
        unsigned int reg_temp;

        // BIT_STRAP_PIN_DDR_LDO_SEL: 1: LDO, 0:SWR
        unsigned char switching_regulator = ((*pstrap_pin & BIT_STRAP_PIN_DDR_LDO_SEL) == BIT_STRAP_PIN_DDR_LDO_SEL) ? 0 : 1;

        /*
        DDRPHY CHIP CONTROL PART (in RL6387 system register)
        */
        // DRAM TYPE register 0x180002b0
        if(dram_info->dev->device_type == DDR_1)
        {
#ifdef NO_CONFIG_RTL8197F
                REG_WRITE_U32(REG_DDR_PLL_0, 0x0);
                REG_WRITE_U32(REG_DDR_PLL_3, 0x20 << 8); // fixed ddr1 bounding error on CLK, CLKn

#else
                reg_temp = REG_READ_U32(REG_BOND_OPTION) & 0xF;
                if(reg_temp >= 4) // bonding MCM
                {
                        REG_WRITE_U32(REG_DDR_PLL_0, 0x0);
                        REG_WRITE_U32(REG_DDR_PLL_3, 0x20 << 8); // fixed ddr1 bonding error on CLK, CLKn
                }
                else
                {
                        REG_WRITE_U32(REG_DDR_PLL_0, 0x4);
                        REG_WRITE_U32(REG_DDR_PLL_3, 0x20 << 8); // fixed ddr1 bonding error on CLK, CLKn                                     
                }
#endif
        }
        else if(dram_info->dev->device_type == DDR_2)
        {
                reg_temp = REG_READ_U32(REG_BOND_OPTION) & 0xF;

                if(reg_temp >= 4) // bonding MCM
                        REG_WRITE_U32(REG_DDR_PLL_0, 0x0);
                else
                        REG_WRITE_U32(REG_DDR_PLL_0, 0x1);              
        }

//set pow_ldo14 0x1800028c [28]
#ifdef NO_CONFIG_RTL8197F
        REG_WRITE_U32(REG_SWR_DDR_3, REG_READ_U32(REG_SWR_DDR_3) | (1<<28));
        //wait 1 ms
udelay(1500); 


        //pow_ldo15 read strap pin, then fill 0x1800028c [27] LOD = 1, SW = 1
        REG_WRITE_U32(REG_SWR_DDR_3, REG_READ_U32(REG_SWR_DDR_3) | (1<<27));

        if(!switching_regulator)
        {
                //if ldo wait 100us, then enable enb_LDO_diode_L 0x18000284[12] = 1
                //REG_WRITE_U32(REG_SWR_DDR_3, REG_READ_U32(REG_SWR_DDR_3) & 0xefffffff);
                udelay(100);
                REG_WRITE_U32(REG_SWR_DDR_1, REG_READ_U32(REG_SWR_DDR_1) | 0x1000);
        }
#else
       udelay(1000);
       REG_WRITE_U32(REG_SWR_DDR_0, REG_READ_U32(REG_SWR_DDR_0) | 0x1); //pow_regu = 1
       udelay(150);
       REG_WRITE_U32(REG_SWR_DDR_0, REG_READ_U32(REG_SWR_DDR_0) | 0x2); //, enb_ldo_diode_H= 1
#endif
        /*
        DDRPHY CRT PART (in RL6387 system register)
        */
        //Enable RBUS control circuit, dpi_crt_rst_n = 1 ( set  sys_reg 0x1800_02EC[1] = 1)
        REG_WRITE_U32(REG_DDR_PLL_15, REG_READ_U32(REG_DDR_PLL_15) | 0x2);

        //PLL LDO enable, dpi_pll_ldo_rst_n = 1 ( set  sys_reg 0x1800_02EC[0] = 1)
        REG_WRITE_U32(REG_DDR_PLL_15, REG_READ_U32(REG_DDR_PLL_15) | 0x1);

        //V10_LDO_EN_33V , en_ldo_ddr 0x1800_0024[31] = 1
        REG_WRITE_U32(REG_PLL_1, REG_READ_U32(REG_PLL_1) |(1<<31) );
        //Enable DDRPLL reference clk (reference clk gating), en_ref_clk_ddr 0x1800_0024[23] = 1
        REG_WRITE_U32(REG_PLL_1, REG_READ_U32(REG_PLL_1) | (1<<23) );

        //PLL init value setting,  1. Enable post PI, reg_dpi_en_post_pi = 0x1F ( set  sys_reg 0x1800_02B8[4:0] = 0x1F)
        REG_WRITE_U32(REG_DDR_PLL_2, REG_READ_U32(REG_DDR_PLL_2) | 0x1F);

        //PLL init value setting,  2. Clock frequency control
        dram_init_clk_frequency(dram_info,pstrap_pin);

        //PLL init value setting,  3. DFI_ratio select, reg_dpi_dfi_rate_sel (set  sys_reg 0x1800_02B4[17] ) 
        //read efuse    
        if(dram_info->dfi_rate == DFI_RATIO_1)
                REG_WRITE_U32(REG_DDR_PLL_1, REG_READ_U32(REG_DDR_PLL_1) | 0x1<<BIT_CRT_DFI_RATE_SEL);
        else
                REG_WRITE_U32(REG_DDR_PLL_1, REG_READ_U32(REG_DDR_PLL_1) & (~(0x1<<BIT_CRT_DFI_RATE_SEL)));

        //set REG_DDR_PLL_1 
        REG_WRITE_U32(REG_DDR_PLL_1, (REG_READ_U32(REG_DDR_PLL_1) & 0xE003FE0F) | (dram_info->phy->reg_ddr_pll_1 & 0x1FFC01F0));
        
        if( 0 <= dram_info->phy->mck_default)
        {
                REG_WRITE_U32(REG_DDR_PLL_3,(REG_READ_U32(REG_DDR_PLL_3) & 0xFFFFC0FF) | (dram_info->phy->mck_default << 8));
        }
        else if(dram_info->phy->mck_default < 0)
        {
                REG_WRITE_U32(REG_DDR_PLL_3,(REG_READ_U32(REG_DDR_PLL_3) & 0xC0FFFFFF) | ((dram_info->phy->mck_default+64) << 8));
        }

        if( 0 <= dram_info->phy->mdqs_default)
        {
                REG_WRITE_U32(REG_DDR_PLL_3,(REG_READ_U32(REG_DDR_PLL_3) & 0xC0FFFFFF) | (dram_info->phy->mdqs_default << 24));
                REG_WRITE_U32(REG_DDR_PLL_4, (dram_info->phy->mdqs_default));
        }
        else if(dram_info->phy->mdqs_default < 0)
        {
                REG_WRITE_U32(REG_DDR_PLL_3,(REG_READ_U32(REG_DDR_PLL_3) & 0xC0FFFFFF) | ((dram_info->phy->mdqs_default+64) << 24));
                REG_WRITE_U32(REG_DDR_PLL_4, (dram_info->phy->mdqs_default+64));        
        }

        //PLL init value setting,  4.   Enable PLL and PHY, dpi_rst_n = 1 ( set  sys_reg 0x1800_02EC[2]  = 1)
        udelay(50);
        REG_WRITE_U32(REG_DDR_PLL_15, REG_READ_U32(REG_DDR_PLL_15) | 0x4);
        //and wait done
        WAIT_DONE_RTK(REG_DDR_PLL_15, 0x10000, 0x10000);
        //wait stable
        udelay(10000); 

        //Turn on clock flow, reg_dpi_ck_en = 0xf (set  sys_reg 0x1800_02B4[3:0] = 0xf) ---> reg_dpi_clk_oe = 0xf (set  sys_reg 0x1800_02B8[19:16] = 0xf)
        REG_WRITE_U32(REG_DDR_PLL_1, REG_READ_U32(REG_DDR_PLL_1) | 0xf);
        REG_WRITE_U32(REG_DDR_PLL_2, REG_READ_U32(REG_DDR_PLL_2) | (0xf<<16));

        return;
}


void dram_init_dpi_ip_ASIC (struct ms_rxi310_portmap *dev_map,
                const struct dram_device_info *dram_info)
{
        //set DDR1 or DDR2 mode, PAD_REF 0x1800_02D0[3]
        if(dram_info->dev->device_type == DDR_1)
        	REG_WRITE_U32(PAD_REF, 0x0);
        else if(dram_info->dev->device_type == DDR_2)
        	REG_WRITE_U32(PAD_REF, 0x8);

        //PAD-DQS single end, DDR1/2 always differential
        if(dram_info->dev->device_type == DDR_1)
        {
        	REG_WRITE_U32(PAD_DQS, REG_READ_U32(PAD_DQS) | (1 << SINGLE_END_ENABLE));
        	REG_WRITE_U32(PAD_DQS_2, REG_READ_U32(PAD_DQS_2) | (1 << SINGLE_END_ENABLE));
        }
        else if(dram_info->dev->device_type == DDR_2)
        {
        	REG_WRITE_U32(PAD_DQS, REG_READ_U32(PAD_DQS) & (~(1 << SINGLE_END_ENABLE)));
        	REG_WRITE_U32(PAD_DQS_2, REG_READ_U32(PAD_DQS_2) & (~(1 << SINGLE_END_ENABLE)));
        }

        //DFI_ratio select, set BIST_2TO1_0(0x1800_03A0) Set 2to1_en [26]
        // Set 2to1_rddelay[28] and 2to1_wrdelay [27], to enable odd delay
        if(dram_info->dfi_rate == DFI_RATIO_1)
        	REG_WRITE_U32(BIST_2TO1_0, 0x10000000);
        else
        	REG_WRITE_U32(BIST_2TO1_0, 0x14000000);

        	dram_set_wrlvl_fb((dram_info->phy->mdqs_default < 0)?(-(dram_info->phy->mdqs_default + 64)):dram_info->phy->mdqs_default);

        //set command/address  output delay control,CMD_CTRL 0x1800_0224, // if set bit[2] = 0, the delay is 7
        REG_WRITE_U32(CMD_CTRL, 0x0);

        //set address delay chain, (0x200 & 0x204), //TODO, how to set?
        REG_WRITE_U32(ADR_DLY_0, dram_info->phy->adr_dly_0); //set default
        REG_WRITE_U32(ADR_DLY_1, dram_info->phy->adr_dly_1);
        REG_WRITE_U32(ADR_DLY_2, (REG_READ_U32(ADR_DLY_2)&0xFFFFF000) | dram_info->phy->ba_dly);
        REG_WRITE_U32(CMD_DLY_0, (REG_READ_U32(CMD_DLY_0)&0xFF000000) | dram_info->phy->cmd_dly_0);
        REG_WRITE_U32(DQ_DLY_0, dram_info->phy->tx_delay_tap_0);
        REG_WRITE_U32(DQ_DLY_0_1, dram_info->phy->tx_delay_tap_1);

        //REG_WRITE_U32(DQ_DLY_1, (REG_READ_U32(DQ_DLY_1) & 0xffffff0f) | dram_info->phy->dm_dly_sel_0 <<4); //joyce
        //REG_WRITE_U32(DQ_DLY_1, (REG_READ_U32(DQ_DLY_1) & 0xffff0fff) | dram_info->phy->dm_dly_sel_1 <<12); //joyce

        REG_WRITE_U32(DQ_DLY_1, (REG_READ_U32(DQ_DLY_1) & 0xFFFF0000) |  dram_info->phy->dqs_dm_delay_tap);
        //REG_WRITE_U32(DPI_CTRL_0, REG_READ_U32(DPI_CTRL_0))	


        //set write_en_0 & fw_set_wr_dly  DPI_CTRL_1, (0x31C)
        REG_WRITE_U32(DPI_CTRL_1, (REG_READ_U32(DPI_CTRL_1) & 0xfffffffc) | 0x3);

        //set READ_CTRL_0(0x244) : tm_dqs_en, TODO how to set? need efuse?
        REG_WRITE_U32(READ_CTRL_0, (REG_READ_U32(READ_CTRL_0) & 0xFFFFFFE0) | dram_info->phy->dqs_en_default);
        REG_WRITE_U32(READ_CTRL_0_1, (REG_READ_U32(READ_CTRL_0_1) & 0xFFFFFFE0) | dram_info->phy->dqs_en_default);

        //set READ_CTRL_1(0x254) : tm_rd_fifo, rd_dly_follow_dq0
        REG_WRITE_U32(READ_CTRL_1, (REG_READ_U32(READ_CTRL_1) & 0xFFFFFFE0) | dram_info->phy->rx_fifo_default);
        //set DPI_CTRL_0(0x318) : fw_set_mode, cal_set_mode
        //REG_WRITE_U32(DPI_CTRL_0, REG_READ_U32(DPI_CTRL_0));

        //set DQS_IN_DLY_0(0x28C) : fw_rd_dly_pos_sel_0
        REG_WRITE_U32(DQS_IN_DLY_0, dram_info->phy->rx_delay_tap_0);
        REG_WRITE_U32(DQS_IN_DLY_0_1, dram_info->phy->rx_delay_tap_2);
        REG_WRITE_U32(DQS_IN_DLY_1, dram_info->phy->rx_delay_tap_1);
        REG_WRITE_U32(DQS_IN_DLY_1_1, dram_info->phy->rx_delay_tap_3);

        //set DQS_IN_DLY_2(0x2AC) : fw_rd_dly_neg_sel_0
        REG_WRITE_U32(DQS_IN_DLY_2, dram_info->phy->rx_delay_tap_n_0);
        REG_WRITE_U32(DQS_IN_DLY_2_1, dram_info->phy->rx_delay_tap_n_2);
        REG_WRITE_U32(DQS_IN_DLY_3, dram_info->phy->rx_delay_tap_n_1);
        REG_WRITE_U32(DQS_IN_DLY_3_1, dram_info->phy->rx_delay_tap_n_3);

        //set 3-point calibration pre/post shift
        REG_WRITE_U32(CAL_LS_SEL, dram_info->phy->pre_shift_0);
        REG_WRITE_U32(CAL_LS_SEL_1, dram_info->phy->pre_shift_1);
        REG_WRITE_U32(CAL_RS_SEL, dram_info->phy->post_shift_0);
        REG_WRITE_U32(CAL_RS_SEL_1, dram_info->phy->post_shift_0);

        //set CAL_SHIFT_CTRL
        REG_WRITE_U32(CAL_SHIFT_CTRL, dram_info->phy->cal_shift_ctrl);
        
        //set DPI_CTRL_1(0x31C) : fw_set_rd_dly
        REG_WRITE_U32(DPI_CTRL_1, REG_READ_U32(DPI_CTRL_1) | 0xc);

        return;
}
void dram_issue_dpin(struct ms_rxi310_portmap *dev_map, uint32_t cdpin, uint32_t tdpin, uint32_t cmd_info)
{
        uint32_t INIT_cnt = 0;

        
        dev_map->csr = 0x700 | cmd_info;
        
        dev_map->cdpin = cdpin;

        //tdpin = 0x7; // ODT, CKE, RST        
        dev_map->tdpin = tdpin;
        // issue dpin function
        dev_map->ccr = 0x8;
        udelay(10);
        while(1)
        {
                if (((dev_map-> ccr)& 0x8) == 0x0)
                {
                        udelay (10);    /* Delay 10ns  */
                        INIT_cnt++;
                        _memctl_debug_printf("\nDPIN is still active\n");
                        if (INIT_cnt>100)
                                break;
                }
                else
                {
                        _memctl_debug_printf("\nDPIN is done\n");
                        break;
                }
        }

        return;
}

void dram_init_flow(struct ms_rxi310_portmap *dev_map,
                                        uint32_t mr0, uint32_t emr1, uint32_t emr2, uint32_t emr3)
{                
        //disable auto refresh
        dev_map->drr = dev_map->drr | (0x1<<28);
        
        //set memory idle
        dev_map->csr = 0x700;
        dev_map->ccr = 0x80000000;
        udelay(10);
        
        // disable dynamic CKE & dynamic ODT
        dev_map-> iocr =  (dev_map-> iocr) | (0x3 << 2);

        // CKE high & ODT LOW
        dram_issue_dpin(dev_map, 0x1E00000, CKE_HIGH_ONLY, 0);
        
        //NOP
        dram_issue_dpin(dev_map, CMD_DPIN_NOP, CKE_RST_HIGH , 0);

        // delay 400ns, more than 400, 10000ns
        udelay(10);

        //precharge all
        dram_issue_dpin(dev_map, CMD_DPIN_PRECHARGE_ALL, CKE_RST_HIGH, 1);

        // EMR2
        emr2 = emr2 | CMD_DPIN_EMR2_ONE_BANK;
        dram_issue_dpin(dev_map, emr2 , CKE_RST_HIGH, 0);
        udelay(1);
        //EMR3
        emr3 = (0<<OFFSET_DPIN_ADDR_A0) | CMD_DPIN_EMR3_ONE_BANK;
        dram_issue_dpin(dev_map, emr3 , CKE_RST_HIGH, 0);
        udelay(1);
        //EMR1
        emr1 = emr1 | CMD_DPIN_EMR1_ONE_BANK;
        dram_issue_dpin(dev_map, emr1 , CKE_RST_HIGH, 0);
        udelay(1);
        //MR for DLL reset
        mr0 = mr0 | CMD_DPIN_MR_ONE_BANK | (0x1<<8);
        dram_issue_dpin(dev_map,mr0 , CKE_RST_HIGH, 0);
        udelay(1);

        //precharge all
        dram_issue_dpin(dev_map, CMD_DPIN_PRECHARGE_ALL, CKE_RST_HIGH, 1);
        udelay(1);

        // auto-Refresh
        dram_issue_dpin(dev_map, CMD_DPIN_AUTO_REFRESH, CKE_RST_HIGH, 0);
        dram_issue_dpin(dev_map, CMD_DPIN_AUTO_REFRESH, CKE_RST_HIGH, 0);
        dram_issue_dpin(dev_map, CMD_DPIN_AUTO_REFRESH, CKE_RST_HIGH, 0);
        udelay(10);

        //MR for disable DLL reset
        mr0 = (mr0 | CMD_DPIN_MR_ONE_BANK) & (~(0x1<<8));
        dram_issue_dpin(dev_map, mr0 , CKE_RST_HIGH, 0);
        udelay(1);

        // issue OCD Calibration Default (A9=A8=A7=HIGH)
        emr1 = emr1 | CMD_DPIN_EMR1_ONE_BANK | (0x7<<7);
        dram_issue_dpin(dev_map, emr1 , CKE_RST_HIGH, 0); 
        udelay(1);

        // disable OCD Calibration Default (A9=A8=A7=LOW)
        emr1 = (emr1 | CMD_DPIN_EMR1_ONE_BANK) & (~(0x7<<7));
        dram_issue_dpin(dev_map, emr1 , CKE_RST_HIGH, 0); 
        udelay(1);

        // Tie CKE low , it look like Power Down Entry
        dram_issue_dpin(dev_map, CMD_DPIN_ALL_HIGH , CKE_RST_HIGH, 0);

        //enable auto refresh
        dev_map->drr = dev_map->drr & (~(0x1<<28));

        //set dynamic CKE & dynamic ODT
        dev_map-> iocr =  ((dev_map-> iocr) & (~(0x3<<2))) | 
        (STATIC_CKE<<PCTL_IOCR_STC_CKE_BFO) | (STATIC_ODT<<PCTL_IOCR_STC_ODT_BFO);
        
        udelay(100);
        
        return;
}

#ifdef CONFIG_RTL8197F
void dram_init_rxi310_ASIC (struct ms_rxi310_portmap *dev_map,
                const struct dram_device_info *dram_info)
{

  _memctl_debug_printf("\nJSW :  dram_init_rxi310 ,dev_map=0x%x \n",(unsigned int)dev_map);

  _memctl_debug_printf("\nJSW :  dram_init_rxi310 ,dram_info=0x%x \n",(unsigned int)dram_info);

 

  uint32_t cr_bst_len = 0 ; // 0:bst_4, 1:bst_8
  uint32_t cas_wr, tphy_wd = 1, mrinfo_wr_lat;    // cas write latency,cas_wr_t,
  uint32_t cas_rd, cas_rd_t, tphy_rd = 0, crl_srt, mrinfo_rd_lat;    // cas read latency, cas_rd_t,crl_srt, 
  uint32_t cas_add, add_lat, mrinfo_add_lat; //add_lat,
  uint32_t dram_emr2 = 0, dram_mr0 = 0;
  uint32_t cr_twr, dram_max_wr, dram_wr;
  uint32_t cr_trtw, cr_trtw_t = 0;
  uint32_t dram_period; //, dram_periodx2;
  enum dram_type         ddr_type = DDR_1;
  enum dram_dq_width     dq_width;
  enum dram_page_size    page;
  uint32_t dfi_rate;



_memctl_debug_printf("\nJSW :  dram_init_rxi310 001\n");
  dfi_rate = 1 << (uint32_t) (dram_info->dfi_rate);
  dram_period = (dram_info-> ddr_period_ps)*(dfi_rate); // according DFI_RATE to setting


_memctl_debug_printf("\ndfi_rate=%d,dram_period=%d\n",dfi_rate,dram_period);

    cas_wr = (dram_info-> mode_reg-> dram_wr_lat ) + 
    	   (dram_info-> mode_reg-> dram_par_lat )+
    	   (dram_info-> mode_reg-> dram_add_lat );
    
    cas_rd = (dram_info-> mode_reg-> dram_rd_lat ) + 
    	   (dram_info-> mode_reg-> dram_par_lat )+
    	   (dram_info-> mode_reg-> dram_add_lat );
    
    cas_add = (dram_info-> mode_reg-> dram_add_lat );


_memctl_debug_printf("\nJSW :  dram_init_rxi310 002\n");
  // In PHY, write latency == 3
  dram_max_wr= (dram_info-> timing -> wr_max_tck)/(dfi_rate) +1;
  dram_wr = ((dram_info-> timing -> twr_ps) / dram_period)+1;

  { // SDR, DDR1
    cr_twr = ((dram_info-> timing -> twr_ps) / dram_period);
	_memctl_debug_printf("\nSDR D1  dram_info-> dev-> device_type=%d ,cr_twr=%d \n",dram_info-> dev-> device_type,cr_twr );
  }

  if (cr_twr < dram_max_wr) {
    cr_twr = cr_twr;
  }
  else {
    cr_twr = dram_max_wr;
  }

  if ((dram_info-> dev-> device_type) == DDR_2) {
   _memctl_debug_printf("\nDDR2  dram_info-> dev-> device_type=%d\n",dram_info-> dev-> device_type);
	
    ddr_type = DDR_2;
    if (dram_info-> mode_reg-> bst_len == BST_LEN_4) {
      cr_bst_len = 0; //bst_4
      cr_trtw_t = 2+2; //4/2+2
      dram_mr0 = 0x2;
	   _memctl_debug_printf("\nDDR2  dram_info-> mode_reg-> bst_len=%d\n",dram_info-> mode_reg-> bst_len);
    }
    else { // BST_LEN_8
      cr_bst_len = 1; // bst_8
      cr_trtw_t = 4+2; // 8/2+2
      dram_mr0 = 0x3;
	     _memctl_debug_printf("\nDDR2  dram_info-> mode_reg-> bst_len=%d\n",dram_info-> mode_reg-> bst_len);
    }
    if (dram_info-> mode_reg-> dram_rd_lat <= PHY_READ_LATENCY) {
      tphy_rd = 1;
    }
    else {
      if(dram_info->dfi_rate == DFI_RATIO_1)
	      tphy_rd = (dram_info-> mode_reg-> dram_rd_lat + PHY_ADDR_CMD_LATENCY - 11) / (dfi_rate);
      else
	      tphy_rd = (dram_info-> mode_reg-> dram_rd_lat - PHY_READ_LATENCY) / (dfi_rate) - 1;
	  	
    }
    if(dram_info->dfi_rate == DFI_RATIO_1)
    	tphy_wd = (dram_info-> mode_reg-> dram_wr_lat + (PHY_ADDR_CMD_LATENCY - PHY_WRITE_DATA_LATENCY))/(dfi_rate);
    else
	tphy_wd = (dram_info-> mode_reg-> dram_wr_lat + (PHY_ADDR_CMD_LATENCY - PHY_WRITE_DATA_LATENCY))/(dfi_rate) + 1;
    
	
    //cas_rd = dram_info-> mode_reg -> mode0_cas;
    //add_lat = dram_info-> mode_reg ->  mode1_all_lat;
    //cas_wr = cas_rd + add_lat -1;
    dram_emr2 = 0;

    dram_mr0  =(((dram_wr%6)-1)                  << (PCTL_MR_OP_BFO+1)) | // write_recovery
               (0                                << PCTL_MR_OP_BFO    ) | // dll
               (dram_info-> mode_reg-> mode0_cas << PCTL_MR_CAS_BFO   ) |
               (dram_info-> mode_reg-> bst_type  << PCTL_MR_BT_BFO    ) |
               dram_mr0;
  } // DDR2
  else if ((dram_info-> dev-> device_type) == DDR_1) {
  	  _memctl_debug_printf("\nDDR1-1   dram_info-> dev-> device_type=%d\n",dram_info-> dev-> device_type);

    REG_WRITE_U32(CMD_CTRL, 1<<2);
    ddr_type = DDR_1;
    if (dram_info-> mode_reg-> bst_len == BST_LEN_4) {
      dram_mr0 = 2; // bst_4
      cr_bst_len = 0; //bst_4
    }
    else { // BST_LEN_8
      dram_mr0 = 3; // bst_8
      cr_bst_len = 1; // bst_8
    }
    //cas_rd = dram_info-> mode_reg -> mode0_cas;
    if (dram_info-> mode_reg-> dram_rd_lat <= 1) {
      tphy_rd = 1;
    }
    else {
      tphy_rd = (dram_info-> mode_reg-> dram_rd_lat - 0) / (dfi_rate); //ddr1_cas
    }

    tphy_wd = 1 + 1;

    
    dram_mr0  =(cas_rd                          << PCTL_MR_CAS_BFO) |
               (dram_info-> mode_reg-> bst_type << PCTL_MR_BT_BFO ) |
               dram_mr0;

    cr_trtw_t = 0; // tic: cas_rd + rd_rtw + rd_pipe

    if (dfi_rate == 1) { // DFI_RATE== 2
      cas_rd = 1;
    }
    else { // DFI_RATE == 1
      cas_rd = cas_rd -1;
    }

    cas_wr = 1;

  } // ddr1
  
  if (dfi_rate == 1) {
    mrinfo_wr_lat = cas_wr;
    mrinfo_rd_lat = cas_rd;
    mrinfo_add_lat = (dram_info-> mode_reg-> dram_add_lat ); 
  }
  else {
    if (cas_wr%dfi_rate) {
      mrinfo_wr_lat = (cas_wr+1)/ dfi_rate;
    }
    else {
       mrinfo_wr_lat = cas_wr / dfi_rate;
    }
  
    if (cas_rd%dfi_rate) {
      mrinfo_rd_lat = (cas_rd+1)/ dfi_rate;
    }
    else {
       mrinfo_rd_lat = cas_rd / dfi_rate;
    }

    if (cas_add%dfi_rate) {
      mrinfo_add_lat = (cas_add+1)/ dfi_rate;
    }
    else {
       mrinfo_add_lat = cas_add / dfi_rate;
    }
  }


  // countting tRTW
  if ((cr_trtw_t & 0x1)) {
    cr_trtw = (cr_trtw_t+1) /(dfi_rate);
  }
  else {
    cr_trtw = cr_trtw_t /(dfi_rate);
  }

  dq_width = (dram_info-> dev-> dq_width); 

  _memctl_debug_printf("\nSDR   dq_width=%d\n", dq_width);
  //page = dram_info-> dev-> page +1; // DQ16 -> memory:byte_unit *2
  page = dram_info-> dev-> page ; // DQ16 -> memory:byte_unit *1 // page size=1K


  // _memctl_debug_printf("\nDCR(0xf8142004)=0x%x\n", REG32(0xf8142004));
  if (dq_width == DQ_32) { // paralle dq_16 => page + 1
    page = page +1;
  }

  if (dq_width == HALF_DQ32) {
    dq_width = 2; // {half_dq, DQ32}
  }


     _memctl_debug_printf("\nSDR   page=%d\n", page);
	  _memctl_debug_printf("\nSDR  dram_info-> dev-> bank =%d\n", dram_info-> dev-> bank );
	    _memctl_debug_printf("\ncr_bst_len =%d\n", cr_bst_len);
  //set CR_CSR(0xc): idle mode
  dev_map->csr = (1 << PCTL_CSR_MEM_IDLE_BFO) | (1 << PCTL_CSR_BIST_IDLE_BFO);
   
  // WRAP_MISC setting
  dev_map-> misc = (
                    (TRUNCATED(page,4)                   << WRAP_MISC_PAGE_SIZE_BFO) |  
                    (TRUNCATED(dram_info-> dev-> bank, 2) << WRAP_MISC_BANK_SIZE_BFO) |
                    (TRUNCATED(cr_bst_len, 2)             << WRAP_MISC_BST_SIZE_BFO )  
                   );

#if 0 //for 8198E FPGA
	dfi_rate=0;
	
#endif
 _memctl_debug_printf("\nSDR   dfi_rate=%d\n", dfi_rate);
	  _memctl_debug_printf("\nSDR  dq_width=%d\n", dq_width );
	    _memctl_debug_printf("\nddr_type =%d\n", ddr_type);

  // PCTL setting
  dev_map-> dcr = (
                   (dfi_rate << PCTL_DCR_DFI_RATE_BFO) |
                   (dq_width << PCTL_DCR_DQ32_BFO    ) |
                   (ddr_type << PCTL_DCR_DDR3_BFO    )
                  );


     _memctl_debug_printf("\nSDR   cas_rd /(dfi_rate)=%d\n", cas_rd /(dfi_rate));
	  _memctl_debug_printf("\nSDR  cas_wr /(dfi_rate) =%d\n", cas_wr /(dfi_rate) );
	    _memctl_debug_printf("\nSDR  dev_map-> dcr =0x%x\n", dev_map-> dcr );
	  

  dev_map-> iocr = (
                    (TRUNCATED(tphy_rd, 5)     << PCTL_IOCR_TPHY_RD_EN_BFO ) |
                    (0                        << PCTL_IOCR_TPHY_WL_BFO   ) |
                    (TRUNCATED(tphy_wd, 5)   << PCTL_IOCR_TPHY_WD_BFO   ) |
                    (0                        << PCTL_IOCR_RD_PIPE_BFO   ) |
                    (CS_2T<<4)
                   // (2                        << PCTL_IOCR_RD_PIPE_BFO   ) //For 8198E FPGA RD_PIPE
                   );

      _memctl_debug_printf("\nSDR  dev_map-> iocr=0x%x\n", dev_map-> iocr );

  if (((dram_info-> dev-> device_type) != SDR) || 
      ((dram_info-> dev-> device_type) != DDR_1)) { // DDR2/3 
    dev_map-> emr2 = dram_emr2;

    dev_map-> emr1 = (
                      ((dram_info->mode_reg->mode1_odt & 0x11) << 2 ) | //RTT
                      ((dram_info->mode_reg->mode1_ocd & 0x1)     << 1 ) | //D.I.C
                      (dram_info-> mode_reg -> mode1_dll_en_n )
                     );
  } // DDR2/3
	else if(((dram_info-> dev-> device_type) == DDR_1))
  {
      dev_map-> emr1 = (
                      ((dram_info->mode_reg->mode1_ocd & 0x21)     << 1 ) | //OCD
                      (dram_info-> mode_reg -> mode1_dll_en_n )
                     );
  }

    _memctl_debug_printf("\ndev_map-> emr1=0x%x\n", dev_map->emr1);
  

  dev_map->emr0 = dram_mr0;

    _memctl_debug_printf("\ndev_map-> mr=0x%x\n", dev_map->emr0);

    // _memctl_debug_printf("\nSoC dram_mr0(0xf8142020)=0x%x\n", REG32(0xf8142020));

  dev_map->mrinfo = (
                      ((mrinfo_add_lat & 0x1f)                        << PCTL_MRINFO_ADD_LAT_BFO) |
                      ((mrinfo_rd_lat  & 0x1f)                        << PCTL_MRINFO_RD_LAT_BFO ) |
                      ((mrinfo_wr_lat  & 0x1f)                        << PCTL_MRINFO_WR_LAT_BFO )
                     );


  
  #if 1
//  if(!dram_info-> timing -> trfc_ps_array)
  dev_map-> drr = (
                   (0                                                 << PCTL_DRR_REF_DIS_BFO) |
                   (9                                                 << PCTL_DRR_REF_NUM_BFO) |
                   (TRUNCATED((((dram_info-> timing -> trefi_ps)/dram_period)+1), 16)<< PCTL_DRR_TREF_BFO   ) |
                   (TRUNCATED((((dram_info-> timing -> trfc_ps)/dram_period)+1), 8)<< PCTL_DRR_TRFC_BFO   )
                  );
 #else
	dev_map-> drr =0x01007842; // For 8198E FPGA 25MHZ DRAM OSC
 #endif


    _memctl_debug_printf("\nSDR  dev_map-> drr=0x%x\n", dev_map-> drr );

 #if 1
  dev_map-> tpr0= (
                   (TRUNCATED((((dram_info-> timing -> trtp_tck)/dfi_rate)+1), 3)   << PCTL_TPR0_TRTP_BFO) |
                   (TRUNCATED(cr_twr, 4)                                            << PCTL_TPR0_TWR_BFO ) |
                   (TRUNCATED((((dram_info-> timing -> tras_ps)/dram_period)+1), 5)<< PCTL_TPR0_TRAS_BFO) |
                   (TRUNCATED((((dram_info-> timing -> trp_ps)/dram_period)+1), 4)  << PCTL_TPR0_TRP_BFO )
                  );
 #else
 dev_map-> tpr0=0xffff;

 #endif
 
 _memctl_debug_printf("\nSDR  dev_map-> tpr0=0x%x\n", dev_map-> tpr0);


  #if 1

  dev_map-> tpr1= (
                   (TRUNCATED(cr_trtw, 4)                                           << PCTL_TPR1_TRTW_BFO) |
                   (TRUNCATED((((dram_info-> timing -> twtr_tck)/dfi_rate)+3), 3)  << PCTL_TPR1_TWTR_BFO) |
                   (TRUNCATED((((dram_info-> timing -> tccd_tck)/dfi_rate)+1), 3)   << PCTL_TPR1_TCCD_BFO) |
                   (TRUNCATED((((dram_info-> timing -> trcd_ps)/dram_period)+1), 4) << PCTL_TPR1_TRCD_BFO) |
                   (TRUNCATED((((dram_info-> timing -> trc_ps)/dram_period)+1), 6)  << PCTL_TPR1_TRC_BFO ) |
                   (TRUNCATED(((dram_info-> timing -> trrd_tck/dfi_rate)+1), 4)       << PCTL_TPR1_TRRD_BFO)
                  );
 #else
 dev_map-> tpr1=0xffffffff ;

 #endif
 
   _memctl_debug_printf("\nSDR  dev_map-> tpr1=0x%x\n", dev_map-> tpr1);


  #if 1
  dev_map-> tpr2= (
                   (TRUNCATED((dram_info-> timing -> tmrd_tck/dfi_rate + 1), 3)<< PCTL_TPR2_TMRD_BFO        ) |
                   (0                              << PCTL_TPR2_INIT_NS_EN_BFO  ) |
                   (2                              << PCTL_TPR2_INIT_REF_NUM_BFO)
                  );
#else
  dev_map-> tpr2= 0xffffffff ; 
#endif


 _memctl_debug_printf("\nSDR  dev_map-> tpr2=0x%x\n", dev_map-> tpr2);


  // set all_mode _idle
  dev_map-> csr = 0x700;

   dev_map-> ccr = 0x0;
   _memctl_debug_printf("\nDRAM init disable\n");
  // start to init
  dev_map-> ccr = 0x1;
   _memctl_debug_printf("\nDRAM init enable\n");
   
  unsigned int INIT_cnt=0;
  #if 0
  while (((dev_map-> ccr)& 0x1) == 0x0); // init done
   #else
   while(1)
   {
   if (((dev_map-> ccr)& 0x1) == 0x0)
   {
       udelay (10);	/* Delay 10ns  */
      INIT_cnt++;
       _memctl_debug_printf("\nDRAM init is still active\n");
   	if (INIT_cnt>1000)
		break;
   }
   else
   {
	_memctl_debug_printf("\nDRAM init is done , jump to DRAM\n");
   	break;
   }
   }
 #ifdef ENABLE_DRAM_ODT
  if ((dram_info-> dev-> device_type) == DDR_2)
  {
   //enable DRAM ODT 
   _memctl_debug_printf("enable DRAM ODT \n");
   dev_map->tdpin = 0x7;
   dev_map->ccr = 0x8;
  }
#endif

   #endif
  // enter mem_mode
  dev_map-> csr= 0x600;
    _memctl_debug_printf("\nSDR  init done , dev_map=0x%x\n",(unsigned int)dev_map );
} // dram_init
#else
#define  	SCH_MISC 0x0
#define RD0 0x4
#define RD0_TIMEOUT0 	0x5 
#define RD0_TIMEOUT1 	0x6 
#define RD1 	0x7 
#define RD1_TIMEOUT0 	0x8 
#define RD1_TIMEOUT1 	0x9 
#define RD2 	0xa 
#define RD2_TIMEOUT0 	0xb 
#define RD2_TIMEOUT1 	0xc 
#define RD3 	0x10 
#define RD3_TIMEOUT0 	0x11 
#define RD3_TIMEOUT1 	0x12 
#define SCHEDULER_TIMEOUT_CYCLE_MODE 0x0
#define SCHEDULER_TIMEOUT_CMD_MODE 0x1

void dram_disable_scheduler (void)
{
        uint32_t temp_reg;
        // no use ms_rxi310_portmap to define scheduler base address, because the 97G rom & sram size is very small
        REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_INDEX_OFFSET, SCH_MISC);
        temp_reg = REG_READ_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_DATA_OFFSET);
        temp_reg = temp_reg | (0x1<<31);

        REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_INDEX_OFFSET, SCH_MISC);
        REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_DATA_OFFSET, temp_reg);
        return;
}

void dram_scheduler_timeout (unsigned int queue_reg, unsigned int count, unsigned int mode)
{
        uint32_t temp_reg;
        
        if(mode == 0)  // if cycle mode set, timeout0
        {
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_INDEX_OFFSET, queue_reg);
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_DATA_OFFSET, count);
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_INDEX_OFFSET, (queue_reg+1));
                temp_reg = REG_READ_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_DATA_OFFSET);
                temp_reg = temp_reg | (1<<16) | (1<<18);
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_INDEX_OFFSET, (queue_reg+1));
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_DATA_OFFSET, temp_reg);
        }
        else // set cmd count
        {
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_INDEX_OFFSET, queue_reg+1);
                temp_reg = REG_READ_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_DATA_OFFSET);
                temp_reg = (temp_reg & 0xFFFF0000) | (1<<17) | (1<<18) | (count & 0xffff);
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_INDEX_OFFSET, (queue_reg+1));
                REG_WRITE_U32(BSP_MS_I_DRAMC_0_BASE + MS_PCTL_SCH_DATA_OFFSET, temp_reg);
        }

        return;
}

void dram_init_rxi310_ASIC_97G (struct ms_rxi310_portmap *dev_map,
                const struct dram_device_info *dram_info)
{

        //_memctl_debug_printf("dev_map=0x%x \n",(unsigned int)dev_map);

        //_memctl_debug_printf("dram_info=0x%x \n",(unsigned int)dram_info);



        uint32_t cr_bst_len = 0 ; // 0:bst_4, 1:bst_8
        uint32_t cas_wr, tphy_wd = 0, mrinfo_wr_lat;    // cas write latency,cas_wr_t,
        uint32_t cas_rd, tphy_rd = 0, mrinfo_rd_lat;    // cas read latency, cas_rd_t,crl_srt, 
        uint32_t cas_add, mrinfo_add_lat; //add_lat,
        uint32_t dram_emr2 = 0, dram_mr0 = 0;
        uint32_t cr_twr, dram_max_wr, dram_wr;
        uint32_t cr_trtw, cr_trtw_t = 0;
        uint32_t dram_period;//, dram_periodx2;
        enum dram_type         ddr_type = DDR_1;
        enum dram_dq_width     dq_width;
        enum dram_page_size    page;
        uint32_t dfi_rate;
        unsigned char bond = BIT_GET_CHIP_ID_CODE(REG32(REG_BOND_OPTION));
        uint8_t temp_val = read_efuse_byte_iram(OFFSE_EFUSE_PHYSICAL_OPTIONS_0x78);
        uint32_t one_bank_col_bit = BIT_GET_EFUSE_DRAM_BIT_SHIFT(temp_val);
        
        dfi_rate = 1 << (uint32_t) (dram_info->dfi_rate);
        dram_period = (dram_info-> ddr_period_ps)*(dfi_rate); // according DFI_RATE to setting


        //_memctl_debug_printf("dfi_rate=%d,dram_period=%d\n",dfi_rate,dram_period);

        cas_wr = (dram_info-> mode_reg-> dram_wr_lat ) + 
                   (dram_info-> mode_reg-> dram_par_lat )+
                   (dram_info-> mode_reg-> dram_add_lat );

        cas_rd = (dram_info-> mode_reg-> dram_rd_lat ) + 
                   (dram_info-> mode_reg-> dram_par_lat )+
                   (dram_info-> mode_reg-> dram_add_lat );

        cas_add = (dram_info-> mode_reg-> dram_add_lat );


        // In PHY, write latency == 3
        dram_max_wr= (dram_info-> timing -> wr_max_tck)/(dfi_rate) +1;
        dram_wr = ((dram_info-> timing -> twr_ps) / dram_period)+1;


        if ((dram_info-> dev-> device_type) == DDR_3) {
                cr_twr = ((dram_info-> timing -> twr_ps) / dram_period) + 1;
        }
        else { // SDR, DDR1
                cr_twr = ((dram_info-> timing -> twr_ps) / dram_period);
                //_memctl_debug_printf("\nSDR D1  dram_info-> dev-> device_type=%d ,cr_twr=%d \n",dram_info-> dev-> device_type,cr_twr );
        }

        if (cr_twr < dram_max_wr) {
                cr_twr = cr_twr;
        }
        else {
                cr_twr = dram_max_wr;
        }

        if ((dram_info-> dev-> device_type) == DDR_2) {
                //_memctl_debug_printf("\nDDR2  dram_info-> dev-> device_type=%d\n",dram_info-> dev-> device_type);

                ddr_type = DDR_2;
                if (dram_info-> mode_reg-> bst_len == BST_LEN_4) {
                        cr_bst_len = 0; //bst_4
                        cr_trtw_t = 2+2; //4/2+2
                        dram_mr0 = 0x2;
                           //_memctl_debug_printf("\nDDR2  dram_info-> mode_reg-> bst_len=%d\n",dram_info-> mode_reg-> bst_len);
                }
                else { // BST_LEN_8
                        cr_bst_len = 1; // bst_8
                        cr_trtw_t = 4+2; // 8/2+2
                        dram_mr0 = 0x3;
                        //_memctl_debug_printf("\nDDR2  dram_info-> mode_reg-> bst_len=%d\n",dram_info-> mode_reg-> bst_len);
                }
                if (dram_info-> mode_reg-> dram_rd_lat <= PHY_READ_LATENCY) {
                        tphy_rd = 1;
                }
                else {
                        if(dram_info->dfi_rate == DFI_RATIO_1)
                                tphy_rd = (dram_info-> mode_reg-> dram_rd_lat + PHY_ADDR_CMD_LATENCY - 11) / (dfi_rate);
                        else
                                tphy_rd = (dram_info-> mode_reg-> dram_rd_lat - PHY_READ_LATENCY) / (dfi_rate) - 1;
                }
                            
                if(dram_info->dfi_rate == DFI_RATIO_1)
                    tphy_wd = (dram_info-> mode_reg-> dram_wr_lat + (PHY_ADDR_CMD_LATENCY - PHY_WRITE_DATA_LATENCY))/(dfi_rate);
                else
                    tphy_wd = (dram_info-> mode_reg-> dram_wr_lat + (PHY_ADDR_CMD_LATENCY - PHY_WRITE_DATA_LATENCY))/(dfi_rate) + 1;
                        
                //cas_rd = dram_info-> mode_reg -> mode0_cas;
                //add_lat = dram_info-> mode_reg ->  mode1_all_lat;
                //cas_wr = cas_rd + add_lat -1;
                dram_emr2 = 0;

                if(dram_info-> mode_reg-> mode0_cas == 8)
                {
                        dram_mr0  =(((dram_wr%7)-1)                  << (PCTL_MR_OP_BFO+1)) | // write_recovery
                               (0                                << PCTL_MR_OP_BFO    ) | // dll
                               (0x1 << PCTL_MR_CAS_BFO   ) |
                               (dram_info-> mode_reg-> bst_type  << PCTL_MR_BT_BFO    ) |
                               dram_mr0;                      
                }
                else if(dram_info-> mode_reg-> mode0_cas == 9)
                {
                        dram_mr0  =(((dram_wr%8)-1)                  << (PCTL_MR_OP_BFO+1)) | // write_recovery
                               (0                                << PCTL_MR_OP_BFO    ) | // dll
                               (0<< PCTL_MR_CAS_BFO   ) |
                               (dram_info-> mode_reg-> bst_type  << PCTL_MR_BT_BFO    ) |
                               dram_mr0;                      
                }
                else
                {
                        dram_mr0  =(((dram_wr%6)-1)                  << (PCTL_MR_OP_BFO+1)) | // write_recovery
                               (0                                << PCTL_MR_OP_BFO    ) | // dll
                               (dram_info-> mode_reg-> mode0_cas << PCTL_MR_CAS_BFO   ) |
                               (dram_info-> mode_reg-> bst_type  << PCTL_MR_BT_BFO    ) |
                               dram_mr0;
                }

        } // DDR2
        else if ((dram_info-> dev-> device_type) == DDR_1) {
                //_memctl_debug_printf("\nDDR1-1   dram_info-> dev-> device_type=%d\n",dram_info-> dev-> device_type);

                REG_WRITE_U32(CMD_CTRL, 1<<2);
                ddr_type = DDR_1;
                if (dram_info-> mode_reg-> bst_len == BST_LEN_4) {
                        dram_mr0 = 2; // bst_4
                        cr_bst_len = 0; //bst_4
                }
                else { // BST_LEN_8
                        dram_mr0 = 3; // bst_8
                        cr_bst_len = 1; // bst_8
                }
                //cas_rd = dram_info-> mode_reg -> mode0_cas;
                if (dram_info-> mode_reg-> dram_rd_lat <= 1) {
                        tphy_rd = 1;
                }
                else {
                        tphy_rd = (dram_info-> mode_reg-> dram_rd_lat - 0) / (dfi_rate);
                }

                tphy_wd = 1 + 1;


                dram_mr0  =(cas_rd                          << PCTL_MR_CAS_BFO) |
                (dram_info-> mode_reg-> bst_type << PCTL_MR_BT_BFO ) |
                dram_mr0;

                cr_trtw_t = 0; // tic: cas_rd + rd_rtw + rd_pipe

                if (dfi_rate == 1) { // DFI_RATE== 2
                        cas_rd = 1;
                }
                else { // DFI_RATE == 1
                        cas_rd = cas_rd -1;
                }

                cas_wr = 1;

        } // ddr1

        if (dfi_rate == 1) {
                mrinfo_wr_lat = cas_wr;
                mrinfo_rd_lat = cas_rd;
                mrinfo_add_lat = (dram_info-> mode_reg-> dram_add_lat ); 
        }
        else {
                if (cas_wr%dfi_rate) {
                        mrinfo_wr_lat = (cas_wr+1)/ dfi_rate;
                }
                else {
                        mrinfo_wr_lat = cas_wr / dfi_rate;
                }

                if (cas_rd%dfi_rate) {
                        mrinfo_rd_lat = (cas_rd+1)/ dfi_rate;
                }
                else {
                        mrinfo_rd_lat = cas_rd / dfi_rate;
                }

                if (cas_add%dfi_rate) {
                        mrinfo_add_lat = (cas_add+1)/ dfi_rate;
                }
                else {
                        mrinfo_add_lat = cas_add / dfi_rate;
                }
        }


        // countting tRTW
        //if ((cr_trtw_t & 0x1)) {  // remove because of coverify
        //        cr_trtw = (cr_trtw_t+1) /(dfi_rate);
        //}
        //else 
        {
                cr_trtw = cr_trtw_t /(dfi_rate);
        }

        dq_width = (dram_info-> dev-> dq_width); 

        //_memctl_debug_printf("\nSDR dq_width=%d\n", dq_width);
        //page = dram_info-> dev-> page +1; // DQ16 -> memory:byte_unit *2
        page = dram_info-> dev-> page ; // DQ16 -> memory:byte_unit *1 // page size=1K


        //_memctl_debug_printf("\nDCR(0xf8142004)=0x%x\n", REG32(0xf8142004));
        if (dq_width == DQ_32) { // paralle dq_16 => page + 1
                page = page +1;
        }

        if (dq_width == HALF_DQ32) {
                dq_width = 2; // {half_dq, DQ32}
        }


        //_memctl_debug_printf("\nSDR page=%d\n", page);
        //_memctl_debug_printf("\nSDR dram_info-> dev-> bank =%d\n", dram_info-> dev-> bank );
        //_memctl_debug_printf("\ncr_bst_len =%d\n", cr_bst_len);
        
        dev_map->csr = (1 << PCTL_CSR_MEM_IDLE_BFO) | (1 << PCTL_CSR_BIST_IDLE_BFO);
        
        // WRAP_MISC setting
        dev_map-> misc = (
                    (page                   << WRAP_MISC_PAGE_SIZE_BFO) |  
                    (dram_info-> dev-> bank << WRAP_MISC_BANK_SIZE_BFO) |
                    (cr_bst_len             << WRAP_MISC_BST_SIZE_BFO )  
                   );
       

        //_memctl_debug_printf("\nSDR   dfi_rate=%d\n", dfi_rate);
        //_memctl_debug_printf("\nSDR  dq_width=%d\n", dq_width );
        //_memctl_debug_printf("\nddr_type =%d\n", ddr_type);

        // PCTL setting
        dev_map-> dcr = (
                   (dfi_rate << PCTL_DCR_DFI_RATE_BFO) |
                   (dq_width << PCTL_DCR_DQ32_BFO    ) |
                   (ddr_type << PCTL_DCR_DDR3_BFO    )
                  );


        //_memctl_debug_printf("\nSDR   cas_rd /(dfi_rate)=%d\n", cas_rd /(dfi_rate));
        //_memctl_debug_printf("\nSDR  cas_wr /(dfi_rate) =%d\n", cas_wr /(dfi_rate) );
        //_memctl_debug_printf("\nSDR  dev_map-> dcr =0x%x\n", dev_map-> dcr );
          

        dev_map-> iocr = (
                          (TRUNCATED(tphy_rd, 5)     << PCTL_IOCR_TPHY_RD_EN_BFO ) |
                          (0                        << PCTL_IOCR_TPHY_WL_BFO   ) |
                          (TRUNCATED(tphy_wd, 5)   << PCTL_IOCR_TPHY_WD_BFO   ) |
                          (0                        << PCTL_IOCR_RD_PIPE_BFO   ) |
                          (CS_2T<<PCTL_IOCR_2N_PRE_EN_BFO) |
                          (STATIC_CKE<<PCTL_IOCR_STC_CKE_BFO) |
                          (STATIC_ODT<<PCTL_IOCR_STC_ODT_BFO) |
                          (tRTW_2T_DIS<<PCTL_IOCR_TRTW_2T_DIS_BFO)
                         );


        //_memctl_debug_printf("\nSDR  dev_map-> iocr=0x%x\n", dev_map-> iocr );

        if (((dram_info-> dev-> device_type) != SDR) && 
        ((dram_info-> dev-> device_type) != DDR_1)) { // DDR2/3 
                dev_map-> emr2 = dram_emr2 | (0x1<<7); // add A7 bit for High temperature self refresh rate enable

                dev_map-> emr1 = (
                              ((dram_info->mode_reg->mode1_odt & 0x91) << 2 ) | //RTT
                              ((dram_info->mode_reg->mode1_ocd & 0x1)     << 1 ) | //D.I.C
                              (dram_info-> mode_reg -> mode1_dll_en_n )
                             );
        } // DDR2/3

        //_memctl_debug_printf("\ndev_map-> emr1=0x%x\n", dev_map->emr1);


        dev_map->emr0 = dram_mr0;

        //_memctl_debug_printf("\ndev_map-> mr=0x%x\n", dev_map->emr0);

        //_memctl_debug_printf("\nSoC dram_mr0(0xf8142020)=0x%x\n", REG32(0xf8142020));

        dev_map->mrinfo = (
                ((mrinfo_add_lat & 0x1f)                        << PCTL_MRINFO_ADD_LAT_BFO) |
                ((mrinfo_rd_lat  & 0x1f)                        << PCTL_MRINFO_RD_LAT_BFO ) |
                ((mrinfo_wr_lat  & 0x1f)                        << PCTL_MRINFO_WR_LAT_BFO )
        );



        dev_map-> drr = (
                   (0                                                 << PCTL_DRR_REF_DIS_BFO) |
                   (9                                                 << PCTL_DRR_REF_NUM_BFO) |
                   (TRUNCATED((((dram_info-> timing -> trefi_ps)/dram_period)+1), 16)<< PCTL_DRR_TREF_BFO   ) |
                   (TRUNCATED((((dram_info-> timing -> trfc_ps)/dram_period)+1), 8)<< PCTL_DRR_TRFC_BFO   )
                  );




        //_memctl_debug_printf("\nSDR  dev_map-> drr=0x%x\n", dev_map-> drr );

        dev_map-> tpr0= (
                         (!(STATIC_CKE) <<  23) |
                         (TRUNCATED((((dram_info-> timing -> tcke_tck)/dfi_rate)+1), 6)   << PCTL_TPR0_TCKE_BFO) |
                         (TRUNCATED(((dram_info-> timing -> trtp_tck/dfi_rate)+1), 4)   << PCTL_TPR0_TRTP_BFO) |
                         (TRUNCATED(cr_twr, 4)                                            << PCTL_TPR0_TWR_BFO ) |
                         (TRUNCATED((((dram_info-> timing -> tras_ps)/dram_period)+1), 5)<< PCTL_TPR0_TRAS_BFO) |
                         (TRUNCATED((((dram_info-> timing -> trp_ps)/dram_period)+1), 4)  << PCTL_TPR0_TRP_BFO )
                        );



        //_memctl_debug_printf("\nSDR  dev_map-> tpr0=0x%x\n", dev_map-> tpr0);


        dev_map-> tpr1= (
                   (TRUNCATED((((dram_info-> timing -> tfaw_ps)/dram_period)+1), 5) << PCTL_TPR1_TFAW_BFO) | 
                   (TRUNCATED(cr_trtw, 4)                                           << PCTL_TPR1_TRTW_BFO) |
                   (TRUNCATED((((dram_info-> timing -> twtr_tck)/dfi_rate)+1), 3)  << PCTL_TPR1_TWTR_BFO) |
                   (TRUNCATED((((dram_info-> timing -> tccd_tck)/dfi_rate)+1), 3)   << PCTL_TPR1_TCCD_BFO) |
                   (TRUNCATED((((dram_info-> timing -> trcd_ps)/dram_period)+1), 4) << PCTL_TPR1_TRCD_BFO) |
                   (TRUNCATED((((dram_info-> timing -> trc_ps)/dram_period)+1), 6)  << PCTL_TPR1_TRC_BFO ) |
                   (TRUNCATED(((dram_info-> timing -> trrd_tck/dfi_rate)+1), 4)       << PCTL_TPR1_TRRD_BFO)
                  );
        //_memctl_debug_printf("\nSDR  dev_map-> tpr1=0x%x\n", dev_map-> tpr1);

        dev_map-> tpr2= (
                   (TRUNCATED((dram_info-> timing -> tmrd_tck/dfi_rate + 1), 4)<< PCTL_TPR2_TMRD_BFO        ) |
                   (2                              << PCTL_TPR2_TNS_BFO  ) |
                   (2                              << PCTL_TPR2_INIT_REF_NUM_BFO)
                  );
        //_memctl_debug_printf("\nSDR  dev_map-> tpr2=0x%x\n", dev_map-> tpr2);

        if(one_bank_col_bit == 0x0)
        {
                // set all_mode _idle
                dev_map-> csr = 0x700;

                dev_map-> ccr = 0x0;
                _memctl_debug_printf("\nDRAM init disable\n");
                // start to init
                dev_map-> ccr = 0x1;
                _memctl_debug_printf("\nDRAM init enable\n");

                //set scheduler 
                dram_scheduler_timeout(RD2_TIMEOUT0, 0x64, SCHEDULER_TIMEOUT_CYCLE_MODE);
                
                unsigned int INIT_cnt=0;

                while(1)
                {
                        if (((dev_map-> ccr)& 0x1) == 0x0)
                        {
                                udelay (10);    /* Delay 10ns  */
                                INIT_cnt++;
                                _memctl_debug_printf("\nDRAM init is still active\n");
                                if (INIT_cnt>100)
                                        break;
                        }
                        else
                        {
                                _memctl_debug_printf("\nDRAM init is done , jump to DRAM\n");
                                break;
                        }
                }

                //if (IS_FB_BONDING(bond) || (pefuse_data->init_dev_val & BIT_EFUSE_DRAM_ENABLE_ODT)) {
                if ((dram_info-> dev-> device_type) == DDR_2) {
                        //enable DRAM ODT 
                        _memctl_debug_printf("enable DRAM ODT \n");
                        dev_map->tdpin = 0x7;
                        dev_map->ccr = 0x8;
                }
                //}

           
                // enter mem_mode
                dev_map-> csr= 0x600;
                _memctl_debug_printf("\nSDR init done dev_map=0x%x\n",(unsigned int)dev_map );
        }
        else
        {
                //disable schduler
                dram_disable_scheduler();
                
                //set bank/col size for one bank ddr
                if(one_bank_col_bit == 0x1)
                {
                        page = PAGE_1K;  // TODO : it should be PAGE_1K
                }
                else
                {
                        page = PAGE_2K;
                }
                REG_WRITE_U32(REG_DDR_PLL_15, (REG_READ_U32(REG_DDR_PLL_15) & (~(0xf<<BIT_SHIFT_CFG_ONE_BANK)))
                        | (one_bank_col_bit << BIT_SHIFT_CFG_ONE_BANK)); // TODO it should be one_bank_col_bit, not (one_bank_col_bit+1)

                // WRAP_MISC setting
                dev_map-> misc = (
                            (1<<30) |
                            (page                   << WRAP_MISC_PAGE_SIZE_BFO) |  
                            (0 << WRAP_MISC_BANK_SIZE_BFO) |
                            (cr_bst_len             << WRAP_MISC_BST_SIZE_BFO )  
                           );

                
                uint32_t dram_emr1 = (
                             ((dram_info->mode_reg->mode1_odt & 0x11) << 2 ) | //RTT
                             ((dram_info->mode_reg->mode1_ocd & 0x1)     << 1 ) | //D.I.C
                             (dram_info-> mode_reg -> mode1_dll_en_n )
                            );
                uint32_t dram_emr3 = 0;
                dram_init_flow(dev_map, dram_mr0, dram_emr1,dram_emr2, dram_emr3);
                _memctl_debug_printf("\nDDR init FLOW DONE\n");
                // update timing
                // dev_map->ccr= 0x80000000;

                // enter mem_mode
                dev_map->csr= 0x600;
        }
}
#endif

