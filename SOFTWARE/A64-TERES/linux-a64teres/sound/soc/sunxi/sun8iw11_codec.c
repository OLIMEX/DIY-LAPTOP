/*
 * sound\soc\sunxi\sun8iw10_codec.c
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
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pm.h>
#include <linux/of_gpio.h>
#include <linux/sys_config.h>
#include "sunxi_cpudai.h"

#define DRV_NAME "sunxi-internal-codec"

void __iomem *codec_digitaladress;
void __iomem *codec_analogadress;
struct spk_gpio_ spk_gpio;

static const DECLARE_TLV_DB_SCALE(dig_vol_tlv, -7424, 0, 0);
static const DECLARE_TLV_DB_SCALE(headphone_vol_tlv, -6300, 100, 0);
static const DECLARE_TLV_DB_SCALE(lineinl_to_routp_mix_vol_tlv, -450, 150, 0);
static const DECLARE_TLV_DB_SCALE(lineinr_to_loutp_mix_vol_tlv, -450, 150, 0);
static const DECLARE_TLV_DB_SCALE(fmlr_to_lroutp_mix_vol_tlv, -450, 150, 0);
static const DECLARE_TLV_DB_SCALE(lineinln_to_lroutp_mix_vol_tlv, -450, 150, 0);

static const DECLARE_TLV_DB_SCALE(mic1_to_lroutp_mix_vol_tlv, -450, 150, 0);
static const DECLARE_TLV_DB_SCALE(mic2_to_lroutp_mix_vol_tlv, -450, 150, 0);
static const DECLARE_TLV_DB_SCALE(mic1_boost_vol_tlv, 0, 300, 0);
static const DECLARE_TLV_DB_SCALE(mic2_boost_vol_tlv, 0, 300, 0);
static const DECLARE_TLV_DB_SCALE(adc_input_gain_tlv, -450, 150, 0);

struct codec_sr {
	unsigned int samplerate;
	int srbit;
};

static const struct codec_sr codec_sr_s[] = {
	{44100, 0},
	{48000, 0},
	{8000, 5},
	{11025, 4},
	{12000, 4},
	{16000, 3},
	{22050, 2},
	{24000, 2},
	{32000, 1},
	{96000, 7},
	{192000, 6},
};

static struct label reg_labels[]={
    LABEL(AC_DAC_DPC),
    LABEL(AC_DAC_FIFOC),
    LABEL(AC_DAC_FIFOS),
    LABEL(AC_ADC_FIFOC),
    LABEL(AC_ADC_FIFOS),
    LABEL(AC_ADC_RXDATA),
    LABEL(AC_ADC_TXDATA),
    LABEL(AC_DAC_CNT),
    LABEL(AC_ADC_CNT),
	LABEL(AC_DAC_DG),
	LABEL(AC_ADC_DG),
	LABEL(AC_HMIC_CTRL),
	LABEL(AC_HMIC_DATA),

	LABEL(HP_VOLC),
	LABEL(LOMIXSC),
	LABEL(ROMIXSC),
	LABEL(DAC_PA_SRC),
	LABEL(LINEIN_GCTRL),
	LABEL(FM_GCTRL),
	LABEL(MICIN_GCTRL),
	LABEL(PAEN_HP_CTRL),
	LABEL(PHONEOUT_CTRL),
	LABEL(MIC2G_LINEEN_CTRL),
	LABEL(MIC1G_MICBIAS_CTRL),
	LABEL(LADCMIXSC),
	LABEL(RADCMIXSC),
	
	LABEL(PA_POP_CTR),
	LABEL(ADC_AP_EN),
	LABEL(ADDA_APTO),
	LABEL(ADDA_APT1),
	LABEL(ADDA_APT2),
	LABEL(DA16_CAL_CTRL),
	LABEL(BIAS_DA16_CAL_CTR),
	LABEL(DA16_CALI_DATA),
	LABEL(BIAS_CALI_DATA),
	LABEL(BIAS_CALI_SET),
	LABEL_END,
};

/*
*enable the codec function which should be enable during system init.
*/
static int codec_init(struct sunxi_codec *sunxi_internal_codec)
{
	struct snd_soc_codec *codec = sunxi_internal_codec->codec;

	if (sunxi_internal_codec->hp_dirused) {
		snd_soc_update_bits(codec, PAEN_HP_CTRL, (0x3<<HPCOM_FC), (0x3<<HPCOM_FC));
		snd_soc_update_bits(codec, PAEN_HP_CTRL, (0x1<<HPCOM_PT), (0x1<<HPCOM_PT));
	} else {
		snd_soc_update_bits(codec, PAEN_HP_CTRL, (0x3<<HPCOM_FC), (0x0<<HPCOM_FC));
		snd_soc_update_bits(codec, PAEN_HP_CTRL, (0x1<<HPCOM_PT), (0x0<<HPCOM_PT));
	}
	snd_soc_update_bits(codec, DAC_PA_SRC, (0x1<<LHPPAMUTE), (0x0<<LHPPAMUTE));
	snd_soc_update_bits(codec, DAC_PA_SRC, (0x1<<RHPPAMUTE), (0x0<<RHPPAMUTE));

	/*when TX FIFO available room less than or equal N,
	* DRQ Requeest will be de-asserted.
	*/
	snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x3<<DAC_DRQ_CLR_CNT), (0x3<<DAC_DRQ_CLR_CNT));
	snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<FIFO_FLUSH), (0x1<<FIFO_FLUSH));
	/*
	*	0:64-Tap FIR
	*	1:32-Tap FIR
	*/
	snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<FIR_VER), (0x0<<FIR_VER));

	snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<ADC_FIFO_FLUSH), (0x1<<ADC_FIFO_FLUSH));

	return 0;
}

