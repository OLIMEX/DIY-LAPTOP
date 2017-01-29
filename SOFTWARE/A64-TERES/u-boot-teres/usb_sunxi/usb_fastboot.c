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
#include <fastboot.h>
#include "usb_fastboot.h"
#include <android_misc.h>
#include <sunxi_board.h>
#include <power/sunxi/pmu.h>
#include <sunxi_mbr.h>
#include <sunxi_flash.h>
#include <private_uboot.h>
#include "../sprite/sparse/sparse.h"
#include "../sprite/sprite_download.h"

DECLARE_GLOBAL_DATA_PTR;

int do_go(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

static  int sunxi_usb_fastboot_write_enable = 0;
static  int sunxi_usb_fastboot_status = SUNXI_USB_FASTBOOT_IDLE;

static  fastboot_trans_set_t  trans_data;

static  uint  all_download_bytes;

int     fastboot_data_flag;

extern int sunxi_usb_exit(void);
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
			dev_dscrptr->bDeviceClass       = 0xff;		//设备类：大容量存储
			dev_dscrptr->bDeviceSubClass    = 0xff;
			dev_dscrptr->bDeviceProtocol    = 0xff;
			dev_dscrptr->bMaxPacketSize0    = 0x40;
			dev_dscrptr->idVendor           = DEVICE_VENDOR_ID;
			dev_dscrptr->idProduct          = DEVICE_PRODUCT_ID;
			dev_dscrptr->bcdDevice          = DEVICE_BCD;
			dev_dscrptr->iManufacturer      = SUNXI_FASTBOOT_DEVICE_STRING_MANUFACTURER_INDEX;
			dev_dscrptr->iProduct           = SUNXI_FASTBOOT_DEVICE_STRING_PRODUCT_INDEX;
			dev_dscrptr->iSerialNumber      = SUNXI_FASTBOOT_DEVICE_STRING_SERIAL_NUMBER_INDEX;
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
			config_dscrptr->iConfiguration     	= SUNXI_FASTBOOT_DEVICE_STRING_CONFIG_INDEX;
			config_dscrptr->bmAttributes       	= 0xC0;
			config_dscrptr->bMaxPower          	= 0xFA;		//最大电流500ms(0xfa * 2)

			bytes_remaining 				   -= config_dscrptr->bLength;
			/* interface */
			inter_dscrptr->bLength             = MIN (bytes_remaining, sizeof(struct usb_interface_descriptor));
			inter_dscrptr->bDescriptorType     = USB_DT_INTERFACE;
			inter_dscrptr->bInterfaceNumber    = 0x00;
			inter_dscrptr->bAlternateSetting   = 0x00;
			inter_dscrptr->bNumEndpoints       = 0x02;
			inter_dscrptr->bInterfaceClass     = 0xff;		//fastboot storage
			inter_dscrptr->bInterfaceSubClass  = FASTBOOT_INTERFACE_SUB_CLASS;
			inter_dscrptr->bInterfaceProtocol  = FASTBOOT_INTERFACE_PROTOCOL;
			inter_dscrptr->iInterface          = SUNXI_FASTBOOT_DEVICE_STRING_INTERFACE_INDEX;

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

				sunxi_udc_send_setup(bLength, (void *)sunxi_usb_fastboot_dev[0]);
			}
			else if(string_index < SUNXI_USB_FASTBOOT_DEV_MAX)
			{
				/* Size of string in chars */
				unsigned char i = 0;
				unsigned char str_length = strlen ((const char *)sunxi_usb_fastboot_dev[string_index]);
				unsigned char bLength = 2 + (2 * str_length);

				buffer[0] = bLength;        /* length */
				buffer[1] = USB_DT_STRING;  /* descriptor = string */

				/* Copy device string to fifo, expand to simple unicode */
				for(i = 0; i < str_length; i++)
				{
					buffer[2+ 2*i + 0] = sunxi_usb_fastboot_dev[string_index][i];
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
			//qua_dscrpt->bRESERVED          = 0;
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
*                     __usb_get_status
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
static int __sunxi_fastboot_send_status(void *buffer, unsigned int buffer_size)
{
	return sunxi_udc_send_data((uchar *)buffer, buffer_size);
}
/*
*******************************************************************************
*                     __fastboot_reboot
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
static int __fastboot_reboot(int word_mode)
{
	char response[8];

	sprintf(response,"OKAY");
	__sunxi_fastboot_send_status(response, strlen(response));
	__msdelay(1000); /* 1 sec */

	sunxi_board_restart(word_mode);

	return 0;
}
/*
*******************************************************************************
*                     __erase_part
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
static int __erase_part(char *name)
{
	void *addr  = (void *)FASTBOOT_ERASE_BUFFER;
	u32   start, unerased_sectors;
	u32   nblock = FASTBOOT_ERASE_BUFFER_SIZE/512;
	char  response[68];

	start            = sunxi_partition_get_offset_byname(name);
	unerased_sectors = sunxi_partition_get_size_byname(name);

    if(gd->lockflag == SUNXI_LOCKING || gd->lockflag == SUNXI_RELOCKING)
    {
        printf("in lock state, sunxi fastboot erase is disabled\n");
		__sunxi_fastboot_send_status(response, strlen(response));
        return 0;
    }

	if((!start) || (!unerased_sectors))
	{
		printf("sunxi fastboot erase FAIL: partition %s does not exist\n", name);
		sprintf(response, "FAILerase: partition %s does not exist", name);

		__sunxi_fastboot_send_status(response, strlen(response));

		return -1;
	}

	memset(addr, 0xff, FASTBOOT_ERASE_BUFFER_SIZE);
	while(unerased_sectors >= nblock)
	{
		if(!sunxi_flash_write(start, nblock, addr))
		{
			printf("sunxi fastboot erase FAIL: failed to erase partition %s \n", name);
			sprintf(response,"FAILerase: failed to erase partition %s", name);

			__sunxi_fastboot_send_status(response, strlen(response));

			return -1;
		}
		start += nblock;
		unerased_sectors -= nblock;
	}
	if(unerased_sectors)
	{
		if(!sunxi_flash_write(start, unerased_sectors, addr))
		{
			printf("sunxi fastboot erase FAIL: failed to erase partition %s \n", name);
			sprintf(response,"FAILerase: failed to erase partition %s", name);

			__sunxi_fastboot_send_status(response, strlen(response));

			return -1;
		}
	}

	printf("sunxi fastboot: partition '%s' erased\n", name);
	sprintf(response, "OKAY");

	__sunxi_fastboot_send_status(response, strlen(response));

	return 0;
}
/*
*******************************************************************************
*                     __flash_to_part
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
static void __flash_to_uboot(void)
{
	struct spare_boot_head_t * temp_buf = (struct spare_boot_head_t *)trans_data.base_recv_buffer;
	char  response[68];
	u32 uboot_length = 0;
	u32 align_size = 0;
	u32 old_uboot_length = 0;
	u32 old_total_length = 0;

	if(strcmp((char*)temp_buf->boot_head.magic,"uboot"))
	{
		printf("sunxi fastboot error: there is not uboot file\n");
		sprintf(response, "FAILdownload:there is not uboot file \n");
		__sunxi_fastboot_send_status(response, strlen(response));
		return ;
	}
	printf("ready to download bytes 0x%x\n", trans_data.try_to_recv);
	if(temp_buf->boot_head.uboot_length == 0)
	{
		printf("==== uboot.bin ====\n");
		memcpy((char *)temp_buf , (char *)CONFIG_SYS_TEXT_BASE ,sizeof(struct spare_boot_head_t));
		old_uboot_length = temp_buf->boot_head.uboot_length;
		old_total_length = temp_buf->boot_head.length ;
		debug("old_uboot_length = %x \n",old_uboot_length);
		debug("old_total_length = %x \n",old_total_length);
		//align uboot
		align_size = temp_buf->boot_head.align_size;
		printf("align_size is 0x%x \n",align_size);

		uboot_length = (trans_data.try_to_recv + align_size) & (~(align_size - 1));
		temp_buf->boot_head.uboot_length = uboot_length;
		//copy sys_config from old uboot
		memcpy((char *)temp_buf + uboot_length , (char *)CONFIG_SYS_TEXT_BASE + old_uboot_length,old_total_length - old_uboot_length);
		//make check_sum again
		temp_buf->boot_head.check_sum = STAMP_VALUE;
		temp_buf->boot_head.length     = uboot_length +old_total_length - old_uboot_length;
		temp_buf->boot_head.check_sum = add_sum((char *)temp_buf,temp_buf->boot_head.length);
	}
	printf("uboot checksum is 0x%x \n",temp_buf->boot_head.check_sum);
	printf("download uboot ing ....\n");
	sunxi_sprite_download_uboot((char *)temp_buf,uboot_spare_head.boot_data.storage_type ,1);
	printf("sunxi fastboot: successed in downloading uboot \n");
	sprintf(response, "OKAY");
	__sunxi_fastboot_send_status(response, strlen(response));

	return ;
}

static int __flash_to_part(char *name)
{
	char *addr = trans_data.base_recv_buffer;
	u32   start, data_sectors;
	u32   part_sectors;
	u32   nblock = FASTBOOT_TRANSFER_BUFFER_SIZE/512;
	char  response[68];

	start        = sunxi_partition_get_offset_byname(name);
	part_sectors = sunxi_partition_get_size_byname(name);

    if(gd->lockflag == SUNXI_LOCKING || gd->lockflag == SUNXI_RELOCKING)
    {
        printf("in lock state, sunxi fastboot flash is disabled\n");
		__sunxi_fastboot_send_status(response, strlen(response));
        return 0;
    }

	if((!start) || (!part_sectors))
	{
		uint  addr_in_hex;
		int   ret;

		printf("sunxi fastboot download FAIL: partition %s does not exist\n", name);
		printf("probe it as a dram address\n");

		ret = strict_strtoul((const char *)name, 16, (long unsigned int*)&addr_in_hex);
		if(ret)
		{
			printf("sunxi fatboot download FAIL: it is not a dram address\n");

			sprintf(response, "FAILdownload: partition %s does not exist", name);
			__sunxi_fastboot_send_status(response, strlen(response));

			return -1;
		}
		else
		{
			printf("ready to move data to 0x%x, bytes 0x%x\n", addr_in_hex, trans_data.try_to_recv);
			memcpy((void *)(ulong)addr_in_hex, addr, trans_data.try_to_recv);
		}
	}
	else
	{
		int  format;

		printf("ready to download bytes 0x%x\n", trans_data.try_to_recv);
		format = unsparse_probe(addr, trans_data.try_to_recv, start);

		if(ANDROID_FORMAT_DETECT == format)
		{
			if(unsparse_direct_write(addr, trans_data.try_to_recv))
			{
				printf("sunxi fastboot download FAIL: failed to write partition %s \n", name);
				sprintf(response,"FAILdownload: write partition %s err", name);

				return -1;
			}
		}
		else
		{
		    data_sectors = (trans_data.try_to_recv + 511)/512;
		    if(data_sectors > part_sectors)
		    {
		    	printf("sunxi fastboot download FAIL: partition %s size 0x%x is smaller than data size 0x%x\n", name, trans_data.act_recv, data_sectors * 512);
				sprintf(response, "FAILdownload: partition size < data size");

				__sunxi_fastboot_send_status(response, strlen(response));

				return -1;
		    }
			while(data_sectors >= nblock)
			{
				if(!sunxi_flash_write(start, nblock, addr))
				{
					printf("sunxi fastboot download FAIL: failed to write partition %s \n", name);
					sprintf(response,"FAILdownload: write partition %s err", name);

					__sunxi_fastboot_send_status(response, strlen(response));

					return -1;
				}
				start += nblock;
				data_sectors -= nblock;
				addr  += FASTBOOT_TRANSFER_BUFFER_SIZE;
			}
			if(data_sectors)
			{
				if(!sunxi_flash_write(start, data_sectors, addr))
				{
					printf("sunxi fastboot download FAIL: failed to write partition %s \n", name);
					sprintf(response,"FAILdownload: write partition %s err", name);

					__sunxi_fastboot_send_status(response, strlen(response));

					return -1;
				}
			}
		}
	}

	printf("sunxi fastboot: successed in downloading partition '%s'\n", name);
	sprintf(response, "OKAY");

	__sunxi_fastboot_send_status(response, strlen(response));

	return 0;
}


/*
*******************************************************************************
*                     __try_to_download
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
static int __try_to_download(char *download_size, char *response)
{
	int  ret = -1;

	trans_data.try_to_recv = simple_strtoul (download_size, NULL, 16);
	all_download_bytes = trans_data.try_to_recv;
	printf("Starting download of %d BYTES\n", trans_data.try_to_recv);
	printf("Starting download of %d MB\n", trans_data.try_to_recv >> 20);

	if (0 == trans_data.try_to_recv)
	{
		/* bad user input */
		sprintf(response, "FAILdownload: data size is 0");
	}
	else if (trans_data.try_to_recv > SUNXI_USB_FASTBOOT_BUFFER_MAX)
	{
		sprintf(response, "FAILdownload: data > buffer");
	}
	else
	{
		/* The default case, the transfer fits
		   completely in the interface buffer */
		sprintf(response, "DATA%08x", trans_data.try_to_recv);
		printf("download response: %s\n", response);

		ret = 0;
	}

	return ret;
}
/*
*******************************************************************************
*                     __boot
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
static void __boot(void)
{
	char  response[68];

	if(all_download_bytes > CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE)
	{
		char start[32];
		char *bootm[3] = { "bootm", NULL, NULL, };
		char *go[3]    = { "go",    NULL, NULL, };

		struct fastboot_boot_img_hdr *fb_hdr =
			(struct fastboot_boot_img_hdr *) trans_data.base_recv_buffer;

		/* Skip the mkbootimage header */
		image_header_t *hdr =
			(image_header_t *)
			&trans_data.base_recv_buffer[CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE];

		bootm[1] = go[1] = start;
		sprintf(start, "0x%lx", (ulong)hdr);

		printf("start addr %s\n", start);
		/* Execution should jump to kernel so send the response
		   now and wait a bit.  */
		sprintf(response, "OKAY");
