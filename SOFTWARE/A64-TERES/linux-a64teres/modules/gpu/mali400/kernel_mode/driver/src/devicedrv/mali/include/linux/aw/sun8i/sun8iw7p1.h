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

#ifndef _MALI_SUN8I_W7P1_H_
#define _MALI_SUN8I_W7P1_H_

#define GPU_PBASE           SUNXI_GPU_PBASE
#define IRQ_GPU_GP          SUNXI_IRQ_GPU_GP
#define IRQ_GPU_GPMMU       SUNXI_IRQ_GPU_GPMMU
#define IRQ_GPU_PP0         SUNXI_IRQ_GPU_PP0
#define IRQ_GPU_PPMMU0      SUNXI_IRQ_GPU_PPMMU0
#define IRQ_GPU_PP1         SUNXI_IRQ_GPU_PP1
#define IRQ_GPU_PPMMU1      SUNXI_IRQ_GPU_PPMMU1

static struct aw_freq_data freq_data =
{
	.normal_freq  = 576,
	.extreme_freq = 600,
};

static struct aw_private_data private_data =
{
	.clk_status        = 0,
	.scene_ctrl_status = 1,
	.sensor_num        = 0,
	.regulator         = NULL,
	.regulator_id      = NULL,
	.tempctrl_data     =
	{
		.temp_ctrl_status = 1,
	},
};

static struct aw_clk_data clk_data[] =
{
	{
		.clk_name            = "pll",
		.clk_id              = PLL_GPU_CLK,
		.clk_handle          = NULL,
	},
	{
		.clk_name            = "mali",
		.clk_id              = GPU_CLK,
		.clk_handle          = NULL,
	},
};

#ifdef CONFIG_CPU_BUDGET_THERMAL
static struct aw_tf_table tf_table[] =
{
	{
		.temp = 70,
		.freq = 432,
	},
	{
		.temp  = 80,
		.freq = 312,
	},
	{
		.temp  = 90,
		.freq = 144,
	},
};
#endif /* CONFIG_CPU_BUDGET_THERMAL */

#endif