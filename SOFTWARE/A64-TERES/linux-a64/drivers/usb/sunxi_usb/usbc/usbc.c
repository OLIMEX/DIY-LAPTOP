/*
 * drivers/usb/sunxi_usb/usbc/usbc.c
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * daniel, 2009.09.01
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

static void __iomem *usbc_base_address;       /* usb base address */
static __usbc_otg_t usbc_otg_array[USBC_MAX_OPEN_NUM];  /* usbc internal use, in charge of USB port */
static __fifo_info_t usbc_info_g;

/*
 * get vbus current state
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 * return the current VBUS state
 */
__u32 USBC_GetVbusStatus(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u8 reg_val = 0;

	if (usbc_otg == NULL) {
		return 0;
	}

	reg_val = USBC_Readb(USBC_REG_DEVCTL(usbc_otg->base_addr));
	reg_val = reg_val >> USBC_BP_DEVCTL_VBUS;
	switch(reg_val & 0x03) {
	case 0x00:
		return USBC_VBUS_STATUS_BELOW_SESSIONEND;
		//break;
	case 0x01:
		return USBC_VBUS_STATUS_ABOVE_SESSIONEND_BELOW_AVALID;
		//break;
	case 0x02:
		return USBC_VBUS_STATUS_ABOVE_AVALID_BELOW_VBUSVALID;
		//break;
	case 0x03:
		return USBC_VBUS_STATUS_ABOVE_VBUSVALID;
		//break;
	default:
		return USBC_VBUS_STATUS_BELOW_SESSIONEND;
	}
}

/*
 * select the function type, now is for host, or device
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 */
void USBC_OTG_SelectMode(__hdle hUSB, __u32 mode)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	if (mode == USBC_OTG_HOST) {

	} else {

	}
}

/*
 * get the length of data that can be read from current FIFO
 * @hUSB:     handle return by USBC_open_otg, include the key data which USBC need
 * @ep_type:  ep type, tx or rx
 *
 * return the data length that can be current read
 */
__u32 USBC_ReadLenFromFifo(__hdle hUSB, __u32 ep_type)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_EP0:
		return USBC_Readw(USBC_REG_COUNT0(usbc_otg->base_addr));
		//break;
	case USBC_EP_TYPE_TX:
		return 0;
		//break;
	case USBC_EP_TYPE_RX:
		return USBC_Readw(USBC_REG_RXCOUNT(usbc_otg->base_addr));
		//break;
	default:
		return 0;
	}
}

/*
 * write data packet to fifo
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 * @fifo: fifo address
 * @cnt:  data length
 * @buff: store the data to be written
 *
 * return the lenght that successfully written
 */
__u32 USBC_WritePacket(__hdle hUSB, void __iomem *fifo, __u32 cnt, void *buff)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 len = 0;
	__u32 i32 = 0;
	__u32 i8  = 0;
	__u8  *buf8  = NULL;
	__u32 *buf32 = NULL;

	if (usbc_otg == NULL || buff == NULL) {
		return 0;
	}

	//--<1>-- adjust data
	buf32 = buff;
	len   = cnt;

	i32 = len >> 2;
	i8  = len & 0x03;

	//--<2>-- deal with 4byte part
	while (i32--) {

		USBC_Writel(*buf32++, fifo);
	}

	//--<3>-- deal with no 4byte part
	buf8 = (__u8 *)buf32;
	while (i8--) {
		USBC_Writeb(*buf8++, fifo);
	}

	return len;
}

/*
 * read data from fifo
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 * @fifo: fifo address
 * @cnt:  data length
 * @buff: store the data that will be read
 *
 * return the lenght that successfully read
 */
__u32 USBC_ReadPacket(__hdle hUSB, void __iomem *fifo, __u32 cnt, void *buff)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 len = 0;
	__u32 i32 = 0;
	__u32 i8  = 0;
	__u8  *buf8  = NULL;
	__u32 *buf32 = NULL;

	if (usbc_otg == NULL || buff == NULL) {
		return 0;
	}

	//--<1>-- adjust data
	buf32 = buff;
	len   = cnt;

	i32 = len >> 2;
	i8  = len & 0x03;

	//--<2>-- deal with 4byte part
	while (i32--) {
		*buf32++ = USBC_Readl(fifo);
	}

	//--<3>-- deal with no 4byte part
	buf8 = (__u8 *)buf32;
	while (i8--) {
		*buf8++ = USBC_Readb(fifo);
	}

	return len;
}

