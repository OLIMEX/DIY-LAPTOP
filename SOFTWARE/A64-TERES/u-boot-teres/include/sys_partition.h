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

#ifndef __SYS_PARTITION_H
#define __SYS_PARTITION_H

extern int sunxi_partition_get_total_num(void);

extern int sunxi_partition_get_name(int index, char *buf);

extern uint sunxi_partition_get_offset(int part_index);

extern uint sunxi_partition_get_size(int part_index);

extern uint sunxi_partition_get_offset_byname(const char *part_name);

extern uint sunxi_partition_get_size_byname(const char *part_name);

extern int sunxi_partition_get_info_byname(const char *part_name, uint *part_offset, uint *part_size);

extern void *sunxi_partition_fetch_mbr(void);

extern int sunxi_partition_refresh(void *buf, uint bytes);

extern int sunxi_partition_init(void);

extern int sunxi_partition_get_partno_byname(const char *part_name);


#endif //__SYS_PARTITION_H
