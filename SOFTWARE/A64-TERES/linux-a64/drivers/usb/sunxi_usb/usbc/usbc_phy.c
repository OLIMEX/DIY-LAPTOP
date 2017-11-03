/*
 * drivers/usb/sunxi_usb/usbc/usbc_phy.c
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * daniel, 2009.10.21
 *
 * usb common ops.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include  "usbc_i.h"

/*
 * define USB PHY controller reg bit
 */

//Common Control Bits for Both PHYs
#define  USBC_PHY_PLL_BW			0x03
#define  USBC_PHY_RES45_CAL_EN			0x0c

//Private Control Bits for Each PHY
#define  USBC_PHY_TX_AMPLITUDE_TUNE		0x20
#define  USBC_PHY_TX_SLEWRATE_TUNE		0x22
#define  USBC_PHY_VBUSVALID_TH_SEL		0x25
#define  USBC_PHY_PULLUP_RES_SEL		0x27
#define  USBC_PHY_OTG_FUNC_EN			0x28
#define  USBC_PHY_VBUS_DET_EN			0x29
#define  USBC_PHY_DISCON_TH_SEL			0x2a


#if 0
/*
 *
 * read out one bit of USB PHY register
 */
static __u32 __USBC_PHY_REG_READ(void __iomem *usbc_base_addr, __u32 usbc_phy_reg_addr)
{
	__u32 reg_val = 0;
	__u32 i = 0;

	USBC_Writeb(usbc_phy_reg_addr, USBC_REG_PHYCTL(USBC0_REGS_BASE) + 1);
	for(i=0; i<0x4; i++);
	reg_val = USBC_Readb(USBC_REG_PHYCTL(USBC0_REGS_BASE) + 2);
	if (usbc_base_addr == USBC0_REGS_BASE)
		return (reg_val & 0x1);
	else
		return ((reg_val >> 1) & 0x1);
}

/*
 *
 * Write one bit of USB PHY register
 */
static void __USBC_PHY_REG_WRITE(void __iomem *usbc_base_addr, __u32 usbc_phy_reg_addr, __u32 usbc_phy_reg_data)
{
	__u32 reg_val = 0;

	USBC_Writeb(usbc_phy_reg_addr, USBC_REG_PHYCTL(USBC0_REGS_BASE) + 1);
	reg_val = USBC_Readb(USBC_REG_PHYCTL(USBC0_REGS_BASE));
	reg_val &= ~(0x1 << 7);
	reg_val |= (usbc_phy_reg_data & 0x1) << 7;
	if (usbc_base_addr == USBC0_REGS_BASE) {
		reg_val &= ~0x1;
		USBC_Writeb(reg_val, USBC_REG_PHYCTL(USBC0_REGS_BASE));
		reg_val |= 0x1;
		USBC_Writeb(reg_val, USBC_REG_PHYCTL(USBC0_REGS_BASE));
		reg_val &= ~0x1;
		USBC_Writeb(reg_val, USBC_REG_PHYCTL(USBC0_REGS_BASE));
	} else {
		reg_val &= ~0x2;
		USBC_Writeb(reg_val, USBC_REG_PHYCTL(USBC0_REGS_BASE));
		reg_val |= 0x2;
		USBC_Writeb(reg_val, USBC_REG_PHYCTL(USBC0_REGS_BASE));
		reg_val &= ~0x2;
		USBC_Writeb(reg_val, USBC_REG_PHYCTL(USBC0_REGS_BASE));
	}
}


/*
 * Set USB PLL BandWidth, val = 0~3, defualt = 0x2
 */
/*
static void __USBC_PHY_SET_PLL_BW(__u32 val)
{
	__USBC_PHY_REG_WRITE(USBC0_REGS_BASE, USBC_PHY_PLL_BW, val);
	__USBC_PHY_REG_WRITE(USBC0_REGS_BASE, USBC_PHY_PLL_BW + 1, val >> 1);
}
*/

/*
 * Enable/Disable USB res45 Calibration, val = 0--Disable；1--Enable, default = 0
 */
static void __USBC_PHY_RES45_CALIBRATION_ENABLE(__u32 val)
{
	__USBC_PHY_REG_WRITE(USBC0_REGS_BASE, USBC_PHY_RES45_CAL_EN, val);
}

/*
 * Set USB TX Signal Amplitude, val = 0~3, default = 0x0
 */
static void __USBC_PHY_SET_TX_AMPLITUDE(void __iomem*  usbc_base_addr, __u32 val)
{
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_TX_AMPLITUDE_TUNE, val);
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_TX_AMPLITUDE_TUNE + 1, val >> 1);
}

/*
 * Set USB TX Signal Slew Rate, val = 0~7, default = 0x5
 */
static void __USBC_PHY_SET_TX_SLEWRATE(void __iomem*  usbc_base_addr, __u32 val)
{
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_TX_SLEWRATE_TUNE, val);
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_TX_SLEWRATE_TUNE + 1, val >> 1);
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_TX_SLEWRATE_TUNE + 2, val >> 2);
}