void USBC_ConfigFIFO_Base(__hdle hUSB, __u32 fifo_mode)
{
	__fifo_info_t *usbc_info = &usbc_info_g;

	usbc_info->port0_fifo_addr = 0x00;
	usbc_info->port0_fifo_size = fifo_mode;	//8k

	return ;
}

/* get port fifo's start address */
void __iomem *USBC_GetPortFifoStartAddr(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	if (usbc_otg->port_num == 0) {
		return usbc_info_g.port0_fifo_addr;
	} else if (usbc_otg->port_num == 1) {
		return usbc_info_g.port1_fifo_addr;
	} else {
		return usbc_info_g.port2_fifo_addr;
	}
}

/* get port fifo's size */
__u32 USBC_GetPortFifoSize(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	if (usbc_otg->port_num == 0) {
		return usbc_info_g.port0_fifo_size;
	} else {
		return usbc_info_g.port1_fifo_size;
	}
}

/*
__u32 USBC_SelectFIFO(__hdle hUSB, __u32 ep_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 fifo = 0;

	if (usbc_otg == NULL) {
		return 0;
	}

	switch(ep_index) {
	case 0:
		fifo = USBC_REG_EPFIFO0(usbc_otg->base_addr);
		break;

	case 1:
		fifo = USBC_REG_EPFIFO1(usbc_otg->base_addr);
		break;

	case 2:
		fifo = USBC_REG_EPFIFO2(usbc_otg->base_addr);
		break;

	case 3:
		fifo = USBC_REG_EPFIFO3(usbc_otg->base_addr);
		break;

	case 4:
		fifo = USBC_REG_EPFIFO4(usbc_otg->base_addr);
		break;

	case 5:
		fifo = USBC_REG_EPFIFO5(usbc_otg->base_addr);
		break;

	default:
		fifo = 0;
	}

	return fifo;
}
*/

void __iomem *USBC_SelectFIFO(__hdle hUSB, __u32 ep_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	return USBC_REG_EPFIFOx(usbc_otg->base_addr, ep_index);
}

static void __USBC_ConfigFifo_TxEp_Default(void __iomem *usbc_base_addr)
{
	USBC_Writew(0x00, USBC_REG_TXFIFOAD(usbc_base_addr));
	USBC_Writeb(0x00, USBC_REG_TXFIFOSZ(usbc_base_addr));
}

/*
 * set tx ep's fifo address and size
 * @hUSB:           handle return by USBC_open_otg, include the key data which USBC need
 * @is_double_fifo: if use hardware double fifo
 * @fifo_size:      fifo size = 2 ^ fifo_size
 * @fifo_addr:      fifo start addr = fifo_addr * 8
 *
 */
static void __USBC_ConfigFifo_TxEp(void __iomem *usbc_base_addr, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
	__u32 temp = 0;
	__u32 size = 0;   //fifo_size = 2^ (size + 3)
	__u32 addr = 0;   //fifo_addr = addr * 8

	//--<1>-- 512 align
	temp = fifo_size + 511;
	temp &= ~511;
	temp >>= 3;
	temp >>= 1;
	while(temp) {
		size++;
		temp >>= 1;
	}

	//--<2>-- calculate addr
	addr = fifo_addr >> 3;

	//--<3>--config fifo addr
	USBC_Writew(addr, USBC_REG_TXFIFOAD(usbc_base_addr));

	//--<4>--config fifo size
	USBC_Writeb((size & 0x0f), USBC_REG_TXFIFOSZ(usbc_base_addr));
	if (is_double_fifo) {
		USBC_REG_set_bit_b(USBC_BP_TXFIFOSZ_DPB, USBC_REG_TXFIFOSZ(usbc_base_addr));
	}
}

static void __USBC_ConfigFifo_RxEp_Default(void __iomem *usbc_base_addr)
{
	USBC_Writew(0x00, USBC_REG_RXFIFOAD(usbc_base_addr));
	USBC_Writeb(0x00, USBC_REG_RXFIFOSZ(usbc_base_addr));
}

/*
 * set rx ep's fifo address and size
 * @hUSB:           handle return by USBC_open_otg, include the key data which USBC need
 * @is_double_fifo: if use hardware double fifo
 * @fifo_size:      fifo size = 2 ^ fifo_size
 * @fifo_addr:      fifo start addr = fifo_addr * 8
 *
 */
