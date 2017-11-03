/*
 * sound\soc\sunxi\sunxi_tdm_utils.h
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

#ifndef SUNXI_TDM_H_
#define SUNXI_TDM_H_
#include "sunxi_dma.h"
/*------------------------------------------------------------*/
/* REGISTER definition */
#define SUNXI_DAUDIOCTL 	  						(0x00)
	#define SUNXI_DAUDIOCTL_BCLKOUT					(1<<18)
	#define SUNXI_DAUDIOCTL_LRCKOUT					(1<<17)
	#define SUNXI_DAUDIOCTL_LRCKROUT				(1<<16)
	#define SUNXI_DAUDIOCTL_SDO3EN					(1<<11)
	#define SUNXI_DAUDIOCTL_SDO2EN					(1<<10)
	#define SUNXI_DAUDIOCTL_SDO1EN					(1<<9)
	#define SUNXI_DAUDIOCTL_SDO0EN					(1<<8)
	#define SUNXI_DAUDIOCTL_OUTMUTE					(1<<6)
	#define SUNXI_DAUDIOCTL_MODESEL					(3<<4)
	#define SUNXI_DAUDIOCTL_LOOP					(1<<3)
	#define SUNXI_DAUDIOCTL_TXEN					(1<<2)
	#define SUNXI_DAUDIOCTL_RXEN					(1<<1)
	#define SUNXI_DAUDIOCTL_GEN					(1<<0)

#define SUNXI_DAUDIOFAT0 							(0x04)
	#define SUNXI_DAUDIOFAT0_SDI_SYNC_SEL				(1<<31)
	#define SUNXI_DAUDIOFAT0_LRCK_WIDTH				(1<<30)
	#define SUNXI_DAUDIOFAT0_LRCKR_PERIOD(v)			((v)<<20)
	#define SUNXI_DAUDIOFAT0_LRCK_POLAYITY				(1<<19)
	#define SUNXI_DAUDIOFAT0_LRCK_PERIOD(v)				((v)<<8)
	#define SUNXI_DAUDIOFAT0_BCLK_POLAYITY				(1<<7)
	#define SUNXI_DAUDIOFAT0_SR					(7<<4)
	#define SUNXI_DAUDIOFAT0_EDGE_TRANSFER				(1<<3)
	#define SUNXI_DAUDIOFAT0_SW					(7<<0)

#define SUNXI_DAUDIOFAT1							(0x08)
	#define SUNXI_DAUDIOFAT1_RX_MLS					(1<<7)
	#define SUNXI_DAUDIOFAT1_TX_MLS					(1<<6)
	#define SUNXI_DAUDIOFAT1_SEXT					(3<<4)
	#define SUNXI_DAUDIOFAT1_RX_PDM					(3<<2)
	#define SUNXI_DAUDIOFAT1_TX_PDM					(3<<0)

#define SUNXI_DAUDIOISTA 							(0x0c)
	#define SUNXI_DAUDIOSTA_TXU_INT					(1<<6)
	#define SUNXI_DAUDIOSTA_TXO_INT					(1<<5)
	#define SUNXI_DAUDIOSTA_TXE_INT					(1<<4)
	#define SUNXI_DAUDIOSTA_RXU_INT					(1<<2)
	#define SUNXI_DAUDIOSTA_RXO_INT					(1<<1)
	#define SUNXI_DAUDIOSTA_RXA_INT					(1<<0)

#define SUNXI_DAUDIORXFIFO							(0x10)

#define SUNXI_DAUDIOFCTL							(0x14)
	#define SUNXI_DAUDIOFCTL_HUBEN					(1<<31)
	#define SUNXI_DAUDIOFCTL_FTX					(1<<25)
	#define SUNXI_DAUDIOFCTL_FRX					(1<<24)
	#define SUNXI_DAUDIOFCTL_TXTL(v)				((v)<<12)
	#define SUNXI_DAUDIOFCTL_RXTL(v)				((v)<<4)
	#define SUNXI_DAUDIOFCTL_TXIM					(1<<2)
	#define SUNXI_DAUDIOFCTL_RXOM					(1<<0)