static int late_enable_dac(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);
	mutex_lock(&sunxi_internal_codec->dac_mutex);
	pr_debug("..dac power state change.\n");
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (sunxi_internal_codec->dac_enable == 0) {
			snd_soc_update_bits(codec, AC_DAC_DPC, (0x1<<EN_DAC), (0x1<<EN_DAC));
		}
		sunxi_internal_codec->dac_enable++;
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (sunxi_internal_codec->dac_enable > 0) {
			sunxi_internal_codec->dac_enable--;
			if (sunxi_internal_codec->dac_enable == 0) {
				snd_soc_update_bits(codec, AC_DAC_DPC, (0x1<<EN_DAC), (0x0<<EN_DAC));
			}
		}
		break;
	}
	mutex_unlock(&sunxi_internal_codec->dac_mutex);
	return 0;
}

static int late_enable_adc(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);
	mutex_lock(&sunxi_internal_codec->adc_mutex);
	pr_debug("..adc power state change.\n");
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (sunxi_internal_codec->adc_enable == 0) {
			snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<EN_AD), (0x1<<EN_AD));
		}
		sunxi_internal_codec->adc_enable++;
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (sunxi_internal_codec->adc_enable > 0) {
			sunxi_internal_codec->adc_enable--;
			if (sunxi_internal_codec->adc_enable == 0) {
				snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<EN_AD), (0x0<<EN_AD));
			}
		}
		break;
	}
	mutex_unlock(&sunxi_internal_codec->adc_mutex);
	return 0;
}

static int ac_headphone_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *k,	int event)
{
	struct snd_soc_codec *codec = w->codec;
	pr_debug("..headphone power state change.\n");
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/*open*/
		snd_soc_update_bits(codec, PAEN_HP_CTRL, (0x1<<HPPAEN), (0x1<<HPPAEN));
		msleep(10);
		snd_soc_update_bits(codec, DAC_PA_SRC, (0x1<<LHPPAMUTE), (0x1<<LHPPAMUTE));
		snd_soc_update_bits(codec, DAC_PA_SRC, (0x1<<RHPPAMUTE), (0x1<<RHPPAMUTE));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/*close*/
		snd_soc_update_bits(codec, PAEN_HP_CTRL, (0x1<<HPPAEN), (0x0<<HPPAEN));
		snd_soc_update_bits(codec, DAC_PA_SRC, (0x1<<LHPPAMUTE), (0x0<<LHPPAMUTE));
		snd_soc_update_bits(codec, DAC_PA_SRC, (0x1<<RHPPAMUTE), (0x0<<RHPPAMUTE));
		break;
	}
	return 0;
}

static const struct snd_kcontrol_new sunxi_codec_controls[] = {
	SOC_SINGLE_TLV("digital volume", AC_DAC_DPC, DVOL, 0x3f, 0, dig_vol_tlv),
	/*analog control*/
	SOC_SINGLE_TLV("headphone volume", HP_VOLC, HPVOL, 0x3f, 0, headphone_vol_tlv),
	SOC_SINGLE_TLV("mic1 to lr output mixer control", MICIN_GCTRL, MIC1_GAIN, 0x7, 0, mic1_to_lroutp_mix_vol_tlv),
	SOC_SINGLE_TLV("mic2 to lr output mixer control", MICIN_GCTRL, MIC2_GAIN, 0x7, 0, mic2_to_lroutp_mix_vol_tlv),
	SOC_SINGLE_TLV("MIC1 boost AMP gain control", MIC1G_MICBIAS_CTRL, MIC1_BOOST, 0x7, 0, mic1_boost_vol_tlv),
	SOC_SINGLE_TLV("MIC2 boost AMP gain control", MIC2G_LINEEN_CTRL, MIC2BOOST, 0x7, 0, mic2_boost_vol_tlv),
	/*ADC*/
	SOC_SINGLE_TLV("ADC input gain control", ADC_AP_EN, ADCG, 0x7, 0, adc_input_gain_tlv),

	SOC_SINGLE_TLV("lineinl to r_output mixer gain", LINEIN_GCTRL, LINEINRG, 0x7, 0, lineinl_to_routp_mix_vol_tlv),
	SOC_SINGLE_TLV("lineinr to l_output mixer gain", LINEIN_GCTRL, LINEINLG, 0x7, 0, lineinr_to_loutp_mix_vol_tlv),
	SOC_SINGLE_TLV("lineinln to lr_output mixer gain", FM_GCTRL, LINEING, 0x7, 0, lineinln_to_lroutp_mix_vol_tlv),
	SOC_SINGLE_TLV("FMlr to lr output mixer gain", FM_GCTRL, FMG, 0x7, 0, fmlr_to_lroutp_mix_vol_tlv),
};

/*
* output mixer source select
* analog:0x01:defined left output mixer
*/
static const struct snd_kcontrol_new ac_loutmix_controls[] = {
	SOC_DAPM_SINGLE("DACR Switch", LOMIXSC, LMIX_RDAC, 1, 0),
	SOC_DAPM_SINGLE("DACL Switch", LOMIXSC, LMIX_LDAC, 1, 0),
	SOC_DAPM_SINGLE("FML Switch", LOMIXSC, LMIX_FML, 1, 0),
	SOC_DAPM_SINGLE("LINEINL Switch", LOMIXSC, LMIX_LINEINL, 1, 0),
	SOC_DAPM_SINGLE("LINEINLR Switch", LOMIXSC, LMIX_LINEINLR, 1, 0),
	SOC_DAPM_SINGLE("MIC2Booststage Switch", LOMIXSC, LMIX_MIC2_BST, 1, 0),
	SOC_DAPM_SINGLE("MIC1Booststage Switch", LOMIXSC, LMIX_MIC1_BST, 1, 0),
};

