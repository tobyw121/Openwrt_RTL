/*
 * Realtek Semiconductor Corp.
 *
 * bsp/bspchip.h:
 *     bsp chip address and IRQ mapping file
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#ifndef _BSPCHIP_H_
#define _BSPCHIP_H_

#define PROM_DEBUG      0


#ifdef CONFIG_SDK_FPGA_PLATFORM
#define MHZ             34
#else
#define MHZ             200
#endif
#define SYSCLK          MHZ * 1000 * 1000

#define BAUDRATE        115200  /* ex. 19200 or 38400 or 57600 or 115200 */                                /* For Early Debug */

/*
 * Register access macro
 */
#define REG32(reg)		(*(volatile unsigned int   *)(reg))
#define REG16(reg)		(*(volatile unsigned short *)(reg))
#define REG08(reg)		(*(volatile unsigned char  *)(reg))
#define REG8(reg)    (*(volatile unsigned char *)((unsigned int)reg))


#define SZ_256K			0x00040000

/*
 * SPRAM
 */
#define BSP_ISPRAM_BASE		0x0
#define BSP_DSPRAM_BASE		0x0

/*
 * IRQ Controller
 */
#define BSP_IRQ_CPU_BASE	0
#define BSP_IRQ_CPU_NUM		8

#define BSP_IRQ_ICTL_BASE	(BSP_IRQ_CPU_BASE + BSP_IRQ_CPU_NUM)
#define BSP_IRQ_ICTL_NUM	32

/*
 * MIPS32R2 counter
 */
#define BSP_COMPARE_IRQ		(BSP_IRQ_CPU_BASE + 2)
#define BSP_PERFCOUNT_IRQ	(BSP_IRQ_CPU_BASE + 3)

/*
 *  ICTL
 */
#define ICTL_OFFSET(irq)	((irq) - BSP_IRQ_ICTL_BASE)

#define BSP_ICTL_BASE		0xb8003000UL
#define BSP_ICTL1_IRQ		(BSP_IRQ_CPU_BASE + 2)
#define BSP_ICTL2_IRQ		(BSP_IRQ_CPU_BASE + 3)
#define BSP_ICTL3_IRQ		(BSP_IRQ_CPU_BASE + 4)
#define BSP_ICTL4_IRQ		(BSP_IRQ_CPU_BASE + 5)
#define BSP_ICTL5_IRQ		(BSP_IRQ_CPU_BASE + 6)
#define GIMR			(BSP_ICTL_BASE + 0x0)
   #define UART0_IE		(1 << ICTL_OFFSET(BSP_UART0_EXT_IRQ))
   #define UART1_IE		(1 << ICTL_OFFSET(BSP_UART1_EXT_IRQ))
   #define TC0_IE		(1 << ICTL_OFFSET(BSP_TC0_EXT_IRQ))
   #define TC1_IE		(1 << ICTL_OFFSET(BSP_TC1_EXT_IRQ))
   #define OCPTO_IE		(1 << ICTL_OFFSET(BSP_OCPTO_EXT_IRQ))
   #define HLXTO_IE		(1 << ICTL_OFFSET(BSP_HLX_EXT_IRQ))
   #define SLXTO_IE		(1 << ICTL_OFFSET(BSP_SLX_EXT_IRQ))
   #define NIC_IE		(1 << ICTL_OFFSET(BSP_NIC_EXT_IRQ))
   #define GPIO_ABCD_IE	(1 << ICTL_OFFSET(BSP_GPIO_ABCD_EXT_IRQ))
   #define GPIO_EFGH_IE	(1 << ICTL_OFFSET(BSP_GPIO_EFGH_EXT_IRQ))
   #define RTC_IE		(1 << ICTL_OFFSET(BSP_RTC_EXT_IRQ))
   #define WDT_IP1_IE	(1 << ICTL_OFFSET(BSP_WDT_IP1_IRQ))
   #define WDT_IP2_IE	(1 << ICTL_OFFSET(BSP_WDT_IP2_IRQ))

