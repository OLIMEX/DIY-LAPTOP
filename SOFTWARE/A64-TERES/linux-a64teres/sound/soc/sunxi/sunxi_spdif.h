/*
 * sound\soc\sunxi\spdif\sunxi_spdif.h
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

#ifndef SUNXI_SPDIF_H_
#define SUNXI_SPDIF_H_
#include "sunxi_dma.h"

/*------------------SPDIF register definition--------------------*/

#define	SUNXI_SPDIF_CTL	(0x00)
#ifdef CONFIG_ARCH_SUN8IW1
	#define SUNXI_SPDIF_CTL_MCLKOUTEN				(1<<2)
#endif
#if defined (CONFIG_ARCH_SUN8IW1) || defined(CONFIG_ARCH_SUN8IW7) || defined(CONFIG_ARCH_SUN50I)\
	|| defined(CONFIG_ARCH_SUN8IW11)
	#define SUNXI_SPDIF_CTL_MCLKDIV(v)				((v)<<4)		//v even
#endif
#ifdef CONFIG_ARCH_SUN8IW10
	#define SUNXI_SPDIF_CTL_LOOP					(1<<2)
#endif
	#define SUNXI_SPDIF_CTL_GEN					(1<<1)
	#define SUNXI_SPDIF_CTL_RESET					(1<<0)

#define SUNXI_SPDIF_TXCFG (0x04)
	#define SUNXI_SPDIF_TXCFG_SINGLEMOD				(1<<31)
	#define SUNXI_SPDIF_TXCFG_ASS					(1<<17)
	#define SUNXI_SPDIF_TXCFG_NONAUDIO				(1<<16)
	#define SUNXI_SPDIF_TXCFG_TXRATIO(v)				((v)<<4)
	#define SUNXI_SPDIF_TXCFG_FMTRVD				(3<<2)
	#define SUNXI_SPDIF_TXCFG_FMT16BIT				(0<<2)
	#define SUNXI_SPDIF_TXCFG_FMT20BIT				(1<<2)
	#define SUNXI_SPDIF_TXCFG_FMT24BIT				(2<<2)
	#define SUNXI_SPDIF_TXCFG_CHSTMODE				(1<<1)
	#define SUNXI_SPDIF_TXCFG_TXEN					(1<<0)

#define SUNXI_SPDIF_RXCFG (0x08)
	#define SUNXI_SPDIF_RXCFG_LOCKFLAG				(1<<4)
	#define SUNXI_SPDIF_RXCFG_CHSTSRC				(1<<3)
	#define SUNXI_SPDIF_RXCFG_CHSTCP				(1<<1)
	#define SUNXI_SPDIF_RXCFG_RXEN					(1<<0)
#if defined(CONFIG_ARCH_SUN9IW1) \
|| defined(CONFIG_ARCH_SUN8IW6) || defined(CONFIG_ARCH_SUN8IW7) || defined(CONFIG_ARCH_SUN50I) \
|| defined(CONFIG_ARCH_SUN8IW10) || defined(CONFIG_ARCH_SUN8IW11)
#define SUNXI_SPDIF_TXFIFO (0x20)
#else
#define SUNXI_SPDIF_TXFIFO (0x0C)
#endif
#define SUNXI_SPDIF_RXFIFO (0x10)

#define SUNXI_SPDIF_FCTL (0x14)

#if defined(CONFIG_ARCH_SUN9IW1) \
	|| defined(CONFIG_ARCH_SUN8IW6) || defined(CONFIG_ARCH_SUN8IW7)|| defined(CONFIG_ARCH_SUN50I) \
	|| defined(CONFIG_ARCH_SUN8IW10) || defined(CONFIG_ARCH_SUN8IW11)
	#define SUNXI_SPDIFFCTL_HUBEN					(1<<31)
#else
	#define SUNXI_SPDIF_FCTL_FIFOSRC				(1<<31)
