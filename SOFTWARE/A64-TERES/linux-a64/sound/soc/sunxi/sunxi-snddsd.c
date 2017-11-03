/*
 * sound\soc\sunxi\sunxi_snddsd.c
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@Reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include <linux/io.h>
#include <linux/of.h>
#include "sunxi_dsd.h"
#include "codec-utils.h"

static int sunxi_snddsd_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret = 0;
	int freq = 22579200;

	switch (params_rate(params)) {
		case 8000:
		case 16000:
		case 32000:
		case 64000:
		case 128000:
		case 12000:
		case 24000:
		case 48000:
		case 96000:
		case 192000:
			freq = 24576000;
			break;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, 0 , freq, 0);
	if (ret < 0)
		return ret;

	return 0;
}

static struct snd_soc_ops sunxi_snddsd_ops = {
	.hw_params 	= sunxi_snddsd_hw_params,
};

static struct snd_soc_dai_link sunxi_snddsd_dai_link = {
	.name 			= "DSD",
	.stream_name 	= "SUNXI-DSD",
	.cpu_dai_name 	= "sunxi-dsd",
	.platform_name 	= "sunxi-dsd",
	.codec_dai_name = "cs4385-dai",
	.codec_name 	= "cs4385.0-0018",
	.ops 			= &sunxi_snddsd_ops,
};

static struct snd_soc_card snd_soc_sunxi_snddsd = {
	.name 		= "snddsd",
	.owner 		= THIS_MODULE,
	.dai_link 	= &sunxi_snddsd_dai_link,
	.num_links 	= 1,
};

static int sunxi_snddsd_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_soc_card *card = &snd_soc_sunxi_snddsd;
	struct device_node *np = pdev->dev.of_node;
	card->dev = &pdev->dev;
	sunxi_snddsd_dai_link.cpu_dai_name = NULL;
	sunxi_snddsd_dai_link.cpu_of_node = of_parse_phandle(np,
				"sunxi,dsd-controller", 0);
	if (!sunxi_snddsd_dai_link.cpu_of_node) {
		dev_err(&pdev->dev,
			"Property 'sunxi,dsd-controller' missing or invalid\n");
			ret = -EINVAL;
	}
	sunxi_snddsd_dai_link.platform_name = NULL;
	sunxi_snddsd_dai_link.platform_of_node = sunxi_snddsd_dai_link.cpu_of_node;

	if (sunxi_snddsd_dai_link.codec_dai_name == NULL
			&& sunxi_snddsd_dai_link.codec_name == NULL){
			codec_utils_probe(pdev);
			sunxi_snddsd_dai_link.codec_dai_name = pdev->name;
			sunxi_snddsd_dai_link.codec_name 	= pdev->name;
	}

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);
	}
	return ret;
}

static int sunxi_snddsd_dev_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	return 0;
}

static const struct of_device_id sunxi_dsd_of_match[] = {
	{ .compatible = "allwinner,sunxi-dsd-machine", },
	{},
};

static struct platform_driver sunxi_dsd_driver = {
	.driver = {
		.name = "snddsd",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_dsd_of_match,
		.pm = &snd_soc_pm_ops,
	},
	.probe = sunxi_snddsd_dev_probe,
	.remove = sunxi_snddsd_dev_remove,
};

static int __init sunxi_snddsd_init(void)
{
	int err = 0;
	if ((err = platform_driver_register(&sunxi_dsd_driver)) < 0)
		return err;
	return 0;
}
module_init(sunxi_snddsd_init);

static void __exit sunxi_snddsd_exit(void)
{
	platform_driver_unregister(&sunxi_dsd_driver);
}
module_exit(sunxi_snddsd_exit);
MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("SUNXI_SNDDSD ALSA SoC audio driver");
MODULE_LICENSE("GPL");


