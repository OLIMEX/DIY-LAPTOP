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

#ifndef  __SUNXI_SPRITE_VERIFY_H__
#define  __SUNXI_SPRITE_VERIFY_H__

#include <common.h>

extern uint add_sum(void *buffer, uint length);

extern uint sunxi_sprite_part_rawdata_verify(uint base_start, long long base_bytes);

extern uint sunxi_sprite_part_sparsedata_verify(void);

extern uint sunxi_sprite_generate_checksum(void *buffer, uint length, uint src_sum);

extern int sunxi_sprite_verify_checksum(void *buffer, uint length, uint src_sum);

extern int sunxi_sprite_verify_dlmap(void *buffer);

extern int sunxi_sprite_verify_mbr(void *buffer);

#endif

