/*
 * driver/pinctrl/pinctrl-axp-test.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *         http://www.allwinnertech.com
 *
 * Author: Ming Li <liming@allwinnertech.com>
 *
 * allwinner axp pinctrl test driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/sys_config.h>

#ifndef TEST_DRIVER_IN_DRIVER_PATH
#define TEST_DRIVER_IN_DRIVER_PATH (1)
#endif

#if TEST_DRIVER_IN_DRIVER_PATH
#include "../core.h"
#endif

#define MODULE_NAME "pinctrl.1"

#define PINCTRL_AXP_TEST_NUMBER 	30
#define PINCTRL_AXP_DEV_NAME_MAX_LEN	20

#define CASE_TEST_SUCCESSED		0
#define CASE_TEST_FAILED		1
#define CASE_HAVE_NOT_TEST		2

struct result_class{
	char 		*name;
	int  		result;
};

struct pinctrl_axp_test_data{
	unsigned		int exec;
	unsigned		int gpio_index;
	unsigned		int funcs;
	unsigned 		int dat;
	unsigned 		int test_result;
	char 			dev_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	struct	device	*dev;
};

static struct result_class  pinctrl_axp_result[PINCTRL_AXP_TEST_NUMBER];
static struct class *pinctrl_axp_test_class;
static unsigned int test_case_number;

static inline int axp_gpio_to_name(int gpio, char *name)
{
	int index;
	if (!name) {
		return -EINVAL;
	}
	if (IS_AXP_PIN(gpio)) {
		/* axp gpio name like this : GPIO0/GPIO1/.. */
		index = gpio - AXP_PIN_BASE;
		sprintf(name, "GPIO%d", index);
	} else {
		sprintf(name, "NOT AXP PIN");
	}
	return 0;
}

static int test_request_all_resource_api(struct device *dev,
				struct pinctrl_axp_test_data *pinctrl_axp_test)
{

	struct pinctrl			*pinctrl;
	struct gpio_config		 pin_cfg;
	script_item_u 			*pin_list;
	u16 		 		pin_count;
	u16 		 		pin_index;
	long unsigned int		config;
	char				pin_name[SUNXI_PIN_NAME_MAX_LEN];
	char				device_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	int				ret = 0;

	pinctrl_axp_result[0].name = "test_request_all_resource_api";
	test_case_number = 0;
	dev_set_name(dev, pinctrl_axp_test->dev_name);
	strcpy(device_name,pinctrl_axp_test->dev_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("device[%s] all pin resource we want to request: \n",device_name);
	script_dump_mainkey(device_name);
	pr_warn("start testing...\n");
	pr_warn("------------------------------------\n");
	pr_warn("step1: request pin all resource.\n");
	pinctrl = devm_pinctrl_get_select_default(dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		pr_warn("request pinctrl handle for device [%s] failed...\n",device_name);
		return -EINVAL;
	}
	pr_warn("step2: get device[%s] pin count.\n",device_name);
	pin_count = script_get_pio_list(pinctrl_axp_test->dev_name,&pin_list);
	if (pin_count == 0) {
		pr_warn(" devices own 0 pin resource or look for main key failed!\n");
		return -EINVAL;
	}
	pr_warn("step3: get device[%s] pin configure and check.\n",device_name);
	for(pin_index = 0; pin_index < pin_count; pin_index++)
	{
		pin_cfg = pin_list[pin_index].gpio;
		sunxi_gpio_to_name(pin_cfg.gpio, pin_name);

		/*check function config */
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
		pin_config_get(MODULE_NAME,pin_name,&config);
		if (pin_cfg.mul_sel != SUNXI_PINCFG_UNPACK_VALUE(config)){
			pr_warn("failed! mul value isn't equal as sys_config's.");
			return -EINVAL;
		}
		/*check pull config */
		if (pin_cfg.pull != GPIO_PULL_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, 0xFFFF);
			pin_config_get(MODULE_NAME, pin_name, &config);
			if (pin_cfg.pull != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("failed! pull value isn't equal as sys_config's.");
				return -EINVAL;
			}
		}
		/*check dlevel config */
		if (pin_cfg.drv_level != GPIO_DRVLVL_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV,0XFFFF);
			pin_config_get(MODULE_NAME,pin_name,&config);
			if(pin_cfg.drv_level != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("failed! dlevel value isn't equal as sys_config's.");
				return -EINVAL;
			}
		}
		/*check data config */
		if (pin_cfg.data != GPIO_DATA_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0XFFFF);
			pin_config_get(MODULE_NAME,pin_name,&config);
			if(pin_cfg.data != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("failed! pin data value isn't equal as sys_config's.");
				return -EINVAL;
			}
		}

	}
	pr_warn("test pinctrl request all resource api success !\n");
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	return ret=0;
}

