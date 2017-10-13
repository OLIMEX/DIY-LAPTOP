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

#include <common.h>
#include <malloc.h>
#include <asm/arch/dma.h>
#include <asm/arch/intc.h>
#include <asm/arch/ccmu.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

#define SUNXI_DMA_MAX     16


#define SUNXI_DMA_CHANNAL_BASE    (SUNXI_DMA_BASE + 0x100)
#define SUNXI_DMA_CHANANL_SIZE    (0x40)

#ifndef SUNXI_DMA_LINK_NULL
#define SUNXI_DMA_LINK_NULL       (0xfffff800)
#endif

struct dma_irq_handler
{
	void                *m_data;
	void (*m_func)( void * data);
};


typedef struct
{
	unsigned int irq_en0;
	unsigned int irq_en1;
	unsigned int reserved0[2];
	unsigned int irq_pending0;
	unsigned int irq_pending1;
	unsigned int reserved1[2];
	unsigned int reserved2[4];
	unsigned int status;
}
sunxi_dma_int_set;

typedef struct sunxi_dma_channal_set_t
{
	volatile unsigned int enable;
	volatile unsigned int pause;
	volatile unsigned int start_addr;		//起始地址
	volatile unsigned int config;
	volatile unsigned int cur_src_addr;		//当前传输地址
	volatile unsigned int cur_dst_addr;
	volatile unsigned int left_bytes;		//剩余未传字节数
	volatile unsigned int parameters;		//参数
}
sunxi_dma_channal_set;



typedef struct sunxi_dma_source_t
{
	unsigned int      		used;
	unsigned int            channal_count;
	sunxi_dma_channal_set	*channal;
	unsigned int			reserved;
	sunxi_dma_start_t       *config;
	struct dma_irq_handler  dma_func;
}
sunxi_dma_source;

#define  DMA_PKG_HALF_INT   (1<<0)
#define  DMA_PKG_END_INT    (1<<1)
#define  DMA_QUEUE_END_INT  (1<<2)

static int    dma_int_count = 0;
static sunxi_dma_source   dma_channal_source[SUNXI_DMA_MAX];

