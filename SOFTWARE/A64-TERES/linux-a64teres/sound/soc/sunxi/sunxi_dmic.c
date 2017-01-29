/*
 * sound\soc\sunxi\sunxi_dmic.c
 * (C) Copyright 2014-2016
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
#include "sunxi_dmic.h"
#include <linux/regulator/consumer.h>

#define DRV_NAME "sunxi-dmic"

struct dmic_rate {
	unsigned int samplerate;
	unsigned int rate_bit;
};
struct dmic_ch_en {
	unsigned int channel;
	unsigned int chan_bit;
};

static const struct dmic_rate dmic_rate_s[] = {
	{44100, 0x0},
	{48000, 0x0},
	{22050, 0x2},
	{24000, 0x2},
	{11025, 0x4},
	{12000, 0x4},
	{32000, 0x1},
	{16000, 0x3},
	{8000, 0x5},
};

static const struct dmic_ch_en dmic_ch_en_s[] = {
	{1, 0x1},
	{2, 0x3},
	{3, 0x7},
	{4, 0xf},
	{5, 0x1f},
	{6, 0x3f},
	{7, 0x7f},
	{8, 0xff},
};

static void dmic_rxctrl_enable(int rx_en,struct sunxi_dmic_info *sunxi_dmic)
{
	u32 reg_val;

	/*flush RX FIFO*/
	reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);
	reg_val |= (0x1<<DMIC_FIFO_FLUSH);
	writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);

	if (rx_en) {
		/*DRQ ENABLE*/
		reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_INTC);
		reg_val |= (0x1<<FIFO_DRQ_EN);
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_INTC);

		/*global enable*/
		reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_EN);
		reg_val |= (1<<GLOBE_EN);
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_EN);
	} else {
		/*DRQ DISABLE*/
		reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_INTC);
		reg_val &= ~(0x1<<FIFO_DRQ_EN);
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_INTC);

		/*global enable*/
		reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_EN);
		reg_val &= ~(1<<GLOBE_EN);
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_EN);
	}
}

static int sunxi_dmic_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params,
						struct snd_soc_dai *dai)
{
	int format = 0, format_bit = 0;
	int reg_val = 0;
	struct sunxi_dmic_info *sunxi_dmic = snd_soc_dai_get_drvdata(dai);

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
	if (format == 16) {
		format_bit = 0;
	} else {
		format_bit = 1;
	}
	reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);
	reg_val |= (format_bit<<SAMPLE_RESOLUTION);
	writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);

	if (format == 24) {
		reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);
		reg_val &= ~(1<<FIFO_MODE);
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);
	} else {
		reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);
		reg_val |= (1<<FIFO_MODE);
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_FIFO_CTR);
	}

	reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_FIFO_STA);
	reg_val |= (0xff<<DMIC_DATA_CNT);
	writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_FIFO_STA);

	return 0;
}

static int sunxi_dmic_trigger(struct snd_pcm_substream *substream,
                              int cmd, struct snd_soc_dai *dai)
{
	int ret = 0;
	struct sunxi_dmic_info *sunxi_dmic = snd_soc_dai_get_drvdata(dai);

	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				dmic_rxctrl_enable(1,sunxi_dmic);
			}
			break;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				dmic_rxctrl_enable(0,sunxi_dmic);
			}
			break;
		default:
			ret = -EINVAL;
			break;
	}
	return ret;
}

