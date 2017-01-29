/*
 * sound\soc\sunxi\sunxi_daudio.c
 * (C) Copyright 2010-2016
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
#include "sunxi_dma.h"
#include "sunxi_tdm_utils.h"

#define DRV_NAME "sunxi-daudio"

static bool  daudio_loop_en 		= false;

static int sunxi_daudio_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(cpu_dai);
	return tdm_set_fmt(fmt,sunxi_daudio);
}

static int sunxi_daudio_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(dai);
	return tdm_hw_params(params,sunxi_daudio);
}

static int sunxi_daudio_trigger(struct snd_pcm_substream *substream,
                              int cmd, struct snd_soc_dai *dai)
{
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(dai);
	u32 reg_val;
	reg_val = readl(sunxi_daudio->regs + SUNXI_DAUDIOCTL);
	if (daudio_loop_en)
		reg_val |= SUNXI_DAUDIOCTL_LOOP;
	else
		reg_val &= ~SUNXI_DAUDIOCTL_LOOP;
	writel(reg_val, sunxi_daudio->regs + SUNXI_DAUDIOCTL);
	return tdm_trigger(substream, cmd,sunxi_daudio);
}

static int sunxi_daudio_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id,
                                 			unsigned int freq, int daudio_pcm_select)
{
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(cpu_dai);

	return tdm_set_sysclk(freq,sunxi_daudio);
}

static int sunxi_daudio_set_clkdiv(struct snd_soc_dai *cpu_dai, int div_id, int sample_rate)
{
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(cpu_dai);
	return tdm_set_clkdiv(sample_rate,sunxi_daudio);
}
static int sunxi_daudio_perpare(struct snd_pcm_substream *substream,
	struct snd_soc_dai *cpu_dai)
{
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(cpu_dai);
	return tdm_perpare(substream,sunxi_daudio);
}
static int sunxi_daudio_dai_probe(struct snd_soc_dai *dai)
{
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(dai);
	dai->capture_dma_data = &sunxi_daudio->capture_dma_param;
	dai->playback_dma_data = &sunxi_daudio->play_dma_param;
	return 0;
}

static int sunxi_daudio_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static int sunxi_daudio_suspend(struct snd_soc_dai *cpu_dai)
{
	int ret = 0;
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(cpu_dai);
	tdm_global_enable(sunxi_daudio,0);
	pr_debug("[daudio] suspend .%s\n",cpu_dai->name);
	if (sunxi_daudio->tdm_moduleclk != NULL) {
		clk_disable(sunxi_daudio->tdm_moduleclk);
	}
	if (sunxi_daudio->tdm_pllclk != NULL) {
		clk_disable(sunxi_daudio->tdm_pllclk);
	}
	if (sunxi_daudio->pinstate_sleep){
		ret = pinctrl_select_state(sunxi_daudio->pinctrl, sunxi_daudio->pinstate_sleep);
		if (ret) {
			pr_warn("[daudio]select pin sleep state failed\n");
			return ret;
		}
	}
	/*release gpio resource*/
	if (sunxi_daudio->pinctrl != NULL)
		devm_pinctrl_put(sunxi_daudio->pinctrl);
	sunxi_daudio->pinctrl = NULL;
	sunxi_daudio->pinstate = NULL;
	sunxi_daudio->pinstate_sleep = NULL;
	return 0;
}

