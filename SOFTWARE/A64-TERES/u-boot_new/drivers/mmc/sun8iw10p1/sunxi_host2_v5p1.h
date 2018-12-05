/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * sunxi_host2_v5p1.h
 * Description: MMC  driver for  mmc2 controller operations of sun8iw10p1
 * Author: ZhengLei
 * Date: 2015/7/27 21:03:00
 */

#ifndef _SUNXI_HOST2_V5P1_H_
#define _SUNXI_HOST2_V5P1_H_

//#define	MMC2_REG_FIFO_OS	(0X20)

struct mmc2_reg_v5p1{
	volatile u32 cmd_arg2;     /* (0x000) Command argument2 register */
	volatile u32 blk_cfg;      /* (0x004) Block size and count register */
	volatile u32 cmd_arg1;     /* (0x008) Command argument1 register */
	volatile u32 cmd;          /* (0x00C) Command register */
	volatile u32 resp0;        /* (0x010) Response 0 register */
	volatile u32 resp1;        /* (0x014) Response 1 register */
	volatile u32 resp2;        /* (0x018) Response 2 register */
	volatile u32 resp3;        /* (0x01C) Response 3 register */
	volatile u32 buff;         /* (0x020) Data buffer port register */
	volatile u32 status;       /* (0x024) Present state register */
	volatile u32 ctrl1;        /* (0x028) Host control 1 register */
	volatile u32 rst_clk_ctrl; /* (0x02C) Software reset and clock control register */
	volatile u32 int_sta;      /* (0x030) Interrupt status register */
	volatile u32 int_sta_en;   /* (0x034) Interrupt status enable register */
	volatile u32 int_sig_en;   /* (0x038) Interrupt signal enable register */
	volatile u32 acmd_err_ctrl2; /* (0x03C) Auto command error status and host control 2 register */
	volatile u32 res_0[4];     /* (0x040 - 0x04C) Reserved */
	volatile u32 set_err;      /* (0x050) Force event register for error status */
	volatile u32 adma_err;     /* (0x054) IDMA error status register */
	volatile u32 adma_addr;    /* (0x058) IDMA system address register  */
	volatile u32 res_1[105];    /* (0x05C - 0x1FC) Reserved */
	volatile u32 ctrl3;        /* (0x200) Host control 3 register */
	volatile u32 cmd_attr;     /* (0x204) Command attribute register */
	volatile u32 to_ctrl;      /* (0x208) Timeout control 2 register */
	volatile u32 thld;          /* (0x20C) Card threshold control register */
	volatile u32 atc;          /* (0x210) Auto timing control register */
	volatile u32 rtc;          /* (0x214) Response phase timing control register */
	volatile u32 ditc0;        /* (0x218) Data input timing control0 register */
	volatile u32 ditc1;        /* (0x21C) Data input timing control1 register */
	volatile u32 tp0;          /* (0x220) Timing parameter 0 register */
	volatile u32 tp1;          /* (0x224) Timing parameter 1 register */
	volatile u32 res_2[2];      /* (0x228 - 0x22C) Reserved */
	volatile u32 ds_dly;       /* (0x230) Data strobe delay control register */
	volatile u32 res_3[3];      /* (0x234 - 0x23C) Reserved */
	volatile u32 crc_sta;      /* (0x240) Data strobe delay control register */
	volatile u32 tbc0;         /* (0x244) Transferred byte count between host controller and device */
	volatile u32 tbc1;         /* (0x248) Transferred byte count between host memory and internal buffer */
	volatile u32 buff_lvl;     /* (0x24C) Internal buffer level register */
	volatile u32 res_4[1];     /* (0x250) Reserved */
	volatile u32 cddlw;        /* (0x254) Current DMA descriptor lower word */
	volatile u32 cddhw;        /* (0x258) Current DMA descriptor higher word */
};

/* control register bit field */
#define ResetAll            (0x1U<<24)
#define ResetCmd            (0x1U<<25)
#define ResetDat            (0x1U<<26)

/*0x200*/
#define CPUAcessBuffEn      (0x1U<<31)
#define FetchDMADesc        (0x1U<<30)
#define StopWriteClkAtBlkGap (0x1U<<9)
#define StopReadClkAtBlkGap (0x1U<<8)
#define SWDebounceMode      (0x1U<<5)
#define DebounceEnb         (0x1U<<4)
#define CDUseD3             (0x1U<<3)
#define ClkIdleCtrl         (0x1U<<2)

/* Struct for SMC Commands */
/*0x18*/
#define CMDType         (0x3U<<22)
#define DataExp         (0x1U<<21)
#define CheckRspIdx     (0x1U<<20)
#define CheckRspCRC     (0x1U<<19)
#define NoRsp           (0x0U<<16)
#define Rsp136          (0x1U<<16)
#define Rsp48           (0x2U<<16)
#define Rsp48b          (0x3U<<16)
#define SingleBlkTrans  (0x0U<<5)
#define MultiBlkTrans   (0x1U<<5)
#define Read            (0x1U<<4)
#define Write           (0x0U<<4)
#define AutoCmd12       (0x1U<<2)
#define AutoCmd23       (0x2U<<2)
#define BlkCntEn        (0x1U<<1)
#define DMAEn           (0x1U<<0)

