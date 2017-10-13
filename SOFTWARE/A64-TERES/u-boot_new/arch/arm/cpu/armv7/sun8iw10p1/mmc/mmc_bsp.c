/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: MMC driver for winner's mmc controller
 * Author: Aaron <leafy.myeh@allwinnertech.com>
 * Date: 2012-2-3 14:18:18
 */
#include "common.h"
#include "mmc_def.h"
#include "mmc_bsp.h"
#include "mmc.h"
#include <asm/arch/gpio.h>
#include <private_boot0.h>
#include <private_toc.h>
#include <private_uboot.h>


#ifdef SUNXI_MMCDBG
//#define MMCDBG(fmt...)	printf("[mmc]: "fmt)

static void dumphex32(char* name, char* base, int len)
{
	__u32 i;

	mmcmsg("dump %s registers:", name);
	for (i=0; i<len; i+=4)
	{
		if (!(i&0xf))
			mmcmsg("\n0x%x : ", base + i);
		mmcmsg("%x ", readl(base + i));
	}
	mmcmsg("\n");
}

static void dumpmmcreg(struct sunxi_mmc *reg)
{
	mmcmsg("gctrl     0x%x\n", reg->gctrl     );
	mmcmsg("clkcr     0x%x\n", reg->clkcr     );
	mmcmsg("timeout   0x%x\n", reg->timeout   );
	mmcmsg("width     0x%x\n", reg->width     );
	mmcmsg("blksz     0x%x\n", reg->blksz     );
	mmcmsg("bytecnt   0x%x\n", reg->bytecnt   );
	mmcmsg("cmd       0x%x\n", reg->cmd       );
	mmcmsg("arg       0x%x\n", reg->arg       );
	mmcmsg("resp0     0x%x\n", reg->resp0     );
	mmcmsg("resp1     0x%x\n", reg->resp1     );
	mmcmsg("resp2     0x%x\n", reg->resp2     );
	mmcmsg("resp3     0x%x\n", reg->resp3     );
	mmcmsg("imask     0x%x\n", reg->imask     );
	mmcmsg("mint      0x%x\n", reg->mint      );
	mmcmsg("rint      0x%x\n", reg->rint      );
	mmcmsg("status    0x%x\n", reg->status    );
	mmcmsg("ftrglevel 0x%x\n", reg->ftrglevel );
	mmcmsg("funcsel   0x%x\n", reg->funcsel   );
	mmcmsg("dmac      0x%x\n", reg->dmac      );
	mmcmsg("dlba      0x%x\n", reg->dlba      );
	mmcmsg("idst      0x%x\n", reg->idst      );
	mmcmsg("idie      0x%x\n", reg->idie      );
}
#else
//#define MMCDBG(fmt...)
#define dumpmmcreg(fmt...)
#define  dumphex32(fmt...)
#endif /* SUNXI_MMCDBG */


extern const boot0_file_head_t BT0_head;
extern int mmc_register(int dev_num, struct mmc *mmc);
extern int mmc_unregister(int dev_num);

/* delay control */
#define SDXC_StartCal        (1<<15)
#define SDXC_CalDone         (1<<14)
#define SDXC_CalDly          (0x3F<<8)
#define SDXC_EnableDly       (1<<7)
#define SDXC_CfgDly          (0x3F<<0)

/* support 4 mmc hosts */
struct mmc mmc_dev[MAX_MMC_NUM];
struct sunxi_mmc_host mmc_host[MAX_MMC_NUM];

void set_mmc_para(int smc_no, void *addr)
{
	struct spare_boot_head_t  *uboot_buf = (struct spare_boot_head_t *)CONFIG_SYS_TEXT_BASE;
	u32 *p = NULL;
	int i = 0;

	memcpy((void *)(uboot_buf->boot_data.sdcard_spare_data), (addr+SDMMC_PRIV_INFO_ADDR_OFFSET), sizeof(struct boot_sdmmc_private_info_t));

	p = (u32 *)(uboot_buf->boot_data.sdcard_spare_data);
	for (i=0; i<5; i++)
		printf("0x%x 0x%x\n", p[i*2 + 0], p[i*2 + 1]);

	return;
}

static int mmc_resource_init(int sdc_no)
{
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];

	mmcdbg("init mmc %d resource\n", sdc_no);
	mmchost->reg = (struct sunxi_mmc *)(MMC_REG_BASE + sdc_no * 0x1000);
	mmchost->database = (u32)mmchost->reg + MMC_REG_FIFO_OS;

	mmchost->hclkbase = CCMU_HCLKGATE0_BASE;
	mmchost->hclkrst  = CCMU_BUS_SOFT_RST_REG0;
	if (sdc_no == 0)
		mmchost->mclkbase = CCMU_MMC0_CLK_BASE;
	else if (sdc_no == 2)
		mmchost->mclkbase = CCMU_MMC2_CLK_BASE;
	else {
		mmcinfo("Wrong mmc NO.: %d\n", sdc_no);
		return -1;
	}
	mmchost->mmc_no = sdc_no;

	return 0;
}

