/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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
#ifndef __BSP_NAND_H__
#define __BSP_NAND_H__

#include <nand.h>

#ifndef __int64
	#define __int64 __u64
#endif
#ifndef uchar
typedef unsigned char   uchar;
#endif
#ifndef uint16
typedef unsigned short  uint16;
#endif
#ifndef uint32
typedef unsigned int    uint32;
#endif
#ifndef sint32
typedef  int            sint32;
#endif
#ifndef uint64
typedef __int64         uint64;
#endif
#ifndef sint16
typedef short           sint16;
#endif
#ifndef UINT8
typedef unsigned char   UINT8;
#endif
#ifndef UINT32
typedef unsigned int    UINT32;
#endif
#ifndef SINT32
typedef  signed int     SINT32;
#endif


//for simplie(boot0)
struct _nand_super_block{
    unsigned short  Block_NO;
    unsigned short  Chip_NO;
};

struct _nand_info{
	unsigned short                  type;
    unsigned short                  SectorNumsPerPage;
    unsigned short                  BytesUserData;
    unsigned short                  PageNumsPerBlk;
	unsigned short                  BlkPerChip;
    unsigned short                  ChipNum;
	__u64                           FullBitmap;
	struct _nand_super_block        bad_block_addr;
	struct _nand_super_block        mbr_block_addr;
	struct _nand_super_block        no_used_block_addr;
    struct _nand_super_block*       factory_bad_block;
    unsigned char*                  mbr_data;
	void*                           phy_partition_head;
};

struct _physic_nand_info{
	unsigned short                  type;
    unsigned short                  SectorNumsPerPage;
    unsigned short                  BytesUserData;
    unsigned short                  PageNumsPerBlk;
	unsigned short                  BlkPerChip;
    unsigned short                  ChipNum;
	__int64                         FullBitmap;
};

typedef struct
{
	__u32		ChannelCnt;
	__u32        ChipCnt;                            //the count of the total nand flash chips are currently connecting on the CE pin
    __u32       ChipConnectInfo;                    //chip connect information, bit == 1 means there is a chip connecting on the CE pin
	__u32		RbCnt;
	__u32		RbConnectInfo;						//the connect  information of the all rb  chips are connected
    __u32        RbConnectMode;						//the rb connect  mode
	__u32        BankCntPerChip;                     //the count of the banks in one nand chip, multiple banks can support Inter-Leave
    __u32        DieCntPerChip;                      //the count of the dies in one nand chip, block management is based on Die
    __u32        PlaneCntPerDie;                     //the count of planes in one die, multiple planes can support multi-plane operation
    __u32        SectorCntPerPage;                   //the count of sectors in one single physic page, one sector is 0.5k
    __u32       PageCntPerPhyBlk;                   //the count of physic pages in one physic block
    __u32       BlkCntPerDie;                       //the count of the physic blocks in one die, include valid block and invalid block
    __u32       OperationOpt;                       //the mask of the operation types which current nand flash can support support
    __u32        FrequencePar;                       //the parameter of the hardware access clock, based on 'MHz'
    __u32        EccMode;                            //the Ecc Mode for the nand flash chip, 0: bch-16, 1:bch-28, 2:bch_32
    __u8        NandChipId[8];                      //the nand chip id of current connecting nand chip
    __u32       ValidBlkRatio;                      //the ratio of the valid physical blocks, based on 1024
	__u32 		good_block_ratio;					//good block ratio get from hwscan
	__u32		ReadRetryType;						//the read retry type
	__u32       DDRType;
	__u32		Reserved[22];
}boot_nand_para_t;

typedef struct boot_flash_info{
	__u32 chip_cnt;
	__u32 blk_cnt_per_chip;
	__u32 blocksize;
	__u32 pagesize;
	__u32 pagewithbadflag; /*bad block flag was written at the first byte of spare area of this page*/
}boot_flash_info_t;


struct boot_physical_param{
	__u32   chip; //chip no
	__u32  block; // block no within chip
	__u32  page; // apge no within block
	__u32  sectorbitmap; //done't care
	void   *mainbuf; //data buf
	void   *oobbuf; //oob buf
};

//extern __s32 PHY_SimpleErase(struct boot_physical_param * eraseop);
//extern __s32 PHY_SimpleErase_2CH(struct boot_physical_param * eraseop);

//extern __s32 PHY_SimpleRead(struct boot_physical_param * readop);
extern int nand_physic_read_page(unsigned int chip,unsigned int block,unsigned int page,unsigned int bitmap,unsigned char*mbuf,unsigned char*sbuf);
//extern __s32 PHY_SimpleRead_2CH (struct boot_physical_param *readop);
//extern __s32 PHY_SimpleWrite(struct boot_physical_param * writeop);

