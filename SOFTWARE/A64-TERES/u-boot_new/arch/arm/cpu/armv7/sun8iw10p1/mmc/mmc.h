/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: MMC driver for general mmc operations
 * Author: Aaron <leafy.myeh@allwinnertech.com>
 * Date: 2012-2-3 14:18:18
 */

#ifndef _MMC_H_
#define _MMC_H_

#define SD_VERSION_SD	0x20000
#define SD_VERSION_2	(SD_VERSION_SD | 0x20)
#define SD_VERSION_1_0	(SD_VERSION_SD | 0x10)
#define SD_VERSION_1_10	(SD_VERSION_SD | 0x1a)
#define MMC_VERSION_MMC		0x10000
#define MMC_VERSION_UNKNOWN	(MMC_VERSION_MMC)
#define MMC_VERSION_1_2		(MMC_VERSION_MMC | 0x12)
#define MMC_VERSION_1_4		(MMC_VERSION_MMC | 0x14)
#define MMC_VERSION_2_2		(MMC_VERSION_MMC | 0x22)
#define MMC_VERSION_3		(MMC_VERSION_MMC | 0x30)
#define MMC_VERSION_4		(MMC_VERSION_MMC | 0x40)
#define MMC_VERSION_4_1		(MMC_VERSION_MMC | 0x41)
#define MMC_VERSION_4_2		(MMC_VERSION_MMC | 0x42)
#define MMC_VERSION_4_3		(MMC_VERSION_MMC | 0x43)
#define MMC_VERSION_4_41	(MMC_VERSION_MMC | 0x44)
#define MMC_VERSION_4_5		(MMC_VERSION_MMC | 0x45)
#define MMC_VERSION_5_0		(MMC_VERSION_MMC | 0x50)
#define MMC_VERSION_5_1		(MMC_VERSION_MMC | 0x51)

#define MMC_MODE_HS		    (1 << 0) /* can run at 26MHz -- DS26_SDR12*/
#define MMC_MODE_HS_52MHz	(1 << 1) /* can run at 52MHz with SDR mode -- HSSDR52_SDR25 */
#define MMC_MODE_4BIT		(1 << 2)
#define MMC_MODE_8BIT		(1 << 3)
#define MMC_MODE_SPI		(1 << 4)
#define MMC_MODE_HC		    (1 << 5)
#define MMC_MODE_DDR_52MHz	(1 << 6) /* can run at 52Mhz with DDR mode -- HSDDR52_DDR50 */
#define MMC_MODE_HS200      (1 << 7) /* can run at 200/208MHz with SDR mode -- HS200_SDR104*/
#define MMC_MODE_HS400      (1 << 8) /* can run at 200MHz with DDR mode -- HS400 */

#define SD_DATA_4BIT	0x00040000

#define IS_SD(x) (x->version & SD_VERSION_SD)

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2

#define NO_CARD_ERR		-16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR		-17 /* Unusable Card */
#define COMM_ERR		-18 /* Communications Error */
#define TIMEOUT			-19

#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_EXT_CSD		8
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_SEND_CID		10
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_WRITE_SINGLE_BLOCK	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_ERASE_GROUP_START	35
#define MMC_CMD_ERASE_GROUP_END		36
#define MMC_CMD_ERASE			38
#define MMC_CMD_APP_CMD			55
#define MMC_CMD_SPI_READ_OCR		58
#define MMC_CMD_SPI_CRC_ON_OFF		59

#define SD_CMD_SEND_RELATIVE_ADDR	3
#define SD_CMD_SWITCH_FUNC		6
#define SD_CMD_SEND_IF_COND		8

#define SD_CMD_APP_SET_BUS_WIDTH	6
#define SD_CMD_ERASE_WR_BLK_START	32
#define SD_CMD_ERASE_WR_BLK_END		33
#define SD_CMD_APP_SEND_OP_COND		41
#define SD_CMD_APP_SEND_SCR		51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY	0x00020000
#define SD_HIGHSPEED_SUPPORTED	0x00020000

#define MMC_HS_TIMING		0x00000100
#define MMC_HS_52MHZ		0x2
#define MMC_DDR_52MHZ       0x4

#define OCR_BUSY		0x80000000
#define OCR_HCS			0x40000000
#define OCR_VOLTAGE_MASK	0x007FFF80
#define OCR_ACCESS_MODE		0x60000000

#define SECURE_ERASE		0x80000000

