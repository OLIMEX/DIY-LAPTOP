/* 
 * sound\soc\sunxi\cs4385.c
 * (C) Copyright 2014-2017
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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include "cs4385.h"

#define CS4385_NAME        "cs4385-codec"
#define CS4385_RATES (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)

/* codec private data */
struct cs4385_priv {
	struct regmap *regmap;
	unsigned int sysclk;
};

static int cs4385_set_dai_sysclk(struct snd_soc_dai *codec_dai,
	int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct cs4385_priv *cs4385 = snd_soc_codec_get_drvdata(codec);

	cs4385->sysclk = freq;
	return 0;
}

static int cs4385_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	/*
	*0x03:CS4385_PCM_CTR:default == 11
	* reg_val(0x0,0x1):functional mode(FM)
	* 00:single-speed mode(4~50kHz sample rates)
	* 01:Double-speed mode(50~100kHz sample rates)
	* 10:quad-speed mode(100~200kHz sample rates)
	* 11:Auto Speed Mode detect(32kHz~200kHz sample rates)
	*/

	snd_soc_write(codec, CS4385_MOD_CTR, (0x1<<7|0x1<<5));
	snd_soc_write(codec, CS4385_DSD_CTR, (0x1<<6|0x7<<2));
	snd_soc_write(codec, CS4385_RAMP_MUTE, 0xfe);

	return 0;
}

static int cs4385_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		/*i2s, up to 24-bit data*/
		snd_soc_update_bits(codec, CS4385_PCM_CTR, (0x7<<4), 0x1<<4);
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		/*left_justified, up to 24-bit data*/
		snd_soc_update_bits(codec, CS4385_PCM_CTR, (0x7<<4), 0x0<<4);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int cs4385_set_bias_level(struct snd_soc_codec *codec,
	enum snd_soc_bias_level level)
{
	switch (level) {
	case SND_SOC_BIAS_ON:
		break;
	case SND_SOC_BIAS_PREPARE:
		break;
	case SND_SOC_BIAS_STANDBY:
		break;
	case SND_SOC_BIAS_OFF:
		break;
	}
	codec->dapm.bias_level = level;
	return 0;
}

static const struct snd_soc_dai_ops cs4385_dai_ops = {
	.hw_params	= cs4385_hw_params,
	.set_fmt	= cs4385_set_dai_fmt,
	.set_sysclk	= cs4385_set_dai_sysclk,
};

static struct snd_soc_dai_driver cs4385_dai = {
	.name = "cs4385-dai",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = CS4385_RATES,
		.formats = SNDRV_PCM_FMTBIT_S8|SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S24_LE|SNDRV_PCM_FMTBIT_S32_LE,
		},
	.ops = &cs4385_dai_ops,
};

static int cs4385_suspend(struct snd_soc_codec *codec)
{
	cs4385_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int cs4385_resume(struct snd_soc_codec *codec)
{
	snd_soc_cache_sync(codec);
	cs4385_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	return 0;
}

static int cs4385_probe(struct snd_soc_codec *codec)
{
	struct cs4385_priv *cs4385 = snd_soc_codec_get_drvdata(codec);
	unsigned int reg_val = 0;
	int ret = 0;

	codec->control_data = cs4385->regmap;
	ret = snd_soc_codec_set_cache_io(codec, 8, 8, SND_SOC_REGMAP);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to set cache I/O: %d\n", ret);
		return ret;
	}
	/* power on device */
	cs4385_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

	reg_val = snd_soc_read(codec, CS4385_CHIP_ID);
	pr_info("CS4385 chip id is %x",reg_val);

	return 0;
}

/* power down chip */
static int cs4385_remove(struct snd_soc_codec *codec)
{
	cs4385_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static const struct regmap_config cs4385_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0x16,
	.cache_type = REGCACHE_RBTREE,
};

static struct snd_soc_codec_driver soc_codec_dev_cs4385 = {
	.probe =	cs4385_probe,
	.remove =	cs4385_remove,
	.suspend =	cs4385_suspend,
	.resume =	cs4385_resume,
	.set_bias_level = cs4385_set_bias_level,
};

static int cs4385_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct cs4385_priv *cs4385;
	int ret = 0;

	cs4385 = devm_kzalloc(&i2c->dev, sizeof(struct cs4385_priv),
			      GFP_KERNEL);
	if (cs4385 == NULL)
		return -ENOMEM;

	cs4385->regmap = devm_regmap_init_i2c(i2c, &cs4385_regmap);
	if (IS_ERR(cs4385->regmap)) {
		ret = PTR_ERR(cs4385->regmap);
		dev_err(&i2c->dev, "Failed to init regmap: %d\n", ret);
		return ret;
	}

	i2c_set_clientdata(i2c, cs4385);

	ret = snd_soc_register_codec(&i2c->dev,
			&soc_codec_dev_cs4385, &cs4385_dai, 1);

	return ret;
}

static int cs4385_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_codec(&client->dev);
	return 0;
}

static const struct i2c_device_id cs4385_i2c_id[] = {
	{"cs4385", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, cs4385_i2c_id);

static struct i2c_driver cs4385_i2c_driver = {
	.driver = {
		.name = "cs4385",
		.owner = THIS_MODULE,
	},
	.probe =    cs4385_i2c_probe,
	.remove =   cs4385_i2c_remove,
	.id_table = cs4385_i2c_id,
};

static int __init cs4385_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&cs4385_i2c_driver);
	if (ret < 0) {
		printk("cs4385_i2c_driver add failed\n");
		return ret;
	}

	return 0;
}
module_init(cs4385_init);

static void __exit cs4385_exit(void)
{
	i2c_del_driver(&cs4385_i2c_driver);
}
module_exit(cs4385_exit);

MODULE_DESCRIPTION("Soc CS4385 driver");
MODULE_AUTHOR("huangxin");
MODULE_LICENSE("GPL");
