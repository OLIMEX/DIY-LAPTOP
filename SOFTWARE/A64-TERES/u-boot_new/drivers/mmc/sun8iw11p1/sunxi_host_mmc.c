/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Aaron <leafy.myeh@allwinnertech.com>
 *
 * MMC driver for allwinner sunxi platform.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/mmc.h>
#include <asm/arch/timer.h>
#include <malloc.h>
#include <mmc.h>
#include <sys_config.h>
#include <libfdt.h>
#include <fdt_support.h>
#include "../mmc_def.h"
#include "../sunxi_mmc.h"
#include "sunxi_host0_2_v4p1.h"
#include "sunxi_host_mmc.h"

extern char *spd_name[];
struct sunxi_mmc_host mmc_host[3];
struct mmc_reg_v4p1 mmc_host_reg_bak[3];

static u8 ext_odly_spd_freq[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
static u8 ext_sdly_spd_freq[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
static u8 ext_odly_spd_freq_sdc0[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
static u8 ext_sdly_spd_freq_sdc0[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];

static int mmc_clear_timing_para(int sdc_no)
{
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	int i, j;

	for (i=0; i<MAX_SPD_MD_NUM; i++)
	{
		for (j=0; j<MAX_CLK_FREQ_NUM; j++)
		{
			mmchost->tm1.odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm1.sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm1.def_odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm1.def_sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm3.odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm3.sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm3.def_odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm3.def_sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm4.odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm4.sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm4.def_odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm4.def_sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
		}
	}

	for (j=0; j<MAX_CLK_FREQ_NUM; j++)
	{
		mmchost->tm4.dsdly[j] = 0xFF;
		mmchost->tm4.def_dsdly[j] = 0xFF;
	}

	return 0;
}

static int mmc_init_default_timing_para(int sdc_no)
{
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];

	if (sdc_no == 0)
	{
		/* timing mode 1 */
		mmchost->tm1.cur_spd_md = DS26_SDR12;
		mmchost->tm1.cur_freq = CLK_400K;
		mmchost->tm1.sample_point_cnt = MMC_CLK_SAMPLE_POINIT_MODE_1;

		mmchost->tm1.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = TM1_OUT_PH180;
		mmchost->tm1.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = TM1_IN_PH180;
		mmchost->tm1.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_OUT_PH180;
		mmchost->tm1.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_IN_PH180;

		mmchost->tm1.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_OUT_PH180;
		mmchost->tm1.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_IN_PH90;
		mmchost->tm1.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_OUT_PH180;
		mmchost->tm1.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_IN_PH90;

		mmchost->tm1.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_OUT_PH90;
		mmchost->tm1.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_IN_PH180;
		mmchost->tm1.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_OUT_PH90;
		mmchost->tm1.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_IN_PH180;

		mmchost->tm1.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_OUT_PH180;
		mmchost->tm1.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_IN_PH90;
		mmchost->tm1.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_OUT_PH180;
		mmchost->tm1.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_IN_PH90;

		/* timing mode 3 */
		mmchost->tm3.cur_spd_md = DS26_SDR12;
		mmchost->tm3.cur_freq = CLK_400K;
		mmchost->tm3.sample_point_cnt = MMC_CLK_SAMPLE_POINIT_MODE_3;

		mmchost->tm3.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = TM3_OUT_PH180;
		mmchost->tm3.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = 0x0;
		mmchost->tm3.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = TM3_OUT_PH180;
		mmchost->tm3.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;

		mmchost->tm3.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = TM3_OUT_PH180;
		mmchost->tm3.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm3.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = TM3_OUT_PH180;
		mmchost->tm3.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = 0x0;

		mmchost->tm3.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_OUT_PH90;
		mmchost->tm3.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm3.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_OUT_PH90;
		mmchost->tm3.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = 0x0;

		mmchost->tm3.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = TM3_OUT_PH180;
		mmchost->tm3.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm3.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = TM3_OUT_PH180;
		mmchost->tm3.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = 0x0;
	}	else if (sdc_no == 2) {
		/* timing mode 4 */
		mmchost->tm4.cur_spd_md = DS26_SDR12;
		mmchost->tm4.cur_freq = CLK_400K;
		mmchost->tm4.sample_point_cnt = MMC_CLK_SAMPLE_POINIT_MODE_4;

		mmchost->tm4.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = 0x0;
		mmchost->tm4.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;

		mmchost->tm4.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_400K] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_400K] = 0;
		mmchost->tm4.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = 0;
		mmchost->tm4.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = 0;

		mmchost->tm4.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_400K] = TM1_OUT_PH90;
		mmchost->tm4.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_400K] = 0xe;
		mmchost->tm4.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_OUT_PH90;
		mmchost->tm4.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = 0xe;
		mmchost->tm4.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_OUT_PH90;
		mmchost->tm4.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = 0xe;

		mmchost->tm4.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm4.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = 0x11;
		mmchost->tm4.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_100M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_100M] = 0x12;
		mmchost->tm4.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_150M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_150M] = 0x13;
		mmchost->tm4.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_200M] = TM4_OUT_PH180;
		mmchost->tm4.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_200M] = 0x6;

		mmchost->tm4.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH90;
		mmchost->tm4.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm4.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_50M] = TM4_OUT_PH90;
		mmchost->tm4.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_50M] = 0x11;
		mmchost->tm4.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_100M] = TM4_OUT_PH90;
		mmchost->tm4.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_100M] = 0x12;
		mmchost->tm4.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_150M] = TM4_OUT_PH90;
		mmchost->tm4.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_150M] = 0x13;
		mmchost->tm4.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_200M] = TM4_OUT_PH90;
		mmchost->tm4.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_200M] = 0x6;

		mmchost->tm4.def_dsdly[CLK_25M] = 0x0;
		mmchost->tm4.def_dsdly[CLK_50M] = 0x18;
		mmchost->tm4.def_dsdly[CLK_100M] = 0xd;
		mmchost->tm4.def_dsdly[CLK_150M] = 0x6;
		mmchost->tm4.def_dsdly[CLK_200M] = 0x3;
	}