//extern __s32 PHY_SimpleWrite_1K(struct boot_physical_param * writeop);
//extern __s32 PHY_SimpleWrite_1KCurCH(struct boot_physical_param * writeop);
//extern __s32 PHY_SimpleWrite_Seq(struct boot_physical_param * writeop);
//extern __s32 PHY_SimpleRead_Seq(struct boot_physical_param * readop);

//
//extern __s32 NFC_LSBEnable(__u32 chip, __u32 read_retry_type);
//extern __s32 NFC_LSBDisable(__u32 chip, __u32 read_retry_type);
//extern __s32 NFC_LSBInit(__u32 read_retry_type);
//extern __s32 NFC_LSBExit(__u32 read_retry_type);
//extern __u32 NAND_GetChannelCnt(void);

//extern void ClearNandStruct( void );

//for param get&set
//extern __u32 NAND_GetFrequencePar(void);
//extern __s32 NAND_SetFrequencePar(__u32 FrequencePar);
extern __u32 nand_get_nand_version(void);
extern __s32 nand_get_param(boot_nand_para_t * nand_param);
extern __s32 NAND_GetFlashInfo(boot_flash_info_t *info);
//extern __s32 NAND_GetBlkCntOfDie(void);
//extern __s32 NAND_GetDieSkipFlag(void);


//extern __u32 NAND_GetPageSize(void);
//extern __u32 NAND_GetPageCntPerBlk(void);
//extern __u32 NAND_GetBlkCntPerChip(void);
//extern __u32 NAND_GetChipCnt(void);
//extern __u32 NAND_GetChipConnect(void);
//extern __u32 NAND_GetBadBlockFlagPos(void);
//extern __u32 NAND_GetValidBlkRatio(void);
//extern void  NAND_GetVersion(unsigned char *oob_buf);


extern  int show_spare(int flag);
extern  int nand_write_nboot_data(unsigned char *buf,unsigned int len);
extern  int nand_read_nboot_data(unsigned char *buf,unsigned int len);
extern  int nand_write_uboot_data(unsigned char *buf,unsigned int len);
extern  int nand_read_uboot_data(unsigned char *buf,unsigned int len);
extern  int nand_check_nboot(unsigned char *buf,unsigned int len);
extern  int nand_check_uboot(unsigned char *buf,unsigned int len);
extern  int nand_get_param(boot_nand_para_t * nand_param);
extern  int NAND_UpdatePhyArch(void);
extern  int nand_uboot_erase_all_chip(unsigned int force_flag);
extern int nand_physic_erase_block(unsigned int chip,unsigned int block);

struct _nand_info* NandHwInit(void);
__s32 NandHwExit(void);

//extern void div_test(void);

//for NFTL
//extern int nftl_initialize(struct _nftl_blk *nftl_blk,int no);
extern int nftl_build_one(struct _nand_info*, int no);
extern int nand_info_init(struct _nand_info* nand_info,unsigned char chip,uint16 start_block,unsigned char* mbr_data);
extern int nftl_build_all(struct _nand_info*nand_info);
extern uint32 get_nftl_num(void);
extern unsigned int get_nftl_cap(void);
extern unsigned int get_first_nftl_cap(void);
extern unsigned int get_phy_partition_num(struct _nand_info*nand_info);
extern unsigned int nftl_read(unsigned int start_sector,unsigned int len,unsigned char *buf);
extern unsigned int nftl_write(unsigned int start_sector,unsigned int len,unsigned char *buf);
extern unsigned int nftl_flush_write_cache(void);
extern void set_capacity_level(struct _nand_info*nand_info,unsigned short capacity_level);


//for partition

#define MAX_PART_COUNT_PER_FTL		24
#define MAX_PARTITION        		4

#define ND_MAX_PARTITION_COUNT      (MAX_PART_COUNT_PER_FTL*MAX_PARTITION)

/* part info */
typedef struct _NAND_PARTITION{
	unsigned  char      classname[16];
	unsigned  int       addr;
	unsigned  int       len;
	unsigned  int       user_type;
	unsigned  int       keydata;
	unsigned  int       ro;
}NAND_PARTITION;    //36bytes

/* mbr info */
typedef struct _PARTITION_MBR{
	unsigned  int		CRC;
	unsigned  int       PartCount;
	NAND_PARTITION      array[ND_MAX_PARTITION_COUNT];	//
}PARTITION_MBR;



#endif  //ifndef __BSP_NAND_H__

