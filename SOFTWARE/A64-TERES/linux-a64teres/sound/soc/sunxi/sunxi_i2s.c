/*
 * sound\soc\sunxi\sunxi-i2s.c
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
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <asm/dma.h>
#include <linux/dma/sunxi-dma.h>
#include "sunxi_i2s.h"
#include "sunxi_rw_func.h"
#include "sunxi_codec.h"

#define DRV_NAME "sunxi-internal-i2s"
#define SUNXI_PCM_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)

static bool  i2s_suspend 		= false;
static struct sunxi_i2s *sunxi_i2s_global = NULL ;
static bool hub_function_en = false;
static u32 sample_resolution =16;
static void sunxi_snd_txctrl(struct snd_pcm_substream *substream, int on,struct snd_soc_dai *dai)
{
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);
	/*clear TX counter*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_TXCNT, 0xffffffff, TX_CNT, 0);
	/*flush TX FIFO*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FCTL, 0x1, FTX, 1);
	if (on) {
		/* enable DMA DRQ mode for play */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_INT, 0x1, TX_DRQ, 1);
	} else {
		/* DISBALE dma DRQ mode */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_INT, 0x1, TX_DRQ, 0);
	}
}

static void sunxi_snd_rxctrl(struct snd_pcm_substream *substream, int on,struct snd_soc_dai *dai)
{
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);
	/*clear RX counter*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_RXCNT, 0xffffffff, RX_CNT, 0);
	/*flush RX FIFO*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FCTL, 0x1, FRX, 1);
	if (on) {
		/* enable DMA DRQ mode for record */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_INT, 0x1, RX_DRQ, 1);
	} else {
		/* DISBALE dma DRQ mode */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_INT, 0x1, RX_DRQ, 0);
	}
}

static int sunxi_i2s_trigger(struct snd_pcm_substream *substream,int cmd, struct snd_soc_dai *dai)
{
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (cmd) {
			case SNDRV_PCM_TRIGGER_START:
			case SNDRV_PCM_TRIGGER_RESUME:
			case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
				/*enable i2s tx*/
				sunxi_snd_txctrl(substream, 1,dai);
				return 0;
			case SNDRV_PCM_TRIGGER_SUSPEND:
			case SNDRV_PCM_TRIGGER_STOP:
			case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
				sunxi_snd_txctrl(substream, 0,dai);
				return 0;
			default:
				return -EINVAL;
			}
	} else {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			sunxi_snd_rxctrl(substream, 1,dai);
			return 0;
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			sunxi_snd_rxctrl(substream, 0,dai);
			return 0;
		default:
			pr_err("error:%s,%d\n", __func__, __LINE__);
			return -EINVAL;
		}
	}
	return 0;
}

static int sunxi_i2s_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	int rs_value  = 0;
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);
	switch (params_format(params))
	{
		case SNDRV_PCM_FORMAT_S16_LE:
			sample_resolution = 16;
			break;
		case SNDRV_PCM_FORMAT_S20_3LE:
			sample_resolution = 24;
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
			sample_resolution = 24;
			break;
		case SNDRV_PCM_FORMAT_S32_LE:
			sample_resolution = 24;
			break;
		default:
			return -EINVAL;
	}

	/* sample rate */
	switch(sample_resolution)
	{
		case 16: rs_value = 0;
			break;
		case 20: rs_value = 1;
			break;
		case 24: rs_value = 2;
			break;
		default:
			return -EINVAL;
	}
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FAT0, 0x3, SR, rs_value);

	if(sample_resolution == 24)
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FCTL, 0xf, RXOM, 0x1);
	else
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FCTL, 0xf, RXOM, 0x5);

	return 0;
}

static int sunxi_i2s_set_sysclk(struct snd_soc_dai *dai,
				  		int clk_id, unsigned int freq, int dir)
{
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);
	if (clk_set_rate(sunxi_i2s->pllclk, freq)) {
		pr_err("[audio-i2s]try to set the pll clk rate failed!\n");
	}
	return 0;
}