static int sunxi_dmic_perpare(struct snd_pcm_substream *substream,
	struct snd_soc_dai *cpu_dai)
{
	int i = 0;
	int dmic_vol = 0xb0;
	int reg_val = 0x0;
	struct sunxi_dmic_info *sunxi_dmic = snd_soc_dai_get_drvdata(cpu_dai);

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		for (i = 0; i < ARRAY_SIZE(dmic_rate_s); i++) {
			if (dmic_rate_s[i].samplerate == substream->runtime->rate) {
				reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_SR);
				reg_val &= ~(0x7<<DMIC_SR);
				reg_val |= (dmic_rate_s[i].rate_bit<<DMIC_SR);
				writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_SR);
			}
		}
		if (substream->runtime->rate > 24000) {
			reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_CTR);
			reg_val &= ~(0x3<<DMIC_OVERSAMPLE_RATE);
			reg_val |= (0x1<<DMIC_OVERSAMPLE_RATE);
			writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_CTR);
		} else {
			reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_CTR);
			reg_val &= ~(0x3<<DMIC_OVERSAMPLE_RATE);
			reg_val |= (0x0<<DMIC_OVERSAMPLE_RATE);
			writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_CTR);
		}
		for (i = 0; i < ARRAY_SIZE(dmic_ch_en_s); i++) {
			if (dmic_ch_en_s[i].channel ==  substream->runtime->channels) {
				reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_EN);
				reg_val &= ~(0xff<<DATA_CH_EN);
				reg_val |= (dmic_ch_en_s[i].chan_bit<<DATA_CH_EN);
				writel(reg_val, sunxi_dmic->regs + DATA_CH_EN);
			}
		}

		/*dmic enable channel numbers are N+1*/		
		reg_val = readl(sunxi_dmic->regs + SUNXI_DMIC_CH_NUM);
		reg_val &= ~(0xf<<DMIC_CH_NUM);
		reg_val |= ((substream->runtime->channels-1)<<DMIC_CH_NUM);
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_CH_NUM);		

		reg_val = DMIC_CHANMAP_DEFAULT;
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_CH_MAP);

		/*dmic vol ctrl*/
		reg_val = 0x0;
		reg_val |= ((dmic_vol<<DATA0R_VOL)|(dmic_vol<<DATA0L_VOL)|(dmic_vol<<DATA1R_VOL)|(dmic_vol<<DATA1L_VOL));
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_DATA0_1_VOL);
		reg_val = 0x0;
		reg_val |= ((dmic_vol<<DATA2R_VOL)|(dmic_vol<<DATA2L_VOL)|(dmic_vol<<DATA3R_VOL)|(dmic_vol<<DATA3L_VOL));
		writel(reg_val, sunxi_dmic->regs + SUNXI_DMIC_DATA2_3_VOL);
	}
	return 0;
}

static int sunxi_dmic_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id,
                                 unsigned int freq, int dir)
{
	struct sunxi_dmic_info *sunxi_dmic = snd_soc_dai_get_drvdata(cpu_dai);

	if (clk_set_rate(sunxi_dmic->pllclk, freq)) {
		pr_err("try to set the dmic_pll rate failed!\n");
	}

	return 0;
}

static int sunxi_dmic_dai_probe(struct snd_soc_dai *dai)
{
	struct sunxi_dmic_info *sunxi_dmic = snd_soc_dai_get_drvdata(dai);
	dai->capture_dma_data = &sunxi_dmic->capture_dma_param;

	return 0;
}

static int sunxi_dmic_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static int sunxi_dmic_suspend(struct snd_soc_dai *cpu_dai)
{
	u32 ret = 0;
	struct sunxi_dmic_info *sunxi_dmic = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[DMIC]Enter %s\n", __func__);

	if (NULL != sunxi_dmic->pinstate_sleep) {
		ret = pinctrl_select_state(sunxi_dmic->pinctrl, sunxi_dmic->pinstate_sleep);
		if (ret) {
			pr_warn("[dmic]select pin sleep state failed\n");
			return ret;
		}
	}
	if (sunxi_dmic->pinctrl !=NULL)
		devm_pinctrl_put(sunxi_dmic->pinctrl);
	sunxi_dmic->pinctrl = NULL;
	sunxi_dmic->pinstate = NULL;
	sunxi_dmic->pinstate_sleep = NULL;
	pr_debug("[DMIC]sunxi_dmic->clk_enable_cnt:%d,%s\n",sunxi_dmic->clk_enable_cnt, __func__);
	if (sunxi_dmic->clk_enable_cnt > 0) {
		if (sunxi_dmic->moduleclk != NULL) {
			clk_disable(sunxi_dmic->moduleclk);
		}
		if (sunxi_dmic->pllclk != NULL) {
			clk_disable(sunxi_dmic->pllclk);
		}
		sunxi_dmic->clk_enable_cnt--;
	}
	pr_debug("[DMIC]End %s\n", __func__);
	return 0;
}

