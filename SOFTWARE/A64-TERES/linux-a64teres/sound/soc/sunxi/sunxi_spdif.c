/*
 * sound\soc\sunxi\sunxi_spdif.c
 * (C) Copyright 2014-2016
 * allwinnertech Technology Co., Ltd. <www.allwinnertech.com>
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <linux/dma/sunxi-dma.h>
#include <linux/pinctrl/consumer.h>
#include "sunxi_spdif.h"
#include <linux/regulator/consumer.h>

#define DRV_NAME "sunxi-spdif"
#ifdef CONFIG_ARCH_SUN8IW10
static bool  spdif_loop_en 		= false;
#endif

void spdif_txctrl_enable(int tx_en, int chan, int hub_en,struct sunxi_spdif_info *sunxi_spdif)
{
	u32 reg_val;

	if (chan == 1) {
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
		reg_val |= SUNXI_SPDIF_TXCFG_SINGLEMOD;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
	}

	/*flush TX FIFO*/
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	reg_val |= SUNXI_SPDIF_FCTL_FTX;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);

	/*clear interrupt status*/
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_ISTA);
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_ISTA);

	/*clear TX counter*/
	writel(0, sunxi_spdif->regs + SUNXI_SPDIF_TXCNT);

	if (tx_en) {
		/*SPDIF TX ENBALE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
		reg_val |= SUNXI_SPDIF_TXCFG_TXEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);

		/*DRQ ENABLE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_INT);
		reg_val |= SUNXI_SPDIF_INT_TXDRQEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_INT);
	} else {
		/*SPDIF TX DISABALE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
		reg_val &= ~SUNXI_SPDIF_TXCFG_TXEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);

		/*DRQ DISABLE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_INT);
		reg_val &= ~SUNXI_SPDIF_INT_TXDRQEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_INT);
	}

	if (hub_en) {
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
		reg_val |= SUNXI_SPDIFFCTL_HUBEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	} else {
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
		reg_val &= ~SUNXI_SPDIFFCTL_HUBEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	}
#ifdef CONFIG_SUNXI_AUDIO_DEBUG
	for(reg_val = 0; reg_val < 0x3c; reg_val=reg_val+4)
		pr_debug("%s,line:%d,0x%x:%x\n",__func__,__LINE__,reg_val,readl(sunxi_spdif->regs + reg_val));
#endif
}
EXPORT_SYMBOL(spdif_txctrl_enable);

static void spdif_rxctrl_enable(int rx_en,struct sunxi_spdif_info *sunxi_spdif)
{
	u32 reg_val;

	/*flush RX FIFO*/
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	reg_val |= SUNXI_SPDIF_FCTL_FRX;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);

	/*clear interrupt status*/
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_ISTA);
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_ISTA);

	/*clear RX counter*/
	writel(0, sunxi_spdif->regs + SUNXI_SPDIF_RXCNT);

	if (rx_en) {
		/*SPDIF RX ENBALE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCFG);
		reg_val |= SUNXI_SPDIF_RXCFG_RXEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCFG);

		/*DRQ ENABLE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_INT);
		reg_val |= SUNXI_SPDIF_INT_RXDRQEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_INT);
	} else {
		/*SPDIF TX DISABALE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCFG);
		reg_val &= ~SUNXI_SPDIF_RXCFG_RXEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCFG);

		/*DRQ DISABLE*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_INT);
		reg_val &= ~SUNXI_SPDIF_INT_RXDRQEN;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_INT);
	}
}

int spdif_set_fmt(unsigned int fmt,struct sunxi_spdif_info *sunxi_spdif)
{
	u32 reg_val;

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
	reg_val &= ~SUNXI_SPDIF_TXCFG_SINGLEMOD;
	reg_val |= SUNXI_SPDIF_TXCFG_ASS;
	reg_val &= ~SUNXI_SPDIF_TXCFG_NONAUDIO;
	reg_val |= SUNXI_SPDIF_TXCFG_CHSTMODE;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
#ifdef CONFIG_ARCH_SUN8IW1
	reg_val &= ~SUNXI_SPDIF_FCTL_FIFOSRC;
#endif
	reg_val |= SUNXI_SPDIF_FCTL_TXTL(16);
	reg_val |= SUNXI_SPDIF_FCTL_RXTL(15);
	reg_val |= SUNXI_SPDIF_FCTL_TXIM(1);
	reg_val |= SUNXI_SPDIF_FCTL_RXOM(3);

	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);

	if (!fmt) {/*PCM*/
		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA0_CHNUM(2));
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA1_SAMWORDLEN(1));
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
	} else {  /*non PCM*/
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
		reg_val |= SUNXI_SPDIF_TXCFG_NONAUDIO;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);

		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA0_CHNUM(2));
		reg_val |= SUNXI_SPDIF_TXCHSTA0_AUDIO;
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA1_SAMWORDLEN(1));
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
	}

	return 0;
}
EXPORT_SYMBOL(spdif_set_fmt);

