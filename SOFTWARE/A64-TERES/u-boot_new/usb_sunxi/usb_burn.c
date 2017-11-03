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
#include <boot_type.h>
#include "usb_burn.h"
#include "usb_efex.h"
#include <smc.h>
#include <securestorage.h>
#include <sunxi_board.h>
#include "key_deal.h"

int __smc_set_sst_crypt_name(char *name)
{
	printf("call weak fun: %s\n",__func__);
	return 0;
}

int smc_set_sst_crypt_name(char *name)

__attribute__((weak, alias("__smc_set_sst_crypt_name")));



static  int sunxi_usb_pburn_write_enable = 0;
#if defined(SUNXI_USB_30)
static  int sunxi_usb_pburn_status_enable = 1;
#endif
static  int sunxi_usb_pburn_status = SUNXI_USB_PBURN_IDLE;

static  pburn_trans_set_t  trans_data;
#ifdef  CONFIG_SUNXI_SECURE_STORAGE
static  u8 *private_data_ext_buff_step, *private_data_ext_buff;
#endif
extern volatile int sunxi_usb_burn_from_boot_handshake;
extern volatile int sunxi_usb_burn_from_boot_setup;
static uint burn_private_start, burn_private_len;
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
			dev_dscrptr->bDeviceClass       = 0;
			dev_dscrptr->bDeviceSubClass    = 0;
			dev_dscrptr->bDeviceProtocol    = 0;
			dev_dscrptr->bMaxPacketSize0    = 0x40;
			dev_dscrptr->idVendor           = DRIVER_VENDOR_ID;
			dev_dscrptr->idProduct          = DRIVER_PRODUCT_ID;
			dev_dscrptr->bcdDevice          = 0xffff;
			//ignored
			//dev_dscrptr->iManufacturer      = SUNXI_USB_STRING_IMANUFACTURER;
			//dev_dscrptr->iProduct           = SUNXI_USB_STRING_IPRODUCT;
			//dev_dscrptr->iSerialNumber      = SUNXI_USB_STRING_ISERIALNUMBER;
			dev_dscrptr->iManufacturer      = 0;
			dev_dscrptr->iProduct           = 0;
			dev_dscrptr->iSerialNumber      = 0;
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
			config_dscrptr->bmAttributes       	= 0x80;		//not self powered
			config_dscrptr->bMaxPower          	= 0xFA;		//最大电流500ms(0xfa * 2)

			bytes_remaining 				   -= config_dscrptr->bLength;
			/* interface */
			inter_dscrptr->bLength             = MIN (bytes_remaining, sizeof(struct usb_interface_descriptor));
			inter_dscrptr->bDescriptorType     = USB_DT_INTERFACE;
			inter_dscrptr->bInterfaceNumber    = 0x00;
			inter_dscrptr->bAlternateSetting   = 0x00;
			inter_dscrptr->bNumEndpoints       = 0x02;
			inter_dscrptr->bInterfaceClass     = 0xff;
			inter_dscrptr->bInterfaceSubClass  = 0xff;
			inter_dscrptr->bInterfaceProtocol  = 0xff;
			inter_dscrptr->iInterface          = 0;

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

				buffer[0] = bLength;
				buffer[1] = USB_DT_STRING;
				buffer[2] = 9;
				buffer[3] = 4;
				sunxi_udc_send_setup(bLength, (void *)buffer);
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
static int __sunxi_pburn_send_status(void *buffer, unsigned int buffer_size)
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
#ifdef CONFIG_SUNXI_SECURE_STORAGE
int __sunxi_burn_key(u8 *buff, uint buff_len)
{
	sunxi_usb_burn_main_info_t *key_main = (sunxi_usb_burn_main_info_t *)buff;
	sunxi_usb_burn_key_info_t  *key_list;
#ifdef CONFIG_SUNXI_SECURE_SYSTEM
	sunxi_efuse_key_info_t      efuse_key_info;
#endif
	int key_count;
	int offset;
	u8  *p_buff = (u8 *)&key_main->key_info;
	//比较key主体的信息
	if(strcmp((const char *)key_main->magic, "key-group-db"))
	{
		printf("key data magic unmatch, err\n");

		return -1;
	}
	key_count = key_main->count;
	printf("key_count=%d\n", key_count);
	if(sunxi_secure_storage_init())
	{
		printf("%s secure storage init failed\n", __func__);

		return -1;
	}
	for(;key_count>0;key_count--, key_list++)
	{
		key_list = (sunxi_usb_burn_key_info_t *)p_buff;
		printf("^^^^^^^^^^^^^^^^^^^\n");
		printf("key index=%d\n", key_main->count - key_count);
		printf("key name=%s\n", key_list->name);
		printf("key type=%d\n", key_list->type);
		printf("key len=%d\n", key_list->len);
		printf("key if_burn=%d\n", key_list->if_burn);
		printf("key if_replace=%d\n", key_list->if_replace);
		printf("key if_crypt=%d\n", key_list->if_crypt);
		printf("key data:\n");
		sunxi_dump(key_list->key_data, key_list->len);
		printf("###################\n");
		offset = (sizeof(sunxi_usb_burn_key_info_t)) + ((key_list->len + 15) & (~15));
		printf("offset=%d\n", offset);
		p_buff += offset;
#ifdef CONFIG_SUNXI_SECURE_SYSTEM
		if(!key_list->type)
		{
			memset(&efuse_key_info, 0, sizeof(efuse_key_info));
			strcpy(efuse_key_info.name, (const char *)key_list->name);
			efuse_key_info.len = key_list->len;
			efuse_key_info.key_data = (u8 *)key_list->key_data;

			if(smc_efuse_writel(&efuse_key_info))
			{
				return -1;
			}
		}
		else
#endif
		{
#ifdef CONFIG_SUNXI_HDCP_IN_SECURESTORAGE
			int ret;

			if(!strcmp("hdcpkey", key_list->name))
			{
				ret = sunxi_deal_hdcp_key((char *)key_list->key_data, key_list->len);
				if(ret)
				{
					printf("sunxi deal with hdcp key failed\n");

					return -1;
				}
			}
			else
#endif
			{
			if(key_list->if_crypt)
				smc_set_sst_crypt_name(key_list->name);

				if(sunxi_secure_object_write(key_list->name, (char *)key_list->key_data, key_list->len))
				{
					return -1;
				}
			}
		}
	}
	if(sunxi_secure_storage_exit())
	{
		printf("sunxi_secure_storage_exit err\n");

		return -1;
	}

	return 0;
}
#endif
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
static int sunxi_pburn_init(void)
{
	sunxi_usb_dbg("sunxi_pburn_init\n");
	memset(&trans_data, 0, sizeof(pburn_trans_set_t));
	sunxi_usb_pburn_write_enable = 0;
    sunxi_usb_pburn_status = SUNXI_USB_PBURN_IDLE;

    trans_data.base_recv_buffer = (u8 *)malloc(SUNXI_PBURN_RECV_MEM_SIZE);
    if(!trans_data.base_recv_buffer)
    {
    	printf("sunxi usb pburn err: unable to malloc memory for pburn receive\n");

    	return -1;
    }

	trans_data.base_send_buffer = (u8 *)malloc(SUNXI_PBURN_SEND_MEM_SIZE);
    if(!trans_data.base_send_buffer)
    {
    	printf("sunxi usb pburn err: unable to malloc memory for pburn send\n");
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
static int sunxi_pburn_exit(void)
{
	sunxi_usb_dbg("sunxi_pburn_exit\n");
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
static void sunxi_pburn_reset(void)
{
	sunxi_usb_pburn_write_enable = 0;
    sunxi_usb_pburn_status = SUNXI_USB_PBURN_IDLE;
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
static void  sunxi_pburn_usb_rx_dma_isr(void *p_arg)
{
	sunxi_usb_dbg("dma int for usb rx occur\n");
	//通知主循环，可以写入数据
	sunxi_usb_pburn_write_enable = 1;
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
static void  sunxi_pburn_usb_tx_dma_isr(void *p_arg)
{
	sunxi_usb_dbg("dma int for usb tx occur\n");
#if defined(SUNXI_USB_30)
	sunxi_usb_pburn_status_enable = 1;
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
static int sunxi_pburn_standard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer)
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
			printf("sunxi pburn error: standard req is not supported\n");

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
static int sunxi_pburn_nonstandard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer, uint data_status)
{
	int ret = SUNXI_USB_REQ_SUCCESSED;

	switch(req->bmRequestType)		//PBURN 特有请求
	{
		case 161:
			if(req->bRequest == 0xFE)
			{
				sunxi_usb_dbg("pburn ask for max lun\n");

				buffer[0] = 0;

				sunxi_udc_send_setup(1, buffer);
			}
			else
			{
				printf("sunxi usb err: unknown ep0 req in pburn\n");

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
static int sunxi_pburn_state_loop(void  *buffer)
{
	static struct umass_bbb_cbw_t  *cbw;
	static struct umass_bbb_csw_t  csw;
	static uint pburn_flash_start = 0;
	//static uint pburn_flash_sectors = 0;
	int    ret;
	sunxi_ubuf_t *sunxi_ubuf = (sunxi_ubuf_t *)buffer;

	switch(sunxi_usb_pburn_status)
	{
		case SUNXI_USB_PBURN_IDLE:
			if(sunxi_ubuf->rx_ready_for_data == 1)
			{
				sunxi_usb_pburn_status = SUNXI_USB_PBURN_SETUP;
			}

			break;

		case SUNXI_USB_PBURN_SETUP:

			sunxi_usb_dbg("SUNXI_USB_PBURN_SETUP\n");

			if(sunxi_ubuf->rx_req_length != sizeof(struct umass_bbb_cbw_t))
			{
				printf("sunxi usb error: received bytes 0x%x is not equal cbw struct size 0x%zx\n", sunxi_ubuf->rx_req_length, sizeof(struct umass_bbb_cbw_t));

				sunxi_ubuf->rx_ready_for_data = 0;
				sunxi_usb_pburn_status = SUNXI_USB_PBURN_IDLE;

				break;
			}

			cbw = (struct umass_bbb_cbw_t *)sunxi_ubuf->rx_req_buffer;
			if(CBWSIGNATURE != cbw->dCBWSignature)
			{
				printf("sunxi usb error: the cbw signature 0x%x is bad, need 0x%x\n", cbw->dCBWSignature, CBWSIGNATURE);

				sunxi_ubuf->rx_ready_for_data = 0;
				sunxi_usb_pburn_status = SUNXI_USB_PBURN_IDLE;

				break;
			}

			csw.dCSWSignature = CSWSIGNATURE;
			csw.dCSWTag 	  = cbw->dCBWTag;

#if defined(SUNXI_USB_30)
			sunxi_usb_pburn_status_enable = 1;
#endif
			sunxi_usb_dbg("usb cbw command = 0x%x\n", cbw->CBWCDB[0]);

			switch(cbw->CBWCDB[0])
	  		{
#ifdef  CONFIG_SUNXI_SECURE_STORAGE
				case 0xf0:			//自定义命令，用于烧录用户数据
	  				sunxi_usb_dbg("usb burn secure storage data\n");
	  				printf("usb command = %d\n", cbw->CBWCDB[1]);
	  				switch(cbw->CBWCDB[1])
	  				{
	  					case 0:				//握手
	  					{
	  						__usb_handshake_sec_t  *handshake = (__usb_handshake_sec_t *)trans_data.base_send_buffer;

                            memset(handshake, 0, sizeof(__usb_handshake_sec_t));
							strcpy(handshake->magic, "usb_burn_handshake");
							sunxi_usb_pburn_status = SUNXI_USB_PBURN_SEND_DATA;

		  					trans_data.act_send_buffer = trans_data.base_send_buffer;
		  					trans_data.send_size = min(cbw->dCBWDataTransferLength, sizeof(__usb_handshake_sec_t));

							sunxi_usb_burn_from_boot_setup = 1;

							private_data_ext_buff = (u8 *)malloc(4 * 1024 * 1024);
							if(private_data_ext_buff == NULL)
							{
								printf("there is no memorfy to store all user key data\n");

								csw.bCSWStatus = -1;
							}
							else
		  					{
		  						csw.bCSWStatus = 0;
		  						sunxi_usb_burn_from_boot_handshake = 1;
		  					}
		  					private_data_ext_buff_step = private_data_ext_buff;
						}
						break;

						case 1:				//小机端接收数据
						{
							trans_data.recv_size = cbw->dCBWDataTransferLength;

                            sunxi_usb_pburn_write_enable = 0;
							sunxi_udc_start_recv_by_dma(private_data_ext_buff_step, trans_data.recv_size);	//start dma to receive data

							printf("recv_size=%d\n", trans_data.recv_size);
							sunxi_dump(private_data_ext_buff, trans_data.recv_size);

					        sunxi_usb_pburn_status = SUNXI_USB_PBURN_RECEIVE_NULL;
						}
						break;

						case 2:             //工具端声明数据传输已经完毕，要求获取烧录状态
						{
							__usb_handshake_ext_t  *handshake = (__usb_handshake_ext_t *)trans_data.base_send_buffer;

							memset(handshake, 0, sizeof(__usb_handshake_ext_t));
							//strcpy(handshake->magic, "usb_burn_receive_data_all");
							sunxi_usb_pburn_status = SUNXI_USB_PBURN_SEND_DATA;

							printf("recv_size=%d\n", trans_data.recv_size);
							sunxi_dump(private_data_ext_buff, trans_data.recv_size);

							int ret = __sunxi_burn_key(private_data_ext_buff, trans_data.recv_size);

		  					trans_data.act_send_buffer = trans_data.base_send_buffer;
		  					trans_data.send_size = min(cbw->dCBWDataTransferLength, sizeof(__usb_handshake_ext_t));
		  					//开始根据数据类型进行烧录动作

		  					if(!ret)	//数据烧写成功
		  					{
		  						strcpy(handshake->magic, "usb_burn_success");
		  						csw.bCSWStatus = 0;
		  					}
		  					else	//数据烧写失败
		  					{
		  						strcpy(handshake->magic, "usb_burn_error");
		  						csw.bCSWStatus = -1;
		  					}
		  					if(private_data_ext_buff)
		  					{
		  						free(private_data_ext_buff);
		  					}
						}
						break;

//						case 3:             //小机端读取每个key
//						{
//							uint start, sectors;
//							uint offset;
//
//							start   = *(int *)(cbw->CBWCDB + 4);		//读数据的偏移量
//							sectors = *(int *)(cbw->CBWCDB + 8);		//扇区数;
//
//							trans_data.send_size 	   = min(cbw->dCBWDataTransferLength, sectors * 512);
//							trans_data.act_send_buffer = (uint)trans_data.base_send_buffer;
//
//							offset = burn_private_start;
//							ret = sunxi_flash_read(start + offset, sectors, trans_data.base_send_buffer);
//							if(!ret)
//							{
//								printf("sunxi flash read err: start,0x%x sectors 0x%x\n", start, sectors);
//
//								csw.bCSWStatus = 1;
//							}
//							else
//							{
//								csw.bCSWStatus = 0;
//							}
//
//							sunxi_usb_pburn_status = SUNXI_USB_PBURN_SEND_DATA;
//						}
//						break;

						case 4:				//关闭usb
						{
							__usb_handshake_ext_t  *handshake = (__usb_handshake_ext_t *)trans_data.base_send_buffer;

                            memset(handshake, 0, sizeof(__usb_handshake_ext_t));
							strcpy(handshake->magic, "usb_burn_finish");

							trans_data.act_send_buffer = trans_data.base_send_buffer;
		  					trans_data.send_size = min(cbw->dCBWDataTransferLength, sizeof(__usb_handshake_ext_t));

							sunxi_udc_send_data((void *)trans_data.act_send_buffer, trans_data.send_size);

		  					csw.bCSWStatus = 0;

		  					sunxi_usb_pburn_status = SUNXI_USB_PBURN_EXIT;

						}
						break;

						case 5:
						{
							__usb_handshake_ext_t  *handshake = (__usb_handshake_ext_t *)trans_data.base_send_buffer;

							memset(handshake, 0, sizeof(__usb_handshake_ext_t));
							strcpy(handshake->magic, "usb_burn_saved");

							trans_data.act_send_buffer = trans_data.base_send_buffer;
		  					trans_data.send_size = min(cbw->dCBWDataTransferLength, sizeof(__usb_handshake_ext_t));

		  					csw.bCSWStatus = 0;

		  					sunxi_usb_pburn_status = SUNXI_USB_PBURN_SEND_DATA;
		  					if(sunxi_secure_storage_write("key_burned_flag", "key_burned", strlen("key_burned")))
		  					{
		  						printf("save burned flag err\n");

		  						csw.bCSWStatus = -1;
		  					}
		  					sunxi_secure_storage_exit();
						}
						break;

					default:
						break;
	  			}
	  			break;
#endif

	  			case 0xf3:			//自定义命令，用于烧录用户数据
	  				sunxi_usb_dbg("usb burn private\n");
	  				printf("usb command = %d\n", cbw->CBWCDB[1]);
	  				switch(cbw->CBWCDB[1])
	  				{
	  					case 0:				//握手
	  					{
	  						__usb_handshake_t  *handshake = (__usb_handshake_t *)trans_data.base_send_buffer;

							burn_private_start = sunxi_partition_get_offset_byname("private");
							burn_private_len   = sunxi_partition_get_size_byname("private");

							if(!burn_private_start)
							{
								printf("private partition is not exist\n");

								csw.bCSWStatus = -1;
							}
							else
							{
								csw.bCSWStatus = 0;
							}

                            memset(handshake, 0, sizeof(__usb_handshake_t));
							strcpy(handshake->magic, "usb_burn_handshake");
							handshake->sizelo = burn_private_len;
							handshake->sizehi = 0;
							sunxi_usb_pburn_status = SUNXI_USB_PBURN_SEND_DATA;

							sunxi_usb_burn_from_boot_setup = 1;
							sunxi_usb_burn_from_boot_handshake = 1;

		  					trans_data.act_send_buffer = trans_data.base_send_buffer;
		  					trans_data.send_size = min(cbw->dCBWDataTransferLength, sizeof(__usb_handshake_t));
						}
						break;

						case 1:				//小机端接收数据
						{
							//pburn_flash_sectors  = *(int *)(cbw->CBWCDB + 8);
							//pburn_flash_start    = *(int *)(cbw->CBWCDB + 4);
							memcpy(&pburn_flash_start,(cbw->CBWCDB + 4),4);

							trans_data.recv_size = cbw->dCBWDataTransferLength;
							trans_data.act_recv_buffer = trans_data.base_recv_buffer;

                            pburn_flash_start += burn_private_start;
                            sunxi_usb_pburn_write_enable = 0;
							sunxi_udc_start_recv_by_dma(trans_data.act_recv_buffer, trans_data.recv_size);	//start dma to receive data

					        sunxi_usb_pburn_status = SUNXI_USB_PBURN_RECEIVE_DATA;
						}
						break;

						case 3:             //小机端发送数据
						{
							uint start, sectors;

							//start   = *(int *)(cbw->CBWCDB + 4);		//读数据的偏移量
							//sectors = *(int *)(cbw->CBWCDB + 8);		//扇区数;
							memcpy(&start,(cbw->CBWCDB + 4),4);
							memcpy(&sectors,(cbw->CBWCDB + 8),4);

							printf("start=%d, sectors=%d\n", start, sectors);

							trans_data.send_size 	   = min(cbw->dCBWDataTransferLength, sectors * 512);
							trans_data.act_send_buffer = trans_data.base_send_buffer;

							printf("send size=%d\n", trans_data.send_size);

							ret = sunxi_flash_read(start + burn_private_start, sectors, trans_data.base_send_buffer);
							if(!ret)
							{
								printf("sunxi flash read err: start,0x%x sectors 0x%x\n", start, sectors);

								csw.bCSWStatus = 1;
							}
							else
							{
								csw.bCSWStatus = 0;
							}

							sunxi_usb_pburn_status = SUNXI_USB_PBURN_SEND_DATA;
						}
						break;

						case 4:				//关闭usb
						{
							__usb_handshake_ext_t  *handshake = (__usb_handshake_ext_t *)trans_data.base_send_buffer;

                            memset(handshake, 0, sizeof(__usb_handshake_ext_t));
							strcpy(handshake->magic, "usb_burn_finish");

							trans_data.act_send_buffer = trans_data.base_send_buffer;
		  					trans_data.send_size = min(cbw->dCBWDataTransferLength, sizeof(__usb_handshake_ext_t));

							sunxi_udc_send_data((void *)trans_data.act_send_buffer, trans_data.send_size);

		  					csw.bCSWStatus = 0;

							sunxi_flash_flush();
		  					sunxi_usb_pburn_status = SUNXI_USB_PBURN_EXIT;

						}
						break;

						case 5:
						{
							__usb_handshake_ext_t  *handshake = (__usb_handshake_ext_t *)trans_data.base_send_buffer;
							char buffer[512];

							memset(handshake, 0, sizeof(__usb_handshake_ext_t));
							strcpy(handshake->magic, "usb_burn_saved");

							trans_data.act_send_buffer = trans_data.base_send_buffer;
		  					trans_data.send_size = min(cbw->dCBWDataTransferLength, sizeof(__usb_handshake_ext_t));

		  					csw.bCSWStatus = 0;

		  					sunxi_usb_pburn_status = SUNXI_USB_PBURN_SEND_DATA;

		  					memset(buffer, 0, 512);
							strcpy(buffer, "key_burned");

		  					if(!sunxi_flash_write(burn_private_start + burn_private_len - (8192+512)/512, 1, buffer))
		  					{
		  						printf("save burned flag err\n");

		  						csw.bCSWStatus = -1;
		  					}
		  					sunxi_flash_flush();
#ifdef CONFIG_SUNXI_SECURE_STORAGE
							if(sunxi_secure_storage_init())
							{
								printf("init secure storage failed\n");

								csw.bCSWStatus = -1;
							}
		  					else
		  					{
		  						if(sunxi_secure_storage_write("key_burned_flag", "key_burned", strlen("key_burned")))
		  						{
		  							printf("save burned flag err\n");

		  							csw.bCSWStatus = -1;
		  						}
		  					}
		  					sunxi_secure_storage_exit();
#endif
						}
						break;

					default:
						break;
	  				}

	  			break;

	  			default:
	  				sunxi_usb_dbg("not supported command 0x%x now\n", cbw->CBWCDB[0]);
	  				sunxi_usb_dbg("asked size 0x%x\n", cbw->dCBWDataTransferLength);

	  				csw.bCSWStatus = 1;

	  				sunxi_usb_pburn_status = SUNXI_USB_PBURN_STATUS;

	  				break;
	  		}

	  		break;

	  	case SUNXI_USB_PBURN_SEND_DATA:

	  		sunxi_usb_dbg("SUNXI_USB_SEND_DATA\n");

			sunxi_usb_pburn_status = SUNXI_USB_PBURN_STATUS;
			printf("SUNXI_USB_SEND_DATA=%d\n", trans_data.send_size);
			sunxi_udc_send_data((void *)trans_data.act_send_buffer, trans_data.send_size);
#if defined(SUNXI_USB_30)
			sunxi_usb_pburn_status_enable = 0;
#endif
	  		break;

	  	case SUNXI_USB_PBURN_RECEIVE_DATA:

	  		sunxi_usb_dbg("SUNXI_USB_RECEIVE_DATA\n");

			if(sunxi_usb_pburn_write_enable == 1)
			{
				sunxi_usb_dbg("write flash, start 0x%x, sectors 0x%x\n", pburn_flash_start, trans_data.recv_size/512);
				ret = sunxi_flash_write(pburn_flash_start, (trans_data.recv_size+511)/512, (void *)trans_data.act_recv_buffer);
				if(!ret)
				{
					printf("sunxi flash write err: start,0x%x sectors 0x%x\n", pburn_flash_start, (trans_data.recv_size+511)/512);

					csw.bCSWStatus = 1;
				}
				else
				{
					csw.bCSWStatus = 0;
  				}
				sunxi_usb_pburn_write_enable = 0;

  				sunxi_usb_pburn_status = SUNXI_USB_PBURN_STATUS;
			}

	  		break;

		case SUNXI_USB_PBURN_STATUS:

			sunxi_usb_dbg("SUNXI_USB_PBURN_STATUS\n");
#if defined(SUNXI_USB_30)
			if(sunxi_usb_pburn_status_enable)
#endif
			{
				sunxi_usb_pburn_status = SUNXI_USB_PBURN_IDLE;
				sunxi_ubuf->rx_ready_for_data = 0;
				__sunxi_pburn_send_status(&csw, sizeof(struct umass_bbb_csw_t));
			}

			break;

		case SUNXI_USB_PBURN_EXIT:

			printf("SUNXI_USB_PBURN_EXIT\n");

			sunxi_usb_pburn_status = SUNXI_USB_PBURN_IDLE;
			sunxi_ubuf->rx_ready_for_data = 0;
			__sunxi_pburn_send_status(&csw, sizeof(struct umass_bbb_csw_t));

			printf("Device will shutdown in 3 Secends...\n");
			__msdelay(3000);

			return SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN;

	  	case SUNXI_USB_PBURN_RECEIVE_NULL:

	  		sunxi_usb_dbg("SUNXI_USB_PBURN_RECEIVE_NULL\n");

			if(sunxi_usb_pburn_write_enable == 1)
			{
				csw.bCSWStatus = 0;
				sunxi_usb_pburn_write_enable = 0;
  				sunxi_usb_pburn_status = SUNXI_USB_PBURN_STATUS;
			}

	  		break;

	  	default:
	  		break;
	}

	return 0;
}


sunxi_usb_module_init(SUNXI_USB_DEVICE_BURN,					\
					  sunxi_pburn_init,							\
					  sunxi_pburn_exit,							\
					  sunxi_pburn_reset,							\
					  sunxi_pburn_standard_req_op,				\
					  sunxi_pburn_nonstandard_req_op,			\
					  sunxi_pburn_state_loop,					\
					  sunxi_pburn_usb_rx_dma_isr,				\
					  sunxi_pburn_usb_tx_dma_isr					\
					  );
