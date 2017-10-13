/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : basic_nf_func.h
*
* Author : Gary.Wang
*
* Version : 1.1.0
*
* Date : 2009.09.13
*
* Description :
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang      2009.09.13       1.1.0        build the file
*
************************************************************************************************************************
*/

#ifndef  __basic_nf_func_h
#define  __basic_nf_func_h

#define BOOT0_START_BLK_NUM             0
#define BLKS_FOR_BOOT0                  2
#define BOOT1_START_BLK_NUM             BLKS_FOR_BOOT0

#define BLKS_FOR_BOOT1_IN_16K_BLK_NF    32
#define BLKS_FOR_BOOT1_IN_32K_BLK_NF    8
#define BLKS_FOR_BOOT1_IN_128K_BLK_NF   5
#define BLKS_FOR_BOOT1_IN_256K_BLK_NF   5
#define BLKS_FOR_BOOT1_IN_512K_BLK_NF   5
#define BLKS_FOR_BOOT1_IN_1M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_2M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_4M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_8M_BLK_NF     5

extern __u32 NF_BLOCK_SIZE;
extern __u32 NF_BLK_SZ_WIDTH;
extern __u32 NF_PAGE_SIZE;
extern __u32 NF_PG_SZ_WIDTH;
extern __u32 BOOT1_LAST_BLK_NUM;
extern __u32 page_with_bad_block;


#define NF_SECTOR_SIZE                  512U
#define NF_SCT_SZ_WIDTH                 9U
#define OOB_BUF_SIZE_PER_SECTOR         4


#define 	NF_OK         				0
#define 	NF_GOOD_BLOCK 				0
#define 	NF_OVERTIME_ERR  			1
#define 	NF_ECC_ERR       			2
#define 	NF_BAD_BLOCK     			3
#define 	NF_ERASE_ERR     			4
#define 	NF_PROG_ERR      			5
#define 	NF_NEW_BAD_BLOCK 			6
#define 	NF_LACK_BLKS     			7
#define 	NF_ERROR     				-1
#define 	NF_ERR_COUNT                8



#define MAX_PAGE_SIZE         SZ_8K
#define BAD_BLK_FLAG          0

typedef enum
{
	ADV_NF_OK               =0,
	ADV_NF_FIND_OK          =0,
	ADV_NF_NO_NEW_BAD_BLOCK =0,
	ADV_NF_ERROR              ,
	ADV_NF_NO_FIND_ERR        ,
	ADV_NF_OVERTIME_ERR       ,
	ADV_NF_LACK_BLKS          ,
	ADV_NF_NEW_BAD_BLOCK      ,
}adv_nf_errer_e;


extern __s32 load_and_check_in_one_blk( __u32 blk_num, void *buf, __u32 size, __u32 blk_size);

extern __s32 load_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
						        __u32 size, __u32 blk_size, __u32 *blks );

extern __s32 write_in_one_blk( __u32 blk_num, void *buf, __u32 size, __u32 blk_size );

extern __s32 write_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
					             __u32 size, __u32 blk_size, __u32 * blks );

extern __s32  NF_open ( void );
extern __s32  NF_close( void );
extern __s32  NF_read ( __u32 sector_num, void *buffer, __u32 N );
extern __s32  NF_write( __u32 sector_num, void *buffer, __u32 N );
extern __s32  NF_erase( __u32 blk_num );
extern __s32  NF_read_status ( __u32 blk_num );
extern __s32  NF_mark_bad_block( __u32 blk_num );
extern __s32  NF_verify_block( __u32 blk_num );

struct boot_physical_param;
//struct boot_flash_info_t;

extern __u32 NAND_Getlsbpage_type(void);
extern __u32 NAND_GetLsbblksize(void);
extern __s32 NFB_PhyInit(void);
extern __s32 NFB_PhyExit(void);
extern __s32 NFB_PhyRead (struct boot_physical_param *readop);
//extern __s32 NFB_GetFlashInfo(boot_flash_info_t *param);
extern __u32 Nand_Is_lsb_page(__u32 page);
extern __u8  *get_page_buf( void );
extern __s32 check_magic( __u32 *mem_base, const char *magic );
extern __u32 load_uboot_in_one_block_judge(__u32 length);

extern int verify_addsum( void *mem_base, __u32 size );
extern __u32 g_mod( __u32 dividend, __u32 divisor, __u32 *quot_p );
extern __s32 check_sum( __u32 *mem_base, __u32 size );

#endif     //  ifndef __basic_nf_func_h


/* end of basic_nf_func.h  */
