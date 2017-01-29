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
#ifndef  __SPRITE_CARTOON_I_H__
#define  __SPRITE_CARTOON_I_H__

#include "sprite_draw/sprite_draw.h"
#include "sprite_progressbar/sprite_progressbar.h"

typedef struct
{
	int 		 screen_width;
	int 		 screen_height;
	int 		 screen_size;
	unsigned int color;
	int     	 this_x;
	int     	 this_y;
	char 		 *screen_buf;
}
sprite_cartoon_source;


extern  sprite_cartoon_source  sprite_source;


#endif  /* __SPRITE_CARTOON_I_H__ */