#define GISR			(BSP_ICTL_BASE + 0x4)
   #define UART0_IP		(1 << ICTL_OFFSET(BSP_UART0_EXT_IRQ))
   #define UART1_IP		(1 << ICTL_OFFSET(BSP_UART1_EXT_IRQ))
   #define TC0_IP		(1 << ICTL_OFFSET(BSP_TC0_EXT_IRQ))
   #define TC1_IP		(1 << ICTL_OFFSET(BSP_TC1_EXT_IRQ))
   #define OCPTO_IP		(1 << ICTL_OFFSET(BSP_OCPTO_EXT_IRQ))
   #define HLXTO_IP		(1 << ICTL_OFFSET(BSP_HLXTO_EXT_IRQ))
   #define SLXTO_IP		(1 << ICTL_OFFSET(BSP_SLXTO_EXT_IRQ))
   #define NIC_IP		(1 << ICTL_OFFSET(BSP_NIC_EXT_IRQ))
   #define GPIO_ABCD_IP	(1 << ICTL_OFFSET(BSP_GPIO_ABCD_EXT_IRQ))
   #define GPIO_EFGH_IP	(1 << ICTL_OFFSET(BSP_GPIO_EFGH_EXT_IRQ))
   #define RTC_IP		(1 << ICTL_OFFSET(BSP_RTC_EXT_IRQ - BSP))
   #define WDT_IP1_IP	(1 << ICTL_OFFSET(BSP_WDT_IP1_IRQ))
   #define WDT_IP2_IP	(1 << ICTL_OFFSET(BSP_WDT_IP2_IRQ))

#define IRR0			(BSP_ICTL_BASE + 0x8)
#define IRR0_SETTING		((UART0_RS  << 28) | \
				 (UART1_RS  << 24) | \
				 (TC0_RS    << 20) | \
				 (TC1_RS    << 16) | \
				 (OCPTO_RS  << 12) | \
				 (HLXTO_RS  << 8)  | \
				 (SLXTO_RS  << 4)  | \
				 (NIC_RS    << 0)    \
				)

#define IRR1			(BSP_ICTL_BASE + 0xc)

#define IRR1_8328_SETTING	((GPIO_ABCD_RS << 28) | \
				 (GPIO_EFGH_RS << 24) | \
				 (RTC_RS       << 20) | \
				 (SWCORE_RS    << 16)   \
				)

#define IRR1_SETTING		((GPIO_ABCD_RS << 28) | \
				 (SWCORE_RS    << 16) |  \
				 (WDT_IP1_RS << 12) \
				)

#define IRR2			(BSP_ICTL_BASE + 0x10)
#define IRR2_SETTING		0

#define IRR3			(BSP_ICTL_BASE + 0x14)
#define IRR3_SETTING		0

/*
 * Interrupt IRQ Assignments
 */
#define BSP_UART0_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 31)
#define BSP_UART1_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 30)
#define BSP_TC0_EXT_IRQ		(BSP_IRQ_ICTL_BASE + 29)
#define BSP_TC1_EXT_IRQ		(BSP_IRQ_ICTL_BASE + 28)
#define BSP_OCPTO_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 27)
#define BSP_HLXTO_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 26)
#define BSP_SLXTO_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 25)
#define BSP_NIC_EXT_IRQ		(BSP_IRQ_ICTL_BASE + 24)
#define BSP_GPIO_ABCD_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 23)
#define BSP_GPIO_EFGH_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 22)
#define BSP_RTC_EXT_IRQ		(BSP_IRQ_ICTL_BASE + 21)
#define	BSP_SWCORE_EXT_IRQ	(BSP_IRQ_ICTL_BASE + 20)
#define	BSP_WDT_IP1_IRQ	    (BSP_IRQ_ICTL_BASE + 19)
#define	BSP_WDT_IP2_IRQ	    (BSP_IRQ_ICTL_BASE + 18)

/* Interrupt Routing Selection */
#define UART0_RS		2
#define UART1_RS		1
#define TC0_RS			5
#define TC1_RS			1
#define OCPTO_RS		1
#define HLXTO_RS		1
#define SLXTO_RS		1
#define NIC_RS			4
#define GPIO_ABCD_RS	4
#define GPIO_EFGH_RS	4
#define RTC_RS			4
#define	SWCORE_RS		3
#define WDT_IP1_RS      4
#define WDT_IP2_RS      5

