/*
 * sound\soc\sunxi\sunxi_sndspdif.c
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * chenpailin <chenpailin@Reuuimllatech.com>
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
#include "sunxi_spdif.h"
#include "codec-utils.h"


static int spdif_format = 1;

#define CONFIG_AW_ASIC_EVB_PLATFORM
static int sunxi_sndspdif_startup(struct snd_pcm_substream *substream)
{
	return 0;
}

static void sunxi_sndspdif_shutdown(struct snd_pcm_substream *substream)
{
}

typedef struct __MCLK_SET_INF
{
    __u32   samp_rate;      /*sample rate*/
	__u16 	mult_fs;        /*multiply of smaple rate*/

    __u8    clk_div;        /*mpll division*/
    __u8    mpll;           /*select mpll, 0 - 24.576 Mhz, 1 - 22.5792 Mhz*/

} __mclk_set_inf;

/*spdif hasn't used the bit clock div factor*/
typedef struct __BCLK_SET_INF
{
    __u8    bitpersamp;     /*bits per sample*/
    __u8    clk_div;        /*clock division*/
    __u16   mult_fs;        /*multiplay of sample rate*/

} __bclk_set_inf;

static __bclk_set_inf BCLK_INF[] =
{
    /*16bits per sample*/
    {16,  4, 128}, {16,  6, 192}, {16,  8, 256},
    {16, 12, 384}, {16, 16, 512},

    /*24 bits per sample*/
    {24,  4, 192}, {24,  8, 384}, {24, 16, 768},

    /*32 bits per sample*/
    {32,  2, 128}, {32,  4, 256}, {32,  6, 384},
    {32,  8, 512}, {32, 12, 768},

    /*end flag*/
    {0xff, 0, 0},
};

/*TX RATIO value*/
static __mclk_set_inf  MCLK_INF[] =
{
    { 88200, 128,  2, 1}, { 88200, 256,  2, 1},

	{ 22050, 128,  8, 1}, { 22050, 256,  8, 1},
    { 22050, 512,  8, 1},

    { 24000, 128,  8, 0}, { 24000, 256, 8, 0}, { 24000, 512, 8, 0},

    /* 32k bitrate   2.048MHz   24/4 = 6*/
    { 32000, 128,  6, 0}, { 32000, 192,  6, 0}, { 32000, 384,  6, 0},
    { 32000, 768,  6, 0},

	/*fpga:{ 48000, 256,  16, 0}
	* evb:{ 48000, 256,  4, 0}
	*/
#ifdef CONFIG_AW_ASIC_EVB_PLATFORM
	/* 48K bitrate   3.072 Mbit/s   16/4 = 4*/
	{ 48000, 128,  4, 0}, { 48000, 256,  4, 0}, { 48000, 512, 4, 0},
#else
	{ 48000, 128,  4, 0}, { 48000, 256,  16, 0}, { 48000, 512, 4, 0},
#endif

    /* 48K bitrate   3.072 Mbit/s   16/4 = 4*/
   // { 48000, 128,  4, 0}, { 48000, 256,  4, 0}, { 48000, 512, 4, 0},

    /* 96k bitrate  6.144  Mbit/s   8/4 = 2*/
    { 96000, 128 , 2, 0}, { 96000, 256,  2, 0},

    /*192k bitrate   12.288  Mbit/s  4/4 = 1*/
    {192000, 128,  1, 0},
    /*44.1k bitrate  2.8224  Mbit/s   16/4 = 4*/
    #ifdef CONFIG_AW_ASIC_EVB_PLATFORM
    { 44100, 128,  4, 1}, { 44100, 256,  4, 1}, { 44100, 512,  4, 1},
    #else
    /*fpga*/
    { 44100, 128,  4, 1}, { 44100, 256,  16, 1}, { 44100, 512,  4, 1},
    #endif
     /*176.4k bitrate  11.2896  Mbit/s 4/4 = 1*/
    {176400, 128, 1, 1},

    /*end flag 0xffffffff*/
    {0xffffffff, 0, 0, 0},
};

s32 get_clock_divder(u32 sample_rate, u32 sample_width, u32 * mclk_div, u32* mpll, u32* bclk_div, u32* mult_fs)
{
	u32 i, j, ret = -EINVAL;

	for(i=0; i< ARRAY_SIZE(MCLK_INF); i++) {
		 if((MCLK_INF[i].samp_rate == sample_rate) &&
		 	((MCLK_INF[i].mult_fs == 256) || (MCLK_INF[i].mult_fs == 128))) {
			  for(j=0; j<ARRAY_SIZE(BCLK_INF); j++) {
					if((BCLK_INF[j].bitpersamp == sample_width) &&
						(BCLK_INF[j].mult_fs == MCLK_INF[i].mult_fs)) {
						 *mclk_div = MCLK_INF[i].clk_div;
						 *mpll = MCLK_INF[i].mpll;
						 *bclk_div = BCLK_INF[j].clk_div;
						 *mult_fs = MCLK_INF[i].mult_fs;
						 ret = 0;
						 break;
					}
			  }
		 }
		 else if(MCLK_INF[i].samp_rate == 0xffffffff)
		 	break;
	}

	return ret;
}
EXPORT_SYMBOL(get_clock_divder);

