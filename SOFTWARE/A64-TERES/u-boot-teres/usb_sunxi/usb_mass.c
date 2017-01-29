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
#include "usb_base.h"
#include <scsi.h>
#include <asm/arch/dma.h>
#include <sys_partition.h>
#include "usb_mass.h"

static  int sunxi_usb_mass_write_enable = 0;
static  int sunxi_usb_mass_status = SUNXI_USB_MASS_IDLE;
#if defined(SUNXI_USB_30)
static  int sunxi_usb_mass_status_enable = 1;
#endif
static  mass_trans_set_t  trans_data;
/*
*******************************************************************************
*                     do_usb_req_set_interface
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static int __usb_set_interface(struct usb_device_request *req)
{
	sunxi_usb_dbg("set interface\n");
	/* Only support interface 0, alternate 0 */
	if((0 == req->wIndex) && (0 == req->wValue))
	{
		sunxi_udc_ep_reset();
	}
	else
	{
		printf("err: invalid wIndex and wValue, (0, %d), (0, %d)\n", req->wIndex, req->wValue);
		return SUNXI_USB_REQ_OP_ERR;
	}

	return SUNXI_USB_REQ_SUCCESSED;
}

/*
*******************************************************************************
*                     do_usb_req_set_address
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static int __usb_set_address(struct usb_device_request *req)
{
	uchar address;

	address = req->wValue & 0x7f;
	printf("set address 0x%x\n", address);

	sunxi_udc_set_address(address);

	return SUNXI_USB_REQ_SUCCESSED;
}

/*
*******************************************************************************
*                     do_usb_req_set_configuration
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static int __usb_set_configuration(struct usb_device_request *req)
{
	sunxi_usb_dbg("set configuration\n");
	/* Only support 1 configuration so nak anything else */
	if (1 == req->wValue)
	{
		sunxi_udc_ep_reset();
	}
	else
	{
		printf("err: invalid wValue, (0, %d)\n", req->wValue);

		return SUNXI_USB_REQ_OP_ERR;
	}
	sunxi_udc_set_configuration(req->wValue);

	return SUNXI_USB_REQ_SUCCESSED;
}
/*
*******************************************************************************
*                     do_usb_req_get_descriptor
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static int __usb_get_descriptor(struct usb_device_request *req, uchar *buffer)
{
	int ret = SUNXI_USB_REQ_SUCCESSED;

	//获取描述符
	switch(req->wValue >> 8)
	{
		case USB_DT_DEVICE:		//设备描述符
		{
			struct usb_device_descriptor *dev_dscrptr;

			sunxi_usb_dbg("get device descriptor\n");

			dev_dscrptr = (struct usb_device_descriptor *)buffer;
			memset((void *)dev_dscrptr, 0, sizeof(struct usb_device_descriptor));

			dev_dscrptr->bLength = MIN(req->wLength, sizeof (struct usb_device_descriptor));
			dev_dscrptr->bDescriptorType    = USB_DT_DEVICE;
#ifdef CONFIG_USB_1_1_DEVICE
			dev_dscrptr->bcdUSB             = 0x110;
#else
			dev_dscrptr->bcdUSB             = 0x200;
#endif
			dev_dscrptr->bDeviceClass       = 0;		//设备类：大容量存储
			dev_dscrptr->bDeviceSubClass    = 0;
			dev_dscrptr->bDeviceProtocol    = 0;
			dev_dscrptr->bMaxPacketSize0    = 0x40;
			dev_dscrptr->idVendor           = 0xBB4;
			dev_dscrptr->idProduct          = 0xfff;
			dev_dscrptr->bcdDevice          = 0x200;
			dev_dscrptr->iManufacturer      = SUNXI_USB_STRING_IMANUFACTURER;
			dev_dscrptr->iProduct           = SUNXI_USB_STRING_IPRODUCT;
			dev_dscrptr->iSerialNumber      = SUNXI_USB_STRING_ISERIALNUMBER;
			dev_dscrptr->bNumConfigurations = 1;

			sunxi_udc_send_setup(dev_dscrptr->bLength, buffer);
		}
		break;

		case USB_DT_CONFIG:		//配置描述符
		{
			struct usb_configuration_descriptor *config_dscrptr;
			struct usb_interface_descriptor 	*inter_dscrptr;
			struct usb_endpoint_descriptor 		*ep_in, *ep_out;
			unsigned char bytes_remaining = req->wLength;
			unsigned char bytes_total = 0;

			sunxi_usb_dbg("get config descriptor\n");

			bytes_total = sizeof (struct usb_configuration_descriptor) + \
						  sizeof (struct usb_interface_descriptor) 	   + \
						  sizeof (struct usb_endpoint_descriptor) 	   + \
						  sizeof (struct usb_endpoint_descriptor);

			memset(buffer, 0, bytes_total);

			config_dscrptr = (struct usb_configuration_descriptor *)(buffer + 0);
			inter_dscrptr  = (struct usb_interface_descriptor 	  *)(buffer + 						\
																	 sizeof(struct usb_configuration_descriptor));
			ep_in 		   = (struct usb_endpoint_descriptor 	  *)(buffer + 						\
																	 sizeof(struct usb_configuration_descriptor) + 	\
																	 sizeof(struct usb_interface_descriptor));
			ep_out 		   = (struct usb_endpoint_descriptor 	  *)(buffer + 						\
																	 sizeof(struct usb_configuration_descriptor) + 	\
																	 sizeof(struct usb_interface_descriptor)	 +	\
																	 sizeof(struct usb_endpoint_descriptor));

			/* configuration */
			config_dscrptr->bLength            	= MIN(bytes_remaining, sizeof (struct usb_configuration_descriptor));
			config_dscrptr->bDescriptorType    	= USB_DT_CONFIG;
			config_dscrptr->wTotalLength 		= bytes_total;
			config_dscrptr->bNumInterfaces     	= 1;
			config_dscrptr->bConfigurationValue	= 1;
			config_dscrptr->iConfiguration     	= 0;
			config_dscrptr->bmAttributes       	= 0xc0;
			config_dscrptr->bMaxPower          	= 0xFA;		//最大电流500ms(0xfa * 2)

			bytes_remaining 				   -= config_dscrptr->bLength;
			/* interface */
			inter_dscrptr->bLength             = MIN (bytes_remaining, sizeof(struct usb_interface_descriptor));
			inter_dscrptr->bDescriptorType     = USB_DT_INTERFACE;
			inter_dscrptr->bInterfaceNumber    = 0x00;
			inter_dscrptr->bAlternateSetting   = 0x00;
			inter_dscrptr->bNumEndpoints       = 0x02;
			inter_dscrptr->bInterfaceClass     = USB_CLASS_MASS_STORAGE;		//mass storage
			inter_dscrptr->bInterfaceSubClass  = US_SC_SCSI;
			inter_dscrptr->bInterfaceProtocol  = US_PR_BULK;
			inter_dscrptr->iInterface          = 1;

			bytes_remaining 				  -= inter_dscrptr->bLength;
			/* ep_in */
			ep_in->bLength            = MIN (bytes_remaining, sizeof (struct usb_endpoint_descriptor));
			ep_in->bDescriptorType    = USB_DT_ENDPOINT;
			ep_in->bEndpointAddress   = sunxi_udc_get_ep_in_type(); /* IN */
			ep_in->bmAttributes       = USB_ENDPOINT_XFER_BULK;
			ep_in->wMaxPacketSize 	  = sunxi_udc_get_ep_max();
			ep_in->bInterval          = 0x00;

			bytes_remaining 		 -= ep_in->bLength;
			/* ep_out */
			ep_out->bLength            = MIN (bytes_remaining, sizeof (struct usb_endpoint_descriptor));
			ep_out->bDescriptorType    = USB_DT_ENDPOINT;
			ep_out->bEndpointAddress   = sunxi_udc_get_ep_out_type(); /* OUT */
			ep_out->bmAttributes       = USB_ENDPOINT_XFER_BULK;
			ep_out->wMaxPacketSize 	   = sunxi_udc_get_ep_max();
			ep_out->bInterval          = 0x00;

			bytes_remaining 		  -= ep_out->bLength;

			sunxi_udc_send_setup(MIN(req->wLength, bytes_total), buffer);
		}
		break;

		case USB_DT_STRING:
		{
			unsigned char bLength = 0;
			unsigned char string_index = req->wValue & 0xff;

			sunxi_usb_dbg("get string descriptor\n");

			/* Language ID */
			if(string_index == 0)
			{
				bLength = MIN(4, req->wLength);

				sunxi_udc_send_setup(bLength, (void *)sunxi_usb_mass_dev[0]);
			}
			else if(string_index < SUNXI_USB_MASS_DEV_MAX)
			{
				/* Size of string in chars */
				unsigned char i = 0;
				unsigned char str_length = strlen ((const char *)sunxi_usb_mass_dev[string_index]);
				unsigned char bLength = 2 + (2 * str_length);

				buffer[0] = bLength;        /* length */
				buffer[1] = USB_DT_STRING;  /* descriptor = string */

				/* Copy device string to fifo, expand to simple unicode */
				for(i = 0; i < str_length; i++)
				{
					buffer[2+ 2*i + 0] = sunxi_usb_mass_dev[string_index][i];
					buffer[2+ 2*i + 1] = 0;
				}
				bLength = MIN(bLength, req->wLength);

				sunxi_udc_send_setup(bLength, buffer);
			}
			else
			{
				printf("sunxi usb err: string line %d is not supported\n", string_index);
			}
		}
		break;

		case USB_DT_DEVICE_QUALIFIER:
		{
#ifdef CONFIG_USB_1_1_DEVICE
			/* This is an invalid request for usb 1.1, nak it */
			USBC_Dev_EpSendStall(sunxi_udc_source.usbc_hd, USBC_EP_TYPE_EP0);
#else
			struct usb_qualifier_descriptor *qua_dscrpt;

			sunxi_usb_dbg("get qualifier descriptor\n");

			qua_dscrpt = (struct usb_qualifier_descriptor *)buffer;
			memset(&buffer, 0, sizeof(struct usb_qualifier_descriptor));

			qua_dscrpt->bLength = MIN(req->wLength, sizeof(sizeof(struct usb_qualifier_descriptor)));
			qua_dscrpt->bDescriptorType    = USB_DT_DEVICE_QUALIFIER;
			qua_dscrpt->bcdUSB             = 0x200;
			qua_dscrpt->bDeviceClass       = 0xff;
			qua_dscrpt->bDeviceSubClass    = 0xff;
			qua_dscrpt->bDeviceProtocol    = 0xff;
			qua_dscrpt->bMaxPacketSize0    = 0x40;
			qua_dscrpt->bNumConfigurations = 1;
			qua_dscrpt->breserved          = 0;

			sunxi_udc_send_setup(qua_dscrpt->bLength, buffer);
#endif
		}
		break;

		default:
			printf("err: unkown wValue(%d)\n", req->wValue);

			ret = SUNXI_USB_REQ_OP_ERR;
	}

	return ret;
}