static int sunxi_i2s_set_clkdiv(struct snd_soc_dai *dai, int div_id, int samplerate )
{
	u32 mclk_div = 0;
	u32 bclk_div = 0;
	int wss_value = 0;
	u32 over_sample_rate = 0;
	u32 word_select_size = 32;
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);
	/*mclk div calculate*/
	switch(samplerate)
	{
		case 8000:
		{
			over_sample_rate = 128;
			mclk_div = 24;
			break;
		}
		case 16000:
		{
			over_sample_rate = 128;
			mclk_div = 12;
			break;
		}
		case 32000:
		{
			over_sample_rate = 128;
			mclk_div = 6;
			break;
		}
		case 64000:
		{
			over_sample_rate = 384;
			mclk_div = 1;
			break;
		}
		case 11025:
		case 12000:
		{
			over_sample_rate = 128;
			mclk_div = 16;
			break;
		}
		case 22050:
		case 24000:
		{
			over_sample_rate = 128;
			mclk_div = 8;
			break;
		 }
		 case 44100:
		 case 48000:
		 {
			over_sample_rate = 128;
			mclk_div = 4;
			break;
		 }
		 case 88200:
		 case 96000:
		{
			 over_sample_rate = 128;
			 mclk_div = 2;
			 break;
		 }
		 case 176400:
		 case 192000:
		 {
			 over_sample_rate = 128;
			 mclk_div = 1;
			 break;
		}

	 }

	 /*bclk div caculate*/
	 bclk_div = over_sample_rate/(2*word_select_size);
	 /*calculate MCLK Divide Ratio*/
	switch(mclk_div)
	{
		case 1: mclk_div = 0;
				 break;
		case 2: mclk_div = 1;
				 break;
		case 4: mclk_div = 2;
				 break;
		case 6: mclk_div = 3;
				 break;
		case 8: mclk_div = 4;
				 break;
		case 12: mclk_div = 5;
				 break;
		case 16: mclk_div = 6;
				 break;
		case 24: mclk_div = 7;
				 break;
		case 32: mclk_div = 8;
				 break;
		case 48: mclk_div = 9;
				 break;
		case 64: mclk_div = 0xA;
				 break;
	}
	mclk_div &= 0xf;

	/*calculate BCLK Divide Ratio*/
	switch(bclk_div)
	 {
		case 2: bclk_div = 0;
				 break;
		case 4: bclk_div = 1;
				 break;
		case 6: bclk_div = 2;
				 break;
		case 8: bclk_div = 3;
				 break;
		case 12: bclk_div = 4;
				 break;
		case 16: bclk_div = 5;
				 break;
		case 32: bclk_div = 6;
				 break;
		case 64: bclk_div = 7;
				break;
	}
	 bclk_div &= 0x7;

	 /*confige mclk and bclk dividor register*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CLKD, 0x7, BCLKDIV, bclk_div);
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CLKD, 0xf, MCLKDIV, mclk_div);
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CLKD, 0x1, 7, 1);

	/* word select size */
	switch(word_select_size)
	{
		case 16: wss_value = 0;
			break;
		case 20: wss_value = 1;
			break;
		case 24: wss_value = 2;
			break;
		case 32: wss_value = 3;
			break;
	}
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FAT0, 0x3, WSS, wss_value);

	return 0;
}

static int sunxi_i2s_set_fmt(struct snd_soc_dai *dai,unsigned int fmt)
{
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);
	/*master mode*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, MS, 0);
	/*i2s mode*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, PCM, 0);

	/* DAI signal inversions */
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FAT0, 0x1, LRCP, 0);
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FAT0, 0x1, BCP, 0);

	/*data format*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FAT0, 0x3, FMT, 0);/*standard i2s fmt*/
	/*RX FIFO trigger level*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FCTL, 0x7f, TXTL, 0x40);
	/*TX FIFO empty trigger level*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_FCTL, 0x1f, RXTL, 0x1f);
	return 0;
}

static int sunxi_i2s_preapre(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	u32 reg_val = 0;
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (substream->runtime->status->state == SNDRV_PCM_STATE_XRUN)
			pr_err("%s,SNDRV_PCM_STATE_XRUN:playback xrun.",__func__);
		reg_val = SUNXI_TXCHSEL_CHNUM(substream->runtime->channels);
		/*confige i2s ap tx channel */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_TXCHSEL, 0x7, TX_CHSEL, reg_val);
		if(substream->runtime->channels == 1) {
			reg_val = 0x00;
		} else {
			reg_val = 0x10;
		}
		/*confige i2s ap tx channel mapping*/
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_TXCHMAP, 0xff, TX_CH0_MAP, reg_val);
		pr_debug("channels:%d,SUNXI_DA_TXCHMAP:%x\n",substream->runtime->channels,readl(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_TXCHMAP));
		/*SDO ON*/
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, SDO_EN, 1);
		/* I2S0 TX ENABLE */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, TXEN, 1);
	} else {
		reg_val = SUNXI_RXCHSEL_CHNUM(substream->runtime->channels);
		/*confige i2s ap rx channel */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_RXCHSEL, 0x7, RX_CHSEL, reg_val);
		if(substream->runtime->channels == 1) {
			reg_val = 0x00;
		} else {
			reg_val = 0x10;
		}
		/*confige i2s ap rx channel mapping*/
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_RXCHMAP, 0xff, RX_CH0_MAP, reg_val);
		/* I2S0 RX ENABLE */
		codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, RXEN, 1);
	}

	return 0;
}

