#ifndef __NAND_FOR_BOOT1__
#define __NAND_FOR_BOOT1__

#define SUCCESS	0
#define FAIL	-1
#define BADBLOCK -2


typedef struct 
{
	__u8 id[8];
	__u8 chip_cnt;
	__u8 chip_connect;
	__u8 rb_cnt;
	__u8 rb_connect;
	__u32 good_block_ratio;
}_nand_connect_info_t;

struct boot_physical_param{
	__u8   chip; //chip no
	__u16  block; // block no within chip
	__u16  page; // page no within block
	__u16  sectorbitmap; //done't care
	void   *mainbuf; //data buf
	void   *oobbuf; //oob buf
};

struct boot_flash_info{
	__u32 chip_cnt;
	__u32 blk_cnt_per_chip;
	__u32 blocksize; //unit by sector 
	__u32 pagesize; //unit by sector
	__u32 pagewithbadflag; /*bad block flag was written at the first byte of spare area of this page*/
};

/*
************************************************************************************************************************
*                       READ ONE SINGLE PAGE
*
*Description: read one page data from nand based on single plane;
*
*Arguments  : *readop - the structure with physical address in nand and data buffer 
*
*Return     :   = SUCESS  read ok;
*               = FAIL    read fail.
************************************************************************************************************************
*/
__s32 NFB0_PhyRead (struct boot_physical_param *readop);

/*
************************************************************************************************************************
*                       GET FLASH INFO
*
*Description: get some info about nand flash;
*
*Arguments  : *param     the stucture with info.
*
*Return     : the result of chip reset;
*               = SUCESS  get ok;
*               = FAIL    get fail.
************************************************************************************************************************
*/
__s32 NFB0_GetFlashInfo(struct boot_flash_info *param);


/*
************************************************************************************************************************
*                       INIT NAND FLASH
*
*Description: initial nand flash,request hardware resources;
*
*Arguments  : void.
*
*Return     :   = SUCESS  initial ok;
*               = FAIL    initial fail.
************************************************************************************************************************
*/
__s32 NFB0_PhyInit(void);
/*
************************************************************************************************************************
*                       RELEASE NAND FLASH
*
*Description: release  nand flash and free hardware resources;
*
*Arguments  : void.
*
*Return     :   = SUCESS  release ok;
*               = FAIL    release fail.
************************************************************************************************************************
*/
__s32 NFB0_PhyExit(void);


#endif
