/*
 * Realtek Semiconductor Corp.
 *
 * bsp/usb.c:
 *     bsp USB initialization code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include "bspchip.h"
#include "usb.h"


/* USB Host Controller */

static struct resource bsp_usb_usb3_resource[] = {
	[0] = {
		.start = BSP_USB_USB3_MAPBASE,
		.end = BSP_USB_USB3_MAPBASE + BSP_USB_USB3_MAPSIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BSP_PS_USB_IRQ_USB3,
		.end = BSP_PS_USB_IRQ_USB3,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource bsp_usb_otg_resource[] = {
	[0] = {
		.start = BSP_USB_OTG_MAPBASE,
		.end = BSP_USB_OTG_MAPBASE + BSP_USB_OTG_MAPSIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BSP_PS_USB_IRQ_OTG,
		.end = BSP_PS_USB_IRQ_OTG,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 bsp_usb_dmamask = 0xFFFFFFFFUL;

struct platform_device bsp_usb_usb3_device = {
	.name = "dwc_usb3",
	.id = -1,
	.num_resources = ARRAY_SIZE(bsp_usb_usb3_resource),
	.resource = bsp_usb_usb3_resource,
	.dev = {
		.dma_mask = &bsp_usb_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

struct platform_device bsp_usb_otg_device = {
	.name = "dwc_otg",
	.id = -1,
	.num_resources = ARRAY_SIZE(bsp_usb_otg_resource),
	.resource = bsp_usb_otg_resource,
	.dev = {
		.dma_mask = &bsp_usb_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

static struct platform_device *bsp_usb_devs[] __initdata = {
	&bsp_usb_usb3_device,
	&bsp_usb_otg_device,
};

static int __init bsp_usb_init(void)
{
	int ret;

	printk("INFO: initializing USB devices ...\n");

	/* core init */

#if 0
	/* force device mode */
	REG32(OTG_GUSBCFG) |= (0x1 << 30) | (0x1 << 3);
	mdelay(25);
#endif

#if 0
	/* force host mode */
	REG32(OTG_GUSBCFG) |= (0x1 << 29) | (0x1 << 3);
	mdelay(25);
#endif

#if 0
	/* clear pending interrupt */
	REG32(OTG_GOTGINT) = 0xFFFFFFFF;
	REG32(OTG_GINTSTS) = 0xFFFFFFFF;

	REG32(OTG_GAHBCFG) |= (0x1 <<  5) | (0x1 << 0);
	REG32(OTG_GUSBCFG) |= (0x1 << 29) | (0x1 << 3);

	/* disable HNP SRP */
	REG32(OTG_GUSBCFG) &= ~(0x1 <<  8) | (0x1 << 9);
#endif

#if 0
	//unmask all channel int
	REG32(OTG_HAINTMSK) = 0xffffffff;
	REG32(OTG_HCINTMSK(CONTROL_CHANNEL)) = 0xffffffff;
	REG32(OTG_HCINTMSK(INTERRUPT_CHANNEL)) = 0xffffffff;
	REG32(OTG_HCINTMSK(IN_CHANNEL)) = 0xffffffff;
	REG32(OTG_HCINTMSK(OUT_CHANNEL)) = 0xffffffff;
#endif

	ret = platform_add_devices(bsp_usb_devs, ARRAY_SIZE(bsp_usb_devs));
	if (ret < 0) {
		printk("ERROR: unable to add devices\n");
		return ret;
	}

#if 0
	/* otg interrupt and mode mismatch interrupt */
	REG32(OTG_GINTMSK) = OTG_GINTMSK_OTGIntMsk | OTG_GINTMSK_ModeMisMsk;
   
	/* 1.GINTMSK.PrtInt to unmask */
	REG32(OTG_GINTMSK) |= OTG_GINTMSK_PrtIntMsk;
	REG32(OTG_GINTMSK) |= OTG_GINTMSK_HChIntMsk;

	/* 3.Program the HPRT.PrtPwr bit, This drives VBUS on the USB. */
	REG32(OTG_HPRT) |= OTG_HPRT_PrtPwr;

	/* clear pending interrupt */
	REG32(OTG_GOTGINT) = 0xFFFFFFFF;
	REG32(OTG_GINTSTS) = 0xFFFFFFFF;
#endif
    
	REG32(BSP_ICTL_MASK) &= ~(1 << (BSP_PS_USB_IRQ_USB3 - BSP_IRQ_ICTL_BASE));
	//REG32(BSP_ICTL_MASK) &= ~(1 << (BSP_PS_USB_IRQ_OTG - BSP_IRQ_ICTL_BASE));
	return 0;
}
arch_initcall(bsp_usb_init);
