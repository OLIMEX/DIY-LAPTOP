/*
 * drivers/usb/sunxi_usb/manager/usb_manager.c
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * javen, 2011-4-14, create this file
 *
 * usb manager.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/kthread.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/of_gpio.h>

#include  "../include/sunxi_usb_config.h"
#include  "usb_manager.h"
#include  "usbc_platform.h"
#include  "usb_hw_scan.h"
#include  "usb_msg_center.h"

struct usb_cfg g_usb_cfg;
int thread_id_irq_run_flag;
int thread_device_run_flag = 0;
int thread_host_run_flag = 0;

__u32 thread_run_flag = 1;
int thread_stopped_flag = 1;
atomic_t thread_suspend_flag;

static int usb_device_scan_thread(void * pArg)
{
	/* delay for udc & hcd ready */
	msleep(3000);

	while(thread_device_run_flag) {

		msleep(1000);  /* 1s */
		hw_rmmod_usb_host();
		hw_rmmod_usb_device();
		usb_msg_center(&g_usb_cfg);

		hw_insmod_usb_device();
		usb_msg_center(&g_usb_cfg);
		thread_device_run_flag = 0;
		DMSG_INFO("device_chose finished %d!\n",__LINE__);
	}

	return 0;
}

 static int usb_host_scan_thread(void * pArg)
{

	/* delay for udc & hcd ready */
	msleep(3000);

	while(thread_host_run_flag) {

		msleep(1000);  /* 1s */
		hw_rmmod_usb_host();
		hw_rmmod_usb_device();
		usb_msg_center(&g_usb_cfg);

		hw_insmod_usb_host();
		usb_msg_center(&g_usb_cfg);
		thread_host_run_flag = 0;
		DMSG_INFO("host_chose finished %d!\n",__LINE__);
	}

	return 0;
}

static int usb_hardware_scan_thread(void * pArg)
{
	struct usb_cfg *cfg = pArg;

	/* delay for udc & hcd ready */

	msleep(3000);

	while(thread_run_flag) {
		msleep(1000);  /* 1s */

		if (atomic_read(&thread_suspend_flag))
			continue;
		usb_hw_scan(cfg);
		usb_msg_center(cfg);
	}

	thread_stopped_flag = 1;
	return 0;
}

static irqreturn_t usb_id_irq(int irq, void *parg)
{
	struct usb_cfg *cfg = parg;

	mdelay(1000);

	/*
	 * rmmod usb device/host driver first, then insmod usb host/device driver.
	 */
	usb_hw_scan(cfg);
	usb_msg_center(cfg);

	usb_hw_scan(cfg);
	usb_msg_center(cfg);

	return IRQ_HANDLED;
}

static int usb_id_irq_thread(void *parg)
{
	struct usb_cfg *cfg = parg;
	int id_irq_num = 0;
	unsigned long irq_flags = 0;
	int ret = 0;

	/* delay for udc & hcd ready */
	msleep(3000);

	while (thread_id_irq_run_flag) {
		msleep(1000);
		hw_rmmod_usb_host();
		hw_rmmod_usb_device();
		usb_msg_center(cfg);

		hw_insmod_usb_device();
		usb_msg_center(cfg);

		if (cfg->port.id.valid) {
			id_irq_num = gpio_to_irq(cfg->port.id.gpio_set.gpio.gpio);
			if (IS_ERR_VALUE(id_irq_num)) {
				DMSG_PANIC("ERR: map usb id gpio to virq failed, err %d\n",
					   id_irq_num);
				return -EINVAL;
			}

			irq_flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
				    IRQF_ONESHOT;
			ret = request_threaded_irq(id_irq_num, NULL, usb_id_irq,
						   irq_flags, "usb_id", cfg);
			if (IS_ERR_VALUE(ret)) {
				DMSG_PANIC("ERR: request usb id virq %d failed, err %d\n",
					 id_irq_num, ret);
				return -EINVAL;
			}
			cfg->port.id_irq_num = id_irq_num;
		}

		thread_id_irq_run_flag = 0;
	}

	return 0;
}

