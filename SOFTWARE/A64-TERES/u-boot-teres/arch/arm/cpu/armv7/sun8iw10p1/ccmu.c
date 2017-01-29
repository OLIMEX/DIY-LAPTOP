/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/arch/io.h>
#include <asm/arch/ccmu.h>
void ccm_clock_enable(u32 clk_id)
{
	switch(clk_id>>8) {
		case AXI_BUS:
			set_wbit(CCM_AXI_GATE_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_BUS0:
			set_wbit(CCM_AHB1_GATE0_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_BUS1:
			set_wbit(CCM_AHB1_GATE1_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case APB1_BUS0:
			set_wbit(CCM_APB1_GATE0_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case APB2_BUS0:
			set_wbit(CCM_APB2_GATE0_CTRL, 0x1U<<(clk_id&0xff));
			break;
	}
}

void ccm_clock_disable(u32 clk_id)
{
	switch(clk_id>>8) {
		case AXI_BUS:
			clr_wbit(CCM_AXI_GATE_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_BUS0:
			clr_wbit(CCM_AHB1_GATE0_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_BUS1:
			clr_wbit(CCM_AHB1_GATE1_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case APB1_BUS0:
			clr_wbit(CCM_APB1_GATE0_CTRL, 0x1U<<(clk_id&0xff));
			break;
		case APB2_BUS0:
			clr_wbit(CCM_APB2_GATE0_CTRL, 0x1U<<(clk_id&0xff));
			break;
	}
}

void ccm_module_enable(u32 clk_id)
{
	switch(clk_id>>8) {
		case AHB1_BUS0:
			set_wbit(CCM_AHB1_RST_REG0, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_BUS1:
			set_wbit(CCM_AHB1_RST_REG1, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_LVDS:
			set_wbit(CCM_AHB1_RST_REG2, 0x1U<<(clk_id&0xff));
			break;
		case APB1_BUS0:
			set_wbit(CCM_APB1_RST_REG, 0x1U<<(clk_id&0xff));
			break;
		case APB2_BUS0:
			set_wbit(CCM_APB2_RST_REG, 0x1U<<(clk_id&0xff));
			break;
	}
}

void ccm_module_disable(u32 clk_id)
{
	switch(clk_id>>8) {
		case AHB1_BUS0:
			clr_wbit(CCM_AHB1_RST_REG0, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_BUS1:
			clr_wbit(CCM_AHB1_RST_REG1, 0x1U<<(clk_id&0xff));
			break;
		case AHB1_LVDS:
			clr_wbit(CCM_AHB1_RST_REG2, 0x1U<<(clk_id&0xff));
			break;
		case APB1_BUS0:
			clr_wbit(CCM_APB1_RST_REG, 0x1U<<(clk_id&0xff));
			break;
		case APB2_BUS0:
			clr_wbit(CCM_APB2_RST_REG, 0x1U<<(clk_id&0xff));
			break;
	}
}

void ccm_module_reset(u32 clk_id)
{
	ccm_module_disable(clk_id);
	ccm_module_disable(clk_id);
	ccm_module_enable(clk_id);
}

void ccm_clock_disable_all(void)
{
	writel(0, CCM_AXI_GATE_CTRL);
	writel(0, CCM_AHB1_GATE0_CTRL);
	writel(0, CCM_AHB1_GATE1_CTRL);
	writel(0, CCM_APB1_GATE0_CTRL);
	writel(0, CCM_APB2_GATE0_CTRL);
//	clr_wbit(CCM_AXI_GATE_CTRL, 0xffffffff);
//	clr_wbit(CCM_AHB1_GATE0_CTRL, 0xffffffff);
//	clr_wbit(CCM_AHB1_GATE1_CTRL, 0xffffffff);
//	clr_wbit(CCM_APB1_GATE0_CTRL, 0xffffffff);
//	clr_wbit(CCM_APB2_GATE0_CTRL, 0xffffffff);
}

void ccm_reset_all_module(void)
{
	writel(0, CCM_AHB1_RST_REG0);
	writel(0, CCM_AHB1_RST_REG1);
	writel(0, CCM_AHB1_RST_REG2);
	writel(0, CCM_APB1_RST_REG);
	writel(0, CCM_APB2_RST_REG);
//	clr_wbit(CCM_AHB1_RST_REG0, 0xffffffff);
//	clr_wbit(CCM_AHB1_RST_REG1, 0xffffffff);
//	clr_wbit(CCM_AHB1_RST_REG2, 0xffffffff);
//	clr_wbit(CCM_APB1_RST_REG, 0xffffffff);
//	clr_wbit(CCM_APB2_RST_REG, 0xffffffff);
}

static void ccm_wait_pll_stable(u32 pll_base)
{
#ifndef FPGA_PLATFORM
	u32 rval = 0;
	u32 time = 0xffff;

	do {
		rval = readl(pll_base);
		time --;
	} while (time && !(rval & CCM_PLL_STABLE_FLAG));
#endif
}

/* pll1 = 24*n*k/m */
u32 ccm_setup_pll1_cpux_clk(u32 pll_clk)
{
	u32 n, k, m = 1;
	u32 rval = 0;
	u32 div = 0;
	u32 mod2, mod3;
	u32 min_mod = 0;

#ifndef FPGA_PLATFORM
	if (pll_clk < 240000000)
		pll_clk = 240000000;
	if (pll_clk > 2000000000)
		pll_clk = 2000000000;
#endif

#ifdef SYSTEM_SIMULATION
	k = 1;
	n = 15;	//for 360M
	rval = (1U << 31) | ((n-1) << 8) | ((k-1) << 4) | (m-1);
	writel(rval, CCM_PLL1_CPUX_CTRL);
	ccm_wait_pll_stable(CCM_PLL1_CPUX_CTRL);
	return PLL1_CPUX_CLK;
#else
	div = pll_clk/24000000;
	if (div <= 32) {
		n = div;
		k = 1;
	} else {
		/* when m=1, we cann't get absolutely accurate value for follow clock:
		 * 840(816), 888(864),
		 * 984(960),
		 * 1032(1008),
		 * 1128(1104), 1176(1152),
		 * 1272(1248)
		 * 1320(1296),
		 * 1416(1392), 1464(1440),
		 * 1560(1536),
		 * 1608(1584),
		 * 1704(1680), 1752(1728),
		 * 1848(1824), 1896(1872),
		 * 1992(1968)
		 */
		mod2 = div&1;
		mod3 = div%3;
		min_mod = mod2;
		k = 2;
		if (min_mod > mod3) {
			min_mod = mod3;
			k = 3;
		}
		n = div / k;
	}

	rval = (1U << 31) | ((n-1) << 8) | ((k-1) << 4) | (m-1);
	writel(rval, CCM_PLL1_CPUX_CTRL);
	ccm_wait_pll_stable(CCM_PLL1_CPUX_CTRL);

	return 24000000 * n * k / m;
#endif
}

u32 ccm_get_pll1_cpux_clk(void)
{
#ifdef FPGA_PLATFORM
	return PLL1_CPUX_CLK;
#else
	u32 rval = 0;
	u32 n, k, m;
	rval = readl(CCM_PLL1_CPUX_CTRL);
	n = (0x1f & (rval >> 8)) + 1;
	k = (0x3 & (rval >> 4)) + 1;
	m = (0x3 & (rval >> 0)) + 1;
	return 24000000 * n * k / m;
#endif
}

/* pll2 = 24*n/p/m */
u32 ccm_setup_pll2_audio_clk(u32 pll2_clk)
{
	return 0;
}

u32 ccm_get_pll2_audio_clk(void)
{
	return 0;
}

/* pll = 24 * n / m */
u32 ccm_setup_de_ve_pll_clk(u32 base, u32 mode_sel, u32 pll_clk)
{
	u32 n = 1, m = 8;
	u32 frac_clk = 1;
	u32 rval;

	if (mode_sel) {
		//integer mode
		if (pll_clk <=384000000) {
			n = pll_clk/3000000;
		} else if (pll_clk <=512000000) {
			m = 6;
			n = pll_clk/4000000;
		} else if (pll_clk <=768000000) {
			m = 4;
			n = pll_clk/6000000;
		} else if (pll_clk <=1024000000) {
			m = 3;
			n = pll_clk/8000000;
		} else if (pll_clk <=1536000000) {
			m = 2;
			n = pll_clk/12000000;
		} else {
			m = 1;
			n = pll_clk/24000000;
		}
	} else {
		//fractional mode
		if (pll_clk == 270000000)
			frac_clk = 0;
	}
	rval = (1U << 31) | (frac_clk << 25) | (mode_sel << 24) | ((n-1) << 8) | (m - 1);
	writel(rval, base);
	ccm_wait_pll_stable(base);
	return pll_clk;
}

u32 ccm_get_de_ve_pll_clk(u32 base)
{
	u32 rval = readl(base);
	u32 mode = 1 & (rval >> 24);
	u32 pll_clk;

	if (mode) {
		//integer mode;
		u32 m = (rval & 0xf) + 1;
		u32 n = ((rval >> 8) & 0x7f) + 1;
		pll_clk = 24000000 * n / m;
	} else {
		if (1 & (rval >> 25))
			pll_clk = 297000000;
		else
			pll_clk = 270000000;
	}
	return pll_clk;
}

/* pll3 = 24*n/m */
u32 ccm_setup_pll3_video0_clk(u32 mode_sel, u32 pll_clk)
{
	return ccm_setup_de_ve_pll_clk(CCM_PLL3_VIDEO_CTRL, mode_sel, pll_clk);
}

u32 ccm_get_pll3_video0_clk(void)
{
#ifdef FPGA_PLATFORM
	return PLL3_VIDEO0_CLK;
#else
	return ccm_get_de_ve_pll_clk(CCM_PLL3_VIDEO_CTRL);
#endif
}

/* pll4 = 24*n/m */
u32 ccm_setup_pll4_ve_clk(u32 mode_sel, u32 pll_clk)
{
	return ccm_setup_de_ve_pll_clk(CCM_PLL4_VE_CTRL, mode_sel, pll_clk);
}

u32 ccm_get_pll4_ve_clk(void)
{
#ifdef FPGA_PLATFORM
	return PLL4_VE_CLK;
#else
	return ccm_get_de_ve_pll_clk(CCM_PLL4_VE_CTRL);
#endif
}

/* pll5 = 24*n*k/m */
u32 ccm_setup_pll5_ddr_clk(u32 pll_clk)
{
	u32 n, k, m = 1;
	u32 mod2, mod3, min_mod;
	u32 div = 0;
	u32 rval;

#ifndef FPGA_PLATFORM
	if (pll_clk < 24000000)
		pll_clk = 24000000;
	if (pll_clk > 1000000000)
		pll_clk = 1000000000;
#endif

#ifdef SYSTEM_SIMULATION
	k = 1;
	n = 15;	//for 360M
	rval = (1U << 31) | ((n-1) << 8) | ((k-1) << 4) | (m-1);
	writel(rval, CCM_PLL5_DDR_CTRL);
	writel(rval|(1U << 20), CCM_PLL5_DDR_CTRL);
	ccm_wait_pll_stable(CCM_PLL5_DDR_CTRL);
	return PLL5_DDR_CLK;
#else
	div = pll_clk/24000000;
	if (div <= 32) {
		n = div;
		k = 1;
	} else {
		/* when m=1, we cann't get absolutely accurate value for follow clock:
		 * 840(816), 888(864),
		 * 984(960)
		 */
		mod2 = div&1;
		mod3 = div%3;
		min_mod = mod2;
		k = 2;
		if (min_mod > mod3) {
			min_mod = mod3;
			k = 3;
		}
		n = div / k;
	}
	rval = (1U << 31)  | ((n-1) << 8) | ((k-1) << 4) | (m-1);
	writel(rval, CCM_PLL5_DDR_CTRL);
	writel(rval|(1U << 20), CCM_PLL5_DDR_CTRL);
	ccm_wait_pll_stable(CCM_PLL5_DDR_CTRL);

	return 24000000 * n * k / m;
#endif
}

u32 ccm_get_pll5_ddr_clk(void)
{
#ifdef FPGA_PLATFORM
	return PLL5_DDR_CLK;
#else
	u32 rval = 0;
	u32 n, k, m;
	rval = readl(CCM_PLL5_DDR_CTRL);
	n = (0x1f & (rval >> 8)) + 1;
	k = (0x3 & (rval >> 4)) + 1;
	m = (0x3 & (rval >> 0)) + 1;
	return 24000000 * n * k / m;
#endif
}

/* pll1 = 24*n*k/2 */
u32 ccm_setup_pll6_dev_clk(u32 pll_clk)
{
	u32 n, k;
	u32 div;
	u32 rval;

#ifndef FPGA_PLATFORM
	if (pll_clk < 240000000)
		pll_clk = 240000000;
	if (pll_clk > 2000000000)
		pll_clk = 2000000000;
#endif

#ifdef SYSTEM_SIMULATION
	k = 1;
	n = 15;	//for 360M
	rval = (1U << 31) | (1U << 24) | (1U << 18) | ((n-1) << 8) | ((k-1) << 4);
	writel(rval, CCM_PLL6_MOD_CTRL);
	ccm_wait_pll_stable(CCM_PLL6_MOD_CTRL);
	return PLL6_DEV_CLK >> 1;
#else
	div = pll_clk/24000000;
	if (div <= 32)
		k = 1;
	else if (div <= 64)
		k = 2;
	else if (div <= 96)
		k = 3;
	else
		k = 4;
	n = div / k;

	rval = (1U << 31) | (1U << 24) | (1U << 18) | ((n-1) << 8) | ((k-1) << 4);
	writel(rval, CCM_PLL6_MOD_CTRL);
	ccm_wait_pll_stable(CCM_PLL6_MOD_CTRL);

	return (24000000 * n * k)>>1;
#endif
}

u32 ccm_get_pll6_dev_clk(void)
{
#ifdef FPGA_PLATFORM
	return PLL6_DEV_CLK >> 1;
#else
	u32 rval = 0;
	u32 n, k;
	rval = readl(CCM_PLL6_MOD_CTRL);
	n = (0x1f & (rval >> 8)) + 1;
	k = (0x3 & (rval >> 4)) + 1;
	return (24000000 * n * k)>>1;
#endif
}

/* pll7 = 24*n/m */
u32 ccm_setup_pll7_video0_clk(u32 mode_sel, u32 pll_clk)
{
	return ccm_setup_de_ve_pll_clk(CCM_PLL7_VIDEO1_CTRL, mode_sel, pll_clk);
}

u32 ccm_get_pll7_video0_clk(void)
{
#ifdef FPGA_PLATFORM
	return PLL7_VIDEO1_CLK;
#else
	return ccm_get_de_ve_pll_clk(CCM_PLL7_VIDEO1_CTRL);
#endif
}

/* pll8 = 24*n/m */
u32 ccm_setup_pll8_gpu_clk(u32 mode_sel, u32 pll_clk)
{
	return ccm_setup_de_ve_pll_clk(CCM_PLL8_GPU_CTRL, mode_sel, pll_clk);
}

u32 ccm_get_pll8_gpu_clk(void)
{
#ifdef FPGA_PLATFORM
	return PLL8_GPU_CLK;
#else
	return ccm_get_de_ve_pll_clk(CCM_PLL8_GPU_CTRL);
#endif
}

void ccm_set_cpu_clk_src(u32 src)
{
	u32 rval = readl(CCM_CPU_L2_AXI_CTRL);
	rval &= ~(3 << 16);
	rval |= src << 16;
	writel(rval, CCM_CPU_L2_AXI_CTRL);
}

u32 ccm_set_mbus0_clk(u32 src, u32 clk)
{
	u32 sclk = 0;
	u32 real = 0;
	u32 m = 1;
	u32 rval;

	switch (src) {
		case 1:
			sclk = ccm_get_pll6_dev_clk();
			break;
		case 2:
			sclk = ccm_get_pll5_ddr_clk();
			break;
		default:
			sclk = 24000000;
			break;
	}
	real = sclk;
	while (real > clk) {
		m++;
		real = sclk / m;
	}
	rval = (1U << 31) | (src << 24) | (m-1);
	writel(rval, CCM_MBUS_SCLK_CTRL0);
	return real;
}

u32 ccm_set_mbus1_clk(u32 src, u32 clk)
{
	u32 sclk = 0;
	u32 real = 0;
	u32 m = 1;
	u32 rval;

	switch (src) {
		case 1:
			sclk = ccm_get_pll6_dev_clk();
			break;
		case 2:
			sclk = ccm_get_pll5_ddr_clk();
			break;
		default:
			sclk = 24000000;
			break;
	}

	real = sclk;
	while (real > clk) {
		m++;
		real = sclk / m;
	}
	rval = (1U << 31) | (src << 24) | (m-1);
	writel(rval, CCM_MBUS_SCLK_CTRL1);
	return real;
}

void ccm_set_cpu_l2_axi_div(u32 periph_div, u32 l2_div, u32 axi_div)
{
	u32 rval = readl(CCM_CPU_L2_AXI_CTRL);
	periph_div >>= 2;
	l2_div = l2_div&0x4 ? 4 : l2_div-1;
	axi_div = axi_div&0x4 ? 4 : axi_div-1;
	rval &= ~((1 << 8) | (7 << 4) | (7));
	rval |= (periph_div << 8) | (l2_div << 4) | (axi_div);
	writel(rval, CCM_CPU_L2_AXI_CTRL);
}

void ccm_set_ahb1_clk_src(u32 src)
{
	u32 rval = readl(CCM_AHB1_APB1_CTRL);
	rval &= ~(3 << 12);
	rval |= src << 12;
	writel(rval, CCM_AHB1_APB1_CTRL);
}

void ccm_set_ahb1_apb1_div(u32 prediv, u32 ahb1_div, u32 apb1_div)
{
	u32 ahb1div;
	u32 rval = readl(CCM_AHB1_APB1_CTRL);

	switch (ahb1_div) {
		case 1:
			ahb1div = 0;
			break;
		case 2:
			ahb1div = 1;
			break;
		case 4:
			ahb1div = 2;
			break;
		case 8:
			ahb1div = 3;
			break;
		default:
			ahb1div = 1;
			break;
	}
	rval &= ~(0x3f << 4); //(3 << 6) | (3 << 4) | (3 << 8)
	rval |= (apb1_div << 8) | ((prediv-1) << 6) | (ahb1div << 4);
	writel(rval, CCM_AHB1_APB1_CTRL);
}

s32 ccm_set_apb2_clk(u32 apb2_clk)
{
	u32 rval;
	u32 src;
	u32 sclk;
	u32 div, n, m;

	if (apb2_clk <= 32000) {
		writel(0, CCM_APB2_CLK_CTRL);
		return 0;
	}
	if (apb2_clk <= 24000000) {
		writel(1U << 24, CCM_APB2_CLK_CTRL);
		return 0;
	}

	src = 2;
	sclk = ccm_get_pll6_dev_clk();
	div = sclk / apb2_clk;
	if (div <= 32)
		n = 0;
	else if (div <= 64)
		n = 1;
	else if (div <= 128)
		n = 2;
	else
		n = 3;
	m = (div / (1 << n)) - 1;

	rval = (src << 24) | (n << 16) | m;
	writel(rval, CCM_APB2_CLK_CTRL);
	return rval;
}

void ccm_set_pll_stable_time(u32 time)
{
	writel(time&0xffff, CCM_PLL_STABLE_REG);
}

void ccm_set_mclk_stable_time(u32 time)
{
	writel(time&0xffff, CCM_MCLK_STABLE_REG);
}

u32 ccm_get_axi_clk(void)
{

#ifdef FPGA_PLATFORM
	return AXICLK;
#else
	u32 src;
	u32 rval;
	u32 axidiv;
	rval = readl(CCM_CPU_L2_AXI_CTRL);
	src = 0x3 & (rval >> 16);
	axidiv = (0x7 & (rval >> 0)) + 1;
	switch (src) {
		case 0:
			src = 32000;
			break;
		case 1:
			src = 24000000;
			break;
		case 2:
		case 3:
			src = ccm_get_pll1_cpux_clk();
			break;
	}
	return src / axidiv;
#endif
}

u32 ccm_get_ahb1_clk(void)
{

#ifdef FPGA_PLATFORM
	return AHB1CLK;
#else
	u32 src;
	u32 rval;
	u32 ahb1pdiv;
	u32 ahb1div;
	rval = readl(CCM_AHB1_APB1_CTRL);
	src = 0x3 & (rval >> 12);
	ahb1pdiv = (0x3 & (rval >> 6)) + 1;
	ahb1div = 1 << (0x3 & (rval >> 4));
	switch (src) {
		case 0:
			src = 32000;
			break;
		case 1:
			src = 24000000;
			break;
		case 2:
			src = ccm_get_axi_clk();
			break;
		case 3:
			src = ccm_get_pll6_dev_clk() / ahb1pdiv;
			break;
	}
	return src / ahb1div;
#endif
}

u32 ccm_get_apb1_clk(void)
{
#ifdef FPGA_PLATFORM
	return APB1CLK;
#else
	u32 rval;
	u32 apb1div;

	rval = readl(CCM_AHB1_APB1_CTRL);
	apb1div = 0x3 & (rval >> 8);
	if (apb1div == 0)
		apb1div = 1;
	apb1div = 1 << apb1div;
	return ccm_get_ahb1_clk() / apb1div;
#endif
}

u32 ccm_get_apb2_clk(void)
{
#ifdef FPGA_PLATFORM
	return APB2CLK;
#else
	u32 src;
	u32 rval;
	u32 n, m;

	rval = readl(CCM_APB2_CLK_CTRL);
	src = 0x3 & (rval >> 24);
	n = 0x3 & (rval >> 16);
	n = 1 << n;
	m = (0x1f & (rval >> 0)) + 1;
	switch (src) {
		case 0:
			src = 32000;
			break;
		case 1:
			src = 24000000;
			break;
		case 2:
		case 3:
			src = ccm_get_pll6_dev_clk();
			break;
	}
	return src / n / m;
#endif
}
/*
s32 ccm_module_clk_gate_test(u32 clkid, u32 addr, u32 wr_val, u32 expt_val)
{
	u32 rval;

	ccm_module_reset(clkid);
	ccm_clock_disable(clkid);

	writel(wr_val, addr);
	rval = readl(addr);
	if (rval == wr_val)
		return -1;

	ccm_clock_enable(clkid);
	writel(wr_val, addr);
	rval = readl(addr);
	if (rval != expt_val)
		return -1;

	return 0;
}

s32 ccm_module_reset_test(u32 clkid, u32 addr, u32 default_value)
{
	u32 rval;

	ccm_module_reset(clkid);
	ccm_clock_enable(clkid);
	rval = readl(addr);
	if (rval == 0 || rval != default_value)
		return -1;

	return 0;
}
*/