#if 0
	printf("0x%x 0x%x 0x%x 0x%x\n", (u32)mmchost->tm4.def_odly, (u32)mmchost->tm4.def_sdly, (u32)mmchost->tm4.sdly, (u32)mmchost->tm4.odly);
	for (j=0; j<MAX_SPD_MD_NUM; j++)
	{
		for (i=0; i<MAX_CLK_FREQ_NUM; i++)
		{
			printf("%02x ", mmchost->tm4.def_odly[j*MAX_CLK_FREQ_NUM + i]);
		}
		printf("\n");
	}
	printf("\n\n");


	for (j=0; j<MAX_SPD_MD_NUM; j++)
	{
		for (i=0; i<MAX_CLK_FREQ_NUM; i++)
		{
			printf("%02x ", mmchost->tm4.def_sdly[j*MAX_CLK_FREQ_NUM + i]);
		}
		printf("\n");
	}
	printf("\n");
#endif

	return 0;
}


static int mmc_get_sdly_auto_sample(int sdc_no)
{
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	u32 *p = (u32 *)&mmchost->cfg.platform_caps.sdly.tm4_smx_fx[0];
	int spd_md, f;
	u32 val;
	int work_mode = uboot_spare_head.boot_data.work_mode;

	if (sdc_no != 2) {
		MMCINFO("don't support auto sample\n");
		return -1;
	}

	/* sdly is invalid */
	if (work_mode != WORK_MODE_BOOT)
		return 0;

#if 0
	for (f=0; f<5; f++)
		MMCINFO("0x%x 0x%x\n", p[f*2 + 0], p[f*2 + 1]);
#endif
	for (spd_md=0; spd_md<MAX_SPD_MD_NUM; spd_md++)
	{
		if (spd_md == HS400)
		{
			val = p[spd_md*2 + 0];
			for (f=0; f<4; f++) {
				mmchost->tm4.dsdly[f] = (val>>(f*8)) & 0xFF;
				//MMCINFO("dsdly-0 0x%x\n", mmchost->tm4.dsdly[f]);
			}

			val = p[spd_md*2 + 1];
			for (f=4; f<MAX_CLK_FREQ_NUM; f++) {
				mmchost->tm4.dsdly[f] = (val>>((f-4)*8)) & 0xFF;
				//MMCINFO("dsdly-1 0x%x\n", mmchost->tm4.dsdly[f]);
			}
		}
		else
		{
			val = p[spd_md*2 + 0];
			for (f=0; f<4; f++) {
				mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM + f] = (val>>(f*8)) & 0xFF;
				//MMCINFO("sdly-0 0x%x\n", mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM + f]);
			}

			val = p[spd_md*2 + 1];
			for (f=4; f<MAX_CLK_FREQ_NUM; f++) {
				mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM + f] = (val>>((f-4)*8)) & 0xFF;
				//MMCINFO("sdly-1 0x%x\n", mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM + f]);
			}
		}

	}

	return 0;
}

static int mmc_get_dly_manual_sample(int sdc_no)
{
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	struct mmc_config *cfg = &mmchost->cfg;
	int imd, ifreq;

	if (sdc_no == 2)
	{
		if (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_4)
		{
			struct sunxi_mmc_timing_mode4 *p = &mmchost->tm4;

			for (imd=0; imd<MAX_SPD_MD_NUM; imd++)
			{
				for (ifreq=0; ifreq<MAX_CLK_FREQ_NUM; ifreq++)
				{
					//MMCINFO("%d %d\n", cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq]);
					p->odly[imd*MAX_CLK_FREQ_NUM + ifreq] = cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq];
					if (imd == HS400) {
						p->dsdly[ifreq] = cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq];
					} else {
						p->sdly[imd*MAX_CLK_FREQ_NUM + ifreq] = cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq];
					}
				}
				//MMCINFO("\n");
			}
		}
		else
		{
			MMCINFO("sdc %d timing mode %d error --1\n", sdc_no, mmchost->timing_mode);
			return -1;
		}
	}
	else if (sdc_no == 0)
	{
		if (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_1)
		{
			struct sunxi_mmc_timing_mode1 *p = &mmchost->tm1;

			for (imd=0; imd<MAX_SPD_MD_NUM; imd++)
			{
				for (ifreq=0; ifreq<MAX_CLK_FREQ_NUM; ifreq++)
				{
					//MMCINFO("%d %d\n", cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq]);
					p->odly[imd*MAX_CLK_FREQ_NUM + ifreq] = cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq];
					p->sdly[imd*MAX_CLK_FREQ_NUM + ifreq] = cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq];
				}
				//MMCINFO("\n");
			}
		}
		else
		{
			MMCINFO("sdc %d timing mode %d error --2\n", sdc_no, mmchost->timing_mode);
			return -1;
		}

	}
	else
	{
		MMCINFO("mmc_get_dly_manual_sample: error sdc_no %d\n", sdc_no);
		return -1;
	}

	return 0;
}

