
/*
 ***************************************************************************************
 * 
 * platform_cfg.h
 * 
 * Hawkview ISP - platform_cfg.h module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 *
 *   2.0		  Yang Feng   	2014/07/24	      Second Version
 * 
 ****************************************************************************************
 */

#ifndef __PLATFORM_CFG__H__
#define __PLATFORM_CFG__H__

//#define FPGA_VER
#define SUNXI_MEM

#ifdef FPGA_VER
#define FPGA_PIN
#else
#define VFE_CLK
#define VFE_GPIO
#define VFE_PMU
#endif

#include <linux/gpio.h>

#ifdef VFE_CLK
#include <linux/clk.h>
#include <linux/clk/sunxi.h>
#include <linux/clk-private.h>
#endif

#ifdef VFE_GPIO
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#endif

#ifdef VFE_PMU
#include <linux/regulator/consumer.h>
#endif

#include <linux/sys_config.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include <linux/slab.h>

#ifdef FPGA_VER
#define DPHY_CLK (48*1000*1000)
#else
#define DPHY_CLK (150*1000*1000)
#endif

#if defined CONFIG_ARCH_SUN50I
#include "platform/sun50iw1p1_vfe_cfg.h"
#define SUNXI_PLATFORM_ID ISP_PLATFORM_SUN50IW1P1
#elif defined CONFIG_ARCH_SUN8IW10P1
#include "platform/sun8iw10p1_vfe_cfg.h"
#define SUNXI_PLATFORM_ID ISP_PLATFORM_SUN8IW10P1
#elif defined CONFIG_ARCH_SUN8IW11P1
#include "platform/sun8iw11p1_vfe_cfg.h"
#define SUNXI_PLATFORM_ID ISP_PLATFORM_SUN8IW11P1
#endif

#define ISP_LUT_MEM_OFS             0x0
#define ISP_LENS_MEM_OFS            (ISP_LUT_MEM_OFS + ISP_LUT_MEM_SIZE)
#define ISP_GAMMA_MEM_OFS           (ISP_LENS_MEM_OFS + ISP_LENS_MEM_SIZE)
#define ISP_LINEAR_MEM_OFS           (ISP_GAMMA_MEM_OFS + ISP_GAMMA_MEM_SIZE)

#define ISP_DRC_MEM_OFS            0x0
#define ISP_DISC_MEM_OFS          (ISP_DRC_MEM_OFS + ISP_DRC_MEM_SIZE)


#define VFE_CORE_CLK_RATE (300*1000*1000)
#define VFE_CLK_NOT_EXIST	NULL

enum{
	VFE_CORE_CLK = 0,
	VFE_MASTER_CLK,
	VFE_MISC_CLK,
	VFE_MIPI_CSI_CLK,		
	VFE_MIPI_DPHY_CLK,
	CLK_NUM,
};

enum{
	VFE_CORE_CLK_SRC = 0,	
	VFE_MASTER_CLK_24M_SRC,
	VFE_MASTER_CLK_PLL_SRC,		
	VFE_MIPI_CSI_CLK_SRC,
	VFE_MIPI_DPHY_CLK_SRC,
	CLK_SRC_NUM,
};

#define NOCLK 			-1

#endif //__PLATFORM_CFG__H__
