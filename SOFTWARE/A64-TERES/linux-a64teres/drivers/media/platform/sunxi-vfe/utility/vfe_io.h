/* 
 ***************************************************************************************
 * 
 * vfe_io.h
 * 
 * Hawkview ISP - vfe_io.h module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   2.0		  Yang Feng   	2014/07/14	      Second Version
 * 
 ****************************************************************************************
 */
 
#ifndef _VFE_IO_H_
#define _VFE_IO_H_
 
#include <linux/io.h>
#include <linux/types.h>
static inline
u32 vfe_reg_readl(volatile void __iomem * addr)
{
	return readl(addr);
}

static inline
void vfe_reg_writel(volatile void __iomem *addr, u32 reg_value)
{
	writel(reg_value, addr);
}

static inline
void vfe_reg_clr( volatile void __iomem * reg, u32 clr_bits)
{
	u32 v = vfe_reg_readl(reg);
	vfe_reg_writel(reg, v & ~clr_bits);
}

static inline
void vfe_reg_set(volatile void __iomem * reg, u32 set_bits)
{
	u32 v = vfe_reg_readl(reg);
	vfe_reg_writel(reg, v | set_bits);
}

//clr_bits for mask
static inline
void vfe_reg_clr_set(volatile void __iomem * reg, u32 clr_bits, u32 set_bits)
{
	u32 v = vfe_reg_readl(reg);
	vfe_reg_writel(reg, (v & ~clr_bits) | (set_bits & clr_bits));
}

#endif /*_VFE_IO_H_*/


