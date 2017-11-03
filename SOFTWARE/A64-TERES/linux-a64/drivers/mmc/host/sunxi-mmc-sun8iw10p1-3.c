#ifdef CONFIG_ARCH_SUN8IW10P1


#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/clk/sunxi.h>

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/reset.h>

#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#include <linux/mmc/host.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/slot-gpio.h>


#include "sunxi-mmc.h"
#include "sunxi-mmc-sun8iw10p1-3.h"


//reg
#define SDXC_REG_EDSD		(0x010C)    /*SMHC eMMC4.5 DDR Start Bit Detection Control Register*/
#define SDXC_REG_CSDC		(0x0054)    /*SMHC CRC Status Detect Control Register*/
#define SDXC_REG_THLD		(0x0100)    /*SMHC Card Threshold Control Register*/
#define SDXC_REG_DRV_DL	 	(0x0140)    /*SMHC Drive Delay Control Register*/
#define SDXC_REG_SAMP_DL	(0x0144)	/*SMHC Sample Delay Control Register*/
#define SDXC_REG_DS_DL		(0x0148)	/*SMHC Data Strobe Delay Control Register*/


//bit
#define SDXC_HS400_MD_EN				(1U<<31)
#define SDXC_CARD_WR_THLD_ENB		(1U<<2)
#define SDXC_CARD_RD_THLD_ENB		(1U)

#define SDXC_DAT_DRV_PH_SEL			(1U<<17)
#define SDXC_CMD_DRV_PH_SEL			(1U<<16)
#define SDXC_SAMP_DL_SW_EN			(1u<<7)
#define SDXC_DS_DL_SW_EN			(1u<<7)


//mask
#define SDXC_CRC_DET_PARA_MASK		(0xf)
#define SDXC_CARD_RD_THLD_MASK		(0x0FFF0000)
#define SDXC_TX_TL_MASK				(0xff)
#define SDXC_RX_TL_MASK				(0x00FF0000)

#define SDXC_SAMP_DL_SW_MASK		(0x0000003F)
#define SDXC_DS_DL_SW_MASK			(0x0000003F)


//value
#define SDXC_CRC_DET_PARA_HS400		(6)
#define SDXC_CRC_DET_PARA_OTHER		(3)
#define SDXC_FIFO_DETH					(1024>>2)

//size
#define SDXC_CARD_RD_THLD_SIZE		(0x00000FFF)

//shit
#define SDXC_CARD_RD_THLD_SIZE_SHIFT		(16)



struct sunxi_mmc_spec_regs {
	u32 drv_dl;//REG_DRV_DL
	u32 samp_dl;//REG_SAMP_DL
	u32 ds_dl;//REG_DS_DL
	//u32 sd_ntsr;//REG_SD_NTSR
	u32 edsd;//REG_EDSD
	u32 csdc;//REG_CSDC
};

static struct sunxi_mmc_spec_regs bak_spec_regs;


/*
enum sunxi_mmc_clk_mode
{
	mmc_clk_400k = 0,
	mmc_clk_26M,
	mmc_clk_52M,
	mmc_clk_52M_DDR4,
	mmc_clk_52M_DDR8,
	mmc_clk_104M,
	mmc_clk_208M,
	mmc_clk_104M_DDR,
	mmc_clk_208M_DDR,
	mmc_clk_mod_num,
};

struct sunxi_mmc_clk_dly {
	enum sunxi_mmc_clk_mode cmod;
	char *mod_str;
	u32 cmd_drv_ph;
	u32 dat_drv_ph;
	u32 sam_dly;
	u32 ds_dly;
};



static struct sunxi_mmc_clk_dly mmc_clk_dly[mmc_clk_mod_num] = {
	[mmc_clk_400k] = {
						.cmod	  = mmc_clk_400k,
						.mod_str = "sunxi-dly-400k",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_26M] = {
						.cmod	  = mmc_clk_26M,
						.mod_str = "sunxi-dly-26M",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_52M] = {
						.cmod	  = mmc_clk_52M,
						.mod_str = "sunxi-dly-52M",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_52M_DDR4] = {
						.cmod	  = mmc_clk_52M_DDR4,
						.mod_str = "sunxi-dly-52M-ddr4",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_52M_DDR8] = {
						.cmod	  = mmc_clk_52M_DDR8,
						.mod_str = "sunxi-dly-52M-ddr8",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_104M] = {
						.cmod	  = mmc_clk_104M,
						.mod_str = "sunxi-dly-104M",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_208M] = {
						.cmod	  = mmc_clk_208M,
						.mod_str = "sunxi-dly-208M",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_104M_DDR] = {
						.cmod	  = mmc_clk_104M_DDR,
						.mod_str = "sunxi-dly-104M-ddr",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
	[mmc_clk_208M_DDR] = {
						.cmod	  = mmc_clk_208M_DDR,
						.mod_str = "sunxi-dly-208M-ddr",
						.cmd_drv_ph = 1,
						.dat_drv_ph = 0,
						.sam_dly	= 0,
						.ds_dly		= 0,
					 },
};

*/

