/*
 * sound\soc\sunxi\sunxi_dsd.h
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.allwinnertech.com>
 * huangxin <huangxin@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef SUNXI_DSD_H_
#define SUNXI_DSD_H_
#include "sunxi_dma.h"

/*------------------DSD register definition--------------------*/
#define DSD_EN_CTRL				0x00
#define DSD_SR_CTRL				0x04
#define DSD_TX_CONF				0x08
#define DSD_TX_FIFO_CTRL		0x14
#define DSD_TX_FIFO_STA			0x18
#define DSD_INT_CTRL			0x1c
#define DSD_TX_DATA				0x20
#define DSD_INT_STA				0x24
#define DSD_TX_CNT				0x28
#define DSD_TX_MAP				0x2c

/*DSD_EN_CTRL:0x00*/
#define TX_EN					1
#define GLOBAL_EN				0

/*DSD_SR_CTRL:0x04*/
#define DSD_SAMPLE_RATE			0//pclk

/*DSD_TX_CONF:0x08*/
#define PCLK_SOURCE_SELECT		20
#define DSD_TX_CHAN_NUM			16
#define DSD_TX_CHAN_EN			8
#define MSB_LSB_FIR_SEL			5
#define DSD_TX_DRIVER_MODE		4
#define DSD_TX_DATA_WIDTH		1
#define DSD_TX_MODE_SELECT		0

/*DSD_TX_FIFO_CTRL:0x14*/
#define TX_FIFO_FLUSH			31
#define TX_FIFO_TRIG_LEVEL		8
#define TXIM					0

/*DSD_TX_FIFO_STA:0x18*/
#define TX_FIFO_FULL			9
#define TX_FIFO_EMPTY			8
#define TX_FIFO_DATA_COUNTER	0

/*DSD_INT_CTRL:0x1c*/
#define TX_FIFO_DRQ_EN			3
#define TX_FIFO_EMPYT_IRQ_EN	2
#define TX_FIFO_UNDERRUN_IRQ_EN	0

/*DSD_TX_DATA:0x20*/
#define TX_DATA					0

/*DSD_INT_STA:0x24*/
#define TXE_INT					2
#define TXO_INT					1
#define TXU_INT					0

/*DSD_TX_CNT:0x28*/
#define TX_CNT					0

/*DSD_TX_MAP:0x2c*/
#define TX_CHAN1_MAP			4
#define TX_CHAN0_MAP			0
#define DSD_CHANMAP_DEFAULT	(0x10)

struct sunxi_dsd_info {
	void __iomem   *regs;    /* dsd base */
	struct clk *pllclk;
	struct clk *moduleclk;
	struct snd_soc_dai_driver dai;
	struct sunxi_dma_params play_dma_param;
	struct pinctrl *pinctrl;
	struct pinctrl_state  *pinstate;
	struct pinctrl_state  *pinstate_sleep;
	u32 clk_enable_cnt;
	u32 mode_select;
};

#endif

