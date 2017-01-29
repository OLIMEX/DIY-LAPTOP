/*
 * drivers/dma/sunxi-dma.c
 *
 * Copyright (C) 2013-2015 Allwinnertech Co., Ltd
 *
 * Author: Sugar <shuge@allwinnertech.com>
 *
 * Sunxi DMA controller driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dmapool.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/dma/sunxi-dma.h>
#include <linux/kthread.h>
#include <mach/platform.h>
#include <mach/sunxi-smc.h>

#include "dmaengine.h"
#include "virt-dma.h"

#define NR_MAX_CHAN	4			/* total of channels */

#define RDMA_IRQ_ID	SUNXI_IRQ_R_DMA		/* SUN9I: 67 */
#define RDMA_PHYS_BASE	SUNXI_R_DMA_PBASE	/* SUN9I: 0x08008000 */

#define RDMA_IRQ_EN	(0x00)			/* Interrupt enable register */
#define RDMA_IRQ_PEN	(0x04)			/* Inetrrupt status register */
#define RDMA_GATE	(0x08)			/* Auto gating register */
#define RDMA_CTRL(x)	(0x100 + ((x) << 5))	/* Control register */
#define RDMA_SRC(x)	(0x104 + ((x) << 5))
#define RDMA_DST(x)	(0x108 + ((x) << 5))
#define RDMA_CNT(x)	(0x10c + ((x) << 5))


#define SHIFT_IRQ_MASK(val, ch) ({	\
		(val) << ((ch) << 1 );	\
		})

#define IRQ_HALF	0x01		/* Half package transfer interrupt pending */
#define IRQ_PKG		0x02		/* One package complete interrupt pending */

/* The detail information of RDMA control register */
#define SRC_DRQ(x)	((x) << 0)
#define SRC_IO_MODE	(0x01 << 5)
#define SRC_LINEAR_MODE	(0x00 << 5)
#define SRC_BURST(x)	((x) << 7)
#define SRC_WIDTH(x)	((x) << 8)

#define REMAIN_MODE	(1 << 15)

#define DST_DRQ(x)	((x) << 16)
#define DST_IO_MODE	(0x01 << 21)
#define DST_LINEAR_MODE	(0x00 << 21)
#define DST_BURST(x)	((x) << 23)
#define DST_WIDTH(x)	((x) << 24)

#define CONTI_MODE	(1 << 29)
#define	BUSY_BIT	(1 << 30)
#define CHAN_START	(1 << 31)

#define WAIT_STATE	(7 << 26)

#define RDMA_MAX_BYTES	((18 << 1) -1)UL

/* lli: linked list ltem, the DMA block descriptor */
struct sunxi_rdma_lli {
	u32		cfg;		/* The configuration fo R_DMA */
	dma_addr_t	src;		/* Source address */
	dma_addr_t	dst;		/* Destination address */
	u32		len;		/* Length of buffers */
	struct sunxi_rdma_lli *v_lln;	/* Next lli virtual address (only for cpu) */
}__attribute__((packed));

struct sunxi_rdmadev {
	struct dma_device	dma_dev;
	void __iomem		*base;
	struct clk		*apbs_clk;	/* AHB clock gate for DMA */

	spinlock_t		lock;
	struct tasklet_struct	task;
	struct list_head	pending;	/* the pending channels list */
};

struct sunxi_desc {
	struct virt_dma_desc	vd;
	struct sunxi_rdma_lli	*head;		/* Mark the head of list */
	struct sunxi_rdma_lli	*cur_lli;	/* virtual start for lli */
};

struct sunxi_chan {
	struct virt_dma_chan	vc;

	struct list_head	node;		/* queue it to pending list */
	struct dma_slave_config	cfg;
	bool	cyclic;

	struct sunxi_desc	*desc;
	u32	irq_type;
};

static inline struct sunxi_rdmadev *to_sunxi_dmadev(struct dma_device *d)
{
	return container_of(d, struct sunxi_rdmadev, dma_dev);
}

static inline struct sunxi_chan *to_sunxi_chan(struct dma_chan *chan)
{
	return container_of(chan, struct sunxi_chan, vc.chan);
}

static inline struct sunxi_desc *to_sunxi_desc(struct dma_async_tx_descriptor *tx)
{
	return container_of(tx, struct sunxi_desc, vd.tx);
}

