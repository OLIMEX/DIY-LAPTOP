/*
 * sunxi-di.c DE-Interlace driver
 *
 * Copyright (C) 2013-2015 allwinner.
 *	Ming Li<liming@allwinnertech.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/sys_config.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <asm/irq.h>
#include "sunxi-di.h"

static di_struct *di_data;
static s32 sunxi_di_major = -1;
static struct class *di_dev_class;
static struct workqueue_struct *di_wq = NULL;
static struct work_struct  di_work;
static struct timer_list *s_timer;
static struct device *di_device = NULL;
static struct clk *di_clk;
static struct clk *di_clk_source;

static u32 debug_mask = 0x0;

static ssize_t di_timeout_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);

	if(error) {
		pr_err("%s strict_strtoul error\n", __FUNCTION__);
		goto exit;
	}

	dprintk(DEBUG_DATA_INFO, "%s data = %ld \n", __func__, data);

	if(data) {
		di_data->time_value = data;
	} else {
		di_data->time_value = DI_TIMEOUT;
	}

	return count;

exit:
	return error;
}

static DEVICE_ATTR(timeout, 0664,
		NULL, di_timeout_store);

static struct attribute *di_attributes[] = {
	&dev_attr_timeout.attr,
	NULL
};

static struct attribute_group di_attribute_group = {
	.attrs = di_attributes
};

#ifdef DI_RESERVED_MEM
#define MY_BYTE_ALIGN(x) ( ( (x + (4*1024-1)) >> 12) << 12)             /* alloc based on 4K byte */
void *sunxi_di_alloc(u32 num_bytes, unsigned long phys_addr)
{
	u32 actual_bytes;
	void* address = NULL;

	if (num_bytes != 0) {
	    actual_bytes = MY_BYTE_ALIGN(num_bytes);

	    address = dma_alloc_coherent(di_device, actual_bytes, (dma_addr_t*)phys_addr, GFP_KERNEL);
	    if (address) {
		printk(KERN_ERR "dma_alloc_coherent ok, address=0x%p, size=0x%x\n", (void*)(*(unsigned long*)phys_addr), num_bytes);
		return address;
	    } else {
		printk(KERN_ERR "dma_alloc_coherent fail, size=0x%x\n", num_bytes);
		return NULL;
	    }
	} else {
	    printk(KERN_ERR "%s size is zero\n", __func__);
	}

	return NULL;
}

void sunxi_di_free(void* virt_addr, unsigned long phys_addr, u32 num_bytes)
{
	u32 actual_bytes;

	actual_bytes = MY_BYTE_ALIGN(num_bytes);
	if (phys_addr && virt_addr)
	    dma_free_coherent(di_device, actual_bytes, virt_addr, (dma_addr_t)phys_addr);

	return ;
}
#endif

static int di_mem_request(__di_mem_t *di_mem)
{
#ifndef DI_RESERVED_MEM
	unsigned map_size = 0;
	struct page *page;

	map_size = PAGE_ALIGN(di_mem->size);

	page = alloc_pages(GFP_KERNEL,get_order(map_size));
	if (page != NULL) {
		di_mem->v_addr = page_address(page);
		if (NULL == di_mem->v_addr)	{
			free_pages((unsigned long)(page),get_order(map_size));
			printk(KERN_ERR "page_address fail!\n");
			return -ENOMEM;
		}
		di_mem->p_addr = virt_to_phys(di_mem->v_addr);
		memset(di_mem->v_addr,0,di_mem->size);

		dprintk(DEBUG_DATA_INFO,"pa=0x%p va=0x%p size:0x%x\n",(void*)di_mem->p_addr, di_mem->v_addr, di_mem->size);
		return 0;
	}	else {
		printk(KERN_ERR "alloc_pages fail!\n");
		return -ENOMEM;
	}
#else
	di_mem->v_addr = sunxi_di_alloc(di_mem->size, di_mem->p_addr);
	if (NULL == di_mem->v_addr) {
		printk(KERN_ERR "%s: failed!\n", __func__);
		return -ENOMEM;
	}
#endif
	return 0;
}

