/*
 * Copyright 2008,2010 Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based (loosely) on the Linux code
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MMC_H_
#define _MMC_H_

#include <linux/list.h>
#include <linux/compiler.h>
#include <part.h>

#define SD_VERSION_SD	0x20000
#define SD_VERSION_3	(SD_VERSION_SD | 0x300)
#define SD_VERSION_2	(SD_VERSION_SD | 0x200)
#define SD_VERSION_1_0	(SD_VERSION_SD | 0x100)
#define SD_VERSION_1_10	(SD_VERSION_SD | 0x10a)
#define MMC_VERSION_MMC		0x10000
#define MMC_VERSION_UNKNOWN	(MMC_VERSION_MMC)
#define MMC_VERSION_1_2		(MMC_VERSION_MMC | 0x102)
#define MMC_VERSION_1_4		(MMC_VERSION_MMC | 0x104)
#define MMC_VERSION_2_2		(MMC_VERSION_MMC | 0x202)
#define MMC_VERSION_3		(MMC_VERSION_MMC | 0x300)
#define MMC_VERSION_4		(MMC_VERSION_MMC | 0x400)
#define MMC_VERSION_4_1		(MMC_VERSION_MMC | 0x401)
#define MMC_VERSION_4_2		(MMC_VERSION_MMC | 0x402)
#define MMC_VERSION_4_3		(MMC_VERSION_MMC | 0x403)
#define MMC_VERSION_4_41	(MMC_VERSION_MMC | 0x429)
#define MMC_VERSION_4_5		(MMC_VERSION_MMC | 0x405)
#define MMC_VERSION_5_0		(MMC_VERSION_MMC | 0x500)
#define MMC_VERSION_5_1		(MMC_VERSION_MMC | 0x501)

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

#define MMC_CMD_MANUAL	1//add by sunxi.not sent stop when read/write multi block,and sent stop when sent cmd12

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2

#define NO_CARD_ERR		-16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR		-17 /* Unusable Card */
#define COMM_ERR		-18 /* Communications Error */
#define TIMEOUT			-19
#define IN_PROGRESS		-20 /* operation is in progress */
#define SWITCH_ERR		-21 /* Card reports failure to switch mode */

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
#define MMC_CMD_SET_BLOCK_COUNT         23
#define MMC_CMD_WRITE_SINGLE_BLOCK	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_ERASE_GROUP_START	35
#define MMC_CMD_ERASE_GROUP_END		36
#define MMC_CMD_ERASE			38
#define MMC_CMD_APP_CMD			55
#define MMC_CMD_SPI_READ_OCR		58
#define MMC_CMD_SPI_CRC_ON_OFF		59
#define MMC_CMD_RES_MAN			62

#define MMC_CMD62_ARG1			0xefac62ec
#define MMC_CMD62_ARG2			0xcbaea7


#define SD_CMD_SEND_RELATIVE_ADDR	3
#define SD_CMD_SWITCH_FUNC		6
#define SD_CMD_SEND_IF_COND		8

#define SD_CMD_APP_SET_BUS_WIDTH	6
#define SD_CMD_ERASE_WR_BLK_START	32
#define SD_CMD_ERASE_WR_BLK_END		33
#define SD_CMD_APP_SEND_OP_COND		41
#define SD_CMD_APP_SEND_SCR		51

/* MMC erase/trim/discard/sanitize/secure erase/secure trim argument */
#define MMC_ERASE_ARG		0x00000000
#define MMC_SECURE_ERASE_ARG	0x80000000
#define MMC_TRIM_ARG		0x00000001
#define MMC_DISCARD_ARG		0x00000003
#define MMC_SECURE_TRIM1_ARG	0x80000001
#define MMC_SECURE_TRIM2_ARG	0x80008000

#define MMC_SECURE_ARGS		0x80000000
#define MMC_TRIM_ARGS		0x00008001


/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY	0x00020000
#define SD_HIGHSPEED_SUPPORTED	0x00020000