static inline struct device *chan2dev(struct dma_chan *chan)
{
	return &chan->dev->device;
}
static inline struct device *chan2parent(struct dma_chan *chan)
{
	return chan->dev->device.parent;
}

/*
 * Fix sconfig's burst size according to sunxi_dmac. We need to convert them as:
 * 1 -> 0, 4 -> 1, 8 -> 2, 16->3
 *
 * NOTE: burst size 2 is not supported by controller.
 *
 * This can be done by finding least significant bit set: n & (n - 1)
 */
static inline void convert_burst(u32 *maxburst)
{
#if 0
	if (*maxburst > 1 && *maxburst < 8)
		*maxburst = fls(*maxburst) - 2;
	else
#endif
		*maxburst = 0;
}

/*
 * Fix sconfig's bus width according to at_dmac.
 * 1 byte -> 0, 2 bytes -> 1, 4 bytes -> 2.
 */
static inline u8 convert_buswidth(enum dma_slave_buswidth addr_width)
{
	switch (addr_width) {
	case DMA_SLAVE_BUSWIDTH_2_BYTES:
		return 1;
	case DMA_SLAVE_BUSWIDTH_4_BYTES:
		return 2;
	default:
		/* For 1 byte width or fallback */
		return 0;
	}
}

static size_t sunxi_get_desc_size(struct sunxi_desc *txd)
{
	struct sunxi_rdma_lli *lli;
	size_t size = 0;

	for (lli = txd->cur_lli; lli != NULL; lli = lli->v_lln)
		size += lli->len;

	return size;
}

/*
 * sunxi_get_chan_size - get the bytes left of one channel.
 * @ch: the channel
 */
static size_t sunxi_get_chan_size(struct sunxi_chan *ch)
{
	struct sunxi_rdma_lli *lli;
	struct sunxi_desc *txd;
	struct sunxi_rdmadev *sdev;
	size_t size = 0;
	u32 chan_num = ch->vc.chan.chan_id;
	u32 ctrl_reg;

	txd = ch->desc;

	if (!txd)
		return 0;

	sdev = to_sunxi_dmadev(ch->vc.chan.device);

	ctrl_reg = sunxi_smc_readl(sdev->base + RDMA_CTRL(chan_num));
	sunxi_smc_writel(REMAIN_MODE | ctrl_reg, sdev->base + RDMA_CTRL(chan_num));
	size = sunxi_smc_readl(sdev->base + RDMA_CNT(chan_num));
	ctrl_reg = sunxi_smc_readl(sdev->base + RDMA_CTRL(chan_num));
	sunxi_smc_writel(ctrl_reg & (~REMAIN_MODE), sdev->base + RDMA_CTRL(chan_num));

	for (lli = txd->cur_lli->v_lln; lli != NULL; lli = lli->v_lln)
		size += lli->len;

	return size;
}

/*
 * sunxi_free_desc - free the struct sunxi_desc.
 * @vd: the virt-desc for this chan
 */
static void sunxi_free_desc(struct virt_dma_desc *vd)
{
	struct sunxi_desc *txd = to_sunxi_desc(&vd->tx);
	struct sunxi_rdma_lli *li_adr, *next_virt;

	if (unlikely(!txd))
		return;

	li_adr = txd->cur_lli;
	while(li_adr != NULL) {
		next_virt = li_adr->v_lln;
		kfree(li_adr);
		li_adr = next_virt;
	}

	kfree(txd);
}

static inline void sunxi_dump_com_regs(struct sunxi_chan *ch)
{
	struct sunxi_rdmadev *sdev;

	sdev = to_sunxi_dmadev(ch->vc.chan.device);

	pr_debug("Common Register: \n"
			"\t\tIRQ_EN (0x00): 0x%08x\n"
			"\t\tIRQ_PEN(0x04): 0x%08x\n"
			"\t\tGATE   (0x08): 0x%08x\n",
			sunxi_smc_readl(sdev->base + RDMA_IRQ_EN),
			sunxi_smc_readl(sdev->base + RDMA_IRQ_PEN),
			sunxi_smc_readl(sdev->base + RDMA_GATE));
}

