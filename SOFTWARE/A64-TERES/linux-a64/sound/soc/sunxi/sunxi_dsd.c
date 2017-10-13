/*
 * sound\soc\sunxi\sunxi_dsd.c
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
#include "sunxi_dsd.h"
#include <linux/regulator/consumer.h>

#define DRV_NAME "sunxi-dsd"

struct dsd_rate {
	unsigned int samplerate;
	unsigned int dsdrate;
	unsigned int rate_bit;
};
struct dsd_data_format {
	unsigned int format;
	unsigned int format_bit;
};

static const struct dsd_rate dsd_rate_s[] = {
	{44100, 64,  0x0},
	{48000, 64,  0x0},
	{96000, 128, 0x1},
	{192000, 256, 0x2},
};

static const struct dsd_data_format dsd_data_format_s[] = {
	{8, 0x0},
	{12, 0x1},
	{16, 0x2},
	{20, 0x3},
	{24, 0x4},
	{32, 0x5},
};

static void dsd_txctrl_enable(int tx_en,struct sunxi_dsd_info *sunxi_dsd)
{
	u32 reg_val;

	/*flush TX FIFO*/
	reg_val = readl(sunxi_dsd->regs + DSD_TX_FIFO_CTRL);
	reg_val |= (0x1<<TX_FIFO_FLUSH);
	writel(reg_val, sunxi_dsd->regs + DSD_TX_FIFO_CTRL);

	reg_val = readl(sunxi_dsd->regs + DSD_TX_FIFO_CTRL);
	reg_val |= (0x68<<TX_FIFO_TRIG_LEVEL);
	writel(reg_val, sunxi_dsd->regs + DSD_TX_FIFO_CTRL);
	if (tx_en) {
		/*DRQ ENABLE*/
		reg_val = readl(sunxi_dsd->regs + DSD_INT_CTRL);
		reg_val |= (0x1<<TX_FIFO_DRQ_EN);
		writel(reg_val, sunxi_dsd->regs + DSD_INT_CTRL);

		reg_val = readl(sunxi_dsd->regs + DSD_EN_CTRL);
		reg_val |= (1<<TX_EN);
		writel(reg_val, sunxi_dsd->regs + DSD_EN_CTRL);

		/*global enable*/
		reg_val = readl(sunxi_dsd->regs + DSD_EN_CTRL);
		reg_val |= (1<<GLOBAL_EN);
		writel(reg_val, sunxi_dsd->regs + DSD_EN_CTRL);
	} else {
		/*DRQ DISABLE*/
		reg_val = readl(sunxi_dsd->regs + DSD_INT_CTRL);
		reg_val &= ~(0x1<<TX_FIFO_DRQ_EN);
		writel(reg_val, sunxi_dsd->regs + DSD_INT_CTRL);

		reg_val = readl(sunxi_dsd->regs + DSD_EN_CTRL);
		reg_val &= ~(1<<TX_EN);
		writel(reg_val, sunxi_dsd->regs + DSD_EN_CTRL);

		/*global enable*/
		reg_val = readl(sunxi_dsd->regs + DSD_EN_CTRL);
		reg_val &= ~(1<<GLOBAL_EN);
		writel(reg_val, sunxi_dsd->regs + DSD_EN_CTRL);
	}
}

static int sunxi_dsd_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params,
						struct snd_soc_dai *dai)
{
	int format = 0;
	int reg_val = 0, i = 0;
	struct sunxi_dsd_info *sunxi_dsd = snd_soc_dai_get_drvdata(dai);

	switch (params_format(params))
	{
		case SNDRV_PCM_FORMAT_S8:
		format = 8;
		break;
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
		format = 32;
		break;
		default:
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(dsd_data_format_s); i++) {
		if (dsd_data_format_s[i].format == format) {
			reg_val = readl(sunxi_dsd->regs + DSD_TX_CONF);
			reg_val &= ~(0x7<<DSD_TX_DATA_WIDTH);
			reg_val |= (dsd_data_format_s[i].format_bit<<DSD_TX_DATA_WIDTH);
			writel(reg_val, sunxi_dsd->regs + DSD_TX_CONF);
		}
	}

	if ((format == 24)||(format == 32)) {
		reg_val = readl(sunxi_dsd->regs + DSD_TX_FIFO_CTRL);
		reg_val &= ~(1<<TXIM);
		writel(reg_val, sunxi_dsd->regs + DSD_TX_FIFO_CTRL);
	} else {
		reg_val = readl(sunxi_dsd->regs + DSD_TX_FIFO_CTRL);
		reg_val |= (1<<TXIM);
		writel(reg_val, sunxi_dsd->regs + DSD_TX_FIFO_CTRL);
	}