#define SUNXI_DAUDIOFSTA   							(0x18)
	#define SUNXI_DAUDIOFSTA_TXE					(1<<28)
	#define SUNXI_DAUDIOFSTA_TXECNT(v)				((v)<<16)
	#define SUNXI_DAUDIOFSTA_RXA					(1<<8)
	#define SUNXI_DAUDIOFSTA_RXACNT(v)				((v)<<0)

#define SUNXI_DAUDIOINT    							(0x1c)
	#define SUNXI_DAUDIOINT_TXDRQEN					(1<<7)
	#define SUNXI_DAUDIOINT_TXUIEN					(1<<6)
	#define SUNXI_DAUDIOINT_TXOIEN					(1<<5)
	#define SUNXI_DAUDIOINT_TXEIEN					(1<<4)
	#define SUNXI_DAUDIOINT_RXDRQEN					(1<<3)
	#define SUNXI_DAUDIOINT_RXUIEN					(1<<2)
	#define SUNXI_DAUDIOINT_RXOIEN					(1<<1)
	#define SUNXI_DAUDIOINT_RXAIEN					(1<<0)

#define SUNXI_DAUDIOTXFIFO							(0x20)

#define SUNXI_DAUDIOCLKD   							(0x24)
	#define SUNXI_DAUDIOCLKD_MCLKOEN				(1<<8)
	#define SUNXI_DAUDIOCLKD_BCLKDIV(v)				((v)<<4)
	#define SUNXI_DAUDIOCLKD_MCLKDIV(v)				((v)<<0)

#define SUNXI_DAUDIOTXCNT  							(0x28)

#define SUNXI_DAUDIORXCNT  							(0x2C)

#define SUNXI_CHCFG								(0x30)
	#define SUNXI_TXCHCFG_TX_SLOT_HIZ				(1<<9)
	#define SUNXI_TXCHCFG_TX_STATE					(1<<8)
#ifdef CONFIG_ARCH_SUN8IW10
	#define SUNXI_TXCHCFG_RX_SLOT_NUM				(15<<4)
	#define SUNXI_TXCHCFG_TX_SLOT_NUM				(15<<0)
#else
	#define SUNXI_TXCHCFG_RX_SLOT_NUM				(7<<4)
	#define SUNXI_TXCHCFG_TX_SLOT_NUM				(7<<0)
#endif

#define SUNXI_DAUDIOTX0CHSEL							(0x34)
#define SUNXI_DAUDIOTX1CHSEL							(0x38)
#define SUNXI_DAUDIOTX2CHSEL							(0x3C)
#define SUNXI_DAUDIOTX3CHSEL							(0x40)
#ifdef CONFIG_ARCH_SUN8IW10
	#define SUNXI_DAUDIOTXn_OFFSET(v)					((v)<<20)
	#define SUNXI_DAUDIOTXn_CHSEL(v)					((v)<<16)
	#define SUNXI_DAUDIOTXn_CHEN(v)						((v)<<0)
	#define CHEN_MASK								0xffff
	#define CHSEL_MASK								0xf
	#define CH_MAX									16
#else
	#define SUNXI_DAUDIOTXn_OFFSET(v)					((v)<<12)
	#define SUNXI_DAUDIOTXn_CHEN(v)					((v)<<4)
	#define SUNXI_DAUDIOTXn_CHSEL(v)					((v)<<0)
	#define CHEN_MASK								0xff
	#define CHSEL_MASK								0x7
	#define CH_MAX									8
#endif
	
#ifdef CONFIG_ARCH_SUN8IW10
#define SUNXI_DAUDIOTX0CHMAP0							(0x44)
#define SUNXI_DAUDIOTX1CHMAP0							(0x4c)
#define SUNXI_DAUDIOTX2CHMAP0							(0x54)
#define SUNXI_DAUDIOTX3CHMAP0							(0x5c)
#define SUNXI_TXCHANMAP0_DEFAULT						(0xfedcba98)
	#define SUNXI_DAUDIOTXn_CH15_MAP				(15<<28)
	#define SUNXI_DAUDIOTXn_CH14_MAP				(15<<24)
	#define SUNXI_DAUDIOTXn_CH13_MAP				(15<<20)
	#define SUNXI_DAUDIOTXn_CH12_MAP				(15<<16)
	#define SUNXI_DAUDIOTXn_CH11_MAP				(15<<12)
	#define SUNXI_DAUDIOTXn_CH10_MAP				(15<<8)
	#define SUNXI_DAUDIOTXn_CH9_MAP					(15<<4)
	#define SUNXI_DAUDIOTXn_CH8_MAP					(15<<0)