#endif
#ifdef CONFIG_ARCH_SUN8IW10
	#define SUNXI_SPDIF_FCTL_FTX					(1<<30)
	#define SUNXI_SPDIF_FCTL_FRX					(1<<29)
	#define SUNXI_SPDIF_FCTL_TXTL(v)				((v)<<12)
	#define SUNXI_SPDIF_FCTL_RXTL(v)				(((v))<<4)
#else
	#define SUNXI_SPDIF_FCTL_FTX					(1<<17)
	#define SUNXI_SPDIF_FCTL_FRX					(1<<16)
	#define SUNXI_SPDIF_FCTL_TXTL(v)				((v)<<8)
	#define SUNXI_SPDIF_FCTL_RXTL(v)				(((v))<<3)
#endif
	#define SUNXI_SPDIF_FCTL_TXIM(v)				((v)<<2)
	#define SUNXI_SPDIF_FCTL_RXOM(v)				((v)<<0)

#define SUNXI_SPDIF_FSTA (0x18)
#ifdef CONFIG_ARCH_SUN8IW10
	#define SUNXI_SPDIF_FSTA_TXE					(1<<31)
	#define SUNXI_SPDIF_FSTA_TXECNTSHT				(16)
	#define SUNXI_SPDIF_FSTA_RXA					(1<<15)
#else
	#define SUNXI_SPDIF_FSTA_TXE					(1<<14)
	#define SUNXI_SPDIF_FSTA_TXECNTSHT				(8)
	#define SUNXI_SPDIF_FSTA_RXA					(1<<6)
#endif
	#define SUNXI_SPDIF_FSTA_RXACNTSHT				(0)

#define SUNXI_SPDIF_INT (0x1C)
	#define SUNXI_SPDIF_INT_RXLOCKEN				(1<<18)
	#define SUNXI_SPDIF_INT_RXUNLOCKEN				(1<<17)
	#define SUNXI_SPDIF_INT_RXPARERREN				(1<<16)
	#define SUNXI_SPDIF_INT_TXDRQEN					(1<<7)
	#define SUNXI_SPDIF_INT_TXUIEN					(1<<6)
	#define SUNXI_SPDIF_INT_TXOIEN					(1<<5)
	#define SUNXI_SPDIF_INT_TXEIEN					(1<<4)
	#define SUNXI_SPDIF_INT_RXDRQEN					(1<<2)
	#define SUNXI_SPDIF_INT_RXOIEN					(1<<1)
	#define SUNXI_SPDIF_INT_RXAIEN					(1<<0)
#if defined(CONFIG_ARCH_SUN9IW1) \
	|| defined(CONFIG_ARCH_SUN8IW6)|| defined(CONFIG_ARCH_SUN8IW7) || defined(CONFIG_ARCH_SUN50I)\
	|| defined(CONFIG_ARCH_SUN8IW10) || defined(CONFIG_ARCH_SUN8IW11)
	#define SUNXI_SPDIF_ISTA (0x0c)
#else
	#define SUNXI_SPDIF_ISTA (0x20)
#endif
	#define SUNXI_SPDIF_ISTA_RXLOCKSTA				(1<<18)
	#define SUNXI_SPDIF_ISTA_RXUNLOCKSTA			(1<<17)
	#define SUNXI_SPDIF_ISTA_RXPARERRSTA			(1<<16)
	#define SUNXI_SPDIF_ISTA_TXUSTA					(1<<6)
	#define SUNXI_SPDIF_ISTA_TXOSTA					(1<<5)
	#define SUNXI_SPDIF_ISTA_TXESTA					(1<<4)
	#define SUNXI_SPDIF_ISTA_RXOSTA					(1<<1)
	#define SUNXI_SPDIF_ISTA_RXASTA					(1<<0)

#define SUNXI_SPDIF_TXCNT	(0x24)

#define SUNXI_SPDIF_RXCNT	(0x28)

