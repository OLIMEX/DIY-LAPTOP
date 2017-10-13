/*
 * drivers/thermal/sunxi-temperature.c
 *
 * Copyright (C) 2013-2014 allwinner.
 *	Qin Yongshen<qinyongshen@allwinnertech.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/thermal.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include "sunxi_ths.h"
#include "sun50i-thsensor.h"

static long sun50_th_reg_to_temp(u32 reg_data)
{
	u64 t;
	t = (MINUPA - reg_data * MULPA);
	do_div(t, DIVPA);
	return (long)t;
}

static u32 sun50_th_temp_to_reg(long temp)
{
	u64 t;
	t = (MINUPA - temp * DIVPA);
	do_div(t, MULPA);
	return (u32)t;
}

static int sun50_th_init_reg(struct sunxi_ths_data *ths_data)
{
	u32 reg_value;
	writel(THS_CTRL1_VALUE, ths_data->base_addr + THS_CTRL1_REG);

	writel(THS_CTRL0_VALUE, ths_data->base_addr + THS_CTRL0_REG);
	writel(THS_CTRL2_VALUE, ths_data->base_addr + THS_CTRL2_REG);
	writel(THS_INT_CTRL_VALUE, ths_data->base_addr + THS_INT_CTRL_REG);
	writel(THS_CLEAR_INT_STA, ths_data->base_addr + THS_INT_STA_REG);
	writel(THS_FILT_CTRL_VALUE, ths_data->base_addr + THS_FILT_CTRL_REG);

	reg_value = sun50_th_temp_to_reg(ths_data->int_temp);
	reg_value = (reg_value<<16);

	writel(reg_value, ths_data->base_addr + THS_INT_SHUT_TH_REG0);
	writel(reg_value, ths_data->base_addr + THS_INT_SHUT_TH_REG1);
	writel(reg_value, ths_data->base_addr + THS_INT_SHUT_TH_REG2);

	thsprintk(DEBUG_INIT, "THS_CTRL_REG = 0x%x\n", readl(ths_data->base_addr + THS_CTRL2_REG));
	thsprintk(DEBUG_INIT, "THS_INT_CTRL_REG = 0x%x\n", readl(ths_data->base_addr + THS_INT_CTRL_REG));
	thsprintk(DEBUG_INIT, "THS_INT_STA_REG = 0x%x\n", readl(ths_data->base_addr + THS_INT_STA_REG));
	thsprintk(DEBUG_INIT, "THS_FILT_CTRL_REG = 0x%x\n", readl(ths_data->base_addr + THS_FILT_CTRL_REG));

	return 0;
}

static int sun50i_th_clear_reg(struct sunxi_ths_data *ths_data)
{
	writel(0, ths_data->base_addr + THS_CTRL2_REG);
	return 0;
}

static long sun50i_th_get_temp(struct sunxi_ths_data *ths_data, u32 sensor_num)
{
	u32 reg_data;
	long temp;
	if(SENSOR_CNT > sensor_num){
		reg_data = readl(ths_data->base_addr + THS_DATA_REG0 + sensor_num * 4);
		thsprintk(DEBUG_DATA_INFO, "THS data%d = 0x%x\n", sensor_num, reg_data);
		temp = sun50_th_reg_to_temp(reg_data);
		if( -20 < temp && 150 > temp)
			return temp;
	}
	return 0;
}

static int sun50i_th_enable(struct sunxi_ths_data *ths_data)
{
	u32 reg_data;
	reg_data = readl(ths_data->base_addr + THS_CTRL2_REG);
	reg_data |= SENS0_ENABLE_BIT | SENS0_ENABLE_BIT | SENS0_ENABLE_BIT;
	writel(reg_data, ths_data->base_addr + THS_CTRL2_REG);
	return 0;
}

static int sun50i_th_disable(struct sunxi_ths_data *ths_data)
{
	u32 reg_data;
	reg_data = readl(ths_data->base_addr + THS_CTRL2_REG);
	reg_data &= ~(SENS0_ENABLE_BIT | SENS0_ENABLE_BIT | SENS0_ENABLE_BIT);
	writel(reg_data, ths_data->base_addr + THS_CTRL2_REG);
	return 0;
}

static int sun50i_th_get_int(struct sunxi_ths_data *data)
{
	return (readl(data->base_addr + THS_INT_STA_REG));
}

static void sun50i_th_clear_int(struct sunxi_ths_data *data)
{
	writel(THS_CLEAR_INT_STA, data->base_addr + THS_INT_STA_REG);
}

struct sunxi_ths_sensor_ops sunxi_ths_ops = {
	.init_reg = sun50_th_init_reg,
	.clear_reg = sun50i_th_clear_reg,
	.enable = sun50i_th_enable,
	.disable = sun50i_th_disable,
	.get_temp = sun50i_th_get_temp,
	.get_int = sun50i_th_get_int,
	.clear_int = sun50i_th_clear_int,
};


