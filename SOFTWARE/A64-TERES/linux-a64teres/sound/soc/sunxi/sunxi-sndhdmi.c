/*
 * sound\soc\sunxi\sunxi-sndhdmi.c
 * (C) Copyright 2014-2016
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
#include "sunxi_tdm_utils.h"

static u32 hdmi_format = 1;
static int sunxi_hdmiaudio_set_audio_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	hdmi_format = ucontrol->value.integer.value[0];
	return 0;
}

static int sunxi_hdmiaudio_get_audio_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = hdmi_format;
	return 0;
}

static const char *hdmiaudio_format_function[] = {"null", "pcm", "AC3", "MPEG1", "MP3", "MPEG2", "AAC", "DTS", "ATRAC", "ONE_BIT_AUDIO", "DOLBY_DIGITAL_PLUS", "DTS_HD", "MAT", "WMAPRO"};
static const struct soc_enum hdmiaudio_format_enum[] = {
        SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(hdmiaudio_format_function), hdmiaudio_format_function),
};

/* pcm dts ac3 Audio Mode Select */
static const struct snd_kcontrol_new sunxi_hdmiaudio_controls[] = {
	SOC_ENUM_EXT("hdmi audio format Function", hdmiaudio_format_enum[0], sunxi_hdmiaudio_get_audio_mode, sunxi_hdmiaudio_set_audio_mode),
};

/*
 * Card initialization
 */
static int sunxi_hdmiaudio_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_card *card = rtd->card;
	int ret;
	/* Add virtual switch */
	ret = snd_soc_add_codec_controls(codec, sunxi_hdmiaudio_controls,
					ARRAY_SIZE(sunxi_hdmiaudio_controls));
	if (ret) {
		dev_warn(card->dev,
				"Failed to register audio mode control, "
				"will continue without it.\n");
	}
	return 0;
}

static int sunxi_sndhdmi_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd = NULL;
	struct snd_soc_dai *cpu_dai 	= NULL;
	struct sunxi_tdm_info  *sunxi_tdmhdmi = NULL;

	u32 freq = 22579200;
	unsigned long sample_rate = params_rate(params);
	if (!substream) {
		pr_err("error:%s,line:%d\n", __func__, __LINE__);
		return -EAGAIN;
	}
	rtd 		= substream->private_data;
	cpu_dai 	= rtd->cpu_dai;
	sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);
	sunxi_tdmhdmi->others = hdmi_format;
	switch (sample_rate) {
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

	/*set system clock source freq and set the mode as i2s0 or pcm*/
	ret = snd_soc_dai_set_sysclk(cpu_dai, 0 , freq, 0);
	if (ret < 0) {
		return ret;
	}

	/*
	* I2S mode \normal bit clock + frame\codec clk & FRM slave
	*/
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(cpu_dai, 0, sample_rate);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static struct snd_soc_ops sunxi_sndhdmi_ops = {
	.hw_params 	= sunxi_sndhdmi_hw_params,
};

static struct snd_soc_dai_link sunxi_sndhdmi_dai_link = {
	.name 			= "HDMIAUDIO",
	.stream_name 	= "SUNXI-HDMIAUDIO",
	.cpu_dai_name 	= "sunxi-hdmiaudio.0",
	.codec_dai_name = "sndhdmi",
	.platform_name 	= "sunxi-hdmiaudio-pcm-audio.0",
	.codec_name 	= "sunxi-hdmiaudio-codec.0",
	.init 			= sunxi_hdmiaudio_init,
	.ops 			= &sunxi_sndhdmi_ops,
};

static struct snd_soc_card snd_soc_sunxi_sndhdmi = {
	.name 		= "sndhdmi",
	.owner 		= THIS_MODULE,
	.dai_link 	= &sunxi_sndhdmi_dai_link,
	.num_links 	= 1,
};

static int sunxi_sndhdmi_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_soc_card *card = &snd_soc_sunxi_sndhdmi;
	struct device_node *np = pdev->dev.of_node;
	card->dev = &pdev->dev;
	if (np) {
		sunxi_sndhdmi_dai_link.cpu_dai_name = NULL;
		sunxi_sndhdmi_dai_link.cpu_of_node = of_parse_phandle(np,
					"sunxi,hdmi-controller", 0);
		if (!sunxi_sndhdmi_dai_link.cpu_of_node) {
			dev_err(&pdev->dev,
				"Property 'sunxi,hdmi-controller' missing or invalid\n");
			ret = -EINVAL;
		}
		sunxi_sndhdmi_dai_link.platform_name = NULL;
		sunxi_sndhdmi_dai_link.platform_of_node = sunxi_sndhdmi_dai_link.cpu_of_node;
	} else {
		dev_err(&pdev->dev,"hdmi dt node missing or invalid\n");
		ret = -EINVAL;
	}

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n",
			ret);
	}
	return ret;
}

static int sunxi_sndhdmi_dev_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);
	return 0;
}

static const struct of_device_id sunxi_hdmi_of_match[] = {
	{ .compatible = "allwinner,sunxi-hdmi-machine", },
	{},
};

static struct platform_driver sunxi_hdmiaudio_driver = {
	.driver = {
		.name = "sndhdmi",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_hdmi_of_match,
		.pm = &snd_soc_pm_ops,
	},
	.probe = sunxi_sndhdmi_dev_probe,
	.remove = sunxi_sndhdmi_dev_remove,
};

static int __init sunxi_sndhdmi_init(void)
{
	int err = 0;
	if ((err = platform_driver_register(&sunxi_hdmiaudio_driver)) < 0)
		return err;

	return 0;
}
module_init(sunxi_sndhdmi_init);

static void __exit sunxi_sndhdmi_exit(void)
{
	platform_driver_unregister(&sunxi_hdmiaudio_driver);
}
module_exit(sunxi_sndhdmi_exit);
MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("SUNXI_SNDHDMI ALSA SoC audio driver");
MODULE_LICENSE("GPL");