/*
*******************************************************************************
*                     do_usb_req_get_status
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static int __usb_get_status(struct usb_device_request *req, uchar *buffer)
{
	unsigned char bLength = 0;

	sunxi_usb_dbg("get status\n");
	if(0 == req->wLength)
	{
		/* sent zero packet */
		sunxi_udc_send_setup(0, NULL);

		return SUNXI_USB_REQ_OP_ERR;
	}

	bLength = MIN(req->wValue, 2);

	buffer[0] = 1;
	buffer[1] = 0;

	sunxi_udc_send_setup(bLength, buffer);

	return SUNXI_USB_REQ_SUCCESSED;
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
static int __sunxi_mass_send_status(void *buffer, unsigned int buffer_size)
{
	return sunxi_udc_send_data((uchar *)buffer, buffer_size);
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
static int sunxi_mass_init(void)
{
	sunxi_usb_dbg("sunxi_mass_init\n");
	memset(&trans_data, 0, sizeof(mass_trans_set_t));
	sunxi_usb_mass_write_enable = 0;
    sunxi_usb_mass_status = SUNXI_USB_MASS_IDLE;

    trans_data.base_recv_buffer = (u8 *)malloc(SUNXI_MASS_RECV_MEM_SIZE);
    if(!trans_data.base_recv_buffer)
    {
    	printf("sunxi usb mass err: unable to malloc memory for mass receive\n");

    	return -1;
    }

	trans_data.base_send_buffer = (u8 *)malloc(SUNXI_MASS_SEND_MEM_SIZE);
    if(!trans_data.base_send_buffer)
    {
    	printf("sunxi usb mass err: unable to malloc memory for mass send\n");
    	free(trans_data.base_recv_buffer);

    	return -1;
    }
	sunxi_usb_dbg("recv addr 0x%x\n", (uint)trans_data.base_recv_buffer);
    sunxi_usb_dbg("send addr 0x%x\n", (uint)trans_data.base_send_buffer);

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
static int sunxi_mass_exit(void)
{
	sunxi_usb_dbg("sunxi_mass_exit\n");
    if(trans_data.base_recv_buffer)
    {
    	free(trans_data.base_recv_buffer);
    }
    if(trans_data.base_send_buffer)
    {
    	free(trans_data.base_send_buffer);
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
static void sunxi_mass_reset(void)
{
	sunxi_usb_mass_write_enable = 0;
    sunxi_usb_mass_status = SUNXI_USB_MASS_IDLE;
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
static void  sunxi_mass_usb_rx_dma_isr(void *p_arg)
{
	sunxi_usb_dbg("dma int for usb rx occur\n");
	//通知主循环，可以写入数据
	sunxi_usb_mass_write_enable = 1;
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
static void  sunxi_mass_usb_tx_dma_isr(void *p_arg)
{
	sunxi_usb_dbg("dma int for usb tx occur\n");
#if defined(SUNXI_USB_30)
	sunxi_usb_mass_status_enable = 1;
#endif
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
static int sunxi_mass_standard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer)
{
	int ret = SUNXI_USB_REQ_OP_ERR;

	switch(cmd)
	{
		case USB_REQ_GET_STATUS:
		{
			ret = __usb_get_status(req, buffer);

			break;
		}
		//case USB_REQ_CLEAR_FEATURE:
		//case USB_REQ_SET_FEATURE:
		case USB_REQ_SET_ADDRESS:
		{
			ret = __usb_set_address(req);

			break;
		}
		case USB_REQ_GET_DESCRIPTOR:
		//case USB_REQ_SET_DESCRIPTOR:
		case USB_REQ_GET_CONFIGURATION:
		{
			ret = __usb_get_descriptor(req, buffer);

			break;
		}
		case USB_REQ_SET_CONFIGURATION:
		{
			ret = __usb_set_configuration(req);

			break;
		}
		//case USB_REQ_GET_INTERFACE:
		case USB_REQ_SET_INTERFACE:
		{
			ret = __usb_set_interface(req);

			break;
		}
		//case USB_REQ_SYNCH_FRAME:
		default:
		{
			printf("sunxi mass error: standard req is not supported\n");

			ret = SUNXI_USB_REQ_DEVICE_NOT_SUPPORTED;

			break;
		}
	}

	return ret;
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
static int sunxi_mass_nonstandard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer, uint data_status)
{
	int ret = SUNXI_USB_REQ_SUCCESSED;

	switch(req->bmRequestType)		//MASS 特有请求
	{
		case 161:
			if(req->bRequest == 0xFE)
			{
				sunxi_usb_dbg("mass ask for max lun\n");

				buffer[0] = 0;

				sunxi_udc_send_setup(1, buffer);
			}
			else
			{
				printf("sunxi usb err: unknown ep0 req in mass\n");

				ret = SUNXI_USB_REQ_DEVICE_NOT_SUPPORTED;
			}
			break;

		default:
			printf("sunxi usb err: unknown non standard ep0 req\n");

			ret = SUNXI_USB_REQ_DEVICE_NOT_SUPPORTED;

			break;
	}

	return ret;
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
static int sunxi_mass_state_loop(void  *buffer)
{
	static struct umass_bbb_cbw_t  *cbw;
	static struct umass_bbb_csw_t  csw;
	static uint mass_flash_start = 0;
	static uint mass_flash_sectors = 0;
	int    ret;
	sunxi_ubuf_t *sunxi_ubuf = (sunxi_ubuf_t *)buffer;

	switch(sunxi_usb_mass_status)
	{
		case SUNXI_USB_MASS_IDLE:
			if(sunxi_ubuf->rx_ready_for_data == 1)
			{
				sunxi_usb_mass_status = SUNXI_USB_MASS_SETUP;
			}

			break;

		case SUNXI_USB_MASS_SETUP:

			sunxi_usb_dbg("SUNXI_USB_MASS_SETUP\n");

			if(sunxi_ubuf->rx_req_length != sizeof(struct umass_bbb_cbw_t))
			{
				printf("sunxi usb error: received bytes 0x%x is not equal cbw struct size 0x%lx\n", sunxi_ubuf->rx_req_length, (long)sizeof(struct umass_bbb_cbw_t));

				sunxi_ubuf->rx_ready_for_data = 0;
				sunxi_usb_mass_status = SUNXI_USB_MASS_IDLE;

				break;
			}

			cbw = (struct umass_bbb_cbw_t *)sunxi_ubuf->rx_req_buffer;
			if(CBWSIGNATURE != cbw->dCBWSignature)
			{
				printf("sunxi usb error: the cbw signature 0x%x is bad, need 0x%x\n", cbw->dCBWSignature, CBWSIGNATURE);

				sunxi_ubuf->rx_ready_for_data = 0;
				sunxi_usb_mass_status = SUNXI_USB_MASS_IDLE;

				break;
			}

			csw.dCSWSignature = CSWSIGNATURE;
			csw.dCSWTag 	  = cbw->dCBWTag;

			sunxi_usb_dbg("usb cbw command = 0x%x\n", cbw->CBWCDB[0]);
#if defined(SUNXI_USB_30)
			sunxi_usb_mass_status_enable = 1;
#endif

			switch(cbw->CBWCDB[0])
	  		{
	  			case SCSI_TST_U_RDY:
	  				sunxi_usb_dbg("SCSI_TST_U_RDY\n");

	  				csw.bCSWStatus = 0;
					sunxi_usb_mass_status = SUNXI_USB_MASS_STATUS;

	  				break;

	  			case SCSI_REQ_SENSE:
	  				sunxi_usb_dbg("SCSI_REQ_SENSE\n");
					sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);

					trans_data.send_size = min(cbw->dCBWDataTransferLength, 18);
					trans_data.act_send_buffer = (uchar *)RequestSense;

					csw.bCSWStatus = 0;
					sunxi_usb_mass_status = SUNXI_USB_MASS_SEND_DATA;

					break;

				case SCSI_VERIFY:
					sunxi_usb_dbg("SCSI_VERIFY\n");
					sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);

					csw.bCSWStatus = 0;
					sunxi_usb_mass_status = SUNXI_USB_MASS_STATUS;

					break;

	  			case SCSI_INQUIRY:
					sunxi_usb_dbg("SCSI_INQUIRY\n");
					sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);

					trans_data.send_size = min(cbw->dCBWDataTransferLength, 36);
					trans_data.act_send_buffer = (uchar *)InquiryData;

					csw.bCSWStatus = 0;
					sunxi_usb_mass_status = SUNXI_USB_MASS_SEND_DATA;

					break;

				case SCSI_MODE_SEN6:
					sunxi_usb_dbg("SCSI_MODE_SEN6\n");
					sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);
					{
						trans_data.base_send_buffer[0] = 3;
						trans_data.base_send_buffer[1] = 0;		//介质类型，为0
						trans_data.base_send_buffer[2] = 0;		//设备标识参数, 为0
						trans_data.base_send_buffer[3] = 0;		//块描述符长度，可以为0

						trans_data.act_send_buffer = trans_data.base_send_buffer;
						trans_data.send_size = min(cbw->dCBWDataTransferLength, 4);
					}

					csw.bCSWStatus = 0;
					sunxi_usb_mass_status = SUNXI_USB_MASS_SEND_DATA;

					break;

				case SCSI_RD_CAPAC:
					sunxi_usb_dbg("SCSI_RD_CAPAC\n");
					sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);
					{
						memset(trans_data.base_send_buffer, 0, 8);

						trans_data.base_send_buffer[2] = 0x80;
						trans_data.base_send_buffer[6] = 2;

						trans_data.act_send_buffer = trans_data.base_send_buffer;
						trans_data.send_size 	   = min(cbw->dCBWDataTransferLength, 8);
					}

					csw.bCSWStatus = 0;
					sunxi_usb_mass_status = SUNXI_USB_MASS_SEND_DATA;

					break;

	  			case SCSI_RD_FMT_CAPAC:
	  				sunxi_usb_dbg("SCSI_RD_FMT_CAPAC\n");
	  				sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);
					{
						memset(trans_data.base_send_buffer, 0, 12);

						trans_data.base_send_buffer[2] = 0x80;
						trans_data.base_send_buffer[6] = 2;
						trans_data.base_send_buffer[8] = 2;
						trans_data.base_send_buffer[10] = 2;

						trans_data.act_send_buffer = trans_data.base_send_buffer;
						trans_data.send_size 	   = min(cbw->dCBWDataTransferLength, 12);
					}

					csw.bCSWStatus = 0;
					sunxi_usb_mass_status = SUNXI_USB_MASS_SEND_DATA;

					break;
				case SCSI_MED_REMOVL:
	  				sunxi_usb_dbg("SCSI_MED_REMOVL\n");

	  				csw.bCSWStatus = 0;
	  				sunxi_usb_mass_status = SUNXI_USB_MASS_STATUS;

					break;
				case SCSI_READ6:
				case SCSI_READ10:		//HOST READ, not this device read
					sunxi_usb_dbg("SCSI_READ\n");
	  				sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);
					{
						uint start, sectors;
						uint offset;

						start = (cbw->CBWCDB[2]<<24) | cbw->CBWCDB[3]<<16 | cbw->CBWCDB[4]<<8 | cbw->CBWCDB[5]<<0;
						sectors = (cbw->CBWCDB[7]<<8) | cbw->CBWCDB[8];
						sunxi_usb_dbg("read start: 0x%x, sectors 0x%x\n", start, sectors);

						trans_data.send_size 	   = min(cbw->dCBWDataTransferLength, sectors * 512);
						trans_data.act_send_buffer = trans_data.base_send_buffer;
						offset = sunxi_partition_get_offset(0);
						ret = sunxi_flash_read(start + offset, sectors, trans_data.base_send_buffer);
						if(!ret)
						{
							printf("sunxi flash read err: start,0x%x sectors 0x%x\n", start, sectors);

							csw.bCSWStatus = 1;
						}
						else
						{
							csw.bCSWStatus = 0;
						}

						sunxi_usb_mass_status = SUNXI_USB_MASS_SEND_DATA;
					}

	  				break;

	  			case SCSI_WRITE6:
				case SCSI_WRITE10:		//HOST WRITE, not this device write
					sunxi_usb_dbg("SCSI_WRITE\n");
	  				sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);

					mass_flash_start   = (cbw->CBWCDB[2]<<24) | cbw->CBWCDB[3]<<16 | cbw->CBWCDB[4]<<8 | cbw->CBWCDB[5]<<0;
					mass_flash_sectors = (cbw->CBWCDB[7]<<8) | cbw->CBWCDB[8];
					sunxi_usb_dbg("command write start: 0x%x, sectors 0x%x\n", mass_flash_start, mass_flash_sectors);

					trans_data.recv_size 	   = min(cbw->dCBWDataTransferLength, mass_flash_sectors * 512);
					trans_data.act_recv_buffer = trans_data.base_recv_buffer;

					mass_flash_start += sunxi_partition_get_offset(0);
					sunxi_usb_dbg("try to receive data 0x%x\n", trans_data.recv_size);

					sunxi_usb_mass_write_enable = 0;
					sunxi_udc_start_recv_by_dma(trans_data.act_recv_buffer, trans_data.recv_size);	//start dma to receive data

					sunxi_usb_mass_status = SUNXI_USB_MASS_RECEIVE_DATA;

	  				break;

	  			default:
	  				sunxi_usb_dbg("not supported command 0x%x now\n", cbw->CBWCDB[0]);
	  				sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);

	  				csw.bCSWStatus = 1;

	  				sunxi_usb_mass_status = SUNXI_USB_MASS_STATUS;

	  				break;
	  		}

	  		break;

	  	case SUNXI_USB_MASS_SEND_DATA:

	  		sunxi_usb_dbg("SUNXI_USB_SEND_DATA\n");

