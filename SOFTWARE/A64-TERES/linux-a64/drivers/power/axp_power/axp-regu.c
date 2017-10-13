/*
 * Regulators driver for allwinnertech AXP
 *
 * Copyright (C) 2014 allwinnertech Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "axp-regu.h"

static inline struct device *to_axp_dev(struct regulator_dev *rdev)
{
	return rdev_get_dev(rdev)->parent->parent;
}

static inline s32 check_range(struct axp_regulator_info *info,
				s32 min_uV, s32 max_uV)
{
	if (min_uV < info->min_uV || min_uV > info->max_uV)
		return -EINVAL;

	return 0;
}


/* AXP common operations */
static s32 axp_set_voltage(struct regulator_dev *rdev,
				  s32 min_uV, s32 max_uV,unsigned *selector)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	u8 val, mask;
	s32 ret = -1;

	if (check_range(info, min_uV, max_uV)) {
		pr_err("invalid voltage range (%d, %d) uV\n", min_uV, max_uV);
		return -EINVAL;
	}
	if ((info->switch_uV != 0) && (info->step2_uV!= 0) &&
	(info->new_level_uV != 0) && (min_uV > info->switch_uV)) {
		val = (info->switch_uV- info->min_uV + info->step1_uV - 1) / info->step1_uV;
		if (min_uV <= info->new_level_uV){
			val += 1;
		} else {
			val += (min_uV - info->new_level_uV) / info->step2_uV;
			val += 1;
		}
		mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	} else if ((info->switch_uV != 0) && (info->step2_uV!= 0) &&
	(min_uV > info->switch_uV) && (info->new_level_uV == 0)) {
		val = (info->switch_uV- info->min_uV + info->step1_uV - 1) / info->step1_uV;
		val += (min_uV - info->switch_uV) / info->step2_uV;
		mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	} else {
		val = (min_uV - info->min_uV + info->step1_uV - 1) / info->step1_uV;
		val <<= info->vol_shift;
		mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	}

	ret = axp_update(axp_dev, info->vol_reg, val, mask);
	if (ret)
		return ret;

	if (0 != info->dvm_enable_reg) {
		ret = axp_read(axp_dev, info->dvm_enable_reg, &val);
		if (ret) {
			printk("read dvm enable reg failed!\n");
			return ret;
		}

		if (val & (0x1<<info->dvm_enable_bit)) {
			/* wait dvm status update */
			udelay(100);
			do {
				ret = axp_read(axp_dev, info->vol_reg, &val);
				if (ret) {
					printk("read dvm status failed!\n");
					break;
				}
			} while (!(val & (0x1<<7)));
		}
	}

	return ret;
}

static s32 axp_get_voltage(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	u8 val, mask;
	s32 ret, switch_val, vol;

	ret = axp_read(axp_dev, info->vol_reg, &val);
	if (ret)
		return ret;

	mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	if (info->step1_uV != 0) {
		switch_val = ((info->switch_uV- info->min_uV + info->step1_uV - 1) / info->step1_uV);
	} else {
		switch_val = 0;
	}
	val = (val & mask) >> info->vol_shift;

	if ((info->switch_uV != 0) && (info->step2_uV!= 0) &&
	(val > switch_val) && (info->new_level_uV != 0)) {
		val -= switch_val;
		vol = info->new_level_uV + info->step2_uV * val;
	} else if ((info->switch_uV != 0) && (info->step2_uV!= 0) &&
	(val > switch_val) && (info->new_level_uV == 0)) {
		val -= switch_val;
		vol = info->switch_uV + info->step2_uV * val;
	} else {
		vol = info->min_uV + info->step1_uV * val;
	}

	return vol;
}

static s32 axp_enable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	return axp_set_bits(axp_dev, info->enable_reg,
					1 << info->enable_bit);
}

static s32 axp_disable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	return axp_clr_bits(axp_dev, info->enable_reg,
					1 << info->enable_bit);
}

static s32 axp_is_enabled(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	u8 reg_val;
	s32 ret;

	ret = axp_read(axp_dev, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	return !!(reg_val & (1 << info->enable_bit));
}

static s32 axp_list_voltage(struct regulator_dev *rdev, unsigned selector)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	s32 ret;

	ret = info->min_uV + info->step1_uV * selector;
	if ((info->switch_uV != 0) && (info->step2_uV != 0) &&
	(ret > info->switch_uV) && (info->new_level_uV != 0)) {
		selector -= ((info->switch_uV-info->min_uV)/info->step1_uV);
		ret = info->new_level_uV + info->step2_uV * selector;
	} else if ((info->switch_uV != 0) && (info->step2_uV != 0) &&
	(ret > info->switch_uV) && (info->new_level_uV == 0)) {
		selector -= ((info->switch_uV-info->min_uV)/info->step1_uV);
		ret = info->switch_uV + info->step2_uV * selector;
	}
	if (ret > info->max_uV)
		return -EINVAL;
	return ret;
}