static inline void sunxi_dump_chan_regs(struct sunxi_chan *ch)
{
	struct sunxi_rdmadev *sdev = to_sunxi_dmadev(ch->vc.chan.device);
	u32 chan_num = ch->vc.chan.chan_id;

	pr_debug("CHAN: %d, Register:\n"
			"\t\tRDMA_SRC (0x%04x): 0x%08x\n"
			"\t\tRDMA_DST (0x%04x): 0x%08x\n"
			"\t\tRDMA_CTRL(0x%04x): 0x%08x\n"
			"\t\tRDMA_CNT (0x%04x): 0x%08x\n",
			chan_num,
			RDMA_SRC(chan_num),
			sunxi_smc_readl(sdev->base + RDMA_SRC(chan_num)),
			RDMA_DST(chan_num),
			sunxi_smc_readl(sdev->base + RDMA_DST(chan_num)),
			RDMA_CTRL(chan_num),
			sunxi_smc_readl(sdev->base + RDMA_CTRL(chan_num)),
			RDMA_CNT(chan_num),
			sunxi_smc_readl(sdev->base + RDMA_CNT(chan_num)));
}


/*
 * sunxi_terminate_all - stop all descriptors that waiting transfer on chan.
 * @ch: the channel to stop
 */
static int sunxi_terminate_all(struct sunxi_chan *ch)
{
	struct sunxi_rdmadev *sdev = to_sunxi_dmadev(ch->vc.chan.device);
	u32 chan_num = ch->vc.chan.chan_id;
	unsigned long flags;
	LIST_HEAD(head);

	spin_lock_irqsave(&ch->vc.lock, flags);

	spin_lock(&sdev->lock);
	list_del_init(&ch->node);
	spin_unlock(&sdev->lock);

	if (ch->desc)
		ch->desc = NULL;

	ch->cyclic = false;

	sunxi_smc_writel(~CHAN_START, sdev->base + RDMA_CTRL(chan_num));

	vchan_get_all_descriptors(&ch->vc, &head);
	spin_unlock_irqrestore(&ch->vc.lock, flags);
	vchan_dma_desc_free_list(&ch->vc, &head);

	return 0;
}

/*
 * sunxi_start_desc - begin to transport the descriptor
 * @ch: the channel of descriptor
 */
static void sunxi_start_desc(struct sunxi_chan *ch)
{
	struct virt_dma_desc *vd = vchan_next_desc(&ch->vc);
	struct sunxi_desc *txd = to_sunxi_desc(&vd->tx);
	struct sunxi_rdmadev *sdev = to_sunxi_dmadev(ch->vc.chan.device);
	u32 chan_num = ch->vc.chan.chan_id;
	u32 value;

	if (!vd) {
		ch->desc = NULL;
		return;
	}

	list_del(&vd->node);

	ch->desc = txd;

	while(sunxi_smc_readl(sdev->base + RDMA_CTRL(chan_num)) & BUSY_BIT)
			cpu_relax();

	ch->irq_type = IRQ_PKG;

	if (ch->cyclic){
		ch->irq_type |= IRQ_HALF;
		sunxi_smc_writel(1 << 16, sdev->base + RDMA_GATE);
	}

	value = sunxi_smc_readl(sdev->base + RDMA_IRQ_EN);
	value |= SHIFT_IRQ_MASK(ch->irq_type, chan_num);
	sunxi_smc_writel(value, sdev->base + RDMA_IRQ_EN);

	/* write the first lli address to register, and start to transfer */
	sunxi_smc_writel(txd->cur_lli->src, sdev->base + RDMA_SRC(chan_num));
	sunxi_smc_writel(txd->cur_lli->dst, sdev->base + RDMA_DST(chan_num));
	sunxi_smc_writel(txd->cur_lli->len, sdev->base + RDMA_CNT(chan_num));
	sunxi_smc_writel(txd->cur_lli->cfg | CHAN_START, sdev->base + RDMA_CTRL(chan_num));

	sunxi_dump_com_regs(ch);
	sunxi_dump_chan_regs(ch);
}

/*
 * sunxi_dump_lli - dump the information for one lli
 * @shcan: the channel
 * @lli: a lli to dump
 */
