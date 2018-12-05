/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * sunxi_host0_2_v4p1.c
 * Description: MMC  driver for  mmc controller operations of sun50iw1p1
 * Author: ZhengLei
 * Date: 2015/7/29 15:50:00
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/timer.h>
#include <malloc.h>
#include <mmc.h>
#include "../mmc_def.h"
#include "../sunxi_mmc.h"
#include "sunxi_host0_2_v4p1.h"

#undef readl
#define  readl(a)   *(volatile uint *)(ulong)(a)

#undef writel
#define  writel(v, c) *(volatile uint *)(ulong)(c) = (v)

#define BIT(x)				(1<<(x))
/* Struct for Intrrrupt Information */
#define SDXC_RespErr		BIT(1) //0x2
#define SDXC_CmdDone		BIT(2) //0x4
#define SDXC_DataOver		BIT(3) //0x8
#define SDXC_TxDataReq		BIT(4) //0x10
#define SDXC_RxDataReq		BIT(5) //0x20
#define SDXC_RespCRCErr		BIT(6) //0x40
#define SDXC_DataCRCErr		BIT(7) //0x80
#define SDXC_RespTimeout	BIT(8) //0x100
#define SDXC_ACKRcv		BIT(8)	//0x100
#define SDXC_DataTimeout	BIT(9)	//0x200
#define SDXC_BootStart		BIT(9)	//0x200
#define SDXC_DataStarve		BIT(10) //0x400
#define SDXC_VolChgDone		BIT(10) //0x400
#define SDXC_FIFORunErr		BIT(11) //0x800
#define SDXC_HardWLocked	BIT(12)	//0x1000
#define SDXC_StartBitErr		BIT(13)	//0x2000
#define SDXC_AutoCMDDone	BIT(14)	//0x4000
#define SDXC_EndBitErr		BIT(15)	//0x8000
#define SDXC_SDIOInt		BIT(16)	//0x10000
#define SDXC_CardInsert		BIT(30) //0x40000000
#define SDXC_CardRemove		BIT(31) //0x80000000
#define SDXC_IntErrBit		(SDXC_RespErr | SDXC_RespCRCErr | SDXC_DataCRCErr \
				| SDXC_RespTimeout | SDXC_DataTimeout | SDXC_FIFORunErr \
				| SDXC_HardWLocked | SDXC_StartBitErr | SDXC_EndBitErr)  //0xbfc2


extern int mmc_clk_io_onoff(int sdc_no, int onoff, int reset_clk);


void mmc_dump_errinfo(struct sunxi_mmc_host* smc_host, struct mmc_cmd *cmd)
{
	MMCMSG(smc_host->mmc, "smc %d err, cmd %d, %s%s%s%s%s%s%s%s%s%s%s\n",
		smc_host->mmc_no, cmd? cmd->cmdidx: -1,
		smc_host->raw_int_bak & SDXC_RespErr     ? " RE"     : "",
		smc_host->raw_int_bak & SDXC_RespCRCErr  ? " RCE"    : "",
		smc_host->raw_int_bak & SDXC_DataCRCErr  ? " DCE"    : "",
		smc_host->raw_int_bak & SDXC_RespTimeout ? " RTO"    : "",
		smc_host->raw_int_bak & SDXC_DataTimeout ? " DTO"    : "",
		smc_host->raw_int_bak & SDXC_DataStarve  ? " DS"     : "",
		smc_host->raw_int_bak & SDXC_FIFORunErr  ? " FE"     : "",
		smc_host->raw_int_bak & SDXC_HardWLocked ? " HL"     : "",
		smc_host->raw_int_bak & SDXC_StartBitErr ? " SBE"    : "",
		smc_host->raw_int_bak & SDXC_EndBitErr   ? " EBE"    : "",
		smc_host->raw_int_bak ==0  ? " STO"    : ""
		);
}

static int mmc_update_clk(struct sunxi_mmc_host* mmchost)
{
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	unsigned int cmd;
	unsigned timeout = 1000;

	writel(readl(&reg->clkcr)|(0x1<<31), &reg->clkcr);

	cmd = (1U << 31) | (1 << 21) | (1 << 13);
  	writel(cmd, &reg->cmd);
	while((readl(&reg->cmd)&0x80000000) && --timeout){
		__msdelay(1);
	}
	if (!timeout){
		MMCINFO("mmc %d update clk failed\n",mmchost->mmc_no);
		dumphex32("mmc", (char*)reg, 0x100);
		return -1;
	}
	writel(readl(&reg->clkcr) & (~(0x1<<31)), &reg->clkcr);
	writel(readl(&reg->rint), &reg->rint);
	return 0;
}

static int mmc_update_phase(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;

	if (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_1)
	{
		MMCDBG("mmc re-update_phase\n");
		return mmc_update_clk(mmchost);
	}

	return 0;
}

#ifndef FPGA_PLATFORM
static int mmc_set_mclk(struct sunxi_mmc_host* mmchost, u32 clk_hz)
{
	unsigned n, m, div, src, sclk_hz = 0;
	unsigned rval;

	MMCDBG("%s: mod_clk %d\n", __FUNCTION__, clk_hz);

	if (clk_hz <= 4000000) { //400000
		src = 0;
		sclk_hz = 24000000;
	} else {
		src = 1;
		sclk_hz = sunxi_clock_get_pll6()*2*1000000; /*use 2x pll-per0 clock */
	}

	div = (2 * sclk_hz + clk_hz) / (2 * clk_hz);
	div = (div==0) ? 1 : div;
	if (div > 128) {
		m = 1;
		n = 0;
		MMCINFO("%s: source clock is too high, clk %d, src %d!!!\n",
			__FUNCTION__, clk_hz, sclk_hz);
	} else if (div > 64) {
		n = 3;
		m = div >> 3;
	} else if (div > 32) {
		n = 2;
		m = div >> 2;
	} else if (div > 16) {
		n = 1;
		m = div >> 1;
	} else {
		n = 0;
		m = div;
	}

	//rval = (1U << 31) | (src << 24) | (n << 16) | (m - 1);
	rval = (src << 24) | (n << 16) | (m - 1);
	writel(rval, mmchost->mclkbase);

	return 0;
}

