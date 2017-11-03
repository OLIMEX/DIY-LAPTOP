/*******************************************************************************
 * Copyright © 2012-2015, Shuge
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
#include <linux/clk.h>
#include <linux/mii.h>
#include <linux/gpio.h>
#include <linux/crc32.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/sunxi-sid.h>
#include <linux/regulator/consumer.h>
#include <linux/of_net.h>
#include <linux/io.h>

#include <asm/io.h>

#include "sunxi-gmac.h"

#ifndef GMAC_CLK
#define GMAC_CLK "gmac"
#endif

#ifndef EPHY_CLK
#define EPHY_CLK "ephy"
#endif

#define DMA_DESC_RX	256
#define DMA_DESC_TX	256
#define BUDGET		(dma_desc_rx/4)
#define TX_THRESH	(dma_desc_tx/4)

#define HASH_TABLE_SIZE	64
#define MAX_BUF_SZ	(SZ_2K - 1)

#undef PKT_DEBUG
#undef DESC_PRINT

#define circ_cnt(head, tail, size) (((head) > (tail)) ? \
					((head) - (tail)) : \
					((head) - (tail)) & ((size)-1))

#define circ_space(head, tail, size) circ_cnt((tail), ((head)+1), (size))

#define circ_inc(n, s) (((n) + 1) % (s))

#define GETH_MAC_ADDRESS "00:00:00:00:00:00"
static char *mac_str = GETH_MAC_ADDRESS;
module_param(mac_str, charp, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(mac_str, "MAC Address String.(xx:xx:xx:xx:xx:xx)");

static int rxmode = 1;
module_param(rxmode, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rxmode, "DMA threshold control value");

static int txmode = 1;
module_param(txmode, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(txmode, "DMA threshold control value");

static int pause = 0x400;
module_param(pause, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pause, "Flow Control Pause Time");

#define TX_TIMEO	5000
static int watchdog = TX_TIMEO;
module_param(watchdog, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(watchdog, "Transmit timeout in milliseconds");

static int dma_desc_rx = DMA_DESC_RX;
module_param(dma_desc_rx, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(watchdog, "The number of receive's descriptors");

static int dma_desc_tx = DMA_DESC_TX;
module_param(dma_desc_tx, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(watchdog, "The number of transmit's descriptors");

/*
 * - 0: Flow Off
 * - 1: Rx Flow
 * - 2: Tx Flow
 * - 3: Rx & Tx Flow
 */
static int flow_ctrl = 0;
module_param(flow_ctrl, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(flow_ctrl, "Flow control [0: off, 1: rx, 2: tx, 3: both]");

static unsigned long tx_delay = 0;
module_param(tx_delay, ulong, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tx_delay, "Adjust transmit clock delay, value: 0~7");

static unsigned long rx_delay = 0;
module_param(rx_delay, ulong, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rx_delay, "Adjust receive clock delay, value: 0~31");

struct geth_priv {
	struct dma_desc *dma_tx;
	struct sk_buff **tx_sk;
	unsigned int tx_clean;
	unsigned int tx_dirty;
	dma_addr_t dma_tx_phy;

	unsigned long buf_sz;

	struct dma_desc *dma_rx;
	struct sk_buff **rx_sk;
	unsigned int rx_clean;
	unsigned int rx_dirty;
	dma_addr_t dma_rx_phy;

	struct net_device *ndev;
	struct device *dev;
	struct napi_struct napi;

	struct geth_extra_stats xstats;

	struct mii_bus *mii;
	int link;
	int speed;
	int duplex;
#define INT_PHY 0
#define EXT_PHY 1
	int phy_ext;
	int phy_interface;

	void __iomem *base;
#ifndef CONFIG_GETH_SCRIPT_SYS
	void __iomem *gpiobase;
#else
	struct pinctrl *pinctrl;
#endif
#ifndef CONFIG_GETH_CLK_SYS
	void __iomem *clkbase;
#else
	struct clk *geth_clk;
	struct clk *ephy_clk;
#endif
	void __iomem *geth_extclk;
	struct regulator **power;
	bool is_suspend;

	spinlock_t lock;
	spinlock_t tx_lock;
};

#ifdef CONFIG_GETH_PHY_POWER
struct geth_power {
	unsigned int vol;
	const char *name;
};

struct geth_power power_tb[5] = {};
#endif
static u64 geth_dma_mask = DMA_BIT_MASK(32);


void sunxi_udelay(int n)
{
	udelay(n);
}

static int geth_stop(struct net_device *ndev);
static int geth_open(struct net_device *ndev);
static void geth_tx_complete(struct geth_priv *priv);
static void geth_rx_refill(struct net_device *ndev);

#ifdef DEBUG
static void desc_print(struct dma_desc *desc, int size)
{
#ifdef DESC_PRINT
	int i;
	for (i = 0; i < size; i++) {
		u32 *x = (u32 *)(desc + i);
		pr_info("\t%d [0x%08lx]: %08x %08x %08x %08x\n",
		       i, (unsigned long)(&desc[i]),
		       x[0], x[1], x[2], x[3]);
	}
	pr_info("\n");
#endif
}
#endif

static int geth_power_on(struct geth_priv *priv)
{
	int value;
#ifdef CONFIG_GETH_PHY_POWER
	struct regulator **regu;
	int ret = 0, i = 0;

	regu = kmalloc(ARRAY_SIZE(power_tb) *
			sizeof(struct regulator *), GFP_KERNEL);
	if (!regu)
		return -1;

	/* Set the voltage */
	for (i = 0; i < ARRAY_SIZE(power_tb) && power_tb[i].name; i++) {
		regu[i] = regulator_get(NULL, power_tb[i].name);
		if (IS_ERR(regu[i])) {
			ret = -1;
			goto err;
		}

		if (power_tb[i].vol != 0) {
			ret = regulator_set_voltage(regu[i], power_tb[i].vol,
					power_tb[i].vol);
			if (ret)
				goto err;
		}

		ret = regulator_enable(regu[i]);
		if (ret) {
			goto err;
		}
		mdelay(3);
	}

	msleep(300);
	priv->power = regu;
#endif

	value = readl(priv->geth_extclk + GETH_CLK_REG);
	if (priv->phy_ext == INT_PHY) {
		value |= (1 << 15);
		value &= ~(1 << 16);
		value |= (3 << 17);
	} else {
		value &= ~(1 << 15);
		value |= (1 << 16);
	}
	writel(value, priv->geth_extclk + GETH_CLK_REG);

	return 0;

#ifdef CONFIG_GETH_PHY_POWER
err:
	for(; i > 0; i--) {
		regulator_disable(regu[i - 1]);
		regulator_put(regu[i - 1]);
	}
	kfree(regu);
	priv->power = NULL;
	return ret;
#endif
}

static void geth_power_off(struct geth_priv *priv)
{
	int value;
#ifdef CONFIG_GETH_PHY_POWER
	struct regulator **regu = priv->power;
	int i = 0;

	if (priv->power == NULL)
		return;

	for (i = 0; i < ARRAY_SIZE(power_tb) && power_tb[i].name; i++) {
		regulator_disable(regu[i]);
		regulator_put(regu[i]);
	}
	kfree(regu);
#endif

	if (priv->phy_ext == INT_PHY) {
		value = readl(priv->geth_extclk + GETH_CLK_REG);
		value |= (1 << 16);
		writel(value, priv->geth_extclk + GETH_CLK_REG);
	}
}

/*
 * PHY interface operations
 */
static int geth_mdio_read(struct mii_bus *bus, int phyaddr, int phyreg)
{
	struct net_device *ndev = bus->priv;
	struct geth_priv *priv = netdev_priv(ndev);

	return (int)sunxi_mdio_read(priv->base,  phyaddr, phyreg);
}

static int geth_mdio_write(struct mii_bus *bus, int phyaddr,
				int phyreg, u16 data)
{
	struct net_device *ndev = bus->priv;
	struct geth_priv *priv = netdev_priv(ndev);

	sunxi_mdio_write(priv->base, phyaddr, phyreg, data);

	return 0;
}

static int geth_mdio_reset(struct mii_bus *bus)
{
	struct net_device *ndev = bus->priv;
	struct geth_priv *priv = netdev_priv(ndev);

	return sunxi_mdio_reset(priv->base);
}

static void geth_adjust_link(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);
	struct phy_device *phydev = ndev->phydev;
	unsigned long flags;
	int new_state = 0;

	if (phydev == NULL)
		return;

	spin_lock_irqsave(&priv->lock, flags);
	if (phydev->link) {
		/* Now we make sure that we can be in full duplex mode.
		 * If not, we operate in half-duplex mode. */
		if (phydev->duplex != priv->duplex) {
			new_state = 1;
			priv->duplex = phydev->duplex;
		}
		/* Flow Control operation */
		if (phydev->pause)
			sunxi_flow_ctrl(priv->base, phydev->duplex,
						 flow_ctrl, pause);

		if (phydev->speed != priv->speed) {
			new_state = 1;
			priv->speed = phydev->speed;
		}


		if (priv->link == 0) {
			new_state = 1;
			priv->link = phydev->link;
		}

#if 0
		/* Fix the A version chip mode, it not work at 1000M mode */
		if (sunxi_get_soc_ver() == SUN9IW1P1_REV_A
				&& priv->speed == SPEED_1000
				&& phydev->link == 1){
			priv->speed = 0;
			priv->link = 0;
			priv->duplex = -1;
			phydev->speed = SPEED_100;
			phydev->autoneg = AUTONEG_DISABLE;
			phydev->advertising &= ~ADVERTISED_Autoneg;
			phydev->state = PHY_UP;
			new_state = 0;
		}
#endif

		if (new_state)
			sunxi_set_link_mode(priv->base, priv->duplex, priv->speed);

#ifdef LOOPBACK_DEBUG
		phydev->state = PHY_FORCING;
#endif

	} else if (priv->link != phydev->link) {
		new_state = 1;
		priv->link = 0;
		priv->speed = 0;
		priv->duplex = -1;
	}

	if (new_state)
		phy_print_status(phydev);

	spin_unlock_irqrestore(&priv->lock, flags);
}