static int di_mem_release(__di_mem_t *di_mem)
{
#ifndef DI_RESERVED_MEM
	unsigned map_size = PAGE_ALIGN(di_mem->size);
	unsigned page_size = map_size;

	if (NULL == di_mem->v_addr) {
		printk(KERN_ERR "%s: failed!\n", __func__);
		return -1;
	}

	free_pages((unsigned long)(di_mem->v_addr),get_order(page_size));
	di_mem->v_addr = NULL;
#else
	if (NULL == di_mem->v_addr) {
		printk(KERN_ERR "%s: failed!\n", __func__);
		return -1;
	}
	sunxi_di_free(di_mem->v_addr, di_mem->p_addr, di_mem->size);
	di_mem->v_addr = NULL;
#endif
  return 0;
}

static void di_complete_check_set(s32 data)
{
	atomic_set(&di_data->di_complete, data);

	return;
}

static s32 di_complete_check_get(void)
{
	s32 data_temp = 0;

	data_temp = atomic_read(&di_data->di_complete);

	return data_temp;
}

static void di_timer_handle(unsigned long arg)
{
	u32 flag_size = 0;

	di_complete_check_set(DI_MODULE_TIMEOUT);
	wake_up_interruptible(&di_data->wait);
	flag_size = (FLAG_WIDTH*FLAG_HIGH)/4;
	di_irq_enable(0);
	di_irq_clear();
	di_reset();
	memset(di_data->mem_in_params.v_addr, 0, flag_size);
	memset(di_data->mem_out_params.v_addr, 0, flag_size);
	printk(KERN_ERR "di_timer_handle: timeout \n");
}

static void di_work_func(struct work_struct *work)
{
	del_timer_sync(s_timer);
	return;
}

static irqreturn_t di_irq_service(int irqno, void *dev_id)
{
	s32 ret;

	dprintk(DEBUG_INT, "%s: enter \n", __func__);
	di_irq_enable(0);
	ret = di_get_status();
	if (0 == ret) {
		di_complete_check_set(0);
		wake_up_interruptible(&di_data->wait);
		queue_work(di_wq, &di_work);
	} else {
		di_complete_check_set(-ret);
		wake_up_interruptible(&di_data->wait);
	}
	di_irq_clear();
	di_reset();
	return IRQ_HANDLED;
}

static s32 di_clk_cfg(struct device_node *node)
{
	unsigned long rate = 0;

	di_clk = of_clk_get(node, 0);
	if (!di_clk || IS_ERR(di_clk)) {
		printk(KERN_ERR "try to get di clock failed!\n");
		return -1;
	}

	di_clk_source = of_clk_get(node, 1);
	if (!di_clk_source || IS_ERR(di_clk_source)) {
		printk(KERN_ERR "try to get di_clk_source clock failed!\n");
		return -1;
	}

	rate = clk_get_rate(di_clk_source);
	dprintk(DEBUG_INIT, "%s: get di_clk_source rate %luHZ\n", __func__, rate);

	if(clk_set_parent(di_clk, di_clk_source)) {
		printk(KERN_ERR "%s: set di_clk parent to di_clk_source failed!\n", __func__);
		return -1;
	}

#if defined CONFIG_ARCH_SUN9IW1P1
#else
	rate = rate/2;
	if (clk_set_rate(di_clk, rate)) {
		printk(KERN_ERR "set di clock freq to PLL_PERIPH0/2 failed!\n");
		return -1;
	}
	rate = clk_get_rate(di_clk);
	dprintk(DEBUG_INIT, "%s: get di_clk rate %dHZ\n", __func__, (__u32)rate);
#endif
	return 0;
}

