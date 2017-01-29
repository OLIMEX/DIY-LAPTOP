
#ifndef _LINUX_SCENELOCK_DATA_SUN8IW1P1_H
#define _LINUX_SCENELOCK_DATA_SUN8IW1P1_H

#include <linux/power/axp_depend.h>

scene_extended_standby_t extended_standby[] = {
	{
		.scene_type	= SCENE_TALKING_STANDBY,
		.name		= "talking_standby",
		.soc_pwr_dep.id             = TALKING_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VDD_DRAMPLL_BIT) | BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT)  | BITMAP(VCC_LDOIN_BIT), 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = 0x0,	// mean all osc is off.
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = BITMAP(PM_PLL_DRAM), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = 0x0,
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x1,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},
#if 0		
		//for pb port
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c20824, 0x0000ffff, 0x00007777},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c20828, 0x00000ff0, 0x00000770},
#endif

	},
	
	{
		.scene_type	= SCENE_USB_STANDBY,
		.name		= "usb_standby",
		.soc_pwr_dep.id  		= USB_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_ldoin, vdd_sys is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VDD_DRAMPLL_BIT) | BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT)  | BITMAP(VCC_LDOIN_BIT) |\
							     BITMAP(VDD_SYS_BIT), 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = BITMAP(OSC_LOSC_BIT) | BITMAP(OSC_HOSC_BIT),	// mean all osc is off. +losc, +hosc
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = BITMAP(PM_PLL_DRAM), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = BITMAP(PM_PLL_PERIPH),
		.soc_pwr_dep.cpux_clk_state.pll_factor[PM_PLL_PERIPH] = { //PLL_PERIPH freq = 24*12/2/2= 72M 
		    .factor1 = 12, //N=12
		    .factor2 = 1, //Div1 = 1 + 1 = 2
		    .factor3 = 1, //Div2 = 1 + 1 = 2
		},
		.soc_pwr_dep.cpux_clk_state.bus_change     = BITMAP(BUS_AHB1) | BITMAP(BUS_AHB2),
		.soc_pwr_dep.cpux_clk_state.bus_factor[BUS_AHB1]     = {
		    .src = CLK_SRC_LOSC,				//need make sure losc is on.
		    .pre_div = 0,
		    .div_ratio = 0,
		},
		.soc_pwr_dep.cpux_clk_state.bus_factor[BUS_AHB2]     = {
		    .src = CLK_SRC_AHB1,				//need make sure AHB1 is on.
		    .pre_div = 0,
		    .div_ratio = 0,
		},
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x0,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},
	},
	{
		.scene_type	= SCENE_MP3_STANDBY,
		.name		= "mp3_standby",
		.soc_pwr_dep.id  		= MP3_STANDBY_FLAG,
	},
	{
		.scene_type	= SCENE_BOOT_FAST,
		.name		= "boot_fast",
		.soc_pwr_dep.id  		= BOOT_FAST_STANDBY_FLAG,
	},
	{
		.scene_type		    = SCENE_SUPER_STANDBY,
		.name			    = "super_standby",
		.soc_pwr_dep.id             = SUPER_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VDD_DRAMPLL_BIT) | BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT)  | BITMAP(VCC_LDOIN_BIT), 
		//mean care about cpua, dram, sys, cpus, dram_pll, vdd_adc, vcc_pl, vcc_io, vcc_cpvdd, vcc_ldoin, vcc_pll 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = 0x0,	// mean all osc is off.
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = BITMAP(PM_PLL_DRAM), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = 0x0,
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x1,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},

	},
	{
		.scene_type		    = SCENE_GPIO_HOLD_STANDBY,
		.name			    = "gpio_hold_standby",
		.soc_pwr_dep.id             = GPIO_HOLD_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VDD_DRAMPLL_BIT) | BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT) | BITMAP(VCC_IO_BIT) | BITMAP(VCC_LDOIN_BIT), 
		//mean care about cpua, dram, sys, cpus, dram_pll, vdd_adc, vcc_pl, vcc_io, vcc_cpvdd, vcc_ldoin, vcc_pll 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = 0x0,	// mean all osc is off.
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = BITMAP(PM_PLL_DRAM), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = 0x0,
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x1,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},

	},
	{
		.scene_type		    = SCENE_NORMAL_STANDBY,
		.name			    = "normal_standby",
		.soc_pwr_dep.id             = NORMAL_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VDD_DRAMPLL_BIT) | BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT)  | BITMAP(VCC_LDOIN_BIT), 
		//mean care about cpua, dram, sys, cpus, dram_pll, vdd_adc, vcc_pl, vcc_io, vcc_cpvdd, vcc_ldoin, vcc_pll 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = 0x0,	// mean all osc is off.
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = BITMAP(PM_PLL_DRAM), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = 0x0,
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x1,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},

	},
	{
		.scene_type		    = SCENE_GPIO_STANDBY,
		.name			    = "gpio_standby",
		.soc_pwr_dep.id             = GPIO_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on. +vdd_sys
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VDD_SYS_BIT) | BITMAP(VDD_DRAMPLL_BIT) | BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT) | BITMAP(VCC_IO_BIT) | BITMAP(VCC_LDOIN_BIT), 
		//mean care about cpua, dram, sys, cpus, dram_pll, vdd_adc, vcc_pl, vcc_io, vcc_cpvdd, vcc_ldoin, vcc_pll 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = BITMAP(OSC_LOSC_BIT),	// mean all osc is off. +losc
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = BITMAP(PM_PLL_DRAM), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = BITMAP(BUS_AHB1),
		.soc_pwr_dep.cpux_clk_state.bus_factor[BUS_AHB1]     = {
		    .src = CLK_SRC_LOSC,				//need make sure losc is on.
		    .pre_div = 0,
		    .div_ratio = 0,
		},
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x0,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},
#if 0		
		//for pb port
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c20824, 0x0000ffff, 0x00007777},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c20828, 0x00000ff0, 0x00000770},
#endif

	},

	{
		.scene_type	= SCENE_MISC_STANDBY,
		.name		= "misc_standby",
		.soc_pwr_dep.id = MISC_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT) | BITMAP(VCC_IO_BIT) ,
		    
		    0x0644, //-vcc_io; -dram, ldoin/ adc/ cpvdd   
		//mean care about cpua, dram, sys, cpus, dram_pll, vdd_adc, vcc_pl, vcc_io, vcc_cpvdd, vcc_ldoin, vcc_pll 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = 0x0,	// mean all osc is off.
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = (~(0x20)), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = 0x0,
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x1,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},
	},
	{
		.scene_type	= SCENE_MISC1_STANDBY,
		.name		= "misc1_standby",
		.soc_pwr_dep.id = MISC1_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT), 
		//mean care about cpua, dram, sys, cpus, dram_pll, vdd_adc, vcc_pl, vcc_io, vcc_cpvdd, vcc_ldoin, vcc_pll 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = 0x0,	// mean all osc is off.
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = (~(0x20)), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = 0x0,
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x1,		//hold gpio
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},
	},
	{
		//for parse sysconfig config. //
		//default config according dram enter selfresh.
		//when not enable enter selfresh, need open vdd_sys.
		.scene_type		    = SCENE_DYNAMIC_STANDBY,
		.name			    = "dynamic_standby",
		.soc_pwr_dep.id             = DYNAMIC_STANDBY_FLAG,
		//mean dram, cpus,dram_pll,vcc_pl, vcc_io, vcc_ldoin is on.
		//note: vcc_pm is marked on, just for cross-platform reason.
		//at a83: with the sys_mask's help, we know we do not need care about vcc_pm state.
		.soc_pwr_dep.soc_pwr_dm_state.state	   = BITMAP(VCC_DRAM_BIT) | BITMAP(VDD_CPUS_BIT) |\
							     BITMAP(VDD_DRAMPLL_BIT) | BITMAP(VCC_PL_BIT) | \
							     BITMAP(VCC_PM_BIT)  | BITMAP(VCC_LDOIN_BIT) | \
							     BITMAP(VDD_SYS_BIT)| BITMAP(VDD_CPUA_BIT),
		//mean care about cpua, dram, sys, cpus, dram_pll, vdd_adc, vcc_pl, vcc_io, vcc_cpvdd, vcc_ldoin, vcc_pll 
		.soc_pwr_dep.soc_pwr_dm_state.volt[0]      = 0x0,	//mean: donot need care about the voltage.
		.soc_pwr_dep.cpux_clk_state.osc_en         = 0x0,	// mean all osc is off.
		.soc_pwr_dep.cpux_clk_state.init_pll_dis   = (BITMAP(PM_PLL_DRAM)), //mean pll5 is shutdowned & open by dram driver.
		.soc_pwr_dep.cpux_clk_state.exit_pll_en    = 0x0,
		.soc_pwr_dep.cpux_clk_state.pll_change     = 0x0,
		.soc_pwr_dep.cpux_clk_state.bus_change     = 0x0,
		.soc_pwr_dep.soc_dram_state.selfresh_flag     = 0x1,	    //enter selfresh, for compatible reason.
		.soc_pwr_dep.soc_io_state.hold_flag     = 0x1,
		//for pf port: set the io to disable state.;
		.soc_pwr_dep.soc_io_state.io_state[0]     = {(unsigned int *)0x01c208b4, 0x00f0f0ff, 0x00707077},
		.soc_pwr_dep.soc_io_state.io_state[1]     = {(unsigned int *)0x01c208b4, 0x000f0f00, 0x00070700},
	},

};

#endif

