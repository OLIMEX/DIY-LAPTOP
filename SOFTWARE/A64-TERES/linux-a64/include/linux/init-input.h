/*
 * Copyright (c) 2013-2015 liming@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef _INIT_INPUT_H
#define _INIT_INPUT_H
#include <linux/sys_config.h>
//#include <mach/system.h>

//#include <mach/hardware.h>
//we dont have this *.h in linux3.10, so define here directly
#define IO_ADDRESS(x)            (void __iomem *)(((x) & 0x0fffffff) + (((x) >> 4) & 0x0f000000) + 0xf0000000)

//this *.h already include in linux/sys_config.h
//#include <mach/sys_config.h>

#include <linux/interrupt.h>
#include <linux/regulator/consumer.h>

typedef u32 (*gpio_int_handle)(void *para);

enum input_sensor_type{
	CTP_TYPE,
	GSENSOR_TYPE,
	GYR_TYPE,
	COMPASS_TYPE,
	LS_TYPE,
	MOTOR_TYPE,
};

struct sensor_config_info{
	enum input_sensor_type input_type;
	int sensor_used;
	__u32 twi_id;
	u32 int_number;
	struct gpio_config irq_gpio;
	char* ldo;
	struct device *dev;
	struct pinctrl *pinctrl;
	const char *sensor_power;
	u32 sensor_power_vol;
	struct regulator *sensor_power_ldo;
};

struct ctp_config_info{
	enum input_sensor_type input_type;
	int ctp_used;
	__u32 twi_id;
	const char * name;
	int screen_max_x;
	int screen_max_y;
	int revert_x_flag;
	int revert_y_flag;
	int exchange_x_y_flag;
	u32 int_number;
	unsigned char device_detect;
	const char *ctp_power;
	u32 ctp_power_vol;
	struct gpio_config ctp_power_io;
	struct regulator *ctp_power_ldo;
	struct gpio_config irq_gpio;
	struct gpio_config wakeup_gpio; 
#ifdef TOUCH_KEY_LIGHT_SUPPORT 
	struct gpio_config key_light_gpio;
#endif
	struct device *dev;
	struct pinctrl *pinctrl;
};

struct motor_config_info{
	enum input_sensor_type input_type;
	int motor_used;
	int vibe_off;
	u32 ldo_voltage;
	char* ldo;
	struct gpio_config motor_gpio;
};

int input_fetch_sysconfig_para(enum input_sensor_type *input_type);
void input_free_platform_resource(enum input_sensor_type *input_type);
int input_init_platform_resource(enum input_sensor_type *input_type);
int input_request_int(enum input_sensor_type *input_type,irq_handler_t handle,
			unsigned long trig_type, void *para);
int input_free_int(enum input_sensor_type *input_type, void *para);
int input_set_int_enable(enum input_sensor_type *input_type, u32 enable);
int input_set_power_enable(enum input_sensor_type *input_type, u32 enable);
#endif
