/*
 * sound\soc\codec\acx00.c
 * (C) Copyright 2010-2016
 * allwinnertech Technology Co., Ltd. <www.allwinnertechtech.com>
 * huangxin <huangxin@allwinnertechtech.com>
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
#include <sound/tlv.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <linux/i2c.h>
#include <linux/switch.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/power/scenelock.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#include <linux/pinctrl/consumer.h>
#include "acx00.h"

static int lineout_val 		= 0;
static int mainmic_val 		= 0;
static int submic_val 		= 0;
static int ac200_used 	= 0;
static int ac100_used 	= 0;

#define acx00_RATES  (SNDRV_PCM_RATE_8000_192000|SNDRV_PCM_RATE_KNOT)
#define acx00_FORMATS (SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | \
		                     SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE)

/*struct for acx00*/
struct acx00_priv {
	struct acx00 *acx00;
	struct snd_soc_codec *codec;
	struct work_struct codec_resume;
	struct mutex aifclk_mutex;
	struct mutex dac_mutex;
	struct mutex adc_mutex;
	u8 dac_enable;
	u8 adc_enable;
	u8 aif1_clken;
};

static void get_configuration(void)
{
}

static void set_configuration(struct snd_soc_codec *codec)
{
	snd_soc_update_bits(codec, DACA_OMIXER_CTRL, (0x7<<MIC1G), (mainmic_val<<MIC1G));
	snd_soc_update_bits(codec, DACA_OMIXER_CTRL, (0x7<<MIC2G), (submic_val<<MIC2G));
	snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUT_RIGHT_SEL), (0x1<<LINEOUT_RIGHT_SEL));
	snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUT_LEFT_SEL), (0x1<<LINEOUT_LEFT_SEL));
	snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUTEN), (0x1<<LINEOUTEN));
	msleep(2500);
//	gpio_set_value(item.gpio.gpio, 1);
}

static int late_enable_dac(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct acx00_priv *acx00 = snd_soc_codec_get_drvdata(codec);

	mutex_lock(&acx00->dac_mutex);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (acx00->dac_enable == 0) {
			ACX00_DBG("%s,line:%d\n",__func__,__LINE__);
			/*enable dac module clk*/
			snd_soc_update_bits(codec, I2S_CTL, (0x1<<RXEN), (0x1<<RXEN));
			snd_soc_update_bits(codec, SYS_CLK_CTL, (0x1<<CTL_DAC_DIGITAL), (0x1<<CTL_DAC_DIGITAL));
			snd_soc_update_bits(codec, SYS_MOD_RST, (0x1<<RST_DAC_DIGITAL), (0x1<<RST_DAC_DIGITAL));
			snd_soc_update_bits(codec, AC_DAC_DPC, (0x1<<ENDA), (0x1<<ENDA));
			snd_soc_update_bits(codec, AC_DAC_DPC, (0x1<<ENHPF), (0x1<<ENHPF));
		}
		acx00->dac_enable++;
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (acx00->dac_enable > 0) {
			acx00->dac_enable--;
			if (acx00->dac_enable == 0) {
				ACX00_DBG("%s,line:%d\n",__func__,__LINE__);
				snd_soc_update_bits(codec, AC_DAC_DPC, (0x1<<ENDA), (0x0<<ENDA));
				/*disable dac module clk*/
				snd_soc_update_bits(codec, SYS_CLK_CTL, (0x1<<CTL_DAC_DIGITAL), (0x0<<CTL_DAC_DIGITAL));
				snd_soc_update_bits(codec, SYS_MOD_RST, (0x1<<RST_DAC_DIGITAL), (0x0<<RST_DAC_DIGITAL));
				snd_soc_update_bits(codec, AC_DAC_DPC, (0x1<<ENHPF), (0x0<<ENHPF));
				snd_soc_update_bits(codec, I2S_CTL, (0x1<<RXEN), (0x0<<RXEN));
			}
		}
		break;
	}
	mutex_unlock(&acx00->dac_mutex);
	return 0;
}

static int late_enable_adc(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct acx00_priv *acx00 = snd_soc_codec_get_drvdata(codec);
	mutex_lock(&acx00->adc_mutex);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (acx00->adc_enable == 0) {
			ACX00_DBG("%s,line:%d\n",__func__,__LINE__);
			/*enable adc module clk*/
			snd_soc_update_bits(codec, I2S_CTL, (0x1<<TXEN), (0x1<<TXEN));
			snd_soc_update_bits(codec, SYS_CLK_CTL, (0x1<<CTL_ADC_DIGITAL), (0x1<<CTL_ADC_DIGITAL));
			snd_soc_update_bits(codec, SYS_MOD_RST, (0x1<<RST_ADC_DIGITAL), (0x1<<RST_ADC_DIGITAL));
			snd_soc_update_bits(codec, AC_ADC_DPC, (0x1<<ENAD), (0x1<<ENAD));
		}
		acx00->adc_enable++;
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (acx00->adc_enable > 0) {
			acx00->adc_enable--;
			if (acx00->adc_enable == 0) {
				ACX00_DBG("%s,line:%d\n",__func__,__LINE__);
				snd_soc_update_bits(codec, AC_ADC_DPC, (0x1<<ENAD), (0x0<<ENAD));
				/*disable adc module clk*/
				snd_soc_update_bits(codec, SYS_CLK_CTL, (0x1<<CTL_ADC_DIGITAL), (0x0<<CTL_ADC_DIGITAL));
				snd_soc_update_bits(codec, SYS_MOD_RST, (0x1<<RST_ADC_DIGITAL), (0x0<<RST_ADC_DIGITAL));
				snd_soc_update_bits(codec, I2S_CTL, (0x1<<TXEN), (0x0<<TXEN));
			}
		}
		break;
	}
	mutex_unlock(&acx00->adc_mutex);
	return 0;
}