static int mmc_clk_io_onoff(int sdc_no, int onoff, const normal_gpio_cfg *gpio_info, int offset)
{
	unsigned int rval;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];

	if(sdc_no == 0)
	{
		boot_set_gpio((void *)gpio_info, 8, 1);
	}
	else // if(sdc_no == 2)
	{
		boot_set_gpio((void *)(gpio_info + offset), 10, 1);
	}
	/* config ahb clock */
	rval = readl(mmchost->hclkbase);
	rval |= (1 << (8 + sdc_no));
	writel(rval, mmchost->hclkbase);

	rval = readl(mmchost->hclkrst);
	rval |= (1 << (8 + sdc_no));
	writel(rval, mmchost->hclkrst);
	/* config mod clock */
	writel(0x80000000, mmchost->mclkbase);
	mmchost->mod_clk = 24000000;
	dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
	dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
	dumphex32("mmc", (char*)mmchost->reg, 0x100);

	return 0;
}

static int mmc_update_clk(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned int cmd;
	int timeout = 0xfffff;

	writel(readl(&mmchost->reg->clkcr)|(0x1<<31), &mmchost->reg->clkcr);

	cmd = (1U << 31) | (1 << 21) | (1 << 13);
	writel(cmd, &mmchost->reg->cmd);
	while((readl(&mmchost->reg->cmd)&0x80000000) && timeout--);
	if (timeout<0){
		mmcinfo("mmc %d update clk failed\n", mmchost->mmc_no);
		writel(0x0, 0x1c2084c);
		mmcinfo("0x%x 0x%x\n", readl(0x1c2084c), readl(0x1c20858));
		dumphex32("mmc", (char*)mmchost->reg, 0x100);
		return -1;
	}

	writel(readl(&mmchost->reg->clkcr) & (~(0x1<<31)), &mmchost->reg->clkcr);

	writel(readl(&mmchost->reg->rint), &mmchost->reg->rint);
	return 0;
}

static int mmc_update_phase(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;

	if (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_1)
	{
		mmcdbg("mmc re-update_phase\n");
		return mmc_update_clk(mmc);
	}

	return 0;
}

/*
 -- speed mode --
sm0: DS26_SDR12
sm1: HSSDR52_SDR25
sm2: HSDDR52_DDR50
sm3: HS200_SDR104
sm4: HS400

-- frequency point --
f0: CLK_400K
f1: CLK_25M
f2: CLK_50M
f3: CLK_100M
f4: CLK_150M
f5: CLK_200M
*/
/* some information is passed in boot0's extend data buffer: BT0_head.prvt_head.storage_data
    these information is write to BT0_head.prvt_head.storage_data before write boot0 during product.
    1. BT0_head.prvt_head.storage_data + 128 Byte
        +0 struct tune_sdly
    2. BT0_head.prvt_head.storage_data + 128 + sizeof(struct tune_sdly) Byte
        +0 Byte: boot0_para
        +1 Byte: boot_odly_50M
        +2 Byte: boot_sdly_50M
        +3 Byte: boot_odly_50M_ddr
        +4 Byte: boot_sdly_50M_ddr
        +5 Byte: boot_hs_f_max
*/
#define BOOT0_PARA_USE_INTERNAL_DEFAULT_TIMING_PARA (1U<<0)
/* 0: pass through struct sdly;  1: pass through boot_odly/sdly_* */
#define BOOT0_PARA_USE_EXTERNAL_INPUT_TIMING_PARA   (1U<<1)
static int mmc_get_timing_cfg_tm4(u32 sdc_no, u32 spd_md_id, u32 freq_id, u8 *odly, u8 *sdly)
{
	s32 ret = 0;
	struct boot_sdmmc_private_info_t *priv_info =
		(struct boot_sdmmc_private_info_t *)(BT0_head.prvt_head.storage_data+SDMMC_PRIV_INFO_ADDR_OFFSET);
	struct tune_sdly *tune_sdly = &(priv_info->tune_sdly);
	u8 boot0_para = priv_info->boot_mmc_cfg.boot0_para;
	u32 spd_md_sdly = 0;
	u32 dly = 0;

	if (boot0_para & BOOT0_PARA_USE_INTERNAL_DEFAULT_TIMING_PARA)
	{
		if (spd_md_id == DS26_SDR12)
		{
			if (freq_id <= CLK_25M) {
				*odly = 0;
				*sdly = 2;
			} else {
				mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
				ret = -1;
			}
		}
		else if (spd_md_id == HSSDR52_SDR25)
		{
			if (freq_id <= CLK_25M) {
				*odly = 0;
				*sdly = 3;
			} else if (freq_id == CLK_50M) {
				*odly = 0;
				*sdly = 4;
			} else {
				mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
				ret = -1;
			}
		}
		else
		{
			mmcinfo("wrong speed mode %d\n", spd_md_id);
			ret = -1;
		}
	}
	else
	{
		if (boot0_para & BOOT0_PARA_USE_EXTERNAL_INPUT_TIMING_PARA)
		{
			if (spd_md_id == DS26_SDR12)
			{
				if (freq_id <= CLK_25M) {
					*odly = 0;
					*sdly = 2;
				} else {
					mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
					ret = -1;
				}
			}
			else if (spd_md_id == HSSDR52_SDR25)
			{
				if (freq_id <= CLK_25M) {
					*odly = 0;
					*sdly = 3;
				} else if (freq_id == CLK_50M) {
					*odly = priv_info->boot_mmc_cfg.boot_odly_50M;
					*sdly = priv_info->boot_mmc_cfg.boot_sdly_50M;
				} else {
					mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
					ret = -1;
				}
			}
			else if (spd_md_id == HSDDR52_DDR50)
			{
				if (freq_id <= CLK_25M) {
					*odly = 0;
					*sdly = 2;
				} else if (freq_id == CLK_50M) {
					*odly = priv_info->boot_mmc_cfg.boot_odly_50M_ddr;
					*sdly = priv_info->boot_mmc_cfg.boot_sdly_50M_ddr;
				} else {
					mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
					ret = -1;
				}
			}
			else
			{
				mmcinfo("wrong speed mode %d\n", spd_md_id);
				ret = -1;
			}
		}
		else
		{
			if ((sdc_no != 2) || (spd_md_id>4) || (freq_id>5)) {
				mmcinfo("wrong input para: mmc %d/2, mode %d/[0,4], freq %d/[0,5]\n", sdc_no, spd_md_id, freq_id);
				ret = -1;
				goto out;
			}

			spd_md_sdly = tune_sdly->tm4_smx_fx[spd_md_id*2 + freq_id/4];
			dly = ((spd_md_sdly>>((freq_id%4)*8)) & 0xff);

			if (dly == 0xff)
			{
				if (spd_md_id == DS26_SDR12)
				{
					if (freq_id <= CLK_25M) {
						dly = 0;
					} else {
						mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
						ret = -1;
					}
				}
				else if (spd_md_id == HSSDR52_SDR25)
				{
					if (freq_id <= CLK_25M) {
						dly = 0;
					} else if (freq_id == CLK_50M){
						dly = 15;
					} else {
						mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
						ret = -1;
					}
				}
				else if (spd_md_id == HSDDR52_DDR50)
				{
					if (freq_id <= CLK_25M) {
						dly = 0;
					} else {
						mmcinfo("wrong freq %d at spd md %d\n", freq_id, spd_md_id);
						ret = -1;
					}
				}
				else
				{
					mmcinfo("wrong speed mode %d\n", spd_md_id);
					ret = -1;
				}
			}

			*odly = 0;
			*sdly = dly;
			mmcdbg("%s: %d %d 0x%x 0x%x, odly %d sdly %d\n", __FUNCTION__, spd_md_id, freq_id, spd_md_sdly, dly, *odly, *sdly);
		}
	}

out:
	return ret;
}