/*
static int sunxi_of_parse_clk_dly(struct sunxi_mmc_host *host)
{
	struct device_node *np;
	struct mmc_host *mhost = host->mmc;
	int i = 0;
	u32 in_clk_dly[4] = {0};
	int ret = 0;

	if (!mhost->parent || !mhost->parent->of_node){
		dev_err(mmc_dev(host->mmc), "no dts to parse clk dly\n");
		return -EINVAL;
	}

	np = mhost->parent->of_node;

	for(i=0;i<mmc_clk_mod_num;i++){
		ret = of_property_read_u32_array(np,mmc_clk_dly[i].mod_str,\
											in_clk_dly,ARRAY_SIZE(in_clk_dly));
		if(ret){
			dev_info(mmc_dev(host->mmc),"faild to get 	%s\n",mmc_clk_dly[i].mod_str);
		}else{
			mmc_clk_dly[i].cmd_drv_ph 	= in_clk_dly[0];
			mmc_clk_dly[i].dat_drv_ph 	= in_clk_dly[1];
			mmc_clk_dly[i].sam_dly		= in_clk_dly[2];
			mmc_clk_dly[i].ds_dly		= in_clk_dly[3];
			dev_info(mmc_dev(host->mmc),"Get %s clk dly ok\n",mmc_clk_dly[i].mod_str);
			dev_info(mmc_dev(host->mmc),"cmd_drv_ph     %d\n",mmc_clk_dly[i].cmd_drv_ph);
			dev_info(mmc_dev(host->mmc),"dat_drv_ph     %d\n",mmc_clk_dly[i].dat_drv_ph);
			dev_info(mmc_dev(host->mmc),"sam_dly        %d\n",mmc_clk_dly[i].sam_dly);
			dev_info(mmc_dev(host->mmc),"ds_dly         %d\n",mmc_clk_dly[i].ds_dly);
		}

	}

	return 0;
}
*/

/*
static void sunxi_set_clk_dly(struct sunxi_mmc_host *host,int clk,int bus_width,int timing)
{
	struct mmc_host *mhost = host->mmc;
	u32 rval		= 0;
	enum sunxi_mmc_clk_mode cmod = mmc_clk_400k;
	u32 in_clk_dly[4] = {0};
	int ret = 0;
	struct device_node *np = NULL;

	if (!mhost->parent || !mhost->parent->of_node){
		dev_err(mmc_dev(host->mmc), "no dts to parse clk dly,use default\n");
		return ;
	}

	np = mhost->parent->of_node;

	if(clk <= 400*1000 ){
		cmod = mmc_clk_400k;
	}else if(clk <= 26*1000*1000){
		cmod = mmc_clk_26M;
	}else if(clk <= 52*1000*1000){
		if((bus_width == MMC_BUS_WIDTH_4)&&(timing == MMC_TIMING_UHS_DDR50)){
			cmod = mmc_clk_52M_DDR4;
		}else if((bus_width == MMC_BUS_WIDTH_8)&&(timing == MMC_TIMING_UHS_DDR50)){
			cmod = mmc_clk_52M_DDR8;
		}else{
			cmod = mmc_clk_52M;
		}
	}else if(clk <= 104*1000*1000){
		if((bus_width == MMC_BUS_WIDTH_8)&&(timing == MMC_TIMING_MMC_HS400)){
			cmod = mmc_clk_104M_DDR;
		}else{
			cmod = mmc_clk_104M;
		}
	}else if(clk <= 208*1000*1000){
		if((bus_width == MMC_BUS_WIDTH_8)&&(timing == MMC_TIMING_MMC_HS400)){
			cmod = mmc_clk_208M_DDR;
		}else{
			cmod = mmc_clk_208M;
		}
	}else{
		dev_err(mmc_dev(mhost),"clk %d is out of range\n",clk);
		return;
	}


	ret = of_property_read_u32_array(np,mmc_clk_dly[cmod].mod_str,\
										in_clk_dly,ARRAY_SIZE(in_clk_dly));
	if(ret){
		dev_info(mmc_dev(host->mmc),"faild to get %s used default\n",mmc_clk_dly[cmod].mod_str);
	}else{
		mmc_clk_dly[cmod].cmd_drv_ph	= in_clk_dly[0];
		mmc_clk_dly[cmod].dat_drv_ph	= in_clk_dly[1];
		mmc_clk_dly[cmod].sam_dly		= in_clk_dly[2];
		mmc_clk_dly[cmod].ds_dly		= in_clk_dly[3];
		dev_info(mmc_dev(host->mmc),"Get %s clk dly ok\n",mmc_clk_dly[cmod].mod_str);

	}

	dev_dbg(mmc_dev(host->mmc),"Try set %s clk dly       ok\n",mmc_clk_dly[cmod].mod_str);
	dev_dbg(mmc_dev(host->mmc),"cmd_drv_ph 	%d\n",mmc_clk_dly[cmod].cmd_drv_ph);
	dev_dbg(mmc_dev(host->mmc),"dat_drv_ph 	%d\n",mmc_clk_dly[cmod].dat_drv_ph);
	dev_dbg(mmc_dev(host->mmc),"sam_dly		%d\n",mmc_clk_dly[cmod].sam_dly);
	dev_dbg(mmc_dev(host->mmc),"ds_dly 		%d\n",mmc_clk_dly[cmod].ds_dly);

	rval = mmc_readl(host,REG_DRV_DL);
	if(mmc_clk_dly[cmod].cmd_drv_ph){
		rval |= SDXC_CMD_DRV_PH_SEL;//180 phase
	}else{
		rval &= ~SDXC_CMD_DRV_PH_SEL;//90 phase
	}

	if(mmc_clk_dly[cmod].dat_drv_ph){
		rval |= SDXC_DAT_DRV_PH_SEL;//180 phase
	}else{
		rval &= ~SDXC_DAT_DRV_PH_SEL;//90 phase
	}
	mmc_writel(host,REG_DRV_DL,rval);

	rval = mmc_readl(host,REG_SAMP_DL);
	rval &= ~SDXC_SAMP_DL_SW_MASK;
	rval |= mmc_clk_dly[cmod].sam_dly & SDXC_SAMP_DL_SW_MASK;
	rval |= SDXC_SAMP_DL_SW_EN;
	mmc_writel(host,REG_SAMP_DL,rval);

	rval = mmc_readl(host,REG_DS_DL);
	rval &= ~SDXC_DS_DL_SW_MASK;
	rval |= mmc_clk_dly[cmod].ds_dly & SDXC_DS_DL_SW_MASK;
	rval |= SDXC_DS_DL_SW_EN;
	mmc_writel(host,REG_DS_DL,rval);

	dev_dbg(mmc_dev(host->mmc)," REG_DRV_DL    %08x\n",mmc_readl(host,REG_DRV_DL));
	dev_dbg(mmc_dev(host->mmc)," REG_SAMP_DL  %08x\n",mmc_readl(host,REG_SAMP_DL));
	dev_dbg(mmc_dev(host->mmc)," REG_DS_DL      %08x\n",mmc_readl(host,REG_DS_DL));

}
*/