static int geth_phy_init(struct net_device *ndev)
{
	int value;
	struct mii_bus *new_bus;
	struct geth_priv *priv = netdev_priv(ndev);
	struct phy_device *phydev = NULL;

	new_bus = mdiobus_alloc();
	if (new_bus == NULL) {
		netdev_err(ndev, "Failed to alloc new mdio bus\n");
		return -ENOMEM;
	}

	new_bus->name = dev_name(priv->dev);
	new_bus->read = &geth_mdio_read;
	new_bus->write = &geth_mdio_write;
	new_bus->reset = &geth_mdio_reset;
	snprintf(new_bus->id, MII_BUS_ID_SIZE, "%s-%x",
		new_bus->name, 0);

	new_bus->parent = priv->dev;
	new_bus->priv = ndev;

	if (mdiobus_register(new_bus)) {
		printk(KERN_ERR "%s: Cannot register as MDIO bus\n", new_bus->name);
		goto reg_fail;
	}

	priv->mii = new_bus;

	phydev = phy_find_first(new_bus);
	if (!phydev) {
		netdev_err(ndev, "No PHY found!\n");
		goto err;
	}

	phydev->irq = PHY_POLL;

	phydev = phy_connect(ndev, dev_name(&phydev->dev),
			&geth_adjust_link, priv->phy_interface);
	if (IS_ERR(phydev)) {
		netdev_err(ndev, "Could not attach to PHY\n");
		goto err;
	} else {
		netdev_info(ndev, "%s: PHY ID %08x at %d IRQ %s (%s)\n",
				ndev->name, phydev->phy_id, phydev->addr,
				"poll", dev_name(&phydev->dev));
#if defined(CONFIG_ARCH_SUN8IW8) || defined(CONFIG_ARCH_SUN8IW7)
		if (priv->phy_ext == INT_PHY) {
			phy_write(phydev, 0x1f, 0x013d);
			phy_write(phydev, 0x10, 0x3ffe);
			phy_write(phydev, 0x1f, 0x063d);
			phy_write(phydev, 0x13, 0x8000);
			phy_write(phydev, 0x1f, 0x023d);
			phy_write(phydev, 0x18, 0x1000);
			phy_write(phydev, 0x1f, 0x063d);
			phy_write(phydev, 0x15, 0x132c);
			phy_write(phydev, 0x1f, 0x013d);
			phy_write(phydev, 0x13, 0xd602);
			phy_write(phydev, 0x17, 0x003b);
			phy_write(phydev, 0x1f, 0x063d);
			phy_write(phydev, 0x14, 0x7088);
			phy_write(phydev, 0x1f, 0x033d);
			phy_write(phydev, 0x11, 0x8530);
			phy_write(phydev, 0x1f, 0x003d);
		}

#endif
#ifdef CONFIG_ARCH_SUN50IW1P1
	//init ephy
	printk("init ephy for pine64\n");
	phy_write(phydev, 0x1f, 0x0007);//sel ext page
	phy_write(phydev, 0x1e, 0x00a4);//sel page 164
	phy_write(phydev, 0x1c, 0xb591);//only enable TX
	phy_write(phydev, 0x1f, 0x0000);//sel page 0
#endif

		phy_write(phydev, MII_BMCR, BMCR_RESET);
		while (BMCR_RESET & phy_read(phydev, MII_BMCR))
			msleep(30);

		value = phy_read(phydev, MII_BMCR);
		phy_write(phydev, MII_BMCR, (value & ~BMCR_PDOWN));

	}

	phydev->supported &= PHY_GBIT_FEATURES;
	phydev->advertising = phydev->supported;


	return 0;

err:
	mdiobus_unregister(new_bus);
reg_fail:
	mdiobus_free(new_bus);

	return -EINVAL;
}

static int geth_phy_release(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);
	struct phy_device *phydev = ndev->phydev;
	int value = 0;

	/* Stop and disconnect the PHY */
	if (phydev) {
		phy_stop(phydev);
		value = phy_read(phydev, MII_BMCR);
		phy_write(phydev, MII_BMCR, (value | BMCR_PDOWN));
		phy_disconnect(phydev);
		ndev->phydev = NULL;
	}

	if (priv->mii) {
		mdiobus_unregister(priv->mii);
		priv->mii->priv = NULL;
		mdiobus_free(priv->mii);
		priv->mii = NULL;
	}
	priv->link = PHY_DOWN;
	priv->speed = 0;
	priv->duplex = -1;

	return 0;
}