static int mmc_get_timing_cfg(u32 sdc_no, u32 spd_md_id, u32 freq_id, u8 *odly, u8 *sdly)
{
	s32 ret = 0;
	u32 tm = mmc_host[sdc_no].timing_mode;

	if ((sdc_no == 2) && (tm == SUNXI_MMC_TIMING_MODE_4))
		return mmc_get_timing_cfg_tm4(sdc_no, spd_md_id, freq_id, odly, sdly);
	else if ((sdc_no == 0) && (tm == SUNXI_MMC_TIMING_MODE_1)) {
		if ((spd_md_id <= HSSDR52_SDR25) && (freq_id<=CLK_50M)) {
			*odly = 0;
			*sdly = 0;
			ret = 0;
		} else {
			mmcinfo("sdc0 spd mode error, %d\n", spd_md_id);
			ret = -1;
		}
	} else {
		mmcinfo("mmc_get_timing_cfg: input para error!\n");
		ret = -1;
	}

	return ret;
}

static int _get_pll_periph0(void)
{
	unsigned int reg_val;
	int factor_n, factor_k, pll6;

	reg_val = readl(CCMU_PLL_PERIPH0_CTRL_REG);
	factor_n = ((reg_val >> 8) & 0x1f) + 1;
	factor_k = ((reg_val >> 4) & 0x03) + 1;
	pll6 = 24 * factor_n * factor_k/2;
	return pll6;
}

