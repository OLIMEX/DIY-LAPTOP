/*
 * sound\soc\codecs\acx00.h
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#ifndef _SNDCODEC_H
#define _SNDCODEC_H
#include <linux/mfd/acx00-mfd.h>
/*pll source*/
#define ACX00_MCLK1 1
#define ACX00_MCLK2 2
#define ACX00_BCLK1 3
#define ACX00_BCLK2 4

#define AIF1_CLK 1
#define AIF2_CLK 2

/*0x2000~0x3fff:8k*/
#define SYS_CLK_CTL			0x2000
#define SYS_MOD_RST			0x2002
#define SYS_SAMP_CTL		0x2004

#define I2S_CTL				0x2100
#define I2S_CLK				0x2102
#define I2S_FMT0			0x2104
#define I2S_FMT1			0x2108
#define I2S_TXCOF			0x210a
#define I2S_TXSEL			0x210c
#define I2S_TXMAP			0x210e
#define I2S_RXSEL			0x2110
#define I2S_RXMAP			0x2112
#define I2S_MIX_SRC			0x2114
#define I2S_MIX_GAIN		0x2116
#define I2S_DACDAT_DVC		0x2118
#define I2S_ADCDAT_DVC		0x211a
#define I2S_ADCDAT_CTL		0x2120 /*ADD*/
#define I2S_DACDAT_CTL		0x2122 /*ADD*/

#define AC_DAC_DPC			0x2200
#define AC_DAC_MIX_SRC		0x2202
#define AC_DAC_MIX_GAIN		0x2204
#define AC_DAC_DBG			0x2206
#define DACA_OMIXER_CTRL	0x2220
#define OMIXER_SR			0x2222
#define LINEOUT_CTRL		0x2224

#define PHOUT_CTRL			0x3360

#define AC_ADC_DPC			0x2300
#define AC_ADC_DBG			0x2304
#define MBIAS_CTRL			0x2310	/*add*/
#define ADC_MIC_CTRL		0x2320
#define ADCMIXER_SR			0x2322
#define ANALOG_TUNING0		0x232a /*modify*/
#define ANALOG_TUNING1		0x232c /*modify*/
//#define DLDO_OSC_CTRL		0x2340 /*del*/
//#define ALDO_CTRL			0x2342 /*del*/

#define AC_AGC_SEL			0x2480	/*add*/

#define AC_ADC_DAPL_CTRL	0x2500
#define AC_ADC_DAPR_CTRL	0x2502
#define AC_ADC_DAPLSTA		0x2504
#define AC_ADC_DAPRSTA		0x2506
#define AC_ADC_DAP_LTL		0x2508
#define AC_ADC_DAP_RTL		0x250a
#define AC_ADC_DAP_LHAC		0x250c
#define AC_ADC_DAP_LLAC		0x250e
#define AC_ADC_DAP_RHAC		0x2510
#define AC_ADC_DAP_RLAC		0x2512
#define AC_ADC_DAP_LDT		0x2514
#define AC_ADC_DAP_LAT		0x2516
#define AC_ADC_DAP_RDT		0x2518
#define AC_ADC_DAP_RAT		0x251a
#define AC_ADC_DAP_NTH		0x251c
#define AC_ADC_DAP_LHNAC	0x251e
#define AC_ADC_DAP_LLNAC	0x2520
#define AC_ADC_DAP_RHNAC	0x2522
#define AC_ADC_DAP_RLNAC	0x2524
#define AC_ADC_DAP_HHPFC	0x2526
#define AC_ADC_DAP_LHPFC	0x2528
#define AC_ADC_DAP_OPT		0x252a

#define AC_DRC_SEL			0x2f80