static int test_re_request_all_resource_api(struct device *dev,
				struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	struct pinctrl 		*pinctrl_1;
	struct pinctrl 		*pinctrl_2;
	int			ret=0;
	char			device_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	pinctrl_axp_result[1].name="test_re_request_all_resource_api";
	test_case_number = 1;
	dev_set_name(dev, pinctrl_axp_test->dev_name);
	strcpy(device_name,pinctrl_axp_test->dev_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("device[%s] all pin resource we want to request: \n",device_name);
	script_dump_mainkey(device_name);
	pr_warn("start testing...\n");
	pr_warn("------------------------------------\n");
	pr_warn("step1: first time request pin all resource.\n");
	/*request all resource */
	pinctrl_1 = devm_pinctrl_get_select_default(dev);
	if (IS_ERR_OR_NULL(pinctrl_1)) {
		pr_warn("request pinctrl handle for device [%s] failed!\n",device_name);
		return -EINVAL;
	}

	/*repeat request */
	pr_warn("step2: secondary request pin all resource.\n");
	pinctrl_2 = devm_pinctrl_get_select_default(dev);
	if (IS_ERR_OR_NULL(pinctrl_2)) {
		pr_warn("repeat request device[%s] all pin resource failed\n", device_name);
		pr_warn("test success! repeat request is unpermitted.\n");
		ret = 0;
		goto done;
	} else {
		pr_warn("repeat request device[%s] all pin resource success.\n", dev_name(dev));
		pr_warn("test failed!  repeat request is unpermitted\n");
		ret = 1;
		goto done;
	}
done:
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	return ret;

}
static int test_pin_function_set_api(struct device *dev,
			struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int ret=0;
	long unsigned int		config_set;
	long unsigned int		config_get;
	char				pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int				func=pinctrl_axp_test->funcs;
	pinctrl_axp_result[2].name = "test_pin_function_set_api";
	test_case_number = 2;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("\npin function we want to set:\n");
	pr_warn(" gpio name: %s	    gpio index: %d       gpio function: %d\n"
		,pin_name,pinctrl_axp_test->gpio_index,func);
	pr_warn("--------------------------------------------\n\n");
	pr_warn("start testing...\n");

	/*check if pin mul setting right */
	pr_warn("step1:get [%s] function value.\n",pin_name);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0XFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config_get);
	pr_warn("     [%s] function value: %ld\n",pin_name,SUNXI_PINCFG_UNPACK_VALUE(config_get));

	pr_warn("step2:set [%s] function value to %d\n",pin_name,func);
	config_set = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,func);
	pin_config_set(MODULE_NAME,pin_name,config_set);

	pr_warn("step3:get [%s] function value and check.\n",pin_name);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0XFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config_get);
	if (func != SUNXI_PINCFG_UNPACK_VALUE(config_get)){
		pr_warn("test pin config for mul setting failed !\n");
		return -EINVAL;
	}
	pr_warn("\n\ntest pin function set success ! \n");
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	return ret;
}
static int test_pin_data_set_api(struct device *dev,
			struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int ret=0;
	long unsigned int		config_set;
	long unsigned int		config_get;
	char				pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int				dat=pinctrl_axp_test->dat;
	pinctrl_axp_result[3].name = "test_pin_data_set_api";
	test_case_number = 3;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("\npin data we want to set:\n");
	pr_warn(" gpio name: %s	    gpio index: %d       gpio data: %d\n"
		,pin_name,pinctrl_axp_test->gpio_index,dat);
	pr_warn("--------------------------------------------\n\n");
	pr_warn("start testing...\n");

	/*check if pin data setting right */
	pr_warn("step1:get [%s] data value.\n",pin_name);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0XFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config_get);
	pr_warn("     [%s] data value: %ld\n",pin_name,SUNXI_PINCFG_UNPACK_VALUE(config_get));

	pr_warn("step2:set [%s] data value to %d\n",pin_name,dat);
	config_set = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,dat);
	pin_config_set(MODULE_NAME,pin_name,config_set);

	pr_warn("step3:get [%s] data value and check.\n",pin_name);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0XFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config_get);
	if (dat != SUNXI_PINCFG_UNPACK_VALUE(config_get)){
		pr_warn("test pin config for dlevel setting failed !\n");
		return -EINVAL;
	}
	pr_warn("\n\ntest pin configure set success ! \n");
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	return ret;
}

static int test_gpio_request_free_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int 			ret=0;
	int 			req_status;
	int 			gpio_index = pinctrl_axp_test->gpio_index;
	char			pin_name[SUNXI_PIN_NAME_MAX_LEN];
	pinctrl_axp_result[4].name = "test_gpio_request_free_api";
	test_case_number = 4;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("gpio name is : %s	gpio index is : %d\n",pin_name
		,pinctrl_axp_test->gpio_index);
	pr_warn("start testing...\n");
	/* request gpio*/
	pr_warn("step1: request gpio[%s]\n",pin_name);
	gpio_free(gpio_index);
	req_status = gpio_request(gpio_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed !return value %d\n",req_status);
		return -EINVAL;
	}
	pr_warn("       request gpio[%s] success\n",pin_name);
	gpio_free(gpio_index);
	pr_warn("step2: free gpio[%s]\n",pin_name);
	pr_warn("test gpio request free api success!\n");
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	return ret;


}
static int test_gpio_repeat_request_free_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int 			ret=0;
	int 			req_status;
	int 			gpio_index = pinctrl_axp_test->gpio_index;
	char			pin_name[SUNXI_PIN_NAME_MAX_LEN];
	pinctrl_axp_result[5].name = "test_gpio_re_request_free_api";
	test_case_number = 5;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("gpio name is : %s	gpio index is : %d\n",pin_name
		,pinctrl_axp_test->gpio_index);
	pr_warn("start testing...\n");
	/* request gpio*/
	pr_warn("step1:first time request gpio[%s]\n",pin_name);
	req_status = gpio_request(gpio_index,NULL);
	if(0 != req_status){
		pr_warn("      first time request gpio [%s]failed !\n",pin_name);
		return -EINVAL;
	}
	pr_warn("      first time request gpio[%s] success!\n",pin_name);
	/* repeat request gpio */
	pr_warn("step2: repeat request gpio[%s]\n",pin_name);
	req_status = gpio_request(gpio_index,NULL);
	if(0 != req_status){
		pr_warn("repeat request gpio [%s] failed.\n",pin_name);
		pr_warn("test success: for repeat request is unpermitted.\n");
		ret = 0;
		goto done;
	}else{
		pr_warn("repeat request gpio[%s]success.\n",pin_name);
		pr_warn("test failed: for repeat request is unpermitted.\n");
		ret = 1;
		goto done;
	}
