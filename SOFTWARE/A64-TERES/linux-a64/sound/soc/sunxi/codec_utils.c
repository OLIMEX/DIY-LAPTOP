/*
 * codec-utils.c
 *
 * Copyright (c) 2015 Allwinner.
 *
 * Author: Liu shaohua <liushaohua@allwinnertech.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/platform_device.h>
#include <linux/export.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include "codec-utils.h"

static struct snd_soc_codec_driver pub_codec;

#define STUB_RATES	SNDRV_PCM_RATE_8000_192000
#define STUB_FORMATS	(SNDRV_PCM_FMTBIT_S8 | \
			SNDRV_PCM_FMTBIT_U8 | \
			SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_U16_LE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_U24_LE | \
			SNDRV_PCM_FMTBIT_S32_LE | \
			SNDRV_PCM_FMTBIT_U32_LE | \
			SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_LE)
static struct snd_soc_dai_driver pub_codec_dai = {
	.playback = {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 384,
		.rates		= STUB_RATES,
		.formats	= STUB_FORMATS,
	},
	.capture = {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max	= 384,
		.rates = STUB_RATES,
		.formats = STUB_FORMATS,
	 },
};

int codec_utils_probe(struct platform_device *pdev)
{
	int ret;
	pub_codec_dai.name = pdev->name;
	pr_debug(" pdev->name:%s\n", pdev->name);

	ret = snd_soc_register_codec(&pdev->dev, &pub_codec, &pub_codec_dai, 1);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL(codec_utils_probe);
int codec_utils_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);

	return 0;
}
EXPORT_SYMBOL(codec_utils_remove);

