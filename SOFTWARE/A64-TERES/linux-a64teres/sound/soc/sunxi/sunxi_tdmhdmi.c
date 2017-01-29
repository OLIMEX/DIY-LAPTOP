/*
 * sound\soc\sunxi\sunxi_tdmhdmi.c
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
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <sound/dmaengine_pcm.h>
#include <linux/gpio.h>
#include <linux/dma/sunxi-dma.h>
#include "sunxi_tdm_utils.h"
#include "sunxi_dma.h"


#define DRV_NAME "sunxi-tdmhdmi"

/*define tdm hw parameters*/
#define DAUDIO_MASTER 4
#define PCM_LRCK_PEROID 32
#define PCM_LRCKR_PERIOD 1
#define SLOT_WIDTH_SELECT 32
#define TX_DATA_MODE 0
#define RX_DATA_MODE 0
#define TDM_CONFIG 1

static int sunxi_hdmi_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);
	return tdm_set_fmt(fmt,sunxi_tdmhdmi);
}

static int sunxi_hdmi_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params,struct snd_soc_dai *dai)
{
	u32 reg_val;
	int raw_flag;
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(dai);
	raw_flag = sunxi_tdmhdmi->others;

	switch (params_format(params))
	{
	case SNDRV_PCM_FORMAT_S16_LE:
		sunxi_tdmhdmi->samp_res = 16;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		sunxi_tdmhdmi->samp_res = 24;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		sunxi_tdmhdmi->samp_res = 24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		sunxi_tdmhdmi->samp_res = 24;
		break;
	default:
		return -EINVAL;
	}
	if (raw_flag > 1) {
		sunxi_tdmhdmi->samp_res = 24;
	}
	reg_val = readl(sunxi_tdmhdmi->regs + SUNXI_DAUDIOFAT0);
	reg_val &= ~SUNXI_DAUDIOFAT0_SR;
	if(sunxi_tdmhdmi->samp_res == 16)
		reg_val |= (3<<4);
	else if(sunxi_tdmhdmi->samp_res == 20)
		reg_val |= (4<<4);
	else
		reg_val |= (5<<4);
	writel(reg_val, sunxi_tdmhdmi->regs + SUNXI_DAUDIOFAT0);

	if (sunxi_tdmhdmi->samp_res == 24) {
		reg_val = readl(sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);
		reg_val &= ~SUNXI_DAUDIOFCTL_TXIM;
		writel(reg_val, sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);
	} else {
		reg_val = readl(sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);
		reg_val |= SUNXI_DAUDIOFCTL_TXIM;
		writel(reg_val, sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);
	}
	return 0;
}

static int sunxi_hdmi_trigger(struct snd_pcm_substream *substream,
                              				int cmd, struct snd_soc_dai *dai)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(dai);
	return tdm_trigger(substream, cmd,sunxi_tdmhdmi);
}

static int sunxi_hdmi_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id,
                                 		unsigned int freq, int hdmi_pcm_select)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);
	return tdm_set_sysclk(freq,sunxi_tdmhdmi);
}

static int sunxi_hdmi_set_clkdiv(struct snd_soc_dai *cpu_dai, int div_id, int sample_rate)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);
	return tdm_set_clkdiv(sample_rate,sunxi_tdmhdmi);
}

static int sunxi_hdmi_perpare(struct snd_pcm_substream *substream,struct snd_soc_dai *cpu_dai)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);
	return tdm_perpare(substream,sunxi_tdmhdmi);
}