done:
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	gpio_free(gpio_index);
	return ret;
}
static int test_gpiolib_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int			ret = 0;
	int			val;
	u16			pin_index;
	char		pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int			req_status;
	int			set_direct_status;
	long unsigned int	config;
	pinctrl_axp_result[6].name = "test_gpiolib_api";
	test_case_number = 6;
	pin_index = pinctrl_axp_test->gpio_index;
	sunxi_gpio_to_name(pin_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("gpio name is : %s	gpio index is : %d\n",pin_name,pin_index);
	pr_warn("start testing...\n");

	/*
	 * test gpio set direction input api
	 */
	pr_warn("-----------------------------------------------\n");
	pr_warn("1.test gpio direction input api:\n");
	pr_warn("step1:request gpio.\n");
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed !\n");
		return -EINVAL;
	}
	pr_warn("step2:set gpio direction input.\n");
	set_direct_status = gpio_direction_input(pin_index);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction input failed!\n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step3:get gpio mux value and check.\n");
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config);
	if (0 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("test gpio set direction input failed !\n");
		goto test_gpiolib_api_failed;
	}
	gpio_free(pin_index);
	pr_warn("step4:gpio free.\n");
	pr_warn("finish API(gpio_direction_input)testing.\n");
	pr_warn("-----------------------------------------------\n\n");

	/*
	 * test gpio set direction output api
	 */
	pr_warn("2.test gpio direction output api:\n");
	pr_warn("step1:request gpio.\n");
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed !\n");
		return -EINVAL;
	}
	pr_warn("step2:set gpio direction output(data value 1).\n");
	set_direct_status = gpio_direction_output(pin_index,1);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed! \n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step3:get gpio mux value and check.\n");
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config);
	if (1 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("faile!FUNC value not the same as expectation.\n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step4:get gpio data value and check.\n");
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0xFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config);
	if (1 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("failed!DATA value not the same as expectation(1).\n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step5:set gpio direction output(data value 0).\n");
	set_direct_status = gpio_direction_output(pin_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed!\n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step6:get gpio data value and check.\n");
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0xFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config);
	if (0 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("failed!DATA value not the same as expectation(0).\n");
		goto test_gpiolib_api_failed;
	}
	gpio_free(pin_index);
	pr_warn("step7:gpio free.\n");
	pr_warn("finish API(gpio_direction_output)testing.\n");
	pr_warn("-----------------------------------------------\n\n");

	/*
	 * test gpio get value api
	 */
	pr_warn("3.test gpio get value api:\n");
	pr_warn("step1:request gpio.\n");
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed !\n");
		return -EINVAL;
	}
	pr_warn("step2:set gpio direction output(data value 0).\n");
	set_direct_status = gpio_direction_output(pin_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed !\n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step3:get gpio data value and check.\n");
	val=__gpio_get_value(pin_index);
	pr_warn("      gpio data value :    %d \n",val);
	if (0 != val){
		pr_warn("failed!DATA value not the same as expectation.\n");
		goto test_gpiolib_api_failed;
	}
	gpio_free(pin_index);
	pr_warn("step4:gpio free.\n");
	pr_warn("finish API(gpio_get_value)testing.\n");
	pr_warn("-----------------------------------------------\n\n");

	/*
	 * test gpio set value api
	 */
	pr_warn("4.test gpio set value api:\n");
	pr_warn("step1:request gpio.\n");
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed!\n");
		return -EINVAL;
	}
	pr_warn("step2:set gpio direction output(set data value 0).\n");
	set_direct_status = gpio_direction_output(pin_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed \n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step3:get gpio data value,then set 1 and check.\n");
	val=__gpio_get_value(pin_index);
	pr_warn("       get gpio data value :    %d \n",val);
	__gpio_set_value(pin_index,1);
	pr_warn("       set gpio data value :    1 \n");
	val=__gpio_get_value(pin_index);
	pr_warn("       get gpio data value :    %d \n",val);
	if (1 != val){
		pr_warn("test gpio set dat value 1 failed ! \n");
		goto test_gpiolib_api_failed;
	}
	pr_warn("step4:get gpio data value,then set 0 and check.\n");
	val=__gpio_get_value(pin_index);
	pr_warn("       get gpio data value :    %d \n",val);
	__gpio_set_value(pin_index,0);
	pr_warn("       set gpio data value :    0 \n");
	val=__gpio_get_value(pin_index);
	pr_warn("       get gpio data value :    %d \n",val);
	if (0 != val){
		pr_warn("test gpio set dat value 0 failed ! \n");
		goto test_gpiolib_api_failed;
	}
	gpio_free(pin_index);
	pr_warn("step5:gpio free.\n");
	pr_warn("finish API(gpio_set_value)testing.\n");
	pr_warn("-----------------------------------------------\n\n");

	pr_warn("test gpiolib api success!\n");
	pr_warn("+++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");

	return ret;

test_gpiolib_api_failed:
	pr_warn("test gpiolib api failed!\n");
	pr_warn("+++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	gpio_free(pin_index);
	return -EINVAL;

}

static int test_pinctrl_scripts_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int		ret = 0;
	char		main_key[256] = {0},sub_key[256] = {0}, str_cmp[256] = {0};
	int		gpio_cnt_cmp, gpio_cnt_get;
	script_item_u 	item_cmp, item_get, *list_get = NULL;
	script_item_value_type_e 	type_cmp, type_get;
	pinctrl_axp_result[7].name = "test_pinctrl_eint_api";
	test_case_number = 7;

	printk("%s, line %d\n", __func__, __LINE__);

	/* test script api */
	strcpy(main_key, "card0_boot_para");
	script_dump_mainkey(main_key);

	/* test for type int */
	strcpy(sub_key, "card_ctrl");
	item_cmp.val = 0;
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_INT;
	type_get = script_get_item(main_key, sub_key, &item_get);
	if(type_get != type_cmp){
		pr_warn("%s err, line %d, %s->%s type should be %d, but get %d\n",
			__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
		return -EINVAL;
	}
	if(item_cmp.val != item_get.val){
		pr_warn("%s err, line %d, %s->%s value should be %d, but get %d\n",
			__func__, __LINE__, main_key, sub_key, item_cmp.val, item_get.val);
		return -EINVAL;
	}
	/* test for type gpio */
	strcpy(sub_key, "sdc_d3");
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_PIO;
	type_get = script_get_item(main_key, sub_key, &item_get);
	if(type_get != type_cmp){
		pr_warn("%s err, line %d, %s->%s type should be %d, but get %d\n",
			__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
		return -EINVAL;
	}
	/* test for gpio list */
	gpio_cnt_cmp = 6;
	gpio_cnt_get = script_get_pio_list(main_key, &list_get);
	if(gpio_cnt_get != gpio_cnt_cmp){
		pr_warn("%s err, line %d, %s gpio cnt should be %d, but get %d\n",
			__func__, __LINE__, main_key, gpio_cnt_cmp, gpio_cnt_get);
		return -EINVAL;
	}
	/* test for str */
	strcpy(main_key, "product");
	strcpy(sub_key, "machine");

#if defined CONFIG_ARCH_SUN8IW6P1
	strcpy(str_cmp, "perf3_v1_0");
#elif defined CONFIG_ARCH_SUN8IW7
	strcpy(str_cmp, "perf");
#else
	strcpy(str_cmp, "evb");

#endif
	script_dump_mainkey(main_key);
	type_cmp = SCIRPT_ITEM_VALUE_TYPE_STR;
	type_get = script_get_item(main_key, sub_key, &item_get);
	if(type_get != type_cmp){
		pr_warn("%s err, line %d, %s->%s type should be %d, but get %d\n",
			__func__, __LINE__, main_key, sub_key, type_cmp, type_get);
		return -EINVAL;
	}
	if(strcmp(str_cmp, item_get.str)){
		pr_warn("%s err, line %d, %s->%s value should be %s, but get %s\n",
			__func__, __LINE__, main_key, sub_key, str_cmp, item_get.str);
		return -EINVAL;
	}
	pr_warn("test sunxi pinctrl scripts success!\n");
	return ret;
}

static int test_pinctrl_request_gpio_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int 			ret=0;
	int 			req_status;
	int 			gpio_index = pinctrl_axp_test->gpio_index;
	char			pin_name[SUNXI_PIN_NAME_MAX_LEN];
	pinctrl_axp_result[8].name = "test_pinctrl_request_gpio_api";
	test_case_number = 8;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("gpio name is : %s	gpio index is : %d\n",pin_name
		,pinctrl_axp_test->gpio_index);
	pinctrl_free_gpio(gpio_index);
	pr_warn("start testing...\n");
	/* request signal pin as gpio*/
	pr_warn("step1: pinctrl request gpio[%s]\n",pin_name);
	req_status = pinctrl_request_gpio(gpio_index);
	if(0 != req_status){
		pr_warn("pinctrl request gpio failed !return value %d\n",req_status);
		return -EINVAL;
	}
	pr_warn("       pinctrl request gpio[%s]success\n",pin_name);
	pinctrl_free_gpio(gpio_index);
	pr_warn("test pinctrl request gpio api success!\n");
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	return ret;

}
static int test_pinctrl_free_gpio_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int		ret = 0;
	int		req_status;
	int		gpio_index = pinctrl_axp_test->gpio_index;
	char	pin_name[SUNXI_PIN_NAME_MAX_LEN];
	pinctrl_axp_result[9].name = "test_pinctrl_free_gpio_api";
	test_case_number = 9;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("gpio name is : %s	gpio index is : %d\n",pin_name
		,pinctrl_axp_test->gpio_index);
	pinctrl_free_gpio(gpio_index);
	pr_warn("start testing...\n");
	/*request signal pin as gpio*/
	pr_warn("step1: pinctrl request gpio[%s]\n",pin_name);
	req_status = pinctrl_request_gpio(gpio_index);
	if(0 != req_status){
		pr_warn("pinctrl request gpio failed !return value %d\n",req_status);
		return -EINVAL;
	}
	pr_warn("       pinctrl request gpio[%s]success\n",pin_name);
	pr_warn("step2: pinctrl free gpio[%s]\n",pin_name);
	pinctrl_free_gpio(gpio_index);
	pr_warn("step3: pinctrl request the same gpio[%s] again..\n",pin_name);
	req_status = pinctrl_request_gpio(gpio_index);
	if(0 != req_status){
		pr_warn("pinctrl request gpio failed !return value %d\n",req_status);
		return -EINVAL;
	}
	pr_warn("       pinctrl request gpio[%s] again success.\n",pin_name);
	pr_warn("test pinctrl free gpio api success!\n");
	pr_warn("++++++++++++++++++++++++++++end++++++++++++++++++++++++++++\n\n\n");
	pinctrl_free_gpio(gpio_index);
	return ret;
}
static int test_pinctrl_gpio_direction_input_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int		ret = 0;
	int		req_status;
	int		gpio_index = pinctrl_axp_test->gpio_index;
	char	pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int		direct_status;
	long unsigned int	config;

	pinctrl_axp_result[10].name = "test_pinctrl_gpio_direction_input_api";
	test_case_number = 10;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("gpio name is : %s	gpio index is : %d\n",pin_name
		,pinctrl_axp_test->gpio_index);
	pinctrl_free_gpio(gpio_index);
	pr_warn("start testing...\n");
	/* test gpio set direction input */
	pr_warn("-----------------------------------------------\n");
	pr_warn("step1:Pinctrl request gpio.\n");
	req_status = pinctrl_request_gpio(gpio_index);
	if(0 != req_status){
		pr_warn("pinctrl request gpio failed !return value %d\n",req_status);
		return -EINVAL;
	}
	pr_warn("step2:Set gpio direction input.\n");
	direct_status = pinctrl_gpio_direction_input(gpio_index);
	if (IS_ERR_VALUE(direct_status)) {
		pr_warn("set pinctrl gpio direction input failed! return value: %d\n"
							,direct_status);
		return -EINVAL;
	}
	pr_warn("step3:Get pin mux value and check.\n");
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config);
	if (0 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("check: set pin direction input failed !\n");
		return -EINVAL;
	}
	pr_warn("step4:Pinctrl free gpio.\n");
	pinctrl_free_gpio(gpio_index);
	pr_warn("test pinctrl gpio direction input api success!\n");
	pr_warn("-----------------------------------------------\n\n");

	return ret;

}
static int test_pinctrl_gpio_direction_output_api(struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	int		ret = 0;
	int		req_status;
	int		gpio_index = pinctrl_axp_test->gpio_index;
	char	pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int		direct_status;
	long unsigned int	config;

	pinctrl_axp_result[11].name = "test_pinctrl_gpio_direction_output_api";
	test_case_number = 11;
	sunxi_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	pr_warn("gpio name is : %s	gpio index is : %d\n",pin_name
		,pinctrl_axp_test->gpio_index);
	pinctrl_free_gpio(gpio_index);
	pr_warn("start testing...\n");
	/* test gpio set direction output */
	pr_warn("-----------------------------------------------\n");
	pr_warn("step1:Pinctrl request gpio.\n");
	req_status = pinctrl_request_gpio(gpio_index);
	if(0 != req_status){
		pr_warn("pinctrl request gpio failed !return value %d\n",req_status);
		return -EINVAL;
	}
	pr_warn("step2:Set gpio direction output.\n");
	direct_status = pinctrl_gpio_direction_output(gpio_index);
	if (IS_ERR_VALUE(direct_status)) {
		pr_warn("set pinctrl gpio direction output failed! return value: %d\n"
							,direct_status);
		return -EINVAL;
	}
	pr_warn("step3:Get pin mux value and check.\n");
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	pin_config_get(MODULE_NAME,pin_name,&config);
	if (1 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("check: set pinctrl gpio direction output failed !\n");
		return -EINVAL;
	}
	pr_warn("step4:Pinctrl free gpio.\n");
	pinctrl_free_gpio(gpio_index);
	pr_warn("test pinctrl gpio direction output api success!\n");
	pr_warn("-----------------------------------------------\n\n");
	return ret;

}

