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
#ifndef ___TWO_FISH____H_____
#define ___TWO_FISH____H_____

#include    "../encrypt.h"


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------

#define TF_SIZE				16			//TF加密分组size 16字节

//------------------------------------------------------------------------------------------------------------
//允许的key长度
//------------------------------------------------------------------------------------------------------------
#define KEY_LEN_128_BIT		(128 / 8)	//128 bit
#define KEY_LEN_192_BIT		(192 / 8)	//192 bit
#define KEY_LEN_256_BIT		(256 / 8)	//256 bit


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
typedef void * HTF;

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
HTF TFInitial(uint * key, uint keylen);

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
uint TFEncode(HTF hTF, void * ibuf, uint len, void * obuf);


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
uint TFDecode(HTF hTF, void * ibuf, uint len, void * obuf);

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
uint TFUnInitial(HTF hTF);


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------

#endif //___TWO_FISH____H_____