#define SUNXI_SPDIF_TXCHSTA0 (0x2C)
	#define SUNXI_SPDIF_TXCHSTA0_CLK(v)				((v)<<28)
	#define SUNXI_SPDIF_TXCHSTA0_SAMFREQ(v)			((v)<<24)
	#define SUNXI_SPDIF_TXCHSTA0_CHNUM(v)			((v)<<20)
	#define SUNXI_SPDIF_TXCHSTA0_SRCNUM(v)			((v)<<16)
	#define SUNXI_SPDIF_TXCHSTA0_CATACOD(v)			((v)<<8)
	#define SUNXI_SPDIF_TXCHSTA0_MODE(v)			((v)<<6)
	#define SUNXI_SPDIF_TXCHSTA0_EMPHASIS(v)	  	((v)<<3)
	#define SUNXI_SPDIF_TXCHSTA0_CP					(1<<2)
	#define SUNXI_SPDIF_TXCHSTA0_AUDIO				(1<<1)
	#define SUNXI_SPDIF_TXCHSTA0_PRO				(1<<0)

#define SUNXI_SPDIF_TXCHSTA1 (0x30)
	#define SUNXI_SPDIF_TXCHSTA1_CGMSA(v)			((v)<<8)
	#define SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(v)		((v)<<4)
	#define SUNXI_SPDIF_TXCHSTA1_SAMWORDLEN(v)		((v)<<1)
	#define SUNXI_SPDIF_TXCHSTA1_MAXWORDLEN			(1<<0)

#define SUNXI_SPDIF_RXCHSTA0 (0x34)
	#define SUNXI_SPDIF_RXCHSTA0_CLK(v)				((v)<<28)
	#define SUNXI_SPDIF_RXCHSTA0_SAMFREQ(v)			((v)<<24)
	#define SUNXI_SPDIF_RXCHSTA0_CHNUM(v)			((v)<<20)
	#define SUNXI_SPDIF_RXCHSTA0_SRCNUM(v)			((v)<<16)
	#define SUNXI_SPDIF_RXCHSTA0_CATACOD(v)			((v)<<8)
	#define SUNXI_SPDIF_RXCHSTA0_MODE(v)			((v)<<6)
	#define SUNXI_SPDIF_RXCHSTA0_EMPHASIS(v)	  	((v)<<3)
	#define SUNXI_SPDIF_RXCHSTA0_CP					(1<<2)
	#define SUNXI_SPDIF_RXCHSTA0_AUDIO				(1<<1)
	#define SUNXI_SPDIF_RXCHSTA0_PRO				(1<<0)

#define SUNXI_SPDIF_RXCHSTA1 (0x38)
	#define SUNXI_SPDIF_RXCHSTA1_CGMSA(v)			((v)<<8)
	#define SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(v)		((v)<<4)
	#define SUNXI_SPDIF_RXCHSTA1_SAMWORDLEN(v)		((v)<<1)
	#define SUNXI_SPDIF_RXCHSTA1_MAXWORDLEN			(1<<0)


/* Clock dividers */
#define SUNXI_DIV_MCLK	0
#define SUNXI_DIV_BCLK	1

struct sunxi_spdif_info {
	void __iomem   *regs;    /* spdif base */
	struct clk *pllclk;
	struct clk *moduleclk;
	struct snd_soc_dai_driver dai;
	struct sunxi_dma_params play_dma_param;
	struct sunxi_dma_params capture_dma_param;
	struct pinctrl *pinctrl;
	struct pinctrl_state  *pinstate;
	struct pinctrl_state  *pinstate_sleep;
	u32 clk_enable_cnt;
};

//extern struct sunxi_spdif_info sunxi_spdif;
extern s32 get_clock_divder(u32 sample_rate, u32 sample_width, u32 * mclk_div, u32* mpll, u32* bclk_div, u32* mult_fs);

extern int spdif_set_fmt(unsigned int fmt,struct sunxi_spdif_info *sunxi_spdif);
extern int spdif_set_clkdiv(int div_id, int div,struct sunxi_spdif_info *sunxi_spdif);
extern int spdif_set_params(int format,struct sunxi_spdif_info *sunxi_spdif);
extern void spdif_txctrl_enable(int tx_en, int chan, int hub_en,struct sunxi_spdif_info *sunxi_spdif);
#endif