static int test_pinctrl_get_api(struct device  *dev,
					struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	struct pinctrl			*pinctrl;
	struct pinctrl_state	*state;
	struct pinctrl_setting	*setting;
	char			device_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	int				ret = 0;

	pinctrl_axp_result[12].name = "test_pinctrl_get_api";
	test_case_number = 12;
	dev_set_name(dev, pinctrl_axp_test->dev_name);
	strcpy(device_name,pinctrl_axp_test->dev_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	script_dump_mainkey(device_name);
	pr_warn("start testing...\n");
	pr_warn("------------------------------------\n");
	pr_warn("step1: get pinctrl handle.\n");
	pinctrl = pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		pr_warn("get pinctrl handle [%s] failed...,return value %ld\n",device_name,PTR_ERR(pinctrl));
		return -EINVAL;
	}
	pr_warn("step2: check pinctrl handle we have getted.\n");
	if(dev_name(dev) != dev_name(pinctrl->dev)){
		pinctrl_put(pinctrl);
		pr_warn("check: pinctrl handle isn't that one we want\n ");
		return -EINVAL;
	}
#if defined(TEST_DRIVER_IN_DRIVER_PATH)
	pr_warn("device: %s current state: %s\n",dev_name(pinctrl->dev),
						pinctrl->state ? pinctrl->state->name : "none");
	list_for_each_entry(state, &pinctrl->states, node) {
		pr_warn("state: %s\n", state->name);
		list_for_each_entry(setting, &state->settings, node) {
			struct pinctrl_dev *pctldev = setting->pctldev;
			pr_warn("    setting type: %d   pin controller %s \n"
								,setting->type,pinctrl_dev_get_name(pctldev));
		}
	}
#endif
	pinctrl_put(pinctrl);
	pr_warn("test pinctrl get api success!\n");
	pr_warn("-----------------------------------------------\n\n");
	return ret;

}
static int test_pinctrl_lookup_state_api(struct device *dev,
							struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	struct pinctrl			*pinctrl;
	struct pinctrl_state	*state;
	char			device_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	int				ret = 0;
	pinctrl_axp_result[13].name = "test_pinctrl_lookup_state_api";
	test_case_number = 13;
	dev_set_name(dev, pinctrl_axp_test->dev_name);
	strcpy(device_name,pinctrl_axp_test->dev_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	script_dump_mainkey(device_name);
	pr_warn("start testing...\n");
	pr_warn("------------------------------------\n");
	pr_warn("step1: get pinctrl handle.\n");
	pinctrl = pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		pr_warn("get pinctrl handle [%s] failed...,return value %ld\n",device_name,PTR_ERR(pinctrl));
		return -EINVAL;
	}
	pr_warn("step2: printk pinctrl current state.\n");
	pr_warn("       device: %s current state: %s\n",dev_name(pinctrl->dev),
						pinctrl->state ? pinctrl->state->name : "none");
	pr_warn("step3: pinctrl lookup state(default state name: default).\n");
	state = pinctrl_lookup_state(pinctrl, "default");
	if (IS_ERR(state)) {
		pr_warn("can not find state: default.\n");
		return -EINVAL;
	}
	pr_warn("step4: check the state we lookup if the one needed.\n");
	if (strcmp(state->name, "default")){
		pr_warn("find state,but isn't the one we need.\n");
		return -EINVAL;
	}
	pinctrl_put(pinctrl);
	pr_warn("test pinctrl look up state api success!\n");
	pr_warn("-----------------------------------------------\n\n");
	return ret;
}
static int test_pinctrl_select_state_api(struct device *dev,
							struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	struct pinctrl			*pinctrl;
	struct pinctrl_state	*state;
	char			device_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	int				ret = 0;
	int				reqstatus;
	pinctrl_axp_result[14].name = "test_pinctrl_select_state_api";
	test_case_number = 14;
	dev_set_name(dev, pinctrl_axp_test->dev_name);
	strcpy(device_name,pinctrl_axp_test->dev_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	script_dump_mainkey(device_name);
	pr_warn("start testing...\n");
	pr_warn("------------------------------------\n");
	pr_warn("step1: get pinctrl handle.\n");
	pinctrl = pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		pr_warn("get pinctrl handle [%s] failed...,return value %ld\n",device_name,PTR_ERR(pinctrl));
		return -EINVAL;
	}
	pr_warn("step2: printk pinctrl current state.\n");
	pr_warn("       device: %s current state: %s\n",dev_name(pinctrl->dev),
						pinctrl->state ? pinctrl->state->name : "none");
	pr_warn("step3: pinctrl lookup state(default state name: default).\n");
	state = pinctrl_lookup_state(pinctrl, "default");
	if (IS_ERR(state)) {
		pinctrl_put(pinctrl);
		pr_warn("can not find state: default.\n");
		return -EINVAL;
	}
	pr_warn("step4: check the state we lookup if the one needed.\n");
	if (strcmp(state->name, "default")){
		pinctrl_put(pinctrl);
		pr_warn("find state,but isn't the one we need.\n");
		return -EINVAL;
	}
	pr_warn("step5: select state for pinctrl handle.\n");
	reqstatus = pinctrl_select_state(pinctrl, state);
	if(reqstatus < 0){
		pinctrl_put(pinctrl);
		pr_warn("pinctrl select state failed. return value %d.\n",reqstatus);
	}
	pinctrl_put(pinctrl);
	pr_warn("test pinctrl select state api success!\n");
	pr_warn("-----------------------------------------------\n\n");
	return ret;
}
static int test_pinctrl_put_api(struct device  *dev,
					struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	struct pinctrl			*pinctrl;
	struct pinctrl_state	*state;
	struct pinctrl_setting	*setting;
	char			device_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	int				ret = 0;

	pinctrl_axp_result[15].name = "test_pinctrl_put_api";
	test_case_number = 15;
	dev_set_name(dev, pinctrl_axp_test->dev_name);
	strcpy(device_name,pinctrl_axp_test->dev_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	script_dump_mainkey(device_name);
	pr_warn("start testing...\n");
	pr_warn("------------------------------------\n");
	pr_warn("step1: get pinctrl handle.\n");
	pinctrl = pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		pr_warn("get pinctrl handle [%s] failed...,return value %ld\n",device_name,PTR_ERR(pinctrl));
		return -EINVAL;
	}
	pr_warn("step2: check pinctrl handle we have getted.\n");
	if(dev_name(dev) != dev_name(pinctrl->dev)){
		pinctrl_put(pinctrl);
		pr_warn("check: pinctrl handle isn't that one we want\n ");
		return -EINVAL;
	}
#if defined(TEST_DRIVER_IN_DRIVER_PATH)
	pr_warn("device: %s current state: %s\n",dev_name(pinctrl->dev),
						pinctrl->state ? pinctrl->state->name : "none");
	list_for_each_entry(state, &pinctrl->states, node) {
		pr_warn("state: %s\n", state->name);
		list_for_each_entry(setting, &state->settings, node) {
			struct pinctrl_dev *pctldev = setting->pctldev;
			pr_warn("    setting type: %d   pin controller %s \n"
								,setting->type,pinctrl_dev_get_name(pctldev));
		}
	}
#endif
	pr_warn("step3: free pinctrl handle we have getted.\n");
	pinctrl_put(pinctrl);
	pr_warn("step4: then repeat get.if get success ,previous free operate success.\n");
	pinctrl = pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		pr_warn("       after free,we repeat get pinctrl handle [%s] failed...,return value %ld\n",device_name,PTR_ERR(pinctrl));
		return -EINVAL;
	}
	pinctrl_put(pinctrl);
	pr_warn("test pinctrl put api success!\n");
	pr_warn("-----------------------------------------------\n\n");
	return ret;

}
static int test_pinctrl_devm_get_put_api(struct device  *dev,
					struct pinctrl_axp_test_data *pinctrl_axp_test)
{
	struct pinctrl			*pinctrl;
	struct pinctrl_state	*state;
	struct pinctrl_setting	*setting;
	char			device_name[PINCTRL_AXP_DEV_NAME_MAX_LEN];
	int				ret = 0;