static int mmc_update_timing_para(int sdc_no)
{
	int ret = 0;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	struct mmc_platform_caps *p = &mmchost->cfg.platform_caps;

	if (p->sample_mode == AUTO_SAMPLE_MODE)
	{
		ret = mmc_get_sdly_auto_sample(sdc_no);
		if (ret) {
			MMCINFO("update auto sample timing para fail!\n");
		}
	}
	else if (p->sample_mode == MAUNAL_SAMPLE_MODE)
	{
		ret = mmc_get_dly_manual_sample(sdc_no);
		if (ret) {
			MMCINFO("update manual sample timing para fail!\n");
		}
	}
	else
	{
		/* use driver's default parameter */
		if (sdc_no == 0)
		{
			mmchost->tm1.odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM + CLK_50M] =
				mmchost->tm1.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM + CLK_50M];
			mmchost->tm1.sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM + CLK_50M] =
				mmchost->tm1.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM + CLK_50M];
		}
		else
		{
			MMCINFO("mmc_update_timing_para: error sdc_no %d\n", sdc_no);
			ret = -1;
		}
	}

	return ret;
}


static void mmc_get_para_from_fex(int sdc_no)
{
	int rval, ret = 0;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	struct mmc_config *cfg = &mmchost->cfg;
	int nodeoffset=0;
	int i, imd, ifreq;
	char ctmp[30];


	if (sdc_no == 0)
	{
		nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_CARD0_BOOT_PARA);
		if(nodeoffset < 0 )
		{
			MMCINFO("get card0 para fail\n");
			return ;
		}
		if(fdt_set_all_pin(FDT_PATH_CARD0_BOOT_PARA,"pinctrl-0"))
		{
			MMCINFO("set card0 pin fail\n");
			return ;
		}
		/* set gpio */
		//gpio_request_simple("card0_boot_para", NULL);

		//ret = script_parser_fetch("card0_boot_para", "sdc_wipe", &rval, 1);
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_wipe", (uint32_t*)(&rval));
		if (ret < 0)
        	MMCDBG("get sdc_wipe fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_SECURE_WIPE) {
				MMCINFO("disable driver secure wipe operation.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_SECURE_WIPE;
			}
			if (rval & DRV_PARA_DISABLE_EMMC_SANITIZE) {
				MMCINFO("disable emmc sanitize feature.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SANITIZE;
			}
			if (rval & DRV_PARA_DISABLE_EMMC_SECURE_PURGE) {
				MMCINFO("disable emmc secure purge feature.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SECURE_PURGE;
			}
			if (rval & DRV_PARA_DISABLE_EMMC_TRIM) {
				MMCINFO("disable emmc trim feature.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_TRIM;
			}
		}

		//ret = script_parser_fetch("card0_boot_para", "sdc_erase", &rval, 1);
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_erase", (uint32_t*)(&rval));
		if (ret < 0)
			MMCDBG("get sdc0 sdc_erase fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_EMMC_ERASE) {
				MMCINFO("disable emmc erase.\n");
				cfg->platform_caps.drv_erase_feature |= DRV_PARA_DISABLE_EMMC_ERASE;
			}
			if (rval & DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE) {
				MMCINFO("enable emmc sanitize when erase.\n");
				cfg->platform_caps.drv_erase_feature |= DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE;
			}
		}

		//ret = script_parser_fetch("card0_boot_para", "sdc_boot", &rval, 1);
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_boot", (uint32_t*)(&rval));
		if (ret<0)
			MMCDBG("get sdc0 sdc_boot fail.\n");
		else {
			if (rval & DRV_PARA_NOT_BURN_USER_PART) {
				MMCINFO("don't burn boot0 to user part.\n");
				cfg->platform_caps.drv_burn_boot_pos |= DRV_PARA_NOT_BURN_USER_PART;
			}
			if (rval & DRV_PARA_BURN_EMMC_BOOT_PART) {
				MMCINFO("burn boot0 to emmc boot part.\n");
				cfg->platform_caps.drv_burn_boot_pos |= DRV_PARA_BURN_EMMC_BOOT_PART;
			}
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_odly_50M", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc0 sdc_odly_50M fail.\n");
			cfg->platform_caps.boot_odly_50M = 0xff;
		} else {
			MMCINFO("get sdc0 sdc_odly_50M %d.\n", rval);
			cfg->platform_caps.boot_odly_50M = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_sdly_50M", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc0 sdc_sdly_50M fail.\n");
			cfg->platform_caps.boot_sdly_50M = 0xff;
		} else {
			MMCINFO("get sdc0 sdc_sdly_50M %d.\n", rval);
			cfg->platform_caps.boot_sdly_50M = rval;

		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_odly_50M_ddr", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc0 sdc_odly_50M_ddr fail.\n");
			cfg->platform_caps.boot_odly_50M_ddr = 0xff;
		} else {
			MMCINFO("get sdc0 sdc_odly_50M_ddr %d.\n", rval);
			cfg->platform_caps.boot_odly_50M_ddr = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_sdly_50M_ddr", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc0 sdc_sdly_50M_ddr fail.\n");
			cfg->platform_caps.boot_sdly_50M_ddr = 0xff;
		} else {
			MMCINFO("get sdc0 sdc_sdly_50M_ddr %d.\n", rval);
			cfg->platform_caps.boot_sdly_50M_ddr = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_freq", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc0 sdc_freq fail.\n");
			cfg->platform_caps.boot_hs_f_max = 0x0;
		} else {
			if (rval >= 50)
				cfg->platform_caps.boot_hs_f_max = 50;
			else
				cfg->platform_caps.boot_hs_f_max = rval;
			MMCINFO("get sdc0 sdc_freq %d.%d\n", rval, cfg->platform_caps.boot_hs_f_max);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_b0p", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc0 sdc_b0p fail.\n");
			cfg->platform_caps.boot0_para = 0x0;
		} else {
			MMCINFO("get sdc0 sdc_b0p %d.\n", rval);
			cfg->platform_caps.boot0_para = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_ex_dly_used", (uint32_t*)(&rval));
		if (ret < 0) {
        	MMCDBG("get card0_boot_para:sdc_ex_dly_used fail\n");
        } else {
	        if (rval == 1) {  /* maual sample point from fex */
				cfg->platform_caps.sample_mode = MAUNAL_SAMPLE_MODE;
				MMCINFO("get sdc_ex_dly_used %d, use manual sdly in fex\n", rval);
			} else {
				cfg->platform_caps.sample_mode = 0x0;
				MMCINFO("undefined value %d, use default dly\n", rval);
			}
		}

		if (cfg->platform_caps.sample_mode == MAUNAL_SAMPLE_MODE)
		{
			for (imd=0; imd<MAX_SPD_MD_NUM; imd++)
			{
				for (ifreq=0; ifreq<MAX_CLK_FREQ_NUM; ifreq++)
				{
					sprintf(ctmp, "sdc_odly_%d_%d", imd, ifreq);
					ret = fdt_getprop_u32(working_fdt, nodeoffset, ctmp, (uint32_t*)(&rval));
					if (ret<0) {
						MMCDBG("get sdc0 %s fail.\n", ctmp);
						cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = 0x0;
					} else {
						cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = rval&0x1;
						MMCINFO("get sdc0 %s 0x%x for %d-%s %d.\n", ctmp,
							cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], imd, spd_name[imd], ifreq);
					}

					sprintf(ctmp, "sdc_sdly_%d_%d", imd, ifreq);
					ret = fdt_getprop_u32(working_fdt, nodeoffset, ctmp, (uint32_t*)(&rval));
					if (ret<0) {
						MMCDBG("get sdc0 %s fail.\n", ctmp);
						cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = 0xff;
					} else {
						cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = rval&0xff;
						MMCINFO("get sdc0 %s 0x%x for %d-%s %d.\n", ctmp,
							cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], imd, spd_name[imd], ifreq);
					}
				}
			}
		}

	}	else if (sdc_no == 2){

		nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_CARD2_BOOT_PARA);
		if(nodeoffset < 0 )
		{
			MMCINFO("get card0 para fail\n");
			return ;
		}
		if(fdt_set_all_pin(FDT_PATH_CARD2_BOOT_PARA,"pinctrl-0"))
		{
			MMCINFO("set card0 pin fail\n");
			return ;
		}
		//gpio_request_simple("card2_boot_para", NULL);
		//MMCINFO("=====~~~~~~~~~~~~0x%x 0x%x 0x%x\n", *(volatile u32 *)(0x1c20848), *(volatile u32 *)(0x1c2084C), *(volatile u32 *)(0x1c20850));

		//ret = script_parser_fetch("card2_boot_para", "sdc_wipe", &rval, 1);
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_wipe", (uint32_t*)(&rval));
		if (ret < 0)
        	MMCDBG("get sdc_wipe fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_SECURE_WIPE) {
				MMCINFO("disable driver secure wipe operation.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_SECURE_WIPE;
			}
			if (rval & DRV_PARA_DISABLE_EMMC_SANITIZE) {
				MMCINFO("disable emmc sanitize feature.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SANITIZE;
			}
			if (rval & DRV_PARA_DISABLE_EMMC_SECURE_PURGE) {
				MMCINFO("disable emmc secure purge feature.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SECURE_PURGE;
			}
			if (rval & DRV_PARA_DISABLE_EMMC_TRIM) {
				MMCINFO("disable emmc trim feature.\n");
				cfg->platform_caps.drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_TRIM;
			}
		}

		//ret = script_parser_fetch("card2_boot_para", "sdc_erase", &rval, 1);
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_erase", (uint32_t*)(&rval));
		if (ret < 0)
			MMCDBG("get sdc2 sdc_erase fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_EMMC_ERASE) {
				MMCINFO("disable emmc erase.\n");
				cfg->platform_caps.drv_erase_feature |= DRV_PARA_DISABLE_EMMC_ERASE;
			}
			if (rval & DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE) {
				MMCINFO("enable emmc sanitize when erase.\n");
				cfg->platform_caps.drv_erase_feature |= DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE;
			}
		}

		//ret = script_parser_fetch("card2_boot_para", "sdc_boot", &rval, 1);
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_boot", (uint32_t*)(&rval));
		if (ret<0)
			MMCDBG("get sdc2 sdc_boot fail.\n");
		else {
			if (rval & DRV_PARA_NOT_BURN_USER_PART) {
				MMCINFO("don't burn boot0 to user part.\n");
				cfg->platform_caps.drv_burn_boot_pos |= DRV_PARA_NOT_BURN_USER_PART;
			}
			if (rval & DRV_PARA_BURN_EMMC_BOOT_PART) {
				MMCINFO("burn boot0 to emmc boot part.\n");
				cfg->platform_caps.drv_burn_boot_pos |= DRV_PARA_BURN_EMMC_BOOT_PART;
			}
		}

		//ret = script_parser_fetch("card2_boot_para", "sdc_ex_dly_used", &rval, 1);
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_ex_dly_used", (uint32_t*)(&rval));
		if (ret < 0) {
        	MMCDBG("get card2_boot_para:sdc_ex_dly_used fail\n");

			/* fill invalid sample config */
			memset(mmchost->cfg.platform_caps.sdly.tm4_smx_fx,
				0xff, sizeof(mmchost->cfg.platform_caps.sdly.tm4_smx_fx));
        }
        //ret1 = script_parser_fetch("mmc2_para", "sdc_ex_dly_used", &rval_ker, 1);
        //if (ret1 < 0)
        //	MMCDBG("get mmc2_para:sdc_ex_dly_used fail\n");
		if (!(ret < 0)/* && !(ret1 < 0)*/)
		{
			int work_mode = uboot_spare_head.boot_data.work_mode;
			struct boot_sdmmc_private_info_t *priv_info =
				(struct boot_sdmmc_private_info_t *)(uboot_spare_head.boot_data.sdcard_spare_data);
			struct tune_sdly *sdly = &(priv_info->tune_sdly);
			u32 *p = (u32 *)&mmchost->cfg.platform_caps.sdly.tm4_smx_fx[0];

			if (/*(rval_ker == 2) &&*/ (rval == 2)) { //only when kernal use auto sample,uboot will use auto sample.
				cfg->platform_caps.sample_mode = AUTO_SAMPLE_MODE;
				MMCINFO("get sdc_ex_dly_used %d, use auto tuning sdly\n", rval);
				if (work_mode != WORK_MODE_BOOT) {
					/*usb product will auto get sample point, no need to get auto sdly, so first used default value*/
					MMCINFO("current is product mode, it will tune sdly later\n");
				} else {
					/* copy sample point cfg from uboot header to internal variable */
					if (sdly != NULL)
						memcpy(p, sdly, sizeof(struct tune_sdly));
					else
						MMCINFO("get sdly from uboot header fail\n");
				}
			} else if (rval == 1) {  /* maual sample point from fex */
				cfg->platform_caps.sample_mode = MAUNAL_SAMPLE_MODE;
				MMCINFO("get sdc_ex_dly_used %d, use manual sdly in fex\n", rval);
			} else {
				cfg->platform_caps.sample_mode = 0x0;
				MMCINFO("undefined value %d, use default dly\n", rval);
			}
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_force_boot_tuning", (uint32_t*)(&rval));
		if (ret < 0)
			MMCDBG("get card2_boot_para:sdc_force_boot_tuning fail\n");
        else {
			if (rval == 1) {
				MMCINFO("card2 force to execute tuning during boot.\n");
				cfg->platform_caps.force_boot_tuning = 1;
			} else {
				MMCDBG("card2 don't force to execute tuning during boot.\n");
				cfg->platform_caps.force_boot_tuning = 0;
			}
        }

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_io_1v8", (uint32_t*)(&rval));
		if (ret < 0)
        	MMCDBG("get card2_boot_para:sdc_io_1v8 fail\n");
        else {
			if (rval == 1) {
				MMCINFO("card2 io is 1.8V.\n");
				cfg->platform_caps.io_is_1v8 = 1;
			} else {
				MMCDBG("card2 io is 3V.\n");
			}
        }

        ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_odly_50M", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_odly_50M fail.\n");
			cfg->platform_caps.boot_odly_50M = 0xff;
		} else {
			MMCINFO("get sdc2 sdc_odly_50M %d.\n", rval);
			cfg->platform_caps.boot_odly_50M = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_sdly_50M", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_sdly_50M fail.\n");
			cfg->platform_caps.boot_sdly_50M = 0xff;
		} else {
			MMCINFO("get sdc2 sdc_sdly_50M %d.\n", rval);
			cfg->platform_caps.boot_sdly_50M = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_odly_50M_ddr", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_odly_50M_ddr fail.\n");
			cfg->platform_caps.boot_odly_50M_ddr = 0xff;
		} else {
			MMCINFO("get sdc2 sdc_odly_50M_ddr %d.\n", rval);
			cfg->platform_caps.boot_odly_50M_ddr = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_sdly_50M_ddr", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_sdly_50M_ddr fail.\n");
			cfg->platform_caps.boot_sdly_50M_ddr = 0xff;
		} else {
			MMCINFO("get sdc2 sdc_sdly_50M_ddr %d.\n", rval);
			cfg->platform_caps.boot_sdly_50M_ddr = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_freq", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_freq fail.\n");
			cfg->platform_caps.boot_hs_f_max = 0x0;
		} else {
			if (rval >= 50)
				cfg->platform_caps.boot_hs_f_max = 50;
			else
				cfg->platform_caps.boot_hs_f_max = rval;
			MMCINFO("get sdc2 sdc_freq %d.%d\n", rval, cfg->platform_caps.boot_hs_f_max);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_b0p", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_b0p fail.\n");
			cfg->platform_caps.boot0_para = 0x0;
		} else {
			cfg->platform_caps.boot0_para = rval;
			MMCINFO("get sdc2 sdc_b0p %d.\n", cfg->platform_caps.boot0_para);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_win_th", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_tm4_win_th fail.\n");
			cfg->platform_caps.tm4_timing_window_th = 12;
		} else {
			if ((rval<4) || (rval>64))
				cfg->platform_caps.tm4_timing_window_th = 12;
			else
				cfg->platform_caps.tm4_timing_window_th = rval;
			MMCINFO("get sdc2 sdc_tm4_win_th %d.\n", cfg->platform_caps.tm4_timing_window_th);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_r_cycle", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_tm4_r_cycle fail.\n");
			cfg->platform_caps.tm4_tune_r_cycle= 0x0;
		} else {
			if ((rval<1) || (rval>40))
				cfg->platform_caps.tm4_tune_r_cycle = 15;
			else
				cfg->platform_caps.tm4_tune_r_cycle = rval;
			MMCINFO("get sdc2 sdc_tm4_r_cycle %d.\n", cfg->platform_caps.tm4_tune_r_cycle);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_hs200_max_freq", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_tm4_hs200_max_freq fail.\n");
			cfg->platform_caps.tm4_tune_hs200_max_freq = 0x0;
		} else {
			if ((rval<50) || (rval>200))
				cfg->platform_caps.tm4_tune_hs200_max_freq = 0x0;
			else
				cfg->platform_caps.tm4_tune_hs200_max_freq = rval;
			MMCINFO("get sdc2 sdc_tm4_hs200_max_freq %d.\n", cfg->platform_caps.tm4_tune_hs200_max_freq);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_hs400_max_freq", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_tm4_hs400_max_freq fail.\n");
			cfg->platform_caps.tm4_tune_hs400_max_freq = 0x0;
		} else {
			if ((rval<50) || (rval>200))
				cfg->platform_caps.tm4_tune_hs400_max_freq = 0x0;
			else
				cfg->platform_caps.tm4_tune_hs400_max_freq = rval;
			MMCINFO("get sdc2 sdc_tm4_hs400_max_freq %d.\n", cfg->platform_caps.tm4_tune_hs400_max_freq);
		}

		for (i=0; i<MAX_EXT_FREQ_POINT_NUM; i++)
		{
			sprintf(ctmp, "sdc_tm4_ext_freq_%d", i);
			ret = fdt_getprop_u32(working_fdt,nodeoffset, ctmp, (uint32_t*)(&rval));
			if (ret<0) {
				MMCDBG("get sdc2 %s fail.\n", ctmp);
				cfg->platform_caps.tm4_tune_ext_freq[i] = 0x0;
			} else {
				MMCINFO("get sdc2 %s 0x%x.\n", ctmp, rval);
				if ((rval & 0xff) >= 25)
					cfg->platform_caps.tm4_tune_ext_freq[i] = rval;
				else {
					cfg->platform_caps.tm4_tune_ext_freq[i] = 0x0;
					MMCINFO("invalid freq %d MHz, discard this ext freq point\n", (rval & 0xff));
				}
			}
		}

		if (cfg->platform_caps.sample_mode == MAUNAL_SAMPLE_MODE)
		{
			for (imd=0; imd<MAX_SPD_MD_NUM; imd++)
			{
				for (ifreq=0; ifreq<MAX_CLK_FREQ_NUM; ifreq++)
				{
					sprintf(ctmp, "sdc_odly_%d_%d", imd, ifreq);
					ret = fdt_getprop_u32(working_fdt, nodeoffset, ctmp, (uint32_t*)(&rval));
					if (ret<0) {
						MMCDBG("get sdc2 %s fail.\n", ctmp);
						cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = 0x0;
					} else {
						cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = rval&0x1;
						MMCINFO("get sdc2 %s 0x%x for %d-%s %d.\n", ctmp,
							cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], imd, spd_name[imd], ifreq);
					}

					sprintf(ctmp, "sdc_sdly_%d_%d", imd, ifreq);
					ret = fdt_getprop_u32(working_fdt, nodeoffset, ctmp, (uint32_t*)(&rval));
					if (ret<0) {
						MMCDBG("get sdc2 %s fail.\n", ctmp);
						cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = 0xff;
					} else {
						cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = rval&0xff;
						MMCINFO("get sdc2 %s 0x%x for %d-%s %d.\n", ctmp,
							cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], imd, spd_name[imd], ifreq);
					}
				}
			}
		}

		/* speed mode caps */
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_dis_host_caps", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_dis_host_caps fail.\n");
			cfg->platform_caps.host_caps_mask = 0x0;
		} else {
			cfg->platform_caps.host_caps_mask = rval;
			MMCINFO("get sdc2 sdc_dis_host_caps 0x%x.\n", cfg->platform_caps.host_caps_mask);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_kernel_no_limit", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc2 sdc_kernel_no_limit fail.\n");
			cfg->platform_caps.tune_limit_kernel_timing = 0x1;
		} else {
			if (rval == 1)
				cfg->platform_caps.tune_limit_kernel_timing = 0x0;
			else
				cfg->platform_caps.tune_limit_kernel_timing = 0x1;
			MMCINFO("get sdc2 sdc_kernel_no_limit 0x%x, limit 0x%x.\n", rval, cfg->platform_caps.tune_limit_kernel_timing);
		}

		/* fmax, fmax_ddr */
	}
	else {
		MMCINFO("%s: input sdc_no error: %d\n", __FUNCTION__, sdc_no);
	}

	return ;
}