/*
* analog:0x02:defined right output mixer
*/
static const struct snd_kcontrol_new ac_routmix_controls[] = {
	SOC_DAPM_SINGLE("DACL Switch", ROMIXSC, RMIX_LDAC, 1, 0),
	SOC_DAPM_SINGLE("DACR Switch", ROMIXSC, RMIX_RDAC, 1, 0),
	SOC_DAPM_SINGLE("FMR Switch", ROMIXSC, RMIX_FMR, 1, 0),
	SOC_DAPM_SINGLE("LINEINR Switch", ROMIXSC, RMIX_LINEINR, 1, 0),
	SOC_DAPM_SINGLE("LINEINLR Switch", ROMIXSC, RMIX_LINEINLR, 1, 0),
	SOC_DAPM_SINGLE("MIC2Booststage Switch", ROMIXSC, RMIX_MIC2_BST, 1, 0),
	SOC_DAPM_SINGLE("MIC1Booststage Switch", ROMIXSC, RMIX_MIC1_BST, 1, 0),
};

/*
* hp source select
* 0x0a:headphone input source
*/
static const char *ac_hp_r_func_sel[] = {
	"DACR HPR Switch", "Right Analog Mixer HPR Switch"};
static const struct soc_enum ac_hp_r_func_enum =
	SOC_ENUM_SINGLE(DAC_PA_SRC, RHPIS, 2, ac_hp_r_func_sel);

static const struct snd_kcontrol_new ac_hp_r_func_controls =
	SOC_DAPM_ENUM("HP_R Mux", ac_hp_r_func_enum);

static const char *ac_hp_l_func_sel[] = {
	"DACL HPL Switch", "Left Analog Mixer HPL Switch"};
static const struct soc_enum ac_hp_l_func_enum =
	SOC_ENUM_SINGLE(DAC_PA_SRC, LHPIS, 2, ac_hp_l_func_sel);

static const struct snd_kcontrol_new ac_hp_l_func_controls =
	SOC_DAPM_ENUM("HP_L Mux", ac_hp_l_func_enum);

/*
* LADC SOURCE SELECT
* 0x0c:defined left input adc mixer
*/
static const struct snd_kcontrol_new ac_ladcmix_controls[] = {
	SOC_DAPM_SINGLE("r_output mixer Switch", LADCMIXSC, LADC_ROUT_MIX, 1, 0),
	SOC_DAPM_SINGLE("l_output mixer Switch", LADCMIXSC, LADC_LOUT_MIX, 1, 0),
	SOC_DAPM_SINGLE("fml mixer Switch", LADCMIXSC, LADC_FML, 1, 0),
	SOC_DAPM_SINGLE("lineinl Switch", LADCMIXSC, LADC_LINEINL, 1, 0),
	SOC_DAPM_SINGLE("lineinlr Switch", LADCMIXSC, LADC_LINEINLR, 1, 0),
	SOC_DAPM_SINGLE("MIC2 boost Switch", LADCMIXSC, LADC_MIC2_BST, 1, 0),
	SOC_DAPM_SINGLE("MIC1 boost Switch", LADCMIXSC, LADC_MIC1_BST, 1, 0),
};

/*
* RADC SOURCE SELECT
* 0x0d:defined right input adc mixer
*/
static const struct snd_kcontrol_new ac_radcmix_controls[] = {
	SOC_DAPM_SINGLE("l_output mixer Switch", RADCMIXSC, RADC_LOUT_MIX, 1, 0),
	SOC_DAPM_SINGLE("r_output mixer Switch", RADCMIXSC, RADC_ROUT_MIX, 1, 0),
	SOC_DAPM_SINGLE("fmr mixer Switch", RADCMIXSC, RADC_FMR, 1, 0),
	SOC_DAPM_SINGLE("lineinr Switch", RADCMIXSC, RADC_LINER, 1, 0),
	SOC_DAPM_SINGLE("lineinlr Switch", RADCMIXSC, RADC_LINEINLR, 1, 0),
	SOC_DAPM_SINGLE("MIC2 boost Switch", RADCMIXSC, RADC_MIC2_BST, 1, 0),
	SOC_DAPM_SINGLE("MIC1 boost Switch", RADCMIXSC, RADC_MIC1_BST, 1, 0),
};

/*
* PHONEOUT SOURCE SELECT
* 0x0d:defined phoneout mixer
*/
static const struct snd_kcontrol_new ac_phoneoutmix_controls[] = {
	SOC_DAPM_SINGLE("l_output mixer Switch", PHONEOUT_CTRL, PHONEOUTS0, 1, 0),
	SOC_DAPM_SINGLE("r_output mixer Switch", PHONEOUT_CTRL, PHONEOUTS1, 1, 0),
	SOC_DAPM_SINGLE("mic2 boost Switch", PHONEOUT_CTRL, PHONEOUTS2, 1, 0),
	SOC_DAPM_SINGLE("mic1 boost Switch", PHONEOUT_CTRL, PHONEOUTS3, 1, 0),
};

/*0x0b:mic2 source select*/
static const char *mic2src_text[] = {"MIC3","MIC2"};

static const struct soc_enum mic2src_enum =
	SOC_ENUM_SINGLE(MIC1G_MICBIAS_CTRL, MIC2_SS, 2, mic2src_text);

static const struct snd_kcontrol_new mic2src_mux =
	SOC_DAPM_ENUM("MIC2 SRC", mic2src_enum);