static inline void sunxi_dump_lli(struct sunxi_chan *schan, struct sunxi_rdma_lli *head)
{
	struct sunxi_rdma_lli *lli;
	pr_debug("[sunxi_rdma]: List of rdma_lli:\n");
	for (lli = head; lli != NULL; lli = lli->v_lln)
		pr_debug("\t\tcfg: 0x%08x, src: 0x%08x, dst: 0x%08x, "
				"len: 0x%08x, next: 0x%08x\n",
				(unsigned int)lli->cfg, (unsigned int)lli->src,
				(unsigned int)lli->dst, (unsigned int)lli->len,
				(unsigned int)lli->v_lln);
}

static void *sunxi_lli_list(struct sunxi_rdma_lli *prev,
		struct sunxi_rdma_lli *next,
		struct sunxi_desc *txd)
{
	if ((!prev && !txd) || !next)
		return NULL;

	if (!prev)
		txd->head = txd->cur_lli = next;
	else
		prev->v_lln = next;

	next->v_lln = NULL;

	return next;
}

static inline void sunxi_cfg_lli(struct sunxi_rdma_lli *lli, dma_addr_t src,
		dma_addr_t dst, u32 len, struct dma_slave_config *config)
{
	u32 src_width, dst_width;

	if (!config)
		return;

	/* Get the data width */
	src_width = convert_buswidth(config->src_addr_width);
	dst_width = convert_buswidth(config->dst_addr_width);

	lli->cfg = SRC_BURST(config->src_maxburst)
			| SRC_WIDTH(src_width)
			| DST_BURST(config->dst_maxburst)
			| DST_WIDTH(dst_width)
			| WAIT_STATE;

	lli->src = src;
	lli->dst = dst;
	lli->len = len;
}


/*
 * sunxi_dma_tasklet - ensure that the desc's lli be putted into hardware.
 * @data: sunxi_rdmadev
 */
static void sunxi_rdma_tasklet(unsigned long data)
{
	struct sunxi_rdmadev *sdev = (struct sunxi_rdmadev *)data;
	LIST_HEAD(head);

	spin_lock_irq(&sdev->lock);
	list_splice_tail_init(&sdev->pending, &head);
	spin_unlock_irq(&sdev->lock);

	while (!list_empty(&head)) {
		struct sunxi_chan *c = list_first_entry(&head,
			struct sunxi_chan, node);

		spin_lock_irq(&c->vc.lock);
		list_del_init(&c->node);
		sunxi_start_desc(c);
		spin_unlock_irq(&c->vc.lock);
	}
}

/*
 * sunxi_lli_load - set the next peroid for cyclic mode
 * @chan: the channels to transfer
 */
static struct sunxi_rdma_lli *sunxi_lli_load(struct sunxi_chan *chan)
{
	struct sunxi_rdma_lli *l_item;
	struct sunxi_rdmadev *sdev = to_sunxi_dmadev(chan->vc.chan.device);
	u32 chan_num = chan->vc.chan.chan_id;

	if (!chan->desc || !chan->desc->cur_lli)
		return NULL;

	l_item = chan->desc->cur_lli->v_lln;

	if (!l_item) {
		if (chan->cyclic) {
			l_item = chan->desc->head;
		} else
			return NULL;
	}

	sunxi_smc_writel(l_item->src, sdev->base + RDMA_SRC(chan_num));
	sunxi_smc_writel(l_item->dst, sdev->base + RDMA_DST(chan_num));

	return l_item;
}

/*
 * sunxi_rdma_interrupt - interrupt handle.
 * @irq: irq number
 * @dev_id: sunxi_rdmadev
 */
static irqreturn_t sunxi_rdma_interrupt(int irq, void *dev_id)
{
	struct sunxi_rdmadev *sdev = (struct sunxi_rdmadev *)dev_id;
	struct sunxi_chan *ch;
	struct sunxi_desc *desc;
	unsigned long flags;
	u32 status = 0;

	/* Get the status of irq */
	status = sunxi_smc_readl(sdev->base + RDMA_IRQ_PEN);
	/* Clear the bit of irq status */
	sunxi_smc_writel(status, sdev->base + RDMA_IRQ_PEN);

	dev_dbg(sdev->dma_dev.dev, "[sunxi_rdma]: DMA irq status: 0x%08x\n", status);

	list_for_each_entry(ch, &sdev->dma_dev.channels, vc.chan.device_node) {
		u32 chan_num = ch->vc.chan.chan_id;

		if (!(SHIFT_IRQ_MASK(ch->irq_type, chan_num) & status))
			continue;

		if (!ch->desc)
			continue;

		spin_lock_irqsave(&ch->vc.lock, flags);
		desc = ch->desc;

		if (ch->cyclic) {
			if (status & IRQ_HALF) {
				sunxi_lli_load(ch);
			}
			if (status & IRQ_PKG) {
				if (desc->cur_lli && !desc->cur_lli->v_lln)
					desc->cur_lli = desc->head;
				else
					desc->cur_lli = desc->cur_lli->v_lln;
			}
			vchan_cyclic_callback(&desc->vd);
		} else {
			vchan_cookie_complete(&desc->vd);
			sunxi_start_desc(ch);
		}
		spin_unlock_irqrestore(&ch->vc.lock, flags);
	}

	return IRQ_HANDLED;
}

