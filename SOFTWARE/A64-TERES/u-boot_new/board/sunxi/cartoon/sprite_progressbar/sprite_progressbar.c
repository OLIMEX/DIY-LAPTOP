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
#include  "../sprite_cartoon_i.h"
#include  "../sprite_cartoon.h"
#include  "../sprite_cartoon_color.h"
#include  "sprite_progressbar_i.h"

progressbar_t   progress;
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
uint sprite_cartoon_progressbar_create(int x1, int y1, int x2, int y2)
{
	progressbar_t *progress = NULL;
	int tmp;

	progress = malloc(sizeof(progressbar_t));
	if(!progress)
	{
		printf("sprite cartoon ui: create progressbar failed\n");

		return 0;
	}
	if(x1 > x2)
	{
		tmp = x1;
		x1 	= x2;
		x2  = tmp;
	}
	if(y1 > y2)
	{
		tmp = y1;
		y1 	= y2;
		y2  = tmp;
	}
	progress->x1 = x1;
	progress->x2 = x2;
	progress->y1 = y1;
	progress->y2 = y2;
	progress->width  = x2 - x1;
	progress->height = y2 - y1;
	progress->st_x	 = progress->pr_x = x1 + 1;
	progress->st_y	 = progress->pr_y = y1 + 1;
	progress->frame_color 	 = SPRITE_CARTOON_GUI_WHITE;
	progress->progress_color = SPRITE_CARTOON_GUI_GREEN;
	progress->progress_ratio = 0;
	progress->thick          = 1;
	progress->frame_color 	 = SPRITE_CARTOON_GUI_GREEN;
	progress->progress_color = SPRITE_CARTOON_GUI_RED;

	return (uint)progress;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sprite_cartoon_progressbar_config(uint p, int frame_color, int progress_color, int thickness)
{
	progressbar_t *progress = (progressbar_t *)p;

	if(!p)
	{
		return -1;
	}
	progress->frame_color 	 = frame_color;
	progress->progress_color = progress_color;
	progress->progress_ratio = 0;
	progress->thick          = thickness;
	progress->st_x	 		 = progress->pr_x = progress->x1 + thickness;
	progress->st_y	 		 = progress->pr_y = progress->y1 + thickness;

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sprite_cartoon_progressbar_active(uint p)
{
	int base_color;
	int i;
	progressbar_t *progress = (progressbar_t *)p;

	if(!p)
	{
		return -1;
	}
	base_color = sprite_cartoon_ui_get_color();
	sprite_cartoon_ui_set_color(progress->frame_color);
	for(i=0;i<progress->thick;i++)
	{
		sprite_cartoon_ui_draw_hollowrectangle(progress->x1+i, progress->y1+i, progress->x2-i, progress->y2-i);
	}
	sprite_cartoon_ui_set_color(base_color);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sprite_cartoon_progressbar_destroy(uint p)
{
	progressbar_t *progress = (progressbar_t *)p;
	int base_color;

	if(!p)
	{
		return -1;
	}
	base_color = sprite_cartoon_ui_get_color();
	sprite_cartoon_ui_set_color(SPRITE_CARTOON_GUI_BLACK);
	sprite_cartoon_ui_draw_solidrectangle(progress->x1, progress->y1, progress->x2, progress->y2);

	sprite_cartoon_ui_set_color(base_color);

	free(progress);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sprite_cartoon_progressbar_upgrate(uint p, int rate)
{
	progressbar_t *progress = (progressbar_t *)p;
	int base_color, progresscolor;
	int pixel;
	int x1, y1;
	int x2, y2;

	if((rate < 0) || (rate > 100))
	{
		printf("sprite_cartoon ui progressbar: invalid progressbar rate\n");
		return -1;
	}
	if(!p)
	{
		printf("sprite_cartoon ui progressbar: invalid progressbar pointer\n");
		return -1;
	}
	pixel = (rate * (progress->width - progress->thick*2)/100);
	if(rate == progress->progress_ratio)
	{
		return 0;
	}
	else
	{
		x1 = progress->pr_x;
		x2 = progress->st_x + pixel;
		progresscolor = (rate > progress->progress_ratio)?(progress->progress_color):(SPRITE_CARTOON_GUI_BLACK);
		progress->pr_x  = x2;
		progress->progress_ratio = rate;

	}
	base_color = sprite_cartoon_ui_get_color();
	sprite_cartoon_ui_set_color(progresscolor);
	y1 = progress->y1 + progress->thick;
	y2 = progress->y2 - progress->thick;

	sprite_cartoon_ui_draw_solidrectangle(x1, y1, x2, y2);

	sprite_cartoon_ui_set_color(base_color);

	return 0;
}


