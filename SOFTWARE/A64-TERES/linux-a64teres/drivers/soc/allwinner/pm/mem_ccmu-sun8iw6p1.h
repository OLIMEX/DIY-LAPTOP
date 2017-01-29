/*
 *  arch/arm/mach-sunxi/pm/ccmu-sun8iw6p1.h
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
#ifndef __MEM_CCMU_SUN8IW6P1_H__
#define __MEM_CCMU_SUN8IW6P1_H__

typedef union{
    __u32 dwval;
    struct
    {
	__u32   FactorM:2;          //bit0,  PLL1 Factor M
	__u32   reserved0:6;        //bit2,  reserved
	__u32   FactorN:8;          //bit8,  PLL1 Factor N
	__u32   FactorP:1;          //bit16, PLL1 Factor P
	__u32   reserved3:7;	//bit17, reserved
	__u32   Locktime:3;         //bit24, pll lock time
	__u32   reserved4:4;        //bit27, reserved
	__u32   PLLEn:1;            //bit31, 0-disable, 1-enable, (24Mhz*N)/(P)

    } bits;
} __ccmu_pll1_reg0000_t;

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

#define AC327_CLKSRC_LOSC   (0)
#define AC327_CLKSRC_HOSC   (1)
#define AC327_CLKSRC_PLL1   (2)
typedef union{
    __u32 dwval;
    struct
    {
	__u32   AXIClkDiv:2;       //bit0,  AXI0 clock divide ratio, 000-1, 001-2, 010-3, 011-4
	__u32   reserved0:10;       //bit2,  reserved
	__u32   CpuClkSrc:1;         //bit12, CPU0/1/2/3 clock source select, 0-HOSC, 1-PLL_C0CPUX
	__u32   reserved1:3;        //bit13, reserved
	__u32   AXI1ClkDiv:2;       //bit16, AXI1 clock divide ratio, 000-1, 001-2, 010-3, 011-4
	__u32   reserved2:10;       //bit18,  reserved
	__u32   C1ClkSrc:1;         //bit28, CPU1/1/2/3 clock source select, 0-HOSC, 1-PLL_C1CPUX
	__u32   reserved3:3;        //bit29, reserved
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
    __u32   Apb1Div:2;          //bit8,  apb1 clock divide ratio 1/2/3/4, source is ahb1
    __u32   reserved1:2;        //bit10, reserved
    __u32   Ahb1ClkSrc:2;       //bit12, ahb1 clock source select, 00-LOSC, 01-OSC24M, 10/11-PLL6/ahb1_pre_div
    __u32   reserved2:18;       //bit14, reserved
} __ccmu_ahb1_ratio_reg0054_t;

#define APB2_CLKSRC_LOSC    (0)
#define APB2_CLKSRC_HOSC    (1)
#define APB2_CLKSRC_PLL6    (2)
typedef union{
    __u32 dwval;
    struct
    {
    __u32   DivM:5;             //bit0,  clock divide ratio m, the divider is from 1 to 32.
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
    volatile __ccmu_pll1_reg0000_t		Pll1Ctl;                //0x0000, PLL cluster 0 cpux control
    volatile __u32				PllC1Ctl;               //0x0004, PLL cluster 1 cpux control
    volatile __u32		                Pll2Ctl;                //0x0008, PLL2 control, audio
    volatile __u32                              reserved1;              //0x000c, reserved
    volatile __u32	                        Pll3Ctl;                //0x0010, PLL3 control, video
    volatile __u32                              reserved2;              //0x0014, reserved
    volatile __u32                              Pll4Ctl;                //0x0018, PLL4 control, ve
    volatile __u32                              reserved3;              //0x001c, reserved
    volatile __u32		                Pll5Ctl;                //0x0020, PLL5 control, ddr0 ctrl
    volatile __u32                              reserved4;              //0x0024, reserved
    volatile __u32                              Pll6Ctl;                //0x0028, PLL6 control, periph
    volatile __u32                              reserved5;              //0x002c, reserved
    volatile __u32                              reserved51;             //0x0030, reserved 
    volatile __u32                              reserved6;              //0x0034, reserved
    volatile __u32		                Pll8Ctl;                //0x0038, PLL8 control, gpu
    volatile __u32                              reserved7;              //0x003c, reserved
    volatile __u32                              reserved71;             //0x0040, reserved
    volatile __u32		                Pll9Ctl;                //0x0044, PLL9 control, hsic?
    volatile __u32		                Pll10Ctl;               //0x0048, PLL10 control,de
    volatile __u32                              PllVe1Ctl;              //0x004c, pll vedio1 ctrl reg
    volatile __ccmu_sysclk_ratio_reg0050_t      SysClkDiv;              //0x0050, system clock divide ratio, cpu axi
    volatile __ccmu_ahb1_ratio_reg0054_t        Ahb1Div;                //0x0054, ahb1/apb1 clock divide ratio
    volatile __ccmu_apb2_ratio_reg0058_t        Apb2Div;                //0x0058, apb2 clock divide ratio
    volatile __u32                              Ahb2Cfg;                //0x005c, ahb2 clk cfg, usb and gmac option src
    volatile __u32        			AhbGate0;               //0x0060, bus gating reg0
    volatile __u32        			AhbGate1;               //0x0064, bus gating reg1
    volatile __u32        			Apb1Gate;               //0x0068, bus gating reg2
    volatile __u32        			Apb2Gate;               //0x006c, bus gating reg3
    volatile __u32                              reserved9[2];           //0x0070, reserved
    volatile __u32                              Cci400Clk;              //0x0078, cci400 clock
    volatile __u32                              reserved912;            //0x007c, reserved
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
    volatile __u32                              I2s2;                   //0x00b8, daudio-2 clock?
    volatile __u32                              TdmClk;                 //0x00bc, 
    volatile __u32                              SpdifClk;               //0x00c0, 
    volatile __u32                              reserved11[2];          //0x00c4, reserved
    volatile __u32                              Usb;                    //0x00cc, usb phy clock
    volatile __u32                              reserved111;            //0x00d0, reserved
    volatile __u32                              reserved12[7];          //0x00d4, reserved
    volatile __u32                              reserved121;            //0x00f0, reserved
    volatile __u32                              DramCfg;                //0x00f4, dram configuration clock
    volatile __u32                              PllDdrCfg;	        //0x00f8, pll ddr config reg
    volatile __u32                              MbusResetReg;           //0x00fc, mbus reset reg
    volatile __u32                              DramGate;               //0x0100, dram module clock
    volatile __u32                              reserved122;            //0x0104, reserved
    volatile __u32                              reserved131;            //0x0108, reserved
    volatile __u32                              reserved1311;           //0x010c, reserved
    volatile __u32                              reserved132;            //0x0110, reserved
    volatile __u32                              reserved133;            //0x0114, reserved
    volatile __u32                              Lcd0Ch0;                //0x0118, LCD0 CH0 module clock
    volatile __u32                              Lcd0Ch1;                //0x011c, LCD0 CH1 module clock
    volatile __u32                              reserved14[4];          //0x0120, reserved
    volatile __u32                              MipiCsi;                //0x0130, mipi csi config reg
    volatile __u32                              Csi0;                   //0x0134, csi0 module clock
    volatile __u32                              reserved142;            //0x0138, reserved
    volatile __u32                              Ve;                     //0x013c, ve module clock
    volatile __u32                              reserved1421;           //0x0140, reserved
    volatile __u32                              Avs;                    //0x0144, avs module clock
    volatile __u32                              reserved143;            //0x0148, reserved
    volatile __u32                              reserved15;             //0x014c, reserved
    volatile __u32                              HdmiClk;                //0x0150, hdmi clk reg
    volatile __u32                              HdmiSlowClk;            //0x0154, hdmi slow clk reg
    volatile __u32                              reserved153;            //0x0158, reserved
    volatile __u32                              MBus0;                  //0x015C, MBUS controller 0 clock
    volatile __u32                              reserved154;            //0x0160, reserved
    volatile __u32                              reserved16;             //0x0164, reserved
    volatile __u32                              MipiDsiReg0;            //0x0168, MIPI DSI clock reg0
    volatile __u32                              MipiDsiReg1;            //0x016c, MIPI DSI clock reg1
    volatile __u32                              reserved17[4];          //0x0170, reserved
    volatile __u32                              reserved1741;           //0x0180, reserved
    volatile __u32                              reserved171;            //0x0184, reserved
    volatile __u32                              reserved172;            //0x0188, reserved
    volatile __u32                              reserved173;            //0x018c, reserved
    volatile __u32                              reserved18[4];          //0x0190, reserved
    volatile __u32                              GpuCore;                //0x01A0, GPU Core clock
    volatile __u32                              GpuMem;                 //0x01A4, GPU Mem clock
    volatile __u32                              GpuHyd;                 //0x01A8, GPU Hyd clock
    volatile __u32                              reserved181;            //0x01AC, reserved
    volatile __u32                              reserved182;            //0x01B0, reserved
    volatile __u32                              reserved19[3];		//0x01B4-0x1c0, reserved
    volatile __u32                              CirTx;                  //0x01C0, cir tx, notice: cir is not existed actually.!
    volatile __u32                              reserved191[15];        //0x01c4-0x200, reserved
    volatile __ccmu_plllock_reg0200_t           PllLock;	        //0x0200, plls stable time //member name need double check!.
    volatile __u32                              Pll1Lock;               //0x0204, pll cpux stable time 
    volatile __u32                              reserved201;            //0x0208, reserved
    volatile __u32                              PllStableStatus;        //0x020c, pll stable status reg 
    volatile __u32                              reserved202[4];         //0x0210-0x21c, reserved
    
    volatile __u32                              PllxBias[1];          //0x220, pll c0cpux bias reg; 
    volatile __u32                              PllAudioBias;           //0x224, pll audio bias reg; 
    volatile __u32                              PllVideo0Bias;          //0x228, pll video0 bias reg; 
    volatile __u32                              PllVeBias;		//0x22c, pll Ve bias reg; 
    volatile __u32                              PllDdrBias;		//0x230, pll ddr bias reg; 
    volatile __u32                              PllPeriphBias;          //0x234, pll periph bias reg; 
    volatile __u32                              PllC1CpuxBias;          //0x238, pll c1cpux bias reg; 
    volatile __u32                              PllGpuBias;		//0x23c, pll gpu bias reg; 
    volatile __u32                              Reserved2021;		//0x240; 
    volatile __u32                              PllHsicBias;		//0x244, pll hsic bias reg; 
    volatile __u32                              PllDeBias;		//0x248, pll de bias reg; 
    volatile __u32                              PllVideo1Bias;          //0x24c, pll Video1 bias reg; 

    volatile __u32                              Pll1Tun;                //0x250, pll cluster0 cpux tun reg
    volatile __u32                              PllC1Tun;               //0x254, pll cluster1 cpux tun reg
   
    volatile __u32                              reserved203[2];         //0x258-0x25c, reserved
    volatile __u32                              reserved2032;           //0x260, reserved
    volatile __u32                              reserved204[3];         //0x264-0x26c, reserved
    volatile __u32                              reserved2043;           //0x270, reserved
    volatile __u32                              reserved205[3];         //0x274-0x27c, reserved
    volatile __u32                              reserved20616;          //0x0280, reserved
    
    volatile __u32                              PllAudioReg0Pattern;    //0x0284, pll audio reg0 pattern 
    volatile __u32                              PllVideo0Reg0Pattern;       //0x0288, pll video0 pattern 
    volatile __u32                              reserved2071;           //0x028c, reserved
    volatile __u32                              PllDdrPattern;		//0x0290, pll ddr pattern 
    volatile __u32                              reserved208[4];         //0x0294-0x2a0, reserved
    volatile __u32                              PllAudioReg1Pattern;    //0x02a4, pll audio reg1 pattern 
    volatile __u32                              PllVideo0Reg1Pattern;   //0x02a8, pll video0 reg1 pattern 
    volatile __u32                              reserved2081;		//0x02ac, reserved
    volatile __u32                              PllDdrReg1Pattern;      //0x02b0, pll ddr reg1 pattern 
    volatile __u32                              reserved2082[3];	//0x02b4-0x2bc, reserved
    
    volatile __u32                              AhbReset0;              //0x02c0, AHB1 module reset register 0
    volatile __u32                              AhbReset1;              //0x02c4, AHB1 module reset register 1
    volatile __u32                              AhbReset2;              //0x02c8, AHB1 module reset register 2
    volatile __u32                              reserved21;             //0x02cc, reserved
    volatile __u32                              Apb1Reset;              //0x02d0, APB1 module reset register
    volatile __u32                              reserved22;             //0x02d4, reserved
    volatile __u32                              Apb2Reset;              //0x02d8, APB2 module reset register
} __ccmu_reg_list_t;

#endif  // #ifndef __MEM_CCMU_SUN8IW6P1_H__

