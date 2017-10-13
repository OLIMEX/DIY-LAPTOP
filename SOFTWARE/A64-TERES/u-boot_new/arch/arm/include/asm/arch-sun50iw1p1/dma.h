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

#ifndef	_DMA_H_
#define	_DMA_H_

#include <asm/arch/intc.h>

//================================
//======    DMA 配置     =========
//================================

/* DMA 基础配置  */
#define DMAC_CFG_CONTINUOUS_ENABLE              (0x01)
#define DMAC_CFG_CONTINUOUS_DISABLE             (0x00)

/* DMA 传输目的端 配置 */
/* DMA 目的端 传输宽度 */
#define	DMAC_CFG_DEST_DATA_WIDTH_8BIT			(0x00)
#define	DMAC_CFG_DEST_DATA_WIDTH_16BIT			(0x01)
#define	DMAC_CFG_DEST_DATA_WIDTH_32BIT			(0x02)

/* DMA 目的端 突发传输模式 */
#define	DMAC_CFG_DEST_1_BURST       			(0x00)
#define	DMAC_CFG_DEST_4_BURST		    		(0x01)
#define	DMAC_CFG_DEST_8_BURST					(0x02)

/* DMA 目的端 地址变化模式 */
#define	DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE		(0x00)
#define	DMAC_CFG_DEST_ADDR_TYPE_IO_MODE 		(0x01)


/* DMA 传输源端 配置 */
/* DMA 源端 传输宽度 */
#define	DMAC_CFG_SRC_DATA_WIDTH_8BIT			(0x00)
#define	DMAC_CFG_SRC_DATA_WIDTH_16BIT			(0x01)
#define	DMAC_CFG_SRC_DATA_WIDTH_32BIT			(0x02)

/* DMA 源端 突发传输模式 */
#define	DMAC_CFG_SRC_1_BURST       				(0x00)
#define	DMAC_CFG_SRC_4_BURST		    		(0x01)
#define	DMAC_CFG_SRC_8_BURST		    		(0x02)

/* DMA 源端 地址变化模式 */
#define	DMAC_CFG_SRC_ADDR_TYPE_LINEAR_MODE		(0x00)
#define	DMAC_CFG_SRC_ADDR_TYPE_IO_MODE 			(0x01)


/* DMA 传输目的端 配置 */
#define	DMAC_CFG_DEST_TYPE_SRAM					(0x00)
#define	DMAC_CFG_DEST_TYPE_DRAM		    		(0x01)

#define DMAC_CFG_DEST_TYPE_NAND					(5)

#define	DMAC_CFG_DEST_TYPE_CODEC	    		(15)

#define	DMAC_CFG_DEST_TYPE_OTG_EP1	    		(17)
#define	DMAC_CFG_DEST_TYPE_OTG_EP2	    		(18)
#define	DMAC_CFG_DEST_TYPE_OTG_EP3	    		(19)
#define	DMAC_CFG_DEST_TYPE_OTG_EP4	    		(20)
#define	DMAC_CFG_DEST_TYPE_OTG_EP5	    		(21)
/* DMA 传输源端 配置 */
#define	DMAC_CFG_SRC_TYPE_SRAM					(0x00)
#define	DMAC_CFG_SRC_TYPE_DRAM		    	   	(0x01)

#define DMAC_CFG_SRC_TYPE_NAND					(5)

#define	DMAC_CFG_SRC_TYPE_CODEC	    			(15)

#define	DMAC_CFG_SRC_TYPE_OTG_EP1	    		(17)
#define	DMAC_CFG_SRC_TYPE_OTG_EP2	    		(18)
#define	DMAC_CFG_SRC_TYPE_OTG_EP3	    		(19)
#define	DMAC_CFG_SRC_TYPE_OTG_EP4	    		(20)
#define	DMAC_CFG_SRC_TYPE_OTG_EP5	    		(21)


typedef struct
{
	unsigned int config;
	unsigned int source_addr;
	unsigned int dest_addr;
	unsigned int byte_count;
	unsigned int commit_para;
	unsigned int link;
	unsigned int reserved[2];
}
sunxi_dma_start_t;


typedef struct
{
    unsigned int      src_drq_type     : 5;            //源地址存储类型，如DRAM, SPI,NAND等，参见  __ndma_drq_type_t
    unsigned int      src_addr_mode    : 2;            //原地址类型 0:递增模式  1:保持不变
    unsigned int      src_burst_length : 2;            //发起一次burst宽度 0:1   1:4   2:8
    unsigned int      src_data_width   : 2;            //数据传输宽度，0:一次传输8bit，1:一次传输16bit，2:一次传输32bit，3:保留
    unsigned int      reserved0        : 5;
    unsigned int      dst_drq_type     : 5;            //目的地址存储类型，如DRAM, SPI,NAND等
    unsigned int      dst_addr_mode    : 2;            //目的地址类型，如递增，或者不变  0:递增模式  1:保持不变
    unsigned int      dst_burst_length : 2;            //发起一次burst宽度 填0对应于1，填1对应于4,
    unsigned int      dst_data_width   : 2;            //数据传输宽度，0:一次传输8bit，1:一次传输16bit，2:一次传输32bit，3:保留
    unsigned int      reserved1        : 5;
}
sunxi_dma_channal_config;

//for user request
typedef struct
{
	sunxi_dma_channal_config  cfg;
	unsigned int	loop_mode;
	unsigned int	data_block_size;
	unsigned int	wait_cyc;
}
sunxi_dma_setting_t;

extern    void          sunxi_dma_init(void);
extern    void          sunxi_dma_exit(void);

extern    ulong 	sunxi_dma_request(unsigned int dmatype);
extern    int 		sunxi_dma_release(ulong hdma);
extern    int 		sunxi_dma_setting(ulong hdma, sunxi_dma_setting_t *cfg);
extern    int 		sunxi_dma_start(ulong hdma, unsigned int saddr, unsigned int daddr, unsigned int bytes);
extern    int 		sunxi_dma_stop(ulong hdma);
extern    int 		sunxi_dma_querystatus(ulong hdma);

extern    int 		sunxi_dma_install_int(ulong hdma, interrupt_handler_t dma_int_func, void *p);
extern    int 		sunxi_dma_disable_int(ulong hdma);

extern    int 		sunxi_dma_enable_int(ulong hdma);
extern    int 		sunxi_dma_free_int(ulong hdma);

#endif	//_DMA_H_

/* end of _DMA_H_ */