static struct snd_soc_dai_ops sunxi_i2s_dai_ops = {
	.trigger 	= sunxi_i2s_trigger,
	.hw_params 	= sunxi_i2s_hw_params,
	.set_fmt 	= sunxi_i2s_set_fmt,
	.set_clkdiv = sunxi_i2s_set_clkdiv,
	.set_sysclk = sunxi_i2s_set_sysclk,
	.prepare = sunxi_i2s_preapre,
};
static int set_hub_function(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	hub_function_en = ucontrol->value.integer.value[0];
	if (hub_function_en) {
	/* enable hub mode for play */
		codec_wr_control(sunxi_i2s_global->sunxi_i2s_membase+SUNXI_DA_FCTL, 0x1, HUB_EN, 1);
	} else {
		codec_wr_control(sunxi_i2s_global->sunxi_i2s_membase+SUNXI_DA_FCTL, 0x1, HUB_EN, 0);
	}
	return 0;
}

static int get_hub_function(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = hub_function_en;
	return 0;
}
static const struct snd_kcontrol_new sunxi_i2s_controls[] = {

	SOC_SINGLE_BOOL_EXT("I2S HUB FUNC", 	0, get_hub_function, 	set_hub_function),

};
static int sunxi_i2s_probe(struct snd_soc_dai *dai)
{
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(dai);

	dai->capture_dma_data = &sunxi_i2s->capture_dma_param;
	dai->playback_dma_data = &sunxi_i2s->play_dma_param;
	snd_soc_add_dai_controls(dai,sunxi_i2s_controls,
					ARRAY_SIZE(sunxi_i2s_controls));
	return 0;
}
static int sunxi_i2s_suspend(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[internal-i2s] suspend entered. %s\n", __func__);
	i2s_suspend = true;
	if (sunxi_i2s->moduleclk != NULL)
		clk_disable(sunxi_i2s->moduleclk);

	if (sunxi_i2s->pllclk != NULL)
		clk_disable(sunxi_i2s->pllclk);

	/*global disable*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, GEN, 0x0);
	pr_debug("[internal-i2s] suspend out. %s\n", __func__);
	return 0;
}

static int sunxi_i2s_resume(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_i2s *sunxi_i2s = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[internal-i2s] resume entered. %s\n", __func__);

	if (sunxi_i2s->pllclk != NULL) {
		if (clk_prepare_enable(sunxi_i2s->pllclk)) {
			pr_err("open sunxi_i2s->pllclk failed! line = %d\n", __LINE__);
		}
	}

	if (sunxi_i2s->moduleclk != NULL) {
		if (clk_prepare_enable(sunxi_i2s->moduleclk)) {
			pr_err("open sunxi_i2s->moduleclk failed! line = %d\n", __LINE__);
		}
	}

	/*global enable*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, GEN, 0x1);
	pr_debug("[internal-i2s] resume out. %s\n", __func__);
	return 0;
}

static struct snd_soc_dai_driver sunxi_pcm_dai = {
	.probe = sunxi_i2s_probe,
	.suspend 	= sunxi_i2s_suspend,
	.resume 	= sunxi_i2s_resume,
	.playback 	= {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SUNXI_PCM_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture 	= {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SUNXI_PCM_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops 		= &sunxi_i2s_dai_ops,

};
static const struct snd_soc_component_driver sunxi_i2s_component = {
	.name		= DRV_NAME,
};
static const struct of_device_id sunxi_i2s_of_match[] = {
	{ .compatible = "allwinner,sunxi-internal-i2s", },
	{},
};
static int __init sunxi_internal_i2s_platform_probe(struct platform_device *pdev)
{
	s32 ret = 0;
	const struct of_device_id *device;
	struct resource res;
	void __iomem  *sunxi_i2s_membase = NULL;
	struct sunxi_i2s *sunxi_i2s;
	struct device_node *node = pdev->dev.of_node;
	if (!node) {
		dev_err(&pdev->dev,
			"can not get dt node for this device.\n");
		ret = -EINVAL;
		goto err0;
	}
	sunxi_i2s = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_i2s), GFP_KERNEL);
	if (!sunxi_i2s) {
		dev_err(&pdev->dev, "Can't allocate sunxi_i2s.\n");
		ret = -ENOMEM;
		goto err0;
	}
	sunxi_i2s_global = sunxi_i2s;
	dev_set_drvdata(&pdev->dev, sunxi_i2s);
	sunxi_i2s->dai = sunxi_pcm_dai;
	sunxi_i2s->dai.name = dev_name(&pdev->dev);

	device = of_match_device(sunxi_i2s_of_match, &pdev->dev);
	if (!device)
		return -ENODEV;
	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}

	sunxi_i2s_membase =ioremap(res.start, resource_size(&res));
	if (NULL ==sunxi_i2s_membase) {
		dev_dbg(&pdev->dev, "could not remap register memory\n");
		return -ENOMEM;
	} else {
		sunxi_i2s->sunxi_i2s_membase = sunxi_i2s_membase;
	}
	sunxi_i2s->pllclk = of_clk_get(node, 0);
	sunxi_i2s->moduleclk= of_clk_get(node, 1);
	if (IS_ERR(sunxi_i2s->pllclk) || IS_ERR(sunxi_i2s->moduleclk)){
		dev_err(&pdev->dev, "[audio-i2s]Can't get i2s clocks\n");
		if (IS_ERR(sunxi_i2s->pllclk))
			ret = PTR_ERR(sunxi_i2s->pllclk);
		else
			ret = PTR_ERR(sunxi_i2s->moduleclk);
		goto err1;
	} else {
		if (clk_set_parent(sunxi_i2s->moduleclk, sunxi_i2s->pllclk)) {
			pr_err("try to set parent of sunxi_spdif->moduleclk to sunxi_spdif->pllclk failed! line = %d\n",__LINE__);
		}
		clk_prepare_enable(sunxi_i2s->pllclk);
		clk_prepare_enable(sunxi_i2s->moduleclk);
	}
	sunxi_i2s->play_dma_param.dma_addr = res.start+SUNXI_DA_TXFIFO;
	sunxi_i2s->play_dma_param.dma_drq_type_num = DRQDST_AUDIO_CODEC;
	sunxi_i2s->play_dma_param.dst_maxburst = 4;
	sunxi_i2s->play_dma_param.src_maxburst = 4;

	sunxi_i2s->capture_dma_param.dma_addr = res.start+SUNXI_DA_RXFIFO;
	sunxi_i2s->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;
	sunxi_i2s->capture_dma_param.src_maxburst = 4;
	sunxi_i2s->capture_dma_param.dst_maxburst = 4;

	ret = snd_soc_register_component(&pdev->dev, &sunxi_i2s_component,
				   &sunxi_i2s->dai, 1);
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
	/*global enable*/
	codec_wr_control(sunxi_i2s->sunxi_i2s_membase+SUNXI_DA_CTL, 0x1, GEN, 0x1);
	return 0;

err2:
	snd_soc_unregister_component(&pdev->dev);
err1:
	iounmap(sunxi_i2s->sunxi_i2s_membase);
err0:
	return ret;

}

static int sunxi_internal_i2s_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sunxi_internal_i2s_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_i2s_of_match,
	},
	.probe = sunxi_internal_i2s_platform_probe,
	.remove = sunxi_internal_i2s_platform_remove,
};
module_platform_driver(sunxi_internal_i2s_driver);
module_param_named(i2s_suspend, i2s_suspend, bool, S_IRUGO | S_IWUSR);
/* Module information */
MODULE_AUTHOR("REUUIMLLA");
MODULE_DESCRIPTION("sunxi i2s-internal SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);