static void mmc_get_para_from_dtb(int sdc_no)
{
	int ret = 0;
	//struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	//struct mmc_config *cfg = &mmchost->cfg;
	int nodeoffset;
	char prop_path[128] = {0};
	//u32 prop_val = 0;

	if (sdc_no == 0)
	{
		strcpy(prop_path, "mmc0");
		nodeoffset = fdt_path_offset(working_fdt, prop_path);
		if (nodeoffset < 0) {
			MMCINFO("can't find node \"%s\",will add new node\n", prop_path);
			goto __ERRRO_END;
		}
	}
	else
	{

	}

	return ;

__ERRRO_END:
	printf ("fdt err returned %s\n", fdt_strerror(ret));
	return ;
}

#define	 MMC_REG_FIFO_OS	(0x200)

static int mmc_resource_init(int sdc_no)
{
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	MMCDBG("init mmc %d resource\n", sdc_no);
	switch (sdc_no) {
		case 0:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_SMHC0_BASE;
			mmchost->database = SUNXI_SMHC0_BASE + MMC_REG_FIFO_OS;
			mmchost->mclkbase = CCMU_SDMMC0_CLK_REG;
			break;
		case 1:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_SMHC1_BASE;
			mmchost->database = SUNXI_SMHC1_BASE + MMC_REG_FIFO_OS;
			mmchost->mclkbase = CCMU_SDMMC1_CLK_REG;
			break;
		case 2:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_SMHC2_BASE;
			mmchost->database = SUNXI_SMHC2_BASE + MMC_REG_FIFO_OS;
			mmchost->mclkbase = CCMU_SDMMC2_CLK_REG;
			break;
		default:
			MMCINFO("Wrong mmc number %d\n", sdc_no);
			break;
	}

	mmchost->hclkbase = CCMU_BUS_CLK_GATING_REG0;
	mmchost->hclkrst  = CCMU_BUS_SOFT_RST_REG0;

	mmchost->mmc_no = sdc_no;

	return 0;
}