#define MMC_STATUS_MASK		(~0x0206BF7F)
#define MMC_STATUS_RDY_FOR_DATA (1 << 8)
#define MMC_STATUS_CURR_STATE	(0xf << 9)
#define MMC_STATUS_ERROR	(1 << 19)

#define MMC_VDD_165_195		0x00000080	/* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21		0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22		0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23		0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24		0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25		0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26		0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27		0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28		0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29		0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30		0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31		0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32		0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33		0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34		0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35		0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36		0x00800000	/* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET		0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01 /* Set bits in EXT_CSD byte
						addressed by index which are
						1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02 /* Clear bits in EXT_CSD byte
						addressed by index, which are
						1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1

/*
 * EXT_CSD fields
 */

#define EXT_CSD_PART_CONF	179	/* R/W */
#define EXT_CSD_BUS_WIDTH	183	/* R/W */
#define EXT_CSD_HS_TIMING	185	/* R/W */
#define EXT_CSD_CARD_TYPE	196	/* RO */
#define EXT_CSD_REV		192	/* RO */
#define EXT_CSD_SEC_CNT		212	/* RO, 4 bytes */

/*
 * EXT_CSD field definitions
 */

#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

//#define EXT_CSD_CARD_TYPE_26	(1 << 0)	/* Card can run at 26MHz */
//#define EXT_CSD_CARD_TYPE_52	(1 << 1)	/* Card can run at 52MHz */

/* -- EXT_CSD[196] DEVICE_TYPE */
#define EXT_CSD_CARD_TYPE_HS_26	        (1<<0)	/* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_HS_52	        (1<<1)	/* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_HS	        (EXT_CSD_CARD_TYPE_HS_26 | EXT_CSD_CARD_TYPE_HS_52)
#define EXT_CSD_CARD_TYPE_DDR_1_8V      (1<<2)   /* Card can run at 52MHz */ /* DDR mode @1.8V or 3V I/O */
#define EXT_CSD_CARD_TYPE_DDR_1_2V      (1<<3)   /* Card can run at 52MHz */ /* DDR mode @1.2V I/O */
#define EXT_CSD_CARD_TYPE_DDR_52        (EXT_CSD_CARD_TYPE_DDR_1_8V | EXT_CSD_CARD_TYPE_DDR_1_2V)
#define EXT_CSD_CARD_TYPE_HS200_1_8V	(1<<4)	/* Card can run at 200MHz */
#define EXT_CSD_CARD_TYPE_HS200_1_2V	(1<<5)	/* Card can run at 200MHz *//* SDR mode @1.2V I/O */
#define EXT_CSD_CARD_TYPE_HS200		    (EXT_CSD_CARD_TYPE_HS200_1_8V | EXT_CSD_CARD_TYPE_HS200_1_2V)
#define EXT_CSD_CARD_TYPE_HS400_1_8V	(1<<6)	/* Card can run at 200MHz DDR, 1.8V */
#define EXT_CSD_CARD_TYPE_HS400_1_2V	(1<<7)	/* Card can run at 200MHz DDR, 1.2V */
#define EXT_CSD_CARD_TYPE_HS400		    (EXT_CSD_CARD_TYPE_HS400_1_8V | EXT_CSD_CARD_TYPE_HS400_1_2V)


/* -- EXT_CSD[183] BUS_WIDTH */
#define EXT_CSD_BUS_WIDTH_1	0	/* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4	1	/* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8	2	/* Card is in 8 bit mode */
#define EXT_CSD_BUS_DDR_4	5	/* Card is in 4 bit ddr mode */
#define EXT_CSD_BUS_DDR_8	6	/* Card is in 8 bit ddr mode */

/* -- EXT_CSD[185] HS_TIMING */
#define EXT_CSD_TIMING_BC	           0	/* Backwards compatibility */
#define EXT_CSD_TIMING_HS	           1	/* High speed */
#define EXT_CSD_TIMING_HS200	       2	/* HS200 */
#define EXT_CSD_TIMING_HS400	       3	/* HS400 */

#define R1_ILLEGAL_COMMAND		(1 << 22)
#define R1_APP_CMD			(1 << 5)

#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
			MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define MMCPART_NOAVAILABLE	(0xff)
#define PART_ACCESS_MASK	(0x7)
#define PART_SUPPORT		(0x1)

struct mmc_cid {
	unsigned long psn;
	unsigned short oid;
	unsigned char mid;
	unsigned char prv;
	unsigned char mdt;
	char pnm[7];
};