#define OCR_BUSY		0x80000000
#define OCR_HCS			0x40000000
#define OCR_VOLTAGE_MASK	0x007FFF80
#define OCR_ACCESS_MODE		0x60000000

#define SECURE_ERASE		0x80000000

#define MMC_STATUS_MASK		(~0x0206BF7F)
#define MMC_STATUS_SWITCH_ERROR	(1 << 7)
#define MMC_STATUS_RDY_FOR_DATA (1 << 8)
#define MMC_STATUS_CURR_STATE	(0xf << 9)
#define MMC_STATUS_ERROR	(1 << 19)
#define MMC_STATUS_ADDR_OUT_OF_RANGE (1<<31) /* WJQ */

#define MMC_STATE_PRG		(7 << 9)

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
#define EXT_CSD_GP_SIZE_MULT		143	/* R/W */
#define EXT_CSD_PARTITIONS_ATTRIBUTE	156	/* R/W */
#define EXT_CSD_PARTITIONING_SUPPORT	160	/* RO */
#define EXT_CSD_RST_N_FUNCTION		162	/* R/W */
#define EXT_CSD_RPMB_MULT		168	/* RO */
#define EXT_CSD_ERASE_GROUP_DEF		175	/* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH		177
#define EXT_CSD_PART_CONF		179	/* R/W */
#define EXT_CSD_BUS_WIDTH		183	/* R/W */
#define EXT_CSD_HS_TIMING		185	/* R/W */
#define EXT_CSD_REV			192	/* RO */
#define EXT_CSD_CARD_TYPE		196	/* RO */
#define EXT_CSD_SEC_CNT			212	/* RO, 4 bytes */
#define EXT_CSD_HC_WP_GRP_SIZE		221	/* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224	/* RO */
#define EXT_CSD_BOOT_MULT		226	/* RO */


/*
 * EXT_CSD fields
 */

