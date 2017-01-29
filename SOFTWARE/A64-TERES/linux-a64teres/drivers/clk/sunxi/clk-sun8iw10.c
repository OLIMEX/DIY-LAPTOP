/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/clk-private.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk/sunxi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include "clk-sunxi.h"
#include "clk-factors.h"
#include "clk-periph.h"
#include "clk-sun8iw10.h"
#include "clk-sun8iw10_tbl.c"

#ifndef CONFIG_EVB_PLATFORM
	#define LOCKBIT(x) 31
#else
	#define LOCKBIT(x) x
#endif
static DEFINE_SPINLOCK(clk_lock);
void __iomem *sunxi_clk_base;
void __iomem *sunxi_clk_cpus_base=0;
int    sunxi_clk_maxreg =SUNXI_CLK_MAX_REG;
int cpus_clk_maxreg = 0;
#ifdef CONFIG_SUNXI_CLK_DUMMY_DEBUG
unsigned int dummy_reg[1024];
unsigned int dummy_readl(unsigned int* regaddr)
{
	unsigned int val;
	val = *regaddr;

	printk("--%s-- dummy_readl to read reg 0x%x with val 0x%x\n",__func__,((unsigned int)regaddr - (unsigned int)&dummy_reg[0]),val);
	return val;
}
void  dummy_writel(unsigned int val,unsigned int* regaddr)
{
	*regaddr = val;
	printk("--%s-- dummy_writel to write reg 0x%x with val 0x%x\n",__func__,((unsigned int)regaddr - (unsigned int)&dummy_reg[0]),val);
}

void dummy_reg_init(void)
{
	memset(dummy_reg,0x0,sizeof(dummy_reg));
	dummy_reg[PLL_CPU/4]=0x00001000;
	dummy_reg[PLL_AUDIO/4]=0x00035514;
	dummy_reg[PLL_VIDEO0/4]=0x03006207;
	dummy_reg[PLL_DDR0/4]=0x00001800;
	dummy_reg[PLL_PERIPH0/4]=0x00041811;
	dummy_reg[PLL_VIDEO1/4]=0x03006207;
	dummy_reg[PLL_24M/4]=0xa00c1801;
	dummy_reg[PLL_PERIPH1/4]=0x00041811;
	dummy_reg[PLL_DE/4]=0x03006207;
	dummy_reg[PLL_DDR1/4]=0x00001800;

	dummy_reg[CPU_CFG/4]=0x00010300;
	dummy_reg[AHB1_CFG/4]=0x00001010;
	dummy_reg[APB2_CFG/4]=0x01000000;
	dummy_reg[PLL_LOCK/4]=0x000000FF;
	dummy_reg[CPU_LOCK/4]=0x000000FF;
}
#endif // of CONFIG_SUNXI_CLK_DUMMY_DEBUG

/*                                          ns  nw  ks  kw  ms  mw  ps  pw  d1s d1w d2s d2w {frac   out mode}   en-s    sdmss   sdmsw   sdmpat         sdmval*/
SUNXI_CLK_FACTORS(          pll_cpu,        8,  5,  4,  2,  0,  2,  16, 2,  0,  0,  0,  0,    0,    0,  0,      31,     24,     0,      PLL_CPUPAT,    0xd1303333);
SUNXI_CLK_FACTORS(          pll_audio,      8,  7,  0,  0,  0,  5,  16, 4,  0,  0,  0,  0,    0,    0,  0,      31,     24,      0,     PLL_AUDIOPAT,  0xd1303333);
SUNXI_CLK_FACTORS(          pll_video0,     8,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,    1,    25, 24,     31,     20,     0,      PLL_VIDEO0PAT, 0xd1303333);
SUNXI_CLK_FACTORS_UPDATE(   pll_ddr0,  8,  7,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,    0,    0,  15,      31,     0,     0,      PLL_DDR0PAT0,  0xf1303333 , 30);
SUNXI_CLK_FACTORS(          pll_periph0,    8,  5,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,    0,    0,  0,      31,     0,      0,      0,             0);
SUNXI_CLK_FACTORS(          pll_periph1,    8,  5,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,    0,    0,  0,      31,     20,     0,      PLL_PERI1PAT,  0xd1303333);
SUNXI_CLK_FACTORS(          pll_video1,     8,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,    1,    25, 24,     31,     20,     0,      PLL_VEDEO1PAT, 0xd1303333);
SUNXI_CLK_FACTORS(          pll_24m,        8,  7, 16,  5,  0,  2,  4,  4,  0,  0,  0,  0,    0,    0 , 0 ,     31,      0,     0,      0, 0);
SUNXI_CLK_FACTORS(          pll_de,         8,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,    1,    25, 24,     31,     20,     0,      PLL_DEPAT   ,  0xd1303333);
SUNXI_CLK_FACTORS_UPDATE(   pll_ddr1,  8,  7,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,    0,     0, 15,     31,      0,     0,      PLL_DDR1PAT0,   0xf1303333 , 30);