#define AC_DRC_CHAN_CTRL	0x3000
#define AC_DRC_HHPFC		0x3002
#define AC_DRC_LHPFC		0x3004
#define AC_DRC_CTRL			0x3006
#define AC_DRC_LPFHAT		0x3008
#define AC_DRC_LPFLAT		0x300a
#define AC_DRC_RPFHAT		0x300c
#define AC_DRC_RPFLAT		0x300e
#define AC_DRC_LPFHRT		0x3010
#define AC_DRC_LPFLRT		0x3012
#define AC_DRC_RPFHRT		0x3014
#define AC_DRC_RPFLRT		0x3016
#define AC_DRC_LRMSHAT		0x3018
#define AC_DRC_LRMSLAT		0x301a
#define AC_DRC_RRMSHAT		0x301c
#define AC_DRC_RRMSLAT		0x301e
#define AC_DRC_HCT			0x3020
#define AC_DRC_LCT			0x3022
#define AC_DRC_HKC			0x3024
#define AC_DRC_LKC			0x3026
#define AC_DRC_HOPC			0x3028
#define AC_DRC_LOPC			0x302a
#define AC_DRC_HLT			0x302c
#define AC_DRC_LLT			0x302e
#define AC_DRC_HKI			0x3030
#define AC_DRC_LKI			0x3032
#define AC_DRC_HOPL			0x3034
#define AC_DRC_LOPL			0x3036
#define AC_DRC_HET			0x3038
#define AC_DRC_LET			0x303a
#define AC_DRC_HKE			0x303c
#define AC_DRC_LKE			0x303e
#define AC_DRC_HOPE			0x3040
#define AC_DRC_LOPE			0x3042
#define AC_DRC_HKN			0x3044
#define AC_DRC_LKN			0x3046
#define AC_DRC_SFHAT		0x3048
#define AC_DRC_SFLAT		0x304a
#define AC_DRC_SFHRT		0x304c
#define AC_DRC_SFLRT		0x304e
#define AC_DRC_MXGHS		0x3050
#define AC_DRC_MXGLS		0x3052
#define AC_DRC_MNGHS		0x3054
#define AC_DRC_MNGLS		0x3056
#define AC_DRC_EPSHC		0x3058
#define AC_DRC_EPSLC		0x305a
#define AC_DRC_OPT			0x305c
#define AC_DRC_HPFHGAIN		0x305e
#define AC_DRC_HPFLGAIN		0x3060
#define AC_DRC_BISTCR		0x3100
#define AC_DRC_BISTST		0x3102

/*SYS_CLK_CTL:0x2000*/
#define CTL_I2S				15
#define CTL_HPF_AGC			7
#define CTL_HPF_DRC			6
#define CTL_ADC_DIGITAL		3
#define CTL_DAC_DIGITAL		2

/*SYS_MOD_RST:0x2002*/
#define RST_I2S					15
#define RST_HPF_AGC				7
#define RST_HPF_DRC				6
#define RST_ADC_DIGITAL			3
#define RST_DAC_DIGITAL			2

/*SYS_SAMP_CTL:0x2004*/
#define SYS_FS					0

/*I2S_CTL:0x2100*/
#define SDO0_EN					3
#define TXEN					2
#define RXEN					1
#define GEN						0

/*I2S_CLK:0x2102*/
/*
	00:codec slave
	11:codec master
*/
#define BCLK_OUT				15
#define LRCK_OUT				14
#define BCLKDIV					10
#define LRCK_PERIOD				0

/*I2S_FMT0:0x2104*/
#define MODE_SEL				14
#define TX_OFFSET				10
#define RX_OFFSET				8
#define SR						4
#define SW						1
#define LOOP					0

/*I2S_FMT1:0x2108*/
#define BCLK_POLARITY			15
#define LRCK_POLARITY			14
#define EDGE_TRANSFER			13
#define RX_MLS					11
#define TX_MLS					10
#define SEXT					8
#define LRCK_WIDTH				4
#define RX_PDM					2
#define TX_PDM					0

/*I2S_TXCOF:0x210a*/
#define TX_SLOT_HIZ				9
#define TX_STATE				8
#define TX_SLOT_NUM				0

/*I2S_TXSEL:0x210c*/
#define TX_CHEN					4
#define TX_CHSEL				0

/*I2S_TXMAP:0x210e*/
#define TX_CH3_MAP				12
#define TX_CH2_MAP				8
#define TX_CH1_MAP				4
#define TX_CH0_MAP				0

/*I2S_RXSEL:0x2110*/
#define RX_CHSEL				0

/*I2S_RXMAP:0x2112*/
#define RX_CH3_MAP				12
#define RX_CH2_MAP				8
#define RX_CH1_MAP				4
#define RX_CH0_MAP				0

/*I2S_MIX_SRC:0x2114*/
#define I2S_MIXL_DACDATL		13
#define I2S_MIXL_ADCL			12
#define I2S_MIXR_DACDATR		9
#define I2S_MIXR_ADCR			8

/*I2S_MIX_GAIN:0x2116*/
#define I2S_MIXL_DACDATL_G		13
#define I2S_MIXL_ADCL_G			12
#define I2S_MIXR_DACDATR_G		9
#define I2S_MIXR_ADCR_G			8

/*I2S_DACDAT_DVC:0x2118*/
#define I2S_DACDAT_VOL_L		8
#define I2S_DACDAT_VOL_R		0

/*I2S_ADCDAT_DVC:0x211a*/
#define I2S_ADCDAT_VOL_L		8
#define I2S_ADCDAT_VOL_R		0

/*I2S_ADCDAT_CTL:0x2120*/
#define I2S_ADCL_SRC			4
#define I2S_ADCR_SRC			0

