/*
 * Copyright (C) 2010-2011 Samsung Electronics Co., Ltd.
 *
 * S3C series device definition for USB-GADGET
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <mach/map.h>
#include <plat/devs.h>
#include <plat/usbgadget.h>
#include <plat/usb-phy.h>

#ifdef CONFIG_USB_GADGET
/* USB Device (Gadget)*/
static struct resource s3c_usbgadget_resource[] = {
	[0] = {
		.start = S5P_PA_HSOTG,
		.end   = S5P_PA_HSOTG + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_USB_HSOTG,
		.end   = IRQ_USB_HSOTG,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s5p_device_usb_gadget_dmamask = 0xffffffffUL;
struct platform_device s3c_device_usbgadget = {
	.name		= "s3c-usbgadget",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3c_usbgadget_resource),
	.resource	= s3c_usbgadget_resource,
	.dev		= {
		.dma_mask		= &s5p_device_usb_gadget_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};

void __init s5p_usbgadget_set_platdata(struct s5p_usbgadget_platdata *pd)
{
	struct s5p_usbgadget_platdata *npd;

	npd = s3c_set_platdata(pd, sizeof(struct s5p_usbgadget_platdata),
			&s3c_device_usbgadget);

	if (!npd->phy_init)
		npd->phy_init = s5p_usb_phy_init;
	if (!npd->phy_exit)
		npd->phy_exit = s5p_usb_phy_exit;
}
#endif /* CONFIG_USB_GADGET */
