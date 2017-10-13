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
#include <common.h>
#include <asm/arch/usb.h>

#define  SUNXI_USB_DMA_A50_MAX   (8)

static uint usb_hd;
static uint usb_dma_used[SUNXI_USB_DMA_A50_MAX];
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
static int __usb_index_check(uint dma_index)
{
	if(dma_index > SUNXI_USB_DMA_A50_MAX)
	{
		printf("usb dma %d is overrange\n", dma_index);

		return -1;
	}
	if(!usb_dma_used[dma_index])
	{
		printf("usb dma %d is not used\n", dma_index);

		return -1;
	}

	return 0;
}
/*
****************************************************************************************************
*
*             DMAC_RequestDma
*
*  Description:
*       request dma
*
*  Parameters:
*		type	0: normal timer
*				1: special timer
*  Return value:
*		dma handler
*		if 0, fail
****************************************************************************************************
*/
int usb_dma_init(uint husb)
{
	usb_hd = husb;

	return 0;
}
/*
****************************************************************************************************
*
*             DMAC_RequestDma
*
*  Description:
*       request dma
*
*  Parameters:
*		type	0: normal timer
*				1: special timer
*  Return value:
*		dma handler
*		if 0, fail
****************************************************************************************************
*/
uint usb_dma_request(void)
{
    int   i;

	for(i=1;i<SUNXI_USB_DMA_A50_MAX;i++)
    {
        if(usb_dma_used[i] == 0)
        {
        	usb_dma_used[i] = 1;

            return i;
        }
    }

    return 0;
}
/*
****************************************************************************************************
*
*             DMAC_ReleaseDma
*
*  Description:
*       release dma
*
*  Parameters:
*       hDma	dma handler
*
*  Return value:
*		EPDK_OK/FAIL
****************************************************************************************************
*/
int usb_dma_release(uint dma_index)
{
	int ret;

	ret = __usb_index_check(dma_index);
	if(ret)
	{
		return ret;
	}

	usb_dma_used[dma_index] = 0;

    return 0;
}
/*
****************************************************************************************************
*
*             sunxi_dma_setting
*
*  Description:
*       start interrupt
*
*  Parameters:
*
*
*
*
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int usb_dma_setting(uint dma_index, uint trans_dir, uint ep)
{
	int ret;

	ret = __usb_index_check(dma_index);
	if(ret)
	{
		return ret;
	}

	return USBC_Dma_Set_ChannalPara(usb_hd, dma_index, trans_dir, ep);
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_start
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
****************************************************************************************************
*/
int usb_dma_set_pktlen(uint dma_index, uint pkt_len)
{
	int ret;

	ret = __usb_index_check(dma_index);
	if(ret)
	{
		return ret;
	}

	return USBC_Dma_Set_PktLen(usb_hd, dma_index, pkt_len);
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_start
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
****************************************************************************************************
*/
int usb_dma_start(uint dma_index, uint addr, uint bytes)
{
	int ret;

	ret = __usb_index_check(dma_index);
	if(ret)
	{
		return ret;
	}

	return USBC_Dma_Start(usb_hd, dma_index, addr, bytes);
}
/*
**********************************************************************************************************************
*
*             usb_dma_int_enable
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int usb_dma_stop(uint dma_index)
{
	int ret;

	ret = __usb_index_check(dma_index);
	if(ret)
	{
		return ret;
	}

	return USBC_Dma_Int_Stop(usb_hd, dma_index);
}
/*
**********************************************************************************************************************
*
*             usb_dma_int_query
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int usb_dma_int_query(uint dma_index)
{
	return USBC_Dma_Int_Query(usb_hd);
}
/*
**********************************************************************************************************************
*
*             usb_dma_int_query
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int usb_dma_int_clear(void)
{
	return USBC_Dma_Int_Clear(usb_hd);
}

