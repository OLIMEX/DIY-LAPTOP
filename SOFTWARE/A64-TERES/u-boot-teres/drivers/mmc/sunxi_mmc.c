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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
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

#include "mmc_def.h"
#include "sunxi_mmc.h"
#include <fdt_support.h>

#undef readl
#define  readl(a)   *(volatile uint *)(ulong)(a)

#undef writel
#define  writel(v, c) *(volatile uint *)(ulong)(c) = (v)

DECLARE_GLOBAL_DATA_PTR;
//#define SUNXI_MMCDBG
//#undef SUNXI_MMCDBG
//#define MMCINFO(fmt...)	printf("[mmc]: "fmt)
#ifndef CONFIG_ARCH_SUN7I
#define MMC_REG_FIFO_OS		(0x200)
#else
#define MMC_REG_FIFO_OS		(0x100)
#endif

#ifdef SUNXI_MMCDBG
//#define MMCDBG(fmt...)	printf("[mmc]: "fmt)

static void dumphex32(char* name, char* base, int len)
{
	__u32 i;

	MMCPRINT("dump %s registers:", name);
	for (i=0; i<len; i+=4) {
		if (!(i&0xf))
			MMCPRINT("\n0x%p : ", base + i);
		MMCPRINT("0x%08x ", readl((ulong)base + i));
	}
	MMCPRINT("\n");
}

/*
static void dumpmmcreg(struct sunxi_mmc *reg)
{
	printf("dump mmc registers:\n");
	printf("gctrl     0x%08x\n", reg->gctrl     );
	printf("clkcr     0x%08x\n", reg->clkcr     );
	printf("timeout   0x%08x\n", reg->timeout   );
	printf("width     0x%08x\n", reg->width     );
	printf("blksz     0x%08x\n", reg->blksz     );
	printf("bytecnt   0x%08x\n", reg->bytecnt   );
	printf("cmd       0x%08x\n", reg->cmd       );
	printf("arg       0x%08x\n", reg->arg       );
	printf("resp0     0x%08x\n", reg->resp0     );
	printf("resp1     0x%08x\n", reg->resp1     );
	printf("resp2     0x%08x\n", reg->resp2     );
	printf("resp3     0x%08x\n", reg->resp3     );
	printf("imask     0x%08x\n", reg->imask     );
	printf("mint      0x%08x\n", reg->mint      );
	printf("rint      0x%08x\n", reg->rint      );
	printf("status    0x%08x\n", reg->status    );
	printf("ftrglevel 0x%08x\n", reg->ftrglevel );
	printf("funcsel   0x%08x\n", reg->funcsel   );
	printf("dmac      0x%08x\n", reg->dmac      );
	printf("dlba      0x%08x\n", reg->dlba      );
	printf("idst      0x%08x\n", reg->idst      );
	printf("idie      0x%08x\n", reg->idie      );
	printf("cbcr      0x%08x\n", reg->cbcr      );
	printf("bbcr      0x%08x\n", reg->bbcr      );
}
*/
#else
//#define MMCDBG(fmt...)
#define dumpmmcreg(fmt...)
#define  dumphex32(fmt...)
#endif /* SUNXI_MMCDBG */

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
#define SDXC_ACKRcv			BIT(8)	//0x100
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
#define SDXC_CardRemove	BIT(31) //0x80000000
#define SDXC_IntErrBit		(SDXC_RespErr | SDXC_RespCRCErr | SDXC_DataCRCErr \
				| SDXC_RespTimeout | SDXC_DataTimeout | SDXC_FIFORunErr \
				| SDXC_HardWLocked | SDXC_StartBitErr | SDXC_EndBitErr)  //0xbfc2

/* IDMA status bit field */
#define SDXC_IDMACTransmitInt	BIT(0)
#define SDXC_IDMACReceiveInt	BIT(1)
#define SDXC_IDMACFatalBusErr	BIT(2)
#define SDXC_IDMACDesInvalid	BIT(4)
#define SDXC_IDMACCardErrSum	BIT(5)
#define SDXC_IDMACNormalIntSum	BIT(8)
#define SDXC_IDMACAbnormalIntSum BIT(9)
#define SDXC_IDMACHostAbtInTx	BIT(10)
#define SDXC_IDMACHostAbtInRx	BIT(10)
#define SDXC_IDMACIdle		(0U << 13)
#define SDXC_IDMACSuspend	(1U << 13)
#define SDXC_IDMACDESCRd	(2U << 13)
#define SDXC_IDMACDESCCheck	(3U << 13)
#define SDXC_IDMACRdReqWait	(4U << 13)
#define SDXC_IDMACWrReqWait	(5U << 13)
#define SDXC_IDMACRd		(6U << 13)
#define SDXC_IDMACWr		(7U << 13)
#define SDXC_IDMACDESCClose	(8U << 13)