static int sunxi_daudio_resume(struct snd_soc_dai *cpu_dai)
{
	int ret = 0;
	struct sunxi_tdm_info  *sunxi_daudio = snd_soc_dai_get_drvdata(cpu_dai);

	pr_debug("[daudio] resume .%s\n",cpu_dai->name);
	if (sunxi_daudio->tdm_pllclk != NULL) {
		if (clk_prepare_enable(sunxi_daudio->tdm_pllclk)) {
			pr_err("open sunxi_daudio->tdm_pllclk failed! line = %d\n", __LINE__);
		}
	}

	if (sunxi_daudio->tdm_moduleclk != NULL) {
		if (clk_prepare_enable(sunxi_daudio->tdm_moduleclk)) {
			pr_err("open sunxi_daudio->tdm_moduleclk failed! line = %d\n", __LINE__);
		}
	}

	if (!sunxi_daudio->pinctrl) {
		sunxi_daudio->pinctrl = devm_pinctrl_get(cpu_dai->dev);
		if (IS_ERR_OR_NULL(sunxi_daudio->pinctrl)) {
			pr_warn("[daudio]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_daudio->pinstate){
		sunxi_daudio->pinstate = pinctrl_lookup_state(sunxi_daudio->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_daudio->pinstate)) {
			pr_warn("[daudio]lookup pin default state failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_daudio->pinstate_sleep){
		sunxi_daudio->pinstate_sleep = pinctrl_lookup_state(sunxi_daudio->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_daudio->pinstate_sleep)) {
			pr_warn("[daudio]lookup pin default state failed\n");
			return -EINVAL;
		}
	}
	ret = pinctrl_select_state(sunxi_daudio->pinctrl, sunxi_daudio->pinstate);
	if (ret) {
		pr_warn("[daudio]select pin default state failed\n");
		return ret;
	}
	tdm_global_enable(sunxi_daudio,1);
	return 0;
}

#define SUNXI_DAUDIO_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)
static struct snd_soc_dai_ops sunxi_daudio_dai_ops = {
	.trigger 	= sunxi_daudio_trigger,
	.hw_params 	= sunxi_daudio_hw_params,
	.set_fmt 	= sunxi_daudio_set_fmt,
	.set_clkdiv = sunxi_daudio_set_clkdiv,
	.set_sysclk = sunxi_daudio_set_sysclk,
	.prepare  =	sunxi_daudio_perpare,
};

static struct snd_soc_dai_driver sunxi_daudio_dai = {
	.probe 		= sunxi_daudio_dai_probe,
	.suspend 	= sunxi_daudio_suspend,
	.resume 	= sunxi_daudio_resume,
	.remove 	= sunxi_daudio_dai_remove,
	.playback 	= {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SUNXI_DAUDIO_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture 	= {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SUNXI_DAUDIO_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops 		= &sunxi_daudio_dai_ops,
};
static const struct of_device_id sunxi_daudio_of_match[] = {
	{ .compatible = "allwinner,sunxi-daudio", },
	{},
};
static const struct snd_soc_component_driver sunxi_daudio_component = {
	.name		= DRV_NAME,
};
static int __init sunxi_daudio_platform_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource res;
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	void __iomem  *sunxi_daudio_membase = NULL;
	struct sunxi_tdm_info  *sunxi_daudio;
	u32 temp_val;

	sunxi_daudio = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_tdm_info), GFP_KERNEL);
	if (!sunxi_daudio) {
		dev_err(&pdev->dev, "Can't allocate sunxi_daudio\n");
		ret = -ENOMEM;
		goto err0;
	}
	dev_set_drvdata(&pdev->dev, sunxi_daudio);
	sunxi_daudio->dai = sunxi_daudio_dai;
	sunxi_daudio->dai.name = dev_name(&pdev->dev);

	device = of_match_device(sunxi_daudio_of_match, &pdev->dev);
	if (!device)
		return -ENODEV;

	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}
	sunxi_daudio_membase =ioremap(res.start, resource_size(&res));

	if (NULL == sunxi_daudio_membase) {
		pr_err("[audio-daudio]Can't map i2s registers\n");
	} else {
		sunxi_daudio->regs = sunxi_daudio_membase;
	}
	sunxi_daudio->tdm_pllclk = of_clk_get(node, 0);
	sunxi_daudio->tdm_moduleclk= of_clk_get(node, 1);
	if (IS_ERR(sunxi_daudio->tdm_pllclk) || IS_ERR(sunxi_daudio->tdm_moduleclk)){
		dev_err(&pdev->dev, "[audio-daudio]Can't get daudio clocks\n");
		if (IS_ERR(sunxi_daudio->tdm_pllclk))
			ret = PTR_ERR(sunxi_daudio->tdm_pllclk);
		else
			ret = PTR_ERR(sunxi_daudio->tdm_moduleclk);
		goto err1;
	} else {
		if (clk_set_parent(sunxi_daudio->tdm_moduleclk, sunxi_daudio->tdm_pllclk)) {
			pr_err("try to set parent of sunxi_daudio->tdm_moduleclk to sunxi_daudio->tdm_pllclk failed! line = %d\n",__LINE__);
		}
		clk_prepare_enable(sunxi_daudio->tdm_pllclk);
		clk_prepare_enable(sunxi_daudio->tdm_moduleclk);
	}
	ret = of_property_read_u32(node, "tdm_num",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]tdm_num configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->tdm_num = temp_val;
	}

	sunxi_daudio->play_dma_param.dma_addr = res.start + SUNXI_DAUDIOTXFIFO;
	if (0 == sunxi_daudio->tdm_num)
		sunxi_daudio->play_dma_param.dma_drq_type_num = DRQDST_DAUDIO_0_TX;
	else if (1 == sunxi_daudio->tdm_num)
		sunxi_daudio->play_dma_param.dma_drq_type_num = DRQDST_DAUDIO_1_TX;
	sunxi_daudio->play_dma_param.dst_maxburst = 4;
	sunxi_daudio->play_dma_param.src_maxburst = 4;

	sunxi_daudio->capture_dma_param.dma_addr = res.start + SUNXI_DAUDIORXFIFO;
	if (0 == sunxi_daudio->tdm_num)
		sunxi_daudio->capture_dma_param.dma_drq_type_num = DRQSRC_DAUDIO_0_RX;
	else
		sunxi_daudio->capture_dma_param.dma_drq_type_num = DRQSRC_DAUDIO_1_RX;

	sunxi_daudio->capture_dma_param.src_maxburst = 4;
	sunxi_daudio->capture_dma_param.dst_maxburst = 4;
	sunxi_daudio->pinctrl = NULL ;

	if (!sunxi_daudio->pinctrl) {
		sunxi_daudio->pinctrl = devm_pinctrl_get(&pdev->dev);
		if (IS_ERR_OR_NULL(sunxi_daudio->pinctrl)) {
			pr_warn("[daudio]request pinctrl handle for audio failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_daudio->pinstate){
		sunxi_daudio->pinstate = pinctrl_lookup_state(sunxi_daudio->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(sunxi_daudio->pinstate)) {
			pr_warn("[daudio]lookup pin default state failed\n");
			return -EINVAL;
		}
	}
	if (!sunxi_daudio->pinstate_sleep){
		sunxi_daudio->pinstate_sleep = pinctrl_lookup_state(sunxi_daudio->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(sunxi_daudio->pinstate_sleep)) {
			pr_warn("[daudio]lookup pin default state failed\n");
			return -EINVAL;
		}
	}

	ret = of_property_read_u32(node, "daudio_master",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]daudio_master configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->daudio_master = temp_val;
	}

	ret = of_property_read_u32(node, "pcm_lrck_period",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]pcm_lrck_period configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->pcm_lrck_period = temp_val;
	}

	ret = of_property_read_u32(node, "pcm_lrckr_period",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]pcm_lrckr_period configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->pcm_lrckr_period = temp_val;
	}

	ret = of_property_read_u32(node, "slot_width_select",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]slot_width_select configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->slot_width_select = temp_val;
	}

	ret = of_property_read_u32(node, "tx_data_mode",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]tx_data_mode configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->tx_data_mode = temp_val;
	}

	ret = of_property_read_u32(node, "rx_data_mode",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]rx_data_mode configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->rx_data_mode = temp_val;
	}


	ret = of_property_read_u32(node, "audio_format",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]audio_format configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->audio_format = temp_val;
	}

	ret = of_property_read_u32(node, "signal_inversion",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]signal_inversion configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->signal_inversion = temp_val;
	}
	ret = of_property_read_u32(node, "frametype",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]frametype configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->frametype = temp_val;
	}
	ret = of_property_read_u32(node, "tdm_config",&temp_val);
	if (ret < 0) {
		pr_err("[audio-daudio]tdm_config configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_daudio->tdm_config = temp_val;
	}
	pr_debug("[daudio]pcm_lrck_period:%d,pcm_lrckr_period:%d,slot_width_select:%d,pcm_lsb_first:%d,\
			tx_data_mode:%d,rx_data_mode:%d,daudio_master:%d,audio_format:%d,signal_inversion:%d,\
			frametype:%d,tdm_config:%d\n",sunxi_daudio->pcm_lrck_period,
						sunxi_daudio->pcm_lrckr_period,
						sunxi_daudio->slot_width_select,
						sunxi_daudio->pcm_lsb_first,
						sunxi_daudio->tx_data_mode,
						sunxi_daudio->rx_data_mode,
						sunxi_daudio->daudio_master,
						sunxi_daudio->audio_format,
						sunxi_daudio->signal_inversion,
						sunxi_daudio->frametype,
						sunxi_daudio->tdm_config);
	ret = snd_soc_register_component(&pdev->dev, &sunxi_daudio_component,
				   &sunxi_daudio->dai, 1);
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
	tdm_global_enable(sunxi_daudio,1);

	return 0;
err2:
	snd_soc_unregister_component(&pdev->dev);
err1:
	iounmap(sunxi_daudio->regs);
err0:
	return ret;
}

static int sunxi_daudio_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sunxi_daudio_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_daudio_of_match,
	},
	.probe = sunxi_daudio_platform_probe,
	.remove = sunxi_daudio_platform_remove,
};
module_platform_driver(sunxi_daudio_driver);
module_param_named(daudio_loop_en, daudio_loop_en, bool, S_IRUGO | S_IWUSR);
/* Module information */
MODULE_AUTHOR("huangxin,liushaohua");
MODULE_DESCRIPTION("sunxi DAUDIO SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);



