/*
 *  arch/arm/mach-sun6i/include/mach/ccmu_regs.h
 *
 * Copyright 2012 (c) Allwinner.
 * kevin.z.m (kevin@allwinnertech.com)
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
#ifndef __MEM_CCMU_SUN8IW1P1_H__
#define __MEM_CCMU_SUN8IW1P1_H__



typedef union{
    __u32 dwval;
    struct
    {
	__u32   FactorM:2;          //bit0,  PLL1 Factor M
	__u32   reserved0:2;        //bit2,  reserved
	__u32   FactorK:2;          //bit4,  PLL1 factor K
	__u32   reserved1:2;        //bit6,  reserved
	__u32   FactorN:5;          //bit8,  PLL1 Factor N
	__u32   reserved2:11;       //bit13, reserved
	__u32   SigmaEn:1;          //bit24, sigma delta enbale
	__u32   reserved3:3;        //bit25, reserved
	__u32   Lock:1;             //bit28, pll is stable flag, 1-pll has stabled
	__u32   reserved4:2;        //bit29, reserved
	__u32   PLLEn:1;            //bit31, 0-disable, 1-enable, (24Mhz*N*K)/(M*P)

    } bits;
} __ccmu_pll1_reg0000_t;


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
    __u32   Lock:1;             //bit27, lock flag
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


typedef struct __CCMU_MIPI_PLL_REG0040
{
    __u32   FactorM:4;          //bit0,  PLL FactorM
    __u32   FactorK:2;          //bit4,  PLL FactorM
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
	__u32   AXIClkDiv:3;        //bit0,  AXI clock divide ratio, 000-1, 001-2, 010-3, 011/1xx-4
	__u32   reserved0:5;        //bit3,  reserved
	__u32   AtbApbClkDiv:2;     //bit8,  ATB/APB clock div, 00-1, 01-2, 1x-4, need care? maybe, just for tips.
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


typedef struct __CCMU_AXI_GATE_REG005c
{
    __u32   Sdram:1;            //bit0,  gating AXI clock for SDRAM, 0-mask, 1-pass
    __u32   reserved0:31;       //bit1,  reserved
} __ccmu_axi_gate_reg005c_t;


typedef struct __CCMU_AHB1_GATE0_REG0060
{
    __u32   MipiCsi:1;          //bit0,  gating AHB clock for mipi csi, 0-mask, 1-pass
    __u32   MipiDsi:1;          //bit1,  gating AHB clock for mipi dsi, 0-mask, 1-pass
    __u32   reserved0:3;        //bit2,  reserved
    __u32   Ss:1;               //bit5,  gating AHB clock for SS, 0-mask, 1-pass
    __u32   Dma:1;              //bit6,  gating AHB clock for DMA, 0-mask, 1-pass
    __u32   reserved1:1;        //bit7,  reserved
    __u32   Sd0:1;              //bit8,  gating AHB clock for SD/MMC0, 0-mask, 1-pass
    __u32   Sd1:1;              //bit9,  gating AHB clock for SD/MMC1, 0-mask, 1-pass
    __u32   Sd2:1;              //bit10, gating AHB clock for SD/MMC2, 0-mask, 1-pass
    __u32   Sd3:1;              //bit11, gating AHB clock for SD/MMC3, 0-mask, 1-pass
    __u32   Nand1:1;            //bit12, gating AHB clock for NAND1, 0-mask, 1-pass
    __u32   Nand0:1;            //bit13, gating AHB clock for NAND0, 0-mask, 1-pass
    __u32   Dram:1;             //bit14, gating AHB clock for SDRAM, 0-mask, 1-pass
    __u32   reserved2:2;        //bit15, reserved
    __u32   Gmac:1;             //bit17, gating AHB clock for GMAC, 0-mask, 1-pass
    __u32   Ts:1;               //bit18, gating AHB clock for TS, 0-mask, 1-pass
    __u32   HsTmr:1;            //bit19, gating AHB clock for High speed timer 0-mask, 1-pass
    __u32   Spi0:1;             //bit20, gating AHB clock for SPI0, 0-mask, 1-pass
    __u32   Spi1:1;             //bit21, gating AHB clock for SPI1, 0-mask, 1-pass
    __u32   Spi2:1;             //bit22, gating AHB clock for SPI2, 0-mask, 1-pass
    __u32   Spi3:1;             //bit23, gating AHB clock for SPI3, 0-mask, 1-pass
    __u32   Otg:1;              //bit24, gating AHB clock for USB-OTG, 0-mask, 1-pass
    __u32   reserved3:1;        //bit25, reserved
    __u32   Ehci0:1;            //bit26, gating AHB clock for EHCI0, 0-mask, 1-pass
    __u32   Ehci1:1;            //bit27, gating AHB clock for EHCI1, 0-mask, 1-pass
    __u32   reserved4:1;        //bit28, reserved
    __u32   Ohci0:1;            //bit29, gating AHB clock for OHCI0, 0-mask, 1-pass
    __u32   Ohci1:1;            //bit30, gating AHB clock for OHCI1, 0-mask, 1-pass
    __u32   Ohci2:1;            //bit31, gating AHB clock for OHCI2, 0-mask, 1-pass
} __ccmu_ahb1_gate0_reg0060_t;


typedef struct __CCMU_AHB1_GATE1_REG0064
{
    __u32   Ve:1;               //bit0,  gating AHB clock for VE, 0-mask, 1-pass
    __u32   reserved0:3;        //bit1,  reserved
    __u32   Lcd0:1;             //bit4,  gating AHB clock for LCD0, 0-mask, 1-pass
    __u32   Lcd1:1;             //bit5,  gating AHB clock for LCD1, 0-mask, 1-pass
    __u32   reserved1:2;        //bit6,  reserved
    __u32   Csi0:1;             //bit8,  gating AHB clock for CSI0, 0-mask, 1-pass
    __u32   Csi1:1;             //bit9,  gating AHB clock for CSI1, 0-mask, 1-pass
    __u32   reserved2:1;        //bit10, reserved
    __u32   Hdmi:1;             //bit11, gating AHB clock for HDMI, 0-mask, 1-pass
    __u32   Be0:1;              //bit12, gating AHB clock for DE-BE0, 0-mask, 1-pass
    __u32   Be1:1;              //bit13, gating AHB clock for DE-BE1, 0-mask, 1-pass
    __u32   Fe0:1;              //bit14, gating AHB clock for DE-FE0, 0-mask, 1-pass
    __u32   Fe1:1;              //bit15, gating AHB clock for DE-FE1, 0-mask, 1-pass
    __u32   reserved3:2;        //bit16, reserved
    __u32   Mp:1;               //bit18, gating AHB clock for MP, 0-mask, 1-pass
    __u32   reserved4:1;        //bit19, reserved
    __u32   Gpu:1;              //bit20, gating AHB clock for GPU, 0-mask, 1-pass
    __u32   MsgBox:1;           //bit21, gating AHB clock for MSG-BOX, 0-mask, 1-pass
    __u32   SpinLock:1;         //bit22, gating AHB clock for SPIN-LOCK, 0-mask, 1-pass
    __u32   Deu0:1;             //bit23, gating AHB clock for DEU0, 0-mask, 1-pass
    __u32   Deu1:1;             //bit24, gating AHB clock for DEU1, 0-mask, 1-pass
    __u32   Drc0:1;             //bit25, gating AHB clock for DRC0, 0-mask, 1-pass
    __u32   Drc1:1;             //bit26, gating AHB clock for DRC1, 0-mask, 1-pass
    __u32   MtcAcc:1;           //bit27, gating AHB clock for MTC-ACC, 0-mask, 1-pass
    __u32   reserved5:4;        //bit28, reserved
} __ccmu_ahb1_gate1_reg0064_t;


typedef struct __CCMU_APB1_GATE_REG0068
{
    __u32   Adda:1;             //bit0,  gating APB clock for audio codec, 0-mask, 1-pass
    __u32   Spdif:1;            //bit1,  gating APB clock for SPDIF, 0-mask, 1-pass
    __u32   reserved0:2;        //bit2,  reserved
    __u32   Dmic:1;             //bit4,  gating APB clock for digital mic 
    __u32   Pio:1;              //bit5,  gating APB clock for PIO, 0-mask, 1-pass
    __u32   reserved1:6;        //bit6,  reserved
    __u32   I2s0:1;             //bit12, gating APB clock for I2s-0, 0-mask, 1-pass
    __u32   I2s1:1;             //bit13, gating APB clock for I2s-1, 0-mask, 1-pass
    __u32   reserved2:18;       //bit14, reserved
} __ccmu_apb1_gate_reg0068_t;


typedef struct __CCMU_APB2_GATE_REG006C
{
    __u32   Twi0:1;             //bit0,  gating APB clock for TWI0, 0-mask, 1-pass
    __u32   Twi1:1;             //bit1,  gating APB clock for TWI1, 0-mask, 1-pass
    __u32   Twi2:1;             //bit2,  gating APB clock for TWI2, 0-mask, 1-pass
    __u32   Twi3:1;             //bit3,  gating APB clock for TWI3, 0-mask, 1-pass
    __u32   reserved0:12;       //bit4,  reserved
    __u32   Uart0:1;            //bit16, gating APB clock for UART0, 0-mask, 1-pass
    __u32   Uart1:1;            //bit17, gating APB clock for UART1, 0-mask, 1-pass
    __u32   Uart2:1;            //bit18, gating APB clock for UART2, 0-mask, 1-pass
    __u32   Uart3:1;            //bit19, gating APB clock for UART3, 0-mask, 1-pass
    __u32   Uart4:1;            //bit20, gating APB clock for UART4, 0-mask, 1-pass
    __u32   Uart5:1;            //bit21, gating APB clock for UART5, 0-mask, 1-pass
    __u32   reserved1:10;       //bit22, reserved
} __ccmu_apb2_gate_reg006c_t;


/* normal module clock */
typedef struct __CCMU_MODULE0_CLK
{
    __u32   DivM:4;             //bit0,  clock divide ratio, divided by (m+1), 1~16 ex.
    __u32   reserved0:4;        //bit4,  reserved
    __u32   OutClkCtrl:3;       //bit8,  output clock phase control, 0~7
    __u32   reserved1:5;        //bit11,  reserved
    __u32   DivN:2;             //bit16, clock pre-divide ratio, predivided by 2^n , 1/2/4/8 ex.
    __u32   reserved2:2;        //bit18, reserved
    __u32   SampClkCtrl:3;      //bit20, sample clock phase control, 0~7
    __u32   reserved3:1;        //bit23, reserved
    __u32   ClkSrc:3;           //bit24, clock source select, defined with different modules
    __u32   reserved4:4;        //bit26, reserved
    __u32   ClkGate:1;          //bit31, Gating special clock, 0-CLOCK OFF, 1-CLOCK ON
} __ccmu_module0_clk_t;


