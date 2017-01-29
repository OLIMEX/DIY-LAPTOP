/*
 * axp192 for standby driver
 *
 * Copyright (C) 2015 allwinnertech Ltd.
 * Author: Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "../standby.h"
#include "../standby_twi.h"
#include "axp152_power.h"

static struct axp_regulator_info axp152_regulator_info[] = {
	{AXP152_DCDC1EN,	7},//buck1
	{AXP152_DCDC2EN,	6},//buck2
	{AXP152_DCDC3EN,	5},//buck3
	{AXP152_DCDC4EN,	4},//buck4
	{AXP152_ANALOG1EN,	3},//ldo2 for analog1
	{AXP152_ANALOG2EN,	2},//ldo3 for analog2
	{AXP152_DIGITAL1EN,	1},//ldo2 for DigtalLDO1
	{AXP152_DIGITAL2EN,	0},//ldo3 for DigtalLDO2
	{AXP152_LDOI0EN,	1},//ldoio0
	{AXP152_LDO0EN,		7},//ldo0
	{AXP152_RTCLDOEN,	0},//ldo1 for rtc
};

static inline struct axp_regulator_info *find_info(u32 id)
{
	struct axp_regulator_info *ri = NULL;
	int i = 0;

	for(i=0; i<=AXP152_ID_MAX; i++) {
		if (id &(0x1 << i)) {
			ri = &axp152_regulator_info[i];
			break;
		}
	}

	return ri;
}

/* AXP common operations */
s32 axp152_set_volt(u32 id, u32 voltage)
{
	return 0;
}

s32 axp152_get_volt(u32 id)
{
	return 0;
}

s32 axp152_set_state(u32 id, u32 state)
{
	struct axp_regulator_info *info = NULL;
	s32 ret = -1;
	uint8_t reg_val;

	info = find_info(id);

	ret = twi_byte_rw(TWI_OP_RD, AXP152_ADDR, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	if (0 == state) {
		reg_val |= (1 << info->enable_bit);

	} else {
		reg_val &= ~(1 << info->enable_bit);
	}

	ret = twi_byte_rw(TWI_OP_WR, AXP152_ADDR, info->enable_reg, &reg_val);

	if (0 > ret)
		printk("axp152_set_state set failed\n");

	return ret;
}

s32 axp152_get_state(u32 id)
{
	struct axp_regulator_info *info = NULL;
	s32 ret = -1;
	uint8_t reg_val;

	info = find_info(id);

	ret = twi_byte_rw(TWI_OP_RD, AXP152_ADDR, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	return !!(reg_val & (1 << info->enable_bit));
}

s32 axp152_suspend(u32 id)
{
	struct axp_regulator_info *info = NULL;
	u32 i = 0;
	s32 ret = 0;
	uint8_t reg_val = 0;

	for(i=0; i<=AXP152_ID_MAX; i++) {
		if (id &(0x1 << i)) {
			info = find_info((0x1 << i));
			reg_val |= (1 << info->enable_bit);
		}
	}

	ret = twi_byte_rw(TWI_OP_WR, AXP152_ADDR, info->enable_reg, &reg_val);
	if (ret) {
		printk("axp152_suspend failed\n");
	}
	return ret;
}

