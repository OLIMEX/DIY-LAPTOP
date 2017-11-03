/*
 * drivers/usb/host/sunxi_hci.c
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * yangnaitian, 2011-5-24, create this file
 * javen, 2011-7-18, add clock and power switch
 *
 * sunxi HCI Driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/dma-mapping.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <linux/regulator/consumer.h>
#include  <linux/of.h>
#include  <linux/of_address.h>
#include  <linux/of_device.h>

#include  "sunxi_hci.h"

static u64 sunxi_hci_dmamask = DMA_BIT_MASK(32);
static DEFINE_MUTEX(usb_passby_lock);
static DEFINE_MUTEX(usb_vbus_lock);
static DEFINE_MUTEX(usb_clock_lock);

#ifndef CONFIG_OF
static char* usbc_name[2] 			= {"usbc0", "usbc1"};
#endif

#ifdef	CONFIG_USB_SUNXI_USB_MANAGER
int usb_otg_id_status(void);
#endif

static struct sunxi_hci_hcd sunxi_ohci0;
static struct sunxi_hci_hcd sunxi_ohci1;
static struct sunxi_hci_hcd sunxi_ehci0;
static struct sunxi_hci_hcd sunxi_ehci1;

#define  USBPHYC_REG_o_PHYCTL		    0x0404

atomic_t usb1_set_vbus_cnt;
atomic_t usb2_set_vbus_cnt;

atomic_t usb1_enable_passly_cnt;
atomic_t usb2_enable_passly_cnt;

static s32 request_usb_regulator_io(struct sunxi_hci_hcd *sunxi_hci)
{

	if(sunxi_hci->regulator_io != NULL){
		sunxi_hci->regulator_io_hdle = regulator_get(NULL, sunxi_hci->regulator_io);
		if(IS_ERR(sunxi_hci->regulator_io_hdle)) {
			DMSG_PANIC("ERR: some error happen, %s,regulator_io_hdle fail to get regulator!", sunxi_hci->hci_name);
			sunxi_hci->regulator_io_hdle = NULL;
			return 0;
		}
	}

	if(sunxi_hci->hsic_flag){
		if(sunxi_hci->hsic_regulator_io != NULL){
			sunxi_hci->hsic_regulator_io_hdle = regulator_get(NULL, sunxi_hci->hsic_regulator_io);
			if(IS_ERR(sunxi_hci->hsic_regulator_io_hdle)) {
				DMSG_PANIC("ERR: some error happen, %s, hsic_regulator_io_hdle fail to get regulator!", sunxi_hci->hci_name);
				sunxi_hci->hsic_regulator_io_hdle = NULL;
				return 0;
			}
		}
	}

	return 0;
}

static s32 release_usb_regulator_io(struct sunxi_hci_hcd *sunxi_hci)
{

	if(sunxi_hci->regulator_io != NULL){
		regulator_put(sunxi_hci->regulator_io_hdle);
	}

	if(sunxi_hci->hsic_flag){
		if(sunxi_hci->hsic_regulator_io != NULL){
			regulator_put(sunxi_hci->hsic_regulator_io_hdle);
		}
	}

	return 0;
}

void __iomem *usb_phy_csr_add(struct sunxi_hci_hcd *sunxi_hci)
{
	return (sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CTRL);
}

void __iomem *usb_phy_csr_read(struct sunxi_hci_hcd *sunxi_hci)
{

	switch(sunxi_hci->usbc_no)
	{
		case 0:
			return (sunxi_hci->otg_vbase + SUNXI_OTG_PHY_STATUS);
		break;

		case 1:
			return (sunxi_hci->usb_vbase + SUNXI_HCI_UTMI_PHY_STATUS);
		break;

		default:

			DMSG_PANIC("usb_phy_csr_write is fial in %d index\n", sunxi_hci->usbc_no);
		break;
	}

	return NULL;
}

void __iomem *usb_phy_csr_write(struct sunxi_hci_hcd *sunxi_hci)
{
	switch(sunxi_hci->usbc_no)
	{
		case 0:
			return sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CTRL;
		break;

		case 1:
			return sunxi_hci->usb_vbase + SUNXI_HCI_PHY_CTRL;
		break;

		default:
			DMSG_PANIC("usb_phy_csr_write is fial in %d index\n", sunxi_hci->usbc_no);
		break;
	}

	return NULL;
}

int usb_phyx_tp_write(struct sunxi_hci_hcd *sunxi_hci, int addr, int data, int len)
{
	int temp = 0;
	int j = 0;
	int reg_value = 0;
	int reg_temp = 0;
	int dtmp = 0;

	if(sunxi_hci->otg_vbase == NULL){
		printk("%s,otg_vbase is null\n", __func__);
		return -1;
	}

	if(usb_phy_csr_add(sunxi_hci) == NULL){
		printk("%s,phy_csr_add is null\n", __func__);
		return -1;
	}

	if(usb_phy_csr_write(sunxi_hci) == NULL){

		printk("%s,phy_csr_write is null\n", __func__);
		return -1;
	}

	reg_value = USBC_Readl(sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG);
	reg_temp = reg_value;
	reg_value |= 0x01;
	USBC_Writel(reg_value, (sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG));

	dtmp = data;
	for(j = 0; j < len; j++)
	{

		USBC_Writeb(addr + j, usb_phy_csr_add(sunxi_hci) + 1);

		temp = USBC_Readb(usb_phy_csr_write(sunxi_hci));
		temp &= ~(0x1 << 0);
		USBC_Writeb(temp, usb_phy_csr_write(sunxi_hci));

		temp = USBC_Readb(usb_phy_csr_add(sunxi_hci));
		temp &= ~(0x1 << 7);
		temp |= (dtmp & 0x1) << 7;
		USBC_Writeb(temp, usb_phy_csr_add(sunxi_hci));

		temp = USBC_Readb(usb_phy_csr_write(sunxi_hci));
		temp |= (0x1 << 0);
		USBC_Writeb(temp, usb_phy_csr_write(sunxi_hci));

		temp = USBC_Readb(usb_phy_csr_write(sunxi_hci));
		temp &= ~(0x1 << 0);
		USBC_Writeb(temp, usb_phy_csr_write(sunxi_hci));

		dtmp >>= 1;
	}

	USBC_Writel(reg_temp, (sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG));
	return 0;
}

int usb_phyx_tp_read(struct sunxi_hci_hcd *sunxi_hci, int addr, int len)
{
	int temp = 0;
	int i = 0;
	int j = 0;
	int ret = 0;
	int reg_value = 0;
	int reg_temp = 0;

	if(sunxi_hci->otg_vbase== NULL){
		printk("%s,otg_vbase is null\n", __func__);
		return -1;
	}

	if(usb_phy_csr_add(sunxi_hci) == NULL){

		printk("%s,phy_csr_add is null\n", __func__);
		return -1;
	}

	if(usb_phy_csr_read(sunxi_hci) == NULL){

		printk("%s,phy_csr_read is null\n", __func__);
		return -1;
	}

	reg_value = USBC_Readl(sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG);
	reg_temp = reg_value;
	reg_value |= 0x01;
	USBC_Writel(reg_value, (sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG));

	for(j = len; j > 0; j--)
	{

		USBC_Writeb((addr + j - 1), usb_phy_csr_add(sunxi_hci) + 1);

		for(i = 0;i < 0x4;i++);

		temp = USBC_Readb(usb_phy_csr_read(sunxi_hci));
		ret <<= 1;
		ret |= (temp & 0x1);
	}

	USBC_Writel(reg_temp, (sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG));

	return ret;
}

static void USBC_SelectPhyToHci(struct sunxi_hci_hcd *sunxi_hci)
{
	int reg_value = 0;
	reg_value = USBC_Readl(sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG);
	reg_value &= ~(0x01);
	USBC_Writel(reg_value, (sunxi_hci->otg_vbase + SUNXI_OTG_PHY_CFG));

	return;
}

static void USBC_Clean_SIDDP(struct sunxi_hci_hcd *sunxi_hci)
{
	int reg_value = 0;
	reg_value = USBC_Readl(sunxi_hci->usb_vbase + SUNXI_HCI_PHY_CTRL);
	reg_value &= ~(0x01 << 1);
	USBC_Writel(reg_value, (sunxi_hci->usb_vbase + SUNXI_HCI_PHY_CTRL));

	return;
}

static int open_clock(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
	//DMSG_INFO("[%s]: open clock, is_open: %d\n", sunxi_hci->hci_name, sunxi_hci->clk_is_open);
	mutex_lock(&usb_clock_lock);

#ifdef  SUNXI_USB_FPGA
	fpga_config_use_hci(sunxi_hci);
#endif

	/*otg and hci0 Controller Shared phy in SUN50I*/
	if(sunxi_hci->usbc_no == HCI0_USBC_NO){
		USBC_SelectPhyToHci(sunxi_hci);
	}

	if(sunxi_hci->ahb && sunxi_hci->mod_usbphy && !sunxi_hci->clk_is_open){
		sunxi_hci->clk_is_open = 1;
		if(clk_prepare_enable(sunxi_hci->ahb)){
			DMSG_PANIC("ERR:try to prepare_enable %s_ahb failed!\n", sunxi_hci->hci_name);
		}
		udelay(10);

		if(sunxi_hci->hsic_flag)
		{
			if(sunxi_hci->hsic_ctrl_flag){
				if(sunxi_hci->hsic_enable_flag){

					if(clk_prepare_enable(sunxi_hci->pll_hsic)){
						DMSG_PANIC("ERR:try to prepare_enable %s pll_hsic failed!\n", sunxi_hci->hci_name);
					}

					if(clk_prepare_enable(sunxi_hci->clk_usbhsic12m)){
						DMSG_PANIC("ERR:try to prepare_enable %s clk_usbhsic12m failed!\n", sunxi_hci->hci_name);
					}

					if(clk_prepare_enable(sunxi_hci->hsic_usbphy)){
						DMSG_PANIC("ERR:try to prepare_enable %s_hsic_usbphy failed!\n", sunxi_hci->hci_name);
					}
				}
			}else{
				if(clk_prepare_enable(sunxi_hci->pll_hsic)){
					DMSG_PANIC("ERR:try to prepare_enable %s pll_hsic failed!\n", sunxi_hci->hci_name);
				}

				if(clk_prepare_enable(sunxi_hci->clk_usbhsic12m)){
					DMSG_PANIC("ERR:try to prepare_enable %s clk_usbhsic12m failed!\n", sunxi_hci->hci_name);
				}

				if(clk_prepare_enable(sunxi_hci->hsic_usbphy)){
					DMSG_PANIC("ERR:try to prepare_enable %s_hsic_usbphy failed!\n", sunxi_hci->hci_name);
				}
			}
		}else{
			if(clk_prepare_enable(sunxi_hci->mod_usbphy)){
				DMSG_PANIC("ERR:try to prepare_enable %s_usbphy failed!\n", sunxi_hci->hci_name);
			}
		}

		udelay(10);

	}else{
		DMSG_PANIC("[%s]: wrn: open clock failed, (0x%p, 0x%p, %d, 0x%p)\n",
			sunxi_hci->hci_name,
			sunxi_hci->ahb, sunxi_hci->mod_usbphy, sunxi_hci->clk_is_open,
			sunxi_hci->mod_usb);
	}

	USBC_Clean_SIDDP(sunxi_hci);

	usb_phyx_tp_write(sunxi_hci, 0x2a, 3, 2);

	mutex_unlock(&usb_clock_lock);

	//DMSG_INFO("[%s]: open clock end\n", sunxi_hci->hci_name);
	return 0;
}