enum sunxi_mmc_speed_mode
{
	SM0_DS26_SDR12 = 0,
	SM1_HSSDR52_SDR25,
	SM2_HSDDR52_DDR50,
	SM3_HS200_SDR104,
	SM4_HS400,
	SM_NUM,
};

struct sunxi_mmc_clk_dly {
	enum sunxi_mmc_speed_mode spm;
	char *mod_str;
	char *raw_tm_sm_str[2];
	u32 raw_tm_sm[2];
	u32 raw_tm_sm_def[2];
};


static struct sunxi_mmc_clk_dly mmc_clk_dly[SM_NUM] = {
	[SM0_DS26_SDR12] = {
						.spm	  = SM0_DS26_SDR12,
						.mod_str=  "DS26_SDR12",
						.raw_tm_sm_str[0] = "sdc_tm4_sm0_freq0",
						.raw_tm_sm_str[1] = "sdc_tm4_sm0_freq1",
						.raw_tm_sm [0] = 0,
						.raw_tm_sm [1] = 0,
						.raw_tm_sm_def [0] = 0,
						.raw_tm_sm_def [1] = 0,
					 },
	[SM1_HSSDR52_SDR25] = {
						.spm	  = SM1_HSSDR52_SDR25,
						.mod_str	=  "HSSDR52_SDR25",
						.raw_tm_sm_str[0] = "sdc_tm4_sm1_freq0",
						.raw_tm_sm_str[1] = "sdc_tm4_sm1_freq1",
						.raw_tm_sm [0] = 0,
						.raw_tm_sm [1] = 0,
						.raw_tm_sm_def [0] = 0,
						.raw_tm_sm_def [1] = 0,
					 },
	[SM2_HSDDR52_DDR50] = {
						.spm	  = SM2_HSDDR52_DDR50,
						.mod_str	=  "HSDDR52_DDR50",
						.raw_tm_sm_str[0] = "sdc_tm4_sm2_freq0",
						.raw_tm_sm_str[1] = "sdc_tm4_sm2_freq1",
						.raw_tm_sm [0] = 0,
						.raw_tm_sm [1] = 0,
						.raw_tm_sm_def [0] = 0,
						.raw_tm_sm_def [1] = 0,
					 },
	[SM3_HS200_SDR104] = {
						.spm	  = SM3_HS200_SDR104,
						.mod_str	=  "HS200_SDR104",
						.raw_tm_sm_str[0] = "sdc_tm4_sm3_freq0",
						.raw_tm_sm_str[1] = "sdc_tm4_sm3_freq1",
						.raw_tm_sm [0] = 0,
						.raw_tm_sm [1] = 0,
						.raw_tm_sm_def [0] = 0,
						.raw_tm_sm_def [1] = 0x00000405,
						},
	[SM4_HS400] = {
						.spm	  = SM4_HS400,
						.mod_str	=  "HS400",
						.raw_tm_sm_str[0] = "sdc_tm4_sm4_freq0",
						.raw_tm_sm_str[1] = "sdc_tm4_sm4_freq1",
						.raw_tm_sm [0] = 0,
						.raw_tm_sm [1] = 0x00000608,
						.raw_tm_sm_def [0] = 0,
						.raw_tm_sm_def [1] = 0x00000408,
						},
};