static s32 axp_enable_time_regulator(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);

	/* Per-regulator power on delay from spec */
	if (40 > info->desc.id)
		return 400;
	else
		return 1200;
}

static s32 axp_set_suspend_voltage(struct regulator_dev *rdev, s32 uV)
{
	return axp_set_voltage(rdev, uV, uV,NULL);
}

static s32 axp_ldoio01_enable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	 axp_set_bits(axp_dev, info->enable_reg,0x03);
	 return axp_clr_bits(axp_dev, info->enable_reg,0x04);
}

static s32 axp_ldoio01_disable(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);

	return axp_clr_bits(axp_dev, info->enable_reg,0x07);
}

static s32 axp_ldoio01_is_enabled(struct regulator_dev *rdev)
{
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	u8 reg_val;
	s32 ret;

	ret = axp_read(axp_dev, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	return (((reg_val &= 0x07)== 0x03)?1:0);
}

static struct regulator_ops axp_ops = {
	.set_voltage	= axp_set_voltage,
	.get_voltage	= axp_get_voltage,
	.list_voltage	= axp_list_voltage,
	.enable		= axp_enable,
	.disable	= axp_disable,
	.is_enabled	= axp_is_enabled,
	.enable_time	= axp_enable_time_regulator,
	.set_suspend_enable		= axp_enable,
	.set_suspend_disable	= axp_disable,
	.set_suspend_voltage	= axp_set_suspend_voltage,
};

static struct regulator_ops axp_ldoio01_ops = {
	.set_voltage	= axp_set_voltage,
	.get_voltage	= axp_get_voltage,
	.list_voltage	= axp_list_voltage,
	.enable		= axp_ldoio01_enable,
	.disable	= axp_ldoio01_disable,
	.is_enabled	= axp_ldoio01_is_enabled,
	.enable_time	= axp_enable_time_regulator,
	.set_suspend_enable		= axp_ldoio01_enable,
	.set_suspend_disable	= axp_ldoio01_disable,
	.set_suspend_voltage	= axp_set_suspend_voltage,
};

static ssize_t workmode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	s32 ret;
	u8 val;

	ret = axp_read(axp_dev, info->mode_reg, &val);
	if (ret)
		return sprintf(buf, "IO ERROR\n");

	if(info->desc.id == 40) {
		switch (val & 0x01) {
			case 0:return sprintf(buf, "AUTO\n");
			case 1:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	} else if(info->desc.id == 41) {
		switch (val & 0x02) {
			case 0:return sprintf(buf, "AUTO\n");
			case 2:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	} else if(info->desc.id == 42) {
		switch (val & 0x04) {
			case 0:return sprintf(buf, "AUTO\n");
			case 4:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	} else if(info->desc.id == 43) {
		switch (val & 0x08) {
			case 0:return sprintf(buf, "AUTO\n");
			case 8:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	} else if(info->desc.id == 44) {
		switch (val & 0x10) {
			case 0:return sprintf(buf, "AUTO\n");
			case 16:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	} else if(info->desc.id == 45) {
		switch (val & 0x20) {
			case 0:return sprintf(buf, "AUTO\n");
			case 30:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	} else if(info->desc.id == 46) {
		switch (val & 0x40) {
			case 0:return sprintf(buf, "AUTO\n");
			case 64:return sprintf(buf, "PWM\n");
			default:return sprintf(buf, "UNKNOWN\n");
		}
	} else
		return sprintf(buf, "IO ID ERROR\n");
}

static ssize_t workmode_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	char mode;
	u8 val;

	if(  buf[0] > '0' && buf[0] < '9' )// 1/AUTO: auto mode; 2/PWM: pwm mode;
		mode = buf[0];
	else
		mode = buf[1];

	switch(mode){
	 case 'U':
	 case 'u':
	 case '1':
		val = 0;break;
	 case 'W':
	 case 'w':
	 case '2':
		val = 1;break;
	 default:
	    val =0;
	}

	if(info->desc.id == 40) {
		if(val)
			axp_set_bits(axp_dev, info->mode_reg, 0x01);
		else
			axp_clr_bits(axp_dev, info->mode_reg, 0x01);
	} else if(info->desc.id == 41) {
		if(val)
			axp_set_bits(axp_dev, info->mode_reg, 0x02);
		else
			axp_clr_bits(axp_dev, info->mode_reg, 0x02);
	} else if(info->desc.id == 42) {
		if(val)
			axp_set_bits(axp_dev, info->mode_reg, 0x04);
		else
			axp_clr_bits(axp_dev, info->mode_reg, 0x04);
	} else if(info->desc.id == 43) {
		if(val)
			axp_set_bits(axp_dev, info->mode_reg, 0x08);
		else
			axp_clr_bits(axp_dev, info->mode_reg, 0x08);
	} else if(info->desc.id == 44) {
		if(val)
			axp_set_bits(axp_dev, info->mode_reg, 0x10);
		else
			axp_clr_bits(axp_dev, info->mode_reg, 0x10);
	} else if(info->desc.id == 45) {
		if(val)
			axp_set_bits(axp_dev, info->mode_reg, 0x20);
		else
			axp_clr_bits(axp_dev, info->mode_reg, 0x20);
	} else if(info->desc.id == 46) {
		if(val)
			axp_set_bits(axp_dev, info->mode_reg, 0x40);
		else
			axp_clr_bits(axp_dev, info->mode_reg, 0x40);
	}
	return count;
}

static ssize_t frequency_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	s32 ret;
	u8 val;

	ret = axp_read(axp_dev, info->freq_reg, &val);
	if (ret)
		return ret;
	ret = val & 0x0F;
	return sprintf(buf, "%d\n",(ret*75 + 750));
}

static ssize_t frequency_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct axp_regulator_info *info = rdev_get_drvdata(rdev);
	struct device *axp_dev = to_axp_dev(rdev);
	u8 val;
	s32 var;

	var = simple_strtoul(buf, NULL, 10);
	if(var < 750)
		var = 750;
	if(var > 1875)
		var = 1875;

	val = (var -750)/75;
	val &= 0x0F;

	axp_update(axp_dev, info->freq_reg, val, 0x0F);

	return count;
}


static struct device_attribute axp_regu_attrs[] = {
	AXP_REGU_ATTR(workmode),
	AXP_REGU_ATTR(frequency),
};

static s32 axp_regu_create_attrs(struct platform_device *pdev)
{
	s32 j,ret;

	for (j = 0; j < ARRAY_SIZE(axp_regu_attrs); j++) {
		ret = device_create_file(&pdev->dev,&axp_regu_attrs[j]);
		if (ret)
			goto sysfs_failed;
	}
    goto succeed;

sysfs_failed:
	while (j--)
		device_remove_file(&pdev->dev,&axp_regu_attrs[j]);
succeed:
	return ret;
}

static s32  axp_regulator_probe(struct platform_device *pdev)
{
	struct regulator_dev *rdev;
	struct  axp_reg_init * platform_data = (struct  axp_reg_init *)(pdev->dev.platform_data);
	struct regulator_config config = { };
	struct axp_regulator_info *info = platform_data->info;
	s32 ret;

	if ((AXP_LDOIO_ID_START > info->desc.id) || (AXP_DCDC_ID_START <= info->desc.id))
		info->desc.ops = &axp_ops;
	if ((AXP_LDOIO_ID_START <= info->desc.id) && (AXP_DCDC_ID_START > info->desc.id))
		info->desc.ops = &axp_ldoio01_ops;

	config.dev = &pdev->dev;
	config.init_data = pdev->dev.platform_data;
	config.driver_data = info;
	rdev = regulator_register(&info->desc, &config);

	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "failed to register regulator %s\n",
				info->desc.name);
		return PTR_ERR(rdev);
	}
	platform_set_drvdata(pdev, rdev);

	if(AXP_DCDC_ID_START <= info->desc.id) {
		ret = axp_regu_create_attrs(pdev);
		if(ret)
			return ret;
	}

	return 0;
}

static s32 axp_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);

	regulator_unregister(rdev);
	return 0;
}

static struct platform_driver axp_regulator_driver = {
	.driver	= {
		.name	= "axp-regulator",
		.owner	= THIS_MODULE,
	},
	.probe		= axp_regulator_probe,
	.remove		= axp_regulator_remove,
};

static s32 __init axp_regulator_init(void)
{
	return platform_driver_register(&axp_regulator_driver);
}
subsys_initcall(axp_regulator_init);

static void __exit axp_regulator_exit(void)
{
	platform_driver_unregister(&axp_regulator_driver);
}
module_exit(axp_regulator_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ming Li");
MODULE_DESCRIPTION("Regulator Driver for allwinnertech PMIC");
MODULE_ALIAS("platform:axp-regulator");