/*built widget*/
static const struct snd_soc_dapm_widget ac_dapm_widgets[] = {
	SND_SOC_DAPM_AIF_IN("DAC_L", "Playback", 0, DAC_PA_SRC, DACALEN, 0),
	SND_SOC_DAPM_AIF_IN("DAC_R", "Playback", 0, DAC_PA_SRC, DACAREN, 0),

	/*0x0a*/
	SND_SOC_DAPM_MIXER_E("Left Output Mixer", DAC_PA_SRC, LMIXEN, 0,
			ac_loutmix_controls, ARRAY_SIZE(ac_loutmix_controls), late_enable_dac, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER_E("Right Output Mixer", DAC_PA_SRC, RMIXEN, 0,
			ac_routmix_controls, ARRAY_SIZE(ac_routmix_controls), late_enable_dac, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MUX("HP_R Mux", SND_SOC_NOPM, 0, 0,	&ac_hp_r_func_controls),
	SND_SOC_DAPM_MUX("HP_L Mux", SND_SOC_NOPM, 0, 0,	&ac_hp_l_func_controls),

	/*output widget*/
	SND_SOC_DAPM_OUTPUT("HPOUTL"),
	SND_SOC_DAPM_OUTPUT("HPOUTR"),
	/*headphone*/
	SND_SOC_DAPM_HP("Headphone", ac_headphone_event),

	/*
	* INPUT widget
	* 0x0e Headset Microphone Bias Control Register
	*/
	/*Microphone Bias Control Register*/
	SND_SOC_DAPM_MICBIAS("MainMic Bias", MIC1G_MICBIAS_CTRL, MMICBIASEN, 0),
	SND_SOC_DAPM_INPUT("MIC1P"),
	SND_SOC_DAPM_INPUT("MIC1N"),
	SND_SOC_DAPM_INPUT("MIC2"),
	SND_SOC_DAPM_INPUT("MIC3"),
	SND_SOC_DAPM_INPUT("LINEINR"),
	SND_SOC_DAPM_INPUT("LINEINL"),
	SND_SOC_DAPM_INPUT("LINEINLR"),
	SND_SOC_DAPM_INPUT("FML"),
	SND_SOC_DAPM_INPUT("FMR"),

	/*mic1 reference*/
	SND_SOC_DAPM_PGA("MIC1 PGA", MIC1G_MICBIAS_CTRL, MIC1_AMPEN, 0, NULL, 0),
	/*0x0a mic2 reference*/
	SND_SOC_DAPM_PGA("MIC2 PGA", MIC2G_LINEEN_CTRL, MIC2AMPEN, 0, NULL, 0),
	/*0x0b: mic2 source select*/
	SND_SOC_DAPM_MUX("MIC2 SRC", SND_SOC_NOPM, 0, 0, &mic2src_mux),

	SND_SOC_DAPM_MIXER_E("LADC input Mixer", ADC_AP_EN, ADCLEN, 0,
		ac_ladcmix_controls, ARRAY_SIZE(ac_ladcmix_controls),late_enable_adc, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER_E("RADC input Mixer", ADC_AP_EN, ADCREN, 0,
		ac_radcmix_controls, ARRAY_SIZE(ac_radcmix_controls),late_enable_adc, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_AIF_OUT("ADC_L", "Capture", 0, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_AIF_OUT("ADC_R", "Capture", 0, SND_SOC_NOPM, 0, 0),

	SND_SOC_DAPM_MIXER("phone Output Mixer", PHONEOUT_CTRL, PHONEOUTEN, 0,
		ac_phoneoutmix_controls, ARRAY_SIZE(ac_phoneoutmix_controls)),
	SND_SOC_DAPM_OUTPUT("PHONEOUTP"),
	SND_SOC_DAPM_OUTPUT("PHONEOUTN"),
};

static const struct snd_soc_dapm_route ac_dapm_routes[] = {
	/*PLAYBACK*/
	{"Left Output Mixer", "DACL Switch",		"DAC_L"},
	{"Left Output Mixer", "DACR Switch",		"DAC_R"},
	{"Left Output Mixer", "FML Switch",			"FML"},
	{"Left Output Mixer", "LINEINL Switch",		"LINEINL"},
	{"Left Output Mixer", "LINEINLR Switch",		"LINEINLR"},
	{"Left Output Mixer", "MIC2Booststage Switch",		"MIC2 PGA"},
	{"Left Output Mixer", "MIC1Booststage Switch",		"MIC1 PGA"},

	{"Right Output Mixer", "DACR Switch",		"DAC_R"},
	{"Right Output Mixer", "DACL Switch",		"DAC_L"},
	{"Right Output Mixer", "FMR Switch",		"FMR"},
	{"Right Output Mixer", "LINEINR Switch",		"LINEINR"},
	{"Right Output Mixer", "LINEINLR Switch",		"LINEINLR"},
	{"Right Output Mixer", "MIC2Booststage Switch",		"MIC2 PGA"},
	{"Right Output Mixer", "MIC1Booststage Switch",		"MIC1 PGA"},

	/*hp mux*/
	{"HP_R Mux", "DACR HPR Switch",		"DAC_R"},
	{"HP_R Mux", "Right Analog Mixer HPR Switch",		"Right Output Mixer"},

	{"HP_L Mux", "DACL HPL Switch",		"DAC_L"},
	{"HP_L Mux", "Left Analog Mixer HPL Switch",		"Left Output Mixer"},

	/*hp endpoint*/
	{"HPOUTR", NULL,				"HP_R Mux"},
	{"HPOUTL", NULL,				"HP_L Mux"},

	{"Headphone", NULL,				"HPOUTR"},
	{"Headphone", NULL,				"HPOUTL"},

	/*CAPTURE*/
	{"MIC1 PGA", NULL,				"MIC1P"},
	{"MIC1 PGA", NULL,				"MIC1N"},

	{"MIC2 SRC", "MIC2",				"MIC2"},
	{"MIC2 SRC", "MIC3",				"MIC3"},
	{"MIC2 PGA", NULL,				"MIC2 SRC"},

	/*LADC SOURCE mixer*/
	{"LADC input Mixer", "MIC1 boost Switch",				"MIC1 PGA"},
	{"LADC input Mixer", "MIC2 boost Switch",				"MIC2 PGA"},
	{"LADC input Mixer", "lineinlr Switch",				"LINEINLR"},
	{"LADC input Mixer", "lineinl Switch",				"LINEINL"},
	{"LADC input Mixer", "fml mixer Switch",				"FML"},
	{"LADC input Mixer", "l_output mixer Switch",				"Left Output Mixer"},
	{"LADC input Mixer", "r_output mixer Switch",				"Right Output Mixer"},

	/*RADC SOURCE mixer*/
	{"RADC input Mixer", "MIC1 boost Switch",				"MIC1 PGA"},
	{"RADC input Mixer", "MIC2 boost Switch",				"MIC2 PGA"},
	{"RADC input Mixer", "lineinlr Switch",				"LINEINLR"},
	{"RADC input Mixer", "lineinr Switch",				"LINEINR"},
	{"RADC input Mixer", "fmr mixer Switch",				"FMR"},
	{"RADC input Mixer", "r_output mixer Switch",				"Right Output Mixer"},
	{"RADC input Mixer", "l_output mixer Switch",				"Left Output Mixer"},

	/*ADC--ADCMUX*/
	{"ADC_L", NULL, "LADC input Mixer"},
	{"ADC_R", NULL, "RADC input Mixer"},

	/*phoneout*/
	{"phone Output Mixer", "MIC1 boost Switch","MIC1 PGA"},
	{"phone Output Mixer", "mic2 boost Switch","MIC2 PGA"},
	{"phone Output Mixer", "r_output mixer Switch","Right Output Mixer"},
	{"phone Output Mixer", "l_output mixer Switch","Left Output Mixer"},

	{"PHONEOUTP", NULL,"phone Output Mixer"},
	{"PHONEOUTN", NULL,"phone Output Mixer"},
};

static int codec_start(struct snd_pcm_substream *substream, struct snd_soc_dai *codec_dai)
{
	return 0;
}

static int codec_mute(struct snd_soc_dai *codec_dai, int mute)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);

	if(sunxi_internal_codec->spkenable == true)
		msleep(sunxi_internal_codec->pa_sleep_time);

	return 0;
}

static void codec_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *codec_dai)
{
}

static int codec_trigger(struct snd_pcm_substream *substream,
                              int cmd, struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (cmd) {
			case SNDRV_PCM_TRIGGER_START:
			case SNDRV_PCM_TRIGGER_RESUME:
			case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
				/*enable dac drq*/
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<DAC_DRQ_EN), (0x1<<DAC_DRQ_EN));
				return 0;
			case SNDRV_PCM_TRIGGER_SUSPEND:
			case SNDRV_PCM_TRIGGER_STOP:
			case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<DAC_DRQ_EN), (0x0<<DAC_DRQ_EN));
				return 0;
			default:
				return -EINVAL;
			}
	} else {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<ADC_DRQ_EN), (0x1<<ADC_DRQ_EN));
			return 0;
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<ADC_DRQ_EN), (0x0<<ADC_DRQ_EN));
			return 0;
		default:
			pr_err("error:%s,%d\n", __func__, __LINE__);
			return -EINVAL;
		}
	}
	return 0;
}