static __s32 usb_script_parse(struct device_node *np, struct usb_cfg *cfg)
{
#ifdef CONFIG_OF
	struct device_node *usbc_np = NULL;
	int ret = -1;
	const char  *used_status;

	usbc_np = of_find_node_by_type(NULL, SET_USB0);

	/* usbc enable */
	ret = of_property_read_string(usbc_np, "status", &used_status);
	if (ret) {
		DMSG_INFO("get usb_used is fail, %d\n", -ret);
		cfg->port.enable = 0;
	}else if (!strcmp(used_status, "okay")) {
		cfg->port.enable = 1;
	}else {
		cfg->port.enable = 0;
	}

	/* usbc port type */
	ret = of_property_read_u32(usbc_np, KEY_USB_PORT_TYPE, &cfg->port.port_type);
	if (ret)
		DMSG_INFO("get usb_port_type is fail, %d\n", -ret);

	/* usbc det mode */
	ret = of_property_read_u32(usbc_np, KEY_USB_DET_MODE, &cfg->port.detect_mode);
	if (ret)
		DMSG_INFO("get usb_detect_mode is fail, %d\n", -ret);

	/* usbc det_vbus */
	ret = of_property_read_string(usbc_np, KEY_USB_DETVBUS_GPIO, &cfg->port.det_vbus_name);
	if (ret) {
		DMSG_INFO("get det_vbus is fail, %d\n", -ret);
		cfg->port.det_vbus.valid = 0;
	}else{
		if (strncmp(cfg->port.det_vbus_name, "axp_ctrl", 8) == 0) {
			cfg->port.det_vbus_type = USB_DET_VBUS_TYPE_AXP;
			cfg->port.det_vbus.valid = 0;
		} else {
			/*get det vbus gpio*/
			cfg->port.det_vbus.gpio_set.gpio.gpio = of_get_named_gpio(usbc_np, KEY_USB_DETVBUS_GPIO, 0);
			if (gpio_is_valid(cfg->port.det_vbus.gpio_set.gpio.gpio)) {
				cfg->port.det_vbus.valid = 1;
				cfg->port.det_vbus_type = USB_DET_VBUS_TYPE_GIPO;
			}else{
				cfg->port.det_vbus.valid = 0;
			}
		}
	}

	/* usbc id gpio*/
	cfg->port.id.gpio_set.gpio.gpio = of_get_named_gpio(usbc_np, KEY_USB_ID_GPIO, 0);
	if (gpio_is_valid(cfg->port.id.gpio_set.gpio.gpio)) {
		cfg->port.id.valid = 1;
	}else{
		cfg->port.id.valid = 0;
	}

#else
        script_item_value_type_e type = 0;
	script_item_u item_temp;

	/* usbc enable */
	type = script_get_item(SET_USB0, KEY_USB_ENABLE, &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		cfg->port.enable = item_temp.val;
	}else{
		DMSG_PANIC("ERR: get usbc enable failed\n");
		cfg->port.enable = 0;
	}

	/* usbc port type */
	type = script_get_item(SET_USB0, KEY_USB_PORT_TYPE, &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		cfg->port.port_type = item_temp.val;
	}else{
		DMSG_PANIC("ERR: get usbc port type failed\n");
		cfg->port.port_type = 0;
	}

	/* usbc det mode */
	type = script_get_item(SET_USB0, KEY_USB_DET_MODE, &item_temp);
	if (type == SCIRPT_ITEM_VALUE_TYPE_INT) {
		cfg->port.detect_mode = item_temp.val;
	} else {
		DMSG_PANIC("ERR: get usbc port type failed\n");
		cfg->port.detect_mode = 0;
	}

	/* usbc id */
	type = script_get_item(SET_USB0, KEY_USB_ID_GPIO, &(cfg->port.id.gpio_set));
	if(type == SCIRPT_ITEM_VALUE_TYPE_PIO){
		cfg->port.id.valid = 1;
	}else{
		cfg->port.id.valid = 0;
		DMSG_PANIC("ERR: get usbc id failed\n");
	}

	/* usbc det_vbus */
	type = script_get_item(SET_USB0, KEY_USB_DETVBUS_GPIO, &(cfg->port.det_vbus.gpio_set));
	if(type == SCIRPT_ITEM_VALUE_TYPE_PIO){
		cfg->port.det_vbus.valid = 1;
		cfg->port.det_vbus_type = USB_DET_VBUS_TYPE_GIPO;
	}else{
		cfg->port.det_vbus.valid = 0;
		cfg->port.det_vbus_type = USB_DET_VBUS_TYPE_NULL;
		DMSG_PANIC("ERR: get usbc det_vbus gpio failed\n");
	}

	if(cfg->port.det_vbus.valid == 0){
		type = script_get_item(set_usbc, KEY_USB_DETVBUS_GPIO, &item_temp);
		if(type == SCIRPT_ITEM_VALUE_TYPE_STR){
			if(strncmp(item_temp.str, "axp_ctrl", 8) == 0){
				cfg->port.det_vbus_type = USB_DET_VBUS_TYPE_AXP;
			}else{
				cfg->port.det_vbus_type = USB_DET_VBUS_TYPE_NULL;
			}
		}else{
				DMSG_PANIC("ERR: get usbc det_vbus axp failed\n");
				cfg->port.det_vbus_type = USB_DET_VBUS_TYPE_NULL;
		}
	}

	/* usbc drv_vbus */
	type = script_get_item(set_usbc, KEY_USB_DRVVBUS_GPIO, &(cfg->port.drv_vbus.gpio_set));
	if(type == SCIRPT_ITEM_VALUE_TYPE_PIO){
		cfg->port.drv_vbus.valid = 1;
	}else{
		cfg->port.drv_vbus.valid = 0;
		DMSG_PANIC("ERR: get usbc det_vbus failed\n");
	}

#endif
	return 0;
}