static int mmc_set_mclk(struct sunxi_mmc_host* mmchost, u32 clk_hz)
{
	unsigned n, m, div, src, sclk_hz = 0;
	unsigned rval;

	mmcdbg("%s: mod_clk %d\n", __FUNCTION__, clk_hz);

	if (clk_hz <= 4000000) { //400000
		src = 0;
		sclk_hz = 24000000;
	} else {
		src = 1;
		sclk_hz = _get_pll_periph0()*2*1000000; /*use 2x pll-per0 clock */
	}

	div = (2 * sclk_hz + clk_hz) / (2 * clk_hz);
	div = (div==0) ? 1 : div;
	if (div > 128) {
		m = 1;
		n = 0;
		mmcinfo("%s: source clock is too high, clk %d, src %d!!!\n",
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
		sclk_hz = _get_pll_periph0()*2*1000000; /* use 2x pll6 */
	else if (src == 2) {
		/*todo*/
	} else {
		mmcinfo("%s: wrong clock source %d\n",__func__, src);
	}

	return (sclk_hz / (1<<n) / (m+1) );
}

static unsigned mmc_config_delay(struct sunxi_mmc_host* mmchost, u32 spd_md_id, u32 freq_id)
{
	int ret = 0;
	unsigned rval = 0;
	unsigned mode = mmchost->timing_mode;
	unsigned spd_md, spd_md_bak, freq;
	u8 odly, sdly, dsdly=0;

	if (mode == SUNXI_MMC_TIMING_MODE_1)
	{
		odly = 0;
		sdly = 0;

		mmcdbg("%s: odly: %d   sldy: %d\n", __FUNCTION__, odly, sdly);
		rval = readl(&mmchost->reg->drv_dl);
		rval &= (~(0x3<<16));
		rval |= (((odly&0x1)<<16) | ((odly&0x1)<<17));
		writel(rval, &mmchost->reg->drv_dl);

		rval = readl(&mmchost->reg->ntsr);
		rval &= (~(0x3<<4));
		rval |= ((sdly&0x3)<<4);
		writel(rval, &mmchost->reg->ntsr);
	}
	else if (mode == SUNXI_MMC_TIMING_MODE_3)
	{
		/* mode 3 is optional on sdc0. we don't support config delay here. */
		odly = 0;
		sdly = 0;

		mmcinfo("mode 3 is optional for sdc0. please use mode 1.\n");
		mmcdbg("%s: odly: %d   sldy: %d\n", __FUNCTION__, odly, sdly);
		rval = readl(&mmchost->reg->drv_dl);
		rval &= (~(0x3<<16));
		rval |= (((odly&0x1)<<16) | ((odly&0x1)<<17));
		writel(rval, &mmchost->reg->drv_dl);

		rval = readl(&mmchost->reg->samp_dl);
		rval &= (~SDXC_CfgDly);
		rval |= ((sdly&SDXC_CfgDly) | SDXC_EnableDly);
		writel(rval, &mmchost->reg->samp_dl);
	}
	else if (mode == SUNXI_MMC_TIMING_MODE_4)
	{
		spd_md = spd_md_id;
		spd_md_bak = spd_md_id;
		freq = freq_id;

		if (spd_md == HS400)
			spd_md = HS200_SDR104; /* use HS200's sdly for HS400's CMD line */

		odly = 0xff;
		sdly = 0xff;
		if (mmc_get_timing_cfg(mmchost->mmc_no, spd_md, freq, &odly, &sdly)) {
			mmcinfo("get timing para error %d\n", __LINE__);
			ret = -1;
			goto OUT;
		}
		if ((odly == 0xff) || (sdly == 0xff)) {
			mmcinfo("timing para error %d\n", __LINE__);
			ret = -1;
			goto OUT;
		}

		rval = readl(&mmchost->reg->drv_dl);
		rval &= (~(0x3<<16));
		rval |= (((odly&0x1)<<16) | ((odly&0x1)<<17));
		writel(rval, &mmchost->reg->drv_dl);

		rval = readl(&mmchost->reg->samp_dl);
		rval &= (~SDXC_CfgDly);
		rval |= ((sdly&SDXC_CfgDly) | SDXC_EnableDly);
		writel(rval, &mmchost->reg->samp_dl);

		spd_md = spd_md_bak;
		if (spd_md == HS400)
		{
			odly = 0xff;
			dsdly = 0xff;
			if (mmc_get_timing_cfg(mmchost->mmc_no, spd_md, freq, &odly, &dsdly)) {
				mmcinfo("get timing para error %d\n", __LINE__);
				ret = -1;
				goto OUT;
			}
			if ((odly == 0xff) || (dsdly == 0xff)) {
				mmcinfo("timing para error %d\n", __LINE__);
				ret = -1;
				goto OUT;
			}

			rval = readl(&mmchost->reg->ds_dl);
			rval &= (~SDXC_CfgDly);
			rval |= ((dsdly&SDXC_CfgDly) | SDXC_EnableDly);
			#ifdef FPGA_PLATFORM
			rval &= (~0x7);
			#endif
			writel(rval, &mmchost->reg->ds_dl);
		}
		mmcdbg("%s: spd_md:%d, freq:%d, odly: %d; sdly: %d; dsdly: %d\n", __FUNCTION__, spd_md, freq, odly, sdly, dsdly);
	}

OUT:
	return ret;
}

static int mmc_config_clock_modex(struct sunxi_mmc_host* mmchost, unsigned clk)
{
	unsigned rval = 0;
	struct mmc *mmc = mmchost->mmc;
	unsigned mode = mmchost->timing_mode;

	unsigned freq;

	/* disable mclk */
	writel(0x0, mmchost->mclkbase);
	mmcdbg("mmc %d mclkbase 0x%x\n", mmchost->mmc_no, readl(mmchost->mclkbase));

	/* enable timing mode 1 */
	if (mode == SUNXI_MMC_TIMING_MODE_1) {
		rval = readl(&mmchost->reg->ntsr);
		rval |= (1<<31);
		writel(rval, &mmchost->reg->ntsr);
		mmcdbg("mmc %d rntsr 0x%x\n", mmchost->mmc_no, rval);
	} else
		writel(0x0, &mmchost->reg->ntsr);

	/* configure clock */
	if ((mode == SUNXI_MMC_TIMING_MODE_1) || (mmc->speed_mode == HSDDR52_DDR50)) {
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
	mmcdbg("get round card clk %d, mod_clk %d\n", mmc->clock, mmchost->mod_clk);

	/* re-enable mclk */
	writel(readl(mmchost->mclkbase)|(1<<31),mmchost->mclkbase);
	mmcdbg("mmc %d mclkbase 0x%x\n", mmchost->mmc_no, readl(mmchost->mclkbase));

	/*
	 * CLKCREG[7:0]: divider
	 * CLKCREG[16]:  on/off
	 * CLKCREG[17]:  power save
	 */
	rval = readl(&mmchost->reg->clkcr);
	rval &= ~(0xFF);
	if ((mode == SUNXI_MMC_TIMING_MODE_1)||(mode == SUNXI_MMC_TIMING_MODE_3)) {
		if (mmc->speed_mode == HSDDR52_DDR50)
			rval |= 0x1;
	} else if (mode == SUNXI_MMC_TIMING_MODE_4) {
		if ((mmc->speed_mode == HSDDR52_DDR50) && (mmc->bus_width == 8))
			rval |= 0x1;
	}
	writel(rval, &mmchost->reg->clkcr);

	if (mmc_update_clk(mmc))
		return -1;

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

	mmc_config_delay(mmchost, mmc->speed_mode, freq);

	//dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
	//dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
	//dumphex32("mmc", (char*)mmchost->reg, 0x100);
	return 0;
}

static int mmc_config_clock(struct mmc *mmc, unsigned clk)
{
	unsigned rval = 0;
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host* )mmc->priv;

	if ((mmc->speed_mode == HSDDR52_DDR50) || (mmc->speed_mode == HS400)) {
		if (clk > mmc->f_max_ddr)
			clk = mmc->f_max_ddr;
	}

	/* disable card clock */
	rval = readl(&mmchost->reg->clkcr);
	rval &= ~(1 << 16);
	writel(rval, &mmchost->reg->clkcr);
	if (mmc_update_clk(mmchost->mmc))
		return -1;

	if ((mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_1)
		|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_3)
		|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_4) )

		mmc_config_clock_modex(mmchost, clk);

	else {
		mmcinfo("mmc %d wrong timing mode: 0x%x\n",
			mmchost->mmc_no, mmchost->timing_mode);
		return -1;
	}

	/* Re-enable card clock */
	rval = readl(&mmchost->reg->clkcr);
	rval |=  (0x1 << 16); //(3 << 16);
	writel(rval, &mmchost->reg->clkcr);
	if (mmc_update_clk(mmchost->mmc)){
		mmcinfo("mmc %d re-enable clock failed\n",mmchost->mmc_no);
		return -1;
	}

	return 0;
}

