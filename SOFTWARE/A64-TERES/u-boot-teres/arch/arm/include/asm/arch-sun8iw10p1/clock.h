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

#ifndef _SUNXI_CLOCK_H
#define _SUNXI_CLOCK_H

struct sunxi_ccm_reg {

	u32 pll1_cfg;             /* 0x00 pll1 control */
	u32 pll1_tun;             /* 0x04 pll1 tuning */
	u32 pll2_cfg;             /* 0x08 pll2 control */
	u32 pll2_tun;             /* 0x0c pll2 tuning */
	u32 pll3_cfg;             /* 0x10 pll3 control */
	u32 res0;
	u32 pll4_cfg;             /* 0x18 pll4 control */
	u32 res1;
	u32 pll5_cfg;             /* 0x20 pll5 control */
	u32 pll5_tun;             /* 0x24 pll5 tuning */
	u32 pll6_cfg;             /* 0x28 pll6 control */
	u32 pll6_tun;             /* 0x2c pll6 tuning */
	u32 pll7_cfg;             /* 0x30 pll7 control */
	u32 res2;
	u32 pll1_tun2;            /* 0x34 pll5 tuning2 */
	u32 pll5_tun2;            /* 0x3c pll5 tuning2 */
        u32 pll8_cfg;             /* 0x40 pll8 control*/
        u32 res3[3];
	u32 osc24m_cfg;           /* 0x50 osc24m control */
	u32 cpu_ahb_apb0_cfg;     /* 0x54 cpu,ahb and apb0 divide ratio */
	u32 apb1_clk_div_cfg;     /* 0x58 apb1 clock dividor */
	u32 res4; 
	u32 ahb_gate0;            /* 0x60 ahb module clock gating 0 */
	u32 ahb_gate1;            /* 0x64 ahb module clock gating 1 */
	u32 apb0_gate;            /* 0x68 apb0 module clock gating */
	u32 apb1_gate;            /* 0x6c apb1 module clock gating */
	u32 res5[4];
	u32 nand_sclk_cfg;        /* 0x80 nand sub clock control */
	u32 ms_sclk_cfg;          /* 0x84 memory stick sub clock control */
	u32 sd0_clk_cfg;          /* 0x88 sd0 clock control */
	u32 sd1_clk_cfg;          /* 0x8c sd1 clock control */
	u32 sd2_clk_cfg;          /* 0x90 sd2 clock control */
	u32 sd3_clk_cfg;          /* 0x94 sd3 clock control */
	u32 ts_clk_cfg;           /* 0x98 transport stream clock control */
	u32 ss_clk_cfg;           /* 0x9c */
	u32 spi0_clk_cfg;         /* 0xa0 */
	u32 spi1_clk_cfg;         /* 0xa4 */
	u32 spi2_clk_cfg;         /* 0xa8 */
	u32 res6; 
	u32 ir0_clk_cfg;          /* 0xb0 */
	u32 ir1_clk_cfg;          /* 0xb4 */
	u32 iis0_clk_cfg;          /* 0xb8 */
	u32 ac97_clk_cfg;         /* 0xbc */
	u32 spdif_clk_cfg;        /* 0xc0 */
	u32 keypad_clk_cfg;       /* 0xc4 */
	u32 sata_clk_cfg;         /* 0xc8 */
	u32 usb_clk_cfg;          /* 0xcc */
	u32 res7; 
	u32 spi3_clk_cfg;         /* 0xd4 */
        u32 iis1_clk_cfg;         /* 0xd8 */
        u32 iis2_clk_cfg;         /* 0xdc */
	u32 res8[8];
	u32 dram_clk_cfg;         /* 0x100 */
	u32 be0_clk_cfg;          /* 0x104 */
	u32 be1_clk_cfg;          /* 0x108 */
	u32 fe0_clk_cfg;          /* 0x10c */
	u32 fe1_clk_cfg;          /* 0x110 */
	u32 mp_clk_cfg;           /* 0x114 */
	u32 lcd0_ch0_clk_cfg;     /* 0x118 */
	u32 lcd1_ch0_clk_cfg;     /* 0x11c */
	u32 csi_isp_clk_cfg;      /* 0x120 */
	u32 res9;
	u32 tvd_clk_reg;          /* 0x128 */
	u32 lcd0_ch1_clk_cfg;     /* 0x12c */
	u32 lcd1_ch1_clk_cfg;     /* 0x130 */
	u32 csi0_clk_cfg;         /* 0x134 */
	u32 csi1_clk_cfg;         /* 0x138 */
	u32 ve_clk_cfg;           /* 0x13c */
	u32 audio_codec_clk_cfg;  /* 0x140 */
	u32 avs_clk_cfg;          /* 0x144 */
	u32 ace_clk_cfg;          /* 0x148 */
	u32 lvds_clk_cfg;         /* 0x14c */
	u32 hdmi_clk_cfg;         /* 0x150 */
	u32 mali_clk_cfg;         /* 0x154 */
        //u32 mbus_clk_cfg;
        //u32 gmac_clk_cfg;
        //u32 hdmi1_rst_reg;
        //u32 hdmi1_cfg_reg;
        //u32 hdmi1_sclk_reg;     [> s stf slow <]
        //u32 dhmi1_rpt_reg;
        //u32 clk_outa_reg;
        //u32 clk_outb_reg;
};


extern int sunxi_clock_get_corepll(void);
extern int sunxi_clock_set_corepll(int frequency, int core_vol);
extern int sunxi_clock_set_pll6(void);
extern int sunxi_clock_get_pll6(void);

extern void set_pll( void );
extern void set_gpio_gate(void);
extern void reset_pll(void);

#endif /* _SUNXI_CLOCK_H */