#define SUNXI_DAUDIOTX0CHMAP1							(0x48)
#define SUNXI_DAUDIOTX1CHMAP1							(0x50)
#define SUNXI_DAUDIOTX2CHMAP1							(0x58)
#define SUNXI_DAUDIOTX3CHMAP1							(0x60)
#define SUNXI_TXCHANMAP1_DEFAULT						(0x76543210)
	#define SUNXI_DAUDIOTXn_CH7_MAP					(15<<28)
	#define SUNXI_DAUDIOTXn_CH6_MAP					(15<<24)
	#define SUNXI_DAUDIOTXn_CH5_MAP					(15<<20)
	#define SUNXI_DAUDIOTXn_CH4_MAP					(15<<16)
	#define SUNXI_DAUDIOTXn_CH3_MAP					(15<<12)
	#define SUNXI_DAUDIOTXn_CH2_MAP					(15<<8)
	#define SUNXI_DAUDIOTXn_CH1_MAP					(15<<4)
	#define SUNXI_DAUDIOTXn_CH0_MAP					(15<<0)

	#define SUNXI_DAUDIORXCHSEL							(0x64)
	#define SUNXI_DAUDIORXCHSEL_RXOFFSET(v)			((v)<<20)
	#define SUNXI_DAUDIORXCHSEL_RXCHSET(v)				((v)<<16)

	#define SUNXI_DAUDIORXCHMAP0						(0x68)
	#define SUNXI_RXCHANMAP0_DEFAULT				(0xfedcba98)
	#define SUNXI_DAUDIORXCHMAP_CH15				(15<<28)
	#define SUNXI_DAUDIORXCHMAP_CH14				(15<<24)
	#define SUNXI_DAUDIORXCHMAP_CH13				(15<<20)
	#define SUNXI_DAUDIORXCHMAP_CH12				(15<<16)
	#define SUNXI_DAUDIORXCHMAP_CH11				(15<<12)
	#define SUNXI_DAUDIORXCHMAP_CH10				(15<<8)
	#define SUNXI_DAUDIORXCHMAP_CH9					(15<<4)
	#define SUNXI_DAUDIORXCHMAP_CH8					(15<<0)

	#define SUNXI_DAUDIORXCHMAP1					(0x6c)
	#define SUNXI_RXCHANMAP1_DEFAULT				(0x76543210)
	#define SUNXI_DAUDIORXCHMAP_CH7					(15<<28)
	#define SUNXI_DAUDIORXCHMAP_CH6					(15<<24)
	#define SUNXI_DAUDIORXCHMAP_CH5					(15<<20)
	#define SUNXI_DAUDIORXCHMAP_CH4					(15<<16)
	#define SUNXI_DAUDIORXCHMAP_CH3					(15<<12)
	#define SUNXI_DAUDIORXCHMAP_CH2					(15<<8)
	#define SUNXI_DAUDIORXCHMAP_CH1					(15<<4)
	#define SUNXI_DAUDIORXCHMAP_CH0					(15<<0)

#define SUNXI_DAUDIODBG								(0x70)
#else
#define SUNXI_DAUDIOTX0CHMAP							(0x44)
#define SUNXI_DAUDIOTX1CHMAP							(0x48)
#define SUNXI_DAUDIOTX2CHMAP							(0x4C)
#define SUNXI_DAUDIOTX3CHMAP							(0x50)
#define SUNXI_TXCHANMAP_DEFAULT							(0x76543210)
	#define SUNXI_DAUDIOTXn_CH7_MAP					(7<<28)
	#define SUNXI_DAUDIOTXn_CH6_MAP					(7<<24)
	#define SUNXI_DAUDIOTXn_CH5_MAP					(7<<20)
	#define SUNXI_DAUDIOTXn_CH4_MAP					(7<<16)
	#define SUNXI_DAUDIOTXn_CH3_MAP					(7<<12)
	#define SUNXI_DAUDIOTXn_CH2_MAP					(7<<8)
	#define SUNXI_DAUDIOTXn_CH1_MAP					(7<<4)
	#define SUNXI_DAUDIOTXn_CH0_MAP					(7<<0)