	pinctrl_axp_result[16].name = "test_pinctrl_devm_get_put_api";
	test_case_number = 16;
	dev_set_name(dev, pinctrl_axp_test->dev_name);
	strcpy(device_name,pinctrl_axp_test->dev_name);
	pr_warn("++++++++++++++++++++++++++++%s++++++++++++++++++++++++++++\n", __func__);
	script_dump_mainkey(device_name);
	pr_warn("start testing...\n");
	pr_warn("------------------------------------\n");
	pr_warn("step1: devm get pinctrl handle.\n");
	pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(pinctrl)) {
		pr_warn("get pinctrl handle [%s] failed...,return value %ld\n",device_name,PTR_ERR(pinctrl));
		return -EINVAL;
	}
	pr_warn("step2: check pinctrl handle we have getted.\n");
	if(dev_name(dev) != dev_name(pinctrl->dev)){
		devm_pinctrl_put(pinctrl);
		pr_warn("check: pinctrl handle isn't that one we want\n ");
		return -EINVAL;
	}
#if defined(TEST_DRIVER_IN_DRIVER_PATH)
	pr_warn("device: %s current state: %s\n",dev_name(pinctrl->dev),
						pinctrl->state ? pinctrl->state->name : "none");
	list_for_each_entry(state, &pinctrl->states, node) {
		pr_warn("state: %s\n", state->name);
		list_for_each_entry(setting, &state->settings, node) {
			struct pinctrl_dev *pctldev = setting->pctldev;
			pr_warn("    setting type: %d   pin controller %s \n"
								,setting->type,pinctrl_dev_get_name(pctldev));
		}
	}
