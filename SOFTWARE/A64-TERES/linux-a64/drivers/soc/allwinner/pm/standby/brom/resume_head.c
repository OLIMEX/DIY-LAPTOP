/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Resumeloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : Resume_head.c
*
* Author : Gary.Wang
*
* Version : 1.1.0
*
* Date : 2007.11.06
*
* Description : This file defines the file head part of Resume, which contains some important
*             infomations such as magic, platform infomation and so on, and MUST be allocted in the
*             head of Resume.
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang       2007.11.06      1.1.0        build the file
*
************************************************************************************************************************
*/
#include "./resumes.h"

#pragma  arm section  rodata="head"


const resume_file_head_t  resume_head =
{
		/* jump_instruction */
		( 0xEA000000 | ( ( ( sizeof( resume_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int ) - 2 ) & 0x00FFFFFF ) ),
		//0xe3a0f000,
		RESUMEX_MAGIC,
		STAMP_VALUE,
		RESUMEX_ALIGN_SIZE,
		sizeof( resume_file_head_t ),
		RESUME_PUB_HEAD_VERSION,
		RESUMEX_FILE_HEAD_VERSION,
		RESUMEX_VERSION,
		EGON_VERSION,
};

/*******************************************************************************
*
*                 Resume_file_head е jump_instruction
*
*  jump_instruction ( B  BACK_OF_Resume_file_head )
*  Resume_file_head
*
*  ARM е B
*          +--------+---------+------------------------------+
*          | 31--28 | 27--24  |            23--0             |
*          +--------+---------+------------------------------+
*          |  cond  | 1 0 1 0 |        signed_immed_24       |
*          +--------+---------+------------------------------+
*  ARM Architecture Reference Manual
*  Syntax :
*  B{<cond>}  <target_address>
*    <cond>    Is the condition under which the instruction is executed. If the
*              <cond> is ommitted, the AL(always,its code is 0b1110 )is used.
*    <target_address>
*              Specified the address to branch to. The branch target address is
*              calculated by:
*              1.  Sign-extending the 24-bit signed(wro's complement)immediate
*                  to 32 bits.
*              2.  Shifting the result left two bits.
*              3.  Adding to the contents of the PC, which contains the address
*                  of the branch instruction plus 8.
*
*    0b11101010 24 Resume_file_head
*  ( sizeof( brom_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int )
*  - 2
*  & 0x00FFFFFF                                signed-immed-24
*  | 0xEA000000                                B
*
*******************************************************************************/