#define EXT_CSD_BOOT_BUS_COND	177	/* R/W */
#define EXT_CSD_PART_CONF	179	/* R/W */
#define EXT_CSD_BUS_WIDTH	183	/* R/W */
#define EXT_CSD_HS_TIMING	185	/* R/W */
#define EXT_CSD_CARD_TYPE	196	/* RO */
#define EXT_CSD_REV		192	/* RO */
#define EXT_CSD_SEC_CNT		212	/* RO, 4 bytes */
#define EXT_CSD_SECURE_REMOAL_TYPE 16 /* R/W */
#define EXT_CSD_FLUSH_CACHE		32      /* W */
#define EXT_CSD_CACHE_CTRL		33      /* R/W */
#define EXT_CSD_POWER_OFF_NOTIFICATION	34	/* R/W */
#define EXT_CSD_PACKED_FAILURE_INDEX	35	/* RO */
#define EXT_CSD_PACKED_CMD_STATUS	36	/* RO */
#define EXT_CSD_EXP_EVENTS_STATUS	54	/* RO, 2 bytes */
#define EXT_CSD_EXP_EVENTS_CTRL		56	/* R/W, 2 bytes */
#define EXT_CSD_DATA_SECTOR_SIZE	61	/* R */
#define EXT_CSD_GP_SIZE_MULT		143	/* R/W */
#define EXT_CSD_PARTITION_ATTRIBUTE	156	/* R/W */
#define EXT_CSD_PARTITION_SUPPORT	160	/* RO */
#define EXT_CSD_HPI_MGMT		161	/* R/W */
#define EXT_CSD_RST_N_FUNCTION		162	/* R/W */
#define EXT_CSD_BKOPS_EN		163	/* R/W */
#define EXT_CSD_BKOPS_START		164	/* W */
#define EXT_CSD_SANITIZE_START		165     /* W */
#define EXT_CSD_WR_REL_PARAM		166	/* RO */
#define EXT_CSD_RPMB_MULT		168	/* RO */
#define EXT_CSD_BOOT_WP			173	/* R/W */
#define EXT_CSD_ERASE_GROUP_DEF		175	/* R/W */
#define EXT_CSD_PART_CONFIG		179	/* R/W */
#define EXT_CSD_ERASED_MEM_CONT		181	/* RO */
#define EXT_CSD_BUS_WIDTH		183	/* R/W */
#define EXT_CSD_HS_TIMING		185	/* R/W */
#define EXT_CSD_POWER_CLASS		187	/* R/W */
#define EXT_CSD_REV			    192	/* RO */
#define EXT_CSD_STRUCTURE		194	/* RO */
#define EXT_CSD_CARD_TYPE		196	/* RO */
#define EXT_CSD_OUT_OF_INTERRUPT_TIME	198	/* RO */
#define EXT_CSD_PART_SWITCH_TIME        199     /* RO */
#define EXT_CSD_PWR_CL_52_195		200	/* RO */
#define EXT_CSD_PWR_CL_26_195		201	/* RO */
#define EXT_CSD_PWR_CL_52_360		202	/* RO */
#define EXT_CSD_PWR_CL_26_360		203	/* RO */
#define EXT_CSD_SEC_CNT			    212	/* RO, 4 bytes */
#define EXT_CSD_S_A_TIMEOUT		    217	/* RO */
#define EXT_CSD_REL_WR_SEC_C		222	/* RO */
#define EXT_CSD_HC_WP_GRP_SIZE		221	/* RO */
#define EXT_CSD_ERASE_TIMEOUT_MULT	223	/* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224	/* RO */
#define EXT_CSD_BOOT_MULT		    226	/* RO */
#define EXT_CSD_SEC_TRIM_MULT		229	/* RO */
#define EXT_CSD_SEC_ERASE_MULT		230	/* RO */
#define EXT_CSD_SEC_FEATURE_SUPPORT	231	/* RO */
#define EXT_CSD_TRIM_MULT		    232	/* RO */
#define EXT_CSD_PWR_CL_200_195		236	/* RO */
#define EXT_CSD_PWR_CL_200_360		237	/* RO */
#define EXT_CSD_PWR_CL_DDR_52_195	238	/* RO */
#define EXT_CSD_PWR_CL_DDR_52_360	239	/* RO */
#define EXT_CSD_BKOPS_STATUS		246	/* RO */
#define EXT_CSD_POWER_OFF_LONG_TIME	247	/* RO */
#define EXT_CSD_GENERIC_CMD6_TIME	248	/* RO */
#define EXT_CSD_CACHE_SIZE		    249	/* RO, 4 bytes */
#define EXT_CSD_PWR_CL_DDR_200_360	253	/* RO */
#define EXT_CSD_PRE_EOL_INFO        267 /* RO */
#define EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_A 268 /* RO */
#define EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_B 269 /* RO */
#define EXT_CSD_VENDOR_HEALTH_REPORT 270 /* 270-301, RO */
#define EXT_CSD_TAG_UNIT_SIZE		498	/* RO */
#define EXT_CSD_DATA_TAG_SUPPORT	499	/* RO */
#define EXT_CSD_MAX_PACKED_WRITES	500	/* RO */
#define EXT_CSD_MAX_PACKED_READS	501	/* RO */
#define EXT_CSD_BKOPS_SUPPORT		502	/* RO */
#define EXT_CSD_HPI_FEATURES		503	/* RO */


/*
 * EXT_CSD field definitions
 */
#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

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

/*  */
#define EXT_CSD_BOOT_ACK_ENABLE			(1 << 6)
#define EXT_CSD_BOOT_PARTITION_ENABLE		(1 << 3)
#define EXT_CSD_PARTITION_ACCESS_ENABLE		(1 << 0)
#define EXT_CSD_PARTITION_ACCESS_DISABLE	(0 << 0)

/*  */
#define EXT_CSD_BOOT_ACK(x)		(x << 6)
#define EXT_CSD_BOOT_PART_NUM(x)	(x << 3)
#define EXT_CSD_PARTITION_ACCESS(x)	(x << 0)

