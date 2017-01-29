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

static struct class_attribute axppower_class_attrs[] = {
	__ATTR(vbuslimit,S_IRUGO|S_IWUSR,vbuslimit_show,vbuslimit_store),
	__ATTR(axpdebug,S_IRUGO|S_IWUSR,axpdebug_show,axpdebug_store),
	__ATTR(regdebug,S_IRUGO|S_IWUSR,axp_regdebug_show,axp_regdebug_store),
	__ATTR(out_factory_mode,S_IRUGO|S_IWUSR,out_factory_mode_show,out_factory_mode_store),
	__ATTR_NULL
};

struct class axppower_class = {
	.name = "axppower",
	.class_attrs = axppower_class_attrs,
};