static int acx00_lineout_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *k,
				int event)
{
	struct snd_soc_codec *codec = w->codec;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		ACX00_DBG("[speaker open ]%s,line:%d\n",__func__,__LINE__);
		snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1f<<LINEOUTVOL), (lineout_val<<LINEOUTVOL));
		break;
	case SND_SOC_DAPM_PRE_PMD :
		ACX00_DBG("[speaker close ]%s,line:%d\n",__func__,__LINE__);
		snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1f<<LINEOUTVOL), (0x0<<LINEOUTVOL));
	default:
		break;
	}
	return 0;
}

int acx00_aif1clk(struct snd_soc_dapm_widget *w,
		  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	struct acx00_priv *acx00 = snd_soc_codec_get_drvdata(codec);

	mutex_lock(&acx00->aifclk_mutex);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (acx00->aif1_clken == 0) {
			ACX00_DBG("%s,l:%d\n", __func__, __LINE__);
			/*enable SDO0 TX GLOBAL*/
			snd_soc_update_bits(codec, SYS_CLK_CTL, (0x1<<CTL_I2S), (0x1<<CTL_I2S));
			snd_soc_update_bits(codec, SYS_MOD_RST, (0x1<<RST_I2S), (0x1<<RST_I2S));
			snd_soc_update_bits(codec, I2S_CTL, (0x1<<SDO0_EN), (0x1<<SDO0_EN));
			snd_soc_update_bits(codec, I2S_CTL, (0x1<<GEN), (0x1<<GEN));
		}
		acx00->aif1_clken++;
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (acx00->aif1_clken > 0) {
			acx00->aif1_clken--;
			if (acx00->aif1_clken == 0) {
				ACX00_DBG("%s,l:%d\n", __func__, __LINE__);
				/*disable SDO0 TX GLOBAL*/
				snd_soc_update_bits(codec, I2S_CTL, (0x1<<SDO0_EN), (0x0<<SDO0_EN));
				snd_soc_update_bits(codec, I2S_CTL, (0x1<<RXEN), (0x0<<RXEN));
				snd_soc_update_bits(codec, I2S_CTL, (0x1<<GEN), (0x0<<GEN));
				snd_soc_update_bits(codec, SYS_CLK_CTL, (0x1<<CTL_I2S), (0x0<<CTL_I2S));
				snd_soc_update_bits(codec, SYS_MOD_RST, (0x1<<RST_I2S), (0x0<<RST_I2S));
			}
		}
		break;
	}
	mutex_unlock(&acx00->aifclk_mutex);
	return 0;
}

static const DECLARE_TLV_DB_SCALE(mic1_boost_vol_tlv, -450, 600, 0);
static const DECLARE_TLV_DB_SCALE(mic2_boost_vol_tlv, -450, 600, 0);
static const DECLARE_TLV_DB_SCALE(linein_amp_vol_tlv, -450, 600, 0);
static const DECLARE_TLV_DB_SCALE(lineout_vol_tlv, -4800, 150, 0);
static const DECLARE_TLV_DB_SCALE(i2s_adc_mix_vol_tlv, -600, 0, 0);
static const struct snd_kcontrol_new acx00_controls[] = {
	SOC_DOUBLE_TLV("i2s ADC mixer gain", I2S_MIX_GAIN, I2S_MIXL_ADCL_G, I2S_MIXR_ADCR_G, 0x3, 0, i2s_adc_mix_vol_tlv),
	SOC_SINGLE_TLV("MIC1 boost amplifier gain", DACA_OMIXER_CTRL, MIC1G, 0x7, 0, mic1_boost_vol_tlv),
	SOC_SINGLE_TLV("MIC2 boost amplifier gain", DACA_OMIXER_CTRL, MIC2G, 0x7, 0, mic2_boost_vol_tlv),
	SOC_SINGLE_TLV("LINEINL-LINEINR pre-amplifier gain", DACA_OMIXER_CTRL, LINEING, 0x7, 0, linein_amp_vol_tlv),
	SOC_SINGLE_TLV("line out volume", LINEOUT_CTRL, LINEOUTVOL, 0x1f, 0, lineout_vol_tlv),
};

/*0x2202 register*/
/*AC_DAC_MIX_SRC SOURCE*/
static const struct snd_kcontrol_new ac_dacl_mixer_src_ctl[] = {
	SOC_DAPM_SINGLE("I2S_DACDATL Switch", AC_DAC_MIX_SRC, I2S_DACDATL, 1, 0),
	SOC_DAPM_SINGLE("ADCDATL Switch", AC_DAC_MIX_SRC, ADCDATL, 1, 0),
};
static const struct snd_kcontrol_new ac_dacr_mixer_src_ctl[] = {
	SOC_DAPM_SINGLE("I2S_DACDATR Switch", AC_DAC_MIX_SRC,  	I2S_DACDATR, 1, 0),
	SOC_DAPM_SINGLE("ADCDATR Switch", AC_DAC_MIX_SRC, ADCDATR, 1, 0),
};

