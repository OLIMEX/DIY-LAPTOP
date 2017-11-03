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
#include "axp_power.h"
#include "axp192_power.h"

#define AXP19_LDO(_id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2,new_level, mode_addr, freq_addr)	\
	 AXP_LDO(AXP19, _id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2, new_level, mode_addr, freq_addr)

#define AXP19_DCDC(_id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2, new_level, mode_addr, freq_addr)	\
	AXP_DCDC(AXP19, _id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2, new_level, mode_addr, freq_addr)

static struct axp_regulator_info axp19_regulator_info[] = {
	 AXP19_DCDC(1,	  700,	  3500,   25,	  DCDC1,  0,	  7,	  DCDC1EN,0, 0, 0, 0, AXP19_DCDC_MODESET, AXP19_DCDC_FREQSET),
	 AXP19_DCDC(2,	  700,	  2275,   25,	  DCDC2,  0,	  6,	  DCDC2EN,0, 0, 0, 0, AXP19_DCDC_MODESET, AXP19_DCDC_FREQSET),
	 AXP19_DCDC(3,	  700,	  3500,   25,	  DCDC3,  0,	  7,	  DCDC3EN,1, 0, 0, 0, AXP19_DCDC_MODESET, AXP19_DCDC_FREQSET),
	 AXP19_LDO(1,	  3000,   3000,   0,	  LDO1,   0,	  0,	  LDO1EN, 0, 0, 0, 0, 0, 0),	    //ldo1 for rtc
	 AXP19_LDO(2,	  1800,   3300,   100,	  LDO2,   4,	  4,	  LDO2EN, 2, 0, 0, 0, 0, 0),	    //ldo2 for analog1
	 AXP19_LDO(3,	  700,	  3500,   25,	  LDO3,   0,	  7,	  LDO3EN, 2, 0, 0, 0, 0, 0),	    //ldo3 for digital
	 AXP19_LDO(4,	  1800,   3300,   100,	  LDO4,   0,	  4,	  LDO4EN, 3, 0, 0, 0, 0, 0),	    //ldo4 for analog2
};

static inline struct axp_regulator_info *find_info(u32 id)
{
	struct axp_regulator_info *ri = NULL;
	int i = 0;

	for(i=0; i<=AXP19_ID_MAX; i++) {
		if (id &(0x1 << i)) {
			ri = &axp19_regulator_info[i];
			break;
		}
	}

	return ri;
}

static inline s32 check_range(struct axp_regulator_info *info,
				u32 voltage)
{
	if (voltage < info->min_uV || voltage > info->max_uV)
		return -1;

	return 0;
}

/* AXP common operations */
s32 axp19x_set_volt(u32 id, u32 voltage)
{
	/*struct axp_regulator_info *info = NULL;
	uint8_t val, mask;
	uint8_t reg_val;
	s32 ret = -1;

	info = find_info(id);

	if (check_range(info, voltage)) {
		//pr_err("invalid voltage  %d, uV\n", voltage);
		return -1;
	}
	if ((info->switch_uV != 0) && (info->step2_uV!= 0) &&
	(info->new_level_uV != 0) && (voltage > info->switch_uV)) {
		val = (info->switch_uV- info->min_uV + info->step1_uV - 1) / info->step1_uV;
		if (voltage <= info->new_level_uV){
			val += 1;
		} else {
			val += (voltage - info->new_level_uV) / info->step2_uV;
			val += 1;
		}
		mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	} else if ((info->switch_uV != 0) && (info->step2_uV!= 0) &&
	(voltage > info->switch_uV) && (info->new_level_uV == 0)) {
		val = (info->switch_uV- info->min_uV + info->step1_uV - 1) / info->step1_uV;
		val += (voltage - info->switch_uV) / info->step2_uV;
		mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	} else {
		val = (voltage - info->min_uV + info->step1_uV - 1) / info->step1_uV;
		val <<= info->vol_shift;
		mask = ((1 << info->vol_nbits) - 1)  << info->vol_shift;
	}

	ret = twi_byte_rw(TWI_OP_RD, AXP192_ADDR, info->vol_reg, &reg_val);
	if (ret)
		return ret;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = twi_byte_rw(TWI_OP_WR, AXP192_ADDR, info->vol_reg, &reg_val);
	}

	return ret;*/
	return 0;
}

s32 axp19x_get_volt(u32 id)
{
	/*struct axp_regulator_info *info = NULL;
	uint8_t val, mask;
	s32 ret = -1, switch_val, vol;

	info = find_info(id);

	ret = twi_byte_rw(TWI_OP_RD, AXP192_ADDR, info->vol_reg, &val);
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

	return vol;*/
	return 0;
}

s32 axp19x_set_state(u32 id, u32 state)
{
	struct axp_regulator_info *info = NULL;
	s32 ret = -1;
	uint8_t reg_val;

	info = find_info(id);

	ret = twi_byte_rw(TWI_OP_RD, AXP192_ADDR, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	if (0 == state) {
		reg_val |= (1 << info->enable_bit);

	} else {
		reg_val &= ~(1 << info->enable_bit);
	}

	ret = twi_byte_rw(TWI_OP_WR, AXP192_ADDR, info->enable_reg, &reg_val);

	if (0 > ret) {
		printk("axp19x_set_state faied, state=%d, id=0x%x\n", state, id);
	}

	return ret;
}

s32 axp19x_get_state(u32 id)
{
	struct axp_regulator_info *info = NULL;
	s32 ret = -1;
	uint8_t reg_val;

	info = find_info(id);

	ret = twi_byte_rw(TWI_OP_RD, AXP192_ADDR, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	return !!(reg_val & (1 << info->enable_bit));
}

s32 axp19x_suspend(u32 id)
{
	struct axp_regulator_info *info = NULL;
	u32 i = 0;
	s32 ret = 0;
	uint8_t reg_val = 0;

	for(i=0; i<=AXP19_ID_MAX; i++) {
		if (id &(0x1 << i)) {
			info = find_info((0x1 << i));
			reg_val |= (1 << info->enable_bit);
		}
	}

	ret = twi_byte_rw(TWI_OP_WR, AXP192_ADDR, info->enable_reg, &reg_val);
	if (ret) {
		printk("axp19x_suspend faied \n" );
	}
	return ret;
}

