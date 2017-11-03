
/* 
 ***************************************************************************************
 * 
 * sunxi_csi.h
 * 
 * Hawkview ISP - sunxi_csi.h module
 * 
 * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   3.0		  Yang Feng   	2015/02/27	ISP Tuning Tools Support
 * 
 ****************************************************************************************
 */

#ifndef _SUNXI_CSI_H_
#define _SUNXI_CSI_H_

#include "../platform_cfg.h"

#define VIDIOC_SUNXI_CSI_GET_FRM_SIZE 			1

struct csi_platform_data
{
	unsigned int csi_sel;
}; 
struct csi_dev
{
	unsigned int  csi_sel;
	int use_cnt;
	struct v4l2_subdev subdev;
	struct platform_device  *pdev;
	unsigned int id;
	spinlock_t slock;
	struct mutex subdev_lock;
	int irq;  
	wait_queue_head_t   wait;
	void __iomem      *base;
	struct bus_info         bus_info;
	struct frame_info       frame_info;
	struct frame_arrange    arrange;
	unsigned int            capture_mode;
};

void sunxi_csi_dump_regs(struct v4l2_subdev *sd);
int sunxi_csi_get_subdev(struct v4l2_subdev **sd, int sel);
int sunxi_csi_put_subdev(struct v4l2_subdev **sd, int sel);
int sunxi_csi_register_subdev(struct v4l2_device *v4l2_dev, struct v4l2_subdev *sd);
void sunxi_csi_unregister_subdev(struct v4l2_subdev *sd);
int sunxi_csi_platform_register(void);
void sunxi_csi_platform_unregister(void);


#endif /*_SUNXI_CSI_H_*/
