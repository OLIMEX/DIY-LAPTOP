/* sound\soc\sunxi\cs4385.h
 * (C) Copyright 2014-2017
 * Reuuimlla Technology Co., Ltd. <www.huangxin.com>
 * huangxin <huangxin@huangxin.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#ifndef _CS4385_H
#define _CS4385_H

#define CS4385_TWI			(0)
/*baseaddress is 0x18, 0x30 = 0x18<<1*/
#define CS4385_TWI_ADD		(0x30)


#define CS4385_CHIP_ID			(0x01)
#define CS4385_MOD_CTR			(0x02)
#define CS4385_PCM_CTR			(0x03)
#define CS4385_DSD_CTR			(0x04)
#define CS4385_FIL_CTR			(0x05)
#define CS4385_INV_CTR			(0x06)
#define CS4385_GRO_CTR			(0x07)
#define CS4385_RAMP_MUTE		(0x08)
#define CS4385_MUTE_CTR			(0x09)


#endif