static void di_clk_uncfg(void)
{
	if(NULL == di_clk || IS_ERR(di_clk)) {
		printk(KERN_ERR "di_clk handle is invalid, just return!\n");
		return;
	} else {
		clk_disable_unprepare(di_clk);
		clk_put(di_clk);
		di_clk = NULL;
	}
	if(NULL == di_clk_source || IS_ERR(di_clk_source)) {
		printk(KERN_ERR "di_clk_source handle is invalid, just return!\n");
		return;
	} else {
		clk_put(di_clk_source);
		di_clk_source = NULL;
	}
	return;
}

static s32 di_clk_enable(void)
{
	if(NULL == di_clk || IS_ERR(di_clk)) {
		printk(KERN_ERR "di_clk handle is invalid, just return!\n");
		return -1;
	} else {
		if (clk_prepare_enable(di_clk)) {
			printk(KERN_ERR "try to enable di_clk failed!\n");
			return -1;
		}
	}
	return 0;
}

static void di_clk_disable(void)
{
	if(NULL == di_clk || IS_ERR(di_clk)) {
		printk(KERN_ERR "di_clk handle is invalid, just return!\n");
		return;
	} else {
		clk_disable_unprepare(di_clk);
	}
	return;
}

static s32 sunxi_di_params_init(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct resource *mem_res = NULL;
	s32 ret = 0;
	ret = di_clk_cfg(node);
	if (ret) {
		printk(KERN_ERR "%s: clk cfg failed.\n", __func__);
		goto clk_cfg_fail;
	}

	di_data->irq_number = platform_get_irq(pdev, 0);
	if (di_data->irq_number < 0) {
		printk(KERN_ERR "%s:get irq number failed!\n",  __func__);
		return -EINVAL;
	}
	if (request_irq(di_data->irq_number, di_irq_service, 0, "DE-Interlace",
			di_device)) {
		ret = -EBUSY;
		printk(KERN_ERR "%s: request irq failed.\n", __func__);
		goto request_irq_err;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res == NULL) {
		printk(KERN_ERR "%s: failed to get MEM res\n", __func__);
		ret = -ENXIO;
		goto mem_io_err;
	}

	if (!request_mem_region(mem_res->start, resource_size(mem_res), mem_res->name)) {
		printk(KERN_ERR  "%s: failed to request mem region\n", __func__);
		ret = -EINVAL;
		goto mem_io_err;
	}

	di_data->base_addr = ioremap(mem_res->start, resource_size(mem_res));
	if (!di_data->base_addr) {
		printk(KERN_ERR  "%s: failed to io remap\n", __func__);
		ret = -EIO;
		goto mem_io_err;
	}

	di_set_reg_base(di_data->base_addr);
	return 0;
mem_io_err:
	free_irq(di_data->irq_number, di_device);
request_irq_err:
	di_clk_uncfg();
clk_cfg_fail:
	return ret;
}

static void sunxi_di_params_exit(void)
{
	di_clk_uncfg();
	free_irq(di_data->irq_number, di_device);
	return ;
}


#ifdef CONFIG_PM
static s32 sunxi_di_suspend(struct device *dev)
{
	dprintk(DEBUG_SUSPEND, "enter: sunxi_di_suspend. \n");

	if (atomic_read(&di_data->enable)) {
		di_irq_enable(0);
		di_reset();
		di_internal_clk_disable();
		di_clk_disable();
		if (NULL != di_data->mem_in_params.v_addr)
		di_mem_release(&(di_data->mem_in_params));
		if (NULL != di_data->mem_out_params.v_addr)
		di_mem_release(&(di_data->mem_out_params));
	}

	return 0;
}

static s32 sunxi_di_resume(struct device *dev)
{
	dprintk(DEBUG_SUSPEND, "enter: sunxi_di_resume. \n");

	return 0;
}
#endif