static struct dma_async_tx_descriptor *sunxi_prep_rdma_memcpy(
		struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
		size_t len, unsigned long flags)
{
	struct sunxi_chan *schan = to_sunxi_chan(chan);
	struct sunxi_desc *txd;
	struct sunxi_rdma_lli *l_item;
	struct sunxi_rdmadev *sdev = to_sunxi_dmadev(chan->device);
	struct dma_slave_config *sconfig = &schan->cfg;

	dev_dbg(chan2dev(chan), "%s; chan: %d, dest: 0x%08x, "
			"src: 0x%08x, len: 0x%08x. flags: 0x%08lx\n",
			__func__, schan->vc.chan.chan_id, dest, src, len, flags);

	if (unlikely(!len)) {
		dev_dbg(chan2dev(chan), "%s: memcpy length is zero!!\n", __func__);
		return NULL;
	}

	txd = kzalloc(sizeof(*txd), GFP_NOWAIT);
	if (!txd) {
		dev_err(chan2dev(chan), "%s: Failed to alloc sunxi_desc!!\n", __func__);
		return NULL;
	}
	vchan_tx_prep(&schan->vc, &txd->vd, flags);

	l_item = kzalloc(sizeof(struct sunxi_rdma_lli), GFP_NOWAIT);
	if (!l_item) {
		sunxi_free_desc(&txd->vd);
		dev_err(sdev->dma_dev.dev, "Failed to alloc lli memory!!!\n");
		return NULL;
	}

	sunxi_cfg_lli(l_item, src, dest, len, sconfig);
	l_item->cfg |= SRC_DRQ(DRQSRC_SDRAM)
			| DST_DRQ(DRQDST_SDRAM)
			| DST_LINEAR_MODE
			| SRC_LINEAR_MODE;

	sunxi_lli_list(NULL, l_item, txd);
	sunxi_dump_lli(schan, txd->head);

	return &txd->vd.tx;
}


/**
 * sunxi_prep_rdma_cyclic - prepare the cyclic DMA transfer
 * @chan: the DMA channel to prepare
 * @buf_addr: physical DMA address where the buffer starts
 * @buf_len: total number of bytes for the entire buffer
 * @period_len: number of bytes for each period
 * @dir: transfer direction, to or from device
 *
 * Must be called before trying to start the transfer. Returns a valid struct
 * sunxi_cyclic_desc if successful or an ERR_PTR(-errno) if not successful.
 */
struct dma_async_tx_descriptor *sunxi_prep_rdma_cyclic( struct dma_chan *chan,
		dma_addr_t buf_addr, size_t buf_len, size_t period_len,
		enum dma_transfer_direction dir, unsigned long flags, void *context)
{
	struct sunxi_desc *txd;
	struct sunxi_chan *schan = to_sunxi_chan(chan);
	struct sunxi_rdma_lli *l_item, *prev = NULL;
	struct dma_slave_config *sconfig = &schan->cfg;

	unsigned int periods = buf_len / period_len;
	unsigned int i;

	txd = kzalloc(sizeof(*txd), GFP_NOWAIT);
	if (!txd) {
		dev_err(chan2dev(chan), "%s: Failed to alloc sunxi_desc!!\n", __func__);
		return NULL;
	}
	vchan_tx_prep(&schan->vc, &txd->vd, flags);