static void __USBC_ConfigFifo_RxEp(void __iomem *usbc_base_addr, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
	__u32 temp = 0;
	__u32 size = 0;   //fifo_size = 2 ^ (size + 3)
	__u32 addr = 0;   //fifo_addr = addr * 8

	//--<1>-- 512 align
	temp = fifo_size + 511;
	temp &= ~511;
	temp >>= 3;
	temp >>= 1;
	while(temp) {
		size++;
		temp >>= 1;
	}

	//--<2>--calculate addr
	addr = fifo_addr >> 3;

	//--<3>--config fifo addr
	USBC_Writew(addr, USBC_REG_RXFIFOAD(usbc_base_addr));

	//--<2>--config fifo size
	USBC_Writeb((size & 0x0f), USBC_REG_RXFIFOSZ(usbc_base_addr));
	if (is_double_fifo) {
		USBC_REG_set_bit_b(USBC_BP_RXFIFOSZ_DPB, USBC_REG_RXFIFOSZ(usbc_base_addr));
	}
}

/*
 * config ep's fifo addr and size
 * @hUSB:     handle return by USBC_open_otg, include the key data which USBC need
 * @ep_type:  ep type
 *
 */
void USBC_ConfigFifo_Default(__hdle hUSB, __u32 ep_type)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_EP0:
		//not support
		break;
	case USBC_EP_TYPE_TX:
		__USBC_ConfigFifo_TxEp_Default(usbc_otg->base_addr);
		break;
	case USBC_EP_TYPE_RX:
		__USBC_ConfigFifo_RxEp_Default(usbc_otg->base_addr);
		break;
	default:
		break;
	}
}

/*
 * config ep's fifo addr and size
 * @hUSB:           handle return by USBC_open_otg, include the key data which USBC need
 * @ep_type:        ep type
 * @is_double_fifo: if use hardware double fifo
 * @fifo_size:      fifo size = 2 ^ fifo_size
 * @fifo_addr:      fifo start addr = fifo_addr * 8
 *
 */
void USBC_ConfigFifo(__hdle hUSB, __u32 ep_type, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_EP0:
		//not support
		break;
	case USBC_EP_TYPE_TX:
		__USBC_ConfigFifo_TxEp(usbc_otg->base_addr, is_double_fifo, fifo_size, fifo_addr);
		break;
	case USBC_EP_TYPE_RX:
		__USBC_ConfigFifo_RxEp(usbc_otg->base_addr, is_double_fifo, fifo_size, fifo_addr);
		break;
	default:
		break;
	}
}

/*
 * get the last frma number
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 */
__u32 USBC_GetLastFrameNumber(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	return USBC_Readl(USBC_REG_FRNUM(usbc_otg->base_addr));
}