static unsigned mmc_get_mclk(struct sunxi_mmc_host* mmchost)
{
	unsigned n, m, src, sclk_hz = 0;
	unsigned rval = readl(mmchost->mclkbase);

	m = rval & 0xf;
	n = (rval>>16) & 0x3;
	src = (rval>>24) & 0x3;

	if (src == 0)
		sclk_hz = 24000000;
	else if (src == 1)
		sclk_hz = sunxi_clock_get_pll6()*2*1000000; /* use 2x pll6 */
	else if (src == 2) {
		/*todo*/
	} else {
		MMCINFO("%s: wrong clock source %d\n",__func__, src);
	}

	return (sclk_hz / (1<<n) / (m+1) );
}
static unsigned mmc_config_delay(struct sunxi_mmc_host* mmchost)
{
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	unsigned rval = 0;
	unsigned mode = mmchost->timing_mode;
	unsigned spd_md, freq;
	u8 odly, sdly, dsdly=0;

	if (mode == SUNXI_MMC_TIMING_MODE_1)
	{
		spd_md = mmchost->tm1.cur_spd_md;
		freq = mmchost->tm1.cur_freq;
		if (mmchost->tm1.odly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			odly = mmchost->tm1.odly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			odly = mmchost->tm1.def_odly[spd_md*MAX_CLK_FREQ_NUM+freq];
		if (mmchost->tm1.sdly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			sdly = mmchost->tm1.sdly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			sdly = mmchost->tm1.def_sdly[spd_md*MAX_CLK_FREQ_NUM+freq];
		mmchost->tm1.cur_odly = odly;
		mmchost->tm1.cur_sdly = sdly;

		MMCDBG("%s: odly: %d   sldy: %d\n", __FUNCTION__, odly, sdly);
		rval = readl(&reg->drv_dl);
		rval &= (~(0x3<<16));
		rval |= (((odly&0x1)<<16) | ((odly&0x1)<<17));
		writel(rval, &reg->drv_dl);

		rval = readl(&reg->ntsr);
		rval &= (~(0x3<<4));
		rval |= ((sdly&0x3)<<4);
		writel(rval, &reg->ntsr);
	}
	else if (mode == SUNXI_MMC_TIMING_MODE_3)
	{
		spd_md = mmchost->tm3.cur_spd_md;
		freq = mmchost->tm3.cur_freq;
		if (mmchost->tm3.odly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			odly = mmchost->tm3.odly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			odly = mmchost->tm3.def_odly[spd_md*MAX_CLK_FREQ_NUM+freq];
		if (mmchost->tm3.sdly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			sdly = mmchost->tm3.sdly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			sdly = mmchost->tm3.def_sdly[spd_md*MAX_CLK_FREQ_NUM+freq];
		mmchost->tm3.cur_odly = odly;
		mmchost->tm3.cur_sdly = sdly;

		MMCDBG("%s: odly: %d   sldy: %d\n", __FUNCTION__, odly, sdly);
		rval = readl(&reg->drv_dl);
		rval &= (~(0x3<<16));
		rval |= (((odly&0x1)<<16) | ((odly&0x1)<<17));
		writel(rval, &reg->drv_dl);

		rval = readl(&reg->samp_dl);
		rval &= (~SDXC_CfgDly);
		rval |= ((sdly&SDXC_CfgDly) | SDXC_EnableDly);
		writel(rval, &reg->samp_dl);
	}
	else if (mode == SUNXI_MMC_TIMING_MODE_4)
	{
		spd_md = mmchost->tm4.cur_spd_md;
		freq = mmchost->tm4.cur_freq;

		if (mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			sdly = mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			sdly = mmchost->tm4.def_sdly[spd_md*MAX_CLK_FREQ_NUM+freq];

		if (mmchost->tm4.odly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			odly = mmchost->tm4.odly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			odly = mmchost->tm4.def_odly[spd_md*MAX_CLK_FREQ_NUM+freq];

		mmchost->tm4.cur_odly = odly;
		mmchost->tm4.cur_sdly = sdly;

		rval = readl(&reg->drv_dl);
		rval &= (~(0x3<<16));
		rval |= (((odly&0x1)<<16) | ((odly&0x1)<<17));
		writel(rval, &reg->drv_dl);

		rval = readl(&reg->samp_dl);
		rval &= (~SDXC_CfgDly);
		rval |= ((sdly&SDXC_CfgDly) | SDXC_EnableDly);
		writel(rval, &reg->samp_dl);

		if (spd_md == HS400)
		{
			if (mmchost->tm4.dsdly[freq] != 0xFF)
				dsdly = mmchost->tm4.dsdly[freq];
			else
				dsdly = mmchost->tm4.def_dsdly[freq];
			mmchost->tm4.cur_dsdly = dsdly;

			rval = readl(&reg->ds_dl);
			rval &= (~SDXC_CfgDly);
			rval |= ((dsdly&SDXC_CfgDly) | SDXC_EnableDly);
			#ifdef FPGA_PLATFORM
			rval &= (~0x7);
			#endif
			writel(rval, &reg->ds_dl);
		}
		MMCDBG("%s: spd_md:%d, freq:%d, odly: %d; sdly: %d; dsdly: %d\n", __FUNCTION__, spd_md, freq, odly, sdly, dsdly);
	}

	return 0;
}
#endif /*FPGA_PLATFORM*/

static int mmc_config_clock_modex(struct sunxi_mmc_host* mmchost, unsigned clk)
{
	unsigned rval = 0;
	struct mmc *mmc = mmchost->mmc;
	unsigned mode = mmchost->timing_mode;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;

#ifndef FPGA_PLATFORM
	unsigned freq_id;

	/* disable mclk */
	writel(0x0, mmchost->mclkbase);
	MMCDBG("mmc %d mclkbase 0x%x\n", mmchost->mmc_no, readl(mmchost->mclkbase));

	/* enable timing mode 1 */
	if (mode == SUNXI_MMC_TIMING_MODE_1) {
		rval = readl(&reg->ntsr);
		rval |= (1<<31);
		writel(rval, &reg->ntsr);
		MMCDBG("mmc %d rntsr 0x%x\n", mmchost->mmc_no, rval);
	} else
		writel(0x0, &reg->ntsr);

	/* configure clock */
	if ((mode == SUNXI_MMC_TIMING_MODE_1)) {
		if (mmc->speed_mode == HSDDR52_DDR50)
			mmchost->mod_clk = clk * 4;
		else
			mmchost->mod_clk = clk * 2;
	} else if (mode == SUNXI_MMC_TIMING_MODE_4) {
		if ((mmc->speed_mode == HSDDR52_DDR50)
			&& (mmc->bus_width == 8))
			mmchost->mod_clk = clk * 4; /* 4xclk: DDR8(HS) */
		else
			mmchost->mod_clk = clk * 2; /* 2xclk: SDR 1/4/8; DDR4(HS); DDR8(HS400)  */
	}

	mmc_set_mclk(mmchost, mmchost->mod_clk);

	/* get mclk */
	if ((mode == SUNXI_MMC_TIMING_MODE_1) || (mode == SUNXI_MMC_TIMING_MODE_3))	{
		if (mmc->speed_mode == HSDDR52_DDR50)
			mmc->clock = mmc_get_mclk(mmchost) / 4;
		else
			mmc->clock = mmc_get_mclk(mmchost) / 2;
	} else if (mode == SUNXI_MMC_TIMING_MODE_4) {
		if ((mmc->speed_mode == HSDDR52_DDR50)
			&& (mmc->bus_width == 8))
			mmc->clock = mmc_get_mclk(mmchost) / 4;
		else
			mmc->clock = mmc_get_mclk(mmchost) / 2;
	}
	mmchost->clock = mmc->clock; /* bankup current clock frequency at host */
	MMCDBG("get round card clk %d, mod_clk %d\n", mmc->clock, mmchost->mod_clk);

	/* re-enable mclk */
	writel(readl(mmchost->mclkbase)|(1<<31),mmchost->mclkbase);
	MMCDBG("mmc %d mclkbase 0x%x\n", mmchost->mmc_no, readl(mmchost->mclkbase));

	/*
	 * CLKCREG[7:0]: divider
	 * CLKCREG[16]:  on/off
	 * CLKCREG[17]:  power save
	 */
	rval = readl(&reg->clkcr);
	rval &= ~(0xFF);
	if ((mode == SUNXI_MMC_TIMING_MODE_1)||(mode == SUNXI_MMC_TIMING_MODE_3)) {
		if (mmc->speed_mode == HSDDR52_DDR50)
			rval |= 0x1;
	} else if (mode == SUNXI_MMC_TIMING_MODE_4) {
		if ((mmc->speed_mode == HSDDR52_DDR50) && (mmc->bus_width == 8))
			rval |= 0x1;
	}
	writel(rval, &reg->clkcr);

	if (mmc_update_clk(mmchost))
		return -1;

	/* configure delay for current frequency and speed mode */
	if (clk <= 400000)
		freq_id = CLK_400K;
	else if (clk <= 26000000)
		freq_id = CLK_25M;
	else if (clk <= 52000000)
		freq_id = CLK_50M;
	else if (clk <= 100000000)
		freq_id = CLK_100M;
	else if (clk <= 150000000)
		freq_id = CLK_150M;
	else if (clk <= 200000000)
		freq_id = CLK_200M;
	else
		freq_id = CLK_25M;

	if (mode == SUNXI_MMC_TIMING_MODE_1) {
		mmchost->tm1.cur_spd_md = mmchost->mmc->speed_mode;
		mmchost->tm1.cur_freq = freq_id;
	} else if (mode == SUNXI_MMC_TIMING_MODE_3) {
		mmchost->tm3.cur_spd_md = mmchost->mmc->speed_mode;
		mmchost->tm3.cur_freq = freq_id;
	} else if (mode == SUNXI_MMC_TIMING_MODE_4) {
		mmchost->tm4.cur_spd_md = mmchost->mmc->speed_mode;
		mmchost->tm4.cur_freq = freq_id;
	}

	mmc_config_delay(mmchost);

#else
	unsigned div, sclk= 24000000;
	unsigned clk_2x = 0;

	if (mode == SUNXI_MMC_TIMING_MODE_1)
	{
		div = (2 * sclk + clk) / (2 * clk);
		rval = readl(&reg->clkcr) & (~0xff);
		if (mmc->io_mode == MMC_MODE_DDR_52MHz)
			rval |= 0x1;
		else
			rval |= div >> 1;
		writel(rval, &reg->clkcr);

		rval = readl(&reg->ntsr);
		rval |= (1<<31);
		writel(rval, &reg->ntsr);
		MMCINFO("mmc %d ntsr 0x%x, ckcr 0x%x\n", mmchost->mmc_no,
			readl(&reg->ntsr), readl(&reg->clkcr));
	}
	if ((mode == SUNXI_MMC_TIMING_MODE_3) || (mode == SUNXI_MMC_TIMING_MODE_4))
	{
		if (mode == SUNXI_MMC_TIMING_MODE_3) {
			if (mmc->io_mode == MMC_MODE_DDR_52MHz)
				clk_2x = clk << 2; //4xclk
			else
				clk_2x = clk << 1; //2xclk
		} else if (mode == SUNXI_MMC_TIMING_MODE_4) {
			if (mmc->io_mode == MMC_MODE_DDR_52MHz && mmc->bus_width == 8)
				clk_2x = clk << 2; //4xclk: DDR8(HS)
			else
				clk_2x = clk << 1; //2xclk: SDR 1/4/8; DDR4(HS); DDR8(HS400)
		}

		div = (2 * sclk + clk_2x) / (2 * clk_2x);
		rval = readl(&reg->clkcr) & (~0xff);
		if (mmc->io_mode == MMC_MODE_DDR_52MHz)
			rval |= 0x1;
		else
			rval |= div >> 1;
		writel(rval, &reg->clkcr);


	}

#if 0
{
	unsigned freq=0;

	/* configure delay for current frequency and speed mode */
	if (clk <= 400000)
		freq = CLK_400K;
	else if (clk <= 26000000)
		freq = CLK_25M;
	else if (clk <= 52000000)
		freq = CLK_50M;
	else if (clk <= 100000000)
		freq = CLK_100M;
	else if (clk <= 150000000)
		freq = CLK_150M;
	else if (clk <= 200000000)
		freq = CLK_200M;
	else
		freq = CLK_25M;

	if (mode == SUNXI_MMC_TIMING_MODE_1) {
		mmchost->tm1.cur_spd_md = mmchost->mmc->speed_mode;
		mmchost->tm1.cur_freq = freq;
	} else if (mode == SUNXI_MMC_TIMING_MODE_3) {
		mmchost->tm3.cur_spd_md = mmchost->mmc->speed_mode;;
		mmchost->tm3.cur_freq = freq;
	} else if (mode == SUNXI_MMC_TIMING_MODE_4) {
		mmchost->tm4.cur_spd_md = mmchost->mmc->speed_mode;;
		mmchost->tm4.cur_freq = freq;
	}

	mmc_config_delay(mmchost);
}
#endif

#endif

	//dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
	//dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
	//dumphex32("mmc", (char*)reg, 0x100);
	return 0;
}

static int mmc_config_clock(struct sunxi_mmc_host* mmchost, unsigned clk)
{
	//struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host* )mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	unsigned rval = 0;

	/* disable card clock */
	rval = readl(&reg->clkcr);
	rval &= ~(1 << 16);
	writel(rval, &reg->clkcr);
	if(mmc_update_clk(mmchost))
		return -1;

	if ((mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_1)
		|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_3)
		|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_4) )

		mmc_config_clock_modex(mmchost, clk);

	else {
		MMCINFO("mmc %d wrong timing mode: 0x%x\n",
			mmchost->mmc_no, mmchost->timing_mode);
		return -1;
	}

	/* Re-enable card clock */
	rval = readl(&reg->clkcr);
	rval |=  (0x1 << 16); //(3 << 16);
	writel(rval, &reg->clkcr);
	if(mmc_update_clk(mmchost)){
		MMCINFO("mmc %d re-enable clock failed\n",mmchost->mmc_no);
		return -1;
	}

	return 0;
}
static void mmc_ddr_mode_onoff(struct mmc *mmc, int on)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	u32 rval = 0;

	rval = readl(&reg->gctrl);
	rval &= (~(1U << 10));

	if (on) {
		rval |= (1U << 10);
		writel(rval, &reg->gctrl);
		MMCDBG("set %d rgctrl 0x%x to enable ddr mode\n", mmchost->mmc_no, readl(&reg->gctrl));
	} else {
		writel(rval, &reg->gctrl);
		MMCDBG("set %d rgctrl 0x%x to disable ddr mode\n", mmchost->mmc_no, readl(&reg->gctrl));
	}
}

static void mmc_hs400_mode_onoff(struct mmc *mmc, int on)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	u32 rval = 0;

	if (mmchost->mmc_no != 2) {
		return ;
	}

	rval = readl(&reg->dsbd);
	rval &= (~(1 << 31));

	if (on) {
		rval |= (1 << 31);
		writel(rval, &reg->dsbd);
		MMCDBG("set %d dsbd 0x%x to enable hs400 mode\n", mmchost->mmc_no, readl(&reg->dsbd));
	} else {
		writel(rval, &reg->dsbd);
		MMCDBG("set %d dsbd 0x%x to disable hs400 mode\n", mmchost->mmc_no, readl(&reg->dsbd));
	}
}

static int mmc_calibrate_delay_unit(struct sunxi_mmc_host* mmchost)
{
 #ifndef FPGA_PLATFORM

	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	unsigned rval = 0;
	unsigned clk[3] = {50*1000*1000, 100*1000*1000, 200*1000*1000};
	unsigned period[3] = {10*1000, 5*1000, 2500}; //ps, module clk is 2xclk at init phase.
	unsigned result = 0;
	int i = 0;

	MMCDBG("start %s, don't access device...\n", __FUNCTION__);

	for (i=0; i<3; i++)
	{
		MMCINFO("%d MHz...\n", clk[i]/1000/1000);
		/* close card clock */
		rval = readl(&reg->clkcr);
		rval &= ~(1 << 16);
		writel(rval, &reg->clkcr);
		if(mmc_update_clk(mmchost))
			return -1;

		/* set card clock to 100MHz */
		if ((mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_1)
			|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_3)
			|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_4) )

			mmc_config_clock_modex(mmchost, clk[i]);
		else {
			MMCINFO("%s: mmc %d wrong timing mode: 0x%x\n",
				__FUNCTION__, mmchost->mmc_no, mmchost->timing_mode);
			return -1;
		}

		/* start carlibrate delay unit */
		writel(0xA0, &reg->samp_dl);
		writel(0x0, &reg->samp_dl);
		rval = SDXC_StartCal;
		writel(rval, &reg->samp_dl);
		writel(0x0, &reg->samp_dl);
		while (!(readl(&reg->samp_dl) & SDXC_CalDone));

		if (mmchost->mmc_no == 2) {
			writel(0xA0, &reg->ds_dl);
			writel(0x0, &reg->ds_dl);
			rval = SDXC_StartCal;
			writel(rval, &reg->ds_dl);
			writel(0x0, &reg->ds_dl);
			while (!(readl(&reg->ds_dl) & SDXC_CalDone));
		}

		/* update result */
		rval = readl(&reg->samp_dl);
		result = (rval & SDXC_CalDly) >> 8;
		MMCDBG("samp_dl result: 0x%x\n", result);
		if (result) {
			rval = period[i] / result;
			mmchost->tm3.sdly_unit_ps = rval;
			mmchost->tm3.dly_calibrate_done = 1;
			mmchost->tm4.sdly_unit_ps = rval;
			MMCINFO("sample: %d - %d(ps)\n", result, mmchost->tm3.sdly_unit_ps);
		} else {
			MMCINFO("%s: cal sample delay fail\n", __FUNCTION__);
		}
		if (mmchost->mmc_no == 2) {
			rval = readl(&reg->ds_dl);
			result = (rval & SDXC_CalDly) >> 8;
			MMCDBG("ds_dl result: 0x%x\n", result);
			if (result) {
				rval = period[i] / result;
				mmchost->tm4.dsdly_unit_ps = rval;
				mmchost->tm4.dly_calibrate_done = 1;
				MMCINFO("ds: %d - %d(ps)\n", result, mmchost->tm4.dsdly_unit_ps);
			} else {
				MMCINFO("%s: cal data strobe delay fail\n", __FUNCTION__);
			}
		}

	}

#endif	/* FPGA_PLATFORM */
	return 0;
}
static int mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	/* Reset controller */
	writel(0x7, &reg->gctrl); /*0x40000007*/
	while(readl(&reg->gctrl)&0x7);
	/* release eMMC reset signal */
	writel(1, &reg->hwrst);
	writel(0, &reg->hwrst);
	udelay(1000);
	writel(1, &reg->hwrst);
	udelay(1000);