/*output mixer source select:0x2222*/
/*defined left output mixer*/
static const struct snd_kcontrol_new acx00_loutmix_controls[] = {
	SOC_DAPM_SINGLE("DACR Switch", OMIXER_SR, L_DACR, 1, 0),
	SOC_DAPM_SINGLE("DACL Switch", OMIXER_SR, L_DACL, 1, 0),
	SOC_DAPM_SINGLE("LINEINL Switch", OMIXER_SR, L_LINEINL, 1, 0),
	SOC_DAPM_SINGLE("PHONEN Switch", OMIXER_SR, L_PHONEN, 1, 0),
	SOC_DAPM_SINGLE("PHONEN-PHONEP Switch", OMIXER_SR, L_PHONENP, 1, 0),
	SOC_DAPM_SINGLE("MIC2Booststage Switch", OMIXER_SR, L_MIC2_BOOST, 1, 0),
	SOC_DAPM_SINGLE("MIC1Booststage Switch", OMIXER_SR, L_MIC1_BOOST, 1, 0),
};

/*defined right output mixer*/
static const struct snd_kcontrol_new acx00_routmix_controls[] = {
	SOC_DAPM_SINGLE("DACL Switch", OMIXER_SR, R_DACL, 1, 0),
	SOC_DAPM_SINGLE("DACR Switch", OMIXER_SR, R_DACR, 1, 0),
	SOC_DAPM_SINGLE("LINEINR Switch", OMIXER_SR, R_LINEINR, 1, 0),
	SOC_DAPM_SINGLE("PHONEP Switch", OMIXER_SR, R_PHONEP, 1, 0),
	SOC_DAPM_SINGLE("PHONEN-PHONEP Switch", OMIXER_SR, R_PHONENP, 1, 0),
	SOC_DAPM_SINGLE("MIC2Booststage Switch", OMIXER_SR, R_MIC2_BOOST, 1, 0),
	SOC_DAPM_SINGLE("MIC1Booststage Switch", OMIXER_SR, R_MIC1_BOOST, 1, 0),
};

/*lineout source select:0x2224*/
static const char *acx00_rlineouts_func_sel[] = {
	"MIXER Switch", "MIXR MIXL Switch"};
static const struct soc_enum acx00_rlineouts_func_enum =
	SOC_ENUM_SINGLE(LINEOUT_CTRL, R_LINEOUT_SOURCE_SEL, 2, acx00_rlineouts_func_sel);

static const struct snd_kcontrol_new acx00_rlineouts_func_controls =
	SOC_DAPM_ENUM("LINEOUT_R Mux", acx00_rlineouts_func_enum);

static const char *acx00_llineouts_l_func_sel[] = {
	"MIXEL Switch", "MIXL MIXR Switch"};
static const struct soc_enum acx00_llineouts_func_enum =
	SOC_ENUM_SINGLE(LINEOUT_CTRL, L_LINEOUT_SOURCE_SEL, 2, acx00_llineouts_l_func_sel);

static const struct snd_kcontrol_new acx00_llineouts_func_controls =
	SOC_DAPM_ENUM("LINEOUT_L Mux", acx00_llineouts_func_enum);
/*0x2122*/
static const char *acx00_i2s_dacdatl_func_sel[] = {
	"I2S_DACL Switch", "I2S_DACR Switch", "SUM I2S_DACL I2S_DACR SWITCH", "AVE I2S_DACL I2S_DACR SWITCH"};
static const struct soc_enum acx00_i2sdacdatl_func_enum =
	SOC_ENUM_SINGLE(I2S_DACDAT_CTL, I2S_DACL_SRC, 4, acx00_i2s_dacdatl_func_sel);
static const struct snd_kcontrol_new acx00_i2sdacdatl_func_controls =
	SOC_DAPM_ENUM("I2S DACDATL Mux", acx00_i2sdacdatl_func_enum);

static const char *acx00_i2s_dacdatr_func_sel[] = {
	"I2S_DACR Switch", "I2S_DACL Switch", "SUM I2S_DACL I2S_DACR SWITCH", "AVE I2S_DACL I2S_DACR SWITCH"};
static const struct soc_enum acx00_i2sdacdatr_func_enum =
	SOC_ENUM_SINGLE(I2S_DACDAT_CTL, I2S_DACR_SRC, 2, acx00_i2s_dacdatr_func_sel);
static const struct snd_kcontrol_new acx00_i2sdacdatr_func_controls =
	SOC_DAPM_ENUM("I2S DACDATR Mux", acx00_i2sdacdatr_func_enum);

/*record register*/
/*0x2114 I2S_MIX_SRC SOURCE*/
static const struct snd_kcontrol_new ac_adcl_mixer_src_ctl[] = {
	SOC_DAPM_SINGLE("I2S_DACDATL Switch", I2S_MIX_SRC, I2S_MIXL_DACDATL, 1, 0),
	SOC_DAPM_SINGLE("ADCDATL Switch", I2S_MIX_SRC, I2S_MIXL_ADCL, 1, 0),
};

static const struct snd_kcontrol_new ac_adcr_mixer_src_ctl[] = {
	SOC_DAPM_SINGLE("I2S_DACDATR Switch", I2S_MIX_SRC, I2S_MIXR_DACDATR, 1, 0),
	SOC_DAPM_SINGLE("ADCDATR Switch", I2S_MIX_SRC, I2S_MIXR_ADCR, 1, 0),
};

