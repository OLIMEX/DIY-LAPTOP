/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2015 Allwinner Technology Co., Ltd.
 *
 * Author: Xiangyun Yu <yuxyun@allwinnertech.com>
 */

#ifndef _MALI_SUN50I_W1P1_H_
#define _MALI_SUN50I_W1P1_H_

static struct aw_freq_data freq_data =
{
	.normal_freq  = 432,
	.extreme_freq = 432,
};

static struct aw_private_data private_data =
{
	.clk_status        = 0,
	.scene_ctrl_status = 0,
	.sensor_num        = 2,
#ifdef CONFIG_MALI_DT
	.np_gpu            = NULL,
#endif
	.regulator         = NULL,
	.regulator_id      = "vdd-sys",
	.tempctrl_data     =
	{
		.temp_ctrl_status = 1,
	},
};

static struct aw_clk_data clk_data[] =
{
	{
		.clk_name   = "pll",
		.clk_handle = NULL,
	},
	{
		.clk_name   = "mali",
		.clk_handle = NULL,
	},
};

#endif
