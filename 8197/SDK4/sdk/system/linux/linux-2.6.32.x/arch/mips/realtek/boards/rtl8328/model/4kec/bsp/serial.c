/*
 * Realtek Semiconductor Corp.
 *
 * bsp/serial.c
 *     BSP serial port initialization
 *
 * Copyright 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <linux/tty.h>
#include <linux/irq.h>

#include <asm/serial.h>
#include "bspchip.h"
#include <linux/serial_8250.h>
#include <linux/serial_core.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
unsigned int last_lcr;

unsigned int dwapb_serial_in(struct uart_port *p, int offset)
{
	offset <<= p->regshift;
	return readl(p->membase + offset);
}

void dwapb_serial_out(struct uart_port *p, int offset, int value)
{
	int save_offset = offset;
	offset <<= p->regshift;

	/* Save the LCR value so it can be re-written when a
	 * Busy Detect interrupt occurs. */
	if (save_offset == UART_LCR) {
		last_lcr = value;
	}
	writel(value, p->membase + offset);
	/* Read the IER to ensure any interrupt is cleared before
	 * returning from ISR. */
	if (save_offset == UART_TX || save_offset == UART_IER)
		value = p->serial_in(p, UART_IER);
}

static int dwapb_serial_irq(struct uart_port *p)
{
	unsigned int iir = readl(p->membase + (UART_IIR << p->regshift));

	if (serial8250_handle_irq(p, iir)) {
		return 1;
	} else if ((iir & UART_IIR_BUSY) == UART_IIR_BUSY) {
		/*
		 * The DesignWare APB UART has an Busy Detect (0x07) interrupt
		 * meaning an LCR write attempt occurred while the UART was
		 * busy. The interrupt must be cleared by reading the UART
		 * status register (USR) and the LCR re-written.
		 */
		(void)readl(p->membase + 0xc0);
		writel(last_lcr, p->membase + (UART_LCR << p->regshift));
		return 1;
	}

	return 0;
}
#endif

int __init bsp_serial_init(void)
{
#ifdef CONFIG_SERIAL_8250
		int ret;
		struct uart_port s;
	
		memset(&s, 0, sizeof(s));
	
		s.type = PORT_16550A;
		s.membase = (unsigned char *) BSP_UART0_BASE;
		s.irq = BSP_UART0_EXT_IRQ;
		s.uartclk = SYSCLK - BAUDRATE * 24;
		s.flags = UPF_SKIP_TEST | UPF_LOW_LATENCY | UPF_SPD_CUST;
		s.iotype = UPIO_MEM;
		s.regshift = 2;
		s.fifosize = 1;  
		s.custom_divisor = SYSCLK / (BAUDRATE * 16) - 1;
		/* Call early_serial_setup() here, to set up 8250 console driver */
		if (early_serial_setup(&s) != 0) {
			ret = 1;
		}
#endif
	return 0;
}