static void sunxi_mmc_set_clk_dly(struct sunxi_mmc_host *host,int clk,int bus_width,int timing)
{
	struct mmc_host *mmc = host->mmc;
	enum sunxi_mmc_speed_mode speed_mod = SM0_DS26_SDR12;
	char *raw_sm_str=  NULL;
	char *m_str	=  NULL;
	struct device_node *np = NULL;
	u32 *raw_sm	= 0;
	u32 *raw_sm_def	= 0;
	u32 rval	= 0;
	int frq_index	= 0;
	u32 cmd_drv_ph	= 1;
	u32 dat_drv_ph	= 0;
	u32 sam_dly	= 0;
	u32 ds_dly	= 0;
	
	if (!mmc->parent || !mmc->parent->of_node){
		dev_err(mmc_dev(host->mmc), "no dts to parse clk dly,use default\n");
		return ;
	}

	np = mmc->parent->of_node;

	switch(timing){
	case	MMC_TIMING_LEGACY:
	case	MMC_TIMING_UHS_SDR12:
		speed_mod = SM0_DS26_SDR12;
		break;
	case	MMC_TIMING_MMC_HS:
	case	MMC_TIMING_SD_HS:
	case	MMC_TIMING_UHS_SDR25:
		speed_mod = SM1_HSSDR52_SDR25;
		break;
	case	MMC_TIMING_UHS_DDR50:
		speed_mod = SM2_HSDDR52_DDR50;
		break;
	case	MMC_TIMING_UHS_SDR50:
	case	MMC_TIMING_UHS_SDR104:
	case	MMC_TIMING_MMC_HS200:
		speed_mod = SM3_HS200_SDR104;
		break;
	case	MMC_TIMING_MMC_HS400:
		speed_mod = SM4_HS400;
		break;
	default:
		dev_err(mmc_dev(mmc),"Wrong timing input\n");
		return;
	}


	if(clk<= 400*1000){
		frq_index = 0;
	}else if(clk<= 25*1000*1000){
		frq_index = 1;
	}else if(clk<=50*1000*1000){
		frq_index = 2;
	}else if(clk<= 100*1000*1000){
		frq_index = 3;
	}else if(clk<= 150*1000*1000){
		frq_index = 4;
	}else if(clk<= 200*1000*1000){
		frq_index = 5;
	}else if(clk<= 250*1000*1000){
		frq_index = 6;
	}else if(clk<= 300*1000*1000){
		frq_index = 7;
	}else{
		dev_err(mmc_dev(mmc),"clk is over 300mhz\n");
		return;
	}

	BUG_ON(frq_index/4 > 2);
	dev_dbg(mmc_dev(host->mmc),"freq %d frq index %d,frq/4 %x\n",clk,frq_index,frq_index/4);
	raw_sm_str 	= mmc_clk_dly[speed_mod].raw_tm_sm_str[frq_index/4];
	raw_sm 		= &mmc_clk_dly[speed_mod].raw_tm_sm[frq_index/4];
	raw_sm_def	= &mmc_clk_dly[speed_mod].raw_tm_sm_def[frq_index/4];
	m_str  		= mmc_clk_dly[speed_mod].mod_str;

	rval = of_property_read_u32(np, raw_sm_str, raw_sm);
	if(rval){
		dev_info(mmc_dev(host->mmc),"faild to get %s used default\n",m_str);
	}else{
		u32 sm_shift	= (frq_index%4)*8;
		rval = ((*raw_sm)>>sm_shift)&0xff;
		if(rval!=0xff){
			if(timing == MMC_TIMING_MMC_HS400){
				u32 raw_sm_hs200  = 0;
				ds_dly	= rval;
				raw_sm_hs200 = mmc_clk_dly[SM3_HS200_SDR104].raw_tm_sm[frq_index/4];
				sam_dly	= ((raw_sm_hs200)>>sm_shift)&0xff;
			}else{
				sam_dly	= rval;
			}
			dev_dbg(mmc_dev(host->mmc),"Get speed mode %s clk dly %s ok\n",m_str,raw_sm_str);
		}else{
			u32 sm_shift	= (frq_index%4)*8;
			dev_dbg(mmc_dev(host->mmc),"%s use default value\n",m_str);
			rval = ((*raw_sm_def)>>sm_shift)&0xff;
			if(timing == MMC_TIMING_MMC_HS400){
				u32 raw_sm_hs200  = 0;
				ds_dly	= rval;
				raw_sm_hs200 = mmc_clk_dly[SM3_HS200_SDR104].raw_tm_sm_def[frq_index/4];
				sam_dly	= ((raw_sm_hs200)>>sm_shift)&0xff;
			}else{
				sam_dly	= rval;
			}
		}

	}

	dev_dbg(mmc_dev(host->mmc),"Try set %s clk dly       ok\n",m_str);
	dev_dbg(mmc_dev(host->mmc),"cmd_drv_ph 	%d\n",cmd_drv_ph);
	dev_dbg(mmc_dev(host->mmc),"dat_drv_ph 	%d\n",dat_drv_ph);
	dev_dbg(mmc_dev(host->mmc),"sam_dly	%d\n",sam_dly);
	dev_dbg(mmc_dev(host->mmc),"ds_dly 	%d\n",ds_dly);

	rval = mmc_readl(host,REG_DRV_DL);
	if(cmd_drv_ph){
		rval |= SDXC_CMD_DRV_PH_SEL;//180 phase
	}else{
		rval &= ~SDXC_CMD_DRV_PH_SEL;//90 phase
	}

	if(dat_drv_ph){
		rval |= SDXC_DAT_DRV_PH_SEL;//180 phase
	}else{
		rval &= ~SDXC_DAT_DRV_PH_SEL;//90 phase
	}
	mmc_writel(host,REG_DRV_DL,rval);

	rval = mmc_readl(host,REG_SAMP_DL);
	rval &= ~SDXC_SAMP_DL_SW_MASK;
	rval |= sam_dly & SDXC_SAMP_DL_SW_MASK;
	rval |= SDXC_SAMP_DL_SW_EN;
	mmc_writel(host,REG_SAMP_DL,rval);

	rval = mmc_readl(host,REG_DS_DL);
	rval &= ~SDXC_DS_DL_SW_MASK;
	rval |= ds_dly & SDXC_DS_DL_SW_MASK;
	rval |= SDXC_DS_DL_SW_EN;
	mmc_writel(host,REG_DS_DL,rval);

	dev_dbg(mmc_dev(host->mmc)," REG_DRV_DL    %08x\n",mmc_readl(host,REG_DRV_DL));
	dev_dbg(mmc_dev(host->mmc)," REG_SAMP_DL  %08x\n",mmc_readl(host,REG_SAMP_DL));
	dev_dbg(mmc_dev(host->mmc)," REG_DS_DL      %08x\n",mmc_readl(host,REG_DS_DL));

}