#endif
	pr_warn("step3: devm free pinctrl handle we have getted.\n");
	devm_pinctrl_put(pinctrl);
	pr_warn("step4: then repeat get.if get success ,previous free operate success.\n");
	pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(pinctrl)) {
		pr_warn("       after free,we repeat get pinctrl handle [%s] failed...,return value %ld\n",device_name,PTR_ERR(pinctrl));
		return -EINVAL;
	}
	pinctrl_put(pinctrl);
	pr_warn("test pinctrl devm get and put api success!\n");
	pr_warn("-----------------------------------------------\n\n");
	return ret;

}


static ssize_t exec_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test = dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",pinctrl_axp_test->exec);
}

static ssize_t exec_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test = dev_get_drvdata(dev);
	char 			*after;
	int 			final =0;
	int 			exec_number =simple_strtoul(buf,&after,10);
	switch(exec_number){
		case 0:
			final = test_request_all_resource_api(dev,pinctrl_axp_test);
			break;
		case 1:
			final = test_re_request_all_resource_api(dev,pinctrl_axp_test);
			break;
		case 2:
			final = test_pin_function_set_api(dev,pinctrl_axp_test);
			break;
		case 3:
			final = test_pin_data_set_api(dev,pinctrl_axp_test);
			break;
		case 4:
			final = test_gpio_request_free_api(pinctrl_axp_test);
			break;
		case 5:
			final = test_gpio_repeat_request_free_api(pinctrl_axp_test);
			break;
		case 6:
			final = test_gpiolib_api(pinctrl_axp_test);
			break;
		case 7:
			final = test_pinctrl_scripts_api(pinctrl_axp_test);
			break;
		case 8:
			final = test_pinctrl_request_gpio_api(pinctrl_axp_test);
			break;
		case 9:
			final = test_pinctrl_free_gpio_api(pinctrl_axp_test);
			break;
		case 10:
			final = test_pinctrl_gpio_direction_input_api(pinctrl_axp_test);
			break;
		case 11:
			final = test_pinctrl_gpio_direction_output_api(pinctrl_axp_test);
			break;
		case 12:
			final = test_pinctrl_get_api(dev,pinctrl_axp_test);
			break;
		case 13:
			final = test_pinctrl_lookup_state_api(dev,pinctrl_axp_test);
			break;
		case 14:
			final = test_pinctrl_select_state_api(dev,pinctrl_axp_test);
			break;
		case 15:
			final = test_pinctrl_put_api(dev,pinctrl_axp_test);
			break;
		case 16:
			final = test_pinctrl_devm_get_put_api(dev,pinctrl_axp_test);
			break;
		default:
			pr_warn(" your input number should less than case number.\n");
			final=1;
			break;
	}

	pinctrl_axp_test->exec = exec_number;
	if (final)
		pinctrl_axp_result[exec_number].result=CASE_TEST_FAILED;
	else{

		if (pinctrl_axp_result[exec_number].result==CASE_HAVE_NOT_TEST)
			pinctrl_axp_result[exec_number].result = CASE_TEST_SUCCESSED;
	}
	return size;
}