static int codec_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *codec_dai)
{
	int i = 0;
	struct snd_soc_codec *codec = codec_dai->codec;

	for (i = 0; i < ARRAY_SIZE(codec_sr_s); i++) {
		if (codec_sr_s[i].samplerate ==  params_rate(params)) {
			if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x7<<DAC_FS), (codec_sr_s[i].srbit<<DAC_FS));
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x7<<DAC_FS), (codec_sr_s[i].srbit<<DAC_FS));
			} else {
				snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x7<<ADFS), (codec_sr_s[i].srbit<<ADFS));
				snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x7<<ADFS), (codec_sr_s[i].srbit<<ADFS));				
			}
			break;
		}
	}

	switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S32_LE:
			if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				/*set TX FIFO MODE:24bit*/
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x3<<FIFO_MODE), (0x2<<FIFO_MODE));
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<TX_SAMPLE_BITS), (0x1<<TX_SAMPLE_BITS));
			} else {
				/*set RX FIFO MODE:24bit*/
				snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<RX_FIFO_MODE), (0x0<<RX_FIFO_MODE));
				snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<RX_SAMPLE_BITS), (0x1<<RX_SAMPLE_BITS));
			}
		break;
		case SNDRV_PCM_FORMAT_S16_LE:
		default:
			if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				/*set TX FIFO MODE:16bit*/
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x3<<FIFO_MODE), (0x3<<FIFO_MODE));
				snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<TX_SAMPLE_BITS), (0x0<<TX_SAMPLE_BITS));
			} else {
				/*set RX FIFO MODE:16bit*/
				snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<RX_FIFO_MODE), (0x1<<RX_FIFO_MODE));
				snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<RX_SAMPLE_BITS), (0x0<<RX_SAMPLE_BITS));
			}
		break;
	}

	if (params_channels(params)==1) {
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<DAC_MONO_EN), (0x1<<DAC_MONO_EN));			
		} else {
			snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<ADC_MONO_EN), (0x1<<ADC_MONO_EN));
		}
	} else {
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			snd_soc_update_bits(codec, AC_DAC_FIFOC, (0x1<<DAC_MONO_EN), (0x0<<DAC_MONO_EN));
		} else {
			snd_soc_update_bits(codec, AC_ADC_FIFOC, (0x1<<ADC_MONO_EN), (0x0<<ADC_MONO_EN));
		}
	}

	return 0;
}

static int codec_set_dai_fmt(struct snd_soc_dai *codec_dai,
			       unsigned int fmt)
{
	return 0;
}