/* normal module clock */
typedef struct __CCMU_MODULE1_CLK
{
    __u32   reserved0:16;       //bit0,  reserved
    __u32   ClkSrc:2;           //bit16, clock source select, 00:PLL2X8, 01:PLL2X8/2, 10:PLLX8/4, 11:PLL2X1
    __u32   reserved1:13;       //bit18, reserved
    __u32   ClkGate:1;          //bit31, Gating special clock, 0-CLOCK OFF, 1-CLOCK ON

} __ccmu_module1_clk_t;


/* normal module clock */
typedef struct __CCMU_MODULE_CLK
{
    __u32   reserved0:31;       //bit0,  reserved
    __u32   ClkGate:1;          //bit31, Gating special clock, 0-CLOCK OFF, 1-CLOCK ON

} __ccmu_module_clk_t;


typedef struct __CCMU_USB_CLK_REG00CC
{
    __u32   UsbPhy0Rst:1;       //bit0,  USB PHY0 reset control, 0-reset valid, 1-reset invalid
    __u32   UsbPhy1Rst:1;       //bit1,  USB PHY1 reset control, 0-reset valid, 1-reset invalid
    __u32   UsbPhy2Rst:1;       //bit2,  USB PHY2 reset control, 0-reset valid, 1-reset invalid
    __u32   reserved0:5;        //bit3,  reserved
    __u32   Phy0Gate:1;         //bit8,  gating special clock for USBPHY0, 0-CLOCK OFF, 1-CLOCK ON
    __u32   Phy1Gate:1;         //bit9,  gating special clock for USBPHY1, 0-CLOCK OFF, 1-CLOCK ON
    __u32   Phy2Gate:1;         //bit10, gating special clock for USBPHY2, 0-CLOCK OFF, 1-CLOCK ON
    __u32   reserved1:5;        //bit11, reserved
    __u32   Ohci0Gate:1;        //bit16, gating special clock for OHCI0, 0-CLOCK OFF, 1-CLOCK ON
    __u32   Ohci1Gate:1;        //bit17, gating special clock for OHCI1, 0-CLOCK OFF, 1-CLOCK ON
    __u32   Ohci2Gate:1;        //bit18, gating special clock for OHCI2, 0-CLOCK OFF, 1-CLOCK ON
    __u32   reserved2:13;       //bit19,  reserved
} __ccmu_usb_clk_reg00cc_t;


