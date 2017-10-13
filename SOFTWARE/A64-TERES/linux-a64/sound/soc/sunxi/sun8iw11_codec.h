/*
 * sound\soc\sunxi\sun8iw11codec.h
 * (C) Copyright 2014-2016
 * allwinner Technology Co., Ltd. <www.allwinnertech.com>
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
#ifndef _SUN8IW11_CODEC_H
#define _SUN8IW11_CODEC_H

#define AC_DAC_DPC		0x00
#define AC_DAC_FIFOC	0x04
#define AC_DAC_FIFOS	0x08
#define AC_ADC_FIFOC	0x10
#define AC_ADC_FIFOS	0x14
#define AC_ADC_RXDATA	0x18
#define AC_ADC_TXDATA	0x20
#define AC_DAC_CNT		0x40
#define AC_ADC_CNT		0x44
#define AC_DAC_DG		0x48
#define AC_ADC_DG		0x4c
#define AC_HMIC_CTRL	0x50
#define AC_HMIC_DATA	0x54

#define AC_PR_CFG		0x300

#define HP_VOLC				0x00
#define LOMIXSC				0x01
#define ROMIXSC 			0x02
#define DAC_PA_SRC			0x03
#define LINEIN_GCTRL    	0x04
#define FM_GCTRL			0x05
#define MICIN_GCTRL			0x06
#define PAEN_HP_CTRL		0x07
#define PHONEOUT_CTRL		0x08
#define MIC2G_LINEEN_CTRL 	0x0a
#define MIC1G_MICBIAS_CTRL	0x0b
#define LADCMIXSC			0x0c
#define RADCMIXSC			0x0d
#define PA_POP_CTR			0x0e
#define ADC_AP_EN			0x0f
#define ADDA_APTO			0x10
#define ADDA_APT1			0x11
#define ADDA_APT2			0x12
#define DA16_CAL_CTRL		0x13
#define BIAS_DA16_CAL_CTR	0x14
#define DA16_CALI_DATA		0x15
#define BIAS_CALI_DATA		0x17
#define BIAS_CALI_SET		0x18

/*AC_DAC_DPC:0x00*/
#define EN_DAC					31
#define MODQU					25
#define HPF_EN					18
#define DVOL					12
#define HUB_EN					0

/*AC_DAC_FIFOC:0x04*/
#define DAC_FS					29
#define FIR_VER					28
#define SEND_LASAT				26
#define FIFO_MODE				24
#define DAC_DRQ_CLR_CNT			21
#define TX_TRIG_LEVEL			8
#define ADDA_LOOP_EN			7
#define DAC_MONO_EN				6
#define TX_SAMPLE_BITS			5
#define DAC_DRQ_EN				4
#define DAC_IRQ_EN				3
#define FIFO_UNDERRUN_IRQ_EN 	2
#define FIFO_OVERRUN_IRQ_EN		1
#define FIFO_FLUSH				0

/*AC_ADC_FIFOC:0x10*/
#define ADFS				29
#define EN_AD				28
#define RX_FIFO_MODE		24
#define ADCFDT				17
#define ADCDFEN				16
#define RX_FIFO_TRG_LEVEL	8
#define ADC_MONO_EN			7
#define RX_SAMPLE_BITS		6
#define ADC_DRQ_EN			4
#define ADC_IRQ_EN			3
#define ADC_OVERRUN_IRQ_EN	1
#define ADC_FIFO_FLUSH		0

/*AC_ADC_TXDATA:0x20*/
#define TX_DATA			0

/*AC_DAC_CNT:0x40*/
#define TX_CNT			0

/*AC_ADC_CNT:0x44*/
#define RX_CNT			0

/*AC_HMIC_CTRL	0x50*/
#define HMIC_M				28
#define HMIC_N				24
#define HMIC_DATA_IRQ_MODE	23
#define HMIC_TH1_HYSTERESIS	21
#define HMIC_PULLOUT_IRQ	20
#define HMIC_PLUGIN_IRQ		19
#define HMIC_KEYUP_IRQ		18
#define HMIC_KEYDOWN_IRQ	17
#define HMIC_DATA_IRQ_EN	16
#define HMIC_SAMPLE_SELECT	14
#define HMIC_TH2_HYSTERESIS	13
#define HMIC_TH2			8
#define HMIC_SF				6
#define KEYUP_CLEAR			5
#define HMIC_TH1			0

/*AC_HMIC_DATA	0x54*/
#define HMIC_PULLOUT_PEND	20
#define HMIC_PLUGIN_PEND	19
#define HMIC_KEYUP_PEND		18
#define HMIC_KEYDOWN_PEND	17
#define HMIC_DATA_PEND		16
#define HMIC_DATA			0

/*AC_DAC_DG:0x48*/
/*
*	DAC Modulator Debug
*	0:DAC Modulator Normal Mode
*	1:DAC Modulator Debug Mode
*/
#define DAC_MODU_SELECT	11
/*
* 	DAC Pattern Select
*	00:Normal(Audio sample from TX fifo)
*	01: -6 dB sin wave
*	10: -60 dB sin wave
*	11: silent wave
*/
#define DAC_PATTERN_SELECT 9
/*
* 	CODEC Clock Source Select
*	0:codec clock from PLL
* 	1:codec clock from OSC(for debug)
*/
#define CODEC_CLK_SELECT	8
/*
*	DAC output channel swap enable
*	0:disable
*	1:enable
*/
#define DA_SWP			6