#define EXT_CSD_BOOT_BUS_WIDTH_MODE(x)	(x << 3)
#define EXT_CSD_BOOT_BUS_WIDTH_RESET(x)	(x << 2)
#define EXT_CSD_BOOT_BUS_WIDTH_WIDTH(x)	(x)

/*  */
#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

//#define EXT_CSD_CARD_TYPE_26	(1 << 0)	/* Card can run at 26MHz */
//#define EXT_CSD_CARD_TYPE_52	(1 << 1)	/* Card can run at 52MHz */

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

/* -- EXT_CSD[231] SEC_FEATURE_SUPPORT */
#define EXT_CSD_SEC_ER_EN	    (1U << 0)
#define EXT_CSD_SEC_BD_BLK_EN	(1U << 2)
#define EXT_CSD_SEC_GB_CL_EN	(1U << 4)
#define EXT_CSD_SEC_SANITIZE	(1U << 6)  /* v4.5 only */

/* MMC_SWITCH boot modes */
#define MMC_SWITCH_MMCPART_NOAVAILABLE	(0xff)
#define MMC_SWITCH_PART_ACCESS_MASK		(0x7)
#define MMC_SWITCH_PART_SUPPORT			(0x1)
#define MMC_SWITCH_PART_BOOT_PART_MASK	(0x7 << 3)
#define MMC_SWITCH_PART_BOOT_PART_NONE	(0x0)
#define MMC_SWITCH_PART_BOOT_PART_1		(0x1)
#define MMC_SWITCH_PART_BOOT_PART_2		(0x2)
#define MMC_SWITCH_PART_BOOT_USER		(0x7)
#define MMC_SWITCH_PART_BOOT_ACK_MASK	(0x1 << 6)
#define MMC_SWITCH_PART_BOOT_ACK_ENB	(0x1)

/* MMC_SWITCH boot condition */
#define MMC_SWITCH_MMCBOOT_BUS_NOAVAILABLE	(0xff)
#define MMC_SWITCH_BOOT_MODE_MASK			(0x3 << 3)
#define MMC_SWITCH_BOOT_SDR_NORMAL			(0x0)
#define MMC_SWITCH_BOOT_SDR_HS				(0x1)
#define MMC_SWITCH_BOOT_DDR					(0x2)
#define MMC_SWITCH_BOOT_RST_BUS_COND_MASK	(0x1 << 2)
#define MMC_SWITCH_BOOT_RST_BUS_COND		(0x0)
#define MMC_SWITCH_BOOT_RETAIN_BUS_COND		(0x1)
#define MMC_SWITCH_BOOT_BUS_WIDTH_MASK		(0x3 << 0)
#define MMC_SWITCH_BOOT_BUS_SDRx1_DDRx4		(0x0)
#define MMC_SWITCH_BOOT_BUS_SDRx4_DDRx4		(0x1)
#define MMC_SWITCH_BOOT_BUS_SDRx8_DDRx8		(0x2)



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
#define PART_ENH_ATTRIB		(0x1f)

/* Maximum block size for MMC */
#define MMC_MAX_BLOCK_LEN	512

/* The number of MMC physical partitions.  These consist of:
 * boot partitions (2), general purpose partitions (4) in MMC v4.4.
 */
#define MMC_NUM_BOOT_PARTITION	2
#define MMC_PART_RPMB           3       /* RPMB partition number */

struct mmc_cid {
	unsigned long psn;
	unsigned short oid;
	unsigned char mid;
	unsigned char prv;
	unsigned char mdt;
	char pnm[7];
};

struct mmc_cmd {
	ushort cmdidx;
	uint resp_type;
	uint cmdarg;
	uint response[4];
	uint flags; /*wjq*/
};

struct mmc_data {
	union {
		char *dest;
		const char *src; /* src buffers don't get written to */
	};
	uint flags;
	uint blocks;
	uint blocksize;
};

/* forward decl. */
struct mmc;

struct mmc_ops {
	int (*send_cmd)(struct mmc *mmc,
			struct mmc_cmd *cmd, struct mmc_data *data);
	void (*set_ios)(struct mmc *mmc);
	int (*init)(struct mmc *mmc);
	int (*getcd)(struct mmc *mmc);
	int (*getwp)(struct mmc *mmc);