static ssize_t gpio_index_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	/*chang gpio index to pin name */
	char pin_name[PINCTRL_AXP_TEST_NUMBER];
	struct pinctrl_axp_test_data *pinctrl_axp_test=dev_get_drvdata(dev);
	axp_gpio_to_name(pinctrl_axp_test->gpio_index, pin_name);

	return sprintf(buf,"%s\n",pin_name);
}

static ssize_t gpio_index_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test = dev_get_drvdata(dev);

	char 			*after;
	int 			gpio_index= simple_strtoul(buf,&after,10);
	size_t			count=after-buf;
	ssize_t 		ret= -EINVAL;
	if (isspace(*after))
		count++;
	if(count==size){
		ret = count;
		pinctrl_axp_test->gpio_index	= gpio_index;
	}
	if (!IS_AXP_PIN(pinctrl_axp_test->gpio_index)) {
		pr_err("NOT AXP PIN\n");
	}
	return ret;
}

static ssize_t function_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test=dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",pinctrl_axp_test->funcs);
}
static ssize_t function_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test=dev_get_drvdata(dev);

	char 			*after;
	int 			funcs =simple_strtoul(buf,&after,10);
	size_t 			count =after-buf;
	ssize_t 		ret =-EINVAL;

	if (isspace(*after))
		count++;
	if(count==size){
		ret=count;
		pinctrl_axp_test->funcs=funcs;
	}
	return ret;
}
static ssize_t data_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test=dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",pinctrl_axp_test->dat);

}
static ssize_t data_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test=dev_get_drvdata(dev);

	char 			*after;
	int 			dat =simple_strtoul(buf,&after,10);
	size_t 			count =after-buf;
	ssize_t 		ret =-EINVAL;

	if (isspace(*after))
		count++;
	if(count==size){
		ret=count;
		pinctrl_axp_test->dat=dat;
	}
	return ret;
}