typedef struct __CCMU_GMAC_CLK_REG00D0
{
    __u32   Gtcs:2;            //bit0,  gmac transmit clock source, 00-transmit clk src for M2, 01-external transmit clk src for GM2 and RGM2,
                               //  10-internal transmit clk src for GM2 and RGM2, 11-reserved
    __u32   Gpit:1;            //bit2,  gmac phy interface type, 0-GM2/M2, 1-RGM2
    __u32   Gtxie:1;           //bit3,  enable gmac transmit clock invertor, 0-disable, 1-enable
    __u32   Grxie:1;           //bit4,  enable gmac recieve clock invertor, 0-disable, 1-enable
    __u32   Grxdc:3;           //bit5,  config gmac recieve clock delay chain
    __u32   reserved0:24;      //bit8,  reserved
} __ccmu_gmac_clk_reg00d0_t;


typedef struct __CCMU_DRAM_CFG_REG00F4
{
    __u32   Div1M:4;            //bit0,  sdr clock divider of configuration 1
    __u32   ClkSrc1:1;          //bit4,  sdr clock source of configuration 1, 0:PLL5, 1:PLL6
    __u32   reserved0:3;        //bit5,  reserved
    __u32   Div0M:4;            //bit8,  sdr clock divider of configuration 0
    __u32   ClkSrc0:1;          //bit12, sdr clock source of configuration 0, 0:PLL5, 1:PLL6
    __u32   reserved1:3;        //bit13, reserved
    __u32   SdrClkUpd:1;        //bit16, sdr clock configuration 0 update, 0:invalid, 1:valid
    __u32   reserved2:14;       //bit14, reserved
    __u32   CtrlerRst:1;        //bit31, sdram controller reset 0:assert, 1:de-assert
} __ccmu_dram_cfg_reg00f4_t;