void sunxi_mmc_dump_dly3(struct sunxi_mmc_host *host)
{
	int i = 0;
	for(i=0 ;i<SM_NUM;i++){
		printk("mod_str %s\n",mmc_clk_dly[i].mod_str);
		printk("raw_tm_sm_str %s\n",mmc_clk_dly[i].raw_tm_sm_str[0]);
		printk("raw_tm_sm_str %s\n",mmc_clk_dly[i].raw_tm_sm_str[1]);
		printk("raw_tm_sm0 %x\n",mmc_clk_dly[i].raw_tm_sm[0]);
		printk("raw_tm_sm1 %x\n",mmc_clk_dly[i].raw_tm_sm[1]);
		printk("********************\n");
	}
}



static int __sunxi_mmc_do_oclk_onoff(struct sunxi_mmc_host *host, u32 oclk_en,u32 pwr_save,u32 ignore_dat0)
{
	unsigned long expire = jiffies + msecs_to_jiffies(250);
	u32 rval;

	rval = mmc_readl(host, REG_CLKCR);
	rval &= ~(SDXC_CARD_CLOCK_ON | SDXC_LOW_POWER_ON | SDXC_MASK_DATA0);

	if (oclk_en)
		rval |= SDXC_CARD_CLOCK_ON;
	if(pwr_save)
		rval |= SDXC_LOW_POWER_ON;
	if(ignore_dat0)
		rval |= SDXC_MASK_DATA0;

	mmc_writel(host, REG_CLKCR, rval);

	dev_dbg(mmc_dev(host->mmc), "%s REG_CLKCR:%x\n",__FUNCTION__,mmc_readl(host,REG_CLKCR));

	rval = SDXC_START | SDXC_UPCLK_ONLY | SDXC_WAIT_PRE_OVER;
	mmc_writel(host, REG_CMDR, rval);

	do {
		rval = mmc_readl(host, REG_CMDR);
	} while (time_before(jiffies, expire) && (rval & SDXC_START));

	/* clear irq status bits set by the command */
	mmc_writel(host, REG_RINTR,
		   mmc_readl(host, REG_RINTR) & ~SDXC_SDIO_INTERRUPT);//????

	if (rval & SDXC_START) {
		dev_err(mmc_dev(host->mmc), "fatal err update clk timeout\n");
		return -EIO;
	}

	/*only use mask data0 when update clk,clear it when not update clk*/
	if(ignore_dat0)
		mmc_writel(host, REG_CLKCR, 
			mmc_readl(host, REG_CLKCR)&~SDXC_MASK_DATA0);

	return 0;
}

static int sunxi_mmc_oclk_onoff(struct sunxi_mmc_host *host, u32 oclk_en)
{
	struct device_node *np 		= NULL;
	struct mmc_host *mmc 	= host->mmc;
	int pwr_save = 0;
	int len	= 0;

	if (!mmc->parent || !mmc->parent->of_node){
		dev_err(mmc_dev(host->mmc), "no dts to parse power save mode\n");
		return -EIO; ;
	}

	np = mmc->parent->of_node;
	if (of_find_property(np, "sunxi-power-save-mode", &len))
		pwr_save = 1;
	return __sunxi_mmc_do_oclk_onoff(host,oclk_en,pwr_save,1);
}
int sunxi_mmc_clk_set_rate_for_sdmmc3(struct sunxi_mmc_host *host,
				  struct mmc_ios *ios)
{
	u32 mod_clk = 0;
	u32 src_clk = 0;
	u32 rval 		= 0;
	s32 err 		= 0;
	u32 rate		= 0;
	char *sclk_name = NULL;
	struct clk *mclk = host->clk_mmc;
	struct clk *sclk = NULL;
	struct device *dev = mmc_dev(host->mmc);
	int div = 0;

