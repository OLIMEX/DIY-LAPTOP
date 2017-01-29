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

#ifndef	_RSB_H
#define _RSB_H

extern int sunxi_rsb_init(unsigned int slave_id);
extern int sunxi_rsb_exit(unsigned int slave_id);
extern int sunxi_rsb_config(unsigned int slave_id, unsigned int rsb_addr);

extern int sunxi_rsb_read(unsigned int slave_id,unsigned int daddr, unsigned char *data, unsigned int len);
extern int sunxi_rsb_write(unsigned int slave_id,unsigned int daddr, unsigned char *data, unsigned int len);



#endif /* _RSB_H */