/* * Interrupt IRQ Assignments */
#define UART0_IRQ       31
#define UART1_IRQ       30
#define TC0_IRQ         29
#define TC1_IRQ         28
#define OCPTO_IRQ       27
#define HLXTO_IRQ       26
#define SLXTO_IRQ       25
#define NIC_IRQ         24
#define GPIO_ABCD_IRQ   23
#define GPIO_EFGH_IRQ   22
#define RTC_IRQ         21
#define	SWCORE_IRQ	    20
#define WDT_IP1_IRQ     19
#define WDT_IP2_IRQ     18

/*
 *  UART 
 */
#define BSP_UART0_BASE		0xb8002000UL
#define BSP_UART0_BAUD		38400  /* ex. 19200 or 38400 or 57600 or 115200 */  
#define BSP_UART0_FREQ		(SYSCLK - BSP_UART0_BAUD * 24)
#define BSP_UART0_MAPBASE	0x18002000UL
#define BSP_UART0_MAPSIZE	0x100
#define BSP_UART0_IRQ		BSP_UART0_EXT_IRQ

#define BSP_UART1_BASE		0xb8002100UL
#define BSP_UART1_BAUD		38400  /* ex. 19200 or 38400 or 57600 or 115200 */  
#define BSP_UART1_FREQ		(SYSCLK - BSP_UART0_BAUD * 24)
#define BSP_UART1_MAPBASE	0x18002100UL
#define BSP_UART1_MAPSIZE	0x100
#define BSP_UART1_IRQ		BSP_UART1_EXT_IRQ

#define UART0_RBR		(BSP_UART0_BASE + 0x000)
#define UART0_THR		(BSP_UART0_BASE + 0x000)
#define UART0_DLL		(BSP_UART0_BASE + 0x000)
#define UART0_IER		(BSP_UART0_BASE + 0x004)
#define UART0_DLM		(BSP_UART0_BASE + 0x004)
#define UART0_IIR		(BSP_UART0_BASE + 0x008)
#define UART0_FCR		(BSP_UART0_BASE + 0x008)
#define UART0_LCR		(BSP_UART0_BASE + 0x00C)
#define UART0_MCR		(BSP_UART0_BASE + 0x010)
#define UART0_LSR		(BSP_UART0_BASE + 0x014)

#define UART1_RBR		(BSP_UART1_BASE + 0x000)
#define UART1_THR		(BSP_UART1_BASE + 0x000)
#define UART1_DLL		(BSP_UART1_BASE + 0x000)
#define UART1_IER		(BSP_UART1_BASE + 0x004)
#define UART1_DLM		(BSP_UART1_BASE + 0x004)
#define UART1_IIR		(BSP_UART1_BASE + 0x008)
#define UART1_FCR		(BSP_UART1_BASE + 0x008)
   #define FCR_EN		0x01
   #define FCR_RXRST		0x02
   #define     RXRST		0x02
   #define FCR_TXRST		0x04
   #define     TXRST		0x04
   #define FCR_DMA		0x08
   #define FCR_RTRG		0xC0
   #define     CHAR_TRIGGER_01	0x00
   #define     CHAR_TRIGGER_04	0x40
   #define     CHAR_TRIGGER_08	0x80
   #define     CHAR_TRIGGER_14	0xC0
#define UART1_LCR		(BSP_UART1_BASE + 0x00C)
   #define LCR_WLN		0x03
   #define     CHAR_LEN_5	0x00
   #define     CHAR_LEN_6	0x01
   #define     CHAR_LEN_7	0x02
   #define     CHAR_LEN_8	0x03
   #define LCR_STB		0x04
   #define     ONE_STOP		0x00
   #define     TWO_STOP		0x04
   #define LCR_PEN		0x08
   #define     PARITY_ENABLE	0x01
   #define     PARITY_DISABLE	0x00
   #define LCR_EPS		0x30
   #define     PARITY_ODD	0x00
   #define     PARITY_EVEN	0x10
   #define     PARITY_MARK	0x20
   #define     PARITY_SPACE	0x30
   #define LCR_BRK		0x40
   #define LCR_DLAB		0x80
   #define     DLAB		0x80