/*
 * get status of DP
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 */
__u32 USBC_GetStatus_Dp(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;

	if (usbc_otg == NULL) {
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	temp = (temp >> USBC_BP_ISCR_EXT_DP_STATUS) & 0x01;

	return temp;
}

/*
 * get status of DM
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 */
__u32 USBC_GetStatus_Dm(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;

	if (usbc_otg == NULL) {
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	temp = (temp >> USBC_BP_ISCR_EXT_DM_STATUS) & 0x01;

	return temp;
}

/*
 * get status of DpDm
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 */
__u32 USBC_GetStatus_DpDm(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;
	__u32 dp = 0;
	__u32 dm = 0;


	if (usbc_otg == NULL) {
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	dp = (temp >> USBC_BP_ISCR_EXT_DP_STATUS) & 0x01;
	dm = (temp >> USBC_BP_ISCR_EXT_DM_STATUS) & 0x01;
	return ((dp << 1) | dm);

}

/*
 * get current OTG mode from vendor0's id
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 * return USBC_OTG_DEVICE / USBC_OTG_HOST
 */
__u32 USBC_GetOtgMode_Form_ID(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 mode = 0;

	if (usbc_otg == NULL) {
		return USBC_OTG_DEVICE;
	}

	mode = USBC_REG_test_bit_l(USBC_BP_ISCR_MERGED_ID_STATUS, USBC_REG_ISCR(usbc_otg->base_addr));
	if (mode) {
		return USBC_OTG_DEVICE;
	} else {
		return USBC_OTG_HOST;
	}
}

/*
 * get current OTG mode from OTG Device's B-Device
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 * return USBC_OTG_DEVICE / USBC_OTG_HOST
 */
__u32 USBC_GetOtgMode_Form_BDevice(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 mode = 0;

	if (usbc_otg == NULL) {
		return USBC_OTG_DEVICE;
	}

	mode = USBC_REG_test_bit_b(USBC_BP_DEVCTL_B_DEVICE, USBC_REG_DEVCTL(usbc_otg->base_addr));
	if (mode) {
		return USBC_OTG_DEVICE;
	} else {
		return USBC_OTG_HOST;
	}
}


/*
 * otg and hci0 Controller Shared phy in SUN50I
 * select 1:to device,0:hci
 */
void USBC_SelectPhyToDevice(void __iomem *usbc_base_addr)
{
	int reg_value = 0;
	reg_value = USBC_Readl(usbc_base_addr + 0x420);
	reg_value |= (0x01);
	USBC_Writel(reg_value, (usbc_base_addr + 0x420));
}

/*
 * select the bus way for data transfer
 * @hUSB:     handle return by USBC_open_otg, include the key data which USBC need
 * @io_type:  bus type, pio or dma
 * @ep_type:  ep type, rx or tx
 * @ep_index: ep index
 *
 */
void USBC_SelectBus(__hdle hUSB, __u32 io_type, __u32 ep_type, __u32 ep_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if (usbc_otg == NULL) {
		return ;
	}

	reg_val = USBC_Readb(USBC_REG_VEND0(usbc_otg->base_addr));
	if (io_type == USBC_IO_TYPE_DMA) {
		if (ep_type == USBC_EP_TYPE_TX) {
			reg_val |= ((ep_index - 0x01) << 1) << USBC_BP_VEND0_DRQ_SEL;  //drq_sel
			reg_val |= 0x1<<USBC_BP_VEND0_BUS_SEL;   //io_dma
		} else {
			reg_val |= ((ep_index << 1) - 0x01) << USBC_BP_VEND0_DRQ_SEL;
			reg_val |= 0x1<<USBC_BP_VEND0_BUS_SEL;
		}
	} else {
		//reg_val &= ~(0x1 << USBC_BP_VEND0_DRQ_SEL);  //clear drq_sel, select pio
		reg_val &= 0x00;  // clear drq_sel, select pio
	}

	/*
	 * in SUN8IW5 SUN8IW6 and later ic, FIFO_BUS_SEL bit(bit24 of reg0x40
	 * for host/device) is fixed to 1, the hw guarantee that it's ok for
	 * cpu/inner_dma/outer_dma transfer.
	 */

	reg_val |= 0x1<<USBC_BP_VEND0_BUS_SEL;

	USBC_Writeb(reg_val, USBC_REG_VEND0(usbc_otg->base_addr));
}

/* get tx ep's interrupt flag */
static __u32 __USBC_INT_TxPending(void __iomem *usbc_base_addr)
{
	return (USBC_Readw(USBC_REG_INTTx(usbc_base_addr)));
}

/* clear tx ep's interrupt flag */
static void __USBC_INT_ClearTxPending(void __iomem *usbc_base_addr, __u8 ep_index)
{
	USBC_Writew((1 << ep_index), USBC_REG_INTTx(usbc_base_addr));
}

/* clear all the tx ep's interrupt flag */
static void __USBC_INT_ClearTxPendingAll(void __iomem *usbc_base_addr)
{
	USBC_Writew(0xffff, USBC_REG_INTTx(usbc_base_addr));
}

/* get rx ep's interrupt flag */
static __u32 __USBC_INT_RxPending(void __iomem *usbc_base_addr)
{
	return (USBC_Readw(USBC_REG_INTRx(usbc_base_addr)));
}

/* clear rx ep's interrupt flag */
static void __USBC_INT_ClearRxPending(void __iomem *usbc_base_addr, __u8 ep_index)
{
	USBC_Writew((1 << ep_index), USBC_REG_INTRx(usbc_base_addr));
}

/* clear all the rx ep's interrupt flag */
static void __USBC_INT_ClearRxPendingAll(void __iomem *usbc_base_addr)
{
	USBC_Writew(0xffff, USBC_REG_INTRx(usbc_base_addr));
}

/* open a tx ep's interrupt */
static void __USBC_INT_EnableTxEp(void __iomem *usbc_base_addr, __u8 ep_index)
{
	USBC_REG_set_bit_w(ep_index, USBC_REG_INTTxE(usbc_base_addr));
}

/* open a rx ep's interrupt */
static void __USBC_INT_EnableRxEp(void __iomem *usbc_base_addr, __u8 ep_index)
{
	USBC_REG_set_bit_w(ep_index, USBC_REG_INTRxE(usbc_base_addr));
}

/* close a tx ep's interrupt */
static void __USBC_INT_DisableTxEp(void __iomem *usbc_base_addr, __u8 ep_index)
{
	USBC_REG_clear_bit_w(ep_index, USBC_REG_INTTxE(usbc_base_addr));
}

/* close a rx ep's interrupt */
static void __USBC_INT_DisableRxEp(void __iomem *usbc_base_addr, __u8 ep_index)
{
	USBC_REG_clear_bit_w(ep_index, USBC_REG_INTRxE(usbc_base_addr));
}

/* close all tx ep's interrupt */
static void __USBC_INT_DisableTxAll(void __iomem *usbc_base_addr)
{
	USBC_Writew(0, USBC_REG_INTTxE(usbc_base_addr));
}

/* close all rx ep's interrupt */
static void __USBC_INT_DisableRxAll(void __iomem *usbc_base_addr)
{
	USBC_Writew(0, USBC_REG_INTRxE(usbc_base_addr));
}

/* get ep's interrupt flag */
__u32 USBC_INT_EpPending(__hdle hUSB, __u32 ep_type)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_EP0:
	case USBC_EP_TYPE_TX:
		return __USBC_INT_TxPending(usbc_otg->base_addr);

	case USBC_EP_TYPE_RX:
		return __USBC_INT_RxPending(usbc_otg->base_addr);

	default:
		return 0;
	}
}

/* clear ep's interrupt flag */
void USBC_INT_ClearEpPending(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_EP0:
	case USBC_EP_TYPE_TX:
		__USBC_INT_ClearTxPending(usbc_otg->base_addr, ep_index);
		break;

	case USBC_EP_TYPE_RX:
		__USBC_INT_ClearRxPending(usbc_otg->base_addr, ep_index);
		break;

	default:
		break;
	}

	return ;
}

