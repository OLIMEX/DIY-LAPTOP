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
#ifndef __SUNXI_SPRITE_SPARSE_H__
#define __SUNXI_SPRITE_SPARSE_H__


#define   ANDROID_FORMAT_UNKNOW    (0)
#define   ANDROID_FORMAT_BAD       (-1)
#define   ANDROID_FORMAT_DETECT    (1)


extern int  unsparse_probe(char *source, unsigned int length, unsigned int flash_start);
extern int  unsparse_direct_write(void *pbuf, unsigned int length);
extern unsigned int unsparse_checksum(void);


#endif /* __SUNXI_SPRITE_SPARSE_H__ */
