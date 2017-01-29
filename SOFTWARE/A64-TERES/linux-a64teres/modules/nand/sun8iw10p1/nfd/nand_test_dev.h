/*
************************************************************************************************************************
*                                                      eNand
*                                     Nand flash driver logic control module define
*
*                             Copyright(C), 2008-2009, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : bsp_nand.h
*
* Author : Kevin.z
*
* Version : v0.1
*
* Date : 2008.03.25
*
* Description :
*
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Kevin.z         2008.03.25      0.1          build the file
*
************************************************************************************************************************
*/
#ifndef __BSP_NAND_TEST_H__
#define __BSP_NAND_TEST_H__

extern int nand_driver_test_init(void);
extern int nand_driver_test_exit(void);
extern unsigned int  get_nftl_num(void);
extern unsigned int get_nftl_cap(void);
extern unsigned int get_first_nftl_cap(void);
extern unsigned int nftl_test_read(unsigned int start_sector,unsigned int len,unsigned char *buf);
extern unsigned int nftl_test_write(unsigned int start_sector,unsigned int len,unsigned char *buf);
extern unsigned int nftl_test_flush_write_cache(void);


#endif  //ifndef __BSP_NAND_TEST_H__
