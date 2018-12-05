#ifndef SUNXI_MMC_H
#define SUNXI_MMC_H

/* speed mode */
#define DS26_SDR12            (0)
#define HSSDR52_SDR25         (1)
#define HSDDR52_DDR50         (2)
#define HS200_SDR104          (3)
#define HS400                 (4)
#define MAX_SPD_MD_NUM        (5)

/* frequency point */
#define CLK_400K         (0)
#define CLK_25M          (1)
#define CLK_50M          (2)
#define CLK_100M         (3)
#define CLK_150M         (4)
#define CLK_200M         (5)
#define MAX_CLK_FREQ_NUM (8)

/*
timing mode
1: output and input are both based on phase.
2: output is based on phase, input is based on delay chain except hs400.
	input of hs400 is based on delay chain.
3: output is based on phase, input is based on delay chain.
4: output is based on phase, input is based on delay chain.
    it also support to use delay chain on data strobe signal.
*/
#define SUNXI_MMC_TIMING_MODE_1 1U
#define SUNXI_MMC_TIMING_MODE_2 2U
#define SUNXI_MMC_TIMING_MODE_3 3U
#define SUNXI_MMC_TIMING_MODE_4 4U

#define MMC_CLK_SAMPLE_POINIT_MODE_1 3U
#define MMC_CLK_SAMPLE_POINIT_MODE_2 2U
#define MMC_CLK_SAMPLE_POINIT_MODE_2_HS400 64U
#define MMC_CLK_SAMPLE_POINIT_MODE_3 64U
#define MMC_CLK_SAMPLE_POINIT_MODE_4 64U

#define TM1_OUT_PH90   (0)
#define TM1_OUT_PH180  (1)
#define TM1_IN_PH90    (0)
#define TM1_IN_PH180   (1)
#define TM1_IN_PH270   (2)

#define TM3_OUT_PH90   (0)
#define TM3_OUT_PH180  (1)

#define TM4_OUT_PH90   (0)
#define TM4_OUT_PH180  (1)

/* error number defination */
#define ERR_NO_BEST_DLY (2)

/* for smhc v4.1x*/
struct sunxi_mmc_timing_mode1 {
	u32 cur_spd_md;
	u32 cur_freq;
	u8 odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u32 sample_point_cnt;
	u8 cur_odly;
	u8 cur_sdly;
};

/* for smhc v4.1x*/
struct sunxi_mmc_timing_mode3 {
	u32 cur_spd_md;
	u32 cur_freq;
	u8 odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u32 sample_point_cnt;
	u32 sdly_unit_ps;
	u8 dly_calibrate_done;
	u8 cur_odly;
	u8 cur_sdly;
};

/* for smhc v4.5x*/
struct sunxi_mmc_timing_mode4 {
	u32 cur_spd_md;
	u32 cur_freq;
	u8 odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 dsdly[MAX_CLK_FREQ_NUM];
	u8 def_odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_dsdly[MAX_CLK_FREQ_NUM];
	u32 sample_point_cnt;
	u32 sdly_unit_ps;
	u32 dsdly_unit_ps;
	u8 dly_calibrate_done;
	u8 cur_odly;
	u8 cur_sdly;
	u8 cur_dsdly;
};

/* for smhc v5.1x*/
struct sunxi_mmc_timing_mode2 {
	u32 cur_spd_md;
	u32 cur_freq;
	u8 odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 dsdly[MAX_CLK_FREQ_NUM];
	u8 def_odly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_sdly[MAX_SPD_MD_NUM*MAX_CLK_FREQ_NUM];
	u8 def_dsdly[MAX_CLK_FREQ_NUM];
	u32 sample_point_cnt;
	//u32 sdly_unit_ps;
	u32 sample_point_cnt_hs400;
	u32 dsdly_unit_ps;
	u8 dly_calibrate_done;
	u8 cur_odly;
	u8 cur_sdly;
	u8 cur_dsdly;
};

struct sunxi_mmc_host {
	u32 mmc_no;

	u32 hclkbase;
	u32 hclkrst;
	u32 mclkbase;
	u32 database;

	u32 fatal_err;
	u32 clock; /* @clock, bankup current clock at host,  is updated when configure clock over */
	u32 mod_clk;
	void *reg;//struct sunxi_mmc *reg;
	void *reg_bak;//struct sunxi_mmc *reg_bak;
	void *pdes;//struct sunxi_mmc_des* pdes;

	/*sample delay and output deley setting*/
	u32 timing_mode;
	struct sunxi_mmc_timing_mode1 tm1;
	struct sunxi_mmc_timing_mode2 tm2;
	struct sunxi_mmc_timing_mode3 tm3;
	struct sunxi_mmc_timing_mode4 tm4;
	/* @retry_cnt used to count the retry times at a spcific speed mode and frequency during initial process or
		tuning process. it is always equal or less than the number of sample point.
	*/
	u32 retry_cnt;

	struct mmc *mmc;
	struct mmc_config cfg;

	/*sample delay and output deley setting*/
	u32 raw_int_bak;
	u32 acmd_err_bak;
	u32 sample_mode;
};



//#define TUNING_LEN		(1)//The address which store the tuninng pattern
//#define TUNING_ADD		(38192-TUNING_LEN)//The address which store the tuninng pattern
#define TUNING_LEN		(60)//The length of the tuninng pattern
#define TUNING_ADD		(24576-4-TUNING_LEN)//The address which store the tuninng pattern
#define REPEAT_TIMES		(30)
#define SAMPLE_MODE 		(2)

//secure storage relate
#define MAX_SECURE_STORAGE_MAX_ITEM		32
#define SDMMC_SECURE_STORAGE_START_ADD		(6*1024*1024/512)//6M
#define SDMMC_ITEM_SIZE				(4*1024/512)//4K

/* IDMA status bit field */
#define SDXC_IDMACTransmitInt		BIT(0)
#define SDXC_IDMACReceiveInt		BIT(1)
#define SDXC_IDMACFatalBusErr		BIT(2)
#define SDXC_IDMACDesInvalid		BIT(4)
#define SDXC_IDMACCardErrSum		BIT(5)
#define SDXC_IDMACNormalIntSum		BIT(8)
#define SDXC_IDMACAbnormalIntSum 	BIT(9)
#define SDXC_IDMACHostAbtInTx		BIT(10)
#define SDXC_IDMACHostAbtInRx		BIT(10)
#define SDXC_IDMACIdle			(0U << 13)
#define SDXC_IDMACSuspend		(1U << 13)
#define SDXC_IDMACDESCRd		(2U << 13)
#define SDXC_IDMACDESCCheck		(3U << 13)
#define SDXC_IDMACRdReqWait		(4U << 13)
#define SDXC_IDMACWrReqWait		(5U << 13)
#define SDXC_IDMACRd			(6U << 13)
#define SDXC_IDMACWr			(7U << 13)
#define SDXC_IDMACDESCClose		(8U << 13)

/* delay control */
#define SDXC_StartCal        		(1<<15)
#define SDXC_CalDone         		(1<<14)
#define SDXC_CalDly          		(0x3F<<8)
#define SDXC_EnableDly       		(1<<7)
#define SDXC_CfgDly          		(0x3F<<0)



extern void dumphex32(char* name, char* base, int len);
int mmc_clk_io_onoff(int sdc_no, int onoff, int reset_clk);

//#define SUPPORT_SUNXI_MMC_FFU

#endif /* SUNXI_MMC_H */