#if 1
#define  SMC_DATA_TIMEOUT     0xffffffU
#define  SMC_RESP_TIMEOUT     0xff
#else
#define  SMC_DATA_TIMEOUT     0x1ffffU
#define  SMC_RESP_TIMEOUT     0x2
#endif
	writel((SMC_DATA_TIMEOUT<<8)|SMC_RESP_TIMEOUT, &reg->timeout); //Set Data & Response Timeout Value

	writel((512<<16)|(1U<<2)|(1U<<0), &reg->thldc);
	writel(3, &reg->csdc);
	writel(0xdeb, &reg->dbgc);

	mmc_calibrate_delay_unit(mmchost);
	return 0;
}

static void mmc_set_ios(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;

	MMCDBG("mmc %d ios: bus: %d, clock: %d, speed mode: %d\n", \
		mmchost->mmc_no,mmc->bus_width, mmc->clock, mmc->speed_mode);
	/* change clock */
	if (mmc->clock && mmc_config_clock((struct sunxi_mmc_host*)mmc->priv, mmc->clock)) {
		MMCINFO("[mmc]: mmc %d update clock failed\n",mmchost->mmc_no);
		mmchost->fatal_err = 1;
		return;
	}

	/* for card2, if clock frequency is greater than 100MHz, increase gpio dirver strength */
#if 0
	if (mmc->clock >= 100000000) {
		writel(0xFFFFFFFF, 0x01c2085C);
		writel(0xFFFFFFFF, 0x01c20860);
	} else {
		writel(0x55555555, 0x01c2085C);
		writel(0x55555555, 0x01c20860);
	}
#endif

	/* Change bus width */
	if (mmc->bus_width == 8)
		writel(2, &reg->width);
	else if (mmc->bus_width == 4)
		writel(1, &reg->width);
	else
		writel(0, &reg->width);
	MMCDBG("host bus width register 0x%x\n", readl(&reg->width));

	/* set speed mode */
	if (mmc->speed_mode == HSDDR52_DDR50) {
		mmc_ddr_mode_onoff(mmc, 1);
		mmc_hs400_mode_onoff(mmc, 0);
	} else if (mmc->speed_mode == HS400) {
		mmc_ddr_mode_onoff(mmc, 0);
		mmc_hs400_mode_onoff(mmc, 1);
	} else {
		mmc_ddr_mode_onoff(mmc, 0);
		mmc_hs400_mode_onoff(mmc, 0);
	}
}

