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

#ifndef __IMAGE_DECODE_H____
#define __IMAGE_DECODE_H____	1

//------------------------------------------------------------------------------------------------------------
#define PLUGIN_TYPE				IMGDECODE_PLUGIN_TYPE
#define PLUGIN_NAME				"imgDecode"				//scott note
#define PLUGIN_VERSION			0x0100
#define PLUGIN_AUTHOR			"scottyu"
#define PLUGIN_COPYRIGHT		"scottyu"

//------------------------------------------------------------------------------------------------------------
//插件的通用接口
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
// image 解析接口
//------------------------------------------------------------------------------------------------------------
typedef void * 		HIMAGE;

typedef void * 		HIMAGEITEM;

extern   HIMAGE 		Img_Open		(char * ImageFile);

extern   long long      Img_GetSize	    (HIMAGE hImage);

extern   HIMAGEITEM 	Img_OpenItem	(HIMAGE hImage, char * MainType, char * subType);

extern   long long 		Img_GetItemSize	(HIMAGE hImage, HIMAGEITEM hItem);

extern   uint 			Img_GetItemStart(HIMAGE hImage, HIMAGEITEM hItem);

extern   uint 			Img_ReadItem	(HIMAGE hImage, HIMAGEITEM hItem, void *buffer, uint buffer_size);

extern   int 			Img_CloseItem	(HIMAGE hImage, HIMAGEITEM hItem);

extern   void 	 		Img_Close		(HIMAGE hImage);

//------------------------------------------------------------------------------------------------------------
// THE END !
//------------------------------------------------------------------------------------------------------------

#endif //__IMAGE_DECODE_H____