static ssize_t test_result_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	return sprintf(buf,"%d\n",pinctrl_axp_result[test_case_number].result);
}

static ssize_t test_result_shore(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

static ssize_t dev_name_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test=dev_get_drvdata(dev);
	return sprintf(buf,"%s\n",pinctrl_axp_test->dev_name);
}

static ssize_t dev_name_shore(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int 		ret;
	struct pinctrl_axp_test_data *pinctrl_axp_test = dev_get_drvdata(dev);
	if(size>PINCTRL_AXP_DEV_NAME_MAX_LEN){
		pr_warn("sunxi dev name max len less than 20 char.\n");
		return -EINVAL;
	}
	ret=strlcpy(pinctrl_axp_test->dev_name,buf, size);
	return ret;
}


static struct device_attribute pinctrl_axp_test_class_attrs[] = {
	__ATTR(exec, S_IRUGO|S_IWUSR, exec_show, exec_store),
	__ATTR(gpio_index,S_IRUGO|S_IWUSR,gpio_index_show,gpio_index_store),
	__ATTR(function,S_IRUGO|S_IWUSR,function_show,function_store),
	__ATTR(dat,S_IRUGO|S_IWUSR,data_show,data_store),
	__ATTR(test_result,S_IRUGO|S_IWUSR,test_result_show,test_result_shore),
	__ATTR(dev_name,S_IRUGO|S_IWUSR,dev_name_show,dev_name_shore),
	__ATTR_NULL
};

static int pinctrl_axp_test_probe(struct platform_device *pdev)
{
	struct pinctrl_axp_test_data *pinctrl_axp_test;
	pinctrl_axp_test=devm_kzalloc(&pdev->dev,sizeof(*pinctrl_axp_test),GFP_KERNEL);
	if(!pinctrl_axp_test){
		dev_err(&pdev->dev,"No enougt memory for device \n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev,pinctrl_axp_test);

	pinctrl_axp_test_class = class_create(THIS_MODULE,"pinctrl_axp_test_class");
	if(IS_ERR(pinctrl_axp_test_class))
		return PTR_ERR(pinctrl_axp_test_class);
	pinctrl_axp_test_class->dev_attrs = pinctrl_axp_test_class_attrs;

	pinctrl_axp_test->dev=device_create(pinctrl_axp_test_class,&pdev->dev,0,
										pinctrl_axp_test,"pinctrl_axp_test");

	if(IS_ERR(pinctrl_axp_test->dev))
		return PTR_ERR(pinctrl_axp_test->dev);
	return 0;

}

static struct platform_driver pinctrl_axp_test_driver={
	.probe		=pinctrl_axp_test_probe,
	.driver		={
		.name		="pinctrl_axp_test",
		.owner		=THIS_MODULE,
	},
};

static struct platform_device pinctrl_axp_test_device={
	.name		="pinctrl_axp_test",
	.id		=PLATFORM_DEVID_NONE,
};

static int __init pinctrl_axp_test_init(void)
{
	int ret,i;
	for(i=0;i<PINCTRL_AXP_TEST_NUMBER;i++){
		pinctrl_axp_result[i].result=CASE_HAVE_NOT_TEST;
	}
	ret=platform_device_register(&pinctrl_axp_test_device);
	if(IS_ERR_VALUE(ret)){
		pr_warn("register sunxi pinctrl test device failed\n");
		return -EINVAL;
	}
	ret=platform_driver_register(&pinctrl_axp_test_driver);
	if(IS_ERR_VALUE(ret)){
		pr_warn("register sunxi pinctrl test driver failed\n");
		return -EINVAL;
	}
	return ret;

	return 0;
}

static void __exit pinctrl_axp_test_exit(void)
{
	platform_device_unregister(&pinctrl_axp_test_device);
	platform_driver_unregister(&pinctrl_axp_test_driver);
	class_destroy(pinctrl_axp_test_class);
}

module_init(pinctrl_axp_test_init);
module_exit(pinctrl_axp_test_exit);

MODULE_AUTHOR("Ming Li <liming@allwinnertech.com");
MODULE_DESCRIPTION("Allwinner pinctrl axp driver test");
MODULE_LICENSE("GPL");