	/*use LSB and Trailing Edge of the PLCK*/
	reg_val = readl(sunxi_dsd->regs + DSD_TX_CONF);
	reg_val |= (0<<MSB_LSB_FIR_SEL) | (1<<DSD_TX_DRIVER_MODE);
	writel(reg_val, sunxi_dsd->regs + DSD_TX_CONF);

	return 0;
}

static int sunxi_dsd_trigger(struct snd_pcm_substream *substream,
                              int cmd, struct snd_soc_dai *dai)
{
	int ret = 0;
	struct sunxi_dsd_info *sunxi_dsd = snd_soc_dai_get_drvdata(dai);

	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				dsd_txctrl_enable(1,sunxi_dsd);
			}
			break;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				dsd_txctrl_enable(0,sunxi_dsd);
			}
			break;
		default:
			ret = -EINVAL;
			break;
	}
	return ret;
}

static int sunxi_dsd_perpare(struct snd_pcm_substream *substream,
	struct snd_soc_dai *cpu_dai)
{
	int i = 0, reg_val = 0;
	struct sunxi_dsd_info *sunxi_dsd = snd_soc_dai_get_drvdata(cpu_dai);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		for (i = 0; i < ARRAY_SIZE(dsd_rate_s); i++) {
			if (dsd_rate_s[i].samplerate == substream->runtime->rate) {
				reg_val = readl(sunxi_dsd->regs + DSD_SR_CTRL);
				reg_val &= ~(0x3<<DSD_SAMPLE_RATE);
				reg_val |= (dsd_rate_s[i].rate_bit<<DSD_SAMPLE_RATE);
				writel(reg_val, sunxi_dsd->regs + DSD_SR_CTRL);
			}
		}

		reg_val = readl(sunxi_dsd->regs + DSD_TX_CONF);
		reg_val &= ~(0x3<<DSD_TX_CHAN_EN);
		if (substream->runtime->channels==1) {
			reg_val |= (substream->runtime->channels<<DSD_TX_CHAN_EN);
		} else {
			reg_val |= (0x3<<DSD_TX_CHAN_EN);
		}
		writel(reg_val, sunxi_dsd->regs + DSD_TX_CONF);

		reg_val = DSD_CHANMAP_DEFAULT;
		writel(reg_val, sunxi_dsd->regs + DSD_TX_MAP);
	}
	return 0;
}

static int sunxi_dsd_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id,
                                 unsigned int freq, int dir)
{
	struct sunxi_dsd_info *sunxi_dsd = snd_soc_dai_get_drvdata(cpu_dai);

	if (clk_set_rate(sunxi_dsd->pllclk, freq)) {
		pr_err("try to set the dsd_pll rate failed!\n");
	}

	return 0;
}

static int sunxi_dsd_dai_probe(struct snd_soc_dai *dai)
{
	struct sunxi_dsd_info *sunxi_dsd = snd_soc_dai_get_drvdata(dai);

	dai->playback_dma_data = &sunxi_dsd->play_dma_param;

	return 0;
}

static int sunxi_dsd_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static int sunxi_dsd_suspend(struct snd_soc_dai *cpu_dai)
{
	u32 ret = 0;
	struct sunxi_dsd_info *sunxi_dsd = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[DSD]Enter %s\n", __func__);

	if (NULL != sunxi_dsd->pinstate_sleep) {
		ret = pinctrl_select_state(sunxi_dsd->pinctrl, sunxi_dsd->pinstate_sleep);
		if (ret) {
			pr_warn("[dsd]select pin sleep state failed\n");
			return ret;
		}
	}
	if (sunxi_dsd->pinctrl !=NULL)
		devm_pinctrl_put(sunxi_dsd->pinctrl);
	sunxi_dsd->pinctrl = NULL;
	sunxi_dsd->pinstate = NULL;
	sunxi_dsd->pinstate_sleep = NULL;
	pr_debug("[DSD]sunxi_dsd->clk_enable_cnt:%d,%s\n",sunxi_dsd->clk_enable_cnt, __func__);
	if (sunxi_dsd->clk_enable_cnt > 0) {
		if (sunxi_dsd->moduleclk != NULL) {
			clk_disable(sunxi_dsd->moduleclk);
		}
		if (sunxi_dsd->pllclk != NULL) {
			clk_disable(sunxi_dsd->pllclk);
		}
		sunxi_dsd->clk_enable_cnt--;
	}
	pr_debug("[DSD]End %s\n", __func__);
	return 0;
}