static long sunxi_di_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	s32 ret = 0;
	u32 field = 0;

	dprintk(DEBUG_TEST, "%s: enter!!\n", __func__);

	switch (cmd) {
	case DI_IOCSTART:
		{
			__di_para_t __user *di_para = argp;

			dprintk(DEBUG_DATA_INFO, "%s: input_fb.addr[0] = 0x%lx\n", __func__, (unsigned long)(di_para->input_fb.addr[0]));
			dprintk(DEBUG_DATA_INFO, "%s: input_fb.addr[1] = 0x%lx\n", __func__, (unsigned long)(di_para->input_fb.addr[1]));
			dprintk(DEBUG_DATA_INFO, "%s: input_fb.size.width = %d\n", __func__, di_para->input_fb.size.width);
			dprintk(DEBUG_DATA_INFO, "%s: input_fb.size.height = %d\n", __func__, di_para->input_fb.size.height);
			dprintk(DEBUG_DATA_INFO, "%s: input_fb.format = %d\n", __func__, di_para->input_fb.format);

			dprintk(DEBUG_DATA_INFO, "%s: pre_fb.addr[0] = 0x%lx\n", __func__, (unsigned long)(di_para->pre_fb.addr[0]));
			dprintk(DEBUG_DATA_INFO, "%s: pre_fb.addr[1] = 0x%lx\n", __func__, (unsigned long)(di_para->pre_fb.addr[1]));
			dprintk(DEBUG_DATA_INFO, "%s: pre_fb.size.width = %d\n", __func__, di_para->pre_fb.size.width);
			dprintk(DEBUG_DATA_INFO, "%s: pre_fb.size.height = %d\n", __func__, di_para->pre_fb.size.height);
			dprintk(DEBUG_DATA_INFO, "%s: pre_fb.format = %d\n", __func__, di_para->pre_fb.format);
			dprintk(DEBUG_DATA_INFO, "%s: source_regn.width = %d\n", __func__, di_para->source_regn.width);
			dprintk(DEBUG_DATA_INFO, "%s: source_regn.height = %d\n", __func__, di_para->source_regn.height);

			dprintk(DEBUG_DATA_INFO, "%s: output_fb.addr[0] = 0x%lx\n", __func__, (unsigned long)(di_para->output_fb.addr[0]));
			dprintk(DEBUG_DATA_INFO, "%s: output_fb.addr[1] = 0x%lx\n", __func__, (unsigned long)(di_para->output_fb.addr[1]));
			dprintk(DEBUG_DATA_INFO, "%s: output_fb.size.width = %d\n", __func__, di_para->output_fb.size.width);
			dprintk(DEBUG_DATA_INFO, "%s: output_fb.size.height = %d\n", __func__, di_para->output_fb.size.height);
			dprintk(DEBUG_DATA_INFO, "%s: output_fb.format = %d\n", __func__, di_para->output_fb.format);
			dprintk(DEBUG_DATA_INFO, "%s: out_regn.width = %d\n", __func__, di_para->out_regn.width);
			dprintk(DEBUG_DATA_INFO, "%s: out_regn.height = %d\n", __func__, di_para->out_regn.height);
			dprintk(DEBUG_DATA_INFO, "%s: field = %d\n", __func__, di_para->field);
			dprintk(DEBUG_DATA_INFO, "%s: top_field_first = %d\n", __func__, di_para->top_field_first);

			/* when di is in work, wait*/
			ret = di_complete_check_get();
			while (1 == ret) {
				msleep(1);
				ret = di_complete_check_get();
			}
			di_complete_check_set(1);

			field = di_para->top_field_first?di_para->field:(1-di_para->field);

			dprintk(DEBUG_DATA_INFO, "%s: field = %d\n", __func__, field);
			dprintk(DEBUG_DATA_INFO, "%s: in_flag_phy = 0x%lx\n", __func__, (unsigned long)(di_data->in_flag_phy));
			dprintk(DEBUG_DATA_INFO, "%s: out_flag_phy = 0x%lx\n", __func__, (unsigned long)(di_data->out_flag_phy));

			if (0 == field)
				ret = di_set_para(di_para, di_data->in_flag_phy, di_data->out_flag_phy, field);
			else
				ret = di_set_para(di_para, di_data->out_flag_phy, di_data->in_flag_phy, field);
			if (ret) {
				printk(KERN_ERR "%s: deinterlace work failed.\n", __func__);
				return -1;
			} else {
				di_irq_enable(1);
				di_start();
				mod_timer(s_timer, jiffies + msecs_to_jiffies(di_data->time_value));
			}

			if (!(filp->f_flags & O_NONBLOCK)) {
				ret = wait_event_interruptible(di_data->wait, (di_complete_check_get() != 1));
				if (ret)
					return ret;
			}
			ret = di_complete_check_get();
		}
		break;
	default:
		break;
	}

	dprintk(DEBUG_TEST, "%s: out!!\n", __func__);
	return ret;
}