/* delay control */
#define SDXC_StartCal        (1<<15)
#define SDXC_CalDone         (1<<14)
#define SDXC_CalDly          (0x3F<<8)
#define SDXC_EnableDly       (1<<7)
#define SDXC_CfgDly          (0x3F<<0)

#define MMC_CLK_400K					0
#define MMC_CLK_25M						1
#define MMC_CLK_50M						2
#define MMC_CLK_50MDDR				3
#define MMC_CLK_50MDDR_8BIT		4
#define MMC_CLK_100M					5
#define MMC_CLK_200M					6
#define MMC_CLK_MOD_NUM				7


extern char *spd_name[];
/* support 4 mmc hosts */
struct sunxi_mmc_host mmc_host[4];
struct sunxi_mmc mmc_host_reg_bak[4];
u8 ext_odly_spd_freq[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
u8 ext_sdly_spd_freq[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
u8 ext_odly_spd_freq_sdc0[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
u8 ext_sdly_spd_freq_sdc0[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];

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
	//int i, j;

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

	} else if (sdc_no == 2) {
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

static void mmc_get_para_from_fex(int sdc_no)
{
	int rval, ret = 0;
	//int rval_ker, ret1 = 0;
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

	}
	else if (sdc_no == 2)
	{

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


static int mmc_clk_io_onoff(int sdc_no, int onoff, int reset_clk)
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
	}
	else
	{
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
		writel(0x80000000, mmchost->mclkbase);
		mmchost->mod_clk = 24000000;
	}

	dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
	dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
	dumphex32("mmc", (char*)mmchost->reg, 0x100);

	return 0;
}

static int mmc_update_clk(struct sunxi_mmc_host* mmchost)
{
	unsigned int cmd;
	unsigned timeout = 1000;

	writel(readl(&mmchost->reg->clkcr)|(0x1<<31), &mmchost->reg->clkcr);

	cmd = (1U << 31) | (1 << 21) | (1 << 13);
  	writel(cmd, &mmchost->reg->cmd);
	while((readl(&mmchost->reg->cmd)&0x80000000) && --timeout){
		__msdelay(1);
	}
	if (!timeout){
		MMCINFO("mmc %d update clk failed\n",mmchost->mmc_no);
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
	unsigned rval = 0;
	unsigned mode = mmchost->timing_mode;
	unsigned spd_md, spd_md_bak, freq;
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
		spd_md = mmchost->tm4.cur_spd_md;
		spd_md_bak = spd_md;
		freq = mmchost->tm4.cur_freq;

		if (spd_md == HS400)
			spd_md = HS200_SDR104; /* use HS200's sdly for HS400's CMD line */

		if (mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			sdly = mmchost->tm4.sdly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			sdly = mmchost->tm4.def_sdly[spd_md*MAX_CLK_FREQ_NUM+freq];

		spd_md = spd_md_bak;

		if (mmchost->tm4.odly[spd_md*MAX_CLK_FREQ_NUM+freq] != 0xFF)
			odly = mmchost->tm4.odly[spd_md*MAX_CLK_FREQ_NUM+freq];
		else
			odly = mmchost->tm4.def_odly[spd_md*MAX_CLK_FREQ_NUM+freq];

		mmchost->tm4.cur_odly = odly;
		mmchost->tm4.cur_sdly = sdly;

		rval = readl(&mmchost->reg->drv_dl);
		rval &= (~(0x3<<16));
		rval |= (((odly&0x1)<<16) | ((odly&0x1)<<17));
		writel(rval, &mmchost->reg->drv_dl);

		rval = readl(&mmchost->reg->samp_dl);
		rval &= (~SDXC_CfgDly);
		rval |= ((sdly&SDXC_CfgDly) | SDXC_EnableDly);
		writel(rval, &mmchost->reg->samp_dl);

		if (spd_md == HS400)
		{
			if (mmchost->tm4.dsdly[freq] != 0xFF)
				dsdly = mmchost->tm4.dsdly[freq];
			else
				dsdly = mmchost->tm4.def_dsdly[freq];
			mmchost->tm4.cur_dsdly = dsdly;

			rval = readl(&mmchost->reg->ds_dl);
			rval &= (~SDXC_CfgDly);
			rval |= ((dsdly&SDXC_CfgDly) | SDXC_EnableDly);
			#ifdef FPGA_PLATFORM
			rval &= (~0x7);
			#endif
			writel(rval, &mmchost->reg->ds_dl);
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


#ifndef FPGA_PLATFORM
	unsigned freq_id;

	/* disable mclk */
	writel(0x0, mmchost->mclkbase);
	MMCDBG("mmc %d mclkbase 0x%x\n", mmchost->mmc_no, readl(mmchost->mclkbase));

	/* enable timing mode 1 */
	if (mode == SUNXI_MMC_TIMING_MODE_1) {
		rval = readl(&mmchost->reg->ntsr);
		rval |= (1<<31);
		writel(rval, &mmchost->reg->ntsr);
		MMCDBG("mmc %d rntsr 0x%x\n", mmchost->mmc_no, rval);
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
	MMCDBG("get round card clk %d, mod_clk %d\n", mmc->clock, mmchost->mod_clk);

	/* re-enable mclk */
	writel(readl(mmchost->mclkbase)|(1<<31),mmchost->mclkbase);
	MMCDBG("mmc %d mclkbase 0x%x\n", mmchost->mmc_no, readl(mmchost->mclkbase));

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
		rval = readl(&mmchost->reg->clkcr) & (~0xff);
		if (mmc->io_mode == MMC_MODE_DDR_52MHz)
			rval |= 0x1;
		else
			rval |= div >> 1;
		writel(rval, &mmchost->reg->clkcr);

		rval = readl(&mmchost->reg->ntsr);
		rval |= (1<<31);
		writel(rval, &mmchost->reg->ntsr);
		MMCINFO("mmc %d ntsr 0x%x, ckcr 0x%x\n", mmchost->mmc_no,
			readl(&mmchost->reg->ntsr), readl(&mmchost->reg->clkcr));
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
		rval = readl(&mmchost->reg->clkcr) & (~0xff);
		if (mmc->io_mode == MMC_MODE_DDR_52MHz)
			rval |= 0x1;
		else
			rval |= div >> 1;
		writel(rval, &mmchost->reg->clkcr);
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
	//dumphex32("mmc", (char*)mmchost->reg, 0x100);
	return 0;
}

static int mmc_config_clock(struct sunxi_mmc_host* mmchost, unsigned clk)
{
	unsigned rval = 0;

	/* disable card clock */
	rval = readl(&mmchost->reg->clkcr);
	rval &= ~(1 << 16);
	writel(rval, &mmchost->reg->clkcr);
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
	rval = readl(&mmchost->reg->clkcr);
	rval |=  (0x1 << 16); //(3 << 16);
	writel(rval, &mmchost->reg->clkcr);
	if(mmc_update_clk(mmchost)){
		MMCINFO("mmc %d re-enable clock failed\n",mmchost->mmc_no);
		return -1;
	}

	return 0;
}

static int mmc_calibrate_delay_unit(struct sunxi_mmc_host* mmchost)
{
	unsigned rval = 0;
	unsigned result = 0;

	MMCINFO("start %s, don't access device...\n", __FUNCTION__);

	/* close card clock */
	rval = readl(&mmchost->reg->clkcr);
	rval &= ~(1 << 16);
	writel(rval, &mmchost->reg->clkcr);
	if(mmc_update_clk(mmchost))
		return -1;

	/* set card clock to 100MHz */
	if ((mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_1)
		|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_3)
		|| (mmchost->timing_mode == SUNXI_MMC_TIMING_MODE_4) )

		mmc_config_clock_modex(mmchost, 100000000);
	else {
		MMCINFO("%s: mmc %d wrong timing mode: 0x%x\n",
			__FUNCTION__, mmchost->mmc_no, mmchost->timing_mode);
		return -1;
	}

	/* start carlibrate delay unit */
	writel(0xA0, &mmchost->reg->samp_dl);
	writel(0x0, &mmchost->reg->samp_dl);
	rval = SDXC_StartCal;
	writel(rval, &mmchost->reg->samp_dl);
	while (!(readl(&mmchost->reg->samp_dl) & SDXC_CalDone));

	if (mmchost->mmc_no == 2) {
		writel(0xA0, &mmchost->reg->ds_dl);
		writel(0x0, &mmchost->reg->ds_dl);
		rval = SDXC_StartCal;
		writel(rval, &mmchost->reg->ds_dl);
		while (!(readl(&mmchost->reg->ds_dl) & SDXC_CalDone));
	}

	/* update result */
	rval = readl(&mmchost->reg->samp_dl);
	result = (rval & SDXC_CalDly) >> 8;
	MMCDBG("ds_dl result: 0x%x\n", result);
	/* 10ns= 10*1000 ps, mod_clk is 5ns */
	if (result) {
		rval = 5000 / result;
		mmchost->tm3.sdly_unit_ps = rval;
		mmchost->tm3.dly_calibrate_done = 1;

		mmchost->tm4.sdly_unit_ps = rval;
		MMCINFO("delay chain cal done, sample: %d(ps)\n", mmchost->tm3.sdly_unit_ps);
	} else {
		MMCINFO("%s: cal sample delay fail\n", __FUNCTION__);
	}

	if (mmchost->mmc_no == 2) {
		rval = readl(&mmchost->reg->ds_dl);
		result = (rval & SDXC_CalDly) >> 8;
		MMCDBG("ds_dl result: 0x%x\n", result);
		if (result) {
			rval = 5000 / result;
			mmchost->tm4.dsdly_unit_ps = rval;
			mmchost->tm4.dly_calibrate_done = 1;
			MMCINFO("delay chain cal done, ds: %d(ps)\n", mmchost->tm4.dsdly_unit_ps);
		} else {
			MMCINFO("%s: cal data strobe delay fail\n", __FUNCTION__);
		}
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
		MMCDBG("set %d rgctrl 0x%x to enable ddr mode\n", mmchost->mmc_no, readl(&mmchost->reg->gctrl));
	} else {
		writel(rval, &mmchost->reg->gctrl);
		MMCDBG("set %d rgctrl 0x%x to disable ddr mode\n", mmchost->mmc_no, readl(&mmchost->reg->gctrl));
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
		MMCDBG("set %d dsbd 0x%x to enable hs400 mode\n", mmchost->mmc_no, readl(&mmchost->reg->dsbd));
	} else {
		writel(rval, &mmchost->reg->dsbd);
		MMCDBG("set %d dsbd 0x%x to disable hs400 mode\n", mmchost->mmc_no, readl(&mmchost->reg->dsbd));
	}
}

static void mmc_set_ios(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;

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
		writel(2, &mmchost->reg->width);
	else if (mmc->bus_width == 4)
		writel(1, &mmchost->reg->width);
	else
		writel(0, &mmchost->reg->width);
	MMCDBG("host bus width register 0x%x\n", readl(&mmchost->reg->width));

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

static int mmc_save_regs(struct sunxi_mmc_host* host)
{
	host->reg_bak->gctrl     = readl(&host->reg->gctrl  );
	host->reg_bak->clkcr     = readl(&host->reg->clkcr  );
	host->reg_bak->timeout   = readl(&host->reg->timeout);
	host->reg_bak->width     = readl(&host->reg->width  );
	host->reg_bak->imask     = readl(&host->reg->imask  );
	host->reg_bak->ftrglevel = readl(&host->reg->ftrglevel);
	host->reg_bak->dbgc      = readl(&host->reg->dbgc   );
	host->reg_bak->csdc      = readl(&host->reg->csdc   );
	host->reg_bak->ntsr      = readl(&host->reg->ntsr   );
	host->reg_bak->hwrst     = readl(&host->reg->hwrst  );
	host->reg_bak->dmac      = readl(&host->reg->dmac   );
	host->reg_bak->idie      = readl(&host->reg->idie   );
	host->reg_bak->thldc     = readl(&host->reg->thldc  );
	host->reg_bak->dsbd      = readl(&host->reg->dsbd   );
	host->reg_bak->drv_dl    = readl(&host->reg->drv_dl );
	host->reg_bak->samp_dl   = readl(&host->reg->samp_dl);
	host->reg_bak->ds_dl     = readl(&host->reg->ds_dl  );

	return 0;
}

static int mmc_restore_regs(struct sunxi_mmc_host* host)
{
	writel(host->reg_bak->gctrl     , &host->reg->gctrl  );
	writel(host->reg_bak->clkcr     , &host->reg->clkcr  );
	writel(host->reg_bak->timeout   , &host->reg->timeout);
	writel(host->reg_bak->width     , &host->reg->width  );
	writel(host->reg_bak->imask     , &host->reg->imask  );
	writel(host->reg_bak->ftrglevel , &host->reg->ftrglevel);
	if (host->reg_bak->dbgc)
		writel(0xdeb, &host->reg->dbgc);
	writel(host->reg_bak->csdc      , &host->reg->csdc   );
	writel(host->reg_bak->ntsr      , &host->reg->ntsr   );
	writel(host->reg_bak->hwrst     , &host->reg->hwrst  );
	writel(host->reg_bak->dmac      , &host->reg->dmac   );
	writel(host->reg_bak->idie      , &host->reg->idie   );
	writel(host->reg_bak->thldc     , &host->reg->thldc  );
	writel(host->reg_bak->dsbd      , &host->reg->dsbd   );
	writel(host->reg_bak->drv_dl    , &host->reg->drv_dl );
	writel(host->reg_bak->samp_dl   , &host->reg->samp_dl);
	writel(host->reg_bak->ds_dl     , &host->reg->ds_dl  );

	return 0;
}

static int mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;

	/* Reset controller */
	writel(0x7, &mmchost->reg->gctrl); /*0x40000007*/
	while(readl(&mmchost->reg->gctrl)&0x7);
	/* release eMMC reset signal */
	writel(1, &mmchost->reg->hwrst);
	writel(0, &mmchost->reg->hwrst);
	udelay(1000);
	writel(1, &mmchost->reg->hwrst);
	udelay(1000);

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

	mmc_calibrate_delay_unit(mmchost);

	return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned i;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned *buff;
	unsigned timeout = 1000;

	if (data->flags & MMC_DATA_READ) {
		buff = (unsigned int *)data->dest;
		for (i=0; i<(byte_cnt>>2); i++) {
			while(--timeout && (readl(&mmchost->reg->status)&(1 << 2))){
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
			while(--timeout && (readl(&mmchost->reg->status)&(1 << 3))){
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
	struct sunxi_mmc_des *pdes = mmchost->pdes;
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
		memset((void*)&pdes[des_idx], 0, sizeof(struct sunxi_mmc_des));
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
	flush_cache((unsigned long)pdes, sizeof(struct sunxi_mmc_des) * (des_idx+1));
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
	while(readl(&mmchost->reg->dmac)& 0x1) {}; /* wait idma reset done */
	writel((1 << 1) | (1 << 7), &mmchost->reg->dmac); /* idma on */
	rval = readl(&mmchost->reg->idie) & (~3);
	if (data->flags & MMC_DATA_WRITE)
		rval |= (1 << 0);
	else
		rval |= (1 << 1);
	writel(rval, &mmchost->reg->idie);
	writel((unsigned long)pdes, &mmchost->reg->dlba);
	if (mmchost->mmc_no == 2)
		writel((3U<<28)|(15U<<16)|240, &mmchost->reg->ftrglevel); /* burst-16, rx/tx trigger level=15/240 */
	else
		writel((2U<<28)|(7U<<16)|248, &mmchost->reg->ftrglevel); /* burst-8, rx/tx trigger level=7/248 */

	return 0;
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
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
			status = readl(&mmchost->reg->status);
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
		writel(data->blocksize, &mmchost->reg->blksz);
		writel(data->blocks * data->blocksize, &mmchost->reg->bytecnt);
	} else {
		if ((cmd->cmdidx == 12)&&(cmd->flags&MMC_CMD_MANUAL)) {
			cmdval |= 1<<14;//stop current data transferin progress.
			cmdval &= ~(1 << 13);//Send command at once, even if previous data transfer has notcompleted
		}
	}

	MMCDBG("mmc %d, cmd %d(0x%08x), arg 0x%08x\n",
		mmchost->mmc_no, cmd->cmdidx, cmdval|cmd->cmdidx, cmd->cmdarg);

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
		MMCDBG("mmc %d trans data %d bytes\n",mmchost->mmc_no, bytecnt);
#ifdef CONFIG_MMC_SUNXI_USE_DMA
		if (bytecnt > 64) {
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
			MMCINFO("mmc %d Transfer failed\n",mmchost->mmc_no);
			error = readl(&mmchost->reg->rint) & 0xbfc2;
			if(!error)
				error = 0xffffffff;
			goto out;
		}
	}

	timeout = 1000;
	do {
		status = readl(&mmchost->reg->rint);
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
			status = readl(&mmchost->reg->rint);
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
					done = ((status & (1<<14)) && (readl(&mmchost->reg->idst) & 0x3)) ? 1 : 0;
				else
					done = status & (1<<14);
			}
			else
			{
				if (usedma)
					done = ((status & (1<<3)) && (readl(&mmchost->reg->idst) & 0x3)) ? 1 : 0;
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
			status = readl(&mmchost->reg->status);
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
		cmd->response[0] = readl(&mmchost->reg->resp3);
		cmd->response[1] = readl(&mmchost->reg->resp2);
		cmd->response[2] = readl(&mmchost->reg->resp1);
		cmd->response[3] = readl(&mmchost->reg->resp0);
		MMCDBG("mmc %d mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
			mmchost->mmc_no,
			cmd->response[3], cmd->response[2],
			cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&mmchost->reg->resp0);
		MMCDBG("mmc %d mmc resp 0x%08x\n",mmchost->mmc_no, cmd->response[0]);
	}
out:
	if(error){
		mmchost->raw_int_bak = readl(&mmchost->reg->rint )& 0xbfc2;
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
		status = readl(&mmchost->reg->idst);
		writel(status, &mmchost->reg->idst);
		writel(0, &mmchost->reg->idie);
		writel(0, &mmchost->reg->dmac);
		writel(readl(&mmchost->reg->gctrl)&(~(1 << 5)), &mmchost->reg->gctrl);
	}
	if (error) {

		/* during tuning sample point, some sample point may cause timing problem.
		for example, if a RTO error occurs, host may stop clock and device may still output data.
		we need to read all data(512bytes) from device to avoid to update clock fail.
		*/
		if (mmc->do_tuning && data && (data->flags&MMC_DATA_READ) && (bytecnt==512)) {
			writel(readl(&mmchost->reg->gctrl)|0x80000000, &mmchost->reg->gctrl);
			writel(0xdeb, &mmchost->reg->dbgc);
			timeout = 1000;
			MMCMSG(mmc, "Read remain data\n");
			while (readl(&mmchost->reg->bbcr)<512) {
				unsigned int tmp = readl(mmchost->database);
				tmp = tmp;
				MMCDBG("Read data 0x%x, bbcr 0x%x\n", tmp, readl(&mmchost->reg->bbcr));
				__usdelay(1);
				if (!(timeout--)) {
					MMCMSG(mmc, "Read remain data timeout\n");
					break;
				}
			}
		}

		writel(0x7, &mmchost->reg->gctrl);
		while(readl(&mmchost->reg->gctrl)&0x7) { };

		{
			mmc_save_regs(mmchost);
			mmc_clk_io_onoff(mmchost->mmc_no, 0, 0);
			MMCMSG(mmc, "mmc %d close bus gating and reset\n", mmchost->mmc_no);
			mmc_clk_io_onoff(mmchost->mmc_no, 1, 0);
			mmc_restore_regs(mmchost);

			writel(0x7, &mmchost->reg->gctrl);
			while(readl(&mmchost->reg->gctrl)&0x7) { };
		}

		mmc_update_clk(mmchost);
		MMCMSG(mmc, "mmc %d mmc cmd %d err 0x%08x\n", mmchost->mmc_no, cmd->cmdidx, error);

	}
	writel(0xffffffff, &mmchost->reg->rint);

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

int sunxi_decide_rty(struct mmc *mmc, int err_no, uint rst_cnt)
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

int sunxi_detail_errno(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	u32 err_no = mmchost->raw_int_bak;
	mmchost->raw_int_bak = 0;
	return err_no;
}

static const struct mmc_ops sunxi_mmc_ops = {
	.send_cmd	= mmc_send_cmd,
	.set_ios	= mmc_set_ios,
	.init		= mmc_core_init,
	.decide_retry = sunxi_decide_rty,
	.get_detail_errno = sunxi_detail_errno,
	.update_phase = mmc_update_phase,
};

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
	memset(&mmc_host_reg_bak[sdc_no], 0, sizeof(struct sunxi_mmc));
	host->reg_bak =  &mmc_host_reg_bak[sdc_no];
	if (sdc_no == 2) {
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
		host->cfg.f_max = 50000000;
	} else if (sdc_no == 2) {
		host->cfg.f_min = 400000;
		host->cfg.f_max = 200000000;
	}

	if ((sdc_no == 0) || (sdc_no == 1))
		host->timing_mode = SUNXI_MMC_TIMING_MODE_1; //SUNXI_MMC_TIMING_MODE_3
	else if (sdc_no == 2)
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
	//mmc_unregister(sdc_no);

	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
	MMCDBG("sunxi mmc%d exit\n", sdc_no);
	return 0;
}