/*
 * Set USB VBUS Valid Threshold, val = 0~3, default = 2
 */
/*
static void __USBC_PHY_SET_VBUS_VALID_THRESHOLD(void __iomem*  usbc_base_addr, __u32 val)
{
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_VBUSVALID_TH_SEL, val);
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_VBUSVALID_TH_SEL + 1, val >> 1);
}
*/

/*
 * Enable/Diasble USB OTG Function, val = 0--Disable；1--Enable, default = 1
 */
/*
static void __USBC_PHY_OTG_FUNC_ENABLE(void __iomem*  usbc_base_addr, __u32 val)
{
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_OTG_FUNC_EN, val);
}
*/

/*
 * Enable/Diasble USB VBUS Detect Function, val = 0--Disable；1--Enable, default = 1
 */
/*
static void __USBC_PHY_VBUS_DET_ENABLE(void __iomem*  usbc_base_addr, __u32 val)
{
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_VBUS_DET_EN, val);
}
*/

/*
 * Set USB Disconnect Detect Threshold, val = 0~3, default = 1
 */
static void __USBC_PHY_SET_DISCON_DET_THRESHOLD(void __iomem*  usbc_base_addr, __u32 val)
{
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_DISCON_TH_SEL, val);
	__USBC_PHY_REG_WRITE(usbc_base_addr, USBC_PHY_DISCON_TH_SEL + 1, val >> 1);
}
#endif

/* usb PHY common set, initialize */
void USBC_PHY_SetCommonConfig(void)
{
	//__USBC_PHY_RES45_CALIBRATION_ENABLE(1);
}

/* usb PHY specific set
 * @hUSB: handle returned by USBC_open_otg, include some key data that the USBC need.
 *
 */
void USBC_PHY_SetPrivateConfig(__hdle hUSB)
{
	//__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	//
	//if (usbc_otg == NULL) {
	//	return ;
	//}
	//
	//USBC_REG_set_bit_l(0, USBC_REG_PHYTUNE(usbc_otg->base_addr));
	//USBC_REG_set_bit_l(7, USBC_REG_PHYTUNE(usbc_otg->base_addr));
	//USBC_REG_set_bit_l(6, USBC_REG_PHYTUNE(usbc_otg->base_addr));
	//USBC_REG_set_bit_l(5, USBC_REG_PHYTUNE(usbc_otg->base_addr));
	//USBC_REG_set_bit_l(4, USBC_REG_PHYTUNE(usbc_otg->base_addr));
	////__USBC_PHY_SET_TX_AMPLITUDE(usbc_otg->base_addr, 2);
	////__USBC_PHY_SET_TX_SLEWRATE(usbc_otg->base_addr, 6);
	////__USBC_PHY_SET_DISCON_DET_THRESHOLD(usbc_otg->base_addr, 3);
}

/* get PHY's common setting. for debug, to see if PHY is set correctly.
 *
 * return the 32bit usb PHY common setting value.
 */
__u32 USBC_PHY_GetCommonConfig(void)
{
	__u32 reg_val = 0;
	/*
	__u32 i = 0;

	reg_val = 0;
	for(i=0; i<0x20; i++)
	{
		reg_val = reg_val << 1;
		reg_val |= __USBC_PHY_REG_READ(USBC0_REGS_BASE, (0x1f - i)) & 0x1;
	}
	*/
	return reg_val;
}

/* write usb PHY0's phy reg setting. mainly for phy0 standby.
 *
 * return the data wrote
 */
static __u32 usb_phy0_write(__u32 addr, __u32 data, __u32 dmask, void __iomem *usbc_base_addr)
{
	__u32 i=0;

	data = data & 0x0f;
	addr = addr & 0x0f;
	dmask = dmask & 0x0f;

	USBC_Writeb((dmask<<4)|data, usbc_base_addr + 0x404 + 2);
	USBC_Writeb(addr|0x10, usbc_base_addr + 0x404);
	for(i=0;i<5;i++);
	USBC_Writeb(addr|0x30, usbc_base_addr + 0x404);
	for(i=0;i<5;i++);
	USBC_Writeb(addr|0x10, usbc_base_addr + 0x404);
	for(i=0;i<5;i++);

	return (USBC_Readb(usbc_base_addr + 0x404 + 3) & 0x0f);
}

/* Standby the usb phy with the input usb phy index number
 * @phy_index: usb phy index number, which used to select the phy to standby
 *
 */
void USBC_phy_Standby(__hdle hUSB, __u32 phy_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (phy_index == 0) {
		usb_phy0_write(0xB, 0x8, 0xf, usbc_otg->base_addr);
		usb_phy0_write(0x7, 0xf, 0xf, usbc_otg->base_addr);
		usb_phy0_write(0x1, 0xf, 0xf, usbc_otg->base_addr);
		usb_phy0_write(0x2, 0xf, 0xf, usbc_otg->base_addr);
	}

	return;
}

/* Recover the standby phy with the input index number
 * @phy_index: usb phy index number
 *
 */