/*AC_ADC_DG:0x4c*/
#define AD_SWP			24

/*AC_PR_CFG:0x300*/
#define AC_PR_RST		28
#define AC_PR_RW		24
#define AC_PR_ADDR		16
#define ADDA_PR_WDAT	8
#define ADDA_PR_RDAT	0

/*HP_VOLC:0x00*/
#define PA_CLK_GATING	7
#define HPVOL			0

/*LOMIXSC:0x01*/
#define LMIXMUTE		0
#define LMIX_MIC1_BST	6
#define LMIX_MIC2_BST	5
#define LMIX_LINEINLR	4
#define LMIX_LINEINL	3
#define LMIX_FML		2
#define LMIX_LDAC		1
#define LMIX_RDAC		0

/*ROMIXSC:0x02*/
#define RMIXMUTE		0
#define RMIX_MIC1_BST	6
#define RMIX_MIC2_BST	5
#define RMIX_LINEINLR	4
#define RMIX_LINEINR	3
#define RMIX_FMR		2
#define RMIX_RDAC		1
#define RMIX_LDAC		0

/*DAC_PA_SRC:0x03*/
#define DACAREN			7
#define DACALEN			6
#define RMIXEN			5
#define LMIXEN			4
#define RHPPAMUTE		3
#define LHPPAMUTE		2
#define RHPIS			1
#define LHPIS			0

/*LINEIN_GCTRL:0x04*/
#define LINEINLG		4
#define LINEINRG		0

/*FM_GCTRL:0x05*/
#define FMG				4
#define LINEING			0

/*MICIN_GCTRL:0x06*/
#define MIC1_GAIN		4 //mic1
#define MIC2_GAIN		0

/*PAEN_HP_CTRL:0x07*/
#define HPPAEN				7
#define HPCOM_FC			5
#define HPCOM_PT			4
#define PA_ANTI_POP_CTRL	2
#define LTRNMUTE			1
#define RTLNMUTE			0

/*PHONEOUT_CTRL:0x08*/
#define PHONEOUTG		5
#define PHONEOUTEN		4
#define PHONEOUTS3		3
#define PHONEOUTS2		2
#define PHONEOUTS1		1
#define PHONEOUTS0		0

/*MIC2G_LINEEN_CTRL:0x0a*/
#define MIC2AMPEN		7
#define MIC2BOOST		4

/*MIC1G_MICBIAS_CTRL:0x0b*/
#define HMICBIASEN		7
#define MMICBIASEN		6
#define HMICBIASMODE	5
#define MIC2_SS			4/*0:MICIN3,1:MICIN2*/
#define MIC1_AMPEN		3
#define MIC1_BOOST		0

/*LADCMIXSC:0x0c*/
#define LADCMIXMUTE		0
#define LADC_MIC1_BST	6
#define LADC_MIC2_BST	5
#define LADC_LINEINLR	4
#define LADC_LINEINL	3
#define LADC_FML		2
#define LADC_LOUT_MIX	1
#define LADC_ROUT_MIX	0

/*RADCMIXSC:0x0d*/
#define RADC_MIC1_BST	6
#define RADC_MIC2_BST	5
#define RADC_LINEINLR 	4
#define RADC_LINER		3
#define RADC_FMR		2
#define RADC_ROUT_MIX	1
#define RADC_LOUT_MIX	0

/*PA_POP_CTR:0x0e*/
#define PA_POP_CTRL		0

/*ADC_AP_EN:0x0f*/
#define ADCREN			7
#define ADCLEN			6
#define ADCG			0

/*ADDA_APT2:0x12*/
#define PA_SLOPE_SELECT	3

/*DA16_CAL_CTRL:0x13*/
#define MMIC_BIASCHOPEN	7
#define DITHER			4

/*BIAS_DA16_CAL_CTR:0x14*/
#define PA_SPEED_SELECT	7

struct label {
    const char *name;
    int value;
};

#define LABEL(constant) { #constant, constant }
#define LABEL_END { NULL, -1 }

struct spk_gpio_ {
	u32 gpio;
	bool cfg;
};
struct gain_config {
	u32 headphonevol;
	u32 spkervol;
	u32 maingain;
	u32 headsetmicgain;
};

struct codec_hw_config {
	u32 adcagc_cfg:1;
	u32 adcdrc_cfg:1;
	u32 dacdrc_cfg:1;
	u32 adchpf_cfg:1;
	u32 dachpf_cfg:1;
};

struct voltage_supply {
	struct regulator *cpvdd;
	struct regulator *avcc;
};

struct sunxi_codec {
	void __iomem *codec_dbase;
	void __iomem *codec_abase;
	struct clk *srcclk;
	struct pinctrl *pinctrl;

	struct gain_config gain_config;
	struct codec_hw_config hwconfig;

	struct mutex dac_mutex;
	struct mutex adc_mutex;
	struct snd_soc_codec *codec;
	struct snd_soc_dai_driver dai;
	struct voltage_supply vol_supply;
	struct clk *pllclk;
	struct clk *moduleclk;
	u32 dac_enable;
	u32 adc_enable;
	u32 pa_sleep_time;
	bool hp_dirused;
	bool spkenable;
};
#endif

