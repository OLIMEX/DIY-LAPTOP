/*
 ***************************************************************************************
 *
 * sensor_info.c
 *
 * Hawkview ISP - sensor_info.c module
 *
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   2.0		  Yang Feng   	2014/02/24	      Second Version
 *
 ****************************************************************************************
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include "sensor_info.h"
#include "../platform_cfg.h"
#define SENSOR_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
struct sensor_item sensor_list_t[] =
{
	//         name                  i2c_addr               sensor type               sensor size          sensor max pclk
	{	"ov2640"		,	0x60,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"ov5640"		,	0x78,		SENSOR_YUV	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"ov5647"		,	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"ov5647_mipi"	,	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"ov5650"		,	0x50,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"ov5648"		,	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"ov8825"		, 	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_8M		, CORE_CLK_RATE_FOR_8M},
	{	"ov8850"		, 	0x20,		SENSOR_RAW	,	 PIXEL_NUM_8M		, CORE_CLK_RATE_FOR_8M},
	{	"ov12830"		, 	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_12M		, CORE_CLK_RATE_FOR_16M},
	{	"ov16825"		, 	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_16M		, CORE_CLK_RATE_FOR_16M},
	{	"gc0329"		,	0x62,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc0309"		,	0x42,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc0307"		,	0x42,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc0308"		,	0x42,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc2035"		,	0x78,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"gt2005"		,	0x78,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"gc2015"		,	0x60,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"gc2235"		,	0x78,		SENSOR_RAW	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"sp0838"		,	0x30,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"sp0718"		, 	0x6c,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"sp2518"		, 	0x6c,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"hi253"			,	0x40,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"hi257"			,	0x40,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"s5k4ec"		,	0x5a,		SENSOR_YUV	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"s5k4ec_mipi"	,	0x5a,		SENSOR_YUV	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"s5k4e1"		,	0x20,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"s5k4e1_mipi"	,	0x20,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"t4k05"			, 	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_8M		, CORE_CLK_RATE_FOR_8M},
	{	"t8et5"			,	0x78,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"ar0330"		,	0x20,		SENSOR_RAW	,	 PIXEL_NUM_3M		, CORE_CLK_RATE_FOR_3M},
	{	"bf3a03"		,	0xDC,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc0311"		,	0x66,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc0311"		,	0x66,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc5004"		,	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"gc5004_mipi"	,	0x6c,		SENSOR_RAW	,	 PIXEL_NUM_5M		, CORE_CLK_RATE_FOR_5M},
	{	"nt99252"		,	0x6c,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"ov7736"		,	0x42,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc2155"		,	0x78,		SENSOR_YUV	,	 PIXEL_NUM_2M		, CORE_CLK_RATE_FOR_2M},
	{	"gc0328c"		,	0x42,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
	{	"gc0328"		,	0x42,		SENSOR_YUV	,	 PIXEL_NUM_0_3M		, CORE_CLK_RATE_FOR_2M},
};
int get_sensor_info(char *sensor_name, struct sensor_item *sensor_info)
{
	int i;
	for(i = 0; i < SENSOR_ARRAY_SIZE(sensor_list_t); i++)
	{
		if(strcmp(sensor_name,sensor_list_t[i].sensor_name) == 0)
		{
			*sensor_info = sensor_list_t[i];
			return 0;
		}
	}
	printk("[VFE_WARN]NOT found this item:  %s, you can add this sensor in the sensor_list_t!\n", sensor_name);
	return -1;
}
