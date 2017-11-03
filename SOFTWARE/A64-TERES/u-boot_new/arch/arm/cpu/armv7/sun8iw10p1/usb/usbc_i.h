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

#ifndef  __USBC_I_H__
#define  __USBC_I_H__

#include  <common.h>
#include  <asm/arch/usb.h>
#include  <asm/arch/cpu.h>
#include  <asm/arch/clock.h>

#define  USBC_MAX_OPEN_NUM    8

/* 记录USB的公共信息 */
typedef struct __fifo_info{
    uint port0_fifo_addr;
	uint port0_fifo_size;

    uint port1_fifo_addr;
	uint port1_fifo_size;

	uint port2_fifo_addr;
	uint port2_fifo_size;
}__fifo_info_t;

/* 记录当前USB port所有的硬件信息 */
typedef struct __usbc_otg{
    uint port_num;
	uint base_addr;        /* usb base address 		*/

	uint used;             /* 是否正在被使用   		*/
    uint no;               /* 在管理数组中的位置 		*/
}__usbc_otg_t;

#endif   //__USBC_I_H__