static int codec_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);

	if (clk_set_rate(sunxi_internal_codec->pllclk, freq)) {
		pr_err("[audio-cpudai]try to set the pll clk rate failed!\n");
	}

	return 0;
}

static int codec_set_bias_level(struct snd_soc_codec *codec,
				      enum snd_soc_bias_level level)
{
	codec->dapm.bias_level = level;
	return 0;
}

static const struct snd_soc_dai_ops codec_dai_ops = {
	.startup		= codec_start,
	.set_fmt		= codec_set_dai_fmt,
	.hw_params		= codec_hw_params,
	.shutdown		= codec_shutdown,
	.digital_mute	= codec_mute,
	.set_sysclk		= codec_set_dai_sysclk,
	.trigger 		= codec_trigger,
};

static struct snd_soc_dai_driver codec_dai[] = {
	{
		.name = "sun8iw11codec",
		.id = 1,
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
		 },
		.ops = &codec_dai_ops,
	},
};

static const struct snd_soc_component_driver sunxi_i2s_component = {
	.name		= DRV_NAME,
};

static int codec_soc_probe(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct snd_soc_dapm_context *dapm = &codec->dapm;
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);

	sunxi_internal_codec->codec = codec;
	sunxi_internal_codec->dac_enable = 0;
	sunxi_internal_codec->adc_enable = 0;
	mutex_init(&sunxi_internal_codec->dac_mutex);
	mutex_init(&sunxi_internal_codec->adc_mutex);

	/* Add virtual switch */
	ret = snd_soc_add_codec_controls(codec, sunxi_codec_controls,
					ARRAY_SIZE(sunxi_codec_controls));
	if (ret) {
		pr_err("[audio-codec] Failed to register audio mode control, "
				"will continue without it.\n");
	}

	snd_soc_dapm_new_controls(dapm, ac_dapm_widgets, ARRAY_SIZE(ac_dapm_widgets));
 	snd_soc_dapm_add_routes(dapm, ac_dapm_routes, ARRAY_SIZE(ac_dapm_routes));

	codec_init(sunxi_internal_codec);

	return 0;
}

int audio_gpio_iodisable(u32 gpio)
{
	char pin_name[8];
	u32 config,ret;
	sunxi_gpio_to_name(gpio, pin_name);
	config = (((7) << 16) | (0 & 0xFFFF));
	ret = pin_config_set(SUNXI_PINCTRL, pin_name, config);
	return ret;
}

static int codec_suspend(struct snd_soc_codec *codec)
{
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);
	pr_debug("[audio codec]:suspend start.\n");

	if (spk_gpio.cfg) {
		audio_gpio_iodisable(spk_gpio.gpio);
	}
	if (sunxi_internal_codec->moduleclk != NULL) {
		clk_disable(sunxi_internal_codec->moduleclk);
	}
	if (sunxi_internal_codec->pllclk != NULL) {
		clk_disable(sunxi_internal_codec->pllclk);
	}

	if (sunxi_internal_codec->vol_supply.cpvdd) {
		regulator_disable(sunxi_internal_codec->vol_supply.cpvdd);
	}

	if (sunxi_internal_codec->vol_supply.avcc) {
		regulator_disable(sunxi_internal_codec->vol_supply.avcc);
	}

	pr_debug("[audio codec]:suspend end..\n");

	return 0;
}

static int codec_resume(struct snd_soc_codec *codec)
{
	int ret ;
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);
	pr_debug("[audio codec]:resume start\n");

	if (sunxi_internal_codec->vol_supply.cpvdd) {
		ret = regulator_enable(sunxi_internal_codec->vol_supply.cpvdd);
		if (ret) {
			pr_err("[%s]: cpvdd:regulator_enable() failed!\n",__func__);
		}
	}

	if (sunxi_internal_codec->vol_supply.avcc) {
		ret = regulator_enable(sunxi_internal_codec->vol_supply.avcc);
		if (ret) {
			pr_err("[%s]: avcc:regulator_enable() failed!\n",__func__);
		}
	}

	if (sunxi_internal_codec->pllclk != NULL) {
		if (clk_prepare_enable(sunxi_internal_codec->pllclk)) {
			pr_err("open sunxi_internal_codec->pllclk failed! line = %d\n", __LINE__);
		}
	}

	if (sunxi_internal_codec->moduleclk != NULL) {
		if (clk_prepare_enable(sunxi_internal_codec->moduleclk)) {
			pr_err("open sunxi_internal_codec->moduleclk failed! line = %d\n", __LINE__);
		}
	}

	codec_init(sunxi_internal_codec);

	if (spk_gpio.cfg) {
		gpio_direction_output(spk_gpio.gpio, 1);
		gpio_set_value(spk_gpio.gpio, 0);
	}

	pr_debug("[audio codec]:resume end..\n");
	return 0;
}

/* power down chip */
static int codec_soc_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static unsigned int codec_read(struct snd_soc_codec *codec,
					  unsigned int reg)
{
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);

	if (reg <= 0x1e) {
		/*analog reg*/
		return read_prcm_wvalue(reg,sunxi_internal_codec->codec_abase);
	} else {
		/*digital reg*/
		return codec_rdreg(sunxi_internal_codec->codec_dbase + reg);
	}
}