typedef struct __CCMU_DRAM_GATE_REG0100
{
    __u32   Ve:1;               //bit0,  Gating dram clock for VE, 0-mask, 1-pass
    __u32   CsiIsp:1;           //bit1,  Gating dram clock for CSI0, CSI1, MIPI_CSI0, ISP, 0-mask, 1-pass
    __u32   reserved0:1;        //bit2,  reserved
    __u32   Ts:1;               //bit3,  Gating dram clock for TS, 0-mask, 1-pass
    __u32   reserved1:12;       //bit4,  reserved
    __u32   Drc0:1;             //bit16, Gating dram clock for DRC0, 0-mask, 1-pass
    __u32   Drc1:1;             //bit17, Gating dram clock for DRC1, 0-mask, 1-pass
    __u32   Deu0:1;             //bit18, Gating dram clock for DEU0, 0-mask, 1-pass
    __u32   Deu1:1;             //bit19, Gating dram clock for DEU1, 0-mask, 1-pass
    __u32   reserved2:4;        //bit20,  reserved
    __u32   Fe0:1;              //bit24, Gating dram clock for DE_FE0, 0-mask, 1-pass
    __u32   Fe1:1;              //bit25, Gating dram clock for DE_FE1, 0-mask, 1-pass
    __u32   Be0:1;              //bit26, Gating dram clock for DE_BE0, 0-mask, 1-pass
    __u32   Be1:1;              //bit27, Gating dram clock for DE_BE1, 0-mask, 1-pass
    __u32   Mp:1;               //bit28, Gating dram clock for MP, 0-mask, 1-pass
    __u32   reserved3:3;        //bit29, reserved
} __ccmu_dram_gate_reg0100_t;


/* display module clock */
typedef struct __CCMU_DISP_CLK
{
    __u32   DivM:4;             //bit0,  clock divide ratio, divied by (m+1), 1~16 ex.
    __u32   reserved0:20;       //bit4,  reserved
    __u32   ClkSrc:3;           //bit24, clock source select, 000-PLL3, 001-PLL7
                                //       010-PLL6X2, 011-PLL8, 100-PLL9, 101-PLL10 110/111-reserved
    __u32   reserved1:4;        //bit27, reserved
    __u32   ClkGate:1;          //bit31, gating special clock, 0-clock off, 1-clock on
} __ccmu_disp_clk_t;


/* csi module clock source */
typedef struct __CCMU_CSI_CLK
{
    __u32   MClkDiv:4;          //bit0,  master clock divide ratio, divided by (m+1), 1~16, ex.
    __u32   reserved0:4;        //bit4,  reserved
    __u32   MClkSrc:3;          //bit8,  clock source select, 000-PLL3(1x), 001-PLL7(1x), 010-OSC24M, 010/011/100/111-reserved, 101-PLL3(2x), 110:PLL7(2x)
    __u32   reserved1:4;        //bit11, reserved
    __u32   MClkGate:1;         //bit15, gating master clock
    __u32   SClkDiv:4;          //bit16, csi clock dirvide ratio, 1~16
    __u32   reserved2:4;        //bit20, reserved
    __u32   SClkSrc:3;          //bit24, special clock source select, 000:PLL3(1x), 001:PLL7(1x), 010:PLL3(2x)
                                //       011:PLL7(2x), 100:mipi pll, 101~111:reserved
    __u32   reserved3:4;        //bit27, reserved
    __u32   SClkGate:1;         //bit31, Gating special clock, 0-clock off, 1-clock on
} __ccmu_csi_clk_t;


typedef struct __CCMU_VE_CLK_REG013C
{
    __u32   reserved0:16;       //bit0,  reserved
    __u32   ClkDiv:3;           //bit16, Clock pre-divide ratio, divided by (n+1), 1~8 ex.
    __u32   reserved1:12;       //bit19, reserved
    __u32   ClkGate:1;          //bit31, gating special clock for VE, 0-mask, 1-pass
}__ccmu_ve_clk_reg013c_t;


typedef struct __CCMU_HDMI_CLK_REG0150
{
    __u32   ClkDiv:4;           //bit0,  clock divide ratio, divided by (m+1), 1~16 ex.
    __u32   reserved0:20;       //bit4,  reserved
    __u32   ClkSrc:2;           //bit24, clock source select, 00-PLL3(1x), 01-PLL7(1x), 10-PLL3(2x), 11-PLL7(2x)
    __u32   reserved1:4;        //bit26, reserved
    __u32   DDCGate:1;          //bit30, Gating ddc clock, 0-clock off, 1-clock on
    __u32   ClkGate:1;          //bit31, Gating special clock, 0-clock off, 1-clock on
} __ccmu_hdmi_clk_reg0150_t;