extern void *malloc_noncache(uint num_bytes);


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
static void sunxi_dma_int_func(void *p)
{
	int i;
	uint pending;
	sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	for(i=0; i<8; i++)
	{
		if(dma_channal_source[i].dma_func.m_func)
		{
			pending = (DMA_PKG_END_INT << (i * 4));
			if(dma_int->irq_pending0 & pending)
			{
				dma_int->irq_pending0 = pending;

				dma_channal_source[i].dma_func.m_func(dma_channal_source[i].dma_func.m_data);
			}
		}
	}
	for(i=8;i<15;i++)
	{
		if(dma_channal_source[i].dma_func.m_func)
		{
			pending = (DMA_PKG_END_INT << (i * 4));
			if(dma_int->irq_pending1 & pending)
			{
				dma_int->irq_pending1 = pending;

				dma_channal_source[i].dma_func.m_func(dma_channal_source[i].dma_func.m_data);
			}
		}
	}
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
void sunxi_dma_init(void)
{
	int i;
	u32 reg_val;
	sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	dma_int->irq_en0 = 0;
	dma_int->irq_en1 = 0;

	dma_int->irq_pending0 = 0xffffffff;
	dma_int->irq_pending1 = 0xffffffff;

	memset((void *)dma_channal_source, 0, SUNXI_DMA_MAX * sizeof(struct sunxi_dma_source_t));

	for(i=0;i<SUNXI_DMA_MAX;i++)
	{
		dma_channal_source[i].used = 0;
		dma_channal_source[i].channal = (struct sunxi_dma_channal_set_t *)(ulong)(SUNXI_DMA_BASE + i * SUNXI_DMA_CHANANL_SIZE + 0x100);
		dma_channal_source[i].config  = (sunxi_dma_start_t *)malloc_noncache(sizeof(sunxi_dma_start_t));
	}

	dma_int_count = 0;
	irq_install_handler(AW_IRQ_DMA, sunxi_dma_int_func, 0);

#if 1
        //auto MCLK gating  disable
        reg_val = *(volatile unsigned int *)(SUNXI_DMA_BASE + 0x20);
        reg_val &= ~7;
        reg_val |= 4;
        *(volatile unsigned int *)(SUNXI_DMA_BASE + 0x20) = reg_val;

#endif
	return ;
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
void sunxi_dma_exit(void)
{
	int i;
	ulong hdma;
	unsigned int reg_val = 0;
	sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	//free dma channal if other module not free it
	for(i=0;i<SUNXI_DMA_MAX;i++)
	{
		if(dma_channal_source[i].used == 1)
		{
		    hdma = (ulong)&dma_channal_source[i];
		    sunxi_dma_disable_int(hdma);
		    sunxi_dma_free_int(hdma);
		    dma_channal_source[i].channal->enable = 0;
		    dma_channal_source[i].used   = 0;
		}
	}
	//irp disable
	dma_int->irq_en0 = 0;
	dma_int->irq_en1 = 0;

	dma_int->irq_pending0 = 0xffffffff;
	dma_int->irq_pending1 = 0xffffffff;


	irq_free_handler(AW_IRQ_DMA);
	//close dma clock when dma exit


	reg_val = readl(CCMU_BUS_CLK_GATING_REG0);
	reg_val &= ~(0x01 << 6);
	writel(reg_val , CCMU_BUS_CLK_GATING_REG0);

}
/*
****************************************************************************************************
*
*             DMAC_RequestDma
*
*  Description:
*       request dma
*
*  Parameters:
*		type	0: normal timer
*				1: special timer
*  Return value:
*		dma handler
*		if 0, fail
****************************************************************************************************
*/
ulong sunxi_dma_request(uint dmatype)
{
    int   i;

	for(i=0;i<SUNXI_DMA_MAX;i++)
    {
        if(dma_channal_source[i].used == 0)
        {
            dma_channal_source[i].used = 1;
            dma_channal_source[i].channal_count = i;

            return (ulong)&dma_channal_source[i];
        }
    }

    return 0;
}
/*
****************************************************************************************************
*
*             DMAC_ReleaseDma
*
*  Description:
*       release dma
*
*  Parameters:
*       hDma	dma handler
*
*  Return value:
*		EPDK_OK/FAIL
****************************************************************************************************
*/
int sunxi_dma_release(ulong hdma)
{
	struct sunxi_dma_source_t  *dma_channal = (struct sunxi_dma_source_t *)hdma;

	if(!dma_channal->used)
	{
		return -1;
	}

	sunxi_dma_disable_int(hdma);
	sunxi_dma_free_int(hdma);

	dma_channal->channal->enable = 0;
	dma_channal->used   = 0;

    return 0;
}
/*
****************************************************************************************************
*
*             sunxi_dma_setting
*
*  Description:
*       start interrupt
*
*  Parameters:
*
*
*
*
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int sunxi_dma_setting(ulong hdma, sunxi_dma_setting_t *cfg)
{
	uint   *config_addr;
	uint   commit_para;
	sunxi_dma_setting_t   		*dma_set     = cfg;
	struct sunxi_dma_source_t  	*dma_channal = (struct sunxi_dma_source_t *)hdma;

	if(!dma_channal->used)
	{
		return -1;
	}

	config_addr = (uint *)&(dma_set->cfg);
	if(dma_set->loop_mode)
	{
		dma_channal->config->link = (uint)(ulong )(dma_channal->config);
 	}
	else
	{
		dma_channal->config->link = SUNXI_DMA_LINK_NULL;
	}

	commit_para  = (dma_set->wait_cyc & 0xff);
	commit_para |= (dma_set->data_block_size & 0xff ) << 8;
	dma_channal->config->commit_para = commit_para;
	dma_channal->config->config = *config_addr;

    return 0;
}

/*
**********************************************************************************************************************
*
*             sunxi_dma_start
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
****************************************************************************************************
*/
int sunxi_dma_start(ulong hdma, uint saddr, uint daddr, uint bytes)
{
	struct sunxi_dma_source_t  	*dma_channal = (struct sunxi_dma_source_t *)hdma;

	if(!dma_channal->used)
	{
		return -1;
	}

 	dma_channal->config->source_addr = saddr;
 	dma_channal->config->dest_addr   = daddr;
 	dma_channal->config->byte_count  = bytes;

	dma_channal->channal->start_addr = (uint)(ulong)(dma_channal->config);

	//guarantee pre data access opration was fisnished here
	asm volatile("dmb sy");
	//flush_cache((uint)&(dma_channal->config), sizeof(sunxi_dma_start_t));

	dma_channal->channal->enable = 1;

    return 0;
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_stop
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int sunxi_dma_stop(ulong hdma)
{
	struct sunxi_dma_source_t  	*dma_channal = (struct sunxi_dma_source_t *)hdma;

	if(!dma_channal->used)
	{
		return -1;
	}

	dma_channal->channal->enable = 0;

    return 0;
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_querystatus
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int sunxi_dma_querystatus(ulong hdma)
{
	uint  channal_count;
	struct sunxi_dma_source_t  	*dma_channal = (struct sunxi_dma_source_t *)hdma;
	sunxi_dma_int_set    		*dma_int 	 = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	if(!dma_channal->used)
	{
		return -1;
	}
	channal_count = dma_channal->channal_count;

	return (dma_int->status >> channal_count) & 0x01;
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
int sunxi_dma_install_int(ulong hdma, interrupt_handler_t dma_int_func, void *p)
{
	sunxi_dma_source     *dma_channal = (sunxi_dma_source  *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}
	channal_count = dma_channal->channal_count;

	if(channal_count < 8)
	{
		dma_status->irq_pending0 = (7 << channal_count*4);
	}
	else
	{
		dma_status->irq_pending1 = (7 << (channal_count - 8)*4);
	}

	if(!dma_channal->dma_func.m_func)
	{
		dma_channal->dma_func.m_func = dma_int_func;
		dma_channal->dma_func.m_data = p;
	}
	else
	{
		printf("dma 0x%lx int is used already, you have to free it first\n", hdma);

		return -1;
	}

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
int sunxi_dma_enable_int(ulong hdma)
{
	sunxi_dma_source     *dma_channal = (sunxi_dma_source *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}

	channal_count = dma_channal->channal_count;
	if(channal_count < 8)
	{
	    if(dma_status->irq_en0 & (DMA_PKG_END_INT << channal_count*4))
	    {
	    	printf("dma 0x%lx int is avaible already\n", hdma);

			return 0;
		}
		dma_status->irq_en0 |= (DMA_PKG_END_INT << channal_count*4);
	}
	else
	{
		if(dma_status->irq_en1 & (DMA_PKG_END_INT << (channal_count - 8)*4))
	    {
	    	printf("dma 0x%lx int is avaible already\n", hdma);

			return 0;
		}
		dma_status->irq_en1 |= (DMA_PKG_END_INT << (channal_count - 8)*4);
	}

	if(!dma_int_count)
	{
		irq_enable(AW_IRQ_DMA);
	}
	dma_int_count ++;

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
int sunxi_dma_disable_int(ulong hdma)
{
	sunxi_dma_source     *dma_channal = (sunxi_dma_source *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}

	channal_count = dma_channal->channal_count;
	if(channal_count < 8)
	{
	    if(!(dma_status->irq_en0 & (DMA_PKG_END_INT << channal_count*4)))
	    {
	    	printf("dma 0x%lx int is not used yet\n", hdma);

			return 0;
		}
		dma_status->irq_en0 &= ~(DMA_PKG_END_INT << channal_count*4);
	}
	else
	{
		if(!(dma_status->irq_en1 & (DMA_PKG_END_INT << (channal_count - 8)*4)))
	    {
	    	printf("dma 0x%lx int is not used yet\n", hdma);

			return 0;
		}
		dma_status->irq_en1 &= ~(DMA_PKG_END_INT << (channal_count - 8)*4);
	}

	//disable golbal int
	if(dma_int_count > 0)
	{
		dma_int_count --;
	}
	if(!dma_int_count)
	{
		irq_disable(AW_IRQ_DMA);
	}

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
int sunxi_dma_free_int(ulong hdma)
{
	sunxi_dma_source     *dma_channal = (sunxi_dma_source *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}
	channal_count = dma_channal->channal_count;
	if(channal_count < 8)
	{
		dma_status->irq_pending0 = (7 << channal_count);
	}
	else
	{
		dma_status->irq_pending1 = (7 << (channal_count - 8));
	}

	if(dma_channal->dma_func.m_func)
	{
		dma_channal->dma_func.m_func = NULL;
		dma_channal->dma_func.m_data = NULL;
	}
	else
	{
		printf("dma 0x%lx int is free, you do not need to free it again\n", hdma);

		return -1;
	}

	return 0;
}