/* clear ep's interrupt flag */
void USBC_INT_ClearEpPendingAll(__hdle hUSB, __u32 ep_type)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_EP0:
	case USBC_EP_TYPE_TX:
		__USBC_INT_ClearTxPendingAll(usbc_otg->base_addr);
		break;

	case USBC_EP_TYPE_RX:
		__USBC_INT_ClearRxPendingAll(usbc_otg->base_addr);
		break;

	default:
		break;
	}

	return ;
}

/* get usb misc's interrupt flag */
__u32 USBC_INT_MiscPending(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	return (USBC_Readb(USBC_REG_INTUSB(usbc_otg->base_addr)));
}

/* clear usb misc's interrupt flag */
void USBC_INT_ClearMiscPending(__hdle hUSB, __u32 mask)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_Writeb(mask, USBC_REG_INTUSB(usbc_otg->base_addr));
}

/* clear all the usb misc's interrupt flag */
void USBC_INT_ClearMiscPendingAll(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_Writeb(0xff, USBC_REG_INTUSB(usbc_otg->base_addr));
}

/* open a ep's interrupt */
void USBC_INT_EnableEp(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_TX:
		__USBC_INT_EnableTxEp(usbc_otg->base_addr, ep_index);
		break;

	case USBC_EP_TYPE_RX:
		__USBC_INT_EnableRxEp(usbc_otg->base_addr, ep_index);
		break;

	default:
		break;
	}

	return ;
}

/* open a usb misc's interrupt */
void USBC_INT_EnableUsbMiscUint(__hdle hUSB, __u32 mask)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if (usbc_otg == NULL) {
		return ;
	}

	reg_val = USBC_Readb(USBC_REG_INTUSBE(usbc_otg->base_addr));
	reg_val |= mask;
	USBC_Writeb(reg_val, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* close a ep's interrupt */
void USBC_INT_DisableEp(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_TX:
		__USBC_INT_DisableTxEp(usbc_otg->base_addr, ep_index);
		break;

	case USBC_EP_TYPE_RX:
		__USBC_INT_DisableRxEp(usbc_otg->base_addr, ep_index);
		break;

	default:
		break;
	}

	return;
}

