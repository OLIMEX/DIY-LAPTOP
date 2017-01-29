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
#include "twofish.h"
#include "twofish_new.h"


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------

//typedef void * HTF;

#define HTF_MAGIC	"TFMG"


TWI	twi_org;

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
HTF TFInitial(uint * key, uint keylen)
{
	TWI	*twi = &twi_org;

	if (NULL == key)
		return NULL;

	if (0 != (keylen %4))
		return NULL;

	switch (keylen)
	{
	case KEY_LEN_128_BIT://		(128 / 8)	//128 bit
		break;
	case KEY_LEN_192_BIT://		(192 / 8)	//192 bit
		break;
	case KEY_LEN_256_BIT://		(256 / 8)	//256 bit
		break;
	default:
		return NULL;
	}

	memset((void *)twi, 0, sizeof(TWI));

	twofish_new_set_key(twi, (uint *)key, keylen);

	return twi;
}

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------

__u32 TFDecode(HTF hTF, void * ibuf, uint len, void * obuf)
{
	TWI	*twi = (TWI	*)hTF;
    //decode data
	uint left_len = len;
	uint this_len = 0;
	uint offset_len = 0;
	unsigned char  *iaddr, *oaddr;

    iaddr = (unsigned char *)ibuf;
    oaddr = (unsigned char *)obuf;
	//逐个分组进行处理
	while(left_len > 0)
	{
		this_len = (left_len > TF_SIZE)? (TF_SIZE) : (left_len);
		twofish_new_decrypt(twi, (uint *)(iaddr +  offset_len), (uint *)(oaddr + offset_len));
        offset_len += this_len;
		left_len -= this_len;
	}

	return 0;
}


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
__u32 TFUnInitial(HTF hTF)
{
	uint ret = 0;
	TWI	*twi = (TWI	*)hTF;

	if (NULL == twi)
		return __LINE__;

	memset(twi, 0, sizeof(TWI));
	twi = NULL;

	return ret;
}



