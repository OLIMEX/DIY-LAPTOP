
/* 
 ***************************************************************************************
 * 
 * sunxi_mipi.h
 * 
 * Hawkview ISP - sunxi_mipi.h module
 * 
 * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   3.0		  Yang Feng   	2015/02/27	ISP Tuning Tools Support
 * 
 ****************************************************************************************
 */
#ifndef _SUNXI_MIPI__H_
#define _SUNXI_MIPI__H_

#include "../platform_cfg.h"
struct mipi_platform_data
{
	unsigned int mipi_sel;
}; 
struct mipi_dev
{
	unsigned int  mipi_sel;
	int use_cnt;
	struct v4l2_subdev subdev;
	struct platform_device  *pdev;
	unsigned int id;
	spinlock_t slock;
//	int irq;  
	wait_queue_head_t   wait;
	void __iomem      *base;
	struct mipi_para        mipi_para;
	struct mipi_fmt         mipi_fmt;
};

int sunxi_mipi_get_subdev(struct v4l2_subdev **sd, int sel);
int sunxi_mipi_put_subdev(struct v4l2_subdev **sd, int sel);
int sunxi_mipi_register_subdev(struct v4l2_device *v4l2_dev, struct v4l2_subdev *sd);
void sunxi_mipi_unregister_subdev(struct v4l2_subdev *sd);
int sunxi_mipi_platform_register(void);
void sunxi_mipi_platform_unregister(void);

#endif /*_SUNXI_MIPI__H_*/
