
/* 
 ***************************************************************************************
 * 
 * isp_cfg.h
 * 
 * Hawkview ISP - isp_cfg.h module
 * 
 * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   3.0		  Yang Feng   	2015/01/18	ISP Tuning Tools Support
 * 
 ****************************************************************************************
 */
 
#ifndef _ISP_CFG_H_
#define _ISP_CFG_H_
#include "../lib/bsp_isp_algo.h"
struct isp_cfg_pt
{
	struct isp_test_param          *isp_test_settings;
	struct isp_3a_param            *isp_3a_settings;
	struct isp_tunning_param     *isp_tunning_settings;
	struct isp_iso_param           *isp_iso_settings;
};
struct isp_cfg_item {
	char isp_cfg_name[32];
	struct isp_cfg_pt *isp_cfg;
};

int get_isp_cfg(char *isp_cfg_name, struct isp_cfg_item *isp_cfg_info);


#endif /*_ISP_CFG_H_*/
 


