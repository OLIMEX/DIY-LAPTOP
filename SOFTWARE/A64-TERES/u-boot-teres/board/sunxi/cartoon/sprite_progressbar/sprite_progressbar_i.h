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
#ifndef  __SPRITE_PROGRESSBAR_I_H__
#define  __SPRITE_PROGRESSBAR_I_H__


typedef struct  _progressbar_t
{
	int  x1;					//左上角x坐标
	int  y1;					//左上角y坐标
	int  x2;					//右下角x坐标
	int  y2;					//右下角y坐标
	int  st_x;					//内部起始点的x坐标
	int  st_y;					//内部起始点的y坐标
	int  pr_x;					//当前进度所在的x坐标
	int  pr_y;					//无效
	int  thick;					//框的厚度，边缘厚度
	int  width;					//整体宽度
	int  height;				//整体高度
	int  frame_color;			//边框颜色
	int  progress_color;		//内部颜色
	int  progress_ratio;		//当前进度百分比
}
progressbar_t;




#endif   //__SPRITE_PROGRESSBAR_I_H__

