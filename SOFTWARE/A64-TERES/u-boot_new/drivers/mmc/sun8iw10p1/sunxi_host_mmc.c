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
#include "sunxi_host0_3_v4p1.h"
#include "sunxi_host2_v5p1.h"
#include "sunxi_host_mmc.h"

extern char *spd_name[];
struct sunxi_mmc_host mmc_host[4];

struct mmc_reg_v4p1 mmc_host_reg_bak[4];
struct mmc2_reg_v5p1 mmc2_host_reg_bak;

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

			mmchost->tm2.odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm2.sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm2.def_odly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
			mmchost->tm2.def_sdly[i*MAX_CLK_FREQ_NUM+j] = 0xFF;
		}
	}

	for (j=0; j<MAX_CLK_FREQ_NUM; j++)
	{
		mmchost->tm4.dsdly[j] = 0xFF;
		mmchost->tm4.def_dsdly[j] = 0xFF;

		mmchost->tm2.dsdly[j] = 0xFF;
		mmchost->tm2.def_dsdly[j] = 0xFF;
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
	} else if (sdc_no == 3) {
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
	}else if (sdc_no == 2) {
		/* timing mode 2 */
		mmchost->tm2.cur_spd_md = DS26_SDR12;
		mmchost->tm2.cur_freq = CLK_400K;
		mmchost->tm2.sample_point_cnt = MMC_CLK_SAMPLE_POINIT_MODE_2;
		mmchost->tm2.sample_point_cnt_hs400 = MMC_CLK_SAMPLE_POINIT_MODE_2_HS400;

		mmchost->tm2.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_400K] = 0x0;
		mmchost->tm2.def_odly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[DS26_SDR12*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;

		mmchost->tm2.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_400K] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_400K] = 0;
		mmchost->tm2.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_25M] = 0;
		mmchost->tm2.def_odly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HSSDR52_SDR25*MAX_CLK_FREQ_NUM+CLK_50M] = 0;

		mmchost->tm2.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_400K] = TM1_OUT_PH90;
		mmchost->tm2.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_400K] = 0x0;
		mmchost->tm2.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = TM1_OUT_PH90;
		mmchost->tm2.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm2.def_odly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = TM1_OUT_PH90;
		mmchost->tm2.def_sdly[HSDDR52_DDR50*MAX_CLK_FREQ_NUM+CLK_50M] = 0x0;

		mmchost->tm2.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm2.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_50M] = 0x0;
		mmchost->tm2.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_100M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_100M] = 0xFF;
		mmchost->tm2.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_150M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_150M] = 0xFF;
		mmchost->tm2.def_odly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_200M] = TM4_OUT_PH180;
		mmchost->tm2.def_sdly[HS200_SDR104*MAX_CLK_FREQ_NUM+CLK_200M] = 0xFF;

		mmchost->tm2.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_25M] = TM4_OUT_PH90;
		mmchost->tm2.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_25M] = 0x0;
		mmchost->tm2.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_50M] = TM4_OUT_PH90;
		mmchost->tm2.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_50M] = 0x0;
		mmchost->tm2.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_100M] = TM4_OUT_PH90;
		mmchost->tm2.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_100M] = 0xFF;
		mmchost->tm2.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_150M] = TM4_OUT_PH90;
		mmchost->tm2.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_150M] = 0xFF;
		mmchost->tm2.def_odly[HS400*MAX_CLK_FREQ_NUM+CLK_200M] = TM4_OUT_PH90;
		mmchost->tm2.def_sdly[HS400*MAX_CLK_FREQ_NUM+CLK_200M] = 0xFF;

		mmchost->tm2.def_dsdly[CLK_25M] = 0x0;
		mmchost->tm2.def_dsdly[CLK_50M] = 0x18;
		mmchost->tm2.def_dsdly[CLK_100M] = 0xFF;
		mmchost->tm2.def_dsdly[CLK_150M] = 0xFF;
		mmchost->tm2.def_dsdly[CLK_200M] = 0xFF;
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

	if ((sdc_no != 3) && (sdc_no != 2)) {
		MMCINFO("%s: don't support auto sample, %d\n", __FUNCTION__, sdc_no);
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
			/* update ds delay config */
			val = p[spd_md*2 + 0];
			for (f=0; f<4; f++) {
				if (sdc_no == 3)
					mmchost->tm4.dsdly[f] = (val>>(f*8)) & 0xFF;
				else if (sdc_no == 2)
					mmchost->tm2.dsdly[f] = (val>>(f*8)) & 0xFF;
				//MMCINFO("dsdly-0 0x%x\n", mmchost->tm4.dsdly[f]);
			}

			val = p[spd_md*2 + 1];
			for (f=4; f<MAX_CLK_FREQ_NUM; f++) {
				if (sdc_no == 3)
					mmchost->tm4.dsdly[f] = (val>>((f-4)*8)) & 0xFF;
				else if (sdc_no == 2)
					mmchost->tm2.dsdly[f] = (val>>((f-4)*8)) & 0xFF;
				//MMCINFO("dsdly-1 0x%x\n", mmchost->tm4.dsdly[f]);
			}

			/* update sample delay config */
			val = p[spd_md*2 + 2 + 0];
			for (f=0; f<4; f++) {
				if (sdc_no == 3)
					mmchost->tm4.sdly[f] = (val>>(f*8)) & 0xFF;
				else if (sdc_no == 2)
					mmchost->tm2.sdly[f] = (val>>(f*8)) & 0xFF;
				//MMCINFO("dsdly-0 0x%x\n", mmchost->tm4.dsdly[f]);
			}

			val = p[spd_md*2 + 2 + 1];
			for (f=4; f<MAX_CLK_FREQ_NUM; f++) {
				if (sdc_no == 3)
					mmchost->tm4.sdly[f] = (val>>((f-4)*8)) & 0xFF;
				else if (sdc_no == 2)
					mmchost->tm2.sdly[f] = (val>>((f-4)*8)) & 0xFF;
				//MMCINFO("dsdly-1 0x%x\n", mmchost->tm4.dsdly[f]);
			}
		}
		else
		{
			val = p[spd_md*2 + 0];
			for (f=0; f<4; f++) {
				if (sdc_no == 3)
					mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM + f] = (val>>(f*8)) & 0xFF;
				else if (sdc_no == 2)
					mmchost->tm2.sdly[spd_md*MAX_CLK_FREQ_NUM + f] = (val>>(f*8)) & 0xFF;
				//MMCINFO("sdly-0 0x%x\n", mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM + f]);
			}

			val = p[spd_md*2 + 1];
			for (f=4; f<MAX_CLK_FREQ_NUM; f++) {
				if (sdc_no == 3)
					mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM + f] = (val>>((f-4)*8)) & 0xFF;
				else if (sdc_no == 2)
					mmchost->tm2.sdly[spd_md*MAX_CLK_FREQ_NUM + f] = (val>>((f-4)*8)) & 0xFF;
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

	if ((sdc_no == 2) || (sdc_no == 3))
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
			MMCINFO("sdc %d timing mode %d error\n", sdc_no, mmchost->timing_mode);
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
			MMCINFO("sdc %d timing mode %d error\n", sdc_no, mmchost->timing_mode);
			return -1;
		}

	}
	else
	{
		MMCINFO("error sdc_no %d\n", sdc_no);
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

		}
	}

	return ret;
}