/*****************************************************************************
 *
 *
 ****************************************************************************/
static void geth_rx_refill(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);
	struct dma_desc *desc;
	struct sk_buff *sk = NULL;
	dma_addr_t paddr;

	while (circ_space(priv->rx_clean, priv->rx_dirty, dma_desc_rx) > 0) {
		int entry = priv->rx_clean;

		/* Find the dirty's desc and clean it */
		desc = priv->dma_rx + entry;

		if (priv->rx_sk[entry] == NULL) {
			sk = netdev_alloc_skb_ip_align(ndev, priv->buf_sz);

			if (unlikely(sk == NULL))
				break;

			priv->rx_sk[entry] = sk;
			paddr = dma_map_single(priv->dev, sk->data,
					priv->buf_sz, DMA_FROM_DEVICE);
			desc_buf_set(desc, paddr, priv->buf_sz);
		}

		wmb();
		desc_set_own(desc);
		priv->rx_clean = circ_inc(priv->rx_clean, dma_desc_rx);
	}
}

/*
 * geth_dma_desc_init - initialize the RX/TX descriptor list
 * @ndev: net device structure
 * Description: initialize the list for dma.
 */
static int geth_dma_desc_init(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);
	unsigned int buf_sz;

	priv->rx_sk = kzalloc(sizeof(struct sk_buff*) * dma_desc_rx,
				GFP_KERNEL);
	if (!priv->rx_sk)
		return -ENOMEM;

	priv->tx_sk = kzalloc(sizeof(struct sk_buff*) * dma_desc_tx,
				GFP_KERNEL);
	if (!priv->tx_sk)
		goto tx_sk_err;

	/* Set the size of buffer depend on the MTU & max buf size */
	buf_sz = MAX_BUF_SZ;

	priv->dma_tx = dma_alloc_coherent(priv->dev,
					dma_desc_tx *
					sizeof(struct dma_desc),
					&priv->dma_tx_phy,
					GFP_KERNEL);
	if (!priv->dma_tx)
		goto dma_tx_err;

	priv->dma_rx = dma_alloc_coherent(priv->dev,
					dma_desc_rx *
					sizeof(struct dma_desc),
					&priv->dma_rx_phy,
					GFP_KERNEL);
	if (!priv->dma_rx)
		goto dma_rx_err;

	priv->buf_sz = buf_sz;

	return 0;

dma_rx_err:
	dma_free_coherent(priv->dev, dma_desc_rx * sizeof(struct dma_desc),
			priv->dma_tx, priv->dma_tx_phy);
dma_tx_err:
	kfree(priv->tx_sk);
tx_sk_err:
	kfree(priv->rx_sk);

	return -ENOMEM;

}

static void geth_free_rx_sk(struct geth_priv *priv)
{
	int i;

	for (i = 0; i < dma_desc_rx; i++) {
		if (priv->rx_sk[i] != NULL) {
			struct dma_desc *desc = priv->dma_rx + i;
			dma_unmap_single(priv->dev, desc_buf_get_addr(desc),
					 desc_buf_get_len(desc),
					 DMA_FROM_DEVICE);
			dev_kfree_skb_any(priv->rx_sk[i]);
			priv->rx_sk[i] = NULL;
		}
	}
}

static void geth_free_tx_sk(struct geth_priv *priv)
{
	int i;

	for (i = 0; i < dma_desc_tx; i++) {
		if (priv->tx_sk[i] != NULL) {
			struct dma_desc *desc = priv->dma_tx + i;
			if (desc_buf_get_addr(desc))
				dma_unmap_single(priv->dev, desc_buf_get_addr(desc),
						 desc_buf_get_len(desc),
						 DMA_TO_DEVICE);
			dev_kfree_skb_any(priv->tx_sk[i]);
			priv->tx_sk[i] = NULL;
		}
	}
}

static void geth_free_dma_desc(struct geth_priv *priv)
{
	/* Free the region of consistent memory previously allocated for
	 * the DMA */
	dma_free_coherent(priv->dev, dma_desc_tx * sizeof(struct dma_desc),
			  priv->dma_tx, priv->dma_tx_phy);
	dma_free_coherent(priv->dev, dma_desc_rx * sizeof(struct dma_desc),
			  priv->dma_rx, priv->dma_rx_phy);

	kfree(priv->rx_sk);
	kfree(priv->tx_sk);
}


/*****************************************************************************
 *
 *
 ****************************************************************************/
#ifdef CONFIG_PM
static int geth_suspend(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct geth_priv *priv = netdev_priv(ndev);

	if (!ndev || !netif_running(ndev))
		return 0;

	priv->is_suspend = true;

	spin_lock(&priv->lock);
	netif_device_detach(ndev);
	spin_unlock(&priv->lock);

	geth_stop(ndev);

	device_enable_async_suspend(dev);

	return 0;
}

static int geth_resume(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct geth_priv *priv = netdev_priv(ndev);
	int ret;

	if (!netif_running(ndev))
		return 0;

	spin_lock(&priv->lock);
	netif_device_attach(ndev);
	spin_unlock(&priv->lock);

	ret = geth_open(ndev);
	priv->is_suspend = false;

	device_disable_async_suspend(dev);

	return ret;
}

static int geth_freeze(struct device *dev)
{
	return 0;
}