static void mmc_ddr_mode_onoff(struct mmc *mmc, int on)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	u32 rval = 0;

	rval = readl(&mmchost->reg->gctrl);
	rval &= (~(1U << 10));

	if (on) {
		rval |= (1U << 10);
		writel(rval, &mmchost->reg->gctrl);
		mmcdbg("set %d rgctrl 0x%x to enable ddr mode\n", mmchost->mmc_no, readl(&mmchost->reg->gctrl));
	} else {
		writel(rval, &mmchost->reg->gctrl);
		mmcdbg("set %d rgctrl 0x%x to disable ddr mode\n", mmchost->mmc_no, readl(&mmchost->reg->gctrl));
	}
}

static void mmc_hs400_mode_onoff(struct mmc *mmc, int on)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	u32 rval = 0;

	if (mmchost->mmc_no != 2) {
		return ;
	}

	rval = readl(&mmchost->reg->dsbd);
	rval &= (~(1 << 31));

	if (on) {
		rval |= (1 << 31);
		writel(rval, &mmchost->reg->dsbd);
		mmcdbg("set %d dsbd 0x%x to enable hs400 mode\n", mmchost->mmc_no, readl(&mmchost->reg->dsbd));
	} else {
		writel(rval, &mmchost->reg->dsbd);
		mmcdbg("set %d dsbd 0x%x to disable hs400 mode\n", mmchost->mmc_no, readl(&mmchost->reg->dsbd));
	}
}


