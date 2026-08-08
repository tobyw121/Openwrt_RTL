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

extern char arcs_cmdline[];

void __init prom_console_init(void)
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
	return "RTL8328";
}

void __init bsp_free_prom_memory(void)
{
	return;
}

/* Do basic initialization */
void __init bsp_init(void)
{
	
	prom_console_init();
	prom_meminit();
	
}
