/*******************************************************************************
 * Copyright © 2012-2014, Shuge
 *		Author: Sugar <shugeLinux@gmail.com>
 *
 * This file is provided under a dual BSD/GPL license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 ********************************************************************************/

#ifndef __SUNXI_GETH_H__
#define __SUNXI_GETH_H__

#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/init.h>
#include <mach/sys_config.h>

#if defined(CONFIG_ARCH_SUN8I)
#define GETH_BASE		0x01c30000
#elif defined(CONFIG_ARCH_SUN9I)
#define GETH_BASE		0x00830000
#endif

/******************************************************************************
 *
 * the system register for geth.
 *
 *****************************************************************************/
#if defined(CONFIG_ARCH_SUN8I)
#define GPIO_BASE		0x01C20800
#elif defined(CONFIG_ARCH_SUN9I)
#define GPIO_BASE		0x06000800
#endif
#define PA_CFG0			(0x00)
#define PA_CFG1			(0x04)
#define PA_CFG2			(0x08)
#define PA_CFG3			(0x0C)

/* Clk control */
#if defined(CONFIG_ARCH_SUN8I)

#define CCMU_BASE		0x01c20000
#define AHB1_GATING		(0x60)
#define AHB1_MOD_RESET		(0x2c0)

#if defined(CONFIG_ARCH_SUN8IW1)
#define SYS_CTL_BASE		CCMU_BASE
#define GETH_CLK_REG		0x00D0
#define GETH_CLK_GPIT		0x00000004
#else
#define SYS_CTL_BASE		0x01c00000
#define GETH_CLK_REG		0x0030
#define GETH_CLK_GPIT		0x00000004
#endif

#elif defined(CONFIG_ARCH_SUN9I)

#define CCMU_BASE		0x06000400
#define AHB1_GATING		(0x0184)
#define AHB1_MOD_RESET		(0x01A4)

#define SYS_CTL_BASE		0x00800000
#define GETH_CLK_REG		0x0030
#define GETH_CLK_GPIT		0x00000004

#endif

#define GETH_RESET_BIT		0x00020000
#define GETH_AHB_BIT		0x00020000

#define PLL1_CFG		0x00
#define PLL6_CFG		0x28

/* GETH_FRAME_FILTER  register value */
#define GETH_FRAME_FILTER_PR	0x00000001	/* Promiscuous Mode */
#define GETH_FRAME_FILTER_HUC	0x00000002	/* Hash Unicast */
#define GETH_FRAME_FILTER_HMC	0x00000004	/* Hash Multicast */
#define GETH_FRAME_FILTER_DAIF	0x00000008	/* DA Inverse Filtering */
#define GETH_FRAME_FILTER_PM	0x00000010	/* Pass all multicast */
#define GETH_FRAME_FILTER_DBF	0x00000020	/* Disable Broadcast frames */
#define GETH_FRAME_FILTER_SAIF	0x00000100	/* Inverse Filtering */
#define GETH_FRAME_FILTER_SAF	0x00000200	/* Source Address Filter */
#define GETH_FRAME_FILTER_HPF	0x00000400	/* Hash or perfect Filter */
#define GETH_FRAME_FILTER_RA	0x80000000	/* Receive all mode */

struct dma_desc {
	u32 desc[4];
}__attribute__((packed,aligned(4)));


extern int sunxi_mdio_read(void *,  int, int);
extern int sunxi_mdio_write(void *, int, int, unsigned short);
extern int sunxi_mdio_reset(void *);
extern void sunxi_set_link_mode(void *iobase, int duplex, int speed);
extern void sunxi_int_disable(void *);
extern int sunxi_int_status(void *, void *x);
extern int sunxi_mac_init(void *, int txmode, int rxmode);
extern void sunxi_set_umac(void *, unsigned char *, int);
extern void sunxi_mac_enable(void *);
extern void sunxi_mac_disable(void *);
extern void sunxi_tx_poll(void *);
extern void sunxi_int_enable(void *);
extern void sunxi_start_rx(void *, unsigned long);
extern void sunxi_start_tx(void *, unsigned long);
extern void sunxi_stop_tx(void *);
extern void sunxi_stop_rx(void *);
extern void sunxi_hash_filter(void *iobase, unsigned long low, unsigned long high);
extern void sunxi_set_filter(void *iobase, unsigned long flags);
extern void sunxi_flow_ctrl(void *iobase, int duplex, int fc, int pause);
extern void sunxi_mac_loopback(void *iobase, int enable);

extern void desc_buf_set(struct dma_desc *p, unsigned long paddr, int size);
extern void desc_set_own(struct dma_desc *p);
extern void desc_init_chain(struct dma_desc *p, unsigned long paddr,  int size);
extern void desc_tx_close(struct dma_desc *first, struct dma_desc *end, int csum_insert);
extern void desc_init(struct dma_desc *p);
extern int desc_get_tx_status(struct dma_desc *desc, void *x);
extern int desc_buf_get_len(struct dma_desc *desc);
extern int desc_buf_get_addr(struct dma_desc *desc);
extern int desc_get_rx_status(struct dma_desc *desc, void *x);
extern int desc_get_own(struct dma_desc *desc);
extern int desc_get_tx_ls(struct dma_desc *desc);
extern int desc_rx_frame_len(struct dma_desc *desc);

extern int sunxi_mac_reset(void *iobase, void (*mdelay)(int), int n);
extern void sunxi_geth_register(void *iobase, int version, unsigned int div);

#if defined(CONFIG_ARCH_SUN8IW3) \
	|| defined(CONFIG_ARCH_SUN9IW1) \
	|| defined(CONFIG_ARCH_SUN7I)
#define HW_VERSION	0
#else
#define HW_VERSION	1
#endif

#endif