/*0x2120*/
static const char *acx00_i2s_adcdatl_func_sel[] = {
	"I2S_ADCL Switch", "I2S_ADCR Switch", "SUM I2S_ADCL I2S_ADCR SWITCH", "AVE I2S_ADCL I2S_ADCR SWITCH"};
static const struct soc_enum acx00_i2sadcdatl_func_enum =
	SOC_ENUM_SINGLE(I2S_ADCDAT_CTL, I2S_ADCL_SRC, 4, acx00_i2s_adcdatl_func_sel);
static const struct snd_kcontrol_new acx00_i2sadcdatl_func_controls =
	SOC_DAPM_ENUM("I2S ADCDATL Mux", acx00_i2sadcdatl_func_enum);

static const char *acx00_i2s_adcdatr_func_sel[] = {
	"I2S_ADCR Switch", "I2S_ADCL Switch", "SUM I2S_ADCL I2S_ADCR SWITCH", "AVE I2S_ADCL I2S_ADCR SWITCH"};
static const struct soc_enum acx00_i2sadcdatr_func_enum =
	SOC_ENUM_SINGLE(I2S_ADCDAT_CTL, I2S_ADCR_SRC, 2, acx00_i2s_adcdatr_func_sel);
static const struct snd_kcontrol_new acx00_i2sadcdatr_func_controls =
	SOC_DAPM_ENUM("I2S ADCDATR Mux", acx00_i2sadcdatr_func_enum);

/*ADC SOURCE SELECT*/
/*defined left input adc mixer*/
static const struct snd_kcontrol_new acx00_ladcmix_controls[] = {
	SOC_DAPM_SINGLE("MIC1 boost Switch", ADCMIXER_SR, LADC_MIC1_BOOST, 1, 0),
	SOC_DAPM_SINGLE("MIC2 boost Switch", ADCMIXER_SR, LADC_MIC2_BOOST, 1, 0),
	SOC_DAPM_SINGLE("PHONEPN Switch", ADCMIXER_SR, LADC_PHONEPN, 1, 0),
	SOC_DAPM_SINGLE("PHONEN Switch", ADCMIXER_SR, LADC_PHONEN, 1, 0),
	SOC_DAPM_SINGLE("LINEINL Switch", ADCMIXER_SR, LADC_LINEINL, 1, 0),
	SOC_DAPM_SINGLE("Lout_Mixer Switch", ADCMIXER_SR, LADC_LEFT_OUTPUT_MIX, 1, 0),
	SOC_DAPM_SINGLE("Rout_Mixer Switch", ADCMIXER_SR, LADC_RIGHT_OUTPUT_MIX, 1, 0),
};

/*defined right input adc mixer*/
static const struct snd_kcontrol_new acx00_radcmix_controls[] = {
	SOC_DAPM_SINGLE("MIC1 boost Switch", ADCMIXER_SR, RADC_MIC1_BOOST, 1, 0),
	SOC_DAPM_SINGLE("MIC2 boost Switch", ADCMIXER_SR, RADC_MIC2_BOOST, 1, 0),
	SOC_DAPM_SINGLE("PHONEPN Switch", ADCMIXER_SR, RADC_PHONEPN, 1, 0),
	SOC_DAPM_SINGLE("PHONEP Switch", ADCMIXER_SR, RADC_PHONEP, 1, 0),
	SOC_DAPM_SINGLE("LIENINR Switch", ADCMIXER_SR, RADC_LINEINR, 1, 0),
	SOC_DAPM_SINGLE("Rout_Mixer Switch", ADCMIXER_SR, RADC_RIGHT_OUTPUT_MIX, 1, 0),
	SOC_DAPM_SINGLE("Lout_Mixer Switch", ADCMIXER_SR, RADC_LEFT_OUTPUT_MIX, 1, 0),
};