typedef struct __CCMU_MIPI_CLK
{
    __u32   PClkDiv:4;          //bit0,  clock divide ratio, divided by (m+1), 1~16 ex.
    __u32   reserved0:4;        //bit4,  reserved
    __u32   PClkSrc:2;          //bit8,  clock source select, 00-PLL3(1x), 01-PLL7(1x), 10-PLL3(2x), 11-PLL7(2x)
    __u32   reserved1:5;        //bit10, reserved
    __u32   PClkGate:1;         //bit15, gating phy clock
    __u32   SClkDiv:4;          //bit16, Special clock divide ratio
    __u32   reserved2:4;        //bit20, reserved
    __u32   SClkSrc:3;          //bit24, special clock source select, 00-PLL3(1x), 01-PLL7(1x), 10-PLL3(2x), 11-PLL7(2x)
    __u32   reserved3:4;        //bit27, reserved
    __u32   SClkGate:1;         //bit31, Gating special clock, 0-clock off, 1-clock on
} __ccmu_mipi_clk_t;


typedef struct __CCMU_PLLLOCK_REG0200
{
    __u32   LockTime:16;        //bit0,  PLL lock time, based on us
    __u32   reserved:16;        //bit16, reserved
} __ccmu_plllock_reg0200_t;


typedef struct __CCMU_MOD_RST_REG02C0
{
    __u32   reserved0:1;        //bit0,  reserved
    __u32   MipiDsi:1;          //bit1,  mipi dsi reset, 0:assert, 1:de-assert
    __u32   reserved1:3;        //bit2,  reserved
    __u32   Ss:1;               //bit5,  ss reset, 0:assert, 1:de-assert
    __u32   Dma:1;              //bit6,  dma reset, 0:assert, 1:de-assert
    __u32   reserved2:1;        //bit7,  reserved
    __u32   Sd0:1;              //bit8,  sd/mmc0 reset, 0:assert, 1:de-assert
    __u32   Sd1:1;              //bit9,  sd/mmc1 reset, 0:assert, 1:de-assert
    __u32   Sd2:1;              //bit10, sd/mmc2 reset, 0:assert, 1:de-assert
    __u32   Sd3:1;              //bit11, sd/mmc3 reset, 0:assert, 1:de-assert
    __u32   Nand1:1;            //bit12, nand1 reset, 0:assert, 1:de-assert
    __u32   Nand0:1;            //bit13, nand0 reset, 0:assert, 1:de-assert
    __u32   Sdram:1;            //bit14, sdram AHB reset, 0:assert, 1:de-assert
    __u32   reserved3:2;        //bit15, reserved
    __u32   Gmac:1;             //bit17, Gmac reset, 0:assert, 1:de-assert
    __u32   Ts:1;               //bit18, ts reset, 0:assert, 1:de-assert
    __u32   HsTmr:1;            //bit19, hstimer reset, 0:assert, 1:de-assert
    __u32   Spi0:1;             //bit20, spi0 reset, 0:assert, 1:de-assert
    __u32   Spi1:1;             //bit21, spi1 reset, 0:assert, 1:de-assert
    __u32   Spi2:1;             //bit22, spi2 reset, 0:assert, 1:de-assert
    __u32   Spi3:1;             //bit23, spi3 reset, 0:assert, 1:de-assert
    __u32   Otg:1;              //bit24, usb otg reset, 0:assert, 1:de-assert
    __u32   reserved4:1;        //bit25, reserved
    __u32   Ehci0:1;            //bit26, usb EHCI0 reset, 0:assert, 1:de-assert
    __u32   Ehci1:1;            //bit27, usb EHCI1 reset, 0:assert, 1:de-assert
    __u32   reserved5:1;        //bit28, reserved
    __u32   Ohci0:1;            //bit29, usb OHCI0 reset, 0:assert, 1:de-assert
    __u32   Ohci1:1;            //bit30, usb OHCI1 reset, 0:assert, 1:de-assert
    __u32   Ohci2:1;            //bit31, usb OHCI2 reset, 0:assert, 1:de-assert

} __ccmu_mod_rst_reg02c0_t;


