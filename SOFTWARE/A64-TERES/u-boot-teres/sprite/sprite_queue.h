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
#ifndef  __SUNXI_SPRITE_QUEUE_H__
#define  __SUNXI_SPRITE_QUEUE_H__

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <asm/arch/queue.h>


#define SUNXI_BUFFER_QUEUE_COUNT   8
#define SUNXI_BUFFER_QUEUE_SIZE    (32 * 1024)



int sunxi_queue_init(void);

void sunxi_queue_destroy(void);

void sunxi_queue_reset(void);

int sunxi_queue_isempty(void);

int sunxi_queue_isfull(void);

int sunxi_inqueue_query(queue_data *qdata);

int sunxi_queue_in(void);

int sunxi_outqueue_query(queue_data *qdata, queue_data *next_qdata);

int sunxi_queue_out(void);


#endif