#define SUNXI_DAUDIORXCHSEL							(0x54)
	#define SUNXI_DAUDIORXCHSEL_RXOFFSET(v)				((v)<<12)
	#define SUNXI_DAUDIORXCHSEL_RXCHSET(v)				((v)<<0)

#define SUNXI_DAUDIORXCHMAP							(0x58)
#define SUNXI_RXCHANMAP_DEFAULT							(0x76543210)
	#define SUNXI_DAUDIORXCHMAP_CH7					(7<<28)
	#define SUNXI_DAUDIORXCHMAP_CH6					(7<<24)
	#define SUNXI_DAUDIORXCHMAP_CH5					(7<<20)
	#define SUNXI_DAUDIORXCHMAP_CH4					(7<<16)
	#define SUNXI_DAUDIORXCHMAP_CH3					(7<<12)
	#define SUNXI_DAUDIORXCHMAP_CH2					(7<<8)
	#define SUNXI_DAUDIORXCHMAP_CH1					(7<<4)
	#define SUNXI_DAUDIORXCHMAP_CH0					(7<<0)

#define SUNXI_DAUDIODBG								(0x5C)
#endif

#define SUNXI_DAUDIOCLKD_MCLK_MASK   0x0f
#define SUNXI_DAUDIOCLKD_MCLK_OFFS   0
#define SUNXI_DAUDIOCLKD_BCLK_MASK   0x070
#define SUNXI_DAUDIOCLKD_BCLK_OFFS   4
#define SUNXI_DAUDIOCLKD_MCLKEN_OFFS 7

#define SUNXI_DAUDIO_DIV_MCLK	(0)
#define SUNXI_DAUDIO_DIV_BCLK	(1)

struct sunxi_tdm_info {
	void __iomem   *regs;
	/*pll clk*/
	struct clk *tdm_pllclk;
	struct clk *tdm_moduleclk;
	struct snd_soc_dai_driver dai;
	struct sunxi_dma_params play_dma_param;
	struct sunxi_dma_params capture_dma_param;

	struct pinctrl *pinctrl;
	struct pinctrl_state  *pinstate;
	struct pinctrl_state  *pinstate_sleep;

	/*tdm arg*/
	u32 pcm_lrck_period;		/*pcm_lrck_period     = 32*/
	u32 pcm_lrckr_period;		/*pcm_lrckr_period    = 1*/
	u32 slot_width_select;			/*slot_width_select   = 16*/

	u32 pcm_lsb_first;		/*msb_lsb_first*/
	u32 tx_data_mode;
	u32 rx_data_mode;
	u32 daudio_master;
	u32 audio_format;
	u32 signal_inversion;
	u32 samp_res;
	u32 others;	/*other use*/
	u32 tdm_num;
	u32 clk_enable_cnt;
	bool tdm_config;	/*1:i2s 0:pcm*/
	bool frametype;	/*pcm format: 0-short frame,1-long frame*/
};

extern int txctrl_tdm(int on,int hub_en,struct sunxi_tdm_info *sunxi_tdm);
extern int  rxctrl_tdm(int on,struct sunxi_tdm_info *sunxi_tdm);
extern int tdm_set_fmt(unsigned int fmt,struct sunxi_tdm_info *sunxi_tdm);
extern int tdm_hw_params(struct snd_pcm_hw_params *params,
					struct sunxi_tdm_info *sunxi_tdm);
extern int tdm_trigger(struct snd_pcm_substream *substream,
                              int cmd, struct sunxi_tdm_info *sunxi_tdm);
extern int tdm_set_sysclk(unsigned int freq,struct sunxi_tdm_info *sunxi_tdm);
extern int tdm_set_clkdiv(int sample_rate,struct sunxi_tdm_info *sunxi_tdm);
extern int tdm_perpare(struct snd_pcm_substream *substream,struct sunxi_tdm_info *sunxi_tdm);
extern int tdm_global_enable(struct sunxi_tdm_info *sunxi_tdm,bool on);
#endif