int mmc_clk_io_onoff(int sdc_no, int onoff, int reset_clk)
{
	int rval;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];

	/* config ahb clock */
	if (onoff)
	{
		rval = readl(mmchost->hclkrst);
		rval |= (1 << (8 + sdc_no));
		writel(rval, mmchost->hclkrst);
		rval = readl(mmchost->hclkbase);
		rval |= (1 << (8 + sdc_no));
		writel(rval, mmchost->hclkbase);

		rval = readl(mmchost->mclkbase);
		rval |= (1U<<31);
		writel(rval, mmchost->mclkbase);
	}
	else
	{

		rval = readl(mmchost->mclkbase);
		rval &= ~(1U<<31);
		writel(rval, mmchost->mclkbase);

		rval = readl(mmchost->hclkbase);
		rval &= ~(1 << (8 + sdc_no));
		writel(rval, mmchost->hclkbase);

		rval = readl(mmchost->hclkrst);
		rval &= ~ (1 << (8 + sdc_no));
		writel(rval, mmchost->hclkrst);
	}

	/* config mod clock */
	if (reset_clk)
	{
		rval = readl(mmchost->mclkbase);
		/*set to 24M default value*/
		rval &= ~(0x7fffffff);
		writel(rval, mmchost->mclkbase);
		mmchost->mod_clk = 24000000;
	}

	//dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
	//dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
	//dumphex32("mmc", (char*)mmchost->reg, 0x100);

	return 0;
}