static int close_clock(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
	//DMSG_INFO("[%s]: close clock, is_open: %d\n", sunxi_hci->hci_name, sunxi_hci->clk_is_open);

	if(sunxi_hci->ahb && sunxi_hci->mod_usbphy && sunxi_hci->clk_is_open){
		sunxi_hci->clk_is_open = 0;

		if(sunxi_hci->hsic_flag){
			if(sunxi_hci->hsic_ctrl_flag){
				if(sunxi_hci->hsic_enable_flag){
					clk_disable_unprepare(sunxi_hci->clk_usbhsic12m);
					clk_disable_unprepare(sunxi_hci->hsic_usbphy);
					clk_disable_unprepare(sunxi_hci->pll_hsic);
				}
			}else{
				clk_disable_unprepare(sunxi_hci->clk_usbhsic12m);
				clk_disable_unprepare(sunxi_hci->hsic_usbphy);
				clk_disable_unprepare(sunxi_hci->pll_hsic);
			}
		}else{

			clk_disable_unprepare(sunxi_hci->mod_usbphy);
		}

		clk_disable_unprepare(sunxi_hci->ahb);
		udelay(10);
	}else{
		DMSG_PANIC("[%s]: wrn: open clock failed, (0x%p, 0x%p, %d, 0x%p)\n",
				sunxi_hci->hci_name,sunxi_hci->ahb,
				sunxi_hci->mod_usbphy, sunxi_hci->clk_is_open,
				sunxi_hci->mod_usb);
	}
	return 0;
}

