/*
 * sound\soc\sunxi\sunxi_tdm_utils.c
 * (C) Copyright 2014-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@Reuuimllatech.com>
 * Liu shaohua <liushaohua@allwinnertech.com>
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
#include <linux/pinctrl/consumer.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <asm/dma.h>
#include <linux/gpio.h>
#include "sunxi_dma.h"
#include "sunxi_tdm_utils.h"
static bool  daudio0_loop_en 		= false;

int txctrl_tdm(int on,int hub_en,struct sunxi_tdm_info *sunxi_tdm)
{
	u32 reg_val;
	struct sunxi_tdm_info *tdm = NULL;
	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;
	/*flush TX FIFO*/
	reg_val = readl(tdm->regs + SUNXI_DAUDIOFCTL);
	reg_val |= SUNXI_DAUDIOFCTL_FTX;
	writel(reg_val, tdm->regs + SUNXI_DAUDIOFCTL);
	/*clear TX counter*/
	writel(0, tdm->regs + SUNXI_DAUDIOTXCNT);

	if (on) {
		/* enable DMA DRQ mode for play */
		reg_val = readl(tdm->regs + SUNXI_DAUDIOINT);
		reg_val |= SUNXI_DAUDIOINT_TXDRQEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOINT);
	} else {
		/* DISBALE dma DRQ mode */
		reg_val = readl(tdm->regs + SUNXI_DAUDIOINT);
		reg_val &= ~SUNXI_DAUDIOINT_TXDRQEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOINT);

		/*DISABLE TXEN*/
		reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
		reg_val &= ~SUNXI_DAUDIOCTL_TXEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
	}
	if (hub_en) {
		reg_val = readl(tdm->regs + SUNXI_DAUDIOFCTL);
		reg_val |= SUNXI_DAUDIOFCTL_HUBEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOFCTL);
	} else {
		reg_val = readl(tdm->regs + SUNXI_DAUDIOFCTL);
		reg_val &= ~SUNXI_DAUDIOFCTL_HUBEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOFCTL);
	}

	return 0;
}
EXPORT_SYMBOL(txctrl_tdm);

int  rxctrl_tdm(int on,struct sunxi_tdm_info *sunxi_tdm)
{
	u32 reg_val;
	struct sunxi_tdm_info *tdm = NULL;
	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;

	if (on) {
		/* enable DMA DRQ mode for record */
		reg_val = readl(tdm->regs + SUNXI_DAUDIOINT);
		reg_val |= SUNXI_DAUDIOINT_RXDRQEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOINT);
	} else {
		/*DISABLE DAUDIO RX */
		reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
		reg_val &= ~SUNXI_DAUDIOCTL_RXEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
		/* DISBALE dma DRQ mode */
		reg_val = readl(tdm->regs + SUNXI_DAUDIOINT);
		reg_val &= ~SUNXI_DAUDIOINT_RXDRQEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOINT);

		/*flush RX FIFO*/
		reg_val = readl(tdm->regs + SUNXI_DAUDIOFCTL);
		reg_val |= SUNXI_DAUDIOFCTL_FRX;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOFCTL);
		/*clear RX counter*/
		writel(0, tdm->regs + SUNXI_DAUDIORXCNT);
		#ifdef CONFIG_ARCH_SUN50I
		/*
		*	while flush RX FIFO, must read RXFIFO DATA three times.
		*	or it wouldn't flush RX FIFO clean; and it will let record data channel reverse!
		*/
		{
			int i = 0;
			for (i = 0; i < 9;i++) {
				reg_val = readl(tdm->regs + SUNXI_DAUDIORXFIFO);
			}
		}
		#endif
	}

	return 0;
}
EXPORT_SYMBOL(rxctrl_tdm);