int spdif_set_params(int format,struct sunxi_spdif_info *sunxi_spdif)
{
	u32 reg_val;

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
	reg_val &= ~SUNXI_SPDIF_TXCFG_FMTRVD;
	if(format == 16)
		reg_val |= SUNXI_SPDIF_TXCFG_FMT16BIT;
	else if(format == 20)
		reg_val |= SUNXI_SPDIF_TXCFG_FMT20BIT;
	else
		reg_val |= SUNXI_SPDIF_TXCFG_FMT24BIT;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);

	if (format == 24) {
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
		reg_val &= ~SUNXI_SPDIF_FCTL_TXIM(1);
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	} else {
		reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
		reg_val |= SUNXI_SPDIF_FCTL_TXIM(1);
		writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	}
	return 0;
}
EXPORT_SYMBOL(spdif_set_params);

int spdif_set_clkdiv(int div_id, int div,struct sunxi_spdif_info *sunxi_spdif )
{
	u32 reg_val = 0;

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xf));
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xf));
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xf));
  	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xf));
  	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);

	switch(div_id) {
		case SUNXI_DIV_MCLK:
		{
			reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
			reg_val &= ~(SUNXI_SPDIF_TXCFG_TXRATIO(0x1F));
			reg_val |= SUNXI_SPDIF_TXCFG_TXRATIO(div-1);
			writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCFG);
			if(clk_get_rate(sunxi_spdif->pllclk) == 24576000){
				switch(div)
				{
					/*24KHZ*/
					case 8:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x6));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x9));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x6));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x9));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					/*32KHZ*/
					case 6:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x3));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xC));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x3));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xC));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					/*48KHZ*/
					case 4:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x2));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xD));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x2));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xD));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					/*96KHZ*/
					case 2:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xA));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x5));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0xA));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x5));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					/*192KHZ*/
					case 1:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xE));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x1));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0xE));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x1));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					default:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
				}
			}else{  /*22.5792MHz*/
				switch(div)
				{
					/*22.05khz*/
					case 8:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x4));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xb));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x4));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xb));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					/*44.1KHZ*/
					case 4:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x0));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xF));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x0));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xF));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					/*88.2khz*/
					case 2:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x8));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x7));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x8));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x7));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					/*176.4KHZ*/
					case 1:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xC));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x3));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0xC));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x3));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
					default:
						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA0);

						reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_RXCHSTA1);
						break;
				}
			}
		}
		break;
		case SUNXI_DIV_BCLK:
		break;

		default:
			return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(spdif_set_clkdiv);

static int sunxi_spdif_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(cpu_dai);

	spdif_set_fmt(fmt,sunxi_spdif);

	return 0;
}

static int sunxi_spdif_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params,
						struct snd_soc_dai *dai)
{
	int format;
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(dai);
	switch (params_format(params))
	{
		case SNDRV_PCM_FORMAT_S16_LE:
		format = 16;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		format = 20;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		format = 24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		format = 24;
		break;
	default:
		return -EINVAL;
	}
	spdif_set_params(format,sunxi_spdif);

	return 0;
}

