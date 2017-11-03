
/* 
 ***************************************************************************************
 * 
 * cci_platform_drv.h
 * 
 * Hawkview ISP - cci_platform_drv.h module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   2.0		  Yang Feng   	2014/06/23	      Second Version
 * 
 ****************************************************************************************
 */
#ifndef _CCI_PLATFORM_DRV_H_
#define _CCI_PLATFORM_DRV_H_

#include "../platform_cfg.h"
struct cci_platform_data
{
	unsigned int cci_sel;
}; 
struct cci_dev
{
	unsigned int  cci_sel;
	struct platform_device  *pdev;
	unsigned int id;
	spinlock_t slock;
	int irq;  
	wait_queue_head_t   wait;

	void __iomem      *base;
};


#endif /*_CCI_PLATFORM_DRV_H_*/
