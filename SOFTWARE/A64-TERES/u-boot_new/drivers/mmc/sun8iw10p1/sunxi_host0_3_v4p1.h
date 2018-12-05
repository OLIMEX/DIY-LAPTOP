/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * sunxi_host0_3_v4p1.h
 * Description: MMC  driver for  mmc controller operations of sun8iw10p1 
 * Author: ZhengLei
 * Date: 2015/7/29 14:10:00
 */

#ifndef __SUNXI_HOST0_3_V4P1_H__
#define	__SUNXI_HOST0_3_V4P1_H__
struct mmc_reg_v4p1 {
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

struct mmc_des_v4p1 {
		u32			:1,
		dic			:1, /* disable interrupt on completion */
		last_des	:1, /* 1-this data buffer is the last buffer */
		first_des	:1, /* 1-data buffer is the first buffer,
						   0-data buffer contained in the next descriptor is 1st buffer */
		des_chain	:1, /* 1-the 2nd address in the descriptor is the next descriptor address */
		end_of_ring	:1, /* 1-last descriptor flag when using dual data buffer in descriptor */
					:24,
		card_err_sum	:1, /* transfer error flag */
		own			:1; /* des owner:1-idma owns it, 0-host owns it */

#define SDXC_DES_NUM_SHIFT 12  /* smhc2!! */
#define SDXC_DES_BUFFER_MAX_LEN	(1 << SDXC_DES_NUM_SHIFT)
	u32	data_buf1_sz	:16,
		data_buf2_sz	:16;

	u32	buf_addr_ptr1;
	u32	buf_addr_ptr2;
};


extern const struct mmc_ops sunxi_mmc_ops;


#endif /* __SUNXI_HOST0_3_V4P1_H__ */
