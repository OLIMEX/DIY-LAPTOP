/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#ifndef  __COMMON_H__
#define  __COMMON_H__

#define  MAX_PATH             (260)

#include "boot0_head.h"
#include "toc_head.h"
#include "uboot_head.h"
#include "script.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

void GetFullPath(char *dName, const char *sName);
__s32 check_magic( void *mem_base, const char *magic );
__s32 check_sum( void *mem_base, __u32 size );
__s32 gen_check_sum( void *boot_buf );
__s32 gen_check_sum_toc0( void *boot_buf );
uint gen_general_checksum(void *buff, uint length);
uint sunxi_sprite_generate_checksum(void *buffer, uint length, uint src_sum);
int getfile_size(FILE *pFile);

#endif