static int sunxi_sndspdif_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret = 0;
	unsigned long rate = params_rate(params);
	unsigned int fmt = 0;
	u32 mclk_div=0, mpll=0, bclk_div=0, mult_fs=0;

	get_clock_divder(rate, 32, &mclk_div, &mpll, &bclk_div, &mult_fs);

	if (ret < 0)
		return ret;

	/*fmt:1:pcm; >1:rawdata*/
	fmt = spdif_format;
	if(fmt > 1){
		fmt = 1;
	}else{
		fmt = 0;
	}

	ret = snd_soc_dai_set_fmt(cpu_dai, fmt);//0:pcm,1:raw data
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(cpu_dai, 0 , mpll, 0);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(cpu_dai, SUNXI_DIV_MCLK, mclk_div);
	if (ret < 0)
		return ret;
	return 0;
}
static int sunxi_spdif_set_audio_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	spdif_format = ucontrol->value.integer.value[0];
	return 0;
}

static int sunxi_spdif_get_audio_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = spdif_format;
	return 0;
}

static const char *spdif_format_function[] = {"null", "pcm", "DTS"};
static const struct soc_enum spdif_format_enum[] = {
        SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(spdif_format_function), spdif_format_function),
};

/* dts pcm Audio Mode Select */
static const struct snd_kcontrol_new sunxi_spdif_controls[] = {
	SOC_ENUM_EXT("spdif audio format Function", spdif_format_enum[0], sunxi_spdif_get_audio_mode, sunxi_spdif_set_audio_mode),
};

/*
 * Card initialization
 */
static int sunxi_spdif_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_card *card = rtd->card;
	int ret;
	ret = snd_soc_add_card_controls(card, sunxi_spdif_controls, ARRAY_SIZE(sunxi_spdif_controls));

	if (ret) {
		dev_warn(card->dev,
				"Failed to register audio mode control, "
				"will continue without it.\n");
	}
	return 0;
}

static struct snd_soc_ops sunxi_sndspdif_ops = {
	.startup 	= sunxi_sndspdif_startup,
	.shutdown 	= sunxi_sndspdif_shutdown,
	.hw_params 	= sunxi_sndspdif_hw_params,
};

static struct snd_soc_dai_link sunxi_sndspdif_dai_link = {
	.name 			= "SPDIF",
	.stream_name 	= "SUNXI-SPDIF",
	.cpu_dai_name 	= "sunxi-spdif",
	.platform_name 	= "sunxi-spdif",
	.init 			= sunxi_spdif_init,
	.ops 			= &sunxi_sndspdif_ops,
};

static struct snd_soc_card snd_soc_sunxi_sndspdif = {
	.name 		= "sndspdif",
	.owner 		= THIS_MODULE,
	.dai_link 	= &sunxi_sndspdif_dai_link,
	.num_links 	= 1,
};

static int sunxi_sndspdif_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_soc_card *card = &snd_soc_sunxi_sndspdif;
	struct device_node *np = pdev->dev.of_node;
	card->dev = &pdev->dev;
	sunxi_sndspdif_dai_link.cpu_dai_name = NULL;
	sunxi_sndspdif_dai_link.cpu_of_node = of_parse_phandle(np,
				"sunxi,spdif-controller", 0);
	if (!sunxi_sndspdif_dai_link.cpu_of_node) {
		dev_err(&pdev->dev,
			"Property 'sunxi,spdif-controller' missing or invalid\n");
			ret = -EINVAL;
	}
	sunxi_sndspdif_dai_link.platform_name = NULL;
	sunxi_sndspdif_dai_link.platform_of_node = sunxi_sndspdif_dai_link.cpu_of_node;

	if (sunxi_sndspdif_dai_link.codec_dai_name == NULL
			&& sunxi_sndspdif_dai_link.codec_name == NULL){
			codec_utils_probe(pdev);
			sunxi_sndspdif_dai_link.codec_dai_name = pdev->name;
			sunxi_sndspdif_dai_link.codec_name 	= pdev->name;
	}

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);
	}
	return ret;
}

static int sunxi_sndspdif_dev_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);
	return 0;
}

static const struct of_device_id sunxi_spdif_of_match[] = {
	{ .compatible = "allwinner,sunxi-spdif-machine", },
	{},
};

static struct platform_driver sunxi_spdif_driver = {
	.driver = {
		.name = "sndspdif",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_spdif_of_match,
		.pm = &snd_soc_pm_ops,
	},
	.probe = sunxi_sndspdif_dev_probe,
	.remove = sunxi_sndspdif_dev_remove,
};

static int __init sunxi_sndspdif_init(void)
{
	int err = 0;
	if ((err = platform_driver_register(&sunxi_spdif_driver)) < 0)
		return err;
	return 0;
}
module_init(sunxi_sndspdif_init);

static void __exit sunxi_sndspdif_exit(void)
{
	platform_driver_unregister(&sunxi_spdif_driver);

}
module_exit(sunxi_sndspdif_exit);
MODULE_AUTHOR("chenpailin");
MODULE_DESCRIPTION("SUNXI_SNDSPDIF ALSA SoC audio driver");
MODULE_LICENSE("GPL");