/*built widget*/
static const struct snd_soc_dapm_widget acx00_dapm_widgets[] = {
	SND_SOC_DAPM_MIXER_E("Left Output Mixer", DACA_OMIXER_CTRL, LMIXEN, 0, acx00_loutmix_controls, ARRAY_SIZE(acx00_loutmix_controls),
		late_enable_dac, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER_E("Right Output Mixer", DACA_OMIXER_CTRL, RMIXEN, 0, acx00_routmix_controls, ARRAY_SIZE(acx00_routmix_controls),
		late_enable_dac, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MIXER("AC DACL mixer", SND_SOC_NOPM, 0, 0, ac_dacl_mixer_src_ctl, ARRAY_SIZE(ac_dacl_mixer_src_ctl)),
	SND_SOC_DAPM_MIXER("AC DACR mixer", SND_SOC_NOPM, 0, 0, ac_dacr_mixer_src_ctl, ARRAY_SIZE(ac_dacr_mixer_src_ctl)),
	SND_SOC_DAPM_MUX("LINEOUT_R Mux", SND_SOC_NOPM, 0, 0, &acx00_rlineouts_func_controls),
	SND_SOC_DAPM_MUX("LINEOUT_L Mux", SND_SOC_NOPM, 0, 0, &acx00_llineouts_func_controls),

	SND_SOC_DAPM_AIF_IN_E("DAC_L", "AIF1 Playback", 0, DACA_OMIXER_CTRL, DACALEN, 0,acx00_aif1clk,
		   SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_AIF_IN_E("DAC_R", "AIF1 Playback", 0, DACA_OMIXER_CTRL, DACAREN, 0,acx00_aif1clk,
		   SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_OUTPUT("LINEOUTP"),
	SND_SOC_DAPM_OUTPUT("LINEOUTN"),
	/*speaker*/
	SND_SOC_DAPM_SPK("External Speaker", acx00_lineout_event),

	/*INPUT widget*/
	SND_SOC_DAPM_INPUT("MIC1P"),
	SND_SOC_DAPM_INPUT("MIC1N"),
	SND_SOC_DAPM_INPUT("MIC2P"),
	SND_SOC_DAPM_INPUT("MIC2N"),
	SND_SOC_DAPM_INPUT("LINEINP"),
	SND_SOC_DAPM_INPUT("LINEINN"),

	SND_SOC_DAPM_MICBIAS("MainMic Bias", MBIAS_CTRL, MMICBIASEN, 0),
	SND_SOC_DAPM_MICBIAS("MMIC Bias choppen", MBIAS_CTRL, MMIC_BIAS_CHOP_EN, 0),

	/*mic reference*/
	SND_SOC_DAPM_PGA("MIC1 PGA", ADC_MIC_CTRL, MIC1AMPEN, 0, NULL, 0),
	SND_SOC_DAPM_PGA("MIC2 PGA", ADC_MIC_CTRL, MIC2AMPEN, 0, NULL, 0),
	SND_SOC_DAPM_PGA("LINEIN PGA", SND_SOC_NOPM, 0, 0, NULL, 0),
	/*aif1 interface*/
	SND_SOC_DAPM_AIF_OUT_E("AIF1ADCL", "AIF1 Capture", 0, SND_SOC_NOPM, 0, 0,acx00_aif1clk,
		   SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_AIF_OUT_E("AIF1ADCR", "AIF1 Capture", 0, SND_SOC_NOPM, 0, 0,acx00_aif1clk,
		   SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MIXER("AC ADCL mixer", SND_SOC_NOPM, 0, 0, ac_adcl_mixer_src_ctl, ARRAY_SIZE(ac_adcl_mixer_src_ctl)),
	SND_SOC_DAPM_MIXER("AC ADCR mixer", SND_SOC_NOPM, 0, 0, ac_adcr_mixer_src_ctl, ARRAY_SIZE(ac_adcr_mixer_src_ctl)),

	SND_SOC_DAPM_MIXER_E("LEFT ADC input Mixer", ADC_MIC_CTRL, ADCLEN, 0,
		acx00_ladcmix_controls, ARRAY_SIZE(acx00_ladcmix_controls),late_enable_adc, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER_E("RIGHT ADC input Mixer", ADC_MIC_CTRL, ADCREN, 0,
		acx00_radcmix_controls, ARRAY_SIZE(acx00_radcmix_controls),late_enable_adc, SND_SOC_DAPM_PRE_PMU|SND_SOC_DAPM_POST_PMD),
};

static const struct snd_soc_dapm_route acx00_dapm_routes[] = {
	/*PLAYBACK:AIF1 DAC in*/
	{"AC DACL mixer", "I2S_DACDATL Switch", "DAC_L"},
	{"AC DACR mixer", "I2S_DACDATR Switch", "DAC_R"},

	{"LINEOUT_L Mux", "MIXEL Switch", "AC DACL mixer"},
	{"LINEOUT_R Mux", "MIXER Switch", "AC DACR mixer"},

	{"Left Output Mixer", "DACL Switch", "LINEOUT_L Mux"},
	{"Right Output Mixer", "DACR Switch", "LINEOUT_R Mux"},

	{"LINEOUTP", NULL,	"Left Output Mixer"},
	{"LINEOUTN", NULL,	"Right Output Mixer"},

	{"External Speaker", NULL,	"LINEOUTP"},
	{"External Speaker", NULL,	"LINEOUTN"},

	/*RECORDER: AIF1 ADC OUT*/
	{"MIC1 PGA", NULL, "MIC1P"},
	{"MIC1 PGA", NULL, "MIC1N"},

	{"MIC2 PGA", NULL, "MIC2P"},
	{"MIC2 PGA", NULL, "MIC2N"},

	{"AC ADCL mixer", "ADCDATL Switch", "LEFT ADC input Mixer"},
	{"AC ADCR mixer", "ADCDATR Switch", "RIGHT ADC input Mixer"},

	/*LADC SOURCE mixer*/
	{"LEFT ADC input Mixer", "MIC1 boost Switch","MIC1 PGA"},
	{"LEFT ADC input Mixer", "MIC2 boost Switch", "MIC2 PGA"},
	/*RADC SOURCE mixer*/
	{"RIGHT ADC input Mixer", "MIC1 boost Switch", "MIC1 PGA"},
	{"RIGHT ADC input Mixer", "MIC2 boost Switch", "MIC2 PGA"},

	{"AIF1ADCL", NULL, "LEFT ADC input Mixer"},
	{"AIF1ADCR", NULL, "RIGHT ADC input Mixer"},
};

struct aif1_fs {
	unsigned int samplerate;
	int aif1_bclk_div;
	int aif1_srbit;
};

struct aif1_word_size {
	int aif1_wsize_val;
	int aif1_wsize_bit;
};

struct aif1_sample_res {
	int sample_res_val;
	int sample_res_bit;
};

static const struct aif1_fs codec_aif1_fs[] = {
	{44100, 0x5, 7},
	{48000, 0x5, 8},
	{8000, 0xa, 0},
	{11025, 0x9, 1},
	{12000, 0x9, 2},
	{16000, 0x8, 3},
	{22050, 0x7, 4},
	{24000, 0x7, 5},
	{32000, 0x6, 6},
	{96000, 0x3, 9},
	{192000, 0x2, 10},
};

static const struct aif1_sample_res codec_sample_wres[] = {
	{8, 1},
	{12, 2},
	{16, 3},
	{20, 4},
	{24, 5},
	{28, 6},
	{32, 7},
};

static const struct aif1_word_size codec_aif1_wsize[] = {
	{8, 1},
	{12, 2},
	{16, 3},
	{20, 4},
	{24, 5},
	{28, 6},
	{32, 7},
};

static int acx00_aif_mute(struct snd_soc_dai *codec_dai, int mute)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	if (mute) {
		snd_soc_write(codec, I2S_DACDAT_DVC, 0);
	} else {
		snd_soc_write(codec, I2S_DACDAT_DVC, 0xa0a0);
	}
	return 0;
}

static void acx00_aif_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *codec_dai)
{
	ACX00_DBG("%s,line:%d\n", __func__, __LINE__);
}

static int acx00_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *codec_dai)
{
	int i = 0;
	int aif1_word_size = 32;
	int aif1_sample_res= 16;
	struct snd_soc_codec *codec = codec_dai->codec;

	for (i = 0; i < ARRAY_SIZE(codec_aif1_fs); i++) {
		if (codec_aif1_fs[i].samplerate ==  params_rate(params)) {
			snd_soc_update_bits(codec, SYS_SAMP_CTL, (0xf<<SYS_FS), ((codec_aif1_fs[i].aif1_srbit)<<SYS_FS));
			snd_soc_update_bits(codec, I2S_CLK, (0xf<<BCLKDIV), ((codec_aif1_fs[i].aif1_bclk_div)<<BCLKDIV));
			break;
		}
	}
	switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S32_LE:
			aif1_word_size = 24;
			aif1_sample_res = 24;
		break;
		case SNDRV_PCM_FORMAT_S16_LE:
		default:
			aif1_word_size = 16;
			aif1_sample_res = 16;
		break;
	}
	for (i = 0; i < ARRAY_SIZE(codec_aif1_wsize); i++) {
		if (codec_aif1_wsize[i].aif1_wsize_val == aif1_word_size) {
			snd_soc_update_bits(codec, I2S_FMT0, (0x7<<SW), ((codec_aif1_wsize[i].aif1_wsize_bit)<<SW));
			break;
		}
	}
	for (i = 0; i < ARRAY_SIZE(codec_sample_wres); i++) {
		if (codec_sample_wres[i].sample_res_val == aif1_sample_res) {
			snd_soc_update_bits(codec, I2S_FMT0, (0x7<<SR), ((codec_sample_wres[i].sample_res_bit)<<SR));
			break;
		}
	}
	ACX00_DBG("%s,line:%d\n", __func__, __LINE__);
	return 0;
}

static int acx00_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int acx00_set_dai_fmt(struct snd_soc_dai *codec_dai,
			       unsigned int fmt)
{
	int reg_val;
	int reg_val1;
	struct snd_soc_codec *codec = codec_dai->codec;

	ACX00_DBG("%s,line:%d\n", __func__, __LINE__);
	/*
	* 	master or slave selection
	*	0 = Master mode
	*	1 = Slave mode
	*/
	reg_val = snd_soc_read(codec, I2S_CLK);
	reg_val &=~(0x1<<LRCK_OUT);
	reg_val &=~(0x1<<BCLK_OUT);
	switch(fmt & SND_SOC_DAIFMT_MASTER_MASK){
		case SND_SOC_DAIFMT_CBM_CFM:   /* codec clk & frm master, ap is slave*/
			reg_val |= (0x1<<LRCK_OUT);
			reg_val |= (0x1<<BCLK_OUT);
			break;
		case SND_SOC_DAIFMT_CBS_CFS:   /* codec clk & frm slave,ap is master*/
			reg_val |= (0x0<<LRCK_OUT);
			reg_val |= (0x0<<BCLK_OUT);
			break;
		default:
			pr_err("unknwon master/slave format\n");
			return -EINVAL;
	}
	snd_soc_write(codec, I2S_CLK, reg_val);

	/* i2s mode selection TODO...*/
	reg_val1 = snd_soc_read(codec, I2S_CLK);
	reg_val = snd_soc_read(codec, I2S_FMT0);
	reg_val &= ~(0x3<<MODE_SEL);
	reg_val1 &= ~(0x3ff)<<LRCK_PERIOD;
	switch(fmt & SND_SOC_DAIFMT_FORMAT_MASK){
		case SND_SOC_DAIFMT_I2S:        /* I2S1 mode */
			reg_val |= (0x1<<MODE_SEL);
			reg_val |= (0x1<<TX_OFFSET);
			reg_val |= (0x1<<RX_OFFSET);
			reg_val1|= (0x1f<<LRCK_PERIOD);/*pcm mode:0x3f;i2s mode:0x1f*/
			break;
		case SND_SOC_DAIFMT_RIGHT_J:    /* Right Justified mode */
			reg_val |= (0x2<<MODE_SEL);
			reg_val |= (0x0<<TX_OFFSET);
			reg_val |= (0x0<<RX_OFFSET);
			reg_val1|= (0x3f<<LRCK_PERIOD);
			break;
		case SND_SOC_DAIFMT_LEFT_J:     /* Left Justified mode */
			reg_val |= (0x1<<MODE_SEL);
			reg_val |= (0x0<<TX_OFFSET);
			reg_val |= (0x0<<RX_OFFSET);
			reg_val1|= (0x3f<<LRCK_PERIOD);
			break;
		case SND_SOC_DAIFMT_DSP_A:      /* L reg_val msb after FRM LRC */
			reg_val |= (0x0<<MODE_SEL);
			reg_val |= (0x1<<TX_OFFSET);
			reg_val |= (0x1<<RX_OFFSET);
			reg_val1|= (0x3f<<LRCK_PERIOD);
			break;
		default:
			pr_err("%s, line:%d\n", __func__, __LINE__);
			return -EINVAL;
	}
	snd_soc_write(codec, I2S_FMT0, reg_val);
	snd_soc_write(codec, I2S_CLK, reg_val1);

	/* DAI signal inversions */
	reg_val = snd_soc_read(codec, I2S_FMT1);
	switch(fmt & SND_SOC_DAIFMT_INV_MASK){
		case SND_SOC_DAIFMT_NB_NF:     /* normal bit clock + nor frame */
			reg_val &= ~(0x1<<BCLK_POLARITY);
			reg_val &= ~(0x1<<LRCK_POLARITY);
			break;
		case SND_SOC_DAIFMT_NB_IF:     /* normal bclk + inv frm */
			reg_val |= (0x1<<BCLK_POLARITY);
			reg_val &= ~(0x1<<LRCK_POLARITY);
			break;
		case SND_SOC_DAIFMT_IB_NF:     /* invert bclk + nor frm */
			reg_val &= ~(0x1<<BCLK_POLARITY);
			reg_val |= (0x1<<LRCK_POLARITY);
			break;
		case SND_SOC_DAIFMT_IB_IF:     /* invert bclk + inv frm */
			reg_val |= (0x1<<BCLK_POLARITY);
			reg_val |= (0x1<<LRCK_POLARITY);
			break;
	}
	snd_soc_write(codec, I2S_FMT1, reg_val);

	return 0;
}

static int acx00_set_fll(struct snd_soc_dai *codec_dai, int pll_id, int source,
									unsigned int freq_in, unsigned int freq_out)
{
	ACX00_DBG("%s, line:%d, pll_id:%d\n", __func__, __LINE__, pll_id);
	if (!freq_out)
		return 0;
	if ((freq_in < 128000) || (freq_in > 24576000)) {
		return -EINVAL;
	} else if ((freq_in == 24576000) || (freq_in == 22579200)) {
		return 0;
	}
	return 0;
}

static int acx00_audio_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *codec_dai)
{
	ACX00_DBG("%s,line:%d\n", __func__, __LINE__);
	return 0;
}

static int acx00_set_bias_level(struct snd_soc_codec *codec,
				      enum snd_soc_bias_level level)
{
	switch (level) {
	case SND_SOC_BIAS_ON:
		ACX00_DBG("%s,line:%d, SND_SOC_BIAS_ON\n", __func__, __LINE__);
		break;
	case SND_SOC_BIAS_PREPARE:
		ACX00_DBG("%s,line:%d, SND_SOC_BIAS_PREPARE\n", __func__, __LINE__);
		break;
	case SND_SOC_BIAS_STANDBY:
		ACX00_DBG("%s,line:%d, SND_SOC_BIAS_STANDBY\n", __func__, __LINE__);
		break;
	case SND_SOC_BIAS_OFF:
		ACX00_DBG("%s,line:%d, SND_SOC_BIAS_OFF\n", __func__, __LINE__);
		break;
	}
	codec->dapm.bias_level = level;
	return 0;
}

static const struct snd_soc_dai_ops acx00_aif1_dai_ops = {
	.set_sysclk	= acx00_set_dai_sysclk,
	.set_fmt	= acx00_set_dai_fmt,
	.hw_params	= acx00_hw_params,
	.shutdown	= acx00_aif_shutdown,
	.digital_mute	= acx00_aif_mute,
	.set_pll	= acx00_set_fll,
	.startup 	= acx00_audio_startup,
};

static struct snd_soc_dai_driver acx00_dai[] = {
	{
		.name = "acx00-aif1",
		.id = 1,
		.playback = {
			.stream_name = "AIF1 Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = acx00_RATES,
			.formats = acx00_FORMATS,
		},
		.capture = {
			.stream_name = "AIF1 Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = acx00_RATES,
			.formats = acx00_FORMATS,
		 },
		.ops = &acx00_aif1_dai_ops,
	},
};

static void codec_resume_work(struct work_struct *work)
{
	struct acx00_priv *acx00 = container_of(work, struct acx00_priv, codec_resume);
	struct snd_soc_codec *codec = acx00->codec;

	get_configuration();
	set_configuration(codec);
}

/************************************************************/
static int acx00_codec_probe(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct acx00_priv *acx00;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	acx00 = dev_get_drvdata(codec->dev);
	if (acx00 == NULL) {
		return -ENOMEM;
	}
	acx00->codec = codec;

	snd_soc_codec_set_drvdata(codec, acx00);
	INIT_WORK(&acx00->codec_resume, codec_resume_work);
	acx00->dac_enable = 0;
	acx00->adc_enable = 0;
	acx00->aif1_clken = 0;
	mutex_init(&acx00->dac_mutex);
	mutex_init(&acx00->adc_mutex);
	mutex_init(&acx00->aifclk_mutex);

	/*request pa gpio*/
//	ret = gpio_request(item.gpio.gpio, NULL);
//	if (0 != ret) {
//		pr_err("request gpio failed!\n");
//	}
	/*
	* config gpio info of audio_pa_ctrl, the default pa config is close(check pa sys_config1.fex)
	*/
//	gpio_direction_output(item.gpio.gpio, 1);
//	gpio_set_value(item.gpio.gpio, 0);
	snd_soc_write(codec, 0x0010,0x3);
	snd_soc_write(codec, 0x0012,0x1);
	schedule_work(&acx00->codec_resume);

	ret = snd_soc_add_codec_controls(codec, acx00_controls,
					ARRAY_SIZE(acx00_controls));
	if (ret) {
		pr_err("[ACX00] Failed to register audio mode control, "
				"will continue without it.\n");
	}

	snd_soc_dapm_new_controls(dapm, acx00_dapm_widgets, ARRAY_SIZE(acx00_dapm_widgets));
 	snd_soc_dapm_add_routes(dapm, acx00_dapm_routes, ARRAY_SIZE(acx00_dapm_routes));

	return 0;
}

/* power down chip */
static int acx00_codec_remove(struct snd_soc_codec *codec)
{
	struct acx00_priv *acx00 = snd_soc_codec_get_drvdata(codec);

	kfree(acx00);

	return 0;
}

static int acx00_codec_suspend(struct snd_soc_codec *codec)
{
//	char pin_name[SUNXI_PIN_NAME_MAX_LEN];
//	unsigned long      config;

	ACX00_DBG("[codec]:suspend\n");
	/* check if called in talking standby */
	if (check_scene_locked(SCENE_TALKING_STANDBY) == 0) {
		pr_err("In talking standby, audio codec do not suspend!!\n");
		return 0;
	}
	acx00_set_bias_level(codec, SND_SOC_BIAS_OFF);

	snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUT_RIGHT_SEL), (0x0<<LINEOUT_RIGHT_SEL));
	snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUT_LEFT_SEL), (0x0<<LINEOUT_LEFT_SEL));
	snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUTEN), (0x0<<LINEOUTEN));
	msleep(30);