static int mmc_save_regs(struct sunxi_mmc_host* mmchost)
{
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	struct mmc_reg_v4p1 *reg_bak = (struct mmc_reg_v4p1 *)mmchost->reg_bak;

	reg_bak->gctrl     = readl(&reg->gctrl  );
	reg_bak->clkcr     = readl(&reg->clkcr  );
	reg_bak->timeout   = readl(&reg->timeout);
	reg_bak->width     = readl(&reg->width  );
	reg_bak->imask     = readl(&reg->imask  );
	reg_bak->ftrglevel = readl(&reg->ftrglevel);
	reg_bak->dbgc      = readl(&reg->dbgc   );
	reg_bak->csdc      = readl(&reg->csdc   );
	reg_bak->ntsr      = readl(&reg->ntsr   );
	reg_bak->hwrst     = readl(&reg->hwrst  );
	reg_bak->dmac      = readl(&reg->dmac   );
	reg_bak->idie      = readl(&reg->idie   );
	reg_bak->thldc     = readl(&reg->thldc  );
	reg_bak->dsbd      = readl(&reg->dsbd   );
	reg_bak->drv_dl    = readl(&reg->drv_dl );
	reg_bak->samp_dl   = readl(&reg->samp_dl);
	reg_bak->ds_dl     = readl(&reg->ds_dl  );

	return 0;
}
static int mmc_restore_regs(struct sunxi_mmc_host* mmchost)
{	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	struct mmc_reg_v4p1 *reg_bak = (struct mmc_reg_v4p1 *)mmchost->reg_bak;

	writel(reg_bak->gctrl     , &reg->gctrl  );
	writel(reg_bak->clkcr     , &reg->clkcr  );
	writel(reg_bak->timeout   , &reg->timeout);
	writel(reg_bak->width     , &reg->width  );
	writel(reg_bak->imask     , &reg->imask  );
	writel(reg_bak->ftrglevel , &reg->ftrglevel);
	if (reg_bak->dbgc)
		writel(0xdeb, &reg->dbgc);
	writel(reg_bak->csdc      , &reg->csdc   );
	writel(reg_bak->ntsr      , &reg->ntsr   );
	writel(reg_bak->hwrst     , &reg->hwrst  );
	writel(reg_bak->dmac      , &reg->dmac   );
	writel(reg_bak->idie      , &reg->idie   );
	writel(reg_bak->thldc     , &reg->thldc  );
	writel(reg_bak->dsbd      , &reg->dsbd   );
	writel(reg_bak->drv_dl    , &reg->drv_dl );
	writel(reg_bak->samp_dl   , &reg->samp_dl);
	writel(reg_bak->ds_dl     , &reg->ds_dl  );

	return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	unsigned i;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned *buff;
	unsigned timeout = 1000;

	if (data->flags & MMC_DATA_READ) {
		buff = (unsigned int *)data->dest;
		for (i=0; i<(byte_cnt>>2); i++) {
			while(--timeout && (readl(&reg->status)&(1 << 2))){
				__msdelay(1);
			}
			if (timeout <= 0)
				goto out;
			buff[i] = readl(mmchost->database);
			timeout = 1000;
		}
	} else {
		buff = (unsigned int *)data->src;
		for (i=0; i<(byte_cnt>>2); i++) {
			while(--timeout && (readl(&reg->status)&(1 << 3))){
				__msdelay(1);
			}
			if (timeout <= 0)
				goto out;
			writel(buff[i], mmchost->database);
			timeout = 1000;
		}
	}

out:
	if (timeout <= 0){
		MMCINFO("transfer by cpu failed\n");
		return -1;
	}

	return 0;
}
static int mmc_trans_data_by_dma(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	struct mmc_des_v4p1 *pdes =(struct mmc_des_v4p1 *)mmchost->pdes;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned char *buff;
	unsigned des_idx = 0;
	unsigned buff_frag_num = 0;
	unsigned remain;
	unsigned i, rval;

	buff = data->flags & MMC_DATA_READ ?
			(unsigned char *)data->dest : (unsigned char *)data->src;
	buff_frag_num = byte_cnt >> SDXC_DES_NUM_SHIFT;
	remain = byte_cnt & (SDXC_DES_BUFFER_MAX_LEN-1);
	if (remain)
		buff_frag_num ++;
	else
		remain = SDXC_DES_BUFFER_MAX_LEN;

	flush_cache((unsigned long)buff, (unsigned long)byte_cnt);
	for (i=0; i < buff_frag_num; i++, des_idx++) {
		memset((void*)&pdes[des_idx], 0, sizeof(struct mmc_des_v4p1));
		pdes[des_idx].des_chain = 1;
		pdes[des_idx].own = 1;
		pdes[des_idx].dic = 1;
		if (buff_frag_num > 1 && i != buff_frag_num-1)
			pdes[des_idx].data_buf1_sz = SDXC_DES_BUFFER_MAX_LEN;
		else
			pdes[des_idx].data_buf1_sz = remain;

		pdes[des_idx].buf_addr_ptr1 = (ulong)buff + i * SDXC_DES_BUFFER_MAX_LEN;
		if (i==0)
			pdes[des_idx].first_des = 1;

		if (i == buff_frag_num-1) {
			pdes[des_idx].dic = 0;
			pdes[des_idx].last_des = 1;
			pdes[des_idx].end_of_ring = 1;
			pdes[des_idx].buf_addr_ptr2 = 0;
		} else {
			pdes[des_idx].buf_addr_ptr2 = (ulong)&pdes[des_idx+1];
		}
		MMCDBG("frag %d, remain %d, des[%d](%08x): "
			"[0] = %08x, [1] = %08x, [2] = %08x, [3] = %08x\n",
			i, remain, des_idx, (u32)&pdes[des_idx],
			(u32)((u32*)&pdes[des_idx])[0], (u32)((u32*)&pdes[des_idx])[1],
			(u32)((u32*)&pdes[des_idx])[2], (u32)((u32*)&pdes[des_idx])[3]);
	}
	flush_cache((unsigned long)pdes, sizeof(struct mmc_des_v4p1) * (des_idx+1));
	__asm("DSB");
	__asm("ISB");

	/*
	 * GCTRLREG
	 * GCTRL[2]	: DMA reset
	 * GCTRL[5]	: DMA enable
	 *
	 * IDMACREG
	 * IDMAC[0]	: IDMA soft reset
	 * IDMAC[1]	: IDMA fix burst flag
	 * IDMAC[7]	: IDMA on
	 *
	 * IDIECREG
	 * IDIE[0]	: IDMA transmit interrupt flag
	 * IDIE[1]	: IDMA receive interrupt flag
	 */
	rval = readl(&reg->gctrl);
	writel(rval|(1 << 5)|(1 << 2), &reg->gctrl);	/* dma enable */
	writel((1 << 0), &reg->dmac); /* idma reset */
	while(readl(&reg->dmac)& 0x1) {}; /* wait idma reset done */
	writel((1 << 1) | (1 << 7), &reg->dmac); /* idma on */
	rval = readl(&reg->idie) & (~3);
	if (data->flags & MMC_DATA_WRITE)
		rval |= (1 << 0);
	else
		rval |= (1 << 1);
	writel(rval, &reg->idie);
	writel((unsigned long)pdes, &reg->dlba);
	if (mmchost->mmc_no == 2)
		writel((3U<<28)|(15U<<16)|240, &reg->ftrglevel); /* burst-16, rx/tx trigger level=15/240 */
	else
		writel((2U<<28)|(7U<<16)|248, &reg->ftrglevel); /* burst-8, rx/tx trigger level=7/248 */

	return 0;
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct mmc_reg_v4p1 *reg = (struct mmc_reg_v4p1 *)mmchost->reg;
	unsigned int cmdval = 0x80000000;
	signed int timeout = 0;
	int error = 0;
	unsigned int status = 0;
	unsigned int usedma = 0;
	unsigned int bytecnt = 0;

	if (mmchost->fatal_err) {
		MMCINFO("mmc %d Found fatal err,so no send cmd\n",mmchost->mmc_no);
		return -1;
	}

	if (cmd->resp_type & MMC_RSP_BUSY)
		MMCDBG("mmc %d mmc cmd %d check rsp busy\n", mmchost->mmc_no,cmd->cmdidx);
	if ((cmd->cmdidx == 12)&&!(cmd->flags&MMC_CMD_MANUAL)){
		MMCDBG("note we don't send stop cmd,only check busy here\n");
		timeout = 500*1000;
		do {
			status = readl(&reg->status);
			if (!timeout--) {
				error = -1;
				MMCINFO("mmc %d cmd12 busy timeout\n",mmchost->mmc_no);
				goto out;
			}
			__usdelay(1);
		} while (status & (1 << 9));
		return 0;
	}
	/*
	 * CMDREG
	 * CMD[5:0]	: Command index
	 * CMD[6]	: Has response
	 * CMD[7]	: Long response
	 * CMD[8]	: Check response CRC
	 * CMD[9]	: Has data
	 * CMD[10]	: Write
	 * CMD[11]	: Steam mode
	 * CMD[12]	: Auto stop
	 * CMD[13]	: Wait previous over
	 * CMD[14]	: About cmd
	 * CMD[15]	: Send initialization
	 * CMD[21]	: Update clock
	 * CMD[31]	: Load cmd
	 */
	if (!cmd->cmdidx)
		cmdval |= (1 << 15);
	if (cmd->resp_type & MMC_RSP_PRESENT)
		cmdval |= (1 << 6);
	if (cmd->resp_type & MMC_RSP_136)
		cmdval |= (1 << 7);
	if (cmd->resp_type & MMC_RSP_CRC)
		cmdval |= (1 << 8);
	if (data) {
		if ((ulong)data->dest & 0x3) {
			MMCINFO("mmc %d dest is not 4 byte align: 0x%08lx\n",mmchost->mmc_no, (ulong)data->dest);
			error = -1;
			goto out;
		}

		cmdval |= (1 << 9) | (1 << 13);
		if (data->flags & MMC_DATA_WRITE)
			cmdval |= (1 << 10);
		if (data->blocks > 1&&!(cmd->flags&MMC_CMD_MANUAL))
			cmdval |= (1 << 12);
		writel(data->blocksize, &reg->blksz);
		writel(data->blocks * data->blocksize, &reg->bytecnt);
	} else {
		if ((cmd->cmdidx == 12)&&(cmd->flags&MMC_CMD_MANUAL)) {
			cmdval |= 1<<14;//stop current data transferin progress.
			cmdval &= ~(1 << 13);//Send command at once, even if previous data transfer has notcompleted
		}
	}

	MMCDBG("mmc %d, cmd %d(0x%08x), arg 0x%08x\n",
		mmchost->mmc_no, cmd->cmdidx, cmdval|cmd->cmdidx, cmd->cmdarg);

	writel(cmd->cmdarg, &reg->arg);
	if (!data)
		writel(cmdval|cmd->cmdidx, &reg->cmd);

	/*
	 * transfer data and check status
	 * STATREG[2] : FIFO empty
	 * STATREG[3] : FIFO full
	 */
	if (data) {
		int ret = 0;

		bytecnt = data->blocksize * data->blocks;
		MMCDBG("mmc %d trans data %d bytes\n",mmchost->mmc_no, bytecnt);
#ifdef CONFIG_MMC_SUNXI_USE_DMA
		if (bytecnt > 64) {
#else
		if (0) {
#endif
			usedma = 1;
			writel(readl(&reg->gctrl)&(~0x80000000), &reg->gctrl);
			ret = mmc_trans_data_by_dma(mmc, data);
			writel(cmdval|cmd->cmdidx, &reg->cmd);
		} else {
			writel(readl(&reg->gctrl)|0x80000000, &reg->gctrl);
			writel(cmdval|cmd->cmdidx, &reg->cmd);
			ret = mmc_trans_data_by_cpu(mmc, data);
		}
		if (ret) {
			MMCINFO("mmc %d Transfer failed\n",mmchost->mmc_no);
			error = readl(&reg->rint) & 0xbfc2;
			if(!error)
				error = 0xffffffff;
			goto out;
		}
	}

	timeout = 1000;
	do {
		status = readl(&reg->rint);
		if (!timeout-- || (status & 0xbfc2)) {
			error = status & 0xbfc2;
			if(!error)
				error = 0xffffffff;//represet software timeout
			MMCMSG(mmc, "mmc %d cmd %d timeout, err %x\n",mmchost->mmc_no, cmd->cmdidx, error);
			goto out;
		}
		__usdelay(1);
	} while (!(status&0x4));

	if (data) {
		unsigned done = 0;
		timeout = usedma ? (50*bytecnt/25) : 0xffffff;//0.04us(25M)*2(4bit width)*25()
		if(timeout < 0xffffff){
			timeout = 0xffffff;
		}
		MMCDBG("mmc %d cacl timeout %x\n",mmchost->mmc_no, timeout);
		do {
			status = readl(&reg->rint);
			if (!timeout-- || (status & 0xbfc2)) {
				error = status & 0xbfc2;
				if(!error)
					error = 0xffffffff;//represet software timeout
				MMCMSG(mmc, "mmc %d data timeout %x\n",mmchost->mmc_no, error);
				goto out;
			}
			if ((timeout == 0xFF0000) && mmc->do_tuning && (data->flags&MMC_DATA_READ)  /*(bytecnt==512)*/) {
				error = 0xffffffff;//represet software timeout
				MMCMSG(mmc, "mmc %d data timeout %x -----------err\n",mmchost->mmc_no, error);
				goto out;
			}

			if ((data->blocks > 1)&&!(cmd->flags&MMC_CMD_MANUAL))//not wait auto stop when MMC_CMD_MANUAL is set
			{
				if (usedma)
					done = ((status & (1<<14)) && (readl(&reg->idst) & 0x3)) ? 1 : 0;
				else
					done = status & (1<<14);
			}
			else
			{
				if (usedma)
					done = ((status & (1<<3)) && (readl(&reg->idst) & 0x3)) ? 1 : 0;
				else
					done = status & (1<<3);
			}
			__usdelay(1);
		} while (!done);
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		if ((cmd->cmdidx == MMC_CMD_ERASE)
			|| ((cmd->cmdidx == MMC_CMD_SWITCH)
				&&(((cmd->cmdarg>>16)&0xFF) == EXT_CSD_SANITIZE_START)))
			timeout = 0x1fffffff;
		else
			timeout = 500*1000;

		do {
			status = readl(&reg->status);
			if (!timeout--) {
				error = -1;
				MMCINFO("mmc %d busy timeout\n",mmchost->mmc_no);
				goto out;
			}
			__usdelay(1);
		} while (status & (1 << 9));

		if ((cmd->cmdidx == MMC_CMD_ERASE)
			|| ((cmd->cmdidx == MMC_CMD_SWITCH)
				&&(((cmd->cmdarg>>16)&0xFF) == EXT_CSD_SANITIZE_START)))
			MMCINFO("%s: cmd %d wait rsp busy 0x%x us \n",__FUNCTION__,
				cmd->cmdidx, 0x1fffffff-timeout);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&reg->resp3);
		cmd->response[1] = readl(&reg->resp2);
		cmd->response[2] = readl(&reg->resp1);
		cmd->response[3] = readl(&reg->resp0);
		MMCDBG("mmc %d mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
			mmchost->mmc_no,
			cmd->response[3], cmd->response[2],
			cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&reg->resp0);
		MMCDBG("mmc %d mmc resp 0x%08x\n",mmchost->mmc_no, cmd->response[0]);
	}
out:
	if(error){
		mmchost->raw_int_bak = readl(&reg->rint )& 0xbfc2;
		mmc_dump_errinfo(mmchost,cmd);
	}
	if (data && usedma) {
		/* IDMASTAREG
		 * IDST[0] : idma tx int
		 * IDST[1] : idma rx int
		 * IDST[2] : idma fatal bus error
		 * IDST[4] : idma descriptor invalid
		 * IDST[5] : idma error summary
		 * IDST[8] : idma normal interrupt sumary
		 * IDST[9] : idma abnormal interrupt sumary
		 */
		status = readl(&reg->idst);
		writel(status, &reg->idst);
		writel(0, &reg->idie);
		writel(0, &reg->dmac);
		writel(readl(&reg->gctrl)&(~(1 << 5)), &reg->gctrl);
	}
	if (error) {

		/* during tuning sample point, some sample point may cause timing problem.
		for example, if a RTO error occurs, host may stop clock and device may still output data.
		we need to read all data(512bytes) from device to avoid to update clock fail.
		*/
		if (mmc->do_tuning && data && (data->flags&MMC_DATA_READ) && (bytecnt==512)) {
			writel(readl(&reg->gctrl)|0x80000000, &reg->gctrl);
			writel(0xdeb, &reg->dbgc);
			timeout = 1000;
			MMCMSG(mmc, "Read remain data\n");
			while (readl(&reg->bbcr)<512) {
				unsigned int tmp = readl(mmchost->database);
				tmp = tmp;
				MMCDBG("Read data 0x%x, bbcr 0x%x\n", tmp, readl(&reg->bbcr));
				__usdelay(1);
				if (!(timeout--)) {
					MMCMSG(mmc, "Read remain data timeout\n");
					break;
				}
			}
		}

		writel(0x7, &reg->gctrl);
		while(readl(&reg->gctrl)&0x7) { };

		{
			mmc_save_regs(mmchost);
			mmc_clk_io_onoff(mmchost->mmc_no, 0, 0);
			MMCMSG(mmc, "mmc %d close bus gating and reset\n", mmchost->mmc_no);
			mmc_clk_io_onoff(mmchost->mmc_no, 1, 0);
			mmc_restore_regs(mmchost);

			writel(0x7, &reg->gctrl);
			while(readl(&reg->gctrl)&0x7) { };
		}

		mmc_update_clk(mmchost);
		MMCMSG(mmc, "mmc %d mmc cmd %d err 0x%08x\n", mmchost->mmc_no, cmd->cmdidx, error);

	}
	writel(0xffffffff, &reg->rint);

	if (data && (data->flags&MMC_DATA_READ)) {
        unsigned char *buff = (unsigned char *)data->dest;
        unsigned byte_cnt = data->blocksize * data->blocks;
        flush_cache((unsigned long)buff, (unsigned long)byte_cnt);
        MMCDBG("invald cache after read complete\n");
    }

	if (error)
		return -1;
	else
		return 0;
}


