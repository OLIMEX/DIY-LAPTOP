/*
 * sound\soc\sunxi\daudio\snddaudio.c
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
#include <linux/delay.h>
#include <linux/slab.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <linux/io.h>
struct snddaudio_priv {
	int sysclk;
	int dai_fmt;

	struct snd_pcm_substream *master_substream;
	struct snd_pcm_substream *slave_substream;
};

//static int daudio_used = 0;
#define snddaudio_RATES  (SNDRV_PCM_RATE_8000_192000|SNDRV_PCM_RATE_KNOT)
#define snddaudio_FORMATS (SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | \
		                     SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE)

static int snddaudio_mute(struct snd_soc_dai *dai, int mute)
{
	return 0;
}

static int snddaudio_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static void snddaudio_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{

}

static int snddaudio_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int snddaudio_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int snddaudio_set_dai_clkdiv(struct snd_soc_dai *codec_dai, int div_id, int div)
{
	return 0;
}

static int snddaudio_set_dai_fmt(struct snd_soc_dai *codec_dai,
			       unsigned int fmt)
{
	return 0;
}

static struct snd_soc_dai_ops snddaudio_dai_ops = {
	.startup = snddaudio_startup,
	.shutdown = snddaudio_shutdown,
	.hw_params = snddaudio_hw_params,
	.digital_mute = snddaudio_mute,
	.set_sysclk = snddaudio_set_dai_sysclk,
	.set_clkdiv = snddaudio_set_dai_clkdiv,
	.set_fmt = snddaudio_set_dai_fmt,
};

static struct snd_soc_dai_driver snddaudio_dai = {
	.name = "snddaudio",
	/* playback capabilities */
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = snddaudio_RATES,
		.formats = snddaudio_FORMATS,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = snddaudio_RATES,
		.formats = snddaudio_FORMATS,
	},
	/* pcm operations */
	.ops = &snddaudio_dai_ops,
};
//EXPORT_SYMBOL(snddaudio_dai);

static int snddaudio_soc_probe(struct snd_soc_codec *codec)
{
	struct snddaudio_priv *snddaudio;

	snddaudio = kzalloc(sizeof(struct snddaudio_priv), GFP_KERNEL);
	if(snddaudio == NULL){
		return -ENOMEM;
	}
	snd_soc_codec_set_drvdata(codec, snddaudio);

	return 0;
}

/* power down chip */
static int snddaudio_soc_remove(struct snd_soc_codec *codec)
{
	struct snddaudio_priv *snddaudio = snd_soc_codec_get_drvdata(codec);

	kfree(snddaudio);

	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_snddaudio = {
	.probe 	=	snddaudio_soc_probe,
	.remove =   snddaudio_soc_remove,
};

static int __init snddaudio_codec_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_snddaudio, &snddaudio_dai, 1);
}

static int __exit snddaudio_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

/*data relating*/
static struct platform_device snddaudio_codec_device = {
	.name = "sunxi-daudio-codec",
};

/*method relating*/
static struct platform_driver snddaudio_codec_driver = {
	.driver = {
		.name = "sunxi-daudio-codec",
		.owner = THIS_MODULE,
	},
	.probe = snddaudio_codec_probe,
	.remove = __exit_p(snddaudio_codec_remove),
};

static int __init snddaudio_codec_init(void)
{
	int err = 0;
	if((err = platform_device_register(&snddaudio_codec_device)) < 0)
		return err;
	if ((err = platform_driver_register(&snddaudio_codec_driver)) < 0)
		return err;
	return 0;
}
module_init(snddaudio_codec_init);

static void __exit snddaudio_codec_exit(void)
{
	platform_driver_unregister(&snddaudio_codec_driver);
	platform_device_unregister(&snddaudio_codec_device);
}
module_exit(snddaudio_codec_exit);

MODULE_DESCRIPTION("SNDI2S0 ALSA soc codec driver");
MODULE_AUTHOR("huangxin");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-daudio-codec");