static int sunxi_dsd_resume(struct snd_soc_dai *cpu_dai)
{
	s32 ret = 0;
	int reg_val = 0;
	struct sunxi_dsd_info *sunxi_dsd = snd_soc_dai_get_drvdata(cpu_dai);
	pr_debug("[DSD]Enter %s\n", __func__);

	if (sunxi_dsd->pllclk != NULL) {
		if (clk_prepare_enable(sunxi_dsd->pllclk)) {
			pr_err("open sunxi_dsd->pllclk failed! line = %d\n", __LINE__);
		}
	}

	if (sunxi_dsd->moduleclk != NULL) {
		if (clk_prepare_enable(sunxi_dsd->moduleclk)) {
			pr_err("open sunxi_dsd->moduleclk failed! line = %d\n", __LINE__);
		}
	}
	sunxi_dsd->clk_enable_cnt++;
	pr_debug("[DSD]sunxi_dsd->clk_enable_cnt:%d,%s\n",sunxi_dsd->clk_enable_cnt, __func__);

	reg_val = readl(sunxi_dsd->regs + DSD_TX_CONF);
	if (!sunxi_dsd->mode_select) {
		reg_val &= ~DSD_TX_MODE_SELECT;
	} else {
		reg_val |= DSD_TX_MODE_SELECT;
	}
	writel(reg_val, sunxi_dsd->regs + DSD_TX_CONF);

	if (!sunxi_dsd->pinctrl) {
		sunxi_dsd->pinctrl = devm_pinctrl_get(cpu_dai->dev);
		if (IS_ERR_OR_NULL(sunxi_dsd->pinctrl)) {
			pr_warn("[dsd]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_dsd->pinstate){
		sunxi_dsd->pinstate = pinctrl_lookup_state(sunxi_dsd->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_dsd->pinstate)) {
			pr_warn("[dsd]lookup pin default state failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_dsd->pinstate_sleep){
		sunxi_dsd->pinstate_sleep = pinctrl_lookup_state(sunxi_dsd->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_dsd->pinstate_sleep)) {
			pr_warn("[dsd]lookup pin sleep state failed\n");
			return -EINVAL;
		}
	}

	ret = pinctrl_select_state(sunxi_dsd->pinctrl, sunxi_dsd->pinstate);
	if (ret) {
		pr_warn("[dsd]select pin default state failed\n");
		return ret;
	}
	pr_debug("[DSD]End %s\n", __func__);
	return 0;
}

#define SUNXI_DSD_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)
static struct snd_soc_dai_ops sunxi_dsd_dai_ops = {
	.trigger 	= sunxi_dsd_trigger,
	.hw_params 	= sunxi_dsd_hw_params,
	.prepare  =	sunxi_dsd_perpare,
	.set_sysclk = sunxi_dsd_set_sysclk,
};

static struct snd_soc_dai_driver sunxi_dsd_dai = {
	.probe 		= sunxi_dsd_dai_probe,
	.suspend 	= sunxi_dsd_suspend,
	.resume 	= sunxi_dsd_resume,
	.remove 	= sunxi_dsd_dai_remove,
	.playback = {
		.channels_min = 1,
		.channels_max = 8,
		.rates = SUNXI_DSD_RATES,
	.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S20_3LE| SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.capture = {
		.channels_min = 1,
		.channels_max = 8,
		.rates = SUNXI_DSD_RATES,
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S20_3LE| SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.ops = &sunxi_dsd_dai_ops,
};
static const struct snd_soc_component_driver sunxi_dsd_component = {
	.name		= DRV_NAME,
};
static const struct of_device_id sunxi_dsd_of_match[] = {
	{ .compatible = "allwinner,sunxi-dsd", },
	{},
};

static int __init sunxi_dsd_dev_probe(struct platform_device *pdev)
{
	u32 ret = 0;
	u32 temp_val = 0;
	int reg_val = 0;
	struct resource res;
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	void __iomem  *sunxi_dsd_membase = NULL;
	struct sunxi_dsd_info *sunxi_dsd;

	sunxi_dsd = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_dsd_info), GFP_KERNEL);
	if (!sunxi_dsd) {
		dev_err(&pdev->dev, "Can't allocate sunxi_dsd\n");
		ret = -ENOMEM;
		goto err0;
	}
	pr_debug("[audio-dsd] platform initial.\n");

	dev_set_drvdata(&pdev->dev, sunxi_dsd);
	sunxi_dsd->dai = sunxi_dsd_dai;
	sunxi_dsd->dai.name = dev_name(&pdev->dev);

	device = of_match_device(sunxi_dsd_of_match, &pdev->dev);
	if (!device)
		return -ENODEV;
	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}

	sunxi_dsd_membase =ioremap(res.start, resource_size(&res));
	if (NULL == sunxi_dsd_membase) {
		pr_err("[audio-dsd]Can't map dsd registers\n");
	} else {
		sunxi_dsd->regs = sunxi_dsd_membase;
	}
	sunxi_dsd->pllclk = of_clk_get(node, 0);
	sunxi_dsd->moduleclk= of_clk_get(node, 1);
	if (IS_ERR(sunxi_dsd->pllclk) || IS_ERR(sunxi_dsd->moduleclk)){
		dev_err(&pdev->dev, "[audio-dsd]Can't get dsd clocks\n");
		if (IS_ERR(sunxi_dsd->pllclk))
			ret = PTR_ERR(sunxi_dsd->pllclk);
		else
			ret = PTR_ERR(sunxi_dsd->moduleclk);
		goto err1;
	} else {
		if (clk_set_parent(sunxi_dsd->moduleclk, sunxi_dsd->pllclk)) {
			pr_err("try to set parent of sunxi_dsd->moduleclk to sunxi_dsd->pllclk failed! line = %d\n",__LINE__);
		}
		clk_prepare_enable(sunxi_dsd->pllclk);
		clk_prepare_enable(sunxi_dsd->moduleclk);
		sunxi_dsd->clk_enable_cnt++;
	}

	sunxi_dsd->play_dma_param.dma_addr = res.start + DSD_TX_DATA;
	sunxi_dsd->play_dma_param.dma_drq_type_num = DRQDST_DSD_TX;
	sunxi_dsd->play_dma_param.dst_maxburst = 8;
	sunxi_dsd->play_dma_param.src_maxburst = 8;
	sunxi_dsd->pinctrl = NULL;
	if (!sunxi_dsd->pinctrl) {
		sunxi_dsd->pinctrl = devm_pinctrl_get(&pdev->dev);
		if (IS_ERR_OR_NULL(sunxi_dsd->pinctrl)) {
			pr_warn("[dsd]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_dsd->pinstate) {
		sunxi_dsd->pinstate = pinctrl_lookup_state(sunxi_dsd->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_dsd->pinstate)) {
			pr_warn("[dsd]lookup pin default state failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_dsd->pinstate_sleep) {
		sunxi_dsd->pinstate_sleep = pinctrl_lookup_state(sunxi_dsd->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_dsd->pinstate_sleep)) {
			pr_warn("[dsd]lookup pin sleep state failed\n");
			return -EINVAL;
		}
	}
	/*
	*	DSD TX Mode Select
	*	0: Normal Mode
	*	1: Phase Modulation Mode
	*/
	ret = of_property_read_u32(node, "mode_select",&temp_val);
	if (ret < 0) {
		pr_err("[dsd]mode_select configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_dsd->mode_select = temp_val;
	}
	reg_val = readl(sunxi_dsd->regs + DSD_TX_CONF);
	if (!sunxi_dsd->mode_select) {
		reg_val &= ~DSD_TX_MODE_SELECT;
	} else {
		reg_val |= DSD_TX_MODE_SELECT;
	}
	writel(reg_val, sunxi_dsd->regs + DSD_TX_CONF);

	ret = snd_soc_register_component(&pdev->dev, &sunxi_dsd_component,
				   &sunxi_dsd->dai, 1);
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
	iounmap(sunxi_dsd->regs);
err0:
	return ret;
}

static int __exit sunxi_dsd_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver sunxi_dsd_driver = {
	.probe = sunxi_dsd_dev_probe,
	.remove = __exit_p(sunxi_dsd_dev_remove),
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_dsd_of_match,
	},
};

static int __init sunxi_dsd_init(void)
{
	return platform_driver_register(&sunxi_dsd_driver);
}
module_init(sunxi_dsd_init);

static void __exit sunxi_dsd_exit(void)
{
	platform_driver_unregister(&sunxi_dsd_driver);
}
module_exit(sunxi_dsd_exit);

/* Module information */
MODULE_AUTHOR("HUANGXIN");
MODULE_DESCRIPTION("sunxi DSD SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-dsd");