static int geth_restore(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops geth_pm_ops = {
	.suspend = geth_suspend,
	.resume = geth_resume,
	.freeze = geth_freeze,
	.restore = geth_restore,
};
#else
static const struct dev_pm_ops geth_pm_ops;
#endif /* CONFIG_PM */


/*****************************************************************************
 *
 *
 ****************************************************************************/
static void geth_check_addr(struct net_device *ndev, unsigned char *mac)
{
	int i;
	char *p = mac;

	if (!is_valid_ether_addr(ndev->dev_addr)) {
		for (i=0; i<ETH_ALEN; i++, p++)
			ndev->dev_addr[i] = simple_strtoul(p, &p, 16);

		if (!is_valid_ether_addr(ndev->dev_addr)) {
			sunxi_get_chipid_mac_addr(ndev->dev_addr);
		}

		if (!is_valid_ether_addr(ndev->dev_addr)) {
			random_ether_addr(ndev->dev_addr);
			printk(KERN_WARNING "%s: Use random mac address\n", ndev->name);
		}
	}
}

static void geth_clk_enable(struct geth_priv *priv)
{
	int phy_interface = 0;
	u32 clk_value;
#ifndef CONFIG_GETH_CLK_SYS
	int value;

	value = readl(priv->clkbase + AHB1_GATING);
	value |= GETH_AHB_BIT;
	writel(value, priv->clkbase + AHB1_GATING);

	value = readl(priv->clkbase + AHB1_MOD_RESET);
	value |= GETH_RESET_BIT;
	writel(value, priv->clkbase + AHB1_MOD_RESET);
#else
	if (priv->phy_ext == INT_PHY
			&& !IS_ERR_OR_NULL(priv->ephy_clk))
		clk_prepare_enable(priv->ephy_clk);
	clk_prepare_enable(priv->geth_clk);
#endif

	phy_interface = priv->phy_interface;

	clk_value = readl(priv->geth_extclk + GETH_CLK_REG);
	if (phy_interface == PHY_INTERFACE_MODE_RGMII)
		clk_value |= 0x00000004;
	else
		clk_value &= (~0x00000004);

	clk_value &= (~0x00002003);
	if (phy_interface == PHY_INTERFACE_MODE_RGMII
			|| phy_interface == PHY_INTERFACE_MODE_GMII)
		clk_value |= 0x00000002;
	else if (phy_interface == PHY_INTERFACE_MODE_RMII)
		clk_value |= 0x00002001;

	/* Adjust Tx/Rx clock delay */
	clk_value &= ~(0x07 << 10);
	clk_value |= ((tx_delay & 0x07) << 10);
	clk_value &= ~(0x1F << 5);
	clk_value |= ((rx_delay & 0x1F) << 5);

	writel(clk_value, priv->geth_extclk + GETH_CLK_REG);
}

static void geth_clk_disable(struct geth_priv *priv)
{
#ifndef CONFIG_GETH_CLK_SYS
	int value;

	value = readl(priv->clkbase + AHB1_GATING);
	value &= ~GETH_AHB_BIT;
	writel(value, priv->clkbase + AHB1_GATING);

	value = readl(priv->clkbase + AHB1_MOD_RESET);
	value &= ~GETH_RESET_BIT;
	writel(value, priv->clkbase + AHB1_MOD_RESET);
#else
	if (priv->phy_ext == INT_PHY)
		clk_disable_unprepare(priv->ephy_clk);
	clk_disable_unprepare(priv->geth_clk);
#endif
}

static void geth_tx_err(struct geth_priv *priv)
{
	netif_stop_queue(priv->ndev);

	sunxi_stop_tx(priv->base);

	geth_free_tx_sk(priv);
	memset(priv->dma_tx, 0, dma_desc_tx * sizeof(struct dma_desc));
	desc_init_chain(priv->dma_tx, (unsigned long)priv->dma_tx_phy, dma_desc_tx);
	priv->tx_dirty = 0;
	priv->tx_clean = 0;
	sunxi_start_tx(priv->base, priv->dma_tx_phy);

	priv->ndev->stats.tx_errors++;
	netif_wake_queue(priv->ndev);
}

static inline void geth_schedule(struct geth_priv *priv)
{
	sunxi_int_disable(priv->base);
	if(likely(napi_schedule_prep(&priv->napi)))
		__napi_schedule(&priv->napi);
}

static irqreturn_t geth_interrupt(int irq, void *dev_id)
{
	struct net_device *ndev = (struct net_device *)dev_id;
	struct geth_priv *priv = netdev_priv(ndev);
	int status;

	if (unlikely(!ndev)) {
		pr_err("%s: invalid ndev pointer\n", __func__);
		return IRQ_NONE;
	}

	status = sunxi_int_status(priv->base, (void *)(&priv->xstats));

	if (likely(status == handle_tx_rx))
		geth_schedule(priv);
	else if (unlikely(status == tx_hard_error_bump_tc)) {
		netdev_info(ndev, "Do nothing for bump tc\n");
	} else if(unlikely(status == tx_hard_error)){
		geth_tx_err(priv);
	}

	return IRQ_HANDLED;
}

static int geth_open(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);
	int ret = 0;

	ret = geth_power_on(priv);
	if (ret) {
		netdev_err(ndev, "Power on is failed\n");
		ret = -EINVAL;
	}

	geth_clk_enable(priv);

	ret = geth_phy_init(ndev);
	if (ret)
		goto err;

	ret = sunxi_mac_reset((void *)priv->base, &sunxi_udelay, 10000);
	if (ret) {
		netdev_err(ndev, "Initialize hardware error\n");
		goto desc_err;
	}

	sunxi_mac_init(priv->base, txmode, rxmode);
	sunxi_set_umac(priv->base, ndev->dev_addr, 0);

	if (!priv->is_suspend) {
		ret = geth_dma_desc_init(ndev);
		if (ret) {
			ret = -EINVAL;
			goto desc_err;
		}
	}

	memset(priv->dma_tx, 0, dma_desc_tx * sizeof(struct dma_desc));
	memset(priv->dma_rx, 0, dma_desc_rx * sizeof(struct dma_desc));

	desc_init_chain(priv->dma_rx, (unsigned long)priv->dma_rx_phy, dma_desc_rx);
	desc_init_chain(priv->dma_tx, (unsigned long)priv->dma_tx_phy, dma_desc_tx);

	priv->rx_clean = priv->rx_dirty = 0;
	priv->tx_clean = priv->tx_dirty = 0;
	geth_rx_refill(ndev);

	/* Extra statistics */
	memset(&priv->xstats, 0, sizeof(struct geth_extra_stats));

	if (ndev->phydev)
		phy_start(ndev->phydev);

	sunxi_start_rx(priv->base, (unsigned long)((struct dma_desc *)
				priv->dma_rx_phy + priv->rx_dirty));
	sunxi_start_tx(priv->base, (unsigned long)((struct dma_desc *)
				priv->dma_tx_phy + priv->tx_clean));

	napi_enable(&priv->napi);
	netif_start_queue(ndev);

	/* Enable the Rx/Tx */
	sunxi_mac_enable(priv->base);

	return 0;

desc_err:
	geth_phy_release(ndev);
err:
	geth_clk_disable(priv);
	if (priv->is_suspend)
		napi_enable(&priv->napi);

	return ret;
}

static int geth_stop(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);

	netif_stop_queue(ndev);
	napi_disable(&priv->napi);

	netif_carrier_off(ndev);

	/* Release PHY resources */
	geth_phy_release(ndev);

	/* Disable Rx/Tx */
	sunxi_mac_disable(priv->base);

	geth_clk_disable(priv);
	geth_power_off(priv);

	netif_tx_lock_bh(ndev);
	/* Release the DMA TX/RX socket buffers */
	geth_free_rx_sk(priv);
	geth_free_tx_sk(priv);
	netif_tx_unlock_bh(ndev);

	/* Ensure that hareware have been stopped */
	if (!priv->is_suspend)
		geth_free_dma_desc(priv);

	return 0;
}

static void geth_tx_complete(struct geth_priv *priv)
{
	unsigned int entry = 0;
	struct sk_buff *skb = NULL;
	struct dma_desc *desc = NULL;
	int tx_stat;

	spin_lock(&priv->tx_lock);
	while (circ_cnt(priv->tx_dirty, priv->tx_clean, dma_desc_tx) > 0) {

		entry = priv->tx_clean;
		desc = priv->dma_tx + entry;

		/* Check if the descriptor is owned by the DMA. */
		if (desc_get_own(desc))
			break;

		/* Verify tx error by looking at the last segment */
		if (desc_get_tx_ls(desc)) {
			tx_stat = desc_get_tx_status(desc, (void *)(&priv->xstats));

			if (likely(!tx_stat))
				priv->ndev->stats.tx_packets++;
			else
				priv->ndev->stats.tx_errors++;
		}

		dma_unmap_single(priv->dev, desc_buf_get_addr(desc),
				desc_buf_get_len(desc), DMA_TO_DEVICE);

		skb = priv->tx_sk[entry];
		priv->tx_sk[entry] = NULL;
		desc_init(desc);

		/* Find next dirty desc */
		priv->tx_clean = circ_inc(entry, dma_desc_tx);

		if (unlikely(skb == NULL))
			continue;

		dev_kfree_skb(skb);
	}

	if (unlikely(netif_queue_stopped(priv->ndev)) &&
		circ_space(priv->tx_dirty, priv->tx_clean, dma_desc_tx) >
			TX_THRESH) {
		netif_wake_queue(priv->ndev);
	}
	spin_unlock(&priv->tx_lock);
}