typedef struct __CCMU_MOD_RST_REG02C4
{
    __u32   Ve:1;               //bit0,  ve reset, 0:assert, 1:de-assert
    __u32   reserved0:3;        //bit1,  reserved
    __u32   Lcd0:1;             //bit4,  lcd0 reset, 0:assert, 1:de-assert
    __u32   Lcd1:1;             //bit5,  lcd1 reset, 0:assert, 1:de-assert
    __u32   reserved1:2;        //bit6,  reserved
    __u32   Csi0:1;             //bit8,  lcd0 reset, 0:assert, 1:de-assert
    __u32   Csi1:1;             //bit9,  lcd1 reset, 0:assert, 1:de-assert
    __u32   reserved2:1;        //bit10, reserved
    __u32   Hdmi:1;             //bit11, hdmi reset, 0:assert, 1:de-assert
    __u32   Be0:1;              //bit12, be0 reset, 0:assert, 1:de-assert
    __u32   Be1:1;              //bit13, be1 reset, 0:assert, 1:de-assert
    __u32   Fe0:1;              //bit14, fe0 reset, 0:assert, 1:de-assert
    __u32   Fe1:1;              //bit15, fe1 reset, 0:assert, 1:de-assert
    __u32   reserved3:2;        //bit16, reserved
    __u32   Mp:1;               //bit18, mp reset, 0:assert, 1:de-assert
    __u32   reserved4:1;        //bit19, reserved

    __u32   Gpu:1;              //bit20, gpu reset, 0:assert, 1:de-assert
    __u32   MsgBox:1;           //bit21, msg-box reset, 0:assert, 1:de-assert
    __u32   SpinLock:1;         //bit22, spin-lock reset, 0:assert, 1:de-assert
    __u32   Deu0:1;             //bit23, deu0 reset, 0:assert, 1:de-assert
    __u32   Deu1:1;             //bit24, deu1 reset, 0:assert, 1:de-assert
    __u32   Drc0:1;             //bit25, drc0 reset, 0:assert, 1:de-assert
    __u32   Drc1:1;             //bit26, drc1 reset, 0:assert, 1:de-assert
    __u32   MtcAcc:1;           //bit27, mtc-acc reset, 0:assert, 1:de-assert
    __u32   reserved5:4;        //bit28, reserved

} __ccmu_mod_rst_reg02c4_t;


typedef struct __CCMU_MOD_RST_REG02C8
{
    __u32   Lvds:1;             //bit0,  lvds reset, 0:assert, 1:de-assert
    __u32   reserved:31;        //bit1,  reserved

} __ccmu_mod_rst_reg02c8_t;


typedef struct __CCMU_MOD_RST_REG02D0
{
    __u32   Adda:1;             //bit0,  audio codec reset, 0:assert, 1:de-assert
    __u32   Spdif:1;            //bit1,  spdif reset, 0:assert, 1:de-assert
    __u32   reserved0:3;        //bit2,  reserved
    __u32   Pio:1;              //bit5,  pio reset, 0:assert, 1:de-assert
    __u32   reserved1:6;        //bit6,  reserved
    __u32   I2s0:1;             //bit12, i2s-0 reset, 0:assert, 1:de-assert
    __u32   I2s1:1;             //bit13, i2s-1 reset, 0:assert, 1:de-assert
    __u32   reserved2:18;       //bit14, reserved

} __ccmu_mod_rst_reg02d0_t;


typedef struct __CCMU_MOD_RST_REG02D8
{
    __u32   Twi0:1;             //bit0,  twi0 reset, 0:assert, 1:de-assert
    __u32   Twi1:1;             //bit1,  twi1 reset, 0:assert, 1:de-assert
    __u32   Twi2:1;             //bit2,  twi2 reset, 0:assert, 1:de-assert
    __u32   Twi3:1;             //bit3,  twi3 reset, 0:assert, 1:de-assert
    __u32   reserved0:12;       //bit4,  reserved
    __u32   Uart0:1;            //bit16, uart0 reset, 0:assert, 1:de-assert
    __u32   Uart1:1;            //bit17, uart1 reset, 0:assert, 1:de-assert
    __u32   Uart2:1;            //bit18, uart2 reset, 0:assert, 1:de-assert
    __u32   Uart3:1;            //bit19, uart3 reset, 0:assert, 1:de-assert
    __u32   Uart4:1;            //bit20, uart4 reset, 0:assert, 1:de-assert
    __u32   Uart5:1;            //bit21, uart5 reset, 0:assert, 1:de-assert
    __u32   reserved1:10;       //bit22, reserved

} __ccmu_mod_rst_reg02d8_t;


typedef struct __CCMU_CLK_OUT
{
    __u32   reserved0:8;       //bit0,  reserved
    __u32   DivM:5;            //bit8,  clock output divide factor m
    __u32   reserved1:7;       //bit13, reserved
    __u32   DivN:2;            //bit20, clock output divide factor n
    __u32   reserved2:2;       //bit22, reserved
    __u32   ClkSrc:4;          //bit24, clock out source select
    __u32   reserved3:3;       //bit28, reserved
    __u32   ClkEn:1;           //bit31, clock out enable

} __ccmu_clk_out_t;