static int codec_write(struct snd_soc_codec *codec,
				  unsigned int reg, unsigned int value)
{
	struct sunxi_codec *sunxi_internal_codec = snd_soc_codec_get_drvdata(codec);

	if (reg <= 0x1e) {
		/*analog reg*/
		write_prcm_wvalue(reg, value,sunxi_internal_codec->codec_abase);
	} else {
		/*digital reg*/
		codec_wrreg(sunxi_internal_codec->codec_dbase + reg, value);
	}
	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_codec = {
	.probe 	 = codec_soc_probe,
	.remove  = codec_soc_remove,
	.suspend = codec_suspend,
	.resume  = codec_resume,
	.set_bias_level = codec_set_bias_level,
	.read 	 = codec_read,
	.write 	 = codec_write,
	.ignore_pmdown_time = 1,
};

static ssize_t show_audio_reg(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int count = 0;
	int i = 0;
	int reg_group =0;

	count += sprintf(buf, "dump audio reg:\n");

	while (reg_labels[i].name != NULL) {
		if (reg_labels[i].value == 0) {
			reg_group++;
		}
		if (reg_group == 1) {
			count +=sprintf(buf + count, "%s 0x%p: 0x%x\n", reg_labels[i].name,
							(codec_digitaladress + reg_labels[i].value),
							readl(codec_digitaladress + reg_labels[i].value) );
		} else if (reg_group == 2) {
			count +=sprintf(buf + count, "%s 0x%x: 0x%x\n", reg_labels[i].name,
							(reg_labels[i].value),
						        read_prcm_wvalue(reg_labels[i].value,codec_analogadress) );
		}
		i++;
	}

	return count;
}

/* ex:
*param 1: 0 read;1 write
*param 2: 1 digital reg; 2 analog reg
*param 3: reg value;
*param 4: write value;
	read:
		echo 0,1,0x00> audio_reg
   		echo 0,2,0x00> audio_reg
	write:
		echo 1,1,0x00,0xa > audio_reg
		echo 1,2,0x00,0xff > audio_reg
*/
static ssize_t store_audio_reg(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int ret;
	int rw_flag;
	int reg_val_read;
	int input_reg_val =0;
	int input_reg_group =0;
	int input_reg_offset =0;

	ret = sscanf(buf, "%d,%d,0x%x,0x%x", &rw_flag,&input_reg_group, &input_reg_offset, &input_reg_val);
	printk("ret:%d, reg_group:%d, reg_offset:%d, reg_val:0x%x\n", ret, input_reg_group, input_reg_offset, input_reg_val);

	if (!(input_reg_group ==1 || input_reg_group ==2)) {
		pr_err("not exist reg group\n");
		ret = count;
		goto out;
	}
	if (!(rw_flag ==1 || rw_flag ==0)) {
		pr_err("not rw_flag\n");
		ret = count;
		goto out;
	}
	if (input_reg_group == 1) {
		if (rw_flag) {
			writel(input_reg_val, codec_digitaladress + input_reg_offset);
		} else {
			reg_val_read = readl(codec_digitaladress + input_reg_offset);
			printk("\n\n Reg[0x%x] : 0x%x\n\n",input_reg_offset,reg_val_read);
		}
	} else if (input_reg_group == 2) {
		if (rw_flag) {
			write_prcm_wvalue(input_reg_offset, input_reg_val & 0xff,codec_analogadress);
		} else {
			 reg_val_read = read_prcm_wvalue(input_reg_offset,codec_analogadress);
			 printk("\n\n Reg[0x%x] : 0x%x\n\n",input_reg_offset,reg_val_read);
		}
	}

	ret = count;

out:
	return ret;
}

static DEVICE_ATTR(audio_reg, 0644, show_audio_reg, store_audio_reg);

static struct attribute *audio_debug_attrs[] = {
	&dev_attr_audio_reg.attr,
	NULL,
};

static struct attribute_group audio_debug_attr_group = {
	.name   = "audio_reg_debug",
	.attrs  = audio_debug_attrs,
};
static const struct of_device_id sunxi_codec_of_match[] = {
	{ .compatible = "allwinner,sunxi-internal-codec", },
	{},
};

static int __init sunxi_internal_codec_probe(struct platform_device *pdev)
{
	s32 ret = 0;
	u32 temp_val = 0;
	struct resource res;
	struct gpio_config config;
	const struct of_device_id *device;
	struct sunxi_codec *sunxi_internal_codec;
	struct device_node *node = pdev->dev.of_node;

	if (!node) {
		dev_err(&pdev->dev,
			"can not get dt node for this device.\n");
		ret = -EINVAL;
		goto err0;
	}

	sunxi_internal_codec = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_codec), GFP_KERNEL);
	if (!sunxi_internal_codec) {
		dev_err(&pdev->dev, "Can't allocate sunxi_codec\n");
		ret = -ENOMEM;
		goto err0;
	}
	dev_set_drvdata(&pdev->dev, sunxi_internal_codec);

	device = of_match_device(sunxi_codec_of_match, &pdev->dev);
	if (!device) {
		ret = -ENODEV;
		goto err1;
	}
	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		return -ENODEV;
	}

	sunxi_internal_codec->pllclk = of_clk_get(node, 0);
	sunxi_internal_codec->moduleclk= of_clk_get(node, 1);
	if (IS_ERR(sunxi_internal_codec->pllclk) || IS_ERR(sunxi_internal_codec->moduleclk)){
		dev_err(&pdev->dev, "[audio-cpudai]Can't get cpudai clocks\n");
		if (IS_ERR(sunxi_internal_codec->pllclk))
			ret = PTR_ERR(sunxi_internal_codec->pllclk);
		else
			ret = PTR_ERR(sunxi_internal_codec->moduleclk);
		goto err1;
	} else {
		if (clk_set_parent(sunxi_internal_codec->moduleclk, sunxi_internal_codec->pllclk)) {
			pr_err("try to set parent of sunxi_spdif->moduleclk to sunxi_spdif->pllclk failed! line = %d\n",__LINE__);
		}
		clk_prepare_enable(sunxi_internal_codec->pllclk);
		clk_prepare_enable(sunxi_internal_codec->moduleclk);
	}