static netdev_tx_t geth_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	struct geth_priv  *priv = netdev_priv(ndev);
	unsigned int entry;
	struct dma_desc *desc, *first;
	unsigned int len, tmp_len = 0;
	int i, csum_insert;
	int nfrags = skb_shinfo(skb)->nr_frags;
	dma_addr_t paddr;

	spin_lock(&priv->tx_lock);
	if (unlikely(circ_space(priv->tx_dirty, priv->tx_clean,
			dma_desc_tx) < (nfrags + 1))) {

		if (!netif_queue_stopped(ndev)) {
			netdev_err(ndev, "%s: BUG! Tx Ring full when queue awake\n", __func__);
			netif_stop_queue(ndev);
		}
		spin_unlock(&priv->tx_lock);

		return NETDEV_TX_BUSY;
	}


	csum_insert = (skb->ip_summed == CHECKSUM_PARTIAL);
	entry = priv->tx_dirty;
	first = desc = priv->dma_tx + entry;

	len = skb_headlen(skb);
	priv->tx_sk[entry] = skb;

#ifdef PKT_DEBUG
	printk("======TX PKT DATA: ============\n");
	/* dump the packet */
	print_hex_dump(KERN_DEBUG, "skb->data: ", DUMP_PREFIX_NONE,
			16, 1, skb->data, 64, true);
#endif

	/* Every desc max size is 2K */
	while (len != 0) {
		desc = priv->dma_tx + entry;
		tmp_len = ((len > MAX_BUF_SZ) ?  MAX_BUF_SZ : len);

		paddr = dma_map_single(priv->dev, skb->data, tmp_len, DMA_TO_DEVICE);
		if (dma_mapping_error(priv->dev, paddr)){
			dev_kfree_skb(skb);
			return -EIO;
		}
		desc_buf_set(desc, paddr, tmp_len);
		/* Don't set the first's own bit, here */
		if (first != desc) {
			priv->tx_sk[entry] = NULL;
			desc_set_own(desc);
		}

		entry = circ_inc(entry, dma_desc_tx);
		len -= tmp_len;
	}

	for (i = 0; i <nfrags; i++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		len = skb_frag_size(frag);

		desc = priv->dma_tx + entry;
		paddr = skb_frag_dma_map(priv->dev, frag, 0, len, DMA_TO_DEVICE);
		if (dma_mapping_error(priv->dev, paddr)) {
			dev_kfree_skb(skb);
			return -EIO;
		}

		desc_buf_set(desc, paddr, len);
		desc_set_own(desc);
		priv->tx_sk[entry] = NULL;
		entry = circ_inc(entry, dma_desc_tx);
	}

	ndev->stats.tx_bytes += skb->len;
	priv->tx_dirty = entry;
	desc_tx_close(first, desc, csum_insert);

	desc_set_own(first);
	spin_unlock(&priv->tx_lock);

	if (circ_space(priv->tx_dirty, priv->tx_clean, dma_desc_tx) <=
			(MAX_SKB_FRAGS + 1)) {
		netif_stop_queue(ndev);
		if (circ_space(priv->tx_dirty, priv->tx_clean, dma_desc_tx) >
				TX_THRESH)
			netif_wake_queue(ndev);
	}

#ifdef DEBUG
	printk("=======TX Descriptor DMA: 0x%08llx\n", priv->dma_tx_phy);
	printk("Tx pointor: dirty: %d, clean: %d\n", priv->tx_dirty, priv->tx_clean);
	desc_print(priv->dma_tx, dma_desc_tx);
#endif
	sunxi_tx_poll(priv->base);
	geth_tx_complete(priv);

	return NETDEV_TX_OK;
}

static int geth_rx(struct geth_priv *priv, int limit)
{
	unsigned int rxcount = 0;
	unsigned int entry;
	struct dma_desc *desc;
	struct sk_buff *skb;
	int status;
	int frame_len;

	while (rxcount < limit) {

		entry = priv->rx_dirty;
		desc = priv->dma_rx + entry;

		if (desc_get_own(desc))
			break;

		rxcount++;
		priv->rx_dirty = circ_inc(priv->rx_dirty, dma_desc_rx);

		/* Get lenght & status from hardware */
		frame_len = desc_rx_frame_len(desc);
		status = desc_get_rx_status(desc, (void *)(&priv->xstats));

		netdev_dbg(priv->ndev, "Rx frame size %d, status: %d\n",
				frame_len, status);

		skb = priv->rx_sk[entry];
		if (unlikely(!skb)){
			netdev_err(priv->ndev, "Skb is null\n");
			priv->ndev->stats.rx_dropped++;
			break;
		}

#ifdef PKT_DEBUG
		printk("======RX PKT DATA: ============\n");
		/* dump the packet */
		print_hex_dump(KERN_DEBUG, "skb->data: ", DUMP_PREFIX_NONE,
				16, 1, skb->data, 64, true);
#endif

		if (status == discard_frame){
			netdev_dbg(priv->ndev, "Get error pkt\n");
			priv->ndev->stats.rx_errors++;
			continue;
		}

		if (unlikely(status != llc_snap))
			frame_len -= ETH_FCS_LEN;

		priv->rx_sk[entry] = NULL;

		skb_put(skb, frame_len);
		dma_unmap_single(priv->dev, desc_buf_get_addr(desc),
				desc_buf_get_len(desc), DMA_FROM_DEVICE);

		skb->protocol = eth_type_trans(skb, priv->ndev);

		skb->ip_summed = CHECKSUM_UNNECESSARY;
		napi_gro_receive(&priv->napi, skb);

		priv->ndev->stats.rx_packets++;
		priv->ndev->stats.rx_bytes += frame_len;
	}

#ifdef DEBUG
	if (rxcount > 0) {
		printk("======RX Descriptor DMA: 0x%08llx=\n", priv->dma_rx_phy);
		printk("RX pointor: dirty: %d, clean: %d\n", priv->rx_dirty, priv->rx_clean);
		desc_print(priv->dma_rx, dma_desc_rx);
	}
#endif
	geth_rx_refill(priv->ndev);

	return rxcount;
}

static int geth_poll(struct napi_struct *napi, int budget)
{
	struct geth_priv *priv = container_of(napi, struct geth_priv, napi);
	int work_done = 0;

	geth_tx_complete(priv);
	work_done = geth_rx(priv, budget);

	if (work_done < budget) {
		napi_complete(napi);
		sunxi_int_enable(priv->base);
	}

	return work_done;
}