static int sunxi_dmic_resume(struct snd_soc_dai *cpu_dai)
{
	s32 ret = 0;
	struct sunxi_dmic_info *sunxi_dmic = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[DMIC]Enter %s\n", __func__);

	if (sunxi_dmic->pllclk != NULL) {
		if (clk_prepare_enable(sunxi_dmic->pllclk)) {
			pr_err("open sunxi_dmic->pllclk failed! line = %d\n", __LINE__);
		}
	}

	if (sunxi_dmic->moduleclk != NULL) {
		if (clk_prepare_enable(sunxi_dmic->moduleclk)) {
			pr_err("open sunxi_dmic->moduleclk failed! line = %d\n", __LINE__);
		}
	}
	sunxi_dmic->clk_enable_cnt++;
	pr_debug("[DMIC]sunxi_dmic->clk_enable_cnt:%d,%s\n",sunxi_dmic->clk_enable_cnt, __func__);

	if (!sunxi_dmic->pinctrl) {
		sunxi_dmic->pinctrl = devm_pinctrl_get(cpu_dai->dev);
		if (IS_ERR_OR_NULL(sunxi_dmic->pinctrl)) {
			pr_warn("[dmic]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_dmic->pinstate){
		sunxi_dmic->pinstate = pinctrl_lookup_state(sunxi_dmic->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_dmic->pinstate)) {
			pr_warn("[dmic]lookup pin default state failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_dmic->pinstate_sleep){
		sunxi_dmic->pinstate_sleep = pinctrl_lookup_state(sunxi_dmic->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_dmic->pinstate_sleep)) {
			pr_warn("[dmic]lookup pin sleep state failed\n");
			return -EINVAL;
		}
	}

	ret = pinctrl_select_state(sunxi_dmic->pinctrl, sunxi_dmic->pinstate);
	if (ret) {
		pr_warn("[dmic]select pin default state failed\n");
		return ret;
	}
	pr_debug("[DMIC]End %s\n", __func__);
	return 0;
}

#define SUNXI_DMIC_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)
static struct snd_soc_dai_ops sunxi_dmic_dai_ops = {
	.trigger 	= sunxi_dmic_trigger,
	.hw_params 	= sunxi_dmic_hw_params,
	.prepare  =	sunxi_dmic_perpare,
	.set_sysclk = sunxi_dmic_set_sysclk,
};

static struct snd_soc_dai_driver sunxi_dmic_dai = {
	.probe 		= sunxi_dmic_dai_probe,
	.suspend 	= sunxi_dmic_suspend,
	.resume 	= sunxi_dmic_resume,
	.remove 	= sunxi_dmic_dai_remove,
	.playback = {
		.channels_min = 1,
		.channels_max = 8,
		.rates = SUNXI_DMIC_RATES,
	.formats = SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S20_3LE| SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.capture = {
		.channels_min = 1,
		.channels_max = 8,
		.rates = SUNXI_DMIC_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S20_3LE| SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.ops = &sunxi_dmic_dai_ops,
};
static const struct snd_soc_component_driver sunxi_dmic_component = {
	.name		= DRV_NAME,
};
static const struct of_device_id sunxi_dmic_of_match[] = {
	{ .compatible = "allwinner,sunxi-dmic", },
	{},
};

static int __init sunxi_dmic_dev_probe(struct platform_device *pdev)
{
	u32 ret = 0;
	struct resource res;
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	void __iomem  *sunxi_dmic_membase = NULL;
	struct sunxi_dmic_info *sunxi_dmic;

	sunxi_dmic = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_dmic_info), GFP_KERNEL);
	if (!sunxi_dmic) {
		dev_err(&pdev->dev, "Can't allocate sunxi_dmic\n");
		ret = -ENOMEM;
		goto err0;
	}
	pr_debug("[audio-dmic] platform initial.\n");
	dev_set_drvdata(&pdev->dev, sunxi_dmic);
	sunxi_dmic->dai = sunxi_dmic_dai;
	sunxi_dmic->dai.name = dev_name(&pdev->dev);

	device = of_match_device(sunxi_dmic_of_match, &pdev->dev);
	if (!device)
		return -ENODEV;
	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}

	sunxi_dmic_membase =ioremap(res.start, resource_size(&res));
	if (NULL == sunxi_dmic_membase) {
		pr_err("[audio-dmic]Can't map dmic registers\n");
	} else {
		sunxi_dmic->regs = sunxi_dmic_membase;
	}
	sunxi_dmic->pllclk = of_clk_get(node, 0);
	sunxi_dmic->moduleclk= of_clk_get(node, 1);
	if (IS_ERR(sunxi_dmic->pllclk) || IS_ERR(sunxi_dmic->moduleclk)){
		dev_err(&pdev->dev, "[audio-dmic]Can't get dmic clocks\n");
		if (IS_ERR(sunxi_dmic->pllclk))
			ret = PTR_ERR(sunxi_dmic->pllclk);
		else
			ret = PTR_ERR(sunxi_dmic->moduleclk);
		goto err1;
	} else {
		if (clk_set_parent(sunxi_dmic->moduleclk, sunxi_dmic->pllclk)) {
			pr_err("try to set parent of sunxi_dmic->moduleclk to sunxi_dmic->pllclk failed! line = %d\n",__LINE__);
		}
		clk_prepare_enable(sunxi_dmic->pllclk);
		clk_prepare_enable(sunxi_dmic->moduleclk);
		sunxi_dmic->clk_enable_cnt++;
	}

	sunxi_dmic->capture_dma_param.dma_addr = res.start + SUNXI_DMIC_DATA;
	sunxi_dmic->capture_dma_param.dma_drq_type_num = DRQSRC_DMIC_RX;
	sunxi_dmic->capture_dma_param.src_maxburst = 8;
	sunxi_dmic->capture_dma_param.dst_maxburst = 8;

	sunxi_dmic->pinctrl = NULL;
	if (!sunxi_dmic->pinctrl) {
		sunxi_dmic->pinctrl = devm_pinctrl_get(&pdev->dev);
		if (IS_ERR_OR_NULL(sunxi_dmic->pinctrl)) {
			pr_warn("[dmic]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_dmic->pinstate){
		sunxi_dmic->pinstate = pinctrl_lookup_state(sunxi_dmic->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_dmic->pinstate)) {
			pr_warn("[dmic]lookup pin default state failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_dmic->pinstate_sleep){
		sunxi_dmic->pinstate_sleep = pinctrl_lookup_state(sunxi_dmic->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_dmic->pinstate_sleep)) {
			pr_warn("[dmic]lookup pin sleep state failed\n");
			return -EINVAL;
		}
	}

	ret = snd_soc_register_component(&pdev->dev, &sunxi_dmic_component,
				   &sunxi_dmic->dai, 1);
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

	return 0;
err2:
	snd_soc_unregister_component(&pdev->dev);
err1:
	iounmap(sunxi_dmic->regs);
err0:
	return ret;
}

static int __exit sunxi_dmic_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver sunxi_dmic_driver = {
	.probe = sunxi_dmic_dev_probe,
	.remove = __exit_p(sunxi_dmic_dev_remove),
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_dmic_of_match,
	},
};

static int __init sunxi_dmic_init(void)
{
	return platform_driver_register(&sunxi_dmic_driver);
}
module_init(sunxi_dmic_init);

static void __exit sunxi_dmic_exit(void)
{
	platform_driver_unregister(&sunxi_dmic_driver);
}
module_exit(sunxi_dmic_exit);

/* Module information */
MODULE_AUTHOR("HUANGXIN");
MODULE_DESCRIPTION("sunxi DMIC SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-dmic");