	if(ios->clock == 0){
		__sunxi_mmc_do_oclk_onoff(host, 0,0,1);
		return 0;
	}



	if((ios->bus_width == MMC_BUS_WIDTH_8)\
		&&(ios->timing == MMC_TIMING_UHS_DDR50)\
	){
		mod_clk = ios->clock<<2;
		div = 1;
	}else{
		mod_clk = ios->clock<<1;
		div = 0;
	}

	if (ios->clock<= 400000) {
		//sclk = of_clk_get(np, 0);
		sclk = clk_get(dev,"osc24m");
		sclk_name = "osc24m";
	} else {
		//sclk = clk_get(np, 1);
		sclk = clk_get(dev,"pll_periph");
		sclk_name = "pll_periph";
	}
	if (IS_ERR(sclk)) {
		dev_err(mmc_dev(host->mmc), "Error to get source clock %s\n",sclk_name);
		return -1;
	}

	sunxi_mmc_oclk_onoff(host, 0);

	err = clk_set_parent(mclk, sclk);
	if(err){
		dev_err(mmc_dev(host->mmc), "set parent failed\n");
		clk_put(sclk);
		return -1;
	}

	rate = clk_round_rate(mclk, mod_clk);

	dev_dbg(mmc_dev(host->mmc),"get round rate %d\n", rate);

	clk_disable_unprepare(host->clk_mmc);

	err = clk_set_rate(mclk, rate);
	if (err) {
		dev_err(mmc_dev(host->mmc),"set mclk rate error, rate %dHz\n",rate);
		clk_put(sclk);
		return -1;
	}

	rval = clk_prepare_enable(host->clk_mmc);
	if (rval) {
		dev_err(mmc_dev(host->mmc), "Enable mmc clk err %d\n", rval);
		return -1;
	}

	src_clk = clk_get_rate(sclk);
	clk_put(sclk);

	dev_dbg(mmc_dev(host->mmc),"set round clock %d, soure clk is %d\n", rate, src_clk);

#ifdef MMC_FPGA
	if((ios->bus_width == MMC_BUS_WIDTH_8)\
		&&(ios->timing == MMC_TIMING_UHS_DDR50)\
	){
		/* clear internal divider */
		rval = mmc_readl(host, REG_CLKCR);
		rval &= ~0xff;
		rval |= 1;
	}else{
		/* support internal divide clock under fpga environment  */
		rval = mmc_readl(host, REG_CLKCR);
		rval &= ~0xff;
		rval |= 24000000 / mod_clk / 2; // =24M/400K/2=0x1E
	}
	mmc_writel(host, REG_CLKCR, rval);
	dev_info(mmc_dev(host->mmc), "--FPGA REG_CLKCR: 0x%08x \n", mmc_readl(host, REG_CLKCR));
#else
	/* clear internal divider */
	rval = mmc_readl(host, REG_CLKCR);
	rval &= ~0xff;
	rval |= div;
	mmc_writel(host, REG_CLKCR, rval);
#endif


	if((ios->bus_width == MMC_BUS_WIDTH_8)\
		&&(ios->timing == MMC_TIMING_MMC_HS400)\
	){
		rval = mmc_readl(host,REG_EDSD);
		rval |= SDXC_HS400_MD_EN;
		mmc_writel(host,REG_EDSD,rval);
		rval = mmc_readl(host,REG_CSDC);
		rval &= ~SDXC_CRC_DET_PARA_MASK;
		rval |= SDXC_CRC_DET_PARA_HS400;
		mmc_writel(host,REG_CSDC,rval);
	}else{
		rval = mmc_readl(host,REG_EDSD);
		rval &= ~ SDXC_HS400_MD_EN;
		mmc_writel(host,REG_EDSD,rval);
		rval = mmc_readl(host,REG_CSDC);
		rval &= ~SDXC_CRC_DET_PARA_MASK;
		rval |= SDXC_CRC_DET_PARA_OTHER;
		mmc_writel(host,REG_CSDC,rval);
	}
	dev_dbg(mmc_dev(host->mmc), "--SDXC_REG_EDSD: 0x%08x \n", mmc_readl(host, REG_EDSD));
	dev_dbg(mmc_dev(host->mmc), "--SDXC_REG_CSDC: 0x%08x \n", mmc_readl(host, REG_CSDC));

	//sunxi_of_parse_clk_dly(host);
	if((ios->bus_width == MMC_BUS_WIDTH_8)\
		&&(ios->timing == MMC_TIMING_UHS_DDR50)\
	){
		ios->clock = rate>>2;
	}else{
		ios->clock = rate>>1;
	}

	sunxi_mmc_set_clk_dly(host,ios->clock,ios->bus_width,ios->timing);

	return sunxi_mmc_oclk_onoff(host, 1);
}