	/*
		add these members to impliment sample point auto-adaption
	*/
	int (*decide_retry)(struct mmc *mmc, int err_no, uint reset_count);
	int (*update_sdly)(struct mmc *mmc, uint sdly);
	int (*get_detail_errno)(struct mmc *mmc);

	int (*update_phase)(struct mmc *mmc);
};


/* size can not over 256 byte now */
#if 0
struct tuning_sdly{
	//u8 sdly_400k;
	u8 sdly_25M;
	u8 sdly_50M;
	u8 sdly_100M;
	u8 sdly_200M;
};
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

struct mmc_platform_caps {
	/* boot0 burn positon */
#define DRV_PARA_NOT_BURN_USER_PART           (1U<<0)
#define DRV_PARA_BURN_EMMC_BOOT_PART          (1U<<1)
	uint drv_burn_boot_pos;

	/* struct mmc/drv_wipe_feature, define for driver secure wipe opeation */
#define DRV_PARA_DISABLE_SECURE_WIPE          (1U<<0)
#define DRV_PARA_DISABLE_EMMC_SANITIZE        (1U<<1)
#define DRV_PARA_DISABLE_EMMC_SECURE_PURGE    (1U<<2)
#define DRV_PARA_DISABLE_EMMC_TRIM            (1U<<3)
	uint drv_wipe_feature;

	/* struct mmc/drv_erase_feature, define for drvier erase operation */
#define DRV_PARA_DISABLE_EMMC_ERASE               (1U<<0)
#define DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE  (1U<<1)
	uint drv_erase_feature;

#define AUTO_SAMPLE_MODE   (2)
#define MAUNAL_SAMPLE_MODE (1)
	uint sample_mode;

	uint io_is_1v8;

#define BOOT0_PARA_USE_INTERNAL_DEFAULT_TIMING_PARA (1U<<0)
/* 0: pass through struct sdly;  1: pass through boot_odly/sdly_* */
#define BOOT0_PARA_USE_EXTERNAL_INPUT_TIMING_PARA   (1U<<1)
	u8 boot0_para;

	u8 boot_odly_50M;
	u8 boot_sdly_50M;
	u8 boot_odly_50M_ddr;
	u8 boot_sdly_50M_ddr;
	u8 boot_hs_f_max;

	u8 *odly_spd_freq; //[40];
	u8 *sdly_spd_freq; //[40];

	u8 tm4_timing_window_th;
	u8 tm4_tune_r_cycle;
	u8 tm4_tune_hs200_max_freq;
	u8 tm4_tune_hs400_max_freq;
	u8 res[2];

	/* bit31: valid; bit23~16: speed mode; bit15~8: freq id; bit7:0 freq value */
#define MAX_EXT_FREQ_POINT_NUM (4)
	u32 tm4_tune_ext_freq[MAX_EXT_FREQ_POINT_NUM];

#define DRV_PARA_DISABLE_MMC_MODE_HS        (1 << 0) /* can run at 26MHz -- DS26_SDR12*/
#define DRV_PARA_DISABLE_MMC_MODE_HS_52MHz	(1 << 1) /* can run at 52MHz with SDR mode -- HSSDR52_SDR25 */
#define DRV_PARA_DISABLE_MMC_MODE_4BIT		(1 << 2)
#define DRV_PARA_DISABLE_MMC_MODE_8BIT		(1 << 3)
#define DRV_PARA_DISABLE_MMC_MODE_SPI		(1 << 4)
#define DRV_PARA_DISABLE_MMC_MODE_HC        (1 << 5)
#define DRV_PARA_DISABLE_MMC_MODE_DDR_52MHz	(1 << 6) /* can run at 52Mhz with DDR mode -- HSDDR52_DDR50 */
#define DRV_PARA_DISABLE_MMC_MODE_HS200     (1 << 7) /* can run at 200/208MHz with SDR mode -- HS200_SDR104*/
#define DRV_PARA_DISABLE_MMC_MODE_HS400     (1 << 8) /* can run at 200MHz with DDR mode -- HS400 */
	u32 host_caps_mask;

