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
#ifndef  __SUNXI_DE_H__
#define  __SUNXI_DE_H__

extern int board_display_layer_request(void);

extern int board_display_layer_release(void);

extern int board_display_layer_open(void);

extern int board_display_layer_close(void);

extern int board_display_layer_para_set(void);

extern int board_display_show(int display_source);

extern int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer);

extern int board_display_framebuffer_change(void *buffer);

extern int board_display_device_open(void);

extern int borad_display_get_screen_width(void);

extern int borad_display_get_screen_height(void);

#endif   //__SUNXI_DE_H__