void sunxi_mmc_thld_ctl_for_sdmmc3(struct sunxi_mmc_host *host,
			  struct mmc_ios *ios, struct mmc_data *data)
{
	u32 bsz = data->blksz;
	u32 tdtl = (host->dma_tl & SDXC_TX_TL_MASK)<<2;		//unit:byte
	u32 rdtl = ((host->dma_tl & SDXC_RX_TL_MASK)>>16)<<2;//unit:byte
	u32 rval = 0;

	if( (data->flags & MMC_DATA_WRITE)
		&& (bsz <= SDXC_CARD_RD_THLD_SIZE)
		&& (bsz <= tdtl) ){
		rval = mmc_readl(host,REG_THLD);
		rval &=~SDXC_CARD_RD_THLD_MASK;
		rval |= data->blksz<<SDXC_CARD_RD_THLD_SIZE_SHIFT;
		rval |= SDXC_CARD_WR_THLD_ENB;
		mmc_writel(host,REG_THLD,rval);
	}else{
		rval = mmc_readl(host,REG_THLD);
		rval &= ~SDXC_CARD_WR_THLD_ENB;
		mmc_writel(host,REG_THLD,rval);
	}


	if( (data->flags & MMC_DATA_READ)
		&& (bsz <= SDXC_CARD_RD_THLD_SIZE)
		&& ((SDXC_FIFO_DETH<<2) >= (rdtl+bsz))      //((SDXC_FIFO_DETH<<2)-bsz) >= (rdtl)
		&& ((ios->timing == MMC_TIMING_MMC_HS200)
			||(ios->timing == MMC_TIMING_MMC_HS400)) ){
		rval = mmc_readl(host,REG_THLD);
		rval &= ~SDXC_CARD_RD_THLD_MASK;
		rval |= data->blksz<<SDXC_CARD_RD_THLD_SIZE_SHIFT;
		rval |= SDXC_CARD_RD_THLD_ENB;
		mmc_writel(host,REG_THLD,rval);
	}else{
		rval = mmc_readl(host,REG_THLD);
		rval &= ~SDXC_CARD_RD_THLD_ENB;
		mmc_writel(host,REG_THLD,rval);
	}

	dev_dbg(mmc_dev(host->mmc), "--SDXC_REG_THLD: 0x%08x \n", mmc_readl(host, REG_THLD));

}


void sunxi_mmc_save_spec_reg3(struct sunxi_mmc_host *host)
{
	bak_spec_regs.drv_dl	= mmc_readl(host,REG_DRV_DL);
	bak_spec_regs.samp_dl	= mmc_readl(host,REG_SAMP_DL);
	bak_spec_regs.ds_dl		= mmc_readl(host,REG_DS_DL);
	//bak_spec_regs.sd_ntsr	= mmc_readl(host,REG_SD_NTSR);
	bak_spec_regs.edsd		= mmc_readl(host,REG_EDSD);
	bak_spec_regs.csdc		= mmc_readl(host,REG_CSDC);
}

void sunxi_mmc_restore_spec_reg3(struct sunxi_mmc_host *host)
{
	mmc_writel(host,REG_DRV_DL,bak_spec_regs.drv_dl);
	mmc_writel(host,REG_SAMP_DL,bak_spec_regs.samp_dl);
	mmc_writel(host,REG_DS_DL,bak_spec_regs.ds_dl);
	//mmc_writel(host,REG_SD_NTSR,bak_spec_regs.sd_ntsr);
	mmc_writel(host,REG_EDSD,bak_spec_regs.edsd);
	mmc_writel(host,REG_CSDC,bak_spec_regs.csdc);
}



/*
extern int mmc_go_idle(struct mmc_host *host);
extern int mmc_send_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr);
extern int mmc_send_status(struct mmc_card *card, u32 *status);
extern void mmc_set_clock(struct mmc_host *host, unsigned int hz);
extern void mmc_set_timing(struct mmc_host *host, unsigned int timing);
extern void mmc_set_bus_width(struct mmc_host *host, unsigned int width);
void sunxi_mmc_do_shutdown2(struct platform_device * pdev)
{
	u32 ocr = 0;
	u32 err = 0;
	struct mmc_host *mmc = NULL;
	struct sunxi_mmc_host *host = NULL;
	u32 status = 0;

	mmc = platform_get_drvdata(pdev);
	if (mmc == NULL) {
		dev_err(&pdev->dev,"%s: mmc is NULL\n", __FUNCTION__);
		goto out;
	}

	host = mmc_priv(mmc);
	if (host == NULL) {
		dev_err(&pdev->dev,"%s: host is NULL\n", __FUNCTION__);
		goto out;
	}

	dev_info(mmc_dev(mmc),"try to disable cache\n");
	mmc_claim_host(mmc);
    err = mmc_cache_ctrl(mmc, 0);
	mmc_release_host(mmc);
    if (err){
		dev_err(mmc_dev(mmc),"disable cache failed\n");
		mmc_claim_host(mmc);//not release host to not allow android to read/write after shutdown
         goto out;
    }

	//claim host to not allow androd read/write during shutdown
	dev_dbg(mmc_dev(mmc),"%s: claim host\n", __FUNCTION__);
	mmc_claim_host(mmc);

	do {
		if (mmc_send_status(mmc->card, &status) != 0) {
			dev_err(mmc_dev(mmc),"%s: send status failed\n", __FUNCTION__);
			goto out; //err_out; //not release host to not allow android to read/write after shutdown
		}
	} while(status != 0x00000900);

	//mmc_card_set_ddr_mode(card);
	mmc_set_timing(mmc, MMC_TIMING_LEGACY);
	mmc_set_bus_width(mmc, MMC_BUS_WIDTH_1);
	mmc_set_clock(mmc, 400000);
	err = mmc_go_idle(mmc);
	if (err) {
		dev_err(mmc_dev(mmc),"%s: mmc_go_idle err\n", __FUNCTION__);
		goto out; //err_out; //not release host to not allow android to read/write after shutdown
	}

	if (mmc->card->type != MMC_TYPE_MMC) {//sd can support cmd1,so not send cmd1
		goto out;//not release host to not allow android to read/write after shutdown
	}

	err = mmc_send_op_cond(mmc, 0, &ocr);
	if (err) {
		dev_err(mmc_dev(mmc),"%s: first mmc_send_op_cond err\n", __FUNCTION__);
		goto out; //err_out; //not release host to not allow android to read/write after shutdown
	}

	err = mmc_send_op_cond(mmc, ocr | (1 << 30), &ocr);
	if (err) {
		dev_err(mmc_dev(mmc),"%s: mmc_send_op_cond err\n", __FUNCTION__);
		goto out; //err_out; //not release host to not allow android to read/write after shutdown
	}

	//do not release host to not allow android to read/write after shutdown
	goto out;

out:
	dev_info(mmc_dev(mmc),"%s: mmc shutdown exit..ok\n", __FUNCTION__);

	return ;
}
*/

