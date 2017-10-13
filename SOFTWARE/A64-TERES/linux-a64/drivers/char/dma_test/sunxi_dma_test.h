/*
 * drivers/char/dma_test/sunxi_dma_test.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sunxi dma test driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SUNXI_DMA_TEST_H
#define __SUNXI_DMA_TEST_H

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/gfp.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/dma.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/dma-mapping.h>
#include <linux/wait.h>
#include <linux/random.h>

#include <linux/dmaengine.h>
#include <linux/dma/sunxi-dma.h>

enum dma_test_case_e {
	DTC_MEMCPY_SINGLE_CHAN,
	DTC_MEMCPY_MULTI_CHAN,
	DTC_MAX
};

extern wait_queue_head_t g_dtc_queue[];
extern atomic_t g_adma_done;

#define BUF_MAX_CNT 	8
#define DMA_MAX_CHAN    6

typedef struct {
	unsigned int src_va;
	unsigned int src_pa;
	unsigned int dst_va;
	unsigned int dst_pa;
	unsigned int size;
}buf_item;

typedef struct {
	unsigned int cnt;
	buf_item item[BUF_MAX_CNT];
}buf_group;

typedef struct {
	struct dma_chan *chan;      /* dma channel handle */
	wait_queue_head_t dma_wq;   /* wait dma transfer done */
	atomic_t	dma_done;   /* dma done flag, used with dma_wq */
}chan_info;

#endif /* __SUNXI_DMA_TEST_H */