/*
 * WARNING!
 *
 * This structure is used by atmel_mci.c only.
 * It works for the AVR32 architecture but NOT
 * for ARM/AT91 architectures.
 * Its use is highly depreciated.
 * After the atmel_mci.c driver for AVR32 has
 * been replaced this structure will be removed.
 */
/*
struct mmc_csd
{
	u8	csd_structure:2,
		spec_vers:4,
		rsvd1:2;
	u8	taac;
	u8	nsac;
	u8	tran_speed;
	u16	ccc:12,
		read_bl_len:4;
	u64	read_bl_partial:1,
		write_blk_misalign:1,
		read_blk_misalign:1,
		dsr_imp:1,
		rsvd2:2,
		c_size:12,
		vdd_r_curr_min:3,
		vdd_r_curr_max:3,
		vdd_w_curr_min:3,
		vdd_w_curr_max:3,
		c_size_mult:3,
		sector_size:5,
		erase_grp_size:5,
		wp_grp_size:5,
		wp_grp_enable:1,
		default_ecc:2,
		r2w_factor:3,
		write_bl_len:4,
		write_bl_partial:1,
		rsvd3:5;
	u8	file_format_grp:1,
		copy:1,
		perm_write_protect:1,
		tmp_write_protect:1,
		file_format:2,
		ecc:2;
	u8	crc:7;
	u8	one:1;
};
*/

#if 0
struct tuning_sdly{
	//u8 sdly_400k;
	u8 sdly_25M;
	u8 sdly_50M;
	u8 sdly_100M;
	u8 sdly_200M;
};//size can not over 256 now
#else
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
struct tune_sdly {
/*
	u32 tm4_sm0_f3210;
	u32 tm4_sm0_f7654;
	u32 tm4_sm1_f3210;
	u32 tm4_sm1_f7654;
	u32 tm4_sm2_f3210;
	u32 tm4_sm2_f7654;
	u32 tm4_sm3_f3210;
	u32 tm4_sm3_f7654;
	u32 tm4_sm4_f3210;
	u32 tm4_sm4_f7654;
*/
	u32 tm4_smx_fx[10];
};

struct boot_mmc_cfg {
	u8 boot0_para;
	u8 boot_odly_50M;
	u8 boot_sdly_50M;
	u8 boot_odly_50M_ddr;
	u8 boot_sdly_50M_ddr;
	u8 boot_hs_f_max;
	u8 res[2];
};

#define SDMMC_PRIV_INFO_ADDR_OFFSET (128)
struct boot_sdmmc_private_info_t {
	struct tune_sdly tune_sdly;
	struct boot_mmc_cfg boot_mmc_cfg;

	#define CARD_TYPE_SD  0x8000001
	#define CARD_TYPE_MMC 0x8000000
	#define CARD_TYPE_NULL 0xffffffff
	u32 card_type;  /*0xffffffff: invalid; 0x8000000: mmc card; 0x8000001: sd card*/
};
#endif

struct mmc {
	char name[32];
	void *priv;
	unsigned voltages;
	unsigned version;
	unsigned has_init;
	unsigned  control_num;
	unsigned f_min;
	unsigned f_max;
	unsigned f_max_ddr;
	int high_capacity;
	unsigned bus_width;
	unsigned clock;
	unsigned card_caps;
	unsigned host_caps;
	unsigned ocr;
	unsigned scr[2];
	unsigned csd[4];
	unsigned cid[4];
	unsigned short rca;
	char part_config;
	char part_num;
	unsigned tran_speed;
	unsigned read_bl_len;
	unsigned write_bl_len;
	unsigned erase_grp_size;
	unsigned long long capacity;
	int (*send_cmd)(struct mmc *mmc,
			struct mmc_cmd *cmd, struct mmc_data *data);
	void (*set_ios)(struct mmc *mmc);
	int (*init)(struct mmc *mmc);
	int (*update_phase)(struct mmc *mmc);
#if 0
	struct tuning_sdly sdly_tuning;
#else
	struct tune_sdly tune_sdly;
#endif
	unsigned b_max;
    unsigned lba;        /* number of blocks */
    unsigned blksz;      /* block size */
	char revision[8+1];	/* CID:  PRV */

    uint speed_mode;
};

#define mmc_host_is_spi(mmc)	((mmc)->host_caps & MMC_MODE_SPI)

#endif /* _MMC_H_ */
