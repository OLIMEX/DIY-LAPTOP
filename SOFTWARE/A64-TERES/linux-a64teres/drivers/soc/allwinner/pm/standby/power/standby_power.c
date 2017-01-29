/*
 * standby driver for allwinnertech
 *
 * Copyright (C) 2015 allwinnertech Ltd.
 * Author: Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "../standby.h"
#include "power.h"

static u32 dm_on;
static u32 dm_off;

/* power domain */
#define IS_DM_ON(dm)  ((dm_on >> dm) & 0x1)
#define IS_DM_OFF(dm) (!((dm_off >> dm) & 0x1))
#define POWER_VOL_OFF ( 0 )
#define POWER_VOL_ON  ( 1 )

static s32 volt_bak[VCC_MAX_INDEX];
static u32 power_regu_tree[VCC_MAX_INDEX];

static s32 pmu_get_voltage(u32 tree)
{
	unsigned int pmux_id = 0;
	s32 ret = -1;
	if (0 == tree) {
		printk("pmu_get_voltage:tree is 0\n");
		return ret;
	}

	pmux_id = (tree >> 28);
	tree &= 0x0fffffff;

	switch(pmux_id)
	{
	case AXP_19X_ID:
		ret = axp19x_get_volt(tree);
		break;
	case AXP_209_ID:
		break;
	case AXP_22X_ID:
		break;
	case AXP_806_ID:
		break;
	case AXP_808_ID:
		break;
	case AXP_809_ID:
		break;
	case AXP_803_ID:
		break;
	case AXP_813_ID:
		break;
	case AXP_152_ID:
		ret = axp152_get_volt(tree);
		break;
	default:
		printk("pmu_get_voltage :pmu id err, tree=0x%x\n", tree);
		return -1;
	}

	if (0 > ret)
		printk("pmu_get_voltage faied\n");

	return ret;
}

static s32 pmu_set_voltage(u32 tree, u32 voltage)
{
	u32 pmux_id = 0;
	s32 ret = -1;
	if (0 == tree) {
		printk("pmu_set_voltage: tree is 0\n");
		return ret;
	}

	pmux_id = (tree >> 28);
	tree &= 0x0fffffff;

	switch(pmux_id)
	{
	case AXP_19X_ID:
		ret = axp19x_set_volt(tree, voltage);
		break;
	case AXP_209_ID:
		break;
	case AXP_22X_ID:
		break;
	case AXP_806_ID:
		break;
	case AXP_808_ID:
		break;
	case AXP_809_ID:
		break;
	case AXP_803_ID:
		break;
	case AXP_813_ID:
		break;
	case AXP_152_ID:
		ret = axp152_set_volt(tree, voltage);
		break;
	default:
		printk("pmu_set_voltage :pmu id err, tree=0x%x\n", tree);
		return -1;
	}

	if (0 > ret)
		printk("pmu_set_voltage faied\n");

	return ret;
}

static s32 pmu_get_state(u32 tree)
{
	u32 pmux_id = 0;
	s32 ret = -1;
	if (0 == tree) {
		printk("pmu_get_state: tree is 0\n");
		return ret;
	}

	pmux_id = (tree >> 28);
	tree &= 0x0fffffff;

	switch(pmux_id)
	{
	case AXP_19X_ID:
		ret = axp19x_get_state(tree);
		break;
	case AXP_209_ID:
		break;
	case AXP_22X_ID:
		break;
	case AXP_806_ID:
		break;
	case AXP_808_ID:
		break;
	case AXP_809_ID:
		break;
	case AXP_803_ID:
		break;
	case AXP_813_ID:
		break;
	case AXP_152_ID:
		ret = axp152_get_state(tree);
		break;
	default:
		printk("pmu_get_state :pmu id err, tree=0x%x\n", tree);
		return -1;
	}

	if (0 > ret)
		printk("pmu_get_state faied\n");

	return ret;
}