//	gpio_set_value(item.gpio.gpio, 0);

	return 0;
}

static int acx00_codec_resume(struct snd_soc_codec *codec)
{
	struct acx00_priv *acx00 = snd_soc_codec_get_drvdata(codec);

	ACX00_DBG("[codec]:resume");
//	gpio_direction_output(item.gpio.gpio, 1);
//	gpio_set_value(item.gpio.gpio, 0);
	acx00_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	schedule_work(&acx00->codec_resume);

	return 0;
}

static unsigned int sndvir_audio_read(struct snd_soc_codec *codec,
					  unsigned int reg)
{
	unsigned int data;
    struct acx00_priv *acx00 = snd_soc_codec_get_drvdata(codec);
    struct acx00 *acx00_dev = acx00->acx00;

	/* Device I/O API */
	data = acx00_reg_read(acx00_dev, reg);

	return data;
}

static int sndvir_audio_write(struct snd_soc_codec *codec,
				  unsigned int reg, unsigned int value)
{
	int ret = 0;
	struct acx00_priv *acx00 = snd_soc_codec_get_drvdata(codec);
	struct acx00 *acx00_dev = acx00->acx00;

	ret = acx00_reg_write(acx00_dev, reg, value);

	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_sndvir_audio = {
	.probe 		=	acx00_codec_probe,
	.remove 	=   acx00_codec_remove,
	.suspend 	= 	acx00_codec_suspend,
	.resume 	=	acx00_codec_resume,
	.set_bias_level = acx00_set_bias_level,
	.read 		= 	sndvir_audio_read,
	.write 		= 	sndvir_audio_write,
	.ignore_pmdown_time = 1,
};

static int acx00_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct acx00_priv *acx00;

	if (ac200_used&&(ac100_used==0)) {
		acx00 = devm_kzalloc(&pdev->dev, sizeof(struct acx00_priv), GFP_KERNEL);
		if (acx00 == NULL) {
			return -ENOMEM;
		}
		platform_set_drvdata(pdev, acx00);

		acx00->acx00 = dev_get_drvdata(pdev->dev.parent);

		ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_sndvir_audio, acx00_dai, ARRAY_SIZE(acx00_dai));
		if (ret < 0) {
			dev_err(&pdev->dev, "Failed to register acx00: %d\n", ret);
		}
	}
	return 0;
}

static void acx00_shutdown(struct platform_device *pdev)
{
	struct acx00_priv *acx00 = NULL;
	struct snd_soc_codec *codec = NULL;

	if (ac200_used&&(ac100_used==0)) {
		acx00 = platform_get_drvdata(pdev);
		codec = acx00->codec;

		/*disable lineout*/
		snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUTEN), (0<<LINEOUTEN));
		/*disable pa_ctrl*/
	}
}

static int acx00_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver acx00_codec_driver = {
	.driver = {
		.name = "acx00-codec",
		.owner = THIS_MODULE,
	},
	.probe = acx00_probe,
	.remove = acx00_remove,
	.shutdown = acx00_shutdown,
};
module_platform_driver(acx00_codec_driver);

MODULE_DESCRIPTION("ASoC ACX00 driver");
MODULE_AUTHOR("huangxin");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:acx00-codec");
