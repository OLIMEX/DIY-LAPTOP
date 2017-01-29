/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Young <guoyingyang@allwinnertech.com>
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
#include"usb_efex_test.h"
#include <common.h>
#include <malloc.h>
#include <command.h>
#include "usb_base.h"
#include <scsi.h>
#include <asm/arch/dma.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <sprite.h>
#include <boot_type.h>
#include "usb_efex.h"
#include <pmu.h>
#include <asm/io.h>
#include "efex_queue.h"
#include "usb_efex_test.h"
#include "asm/arch/clock.h"
#include <power.h>

#define _EFEX_USE_BUF_QUEUE_

#define  SUNXI_USB_EFEX_IDLE					 (0)
#define  SUNXI_USB_EFEX_SETUP					 (1)
#define  SUNXI_USB_EFEX_SEND_DATA				 (2)
#define  SUNXI_USB_EFEX_RECEIVE_DATA			 (3)
#define  SUNXI_USB_EFEX_STATUS					 (4)
#define  SUNXI_USB_EFEX_EXIT					 (5)

#define  SUNXI_USB_EFEX_SETUP_NEW                (11)
#define  SUNXI_USB_EFEX_SEND_DATA_NEW            (12)
#define  SUNXI_USB_EFEX_RECEIVE_DATA_NEW         (13)
#define  FES_NEW_CMD_LEN                         (20)



#define  SUNXI_USB_EFEX_APPS_MAST				 (0xf0000)

#define  SUNXI_USB_EFEX_APPS_IDLE				 (0x10000)
#define  SUNXI_USB_EFEX_APPS_CMD				 (0x20000)
#define  SUNXI_USB_EFEX_APPS_DATA				 (0x30000)
#define  SUNXI_USB_EFEX_APPS_SEND_DATA		     (SUNXI_USB_EFEX_APPS_DATA | SUNXI_USB_EFEX_SEND_DATA)
#define  SUNXI_USB_EFEX_APPS_RECEIVE_DATA		 (SUNXI_USB_EFEX_APPS_DATA | SUNXI_USB_EFEX_RECEIVE_DATA)
#define  SUNXI_USB_EFEX_APPS_STATUS				 ((0x40000)  | SUNXI_USB_EFEX_STATUS)
#define  SUNXI_USB_EFEX_APPS_EXIT				 ((0x50000)  | SUNXI_USB_EFEX_EXIT)

#define DRIVER_VENDOR_ID			0x1F3A
#define DRIVER_PRODUCT_ID			0xEfE8

static  int sunxi_usb_efex_write_enable = 0;
static  int sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
static  int sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_IDLE;
static  efex_trans_set_t  trans_data;
static  u8  *cmd_buf;
static  u32 sunxi_efex_next_action = 0;
boot_logo_info  logo_info ;
#if defined(SUNXI_USB_30)
static  int sunxi_usb_efex_status_enable = 1;
#endif

extern int do_savecfg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern void board_status_probe(int standby_mode);

int  efex_test = 0;
char* logo_buf = NULL;
DECLARE_GLOBAL_DATA_PTR;

int sunxi_probe_axp_each_volt(PMU_VOLT_t *pmu_para)
{
	int  ret;//onoff;
	uint power_supply_hd;
	char pmu_name[16];
	int i = 0;
	int  power_vol, power_index = 0;
	PMU_VOLT_t *pmu_temp = NULL;
	if(!pmu_para)
	{
		printf("pmu_para is NULL \n");
		return -1;
	}
	printf("start =====\n");
	pmu_temp = pmu_para;
#if defined(CONFIG_SUNXI_AXP20)
    power_supply_hd = script_parser_fetch_subkey_start("target");
#elif defined(CONFIG_SUNXI_AXP15)
    power_supply_hd = script_parser_fetch_subkey_start("axp15_para");
#else
    power_supply_hd = script_parser_fetch_subkey_start("power_sply");
#endif
    if(!power_supply_hd)
    {
        printf("unable to find power supply\n");

		return -1;
	}
	i = 0;
	while(axp_probe_supply_pmu_name(pmu_name) == 0)
	{
		printf("current pmu_name == %s == \n",pmu_name);
		do
		{
			memset(pmu_temp->volt[i].pmu_type, 0, 16);
			strcpy((void *)pmu_temp->volt[i].pmu_type,pmu_name);
			memset(pmu_temp->volt[i].vol_name, 0, 16);
			ret = script_parser_fetch_subkey_next(power_supply_hd, pmu_temp->volt[i].vol_name, &power_vol, &power_index);
			if(ret < 0)
			{
				printf("find power_sply to end\n");
				pmu_temp->voltCnt = i;
				i++;
				break;
			}
			pmu_temp->volt[i].voltage = axp_probe_supply_status_byname(pmu_temp->volt[i].pmu_type,pmu_temp->volt[i].vol_name);
			if(pmu_temp->volt[i].voltage == -1)
			{
				printf("axp probe %s failed\n", pmu_temp->volt[i].vol_name);
				return -1;
			}
			printf("%d \n",pmu_temp->volt[i].voltage);
			i++;
		}
		while(1);
		power_supply_hd = script_parser_fetch_subkey_start("slave_power_sply");
	    if(!power_supply_hd)
		{
			printf("unable to find slave power supply\n");
			return 0;
		}
		power_index = 0;

	}
	printf("end====\n");
    return 0;

}
uint replace_boot_logo(bmp_image_t *bmp_info)
{
#if 1
    FATFS script_mount;
	int  ret;
	uint write_bytes = 0;
	FILE  script_fs;
	
	if(bmp_info == NULL)
	{
		printf("bmp_info is invaild \n");
		return -1;
	}
	f_mount_ex(0, &script_mount, 0);
	ret = f_open (&script_fs, "0:bootlogo.bmp", FA_OPEN_EXISTING | FA_READ | FA_WRITE );
	if(ret)
	{
		printf("cant open bootlogo.bmp, maybe it is not exist\n");
	}
	else
	{
		puts("try to unlink bootlogo\n");
		f_close(&script_fs);
		printf("%d\n", f_unlink("0:bootlogo.bmp"));
	}
	ret = f_open (&script_fs, "0:bootlogo.bmp", FA_CREATE_NEW | FA_READ | FA_WRITE );
    if(ret)
    {
        printf("creat fliled\n");
    }
    printf("logo size is %d \n",bmp_info->header.file_size);
    flush_cache((uint)bmp_info,write_bytes);
    printf("check_sum = %x \n",crc32(0,(const unsigned char*)bmp_info,bmp_info->header.file_size));
    if(!f_write(&script_fs, bmp_info, bmp_info->header.file_size, &write_bytes))
	{
		printf("f_write write bytes = %d\n", write_bytes);
	}
	f_close(&script_fs);
//	ret = f_open (&script_fs, "0:bootlogo.bmp", FA_OPEN_EXISTING | FA_READ | FA_WRITE );

//	if(!f_read(&script_fs, read_buf, bmp_info->header.file_size, &read_bytes))
//	{
//		printf("f_read read bytes = %d \n",read_bytes);
//	}
//	if(memcmp(bmp_info,read_buf,1024*1024)==0)
//	{
//		printf("=========cmp successed ============\n");
//	}
//	f_close(&script_fs);

	f_mount(0, NULL, NULL);
	return write_bytes;
#else
    char bmp_addr[8];
    char bmp_size[16];
    char *const bmp_argv[6] = { "fatdown" , "sunxi_flash" ,"0" , bmp_addr ,"bootlogo.bmp", bmp_size};
    memset(bmp_addr , 0x00 , 8);
    memset(bmp_size , 0x00 , 16);
    sprintf(bmp_addr,"%x",(uint)bmp_info);
    sprintf(bmp_size,"%x",bmp_info->header.file_size);
    printf("bmp_addr is %s \n",bmp_addr);
    printf("bmp_size is %s \n",bmp_size); 
    if(do_fat_fsdown(0,0,6,bmp_argv))
    {
        printf("efex_test : unable to download logo into machine \n");
        return -1;
    }
    return 0;
#endif
}

