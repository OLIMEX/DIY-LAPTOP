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
#include "standby.h"
#include "main.h"

static struct aw_pm_info pm_info;
static extended_standby_t extended_standby_para_info;
static struct pll_factor_t orig_pll;
static struct pll_factor_t local_pll;
static struct standby_clk_div_t  clk_div;
static struct standby_clk_div_t  tmp_clk_div;

static int dram_enter_selfresh(extended_standby_t *para);
static int dram_exit_selfresh(void);
static int cpu_enter_lowfreq(void);
static int cpu_freq_resume(void);
static int bus_enter_lowfreq(extended_standby_t *para);
static int bus_freq_resume(extended_standby_t *para);
static void query_wakeup_source(struct aw_pm_info *arg);

int standby_main(struct aw_pm_info *arg)
{
	save_mem_status(STANDBY_START | 0X01);
	/* copy standby parameter from dram */
	standby_memcpy(&pm_info, arg, sizeof(pm_info));

	/* copy extended standby info */
	if(0 != pm_info.standby_para.pextended_standby) {
		standby_memcpy(&extended_standby_para_info, (void *)(pm_info.standby_para.pextended_standby), sizeof(extended_standby_para_info));
	}

	mem_clk_init(1);
	/* init uart for print */
	if(unlikely(pm_info.standby_para.debug_mask&PM_STANDBY_PRINT_STANDBY)){
		serial_init_manager();
	}

	save_mem_status(STANDBY_START | 0X02);
	/* enable dram enter into self-refresh */
	dram_enter_selfresh(&extended_standby_para_info);

	save_mem_status(STANDBY_START | 0X03);
	/* cpu reduce frequency */
	cpu_enter_lowfreq();

	save_mem_status(STANDBY_START | 0X04);
	/* power domain suspend */
#ifdef CONFIG_AW_AXP
	standby_twi_init(pm_info.pmu_arg.twi_port);
	if (SUPER_STANDBY_FLAG == extended_standby_para_info.id)
		power_enter_super(&pm_info, &extended_standby_para_info);
	dm_suspend(&pm_info, &extended_standby_para_info);
#endif
	printk("test printk...\n");

	save_mem_status(STANDBY_START | 0X05);
	/* bus reduce frequency */
	bus_enter_lowfreq(&extended_standby_para_info);

	/* cpu enter sleep, wait wakeup by interrupt */
	asm("WFI");

	/* bus freq resume */
	bus_freq_resume(&extended_standby_para_info);

	save_mem_status(RESUME0_START | 0X01);
	/* cpu freq resume */
	cpu_freq_resume();

	save_mem_status(RESUME0_START | 0X02);
	/* power domain resume */
#ifdef CONFIG_AW_AXP
	dm_resume(&extended_standby_para_info);
	standby_twi_exit();
#endif

	save_mem_status(RESUME0_START | 0X03);
	/* dram out self-refresh */
	dram_exit_selfresh();

	save_mem_status(RESUME0_START | 0X04);
	if(unlikely(pm_info.standby_para.debug_mask&PM_STANDBY_PRINT_STANDBY)){
		serial_exit_manager();
	}
	return 0;
}


static int dram_enter_selfresh(extended_standby_t *para)
{
	s32 ret = -1;

	return ret;
}

static int dram_exit_selfresh(void)
{
	s32 ret = -1;

	return ret;
}

static int cpu_enter_lowfreq(void)
{
	standby_clk_init();
	 /* backup cpu freq */
	standby_clk_get_pll_factor(&orig_pll);
	/* backup bus src */
	standby_clk_bus_src_backup();

	/*lower freq from 1008M to 408M*/
	local_pll.FactorN = 16;
	local_pll.FactorK = 0;
	local_pll.FactorM = 0;
	local_pll.FactorP = 0;
	standby_clk_set_pll_factor(&local_pll);

	delay_ms(10);

	/* switch cpu clock to HOSC, and disable pll */
	standby_clk_core2hosc();
	delay_us(1);

	return 0;
}

static int cpu_freq_resume(void)
{
	 /* switch cpu clock to core pll */
	standby_clk_core2pll();
	change_runtime_env();
	delay_ms(10);

	/*restore freq from 384 to 1008M*/
	standby_clk_set_pll_factor(&orig_pll);
	change_runtime_env();
	delay_ms(5);

	return 0;
}

static int bus_enter_lowfreq(extended_standby_t *para)
{
	/* change ahb src to axi? losc?*/
	standby_clk_bus_src_set();

	standby_clk_getdiv(&clk_div);
	/* set clock division cpu:axi:ahb:apb = 2:2:2:1 */
	tmp_clk_div.axi_div = 0;
	tmp_clk_div.ahb_div = 0;
	tmp_clk_div.ahb_pre_div = 0;
	tmp_clk_div.apb_div = 0;
	tmp_clk_div.apb_pre_div = 0;
	standby_clk_setdiv(&tmp_clk_div);

	/* swtich apb2 to losc */
	standby_clk_apb2losc();
	change_runtime_env();
		//delay_ms(1);
	standby_clk_plldisable();

	/* switch cpu to 32k */
	standby_clk_core2losc();

	if(1 == para->soc_dram_state.selfresh_flag){
		// disable HOSC, and disable LDO
		standby_clk_hoscdisable();
		standby_clk_ldodisable();
	}

	return 0;
}

static int bus_freq_resume(extended_standby_t *para)
{
	if(1 == para->soc_dram_state.selfresh_flag){
		/* enable LDO, enable HOSC */
		standby_clk_ldoenable();
		/* delay 1ms for power be stable */
		//3ms
		standby_delay_cycle(1);
		standby_clk_hoscenable();
		//3ms
		standby_delay_cycle(1);
	}

	/* switch clock to hosc */
	standby_clk_core2hosc();

	/* swtich apb2 to hosc */
	standby_clk_apb2hosc();

	/* restore clock division */
	standby_clk_setdiv(&clk_div);

	/* enable pll */
	standby_clk_pllenable();
	delay_ms(10);

	standby_clk_bus_src_restore();

	return 0;
}

