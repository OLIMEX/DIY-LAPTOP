/* sound\soc\sunxi\sunxi_dma.h
 * (C) Copyright 2014-2017
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * chenpailin <chenpailin@Reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#ifndef SUNXI_DMA_H_
#define SUNXI_DMA_H_

struct sunxi_dma_params {
	char *name;
	dma_addr_t dma_addr;
	u8 src_maxburst;
	u8 dst_maxburst;
	u8 dma_drq_type_num;
};
extern int asoc_dma_platform_register(struct device *dev,unsigned int flags);
extern void asoc_dma_platform_unregister(struct device *dev);
#endif