static int sunxi_di_open(struct inode *inode, struct file *file)
{
	s32 ret = 0;

	dprintk(DEBUG_DATA_INFO, "%s: enter!!\n", __func__);

	atomic_set(&di_data->enable, 1);

	di_data->flag_size = (FLAG_WIDTH*FLAG_HIGH)/4;

	di_data->mem_in_params.size = di_data->flag_size;
	ret = di_mem_request(&(di_data->mem_in_params));
	if ( ret < 0 ) {
		printk(KERN_ERR "%s: request in_flag mem failed\n", __func__);
		return -1;
	} else {
		di_data->in_flag_phy = (void *)di_data->mem_in_params.p_addr;
	}

	di_data->mem_out_params.size = di_data->flag_size;
	ret = di_mem_request(&(di_data->mem_out_params));
	if ( ret < 0 ) {
		printk(KERN_ERR "%s: request out_flag mem failed\n", __func__);
		di_mem_release(&(di_data->mem_in_params));
		return -1;
	} else {
		di_data->out_flag_phy = (void *)di_data->mem_out_params.p_addr;
	}

	ret = di_clk_enable();
	if (ret) {
		di_mem_release(&(di_data->mem_in_params));
		di_mem_release(&(di_data->mem_out_params));
		printk(KERN_ERR "%s: enable clk failed.\n", __func__);
		return ret;
	}
	di_internal_clk_enable();
	di_set_init();

	return 0;
}

static int sunxi_di_release(struct inode *inode, struct file *file)
{
	dprintk(DEBUG_DATA_INFO, "%s: enter!!\n", __func__);

	atomic_set(&di_data->enable, 0);

	di_irq_enable(0);
	di_reset();
	di_internal_clk_disable();
	di_clk_disable();
	if (NULL != di_data->mem_in_params.v_addr)
		di_mem_release(&(di_data->mem_in_params));
	if (NULL != di_data->mem_out_params.v_addr)
		di_mem_release(&(di_data->mem_out_params));

	return 0;
}

static const struct file_operations sunxi_di_fops = {
	.owner = THIS_MODULE,
	.llseek = noop_llseek,
	.unlocked_ioctl = sunxi_di_ioctl,
	.open = sunxi_di_open,
	.release = sunxi_di_release,
};