	struct tune_sdly sdly;
};

struct mmc_config {
	const char *name;
	const struct mmc_ops *ops;
	uint host_no;
	uint host_caps;
	struct mmc_platform_caps platform_caps;
	uint voltages;
	uint f_min;
	uint f_max;
	uint b_max;
	unsigned char part_type;
};

/* TODO struct mmc should be in mmc_private but it's hard to fix right now */
struct mmc {
	struct list_head link;
	const struct mmc_config *cfg;	/* provided configuration */
	uint version;
	void *priv;
	uint has_init;
	int high_capacity;
	uint bus_width;
	uint clock;
	uint card_caps;
	uint ocr;
	uint dsr;
	uint dsr_imp;
	uint scr[2];
	uint csd[4];

#define MMC_MID_HYNIX   0x90
#define MMC_MID_SANDISK 0x45
#define MMC_MID_SAMSUNG 0x15
#define MMC_MID_TOSHIBA 0x11
	uint cid[4];
	ushort rca;
	char part_config;
	char part_num;
	uint tran_speed;  /*  --WJQ: frequency requested at mmc layer */
	uint read_bl_len;
	uint write_bl_len;
	uint erase_grp_size;
	u64 capacity;
	u64 capacity_user;
	u64 capacity_boot;
	u64 capacity_rpmb;
	u64 capacity_gp[4];
	block_dev_desc_t block_dev;
	char op_cond_pending;	/* 1 if we are waiting on an op_cond command */
	char init_in_progress;	/* 1 if we have done mmc_start_init() */
	char preinit;		/* start init as early as possible */
	uint op_cond_response;	/* the response byte from the last op_cond */

	/*wjq*/
	uint speed_mode;
	uint io_mode;
	uint clock_after_init;
	uint boot_support;
	uchar boot_bus_cond;

	uint erase_timeout; /*default erasetimeout or hc_erase_timeout*/
	uint trim_discard_timeout;
	uint secure_erase_timeout;
	uint secure_trim_timeout;

	uchar secure_feature; // extcsd[231]
	uchar secure_removal_type; //extcsd[16]
	uchar pre_eol_info; //extcsd[267]
	uchar dev_life_time_typea; //extcsd[268]
	uchar dev_life_time_typeb; //extcsd[267]
	uchar vendor_health_report[32]; //extcsd[301-270]

	/*
		add these members to impliment sample point auto-adaption
	*/
	//u32 sample_mode;
	u32 pll_clock;
	u32 msglevel;
	u32 do_tuning;
};