static int usb_get_hsic_phy_ctrl(int value, int enable)
{
	if(enable){
		value |= (0x07<<8);
		value |= (0x01<<1);
		value |= (0x01<<0);
		value |= (0x01<<16);
		value |= (0x01<<20);
	}else{
		value &= ~(0x07<<8);
		value &= ~(0x01<<1);
		value &= ~(0x01<<0);
		value &= ~(0x01<<16);
		value &= ~(0x01<<20);
	}

	return value;
}

static void usb_passby(struct sunxi_hci_hcd *sunxi_hci, u32 enable)
{
	unsigned long reg_value = 0;
	spinlock_t lock;
	unsigned long flags = 0;

	mutex_lock(&usb_passby_lock);

	spin_lock_init(&lock);
	spin_lock_irqsave(&lock, flags);

	/*enable passby*/
	if(sunxi_hci->usbc_no == HCI0_USBC_NO){
		reg_value = USBC_Readl(sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE);
		if(enable && (atomic_read(&usb1_enable_passly_cnt) == 0)){
			if(sunxi_hci->hsic_flag){
				reg_value = usb_get_hsic_phy_ctrl(reg_value, enable);
			}else{
				reg_value |= (1 << 10);		/* AHB Master interface INCR8 enable */
				reg_value |= (1 << 9);		/* AHB Master interface burst type INCR4 enable */
				reg_value |= (1 << 8);		/* AHB Master interface INCRX align enable */
#ifdef SUNXI_USB_FPGA
				reg_value |= (0 << 0);		/* enable ULPI, disable UTMI */
#else
				reg_value |= (1 << 0);		/* enable UTMI, disable ULPI */
#endif
			}
		}else if(!enable && (atomic_read(&usb1_enable_passly_cnt) == 1)){
			if(sunxi_hci->hsic_flag){
				reg_value = usb_get_hsic_phy_ctrl(reg_value, enable);
			}else{
				reg_value &= ~(1 << 10);	/* AHB Master interface INCR8 disable */
				reg_value &= ~(1 << 9);		/* AHB Master interface burst type INCR4 disable */
				reg_value &= ~(1 << 8);		/* AHB Master interface INCRX align disable */
				reg_value &= ~(1 << 0);		/* ULPI bypass disable */
			}
		}
		USBC_Writel(reg_value, (sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE));

		if(enable){
			atomic_add(1, &usb1_enable_passly_cnt);
		}else{
			atomic_sub(1, &usb1_enable_passly_cnt);
		}
	}else if(sunxi_hci->usbc_no == HCI1_USBC_NO){
		reg_value = USBC_Readl(sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE);
		if(enable && (atomic_read(&usb2_enable_passly_cnt) == 0)){
			if(sunxi_hci->hsic_flag){
				reg_value = usb_get_hsic_phy_ctrl(reg_value, enable);
			}else{
				reg_value |= (1 << 10);		/* AHB Master interface INCR8 enable */
				reg_value |= (1 << 9);		/* AHB Master interface burst type INCR4 enable */
				reg_value |= (1 << 8);		/* AHB Master interface INCRX align enable */
				reg_value |= (1 << 0);		/* ULPI bypass enable */
			}
		}else if(!enable && (atomic_read(&usb2_enable_passly_cnt) == 1)){
			if(sunxi_hci->hsic_flag){
				reg_value = usb_get_hsic_phy_ctrl(reg_value, enable);
			}else{
				reg_value &= ~(1 << 10);	/* AHB Master interface INCR8 disable */
				reg_value &= ~(1 << 9);		/* AHB Master interface burst type INCR4 disable */
				reg_value &= ~(1 << 8);		/* AHB Master interface INCRX align disable */
				reg_value &= ~(1 << 0);		/* ULPI bypass disable */
			}
		}
		USBC_Writel(reg_value, (sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE));

		if(enable){
			atomic_add(1, &usb2_enable_passly_cnt);
		}else{
			atomic_sub(1, &usb2_enable_passly_cnt);
		}
	}else{
		DMSG_PANIC("EER: unkown usbc_no(%d)\n", sunxi_hci->usbc_no);
		spin_unlock_irqrestore(&lock, flags);

		mutex_unlock(&usb_passby_lock);
		return;
	}

	spin_unlock_irqrestore(&lock, flags);

	mutex_unlock(&usb_passby_lock);

	return;
}

