
/* 
 ***************************************************************************************
 * 
 * vfe_resource.h
 * 
 * Hawkview ISP - vfe_resource.h module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   2.0		  Yang Feng   	2014/07/24	      Second Version
 * 
 ****************************************************************************************
 */

 
#ifndef _VFE_RESOURCE_H_
#define _VFE_RESOURCE_H_


#if defined CONFIG_ARCH_SUN8IW6P1

static struct resource vfe_vip0_resource[] = {
	[0] = {
		.name		= "csi",
		.start  = CSI0_REGS_BASE,
		.end    = CSI0_REGS_BASE + CSI0_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	}, 
	[1] = {
		.start  = SUNXI_IRQ_CSI,
		.end    = SUNXI_IRQ_CSI,
		.flags  = IORESOURCE_IRQ,
	}, 
	[2] = {
		.name		= "isp",
		.start  = ISP_REGS_BASE,
		.end    = ISP_REGS_BASE + ISP_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct resource vfe_vip1_resource[] = {

};

#elif defined CONFIG_ARCH_SUN8IW7P1

static struct resource vfe_vip0_resource[] = {
	[0] = {
		.name		= "csi",
		.start  = CSI0_REGS_BASE,
		.end    = CSI0_REGS_BASE + CSI0_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	}, 
	[1] = {
		.start  = SUNXI_IRQ_CSI,
		.end    = SUNXI_IRQ_CSI,
		.flags  = IORESOURCE_IRQ,
	}, 
	[2] = {
		.name		= "isp",
		.start  = ISP_REGS_BASE,
		.end    = ISP_REGS_BASE + ISP_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},

};

static struct resource vfe_vip1_resource[] = {

};

#elif defined CONFIG_ARCH_SUN8IW8P1

static struct resource vfe_vip0_resource[] = {
	[0] = {
		.name		= "csi",
		.start  = CSI0_REGS_BASE,
		.end    = CSI0_REGS_BASE + CSI0_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	}, 
	[1] = {
		.start  = SUNXI_IRQ_CSI0,
		.end    = SUNXI_IRQ_CSI0,
		.flags  = IORESOURCE_IRQ,
	}, 
	[2] = {
		.name		= "isp",
		.start  = ISP_REGS_BASE,
		.end    = ISP_REGS_BASE + ISP_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct resource vfe_vip1_resource[] = {
	[0] = {
		.name		= "csi",
		.start  = CSI1_REGS_BASE,
		.end    = CSI1_REGS_BASE + CSI1_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},

	[1] = {
		.start  = SUNXI_IRQ_CSI1,
		.end    = SUNXI_IRQ_CSI1,
		.flags  = IORESOURCE_IRQ,
	}, 
	[2] = {
		.name		= "isp",
		.start  = ISP_REGS_BASE,
		.end    = ISP_REGS_BASE + ISP_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

#elif defined CONFIG_ARCH_SUN8IW9P1

static struct resource vfe_vip0_resource[] = {
	[0] = {
		.name		= "csi",
		.start  = CSI0_REGS_BASE,
		.end    = CSI0_REGS_BASE + CSI0_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	}, 
	[1] = {
		.start  = SUNXI_IRQ_CSI0,
		.end    = SUNXI_IRQ_CSI0,
		.flags  = IORESOURCE_IRQ,
	}, 
	[2] = {
		.name		= "isp",
		.start  = ISP_REGS_BASE,
		.end    = ISP_REGS_BASE + ISP_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct resource vfe_vip1_resource[] = {
	[0] = {
		.name		= "csi",
		.start  = CSI1_REGS_BASE,
		.end    = CSI1_REGS_BASE + CSI1_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},

	[1] = {
		.start  = SUNXI_IRQ_CSI1,
		.end    = SUNXI_IRQ_CSI1,
		.flags  = IORESOURCE_IRQ,
	}, 
	[2] = {
		.name		= "isp",
		.start  = ISP_REGS_BASE,
		.end    = ISP_REGS_BASE + ISP_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

#elif defined CONFIG_ARCH_SUN9IW1P1

static struct resource vfe_vip0_resource[] = {

	[0] = {
		.start  = SUNXI_IRQ_CSI0,
		.end    = SUNXI_IRQ_CSI0,
		.flags  = IORESOURCE_IRQ,
	}, 
};

static struct resource vfe_vip1_resource[] = {
	[0] = {
		.start  = SUNXI_IRQ_CSI1,
		.end    = SUNXI_IRQ_CSI1,
		.flags  = IORESOURCE_IRQ,
	},
};

#endif

#endif /*_VFE_RESOURCE_H_*/