static void mmc_set_ios(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;


	mmcdbg("mmc %d ios: bus: %d, clock: %d\n",mmchost ->mmc_no, mmc->bus_width, mmc->clock);

	if (mmc->clock && mmc_config_clock(mmc, mmc->clock)) {
	    mmcinfo("[mmc]: " "*** update clock failed\n");
		mmchost->fatal_err = 1;
		return;
	}
	/* Change bus width */
	if (mmc->bus_width == 8)
		writel(2, &mmchost->reg->width);
	else if (mmc->bus_width == 4)
		writel(1, &mmchost->reg->width);
	else
		writel(0, &mmchost->reg->width);

	/* set ddr mode */
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

static int mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	u32 rval = 0;
	
	/* Reset controller */
	writel(0x7, &mmchost->reg->gctrl);
	while(readl(&mmchost->reg->gctrl)&0x7);

#if 1
#define  SMC_DATA_TIMEOUT     0xffffffU
#define  SMC_RESP_TIMEOUT     0xff
#else
#define  SMC_DATA_TIMEOUT     0x1ffffU
#define  SMC_RESP_TIMEOUT     0x2
#endif
	writel((SMC_DATA_TIMEOUT<<8)|SMC_RESP_TIMEOUT, &mmchost->reg->timeout); //Set Data & Response Timeout Value

	writel((512<<16)|(1U<<2)|(1U<<0), &mmchost->reg->thldc);
	writel(3, &mmchost->reg->csdc);
	writel(0xdeb, &mmchost->reg->dbgc);

	/* release eMMC reset signal */
	writel(1, &mmchost->reg->hwrst);
	writel(0, &mmchost->reg->hwrst);
	__msdelay(1);
	writel(1, &mmchost->reg->hwrst);
	__msdelay(1);
	
	if (mmc->control_num == 0) {
		/* enable 2xclk mode, and use default input phase */
		rval = readl(&mmchost->reg->ntsr);
		rval |= (1U<<31); //|(1U<<4);
		writel(rval, &mmchost->reg->ntsr);
	
		/* use 90 degree output phase */
		rval = readl(&mmchost->reg->drv_dl);
		//rval |= ((1U<<16)|((1U<<17)));
		writel(rval, &mmchost->reg->drv_dl);
	} else {
		/* configure input delay time to 0, use default input phase */		
		rval = readl(&mmchost->reg->samp_dl);
		rval &= ~(0x3F);
		rval |= (1U<<7); //|(0x3f);
		writel(rval, &mmchost->reg->samp_dl);
	
		/* use 90 degree output phase */
		rval = readl(&mmchost->reg->drv_dl);
		//rval |= ((1U<<16)|((1U<<17)));
		writel(rval, &mmchost->reg->drv_dl);
	}

	return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned i;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned *buff;
	unsigned timeout = 0xffffff;

	if (data->flags & MMC_DATA_READ) {
		buff = (unsigned int *)data->b.dest;
		for (i=0; i<(byte_cnt>>2); i++) {
			while(--timeout && (readl(&mmchost->reg->status)&(1 << 2)));
			if (timeout <= 0)
				goto out;
			buff[i] = readl(mmchost->database);
			timeout = 0xffffff;
		}
	} else {
		buff = (unsigned int *)data->b.src;
		for (i=0; i<(byte_cnt>>2); i++) {
			while(--timeout && (readl(&mmchost->reg->status)&(1 << 3)));
			if (timeout <= 0)
				goto out;
			writel(buff[i], mmchost->database);
			timeout = 0xffffff;
		}
	}

out:
	if (timeout <= 0){
		mmcinfo("mmc %d transfer by cpu failed\n",mmchost ->mmc_no);
		return -1;
	}

	return 0;
}