static int geth_change_mtu(struct net_device *ndev, int new_mtu)
{
	int max_mtu;

	if (netif_running(ndev)) {
		pr_err("%s: must be stopped to change its MTU\n", ndev->name);
		return -EBUSY;
	}

	max_mtu = SKB_MAX_HEAD(NET_SKB_PAD + NET_IP_ALIGN);

	if ((new_mtu < 46) || (new_mtu > max_mtu)) {
		pr_err("%s: invalid MTU, max MTU is: %d\n", ndev->name, max_mtu);
		return -EINVAL;
	}

	ndev->mtu = new_mtu;
	netdev_update_features(ndev);

	return 0;
}

static netdev_features_t geth_fix_features(struct net_device *ndev,
	netdev_features_t features)
{
	return features;
}

static void geth_set_rx_mode(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);
	unsigned int value = 0;

	pr_debug(KERN_INFO "%s: # mcasts %d, # unicast %d\n",
		 __func__, netdev_mc_count(ndev), netdev_uc_count(ndev));

	spin_lock(&priv->lock);
	if (ndev->flags & IFF_PROMISC)
		value = GETH_FRAME_FILTER_PR;
	else if ((netdev_mc_count(ndev) > HASH_TABLE_SIZE)
		   || (ndev->flags & IFF_ALLMULTI)) {
		value = GETH_FRAME_FILTER_PM;	/* pass all multi */
		sunxi_hash_filter(priv->base, ~0UL, ~0UL);
	} else if (!netdev_mc_empty(ndev)) {
		u32 mc_filter[2];
		struct netdev_hw_addr *ha;

		/* Hash filter for multicast */
		value = GETH_FRAME_FILTER_HMC;

		memset(mc_filter, 0, sizeof(mc_filter));
		netdev_for_each_mc_addr(ha, ndev) {
			/* The upper 6 bits of the calculated CRC are used to
			   index the contens of the hash table */
			int bit_nr =
			    bitrev32(~crc32_le(~0, ha->addr, 6)) >> 26;
			/* The most significant bit determines the register to
			 * use (H/L) while the other 5 bits determine the bit
			 * within the register. */
			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
		}
		sunxi_hash_filter(priv->base, mc_filter[0], mc_filter[1]);
	}

	/* Handle multiple unicast addresses (perfect filtering)*/
	if (netdev_uc_count(ndev) > 16)
		/* Switch to promiscuous mode is more than 8 addrs
		   are required */
		value |= GETH_FRAME_FILTER_PR;
	else {
		int reg = 1;
		struct netdev_hw_addr *ha;

		netdev_for_each_uc_addr(ha, ndev) {
			sunxi_set_umac(priv->base, ha->addr, reg);
			reg++;
		}
	}

#ifdef FRAME_FILTER_DEBUG
	/* Enable Receive all mode (to debug filtering_fail errors) */
	value |= GETH_FRAME_FILTER_RA;
#endif
	sunxi_set_filter(priv->base, value);
	spin_unlock(&priv->lock);
}

static void geth_tx_timeout(struct net_device *ndev)
{
	struct geth_priv *priv = netdev_priv(ndev);

	geth_tx_err(priv);
}

static int geth_ioctl(struct net_device *ndev, struct ifreq *rq, int cmd)
{
	if (!netif_running(ndev))
		return -EINVAL;

	if (!ndev->phydev)
		return -EINVAL;

	return phy_mii_ioctl(ndev->phydev, rq, cmd);
}

/* Configuration changes (passed on by ifconfig) */
static int geth_config(struct net_device *ndev, struct ifmap *map)
{
	if (ndev->flags & IFF_UP)	/* can't act on a running interface */
		return -EBUSY;

	/* Don't allow changing the I/O address */
	if (map->base_addr != ndev->base_addr) {
		printk(KERN_WARNING "%s: can't change I/O address\n",
			ndev->name);
		return -EOPNOTSUPP;
	}

	/* Don't allow changing the IRQ */
	if (map->irq != ndev->irq) {
		printk(KERN_WARNING "%s: can't change IRQ number %d\n",
		       ndev->name, ndev->irq);
		return -EOPNOTSUPP;
	}

	return 0;
}

static int geth_set_mac_address(struct net_device *ndev, void *p)
{
	struct geth_priv *priv = netdev_priv(ndev);
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(ndev->dev_addr, addr->sa_data, ndev->addr_len);
	sunxi_set_umac(priv->base, ndev->dev_addr, 0);

	return 0;
}

