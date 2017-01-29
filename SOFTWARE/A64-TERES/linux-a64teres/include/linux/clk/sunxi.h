/*
 * Copyright 2012 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * 2013-4-17 15:43 add periph reset assert/deassert, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_CLK_SUNXI_H_
#define __LINUX_CLK_SUNXI_H_


int sunxi_periph_reset_deassert(struct clk *c);
int sunxi_periph_reset_assert(struct clk *c);

void __init sunxi_init_clocks(void);

#endif
