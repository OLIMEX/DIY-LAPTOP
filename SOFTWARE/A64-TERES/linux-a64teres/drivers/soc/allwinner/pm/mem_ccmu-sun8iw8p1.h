/*
 *  arch/arm/mach-sunxi/pm/ccmu-sun8iw8p1.h
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
#ifndef __MEM_CCMU_SUN8IW8P1_H__
#define __MEM_CCMU_SUN8IW8P1_H__

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

#define AC327_CLKSRC_LOSC   (0)
#define AC327_CLKSRC_HOSC   (1)
#define AC327_CLKSRC_PLL1   (2)
typedef union{
    __u32 dwval;
    struct
    {
	__u32   AXIClkDiv:3;        //bit0,  AXI clock divide ratio, 000-1, 001-2, 010-3, 011/1xx-4
	__u32   reserved0:5;        //bit3,  reserved
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
typedef union
{
    __u32 dwval;
    
    struct{
    __u32   reserved0:4;        //bit0,  reserved
    __u32   Ahb1Div:2;          //bit4,  ahb1 clock divide ratio,1/2/4/8
    __u32   Ahb1PreDiv:2;       //bit6,  ahb1 clock pre-divide ratio 1/2/3/4
    __u32   Apb1Div:2;          //bit8,  apb1 clock divide ratio 2/2/4/8, source is ahb1
    __u32   reserved1:2;        //bit10, reserved
    __u32   ClkSrc:2;       //bit12, ahb1 clock source select, 00-LOSC, 01-OSC24M, 10-AXI, 11-PLL6/ahb1_pre_div
    __u32   reserved2:18;       //bit26, reserved
    }bits;

} __ccmu_ahb1_ratio_reg0054_t;


#define APB2_CLKSRC_LOSC    (0)
#define APB2_CLKSRC_HOSC    (1)
#define APB2_CLKSRC_PLL6    (2)
typedef union
{
    __u32 dwval;
   
    struct{
    __u32   DivM:5;             //bit0,  clock divide ratio m
    __u32   reserved:11;        //bit5,  reserved
    __u32   DivN:2;             //bit16, clock pre-divide ratio 1/2/4/8
    __u32   reserved1:6;        //bit18, reserved
    __u32   ClkSrc:2;           //bit24, clock source select, 00-LOSC, 01-OSC24M, 10/11-PLL6
    __u32   reserved2:6;        //bit26, reserved
    }bits;

} __ccmu_apb2_ratio_reg0058_t;

#define AHB2_CLKSRC_AHB1    (0)
#define AHB2_CLKSRC_PLL6    (1)
typedef union
{
    __u32 dwval;
   
    struct{
    __u32   ClkSrc:2;           //bit0  clock source select, 00-ahb1, 01-pll_periph/2, 
    __u32   reserved:30;        //bit2,  reserved
    }bits;
} __ccmu_ahb2_ratio_reg005c_t;

typedef struct __CCMU_PLLLOCK_REG0200
{
    __u32   LockTime:16;        //bit0,  PLL lock time, based on us
    __u32   reserved:16;        //bit16, reserved
} __ccmu_plllock_reg0200_t;

typedef struct __CCMU_REG_LIST
{
    volatile __ccmu_pll1_reg0000_t              Pll1Ctl;//0x0000, PLL_CPU control
    volatile __u32              reserved0;              //0x0004, reserved
    volatile __u32              Pll2Ctl;                //0x0008, PLL_AUDIO control
    volatile __u32              reserved1;              //0x000c, reserved
    volatile __u32              Pll3Ctl;                //0x0010, PLL_VIDEO control
    volatile __u32              reserved2;              //0x0014, reserved
    volatile __u32              Pll4Ctl;                //0x0018, PLL_VE control
    volatile __u32              reserved3;              //0x001c, reserved
    volatile __u32              Pll5Ctl;                //0x0020, PLL_DDR0 control
    volatile __u32              reserved4;              //0x0024, reserved
    volatile __u32              Pll6Ctl;                //0x0028, PLL_PERI0 control
    volatile __u32              PllIspCtl;              //0x002c, PLL_ISP control
    volatile __u32              reserved51;             //0x0030, reserved 
    volatile __u32              reserved6;              //0x0034, reserved
    volatile __u32              reserved61;             //0x0038, reserved
    volatile __u32              reserved7;              //0x003c, reserved
    volatile __u32		reserved71;             //0x0040, reserved
    volatile __u32              Pll9Ctl;                //0x0044, PLL_PERI1 control
    volatile __u32              reserved72;             //0x0048, reserved
    volatile __u32              reserved8;              //0x004c, PLL_DDR1 control
    volatile __ccmu_sysclk_ratio_reg0050_t      SysClkDiv;              //0x0050, system clock divide ratio,cpu/axi
    volatile __ccmu_ahb1_ratio_reg0054_t        Ahb1Div;                //0x0054, ahb1/apb1 clock divide ratio,ahb1/apb1
    volatile __ccmu_apb2_ratio_reg0058_t        Apb2Div;                //0x0058, apb2 clock divide ratio,apb2
    volatile __ccmu_ahb2_ratio_reg005c_t        Ahb2Div;		//0x005c, ahb2
    volatile __u32        			AhbGate0;               //0x0060, ahb1 clock gate 0, bus gating reg0
    volatile __u32        			AhbGate1;               //0x0064, ahb1 clock gate 1, bus gating reg1
    volatile __u32        			Apb1Gate;               //0x0068, apb1 clock gate, bus gating reg2
    volatile __u32        			Apb2Gate;               //0x006c, apb2 clock gate, bus gating reg3
    volatile __u32                              BusGate4;		//0x0070, bus gating reg4
    volatile __u32                              reserved80[5];              //0x0074-0x84, reserved
    volatile __u32                              Sd0;                    //0x0088, sd/mmc controller 0 clock
    volatile __u32                              Sd1;                    //0x008c, sd/mmc controller 1 clock
    volatile __u32                              Sd2;                    //0x0090, sd/mmc controller 2 clock
    volatile __u32		                reserved92[2];          //0x0094, reserved
    volatile __u32                              SsClk; 			//0x009c, ss clk
    volatile __u32                              Spi0;                   //0x00a0, spi controller 0 clock
    volatile __u32                              Spi1;                   //0x00a4, reserved, not exist;
    volatile __u32                              reserved93[2];          //0x00a8, reserved
    volatile __u32                              I2s0;                   //0x00b0, I2s-0 clock, daudio clk;
    volatile __u32                              I2s1;                   //0x00b4, reserved, not exist;
    volatile __u32                              reserved10[2];          //0x00b8, reserved
    volatile __u32                              reserved101;            //0x00c0, reserved
    volatile __u32                              reserved11[2];          //0x00c4, reserved
    volatile __u32                              Usb;                    //0x00cc, usb clock, usbphy cfg reg
    volatile __u32                              reserved111;            //0x00d0, reserved
    volatile __u32                              reserved12[7];          //0x00d4, reserved
    volatile __u32                              reserved121;            //0x00f0, reserved
    volatile __u32                              DramCfg;                //0x00f4, dram configuration clock
    volatile __u32                              PllDdr1Cfg;	        //0x00f8, pll_ddr1 cfg reg
    volatile __u32                              MbusResetReg;	        //0x00fc, mbus reset reg
    volatile __u32                              DramGate;               //0x0100, dram module clock
    volatile __u32                              Be0;                    //0x0104, DE module clock
    volatile __u32                              reserved131;            //0x0108, reserved
    volatile __u32                              Fe0;                    //0x010c, reseved
    volatile __u32                              reserved132;            //0x0110, reserved
    volatile __u32                              reserved133;            //0x0114, reserved
    volatile __u32                              Lcd0Ch0;                //0x0118, TCON clk reg;
    volatile __u32                              reserved134;            //0x011c, reserved
    volatile __u32                              reserved14[3];          //0x0120, reserved
    volatile __u32                              Lcd0Ch1;                //0x012c, reserved
    volatile __u32                              Csi0;                   //0x0130, csi 0 clk reg;
    volatile __u32                              Csi1;	                //0x0134, CSI_1 clk reg;
    volatile __u32                              reserved142;            //0x0138, reserved
    volatile __u32                              Ve;                     //0x013c, ve module clock
    volatile __u32                              Adda;                   //0x0140, ac digital clock
    volatile __u32                              Avs;                    //0x0144, avs module clock
    volatile __u32                              reserved143;            //0x0148, reserved
    volatile __u32                              reserved15;             //0x014c, reserved
    volatile __u32                              reserved151;            //0x0150, reserved
    volatile __u32                              reserved152;            //0x0154, reserved
    volatile __u32                              reserved153;            //0x0158, reserved
    volatile __u32                              MBus0;                  //0x015C, MBUS clock reg
    volatile __u32                              reserved154;            //0x0160, reserved
    volatile __u32                              reserved16;             //0x0164, reserved
    volatile __u32                              reserved161;            //0x0168, reserved
    volatile __u32                              MipiCsi;                //0x016C, MIPI_CSI
    volatile __u32                              reserved17[4];          //0x0170, reserved
    volatile __u32                              reserved175;            //0x0180, reserved
    volatile __u32                              reserved171;            //0x0184, reserved
    volatile __u32                              reserved172;            //0x0188, reserved
    volatile __u32                              reserved173;            //0x018c, reserved
    volatile __u32                              reserved18[4];          //0x0190, reserved
    volatile __u32                              reserved185[4];         //0x01A0, reserved
    volatile __u32                              reserved19[20];         //0x01B0, reserved
    volatile __u32		                PllLock;                //0x0200, pll lock time
    volatile __u32                              Pll1Lock;               //0x0204, pll1 cpu lock time 
    volatile __u32                              reserved201[6];         //0x0208-0x21c, reserved
    volatile __u32                              PllxBias[12];           //0x220-0x24c, pllx bias reg, (0x23c,0x240,0x248) not exist. 
    volatile __u32                              Pll1Tun;                //0x250, pll1 tun reg
    volatile __u32                              reserved203[3];         //0x254-0x25c, reserved
    volatile __u32                              Pll5Tun;                //0x260, pll5 tun reg
    volatile __u32                              reserved204[3];         //0x264-0x26c, reserved
    volatile __u32                              reserved205[4];         //0x270-0x27c, reserved
    volatile __u32                              reserved206[13];        //0x0280-0x2b0, pllx pattern reg, 0x294,0x29c,0x2a0,0x2a8 not exitst
    volatile __u32                              reserved207[3];         //0x2b4-0x2bc, reserved
    volatile __u32                              AhbReset0;              //0x02c0, AHB1 module reset register 0
    volatile __u32                              AhbReset1;              //0x02c4, AHB1 module reset register 1
    volatile __u32                              AhbReset2;              //0x02c8, AHB1 module reset register 2
    volatile __u32                              reserved21;             //0x02cc, reserved
    volatile __u32                              Apb1Reset;              //0x02d0, APB1 module reset register 3
    volatile __u32                              reserved22;             //0x02d4, reserved
    volatile __u32                              Apb2Reset;              //0x02d8, APB2 module reset register 4
    volatile __u32                              reserved23[9];          //0x02dc-0x02fc, reserved
    volatile __u32                              PsCtrlReg;              //0x0300, PS control reg;
    volatile __u32                              PsCounterReg;           //0x0304, PS counter reg;
} __ccmu_reg_list_t;

#endif  // #ifndef __MEM_CCMU_SUN8IW8P1_H__