#define UART1_MCR		(BSP_UART1_BASE + 0x010)
#define UART1_LSR		(BSP_UART1_BASE + 0x014)
   #define LSR_DR		0x01
   #define     RxCHAR_AVAIL	0x01
   #define LSR_OE		0x02
   #define LSR_PE		0x04
   #define LSR_FE		0x08
   #define LSR_BI		0x10
   #define LSR_THRE		0x20
   #define     TxCHAR_AVAIL	0x00
   #define     TxCHAR_EMPTY	0x20
   #define LSR_TEMT		0x40
   #define LSR_RFE		0x80

/*
 *  Timer/counter for 8390/80/28 TC & MP chip
 */
#define BSP_TIMER0_BASE		0xb8003100UL
#define BSP_TIMER0_IRQ		BSP_TC0_EXT_IRQ
#define TC0DATA			(BSP_TIMER0_BASE + 0x00)
#define TC1DATA			(BSP_TIMER0_BASE + 0x04)
   #define TCD_OFFSET		8
#define TC0CNT			(BSP_TIMER0_BASE + 0x08)
#define TC1CNT			(BSP_TIMER0_BASE + 0x0C)
#define TCCNR			(BSP_TIMER0_BASE + 0x10)
   #define TC0EN		(1 << 31)
   #define TC0MODE_TIMER	(1 << 30)
   #define TC1EN		(1 << 29)
   #define TC1MODE_TIMER	(1 << 28)
#define TCIR			(BSP_TIMER0_BASE + 0x14)
   #define TC0IE		(1 << 31)
   #define TC1IE		(1 << 30)
   #define TC0IP		(1 << 29)
   #define TC1IP		(1 << 28)
#define CDBR			(BSP_TIMER0_BASE + 0x18)
   #define DIVF_OFFSET		16
#define WDTCNR			(BSP_TIMER0_BASE + 0x1C)
#define RTL8390TC_TC1DATA			(BSP_TIMER0_BASE + 0x04)
   #define RTL8390TC_TCD_OFFSET		8
#define RTL8390TC_TC0CNT			(BSP_TIMER0_BASE + 0x08)
#define RTL8390TC_TC1CNT			(BSP_TIMER0_BASE + 0x0C)
#define RTL8390TC_TCCNR			(BSP_TIMER0_BASE + 0x10)
   #define RTL8390TC_TC0EN		(1 << 31)
   #define RTL8390TC_TC0MODE_TIMER	(1 << 30)
   #define RTL8390TC_TC1EN		(1 << 29)
   #define RTL8390TC_TC1MODE_TIMER	(1 << 28)
#define RTL8390TC_TCIR			(BSP_TIMER0_BASE + 0x14)
   #define RTL8390TC_TC0IE		(1 << 31)
   #define RTL8390TC_TC1IE		(1 << 30)
   #define RTL8390TC_TC0IP		(1 << 29)
   #define RTL8390TC_TC1IP		(1 << 28)
#define RTL8390TC_CDBR			(BSP_TIMER0_BASE + 0x18)
   #define RTL8390TC_DIVF_OFFSET		16
#define RTL8390TC_WDTCNR			(BSP_TIMER0_BASE + 0x1C)

#define RTL8390MP_TC1DATA          (BSP_TIMER0_BASE + 0x10)
#define RTL8390MP_TC0CNT           (BSP_TIMER0_BASE + 0x04)
#define RTL8390MP_TC1CNT           (BSP_TIMER0_BASE + 0x14)
#define RTL8390MP_TC0CTL           (BSP_TIMER0_BASE + 0x08)
#define RTL8390MP_TC1CTL           (BSP_TIMER0_BASE + 0x18)
   #define RTL8390MP_TCEN          (1 << 28)
   #define RTL8390MP_TCMODE_TIMER  (1 << 24)
   #define RTL8390MP_TCDIV_FACTOR  (0xFFFF << 0)
