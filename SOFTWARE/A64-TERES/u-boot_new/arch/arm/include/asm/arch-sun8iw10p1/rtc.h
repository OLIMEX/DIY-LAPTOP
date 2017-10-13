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

#ifndef _SUNXI_RTC_H_
#define _SUNXI_RTC_H_


/* Rtc */
struct sunxi_rtc_regs {
	volatile u32 losc_ctrl;
	volatile u32 losc_auto_swt_status;
	volatile u32 clock_prescalar;
	volatile u32 res0[1];
	volatile u32 yymmdd;
	volatile u32 hhmmss;
	volatile u32 res1[2];
	volatile u32 alarm0_counter;
	volatile u32 alarm0_current_value;
	volatile u32 alarm0_enable;
	volatile u32 alarm0_irq_enable;
	volatile u32 alarm0_irq_status;
	volatile u32 res2[3];
	volatile u32 alarm1_wk_hms;
	volatile u32 alarm1_enable;
	volatile u32 alarm1_irq_enable;
	volatile u32 alarm1_irq_status;
	volatile u32 alarm_config;
};



#endif