static int alloc_pin(struct sunxi_hci_hcd *sunxi_hci)
{
	u32 ret = 1;

	if(sunxi_hci->drv_vbus_gpio_valid){
		ret = gpio_request(sunxi_hci->drv_vbus_gpio_set.gpio.gpio, NULL);
		if(ret != 0){
			DMSG_PANIC("request %s gpio:%d\n", sunxi_hci->hci_name, sunxi_hci->drv_vbus_gpio_set.gpio.gpio);
		}else{
			gpio_direction_output(sunxi_hci->drv_vbus_gpio_set.gpio.gpio, 0);
		}
	}

	if(sunxi_hci->hsic_flag){
		/* Marvell 4G HSIC ctrl*/
		if(sunxi_hci->usb_host_hsic_rdy_valid){
			ret = gpio_request(sunxi_hci->usb_host_hsic_rdy.gpio.gpio, NULL);
			if(ret != 0){
				DMSG_PANIC("ERR: gpio_request failed\n");
				sunxi_hci->usb_host_hsic_rdy_valid = 0;
			}else{
				gpio_direction_output(sunxi_hci->usb_host_hsic_rdy.gpio.gpio, 0);
			}
		}

		/* SMSC usb3503 HSIC HUB ctrl */
		if(sunxi_hci->usb_hsic_usb3503_flag){
			if(sunxi_hci->usb_hsic_hub_connect_valid){
				ret = gpio_request(sunxi_hci->usb_hsic_hub_connect.gpio.gpio, NULL);
				if(ret != 0){
					DMSG_PANIC("ERR: gpio_request failed\n");
					sunxi_hci->usb_hsic_hub_connect_valid = 0;
				}else{
					gpio_direction_output(sunxi_hci->usb_hsic_hub_connect.gpio.gpio, 1);
				}
			}

			if(sunxi_hci->usb_hsic_int_n_valid){
				ret = gpio_request(sunxi_hci->usb_hsic_int_n.gpio.gpio, NULL);
				if(ret != 0){
					DMSG_PANIC("ERR: gpio_request failed\n");
					sunxi_hci->usb_hsic_int_n_valid = 0;
				}else{
					gpio_direction_output(sunxi_hci->usb_hsic_int_n.gpio.gpio, 1);
				}
			}

			msleep(10);

			if(sunxi_hci->usb_hsic_reset_n_valid){
				ret = gpio_request(sunxi_hci->usb_hsic_reset_n.gpio.gpio, NULL);
				if(ret != 0){
					DMSG_PANIC("ERR: gpio_request failed\n");
					sunxi_hci->usb_hsic_reset_n_valid = 0;
				}else{
					gpio_direction_output(sunxi_hci->usb_hsic_reset_n.gpio.gpio, 1);
				}
			}

			/* usb3503 device goto hub connect statua is need 100ms after reset */
			msleep(100);
		}
	}

	return 0;
}

static void free_pin(struct sunxi_hci_hcd *sunxi_hci)
{

	if(sunxi_hci->drv_vbus_gpio_valid){
		gpio_free(sunxi_hci->drv_vbus_gpio_set.gpio.gpio);
		sunxi_hci->drv_vbus_gpio_valid = 0;
	}

	if(sunxi_hci->hsic_flag){
		/* Marvell 4G HSIC ctrl*/
		if(sunxi_hci->usb_host_hsic_rdy_valid){
			gpio_free(sunxi_hci->usb_host_hsic_rdy.gpio.gpio);
			sunxi_hci->usb_host_hsic_rdy_valid = 0;
		}

		/* SMSC usb3503 HSIC HUB ctrl */
		if(sunxi_hci->usb_hsic_usb3503_flag){
			if(sunxi_hci->usb_hsic_hub_connect_valid){
				gpio_free(sunxi_hci->usb_hsic_hub_connect.gpio.gpio);
				sunxi_hci->usb_hsic_hub_connect_valid = 0;
			}

			if(sunxi_hci->usb_hsic_int_n_valid){
				gpio_free(sunxi_hci->usb_hsic_int_n.gpio.gpio);
				sunxi_hci->usb_hsic_int_n_valid = 0;
			}

			if(sunxi_hci->usb_hsic_reset_n_valid){
				gpio_free(sunxi_hci->usb_hsic_reset_n.gpio.gpio);
				sunxi_hci->usb_hsic_reset_n_valid = 0;
			}
		}
	}

	return;
}

void sunxi_set_host_hisc_rdy(struct sunxi_hci_hcd *sunxi_hci, int is_on)
{
	if (sunxi_hci->usb_host_hsic_rdy_valid) {
		/* set config, output */
		gpio_direction_output(sunxi_hci->usb_host_hsic_rdy.gpio.gpio, is_on);
	}
}

void sunxi_set_host_vbus(struct sunxi_hci_hcd *sunxi_hci, int is_on)
{
	if(sunxi_hci->drv_vbus_gpio_valid){
		__gpio_set_value(sunxi_hci->drv_vbus_gpio_set.gpio.gpio, is_on);
	}
}