	for (i = 0; i < periods; i++){
		l_item = kzalloc(sizeof(struct sunxi_rdma_lli), GFP_NOWAIT);
		if (!l_item) {
			sunxi_free_desc(&txd->vd);
			return NULL;
		}

		if (dir == DMA_MEM_TO_DEV) {
			sunxi_cfg_lli(l_item, (buf_addr + period_len * i),
					sconfig->dst_addr, period_len, sconfig);
			l_item->cfg |= GET_DST_DRQ(sconfig->slave_id)
					| SRC_LINEAR_MODE
					| DST_IO_MODE
					| SRC_DRQ(DRQSRC_SDRAM);
		} else if (dir == DMA_DEV_TO_MEM) {
			sunxi_cfg_lli(l_item, sconfig->src_addr,
					(buf_addr + period_len * i), period_len, sconfig);
			l_item->cfg |= GET_SRC_DRQ(sconfig->slave_id)
					| DST_LINEAR_MODE
					| SRC_IO_MODE
					| DST_DRQ(DRQDST_SDRAM);
		}

		l_item->cfg |= CONTI_MODE;
		prev = sunxi_lli_list(prev, l_item, txd);
	}

	/* Make a cyclic list */
	schan->cyclic = true;

	sunxi_dump_lli(schan, txd->head);

	return &txd->vd.tx;
}

static int sunxi_set_runtime_config(struct dma_chan *chan,
		struct dma_slave_config *sconfig)
{
	struct sunxi_chan *schan = to_sunxi_chan(chan);

	memcpy(&schan->cfg, sconfig, sizeof(struct dma_slave_config));

	convert_burst(&schan->cfg.src_maxburst);
	convert_burst(&schan->cfg.dst_maxburst);

	return 0;
}

static int sunxi_rdma_control(struct dma_chan *chan, enum dma_ctrl_cmd cmd,
		       unsigned long arg)
{
	struct sunxi_chan *schan = to_sunxi_chan(chan);
	int ret = 0;

	switch(cmd) {
	case DMA_RESUME:
		break;
	case DMA_PAUSE:
		break;
	case DMA_TERMINATE_ALL:
		ret = sunxi_terminate_all(schan);
		break;
	case DMA_SLAVE_CONFIG:
		ret = sunxi_set_runtime_config(chan, (struct dma_slave_config *)arg);
		break;
	default:
		ret = -ENXIO;
		break;
	}
	return ret;
}

static enum dma_status sunxi_tx_status(struct dma_chan *chan,
	      dma_cookie_t cookie, struct dma_tx_state *txstate)
{
	struct sunxi_chan *schan = to_sunxi_chan(chan);
	struct virt_dma_desc *vd;
	enum dma_status ret;
	unsigned long flags;
	size_t bytes = 0;

	ret = dma_cookie_status(chan, cookie, txstate);
	if (ret == DMA_SUCCESS || !txstate) {
		return ret;
	}

	spin_lock_irqsave(&schan->vc.lock, flags);
	vd = vchan_find_desc(&schan->vc, cookie);
	if (vd) {
		bytes = sunxi_get_desc_size(to_sunxi_desc(&vd->tx));
	} else if (schan->desc && schan->desc->vd.tx.cookie == cookie) {
		bytes = sunxi_get_chan_size(to_sunxi_chan(chan));
	}

	/*
	 * This cookie not complete yet
	 * Get number of bytes left in the active transactions and queue
	 */
	dma_set_residue(txstate, bytes);
	spin_unlock_irqrestore(&schan->vc.lock, flags);

	return ret;
}

/*
 * sunxi_issue_pending - try to finish work
 * @chan: target DMA channel
 *
 * It will call vchan_issue_pending(), which can move the desc_submitted
 * list to desc_issued list. And we will move the chan to pending list of
 * sunxi_rdmadev.
 */
static void sunxi_issue_pending(struct dma_chan *chan)
{
	struct sunxi_chan *schan = to_sunxi_chan(chan);
	struct sunxi_rdmadev *sdev = to_sunxi_dmadev(chan->device);
	unsigned long flags;

	spin_lock_irqsave(&schan->vc.lock, flags);
	if (vchan_issue_pending(&schan->vc) && !schan->desc) {
		if (schan->cyclic){
			sunxi_start_desc(schan);
			goto out;
		}

		spin_lock(&sdev->lock);
		if (list_empty(&schan->node))
			list_add_tail(&schan->node, &sdev->pending);
		spin_unlock(&sdev->lock);
		tasklet_schedule(&sdev->task);
	}
out:
	spin_unlock_irqrestore(&schan->vc.lock, flags);
}

