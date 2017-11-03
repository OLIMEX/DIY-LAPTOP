/*
 * Battery charger driver for X-POWERS AXP28X
 *
 * Copyright (C) 2014 X-POWERS Ltd.
 *  Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef CONFIG_AW_AXP81X
#include "axp81x-sply.h"
#include "axp81x-common.h"
#endif

s32 axp_debug = 0x0;
s32 vbus_curr_limit_debug = 1;
static s32 reg_debug = 0x0;

static ssize_t axpdebug_store(struct class *class,
			struct class_attribute *attr,	const char *buf, size_t count)
{
	s32 var;
	var = simple_strtoul(buf, NULL, 16);
	printk("%s: var=%d\n", __func__, var);
	if(var)
		axp_debug = var;
	else
		axp_debug = 0;
	return count;
}

static ssize_t axpdebug_show(struct class *class,
			struct class_attribute *attr,	char *buf)
{
	return sprintf(buf, "%x\n", axp_debug);
}

static ssize_t axp_regdebug_store(struct class *class,
			struct class_attribute *attr,	const char *buf, size_t count)
{
	s32 var;
	var = simple_strtoul(buf, NULL, 16);
	if(var)
		reg_debug = var;
	else
		reg_debug = 0;
	return count;
}

static ssize_t axp_regdebug_show(struct class *class,
			struct class_attribute *attr,	char *buf)
{
	return sprintf(buf, "%x\n", reg_debug);
}

void axp_reg_debug(s32 reg, s32 len, u8 *val)
{
	s32 i = 0;
	if (reg_debug != 0) {
		for (i=0; i<len; i++) {
			if (reg+i == reg_debug)
				printk(KERN_ERR "###***axp_reg 0x%x write value 0x%x\n", reg_debug, *(val+i));
		}
	}
	return;
}

static ssize_t vbuslimit_store(struct class *class,
			struct class_attribute *attr,	const char *buf, size_t count)
{
	if(buf[0] == '1')
		vbus_curr_limit_debug = 1;
	else
		vbus_curr_limit_debug = 0;
	return count;
}

static ssize_t vbuslimit_show(struct class *class,
			struct class_attribute *attr,	char *buf)
{
	return sprintf(buf, "%d\n", vbus_curr_limit_debug);
}

static ssize_t out_factory_mode_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	u8 addr = AXP_BUFFERC;
	u8 data;
	axp_read(axp_charger->master, addr , &data);
	return sprintf(buf, "0x%x\n",data);
}

static ssize_t out_factory_mode_store(struct class *class,
		struct class_attribute *attr, const char *buf, size_t count)
{
	u8 addr = AXP_BUFFERC;
	u8 data;
	s32 var;
	var = simple_strtoul(buf, NULL, 10);
	if(var){
	  data = 0x0d;
	  axp_write(axp_charger->master, addr , data);
	}
	else{
	  data = 0x00;
	  axp_write(axp_charger->master, addr , data);
	}
	return count;
}

static ssize_t axp_rdc_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	u8 temp_val[2];
	axp_reads(axp_charger->master, 0xba, 2, temp_val);
	return sprintf(buf, "%d\n", (((temp_val[0] & 0x1f) <<8) + temp_val[1])*10742/10000);
}

static ssize_t axp_batt_max_cap_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	u8 temp_val[2];
	axp_reads(axp_charger->master, 0xe0, 2, temp_val);
	return sprintf(buf, "%d\n", (((temp_val[0] & 0x7f) <<8) + temp_val[1])*1456/1000);
}

static ssize_t axp_coulumb_counter_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	u8 temp_val[2];
	axp_reads(axp_charger->master, 0xe2, 2, temp_val);
	return sprintf(buf, "%d\n", (((temp_val[0] & 0x7f) <<8) + temp_val[1])*1456/1000);
}

static ssize_t axp_power_sply_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	u64 power_sply = axp_charger->disvbat * axp_charger->disibat;
	return sprintf(buf, "%lld\n", power_sply);
}

static ssize_t axp_ic_temp_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->ic_temp);
}

static ssize_t axp_bat_temp_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->bat_temp);
}

static ssize_t axp_vbat_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->vbat);
}

static ssize_t axp_ibat_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->ibat);
}

static ssize_t axp_ocv_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->ocv);
}

static ssize_t axp_disvbat_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->disvbat);
}

static ssize_t axp_disibat_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->disibat);
}

static ssize_t axp_is_on_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->is_on);
}

static ssize_t axp_bat_current_direction_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->bat_current_direction);
}

static ssize_t axp_charge_on_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->charge_on);
}

static ssize_t axp_ext_valid_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", axp_charger->ext_valid);
}

static struct class_attribute axppower_class_attrs[] = {
	__ATTR(vbuslimit,S_IRUGO|S_IWUSR,vbuslimit_show,vbuslimit_store),
	__ATTR(axpdebug,S_IRUGO|S_IWUSR,axpdebug_show,axpdebug_store),
	__ATTR(regdebug,S_IRUGO|S_IWUSR,axp_regdebug_show,axp_regdebug_store),
	__ATTR(out_factory_mode,S_IRUGO|S_IWUSR,out_factory_mode_show,out_factory_mode_store),
	__ATTR(rdc_show,S_IRUGO,axp_rdc_show,NULL),
	__ATTR(batt_max_cap,S_IRUGO,axp_batt_max_cap_show,NULL),
	__ATTR(coulumb_counter,S_IRUGO,axp_coulumb_counter_show,NULL),
	__ATTR(power_sply,S_IRUGO,axp_power_sply_show,NULL),
	__ATTR(ic_temp,S_IRUGO,axp_ic_temp_show,NULL),
	__ATTR(bat_temp,S_IRUGO,axp_bat_temp_show,NULL),
	__ATTR(vbat,S_IRUGO,axp_vbat_show,NULL),
	__ATTR(ibat,S_IRUGO,axp_ibat_show,NULL),
	__ATTR(ocv,S_IRUGO,axp_ocv_show,NULL),
	__ATTR(disvbat,S_IRUGO,axp_disvbat_show,NULL),
	__ATTR(disibat,S_IRUGO,axp_disibat_show,NULL),
	__ATTR(is_on,S_IRUGO,axp_is_on_show,NULL),
	__ATTR(bat_current_direction,S_IRUGO,axp_bat_current_direction_show,NULL),
	__ATTR(charge_on,S_IRUGO,axp_charge_on_show,NULL),
	__ATTR(ext_valid,S_IRUGO,axp_ext_valid_show,NULL),
	__ATTR_NULL
};

struct class axppower_class = {
	.name = "axppower",
	.class_attrs = axppower_class_attrs,
};