int tdm_set_fmt(unsigned int fmt,struct sunxi_tdm_info *sunxi_tdm)
{
	u32 reg_val = 0;
	u32 reg_val1 = 0;
	u32 reg_val2 = 0;
	struct sunxi_tdm_info *tdm = NULL;

	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;
	/* master or slave selection */
	reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
	switch(fmt & SND_SOC_DAIFMT_MASTER_MASK){
		case SND_SOC_DAIFMT_CBM_CFM:   /* codec clk & frm master, ap is slave*/
			reg_val &= ~SUNXI_DAUDIOCTL_LRCKOUT;
			reg_val &= ~SUNXI_DAUDIOCTL_BCLKOUT;
			break;
		case SND_SOC_DAIFMT_CBS_CFS:   /* codec clk & frm slave,ap is master*/
			reg_val |= SUNXI_DAUDIOCTL_LRCKOUT;
			reg_val |= SUNXI_DAUDIOCTL_BCLKOUT;
			break;
		default:
			pr_err("unknwon master/slave format\n");
			return -EINVAL;
	}
	writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
	/* pcm or tdm mode selection */
	reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
	reg_val1 = readl(tdm->regs + SUNXI_DAUDIOTX0CHSEL);
	reg_val2 = readl(tdm->regs + SUNXI_DAUDIORXCHSEL);
	reg_val &= ~SUNXI_DAUDIOCTL_MODESEL;
	reg_val1 &= ~(SUNXI_DAUDIOTXn_OFFSET(3));
	reg_val2 &= ~(SUNXI_DAUDIORXCHSEL_RXOFFSET(3));
	switch(fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
		case SND_SOC_DAIFMT_I2S:        /* i2s mode */
			reg_val  |= (1<<4);
			reg_val1 |= SUNXI_DAUDIOTXn_OFFSET(1);
			reg_val2 |= SUNXI_DAUDIORXCHSEL_RXOFFSET(1);
			break;
		case SND_SOC_DAIFMT_RIGHT_J:    /* Right Justified mode */
			reg_val  |= (2<<4);
			break;
		case SND_SOC_DAIFMT_LEFT_J:     /* Left Justified mode */
			reg_val  |= (1<<4);
			reg_val1 |= SUNXI_DAUDIOTXn_OFFSET(0);
			reg_val2 |= SUNXI_DAUDIORXCHSEL_RXOFFSET(0);
			break;
		case SND_SOC_DAIFMT_DSP_A:      /* L data msb after FRM LRC */
			reg_val  |= (0<<4);
			break;
		case SND_SOC_DAIFMT_DSP_B:      /* L data msb during FRM LRC */
			reg_val  |= (0<<4);
			break;
		default:
			return -EINVAL;
	}
	writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
	writel(reg_val1, tdm->regs + SUNXI_DAUDIOTX0CHSEL);
	writel(reg_val2, tdm->regs + SUNXI_DAUDIORXCHSEL);
	/* DAI signal inversions */
	reg_val1 = readl(tdm->regs + SUNXI_DAUDIOFAT0);
	switch(fmt & SND_SOC_DAIFMT_INV_MASK){
		case SND_SOC_DAIFMT_NB_NF:     /* normal bit clock + frame */
			reg_val1 &= ~SUNXI_DAUDIOFAT0_BCLK_POLAYITY;
			reg_val1 &= ~SUNXI_DAUDIOFAT0_LRCK_POLAYITY;
			break;
		case SND_SOC_DAIFMT_NB_IF:     /* normal bclk + inv frm */
			reg_val1 |= SUNXI_DAUDIOFAT0_LRCK_POLAYITY;
			reg_val1 &= ~SUNXI_DAUDIOFAT0_BCLK_POLAYITY;
			break;
		case SND_SOC_DAIFMT_IB_NF:     /* invert bclk + nor frm */
			reg_val1 &= ~SUNXI_DAUDIOFAT0_LRCK_POLAYITY;
			reg_val1 |= SUNXI_DAUDIOFAT0_BCLK_POLAYITY;
			break;
		case SND_SOC_DAIFMT_IB_IF:     /* invert bclk + frm */
			reg_val1 |= SUNXI_DAUDIOFAT0_LRCK_POLAYITY;
			reg_val1 |= SUNXI_DAUDIOFAT0_BCLK_POLAYITY;
			break;
	}
	writel(reg_val1, tdm->regs + SUNXI_DAUDIOFAT0);

	return 0;
}
EXPORT_SYMBOL(tdm_set_fmt);

