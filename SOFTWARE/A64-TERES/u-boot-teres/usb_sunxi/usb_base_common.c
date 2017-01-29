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
#include "usb_module.h"

extern sunxi_usb_setup_req_t     *sunxi_udev_active;
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
int sunxi_usb_dev_register(uint dev_name)
{
	int ret = 0;
	sunxi_usb_dbg("sunxi_usb_dev_register\n");
	switch(dev_name)
	{
#ifdef  SUNXI_USB_DEVICE_MASS
		case SUNXI_USB_DEVICE_MASS:
			sunxi_usb_dbg("register SUNXI_USB_DEVICE_MASS begin\n");
			sunxi_usb_module_reg(SUNXI_USB_DEVICE_MASS);
			sunxi_usb_dbg("register SUNXI_USB_DEVICE_MASS ok\n");
			break;
#endif

#ifdef  SUNXI_USB_DEVICE_EFEX
		case SUNXI_USB_DEVICE_EFEX:
			sunxi_usb_module_reg(SUNXI_USB_DEVICE_EFEX);

			break;
#endif

#ifdef  SUNXI_USB_DEVICE_FASTBOOT
		case SUNXI_USB_DEVICE_FASTBOOT:
			sunxi_usb_module_reg(SUNXI_USB_DEVICE_FASTBOOT);

			break;
#endif

#ifdef SUNXI_USB_DEVICE_BURN
        case SUNXI_USB_DEVICE_BURN:
        	sunxi_usb_module_reg(SUNXI_USB_DEVICE_BURN);

        	break;
#endif

#ifdef SUNXI_USB_DEVICE_EFEX_TEST
        case SUNXI_USB_DEVICE_EFEX_TEST:
            sunxi_usb_module_reg(SUNXI_USB_DEVICE_EFEX_TEST);
            break;
#endif
		default:
			ret = -1;
			break;
	}

	return ret;
}