static int sunxi_alloc_chan_resources(struct dma_chan *chan)
{
	struct sunxi_chan *schan = to_sunxi_chan(chan);
	dev_dbg(chan2parent(chan), "%s: Now alloc chan resources!\n", __func__);

	schan->cyclic = false;

	return 0;
}

/*
 * sunxi_free_chan_resources - free the resources of channel
 * @chan: the channel to free
 */
static void sunxi_free_chan_resources(struct dma_chan *chan)
{
	struct sunxi_chan *schan = to_sunxi_chan(chan);

	vchan_free_chan_resources(&schan->vc);

	dev_dbg(chan2parent(chan), "%s: Now free chan resources!!\n", __func__);
}

/*
 * sunxi_chan_free - free the channle on dmadevice
 * @sdev: the dmadevice of sunxi
 */
static inline void sunxi_chan_free(struct sunxi_rdmadev *sdev)
{
	struct sunxi_chan *ch;

	tasklet_kill(&sdev->task);
	while(!list_empty(&sdev->dma_dev.channels)) {
		ch = list_first_entry(&sdev->dma_dev.channels,
				struct sunxi_chan, vc.chan.device_node);
		list_del(&ch->vc.chan.device_node);
		tasklet_kill(&ch->vc.task);
		kfree(ch);
	}

}

static int sunxi_rdma_probe(struct platform_device *pdev)
{
	struct sunxi_rdmadev *sunxi_dev;
	struct sunxi_chan *schan;
	struct resource *res;
	int irq;
	int ret, i;

	sunxi_dev = kzalloc(sizeof(struct sunxi_rdmadev), GFP_KERNEL);
	if (!sunxi_dev)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -EINVAL;
		goto io_err;
	}

	sunxi_dev->base = ioremap(res->start, resource_size(res));
	if (!sunxi_dev->base) {
		dev_err(&pdev->dev, "Remap I/O memory failed!\n");
		ret = -ENOMEM;
		goto io_err;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		ret = irq;
		goto irq_err;
	}

	ret = request_irq(irq, sunxi_rdma_interrupt, IRQF_PROBE_SHARED,
			dev_name(&pdev->dev), sunxi_dev);
	if (ret) {
		dev_err(&pdev->dev, "NO IRQ found!!!\n");
		goto irq_err;
	}

	platform_set_drvdata(pdev, sunxi_dev);
	INIT_LIST_HEAD(&sunxi_dev->pending);
	spin_lock_init(&sunxi_dev->lock);

	/* Initialize dmaengine */
	dma_cap_set(DMA_MEMCPY, sunxi_dev->dma_dev.cap_mask);
	dma_cap_set(DMA_SLAVE, sunxi_dev->dma_dev.cap_mask);
	dma_cap_set(DMA_CYCLIC, sunxi_dev->dma_dev.cap_mask);

	INIT_LIST_HEAD(&sunxi_dev->dma_dev.channels);
	sunxi_dev->dma_dev.device_alloc_chan_resources	= sunxi_alloc_chan_resources;
	sunxi_dev->dma_dev.device_free_chan_resources	= sunxi_free_chan_resources;
	sunxi_dev->dma_dev.device_tx_status		= sunxi_tx_status;
	sunxi_dev->dma_dev.device_issue_pending		= sunxi_issue_pending;
	sunxi_dev->dma_dev.device_prep_dma_cyclic	= sunxi_prep_rdma_cyclic;
	sunxi_dev->dma_dev.device_prep_dma_memcpy	= sunxi_prep_rdma_memcpy;
	sunxi_dev->dma_dev.device_control		= sunxi_rdma_control;

	sunxi_dev->dma_dev.dev = &pdev->dev;

	tasklet_init(&sunxi_dev->task, sunxi_rdma_tasklet, (unsigned long)sunxi_dev);

	for (i = 0; i < NR_MAX_CHAN; i++){
		schan = kzalloc(sizeof(*schan), GFP_KERNEL);
		if (!schan){
			dev_err(&pdev->dev, "%s: no memory for channel\n", __func__);
			ret = -ENOMEM;
			goto chan_err;
		}
		INIT_LIST_HEAD(&schan->node);
		sunxi_dev->dma_dev.chancnt++;
		schan->vc.desc_free = sunxi_free_desc;
		vchan_init(&schan->vc, &sunxi_dev->dma_dev);
	}

	/* Register the sunxi-dma to dmaengine */
	ret = dma_async_device_register(&sunxi_dev->dma_dev);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to register DMA engine device: %d\n", ret);
		goto chan_err;
	}

	sunxi_smc_writel(sunxi_smc_readl((void *)(0xf8001400 + 0x28)) | (1 << 16), (void *)(0xf8001400 + 0x28));
	sunxi_smc_writel(sunxi_smc_readl((void *)(0xf8001400 + 0xB0)) | (1 << 16), (void *)(0xf8001400 + 0xB0));

	return 0;