/* close a usb misc's interrupt */
void USBC_INT_DisableUsbMiscUint(__hdle hUSB, __u32 mask)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if (usbc_otg == NULL) {
		return ;
	}

	reg_val = USBC_Readb(USBC_REG_INTUSBE(usbc_otg->base_addr));
	reg_val &= ~mask;
	USBC_Writeb(reg_val, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* close all ep's interrupt */
void USBC_INT_DisableEpAll(__hdle hUSB, __u32 ep_type)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(ep_type) {
	case USBC_EP_TYPE_TX:
		__USBC_INT_DisableTxAll(usbc_otg->base_addr);
		break;

	case USBC_EP_TYPE_RX:
		__USBC_INT_DisableRxAll(usbc_otg->base_addr);
		break;

	default:
		break;
	}

	return;
}

/* close all usb misc's interrupt */
void USBC_INT_DisableUsbMiscAll(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_Writeb(0, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* get current active ep */
__u32 USBC_GetActiveEp(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return 0;
	}

	return USBC_Readb(USBC_REG_EPIND(usbc_otg->base_addr));
}

/* set the active ep */
void USBC_SelectActiveEp(__hdle hUSB, __u8 ep_index)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_Writeb(ep_index, USBC_REG_EPIND(usbc_otg->base_addr));
}

/* enhance usb transfer signal */
void USBC_EnhanceSignal(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	return;
}

