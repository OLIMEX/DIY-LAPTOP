/*
 * sound\soc\sunxi\sunxi-cpudai.c
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
#include "sunxi_cpudai.h"

#define DRV_NAME "sunxi-internal-cpudai"
#define SUNXI_PCM_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)

static int sunxi_cpudai_set_sysclk(struct snd_soc_dai *dai,
				  		int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static struct snd_soc_dai_ops sunxi_cpudai_dai_ops = {
	.set_sysclk = sunxi_cpudai_set_sysclk,
};

static int sunxi_cpudai_probe(struct snd_soc_dai *dai)
{
	struct sunxi_cpudai *sunxi_cpudai = snd_soc_dai_get_drvdata(dai);

	dai->capture_dma_data = &sunxi_cpudai->capture_dma_param;
	dai->playback_dma_data = &sunxi_cpudai->play_dma_param;

	return 0;
}

static int sunxi_cpudai_suspend(struct snd_soc_dai *cpu_dai)
{
	return 0;
}

static int sunxi_cpudai_resume(struct snd_soc_dai *cpu_dai)
{
	return 0;
}

static struct snd_soc_dai_driver sunxi_pcm_dai = {
	.probe = sunxi_cpudai_probe,
	.suspend 	= sunxi_cpudai_suspend,
	.resume 	= sunxi_cpudai_resume,
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
	.ops 		= &sunxi_cpudai_dai_ops,

};

static const struct snd_soc_component_driver sunxi_cpudai_component = {
	.name		= DRV_NAME,
};
static const struct of_device_id sunxi_cpudai_of_match[] = {
	{ .compatible = "allwinner,sunxi-internal-cpudai", },
	{},
};

static int __init sunxi_internal_cpudai_platform_probe(struct platform_device *pdev)
{
	s32 ret = 0;
	const struct of_device_id *device;
	struct resource res;
	struct sunxi_cpudai *sunxi_cpudai;
	struct device_node *node = pdev->dev.of_node;

	if (!node) {
		dev_err(&pdev->dev,
			"can not get dt node for this device.\n");
		ret = -EINVAL;
		goto err0;
	}
	sunxi_cpudai = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_cpudai), GFP_KERNEL);
	if (!sunxi_cpudai) {
		dev_err(&pdev->dev, "Can't allocate sunxi_cpudai.\n");
		ret = -ENOMEM;
		goto err0;
	}
	dev_set_drvdata(&pdev->dev, sunxi_cpudai);
	sunxi_cpudai->dai = sunxi_pcm_dai;
	sunxi_cpudai->dai.name = dev_name(&pdev->dev);

	device = of_match_device(sunxi_cpudai_of_match, &pdev->dev);
	if (!device)
		return -ENODEV;
	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}

	sunxi_cpudai->play_dma_param.dma_addr = res.start+AC_ADC_TXDATA;
	sunxi_cpudai->play_dma_param.dma_drq_type_num = DRQDST_AUDIO_CODEC;
	sunxi_cpudai->play_dma_param.dst_maxburst = 4;
	sunxi_cpudai->play_dma_param.src_maxburst = 4;

	sunxi_cpudai->capture_dma_param.dma_addr = res.start+AC_ADC_RXDATA;
	sunxi_cpudai->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;
	sunxi_cpudai->capture_dma_param.src_maxburst = 4;
	sunxi_cpudai->capture_dma_param.dst_maxburst = 4;

	ret = snd_soc_register_component(&pdev->dev, &sunxi_cpudai_component,
				   &sunxi_cpudai->dai, 1);
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
err0:
	return ret;

}

static int sunxi_internal_cpudai_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sunxi_internal_cpudai_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_cpudai_of_match,
	},
	.probe = sunxi_internal_cpudai_platform_probe,
	.remove = sunxi_internal_cpudai_platform_remove,
};
module_platform_driver(sunxi_internal_cpudai_driver);

/* Module information */
MODULE_AUTHOR("REUUIMLLA");
MODULE_DESCRIPTION("sunxi cpudai-internal SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);