#if 0
	/*voltage*/
	sunxi_internal_codec->vol_supply.cpvdd =  regulator_get(NULL, "vcc-cpvdd");/*HPVCC*/
	if (!sunxi_internal_codec->vol_supply.cpvdd) {
		pr_err("get audio cpvdd failed\n");
		ret = -EFAULT;
		goto err1;
	} else {
		ret = regulator_enable(sunxi_internal_codec->vol_supply.cpvdd);
		if (ret) {
			pr_err("[%s]: cpvdd:regulator_enable() failed!\n",__func__);
			goto err1;
		}
	}

	sunxi_internal_codec->vol_supply.avcc = regulator_get(NULL, "vcc-avcc");
	if (!sunxi_internal_codec->vol_supply.avcc) {
		pr_err("[%s]:get audio avcc failed\n",__func__);
		ret = -EFAULT;
		goto err1;
	} else {
		ret = regulator_enable(sunxi_internal_codec->vol_supply.avcc);
		if (ret) {
			pr_err("[%s]: avcc:regulator_enable() failed!\n",__func__);
			goto err1;
		}
	}
#endif

	sunxi_internal_codec->codec_abase = NULL;
	sunxi_internal_codec->codec_dbase = NULL;
	sunxi_internal_codec->codec_dbase = of_iomap(node, 0);
	if (NULL == sunxi_internal_codec->codec_dbase) {
		pr_err("[audio-codec]Can't map codec digital registers\n");
	} else {
		codec_digitaladress = sunxi_internal_codec->codec_dbase;
	}
	sunxi_internal_codec->codec_abase = of_iomap(node, 1);
	if (NULL == sunxi_internal_codec->codec_abase) {
		pr_err("[audio-codec]Can't map codec analog registers\n");
	} else {
		codec_analogadress = sunxi_internal_codec->codec_abase;
	}

	/*initial speaker gpio */
	spk_gpio.gpio = of_get_named_gpio_flags(node, "gpio-spk", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(spk_gpio.gpio)) {
		pr_err("failed to get gpio-spk gpio from dts,spk_gpio:%d\n",spk_gpio.gpio);
		spk_gpio.cfg = 0;
	} else {
		ret = devm_gpio_request(&pdev->dev, spk_gpio.gpio, "SPK");
		if (ret) {
			spk_gpio.cfg = 0;
			pr_err("failed to request gpio-spk gpio\n");
			goto err1;
		} else {
			spk_gpio.cfg = 1;
			gpio_direction_output(spk_gpio.gpio, 1);
			gpio_set_value(spk_gpio.gpio, 0);
		}
	}

	ret = of_property_read_u32(node, "headphonevol",&temp_val);
	if (ret < 0) {
		pr_err("[audio-codec]headphonevol configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_internal_codec->gain_config.headphonevol = temp_val;
	}
	ret = of_property_read_u32(node, "spkervol",&temp_val);
	if (ret < 0) {
		pr_err("[audio-codec]spkervol configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_internal_codec->gain_config.spkervol = temp_val;
	}
	ret = of_property_read_u32(node, "maingain",&temp_val);
	if (ret < 0) {
		pr_err("[audio-codec]maingain configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_internal_codec->gain_config.maingain = temp_val;
	}
	ret = of_property_read_u32(node, "headsetmicgain",&temp_val);
	if (ret < 0) {
		pr_err("[audio-codec]headsetmicgain configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_internal_codec->gain_config.headsetmicgain = temp_val;
	}
	ret = of_property_read_u32(node, "hp_dirused",&temp_val);
	if (ret < 0) {
		pr_err("[audio-codec]hp_dirused configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_internal_codec->hp_dirused = temp_val;
	}
	ret = of_property_read_u32(node, "pa_sleep_time",&temp_val);
	if (ret < 0) {
		pr_err("[audio-codec]pa_sleep_time configurations missing or invalid.\n");
		ret = -EINVAL;
		goto err1;
	} else {
		sunxi_internal_codec->pa_sleep_time = temp_val;
	}

	pr_debug("headphonevol:%d, spkervol:%d, maingain:%d, pa_sleep_time:%d\n",
		sunxi_internal_codec->gain_config.headphonevol,
		sunxi_internal_codec->gain_config.spkervol,
		sunxi_internal_codec->gain_config.maingain,
		sunxi_internal_codec->pa_sleep_time
	);

	snd_soc_register_codec(&pdev->dev, &soc_codec_dev_codec, codec_dai, ARRAY_SIZE(codec_dai));

	ret  = sysfs_create_group(&pdev->dev.kobj, &audio_debug_attr_group);
	if (ret){
		pr_err("[audio-codec]failed to create attr group\n");
	}
	return 0;
err1:
	devm_kfree(&pdev->dev, sunxi_internal_codec);
err0:
	return ret;
}

static int __exit sunxi_internal_codec_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &audio_debug_attr_group);
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static void sunxi_internal_codec_shutdown(struct platform_device *pdev)
{
	struct sunxi_codec * sunxi_internal_codec = dev_get_drvdata(&pdev->dev);

	snd_soc_update_bits(sunxi_internal_codec->codec, PAEN_HP_CTRL, (0x1<<HPPAEN), (0x0<<HPPAEN));

	if (spk_gpio.cfg)
		gpio_set_value(spk_gpio.gpio, 0);
}

static struct platform_driver sunxi_internal_codec_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_codec_of_match,
	},
	.probe = sunxi_internal_codec_probe,
	.remove = __exit_p(sunxi_internal_codec_remove),
	.shutdown = sunxi_internal_codec_shutdown,
};

module_platform_driver(sunxi_internal_codec_driver);

MODULE_DESCRIPTION("codec ALSA soc codec driver");
MODULE_AUTHOR("huanxin<huanxin@allwinnertech.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-pcm-codec");