struct mmc_ext_csd {
	u8			rev;
	u8			erase_group_def;
	u8			sec_feature_support;
	u8			rel_sectors;
	u8			rel_param;
	u8			part_config;
	u8			cache_ctrl;
	u8			rst_n_function;
	u8			max_packed_writes;
	u8			max_packed_reads;
	u8			packed_event_en;
	unsigned int		part_time;		/* Units: ms */
	unsigned int		sa_timeout;		/* Units: 100ns */
	unsigned int		generic_cmd6_time;	/* Units: 10ms */
	unsigned int            power_off_longtime;     /* Units: ms */
	u8			power_off_notification;	/* state */
	unsigned int		hs_max_dtr;
	unsigned int		hs200_max_dtr;
#define MMC_HIGH_26_MAX_DTR	26000000
#define MMC_HIGH_52_MAX_DTR	52000000
#define MMC_HIGH_DDR_MAX_DTR	52000000
#define MMC_HS200_MAX_DTR	200000000
	unsigned int		sectors;
	unsigned int		hc_erase_size;		/* In sectors */
	unsigned int		hc_erase_timeout;	/* In milliseconds */
	unsigned int		sec_trim_mult;	/* Secure trim multiplier  */
	unsigned int		sec_erase_mult;	/* Secure erase multiplier */
	unsigned int		trim_timeout;		/* In milliseconds */
	u8			enhanced_area_en;	/* enable bit */
	unsigned long long	enhanced_area_offset;	/* Units: Byte */
	unsigned int		enhanced_area_size;	/* Units: KB */
	unsigned int		cache_size;		/* Units: KB */
	u8			hpi_en;			/* HPI enablebit */
	u8			hpi;			/* HPI support bit */
	unsigned int		hpi_cmd;		/* cmd used as HPI */
	u8			bkops;		/* background support bit */
	u8			bkops_en;	/* background enable bit */
	unsigned int            data_sector_size;       /* 512 bytes or 4KB */
	unsigned int            data_tag_unit_size;     /* DATA TAG UNIT size */
	unsigned int		boot_ro_lock;		/* ro lock support */
	u8			boot_ro_lockable;
	u8			raw_exception_status;	/* 54 */
	u8			raw_partition_support;	/* 160 */
	u8			raw_rpmb_size_mult;	/* 168 */
	u8			raw_erased_mem_count;	/* 181 */
	u8			raw_ext_csd_structure;	/* 194 */
	u8			raw_card_type;		/* 196 */
	u8			out_of_int_time;	/* 198 */
	u8			raw_pwr_cl_52_195;	/* 200 */
	u8			raw_pwr_cl_26_195;	/* 201 */
	u8			raw_pwr_cl_52_360;	/* 202 */
	u8			raw_pwr_cl_26_360;	/* 203 */
	u8			raw_s_a_timeout;	/* 217 */
	u8			raw_hc_erase_gap_size;	/* 221 */
	u8			raw_erase_timeout_mult;	/* 223 */
	u8			raw_hc_erase_grp_size;	/* 224 */
	u8			raw_sec_trim_mult;	/* 229 */
	u8			raw_sec_erase_mult;	/* 230 */
	u8			raw_sec_feature_support;/* 231 */
	u8			raw_trim_mult;		/* 232 */
	u8			raw_pwr_cl_200_195;	/* 236 */
	u8			raw_pwr_cl_200_360;	/* 237 */
	u8			raw_pwr_cl_ddr_52_195;	/* 238 */
	u8			raw_pwr_cl_ddr_52_360;	/* 239 */
	u8			raw_pwr_cl_ddr_200_360;	/* 253 */
	u8			raw_bkops_status;	/* 246 */
	u8			raw_sectors[4];		/* 212 - 4 bytes */

	unsigned int            feature_support;
#define MMC_DISCARD_FEATURE	BIT(0)                  /* CMD38 feature */
};




int mmc_register(struct mmc *mmc);
struct mmc *mmc_create(const struct mmc_config *cfg, void *priv);
void mmc_destroy(struct mmc *mmc);
int mmc_initialize(bd_t *bis);
int mmc_init(struct mmc *mmc);
int mmc_read(struct mmc *mmc, u64 src, uchar *dst, int size);
void mmc_set_clock(struct mmc *mmc, uint clock);
struct mmc *find_mmc_device(int dev_num);
int mmc_set_dev(int dev_num);
void print_mmc_devices(char separator);
int get_mmc_num(void);
int board_mmc_getcd(struct mmc *mmc);
int mmc_switch_part(int dev_num, unsigned int part_num);
int mmc_getcd(struct mmc *mmc);
int mmc_getwp(struct mmc *mmc);
int mmc_set_dsr(struct mmc *mmc, u16 val);
/* Function to change the size of boot partition and rpmb partitions */
int mmc_boot_partition_size_change(struct mmc *mmc, unsigned long bootsize,
					unsigned long rpmbsize);