//		fastboot_tx_status(response, strlen(response));
		__msdelay (1000); /* 1 sec */

		if (ntohl(hdr->ih_magic) == IH_MAGIC) {
			/* Looks like a kernel.. */
			printf ("Booting kernel..\n");

			/*
			 * Check if the user sent a bootargs down.
			 * If not, do not override what is already there
			 */
			if (strlen ((char *) &fb_hdr->cmdline[0])) {
				printf("Image has cmdline:");
				printf("%s\n", &fb_hdr->cmdline[0]);
				setenv ("bootargs", (char *) &fb_hdr->cmdline[0]);
			}
			do_bootm (NULL, 0, 2, bootm);
		} else {
			/* Raw image, maybe another uboot */
			printf ("Booting raw image..\n");

			do_go (NULL, 0, 2, go);
		}
		printf ("ERROR : bootting failed\n");
		printf ("You should reset the board\n");
	}
	else
	{
		sprintf(response, "FAILinvalid boot image");
	}
}
/*
*******************************************************************************
*                     __get_var
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
static void __get_var(char *ver_name)
{
	char response[68];

	memset(response, 0, 68);
	strcpy(response,"OKAY");

	if(!strcmp(ver_name, "version"))
	{
		strcpy(response + 4, FASTBOOT_VERSION);
	}
	else if(!strcmp(ver_name, "product"))
	{
		strcpy(response + 4, SUNXI_FASTBOOT_DEVICE_PRODUCT);
	}
	else if(!strcmp(ver_name, "serialno"))
	{
		strcpy(response + 4, SUNXI_FASTBOOT_DEVICE_SERIAL_NUMBER);
	}
	else if(!strcmp(ver_name, "downloadsize"))
	{
		sprintf(response + 4, "0x%08x", SUNXI_USB_FASTBOOT_BUFFER_MAX);
		printf("response: %s\n", response);
	}
	else if(!strcmp(ver_name, "secure"))
	{
		strcpy(response + 4, "yes");
	}
	else if(!strcmp(ver_name, "max-download-size"))
	{
		sprintf(response + 4, "0x%08x", SUNXI_USB_FASTBOOT_BUFFER_MAX);
		printf("response: %s\n", response);
	}
	else
	{
		strcpy(response + 4, "not supported");
	}

	__sunxi_fastboot_send_status(response, strlen(response));

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
static void __oem_operation(char *operation)
{
	#if 0
	char response[68];
	char lock_info[64];
	int  lockflag;
	int  ret;

	memset(lock_info, 0, 64);
	memset(response, 0, 68);

	if(!strncmp(operation, "lock", 4))
	{
		lockflag = SUNXI_RELOCKING;
	}
	else if(!strncmp(operation, "unlock", 6))
	{
		lockflag = SUNXI_UNLOCK;
	}
	else
	{
		if(!strncmp(operation, "efex", 4))
		{
			strcpy(response, "OKAY");
			__sunxi_fastboot_send_status(response, strlen(response));

			sunxi_board_run_fel();
		}
		else
		{
			const char *info = "fastboot oem operation fail: unknown cmd";

			printf("%s\n", info);
			strcpy(response, "FAIL");
			strcat(response, info);

			__sunxi_fastboot_send_status(response, strlen(response));
		}

		return ;
	}

	ret = sunxi_oem_op_lock(lockflag, lock_info, 0);
	if(!ret)
	{
		strcpy(response, "OKAY");
	}
	else
	{
		strcpy(response, "FAIL");
	}
	strcat(response, lock_info);
	printf("%s\n", response);

	__sunxi_fastboot_send_status(response, strlen(response));

	return ;
	#endif
	printf("not implement yet \n");
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
static void __continue(void)
{
	char response[32];

	memset(response, 0, 32);
	strcpy(response,"OKAY");

	__sunxi_fastboot_send_status(response, strlen(response));

	sunxi_usb_exit();

	if(uboot_spare_head.boot_data.storage_type)
	{
		setenv("bootcmd", "run setargs_mmc boot_normal");
	}
	else
	{
		setenv("bootcmd", "run setargs_nand boot_normal");
	}
	do_bootd(NULL, 0, 1, NULL);

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
static void __unsupported_cmd(void)
{
	char response[32];

	memset(response, 0, 32);
	strcpy(response,"FAIL");

	__sunxi_fastboot_send_status(response, strlen(response));

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
static int sunxi_fastboot_init(void)
{
	printf("sunxi_fastboot_init\n");
	memset(&trans_data, 0, sizeof(fastboot_trans_set_t));
	sunxi_usb_fastboot_write_enable = 0;
    sunxi_usb_fastboot_status = SUNXI_USB_FASTBOOT_IDLE;

	all_download_bytes = 0;
	fastboot_data_flag = 0;

    trans_data.base_recv_buffer = (char *)FASTBOOT_TRANSFER_BUFFER;

	trans_data.base_send_buffer = (char *)malloc(SUNXI_FASTBOOT_SEND_MEM_SIZE);
    if(!trans_data.base_send_buffer)
    {
    	printf("sunxi usb fastboot err: unable to malloc memory for fastboot send\n");
    	free(trans_data.base_recv_buffer);

    	return -1;
    }
	printf("recv addr 0x%lx\n", (ulong)trans_data.base_recv_buffer);
    printf("send addr 0x%lx\n", (ulong)trans_data.base_send_buffer);

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
static int sunxi_fastboot_exit(void)
{
	printf("sunxi_fastboot_exit\n");
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
static void sunxi_fastboot_reset(void)
{
	sunxi_usb_fastboot_write_enable = 0;
    sunxi_usb_fastboot_status = SUNXI_USB_FASTBOOT_IDLE;
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
static void  sunxi_fastboot_usb_rx_dma_isr(void *p_arg)
{
	printf("dma int for usb rx occur\n");
	//通知主循环，可以写入数据
	sunxi_usb_fastboot_write_enable = 1;
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
static void  sunxi_fastboot_usb_tx_dma_isr(void *p_arg)
{
	printf("dma int for usb tx occur\n");
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
static int sunxi_fastboot_standard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer)
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
			tick_printf("sunxi fastboot error: standard req is not supported\n");

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
static int sunxi_fastboot_nonstandard_req_op(uint cmd, struct usb_device_request *req, uchar *buffer, uint data_status)
{
	return SUNXI_USB_REQ_DEVICE_NOT_SUPPORTED;
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
static int sunxi_fastboot_state_loop(void  *buffer)
{
	int ret;
	sunxi_ubuf_t *sunxi_ubuf = (sunxi_ubuf_t *)buffer;
	char  response[68];

	switch(sunxi_usb_fastboot_status)
	{
		case SUNXI_USB_FASTBOOT_IDLE:
			if(sunxi_ubuf->rx_ready_for_data == 1)
			{
				sunxi_usb_fastboot_status = SUNXI_USB_FASTBOOT_SETUP;
			}

			break;

		case SUNXI_USB_FASTBOOT_SETUP:

			tick_printf("SUNXI_USB_FASTBOOT_SETUP\n");

			tick_printf("fastboot command = %s\n", sunxi_ubuf->rx_req_buffer);

			sunxi_usb_fastboot_status = SUNXI_USB_FASTBOOT_IDLE;
			sunxi_ubuf->rx_ready_for_data = 0;
			if(memcmp(sunxi_ubuf->rx_req_buffer, "reboot-bootloader", strlen("reboot-bootloader")) == 0)
			{
				tick_printf("reboot-bootloader\n");
				__fastboot_reboot(PMU_PRE_FASTBOOT_MODE);
			}
            else if(memcmp(sunxi_ubuf->rx_req_buffer, "reboot", 6) == 0)
			{
				tick_printf("reboot\n");
				__fastboot_reboot(0);
			}
			else if(memcmp(sunxi_ubuf->rx_req_buffer, "erase:", 6) == 0)
			{
				tick_printf("erase\n");
				__erase_part((char *)(sunxi_ubuf->rx_req_buffer + 6));
			}
			else if(memcmp(sunxi_ubuf->rx_req_buffer, "flash:", 6) == 0)
			{
				tick_printf("flash\n");
				if(memcmp((char *)(sunxi_ubuf->rx_req_buffer + 6),"u-boot",6) == 0)
					__flash_to_uboot();
				else
					__flash_to_part((char *)(sunxi_ubuf->rx_req_buffer + 6));
			}
			else if(memcmp(sunxi_ubuf->rx_req_buffer, "download:", 9) == 0)
			{
				tick_printf("download\n");
				ret = __try_to_download((char *)(sunxi_ubuf->rx_req_buffer + 9), response);
				if(ret >= 0)
				{
					fastboot_data_flag = 1;
					sunxi_ubuf->rx_req_buffer  = (uchar *)trans_data.base_recv_buffer;
					sunxi_usb_fastboot_status  = SUNXI_USB_FASTBOOT_RECEIVE_DATA;
				}
				__sunxi_fastboot_send_status(response, strlen(response));
			}
			else if(memcmp(sunxi_ubuf->rx_req_buffer, "boot", 4) == 0)
			{
				tick_printf("boot\n");
				__boot();
			}
			else if(memcmp(sunxi_ubuf->rx_req_buffer, "getvar:", 7) == 0)
			{
				tick_printf("getvar\n");
				__get_var((char *)(sunxi_ubuf->rx_req_buffer + 7));
			}
			else if(memcmp(sunxi_ubuf->rx_req_buffer, "oem", 3) == 0)
			{
				tick_printf("oem operations\n");
				__oem_operation((char *)(sunxi_ubuf->rx_req_buffer + 4));
			}
			else if(memcmp(sunxi_ubuf->rx_req_buffer, "continue", 8) == 0)
			{
				tick_printf("continue\n");
				__continue();
			}
			else
			{
				tick_printf("not supported fastboot cmd\n");
				__unsupported_cmd();
			}

			break;

	  	case SUNXI_USB_FASTBOOT_SEND_DATA:

	  		tick_printf("SUNXI_USB_FASTBOOT_SEND_DATA\n");

	  		break;

	  	case SUNXI_USB_FASTBOOT_RECEIVE_DATA:

	  		//tick_printf("SUNXI_USB_FASTBOOT_RECEIVE_DATA\n");
	  		if((fastboot_data_flag == 1) && ((char *)sunxi_ubuf->rx_req_buffer == all_download_bytes + trans_data.base_recv_buffer))	//传输完毕
	  		{
	  			tick_printf("fastboot transfer finish\n");
	  			fastboot_data_flag = 0;
	  			sunxi_usb_fastboot_status  = SUNXI_USB_FASTBOOT_IDLE;

		  		sunxi_ubuf->rx_req_buffer = sunxi_ubuf->rx_base_buffer;

		  		sprintf(response,"OKAY");
		  		__sunxi_fastboot_send_status(response, strlen(response));
			}

	  		break;

	  	default:
	  		break;
	}

	return 0;
}


sunxi_usb_module_init(SUNXI_USB_DEVICE_FASTBOOT,					\
					  sunxi_fastboot_init,							\
					  sunxi_fastboot_exit,							\
					  sunxi_fastboot_reset,							\
					  sunxi_fastboot_standard_req_op,				\
					  sunxi_fastboot_nonstandard_req_op,			\
					  sunxi_fastboot_state_loop,					\
					  sunxi_fastboot_usb_rx_dma_isr,				\
					  sunxi_fastboot_usb_tx_dma_isr					\
					  );
