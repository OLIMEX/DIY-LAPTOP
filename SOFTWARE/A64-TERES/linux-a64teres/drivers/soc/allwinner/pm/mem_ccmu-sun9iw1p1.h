/*
 *  arch/arm/mach-sunxi/pm/ccmu-sun9iw1p1.h
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
#ifndef __MEM_CCMU_SUN9IW1P1_H__
#define __MEM_CCMU_SUN9IW1P1_H__

//
// Register Offset
//
#define PLL_C0_CFG_REG_OFF          		0x0000
#define PLL_C1_CFG_REG_OFF          		0x0004
#define PLL_AUDIO_CFG_REG_OFF       		0x0008
#define PLL_PERIPH1_CFG_REG_OFF     		0x000C
#define PLL_VE_CFG_REG_OFF          		0x0010
#define PLL_DDR_CFG_REG_OFF         		0x0014
#define PLL_VIDEO1_CFG_REG_OFF      		0x0018
#define PLL_VIDEO2_CFG_REG_OFF      		0x001C
#define PLL_GPU_CFG_REG_OFF         		0x0020
#define PLL_DE_CFG_REG_OFF          		0x0024
#define PLL_ISP_CFG_REG_OFF         		0x0028
#define PLL_PERIPH2_CFG_REG_OFF     		0x002C
#define CPU_CLK_SRC_REG_OFF         		0x0050
#define AXI0_CFG_REG_OFF            		0x0054
#define AXI1_CFG_REG_OFF            		0x0058
#define GTCLK_CFG_REG_OFF           		0x005C
#define AHB0_CFG_REG_OFF            		0x0060
#define AHB1_CFG_REG_OFF            		0x0064
#define AHB2_CFG_REG_OFF            		0x0068
#define APB0_CFG_REG_OFF            		0x0070
#define APB1_CFG_REG_OFF            		0x0074
#define CCI400_CFG_REG_OFF          		0x0078
#define ATS_CLK_REG_OFF             		0x0080
#define TRACE_CLK_REG_OFF           		0x0084
#define PLL_LOCK_CFG_REG0_OFF       		0x0090
#define PLL_LOCK_CFG_REG1_OFF       		0x0094
#define PLL_LOCK_STATUS_REG_OFF     		0x009C
#define PLL_C0_BIAS_REG_OFF         		0x00A0
#define PLL_C1_BIAS_REG_OFF         		0x00A4
#define PLL_AUDIO_BIAS_REG_OFF      		0x00A8
#define PLL_PERIPH1_BIAS_REG_OFF    		0x00AC
#define PLL_VE_BIAS_REG_OFF         		0x00B0
#define PLL_DDR_BIAS_REG_OFF        		0x00B4
#define PLL_VIDEO1_BIAS_REG_OFF     		0x00B8
#define PLL_VIDEO2_BIAS_REG_OFF     		0x00BC
#define PLL_GPU_BIAS_REG_OFF        		0x00C0
#define PLL_DE_BIAS_REG_OFF         		0x00C4
#define PLL_ISP_BIAS_REG_OFF        		0x00C8
#define PLL_PERIPH2_BIAS_REG_OFF    		0x00CC
#define PLL_C0_TUN_REG_OFF          		0x00E0
#define PLL_C1_TUN_REG_OFF          		0x00E4
#define PLL_AUDIO_PAT_CFG_REG_OFF   		0x0108
#define PLL_PERIPH1_PAT_CFG_REG_OFF 		0x010C
#define PLL_VE_PAT_CFG_REG_OFF      		0x0110
#define PLL_DDR_PAT_CFG_REG_OFF     		0x0114
#define PLL_VIDEO1_PAT_CFG_REG_OFF  		0x0118
#define PLL_VIDEO2_PAT_CFG_REG_OFF  		0x011C
#define PLL_GPU_PAT_CFG_REG_OFF     		0x0120
#define PLL_DE_PAT_CFG_REG_OFF      		0x0124
#define PLL_ISP_PAT_CFG_REG_OFF     		0x0128
#define PLL_PERIPH2_PAT_CFG_REG_OFF 		0x012C
#define CLK_OUTA_REG_OFF            		0x0180
#define CLK_OUTB_REG_OFF            		0x0184

//
// Register Address
//
#define PLL_C0_CFG_REG_ADDR         		( CCM_VBASE + PLL_C0_CFG_REG_OFF          )		// 
#define PLL_C1_CFG_REG_ADDR         		( CCM_VBASE + PLL_C1_CFG_REG_OFF          )		// 
#define PLL_AUDIO_CFG_REG_ADDR      		( CCM_VBASE + PLL_AUDIO_CFG_REG_OFF       )		// 
#define PLL_PERIPH1_CFG_REG_ADDR    		( CCM_VBASE + PLL_PERIPH1_CFG_REG_OFF     )		// 
#define PLL_VE_CFG_REG_ADDR         		( CCM_VBASE + PLL_VE_CFG_REG_OFF          )		// 
#define PLL_DDR_CFG_REG_ADDR        		( CCM_VBASE + PLL_DDR_CFG_REG_OFF         )		// 
#define PLL_VIDEO1_CFG_REG_ADDR     		( CCM_VBASE + PLL_VIDEO1_CFG_REG_OFF      )		// 
#define PLL_VIDEO2_CFG_REG_ADDR     		( CCM_VBASE + PLL_VIDEO2_CFG_REG_OFF      )		// 
#define PLL_GPU_CFG_REG_ADDR        		( CCM_VBASE + PLL_GPU_CFG_REG_OFF         )		// 
#define PLL_DE_CFG_REG_ADDR         		( CCM_VBASE + PLL_DE_CFG_REG_OFF          )		// 
#define PLL_ISP_CFG_REG_ADDR        		( CCM_VBASE + PLL_ISP_CFG_REG_OFF         )		// 
#define PLL_PERIPH2_CFG_REG_ADDR    		( CCM_VBASE + PLL_PERIPH2_CFG_REG_OFF     )		// 
#define CPU_CLK_SRC_REG_ADDR        		( CCM_VBASE + CPU_CLK_SRC_REG_OFF         )		// CPU CLK ratio register
#define AXI0_CFG_REG_ADDR           		( CCM_VBASE + AXI0_CFG_REG_OFF            )		// AXI0 CLK ratio register
#define AXI1_CFG_REG_ADDR           		( CCM_VBASE + AXI1_CFG_REG_OFF            )		// AXI1 CLK ratio register
#define GTCLK_CFG_REG_ADDR          		( CCM_VBASE + GTCLK_CFG_REG_OFF           )		// GTBUS CLK control register
#define AHB0_CFG_REG_ADDR           		( CCM_VBASE + AHB0_CFG_REG_OFF            )		// AHB0 CLK ratio register
#define AHB1_CFG_REG_ADDR           		( CCM_VBASE + AHB1_CFG_REG_OFF            )		// AHB1 CLK ratio register
#define AHB2_CFG_REG_ADDR           		( CCM_VBASE + AHB2_CFG_REG_OFF            )		// AHB2 CLK ratio register
#define APB0_CFG_REG_ADDR           		( CCM_VBASE + APB0_CFG_REG_OFF            )		// APB0 clock divider register
#define APB1_CFG_REG_ADDR           		( CCM_VBASE + APB1_CFG_REG_OFF            )		// APB1 clock divider register
#define CCI400_CFG_REG_ADDR         		( CCM_VBASE + CCI400_CFG_REG_OFF          )		// CCI-400 clock divider register
#define ATS_CLK_REG_ADDR            		( CCM_VBASE + ATS_CLK_REG_OFF             )		// ATS CLK ratio register
#define TRACE_CLK_REG_ADDR          		( CCM_VBASE + TRACE_CLK_REG_OFF           )		// TRACE CLK ratio register
#define PLL_LOCK_CFG_REG0_ADDR      		( CCM_VBASE + PLL_LOCK_CFG_REG0_OFF       )		// PLL(except PLLC0/C1) lock time control register
#define PLL_LOCK_CFG_REG1_ADDR      		( CCM_VBASE + PLL_LOCK_CFG_REG1_OFF       )		// PLLC0 and PLLC1 lock time control register
#define PLL_LOCK_STATUS_REG_ADDR    		( CCM_VBASE + PLL_LOCK_STATUS_REG_OFF     )		// PLL lock status register
#define PLL_C0_BIAS_REG_ADDR        		( CCM_VBASE + PLL_C0_BIAS_REG_OFF         )		// PLLC0 Bias register
#define PLL_C1_BIAS_REG_ADDR        		( CCM_VBASE + PLL_C1_BIAS_REG_OFF         )		// PLLC1 Bias register
#define PLL_AUDIO_BIAS_REG_ADDR     		( CCM_VBASE + PLL_AUDIO_BIAS_REG_OFF      )		// PLL_AUDIO Bias register
#define PLL_PERIPH1_BIAS_REG_ADDR   		( CCM_VBASE + PLL_PERIPH1_BIAS_REG_OFF    )		// PLL_PERIPH1 Bias register
#define PLL_VE_BIAS_REG_ADDR        		( CCM_VBASE + PLL_VE_BIAS_REG_OFF         )		// PLL_VE Bias register
#define PLL_DDR_BIAS_REG_ADDR       		( CCM_VBASE + PLL_DDR_BIAS_REG_OFF        )		// PLL_DDR Bias register
#define PLL_VIDEO1_BIAS_REG_ADDR    		( CCM_VBASE + PLL_VIDEO1_BIAS_REG_OFF     )		// PLL_VIDEO1 Bias register
#define PLL_VIDEO2_BIAS_REG_ADDR    		( CCM_VBASE + PLL_VIDEO2_BIAS_REG_OFF     )		// PLL_VIDEO2 Bias register
#define PLL_GPU_BIAS_REG_ADDR       		( CCM_VBASE + PLL_GPU_BIAS_REG_OFF        )		// PLL_GPU Bias register
#define PLL_DE_BIAS_REG_ADDR        		( CCM_VBASE + PLL_DE_BIAS_REG_OFF         )		// PLL_DE Bias register
#define PLL_ISP_BIAS_REG_ADDR       		( CCM_VBASE + PLL_ISP_BIAS_REG_OFF        )		// PLL_ISP Bias register
#define PLL_PERIPH2_BIAS_REG_ADDR   		( CCM_VBASE + PLL_PERIPH2_BIAS_REG_OFF    )		// PLL_PERIPH2 Bias register
#define PLL_C0_TUN_REG_ADDR         		( CCM_VBASE + PLL_C0_TUN_REG_OFF          )		// PLL-C0 tuning register
#define PLL_C1_TUN_REG_ADDR         		( CCM_VBASE + PLL_C1_TUN_REG_OFF          )		// PLL-C1 tuning register
#define PLL_AUDIO_PAT_CFG_REG_ADDR  		( CCM_VBASE + PLL_AUDIO_PAT_CFG_REG_OFF   )		// PLL_AUDIO-PATTERN Control Register
#define PLL_PERIPH1_PAT_CFG_REG_ADDR		( CCM_VBASE + PLL_PERIPH1_PAT_CFG_REG_OFF )		// PLL_PERIPH1-PATTERN Control Register
#define PLL_VE_PAT_CFG_REG_ADDR     		( CCM_VBASE + PLL_VE_PAT_CFG_REG_OFF      )		// PLL_VE-PATTERN Control Register
#define PLL_DDR_PAT_CFG_REG_ADDR    		( CCM_VBASE + PLL_DDR_PAT_CFG_REG_OFF     )		// PLL_DDR-PATTERN Control Register
#define PLL_VIDEO1_PAT_CFG_REG_ADDR 		( CCM_VBASE + PLL_VIDEO1_PAT_CFG_REG_OFF  )		// PLL_VIDEO1-PATTERN Control Register
#define PLL_VIDEO2_PAT_CFG_REG_ADDR 		( CCM_VBASE + PLL_VIDEO2_PAT_CFG_REG_OFF  )		// PLL_VIDEO2-PATTERN Control Register
#define PLL_GPU_PAT_CFG_REG_ADDR    		( CCM_VBASE + PLL_GPU_PAT_CFG_REG_OFF     )		// PLL_GPU-PATTERN Control Register
#define PLL_DE_PAT_CFG_REG_ADDR     		( CCM_VBASE + PLL_DE_PAT_CFG_REG_OFF      )		// PLL_DE-PATTERN Control Register
#define PLL_ISP_PAT_CFG_REG_ADDR    		( CCM_VBASE + PLL_ISP_PAT_CFG_REG_OFF     )		// PLL_ISP-PATTERN Control Register
#define PLL_PERIPH2_PAT_CFG_REG_ADDR		( CCM_VBASE + PLL_PERIPH2_PAT_CFG_REG_OFF )		// PLL_PERIPH2-PATTERN Control Register
#define CLK_OUTA_REG_ADDR           		( CCM_VBASE + CLK_OUTA_REG_OFF            )		// CLK OUTA Control Register
#define CLK_OUTB_REG_ADDR           		( CCM_VBASE + CLK_OUTB_REG_OFF            )		// CLK OUTB Control Register

// 
// Detail information of registers
//

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 pll_postdiv_m        :  2 ;    // Default: 0x0; 
		__u32 res0                 :  6 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x11; 
		__u32 pll_out_ext_divp     :  1 ;    // Default: 0x0; 
		__u32 res1                 :  7 ;    // Default: ; 
		__u32 pll_lock_time        :  3 ;    // Default: 0x2; 
		__u32 res2                 :  4 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_C0_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 pll_postdiv_m        :  2 ;    // Default: 0x0; 
		__u32 res0                 :  6 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x11; 
		__u32 pll_out_ext_divp     :  1 ;    // Default: 0x0; 
		__u32 res1                 :  7 ;    // Default: ; 
		__u32 pll_lock_time        :  3 ;    // Default: 0x2; 
		__u32 res2                 :  4 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_C1_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 pll_postdiv_p        :  6 ;    // Default: 0x14; 
		__u32 res0                 :  2 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x2B; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x1; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_AUDIO_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x28; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x0; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_PERIPH1_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x24; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x1; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_VE_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x24; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x1; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  5 ;    // Default: ; 
		__u32 sdrpll_upd           :  1 ;    // Default: 0x0; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_DDR_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x63; 
		__u32 pll_div              :  1 ;    // Default: 0x1; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x0; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_VIDEO1_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 pll_out_ext_divp     :  2 ;    // Default: 0x0; 
		__u32 res0                 :  6 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x63; 
		__u32 pll_div              :  1 ;    // Default: 0x1; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x0; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_VIDEO2_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x24; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x1; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_GPU_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x24; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x1; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_DE_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x24; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x1; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_ISP_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 pll_factor_n         :  8 ;    // Default: 0x24; 
		__u32 pll_div1             :  1 ;    // Default: 0x0; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 pll_div2             :  1 ;    // Default: 0x1; 
		__u32 res2                 :  5 ;    // Default: ; 
		__u32 pll_sdm_en           :  1 ;    // Default: 0x0; 
		__u32 res3                 :  6 ;    // Default: ; 
		__u32 pll_enable           :  1 ;    // Default: 0x0; 
	} bits;
} PLL_PERIPH2_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 cpu_c0_clk_src_sel   :  1 ;    // Default: 0x0; 
		__u32 res0                 :  7 ;    // Default: ; 
		__u32 cpu_c1_clk_src_sel   :  1 ;    // Default: 0x0; 
		__u32 res1                 : 23 ;    // Default: ; 
	} bits;
} CPU_CLK_SRC_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 axi0_clk_div_ratio   :  3 ;    // Default: 0x0; 
		__u32 res0                 :  5 ;    // Default: ; 
		__u32 atb0_apb_clk_div     :  2 ;    // Default: 0x1; 
		__u32 res1                 :  6 ;    // Default: ; 
	} bits;
} AXI0_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 axi1_clk_div_ratio   :  3 ;    // Default: 0x0; 
		__u32 res0                 :  5 ;    // Default: ; 
		__u32 atb1_apb_clk_div     :  2 ;    // Default: 0x1; 
		__u32 res1                 :  6 ;    // Default: ; 
	} bits;
} AXI1_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 gtclk_clk_div_ratio  :  2 ;    // Default: 0x0; 
		__u32 res0                 : 22 ;    // Default: ; 
		__u32 gtclk_src_sel        :  2 ;    // Default: 0x0; 
		__u32 res1                 :  6 ;    // Default: ; 
	} bits;
} GTCLK_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 ahb0_clk_div_ratio   :  2 ;    // Default: 0x0; 
		__u32 res0                 : 22 ;    // Default: ; 
		__u32 ahb0_clk_src_sel     :  2 ;    // Default: 0x0; 
		__u32 res1                 :  6 ;    // Default: ; 
	} bits;
} AHB0_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 ahb1_clk_div_ratio   :  2 ;    // Default: 0x0; 
		__u32 res0                 : 22 ;    // Default: ; 
		__u32 ahb1_clk_src_sel     :  2 ;    // Default: 0x0; 
		__u32 res1                 :  6 ;    // Default: ; 
	} bits;
} AHB1_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 ahb2_clk_div_ratio   :  2 ;    // Default: 0x0; 
		__u32 res0                 : 22 ;    // Default: ; 
		__u32 ahb2_clk_src_sel     :  2 ;    // Default: 0x0; 
		__u32 res1                 :  6 ;    // Default: ; 
	} bits;
} AHB2_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 apb0_clk_div_ratio   :  2 ;    // Default: 0x0; 
		__u32 res0                 : 22 ;    // Default: ; 
		__u32 apb0_clk_src_sel     :  1 ;    // Default: 0x0; 
		__u32 res1                 :  7 ;    // Default: ; 
	} bits;
} APB0_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 clk_rat_m            :  5 ;    // Default: 0x0; 
		__u32 res0                 : 11 ;    // Default: ; 
		__u32 clk_rat_n            :  2 ;    // Default: 0x0; 
		__u32 res1                 :  6 ;    // Default: ; 
		__u32 apb1_clk_src_sel     :  1 ;    // Default: 0x0; 
		__u32 res2                 :  7 ;    // Default: ; 
	} bits;
} APB1_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 cci400_clk_div_ratio :  2 ;    // Default: 0x0; 
		__u32 res0                 : 22 ;    // Default: ; 
		__u32 cci400_clk_src_sel   :  2 ;    // Default: 0x0; 
		__u32 res1                 :  6 ;    // Default: ; 
	} bits;
} CCI400_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 clk_div_ratio_m      :  3 ;    // Default: 0x0; 
		__u32 res0                 : 21 ;    // Default: ; 
		__u32 clk_src_sel          :  2 ;    // Default: 0x0; 
		__u32 res1                 :  5 ;    // Default: ; 
		__u32 sclk_gating          :  1 ;    // Default: 0x1; 
	} bits;
} ATS_CLK_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 clk_div_ratio_m      :  3 ;    // Default: 0x0; 
		__u32 res0                 : 21 ;    // Default: ; 
		__u32 clk_src_sel          :  2 ;    // Default: 0x0; 
		__u32 res1                 :  5 ;    // Default: ; 
		__u32 sclk_gating          :  1 ;    // Default: 0x1; 
	} bits;
} TRACE_CLK_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 pll_lock_time        : 16 ;    // Default: 0x00FF; 
		__u32 res0                 : 16 ;    // Default: ; 
	} bits;
} PLL_LOCK_CFG_REG0_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 pll_lock_time        : 16 ;    // Default: 0x00FF; 
		__u32 res0                 : 16 ;    // Default: ; 
	} bits;
} PLL_LOCK_CFG_REG1_t;

typedef union
{
	__u32 dwval;
} PLL_LOCK_STATUS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_cur         :  5 ;    // Default: 0x10; 
		__u32 res1                 : 10 ;    // Default: ; 
		__u32 vco_rst              :  1 ;    // Default: 0x1; 
	} bits;
} PLL_C0_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_cur         :  5 ;    // Default: 0x10; 
		__u32 res1                 : 10 ;    // Default: ; 
		__u32 vco_rst              :  1 ;    // Default: 0x1; 
	} bits;
} PLL_C1_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_AUDIO_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_PERIPH1_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_VE_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_cp_ctrl          :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_DDR_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x10; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_VIDEO1_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x10; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_VIDEO2_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_GPU_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_DE_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_ISP_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 : 16 ;    // Default: ; 
		__u32 pll_bias_ctrl        :  5 ;    // Default: 0x8; 
		__u32 res1                 : 11 ;    // Default: ; 
	} bits;
} PLL_PERIPH2_BIAS_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 c_b_out              :  7 ;    // Default: 0x0; 
		__u32 c_od1                :  1 ;    // Default: 0x0; 
		__u32 c_b_in               :  7 ;    // Default: 0x40; 
		__u32 c_od                 :  1 ;    // Default: 0x0; 
		__u32 cnt_int_ctrl         :  7 ;    // Default: 0x40; 
		__u32 res0                 :  1 ;    // Default: ; 
		__u32 vco_gain_ctrl        :  3 ;    // Default: 0x4; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 vco_rng_ctrl         :  3 ;    // Default: 0x4; 
		__u32 res2                 :  1 ;    // Default: ; 
	} bits;
} PLL_C0_TUN_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 c_b_out              :  7 ;    // Default: 0x0; 
		__u32 c_od1                :  1 ;    // Default: 0x0; 
		__u32 c_b_in               :  7 ;    // Default: 0x40; 
		__u32 c_od                 :  1 ;    // Default: 0x0; 
		__u32 cnt_int_ctrl         :  7 ;    // Default: 0x40; 
		__u32 res0                 :  1 ;    // Default: ; 
		__u32 vco_gain_ctrl        :  3 ;    // Default: 0x4; 
		__u32 res1                 :  1 ;    // Default: ; 
		__u32 vco_rng_ctrl         :  3 ;    // Default: 0x4; 
		__u32 res2                 :  1 ;    // Default: ; 
	} bits;
} PLL_C1_TUN_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_AUDIO_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_PERIPH1_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_VE_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_DDR_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_VIDEO1_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_VIDEO2_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_GPU_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_DE_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_ISP_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 wave_bot             : 17 ;    // Default: 0x0; 
		__u32 freq                 :  2 ;    // Default: 0x0; 
		__u32 clk_src_sel          :  1 ;    // Default: 0x0; 
		__u32 wave_step            :  9 ;    // Default: 0x0; 
		__u32 spr_freq_mode        :  2 ;    // Default: 0x0; 
		__u32 sig_delt_pat_en      :  1 ;    // Default: 0x0; 
	} bits;
} PLL_PERIPH2_PAT_CFG_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 divider_m            :  5 ;    // Default: 0x0; 
		__u32 res1                 :  7 ;    // Default: ; 
		__u32 divider_n            :  2 ;    // Default: 0x0; 
		__u32 res2                 :  2 ;    // Default: ; 
		__u32 clk_out_src_sel      :  2 ;    // Default: 0x0; 
		__u32 res3                 :  5 ;    // Default: ; 
		__u32 clk_out_en           :  1 ;    // Default: 0x0; 
	} bits;
} CLK_OUTA_REG_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                 :  8 ;    // Default: ; 
		__u32 divider_m            :  5 ;    // Default: 0x0; 
		__u32 res1                 :  7 ;    // Default: ; 
		__u32 divider_n            :  2 ;    // Default: 0x0; 
		__u32 res2                 :  2 ;    // Default: ; 
		__u32 clk_out_src_sel      :  2 ;    // Default: 0x0; 
		__u32 res3                 :  5 ;    // Default: ; 
		__u32 clk_out_en           :  1 ;    // Default: 0x0; 
	} bits;
} CLK_OUTB_REG_t;

typedef struct __CCMU_REG_LIST
{
	//pll1 - pll12
	volatile PLL_C0_CFG_REG_t              Pll_C0_Cfg;    	        //0x0000
	volatile PLL_C1_CFG_REG_t              Pll_C1_Cfg;    	        //0x0004
	volatile PLL_AUDIO_CFG_REG_t           Pll_Audio_Cfg;    	//0x0008
	volatile PLL_PERIPH1_CFG_REG_t         Pll_Periph1_Cfg;    	//0x000c
	volatile PLL_VE_CFG_REG_t              Pll_Ve_Cfg;    	        //0x0010
	volatile PLL_DDR_CFG_REG_t             Pll_Ddr_Cfg;    	        //0x0014
	volatile PLL_VIDEO1_CFG_REG_t          Pll_Video1_Cfg;    	//0x0018
	volatile PLL_VIDEO2_CFG_REG_t          Pll_Video2_Cfg;    	//0x001c
	volatile PLL_GPU_CFG_REG_t             Pll_Gpu_Cfg;    	        //0x0020
	volatile PLL_DE_CFG_REG_t              Pll_De_Cfg;    	        //0x0024
	volatile PLL_ISP_CFG_REG_t             Pll_Isp_Cfg;    	        //0x0028
	volatile PLL_PERIPH2_CFG_REG_t         Pll_Periph2_Cfg;    	//0x002c
	//
	volatile __u32                          reserved0[8];           //0x30-0x4c, reserved
	volatile CPU_CLK_SRC_REG_t              Cpu_Clk_Src;            //0x0050
	volatile AXI0_CFG_REG_t                 Axi0_Cfg;	        //0x0054
	volatile AXI1_CFG_REG_t                 Axi1_Cfg;	        //0x0058
	volatile GTCLK_CFG_REG_t                Gtclk_Cfg;	        //0x005c
	volatile AHB0_CFG_REG_t                 Ahb0_Cfg;	        //0x0060
	volatile AHB1_CFG_REG_t                 Ahb1_Cfg;	        //0x0064
	volatile AHB2_CFG_REG_t                 Ahb2_Cfg;	        //0x0068
	volatile __u32			        reserved1;	        //0x6c
	volatile APB0_CFG_REG_t                 Apb0_Cfg;	        //0x0070
	volatile APB1_CFG_REG_t      	        Apb1_Cfg;	        //0x0074
	volatile CCI400_CFG_REG_t               Cci400_Cfg;             //0x0078
	volatile __u32			        reserved2;	        //0x7c
	volatile ATS_CLK_REG_t        		Ats_Clk;                //0x0080
	volatile TRACE_CLK_REG_t      		Trace_Clk;              //0x0084
	volatile __u32			        reserved3[2];	        //0x88-0x8c
	volatile PLL_LOCK_CFG_REG0_t  		Pll_Lock_Cfg;           //0x0090
	volatile PLL_LOCK_CFG_REG1_t            Pll_Lock_Cfg_1;         //0x0094
	volatile __u32			        reserved4;	        //0x98
	volatile PLL_LOCK_STATUS_REG_t          Pll_Lock_Status;        //0x009c
	//
	volatile PLL_C0_BIAS_REG_t              Pll_C0_Bias;            //0x00a0
	volatile PLL_C1_BIAS_REG_t              Pll_C1_Bias;            //0x00a4
	volatile PLL_AUDIO_BIAS_REG_t           Pll_Audio_Bias;         //0x00a8
	volatile PLL_PERIPH1_BIAS_REG_t         Pll_Periph1_Bias;       //0x00ac
	volatile PLL_VE_BIAS_REG_t              Pll_Ve_Bias;            //0x00b0
	volatile PLL_DDR_BIAS_REG_t             Pll_Ddr_Bias;           //0x00b4
	volatile PLL_VIDEO1_BIAS_REG_t          Pll_Vedio1_Bias;        //0x00b8
	volatile PLL_VIDEO2_BIAS_REG_t          Pll_Vedio2_Bias;        //0x00bc
	volatile PLL_GPU_BIAS_REG_t             Pll_Gpu_Bias;           //0x00c0
	volatile PLL_DE_BIAS_REG_t              Pll_De_Bias;            //0x00c4
	volatile PLL_ISP_BIAS_REG_t             Pll_Isp_Bias;           //0x00c8
	volatile PLL_PERIPH2_BIAS_REG_t         Pll_Periph2_Bias;       //0x00cc
	volatile __u32			        reserved5[4];	        //0xd0-0xdc
	volatile PLL_C0_TUN_REG_t               Pll_C0_Tun;             //0x00e0 
	volatile PLL_C1_TUN_REG_t               Pll_C1_Tun;             //0x00e4
	volatile __u32			        reserved6[7];	        //0xe8-0x104
	volatile PLL_AUDIO_PAT_CFG_REG_t        Pll_Audio_Pat_Cfg;      //0x0108
	volatile PLL_PERIPH1_PAT_CFG_REG_t      Pll_Periph1_Pat_Cfg;    //0x010c
	volatile PLL_VE_PAT_CFG_REG_t           Pll_Ve_Pat_Cfg;         //0x0110
	volatile PLL_DDR_PAT_CFG_REG_t          Pll_Ddr_Pat_Cfg;        //0x0114
	volatile PLL_VIDEO1_PAT_CFG_REG_t       Pll_Video1_Pat_Cfg;     //0x0118
	volatile PLL_VIDEO2_PAT_CFG_REG_t       Pll_Video2_Pat_Cfg;     //0x011c
	volatile PLL_GPU_PAT_CFG_REG_t          Pll_Gpu_Pat_Cfg;        //0x0120
	volatile PLL_DE_PAT_CFG_REG_t           Pll_De_Pat_Cfg;         //0x0124
	volatile PLL_ISP_PAT_CFG_REG_t          Pll_Isp_Pat_Cfg;        //0x0128
	volatile PLL_PERIPH2_PAT_CFG_REG_t      Pll_Periph2_Pat_Cfg;    //0x012c
	volatile __u32			        reserved7[20];	        //0x130-0x17c
	volatile CLK_OUTA_REG_t                 Clk_Outa;  	        //0x0180
	volatile CLK_OUTB_REG_t         	Clk_Outb;  	        //0x0184
} __ccmu_reg_list_t;

// 
// Detail information of registers
//
typedef struct __CCMU_MOD_REG_LIST{
        volatile __u32 nand0_sclk0_cfg; 		//0x0000
        volatile __u32 nand0_sclk1_cfg;                 //0x0004
        volatile __u32 nand1_sclk0_cfg; 		//0x0008
        volatile __u32 nand1_sclk1_cfg; 		//0x000C
        volatile __u32 sd0_clk;         		//0x0010
        volatile __u32 sd1_clk;         		//0x0014
        volatile __u32 sd2_clk;         		//0x0018
        volatile __u32 sd3_clk;         		//0x001C
        volatile __u32 reserved0[2];                    //0x0020-0x24
        volatile __u32 ts_clk;          		//0x0028
        volatile __u32 ss_clk;          		//0x002C
        volatile __u32 spi0_clk;          		//0x0030
        volatile __u32 spi1_clk;        		//0x0034
        volatile __u32 spi2_clk;        		//0x0038
        volatile __u32 spi3_clk;        		//0x003C
        volatile __u32 daudio0_clk;     		//0x0040
        volatile __u32 daudio1_clk;     		//0x0044
        volatile __u32 reserved1;                       //0x0048, reserved
        volatile __u32 spdif_clk;                       //0x004C
        volatile __u32 usbphy0_cfg;                     //0x0050
        volatile __u32 reserved2[11];                   //0x0054-0x7c, reserved
        volatile __u32 mdfs_clk;                        //0x0080
        volatile __u32 dram_cfg;                        //0x0084
        volatile __u32 reserved3[2];                    //0x0088-0x8c, reserved
        volatile __u32 de_sclk_cfg;                     //0x0090
        volatile __u32 edp_sclk_cfg;    		//0x0094
        volatile __u32 mp_clk;          		//0x0098
        volatile __u32 lcd0_clk;                	//0x009c	
        volatile __u32 lcd1_clk;                	//0x00a0
        volatile __u32 reserved4;                       //0x00a4, reserved
        volatile __u32 mipi_dsi_clk0;   		//0x00A8
        volatile __u32 mipi_dsi_clk1;   		//0x00AC
        volatile __u32 hdmi_sclk;              		//0x00B0
        volatile __u32 hdmi_slow_clk0;  		//0x00B4
        volatile __u32 reserved5;                       //0x00b8, reserved
        volatile __u32 mipi_csi_cfg;    		//0x00BC
        volatile __u32 csi_isp_clk;     		//0x00C0
        volatile __u32 csi0_mclk;       		//0x00C4
        volatile __u32 csi1_mclk;       		//0x00C8
        volatile __u32 fd_clk;          		//0x00CC
        volatile __u32 ve_clk;          		//0x00D0
        volatile __u32 avs_clk;         		//0x00D4
        volatile __u32 reserved6[6];                    //0x00d8 - 0xec, reserved
        volatile __u32 gpu_core_clk;    		//0x00F0
        volatile __u32 gpu_mem_clk;     		//0x00F4
        volatile __u32 gpu_axi_clk;      		//0x00F8
        volatile __u32 reserved7;                       //0x00fc, reserved
        volatile __u32 sata_clk;                        //0x0100
        volatile __u32 ac97_clk;        		//0x0104
        volatile __u32 mipi_hsi_clk;    		//0x0108
        volatile __u32 gp_adc;          		//0x010C
        volatile __u32 cir_tx_clk;      		//0x0110
        volatile __u32 reserved8[27];                   //0x0114-0x17c, reserved
        volatile __u32 ahb0_gating;     		//0x0180
        volatile __u32 ahb1_gating;         		//0x0184
        volatile __u32 ahb2_gating;     		//0x0188
        volatile __u32 reserved9;                       //0x018c, reserved
        volatile __u32 apb0_gating;     		//0x0190
        volatile __u32 apb1_gating;     		//0x0194
        volatile __u32 reserved10[2];                   //0x0198, 0x19c, reserved
        volatile __u32 ahb0_rst;               		//0x01A0
        volatile __u32 ahb1_rst;        		//0x01A4
        volatile __u32 ahb2_rst;        		//0x01A8
        volatile __u32 reserved11;                      //0x01ac, reserved
        volatile __u32 apb0_rst;        		//0x01B0
        volatile __u32 apb1_rst;               		//0x01B4
}__ccmu_mod_reg_list_t; 



#endif  // #ifndef __MEM_CCMU_SUN9IW1P1_H__