static int get_factors_pll_cpu(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{

	int index;
	u64 tmp_rate;
	if(!factor)
		return -1;
	tmp_rate = rate>pllcpu_max ? pllcpu_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_cpu,factor, factor_pllcpu_tbl,index,sizeof(factor_pllcpu_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}

static int get_factors_pll_audio(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	if(rate == 22579200) {
		factor->factorn = 78;
		factor->factorm = 20;
		factor->factorp = 3;
	} else if(rate == 24576000) {
		factor->factorn = 85;
		factor->factorm = 20;
		factor->factorp = 3;
	} else
		return -1;

	return 0;
}

static int get_factors_pll_video0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate;
	int index;

	if(!factor)
		return -1;

	tmp_rate = rate>pllvideo0_max ? pllvideo0_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_video0,factor, factor_pllvideo0_tbl,index,sizeof(factor_pllvideo0_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	if(rate == 297000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 1;
		factor->factorm = 0;
	}
	else if(rate == 270000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 0;
		factor->factorm = 0;
	} else {
		factor->frac_mode = 1;
		factor->frac_freq = 0;
	}

	return 0;
}

static int get_factors_pll_ddr0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	u64 tmp_rate;

	if (!factor)
		return -1;

	tmp_rate = rate > pllddr0_max ? pllddr0_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if (sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_ddr0, factor,
					 factor_pllddr0_tbl, index,
					 sizeof(factor_pllddr0_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}

static int get_factors_pll_periph0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	u64 tmp_rate;

	if(!factor)
		return -1;

	tmp_rate = rate>pllperiph0_max ? pllperiph0_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;

	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_periph0,factor, factor_pllperiph0_tbl,index,sizeof(factor_pllperiph0_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;
	return 0;
}

static int get_factors_pll_periph1(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	u64 tmp_rate;

	if(!factor)
		return -1;
	tmp_rate = rate>pllperiph1_max ? pllperiph1_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;

	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_periph1,factor, factor_pllperiph1_tbl,index,sizeof(factor_pllperiph1_tbl)/sizeof(struct sunxi_clk_factor_freq)))
			return -1;
	return 0;
}

static int get_factors_pll_video1(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate;
	int index;

	if(!factor)
		return -1;

	tmp_rate = rate>pllvideo1_max ? pllvideo1_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_video1,factor, factor_pllvideo1_tbl,index,sizeof(factor_pllvideo1_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;
	if(rate == 297000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 1;
		factor->factorm = 0;
	}
	else if(rate == 270000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 0;
		factor->factorm = 0;
	} else {
		factor->frac_mode = 1;
		factor->frac_freq = 0;
	}

	return 0;
}

static int get_factors_pll_24m(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate;
	int index;

	if(!factor)
		return -1;

	tmp_rate = rate>pll24m_max ? pll24m_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;

	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_de,factor, factor_pllde_tbl,index,sizeof(factor_pllde_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}


static int get_factors_pll_de(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate;
	int index;

	if(!factor)
		return -1;

	tmp_rate = rate>pllde_max ? pllde_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;

	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_de,factor, factor_pllde_tbl,index,sizeof(factor_pllde_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;
	if(rate == 297000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 1;
		factor->factorm = 0;
	}
	else if(rate == 270000000) {
		factor->frac_mode = 0;
		factor->frac_freq = 0;
		factor->factorm = 0;
	} else {
		factor->frac_mode = 1;
		factor->frac_freq = 0;
	}

	return 0;
}

static int get_factors_pll_ddr1(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	u64 tmp_rate;

	if (!factor)
		return -1;
	tmp_rate = rate > pllddr1_max ? pllddr1_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if (sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_ddr1, factor,
					 factor_pllddr1_tbl, index,
					 sizeof(factor_pllddr1_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}

/*    pll_cpux: 24*N*K/(M*P)    */
static unsigned long calc_rate_pll_cpu(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	tmp_rate = tmp_rate * (factor->factorn+1) * (factor->factork+1);
	do_div(tmp_rate, (factor->factorm+1) * (1 << factor->factorp));
	return (unsigned long)tmp_rate;
}
/*    pll_audio:24*N/(M*P)    */
static unsigned long calc_rate_pll_audio(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	if((factor->factorn == 78) && (factor->factorm == 20) && (factor->factorp == 3))
		return 22579200;
	else if((factor->factorn == 85) && (factor->factorm == 20) && (factor->factorp == 3))
		return 24576000;
	else
	{
		tmp_rate = tmp_rate * (factor->factorn+1);
		do_div(tmp_rate, (factor->factorm+1) * (factor->factorp+1));
		return (unsigned long)tmp_rate;
	}
}
/*    pll_video0:24*N/M    */
static unsigned long calc_rate_media(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	if(factor->frac_mode == 0)
	{
		if(factor->frac_freq == 1)
			return 297000000;
		else
			return 270000000;
	}
	else
	{
		tmp_rate = tmp_rate * (factor->factorn+1);
		do_div(tmp_rate, factor->factorm+1);
		return (unsigned long)tmp_rate;
	}
}
/*    pll_ddr:24*N/M    */
static unsigned long calc_rate_pll_ddr(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:24000000);
	tmp_rate = tmp_rate * (factor->factorn+1);
	do_div(tmp_rate, factor->factorm+1);
	return (unsigned long)tmp_rate;
}

/*    pll_periph0:24*N*K/2    */
static unsigned long calc_rate_pll_periph(u32 parent_rate, struct clk_factors_value *factor)
{
	return (unsigned long)(parent_rate?(parent_rate/2):12000000) * (factor->factorn+1) * (factor->factork+1);
}

/*    pll_ddr:24*N/K/M/P    */
static unsigned long calc_rate_pll_24m(u32 parent_rate, struct clk_factors_value *factor)
{
	u64 tmp_rate = (parent_rate?parent_rate:26000000);
	tmp_rate = tmp_rate * (factor->factorn);
	do_div(tmp_rate, factor->factorp+1);
	do_div(tmp_rate, factor->factorm+1);
	do_div(tmp_rate, factor->factork+1);
	return (unsigned long)tmp_rate;
}

static const char *hosc_parents[] = {"hosc"};
static const char *pll_24m_parents[] = {"pll_24m"};

struct factor_init_data sunxi_factos[] = {
	/* name             parent          parent_num,     flags                                               reg             lock_reg        lock_bit       pll_lock_ctrl_reg    lock_en_bit     lock_mode                config                                get_factors                     calc_rate                   priv_ops*/
	{"pll_cpu",     pll_24m_parents, 1,          CLK_IGNORE_DISABLE | CLK_GET_RATE_NOCACHE, PLL_CPU,     PLL_CPU,     LOCKBIT(28), PLL_CLK_CTRL,     0,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_cpu,     &get_factors_pll_cpu,     &calc_rate_pll_cpu,    (struct clk_ops*)NULL},
	{"pll_audio",   pll_24m_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_AUDIO,   PLL_AUDIO,   LOCKBIT(28), PLL_CLK_CTRL,     1,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_audio,   &get_factors_pll_audio,   &calc_rate_pll_audio,  (struct clk_ops*)NULL},
	{"pll_video0",  pll_24m_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_VIDEO0,  PLL_VIDEO0,  LOCKBIT(28), PLL_CLK_CTRL,     2,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_video0,  &get_factors_pll_video0,  &calc_rate_media,      (struct clk_ops*)NULL},
	{"pll_ddr0",    pll_24m_parents, 1,          CLK_IGNORE_DISABLE | CLK_GET_RATE_NOCACHE, PLL_DDR0,    PLL_DDR0,    LOCKBIT(28), PLL_CLK_CTRL,     3,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_ddr0,    &get_factors_pll_ddr0,    &calc_rate_pll_ddr,   (struct clk_ops*)NULL},
	{"pll_periph0", pll_24m_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_PERIPH0, PLL_PERIPH0, LOCKBIT(28), PLL_CLK_CTRL,     4,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_periph0, &get_factors_pll_periph0, &calc_rate_pll_periph, (struct clk_ops*)NULL},
	{"pll_periph1", pll_24m_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_PERIPH1, PLL_PERIPH1, LOCKBIT(28), PLL_CLK_CTRL,     7,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_periph1, &get_factors_pll_periph1, &calc_rate_pll_periph, (struct clk_ops*)NULL},
	{"pll_video1",  pll_24m_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_VIDEO1,  PLL_VIDEO1,  LOCKBIT(28), PLL_CLK_CTRL,     5,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_video1,  &get_factors_pll_video1,  &calc_rate_media,      (struct clk_ops*)NULL},
	{"pll_24m",     hosc_parents, 1,          CLK_IGNORE_DISABLE | CLK_GET_RATE_NOCACHE, PLL_24M,     PLL_24M,     LOCKBIT(28), PLL_CLK_CTRL,     6,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_24m,     &get_factors_pll_24m,     &calc_rate_pll_24m,    (struct clk_ops*)NULL},
	{"pll_de",      pll_24m_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_DE,      PLL_DE,      LOCKBIT(28), PLL_CLK_CTRL,     8,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_de,      &get_factors_pll_de,      &calc_rate_media,      (struct clk_ops*)NULL},
	{"pll_ddr1",    pll_24m_parents, 1,          CLK_IGNORE_DISABLE | CLK_GET_RATE_NOCACHE, PLL_DDR1,    PLL_DDR1,    LOCKBIT(28), PLL_CLK_CTRL,     9,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_ddr1,    &get_factors_pll_ddr1,    &calc_rate_pll_ddr,   (struct clk_ops*)NULL},
};

struct periph_init_data {
	const char              *name;
	unsigned long           flags;
	const char              **parent_names;
	int                     num_parents;
	struct sunxi_clk_periph *periph;
};

static const char *cpu_parents[] = {"losc", "hosc", "pll_cpu", "pll_cpu"};
static const char *cpuapb_parents[] = {"cpu"};
static const char *axi_parents[] = {"cpu"};
static const char *pll_periphahb0_parents[] = {"pll_periph0"};
static const char *ahb1_parents[] = {"losc", "hosc", "axi", "pll_periphahb0"};
static const char *apb1_parents[] = {"ahb1"};
static const char *apb2_parents[] = {"losc", "hosc", "pll_24m", "pll_periph0x2"};
static const char *ths_parents[] = {"hosc",};
static const char *periphx2_parents[] = {"hosc", "pll_periph0x2","pll_periph1x2",""};
static const char *i2s_parents[] = {"pll_audiox8", "pll_audiox4", "pll_audiox2", "pll_audio", "ext_clk0", "ext_clk1", "", ""};
static const char *mbus_parents[] = {"hosc", "pll_periph0x2", "pll_ddr0", "pll_ddr1"};
static const char *de_parents[] = {"pll_periph0x2", "pll_de", "", "", "", "","",""};
static const char *tcon0_parents[] = {"pll_video0", "pll_video1", "", "", "", "", "", ""};
static const char *periphx_parents[] = {"pll_periph0","pll_video1","","","","","",""};
static const char *csi_m_parents[] = {"pll_24m", "pll_video1", "pll_periph0", "pll_periph1", "", "","",""};
static const char *csi_misc_parents[] = {"pll_24m", "hosc","",""};
static const char *adda_parents[] = {"pll_audio", "ext_clk0", "ext_clk1", ""};
static const char *wlan_parents[] = {"hosc", "ext_hosc"};
static const char *ahb1mod_parents[] = {"ahb1"};
static const char *apb1mod_parents[] = {"apb1"};
static const char *apb2mod_parents[] = {"apb2"};
static const char *sdram_parents[] = {"pll_ddr0", "pll_ddr1", "pll_periph0x2",""};
static const char *usbohci_parents[] = {"usbohci_16"};
static const char *losc_parents[] = {"losc"};

struct sunxi_clk_comgate com_gates[]={
{"csi",       0,  0x7,    BUS_GATE_SHARE|RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
};

/*
SUNXI_CLK_PERIPH(name,           mux_reg,    mux_sft, mux_wid,  div_reg,            div_msft,  div_mwid, div_nsft, div_nwid, gate_flag,       en_reg,         rst_reg,      bus_gate_reg,  drm_gate_reg,  en_sft,    rst_sft, bus_gate_sft, dram_gate_sft, lock,  com_gate,   com_gate_off)
*/
SUNXI_CLK_PERIPH(cpu,            CPU_CFG,         16,   2,          0,                  0,      0,          0,          0,          0,          0,               0,               0,             0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(cpuapb,         0,               0,    0,          CPU_CFG,            8,      2,          0,          0,          0,          0,               0,               0,             0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(axi,            0,               0,    0,          CPU_CFG,            0,      2,          0,          0,          0,          0,               0,               0,             0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(pll_periphahb0, 0,               0,    0,          AHB1_CFG,           6,      2,          0,          0,          0,          0,               0,               0,             0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(ahb1,           AHB1_CFG,        12,   2,          AHB1_CFG,           0,      0,          4,          2,          0,          0,               0,               0,             0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(apb1,           0,               0,    0,          AHB1_CFG,           0,      0,          8,          2,          0,          0,               0,               0,             0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(apb2,           APB2_CFG,        24,   2,          APB2_CFG,           0,      5,          16,         2,          0,          0,               0,               0,             0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(ths,            0,                0,   0,          0,                  0,      0,          0,          0,          0,          THS_CFG,         BUS_RST2,        BUS_GATE2,     0,         31,         8,          8,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(nand,           NAND_CFG,        24,   2,          NAND_CFG,           0,      4,          16,         2,          0,          NAND_CFG,        BUS_RST0,        BUS_GATE0,     0,         31,         13,         13,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc0_mod,     SD0_CFG,         24,   2,          SD0_CFG,            0,      4,          16,         2,          0,          SD0_CFG,         0,               0,             0,         31,         0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc0_bus,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               0,               BUS_GATE0,     0,         0,          0,          8,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc0_rst,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        0,             0,         0,          8,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc1_mod,     SD1_CFG,         24,   2,          SD1_CFG,            0,      4,          16,         2,          0,          SD1_CFG,         0,               0,             0,         31,         0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc1_bus,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               0,               BUS_GATE0,     0,         0,          0,          9,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc1_rst,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        0,             0,         0,          9,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc2_mod,     SD2_CFG,         24,   2,          SD2_CFG,            0,      4,          16,         2,          0,          SD2_CFG,         0,               0,             0,         31,         0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc2_bus,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               0,               BUS_GATE0,     DRAM_GATE, 0,          0,          10,             2,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc2_rst,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        0,             0,         0,          10,         0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc3_mod,     SD3_CFG,         24,   2,          SD3_CFG,            0,      4,          16,         2,          0,          SD3_CFG,         0,               0,             0,         31,         0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc3_bus,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               0,               BUS_GATE0,     0,         0,          0,          11,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdmmc3_rst,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        0,             0,         0,          11,         0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(spi0,           SPI0_CFG,        24,   2,          SPI0_CFG,           0,      4,          16,         2,          0,          SPI0_CFG,        BUS_RST0,        BUS_GATE0,     0,         31,         20,         20,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(spi1,           SPI1_CFG,        24,   2,          SPI1_CFG,           0,      4,          16,         2,          0,          SPI1_CFG,        BUS_RST0,        BUS_GATE0,     0,         31,         21,         21,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(spi2,           SPI2_CFG,        24,   2,          SPI2_CFG,           0,      4,          16,         2,          0,          SPI2_CFG,        BUS_RST0,        BUS_GATE0,     0,         31,         22,         22,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(i2s0,           I2S0_CFG,        16,   3,          0,                  0,      0,          0,          0,          0,          I2S0_CFG,        BUS_RST2,        BUS_GATE2,     0,         31,         12,         12,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(i2s1,           I2S1_CFG,        16,   3,          0,                  0,      0,          0,          0,          0,          I2S1_CFG,        BUS_RST2,        BUS_GATE2,     0,         31,         13,         13,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(spdif,          SPDIF_CFG,       16,   3,          SPDIF_CFG,          0,      4,          0,          0,          0,          SPDIF_CFG,       BUS_RST2,        BUS_GATE2,     0,         31,         1,          1,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(dsd,            DSD_CFG,         16,   2,          0,                  0,      0,          0,          0,          0,          DSD_CFG,         BUS_RST2,        BUS_GATE2,     0,         31,         2,          2,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(dmic, 	   DMIC_CFG,	    16,   2,	      0,		  0,	  0,	      0,          0,	      0,          DMIC_CFG,        BUS_RST2,        BUS_GATE2,     0,         31,         3,          3,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(usbphy0,        0,               0,    0,          0,                  0,      0,          0,          0,          0,          USB_CFG,         USB_CFG,         0,             0,         8,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(usbohci_16,     0,               0,    0,          0,                  0,      0,          0,          0,          0,          USB_CFG,         0,               0,             0,         16,         0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(usbohci0,       0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        BUS_GATE0,     DRAM_GATE, 0,          29,         29,             18,  &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(usbehci0,       0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        BUS_GATE0,     DRAM_GATE, 0,          26,         26,             17,  &clk_lock, NULL,             1);
SUNXI_CLK_PERIPH(de,             DE_CFG,          24,   3,          DE_CFG,             0,      4,          0,          0,          0,          DE_CFG,          BUS_RST1,        BUS_GATE1,     0,         31,         12,         12,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(ee,             EE_CFG,          24,   3,          EE_CFG,             0,      4,          0,          0,          0,          EE_CFG,          BUS_RST1,        BUS_GATE1,     DRAM_GATE, 31,         13,         13,             3,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(edma,	   EDMA_CFG,	    24,	  3,	      EDMA_CFG,		  0,	  4,	      0,	  0,	      0,	  EDMA_CFG, 	   0,               0,             DRAM_GATE, 31, 	  0,	      0, 	      4,   &clk_lock, NULL,	        0);
SUNXI_CLK_PERIPH(tcon0,          TCON_CFG,        24,   3,          0,                  0,      0,          0,          0,          0,          TCON_CFG,        BUS_RST1,        BUS_GATE1,     0,         31,         4,          4,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(csi_s,          CSI_CFG,         24,   3,          CSI_CFG,            16,     4,          0,          0,          0,          CSI_CFG,         BUS_RST1,        BUS_GATE1,     DRAM_GATE, 31,         8,          8,              1,   &clk_lock, &com_gates[0],    0);
SUNXI_CLK_PERIPH(csi_m,          CSI_CFG,         8,    3,          CSI_CFG,            0,      5,          0,          0,          0,          CSI_CFG,         BUS_RST1,        BUS_GATE1,     DRAM_GATE, 15,         8,          8,              1,   &clk_lock, &com_gates[0],    1);
SUNXI_CLK_PERIPH(csi_misc,       0,               0,    0,          0,                  0,      0,          0,          0,          0,          CSI_MISC,        BUS_RST1,        BUS_GATE1,     DRAM_GATE, 31,         8,          8,              1,   &clk_lock, &com_gates[0],    2);
SUNXI_CLK_PERIPH(adda,           ADDA_CFG,        24,   2,          0,                  0,      0,          0,          0,          0,          ADDA_CFG,        BUS_RST2,        BUS_GATE2,     0,         31,         0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(wlan,           WLAN_CFG,        8,    1,          0,                  0,      0,          0,          0,          0,          WLAN_CFG,        BUS_RST1,        0,             0,         31,         1,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(mbus,           MBUS_CFG,        24,   2,          MBUS_CFG,           0,      3,          0,          0,          0,          MBUS_CFG,        MBUS_RST,        0,             0,         31,         31,         0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(usbotg,         0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        BUS_GATE0,     0,         0,          24,         24,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(sdram,          DRAM_CFG,        20,   2,          DRAM_CFG,           0,      2,          0,          0,          0,          DRAM_CFG,        BUS_RST0,        BUS_GATE0,     0,         31,         14,         14,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(psram,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        BUS_GATE0,     0,         0,          15,         15,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(dma,            0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST0,        BUS_GATE0,     0,         0,          6,          6,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(uart0,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          16,         16,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(uart1,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          17,         17,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(uart2,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          18,         18,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(uart3,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          19,         19,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(uart4,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          20,         20,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(uart5,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          21,         21,             0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(twi0,           0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          0,          0,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(twi1,           0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          1,          1,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(twi2,           0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST3,        BUS_GATE3,     0,         0,          2,          2,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(pio,            0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               0,               BUS_GATE2,     0,         0,          0,          5,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(gpadc,          0,               0,    0,          0,                  0,      0,          0,          0,          0,          GPADC_CFG,       BUS_RST2,        BUS_GATE2,     0,        31,         10,         10,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(keyadc,         0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               BUS_RST2,        BUS_GATE2,     0,         0,          9,          9,              0,   &clk_lock, NULL,             0);
SUNXI_CLK_PERIPH(losc_out,       0,               0,    0,          0,                  0,      0,          0,          0,          0,          0,               0,               LOSC_OUT_GATE, 0,         0,          0,          0,              0,   &clk_lock, NULL,             0);

struct periph_init_data sunxi_periphs_init[] = {
	{"cpu",            CLK_GET_RATE_NOCACHE, cpu_parents,            ARRAY_SIZE(cpu_parents),            &sunxi_clk_periph_cpu              },
	{"cpuapb",         0,                    cpuapb_parents,         ARRAY_SIZE(cpuapb_parents),         &sunxi_clk_periph_cpuapb           },
	{"axi",            0,                    axi_parents,            ARRAY_SIZE(axi_parents),            &sunxi_clk_periph_axi              },
	{"pll_periphahb0", CLK_IGNORE_SYNCBOOT,  pll_periphahb0_parents, ARRAY_SIZE(pll_periphahb0_parents), &sunxi_clk_periph_pll_periphahb0   },
	{"ahb1",           0,                    ahb1_parents,           ARRAY_SIZE(ahb1_parents),           &sunxi_clk_periph_ahb1             },
	{"apb1",           0,                    apb1_parents,           ARRAY_SIZE(apb1_parents),           &sunxi_clk_periph_apb1             },
	{"apb2",           0,                    apb2_parents,           ARRAY_SIZE(apb2_parents),           &sunxi_clk_periph_apb2             },
	{"ths",            0,                    ths_parents,            ARRAY_SIZE(ths_parents),            &sunxi_clk_periph_ths              },
	{"nand",           0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_nand             },
	{"sdmmc0_mod",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc0_mod       },
	{"sdmmc0_bus",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc0_bus       },
	{"sdmmc0_rst",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc0_rst       },
	{"sdmmc1_mod",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc1_mod       },
	{"sdmmc1_bus",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc1_bus       },
	{"sdmmc1_rst",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc1_rst       },
	{"sdmmc2_mod",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc2_mod       },
	{"sdmmc2_bus",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc2_bus       },
	{"sdmmc2_rst",     0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_sdmmc2_rst       },
	{"sdmmc3_mod",	   0,			 periphx2_parents,	 ARRAY_SIZE(periphx2_parents),	     &sunxi_clk_periph_sdmmc3_mod	},
	{"sdmmc3_bus",	   0,			 periphx2_parents,	 ARRAY_SIZE(periphx2_parents),	     &sunxi_clk_periph_sdmmc3_bus	},
	{"sdmmc3_rst",	   0,			 periphx2_parents,	 ARRAY_SIZE(periphx2_parents),	     &sunxi_clk_periph_sdmmc3_rst	},
	{"spi0",           0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_spi0             },
	{"spi1",           0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_spi1             },
	{"spi2",           0,                    periphx2_parents,       ARRAY_SIZE(periphx2_parents),       &sunxi_clk_periph_spi2             },
	{"i2s0",           0,                    i2s_parents,            ARRAY_SIZE(i2s_parents),            &sunxi_clk_periph_i2s0             },
	{"i2s1",           0,                    i2s_parents,            ARRAY_SIZE(i2s_parents),            &sunxi_clk_periph_i2s1             },
	{"spdif",          0,                    i2s_parents,            ARRAY_SIZE(i2s_parents),            &sunxi_clk_periph_spdif            },
	{"dsd",            0,                    adda_parents,           ARRAY_SIZE(adda_parents),           &sunxi_clk_periph_dsd              },
	{"dmic",           0,                    adda_parents,           ARRAY_SIZE(adda_parents),           &sunxi_clk_periph_dmic             },
	{"usbphy0",        0,                    hosc_parents,           ARRAY_SIZE(hosc_parents),           &sunxi_clk_periph_usbphy0          },
	{"usbohci_16",     0,                    ahb1mod_parents,        ARRAY_SIZE(ahb1mod_parents),        &sunxi_clk_periph_usbohci_16       },
	{"usbohci0",       0,                    usbohci_parents,        ARRAY_SIZE(usbohci_parents),        &sunxi_clk_periph_usbohci0         },
	{"de",             0,                    de_parents,             ARRAY_SIZE(de_parents),             &sunxi_clk_periph_de               },
	{"ee",             0,                    de_parents,             ARRAY_SIZE(de_parents),             &sunxi_clk_periph_ee               },
	{"edma",           0,                    de_parents,             ARRAY_SIZE(de_parents),             &sunxi_clk_periph_edma             },
	{"tcon0",          0,                    tcon0_parents,          ARRAY_SIZE(tcon0_parents),          &sunxi_clk_periph_tcon0            },
	{"csi_s",          0,                    periphx_parents,        ARRAY_SIZE(periphx_parents),        &sunxi_clk_periph_csi_s            },
	{"csi_m",          0,                    csi_m_parents,          ARRAY_SIZE(csi_m_parents),          &sunxi_clk_periph_csi_m            },
	{"csi_misc",       0,                    csi_misc_parents,       ARRAY_SIZE(csi_misc_parents),       &sunxi_clk_periph_csi_misc         },
	{"adda",           0,                    adda_parents,           ARRAY_SIZE(adda_parents),           &sunxi_clk_periph_adda             },
	{"wlan",           0,                    wlan_parents,           ARRAY_SIZE(wlan_parents),           &sunxi_clk_periph_wlan             },
	{"mbus",           0,                    mbus_parents,           ARRAY_SIZE(mbus_parents),           &sunxi_clk_periph_mbus             },
	{"usbehci0",       0,                    ahb1mod_parents,        ARRAY_SIZE(ahb1mod_parents),        &sunxi_clk_periph_usbehci0         },
	{"usbotg",         0,                    ahb1mod_parents,        ARRAY_SIZE(ahb1mod_parents),        &sunxi_clk_periph_usbotg           },
	{"sdram",          0,                    sdram_parents,          ARRAY_SIZE(sdram_parents),          &sunxi_clk_periph_sdram            },
	{"psram",          0,                    ahb1mod_parents,        ARRAY_SIZE(ahb1mod_parents),        &sunxi_clk_periph_psram            },
	{"dma",            0,                    ahb1mod_parents,        ARRAY_SIZE(ahb1mod_parents),        &sunxi_clk_periph_dma              },
	{"uart0",          0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_uart0            },
	{"uart1",          0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_uart1            },
	{"uart2",          0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_uart2            },
	{"uart3",          0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_uart3            },
	{"uart4",          0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_uart4            },
	{"uart5",          0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_uart5            },
	{"twi0",           0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_twi0             },
	{"twi1",           0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_twi1             },
	{"twi2",           0,                    apb2mod_parents,        ARRAY_SIZE(apb2mod_parents),        &sunxi_clk_periph_twi2             },
	{"pio",            0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_pio              },
	{"gpadc",          0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_gpadc            },
	{"keyadc",         0,                    apb1mod_parents,        ARRAY_SIZE(apb1mod_parents),        &sunxi_clk_periph_keyadc           },
	{"losc_out",       0,                    losc_parents,           ARRAY_SIZE(losc_parents),           &sunxi_clk_periph_losc_out         },
};

void __init sunxi_init_clocks(void)
{
}

#ifdef CONFIG_OF
/**
 * set default rate for clk
 */
static int __set_clk_rates(struct device_node *node , struct clk* clk)
{
	u32 assigned_clock_rates = 0;
	bool res = -1;

	/*set pll default rate here , and make you know it is setted succeed or not*/
	if( !of_property_read_u32( node , "assigned-clock-rates" , &assigned_clock_rates) )
	{
		u32 real_clock_rate = 0;
		clk_set_rate(clk , assigned_clock_rates);
		real_clock_rate = clk_get_rate(clk);
		if( real_clock_rate != assigned_clock_rates )
		{
			pr_info("%s-set_default_rate=%u , but real_get_rate=%u failured!\n" ,
				__clk_get_name(clk) , assigned_clock_rates , real_clock_rate );
		}
		else
		{
			pr_info("%s-set_default_rate=%u success!\n",
				__clk_get_name(clk) , assigned_clock_rates);
			res = 0;
		}
	}

	return res;
}

/**
 * set default clk source for clk
 */
static int __set_clk_parents(struct device_node *node , struct clk* clk)
{
	int index = 0, rc ;
	struct of_phandle_args clkspec;
	struct clk *pclk;

	rc = of_parse_phandle_with_args(node, "assigned-clock-parents",
				"#clock-cells",    index, &clkspec);
	if (rc < 0)
	{
		/* skip empty (null) phandles */
		return rc;
	}

	pclk = of_clk_get_from_provider(&clkspec);
	if (IS_ERR(pclk))
	{
		pr_warn("clk: couldn't get parent clock %d for %s\n",
				index, node->full_name);
		return PTR_ERR(pclk);
	}

	rc = clk_set_parent(clk, pclk);
	if (rc < 0)
	{
		pr_err("%s-set_default_source=%s failed at: %d\n",
			__clk_get_name(clk), __clk_get_name(pclk), rc);
	}
	else
	{
		pr_info("%s-set_default_source=%s success!\n",
			__clk_get_name(clk) , __clk_get_name(pclk) );
	}

	return rc;
}

/**
*of_sunxi_clocks_init() - Clocks initialize
*/
void of_sunxi_clocks_init(struct device_node *node)
{
#ifdef CONFIG_SUNXI_CLK_DUMMY_DEBUG
	sunxi_clk_base = &dummy_reg[0];
	dummy_reg_init();
#else
	sunxi_clk_base = of_iomap(node ,0);
#endif
	sunxi_clk_periph_losc_out.gate.bus = of_iomap(node , 2);
	/*do some initialize arguments here*/
	sunxi_clk_factor_initlimits();

	/*sunxi_clk_get_factors_ops(&pll_mipi_ops);
	pll_mipi_ops.get_parent = get_parent_pll_mipi;
	pll_mipi_ops.set_parent = set_parent_pll_mipi;
	pll_mipi_ops.enable = clk_enable_pll_mipi;
	pll_mipi_ops.disable = clk_disable_pll_mipi;*/
	pr_info( "%s : sunxi_clk_base[0x%lx]\n", __func__ , (unsigned long)sunxi_clk_base);

	clk_add_alias("pll1",NULL,"pll_cpu",NULL);
	clk_add_alias("pll2",NULL,"pll_audio",NULL);
	clk_add_alias("pll3",NULL,"pll_video0",NULL);
	clk_add_alias("pll4",NULL,"pll_ddr0",NULL);
	clk_add_alias("pll5",NULL,"pll_periph0",NULL);
	clk_add_alias("pll5ahb0",NULL,"pll_periphahb0",NULL);
	clk_add_alias("pll6",NULL,"pll_video1",NULL);
	clk_add_alias("pll7",NULL,"pll_24m",NULL);
	clk_add_alias("pll8",NULL,"pll_periph1",NULL);
	clk_add_alias("pll9",NULL,"pll_de",NULL);
	clk_add_alias("pll10",NULL,"pll_ddr1",NULL);
#ifdef CONFIG_COMMON_CLK_ENABLE_SYNCBOOT_EARLY
	clk_syncboot();
#endif
}

void of_sunxi_fixed_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	u32 rate;

	if (of_property_read_u32(node, "clock-frequency", &rate))
		return;

	of_property_read_string(node, "clock-output-names", &clk_name);

	clk = clk_register_fixed_rate(NULL, clk_name, NULL, CLK_IS_ROOT, rate);
	if (!IS_ERR(clk))
	{
		clk_register_clkdev(clk, clk_name, NULL);
		of_clk_add_provider(node, of_clk_src_simple_get, clk);
	}
}

void of_sunxi_fixed_factor_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	const char *parent_name;
	u32 div, mult;

	if (of_property_read_u32(node, "clock-div", &div)) {
		pr_err("%s Fixed factor clock <%s> must have a clock-div property\n",
			__func__, node->name);
		return;
	}

	if (of_property_read_u32(node, "clock-mult", &mult)) {
		pr_err("%s Fixed factor clock <%s> must have a clokc-mult property\n",
			__func__, node->name);
		return;
	}

	of_property_read_string(node, "clock-output-names", &clk_name);
	parent_name = of_clk_get_parent_name(node, 0);

	clk = clk_register_fixed_factor(NULL, clk_name, parent_name, 0,
					mult, div);
	if (!IS_ERR(clk))
	{
		clk_register_clkdev(clk, clk_name, NULL);
		of_clk_add_provider(node, of_clk_src_simple_get, clk);
	}
}

/**
 * of_pll_clk_setup() - Setup function for pll factors clk
 */
void of_pll_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	const char *lock_mode = NULL;
	struct factor_init_data *factor;
	int i;
	int ret;

	of_property_read_string(node, "clock-output-names", &clk_name);
	ret = of_property_read_string(node, "lock-mode", &lock_mode);

	/*get pll clk init config */
	for(i=0; i<ARRAY_SIZE(sunxi_factos); i++)
	{
		factor = &sunxi_factos[i];
		if( 0 == strcmp(clk_name , factor->name) )
		{
			if (IS_ERR_VALUE(ret)) {
				factor->lock_mode = PLL_LOCK_NONE_MODE;
			} else {
				if (strcmp(lock_mode, "new") == 0) {
					factor->lock_mode = PLL_LOCK_NEW_MODE;
				} else if (strcmp(lock_mode, "old") == 0) {
					factor->lock_mode = PLL_LOCK_OLD_MODE;
				} else if (strcmp(lock_mode, "none") == 0) {
					factor->lock_mode = PLL_LOCK_NONE_MODE;
				} else {
					factor->lock_mode = PLL_LOCK_NONE_MODE;
				}
			}

			/*register clk */
			clk = sunxi_clk_register_factors( NULL ,  sunxi_clk_base , &clk_lock , factor );
			/*add to of */
			if (!IS_ERR(clk))
			{
				clk_register_clkdev(clk, clk_name, NULL);
				of_clk_add_provider(node, of_clk_src_simple_get, clk);
				__set_clk_parents(node , clk);
				__set_clk_rates(node , clk);
				/*pr_err( "%s : %s \n", __func__ , clk_name );*/
				return ;
			}

		}
	}

	pr_err("clk %s not found in %s\n",clk_name , __func__ );
}

/**
 * of_periph_clk_setup() - Setup function for periph clk
 */
void of_periph_clk_setup(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	struct periph_init_data *periph;
	unsigned int i;

	of_property_read_string(node, "clock-output-names", &clk_name);

	/*get periph clk init config */
	for(i=0; i<ARRAY_SIZE(sunxi_periphs_init); i++)
	{
		periph = &sunxi_periphs_init[i];
		if( 0 == strcmp(clk_name , periph->name) )
		{
			/*register clk */
			clk = sunxi_clk_register_periph( clk_name , periph->parent_names,
						periph->num_parents , periph->flags , sunxi_clk_base , periph->periph );
			/*add to of */
			if (!IS_ERR(clk))
			{
				clk_register_clkdev(clk, clk_name, NULL);
				of_clk_add_provider(node, of_clk_src_simple_get, clk);
				__set_clk_parents(node , clk);
				__set_clk_rates(node , clk);
				/*pr_err( "%s : %s \n", __func__ , clk_name );*/
				return ;
			}
		}
	}
	pr_err("clk %s not found in %s\n",clk_name , __func__ );
}

CLK_OF_DECLARE(sunxi_clocks_init, "allwinner,sunxi-clk-init", of_sunxi_clocks_init);
CLK_OF_DECLARE(sunxi_fixed_clk, "allwinner,fixed-clock", of_sunxi_fixed_clk_setup);
CLK_OF_DECLARE(pll_clk, "allwinner,sunxi-pll-clock", of_pll_clk_setup);
CLK_OF_DECLARE(sunxi_fixed_factor_clk, "allwinner,fixed-factor-clock", of_sunxi_fixed_factor_clk_setup);
CLK_OF_DECLARE(periph_clk, "allwinner,sunxi-periph-clock", of_periph_clk_setup);
#endif

