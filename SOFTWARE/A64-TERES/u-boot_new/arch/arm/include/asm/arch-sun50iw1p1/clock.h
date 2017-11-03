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

#ifndef _SUNXI_CLOCK_H
#define _SUNXI_CLOCK_H


extern int sunxi_clock_get_corepll(void);
extern int sunxi_clock_set_corepll(int frequency);
extern int sunxi_clock_get_pll6(void);
extern int sunxi_clock_get_ahb(void);
extern int sunxi_clock_get_apb(void);
extern int sunxi_clock_get_axi(void);
extern int sunxi_clock_get_ahb2(void);
extern int sunxi_clock_get_mbus(void);

extern void set_pll( void );
extern void set_gpio_gate(void);
extern void reset_pll(void);

#endif /* _SUNXI_CLOCK_H */
