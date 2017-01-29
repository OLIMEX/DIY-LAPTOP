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
#ifndef  __SPRITE_DRAW_H__
#define  __SPRITE_DRAW_H__

extern  int sprite_cartoon_ui_draw_hollowrectangle(int x1, int y1, int x2, int y2);
extern  int sprite_cartoon_ui_draw_solidrectangle (int x1, int y1, int x2, int y2);
extern  int sprite_cartoon_ui_draw_line(int x1, int y1, int x2, int y2);
extern  int sprite_cartoon_ui_draw_lineto(int x, int y);
extern  int sprite_cartoon_ui_draw_point(int x, int y);
extern  int sprite_cartoon_ui_set_color(int color);
extern  int sprite_cartoon_ui_get_color(void);
extern  int sprite_cartoon_ui_moveto(int x, int y);


#endif   //__SPRITE_DRAW_H__

