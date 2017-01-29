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
	1: output and input are both based on phase
	3: output is based on phase, input is based on delay chain
	4: output is based on phase, input is based on delay chain.
	    it also support to use delay chain on data strobe signal.
*/
#define SUNXI_MMC_TIMING_MODE_1 1U
#define SUNXI_MMC_TIMING_MODE_3 3U
#define SUNXI_MMC_TIMING_MODE_4 4U

#define MMC_CLK_SAMPLE_POINIT_MODE_1 3U
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


/* smhc0&1 */
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

/* smhc2 */
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

struct sunxi_mmc_des {
	u32			:1,
		dic		:1, /* disable interrupt on completion */
		last_des	:1, /* 1-this data buffer is the last buffer */
		first_des	:1, /* 1-data buffer is the first buffer,
						   0-data buffer contained in the next descriptor is 1st buffer */
		des_chain	:1, /* 1-the 2nd address in the descriptor is the next descriptor address */
		end_of_ring	:1, /* 1-last descriptor flag when using dual data buffer in descriptor */
				:24,
		card_err_sum	:1, /* transfer error flag */
		own		:1; /* des owner:1-idma owns it, 0-host owns it */

#define SDXC_DES_NUM_SHIFT 12  /* smhc2!! */
#define SDXC_DES_BUFFER_MAX_LEN	(1 << SDXC_DES_NUM_SHIFT)
	u32	data_buf1_sz	:16,
		data_buf2_sz	:16;

	u32	buf_addr_ptr1;
	u32	buf_addr_ptr2;
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
	struct sunxi_mmc *reg;
	struct sunxi_mmc *reg_bak;
	struct sunxi_mmc_des* pdes;

	/*sample delay and output deley setting*/
	u32 timing_mode;
	struct sunxi_mmc_timing_mode1 tm1;
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
	u32 sample_mode;
};



//#define TUNING_LEN		(1)//The address which store the tuninng pattern
//#define TUNING_ADD		(38192-TUNING_LEN)//The address which store the tuninng pattern
#define TUNING_LEN		(10)//The length of the tuninng pattern
#define TUNING_ADD		(38192-2-TUNING_LEN)//The address which store the tuninng pattern
#define REPEAT_TIMES	(30)
#define SAMPLE_MODE 	(2)

//secure storage relate
#define MAX_SECURE_STORAGE_MAX_ITEM		32
#define SDMMC_SECURE_STORAGE_START_ADD	(6*1024*1024/512)//6M
#define SDMMC_ITEM_SIZE					(4*1024/512)//4K

#endif /* SUNXI_MMC_H */