/*
int mmc_add_gpio_for_sdmmc(int sdc_no, void* gpio_buffer)
{
	user_gpio_set_t pin_set[32];
	int pin_count = 0;
	int ret = 0;
	int nodeoffset;
	int i = 0;

	normal_gpio_cfg *sdmmc_gpio = (normal_gpio_cfg *)gpio_buffer;

	if (sdc_no == 3)
	{
		nodeoffset = fdt_path_offset(working_fdt, FDT_PATH_CARD3_BOOT_PARA);
		if (nodeoffset < 0)
		{
			MMCINFO("%s:get proverty value [%s] fail\n", __func__, FDT_PATH_CARD3_BOOT_PARA);
			return -1;
		}
	}
	else if (sdc_no == 2)
	{
		nodeoffset = fdt_path_offset(working_fdt, FDT_PATH_CARD2_BOOT_PARA);
		if (nodeoffset < 0)
		{
			MMCINFO("%s:get proverty value [%s] fail\n", __func__, FDT_PATH_CARD2_BOOT_PARA);
			return -1;
		}
	}
	else
	{
		MMCINFO("%s: input sdc_no error %d\n", __func__, sdc_no);
		return -1;
	}


	pin_count = fdt_get_all_pin(nodeoffset, "pinctrl-0", pin_set);
	if (pin_count < 0)
	{
		MMCINFO("%s:fdt_get_all_pin() err\n", __func__);
		return -1;
	}

	for (i = 0; i < pin_count; i++)
	{
		sdmmc_gpio->port = pin_set[i].port;
		sdmmc_gpio->port_num= pin_set[i].port_num;
		sdmmc_gpio->mul_sel= pin_set[i].mul_sel;
		sdmmc_gpio->pull= pin_set[i].pull;
		sdmmc_gpio->drv_level= pin_set[i].drv_level;
		sdmmc_gpio->data= pin_set[i].data;
		sdmmc_gpio++;
	}
	for(i = 0; i < pin_count; i++)
	{
		MMCDBG(" port = %d,portnum=%d,mul_sel=%d,pull=%d drive= %d, data=%d\n",
			sdmmc_gpio[i].port,
			sdmmc_gpio[i].port_num,
			sdmmc_gpio[i].mul_sel,
			sdmmc_gpio[i].pull,
			sdmmc_gpio[i].drv_level,
			sdmmc_gpio[i].data);
	}

	return ret;
}
*/

