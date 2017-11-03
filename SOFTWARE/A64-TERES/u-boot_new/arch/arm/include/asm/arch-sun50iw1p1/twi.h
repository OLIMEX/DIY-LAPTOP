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

#ifndef _TWIC_REGS_H_
#define _TWIC_REGS_H_

#include <linux/types.h>
/*
*********************************************************************************************************
*   Interrupt controller define
*********************************************************************************************************
*/
#define TWI_CONTROL_OFFSET             0x400
#define SUNXI_I2C_CONTROLLER             3

struct sunxi_twi_reg
{
    volatile unsigned int addr;        /* slave address     */
    volatile unsigned int xaddr;       /* extend address    */
	volatile unsigned int data;        /* data              */
    volatile unsigned int ctl;         /* control           */
    volatile unsigned int status;      /* status            */
    volatile unsigned int clk;         /* clock             */
    volatile unsigned int srst;        /* soft reset        */
    volatile unsigned int eft;         /* enhanced future   */
    volatile unsigned int lcr;         /* line control      */
    volatile unsigned int dvfs;        /* dvfs control      */
};


#endif  /* _TWIC_REGS_H_ */

