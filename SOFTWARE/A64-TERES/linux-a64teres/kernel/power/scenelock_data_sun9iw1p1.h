
#ifndef _LINUX_SCENELOCK_DATA_SUN9IW1P1_H
#define _LINUX_SCENELOCK_DATA_SUN9IW1P1_H

#include <linux/power/axp_depend.h>

scene_extended_standby_t extended_standby[] = {
	{
		{
			.id           	= TALKING_STANDBY_FLAG,
			.pwr_dm_en      = 0x0ff7,     /* only cpul is power off                           */
			.osc_en         = 0xf,        /* 24M & pll_ldo are reversed open                  */
			.init_pll_dis   = 0x1f0e,     /* pll_c0/ve/video0/1 can not be closed             */
			.exit_pll_en    = 0x0,        /* all plls are reversed                            */
			.pll_change     = 0x0,
			.bus_change     = 0x0,
		},
		.scene_type	= SCENE_TALKING_STANDBY,
		.name		= "talking_standby",
	},
	{
		{
			.id  		= USB_STANDBY_FLAG,
			.pwr_dm_en      = 0x0a7,      /* only dc5ldo & dcdc1/4/5 & s-bldo2 power on       */
			.osc_en         = 0x9,        /* 24M & pll_ldo are reversed open                  */
			.init_pll_dis   = 0x1fdf,     /* all plls ard closed                              */
			.exit_pll_en    = 0x0d1,      /* open pll_c0/VE/vedio0/1 for system resume stable */
			.bus_change     = 0x07fc0,    /* adjust all BUSES to lowest freq for power save   */
			.bus_factor[6]  = {1<<0, 0, 4, 0, 0},   /* GTBUS set to 24M/4=6M for AHB0 use for USB HOST  */
			.bus_factor[7]  = {1<<7, 0, 8, 0, 0},   /* AHB0 need adjust to GTBUS/8=750K for USB HOST    */
			.bus_factor[8]  = {1<<5, 0, 8, 0, 0},   /* AHB1 adjust to pll_per0(0MHz) for power save     */
			.bus_factor[9]  = {1<<5, 0, 8, 0, 0},   /* AHB2 adjust to pll_per0(0MHz) for power save     */
			.bus_factor[10] = {1<<0, 0, 8, 0, 0},   /* APB0 adjust to 24M/8=3M for power save           */
			.bus_factor[11] = {1<<5, 0, 8, 0,32},   /* APB1 adjust to pll_per0(0MHz) for power save     */
			.bus_factor[12] = {1<<5, 0, 4, 0, 0},   /* CCI adjust to pll_per0(0MHz) for power save      */
			.bus_factor[13] = {1<<5, 0, 0, 0, 8},   /* ATS adjust to pll_per0(0MHz) for power save      */
			.bus_factor[14] = {1<<5, 0, 0, 0, 8},   /* TRACE adjust to pll_per0(0MHz) for power save    */
		},
		.scene_type	= SCENE_USB_STANDBY,
		.name		= "usb_standby",
	},
	{
		{
			.id  		= MP3_STANDBY_FLAG,
		},
		.scene_type	= SCENE_MP3_STANDBY,
		.name		= "mp3_standby",
	},
	{
		{
			.id		= BOOT_FAST_STANDBY_FLAG,
		},
		.scene_type	= SCENE_BOOT_FAST,
		.name		= "boot_fast",
	},
	{
		{
			.id             = SUPER_STANDBY_FLAG,
			.pwr_dm_en      = 0x0084,     /* reserve dcdc5 & dc5ldo power                     */
			.osc_en         = 0,          /* 24M & pll_ldo close, so all plls will can't used */
			.init_pll_dis   = 0,          /* all plls should be closed, but no need disabled  */
			.exit_pll_en    = 0x0,        /* all plls is default, for BROM need default envir */
			.pll_change     = 0x0,
			.bus_change     = 0x0,
		},
		.scene_type	= SCENE_SUPER_STANDBY,
		.name		= "super_standby",
	},
	{
		{
			.id             = NORMAL_STANDBY_FLAG,
			.pwr_dm_en      = 0x0fff,     /* mean all power domains are reversed power on     */
			.osc_en         = 0xf,        /* 24M & pll_ldo are reversed open                  */
			.init_pll_dis   = 0x1f0e,     /* pll_c0/ve/video0/1 can not be closed             */
			.exit_pll_en    = 0,
			.pll_change     = 0,
			.pll_factor[0]  = {0x10,0,0,0},
			.bus_change     = 0,
			.bus_factor[0]  = {0x2,0,0,0,0},
			.bus_factor[2]  = {0x2,0,0,0,0},
		},
		.scene_type	= SCENE_NORMAL_STANDBY,
		.name		= "normal_standby",
	},
	{
		{
			.id             = GPIO_STANDBY_FLAG,
			.pwr_dm_en      = 0x0fab,     /* mean avcc, dram, sys, io, cpus is on             */
			.osc_en         = 0xf,        /* 24M & pll_ldo are reversed open                  */
			.init_pll_dis   = 0x1f0e,     /* pll_c0/ve/video0/1 can not be closed             */
			.exit_pll_en    = 0x0,
			.pll_change     = 0x0,
			.bus_change     = 0x0,
			.bus_factor[2]  = {0x0,0,0,0,0},
		},
		.scene_type	= SCENE_GPIO_STANDBY,
		.name		= "gpio_standby",
	},
	{
		{
			.id		= MISC_STANDBY_FLAG,
		},
		.scene_type	= SCENE_MISC_STANDBY,
		.name		= "misc_standby",
	},
};

#endif