static int mmc_get_sdc_phy_no(int sdc_no)
{
	char *cp = NULL;
	int nodeoffset=0;
	int ret, rval = -1;

	if (sdc_no == 0)
		cp = FDT_PATH_CARD0_BOOT_PARA;
	else if (sdc_no == 2)
		cp = FDT_PATH_CARD2_BOOT_PARA;
	else if (sdc_no == 3)
		cp = FDT_PATH_CARD3_BOOT_PARA;
	else {
		MMCINFO("%s: input sdc_no error, %d", __FUNCTION__, sdc_no);
		return -1;
	}

	nodeoffset =  fdt_path_offset(working_fdt, cp);
	if (nodeoffset < 0)
	{
		MMCINFO("%s: get card%d para fail\n", __FUNCTION__, sdc_no);
		return -1;
	}

	ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_phy_no", (uint32_t*)(&rval));
	if (ret < 0) {
		MMCINFO("%s: get card%d sdc_phy_no fail.\n", __FUNCTION__, sdc_no);
	}

	return rval;
}

static void _get_para_from_fex_sdc0(int sdc_no)
{
	int rval, ret = 0;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	struct mmc_config *cfg = &mmchost->cfg;
	int nodeoffset=0;
	int imd, ifreq;
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

	}else {
		MMCINFO("%s: input sdc_no error: %d\n", __FUNCTION__, sdc_no);
	}

	return ;
}

/*
  * note:
  * FDT_PATH_CARD3_BOOT_PARA and FDT_PATH_CARD2_BOOT_PARA are same.
  * they are "/soc/card2_boot_para".
  * the config of mmc2 and mmc3 should be both placed under [card2_boot_para] in sys_config.fex.
  * if want to use mmc2, the items of [card2_boot_para] should match to mmc2.
  * if want to use mmc3, the items of [card2_boot_para] should match to mmc3.
  */