static void __sunxi_set_vbus(struct sunxi_hci_hcd *sunxi_hci, int is_on)
{

	//DMSG_INFO("[%s]: Set USB Power %s\n", sunxi_hci->hci_name, (is_on ? "ON" : "OFF"));

	/* set power flag */
	sunxi_hci->power_flag = is_on;

	if((sunxi_hci->regulator_io != NULL) && (sunxi_hci->regulator_io_hdle != NULL)){
		if(is_on){
			if(regulator_enable(sunxi_hci->regulator_io_hdle) < 0){
				DMSG_INFO("%s: regulator_enable fail\n", sunxi_hci->hci_name);
			}
		}else{
			if(regulator_disable(sunxi_hci->regulator_io_hdle) < 0){
				DMSG_INFO("%s: regulator_disable fail\n", sunxi_hci->hci_name);
			}
		}
	}

	if(sunxi_hci->hsic_flag){
		if((sunxi_hci->hsic_regulator_io != NULL) && (sunxi_hci->hsic_regulator_io_hdle != NULL)){
			if(is_on){
				if(regulator_enable(sunxi_hci->hsic_regulator_io_hdle) < 0){
					DMSG_INFO("%s: hsic_regulator_enable fail\n", sunxi_hci->hci_name);
				}
			}else{
				if(regulator_disable(sunxi_hci->hsic_regulator_io_hdle) < 0){
					DMSG_INFO("%s: hsic_regulator_disable fail\n", sunxi_hci->hci_name);
				}
			}
		}
	}

//no care of usb0 vbus when otg connect pc setup system without battery  and to return
#ifdef	CONFIG_USB_SUNXI_USB_MANAGER
	if(sunxi_hci->usbc_no == HCI0_USBC_NO){
		if(is_on){
			if(usb_otg_id_status() == 1){
				return;
			}
		}
	}
#endif

	if(sunxi_hci->drv_vbus_gpio_valid){
		__gpio_set_value(sunxi_hci->drv_vbus_gpio_set.gpio.gpio, is_on);
	}

	return;
}

static void sunxi_set_vbus(struct sunxi_hci_hcd *sunxi_hci, int is_on)
{

	DMSG_DEBUG("[%s]: sunxi_set_vbus cnt %d\n",
		sunxi_hci->hci_name,
		(sunxi_hci->usbc_no == 1) ? atomic_read(&usb1_set_vbus_cnt) : atomic_read(&usb2_set_vbus_cnt));

	mutex_lock(&usb_vbus_lock);

	if(sunxi_hci->usbc_no == HCI0_USBC_NO){
		if(is_on && (atomic_read(&usb1_set_vbus_cnt) == 0)){
			__sunxi_set_vbus(sunxi_hci, is_on);  /* power on */
		}else if(!is_on && atomic_read(&usb1_set_vbus_cnt) == 1){
			__sunxi_set_vbus(sunxi_hci, is_on);  /* power off */
		}

		if(is_on){
			atomic_add(1, &usb1_set_vbus_cnt);
		}else{
			atomic_sub(1, &usb1_set_vbus_cnt);
		}
	}else if(sunxi_hci->usbc_no == HCI1_USBC_NO){
		if(is_on && (atomic_read(&usb2_set_vbus_cnt) == 0)){
			__sunxi_set_vbus(sunxi_hci, is_on);  /* power on */
		}else if(!is_on && atomic_read(&usb2_set_vbus_cnt) == 1){
			__sunxi_set_vbus(sunxi_hci, is_on);  /* power off */
		}

		if(is_on){
			atomic_add(1, &usb2_set_vbus_cnt);
		}else{
			atomic_sub(1, &usb2_set_vbus_cnt);
		}
	}else{
		DMSG_INFO("[%s]: sunxi_set_vbus no: %d\n", sunxi_hci->hci_name, sunxi_hci->usbc_no);
	}

	mutex_unlock(&usb_vbus_lock);

	return;
}

static int sunxi_get_hci_base(struct platform_device *pdev, struct sunxi_hci_hcd *sunxi_hci)
{
	struct device_node *np = pdev->dev.of_node;
	struct resource res;
	int ret = 0;

	sunxi_hci->usb_vbase  = of_iomap(np, 0);
	if (sunxi_hci->usb_vbase == NULL) {
		dev_err(&pdev->dev, "%s, can't get vbase resource\n", sunxi_hci->hci_name);
		return -EINVAL;
	}

	sunxi_hci->otg_vbase  = of_iomap(np, 2);
	if (sunxi_hci->otg_vbase == NULL) {
		dev_err(&pdev->dev, "%s, can't get otg_vbase resource\n", sunxi_hci->hci_name);
		return -EINVAL;
	}

		//DMSG_INFO("OTG,Vbase:0x%p\n", sunxi_hci->otg_vbase);

	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "could not get regs\n");
	}

	sunxi_hci->usb_base_res = &res;

	//DMSG_INFO("%s,Vbase:0x%p, base res:0x%p\n", sunxi_hci->hci_name, sunxi_hci->usb_vbase, sunxi_hci->usb_base_res);

	return 0;
}

