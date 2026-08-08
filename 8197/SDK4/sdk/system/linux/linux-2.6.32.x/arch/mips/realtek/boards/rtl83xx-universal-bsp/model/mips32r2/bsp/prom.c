/*
 * Realtek Semiconductor Corp.
 *
 * bsp/prom.c
 *     bsp early initialization code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/page.h>
#include <asm/cpu.h>

#include "bspcpu.h"
#include "bspchip.h"
#include "chip.h"


extern char arcs_cmdline[];

unsigned int bsp_chip_id, bsp_chip_rev_id;
unsigned int bsp_chip_family_id, bsp_chip_type;

extern int bsp_drv_swcore_cid_get(unsigned int unit, unsigned int *pCid, unsigned int *pCrevid);


#if defined(CONFIG_RTL8328_SERIES)
void prom_console_init(void)
{
   /* 8 bits, 1 stop bit, no parity. */
   REG8(UART0_LCR) = CHAR_LEN_8 | ONE_STOP | PARITY_DISABLE;

   /* Reset/Enable the FIFO */
   REG8(UART0_FCR) = FCR_EN | RXRST | TXRST | CHAR_TRIGGER_14;

   /* Disable All Interrupts */
   REG8(UART0_IER) = 0x00000000;

   /* Enable Divisor Latch */
   REG8(UART0_LCR) |= DLAB;

   /* Set Divisor */
   REG8(UART0_DLL) = (SYSCLK / (BAUDRATE * 16) - 1) & 0x00FF;
   REG8(UART0_DLM) = ((SYSCLK / (BAUDRATE * 16) - 1) & 0xFF00) >> 8;

   /* Disable Divisor Latch */
   REG8(UART0_LCR) &= (~DLAB);
}
#endif

#if defined(CONFIG_RTL8380_SERIES) || defined(CONFIG_RTL8390_SERIES)
void bsp_console_init(void)
{
		unsigned int value = 0;
		/* 8 bits, 1 stop bit, no parity. */
		REG32(UART0_LCR) = ((CHAR_LEN_8 | ONE_STOP | PARITY_DISABLE) << 24);
		/* Reset/Enable the FIFO */
		REG32(UART0_FCR) = ((FCR_EN | RXRST | TXRST | CHAR_TRIGGER_14) << 24);
		/* Disable All Interrupts */
		REG32(UART0_IER) = 0x00000000;
		/* Enable Divisor Latch */
		REG32(UART0_LCR) |= (DLAB << 24);
		/* Set Divisor */
		value = (SYSCLK / (BAUDRATE * 16) - 1) & 0x00FF;
		REG32(UART0_DLL) = (value << 24);

		value = (((SYSCLK / (BAUDRATE * 16) - 1) & 0xFF00) >> 8);
		REG32(UART0_DLM) = (value << 24);

		/* Disable Divisor Latch */
		REG32(UART0_LCR) &= ((~DLAB) << 24);

}
#endif
void __init prom_meminit(void)
{
	char *ptr;
	unsigned int memsize;

	/* Check the command line first for a memsize directive */
	ptr = strstr(arcs_cmdline, "mem=");

	if (ptr)
	   memsize = memparse(ptr + 4, &ptr);
	else
	   memsize = 0x02000000;  /* Default to 32MB */

	/*
	 * call <add_memory_region> to register boot_mem_map
	 * add_memory_region(base, size, type);
	 * type: BOOT_MEM_RAM, BOOT_MEM_ROM_DATA or BOOT_MEM_RESERVED
	 */
	add_memory_region(0, memsize, BOOT_MEM_RAM);
}


#ifdef CONFIG_EARLY_PRINTK
static int promcons_output __initdata = 0;

void unregister_prom_console(void)
{
	if (promcons_output)
		promcons_output = 0;
}

void disable_early_printk(void)
    __attribute__ ((alias("unregister_prom_console")));

void prom_putchar(char c)
{
}

char prom_getchar(void)
{
	return '\0';
}
#endif

const char *get_system_type(void)
{
	if(bsp_chip_family_id == RTL8390_FAMILY_ID)
		return "RTL8390";
	if(bsp_chip_family_id == RTL8380_FAMILY_ID)
		return "RTL8380";
	if(bsp_chip_family_id == RTL8328_FAMILY_ID)
		return "RTL8328";
}

void __init bsp_free_prom_memory(void)
{
	return;
}

/* Do basic initialization */
void __init bsp_init(void)
{
	int ret;

    int argc = fw_arg0;
    char **arg = (char **)fw_arg1;
    int i;

	bsp_chip_id = 0;
	bsp_chip_rev_id = 0;
	bsp_chip_family_id = 0;
	bsp_chip_type = 0;

	ret = bsp_drv_swcore_cid_get((unsigned int)0, (unsigned int *)&bsp_chip_id, (unsigned int *)&bsp_chip_rev_id);
	if(ret == -1)
	{
		printk("\nbsp_init(), RTK Switch chip is not found!!!\n");
	}else
	{
#if defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8390_FAMILY_ID)
		bsp_chip_family_id = RTL8390_FAMILY_ID;
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8350_FAMILY_ID)
		bsp_chip_family_id = RTL8350_FAMILY_ID;
#endif
#if defined(CONFIG_RTL8380_SERIES)
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8380_FAMILY_ID)
		bsp_chip_family_id = RTL8380_FAMILY_ID;
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8330_FAMILY_ID)
		bsp_chip_family_id = RTL8330_FAMILY_ID;
#endif
#if defined(CONFIG_RTL8328_SERIES)
	if((bsp_chip_id & RTL8328_FAMILY_ID) == RTL8328_FAMILY_ID)
		bsp_chip_family_id = RTL8328_FAMILY_ID;
#endif
	}

#if defined(CONFIG_RTL8328_SERIES)
	if(bsp_chip_family_id == RTL8328_FAMILY_ID)
		prom_console_init();
#endif
#if defined(CONFIG_RTL8380_SERIES) || defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_family_id == RTL8380_FAMILY_ID) || (bsp_chip_family_id == RTL8330_FAMILY_ID) || (bsp_chip_family_id == RTL8390_FAMILY_ID) || (bsp_chip_family_id == RTL8350_FAMILY_ID))
		bsp_console_init();
#endif
	prom_meminit();

    /* if user passes kernel args, ignore the default one */
    if (argc > 1)
        arcs_cmdline[0] = '\0';

    /* arg[0] is "g", the rest is boot parameters */
    for (i = 1; i < argc; i++) {
        if (strlen(arcs_cmdline) + strlen(arg[i] + 1)
            >= sizeof(arcs_cmdline))
            break;
        strcat(arcs_cmdline, arg[i]);
        strcat(arcs_cmdline, " ");
    }

}