int geth_set_features(struct net_device *ndev, netdev_features_t features)
{
	struct geth_priv *priv = netdev_priv(ndev);

	if (features & NETIF_F_LOOPBACK && netif_running(ndev))
		sunxi_mac_loopback(priv->base, 1);
	else
		sunxi_mac_loopback(priv->base, 0);

	return 0;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/* Polling receive - used by NETCONSOLE and other diagnostic tools
 * to allow network I/O with interrupts disabled. */
static void geth_poll_controller(struct net_device *dev)
{
	disable_irq(dev->irq);
	geth_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif


static const struct net_device_ops geth_netdev_ops = {
	.ndo_init = NULL,
	.ndo_open = geth_open,
	.ndo_start_xmit = geth_xmit,
	.ndo_stop = geth_stop,
	.ndo_change_mtu = geth_change_mtu,
	.ndo_fix_features = geth_fix_features,
	.ndo_set_rx_mode = geth_set_rx_mode,
	.ndo_tx_timeout = geth_tx_timeout,
	.ndo_do_ioctl = geth_ioctl,
	.ndo_set_config = geth_config,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = geth_poll_controller,
#endif
	.ndo_set_mac_address = geth_set_mac_address,
	.ndo_set_features = geth_set_features,
};

/*****************************************************************************
 *
 *
 ****************************************************************************/
static int geth_check_if_running(struct net_device *ndev)
{
	if (!netif_running(ndev))
		return -EBUSY;
	return 0;
}

static int geth_get_sset_count(struct net_device *netdev, int sset)
{
	int len;

	switch (sset) {
	case ETH_SS_STATS:
		len = 0;
		return len;
	default:
		return -EOPNOTSUPP;
	}
}

static int geth_ethtool_getsettings(struct net_device *ndev,
				      struct ethtool_cmd *cmd)
{
	struct geth_priv *priv = netdev_priv(ndev);
	struct phy_device *phy = ndev->phydev;
	int rc;

	if (phy == NULL) {
		netdev_err(ndev, "%s: %s: PHY is not registered\n",
		       __func__, ndev->name);
		return -ENODEV;
	}

	if (!netif_running(ndev)) {
		pr_err("%s: interface is disabled: we cannot track "
		"link speed / duplex setting\n", ndev->name);
		return -EBUSY;
	}

	cmd->transceiver = XCVR_INTERNAL;
	spin_lock_irq(&priv->lock);
	rc = phy_ethtool_gset(phy, cmd);
	spin_unlock_irq(&priv->lock);

	return rc;
}

static int geth_ethtool_setsettings(struct net_device *ndev,
				      struct ethtool_cmd *cmd)
{
	struct geth_priv *priv = netdev_priv(ndev);
	struct phy_device *phy = ndev->phydev;
	int rc;

	spin_lock(&priv->lock);
	rc = phy_ethtool_sset(phy, cmd);
	spin_unlock(&priv->lock);

	return rc;
}

static void geth_ethtool_getdrvinfo(struct net_device *ndev,
				      struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, "sunxi_geth", sizeof(info->driver));

#define DRV_MODULE_VERSION "SUNXI Gbgit driver V1.1"

	strcpy(info->version, DRV_MODULE_VERSION);
	info->fw_version[0] = '\0';
}

static const struct ethtool_ops geth_ethtool_ops = {
	.begin = geth_check_if_running,
	.get_settings = geth_ethtool_getsettings,
	.set_settings = geth_ethtool_setsettings,
	.get_link = ethtool_op_get_link,
	.get_pauseparam = NULL,
	.set_pauseparam = NULL,
	.get_ethtool_stats = NULL,
	.get_strings = NULL,
	.get_wol = NULL,
	.set_wol = NULL,
	.get_sset_count = geth_get_sset_count,
	.get_drvinfo = geth_ethtool_getdrvinfo,
};


/*****************************************************************************
 *
 *
 ****************************************************************************/
static int geth_script_parse(struct platform_device *pdev)
{
#ifdef CONFIG_GETH_SCRIPT_SYS
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct geth_priv *priv = netdev_priv(ndev);
	struct device_node *np = pdev->dev.of_node;
	u32 value;
#ifdef CONFIG_GETH_PHY_POWER
	int ret;
	char power[20];
	int cnt;
#endif

	priv->phy_ext = EXT_PHY;

#ifdef CONFIG_GETH_PHY_POWER
	memset(power_tb, 0, sizeof(power_tb));
	for (cnt = 0; cnt < ARRAY_SIZE(power_tb); cnt++) {
		char *vol;
		const char *ptr;
		size_t len;
		snprintf(power, 15, "gmac_power%u", (cnt+1));
		ret = of_property_read_string(np, power, &ptr);
		if(ret)
			continue;

		/* Power format: \w\+:[0-9]\+ */
		len = strlen((char *)ptr);
		vol = strnchr((const char *)ptr, len, ':');
		if (vol) {
			len = (size_t)(vol - ptr);
			power_tb[cnt].vol = simple_strtoul(++vol, NULL, 0);
		}

		power_tb[cnt].name = kstrndup((char *)ptr, len, GFP_KERNEL);
	}
#endif

#if 0
	/* Default mode is PHY_INTERFACE_MODE_RGMII */
	priv->phy_interface = PHY_INTERFACE_MODE_RGMII;
	type = script_get_item((char *)dev_name(&pdev->dev), "gmac_mode", &item);
	if (SCIRPT_ITEM_VALUE_TYPE_STR == type) {
		if (!strncasecmp((char *)item.val, "MII", 3))
			priv->phy_interface = PHY_INTERFACE_MODE_MII;
		else if(!strncasecmp((char *)item.val, "GMII", 4))
			priv->phy_interface = PHY_INTERFACE_MODE_GMII;
		else if(!strncasecmp((char *)item.val, "RMII", 4))
			priv->phy_interface = PHY_INTERFACE_MODE_RMII;
	}
#endif
	priv->phy_interface = of_get_phy_mode(np);
	if (priv->phy_interface != PHY_INTERFACE_MODE_MII
			&& priv->phy_interface != PHY_INTERFACE_MODE_RGMII
			&& priv->phy_interface != PHY_INTERFACE_MODE_RMII) {
		dev_err(&pdev->dev, "Not support phy type!\n");
		priv->phy_interface = PHY_INTERFACE_MODE_MII;
	}

	if (priv->phy_ext == INT_PHY)
		priv->phy_interface = PHY_INTERFACE_MODE_MII;
#endif
	if(!of_property_read_u32(np, "tx-delay", &value))
		tx_delay = value;

	if(!of_property_read_u32(np, "rx-delay", &value))
		rx_delay = value;

	return 0;
}

static int geth_sys_request(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct geth_priv *priv = netdev_priv(ndev);
	struct device_node *np = pdev->dev.of_node;
	int ret = 0;
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (unlikely(!res)){
		ret = -ENODEV;
		printk(KERN_ERR "Failed to get gmac clk reg!\n");
		goto out;
	}

	priv->geth_extclk = devm_ioremap_resource(&pdev->dev, res);
	if (unlikely(!priv->geth_extclk)) {
		ret = -ENOMEM;
		printk(KERN_ERR "Failed to ioremap the address of gmac register\n");
		goto out;
	}

#ifndef CONFIG_GETH_CLK_SYS
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "geth_clk");
	if (unlikely(!res)){
		ret = -ENODEV;
		printk(KERN_ERR "Failed to get gmac clk bus!\n");
		goto clk_err;
	}
	priv->clkbase = devm_ioremap_resource(&pdev->dev, res);
	if (unlikely(!priv->clkbase)) {
		ret = -ENOMEM;
		goto clk_err;
	}
#else
	priv->geth_clk = of_clk_get_by_name(np, GMAC_CLK);
	if (unlikely(!priv->geth_clk || IS_ERR(priv->geth_clk))) {
		printk(KERN_ERR "ERROR: Get clock is failed!\n");
		ret = -EINVAL;
		goto out;
	}

	priv->ephy_clk = of_clk_get_by_name(np, EPHY_CLK);
	if (unlikely(!priv->ephy_clk || IS_ERR(priv->ephy_clk))) {
		printk(KERN_WARNING "WARNING: Get ephy clock is failed\n");
		priv->ephy_clk = NULL;
		priv->phy_ext = EXT_PHY;
	}
#endif

#ifndef CONFIG_GETH_SCRIPT_SYS
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "geth_pio");
	if (unlikely(!res)){
		ret = -ENODEV;
		goto pin_err;
	}

	priv->gpiobase = devm_ioremap_resource(&pdev->dev, res);
	if (unlikely(!priv->gpiobase)) {
		printk(KERN_ERR "%s: ERROR: memory mapping failed", __func__);
		ret = -ENOMEM;
		goto pin_err;
	}
	writel(0x22222222, priv->gpiobase + PA_CFG0);
	writel(0x22222222, priv->gpiobase + PA_CFG1);
	writel(0x00000022 |
		((readl(priv->gpiobase + PA_CFG2) >> 8) << 8),
		priv->gpiobase + PA_CFG2);
#else
	priv->pinctrl = devm_pinctrl_get_select_default(&pdev->dev);
	if (IS_ERR_OR_NULL(priv->pinctrl)) {
		printk(KERN_WARNING "Gmac: devm_pinctrl is failed\n");
		priv->pinctrl = NULL;
		priv->phy_ext = INT_PHY;
	}
#endif
	return 0;

#ifndef CONFIG_GETH_CLK_SYS
pin_err:
	devm_iounmap(&pdev->dev,(priv->clkbase));
clk_err:
#endif
	devm_iounmap(&pdev->dev,(priv->geth_extclk));
out:
	return ret;
}

static void geth_sys_release(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct geth_priv *priv = netdev_priv(ndev);

#ifndef CONFIG_GETH_SCRIPT_SYS
	devm_iounmap(&pdev->dev,((void *)priv->gpiobase));
#else
	if (!IS_ERR_OR_NULL(priv->pinctrl))
		devm_pinctrl_put(priv->pinctrl);
#endif

	devm_iounmap(&pdev->dev,(priv->geth_extclk));

#ifndef CONFIG_GETH_CLK_SYS
	devm_iounmap(&pdev->dev,((void *)priv->clkbase));
#else
	if (priv->phy_ext == INT_PHY && priv->ephy_clk)
		clk_put(priv->ephy_clk);

	if (priv->geth_clk)
		clk_put(priv->geth_clk);
#endif
}