#if defined(SUNXI_USB_30)
			sunxi_usb_mass_status_enable = 0;
#endif
			sunxi_usb_mass_status = SUNXI_USB_MASS_STATUS;
			sunxi_udc_send_data((void *)trans_data.act_send_buffer, trans_data.send_size);

	  		break;

	  	case SUNXI_USB_MASS_RECEIVE_DATA:

	  		sunxi_usb_dbg("SUNXI_USB_RECEIVE_DATA\n");

			if(sunxi_usb_mass_write_enable == 1)
			{
				sunxi_usb_dbg("write flash, start 0x%x, sectors 0x%x\n", mass_flash_start, trans_data.recv_size/512);
				ret = sunxi_flash_write(mass_flash_start, trans_data.recv_size/512, (void *)trans_data.act_recv_buffer);
				if(!ret)
				{
					printf("sunxi flash write err: start,0x%x sectors 0x%x\n", mass_flash_start, trans_data.recv_size/512);

					csw.bCSWStatus = 1;
				}
				else
				{
					csw.bCSWStatus = 0;
  				}
				sunxi_usb_mass_write_enable = 0;

  				sunxi_usb_mass_status = SUNXI_USB_MASS_STATUS;
			}

	  		break;

		case SUNXI_USB_MASS_STATUS:

			sunxi_usb_dbg("SUNXI_USB_MASS_STATUS\n");
#if defined(SUNXI_USB_30)
			if(sunxi_usb_mass_status_enable)
#endif
			{
				sunxi_usb_mass_status = SUNXI_USB_MASS_IDLE;
				sunxi_ubuf->rx_ready_for_data = 0;
				__sunxi_mass_send_status(&csw, sizeof(struct umass_bbb_csw_t));
			}

			break;
	  	default:
	  		break;
	}

	return 0;
}


sunxi_usb_module_init(SUNXI_USB_DEVICE_MASS,					\
					  sunxi_mass_init,							\
					  sunxi_mass_exit,							\
					  sunxi_mass_reset,							\
					  sunxi_mass_standard_req_op,				\
					  sunxi_mass_nonstandard_req_op,			\
					  sunxi_mass_state_loop,					\
					  sunxi_mass_usb_rx_dma_isr,				\
					  sunxi_mass_usb_tx_dma_isr					\
					  );
