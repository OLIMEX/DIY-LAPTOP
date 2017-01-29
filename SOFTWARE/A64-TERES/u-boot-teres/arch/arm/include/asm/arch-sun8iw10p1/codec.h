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

#ifndef __SUNXI_CODEC_H__
#define __SUNXI_CODEC_H__

struct sunxi_codec_t {
	volatile u32 dac_dpc;
	volatile u32 dac_fifoc;
	volatile u32 dac_fifos;
	volatile u32 dac_txdata;
	volatile u32 dac_actl;
	volatile u32 dac_tune;
	volatile u32 dac_dg;
	volatile u32 adc_fifoc;
	volatile u32 adc_fifos;
	volatile u32 adc_rxdata;
	volatile u32 adc_actl;
	volatile u32 adc_dg;
	volatile u32 dac_cnt;
	volatile u32 adc_cnt;
	volatile u32 ac_dg;
};


extern  int sunxi_codec_init(void);
extern  int sunxi_codec_config(int samplerate, int samplebit, int channel);
extern  int sunxi_codec_start(void *buffer, uint length, uint loop_mode);
extern  int sunxi_codec_stop(void);
extern  int sunxi_codec_wink(void *buffer, uint length);
extern  int sunxi_codec_exit(void);


#endif


