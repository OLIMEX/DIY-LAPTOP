/*
 * sunxi-nmi.c NMI driver
 *
 * Copyright (C) 2014-2015 allwinner.
 *	Ming Li<liming@allwinnertech.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/arisc/arisc.h>
#include "sunxi-nmi.h"

static u32 debug_mask = 0x0;
static nmi_struct *nmi_data;

void clear_nmi_status(void)
{
	arisc_clear_nmi_status();

	return;
}
EXPORT_SYMBOL(clear_nmi_status);


void enable_nmi(void)
{
	arisc_enable_nmi_irq();

	return;
}
EXPORT_SYMBOL(enable_nmi);

void disable_nmi(void)
{
	arisc_disable_nmi_irq();

	return;
}
EXPORT_SYMBOL(disable_nmi);

void set_nmi_trigger(u32 trigger)
{
	u32 tmp = 0;

	if (IRQF_TRIGGER_LOW==trigger)
		tmp = NMI_IRQ_LOW_LEVEL;
	else if (IRQF_TRIGGER_FALLING==trigger)
		tmp = NMI_IRQ_NE_EDGE;
	else if (IRQF_TRIGGER_HIGH==trigger)
		tmp = NMI_IRQ_HIGH_LEVEL;
	else if (IRQF_TRIGGER_RISING==trigger)
		tmp = NMI_IRQ_PO_EDGE;

	arisc_set_nmi_trigger(tmp);

	return;
}
EXPORT_SYMBOL(set_nmi_trigger);

static int sunxi_nmi_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct resource *mem_res = NULL;
	s32 ret;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	if (!of_device_is_available(node)) {
		printk("%s: nmi status disable!!\n", __func__);
		return -EPERM;
	}

	nmi_data = kzalloc(sizeof(*nmi_data), GFP_KERNEL);
	if (nmi_data == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res == NULL) {
		printk(KERN_ERR "%s: failed to get MEM res\n", __func__);
		ret = -ENXIO;
		goto mem_io_err;
	}

	if (!request_mem_region(mem_res->start, resource_size(mem_res), mem_res->name)) {
		printk(KERN_ERR  "%s: failed to request mem region\n", __func__);
		ret = -EINVAL;
		goto mem_io_err;
	}

	nmi_data->base_addr = ioremap(mem_res->start, resource_size(mem_res));
	if (!nmi_data->base_addr) {
		printk(KERN_ERR  "%s: failed to io remap\n", __func__);
		ret = -EIO;
		goto mem_io_err;
	}

	if (of_property_read_u32(node, "nmi_irq_ctrl", &nmi_data->nmi_irq_ctrl))
		nmi_data->nmi_irq_ctrl = 0xffffffff;

	if (of_property_read_u32(node, "nmi_irq_en", &nmi_data->nmi_irq_en))
		nmi_data->nmi_irq_en = 0xffffffff;

	if (of_property_read_u32(node, "nmi_irq_status", &nmi_data->nmi_irq_status))
		nmi_data->nmi_irq_status = 0xffffffff;

	if (of_property_read_u32(node, "nmi_irq_mask", &nmi_data->nmi_irq_mask))
		nmi_data->nmi_irq_mask = 0xffffffff;

	return 0;

mem_io_err:
	kfree(nmi_data);

	return ret;
}

static int sunxi_nmi_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "%s: module unloaded\n", __func__);

	return 0;
}


static const struct of_device_id sunxi_nmi_match[] = {
	 { .compatible = "allwinner,sunxi-nmi", },
	 {},
};
MODULE_DEVICE_TABLE(of, sunxi_nmi_match);

static struct platform_driver nmi_platform_driver = {
	.probe  = sunxi_nmi_probe,
	.remove = sunxi_nmi_remove,
	.driver = {
		.name	= NMI_MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_nmi_match,
	},
};

static int __init sunxi_nmi_init(void)
{
	return platform_driver_register(&nmi_platform_driver);
}

static void __exit sunxi_nmi_exit(void)
{
	platform_driver_unregister(&nmi_platform_driver);
}

arch_initcall(sunxi_nmi_init);
module_exit(sunxi_nmi_exit);
module_param_named(debug_mask, debug_mask, int, 0644);
MODULE_DESCRIPTION("sunxi nmi driver");
MODULE_AUTHOR("Ming Li<liming@allwinnertech.com>");
MODULE_LICENSE("GPL");