static int sunxi_get_hci_clock(struct platform_device *pdev, struct sunxi_hci_hcd *sunxi_hci)
{
	struct device_node *np = pdev->dev.of_node;

	sunxi_hci->ahb = of_clk_get(np, 1);
	if (IS_ERR(sunxi_hci->ahb)) {
		sunxi_hci->ahb = NULL;
		DMSG_PANIC("ERR: %s get usb ahb_otg clk failed.\n", sunxi_hci->hci_name);
		return -EINVAL;
	}

	sunxi_hci->mod_usbphy = of_clk_get(np, 0);
	if (IS_ERR(sunxi_hci->mod_usbphy)) {
		sunxi_hci->mod_usbphy = NULL;
		DMSG_PANIC("ERR: %s get usb mod_usbphy failed.\n", sunxi_hci->hci_name);
		return -EINVAL;
	}

	if(sunxi_hci->hsic_flag){
		sunxi_hci->hsic_usbphy = of_clk_get(np, 2);
		if (IS_ERR(sunxi_hci->hsic_usbphy)) {
			sunxi_hci->hsic_usbphy = NULL;
			DMSG_PANIC("ERR: %s get usb hsic_usbphy failed.\n", sunxi_hci->hci_name);
		}

		sunxi_hci->clk_usbhsic12m = of_clk_get(np, 3);
		if (IS_ERR(sunxi_hci->clk_usbhsic12m)) {
			sunxi_hci->clk_usbhsic12m = NULL;
			DMSG_PANIC("ERR: %s get usb clk_usbhsic12m failed.\n", sunxi_hci->hci_name);
		}

		sunxi_hci->pll_hsic = of_clk_get(np, 4);
		if (IS_ERR(sunxi_hci->pll_hsic)) {
			sunxi_hci->pll_hsic = NULL;
			DMSG_PANIC("ERR: %s get usb pll_hsic failed.\n", sunxi_hci->hci_name);
		}
	}

	return 0;
}