#define RTL8390MP_TC0INT           (BSP_TIMER0_BASE + 0xC)
#define RTL8390MP_TC1INT           (BSP_TIMER0_BASE + 0x1C)
   #define RTL8390MP_TCIE          (1 << 20)
   #define RTL8390MP_TCIP          (1 << 16)
#define RTL8390MP_WDTCNR          (BSP_TIMER0_BASE + 0x50)

#define RTL8380TC_TC1DATA			(BSP_TIMER0_BASE + 0x04)
   #define RTL8380TC_TCD_OFFSET		8
#define RTL8380TC_TC0CNT			(BSP_TIMER0_BASE + 0x08)
#define RTL8380TC_TC1CNT			(BSP_TIMER0_BASE + 0x0C)
#define RTL8380TC_TCCNR			(BSP_TIMER0_BASE + 0x10)
   #define RTL8380TC_TC0EN		(1 << 31)
   #define RTL8380TC_TC0MODE_TIMER	(1 << 30)
   #define RTL8380TC_TC1EN		(1 << 29)
   #define RTL8380TC_TC1MODE_TIMER	(1 << 28)
#define RTL8380TC_TCIR			(BSP_TIMER0_BASE + 0x14)
   #define RTL8380TC_TC0IE		(1 << 31)
   #define RTL8380TC_TC1IE		(1 << 30)
   #define RTL8380TC_TC0IP		(1 << 29)
   #define RTL8380TC_TC1IP		(1 << 28)
#define RTL8380TC_CDBR			(BSP_TIMER0_BASE + 0x18)
   #define RTL8380TC_DIVF_OFFSET		16
#define RTL8380TC_WDTCNR			(BSP_TIMER0_BASE + 0x1C)

#define RTL8380MP_TC1DATA          (BSP_TIMER0_BASE + 0x10)
#define RTL8380MP_TC0CNT           (BSP_TIMER0_BASE + 0x04)
#define RTL8380MP_TC1CNT           (BSP_TIMER0_BASE + 0x14)
#define RTL8380MP_TC0CTL           (BSP_TIMER0_BASE + 0x08)
#define RTL8380MP_TC1CTL           (BSP_TIMER0_BASE + 0x18)
   #define RTL8380MP_TCEN          (1 << 28)
   #define RTL8380MP_TCMODE_TIMER  (1 << 24)
   #define RTL8380MP_TCDIV_FACTOR  (0xFFFF << 0)
#define RTL8380MP_TC0INT           (BSP_TIMER0_BASE + 0xC)
#define RTL8380MP_TC1INT           (BSP_TIMER0_BASE + 0x1C)
   #define RTL8380MP_TCIE          (1 << 20)
   #define RTL8380MP_TCIP          (1 << 16)
#define RTL8380MP_WDTCNR          (BSP_TIMER0_BASE + 0x50)

#define DIVISOR_RTL8390			55
#define DIVISOR_RTL8380			1000
#define DIVISOR			1000

#if DIVISOR_RTL8390 > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif
#if DIVISOR_RTL8380 > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif
#if DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 * Memory Controller
 */
#define MC_MCR			0xB8001000
   #define MC_MCR_VAL		0x00000000

#define MC_DCR			0xB8001004
   #define MC_DCR0_VAL		0x54480000

#define MC_DTCR			0xB8001008
   #define MC_DTCR_VAL		0xFFFF05C0

/*
 * Reset
 */
#define	RGCR			0xBB001E70

#if defined(CONFIG_RTL8380_SERIES)
/* flash controller disable 4-byte address mode */
#define OTTO838x_FLASH_DISABLE_4BYTE_ADDR_MODE() \
    ({ \
        *((volatile unsigned int *)(0xBB000058)) = 0x3; \
        switch(*((volatile unsigned int *)(0xBB0000D0))){ \
            case 0x0:\
                break; \
            case 0x2:\
            default: \
				tmp = *((volatile unsigned int *)(0xBB000FF8));\
				tmp &= ~(0xC0000000);\
				tmp |= (0x8FFFFFFF);\
				*((volatile unsigned int *)(0xBB000FF8)) = tmp;\
                break; \
        }\
        *((volatile unsigned int *)(0xBB000058)) = 0x0; \
    })
#endif


#endif   /* _BSPCHIP_H */