static int sunxi_hdmi_dai_probe(struct snd_soc_dai *dai)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(dai);

	dai->capture_dma_data = &sunxi_tdmhdmi->capture_dma_param;
	dai->playback_dma_data = &sunxi_tdmhdmi->play_dma_param;

	return 0;
}
static int sunxi_hdmi_suspend(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);
	tdm_global_enable(sunxi_tdmhdmi,0);
	pr_debug("[HDMI-TDM]Entered sunxi_tdmhdmi->clk_enable_cnt:%d,%s\n", sunxi_tdmhdmi->clk_enable_cnt,__func__);
	if (sunxi_tdmhdmi->clk_enable_cnt > 0) {
		if (sunxi_tdmhdmi->tdm_moduleclk != NULL) {
			clk_disable(sunxi_tdmhdmi->tdm_moduleclk);
		}
		if (sunxi_tdmhdmi->tdm_pllclk != NULL) {
			clk_disable(sunxi_tdmhdmi->tdm_pllclk);
		}
		sunxi_tdmhdmi->clk_enable_cnt--;
	}
	return 0;
}

static int sunxi_hdmi_resume(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_tdm_info  *sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);

	pr_debug("[HDMI-TDM]Entered sunxi_tdmhdmi->clk_enable_cnt:%d,%s\n", sunxi_tdmhdmi->clk_enable_cnt,__func__);
	if (sunxi_tdmhdmi->tdm_pllclk != NULL) {
		if (clk_prepare_enable(sunxi_tdmhdmi->tdm_pllclk)) {
			pr_err("open sunxi_tdmhdmi->tdm_pllclk failed! line = %d\n", __LINE__);
		}
	}

	if (sunxi_tdmhdmi->tdm_moduleclk != NULL) {
		if (clk_prepare_enable(sunxi_tdmhdmi->tdm_moduleclk)) {
			pr_err("open sunxi_tdmhdmi->tdm_moduleclk failed! line = %d\n", __LINE__);
		}
	}
	sunxi_tdmhdmi->clk_enable_cnt++;
	tdm_global_enable(sunxi_tdmhdmi,1);
	return 0;
}

#define SUNXI_DAUDIO_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)
static struct snd_soc_dai_ops sunxi_hdmi_dai_ops = {
	.trigger 	= sunxi_hdmi_trigger,
	.hw_params 	= sunxi_hdmi_hw_params,
	.set_fmt 	= sunxi_hdmi_set_fmt,
	.set_clkdiv = sunxi_hdmi_set_clkdiv,
	.set_sysclk = sunxi_hdmi_set_sysclk,
	.prepare  =	sunxi_hdmi_perpare,
};