static void _get_para_from_fex_sdc2_3(int sdc_no)
{
	int rval, ret = 0;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	struct mmc_config *cfg = &mmchost->cfg;
	int nodeoffset=0;
	int i, imd, ifreq;
	char ctmp[30];

	MMCDBG("%s: sdc_no %d\n", __FUNCTION__, sdc_no);

	if ((3 == sdc_no) || (2 == sdc_no))
	{
		if (3 == sdc_no) {
			nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_CARD3_BOOT_PARA);
			if(nodeoffset < 0 )
			{
				MMCINFO("get card%d para fail\n", sdc_no);
				return ;
			}
			if(fdt_set_all_pin(FDT_PATH_CARD3_BOOT_PARA,"pinctrl-0"))
			{
				MMCINFO("set card%d pin fail\n", sdc_no);
				return ;
			}
		} else {
			nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_CARD2_BOOT_PARA);
			if(nodeoffset < 0 )
			{
				MMCINFO("get card%d para fail\n", sdc_no);
				return ;
			}
			if(fdt_set_all_pin(FDT_PATH_CARD2_BOOT_PARA,"pinctrl-0"))
			{
				MMCINFO("set card%d pin fail\n", sdc_no);
				return ;
			}
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_wipe", (uint32_t*)(&rval));
		if (ret < 0)
			MMCDBG("get sdc%d sdc_wipe fail.\n", sdc_no);
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

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_erase", (uint32_t*)(&rval));
		if (ret < 0)
			MMCDBG("get sdc%d sdc_erase fail.\n", sdc_no);
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

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_boot", (uint32_t*)(&rval));
		if (ret<0)
			MMCDBG("get sdc%d sdc_boot fail.\n", sdc_no);
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

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_ex_dly_used", (uint32_t*)(&rval));
		if (ret < 0) {
			MMCDBG("get card%d_boot_para:sdc_ex_dly_used fail\n", sdc_no);

			/* fill invalid sample config */
			memset(mmchost->cfg.platform_caps.sdly.tm4_smx_fx,
				0xff, sizeof(mmchost->cfg.platform_caps.sdly.tm4_smx_fx));
		}

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
			MMCDBG("get card%d_boot_para:sdc_io_1v8 fail\n", sdc_no);
		else {
			if (rval == 1) {
				MMCINFO("card%d io is 1.8V.\n", sdc_no);
				cfg->platform_caps.io_is_1v8 = 1;
			} else {
				MMCDBG("card%d io is 3V.\n", sdc_no);
			}
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_odly_50M", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_odly_50M fail.\n", sdc_no);
			cfg->platform_caps.boot_odly_50M = 0xff;
		} else {
			MMCINFO("get sdc%d sdc_odly_50M %d.\n", sdc_no, rval);
			cfg->platform_caps.boot_odly_50M = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_sdly_50M", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_sdly_50M fail.\n", sdc_no);
			cfg->platform_caps.boot_sdly_50M = 0xff;
		} else {
			MMCINFO("get sdc%d sdc_sdly_50M %d.\n", sdc_no, rval);
			cfg->platform_caps.boot_sdly_50M = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_odly_50M_ddr", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_odly_50M_ddr fail.\n", sdc_no);
			cfg->platform_caps.boot_odly_50M_ddr = 0xff;
		} else {
			MMCINFO("get sdc%d sdc_odly_50M_ddr %d.\n", sdc_no, rval);
			cfg->platform_caps.boot_odly_50M_ddr = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_sdly_50M_ddr", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_sdly_50M_ddr fail.\n", sdc_no);
			cfg->platform_caps.boot_sdly_50M_ddr = 0xff;
		} else {
			MMCINFO("get sdc%d sdc_sdly_50M_ddr %d.\n", sdc_no, rval);
			cfg->platform_caps.boot_sdly_50M_ddr = rval;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_freq", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_freq fail.\n", sdc_no);
			cfg->platform_caps.boot_hs_f_max = 0x0;
		} else {
			if (rval >= 50)
				cfg->platform_caps.boot_hs_f_max = 50;
			else
				cfg->platform_caps.boot_hs_f_max = rval;
			MMCINFO("get sdc%d sdc_freq %d.%d\n", sdc_no, rval, cfg->platform_caps.boot_hs_f_max);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_b0p", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_b0p fail.\n", sdc_no);
			cfg->platform_caps.boot0_para = 0x0;
		} else {
			cfg->platform_caps.boot0_para = rval;
			MMCINFO("get sdc%d sdc_b0p %d.\n", sdc_no, cfg->platform_caps.boot0_para);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_win_th", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_tm4_win_th fail.\n", sdc_no);
			cfg->platform_caps.tm4_timing_window_th = 12;
		} else {
			if ((rval<4) || (rval>64))
				cfg->platform_caps.tm4_timing_window_th = 12;
			else
				cfg->platform_caps.tm4_timing_window_th = rval;
			MMCINFO("get sdc%d sdc_tm4_win_th %d.\n", sdc_no, cfg->platform_caps.tm4_timing_window_th);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_r_cycle", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_tm4_r_cycle fail.\n", sdc_no);
			cfg->platform_caps.tm4_tune_r_cycle= 0x0;
		} else {
			if ((rval<1) || (rval>40))
				cfg->platform_caps.tm4_tune_r_cycle = 15;
			else
				cfg->platform_caps.tm4_tune_r_cycle = rval;
			MMCINFO("get sdc%d sdc_tm4_r_cycle %d.\n", sdc_no, cfg->platform_caps.tm4_tune_r_cycle);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_hs200_max_freq", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_tm4_hs200_max_freq fail.\n", sdc_no);
			cfg->platform_caps.tm4_tune_hs200_max_freq = 0x0;
		} else {
			if ((rval<50) || (rval>200))
				cfg->platform_caps.tm4_tune_hs200_max_freq = 0x0;
			else
				cfg->platform_caps.tm4_tune_hs200_max_freq = rval;
			MMCINFO("get sdc%d sdc_tm4_hs200_max_freq %d.\n", sdc_no, cfg->platform_caps.tm4_tune_hs200_max_freq);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_tm4_hs400_max_freq", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_tm4_hs400_max_freq fail.\n", sdc_no);
			cfg->platform_caps.tm4_tune_hs400_max_freq = 0x0;
		} else {
			if ((rval<50) || (rval>200))
				cfg->platform_caps.tm4_tune_hs400_max_freq = 0x0;
			else
				cfg->platform_caps.tm4_tune_hs400_max_freq = rval;
			MMCINFO("get sdc%d sdc_tm4_hs400_max_freq %d.\n", sdc_no, cfg->platform_caps.tm4_tune_hs400_max_freq);
		}

		for (i=0; i<MAX_EXT_FREQ_POINT_NUM; i++)
		{
			sprintf(ctmp, "sdc_tm4_ext_freq_%d", i);
			ret = fdt_getprop_u32(working_fdt,nodeoffset, ctmp, (uint32_t*)(&rval));
			if (ret<0) {
				MMCDBG("get sdc%d %s fail.\n", sdc_no, ctmp);
				cfg->platform_caps.tm4_tune_ext_freq[i] = 0x0;
			} else {
				MMCINFO("get sdc%d %s 0x%x.\n", sdc_no, ctmp, rval);
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
						MMCDBG("get sdc%d %s fail.\n", sdc_no, ctmp);
						cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = 0x0;
					} else {
						cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = rval&0x1;
						MMCINFO("get sdc%d %s 0x%x for %d-%s %d.\n", sdc_no, ctmp,
							cfg->platform_caps.odly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], imd, spd_name[imd], ifreq);
					}

					sprintf(ctmp, "sdc_sdly_%d_%d", imd, ifreq);
					ret = fdt_getprop_u32(working_fdt, nodeoffset, ctmp, (uint32_t*)(&rval));
					if (ret<0) {
						MMCDBG("get sdc%d %s fail.\n", sdc_no, ctmp);
						cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = 0xff;
					} else {
						cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq] = rval&0xff;
						MMCINFO("get sdc%d %s 0x%x for %d-%s %d.\n", sdc_no, ctmp,
							cfg->platform_caps.sdly_spd_freq[imd*MAX_CLK_FREQ_NUM + ifreq], imd, spd_name[imd], ifreq);
					}
				}
			}
		}

		/* speed mode caps */
		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_dis_host_caps", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_dis_host_caps fail.\n", sdc_no);
			cfg->platform_caps.host_caps_mask = 0x0;
		} else {
			cfg->platform_caps.host_caps_mask = rval;
			MMCINFO("get sdc%d sdc_dis_host_caps 0x%x.\n", sdc_no, cfg->platform_caps.host_caps_mask);
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"sdc_kernel_no_limit", (uint32_t*)(&rval));
		if (ret<0) {
			MMCDBG("get sdc%d sdc_kernel_no_limit fail.\n", sdc_no);
			cfg->platform_caps.tune_limit_kernel_timing = 0x1;
		} else {
			if (rval == 1)
				cfg->platform_caps.tune_limit_kernel_timing = 0x0;
			else
				cfg->platform_caps.tune_limit_kernel_timing = 0x1;
			MMCINFO("get sdc%d sdc_kernel_no_limit 0x%x, limit 0x%x.\n", sdc_no, rval, cfg->platform_caps.tune_limit_kernel_timing);
		}

		/* fmax, fmax_ddr */
	} else {
		MMCINFO("%s: input sdc_no error: %d\n", __FUNCTION__, sdc_no);
	}

	return ;
}

