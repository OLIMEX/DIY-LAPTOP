/*
 * reg-virtual-consumer.c
 *
 * Copyright 2008 X-POWERS Ltd.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/module.h>
#include "virtual.h"

static s32 regulator_virtual_consumer_probe(struct platform_device *pdev)
{
	char *reg_id = pdev->dev.platform_data;
	struct virtual_consumer_data *drvdata;
	s32 ret, i;

	drvdata = kzalloc(sizeof(struct virtual_consumer_data), GFP_KERNEL);
	if (drvdata == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	mutex_init(&drvdata->lock);

	drvdata->regulator = regulator_get(NULL, reg_id);
	if (IS_ERR(drvdata->regulator)) {
		ret = PTR_ERR(drvdata->regulator);
		goto err;
	}

	for (i = 0; i < ARRAY_SIZE(attributes_virtual); i++) {
		ret = device_create_file(&pdev->dev, attributes_virtual[i]);
		if (ret != 0)
			goto err;
	}

	drvdata->mode = regulator_get_mode(drvdata->regulator);

	platform_set_drvdata(pdev, drvdata);

	return 0;

err:
	for (i = 0; i < ARRAY_SIZE(attributes_virtual); i++)
		device_remove_file(&pdev->dev, attributes_virtual[i]);
	kfree(drvdata);
	return ret;
}

static s32 regulator_virtual_consumer_remove(struct platform_device *pdev)
{
	struct virtual_consumer_data *drvdata = platform_get_drvdata(pdev);
	s32 i;

	for (i = 0; i < ARRAY_SIZE(attributes_virtual); i++)
		device_remove_file(&pdev->dev, attributes_virtual[i]);
	if (drvdata->enabled)
		regulator_disable(drvdata->regulator);
	regulator_put(drvdata->regulator);

	kfree(drvdata);

	return 0;
}

static struct platform_driver regulator_virtual_consumer_driver[] = {
	{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-rtc",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-aldo1",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-aldo2",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-aldo3",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dldo1",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dldo2",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dldo3",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dldo4",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-eldo1",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-eldo2",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-eldo3",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-fldo1",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-fldo2",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dcdc1",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dcdc2",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dcdc3",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dcdc4",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dcdc5",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dcdc6",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-dcdc7",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-gpio0ldo",
		},
	},{
		.probe		= regulator_virtual_consumer_probe,
		.remove		= regulator_virtual_consumer_remove,
		.driver		= {
			.name		= "reg-81x-cs-gpio1ldo",
		},
	},
};


static s32 __init regulator_virtual_consumer_init(void)
{
	s32 j,ret;

	for (j = 0; j < ARRAY_SIZE(regulator_virtual_consumer_driver); j++){
		ret =  platform_driver_register(&regulator_virtual_consumer_driver[j]);
		if (ret)
			goto creat_drivers_failed;
	}
	return ret;

creat_drivers_failed:
	while (j--)
		platform_driver_unregister(&regulator_virtual_consumer_driver[j]);
	return ret;
}
module_init(regulator_virtual_consumer_init);

static void __exit regulator_virtual_consumer_exit(void)
{
	s32 j;

	for (j = ARRAY_SIZE(regulator_virtual_consumer_driver) - 1; j >= 0; j--){
			platform_driver_unregister(&regulator_virtual_consumer_driver[j]);
	}
}
module_exit(regulator_virtual_consumer_exit);

MODULE_AUTHOR("Weijin Zhong X-POWERS");
MODULE_DESCRIPTION("Virtual regulator consumer");
MODULE_LICENSE("GPL");