static int sunxi_decide_rty(struct mmc *mmc, int err_no, uint rst_cnt)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned tmode = mmchost->timing_mode;
	u32 spd_md, freq;
	u8 *sdly;
	u8 tm1_retry_gap = 1;
	u8 tm3_retry_gap = 8;
	u8 tm4_retry_gap = 8;

	if (rst_cnt)
	{
		mmchost->retry_cnt = 0;
	}

	if (err_no && (!(err_no & SDXC_RespTimeout)||(err_no==0xffffffff)))
	{
		mmchost->retry_cnt++;

		if (tmode == SUNXI_MMC_TIMING_MODE_1)
		{
			spd_md = mmchost->tm1.cur_spd_md;
			freq = mmchost->tm1.cur_freq;
			sdly = &mmchost->tm1.sdly[spd_md*MAX_CLK_FREQ_NUM+freq];

			if (mmchost->retry_cnt * tm1_retry_gap <  MMC_CLK_SAMPLE_POINIT_MODE_1) {
				if ( (*sdly + tm1_retry_gap) < MMC_CLK_SAMPLE_POINIT_MODE_1) {
					*sdly = *sdly + tm1_retry_gap;
				} else {
					*sdly = *sdly + tm1_retry_gap - MMC_CLK_SAMPLE_POINIT_MODE_1;
				}
				MMCINFO("Get next samply point %d at spd_md %d freq_id %d\n", *sdly, spd_md, freq);
			} else {
				MMCINFO("Beyond the retry times\n");
				return -1;
			}
		}
		else if (tmode == SUNXI_MMC_TIMING_MODE_3)
		{
			spd_md = mmchost->tm3.cur_spd_md;
			freq = mmchost->tm3.cur_freq;
			sdly = &mmchost->tm3.sdly[spd_md*MAX_CLK_FREQ_NUM+freq];

			if (mmchost->retry_cnt * tm3_retry_gap <  MMC_CLK_SAMPLE_POINIT_MODE_3) {
				if ( (*sdly + tm3_retry_gap) < MMC_CLK_SAMPLE_POINIT_MODE_3) {
					*sdly = *sdly + tm3_retry_gap;
				} else {
					*sdly = *sdly + tm3_retry_gap - MMC_CLK_SAMPLE_POINIT_MODE_3;
				}
				MMCINFO("Get next samply point %d at spd_md %d freq_id %d\n", *sdly, spd_md, freq);
			} else {
				MMCINFO("Beyond the retry times\n");
				return -1;
			}
		}
		else if (tmode == SUNXI_MMC_TIMING_MODE_4)
		{
			spd_md = mmchost->tm4.cur_spd_md;
			freq = mmchost->tm4.cur_freq;
			if (spd_md == HS400)
				sdly = &mmchost->tm4.dsdly[freq];
			else
				sdly = &mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM+freq];
			MMCINFO("Current spd_md %d freq_id %d sldy %d\n", spd_md, freq, *sdly);

			if (mmchost->retry_cnt * tm4_retry_gap <  MMC_CLK_SAMPLE_POINIT_MODE_4) {
				if ( (*sdly + tm4_retry_gap) < MMC_CLK_SAMPLE_POINIT_MODE_4) {
					*sdly = *sdly + tm4_retry_gap;
				} else {
					*sdly = *sdly + tm4_retry_gap - MMC_CLK_SAMPLE_POINIT_MODE_4;
				}
				MMCINFO("Get next samply point %d at spd_md %d freq_id %d\n", *sdly, spd_md, freq);
			} else {
				MMCINFO("Beyond the retry times\n");
				return -1;
			}
		}

		mmchost->raw_int_bak = 0;
		return 0;
	}
	MMCDBG("rto or no error or software timeout,no need retry\n");

	return -1;
}
static int sunxi_detail_errno(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	u32 err_no = mmchost->raw_int_bak;
	mmchost->raw_int_bak = 0;
	return err_no;
}

const struct mmc_ops sunxi_mmc_ops = {
	.send_cmd		= mmc_send_cmd,
	.set_ios		= mmc_set_ios,
	.init			= mmc_core_init,
	.decide_retry 		= sunxi_decide_rty,
	.get_detail_errno 	= sunxi_detail_errno,
	.update_phase 		= mmc_update_phase,
};