/* enter TestPacket mode */
void USBC_EnterMode_TestPacket(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_PACKET, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* enter Test_K mode */
void USBC_EnterMode_Test_K(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_K, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* enter Test_J mode */
void USBC_EnterMode_Test_J(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_J, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* enter Test_SE0_NAK mode */
void USBC_EnterMode_Test_SE0_NAK(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_SE0_NAK, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* clear all test mode */
void USBC_EnterMode_Idle(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_PACKET, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_K, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_J, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_SE0_NAK, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* vbus, id, dpdm, these bit is set 1 to clear, so we clear these bit when operate other bits */
static __u32 __USBC_WakeUp_ClearChangeDetect(__u32 reg_val)
{
	__u32 temp = reg_val;

	temp &= ~(1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_ID_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT);

	return temp;
}

void USBC_SetWakeUp_Default(__hdle hUSB)
{
	return;
}

void USBC_EnableIdPullUp(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	/* vbus, id, dpdm, these bit is set 1 to clear, so we clear these bit when operate other bits */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= (1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_DisableIdPullUp(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	/* vbus, id, dpdm, these bit is set 1 to clear, so we clear these bit when operate other bits */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_EnableDpDmPullUp(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	/* vbus, id, dpdm, these bit is set 1 to clear, so we clear these bit when operate other bits */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= (1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_DisableDpDmPullUp(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	/* vbus, id, dpdm, these bit is set 1 to clear, so we clear these bit when operate other bits */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

static void __USBC_ForceIdDisable(void __iomem *usbc_base_addr)
{
	__u32 reg_val = 0;

	/* vbus, id, dpdm, these bit is set 1 to clear, so we clear these bit when operate other bits */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceIdToLow(void __iomem *usbc_base_addr)
{
	__u32 reg_val = 0;

	/* first write 00, then write 10 */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x02 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceIdToHigh(void __iomem *usbc_base_addr)
{
	__u32 reg_val = 0;

	/* first write 00, then write 10 */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	//reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

/* force id to (id_type) */
void USBC_ForceId(__hdle hUSB, __u32 id_type)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	switch(id_type) {
	case USBC_ID_TYPE_HOST:
		__USBC_ForceIdToLow(usbc_otg->base_addr);
		break;

	case USBC_ID_TYPE_DEVICE:
		__USBC_ForceIdToHigh(usbc_otg->base_addr);
		break;

	default:
		__USBC_ForceIdDisable(usbc_otg->base_addr);
	}
}

static void __USBC_ForceVbusValidDisable(void __iomem *usbc_base_addr)
{
	__u32 reg_val = 0;

	/* first write 00, then write 10 */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceVbusValidToLow(void __iomem *usbc_base_addr)
{
	__u32 reg_val = 0;

	/* first write 00, then write 10 */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val |= (0x02 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceVbusValidToHigh(void __iomem *usbc_base_addr)
{
	__u32 reg_val = 0;

	/* first write 00, then write 11 */
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	//reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

/* force vbus valid to (id_type) */
void USBC_ForceVbusValid(__hdle hUSB, __u32 vbus_type)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return ;
	}

	switch(vbus_type) {
	case USBC_VBUS_TYPE_LOW:
		__USBC_ForceVbusValidToLow(usbc_otg->base_addr);
		break;

	case USBC_VBUS_TYPE_HIGH:
		__USBC_ForceVbusValidToHigh(usbc_otg->base_addr);
		break;

	default:
		__USBC_ForceVbusValidDisable(usbc_otg->base_addr);
	}
	return ;
}

void USBC_A_valid_InputSelect(__hdle hUSB, __u32 source)
{
	return;
}

void USBC_EnableUsbLineStateBypass(__hdle hUSB)
{
	return;
}

void USBC_DisableUsbLineStateBypass(__hdle hUSB)
{
	return;
}

void USBC_EnableHosc(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= 1 << USBC_BP_ISCR_HOSC_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* forbidde Hosc */
void USBC_DisableHosc(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_HOSC_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* check if vbus irq occur */
__u32 USBC_IsVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

	// when read the volatile bit, write 1 clear it synchronously.
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	reg_val |= 1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* check if id irq occur */
__u32 USBC_IsIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

	// when read the volatile bit, write 1 clear it synchronously.
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_ID_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	reg_val |= 1 << USBC_BP_ISCR_ID_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* check if dpdm irq occur */
__u32 USBC_IsDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

	// when read the volatile bit, write 1 clear it synchronously.
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	reg_val |= 1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* disable wake irq */
void USBC_DisableWakeIrq(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_IRQ_ENABLE);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* disable vbus irq */
void USBC_DisableVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* disable id irq */
void USBC_DisableIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_ID_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* disable dpdm irq */
void USBC_DisableDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* enable wake irq */
void USBC_EnableWakeIrq(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= 1 << USBC_BP_ISCR_IRQ_ENABLE;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* enable vbus irq */
void USBC_EnableVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= 1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* enable id irq */
void USBC_EnableIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= 1 << USBC_BP_ISCR_ID_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* enable dmdp irq */
void USBC_EnableDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= 1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* test mode, get the reg value */
__u32 USBC_TestMode_ReadReg(__hdle hUSB, __u32 offset, __u32 reg_width)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if (usbc_otg == NULL) {
		return reg_val;
	}

	if (reg_width == 8) {
		reg_val = USBC_Readb(usbc_otg->base_addr + offset);
	} else if (reg_width == 16) {
		reg_val = USBC_Readw(usbc_otg->base_addr + offset);
	} else if (reg_width == 32) {
		reg_val = USBC_Readl(usbc_otg->base_addr + offset);
	} else {
		reg_val = 0;
	}

	return reg_val;
}

__hdle USBC_open_otg(__u32 otg_no)
{
	__usbc_otg_t *usbc_otg = usbc_otg_array;

	usbc_otg->used      = 1;
	usbc_otg->no        = otg_no;
	usbc_otg->port_num  = otg_no;
	usbc_otg->base_addr = usbc_base_address;

	return (__hdle)(usbc_otg);
}

/*
 * release otg's usage
 * @hUSB: handle return by USBC_open_otg, include the key data which USBC need
 *
 * return 0 on success, !0 otherwise.
 */
__s32  USBC_close_otg(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if (usbc_otg == NULL) {
		return -1;
	}

	memset(usbc_otg, 0, sizeof(__usbc_otg_t));

	return 0;
}

__s32 USBC_init(bsp_usbc_t *usbc)
{
	if (usbc->usbc_info.base) {
		usbc_base_address = usbc->usbc_info.base;
	}

	return 0;
}

__s32 USBC_exit(bsp_usbc_t *usbc)
{
	__usbc_otg_t *usbc_otg = usbc_otg_array;

	memset(&usbc_info_g, 0, sizeof(__fifo_info_t));
	memset(usbc_otg, 0, (USBC_MAX_OPEN_NUM * sizeof(__usbc_otg_t)));

	return 0;
}

/* USB transfer type select, read/write ops. etc */
EXPORT_SYMBOL(USBC_OTG_SelectMode);

EXPORT_SYMBOL(USBC_ReadLenFromFifo);
EXPORT_SYMBOL(USBC_WritePacket);
EXPORT_SYMBOL(USBC_ReadPacket);

EXPORT_SYMBOL(USBC_ConfigFIFO_Base);
EXPORT_SYMBOL(USBC_GetPortFifoStartAddr);
EXPORT_SYMBOL(USBC_GetPortFifoSize);
EXPORT_SYMBOL(USBC_SelectFIFO);
EXPORT_SYMBOL(USBC_ConfigFifo_Default);
EXPORT_SYMBOL(USBC_ConfigFifo);

EXPORT_SYMBOL(USBC_SelectBus);

EXPORT_SYMBOL(USBC_GetActiveEp);
EXPORT_SYMBOL(USBC_SelectActiveEp);

EXPORT_SYMBOL(USBC_EnhanceSignal);

EXPORT_SYMBOL(USBC_GetLastFrameNumber);


/* usb interrupt ops */
EXPORT_SYMBOL(USBC_INT_EpPending);
EXPORT_SYMBOL(USBC_INT_MiscPending);
EXPORT_SYMBOL(USBC_INT_ClearEpPending);
EXPORT_SYMBOL(USBC_INT_ClearMiscPending);
EXPORT_SYMBOL(USBC_INT_ClearEpPendingAll);
EXPORT_SYMBOL(USBC_INT_ClearMiscPendingAll);

EXPORT_SYMBOL(USBC_INT_EnableEp);
EXPORT_SYMBOL(USBC_INT_EnableUsbMiscUint);

EXPORT_SYMBOL(USBC_INT_DisableEp);
EXPORT_SYMBOL(USBC_INT_DisableUsbMiscUint);

EXPORT_SYMBOL(USBC_INT_DisableEpAll);
EXPORT_SYMBOL(USBC_INT_DisableUsbMiscAll);

/* usb control ops */
EXPORT_SYMBOL(USBC_GetVbusStatus);
EXPORT_SYMBOL(USBC_GetStatus_Dp);
EXPORT_SYMBOL(USBC_GetStatus_Dm);
EXPORT_SYMBOL(USBC_GetStatus_DpDm);

EXPORT_SYMBOL(USBC_GetOtgMode_Form_ID);
EXPORT_SYMBOL(USBC_GetOtgMode_Form_BDevice);

EXPORT_SYMBOL(USBC_SetWakeUp_Default);

EXPORT_SYMBOL(USBC_EnableIdPullUp);
EXPORT_SYMBOL(USBC_DisableIdPullUp);
EXPORT_SYMBOL(USBC_EnableDpDmPullUp);
EXPORT_SYMBOL(USBC_DisableDpDmPullUp);

EXPORT_SYMBOL(USBC_ForceId);
EXPORT_SYMBOL(USBC_ForceVbusValid);

EXPORT_SYMBOL(USBC_A_valid_InputSelect);

EXPORT_SYMBOL(USBC_EnableUsbLineStateBypass);
EXPORT_SYMBOL(USBC_DisableUsbLineStateBypass);
EXPORT_SYMBOL(USBC_EnableHosc);
EXPORT_SYMBOL(USBC_DisableHosc);

EXPORT_SYMBOL(USBC_IsVbusChange);
EXPORT_SYMBOL(USBC_IsIdChange);
EXPORT_SYMBOL(USBC_IsDpDmChange);

EXPORT_SYMBOL(USBC_DisableWakeIrq);
EXPORT_SYMBOL(USBC_DisableVbusChange);
EXPORT_SYMBOL(USBC_DisableIdChange);
EXPORT_SYMBOL(USBC_DisableDpDmChange);

EXPORT_SYMBOL(USBC_EnableWakeIrq);
EXPORT_SYMBOL(USBC_EnableVbusChange);
EXPORT_SYMBOL(USBC_EnableIdChange);
EXPORT_SYMBOL(USBC_EnableDpDmChange);

/* usb test ops */
EXPORT_SYMBOL(USBC_EnterMode_TestPacket);
EXPORT_SYMBOL(USBC_EnterMode_Test_K);
EXPORT_SYMBOL(USBC_EnterMode_Test_J);
EXPORT_SYMBOL(USBC_EnterMode_Test_SE0_NAK);
EXPORT_SYMBOL(USBC_EnterMode_Idle);

EXPORT_SYMBOL(USBC_TestMode_ReadReg);

EXPORT_SYMBOL(USBC_open_otg);
EXPORT_SYMBOL(USBC_close_otg);
EXPORT_SYMBOL(USBC_init);
EXPORT_SYMBOL(USBC_exit);

