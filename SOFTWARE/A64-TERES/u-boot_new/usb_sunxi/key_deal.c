/*
 * (C) Copyright 2007-2015
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
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <securestorage.h>
#include <smc.h>

void hdcp_key_convert(unsigned char *keyi,unsigned char *keyo)
{
	unsigned i;
	for(i=0; i<5; i++)
	{
		keyo[i] = keyi[i];
	}

	keyo[5] = keyo[6] = 0;

	for(i=0; i<280; i++)
	{
		keyo[7+i] = keyi[8+i];
	}

	keyo[287] = 0;
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
int sunxi_deal_hdcp_key(char *keydata, int keylen)
{
	int  ret, real_len;
	char buffer[4096];
	char buffer_convert[4096];

	hdcp_key_convert((unsigned char *)keydata, (unsigned char *)buffer_convert);

	ret = smc_aes_bssk_encrypt_to_dram(buffer_convert, 288, buffer, &real_len);
	if(ret<0)
	{
		printf("smc aes bssk encrypt failed\n");

		return -1;
	}

	ret = sunxi_secure_storage_write("hdcpkey", buffer, real_len);
	if(ret<0)
	{
		printf("sunxi secure storage write failed\n");

		return -1;
	}

	return 0;
}