void USBC_Phy_Standby_Recover(__hdle hUSB, __u32 phy_index)
{
	__u32 i;

	if (phy_index == 0) {
		for(i=0; i<0x10; i++);
	}

	return;
}

#if 0
static __u32 USBC_Phy_TpRead(__u32 usbc_no, __u32 addr, __u32 len)
{
	void __iomem *otgc_base = NULL;
	void __iomem *phyctl_val = NULL;
	__u32 temp = 0, ret = 0;
	__u32 i=0;
	__u32 j=0;

	otgc_base = get_otgc_vbase();
	if(otgc_base == NULL){
		return 0;
	}

	phyctl_val = otgc_base + USBPHYC_REG_o_PHYCTL;

	for(j = len; j > 0; j--)
	{
		/* set  the bit address to be read */
		temp = USBC_Readl(phyctl_val);
		temp &= ~(0xff << 8);
		temp |= ((addr + j -1) << 8);
		USBC_Writel(temp, phyctl_val);

		for(i = 0; i < 0x4; i++);

		temp = USBC_Readl(phyctl_val);
		ret <<= 1;
		ret |= ((temp >> (16 + usbc_no)) & 0x1);
	}

	return ret;
}
#endif

static __u32 USBC_Phy_TpWrite(__u32 usbc_no, __u32 addr, __u32 data, __u32 len)
{
	void __iomem *otgc_base = NULL;
	void __iomem *phyctl_val = NULL;
	__u32 temp = 0, dtmp = 0;
	__u32 j=0;

	otgc_base = get_otgc_vbase();
	if(otgc_base == NULL){
		return 0;
	}

	phyctl_val = otgc_base + USBPHYC_REG_o_PHYCTL;

	dtmp = data;
	for(j = 0; j < len; j++)
	{
		/* set the bit address to be write */
		temp = USBC_Readl(phyctl_val);
		temp &= ~(0xff << 8);
		temp |= ((addr + j) << 8);
		USBC_Writel(temp, phyctl_val);

		temp = USBC_Readb(phyctl_val);
		temp &= ~(0x1 << 7);
		temp |= (dtmp & 0x1) << 7;
		temp &= ~(0x1 << (usbc_no << 1));
		USBC_Writeb(temp, phyctl_val);

		temp = USBC_Readb(phyctl_val);
		temp |= (0x1 << (usbc_no << 1));
		USBC_Writeb( temp, phyctl_val);

		temp = USBC_Readb(phyctl_val);
		temp &= ~(0x1 << (usbc_no <<1 ));
		USBC_Writeb(temp, phyctl_val);
		dtmp >>= 1;
	}

	return data;
}

#if 0
static __u32 USBC_Phy_Read(__u32 usbc_no, __u32 addr, __u32 len)
{
	return USBC_Phy_TpRead(usbc_no, addr, len);
}
#endif

static __u32 USBC_Phy_Write(__u32 usbc_no, __u32 addr, __u32 data, __u32 len)
{
	return USBC_Phy_TpWrite(usbc_no, addr, data, len);
}

void UsbPhyCtl(void __iomem *regs)
{
	__u32 reg_val = 0;

	reg_val = USBC_Readl(regs + USBPHYC_REG_o_PHYCTL);
	reg_val |= (0x01 << USBC_PHY_CTL_VBUSVLDEXT);
	USBC_Writel(reg_val, (regs + USBPHYC_REG_o_PHYCTL));

	return;
}

void USBC_PHY_Set_Ctl(void __iomem *regs, __u32 mask)
{
	__u32 reg_val = 0;

	reg_val = USBC_Readl(regs + USBPHYC_REG_o_PHYCTL);
	reg_val |= (0x01 << mask);
	USBC_Writel(reg_val, (regs + USBPHYC_REG_o_PHYCTL));

	return;
}

void USBC_PHY_Clear_Ctl(void __iomem *regs, __u32 mask)
{
	__u32 reg_val = 0;
	reg_val = USBC_Readl(regs + USBPHYC_REG_o_PHYCTL);
	reg_val &= ~(0x01 << mask);
	USBC_Writel(reg_val, (regs + USBPHYC_REG_o_PHYCTL));

	return;
}

void UsbPhyInit(__u32 usbc_no)
{

	/* adjust the 45 ohm resistor */
	if (usbc_no == 0) {
		USBC_Phy_Write(usbc_no, 0x0c, 0x01, 1);
	}

	/* adjust USB0 PHY range and rate */
	USBC_Phy_Write(usbc_no, 0x20, 0x14, 5);

	/* adjust disconnect threshold */
	USBC_Phy_Write(usbc_no, 0x2a, 3, 2);
	/*by wangjx*/

	return;
}

void UsbPhyEndReset(__u32 usbc_no)
{
	int i;

	if (usbc_no == 0) {
		//Disable Sequelch Detect for a while before Release USB Reset
		USBC_Phy_Write(usbc_no, 0x3c, 0x2, 2);
		for(i=0; i<0x100; i++);
		USBC_Phy_Write(usbc_no, 0x3c, 0x0, 2);
	}
	return;
}

