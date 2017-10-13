/*
 *  arch/arm/mach-sunxi/pm/ccmu-sun8iw5p1.h
 *
 * Copyright 2012 (c) njubietech.
 * gq.yang (yanggq@njubietech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __MEM_CCMU_SUN8IW5P1_H__
#define __MEM_CCMU_SUN8IW5P1_H__

typedef union{
    __u32 dwval;
    struct
    {
	__u32   FactorM:2;          //bit0,  PLL1 Factor M
	__u32   reserved0:2;        //bit2,  reserved
	__u32   FactorK:2;          //bit4,  PLL1 factor K
	__u32   reserved1:2;        //bit6,  reserved
	__u32   FactorN:5;          //bit8,  PLL1 Factor N
	__u32   reserved2:3;        //bit13, reserved
	__u32   FactorP:2;          //bit16, PLL1 Factor P
	__u32   reserved3:6;	//bit18, reserved
	__u32   SigmaEn:1;          //bit24, sigma delta enbale
	__u32   reserved4:3;        //bit25, reserved
	__u32   Lock:1;             //bit28, pll is stable flag, 1-pll has stabled
	__u32   reserved5:2;        //bit29, reserved
	__u32   PLLEn:1;            //bit31, 0-disable, 1-enable, (24Mhz*N*K)/(M*P)

    } bits; 
}__ccmu_pll1_reg0000_t;


typedef struct __CCMU_PLL2_REG0008
{
    __u32   FactorM:5;          //bit0,  PLL2 prev division M
    __u32   reserved0:3;        //bit5,  reserved
    __u32   FactorN:7;          //bit8,  PLL2 factor N
    __u32   reserved1:1;        //bit15, reserved
    __u32   FactorP:4;          //bit16, PLL2 post division
    __u32   reserved2:4;        //bit20, reserved
    __u32   SdmEn:1;            //bit24, pll sdm enable, factorN only low 4 bits valid when enable
    __u32   reserved3:3;        //bit25, reserved
    __u32   Lock:1;             //bit28, pll stable flag
    __u32   reserved4:2;        //bit29, reserved
    __u32   PLLEn:1;            //bit31, PLL2 enable
} __ccmu_pll2_reg0008_t;


typedef struct __CCMU_MEDIA_PLL
{
    __u32   FactorM:4;          //bit0,  PLL3 FactorM
    __u32   reserved0:4;        //bit4,  reserved
    __u32   FactorN:7;          //bit8,  PLL factor N
    __u32   reserved1:5;        //bit15, reserved
    __u32   SdmEn:1;            //bit20, sdm enable
    __u32   reserved2:3;        //bit21, reserved
    __u32   ModeSel:1;          //bit24, PLL mode select
    __u32   FracMod:1;          //bit25, PLL out is 0:270Mhz, 1:297Mhz
    __u32   reserved3:2;        //bit26, reserved
    __u32   Lock:1;             //bit28, lock flag
    __u32   reserved4:1;        //bit29, reserved
    __u32   CtlMode:1;          //bit30, control mode, 0-controled by cpu, 1-control by DE
    __u32   PLLEn:1;            //bit31, PLL3 enable
} __ccmu_media_pll_t;


typedef struct __CCMU_PLL5_REG0020
{
    __u32   FactorM:2;          //bit0,  PLL5 factor M
    __u32   reserved0:2;        //bit2,  reserved
    __u32   FactorK:2;          //bit4,  PLL5 factor K
    __u32   reserved1:2;        //bit6,  reserved
    __u32   FactorN:5;          //bit8,  PLL5 factor N
    __u32   reserved2:7;        //bit13, reserved
    __u32   PLLCfgUpdate:1;     //bit20, PLL configuration update
    __u32   reserved3:3;        //bit21, reserved
    __u32   SigmaDeltaEn:1;     //bit24, sdram sigma delta enable
    __u32   reserved4:3;        //bit25, reserved
    __u32   Lock:1;             //bit28, lock flag
    __u32   reserved5:2;        //bit29, reserved
    __u32   PLLEn:1;            //bit31, PLL5 Enable
} __ccmu_pll5_reg0020_t;


typedef struct __CCMU_PLL6_REG0028
{
    __u32   FactorM:2;          //bit0,  PLL6 factor M
    __u32   reserved0:2;        //bit2,  reserved
    __u32   FactorK:2;          //bit4,  PLL6 factor K
    __u32   reserved1:2;        //bit6,  reserved
    __u32   FactorN:5;          //bit8,  PLL6 factor N
    __u32   reserved2:3;        //bit13, reserved
    __u32   Pll24MPdiv:2;       //bit16, PLL 24M output clock post divider
    __u32   Pll24MOutEn:1;      //bit18, PLL 24M output enable
    __u32   reserved3:5;        //bit19, reserved
    __u32   PllClkOutEn:1;      //bit24, pll clock output enable
    __u32   PLLBypass:1;        //bit25, PLL6 output bypass enable
    __u32   reserved4:2;        //bit26, reserved
    __u32   Lock:1;             //bit28, lock flag
    __u32   reserved5:2;        //bit29, reserved
    __u32   PLLEn:1;            //bit31, PLL6 enable
} __ccmu_pll6_reg0028_t;

typedef struct __CCMU_PLL8_REG0038
{
    __u32   FactorM:4;          //bit0,  PLL3 FactorM
    __u32   reserved0:4;        //bit4,  reserved
    __u32   FactorN:7;          //bit8,  PLL factor N
    __u32   reserved1:5;        //bit15, reserved
    __u32   SdmEn:1;            //bit20, sdm enable
    __u32   reserved2:3;        //bit21, reserved
    __u32   ModeSel:1;          //bit24, PLL mode select
    __u32   FracMod:1;          //bit25, PLL out is 0:270Mhz, 1:297Mhz
    __u32   reserved3:2;        //bit26, reserved
    __u32   Lock:1;             //bit28, lock flag
    __u32   reserved4:2;        //bit29, reserved
    __u32   PLLEn:1;            //bit31, PLL3 enable
} __ccmu_pll8_reg0038_t;


typedef struct __CCMU_MIPI_PLL_REG0040
{
    __u32   FactorM:4;          //bit0,  PLL FactorM
    __u32   FactorK:2;          //bit4,  PLL FactorK
    __u32   reserved0:2;        //bit6,  reserved
    __u32   FactorN:4;          //bit8,  PLL factor N
    __u32   reserved1:4;        //bit12, reserved
    __u32   VfbSel:1;           //bit16, 0-mipi mode(n,k,m valid), 1-hdmi mode(sint_frac, sdiv2
                                //       s6p25_7p5, pll_feedback_div valid)
    __u32   FeedBackDiv:1;      //bit17, pll feedback division, 0:x5, 1:x7
    __u32   reserved2:2;        //bit18, reserved
    __u32   SdmEn:1;            //bit20, sdm enable
    __u32   PllSrc:1;           //bit21, PLL source, 0:video pll0, 1:video pll1
    __u32   Ldo2En:1;           //bit22, LDO2 enable
    __u32   Ldo1En:1;           //bit23, LDO1 enable
    __u32   reserved3:1;        //bit24, reserved
    __u32   Sel625Or750:1;      //bit25, select pll out is input*6.25 or 7.50
    __u32   SDiv2:1;            //bit26, PLL output seclect, 0:pll output, 1:pll output x2
    __u32   FracMode:1;         //bit27, PLL output mode, 0:integer mode, 1:fraction mode
    __u32   Lock:1;             //bit28, lock flag
    __u32   reserved4:2;        //bit29, reserved
    __u32   PLLEn:1;            //bit31, PLL enable
} __ccmu_mipi_pll_reg0040_t;


#define AC327_CLKSRC_LOSC   (0)
#define AC327_CLKSRC_HOSC   (1)
#define AC327_CLKSRC_PLL1   (2)
typedef union{
    __u32 dwval;
    struct
    {
	__u32   AXIClkDiv:2;        //bit0,  AXI clock divide ratio, 000-1, 001-2, 010-3, 011-4
	__u32   reserved0:6;        //bit2,  reserved
	__u32   AtbApbClkDiv:2;     //bit8,  ATB/APB clock div, 00-1, 01-2, 1x-4
	__u32   reserved1:6;        //bit10, reserved
	__u32   CpuClkSrc:2;        //bit16, CPU1/2/3/4 clock source select, 00-internal LOSC, 01-HOSC, 10/11-PLL1
	__u32   reserved2:14;       //bit18, reserved
    } bits;
}__ccmu_sysclk_ratio_reg0050_t;


#define AHB1_CLKSRC_LOSC    (0)
#define AHB1_CLKSRC_HOSC    (1)
#define AHB1_CLKSRC_AXI     (2)
#define AHB1_CLKSRC_PLL6    (3)
typedef struct __CCMU_AHB1_RATIO_REG0054
{
    __u32   reserved0:4;        //bit0,  reserved
    __u32   Ahb1Div:2;          //bit4,  ahb1 clock divide ratio,1/2/4/8
    __u32   Ahb1PreDiv:2;       //bit6,  ahb1 clock pre-divide ratio 1/2/3/4
    __u32   Apb1Div:2;          //bit8,  apb1 clock divide ratio 2/2/4/8, source is ahb1
    __u32   reserved1:2;        //bit10, reserved
    __u32   Ahb1ClkSrc:2;       //bit12, ahb1 clock source select, 00-LOSC, 01-OSC24M, 10-AXI, 11-PLL6/ahb1_pre_div
    __u32   reserved2:18;       //bit26, reserved
} __ccmu_ahb1_ratio_reg0054_t;


#define APB2_CLKSRC_LOSC    (0)
#define APB2_CLKSRC_HOSC    (1)
#define APB2_CLKSRC_PLL6    (2)
typedef union{
    __u32 dwval;
    struct
    {
	__u32   DivM:5;             //bit0,  clock divide ratio m
	__u32   reserved:11;        //bit5,  reserved
	__u32   DivN:2;             //bit16, clock pre-divide ratio 1/2/4/8
	__u32   reserved1:6;        //bit18, reserved
	__u32   ClkSrc:2;           //bit24, clock source select, 00-LOSC, 01-OSC24M, 10/11-PLL6
	__u32   reserved2:6;        //bit26, reserved
    } bits;
} __ccmu_apb2_ratio_reg0058_t;

typedef struct __CCMU_PLLLOCK_REG0200
{
    __u32   LockTime:16;        //bit0,  PLL lock time, based on us
    __u32   reserved:16;        //bit16, reserved
} __ccmu_plllock_reg0200_t;

typedef struct __CCMU_REG_LIST
{
    volatile __ccmu_pll1_reg0000_t              Pll1Ctl;                //0x0000, PLL1 control, cpux
    volatile __u32                              reserved0;              //0x0004, reserved
    volatile __ccmu_pll2_reg0008_t              Pll2Ctl;                //0x0008, PLL2 control, audio
    volatile __u32                              reserved1;              //0x000c, reserved
    volatile __u32				Pll3Ctl;                //0x0010, PLL3 control, video
    volatile __u32                              reserved2;              //0x0014, reserved
    volatile __u32                              Pll4Ctl;                //0x0018, PLL4 control, ve
    volatile __u32                              reserved3;              //0x001c, reserved
    volatile __ccmu_pll5_reg0020_t              Pll5Ctl;                //0x0020, PLL5 control, ddr0 ctrl
    volatile __u32                              reserved4;              //0x0024, reserved
    volatile __ccmu_pll6_reg0028_t              Pll6Ctl;                //0x0028, PLL6 control, periph
    volatile __u32                              reserved5;              //0x002c, reserved
    volatile __u32                              reserved51;             //0x0030, reserved 
    volatile __u32                              reserved6;              //0x0034, reserved
    volatile __ccmu_pll8_reg0038_t              Pll8Ctl;                //0x0038, PLL8 control, gpu
    volatile __u32                              reserved7;              //0x003c, reserved
    volatile __ccmu_mipi_pll_reg0040_t          MipiPllCtl;             //0x0040, MIPI PLL control
    volatile __u32		                Pll9Ctl;                //0x0044, PLL9 control, hsic?
    volatile __u32		                Pll10Ctl;               //0x0048, PLL10 control,de
    volatile __u32                              PllDdr1Ctl;             //0x004c, pll ddr1 ctrl reg
    volatile __ccmu_sysclk_ratio_reg0050_t      SysClkDiv;              //0x0050, system clock divide ratio
    volatile __ccmu_ahb1_ratio_reg0054_t        Ahb1Div;                //0x0054, ahb1/apb1 clock divide ratio
    volatile __ccmu_apb2_ratio_reg0058_t        Apb2Div;                //0x0058, apb2 clock divide ratio
    volatile __u32                              reserved81;             //0x005c, reserved
    volatile __u32        			AhbGate0;               //0x0060, bus gating reg0
    volatile __u32        			AhbGate1;               //0x0064, bus gating reg1
    volatile __u32        			Apb1Gate;               //0x0068, bus gating reg2
    volatile __u32        			Apb2Gate;               //0x006c, bus gating reg3
    volatile __u32                              reserved9[4];           //0x0070, reserved
    volatile __u32                              Nand0;                  //0x0080, nand controller 0 clock
    volatile __u32                              reserved91;             //0x0084, reserved
    volatile __u32                              Sd0;                    //0x0088, sd/mmc controller 0 clock
    volatile __u32                              Sd1;                    //0x008c, sd/mmc controller 1 clock
    volatile __u32                              Sd2;                    //0x0090, sd/mmc controller 2 clock
    volatile __u32		                reserved92[2];          //0x0094, reserved
    volatile __u32		                Ss;		        //0x009c, ss clk
    volatile __u32                              Spi0;                   //0x00a0, spi controller 0 clock
    volatile __u32                              Spi1;                   //0x00a4, spi controller 1 clock
    volatile __u32                              reserved93[2];          //0x00a8, reserved
    volatile __u32                              I2s0;                   //0x00b0, daudio-0 clock?
    volatile __u32                              I2s1;                   //0x00b4, daudio-1 clock?
    volatile __u32                              reserved10[2];          //0x00b8, reserved
    volatile __u32                              reserved101;            //0x00c0, reserved
    volatile __u32                              reserved11[2];          //0x00c4, reserved
    volatile __u32                              Usb;                    //0x00cc, usb phy clock
    volatile __u32                              reserved111;            //0x00d0, reserved
    volatile __u32                              reserved12[7];          //0x00d4, reserved
    volatile __u32                              reserved121;            //0x00f0, reserved
    volatile __u32                              DramCfg;                //0x00f4, dram configuration clock
    volatile __u32                              PllDdrCfg;	        //0x00f8, pll ddr config reg
    volatile __u32                              MbusResetReg;           //0x00fc, mbus reset reg
    volatile __u32                              DramGate;               //0x0100, dram module clock
    volatile __u32                              Be0;                    //0x0104, BE0 module clock
    volatile __u32                              reserved131;            //0x0108, reserved
    volatile __u32                              Fe0;                    //0x010c, FE0 module clock
    volatile __u32                              reserved132;            //0x0110, reserved
    volatile __u32                              reserved133;            //0x0114, reserved
    volatile __u32                              Lcd0Ch0;                //0x0118, LCD0 CH0 module clock
    volatile __u32                              reserved134;            //0x011c, reserved
    volatile __u32                              reserved14[3];          //0x0120, reserved
    volatile __u32                              Lcd0Ch1;                //0x012c, LCD0 CH1 module clock
    volatile __u32                              reserved141;            //0x0130, reserved
    volatile __u32                              Csi0;                   //0x0134, csi0 module clock
    volatile __u32                              reserved142;            //0x0138, reserved
    volatile __u32                              Ve;                     //0x013c, ve module clock
    volatile __u32                              Adda;                   //0x0140, adda digital clock register
    volatile __u32                              Avs;                    //0x0144, avs module clock
    volatile __u32                              reserved143;            //0x0148, reserved
    volatile __u32                              reserved15;             //0x014c, reserved
    volatile __u32                              reserved151;            //0x0150, reserved
    volatile __u32                              reserved152;            //0x0154, reserved
    volatile __u32                              reserved153;            //0x0158, reserved
    volatile __u32                              MBus0;                  //0x015C, MBUS controller 0 clock
    volatile __u32                              reserved154;            //0x0160, reserved
    volatile __u32                              reserved16;             //0x0164, reserved
    volatile __u32                              MipiDsi;                //0x0168, MIPI DSI clock
    volatile __u32                              reserved161;            //0x016C, reserved
    volatile __u32                              reserved17[4];          //0x0170, reserved
    volatile __u32                              IepDrc0;                //0x0180, IEP DRC0 clock
    volatile __u32                              reserved171;            //0x0184, reserved
    volatile __u32                              reserved172;            //0x0188, reserved
    volatile __u32                              reserved173;            //0x018c, reserved
    volatile __u32                              reserved18[4];          //0x0190, reserved
    volatile __u32                              GpuCore;                //0x01A0, GPU Core clock
    volatile __u32                              GpuMem;                 //0x01A4, GPU Mem clock, actually, not used.
    volatile __u32                              GpuHyd;                 //0x01A8, GPU Hyd clock, actually, not used.
    volatile __u32                              reserved181;            //0x01AC, reserved
    volatile __u32                              Ats;                    //0x01B0, ats
    volatile __u32                              reserved19[19];         //0x01B4, reserved
    volatile __ccmu_plllock_reg0200_t           PllLock;                //0x0200, pll lock time
    volatile __u32                              Pll1Lock;               //0x0204, pll1 cpu lock time 
    volatile __u32                              reserved201[6];         //0x0208-0x21c, reserved
    volatile __u32                              PllxBias[1];            //0x220,  pll cpux  bias reg
    volatile __u32                              PllAudioBias;           //0x224,  pll audio bias reg
    volatile __u32                              PllVedioBias;           //0x228,  pll vedio bias reg
    volatile __u32                              PllVeBias;              //0x22c,  pll ve    bias reg
    volatile __u32                              PllDram0Bias;           //0x230,  pll dram0 bias reg
    volatile __u32                              PllPeriphBias;          //0x234,  pll periph bias reg
    volatile __u32                              reserved202;            //0x238,      reserved
    volatile __u32                              PllGpuBias;             //0x23c,  pll gpu   bias reg
    volatile __u32                              PllMipiBias;            //0x240,  pll mipi  bias reg
    volatile __u32                              PllHsicBias;            //0x244,  pll hsic  bias reg
    volatile __u32                              PllDeBias;              //0x248,  pll de    bias reg
    volatile __u32                              PllDram1BiasReg;        //0x24c,  pll dram1 bias 
    volatile __u32                              Pll1Tun;                //0x250, pll1 tun reg
    volatile __u32                              reserved203[3];         //0x254-0x25c, reserved
    volatile __u32                              Pll5Tun;                //0x260, pll5 tun reg
    volatile __u32                              reserved204[3];         //0x264-0x26c, reserved
    volatile __u32                              MipiPllTun;             //0x270, mipi pll tun reg
    volatile __u32                              reserved205[3];         //0x274-0x27c, reserved
    volatile __u32                              Pll1Pattern;            //0x280,  pll cpux  pattern reg
    volatile __u32                              PllAudioPattern;        //0x284,  pll audio pattern reg
    volatile __u32                              PllVedioPattern;        //0x288,  pll vedio pattern reg
    volatile __u32                              PllVePattern;           //0x28c,  pll ve    pattern reg
    volatile __u32                              PllDram0Pattern;        //0x290,  pll dram0 pattern reg
    volatile __u32                              reserved2051[2];        //0x294-0x298,      reserved
    volatile __u32                              PllGpuPattern;          //0x29c,  pll gpu   pattern reg
    volatile __u32                              PllMipiPattern;         //0x2a0,  pll mipi  pattern reg
    volatile __u32                              PllHsicPattern;         //0x2a4,  pll hsic  pattern reg
    volatile __u32                              PllDePattern;           //0x2a8,  pll de    pattern reg
    volatile __u32                              PllDram1PatternReg0;    //0x2ac,  pll dram1 pattern reg0
    volatile __u32                              PllDram1PatternReg1;    //0x2b0,  pll dram1 pattern reg1
    volatile __u32                              reserved206[3];         //0x02b4-0x2bc, reserved
    volatile __u32                              AhbReset0;              //0x02c0, AHB1 module reset register 0
    volatile __u32                              AhbReset1;              //0x02c4, AHB1 module reset register 1
    volatile __u32                              AhbReset2;              //0x02c8, AHB1 module reset register 2
    volatile __u32                              reserved21;             //0x02cc, reserved
    volatile __u32                              Apb1Reset;              //0x02d0, APB1 module reset register
    volatile __u32                              reserved22;             //0x02d4, reserved
    volatile __u32                              Apb2Reset;              //0x02d8, APB2 module reset register
} __ccmu_reg_list_t;

#endif  // #ifndef __MEM_CCMU_SUN8IW5P1_H__