static int sunxi_spdif_trigger(struct snd_pcm_substream *substream,
                              int cmd, struct snd_soc_dai *dai)
{
	int ret = 0;
#ifdef CONFIG_ARCH_SUN8IW10
	u32 reg_val = 0;
#endif
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(dai);

	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				spdif_rxctrl_enable(1,sunxi_spdif);
			} else {
				spdif_txctrl_enable(1,substream->runtime->channels, 0,sunxi_spdif);
			}
#ifdef CONFIG_ARCH_SUN8IW10
			if (spdif_loop_en) {
				reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_CTL);
				reg_val |= SUNXI_SPDIF_CTL_LOOP;
				writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_CTL);
			}
#endif
			break;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				spdif_rxctrl_enable(0,sunxi_spdif);
			} else {
			  spdif_txctrl_enable(0, substream->runtime->channels, 0,sunxi_spdif);
			}
			break;
		default:
			ret = -EINVAL;
			break;
	}

		return ret;
}
#ifdef CONFIG_ARCH_SUN8IW10
module_param_named(spdif_loop_en, spdif_loop_en, bool, S_IRUGO | S_IWUSR);
#endif

/*freq:   1: 22.5792MHz   0: 24.576MHz  */
static int sunxi_spdif_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id,
                                 unsigned int freq, int dir)
{
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(cpu_dai);
	if (!freq) {

		if (clk_set_rate(sunxi_spdif->pllclk, 24576000)) {
			pr_err("try to set the spdif_pll rate failed!\n");
		}

	} else {
		if (clk_set_rate(sunxi_spdif->pllclk, 22579200)) {
			pr_err("try to set the spdif_pll rate failed!\n");
		}
	}

	return 0;
}

static int sunxi_spdif_set_clkdiv(struct snd_soc_dai *cpu_dai, int div_id, int div)
{
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(cpu_dai);
	spdif_set_clkdiv(div_id, div,sunxi_spdif);
	return 0;
}

static int sunxi_spdif_dai_probe(struct snd_soc_dai *dai)
{
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(dai);
	dai->capture_dma_data = &sunxi_spdif->capture_dma_param;
	dai->playback_dma_data = &sunxi_spdif->play_dma_param;

	return 0;
}
static int sunxi_spdif_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static int sunxi_spdif_suspend(struct snd_soc_dai *cpu_dai)
{
	u32 reg_val = 0,ret = 0;
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[SPDIF]Enter %s\n", __func__);
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_CTL);
	reg_val &= ~SUNXI_SPDIF_CTL_GEN;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_CTL);
	if (NULL != sunxi_spdif->pinstate_sleep) {
		ret = pinctrl_select_state(sunxi_spdif->pinctrl, sunxi_spdif->pinstate_sleep);
		if (ret) {
			pr_warn("[spdif]select pin sleep state failed\n");
			return ret;
		}
	}
	if (sunxi_spdif->pinctrl !=NULL)
		devm_pinctrl_put(sunxi_spdif->pinctrl);
	sunxi_spdif->pinctrl = NULL;
	sunxi_spdif->pinstate = NULL;
	sunxi_spdif->pinstate_sleep = NULL;
	pr_debug("[SPDIF]sunxi_spdif->clk_enable_cnt:%d,%s\n",sunxi_spdif->clk_enable_cnt, __func__);
	if (sunxi_spdif->clk_enable_cnt > 0) {
		if (sunxi_spdif->moduleclk != NULL) {
			clk_disable(sunxi_spdif->moduleclk);
		}
		if (sunxi_spdif->pllclk != NULL) {
			clk_disable(sunxi_spdif->pllclk);
		}
		sunxi_spdif->clk_enable_cnt--;
	}
	pr_debug("[SPDIF]End %s\n", __func__);
	return 0;
}