static int get_usb_cfg(struct platform_device *pdev, struct sunxi_hci_hcd *sunxi_hci)
{
#ifdef CONFIG_OF
	struct device_node *usbc_np = NULL;
	char np_name[10];
	int ret = -1;

	sprintf(np_name, "usbc%d", sunxi_get_hci_num(pdev));

	usbc_np = of_find_node_by_type(NULL, np_name);

	/* usbc enable */
	ret = of_property_read_string(usbc_np, "status", &sunxi_hci->used_status);
	if (ret) {
		 DMSG_PRINT("get %s used is fail, %d\n", sunxi_hci->hci_name, -ret);
		 sunxi_hci->used = 0;
	}else if (!strcmp(sunxi_hci->used_status, "okay")) {

		 sunxi_hci->used = 1;
	}else {
		 sunxi_hci->used = 0;
	}

	/* usbc init_state */
	ret = of_property_read_u32(usbc_np, KEY_USB_HOST_INIT_STATE, &sunxi_hci->host_init_state);
	if (ret) {
		 DMSG_PRINT("get %s init_state is fail, %d\n", sunxi_hci->hci_name, -ret);
	}

	sunxi_hci->hsic_flag = 0;

	if(sunxi_hci->usbc_no == HCI1_USBC_NO){
		ret = of_property_read_u32(usbc_np, KEY_USB_HSIC_USBED, &sunxi_hci->hsic_flag);
		if (ret) {
			 DMSG_PRINT("get %s usb_hsic_used is fail, %d\n", sunxi_hci->hci_name, -ret);
			 sunxi_hci->hsic_flag = 0;
		}

		if(sunxi_hci->hsic_flag){
			if(!strncmp(sunxi_hci->hci_name, "ohci", strlen("ohci"))){
				printk("HSIC is no susport in %s, and to return\n", sunxi_hci->hci_name);
				sunxi_hci->used = 0;
				return 0;
			}

			/* hsic regulator_io */
			ret = of_property_read_string(usbc_np, KEY_USB_HSIC_REGULATOR_IO, &sunxi_hci->hsic_regulator_io);
			if (ret){
				printk("get %s, hsic_regulator_io is fail, %d\n", sunxi_hci->hci_name, -ret);
				sunxi_hci->hsic_regulator_io = NULL;
			}else{
				if (!strcmp(sunxi_hci->hsic_regulator_io, "nocare")) {
					 printk("get %s, hsic_regulator_io is no nocare\n", sunxi_hci->hci_name);
					sunxi_hci->hsic_regulator_io = NULL;
				}
			}

			/* Marvell 4G HSIC ctrl */
			ret = of_property_read_u32(usbc_np, KEY_USB_HSIC_CTRL, &sunxi_hci->hsic_ctrl_flag);
			if (ret) {
				DMSG_PRINT("get %s usb_hsic_ctrl is fail, %d\n", sunxi_hci->hci_name, -ret);
				sunxi_hci->hsic_ctrl_flag = 0;
			}
			if(sunxi_hci->hsic_ctrl_flag){
				sunxi_hci->usb_host_hsic_rdy.gpio.gpio = of_get_named_gpio(usbc_np, KEY_USB_HSIC_RDY_GPIO, 0);
				if(gpio_is_valid(sunxi_hci->usb_host_hsic_rdy.gpio.gpio)){
					sunxi_hci->usb_host_hsic_rdy_valid = 1;
				}else{
					sunxi_hci->usb_host_hsic_rdy_valid = 0;
					DMSG_PRINT("get %s drv_vbus_gpio is fail\n", sunxi_hci->hci_name);
				}
			}else{
				sunxi_hci->usb_host_hsic_rdy_valid = 0;
			}

			/* SMSC usb3503 HSIC HUB ctrl */
			ret = of_property_read_u32(usbc_np, "usb_hsic_usb3503_flag", &sunxi_hci->usb_hsic_usb3503_flag);
			if (ret) {
				DMSG_PRINT("get %s usb_hsic_usb3503_flag is fail, %d\n", sunxi_hci->hci_name, -ret);
				sunxi_hci->usb_hsic_usb3503_flag = 0;
			}


			if(sunxi_hci->usb_hsic_usb3503_flag){
				sunxi_hci->usb_hsic_hub_connect.gpio.gpio = of_get_named_gpio(usbc_np, "usb_hsic_hub_connect_gpio", 0);
				if(gpio_is_valid(sunxi_hci->usb_hsic_hub_connect.gpio.gpio)){
					sunxi_hci->usb_hsic_hub_connect_valid = 1;
				}else{
					sunxi_hci->usb_hsic_hub_connect_valid = 0;
					DMSG_PRINT("get %s usb_hsic_hub_connect is fail\n", sunxi_hci->hci_name);
				}


				sunxi_hci->usb_hsic_int_n.gpio.gpio = of_get_named_gpio(usbc_np, "usb_hsic_int_n_gpio", 0);
				if(gpio_is_valid(sunxi_hci->usb_hsic_int_n.gpio.gpio)){
					sunxi_hci->usb_hsic_int_n_valid = 1;
				}else{
					sunxi_hci->usb_hsic_int_n_valid = 0;
					DMSG_PRINT("get %s usb_hsic_int_n is fail\n", sunxi_hci->hci_name);
				}


				sunxi_hci->usb_hsic_reset_n.gpio.gpio = of_get_named_gpio(usbc_np, "usb_hsic_reset_n_gpio", 0);
				if(gpio_is_valid(sunxi_hci->usb_hsic_int_n.gpio.gpio)){
					sunxi_hci->usb_hsic_reset_n_valid = 1;
				}else{
					sunxi_hci->usb_hsic_reset_n_valid = 0;
					DMSG_PRINT("get %s usb_hsic_int_n is fail\n", sunxi_hci->hci_name);
				}

			}else{
				sunxi_hci->usb_hsic_hub_connect_valid = 0;
				sunxi_hci->usb_hsic_int_n_valid = 0;
				sunxi_hci->usb_hsic_reset_n_valid = 0;
			}

		}else{
			sunxi_hci->hsic_ctrl_flag = 0;
			sunxi_hci->usb_host_hsic_rdy_valid = 0;
			sunxi_hci->usb_hsic_hub_connect_valid = 0;
			sunxi_hci->usb_hsic_int_n_valid = 0;
			sunxi_hci->usb_hsic_reset_n_valid = 0;
		}
	}

	/* usbc wakeup_suspend */
	ret = of_property_read_u32(usbc_np, KEY_USB_WAKEUP_SUSPEND, &sunxi_hci->wakeup_suspend);
	if (ret) {
		 DMSG_PRINT("get %s wakeup_suspend is fail, %d\n", sunxi_hci->hci_name, -ret);
	}

	sunxi_hci->drv_vbus_gpio_set.gpio.gpio = of_get_named_gpio(usbc_np, KEY_USB_DRVVBUS_GPIO, 0);
	if(gpio_is_valid(sunxi_hci->drv_vbus_gpio_set.gpio.gpio)){
		sunxi_hci->drv_vbus_gpio_valid = 1;
	}else{
		sunxi_hci->drv_vbus_gpio_valid = 0;
		DMSG_PRINT("get %s drv_vbus_gpio is fail\n", sunxi_hci->hci_name);
	}

	/* usbc regulator_io */
	ret = of_property_read_string(usbc_np, KEY_USB_REGULATOR_IO, &sunxi_hci->regulator_io);
	if (ret){
		printk("get %s, regulator_io is fail, %d\n", sunxi_hci->hci_name, -ret);
		sunxi_hci->regulator_io = NULL;
	}else{
		if (!strcmp(sunxi_hci->regulator_io, "nocare")) {
			 printk("get %s, regulator_io is no nocare\n", sunxi_hci->hci_name);
			sunxi_hci->regulator_io = NULL;
		}
	}

#else
	script_item_value_type_e type = 0;
	script_item_u item_temp;

	/* usbc enable */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], KEY_USB_ENABLE, &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		sunxi_hci->used = item_temp.val;
	}else{
		DMSG_INFO("get %s usbc enable failed\n" ,sunxi_hci->hci_name);
		sunxi_hci->used = 0;
	}

	/* host_init_state */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], KEY_USB_HOST_INIT_STATE, &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		sunxi_hci->host_init_state = item_temp.val;
	}else{
		DMSG_INFO("script_parser_fetch host_init_state failed\n");
		sunxi_hci->host_init_state = 1;
	}

	type = script_get_item(usbc_name[sunxi_hci->usbc_no], KEY_USB_WAKEUP_SUSPEND, &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		sunxi_hci->not_suspend = item_temp.val;
	}else{
		DMSG_INFO("get usb_restrict_flag failed\n");
		sunxi_hci->not_suspend = 0;
	}


	/* usbc drv_vbus */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], KEY_USB_DRVVBUS_GPIO, &sunxi_hci->drv_vbus_gpio_set);
	if(type == SCIRPT_ITEM_VALUE_TYPE_PIO){
		sunxi_hci->drv_vbus_gpio_valid = 1;
	}else{
		DMSG_INFO("%s(drv vbus) is invalid\n", sunxi_hci->hci_name);
		sunxi_hci->drv_vbus_gpio_valid = 0;
	}


	/* get regulator io information */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], KEY_USB_REGULATOR_IO, &item_temp);
	if (type == SCIRPT_ITEM_VALUE_TYPE_STR) {
		if (!strcmp(item_temp.str, "nocare")) {
			DMSG_INFO("get usb_regulator is nocare\n");
			sunxi_hci->regulator_io = NULL;
		}else{
			sunxi_hci->regulator_io = item_temp.str;

			type = script_get_item(usbc_name[sunxi_hci->usbc_no], KEY_USB_REGULATOR_IO_VOL, &item_temp);
			if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
				sunxi_hci->regulator_value = item_temp.val;
			}else{
				DMSG_INFO("get usb_voltage is failed\n");
				sunxi_hci->regulator_value = 0;
				sunxi_hci->regulator_io = NULL;
			}
		}
	}else {
		DMSG_INFO("get usb_regulator is failed\n");
		sunxi_hci->regulator_io = NULL;
	}