int tdm_hw_params(struct snd_pcm_hw_params *params,struct sunxi_tdm_info *sunxi_tdm)
{
	u32 reg_val = 0;
	//u32 sample_resolution = 0;
	struct sunxi_tdm_info *tdm = NULL;
	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;

	switch (params_format(params))
	{
	case SNDRV_PCM_FORMAT_S16_LE:
		tdm->samp_res = 16;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		tdm->samp_res = 24;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		tdm->samp_res = 24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		tdm->samp_res = 24;
		break;
	default:
		return -EINVAL;
	}
	reg_val = readl(tdm->regs + SUNXI_DAUDIOFAT0);
	reg_val &= ~SUNXI_DAUDIOFAT0_SR;
	if(tdm->samp_res == 16)
		reg_val |= (3<<4);
	else if(tdm->samp_res == 20)
		reg_val |= (4<<4);
	else
		reg_val |= (5<<4);
	writel(reg_val, tdm->regs + SUNXI_DAUDIOFAT0);

	if (tdm->samp_res == 24) {
		reg_val = readl(tdm->regs + SUNXI_DAUDIOFCTL);
		reg_val &= ~SUNXI_DAUDIOFCTL_TXIM;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOFCTL);
	} else {
		reg_val = readl(tdm->regs + SUNXI_DAUDIOFCTL);
		reg_val |= SUNXI_DAUDIOFCTL_TXIM;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOFCTL);
	}
	return 0;
}
EXPORT_SYMBOL(tdm_hw_params);

int tdm_trigger(struct snd_pcm_substream *substream,int cmd, struct sunxi_tdm_info *sunxi_tdm)
{
	s32 ret = 0;
	u32 reg_val = 0;
	struct sunxi_tdm_info *tdm = NULL;

	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;

	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				rxctrl_tdm(1,tdm);
			} else {
				txctrl_tdm(1,0,tdm);
			}
		if (daudio0_loop_en) {
			reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
			reg_val |= SUNXI_DAUDIOCTL_LOOP; /*for test*/
			writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
		}
			break;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				rxctrl_tdm(0,tdm);
			} else {
			  	txctrl_tdm(0,0,tdm);
			}
			break;
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}
EXPORT_SYMBOL(tdm_trigger);
module_param_named(daudio0_loop_en, daudio0_loop_en, bool, S_IRUGO | S_IWUSR);

int tdm_set_sysclk(unsigned int freq,struct sunxi_tdm_info *sunxi_tdm)
{
	struct sunxi_tdm_info *tdm = NULL;
	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;
	if (clk_set_rate(tdm->tdm_pllclk, freq)) {
		pr_err("try to set the tdm_pll2clk failed!\n");
	}
	return 0;
}
EXPORT_SYMBOL(tdm_set_sysclk);