static struct snd_soc_dai_driver sunxi_hdmi_dai = {
	.probe 		= sunxi_hdmi_dai_probe,
	.suspend 	= sunxi_hdmi_suspend,
	.resume 	= sunxi_hdmi_resume,
	.playback 	= {
		.channels_min = 1,
		.channels_max = 8,
		.rates = SUNXI_DAUDIO_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture 	= {
		.channels_min = 1,
		.channels_max = 8,
		.rates = SUNXI_DAUDIO_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops 		= &sunxi_hdmi_dai_ops,
};

static const struct of_device_id sunxi_hdmi_of_match[] = {
	{ .compatible = "allwinner,sunxi-tdmhdmi", },
	{},
};
static const struct snd_soc_component_driver sunxi_hdmi_component = {
	.name		= DRV_NAME,
};

static int __init sunxi_hdmi_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	struct resource res;
	void __iomem  *sunxi_tdmhdmi_membase = NULL;
	struct sunxi_tdm_info  *sunxi_tdmhdmi;
	sunxi_tdmhdmi = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_tdm_info), GFP_KERNEL);
	if (!sunxi_tdmhdmi) {
		dev_err(&pdev->dev, "Can't allocate sunxi_tdmhdmi\n");
		ret = -ENOMEM;
		goto err0;
	}
	pr_debug("[audio-tdmhdmi] platform initial.\n");
	dev_set_drvdata(&pdev->dev, sunxi_tdmhdmi);
	sunxi_tdmhdmi->dai = sunxi_hdmi_dai;
	sunxi_tdmhdmi->dai.name = dev_name(&pdev->dev);
	device = of_match_device(sunxi_hdmi_of_match, &pdev->dev);
	if (!device)
		return -ENODEV;

	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}

	sunxi_tdmhdmi_membase =ioremap(res.start, resource_size(&res));
	if (NULL == sunxi_tdmhdmi_membase) {
		pr_err("[audio-tdmhdmi]Can't map tdmhdmi registers\n");
	} else {
		sunxi_tdmhdmi->regs = sunxi_tdmhdmi_membase;
	}
	sunxi_tdmhdmi->tdm_pllclk = of_clk_get(node, 0);
	sunxi_tdmhdmi->tdm_moduleclk= of_clk_get(node, 1);
	if (IS_ERR(sunxi_tdmhdmi->tdm_pllclk) || IS_ERR(sunxi_tdmhdmi->tdm_moduleclk)){
		dev_err(&pdev->dev, "[audio-tdmhdmi]Can't get daudio clocks\n");
		if (IS_ERR(sunxi_tdmhdmi->tdm_pllclk))
			ret = PTR_ERR(sunxi_tdmhdmi->tdm_pllclk);
		else
			ret = PTR_ERR(sunxi_tdmhdmi->tdm_moduleclk);
		goto err1;
	} else {
		if (clk_set_parent(sunxi_tdmhdmi->tdm_moduleclk, sunxi_tdmhdmi->tdm_pllclk)) {
			pr_err("try to set parent of sunxi_tdmhdmi->tdm_moduleclk to sunxi_tdmhdmi->tdm_pllclk failed! line = %d\n",__LINE__);
		}
		clk_prepare_enable(sunxi_tdmhdmi->tdm_pllclk);
		clk_prepare_enable(sunxi_tdmhdmi->tdm_moduleclk);
		sunxi_tdmhdmi->clk_enable_cnt++;
	}
	sunxi_tdmhdmi->play_dma_param.dma_addr = res.start + SUNXI_DAUDIOTXFIFO;
	sunxi_tdmhdmi->play_dma_param.dma_drq_type_num = DRQDST_DAUDIO_2_TX;
	sunxi_tdmhdmi->play_dma_param.dst_maxburst = 8;
	sunxi_tdmhdmi->play_dma_param.src_maxburst = 8;

	/*tdm2:default para*/
	sunxi_tdmhdmi->daudio_master = DAUDIO_MASTER;
	sunxi_tdmhdmi->pcm_lrck_period = PCM_LRCK_PEROID;
	sunxi_tdmhdmi->pcm_lrckr_period = PCM_LRCKR_PERIOD;
	sunxi_tdmhdmi->slot_width_select = SLOT_WIDTH_SELECT;
	sunxi_tdmhdmi->tx_data_mode = TX_DATA_MODE;
	sunxi_tdmhdmi->rx_data_mode = RX_DATA_MODE;
	sunxi_tdmhdmi->tdm_config = TDM_CONFIG;

	ret = snd_soc_register_component(&pdev->dev, &sunxi_hdmi_component,
				   &sunxi_tdmhdmi->dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "Could not register DAI: %d\n", ret);
		ret = -ENOMEM;
		goto err1;
	}
	ret = asoc_dma_platform_register(&pdev->dev,SND_DMAENGINE_PCM_FLAG_NO_RESIDUE);
	if (ret) {
		dev_err(&pdev->dev, "Could not register PCM: %d\n", ret);
		goto err2;
	}
	tdm_global_enable(sunxi_tdmhdmi,1);
	return 0;
err2:
	snd_soc_unregister_component(&pdev->dev);
err1:
	iounmap(sunxi_tdmhdmi->regs);
err0:
	return ret;

	return 0;
}

static int __exit sunxi_hdmi_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

/*method relating*/
static struct platform_driver sunxi_hdmi_driver = {
	.probe = sunxi_hdmi_dev_probe,
	.remove = __exit_p(sunxi_hdmi_dev_remove),
	.driver = {
		.name 	= DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_hdmi_of_match,
	},
};
module_platform_driver(sunxi_hdmi_driver);
/* Module information */
MODULE_AUTHOR("huangxin , liushaohua");
MODULE_DESCRIPTION("sunxi DAUDIO SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-hdmi");