static void mmc_get_para_from_fex(int sdc_no)
{
	if (sdc_no == 0) {
		return _get_para_from_fex_sdc0(sdc_no);
	} else if ((sdc_no == 2) || (sdc_no == 3)) {
		return _get_para_from_fex_sdc2_3(sdc_no);
	} else {
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
			mmchost->database = SUNXI_SMHC2_BASE + MMC2_REG_FIFO_OS;//MMC_REG_FIFO_OS;
			mmchost->mclkbase = CCMU_SDMMC2_CLK_REG;
			break;
#ifdef SUNXI_SMHC3_BASE
		case 3:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_SMHC3_BASE;
			mmchost->database = SUNXI_SMHC3_BASE + MMC_REG_FIFO_OS;
			mmchost->mclkbase = CCMU_SDMMC3_CLK_REG;
			break;
#endif
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

	if ((sdc_no != 3) && (sdc_no != 2) && (sdc_no != 0)) {
		MMCINFO("sdc_on error, %d\n", sdc_no);
		return -1;
	}

	if (sdc_no != 0)
	{
		if (mmc_get_sdc_phy_no(sdc_no) != sdc_no) {
			MMCINFO("get sdc_phy_no error %d\n", mmc_get_sdc_phy_no(sdc_no));
			return -1;
		}
	}

	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
	host = &mmc_host[sdc_no];

	if(2 == sdc_no){
		memset(&mmc2_host_reg_bak, 0, sizeof(struct mmc2_reg_v5p1));
		host->reg_bak = (void*)&mmc2_host_reg_bak;
	} else {
		memset(&mmc_host_reg_bak[sdc_no], 0, sizeof(struct mmc_reg_v4p1));
		host->reg_bak = (void*)&mmc_host_reg_bak[sdc_no];
	}

	if ((sdc_no == 2) || (sdc_no == 3)) {
		host->cfg.platform_caps.odly_spd_freq = &ext_odly_spd_freq[0];
		host->cfg.platform_caps.sdly_spd_freq = &ext_sdly_spd_freq[0];
	} else if (sdc_no == 0) {
		host->cfg.platform_caps.odly_spd_freq = &ext_odly_spd_freq_sdc0[0];
		host->cfg.platform_caps.sdly_spd_freq = &ext_sdly_spd_freq_sdc0[0];
	}

	host->cfg.name = "SUNXI SD/MMC";
	host->cfg.host_no = sdc_no;
	if(2 == sdc_no){
		host->cfg.ops = &sunxi_mmc2_ops;
	} else {
		host->cfg.ops = &sunxi_mmc_ops;
	}

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
	} else if ((sdc_no == 2) || (sdc_no == 3)) {
		host->cfg.f_min = 400000;
#ifdef FPGA_PLATFORM
		host->cfg.f_max = 12000000;
#else
		host->cfg.f_max = 200000000;
#endif
	}

	if ((sdc_no == 0) || (sdc_no == 1))
		host->timing_mode = SUNXI_MMC_TIMING_MODE_1; //SUNXI_MMC_TIMING_MODE_3
	else if (sdc_no == 3)
		host->timing_mode = SUNXI_MMC_TIMING_MODE_4;
	else if (sdc_no == 2)
		host->timing_mode = SUNXI_MMC_TIMING_MODE_2;

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
	if ((host->cfg.host_no == 2) || (host->cfg.host_no == 3))
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
	if(2 == sdc_no)
		memset(&mmc2_host_reg_bak, 0, sizeof(struct mmc2_reg_v5p1));
	else
		memset(&mmc_host_reg_bak[sdc_no], 0, sizeof(struct mmc_reg_v4p1));
	MMCDBG("sunxi mmc%d exit\n", sdc_no);
	return 0;
}