int mmc_card_sleep(struct mmc_host *host);
int mmc_deselect_cards(struct mmc_host *host);
void mmc_power_off(struct mmc_host *host);
int mmc_card_sleepawake(struct mmc_host *host, int sleep);


static int sunxi_mmc_can_poweroff_notify(const struct mmc_card *card)
{
	return card &&
		mmc_card_mmc(card) &&
		(card->ext_csd.power_off_notification == EXT_CSD_POWER_ON);
}


static int sunxi_mmc_poweroff_notify(struct mmc_card *card, unsigned int notify_type)
{
	unsigned int timeout = card->ext_csd.generic_cmd6_time;
	int err;

	/* Use EXT_CSD_POWER_OFF_SHORT as default notification type. */
	if (notify_type == EXT_CSD_POWER_OFF_LONG)
		timeout = card->ext_csd.power_off_longtime;

	err = __mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			EXT_CSD_POWER_OFF_NOTIFICATION,
			notify_type, timeout, true, false, false);
	if (err)
		pr_err("%s: Power Off Notification timed out, %u\n",
		       mmc_hostname(card->host), timeout);

	/* Disable the power off notification after the switch operation. */
	card->ext_csd.power_off_notification = EXT_CSD_NO_POWER_NOTIFICATION;

	return err;
}


static int sunxi_mmc_sleep(struct mmc_host *host)
{
	struct mmc_card *card = host->card;
	int err = -ENOSYS;

	if (card && card->ext_csd.rev >= 3) {
		err = mmc_card_sleepawake(host, 1);
		if (err < 0)
			pr_debug("%s: Error %d while putting card into sleep",
				 mmc_hostname(host), err);
	}

	return err;
}



static int sunxi_mmc_suspend(struct mmc_host *host, bool is_suspend)
{
	int err = 0;
	unsigned int notify_type = is_suspend ? EXT_CSD_POWER_OFF_SHORT :
					EXT_CSD_POWER_OFF_LONG;

	BUG_ON(!host);
	BUG_ON(!host->card);

	mmc_claim_host(host);

	//if (mmc_card_suspended(host->card))
	//	goto out;

	if (mmc_card_doing_bkops(host->card)) {
		err = mmc_stop_bkops(host->card);
		if (err)
			goto out;
	}

	err = mmc_flush_cache(host->card);
	
	if (err)
		goto out;

	if (sunxi_mmc_can_poweroff_notify(host->card) &&
		((host->caps2 & MMC_CAP2_POWEROFF_NOTIFY) || !is_suspend)){
		err = sunxi_mmc_poweroff_notify(host->card, notify_type);
	}else if (mmc_card_can_sleep(host)){
		err = sunxi_mmc_sleep(host);
	}else if (!mmc_host_is_spi(host)){
		err = mmc_deselect_cards(host);
	}

	if (!err) {
		pr_info("%s: %s %d\n",
			mmc_hostname(host),__FUNCTION__,__LINE__);
		mmc_power_off(host);
//		mmc_card_set_suspended(host->card);
	}

out:
	mmc_release_host(host);
	return err;
}



void sunxi_mmc_do_shutdown3(struct platform_device * pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	u32 shutdown_notify_type = 0;
	u32 rval = of_property_read_u32(mmc->parent->of_node, "shutdown_notify_type", &shutdown_notify_type);
	if(!rval){
		sunxi_mmc_suspend(mmc ,shutdown_notify_type);
	}else{
		sunxi_mmc_suspend(mmc ,false);
	}
}


#endif