int sunxi_mmc_init(int sdc_no)
{
	struct sunxi_mmc_host *host = NULL;

	MMCINFO("mmc driver ver %s\n", DRIVER_VER);

	if ((sdc_no != 2) && (sdc_no != 0)) {
		MMCINFO("sdc_on error, %d\n", sdc_no);
		return -1;
	}

	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
	host = &mmc_host[sdc_no];
	memset(&mmc_host_reg_bak[sdc_no], 0, sizeof(struct mmc_reg_v4p1));
	host->reg_bak =  &mmc_host_reg_bak[sdc_no];

	if ((sdc_no == 2)) {
		host->cfg.platform_caps.odly_spd_freq = &ext_odly_spd_freq[0];
		host->cfg.platform_caps.sdly_spd_freq = &ext_sdly_spd_freq[0];
	} else if (sdc_no == 0) {
		host->cfg.platform_caps.odly_spd_freq = &ext_odly_spd_freq_sdc0[0];
		host->cfg.platform_caps.sdly_spd_freq = &ext_sdly_spd_freq_sdc0[0];
	}

	host->cfg.name = "SUNXI SD/MMC";
	host->cfg.host_no = sdc_no;
	host->cfg.ops = &sunxi_mmc_ops;

	host->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34
		| MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30
		| MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_34_35
		| MMC_VDD_35_36;

	host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	if (sdc_no == 0) {
		host->cfg.f_min = 400000;
#ifdef FPGA_PLATFORM
		host->cfg.f_max = 12000000;
#else
		host->cfg.f_max = 50000000;
#endif
	} else if ((sdc_no == 2)) {
		host->cfg.f_min = 400000;
#ifdef FPGA_PLATFORM
		host->cfg.f_max = 12000000;
#else
		host->cfg.f_max = 200000000;
#endif
	}

	if ((sdc_no == 0) || (sdc_no == 1))
		host->timing_mode = SUNXI_MMC_TIMING_MODE_1; //SUNXI_MMC_TIMING_MODE_3
	else if ((sdc_no == 2))
		host->timing_mode = SUNXI_MMC_TIMING_MODE_4;

	host->pdes = malloc(256 * 1024);
	if (host->pdes == NULL) {
		MMCINFO("%s: get mem for descriptor failed\n", __FUNCTION__);
		return -1;
	}

	mmc_clear_timing_para(sdc_no);
	mmc_init_default_timing_para(sdc_no);
	mmc_get_para_from_fex(sdc_no);
	mmc_get_para_from_dtb(sdc_no);
	mmc_update_timing_para(sdc_no);
	mmc_resource_init(sdc_no);
	mmc_clk_io_onoff(sdc_no, 1, 1);

	host->cfg.host_caps = MMC_MODE_4BIT | MMC_MODE_HS | MMC_MODE_HC \
							| MMC_MODE_HS_52MHz | MMC_MODE_DDR_52MHz;
	if (host->cfg.host_no == 2)
		host->cfg.host_caps |= MMC_MODE_8BIT;
	if (host->cfg.platform_caps.io_is_1v8) {
		host->cfg.host_caps |= MMC_MODE_HS200;
		if (host->cfg.host_caps & MMC_MODE_8BIT)
			host->cfg.host_caps |= MMC_MODE_HS400;
	}

	if (host->cfg.platform_caps.host_caps_mask) {
		u32 mask = host->cfg.platform_caps.host_caps_mask;
		if (mask & DRV_PARA_DISABLE_MMC_MODE_HS400)
			host->cfg.host_caps &= (~DRV_PARA_DISABLE_MMC_MODE_HS400);
		if (mask & DRV_PARA_DISABLE_MMC_MODE_HS200)
			host->cfg.host_caps &= (~(DRV_PARA_DISABLE_MMC_MODE_HS200
										| DRV_PARA_DISABLE_MMC_MODE_HS400));
		if (mask & DRV_PARA_DISABLE_MMC_MODE_DDR_52MHz)
			host->cfg.host_caps &= (~(DRV_PARA_DISABLE_MMC_MODE_DDR_52MHz
										| DRV_PARA_DISABLE_MMC_MODE_HS400
										| DRV_PARA_DISABLE_MMC_MODE_HS200));
		if (mask & DRV_PARA_DISABLE_MMC_MODE_HS_52MHz)
			host->cfg.host_caps &= (~(DRV_PARA_DISABLE_MMC_MODE_HS_52MHz
										| DRV_PARA_DISABLE_MMC_MODE_DDR_52MHz
										| DRV_PARA_DISABLE_MMC_MODE_HS400
										| DRV_PARA_DISABLE_MMC_MODE_HS200));
		if (mask & DRV_PARA_DISABLE_MMC_MODE_8BIT)
			host->cfg.host_caps &= (~(DRV_PARA_DISABLE_MMC_MODE_8BIT
										| DRV_PARA_DISABLE_MMC_MODE_HS400));
	}
	MMCDBG("mmc->host_caps %x\n", host->cfg.host_caps);

	host->mmc = mmc_create(&host->cfg, host);
	if (host->mmc == NULL) {
		MMCINFO("%s: register mmc %d failed\n", __FUNCTION__, sdc_no);
		return -1;
	} else
		MMCDBG("%s: register mmc %d ok\n", __FUNCTION__, __LINE__);

	return 0;
}

int sunxi_mmc_exit(int sdc_no)
{
	mmc_clk_io_onoff(sdc_no, 0, 1);
	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
	memset(&mmc_host_reg_bak[sdc_no], 0, sizeof(struct mmc_reg_v4p1));
	MMCDBG("sunxi mmc%d exit\n", sdc_no);
	return 0;
}
