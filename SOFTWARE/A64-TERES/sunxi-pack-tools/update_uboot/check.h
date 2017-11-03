/*
******************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : check.h
*
* Author : Gary.Wang
*
* Version : 1.1.0
*
* Date : 2007.10.12
*
* Description £ºThis file provides a function to check Boot0 and Boot1.
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang       2007.10.12      1.1.0        build the file
*
************************************************************************************************************************
*/
#ifndef  __script_check_h
#define  __script_check_h

#include "types.h"

#define CHECK_IS_WRONG             1
#define CHECK_IS_CORRECT           0


extern __s32 check_file   ( __u32 *mem_base, __u32 size, const char *magic );
extern __s32 gen_check_sum( void *boot_buf );






#endif // end of #ifndef __check_h

/* end of check.h  */