/*I2S_DACDAT_CTL:0x2122*/
#define I2S_DACL_SRC			4
#define I2S_DACR_SRC			0

/*AC_DAC_DPC:0x2200*/
#define ENDA					15
#define ENHPF					14
#define DAFIR32					13
#define MODQU					8

/*AC_DAC_MIX_SRC:0x2202*/
#define I2S_DACDATL				13
#define ADCDATL					12
#define I2S_DACDATR				9
#define ADCDATR					8

/*AC_DAC_MIX_GAIN:0x2204*/
#define DACL_MXR_SRC_G			12
#define DACR_MXR_SRC_G			8

/*AC_DAC_DBG:0x2206*/
#define DASW					15
#define ENDWA_N					14
#define DAC_MOD_DBG				13
#define DAC_PTN_SEL				6
#define DVC						0

/*DACA_OMIXER_CTRL:0x2220*/
#define DACAREN					15
#define DACALEN					14
#define RMIXEN					13
#define LMIXEN					12
#define LINEING					8
#define MIC1G					4
#define MIC2G					0

/*OMIXER_SR:0x2222*/
#define R_MIC1_BOOST			14
#define R_MIC2_BOOST			13
#define R_PHONENP				12
#define R_PHONEP				11
#define R_LINEINR				10
#define R_DACR					9
#define R_DACL					8
#define L_MIC1_BOOST			6
#define L_MIC2_BOOST			5
#define L_PHONENP				4
#define L_PHONEN				3
#define L_LINEINL				2
#define L_DACL					1
#define L_DACR					0

/*LINEOUT_CTRL:0x2224*/
#define LINEOUTEN				15
#define LINEOUT_LEFT_SEL		14
#define LINEOUT_RIGHT_SEL		13
#define L_LINEOUT_SOURCE_SEL	12
#define R_LINEOUT_SOURCE_SEL	11
#define LINEOUT_SLOPE_SELECT	10
#define LINEOUT_SLOPE_LENGTH_CTRL 8
#define ANTI_POP_CTRL			5
#define LINEOUTVOL				0

/*PHOUT_CTRL:0x2230*/
#define PHONEPG					12
#define PHONEG					8
#define PHONEOUTG				5
#define PHONEOUT_EN				4
#define PHONEOUTS3				3
#define PHONEOUTS2				2
#define PHONEOUTS1				1
#define PHONEOUTS0				0

/*AC_ADC_DPC:0x2300*/
#define ENAD					15
#define ENDM					14
#define ADFIR32					13
#define ADOUT_DTS				2
#define ADOUT_DLY				1

/*AC_ADC_DBG:0x2304*/
#define ADSW					15

/*MBIAS_CTRL:0x2310*/
#define MMICBIASEN				15
#define MMIC_BIAS_CHOP_EN		14
#define MMIC_BIAS_CHOP_CLK_SEL	12
#define MBIASSEL				8

/*ADC_MIC_CTRL:0x2320*/
#define ADCREN					15
#define ADCLEN					14
#define ADCG					8
#define MIC1AMPEN				7
#define MIC1BOOST				4
#define MIC2AMPEN				3
#define MIC2BOOST				0

/*ADCMIXER_SR:0x2322*/
#define RADC_MIC1_BOOST			14
#define RADC_MIC2_BOOST			13
#define RADC_PHONEPN			12
#define RADC_PHONEP				11
#define RADC_LINEINR			10
#define RADC_RIGHT_OUTPUT_MIX	9
#define RADC_LEFT_OUTPUT_MIX	8
#define LADC_MIC1_BOOST			6
#define LADC_MIC2_BOOST			5
#define LADC_PHONEPN			4
#define LADC_PHONEN				3
#define LADC_LINEINL			2
#define LADC_LEFT_OUTPUT_MIX	1
#define LADC_RIGHT_OUTPUT_MIX	0

/*ALALOG_TUNING0:0x232a*/
#define ALDOEN					15
#define DITHER					11
#define DITHER_CLK_SELECT		8
#define LINEOUT_SPEED_SELECT	7
#define CURRENT_TEST_SELECT		6

/*AC_AGC_SEL:0x2480*/
#define AGC_SEL					0
/*AC_ADC_DAPLCTRL:0x2500*/
#define LEFT_AGC_EN				14
#define LEFT_HPF_EN				13
#define LEFT_NOISE_DET_EN		12

/*AC_ADC_DAPRCTRL:0x2502*/
#define RIGHT_AGC_EN			14
#define RIGHT_HPF_EN			13
#define RIGHT_NOISE_DET_EN		12

//#define ACX00_DEBG
#ifdef ACX00_DEBG
    #define ACX00_DBG(format,args...)  pr_err("[ACX00] "format,##args)
#else
    #define ACX00_DBG(...)
#endif

#endif
