
/* 
 ***************************************************************************************
 * 
 * isp_cfg.c
 * 
 * Hawkview ISP - isp_cfg.c module
 * 
 * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   3.0		  Yang Feng   	2015/01/18	ISP Tuning Tools Support
 * 
 ****************************************************************************************
 */


#include <linux/kernel.h>
#include <linux/string.h>
#include "isp_cfg.h"
#include "SENSOR_H/ov2710_mipi_isp_cfg.h"
#define ISP_CFG_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct isp_cfg_item isp_cfg_array[] = 
{
	{	"ov2710_mipi",		&ov2710_mipi_isp_cfg,	},
};

int get_isp_cfg(char *isp_cfg_name, struct isp_cfg_item *isp_cfg_info)
{
	int i;
	for(i = 0; i < ISP_CFG_ARRAY_SIZE(isp_cfg_array); i++)
	{
		if(strcmp(isp_cfg_name,isp_cfg_array[i].isp_cfg_name) == 0)
		{
			*isp_cfg_info = isp_cfg_array[i];
			return 0;
		}
	}
	printk("[VFE_WARN]NOT found this item:  %s, you can add this ISP Config in the isp_cfg_array!\n", isp_cfg_name);
	return -1;
}