static int sunxi_spdif_resume(struct snd_soc_dai *cpu_dai)
{
	u32 reg_val;
	s32 ret = 0;
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[SPDIF]Enter %s\n", __func__);

	if (sunxi_spdif->pllclk != NULL) {
		if (clk_prepare_enable(sunxi_spdif->pllclk)) {
			pr_err("open sunxi_spdif->pllclk failed! line = %d\n", __LINE__);
		}
	}

	if (sunxi_spdif->moduleclk != NULL) {
		if (clk_prepare_enable(sunxi_spdif->moduleclk)) {
			pr_err("open sunxi_spdif->moduleclk failed! line = %d\n", __LINE__);
		}
	}
	sunxi_spdif->clk_enable_cnt++;
	pr_debug("[SPDIF]sunxi_spdif->clk_enable_cnt:%d,%s\n",sunxi_spdif->clk_enable_cnt, __func__);
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_CTL);
	reg_val |= SUNXI_SPDIF_CTL_GEN;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_CTL);
	if (!sunxi_spdif->pinctrl) {
		sunxi_spdif->pinctrl = devm_pinctrl_get(cpu_dai->dev);
		if (IS_ERR_OR_NULL(sunxi_spdif->pinctrl)) {
			pr_warn("[spdif]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_spdif->pinstate){
		sunxi_spdif->pinstate = pinctrl_lookup_state(sunxi_spdif->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_spdif->pinstate)) {
			pr_warn("[spdif]lookup pin default state failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_spdif->pinstate_sleep){
		sunxi_spdif->pinstate_sleep = pinctrl_lookup_state(sunxi_spdif->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_spdif->pinstate_sleep)) {
			pr_warn("[spdif]lookup pin sleep state failed\n");
			return -EINVAL;
		}
	}

	ret = pinctrl_select_state(sunxi_spdif->pinctrl, sunxi_spdif->pinstate);
	if (ret) {
		pr_warn("[spdif]select pin default state failed\n");
		return ret;
	}
	pr_debug("[SPDIF]End %s\n", __func__);
	return 0;
}


#define SUNXI_SPDIF_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)
static struct snd_soc_dai_ops sunxi_spdif_dai_ops = {
	.trigger 	= sunxi_spdif_trigger,
	.hw_params 	= sunxi_spdif_hw_params,
	.set_fmt 	= sunxi_spdif_set_fmt,
	.set_clkdiv = sunxi_spdif_set_clkdiv,
	.set_sysclk = sunxi_spdif_set_sysclk,
};
static struct snd_soc_dai_driver sunxi_spdif_dai = {
	.probe 		= sunxi_spdif_dai_probe,
	.suspend 	= sunxi_spdif_suspend,
	.resume 	= sunxi_spdif_resume,
	.remove 	= sunxi_spdif_dai_remove,
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SUNXI_SPDIF_RATES,
	.formats = SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S20_3LE| SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.capture = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SUNXI_SPDIF_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S20_3LE| SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.ops = &sunxi_spdif_dai_ops,
};
static const struct snd_soc_component_driver sunxi_spdif_component = {
	.name		= DRV_NAME,
};
static const struct of_device_id sunxi_spdif_of_match[] = {
	{ .compatible = "allwinner,sunxi-spdif", },
	{},
};

static int __init sunxi_spdif_dev_probe(struct platform_device *pdev)
{
	u32 ret = 0,reg_val = 0;
	struct resource res;
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	void __iomem  *sunxi_spdif_membase = NULL;
	struct sunxi_spdif_info *sunxi_spdif;
	sunxi_spdif = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_spdif_info), GFP_KERNEL);
	if (!sunxi_spdif) {
		dev_err(&pdev->dev, "Can't allocate sunxi_spdif\n");
		ret = -ENOMEM;
		goto err0;
	}
	pr_debug("[audio-spdif] platform initial.\n");
	dev_set_drvdata(&pdev->dev, sunxi_spdif);
	sunxi_spdif->dai = sunxi_spdif_dai;
	sunxi_spdif->dai.name = dev_name(&pdev->dev);

	device = of_match_device(sunxi_spdif_of_match, &pdev->dev);
	if (!device)
		return -ENODEV;

	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}

	sunxi_spdif_membase =ioremap(res.start, resource_size(&res));
	if (NULL == sunxi_spdif_membase) {
		pr_err("[audio-spdif]Can't map spdif registers\n");
	} else {
		sunxi_spdif->regs = sunxi_spdif_membase;
	}
	sunxi_spdif->pllclk = of_clk_get(node, 0);
	sunxi_spdif->moduleclk= of_clk_get(node, 1);
	if (IS_ERR(sunxi_spdif->pllclk) || IS_ERR(sunxi_spdif->moduleclk)){
		dev_err(&pdev->dev, "[audio-spdif]Can't get spdif clocks\n");
		if (IS_ERR(sunxi_spdif->pllclk))
			ret = PTR_ERR(sunxi_spdif->pllclk);
		else
			ret = PTR_ERR(sunxi_spdif->moduleclk);
		goto err1;
	} else {
		if (clk_set_parent(sunxi_spdif->moduleclk, sunxi_spdif->pllclk)) {
			pr_err("try to set parent of sunxi_spdif->moduleclk to sunxi_spdif->pllclk failed! line = %d\n",__LINE__);
		}
		clk_prepare_enable(sunxi_spdif->pllclk);
		clk_prepare_enable(sunxi_spdif->moduleclk);
		sunxi_spdif->clk_enable_cnt++;
	}

	sunxi_spdif->play_dma_param.dma_addr = res.start + SUNXI_SPDIF_TXFIFO;
	sunxi_spdif->play_dma_param.dma_drq_type_num = DRQDST_SPDIFTX;
	sunxi_spdif->play_dma_param.dst_maxburst = 8;
	sunxi_spdif->play_dma_param.src_maxburst = 8;

	sunxi_spdif->capture_dma_param.dma_addr = res.start + SUNXI_SPDIF_RXFIFO;
	sunxi_spdif->capture_dma_param.dma_drq_type_num = DRQSRC_SPDIFRX;
	sunxi_spdif->capture_dma_param.src_maxburst = 8;
	sunxi_spdif->capture_dma_param.dst_maxburst = 8;

	sunxi_spdif->pinctrl = NULL;
	if (!sunxi_spdif->pinctrl) {
		sunxi_spdif->pinctrl = devm_pinctrl_get(&pdev->dev);
		if (IS_ERR_OR_NULL(sunxi_spdif->pinctrl)) {
			pr_warn("[spdif]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_spdif->pinstate){
		sunxi_spdif->pinstate = pinctrl_lookup_state(sunxi_spdif->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_spdif->pinstate)) {
			pr_warn("[spdif]lookup pin default state failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_spdif->pinstate_sleep){
		sunxi_spdif->pinstate_sleep = pinctrl_lookup_state(sunxi_spdif->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_spdif->pinstate_sleep)) {
			pr_warn("[spdif]lookup pin sleep state failed\n");
			return -EINVAL;
		}
	}

	ret = snd_soc_register_component(&pdev->dev, &sunxi_spdif_component,
				   &sunxi_spdif->dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "Could not register DAI: %d\n", ret);
		ret = -ENOMEM;
		goto err1;
	}
	ret = asoc_dma_platform_register(&pdev->dev,0);
	if (ret) {
		dev_err(&pdev->dev, "Could not register PCM: %d\n", ret);
		goto err2;
	}
	/*global enbale*/
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_CTL);
	reg_val |= SUNXI_SPDIF_CTL_GEN;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_CTL);

	return 0;
err2:
	snd_soc_unregister_component(&pdev->dev);
err1:
	iounmap(sunxi_spdif->regs);
err0:
	return ret;
}

static int __exit sunxi_spdif_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver sunxi_spdif_driver = {
	.probe = sunxi_spdif_dev_probe,
	.remove = __exit_p(sunxi_spdif_dev_remove),
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_spdif_of_match,
	},
};

static int __init sunxi_spdif_init(void)
{
	return platform_driver_register(&sunxi_spdif_driver);
}
module_init(sunxi_spdif_init);

static void __exit sunxi_spdif_exit(void)
{
	platform_driver_unregister(&sunxi_spdif_driver);
}
module_exit(sunxi_spdif_exit);

/* Module information */
MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("sunxi SPDIF SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-spdif");