int tdm_set_clkdiv(int sample_rate,struct sunxi_tdm_info *sunxi_tdm)
{
	u32 reg_val = 0;
	u32 mclk_div = 0;
	u32 bclk_div = 0;
	struct sunxi_tdm_info *tdm = NULL;

	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;

	reg_val = readl(tdm->regs + SUNXI_DAUDIOCLKD);
	/*i2s mode*/
	if (tdm->tdm_config) {
		switch (sample_rate) {
			case 192000:
			case 96000:
			case 48000:
			case 32000:
			case 24000:
			case 12000:
			case 16000:
			case 8000:
				bclk_div = ((24576000/sample_rate)/(2*tdm->pcm_lrck_period));
				mclk_div = 1;
			break;
			default:
				bclk_div = ((22579200/sample_rate)/(2*tdm->pcm_lrck_period));
				mclk_div = 1;
			break;
		}
	} else {/*pcm mode*/
		bclk_div = ((24576000/sample_rate)/(tdm->pcm_lrck_period));
		mclk_div = 1;
	}

	switch(mclk_div)
	{
		case 1: mclk_div = 1;
			break;
		case 2: mclk_div = 2;
			break;
		case 4: mclk_div = 3;
			break;
		case 6: mclk_div = 4;
			break;
		case 8: mclk_div = 5;
			break;
		case 12: mclk_div = 6;
			 break;
		case 16: mclk_div = 7;
			 break;
		case 24: mclk_div = 8;
			 break;
		case 32: mclk_div = 9;
			 break;
		case 48: mclk_div = 10;
			 break;
		case 64: mclk_div = 11;
			 break;
		case 96: mclk_div = 12;
			 break;
		case 128: mclk_div = 13;
			 break;
		case 176: mclk_div = 14;
			 break;
		case 192: mclk_div = 15;
			 break;
	}

	reg_val &= ~(0xf<<0);
	reg_val |= mclk_div<<0;
	switch(bclk_div)
	{
		case 1: bclk_div = 1;
			break;
		case 2: bclk_div = 2;
			break;
		case 4: bclk_div = 3;
			break;
		case 6: bclk_div = 4;
			break;
		case 8: bclk_div = 5;
			break;
		case 12: bclk_div = 6;
			break;
		case 16: bclk_div = 7;
			break;
		case 24: bclk_div = 8;
			break;
		case 32: bclk_div = 9;
			break;
		case 48: bclk_div = 10;
			break;
		case 64: bclk_div = 11;
			break;
		case 96: bclk_div = 12;
			break;
		case 128: bclk_div = 13;
			break;
		case 176: bclk_div = 14;
			break;
		case 192:bclk_div = 15;
	}
	reg_val &= ~(0xf<<4);
	reg_val |= bclk_div<<4;
	writel(reg_val, tdm->regs + SUNXI_DAUDIOCLKD);

	reg_val = readl(tdm->regs + SUNXI_DAUDIOFAT0);
	reg_val &= ~(0x3ff<<20);
	reg_val &= ~(0x3ff<<8);
	reg_val |= (tdm->pcm_lrck_period-1)<<8;
	reg_val |= (tdm->pcm_lrckr_period-1)<<20;
	writel(reg_val, tdm->regs + SUNXI_DAUDIOFAT0);

	reg_val = readl(tdm->regs + SUNXI_DAUDIOFAT0);
	reg_val &= ~SUNXI_DAUDIOFAT0_SW;
	if(tdm->slot_width_select == 16)
		reg_val |= (3<<0);
	else if(tdm->slot_width_select == 20)
		reg_val |= (4<<0);
	else if(tdm->slot_width_select == 24)
		reg_val |= (5<<0);
	else if(tdm->slot_width_select == 28)
		reg_val |= (6<<0);
	else
		reg_val |= (7<<0);

	/*pcm mode
	*	(Only apply in PCM mode) LRCK width
	*	0: LRCK = 1 BCLK width(short frame)
	*	1: LRCK = 2 BCLK width(long frame)
	*/
	if(tdm->frametype)
		reg_val |= SUNXI_DAUDIOFAT0_LRCK_WIDTH;
	else
		reg_val &= ~SUNXI_DAUDIOFAT0_LRCK_WIDTH;
	writel(reg_val, tdm->regs + SUNXI_DAUDIOFAT0);

	reg_val = readl(tdm->regs + SUNXI_DAUDIOFAT1);
	reg_val |= tdm->pcm_lsb_first<<7;
	reg_val |= tdm->pcm_lsb_first<<6;
	/*linear or u/a-law*/
	reg_val &= ~(0xf<<0);
	reg_val |= (tdm->tx_data_mode)<<2;
	reg_val |= (tdm->rx_data_mode)<<0;
	writel(reg_val, tdm->regs + SUNXI_DAUDIOFAT1);

	return 0;
}
EXPORT_SYMBOL(tdm_set_clkdiv);

int tdm_perpare(struct snd_pcm_substream *substream,
					struct sunxi_tdm_info *sunxi_tdm)
{
	u32 reg_val;
	struct sunxi_tdm_info *tdm = NULL;
	if (sunxi_tdm != NULL)
		tdm = sunxi_tdm;
	else
		return -1;
	/* play or record */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		reg_val = readl(tdm->regs + SUNXI_CHCFG);
		reg_val &= ~(SUNXI_TXCHCFG_TX_SLOT_NUM<<0);
		reg_val |= (substream->runtime->channels-1)<<0;
		writel(reg_val, tdm->regs + SUNXI_CHCFG);