/*0x24*/
#define CmdLineSta      (0x1U<<24)
#define Dat3LineSta     (0x1U<<23)
#define Dat2LineSta     (0x1U<<22)
#define Dat1LineSta     (0x1U<<21)
#define Dat0LineSta     (0x1U<<20)
#define WpPinSta        (0x1U<<19)
#define CdPinInvSta     (0x1U<<18)
#define CardStable      (0x1U<<17)
#define CardInsert      (0x1U<<16)
#define BuffRDEn        (0x1U<<11)
#define BuffWREn        (0x1U<<10)
#define RDTransActive   (0x1U<<9)
#define WRTransActive   (0x1U<<8)
#define DatLineActive   (0x1U<<2)
#define CmdInhibitDat   (0x1U<<1)
#define CmdInhibitCmd   (0x1U<<0)

/*0x30*/
#define DSFO              (0x1U<<30)
#define BootDataStart     (0x1U<<29)
#define BootAckRcv        (0x1U<<28)
//
#define DmaErrInt         (0x1U<<25)
#define AcmdErrInt        (0x1U<<24)
#define DatEndBitErrInt   (0x1U<<22)
#define DatCRCErrInt      (0x1U<<21)
#define DatTimeoutErrInt  (0x1U<<20)
#define CmdIdxErrInt      (0x1U<<19)
#define CmdEndBitErrInt   (0x1U<<18)
#define CmdCRCErrInt      (0x1U<<17)
#define CmdTimeoutErrInt  (0x1U<<16)
#define ErrInt            (0x1U<<15)
#define CardInt           (0x1U<<8)
#define CardRemoveInt     (0x1U<<7)
#define CardInsertInt     (0x1U<<6)
#define BuffRDRdyInt      (0x1U<<5)
#define BuffWRRdyInt      (0x1U<<4)
#define DmaInt            (0x1U<<3)
//#define BlkGapEvtInt      (0x1U<<2)
#define TransOverInt      (0x1U<<1)
#define CmdOverInt        (0x1U<<0)
#define TxDatIntBit       ( DmaInt | TransOverInt | DmaErrInt | ErrInt)
#define RxDatIntBit       ( DmaInt | TransOverInt | DmaErrInt | ErrInt)
#define DmaIntBit         (DmaInt | DmaErrInt)
#define ErrIntBit         (0x1ff<<16)

/*0x3C Auto CMD Error Status */
#define NoAcmd12          (0x1U<<7)
#define AcmdIdxErr        (0x1U<<4)
#define AcmdEndBitErr     (0x1U<<3)
#define AcmdCRCErr        (0x1U<<2)
#define AcmdTimeoutErr    (0x1U<<1)
#define NotIssueAcmd      (0x0<<0)

/*0x204*/
#define SendInitSeq     (0x1U<<4)
#define DisableBoot     (0x1U<<3)
#define BootACKExp      (0x1U<<2)
#define AltBootMode     (0x2U<<0)
#define MandBootMode    (0x1U<<0)

/* 0x230: delay control */
#define StartCal        (1<<15)
#define CalDone         (1<<14)
#define CalDly          (0x3F<<8)
#define EnableDly       (1<<7)
#define CfgDly          (0x3F<<0)

#define FIFO_DEPTH_WORD     (256)  //1KB

enum
{
	ACT_NOP = 0,
	ACT_RSV,
	ACT_TRANS,
	ACT_LINK,
};


#define SMHC2_V5P1_DES_NUM_SHIFT            (12)
#define SMHC2_V5P1_DES_BUFFER_MAX_LEN       (1 << SMHC2_V5P1_DES_NUM_SHIFT)

struct mmc2_des_v5p1 {
	u32 valid           :1, //=1: indicates this line of descriptor is effective. =0: generate ADMA Error interrupt and stop ADMA to prevent runaway.
		end             :1, //=1: indicates end of descriptor. The Transfer Complete Interrupt is generated when the operation of the descriptor line is completed.
		int_en          :1, //=1: generates DMA Interrupt when the operation of the descriptor line is completed.
		                :1,
		act             :2, //00b: Nop, No Operation, Do not execute current line and go to next line.
		                    //01b: rsv, reserved, (Same as Nop. Do not execute current line and go to next line.)
		                    //10b: Tran, transfer data, Transfer data of one descriptor line Transfer data of one descriptor line
		                    //11b: Link, Link Descriptor, Link to another descriptor
		                :10,
		length          :16;
	u32 addr;
};


extern const struct mmc_ops sunxi_mmc2_ops;

#endif /* _SUNXI_HOST2_V5P1_H_ */
