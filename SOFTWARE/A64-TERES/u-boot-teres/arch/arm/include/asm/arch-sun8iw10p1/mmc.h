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

#ifndef _SUNXI_MMC_H
#define _SUNXI_MMC_H

struct sunxi_mmc {
	volatile u32 gctrl;              /* (0x00) SMC Global Control Register */
	volatile u32 clkcr;              /* (0x04) SMC Clock Control Register */
	volatile u32 timeout;        /* (0x08) SMC Time Out Register */
	volatile u32 width;           /* (0x0C) SMC Bus Width Register */
	volatile u32 blksz;            /* (0x10) SMC Block Size Register */
	volatile u32 bytecnt;       /* (0x14) SMC Byte Count Register */
	volatile u32 cmd;             /* (0x18) SMC Command Register */
	volatile u32 arg;              /* (0x1C) SMC Argument Register */
	volatile u32 resp0;          /* (0x20) SMC Response Register 0 */
	volatile u32 resp1;          /* (0x24) SMC Response Register 1 */
	volatile u32 resp2;          /* (0x28) SMC Response Register 2 */
	volatile u32 resp3;          /* (0x2C) SMC Response Register 3 */
	volatile u32 imask;          /* (0x30) SMC Interrupt Mask Register */
	volatile u32 mint;            /* (0x34) SMC Masked Interrupt Status Register */
	volatile u32 rint;              /* (0x38) SMC Raw Interrupt Status Register */
	volatile u32 status;         /* (0x3C) SMC Status Register */
	volatile u32 ftrglevel;     /* (0x40) SMC FIFO Threshold Watermark Register */
	volatile u32 funcsel;       /* (0x44) SMC Function Select Register */
	volatile u32 cbcr;            /* (0x48) SMC CIU Byte Count Register */
	volatile u32 bbcr;            /* (0x4C) SMC BIU Byte Count Register */
	volatile u32 dbgc;           /* (0x50) SMC Debug Enable Register */
	volatile u32 csdc;           /* (0x54) CRC status detect control register*/
	volatile u32 a12a;          /* (0x58)Auto command 12 argument*/
	volatile u32 ntsr;            /* (0x5c)SMC2 Newtiming Set Register */
	volatile u32 res1[6];     /* (0x54~0x74) */
	volatile u32 hwrst;        /* (0x78) SMC eMMC Hardware Reset Register */
	volatile u32 res2;          /*  (0x7c) */
	volatile u32 dmac;        /*  (0x80) SMC IDMAC Control Register */
	volatile u32 dlba;          /*  (0x84) SMC IDMAC Descriptor List Base Address Register */
	volatile u32 idst;           /*  (0x88) SMC IDMAC Status Register */
	volatile u32 idie;           /*  (0x8C) SMC IDMAC Interrupt Enable Register */
	volatile u32 chda;         /*  (0x90) */
	volatile u32 cbda;         /*  (0x94) */
	volatile u32 res3[26];  /*  (0x98~0xff) */
	volatile u32 thldc;		/*  (0x100) Card Threshold Control Register */
	volatile u32 res4[2];    /*  (0x104~0x10b) */
	volatile u32 dsbd;		/* (0x10c) eMMC4.5 DDR Start Bit Detection Control */
	volatile u32 res5[12];  /* (0x110~0x13c) */
	volatile u32 drv_dl;    /* (0x140) drive delay control register*/
	volatile u32 samp_dl;   /* (0x144) sample delay control register*/
	volatile u32 ds_dl;     /* (0x148) data strobe delay control register */
	volatile u32 res6[45];  /* (0x110~0x1ff) */
	volatile u32 fifo;           /* (0x200) SMC FIFO Access Address */
};

#endif /* _SUNXI_MMC_H */