static int mmc_trans_data_by_dma(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct sunxi_mmc_des *pdes = NULL;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned char *buff;
	unsigned des_idx = 0;
	unsigned buff_frag_num = 0;
	unsigned remain;
	unsigned i, rval;

	buff = data->flags & MMC_DATA_READ ?
			(unsigned char *)data->b.dest : (unsigned char *)data->b.src;
	buff_frag_num = byte_cnt >> SDXC_DES_NUM_SHIFT;
	remain = byte_cnt & (SDXC_DES_BUFFER_MAX_LEN-1);
	if (remain)
		buff_frag_num ++;
	else
		remain = SDXC_DES_BUFFER_MAX_LEN;

	pdes = mmchost->pdes;

	//OSAL_CacheRangeFlush(buff, (unsigned long)byte_cnt, CACHE_CLEAN_FLUSH_D_CACHE_REGION);
	for (i=0; i < buff_frag_num; i++, des_idx++) {
		memset((void*)&pdes[des_idx], 0, sizeof(struct sunxi_mmc_des));
		pdes[des_idx].des_chain = 1;
		pdes[des_idx].own = 1;
		pdes[des_idx].dic = 1;
		if (buff_frag_num > 1 && i != buff_frag_num-1) {
			pdes[des_idx].data_buf1_sz = SDXC_DES_BUFFER_MAX_LEN;
		} else
			pdes[des_idx].data_buf1_sz = remain;

		pdes[des_idx].buf_addr_ptr1 = (u32)buff + i * SDXC_DES_BUFFER_MAX_LEN;
		if (i==0)
			pdes[des_idx].first_des = 1;

		if (i == buff_frag_num-1) {
			pdes[des_idx].dic = 0;
			pdes[des_idx].last_des = 1;
			pdes[des_idx].end_of_ring = 1;
			pdes[des_idx].buf_addr_ptr2 = 0;
		} else {
			pdes[des_idx].buf_addr_ptr2 = (u32)&pdes[des_idx+1];
		}
		mmcdbg("mmc %d frag %d, remain %d, des[%d](%x): "
			"[0] = %x, [1] = %x, [2] = %x, [3] = %x\n",mmchost ->mmc_no,
			i, remain, des_idx, (u32)&pdes[des_idx],
			(u32)((u32*)&pdes[des_idx])[0], (u32)((u32*)&pdes[des_idx])[1],
			(u32)((u32*)&pdes[des_idx])[2], (u32)((u32*)&pdes[des_idx])[3]);
	}
	//OSAL_CacheRangeFlush(pdes, sizeof(struct sunxi_mmc_des) * (des_idx+1), CACHE_CLEAN_FLUSH_D_CACHE_REGION);
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
	rval = readl(&mmchost->reg->gctrl);
	writel(rval|(1 << 5)|(1 << 2), &mmchost->reg->gctrl);	/* dma enable */
	writel((1 << 0), &mmchost->reg->dmac); /* idma reset */
	writel((1 << 1) | (1 << 7), &mmchost->reg->dmac); /* idma on */
	rval = readl(&mmchost->reg->idie) & (~3);
	if (data->flags & MMC_DATA_WRITE)
		rval |= (1 << 0);
	else
		rval |= (1 << 1);
	writel(rval, &mmchost->reg->idie);
	writel((u32)pdes, &mmchost->reg->dlba);
	writel((2U<<28)|(7<<16)|248, &mmchost->reg->ftrglevel); /* burst-8, rx/tx trigger level=7/248 */

	return 0;
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned int cmdval = 0x80000000;
	unsigned int timeout = 0;
	int error = 0;
	unsigned int status = 0;
	unsigned int usedma = 0;
	unsigned int bytecnt = 0;

	if (mmchost->fatal_err){
		mmcinfo("mmc %d Found fatal err,so no send cmd\n",mmchost ->mmc_no);
		return -1;
	}
	if (cmd->resp_type & MMC_RSP_BUSY)
		mmcdbg("mmc %d cmd %d check rsp busy\n",mmchost ->mmc_no, cmd->cmdidx);
	if (cmd->cmdidx == 12)
		return 0;
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
		if ((u32)data->b.dest & 0x3) {
			mmcinfo("mmc %d dest is not 4 byte align\n",mmchost ->mmc_no);
			error = -1;
			goto out;
		}

		cmdval |= (1 << 9) | (1 << 13);
		if (data->flags & MMC_DATA_WRITE)
			cmdval |= (1 << 10);
		if (data->blocks > 1)
			cmdval |= (1 << 12);
		writel(data->blocksize, &mmchost->reg->blksz);
		writel(data->blocks * data->blocksize, &mmchost->reg->bytecnt);
	}

	mmcdbg("mmc %d, cmd %d(0x%x), arg 0x%x\n", mmchost->mmc_no, cmd->cmdidx, cmdval|cmd->cmdidx, cmd->cmdarg);
	writel(cmd->cmdarg, &mmchost->reg->arg);
	if (!data)
		writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);

	/*
	 * transfer data and check status
	 * STATREG[2] : FIFO empty
	 * STATREG[3] : FIFO full
	 */
	if (data) {
		int ret = 0;
		bytecnt = data->blocksize * data->blocks;
		mmcdbg("mmc %d trans data %d bytes\n",mmchost ->mmc_no, bytecnt);
#ifdef MMC_TRANS_BY_DMA
		if (bytecnt > 512) {
#else
		if (0) {
#endif
			usedma = 1;
			writel(readl(&mmchost->reg->gctrl)&(~0x80000000), &mmchost->reg->gctrl);
			ret = mmc_trans_data_by_dma(mmc, data);
			writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);
		} else {
			writel(readl(&mmchost->reg->gctrl)|0x80000000, &mmchost->reg->gctrl);
			writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);
			ret = mmc_trans_data_by_cpu(mmc, data);
		}
		if (ret) {
			mmcinfo("mmc %d Transfer failed\n",mmchost ->mmc_no);
			error = readl(&mmchost->reg->rint) & 0xbbc2;
			if(!error)
				error = 0xffffffff;
			goto out;
		}
	}

	timeout = 0xffffff;
	do {
		status = readl(&mmchost->reg->rint);
		if (!timeout-- || (status & 0xbbc2)) {
			error = status & 0xbbc2;
			if(!error)
				error = 0xffffffff;//represet software timeout
			mmcinfo("mmc %d cmd %d timeout, err %x\n",mmchost ->mmc_no, cmd->cmdidx, error);
			goto out;
		}
	} while (!(status&0x4));

	if (data) {
		unsigned done = 0;
		timeout = usedma ? 0xffff*bytecnt : 0xffff;

		do {
			status = readl(&mmchost->reg->rint);
			if (!timeout-- || (status & 0xbbc2)) {
				error = status & 0xbbc2;
				if(!error)
					error = 0xffffffff;//represet software timeout
				mmcinfo("mmc %d data timeout, err %x\n",mmchost ->mmc_no, error);
				goto out;
			}
			if (data->blocks > 1)
				done = status & (1 << 14);
			else
				done = status & (1 << 3);
		} while (!done);

		if ((data->flags & MMC_DATA_READ)&& usedma) {
			timeout = 0xffffff;
			done = 0;
			status = 0;
			mmcdbg("mmc %d cacl rd dma timeout %x\n", mmchost->mmc_no, timeout);
			do {
					status = readl(&mmchost->reg->idst);
					if (!timeout-- || (status & 0x234)) {
						error = status & 0x1E34;
						if(!error)
							error = 0xffffffff; //represet software timeout
						mmcinfo("mmc %d wait dma over err %x\n", mmchost->mmc_no, error);
						goto out;
					}
					done = status & (1 << 1);
					//usdelay(1);
			} while (!done);
			mmcdbg("idst *****0x%d******\n",readl(&mmchost->reg->idst));
		}
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		timeout = 0x4ffffff;
		do {
			status = readl(&mmchost->reg->status);
			if (!timeout--) {
				error = -1;
				mmcinfo("mmc %d busy timeout\n",mmchost ->mmc_no);
				goto out;
			}
		} while (status & (1 << 9));
	}
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&mmchost->reg->resp3);
		cmd->response[1] = readl(&mmchost->reg->resp2);
		cmd->response[2] = readl(&mmchost->reg->resp1);
		cmd->response[3] = readl(&mmchost->reg->resp0);
		mmcdbg("mmc %d resp 0x%x 0x%x 0x%x 0x%x\n",mmchost ->mmc_no,
			cmd->response[3], cmd->response[2],
			cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&mmchost->reg->resp0);
		mmcdbg("mmc %d resp 0x%x\n",mmchost ->mmc_no, cmd->response[0]);
	}