int usb_otg_id_status(void)
{
	struct usb_cfg *cfg = NULL;
	int id_status = -1;

	cfg = &g_usb_cfg;
	if(cfg == NULL){
		return -1;
	}

	if(cfg->port.port_type == USB_PORT_TYPE_DEVICE){
		return 1;
	}

	if(cfg->port.port_type == USB_PORT_TYPE_OTG) {
		if(cfg->port.id.valid){
			id_status = __gpio_get_value(cfg->port.id.gpio_set.gpio.gpio);
		}
	}

	return id_status;
}
EXPORT_SYMBOL(usb_otg_id_status);

static int sunxi_otg_manager_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct task_struct *device_th = NULL;
	struct task_struct *host_th = NULL;
	struct task_struct *th = NULL;
	struct task_struct *id_irq_th = NULL;
	int ret = -1;

	memset(&g_usb_cfg, 0, sizeof(struct usb_cfg));
	g_usb_cfg.usb_global_enable = 1;
	usb_msg_center_init();

	ret = usb_script_parse(np, &g_usb_cfg);
	if (ret != 0) {
		DMSG_PANIC("ERR: get_usb_cfg failed\n");
		return -1;
	}

	if (g_usb_cfg.port.enable == 0) {
		DMSG_PANIC("wrn: usb0 is disable\n");
		return 0;
	}

	create_node_file(pdev);

	if (g_usb_cfg.port.port_type == USB_PORT_TYPE_DEVICE) {

		thread_device_run_flag = 1;
		device_th = kthread_create(usb_device_scan_thread, NULL, "usb_device_chose");
		if (IS_ERR(device_th)) {
			DMSG_PANIC("ERR: device kthread_create failed\n");
			return -1;
		}

		wake_up_process(device_th);
	}

	if (g_usb_cfg.port.port_type == USB_PORT_TYPE_HOST) {

		set_usb_role_ex(USB_ROLE_HOST);

		thread_host_run_flag = 1;
		host_th = kthread_create(usb_host_scan_thread, NULL, "usb_host_chose");
		if (IS_ERR(host_th)) {
			DMSG_PANIC("ERR: host kthread_create failed\n");
			return -1;
		}

		wake_up_process(host_th);
	}

	if (g_usb_cfg.port.port_type == USB_PORT_TYPE_OTG) {
		usb_hw_scan_init(&g_usb_cfg);

		if (g_usb_cfg.port.detect_mode == USB_DETECT_MODE_THREAD) {
			atomic_set(&thread_suspend_flag, 0);
			thread_run_flag = 1;
			thread_stopped_flag = 0;

			th = kthread_create(usb_hardware_scan_thread, &g_usb_cfg,
					    "usb-hardware-scan");
			if (IS_ERR(th)) {
				DMSG_PANIC("ERR: kthread_create failed\n");
				return -1;
			}

			wake_up_process(th);
		} else if (g_usb_cfg.port.detect_mode == USB_DETECT_MODE_INTR) {
			thread_id_irq_run_flag = 1;
			id_irq_th = kthread_create(usb_id_irq_thread, &g_usb_cfg,
						   "usb_id_irq");
			if (IS_ERR(id_irq_th)) {
				DMSG_PANIC("ERR: id_irq kthread_create failed\n");
				return -1;
			}

			wake_up_process(id_irq_th);
		} else {
			DMSG_PANIC("ERR: usb detect mode isn't supported\n");
			return -1;
		}
	}

	return 0;
}