static s32 pmu_set_state(u32 tree, u32 state)
{
	u32 pmux_id = 0;
	s32 ret = -1;
	if (0 == tree) {
		printk("pmu_set_state: tree is 0\n");
		return ret;
	}

	pmux_id = (tree >> 28);
	tree &= 0x0fffffff;

	switch(pmux_id)
	{
	case AXP_19X_ID:
		ret = axp19x_set_state(tree, state);
		break;
	case AXP_209_ID:
		break;
	case AXP_22X_ID:
		break;
	case AXP_806_ID:
		break;
	case AXP_808_ID:
		break;
	case AXP_809_ID:
		break;
	case AXP_803_ID:
		break;
	case AXP_813_ID:
		break;
	case AXP_152_ID:
		ret = axp152_set_state(tree, state);
		break;
	default:
		printk("pmu_set_state :pmu id err, tree=0x%x\n", tree);
		return -1;
	}

	if (0 > ret)
		printk("pmu_set_state faied\n");

	return ret;
}

static void pmu_suspend(u32 mask)
{
	u32 pmux_id = 0;
	s32 ret = -1;
	if (0 == mask) {
		printk("pmu_suspend: tree is 0\n");
		return ;
	}

	pmux_id = (mask >> 28);
	mask &= 0x0fffffff;

	switch(pmux_id)
	{
	case AXP_19X_ID:
		ret = axp19x_suspend(mask);
		break;
	case AXP_209_ID:
		break;
	case AXP_22X_ID:
		break;
	case AXP_806_ID:
		break;
	case AXP_808_ID:
		break;
	case AXP_809_ID:
		break;
	case AXP_803_ID:
		break;
	case AXP_813_ID:
		break;
	case AXP_152_ID:
		ret = axp152_suspend(mask);
		break;
	default:
		printk("pmu_suspend :pmu id err, tree=0x%x\n", mask);
		return ;
	}

	if (0 > ret)
		printk("pmu_suspend faied\n");

	return ;
}


void power_enter_super(struct aw_pm_info * config, extended_standby_t * extended_config)
{
	s32 dm;
	u32 close_mask = 0;

	if (extended_config->pmu_id) {
		standby_memcpy(&power_regu_tree, config->pmu_arg.soc_power_tree, sizeof(power_regu_tree));

		dm_on = extended_config->soc_pwr_dm_state.sys_mask & extended_config->soc_pwr_dm_state.state;
		dm_off = (~(extended_config->soc_pwr_dm_state.sys_mask) | extended_config->soc_pwr_dm_state.state) | dm_on;

		for (dm = VCC_MAX_INDEX - 1; dm >= 0; dm--) {
			if (IS_DM_OFF(dm)) {
				close_mask |= power_regu_tree[dm];
			}
		}

		pmu_suspend(close_mask);
	}
}


void dm_suspend(struct aw_pm_info * config, extended_standby_t * extended_config)
{
	/* one dm maybe have some output */
	s32 dm;

	if (extended_config->pmu_id) {
		standby_memcpy(&power_regu_tree, config->pmu_arg.soc_power_tree, sizeof(power_regu_tree));

		dm_on = extended_config->soc_pwr_dm_state.sys_mask & extended_config->soc_pwr_dm_state.state;
		dm_off = (~(extended_config->soc_pwr_dm_state.sys_mask) | extended_config->soc_pwr_dm_state.state) | dm_on;

		for (dm = VCC_MAX_INDEX - 1; dm >= 0; dm--) {
			if (IS_DM_ON(dm)) {
				if (extended_config->soc_pwr_dm_state.volt[dm] != 0) {
					volt_bak[dm] = pmu_get_voltage(power_regu_tree[dm]);
					if (0 > volt_bak[dm])
						printk("volt_bak[%d]=%d\n", dm, volt_bak[dm]);
					pmu_set_voltage(power_regu_tree[dm], extended_config->soc_pwr_dm_state.volt[dm]);
				}
			}
		}
		for (dm = VCC_MAX_INDEX - 1; dm >= 0; dm--) {
			if (IS_DM_OFF(dm)) {
				pmu_set_state(power_regu_tree[dm], POWER_VOL_OFF);
			}
		}
	}

	return;
}

void dm_resume(extended_standby_t * extended_config)
{
	s32 dm;

	if (extended_config->pmu_id) {
		for (dm = 0; dm < VCC_MAX_INDEX; dm++) {
			if (IS_DM_ON(dm)) {
				if (extended_config->soc_pwr_dm_state.volt[dm] != 0) {
					pmu_set_voltage(power_regu_tree[dm], volt_bak[dm]);
				}
			}
		}
		for (dm = 0; dm < VCC_MAX_INDEX; dm++) {
			if (IS_DM_OFF(dm)) {
				pmu_set_state(power_regu_tree[dm], POWER_VOL_ON);
			}
		}
	}
	return;
}

