/*
 * Battery charger driver for allwinnertech AXP81X
 *
 * Copyright (C) 2014 ALLWINNERTECH.
 *  Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/power_supply.h>
#include <linux/apm_bios.h>
#include <linux/apm-emulation.h>
#include <linux/mfd/axp-mfd.h>
#include <linux/module.h>
#include "axp81x-mfd.h"
#include "axp-regu.h"
#include "axp81x-regu.h"
#include "axp-cfg.h"

struct axp_config_info axp81x_config;
static struct axp_dev axp81x_dev;

static struct power_supply_info battery_data ={
	.name ="PTI PL336078",
	.technology = POWER_SUPPLY_TECHNOLOGY_LiFe,
	.voltage_max_design = 4200000,
	.voltage_min_design = 3500000,
	.use_for_apm = 1,
};


static struct axp_supply_init_data axp_sply_init_data = {
	.battery_info = &battery_data,
	.chgcur = 1500000,
	.chgvol = 4200000,
	.chgend = 10,
	.chgen = 1,
	.sample_time = 800,
	.chgpretime = 50,
	.chgcsttime = 720,
};

static struct axp_funcdev_info axp_splydev[]={
	{
		.name = "axp81x-supplyer",
		.id = AXP81X_ID_SUPPLY,
		.platform_data = &axp_sply_init_data,
	},
};

static struct axp_platform_data axp_pdata = {
	.num_sply_devs = ARRAY_SIZE(axp_splydev),
	.sply_devs = axp_splydev,
};

static struct axp_mfd_chip_ops axp81x_ops[] = {
	[0] = {
		.init_chip    = axp81x_init_chip,
		.enable_irqs  = axp81x_enable_irqs,
		.disable_irqs = axp81x_disable_irqs,
		.read_irqs    = axp81x_read_irqs,
	},
};

#ifdef	CONFIG_AXP_TWI_USED
static const struct i2c_device_id axp_i2c_id_table[] = {
	{ "axp81x_board", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, axp_i2c_id_table);

static struct i2c_board_info __initdata axp_mfd_i2c_board_info[] = {
	{
		.type = "axp81x_board",
		.addr = 0x36,
	},
};

static irqreturn_t axp_mfd_irq_handler(int irq, void *data)
{
	struct axp_dev *chip = data;
	disable_irq_nosync(irq);
	(void)schedule_work(&chip->irq_work);

	return IRQ_HANDLED;
}

static s32 axp_i2c_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	s32 ret = 0;

	if (axp81x_config.pmu_used) {
		battery_data.voltage_max_design = axp81x_config.pmu_init_chgvol;
		battery_data.voltage_min_design = axp81x_config.pmu_pwroff_vol;
		battery_data.energy_full_design = axp81x_config.pmu_battery_cap;
		axp_sply_init_data.chgcur = axp81x_config.pmu_runtime_chgcur;
		axp_sply_init_data.chgvol = axp81x_config.pmu_init_chgvol;
		axp_sply_init_data.chgend = axp81x_config.pmu_init_chgend_rate;
		axp_sply_init_data.chgen = axp81x_config.pmu_init_chg_enabled;
		axp_sply_init_data.sample_time = axp81x_config.pmu_init_adc_freq;
		axp_sply_init_data.chgpretime = axp81x_config.pmu_init_chg_pretime;
		axp_sply_init_data.chgcsttime = axp81x_config.pmu_init_chg_csttime;
	}else {
		return -1;
	}

	axp81x_dev.client = client;
	axp81x_dev.dev = &client->dev;

	i2c_set_clientdata(client, &axp81x_dev);

	axp81x_dev.ops = &axp81x_ops[0];
	axp81x_dev.attrs = axp81x_mfd_attrs;
	axp81x_dev.attrs_number = ARRAY_SIZE(axp81x_mfd_attrs);
	axp81x_dev.pdata = &axp_pdata;
	ret = axp_register_mfd(&axp81x_dev);
	if (ret < 0) {
		printk("axp mfd register failed\n");
		return ret;
	}

	ret = request_irq(client->irq, axp_mfd_irq_handler,
		IRQF_SHARED|IRQF_DISABLED, "axp81x", &axp81x_dev);
	if (ret) {
		dev_err(&client->dev, "failed to request irq %d\n",
				client->irq);
		goto out_free_chip;
	}

	return ret;


out_free_chip:
	i2c_set_clientdata(client, NULL);
	return ret;
}

static s32 axp_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static struct i2c_driver axp_i2c_driver = {
	.driver	= {
		.name	= "axp81x_board",
		.owner	= THIS_MODULE,
	},
	.probe		= axp_i2c_probe,
	.remove		= axp_i2c_remove,
	.id_table	= axp_i2c_id_table,
};
#else
static s32  axp81x_platform_probe(struct platform_device *pdev)
{
	s32 ret = 0;

	if (axp81x_config.pmu_used) {
		battery_data.voltage_max_design = axp81x_config.pmu_init_chgvol;
		battery_data.voltage_min_design = axp81x_config.pmu_pwroff_vol;
		battery_data.energy_full_design = axp81x_config.pmu_battery_cap;
		axp_sply_init_data.chgcur = axp81x_config.pmu_runtime_chgcur;
		axp_sply_init_data.chgvol = axp81x_config.pmu_init_chgvol;
		axp_sply_init_data.chgend = axp81x_config.pmu_init_chgend_rate;
		axp_sply_init_data.chgen = axp81x_config.pmu_init_chg_enabled;
		axp_sply_init_data.sample_time = axp81x_config.pmu_init_adc_freq;
		axp_sply_init_data.chgpretime = axp81x_config.pmu_init_chg_pretime;
		axp_sply_init_data.chgcsttime = axp81x_config.pmu_init_chg_csttime;
	}else {
		return -1;
	}

	axp81x_dev.dev = &pdev->dev;
	dev_set_drvdata(axp81x_dev.dev, &axp81x_dev);
	axp81x_dev.client = (struct i2c_client *)pdev;

	axp81x_dev.ops = &axp81x_ops[0];
	axp81x_dev.attrs = axp81x_mfd_attrs;
	axp81x_dev.attrs_number = ARRAY_SIZE(axp81x_mfd_attrs);
	axp81x_dev.pdata = &axp_pdata;
	axp81x_dev.irq_number = axp81x_config.pmu_irq_id;
	ret = axp_register_mfd(&axp81x_dev);
	if (ret < 0) {
		printk("axp81x mfd register failed\n");
		return ret;
	}
	return 0;
}

static struct platform_driver axp81x_platform_driver = {
	.probe		= axp81x_platform_probe,
	.driver		= {
		.name	= "axp81x_board",
		.owner	= THIS_MODULE,
	},
};
#endif

static s32 __init axp81x_board_init(void)
{
	s32 ret = 0;
	struct axp_funcdev_info *axp_regu_info = NULL;

	ret = axp_device_tree_parse("pmu0", &axp81x_config);

	if (ret) {
		printk("%s parse sysconfig or device tree err\n", __func__);
		return -1;
	}

	axp_regu_info = axp81x_regu_init();
	if (NULL == axp_regu_info) {
		printk("%s fetch regu tree err\n", __func__);
		return -1;
	} else {
		axp_pdata.num_regl_devs = 23;//sizeof(*axp_regu_info)/sizeof(struct axp_funcdev_info);
		printk(KERN_ERR "%s: axp regl_devs num = %d\n", __func__, axp_pdata.num_regl_devs);
		axp_pdata.regl_devs = (struct axp_funcdev_info *)axp_regu_info;
		if (NULL == axp_pdata.regl_devs) {
			printk(KERN_ERR "%s: get regl_devs failed\n", __func__);
			return -1;
		}
	}

#ifdef	CONFIG_AXP_TWI_USED
	ret = i2c_add_driver(&axp_i2c_driver);
	if (ret < 0) {
		printk("axp_i2c_driver add failed\n");
		return ret;
	}

	axp_mfd_i2c_board_info[0].addr = axp81x_config.pmu_twi_addr;
	axp_mfd_i2c_board_info[0].irq = axp81x_config.pmu_irq_id;
	ret = i2c_register_board_info(axp81x_config.pmu_twi_id, axp_mfd_i2c_board_info, ARRAY_SIZE(axp_mfd_i2c_board_info));
	if (ret < 0) {
		printk("axp_i2c_board_info add failed\n");
		return ret;
	}
#else
	ret = platform_driver_register(&axp81x_platform_driver);
	if (IS_ERR_VALUE(ret)) {
		printk("register axp81x platform driver failed\n");
		return ret;
	}
#endif
        return ret;
}

arch_initcall(axp81x_board_init);

MODULE_DESCRIPTION("ALLWINNERTECH axp board");
MODULE_AUTHOR("Ming Li");
MODULE_LICENSE("GPL");