chan_err:
	sunxi_chan_free(sunxi_dev);
	platform_set_drvdata(pdev, NULL);
	free_irq(irq, sunxi_dev);
irq_err:
	iounmap(sunxi_dev->base);
io_err:
	kfree(sunxi_dev);
	return ret;
}

static int sunxi_rdma_remove(struct platform_device *pdev)
{
	struct sunxi_rdmadev *sunxi_dev = platform_get_drvdata(pdev);

	dma_async_device_unregister(&sunxi_dev->dma_dev);

	sunxi_chan_free(sunxi_dev);

	free_irq(platform_get_irq(pdev, 0), sunxi_dev);
	iounmap(sunxi_dev->base);
	kfree(sunxi_dev);

	return 0;
}

static void sunxi_rdma_shutdown(struct platform_device *pdev)
{
}

static int sunxi_suspend_noirq(struct device *dev)
{
	return 0;
}

static int sunxi_resume_noirq(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops sunxi_dev_pm_ops = {
	.suspend_noirq = sunxi_suspend_noirq,
	.resume_noirq = sunxi_resume_noirq,
	.freeze_noirq = sunxi_suspend_noirq,
	.thaw_noirq = sunxi_resume_noirq,
	.restore_noirq = sunxi_resume_noirq,
	.poweroff_noirq = sunxi_suspend_noirq,
};

static struct resource sunxi_rdma_reousce[] = {
	[0] = {
		.start = RDMA_PHYS_BASE,
		.end = RDMA_PHYS_BASE + RDMA_CNT(NR_MAX_CHAN),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = RDMA_IRQ_ID,
		.end = RDMA_IRQ_ID,
		.flags = IORESOURCE_IRQ,
	},
};

u64 sunxi_rdma_mask = DMA_BIT_MASK(32);
static struct platform_device sunxi_rdma_device = {
	.name = "sunxi_rdmac",
	.id = -1,
	.resource = sunxi_rdma_reousce,
	.num_resources = ARRAY_SIZE(sunxi_rdma_reousce),
	.dev = {
		.dma_mask = &sunxi_rdma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

static struct platform_driver sunxi_rdma_driver = {
	.probe		= sunxi_rdma_probe,
	.remove		= sunxi_rdma_remove,
	.shutdown	= sunxi_rdma_shutdown,
	.driver = {
		.name	= "sunxi_rdmac",
		.pm	= &sunxi_dev_pm_ops,
	},
};

bool sunxi_rdma_filter_fn(struct dma_chan *chan, void *param)
{
	bool ret = false;
	if (chan->device->dev->driver == &sunxi_rdma_driver.driver) {
		const char *p = param;
		ret = !strcmp("sunxi_rdmac", p);
		pr_debug("[sunxi_rdma]: sunxi_rdma_filter_fn: %s\n", p);
	}
	return ret;
}
EXPORT_SYMBOL_GPL(sunxi_rdma_filter_fn);

static int __init sunxi_rdma_init(void)
{
	int ret;

	platform_device_register(&sunxi_rdma_device);
	ret = platform_driver_register(&sunxi_rdma_driver);

	return ret;
}
subsys_initcall_sync(sunxi_rdma_init);

static void __exit sunxi_rdma_exit(void)
{
	platform_driver_unregister(&sunxi_rdma_driver);
	platform_device_unregister(&sunxi_rdma_device);
}
module_exit(sunxi_rdma_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sunxi RDMA Controller driver");
MODULE_AUTHOR("Sugar");
MODULE_ALIAS("platform:sunxi_rdmac");
