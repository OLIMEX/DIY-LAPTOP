/* 
 ***************************************************************************************
 * 
 * sensor_info.h
 * 
 * Hawkview ISP - sensor_info.h module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   2.0		  Yang Feng   	2014/02/24	      Second Version
 * 
 ****************************************************************************************
 */
#ifndef _SENSOR_INFO_H_
#define _SENSOR_INFO_H_

enum sensor_type_t {
        SENSOR_YUV             = 0,
        SENSOR_RAW            = 1,
        SENSOR_YUV_USE_ISP             = 2,
};
enum sensor_size_t {
        PIXEL_NUM_0_3M		 = 0,
        PIXEL_NUM_1M		,
        PIXEL_NUM_2M 		,
        PIXEL_NUM_3M 		,
        PIXEL_NUM_4M 		,
        PIXEL_NUM_5M 		,
        PIXEL_NUM_8M 		,
        PIXEL_NUM_12M 		,
        PIXEL_NUM_13M 		,
        PIXEL_NUM_16M 		,
        PIXEL_NUM_20M 		,
        PIXEL_NUM_22M 		,
};

struct sensor_item {
	char sensor_name[32];
	int i2c_addr;
	enum sensor_type_t  sensor_type;
	enum sensor_size_t  sensor_size;
	int core_clk_for_sensor;
};

int get_sensor_info(char *sensor_name, struct sensor_item *sensor_info);



#endif /*_SENSOR_INFO_H_*/