typedef struct __CCMU_REG_LIST
{
    volatile __ccmu_pll1_reg0000_t              Pll1Ctl;    //0x0000, PLL1 control
    volatile __u32                              reserved0;  //0x0004, reserved
    volatile __ccmu_pll2_reg0008_t              Pll2Ctl;    //0x0008, PLL2 control
    volatile __u32                              reserved1;  //0x000c, reserved
    volatile __ccmu_media_pll_t                 Pll3Ctl;    //0x0010, PLL3 control
    volatile __u32                              reserved2;  //0x0014, reserved
    volatile __ccmu_media_pll_t                 Pll4Ctl;    //0x0018, PLL4 control
    volatile __u32                              reserved3;  //0x001c, reserved
    volatile __ccmu_pll5_reg0020_t              Pll5Ctl;    //0x0020, PLL5 control
    volatile __u32                              reserved4;  //0x0024, reserved
    volatile __ccmu_pll6_reg0028_t              Pll6Ctl;    //0x0028, PLL6 control
    volatile __u32                              reserved5;  //0x002c, reserved
    volatile __ccmu_media_pll_t                 Pll7Ctl;    //0x0030, PLL7 control
    volatile __u32                              reserved6;  //0x0034, reserved
    volatile __ccmu_media_pll_t                 Pll8Ctl;    //0x0038, PLL8 control
    volatile __u32                              reserved7;  //0x003c, reserved
    volatile __ccmu_mipi_pll_reg0040_t          MipiPllCtl; //0x0040, MIPI PLL control
    volatile __ccmu_media_pll_t                 Pll9Ctl;    //0x0044, PLL9 control
    volatile __ccmu_media_pll_t                 Pll10Ctl;   //0x0048, PLL10 control
    volatile __u32                              reserved8;  //0x004c, reserved
    volatile __ccmu_sysclk_ratio_reg0050_t      SysClkDiv;  //0x0050, system clock divide ratio
    volatile __ccmu_ahb1_ratio_reg0054_t        Ahb1Div;    //0x0054, ahb1/apb1 clock divide ratio
    volatile __ccmu_apb2_ratio_reg0058_t        Apb2Div;    //0x0058, apb2 clock divide ratio
    volatile __ccmu_axi_gate_reg005c_t          AxiGate;    //0x005c, axi clock gating
    volatile __ccmu_ahb1_gate0_reg0060_t        AhbGate0;   //0x0060, ahb clock gate 0
    volatile __ccmu_ahb1_gate1_reg0064_t        AhbGate1;   //0x0064, ahb clock gate 1
    volatile __ccmu_apb1_gate_reg0068_t         Apb1Gate;   //0x0068, apb1 clock gate
    volatile __ccmu_apb2_gate_reg006c_t         Apb2Gate;   //0x006c, apb2 clock gate
    volatile __u32                              reserved9[4];   //0x0070, reserved
    volatile __ccmu_module0_clk_t               Nand0;      //0x0080, nand controller 0 clock
    volatile __ccmu_module0_clk_t               Nand1;      //0x0084, nand controller 1 clock
    volatile __ccmu_module0_clk_t               Sd0;        //0x0088, sd/mmc controller 0 clock
    volatile __ccmu_module0_clk_t               Sd1;        //0x008c, sd/mmc controller 1 clock
    volatile __ccmu_module0_clk_t               Sd2;        //0x0090, sd/mmc controller 2 clock
    volatile __ccmu_module0_clk_t               Sd3;        //0x0094, sd/mmc controller 3 clock
    volatile __ccmu_module0_clk_t               Ts;         //0x0098, TS controller clock
    volatile __ccmu_module0_clk_t               Ss;         //0x009c, SS controller clock
    volatile __ccmu_module0_clk_t               Spi0;       //0x00a0, spi controller 0 clock
    volatile __ccmu_module0_clk_t               Spi1;       //0x00a4, spi controller 1 clock
    volatile __ccmu_module0_clk_t               Spi2;       //0x00a8, spi controller 2 clock
    volatile __ccmu_module0_clk_t               Spi3;       //0x00ac, spi controller 3 clock
    volatile __ccmu_module1_clk_t               I2s0;       //0x00b0, I2s-0 clock
    volatile __ccmu_module1_clk_t               I2s1;       //0x00b4, I2s-1 clock
    volatile __u32                              reserved10[2];  //0x00b8, reserved
    volatile __ccmu_module1_clk_t               Spdif;      //0x00c0, SPDIF clock
    volatile __u32                              reserved11[2];  //0x00c4, reserved
    volatile __ccmu_usb_clk_reg00cc_t           Usb;        //0x00cc, usb clock
    volatile __ccmu_gmac_clk_reg00d0_t          Gmac;       //0x00d0, gmac clock
    volatile __u32                              reserved12[7];  //0x00d4, reserved
    volatile __ccmu_module0_clk_t               Mdfs;       //0x00f0, mdfs clock
    volatile __ccmu_dram_cfg_reg00f4_t          DramCfg;    //0x00f4, dram configuration clock
    volatile __u32                              reserved13[2];  //0x00f8, reserved
    volatile __ccmu_dram_gate_reg0100_t         DramGate;   //0x0100, dram module clock
    volatile __ccmu_disp_clk_t                  Be0;        //0x0104, BE0 module clock
    volatile __ccmu_disp_clk_t                  Be1;        //0x0108, BE1 module clock
    volatile __ccmu_disp_clk_t                  Fe0;        //0x010c, FE0 module clock
    volatile __ccmu_disp_clk_t                  Fe1;        //0x0110, FE1 module clock
    volatile __ccmu_disp_clk_t                  Mp;         //0x0114, MP module clock
    volatile __ccmu_disp_clk_t                  Lcd0Ch0;    //0x0118, LCD0 CH0 module clock
    volatile __ccmu_disp_clk_t                  Lcd1Ch0;    //0x011c, LCD1 CH0 module clock
    volatile __u32                              reserved14[3];  //0x0120, reserved
    volatile __ccmu_disp_clk_t                  Lcd0Ch1;    //0x012c, LCD0 CH1 module clock
    volatile __ccmu_disp_clk_t                  Lcd1Ch1;    //0x0130, LCD1 CH1 module clock
    volatile __ccmu_csi_clk_t                   Csi0;       //0x0134, csi0 module clock
    volatile __ccmu_csi_clk_t                   Csi1;       //0x0138, csi1 module clock
    volatile __ccmu_ve_clk_reg013c_t            Ve;         //0x013c, ve module clock
    volatile __ccmu_module1_clk_t               Adda;       //0x0140, adda module clock
    volatile __ccmu_module_clk_t                Avs;        //0x0144, avs module clock
    volatile __ccmu_module1_clk_t               Dmic;       //0x0148, digtal mic module clock
    volatile __u32                              reserved15; //0x014c, reserved
    volatile __ccmu_hdmi_clk_reg0150_t          Hdmi;       //0x0150, hdmi module clock
    volatile __ccmu_module1_clk_t               Ps;         //0x0154, ps module clock
    volatile __ccmu_module0_clk_t               MtcAcc;     //0x0158, MTC ACC clock
    volatile __ccmu_module0_clk_t               MBus0;      //0x015C, MBUS controller 0 clock
    volatile __ccmu_module0_clk_t               MBus1;      //0x0160, MBUS controller 1 clock
    volatile __u32                              reserved16; //0x0164, reserved
    volatile __ccmu_mipi_clk_t                  MipiDsi;    //0x0168, MIPI DSI clock
    volatile __ccmu_mipi_clk_t                  MipiCsi;    //0x016C, MIPI CSI clock
    volatile __u32                              reserved17[4];  //0x0170, reserved
    volatile __ccmu_module0_clk_t               IepDrc0;    //0x0180, IEP DRC0 clock
    volatile __ccmu_module0_clk_t               IepDrc1;    //0x0184, IEP DRC1 clock
    volatile __ccmu_module0_clk_t               IepDeu0;    //0x0188, IEP DEU0 clock
    volatile __ccmu_module0_clk_t               IepDeu1;    //0x018c, IEP DEU1 clock
    volatile __u32                              reserved18[4];  //0x0190, reserved
    volatile __ccmu_module0_clk_t               GpuCore;    //0x01A0, GPU Core clock
    volatile __ccmu_module0_clk_t               GpuMem;     //0x01A4, GPU Memory clock
    volatile __ccmu_module0_clk_t               GpuHyd;     //0x01A8, GPU hyd clock
    volatile __u32                              reserved19[21]; //0x01AC, reserved

    volatile __ccmu_plllock_reg0200_t           PllLock;    //0x0200, pll lock time
    volatile __u32                              Pll1Lock;	//0x0204, pll1 lock time 
    volatile __u32                              reserved201[6]; //0x0208-0x21c, reserved
    volatile __u32                              PllxBias[11];	//0x220-0x248, pllx bias reg
    volatile __u32                              reserved202; //0x24c, reserved
    volatile __u32                              Pll1Tun; //0x250, pll1 tun reg
    volatile __u32                              reserved203[3]; //0x254-0x25c, reserved
    volatile __u32                              Pll5Tun; //0x260, pll5 tun reg
    volatile __u32                              reserved204[3]; //0x264-0x26c, reserved
    volatile __u32                              MipiPllTun; //0x270, mipi pll tun reg
    volatile __u32                              reserved205[3]; //0x274-0x27c, reserved
    volatile __u32                              reserved206[16]; //0x0280-0x2bc, reserved

    volatile __ccmu_mod_rst_reg02c0_t           AhbReset0;  //0x02c0, AHB1 module reset register 0
    volatile __ccmu_mod_rst_reg02c4_t           AhbReset1;  //0x02c4, AHB1 module reset register 1
    volatile __ccmu_mod_rst_reg02c8_t           AhbReset2;  //0x02c8, AHB1 module reset register 2
    volatile __u32                              reserved21; //0x02cc, reserved
    volatile __ccmu_mod_rst_reg02d0_t           Apb1Reset;  //0x02d0, APB1 module reset register
    volatile __u32                              reserved22; //0x02d4, reserved
    volatile __ccmu_mod_rst_reg02d8_t           Apb2Reset;  //0x02d8, APB2 module reset register
    volatile __u32                              reserved23[9];  //0x02dc, reserved
    volatile __ccmu_clk_out_t                   ClkOutA;    //0x0300, pll lock time
    volatile __ccmu_clk_out_t                   ClkOutB;    //0x0304, pll lock time
    volatile __ccmu_clk_out_t                   ClkOutC;    //0x0308, pll lock time

} __ccmu_reg_list_t;


#endif  // #ifndef __MEM_CCMU_SUN8IW1P1_H__