u32 read_file(char *file_name,uchar * buffer)
{
#if 1
	FATFS script_mount;
	int  ret;
	u32 read_bytes = 0;
	FILE  script_fs;
	uchar  *buf = buffer;

	f_mount_ex(0, &script_mount, 0);
	ret = f_open (&script_fs, file_name, FA_OPEN_EXISTING | FA_READ | FA_WRITE );
	if(ret)
	{
		printf("cant open script.bin, maybe it is not exist\n");
		return -1;
	}
	else
	{
		if(!f_read(&script_fs, buf, 1024 * 1024, &read_bytes))
		{
			printf("f_read read bytes = %d\n", read_bytes);
		}
        printf("read_buf check_sum is %x \n",crc32(0,(const unsigned char *)buf,read_bytes));
		f_close(&script_fs);
	}
	f_mount(0, NULL, NULL);
	return read_bytes;
#endif
//char *const bmp_argv[6] = { "fatload", "sunxi_flash", "0", load_addr, filename, NULL};   
}


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
			qua_dscrpt->bRESERVED          = 0;

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
static int __sunxi_efex_send_status(void *buffer, unsigned int buffer_size)
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
static int sunxi_efex_test_init(void)
{
	sunxi_usb_dbg("sunxi_efex_init\n");
	memset(&trans_data, 0, sizeof(efex_trans_set_t));
	sunxi_usb_efex_write_enable = 0;
    sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
    sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_IDLE;

	cmd_buf = (u8 *)malloc(CBW_MAX_CMD_SIZE);
	if(!cmd_buf)
	{
		printf("sunxi usb efex err: unable to malloc memory for cmd\n");

		return -1;
	}
    trans_data.base_recv_buffer = (u8 *)malloc(SUNXI_EFEX_RECV_MEM_SIZE);
    if(!trans_data.base_recv_buffer)
    {
    	printf("sunxi usb efex err: unable to malloc memory for efex receive\n");
		free(cmd_buf);

    	return -1;
    }

	trans_data.base_send_buffer = (u8 *)malloc(SUNXI_EFEX_RECV_MEM_SIZE);
    if(!trans_data.base_send_buffer)
    {
    	printf("sunxi usb efex err: unable to malloc memory for efex send\n");
    	free(trans_data.base_recv_buffer);
		free(cmd_buf);

    	return -1;
    }
	sunxi_usb_dbg("recv addr 0x%x\n", (uint)trans_data.base_recv_buffer);
    sunxi_usb_dbg("send addr 0x%x\n", (uint)trans_data.base_send_buffer);

#ifdef _EFEX_USE_BUF_QUEUE_
    if(efex_queue_init()) 
    {
        return -1;
    }
#endif
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
static int sunxi_efex_test_exit(void)
{
	sunxi_usb_dbg("sunxi_efex_exit\n");
    if(trans_data.base_recv_buffer)
    {
    	free(trans_data.base_recv_buffer);
    }
    if(trans_data.base_send_buffer)
    {
    	free(trans_data.base_send_buffer);
    }
    if(cmd_buf)
    {
    	free(cmd_buf);
	}
#ifdef _EFEX_USE_BUF_QUEUE_
    efex_queue_exit();
#endif
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
static void sunxi_efex_test_reset(void)
{
	sunxi_usb_efex_write_enable = 0;
    sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
    sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_IDLE;
    trans_data.to_be_recved_size = 0;
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
static void  sunxi_efex_test_usb_rx_dma_isr(void *p_arg)
{
	sunxi_usb_dbg("dma int for usb rx occur\n");
	//通知主循环，可以写入数据
	sunxi_usb_efex_write_enable = 1;
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
static void  sunxi_efex_test_usb_tx_dma_isr(void *p_arg)
{
	sunxi_usb_dbg("dma int for usb tx occur\n");

#if defined(SUNXI_USB_30)
	sunxi_usb_efex_status_enable ++;
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
static int sunxi_efex_test_standard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer)
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
			printf("sunxi efex error: standard req is not supported\n");

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
static int sunxi_efex_test_nonstandard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer, uint data_status)
{
	int ret = SUNXI_USB_REQ_SUCCESSED;

	switch(req->bmRequestType)
	{
		case 161:
			if(req->bRequest == 0xFE)
			{
				sunxi_usb_dbg("efex ask for max lun\n");

				buffer[0] = 0;

				sunxi_udc_send_setup(1, buffer);
			}
			else
			{
				printf("sunxi usb err: unknown ep0 req in efex\n");

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
static void __sunxi_usb_efex_fill_status(void)
{
	Status_t     *efex_status;

	efex_status = (Status_t *)trans_data.base_send_buffer;
	memset(efex_status, 0, sizeof(Status_t));
	efex_status->mark  = 0xffff;
	efex_status->tag   = 0;
	efex_status->state = 0;

	trans_data.act_send_buffer = (uint)trans_data.base_send_buffer;
	trans_data.send_size = sizeof(Status_t);

	return;
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
static int __sunxi_usb_efex_test_op_cmd(u8 *cmd_buffer)
{
	struct global_cmd_s  *cmd = (struct global_cmd_s *)cmd_buffer;

	switch(cmd->app_cmd)
	{
		printf("======guoyingyang =====\n");
		case FEX_CMD_fes_trans:
			sunxi_usb_dbg("FEX_CMD_fes_trans\n");

			//需要发送数据
			{
				fes_trans_old_t  *fes_old_data = (fes_trans_old_t *)cmd_buf;

				if(fes_old_data->len)
				{
					if(fes_old_data->u2.DOU == 2)		//上传数据
					{
#ifdef SUNXI_USB_DEBUG
						uint value;

						value = *(uint *)fes_old_data->addr;
#endif
						sunxi_usb_dbg("send id 0x%x, addr 0x%x, length 0x%x\n", value, fes_old_data->addr, fes_old_data->len);
						trans_data.act_send_buffer   = fes_old_data->addr;	//设置发送地址
						trans_data.send_size         = fes_old_data->len;	//设置发送长度
						trans_data.last_err          = 0;
						trans_data.app_next_status   = SUNXI_USB_EFEX_APPS_SEND_DATA;
					}
					else	//(fes_old_data->u2.DOU == (0 or 1))	//下载数据
					{
#ifdef SUNXI_USB_DEBUG
						uint value;

						value = *(uint *)fes_old_data->addr;
#endif
						sunxi_usb_dbg("receive id 0x%x, addr 0x%x, length 0x%x\n", value, fes_old_data->addr, fes_old_data->len);

						trans_data.type = SUNXI_EFEX_DRAM_TAG;		//写到dram的数据
						trans_data.act_recv_buffer   = fes_old_data->addr;	//设置接收地址
						trans_data.recv_size         = fes_old_data->len;	//设置接收长度
						trans_data.last_err          = 0;
						trans_data.app_next_status   = SUNXI_USB_EFEX_APPS_RECEIVE_DATA;
					}
				}
				else
				{
					printf("FEX_CMD_fes_trans: no data need to send or receive\n");

					trans_data.app_next_status = SUNXI_USB_EFEX_APPS_STATUS;
				}
			}
			trans_data.last_err = 0;

			break;

		case FEX_CMD_fes_run:
			sunxi_usb_dbg("FEX_CMD_fes_run\n");
			{
#ifdef  CONFIG_CMD_ELF
				fes_run_t   *runs = (fes_run_t *)cmd_buf;
				int         *app_ret;
				char run_addr[32] = {0};
				char paras[8][16];
				char *const  usb_runs_args[12] = {NULL, run_addr, 					\
					                            (char *)&paras[0][0], 				\
					                            (char *)&paras[1][0],				\
					                            (char *)&paras[2][0],				\
					                            (char *)&paras[3][0],				\
					                            (char *)&paras[4][0],				\
					                            (char *)&paras[5][0],				\
					                            (char *)&paras[6][0],				\
					                        	(char *)&paras[7][0]};

				sprintf(run_addr, "%x", runs->addr);
				printf("usb run addr      = %s\n", run_addr);
				printf("usb run paras max = %d\n", runs->max_para);
				{
					int i;
					int *data;

					data = runs->para_addr;
					for(i=0;i<runs->max_para;i++)
					{
						printf("usb run paras[%d] = 0x%x\n", i, data[i]);
						sprintf((char *)&paras[i][0], "%x", data[i]);
					}
				}

				app_ret = (int *)trans_data.base_send_buffer;
				*app_ret = do_bootelf(NULL, 0, runs->max_para + 1, usb_runs_args);
				printf("usb get result = %d\n", *app_ret);
				trans_data.act_send_buffer   = (uint)trans_data.base_send_buffer;
				trans_data.send_size         = 4;
				trans_data.last_err          = 0;
				trans_data.app_next_status   = SUNXI_USB_EFEX_APPS_SEND_DATA;
#else
				int         *app_ret;

				app_ret = (int *)trans_data.base_send_buffer;
				*app_ret = -1;
				trans_data.act_send_buffer   = (uint)trans_data.base_send_buffer;
				trans_data.send_size         = 4;
				trans_data.last_err          = 0;
				trans_data.app_next_status   = SUNXI_USB_EFEX_APPS_SEND_DATA;
#endif
			}

			break;

		case FEX_CMD_fes_down:
			sunxi_usb_dbg("FEX_CMD_fes_down\n");
			{
				fes_trans_t  *trans = (fes_trans_t *)cmd_buf;

				trans_data.type  = trans->type;									 //数据类型，MBR,BOOT1,BOOT0...以及分区类型
				if((trans->type & SUNXI_EFEX_DRAM_MASK) == SUNXI_EFEX_DRAM_MASK) //如果属于内存数据，则执行这里
				{
					if((SUNXI_EFEX_DRAM_MASK | SUNXI_EFEX_TRANS_FINISH_TAG) == trans->type)
					{
					    trans_data.act_recv_buffer = (uint)trans_data.base_recv_buffer;
						trans_data.dram_trans_buffer = trans->addr;
						//printf("dram write: start 0x%x: length 0x%x\n", trans->addr, trans->len);
					}
					else
					{
						trans_data.act_recv_buffer   = (uint)trans_data.base_recv_buffer + trans_data.to_be_recved_size;	 //设置接收地址
					}
					trans_data.recv_size         = trans->len;	//设置接收长度，字节单位
					trans_data.to_be_recved_size += trans->len;
					sunxi_usb_dbg("down dram: start 0x%x, sectors 0x%x\n", trans_data.flash_start, trans_data.flash_sectors);
				}
				else	//属于flash数据，分别表示起始扇区，扇区数
				{
					trans_data.act_recv_buffer   = (uint)(trans_data.base_recv_buffer + SUNXI_EFEX_RECV_MEM_SIZE/2);	 //设置接收地址
					trans_data.recv_size         = trans->len;	//设置接收长度，字节单位

					trans_data.flash_start       = trans->addr;
					trans_data.flash_sectors     = (trans->len + 511) >> 9;
					sunxi_usb_dbg("down flash: start 0x%x, sectors 0x%x\n", trans_data.flash_start, trans_data.flash_sectors);
				}
				trans_data.last_err          = 0;
				trans_data.app_next_status   = SUNXI_USB_EFEX_APPS_RECEIVE_DATA;
			}

			break;
		case FEX_CMD_fes_up:
			printf("FEX_CMD_fes_up\n");

			{
				fes_trans_t  *trans = (fes_trans_t *)cmd_buf;

				trans_data.last_err          	 = 0;
				trans_data.app_next_status   	 = SUNXI_USB_EFEX_APPS_SEND_DATA;
				trans_data.type  = trans->type;									 //数据类型，MBR,BOOT1,BOOT0...以及分区类型
                printf("cmd is %x \n",trans_data.type);
				if((trans->type & SUNXI_EFEX_DRAM_MASK) == SUNXI_EFEX_DRAM_MASK) //如果属于内存数据，则执行这里
				{
#if 0
					if((SUNXI_EFEX_DRAM_MASK | SUNXI_EFEX_TRANS_FINISH_TAG) == trans->type)
					{
					    trans_data.act_send_buffer = (uint)trans_data.base_send_buffer;
						trans_data.dram_trans_buffer = trans->addr;
						printf("dram write: start 0x%x: length 0x%x\n", trans->addr, trans->len);
					}
					else
					{
						trans_data.act_send_buffer   = (uint)trans_data.base_send_buffer + trans_data.send_size;	 //设置发送数据地址
					}
#endif

					trans_data.act_send_buffer   = trans->addr;	//设置发送地址，属于字节单位
					trans_data.send_size         = trans->len;	//设置接收长度，字节单位
					sunxi_usb_dbg("dram read: start 0x%x: length 0x%x\n", trans->addr, trans->len);
				}
				else if((trans->type & FEX_CMD_BOOT2_GET_VOLTAGE) == FEX_CMD_BOOT2_GET_VOLTAGE )
				{
					printf("PC get axp_each_volt form machine\n");
					PMU_VOLT_t *pmu_para = NULL;
					pmu_para = (PMU_VOLT_t *)trans_data.base_send_buffer;
					trans_data.act_send_buffer   = (uint)trans_data.base_send_buffer;
					trans_data.send_size         = sizeof(PMU_VOLT_t);
					trans_data.last_err = sunxi_probe_axp_each_volt(pmu_para);
				}
				else if((trans->type & FEX_CMD_BOOT2_GET_FREQ) == FEX_CMD_BOOT2_GET_FREQ)
				{
					printf("PC get cpu freqency \n");
					boot_clock *cpu_pll = NULL;
					cpu_pll = (boot_clock *)trans_data.base_send_buffer;
					trans_data.act_send_buffer   = (uint)trans_data.base_send_buffer;
					trans_data.send_size         = sizeof(boot_clock);
					cpu_pll->cpu_id = 0;
					cpu_pll->gate  = 1;
					cpu_pll->freqency = sunxi_clock_get_corepll();
				}
				else if((trans->type & FEX_CMD_BOOT2_GET_CURRENT_LIMIT) == FEX_CMD_BOOT2_GET_CURRENT_LIMIT)
				{
					uint *charger_current = NULL;
					charger_current = (uint *)trans_data.base_send_buffer;
					*charger_current = axp_probe_vbus_cur_limit();
					printf("========  current limit is %d =======\n",*charger_current);
					trans_data.act_send_buffer   = (uint)trans_data.base_send_buffer;
					trans_data.send_size = sizeof(uint);
				}
				else if((trans->type & FEX_CMD_BOOT2_GET_LOGO_INFO) ==FEX_CMD_BOOT2_GET_LOGO_INFO)
				{
					//boot_logo_info  logo_info ;
                    logo_buf = (char *)malloc(BOOT_LOGO_MAX_SIZE);
                    memset(logo_buf,0x00,BOOT_LOGO_MAX_SIZE);
					logo_info.boot_logo_size = read_file("0:bootlogo.bmp",(uchar *)logo_buf);
					if(logo_info.boot_logo_size == -1)
					{
						printf("read file fail \n");
						trans_data.last_err = -1;
					}
					else
					{
						printf("send logo info %d bytes\n",logo_info.boot_logo_size);
                        //printf("logo buf addr  %x \n",logo_info.boot_logo_addr);
                        logo_info.boot_logo_addr = (uint)logo_buf;
                        printf("logo buf addr  %x \n",logo_info.boot_logo_addr);
                        logo_info.check_sum = crc32(0,(const unsigned char *)logo_buf,logo_info.boot_logo_size);
                        printf("check_sum   %x \n",logo_info.check_sum);
						trans_data.act_send_buffer = (uint)&logo_info;
						trans_data.send_size = sizeof(boot_logo_info);
						trans_data.last_err = 0;
					}
				}
				else	//属于flash数据，分别表示起始扇区，扇区数
				{
					trans_data.act_send_buffer   = (uint)trans_data.base_send_buffer;	 //设置发送地址
					trans_data.send_size         = trans->len;	//设置接收长度，字节单位

					trans_data.flash_start       = trans->addr; //设置发送地址，属于扇区单位
					trans_data.flash_sectors     = (trans->len + 511) >> 9;

					sunxi_usb_dbg("upload flash: start 0x%x, sectors 0x%x\n", trans_data.flash_start, trans_data.flash_sectors);
					if(!sunxi_sprite_read(trans_data.flash_start, trans_data.flash_sectors, (void *)trans_data.act_send_buffer))
					{
						printf("flash read err: start 0x%x, sectors 0x%x\n", trans_data.flash_start, trans_data.flash_sectors);

						trans_data.last_err      = -1;
					}
				}
			}

			break;
			printf("sunxi_efex_next_action=%d\n", sunxi_efex_next_action);
			trans_data.last_err          = 0;
			//before product finish, clear suspend flag
		//	efex_test_suspend_flag            = 0;

			break;

		case FEX_CMD_fes_reset_cpu:
			printf("reset cpu \n");
		    sunxi_efex_next_action = SUNXI_UPDATE_NEXT_ACTION_REBOOT;
		    trans_data.app_next_status   = SUNXI_USB_EFEX_APPS_EXIT;
            trans_data.last_err = 0;
            if(logo_buf)
            {
                free(logo_buf);
                logo_buf = NULL;
            }
			break;
		case FEX_CMD_fes_low_power_manger:
			printf("run low power manger \n");
			efex_test = 1;
			gd->power_step_level = BATTERY_RATIO_TOO_LOW_WITHOUT_DCIN;
			board_status_probe(0);
            trans_data.app_next_status = SUNXI_USB_EFEX_APPS_STATUS;
            trans_data.last_err = 0;
			break ;
		default:
			printf("not supported command 0x%x now\n", cmd->app_cmd);

			trans_data.last_err        = -1;
			trans_data.app_next_status = SUNXI_USB_EFEX_APPS_STATUS;
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
static void dram_data_recv_finish(uint data_type)
{
	if (data_type ==  FEX_CMD_BOOT2_DOWNLOAD_SYS_CONFIG)
	{
		printf("download sys_config form PC \n");
		//judge sys_config.fex
        gd->force_download_uboot = 1;
		memcpy((void *)SYS_CONFIG_MEMBASE,(void *)trans_data.act_recv_buffer,trans_data.recv_size);
		do_savecfg(0,0,1,0);
	}
	else if(data_type == FEX_CMD_BOOT2_DOWNLOAD_LOGO)
	{
		bmp_image_t *bmp_info = NULL ;
		printf("recv boot_logo_info from pc %d  \n",trans_data.to_be_recved_size);
		//judge logo is bmp
		bmp_info = (bmp_image_t *)trans_data.base_recv_buffer;
		if((bmp_info->header.signature[0]!='B') || (bmp_info->header.signature[1] !='M')) 
		{
			printf("this is not a bmp picture \n");
			trans_data.last_err = -1;
		}
		else
		{
			u32 ret = 0;
			ret = replace_boot_logo(bmp_info);
			if(ret == -1)
				trans_data.last_err = -1;
			else
				trans_data.last_err = 0;
		}
        sunxi_flash_exit(1);
		
	}
    else//其它数据，直接写入内存
	{
        memcpy((void *)trans_data.dram_trans_buffer, (void *)trans_data.act_recv_buffer, trans_data.recv_size);

		sunxi_usb_dbg("SUNXI_EFEX_DRAM_TAG\n");

		trans_data.last_err = 0;
	}
	trans_data.to_be_recved_size = 0;
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
static int sunxi_efex_test_state_loop(void  *buffer)
{
	static struct sunxi_efex_cbw_t  *cbw;
	static struct sunxi_efex_csw_t   csw;
	sunxi_ubuf_t *sunxi_ubuf = (sunxi_ubuf_t *)buffer;
    int  efex_write_error_flag = 0;

	switch(sunxi_usb_efex_status)
	{
		case SUNXI_USB_EFEX_IDLE:
			if(sunxi_ubuf->rx_ready_for_data == 1)
			{
				sunxi_usb_efex_status = SUNXI_USB_EFEX_SETUP;
			}
			//when product finish and usb disconnect ,shutdown machine
			if( sunxi_efex_next_action == SUNXI_UPDATE_NEXT_ACTION_NORMAL ||
				sunxi_efex_next_action >  SUNXI_UPDATE_NEXT_ACTION_REUPDATE )
			{
				//if(efex_suspend_flag)
				//{
				//	return SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN;
				//}
			}

			break;

		case SUNXI_USB_EFEX_SETUP:		//cbw

			sunxi_usb_dbg("SUNXI_USB_EFEX_SETUP\n");

            if((sunxi_ubuf->rx_req_length == sizeof(struct sunxi_efex_cbw_t)))
            {
                cbw = (struct sunxi_efex_cbw_t *)sunxi_ubuf->rx_req_buffer;
                if(CBW_MAGIC != cbw->magic)
                {
                    printf("sunxi usb error: the cbw signature 0x%x is bad, need 0x%x\n", cbw->magic, CBW_MAGIC);
                    sunxi_ubuf->rx_ready_for_data = 0;
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
                    return -1;
                }
            }
            else if(sunxi_ubuf->rx_req_length == FES_NEW_CMD_LEN)
            {
                sunxi_usb_dbg("----------new cmd format--------\n");
                if(CBW_MAGIC != ((u32*)(sunxi_ubuf->rx_req_buffer))[4])
                {
                    printf("sunxi usb error: the cmd signature 0x%x is bad, need 0x%x\n",
                          ((u32*)(sunxi_ubuf->rx_req_buffer))[4],CBW_MAGIC);


                    sunxi_ubuf->rx_ready_for_data = 0;
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
                    //data value err
                    return -1;
                }

                sunxi_usb_efex_status = SUNXI_USB_EFEX_SETUP_NEW;
                break;
            }
            else
            {
                printf("sunxi usb error: received bytes 0x%x is not equal cbw struct size 0x%x or new cmd size 0x%x\n",
                    sunxi_ubuf->rx_req_length, sizeof(struct sunxi_efex_cbw_t),FES_NEW_CMD_LEN);
                sunxi_ubuf->rx_ready_for_data = 0;
                sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
				return -1;
            }

			csw.magic = CSW_MAGIC;		//"AWUS"
			csw.tag   = cbw->tag;

#if defined(SUNXI_USB_30)
			sunxi_usb_efex_status_enable = 1;
#endif

			sunxi_usb_dbg("usb cbw trans direction = 0x%x\n", cbw->cmd_package.direction);
			if(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_IDLE)
			{
				sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_IDLE\n");
				if(cbw->cmd_package.direction == TL_CMD_RECEIVE)	//小机端接收数据
				{
					sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_IDLE: TL_CMD_RECEIVE\n");
					sunxi_ubuf->request_size = min(cbw->data_transfer_len, CBW_MAX_CMD_SIZE);
					sunxi_usb_dbg("try to receive data 0x%x\n", sunxi_ubuf->request_size);
					sunxi_usb_efex_write_enable = 0;
					if(sunxi_ubuf->request_size)
					{
						sunxi_udc_start_recv_by_dma((uint)cmd_buf, sunxi_ubuf->request_size);	//start dma to receive data
					}
					else
					{
						printf("APPS: SUNXI_USB_EFEX_APPS_IDLE: the send data length is 0\n");

						return -1;
					}
					//下一阶段接收到的数据是app
					sunxi_usb_efex_status   = SUNXI_USB_EFEX_RECEIVE_DATA;	//传输阶段，下一阶段将接收数据
					sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_CMD;		//命令阶段，下一阶段接收的是命令
				}
				else	//setup阶段即usb的bulk传输第一阶段，只能接收数据，不能发送
				{
					printf("APPS: SUNXI_USB_EFEX_APPS_IDLE: INVALID direction\n");
					printf("sunxi usb efex app cmd err: usb transfer direction is receive only\n");

					return -1;
				}
			}
			else if((sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_SEND_DATA) ||			\
				(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_RECEIVE_DATA))	//收到的第二阶段，此时开始解析命令
			{
				sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_DATA\n");
				if(cbw->cmd_package.direction == TL_CMD_RECEIVE)	//如果要接收数据，则事先启动dma开始接收
				{
					sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_DATA: TL_CMD_RECEIVE\n");
					sunxi_ubuf->request_size = MIN(cbw->data_transfer_len, trans_data.recv_size);	//接收长度
					//sunxi_usb_dbg("try to receive data 0x%x\n", sunxi_ubuf->request_size);
					sunxi_usb_efex_write_enable = 0;				//设置标志
					if(sunxi_ubuf->request_size)
					{
						sunxi_usb_dbg("dma recv addr = 0x%x\n", trans_data.act_recv_buffer);
						sunxi_udc_start_recv_by_dma(trans_data.act_recv_buffer, sunxi_ubuf->request_size);	//start dma to receive data
					}
					else
					{
						printf("APPS: SUNXI_USB_EFEX_APPS_DATA: the send data length is 0\n");

						return -1;
					}
				}
				//处理命令，返回命令阶段的下一个状态
				sunxi_usb_efex_app_step = trans_data.app_next_status;	//根据命令，获取下一命令阶段状态
				//sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_CMD_DECODE finish\n");
				//sunxi_usb_dbg("sunxi_usb_efex_app_step = 0x%x\n", sunxi_usb_efex_app_step);
				sunxi_usb_efex_status   = sunxi_usb_efex_app_step & 0xffff;						//识别出传输阶段下一阶段状态
																								//可能是发送数据，接收数据，发送状态(csw)
			}
			else if(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_STATUS)
			{
				sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_STATUS\n");
				if(cbw->cmd_package.direction == TL_CMD_TRANSMIT)		//发送数据
				{
					sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_STATUS: TL_CMD_TRANSMIT\n");
					__sunxi_usb_efex_fill_status();

					sunxi_usb_efex_status = SUNXI_USB_EFEX_SEND_DATA;
					sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_IDLE;
				}
				else	//最后一个阶段，只能发送数据，不能接收
				{
					printf("APPS: SUNXI_USB_EFEX_APPS_STATUS: INVALID direction\n");
					printf("sunxi usb efex app status err: usb transfer direction is transmit only\n");
				}
			}
			else if(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_EXIT)
			{
				sunxi_usb_dbg("APPS: SUNXI_USB_EFEX_APPS_EXIT\n");
				__sunxi_usb_efex_fill_status();

				sunxi_usb_efex_status = SUNXI_USB_EFEX_SEND_DATA;
			}

			break;

	  	case SUNXI_USB_EFEX_SEND_DATA:

	  		sunxi_usb_dbg("SUNXI_USB_EFEX_SEND_DATA\n");
			{
				uint tx_length = MIN(cbw->data_transfer_len, trans_data.send_size);

#if defined(SUNXI_USB_30)
				sunxi_usb_efex_status_enable = 0;
#endif
				sunxi_usb_dbg("send data start 0x%x, size 0x%x\n", trans_data.act_send_buffer, tx_length);
				if(tx_length)
				{
					sunxi_udc_send_data((void *)trans_data.act_send_buffer, tx_length);
				}
				sunxi_usb_efex_status = SUNXI_USB_EFEX_STATUS;
				if(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_SEND_DATA)//来自于命令阶段，要求发送数据，下一阶段只能是发送状态(status_t)
				{
					sunxi_usb_dbg("SUNXI_USB_EFEX_SEND_DATA next: SUNXI_USB_EFEX_APPS_STATUS\n");
					sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_STATUS;
				}
				else if(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_EXIT)
				{
					sunxi_usb_dbg("SUNXI_USB_EFEX_SEND_DATA next: SUNXI_USB_EFEX_APPS_EXIT\n");
					sunxi_usb_efex_status = SUNXI_USB_EFEX_EXIT;
					//sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_EXIT;
				}
			}
	  		break;

	  	case SUNXI_USB_EFEX_RECEIVE_DATA:

	  		sunxi_usb_dbg("SUNXI_USB_RECEIVE_DATA\n");
			if(sunxi_usb_efex_write_enable == 1)		//数据部分接收完毕
			{
				csw.status = 0;
				//区分出是命令还是数据
				if(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_CMD)
				{
					//拷贝到cmd_buf，下次处理也需要
					sunxi_usb_dbg("SUNXI_USB_RECEIVE_DATA: SUNXI_USB_EFEX_APPS_CMD\n");
					if(sunxi_ubuf->request_size != CBW_MAX_CMD_SIZE)		//错误的数据，则返回
					{
						printf("sunxi usb efex err: received cmd size 0x%x is not equal to CBW_MAX_CMD_SIZE 0x%x\n", sunxi_ubuf->request_size, CBW_MAX_CMD_SIZE);

						sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
						csw.status = -1;
					}
					else
					{
						__sunxi_usb_efex_test_op_cmd(cmd_buf);
						csw.status = trans_data.last_err;
					}
					//sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_DATA;	//命令阶段，命令接收完成，下一阶段处理数据
					sunxi_usb_efex_app_step = trans_data.app_next_status;
				}
				else if(sunxi_usb_efex_app_step == SUNXI_USB_EFEX_APPS_RECEIVE_DATA)//来自于命令阶段，要求接收数据，下一阶段只能是发送状态(status_t)
				{
					//表示当次数据已经接收完成
					uint data_type = trans_data.type & SUNXI_EFEX_DATA_TYPE_MASK;

					sunxi_usb_dbg("SUNXI_USB_RECEIVE_DATA: SUNXI_USB_EFEX_APPS_RECEIVE_DATA\n");
					sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_STATUS;
					if(trans_data.type & SUNXI_EFEX_DRAM_MASK)		//表示属于内存数据，需要事先保存到内存中
					{
						sunxi_usb_dbg("SUNXI_EFEX_DRAM_MASK\n");
						if(trans_data.type & SUNXI_EFEX_TRANS_FINISH_TAG)	//表示当前类型数据已经接收完成
						{
                            dram_data_recv_finish(data_type);
                        }
						//数据还没有接收完毕，等待继续接收
					}
					else		//表示当前数据需要写入flash
					{
						sunxi_usb_dbg("SUNXI_EFEX_FLASH_MASK\n");
						if(!sunxi_sprite_write(trans_data.flash_start, trans_data.flash_sectors, (void *)trans_data.act_recv_buffer))
						{
							printf("sunxi usb efex err: write flash from 0x%x, sectors 0x%x failed\n", trans_data.flash_start, trans_data.flash_sectors);
							csw.status = -1;
							trans_data.last_err = -1;

							sunxi_usb_efex_app_step = SUNXI_USB_EFEX_APPS_IDLE;
						}
					}
				}
				sunxi_usb_efex_status   = SUNXI_USB_EFEX_STATUS;			//传输阶段，下一阶段传输状态(csw)
			}

			break;

            case SUNXI_USB_EFEX_SETUP_NEW:      //
            {
                sunxi_usb_dbg("SUNXI_USB_EFEX_SETUP_NEW\n");

                memcpy(cmd_buf,sunxi_ubuf->rx_req_buffer,FES_NEW_CMD_LEN);
#ifdef _EFEX_USE_BUF_QUEUE_ 
                //flush queue buff   when verify cmd or flash set off cmd coming
                if(FEX_CMD_fes_verify_value == ((struct global_cmd_s *)cmd_buf)->app_cmd
                   ||  FEX_CMD_fes_flash_set_off==  ((struct global_cmd_s *)cmd_buf)->app_cmd )
                {
                    if(efex_queue_write_all_page())
                    {
                        printf("efex queue error: buf_queue_write_all_page fail\n");
                        efex_write_error_flag = 1;
                    }
                }
#endif
                __sunxi_usb_efex_test_op_cmd(cmd_buf);
                csw.status = trans_data.last_err;
                csw.magic = CSW_MAGIC;      //"AWUS"
                csw.tag   = 0;

                if(csw.status != 0 )
                {
                    printf("sunxi usb cmd error: 0x%x\n",csw.status);
                    sunxi_ubuf->rx_ready_for_data = 0;
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;
                    return -1;
                }

#if defined(SUNXI_USB_30)
                sunxi_usb_efex_status_enable = 1;
#endif
                if(trans_data.app_next_status == SUNXI_USB_EFEX_APPS_SEND_DATA)
                {
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_SEND_DATA_NEW;
                }
                else if(trans_data.app_next_status == SUNXI_USB_EFEX_APPS_RECEIVE_DATA)
                {
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_RECEIVE_DATA_NEW;

                    sunxi_ubuf->request_size =  trans_data.recv_size;   //接收长度
                    sunxi_usb_efex_write_enable = 0;                //设置标志
                    if(sunxi_ubuf->request_size)
                    {
                        sunxi_usb_dbg("dma recv addr = 0x%x, size =0x%x\n", trans_data.act_recv_buffer,sunxi_ubuf->request_size);
                        sunxi_udc_start_recv_by_dma(trans_data.act_recv_buffer, sunxi_ubuf->request_size);  //start dma to receive data
                    }
                    else
                    {
                        printf("sunxi usb trans error: the request data length is 0\n");

                        return -1;
                    }

                }
                else if(trans_data.app_next_status == SUNXI_USB_EFEX_APPS_STATUS)
                {
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_STATUS;
                }
                else if(trans_data.app_next_status == SUNXI_USB_EFEX_APPS_EXIT)
                {
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_EXIT;
                }
                else
                {
                    printf("sunxi usb next status set error:0x%x\n", trans_data.app_next_status);
                    sunxi_ubuf->rx_ready_for_data = 0;
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;

                    return -1;
                }
                break;
			}

            case SUNXI_USB_EFEX_SEND_DATA_NEW:
                sunxi_usb_dbg("SUNXI_USB_EFEX_SEND_DATA_NEW\n");
                {
                    uint tx_length =  trans_data.send_size;
#if defined(SUNXI_USB_30)
                    sunxi_usb_efex_status_enable = 0;
#endif
                    sunxi_usb_dbg("dma send data start 0x%x, size 0x%x\n", trans_data.act_send_buffer, tx_length);
                    if(tx_length)
                    {
                        sunxi_udc_send_data((void *)trans_data.act_send_buffer, tx_length);
                    }
                    sunxi_usb_efex_status = SUNXI_USB_EFEX_STATUS;
                }
                break;

            case SUNXI_USB_EFEX_RECEIVE_DATA_NEW:
            {
                sunxi_usb_dbg("wait dma recv finish...\n");
                //wait for dma recv finish
                if(!sunxi_usb_efex_write_enable)
                {
#ifdef _EFEX_USE_BUF_QUEUE_
                    if(efex_queue_write_one_page())
                    {
                        printf("sunxi efex queue: buf_queue_write_one_page() err\n");
                        efex_write_error_flag = 1;
                    }
#endif
                    break;
                }
                sunxi_usb_dbg("SUNXI_USB_RECEIVE_DATA_NEW\n");

                //表示当次数据已经接收完成
                uint data_type = trans_data.type & SUNXI_EFEX_DATA_TYPE_MASK;
                if(trans_data.type & SUNXI_EFEX_DRAM_MASK)      //表示属于内存数据，需要事先保存到内存中
                {
                    sunxi_usb_dbg("SUNXI_EFEX_DRAM_MASK\n");
                    if(trans_data.type & SUNXI_EFEX_TRANS_FINISH_TAG)   //表示当前类型数据已经接收完成
                    {
                        dram_data_recv_finish(data_type);
                    }
                    //数据还没有接收完毕，等待继续接收
                }
                else        //表示当前数据需要写入flash
                {
                    sunxi_usb_dbg("SUNXI_EFEX_FLASH_MASK\n");
#ifdef _EFEX_USE_BUF_QUEUE_
                    if(0 != efex_save_buff_to_queue(trans_data.flash_start,trans_data.flash_sectors,(void *)trans_data.act_recv_buffer))
                    {
                        printf("efex queue not enough space...\n");
                        trans_data.last_err = -1;
                    }

#else
                    if(!sunxi_sprite_write(trans_data.flash_start, trans_data.flash_sectors, (void *)trans_data.act_recv_buffer))
                    {
                        printf("sunxi usb efex err: write flash from 0x%x, sectors 0x%x failed\n", trans_data.flash_start, trans_data.flash_sectors);
                        trans_data.last_err = -1;
                    }
#endif
                }
                csw.status = trans_data.last_err;
                sunxi_usb_efex_status   = SUNXI_USB_EFEX_STATUS;
            }
            break;

		case SUNXI_USB_EFEX_STATUS:
			sunxi_usb_dbg("SUNXI_USB_EFEX_STATUS\n");
#if defined(SUNXI_USB_30)
			if(sunxi_usb_efex_status_enable)
#endif
			{
				sunxi_usb_efex_status = SUNXI_USB_EFEX_IDLE;

				sunxi_ubuf->rx_ready_for_data = 0;
                                //when call efex queue write error,set stauts to tell usbtools
                                if(efex_write_error_flag)
                                {
                                        csw.status = -1;
                                }
				__sunxi_efex_send_status(&csw, sizeof(struct sunxi_efex_csw_t));
			}

			break;

		case SUNXI_USB_EFEX_EXIT:
			sunxi_usb_dbg("SUNXI_USB_EFEX_EXIT\n");

                        //when call efex queue write error,set stauts to tell usbtools
                        if(efex_write_error_flag)
                        {
                                csw.status = -1;
                        }
#if defined(SUNXI_USB_30)
			if(sunxi_usb_efex_status_enable == 1)
			{
				sunxi_ubuf->rx_ready_for_data = 0;

				__sunxi_efex_send_status(&csw, sizeof(struct sunxi_efex_csw_t));
			}
			else if(sunxi_usb_efex_status_enable >= 2)
			{
				return sunxi_efex_next_action;
			}
#else
			sunxi_ubuf->rx_ready_for_data = 0;

			__sunxi_efex_send_status(&csw, sizeof(struct sunxi_efex_csw_t));

			return sunxi_efex_next_action;
#endif
	  	default:
	  		break;
	}

	return 0;
}

sunxi_usb_module_init(SUNXI_USB_DEVICE_EFEX_TEST,					\
					  sunxi_efex_test_init,							\
					  sunxi_efex_test_exit,							\
					  sunxi_efex_test_reset,							\
					  sunxi_efex_test_standard_req_op,				\
					  sunxi_efex_test_nonstandard_req_op,			\
					  sunxi_efex_test_state_loop,					\
					  sunxi_efex_test_usb_rx_dma_isr,				\
					  sunxi_efex_test_usb_tx_dma_isr					\
					  );