out:
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
		status = readl(&mmchost->reg->idst);
		writel(status, &mmchost->reg->idst);
        writel(0, &mmchost->reg->idie);
        writel(0, &mmchost->reg->dmac);
        writel(readl(&mmchost->reg->gctrl)&(~(1 << 5)), &mmchost->reg->gctrl);
    //OSAL_CacheRangeFlush(data->b.dest, data->blocksize * data->blocks, 4);
	}
	if (error) {
		dumphex32("mmc", (char*)mmchost->reg, 0x200);
		writel(0x7, &mmchost->reg->gctrl);
		while(readl(&mmchost->reg->gctrl)&0x7);
		mmc_update_clk(mmc);
		mmcinfo("mmc %d cmd %d err %x\n",mmchost ->mmc_no, cmd->cmdidx, error);
	}
	writel(0xffffffff, &mmchost->reg->rint);

	if (error)
		return -1;
	else
		return 0;
}

int sunxi_mmc_init(int sdc_no, unsigned bus_width, const normal_gpio_cfg *gpio_info, int offset ,void *extra_data)
{
	struct mmc *mmc;
	int ret;
	u8 sdly=0xff, odly=0xff;
	struct boot_sdmmc_private_info_t *priv_info =
		(struct boot_sdmmc_private_info_t *)(BT0_head.prvt_head.storage_data + SDMMC_PRIV_INFO_ADDR_OFFSET);
	u8 ext_f_max = priv_info->boot_mmc_cfg.boot_hs_f_max;

	mmcinfo("mmc driver ver %s\n", DRIVER_VER);

	memset(&mmc_dev[sdc_no], 0, sizeof(struct mmc));
	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
	mmc = &mmc_dev[sdc_no];
	mmc_host[sdc_no].mmc = mmc;

	if ((sdc_no == 0) || (sdc_no == 1))
		mmc_host[sdc_no].timing_mode = SUNXI_MMC_TIMING_MODE_1; //SUNXI_MMC_TIMING_MODE_3
	else if (sdc_no == 2)
		mmc_host[sdc_no].timing_mode = SUNXI_MMC_TIMING_MODE_4;

	strcpy(mmc->name, "SUNXI SD/MMC");
	mmc->priv = &mmc_host[sdc_no];
	mmc->send_cmd = mmc_send_cmd;
	mmc->set_ios = mmc_set_ios;
	mmc->init = mmc_core_init;
	mmc->update_phase = mmc_update_phase;

	mmc->voltages = MMC_VDD_29_30|MMC_VDD_30_31|MMC_VDD_31_32|MMC_VDD_32_33|
	                MMC_VDD_33_34|MMC_VDD_34_35|MMC_VDD_35_36;
	mmc->host_caps = MMC_MODE_HS_52MHz|MMC_MODE_HS|MMC_MODE_HC;
	if (bus_width == 4)
		mmc->host_caps |= MMC_MODE_4BIT;
	if ((sdc_no == 2) && (bus_width == 8)) {
		mmc->host_caps |= MMC_MODE_8BIT | MMC_MODE_4BIT;
	}

	mmc->f_min = 400000;
	mmc->f_max = 50000000;
	mmc->f_max_ddr = 50000000;
	if (!mmc_get_timing_cfg(sdc_no, 1, 2, &odly, &sdly)) {
		if (!((odly != 0xff) && (sdly != 0xff)))
			mmc->f_max = 25000000;
	} else
		mmc->f_max = 25000000;

	if ( !mmc_get_timing_cfg(sdc_no, 2, 2, &odly, &sdly) ) {
		if ((odly != 0xff) && (sdly != 0xff))
			mmc->host_caps |= MMC_MODE_DDR_52MHz;
		else
			mmc->f_max_ddr = 25000000;
	} else
		mmc->f_max_ddr = 25000000;

	if (ext_f_max && ((mmc->f_max_ddr/1000000) > ext_f_max))
		mmc->f_max_ddr = ext_f_max * 1000000;
	if (ext_f_max && ((mmc->f_max/1000000) > ext_f_max))
		mmc->f_max = ext_f_max * 1000000;

	mmc->control_num = sdc_no;

    mmc_host[sdc_no].pdes = (struct sunxi_mmc_des*)DMAC_DES_BASE_IN_SDRAM;
	if (mmc_resource_init(sdc_no)){
		mmcinfo("mmc %d resource init failed\n",sdc_no);
		return -1;
	}

	mmc_clk_io_onoff(sdc_no, 1, gpio_info, offset);
	ret = mmc_register(sdc_no, mmc);
	if (ret < 0){
		mmcinfo("mmc %d register failed\n",sdc_no);
		return -1;
	}

	return mmc->lba;
}

int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset)
{
	mmc_clk_io_onoff(sdc_no, 0, gpio_info, offset);
	mmc_unregister(sdc_no);
	memset(&mmc_dev[sdc_no], 0, sizeof(struct mmc));
	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));

	mmcdbg("sunxi mmc%d exit\n",sdc_no);
	return 0;
}
