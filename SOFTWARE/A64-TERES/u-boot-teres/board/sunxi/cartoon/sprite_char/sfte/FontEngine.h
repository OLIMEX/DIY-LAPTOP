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
#ifndef  __FontEngine_h
#define  __FontEngine_h

#include <asm/io.h>
#include "sfte.h"
typedef struct _FE_FONT_t
{
	SFTE_Face  face;
	int             bbox_ymin;     // unit : pixel
	unsigned  int   line_distance; //
	unsigned  int   base_width;
	unsigned  char  *base_addr;
	unsigned  int   base_psize;

}FE_FONT_t, *FE_FONT;


extern int     open_font( const char *font_file, int pixel_size,  unsigned int width, unsigned char *addr);
extern int     close_font( void );
extern int     check_change_line(unsigned int x, unsigned char ch);
extern int     draw_bmp_ulc( unsigned int ori_x, unsigned int ori_y , unsigned int color);



#endif     //  ifndef __FontEngine_h

/* end of FontEngine.h  */
