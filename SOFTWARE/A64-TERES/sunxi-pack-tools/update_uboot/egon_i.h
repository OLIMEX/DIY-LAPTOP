/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : egon_i.h
*
* Author      : Gary.Wang
*
* Version     : 1.1.0
*
* Date        : 2009.05.21
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang      2009.05.21       1.1.0        build the file
*
************************************************************************************************************************
*/
#ifndef  __egon_i_h
#define  __egon_i_h


#define EPDK_CHIP_BDD0                  0
#define EPDK_CHIP_AF0B                  1
#define EPDK_CHIP_AF0C                  2
#define EPDK_CHIP_AFA0                  3
#define EPDK_CHIP_AFAA                  4


#define BOOT0_IN_NF           // BOOT0_IN_NF, BOOT0_IN_SF, BOOT0_IN_IE
#define BOOT1_IN_NF           // BOOT1_IN_NF, BOOT1_IN_SF, BOOT1_IN_IE
#define KENEL_IN_NF           // KENEL_IN_RAM, KENEL_IN_NF


#if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_NAND_FLASH
	#define BOOT0_ALIGN_SIZE  NF_ALIGN_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SPI_NOR_FLASH
	#define BOOT0_ALIGN_SIZE  BOOT_SECTOR_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SD_CARD
    #define BOOT0_ALIGN_SIZE  BOOT_SECTOR_SIZE
#else
	#error The storage media has not been defined.
#endif


#if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_NAND_FLASH
	#define BOOT1_ALIGN_SIZE  NF_ALIGN_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SPI_NOR_FLASH
	#define BOOT1_ALIGN_SIZE  BOOT_SECTOR_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SD_CARD
	#define BOOT1_ALIGN_SIZE  BOOT_SECTOR_SIZE
#else
	#error The storage media has not been defined.
#endif


#define BOOT_PUB_HEAD_VERSION           "1100"    // X.X.XX
#define EGON_VERSION                    "1100"    // X.X.XX



#endif     //  ifndef __egon_i_h

/* end of egon_i.h */
