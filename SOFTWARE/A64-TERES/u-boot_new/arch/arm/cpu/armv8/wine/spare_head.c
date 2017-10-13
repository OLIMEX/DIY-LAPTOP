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

#include <common.h>
#include <private_uboot.h>

struct spare_boot_head_t  uboot_spare_head = {
	                                  {
	      /* jump_instruction */          ( 0xEA000000 | ( ( ( sizeof( struct spare_boot_head_t ) + sizeof( int ) - 1 ) / sizeof( int ) - 2 ) & 0x00FFFFFF ) ),
							   		      UBOOT_MAGIC,
							   		      STAMP_VALUE,
							   		      ALIGN_SIZE,
							   		      0,
							   		      0,
							   		      UBOOT_VERSION,
							   		      UBOOT_PLATFORM,
							   		      {CONFIG_SYS_TEXT_BASE}
							 	      },
							 	      {
										  { 0 },		//dram para
							 	      	  1008,			//run core clock
							 	      	  1200,			//run core vol
							 	      	  0,			//uart port
							 	      	  {             //uart gpio
							 	      	    {0}, {0}
										  },
							 	      	  0,			//twi port
							 	      	  {             //twi gpio
							 	      	  	{0}, {0}
		                                  },
							 	      	  0,			//work mode
							 	      	  0,			//storage mode
							 	      	  { {0} },		//nand gpio
							 	      	  { 0 },		//nand spare data
							 	      	  { {0} },		//sdcard gpio
							 	      	  { 0 }, 		//sdcard spare data
							 	      	  0,
							 	      	  { 0 }			//reserved data
							 	      }
							 	  };


/*******************************************************************************
*
*                  关于Boot_file_head中的jump_instruction字段
*
*  jump_instruction字段存放的是一条跳转指令：( B  BACK_OF_Boot_file_head )，此跳
*转指令被执行后，程序将跳转到Boot_file_head后面第一条指令。
*
*  ARM指令中的B指令编码如下：
*          +--------+---------+------------------------------+
*          | 31--28 | 27--24  |            23--0             |
*          +--------+---------+------------------------------+
*          |  cond  | 1 0 1 0 |        signed_immed_24       |
*          +--------+---------+------------------------------+
*  《ARM Architecture Reference Manual》对于此指令有如下解释：
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
*  由此可知，此指令编码的最高8位为：0b11101010，低24位根据Boot_file_head的大小动
*态生成，所以指令的组装过程如下：
*  ( sizeof( boot_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int )    求出文件头
*                                              占用的“字”的个数
*  - 2                                         减去PC预取的指令条数
*  & 0x00FFFFFF                                求出signed-immed-24
*  | 0xEA000000                                组装成B指令
*
*******************************************************************************/
