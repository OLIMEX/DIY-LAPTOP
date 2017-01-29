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
#ifndef __twofish_h__
#define __twofish_h__

#include    "twofish.h"

#define rotl(x,n)   (((x) << ((__u32)(n))) | ((x) >> (32 - (__u32)(n))))
#define rotr(x,n)   (((x) >> ((__u32)(n))) | ((x) << (32 - (__u32)(n))))
#define rotl16(x,n)   (((x) << ((u16)(n))) | ((x) >> (16 - (u16)(n))))
#define rotr16(x,n)   (((x) >> ((u16)(n))) | ((x) << (16 - (u16)(n))))
#define byteswap(x)	((rotl(x, 8) & 0x00ff00ff) | (rotr(x, 8) & 0xff00ff00))

typedef struct
{
	uint  k_len;
	uint  l_key[40];
	uint  s_key[4];

	uint qt_gen;
	unsigned char q_tab[2][256];

	uint mt_gen;
	uint m_tab[4][256];

	uint mk_tab[4][256];
}
__attribute__ ((packed)) TWI;

void twofish_new_set_key(TWI *pkey,const uint in_key[],const uint key_len);
void twofish_new_encrypt(TWI *pkey,uint *in_blk);
//void twofish_new_decrypt(TWI *pkey,uint *in_blk);
void twofish_new_decrypt(TWI * pkey, uint * in_blk, uint * out_blk);

#endif /* !__twofish_h__ */