#endif

	return 0;
}

int sunxi_get_hci_num(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	int ret = 0;
	int hci_num = 0;

	ret = of_property_read_u32(np, HCI_USBC_NO, &hci_num);
	if (ret) {
		 DMSG_PANIC("get hci_ctrl_num is fail, %d\n", -ret);
	}

	return hci_num;
}

static int sunxi_get_hci_name(struct platform_device *pdev, struct sunxi_hci_hcd *sunxi_hci)
{
	struct device_node *np = pdev->dev.of_node;

	sprintf(sunxi_hci->hci_name, "%s", np->name);

	return 0;
}

static int sunxi_get_hci_irq_no(struct platform_device *pdev, struct sunxi_hci_hcd *sunxi_hci)
{
	sunxi_hci->irq_no = platform_get_irq(pdev, 0);

	//DMSG_INFO("%s,irq_no:%d\n", sunxi_hci->hci_name, sunxi_hci->irq_no);
	return 0;
}

#ifdef  SUNXI_USB_FPGA
static int sunxi_get_sram_base(struct platform_device *pdev, struct sunxi_hci_hcd *sunxi_hci)
{

	struct device_node *np = pdev->dev.of_node;

	if(sunxi_hci->usbc_no == HCI0_USBC_NO){
		sunxi_hci->sram_vbase = of_iomap(np, 1);
		if (sunxi_hci->sram_vbase == NULL) {
			dev_err(&pdev->dev, "%s, can't get sram resource\n", sunxi_hci->hci_name);
			return -EINVAL;
		}

		//DMSG_INFO("%s sram_vbase: %p\n", sunxi_hci->hci_name, sunxi_hci->sram_vbase);
	}

	return 0;
}
#endif

static int sunxi_get_hci_resource(struct platform_device *pdev, struct sunxi_hci_hcd *sunxi_hci, int usbc_no)
{

	if(sunxi_hci == NULL){
		dev_err(&pdev->dev, "sunxi_hci is NULL\n");
		return -1;
	}

	memset(sunxi_hci, 0, sizeof(struct sunxi_hci_hcd));

	sunxi_hci->usbc_no = usbc_no;
	sunxi_get_hci_name(pdev, sunxi_hci);
	get_usb_cfg(pdev, sunxi_hci);

	if(sunxi_hci->used == 0){
		DMSG_INFO("sunxi %s is no enable\n", sunxi_hci->hci_name);
		return -1;
	}

	sunxi_get_hci_base(pdev, sunxi_hci);
	sunxi_get_hci_clock(pdev, sunxi_hci);
	sunxi_get_hci_irq_no(pdev, sunxi_hci);

	request_usb_regulator_io(sunxi_hci);
	sunxi_hci->open_clock	= open_clock;
	sunxi_hci->close_clock	= close_clock;
	sunxi_hci->set_power	= sunxi_set_vbus;
	sunxi_hci->usb_passby	= usb_passby;

#ifdef  SUNXI_USB_FPGA
	sunxi_get_sram_base(pdev, sunxi_hci);
	fpga_config_use_hci(sunxi_hci);
#endif

	alloc_pin(sunxi_hci);

	pdev->dev.platform_data = sunxi_hci;
	return 0;
}

int exit_sunxi_hci(struct sunxi_hci_hcd *sunxi_hci)
{
	release_usb_regulator_io(sunxi_hci);
	free_pin(sunxi_hci);
	return 0;
}

int init_sunxi_hci(struct platform_device *pdev, int usbc_type)
{
	struct sunxi_hci_hcd *sunxi_hci = NULL;
	int usbc_no = 0;
	int hci_num = -1;
	int ret = -1;

#ifdef CONFIG_OF
	pdev->dev.dma_mask = &sunxi_hci_dmamask;
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
#endif

	atomic_set(&usb1_set_vbus_cnt, 0);
	atomic_set(&usb2_set_vbus_cnt, 0);

	atomic_set(&usb1_enable_passly_cnt, 0);
	atomic_set(&usb2_enable_passly_cnt, 0);

	hci_num = sunxi_get_hci_num(pdev);

	switch(hci_num){
		case HCI0_USBC_NO:
			usbc_no = HCI0_USBC_NO;
			if(usbc_type == SUNXI_USB_EHCI){
				sunxi_hci =  &sunxi_ehci0;
			}else if(usbc_type == SUNXI_USB_OHCI){
				sunxi_hci =  &sunxi_ohci0;
			}else{
				dev_err(&pdev->dev, "get hci num fail: %d\n", hci_num);
				return -1;
			}
		break;

		case HCI1_USBC_NO:
			usbc_no = HCI1_USBC_NO;
			if(usbc_type == SUNXI_USB_EHCI){
				sunxi_hci =  &sunxi_ehci1;
			}else if(usbc_type == SUNXI_USB_OHCI){
				sunxi_hci =  &sunxi_ohci1;
			}else{
				dev_err(&pdev->dev, "get hci num fail: %d\n", hci_num);
				return -1;
			}
		break;

		default:
			dev_err(&pdev->dev, "get hci num fail: %d\n", hci_num);
		return -1;

	}

	ret = sunxi_get_hci_resource(pdev, sunxi_hci, usbc_no);

	return ret;
}