/**
 * geth_probe
 * @pdev: platform device pointer
 * Description: the driver is initialized through platform_device.
 */
static int geth_probe(struct platform_device *pdev)
{
	int ret = 0;
	int irq = 0;
	struct resource *res;
	struct net_device *ndev = NULL;
	struct geth_priv *priv;

#ifdef CONFIG_OF
	pdev->dev.dma_mask = &geth_dma_mask;
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
#endif

	ndev = alloc_etherdev(sizeof(struct geth_priv));
	if (!ndev) {
		dev_err(&pdev->dev, "could not allocate device.\n");
		return -ENOMEM;
	}
	SET_NETDEV_DEV(ndev, &pdev->dev);

	priv = netdev_priv(ndev);
	platform_set_drvdata(pdev, ndev);

	/* Must set private data to pdev, before call it */
	ret = geth_script_parse(pdev);
	if (ret)
		goto out_err;

	ret = geth_sys_request(pdev);
	if (ret)
		goto out_err;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret =  -ENODEV;
		goto map_err;
	}

	priv->base = devm_ioremap_resource(&pdev->dev, res);
	if (!priv->base) {
		pr_err("%s: ERROR: memory mapping failed", __func__);
		ret = -ENOMEM;
		goto map_err;
	}

	/* Get the MAC information */
	irq = platform_get_irq_byname(pdev, "gmacirq");
	if (irq == -ENXIO) {
		printk(KERN_ERR "%s: ERROR: MAC IRQ configuration "
		       "information not found\n", __func__);
		ret = -ENXIO;
		goto irq_err;
	}
	ret = request_irq(irq, geth_interrupt, IRQF_SHARED,
			dev_name(&pdev->dev), ndev);
	if (unlikely(ret < 0)) {
		netdev_err(ndev, "Could not request irq %d, error: %d\n",
				ndev->irq, ret);
		goto irq_err;
	}

	/* setup the netdevice, fill the field of netdevice */
	ether_setup(ndev);
	ndev->netdev_ops = &geth_netdev_ops;
	SET_ETHTOOL_OPS(ndev, &geth_ethtool_ops);
	ndev->base_addr = (unsigned long)priv->base;
	ndev->irq = irq;

	priv->ndev = ndev;
	priv->dev = &pdev->dev;

	/* TODO: support the VLAN frames */
	ndev->hw_features = NETIF_F_SG | NETIF_F_HIGHDMA | NETIF_F_IP_CSUM |
				NETIF_F_IPV6_CSUM | NETIF_F_RXCSUM;

	ndev->features |= ndev->hw_features;
	ndev->hw_features |= NETIF_F_LOOPBACK;
	ndev->priv_flags |= IFF_UNICAST_FLT;

	ndev->watchdog_timeo = msecs_to_jiffies(watchdog);

	netif_napi_add(ndev, &priv->napi, geth_poll,  BUDGET);

	spin_lock_init(&priv->lock);
	spin_lock_init(&priv->tx_lock);

	/* The last val is mdc clock ratio */
	sunxi_geth_register((void *)ndev->base_addr, HW_VERSION, 0x03);

	ret = register_netdev(ndev);
	if (ret) {
		netif_napi_del(&priv->napi);
		printk(KERN_ERR "Error: Register %s failed\n", ndev->name);
		goto reg_err;
	}

	/* Before open the device, the mac address is be set */
	geth_check_addr(ndev, mac_str);

	return 0;

reg_err:
	free_irq(irq, ndev);
irq_err:
	devm_iounmap(&pdev->dev, priv->base);
map_err:
	geth_sys_release(pdev);
out_err:
	platform_set_drvdata(pdev, NULL);
	free_netdev(ndev);

	return ret;
}

static int geth_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct geth_priv *priv = netdev_priv(ndev);
#if defined(CONFIG_GETH_PHY_POWER)
	int i;

	for (i=0; i < ARRAY_SIZE(power_tb); i++) {
		if (power_tb[i].name)
			kfree(power_tb[i].name);
	}
#endif

	netif_napi_del(&priv->napi);
	unregister_netdev(ndev);

	devm_iounmap(&pdev->dev,(priv->base));
	free_irq(ndev->irq, ndev);

	geth_sys_release(pdev);

	platform_set_drvdata(pdev, NULL);
	free_netdev(ndev);

	return 0;
}

static const struct of_device_id geth_of_match[] = {
	{.compatible = "allwinner,sunxi-gmac",},
	{},
};
MODULE_DEVICE_TABLE(of, geth_of_match);

static struct platform_driver geth_driver = {
	.probe	= geth_probe,
	.remove = geth_remove,
	.driver = {
		   .name = "sunxi-gmac",
		   .owner = THIS_MODULE,
		   .pm = &geth_pm_ops,
		   .of_match_table = geth_of_match,
	},
};
module_platform_driver(geth_driver);

#ifndef CONFIG_OF
static struct resource geth_resources[] = {
	{
		.name	= "geth_io",
		.start	= GETH_BASE,
		.end	= GETH_BASE + 0x1054,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "geth_extclk",
		.start	= SYS_CTL_BASE,
		.end	= SYS_CTL_BASE + GETH_CLK_REG,
		.flags	= IORESOURCE_MEM,
	},
#ifndef CONFIG_GETH_CLK_SYS
	{
		.name	= "geth_clk",
		.start	= CCMU_BASE,
		.end	= CCMU_BASE + 1024,
		.flags	= IORESOURCE_MEM,
	},
#endif
#ifndef CONFIG_GETH_SCRIPT_SYS
	{
		.name	= "geth_pio",
		.start	= GPIO_BASE,
		.end	= GPIO_BASE + 0x200,
		.flags	= IORESOURCE_MEM,
	},
#endif
	{
		.name	= "gmacirq",
		.start	= SUNXI_IRQ_GMAC,
		.end	= SUNXI_IRQ_GMAC,
		.flags	= IORESOURCE_IRQ,
	}
};

static void geth_device_release(struct device *dev)
{
}

static struct platform_device geth_device = {
	.name = "gmac0",
	.id = -1,
	.resource = geth_resources,
	.num_resources = ARRAY_SIZE(geth_resources),
	.dev = {
		.release = geth_device_release,
		.platform_data = NULL,
		.dma_mask = &geth_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

static int __init geth_init(void)
{
	int ret;

	ret = platform_device_register(&geth_device);
	if (ret)
		return ret;

	return platform_driver_register(&geth_driver);
}

static void __exit geth_exit(void)
{
	platform_driver_unregister(&geth_driver);
	platform_device_unregister(&geth_device);
}
#endif

#ifndef MODULE
static int __init set_mac_addr(char *str)
{
	char *p = str;

	if (str != NULL && strlen(str))
		memcpy(mac_str, p, 18);

	return 0;
}
__setup("mac_addr=", set_mac_addr);
#endif

MODULE_DESCRIPTION("Allwinner Gigabit Ethernet driver");
MODULE_AUTHOR("Sugar <shugeLinux@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");