/* Function to modify the PARTITION_CONFIG field of EXT_CSD */
int mmc_set_part_conf(struct mmc *mmc, u8 ack, u8 part_num, u8 access);
/* Function to modify the BOOT_BUS_WIDTH field of EXT_CSD */
int mmc_set_boot_bus_width(struct mmc *mmc, u8 width, u8 reset, u8 mode);
/* Function to modify the RST_n_FUNCTION field of EXT_CSD */
int mmc_set_rst_n_function(struct mmc *mmc, u8 enable);
/* Functions to read / write the RPMB partition */
int mmc_rpmb_set_key(struct mmc *mmc, void *key);
int mmc_rpmb_get_counter(struct mmc *mmc, unsigned long *counter);
int mmc_rpmb_read(struct mmc *mmc, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key);
int mmc_rpmb_write(struct mmc *mmc, void *addr, unsigned short blk,
		   unsigned short cnt, unsigned char *key);
/**
 * Start device initialization and return immediately; it does not block on
 * polling OCR (operation condition register) status.  Then you should call
 * mmc_init, which would block on polling OCR status and complete the device
 * initializatin.
 *
 * @param mmc	Pointer to a MMC device struct
 * @return 0 on success, IN_PROGRESS on waiting for OCR status, <0 on error.
 */
int mmc_start_init(struct mmc *mmc);

/**
 * Set preinit flag of mmc device.
 *
 * This will cause the device to be pre-inited during mmc_initialize(),
 * which may save boot time if the device is not accessed until later.
 * Some eMMC devices take 200-300ms to init, but unfortunately they
 * must be sent a series of commands to even get them to start preparing
 * for operation.
 *
 * @param mmc		Pointer to a MMC device struct
 * @param preinit	preinit flag value
 */
void mmc_set_preinit(struct mmc *mmc, int preinit);

#ifdef CONFIG_GENERIC_MMC
#ifdef CONFIG_MMC_SPI
#define mmc_host_is_spi(mmc)	((mmc)->cfg->host_caps & MMC_MODE_SPI)
#else
#define mmc_host_is_spi(mmc)	0
#endif
struct mmc *mmc_spi_init(uint bus, uint cs, uint speed, uint mode);
#else
int mmc_legacy_init(int verbose);
#endif

int board_mmc_init(bd_t *bis);

/* Set block count limit because of 16 bit register limit on some hardware*/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

int mmc_send_ext_csd(struct mmc *mmc, char *ext_csd);
int mmc_decode_ext_csd(struct mmc *mmc,struct mmc_ext_csd *dec_ext_csd, char *ext_csd);
int mmc_do_switch(struct mmc *mmc, u8 set, u8 index, u8 value, u32 timeout);

ulong mmc_bread(int dev_num, lbaint_t start, lbaint_t blkcnt, void *dst);
ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt, const void *src);
ulong mmc_berase(int dev_num, lbaint_t start, lbaint_t blkcnt);
int mmc_switch_boot_bus_cond(int dev_num, u32 boot_mode, u32 rst_bus_cond, u32 bus_width);
int mmc_switch_boot_part(int dev_num, u32 boot_ack, u32 boot_part);
int mmc_switch_part(int dev_num, unsigned int part_num);
int mmc_secure_wipe(int dev_num, unsigned int start,
	unsigned int blkcnt, unsigned int *skip_space);
int mmc_mmc_erase(int dev_num, unsigned int start,
	unsigned int blkcnt, unsigned int *skip_space);
int mmc_mmc_sanitize(int dev_num);
int mmc_mmc_trim(int dev_num, unsigned int start, unsigned int blkcnt);
int mmc_mmc_discard(int dev_num, unsigned int start, unsigned int blkcnt);
int mmc_mmc_secure_erase(int dev_num, unsigned int start,
	unsigned int blkcnt, unsigned int *skip_space);
int mmc_mmc_secure_trim(int dev_num, unsigned int start, unsigned int blkcnt);

int mmc_send_manual_stop(struct mmc *mmc);
//void mmc_set_bus_width(struct mmc *mmc, uint width);
int sunxi_need_rty(struct mmc *mmc);
int sunxi_write_tuning(struct mmc *mmc);
int sunxi_bus_tuning(struct mmc *mmc);
int sunxi_switch_to_best_bus(struct mmc *mmc);
int mmc_init_product(struct mmc *mmc);
int mmc_exit(void);


#endif /* _MMC_H_ */