static int sunxi_di_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	s32 ret;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	if (!of_device_is_available(node)) {
		printk("%s: di status disable!!\n", __func__);
		return -EPERM;
	}

	di_data = kzalloc(sizeof(*di_data), GFP_KERNEL);
	if (di_data == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	atomic_set(&di_data->di_complete, 0);
	atomic_set(&di_data->enable, 0);
	di_data->mem_in_params.v_addr = NULL;
	di_data->mem_out_params.v_addr = NULL;

	init_waitqueue_head(&di_data->wait);

	s_timer = kmalloc(sizeof(struct timer_list), GFP_KERNEL);
	if (!s_timer) {
		kfree(di_data);
		ret =  - ENOMEM;
		printk(KERN_ERR " %s FAIL TO  Request Time\n", __func__);
		return -1;
	}
	init_timer(s_timer);
	s_timer->function = &di_timer_handle;
	di_data->time_value = DI_TIMEOUT;

	di_wq = create_singlethread_workqueue("di_wq");
	if (!di_wq) {
		printk(KERN_ERR "Creat DE-Interlace workqueue failed.\n");
		ret = -ENOMEM;
		goto create_work_err;
	}

	INIT_WORK(&di_work, di_work_func);

	if (sunxi_di_major == -1) {
		if ((sunxi_di_major = register_chrdev (0, DI_MODULE_NAME, &sunxi_di_fops)) < 0) {
			printk(KERN_ERR "%s: Failed to register character device\n", __func__);
			ret = -1;
			goto register_chrdev_err;
		} else
			dprintk(DEBUG_INIT, "%s: sunxi_di_major = %d\n", __func__, sunxi_di_major);
	}

	di_dev_class = class_create(THIS_MODULE, DI_MODULE_NAME);
	if (IS_ERR(di_dev_class))
		return -1;
	di_device = device_create(di_dev_class, NULL,  MKDEV(sunxi_di_major, 0), NULL, DI_MODULE_NAME);

	ret = sunxi_di_params_init(pdev);
	if (ret) {
		printk(KERN_ERR "%s di init params failed!\n", __func__);
		goto init_para_err;
	}

#ifdef CONFIG_PM
	di_data->di_pm_domain.ops.suspend = sunxi_di_suspend;
	di_data->di_pm_domain.ops.resume = sunxi_di_resume;
	di_device->pm_domain = &(di_data->di_pm_domain);
#endif

	ret = sysfs_create_group(&di_device->kobj, &di_attribute_group);
	if (ret) {
		printk(KERN_ERR "%s di_attribute_group create failed!\n", __func__);
		return ret;
	}

	return 0;

init_para_err:
	if (sunxi_di_major > 0) {
		device_destroy(di_dev_class, MKDEV(sunxi_di_major, 0));
		class_destroy(di_dev_class);
		unregister_chrdev(sunxi_di_major, DI_MODULE_NAME);
	}
register_chrdev_err:
	cancel_work_sync(&di_work);
	if (NULL != di_wq) {
		flush_workqueue(di_wq);
		destroy_workqueue(di_wq);
		di_wq = NULL;
	}
create_work_err:
	kfree(s_timer);
	kfree(di_data);

	return ret;
}

static int sunxi_di_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&di_device->kobj, &di_attribute_group);
	sunxi_di_params_exit();
	if (sunxi_di_major > 0) {
		device_destroy(di_dev_class, MKDEV(sunxi_di_major, 0));
		class_destroy(di_dev_class);
		unregister_chrdev(sunxi_di_major, DI_MODULE_NAME);
	}
	cancel_work_sync(&di_work);
	if (NULL != di_wq) {
		flush_workqueue(di_wq);
		destroy_workqueue(di_wq);
		di_wq = NULL;
	}
	kfree(s_timer);
	kfree(di_data);

	printk(KERN_INFO "%s: module unloaded\n", __func__);

	return 0;
}

static const struct of_device_id sunxi_di_match[] = {
	 { .compatible = "allwinner,sunxi-deinterlace", },
	 {},
};
MODULE_DEVICE_TABLE(of, sunxi_di_match);


static struct platform_driver di_platform_driver = {
	.probe  = sunxi_di_probe,
	.remove = sunxi_di_remove,
	.driver = {
		.name	= DI_MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_di_match,
	},
};

static int __init sunxi_di_init(void)
{
	return platform_driver_register(&di_platform_driver);
}

static void __exit sunxi_di_exit(void)
{
	platform_driver_unregister(&di_platform_driver);
}

module_init(sunxi_di_init);
module_exit(sunxi_di_exit);
module_param_named(debug_mask, debug_mask, int, 0644);
MODULE_DESCRIPTION("DE-Interlace driver");
MODULE_AUTHOR("Ming Li<liming@allwinnertech.com>");
MODULE_LICENSE("GPL");