static int sunxi_otg_manager_remove(struct platform_device *pdev)
{
	if (g_usb_cfg.port.enable == 0) {
		DMSG_PANIC("wrn: usb0 is disable\n");
		return 0;
	}

	remove_node_file(pdev);
	if (g_usb_cfg.port.port_type == USB_PORT_TYPE_OTG) {
		thread_run_flag = 0;
		while(!thread_stopped_flag) {
			DMSG_INFO("waitting for usb_hardware_scan_thread stop\n");
			msleep(10);
		}
		if (g_usb_cfg.port.detect_mode == USB_DETECT_MODE_INTR)
			if (g_usb_cfg.port.id.valid && g_usb_cfg.port.id_irq_num)
					free_irq(g_usb_cfg.port.id_irq_num,
						 &g_usb_cfg);
		usb_hw_scan_exit(&g_usb_cfg);
	}

	return 0;
}

#ifdef CONFIG_PM
static int sunxi_otg_manager_suspend(struct device *dev)
{
	atomic_set(&thread_suspend_flag, 1);
	return 0;
}

static int sunxi_otg_manager_resume(struct device *dev)
{
	atomic_set(&thread_suspend_flag, 0);
	return 0;
}

static const struct dev_pm_ops sunxi_otg_manager_pm_ops = {
	.suspend = sunxi_otg_manager_suspend,
	.resume = sunxi_otg_manager_resume,
};
#define OTG_MANAGER_PM_OPS        (&sunxi_otg_manager_pm_ops)

#else /* !CONFIG_PM_SLEEP */

#define OTG_MANAGER_PM_OPS        NULL
#endif /* CONFIG_PM_SLEEP */

static const struct of_device_id sunxi_otg_manager_match[] = {
	{.compatible = "allwinner,sunxi-otg-manager", },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_otg_manager_match);

static struct platform_driver sunxi_otg_manager_platform_driver = {
	.probe  = sunxi_otg_manager_probe,
	.remove = sunxi_otg_manager_remove,
	.driver = {
		.name  = "otg manager",
		.pm    = OTG_MANAGER_PM_OPS,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_otg_manager_match,
	},
};

static int __init usb_manager_init(void)
{
	return platform_driver_register(&sunxi_otg_manager_platform_driver);
}

static void __exit usb_manager_exit(void)
{
	return platform_driver_unregister(&sunxi_otg_manager_platform_driver);
}

fs_initcall(usb_manager_init);
module_exit(usb_manager_exit);

MODULE_AUTHOR("wangjx<wangjx@allwinnertech.com>");
MODULE_DESCRIPTION("Driver for Allwinner usb otg manager");
MODULE_LICENSE("GPL");