		reg_val = readl(tdm->regs + SUNXI_DAUDIOTX0CHSEL);
		reg_val &= ~SUNXI_DAUDIOTXn_CHEN(CHEN_MASK);
		reg_val &= ~SUNXI_DAUDIOTXn_CHSEL(CHSEL_MASK);
		reg_val |= SUNXI_DAUDIOTXn_CHEN((CHEN_MASK>>(CH_MAX-substream->runtime->channels)));
		reg_val |= SUNXI_DAUDIOTXn_CHSEL(substream->runtime->channels-1);
		writel(reg_val, tdm->regs + SUNXI_DAUDIOTX0CHSEL);

#ifdef CONFIG_ARCH_SUN8IW10
		reg_val = SUNXI_TXCHANMAP0_DEFAULT;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOTX0CHMAP0);
		reg_val = SUNXI_TXCHANMAP1_DEFAULT;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOTX0CHMAP1);
#else
		reg_val = SUNXI_TXCHANMAP_DEFAULT;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOTX0CHMAP);
#endif
		/*clear TX counter*/
		writel(0, tdm->regs + SUNXI_DAUDIOTXCNT);

		/* DAUDIO TX ENABLE */
		reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
		reg_val |= SUNXI_DAUDIOCTL_TXEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
	} else {
		reg_val = readl(tdm->regs + SUNXI_CHCFG);
		reg_val &= ~SUNXI_TXCHCFG_RX_SLOT_NUM;
		reg_val |= (substream->runtime->channels-1)<<4;
		writel(reg_val, tdm->regs + SUNXI_CHCFG);

		reg_val = readl(tdm->regs + SUNXI_DAUDIORXCHSEL);
		reg_val &= ~SUNXI_DAUDIORXCHSEL_RXCHSET(CHSEL_MASK);
		reg_val |= SUNXI_DAUDIORXCHSEL_RXCHSET(substream->runtime->channels-1);
		writel(reg_val, tdm->regs + SUNXI_DAUDIORXCHSEL);

#ifdef CONFIG_ARCH_SUN8IW10
		reg_val = SUNXI_RXCHANMAP0_DEFAULT;
		writel(reg_val, tdm->regs + SUNXI_DAUDIORXCHMAP0);
		reg_val = SUNXI_RXCHANMAP1_DEFAULT;
		writel(reg_val, tdm->regs + SUNXI_DAUDIORXCHMAP1);
#else
		reg_val = SUNXI_RXCHANMAP_DEFAULT;
		writel(reg_val, tdm->regs + SUNXI_DAUDIORXCHMAP);
#endif

		reg_val = readl(tdm->regs + SUNXI_DAUDIOFCTL);
		reg_val |= SUNXI_DAUDIOFCTL_RXOM;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOFCTL);

		/*clear RX counter*/
		writel(0, tdm->regs + SUNXI_DAUDIORXCNT);

		/* DAUDIO RX ENABLE */
		reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
		reg_val |= SUNXI_DAUDIOCTL_RXEN;
		writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
	}
	return 0;
}
EXPORT_SYMBOL(tdm_perpare);

int tdm_global_enable(struct sunxi_tdm_info *sunxi_tdm,bool on)
{
	u32 reg_val = 0;
	u32 reg_val1 = 0;
	struct sunxi_tdm_info *tdm = NULL;

	if (sunxi_tdm != NULL) {
		tdm = sunxi_tdm;
	} else {
		return -1;
	}

	reg_val = readl(tdm->regs + SUNXI_DAUDIOCTL);
	reg_val1 = readl(tdm->regs + SUNXI_DAUDIOCLKD);
	if (!on) {
		reg_val &= ~SUNXI_DAUDIOCTL_GEN;
		reg_val &= ~SUNXI_DAUDIOCTL_SDO0EN;
		reg_val1 &= ~SUNXI_DAUDIOCLKD_MCLKOEN;
	} else {
		reg_val |= SUNXI_DAUDIOCTL_GEN;
		reg_val |= SUNXI_DAUDIOCTL_SDO0EN;
		reg_val1 |= SUNXI_DAUDIOCLKD_MCLKOEN;
		reg_val1 |= SUNXI_DAUDIOCLKD_MCLKDIV(1);
	}
	writel(reg_val, tdm->regs + SUNXI_DAUDIOCTL);
	writel(reg_val1, tdm->regs + SUNXI_DAUDIOCLKD);

	return 0;
}
EXPORT_SYMBOL(tdm_global_enable);
MODULE_LICENSE("GPL");

