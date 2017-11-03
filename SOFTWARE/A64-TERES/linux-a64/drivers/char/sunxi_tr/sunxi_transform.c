/* linux/drivers/char/sunxi_tr/sunxi_tr.c
 *
 * Copyright (c) 2014 Allwinnertech Co., Ltd.
 * Author: Tyle <tyle@allwinnertech.com>
 *
 * Transform driver for sunxi platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "transform.h"

#define TR_CHAN 1
//#define TR_POLL
//#define TR_CHECK_THREAD

struct sunxi_transform
{
	int id; /* chan id */
	struct list_head list;
	bool requested; /* indicate if have request */
	bool busy; /* at busy state when transforming */
	bool error;/* indicate some error happens in this channel */
	unsigned long start_time; /* the time when starting transform */
	unsigned long timeout;//ms
	tr_info info;
};

struct sunxi_trdev
{
	struct device       *dev;
	void __iomem        *base;
	int                 irq;
	struct clk          *clk;   /* clock gate for tr */
	spinlock_t          slock;
	struct mutex        mlock;
	struct list_head    trs;    /* transform chan list */
	unsigned int        count;  /* transform channel counter */
	struct sunxi_transform *cur_tr; /* curent transform channel processing */
	bool busy;
	struct task_struct *task;
};

static struct sunxi_trdev *gsunxi_dev = NULL;

static struct cdev *tr_cdev;
static dev_t devid ;
static struct class *tr_class;
static struct device *tr_dev;
static bool dev_init = false;

//#define struct sunxi_trdev *to_sunxi_trdev(dev) container_of(dev, struct sunxi_trdev, dev)
static int sunxi_tr_finish_procss(void);

#if !defined(CONFIG_OF)
static struct resource tr_resource[] = {
	[0] = {
		.start = (int __force)SUNXI_DE_VBASE,
		.end = (int __force)(SUNXI_DE_VBASE + SUNXI_DE_SIZE),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = (int __force)SUNXI_IRQ_DEIRQ1,
		.end = (int __force)SUNXI_IRQ_DEIRQ1,
		.flags = IORESOURCE_IRQ,
	},
};
#endif

#if defined(TR_CHECK_THREAD)
static int tr_thread(void *parg)
{
	while (1) {
		struct sunxi_transform *tr = NULL;
		unsigned long timeout = 0;

		if(kthread_should_stop()) {
			break;
		}
		tr = gsunxi_dev->cur_tr;
		if(tr) {
			timeout = tr->start_time + msecs_to_jiffies(tr->timeout);
			if (time_after_eq(jiffies, timeout)) {
				pr_warn("%s, timeout(%d ms)\n", __func__, jiffies_to_msecs(jiffies - tr->start_time));
				de_tr_reset();
				tr->busy = false;
				tr->error = true;

				sunxi_tr_finish_procss();
			}
		}

		msleep(10);
	}

	return 0;
}
#endif

static int tr_check_timeout(void)
{
	struct sunxi_transform *tr = NULL;
	unsigned long timeout = 0;
	unsigned long flags;

	tr = gsunxi_dev->cur_tr;
	if(NULL == tr) {
		return 0;
	}

	spin_lock_irqsave(&gsunxi_dev->slock, flags);
	timeout = tr->start_time + msecs_to_jiffies(tr->timeout);
	if (tr->busy && time_after_eq(jiffies, timeout)) {
		tr->busy = false;
		tr->error = true;
		spin_unlock_irqrestore(&gsunxi_dev->slock, flags);
		de_tr_exception();
		pr_warn("%s, timeout(%d ms)\n", __func__, jiffies_to_msecs(jiffies - tr->start_time));
		sunxi_tr_finish_procss();
	} else {
		spin_unlock_irqrestore(&gsunxi_dev->slock, flags);
	}

	return 0;
}

/* find a tr which has request and a longest time to process */
static struct sunxi_transform * tr_find_proper_task(void)
{
	struct sunxi_transform *tr = NULL, *proper_tr = NULL;
	unsigned long min_time = jiffies;

	list_for_each_entry(tr, &gsunxi_dev->trs, list) {
		bool condition1 = (true == tr->requested);
		bool condition2 = time_after_eq(min_time, tr->start_time);

		if(condition1 && condition2) {
			min_time = tr->start_time;
			proper_tr = tr;
		} else {
			//printk("find_task: %d,%d, %ld,%ld\n", condition1, condition2, min_time, tr->start_time);
		}
	}

	return proper_tr;
}

/* protect by @slock */
static int tr_process_next_proper_task(u32 from)
{
	unsigned long flags;
	struct sunxi_transform *tr = NULL;

	spin_lock_irqsave(&gsunxi_dev->slock, flags);
	if(gsunxi_dev->busy) {
		spin_unlock_irqrestore(&gsunxi_dev->slock, flags);
		return 0;
	}

	/* find a tr which has request */
	tr = tr_find_proper_task();
	if(NULL != tr) {
		/* process request */
		gsunxi_dev->busy = true;
		tr->busy = true;
		tr->error = false;
		tr->start_time = jiffies;
		tr->requested = false;

		gsunxi_dev->cur_tr = tr;
	}
	spin_unlock_irqrestore(&gsunxi_dev->slock, flags);

	if(NULL != tr)
		de_tr_set_cfg(&tr->info);

	return 0;
}

static int sunxi_tr_finish_procss(void)
{
	unsigned long flags;

	/* correct interrupt */
	spin_lock_irqsave(&gsunxi_dev->slock, flags);
	if(gsunxi_dev->cur_tr) {
		gsunxi_dev->cur_tr->busy = false;
		gsunxi_dev->cur_tr = NULL;
		gsunxi_dev->busy = false;
	}
	spin_unlock_irqrestore(&gsunxi_dev->slock, flags);

	tr_process_next_proper_task(0);

	return 0;
}

/*
 * sunxi_tr_request - request transform channel
 * On success, returns transform handle.  On failure, returns 0.
 */
unsigned long sunxi_tr_request(void)
{
	struct sunxi_transform* tr = NULL;
	unsigned long flags;
	unsigned int count = 0;

	tr = kzalloc(sizeof(struct sunxi_transform), GFP_KERNEL);
	if (!tr) {
		pr_warn("alloc fail\n");
    return 0;
	}

	tr->requested = false;
	tr->busy = false;
	tr->error = false;
	tr->timeout = 50; //default 50ms timeout
	tr->start_time = jiffies;
	spin_lock_irqsave(&gsunxi_dev->slock, flags);
	list_add_tail(&tr->list, &gsunxi_dev->trs);
	gsunxi_dev->count ++;
	count = gsunxi_dev->count;
	spin_unlock_irqrestore(&gsunxi_dev->slock, flags);

	mutex_lock(&gsunxi_dev->mlock);
	if(1 == count) {
		if(gsunxi_dev->clk)
			clk_prepare_enable(gsunxi_dev->clk);
		de_tr_init();

#if defined(TR_CHECK_THREAD)
		gsunxi_dev->task = kthread_create(tr_thread, (void*)0, "tr_thread");
		if(IS_ERR(gsunxi_dev->task)) {
			pr_warn("Unable to start kernel thread %s.\n","tr_thread");
			gsunxi_dev->task = NULL;
		} else
			wake_up_process(gsunxi_dev->task);
#endif
	}
	mutex_unlock(&gsunxi_dev->mlock);
	pr_info("%s, count=%d\n", __func__, count);

	return (unsigned long)tr;
}
EXPORT_SYMBOL_GPL(sunxi_tr_request);
/*
 * sunxi_tr_release - release transform channel
 * @hdl: transform handle which return by sunxi_tr_request
 * On success, returns 0. On failure, returns ERR_PTR(-errno).
 */
int sunxi_tr_release(unsigned long hdl)
{
	struct sunxi_transform* tr = (struct sunxi_transform*)hdl;
	unsigned long flags;
	unsigned int count = 0;

	if(NULL == tr) {
		pr_warn("%s, hdl is NULL!\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&gsunxi_dev->slock, flags);
	list_del(&tr->list);
	gsunxi_dev->count --;
	count = gsunxi_dev->count;
	kfree((void*)tr);
	spin_unlock_irqrestore(&gsunxi_dev->slock, flags);

	mutex_lock(&gsunxi_dev->mlock);
	if(0 == count) {
#if defined(TR_CHECK_THREAD)
		kthread_stop(gsunxi_dev->task);
		gsunxi_dev->task = NULL;
#endif
		de_tr_exit();
		if(gsunxi_dev->clk)
			clk_disable(gsunxi_dev->clk);
	}
	mutex_unlock(&gsunxi_dev->mlock);
	pr_info("%s, count=%d\n", __func__, count);

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_tr_release);
/*
 * sunxi_tr_commit - commit an transform request
 * @hdl: transform handle which return by sunxi_tr_request
 * On success, returns fd. On failure, returns ERR_PTR(-errno).
 */
int sunxi_tr_commit(unsigned long hdl, tr_info *info)
{
	struct sunxi_transform* tr = (struct sunxi_transform*)hdl;
	int fd = 0;

	if(NULL == tr) {
		pr_warn("%s, hdl is NULL\n", __func__);
		return -EINVAL;
	}

#if 0
	pr_info("in: fmt=%d, pitch=%d,%d,%d, rect=%d,%d,%d,%d\n",
	    info->src_frame.fmt, info->src_frame.pitch[0], info->src_frame.pitch[1], info->src_frame.pitch[2],
	    info->src_rect.x, info->src_rect.y, info->src_rect.w, info->src_rect.h);
	pr_info("out: fmt=%d, pitch=%d,%d,%d, rect=%d,%d,%d,%d\n",
	    info->dst_frame.fmt, info->dst_frame.pitch[0], info->dst_frame.pitch[1], info->dst_frame.pitch[2],
	    info->dst_rect.x, info->dst_rect.y, info->dst_rect.w, info->dst_rect.h);
#endif

	if(!tr->requested && !tr->busy) {
		memcpy(&tr->info, info, sizeof(tr_info));
		tr->requested = true;

		tr_process_next_proper_task(1);
	}

#if defined(TR_POLL)
{
	int wait_cnt = 5, delay_ms = 10, i = 0;

	while((i < wait_cnt) && (0 != de_tr_irq_query())) {
		msleep(delay_ms);
		i ++;
	}
	if(wait_cnt >= 5)
		pr_warn("%s, timeout !!!\n", __func__);
	sunxi_tr_finish_procss();
}
#endif

	return fd;
}
EXPORT_SYMBOL_GPL(sunxi_tr_commit);
/*
 * sunxi_tr_query - query transform status
 * @hdl: transform handle which return by sunxi_tr_request
 * On finish, returns 0. On failure, returns ERR_PTR(-errno).
 * On busy,returns 1.
 */
int sunxi_tr_query(unsigned long hdl)
{
	struct sunxi_transform* tr = (struct sunxi_transform*)hdl;
	int status = 0;

	if(NULL == tr) {
		pr_warn("%s, hdl is NULL!\n", __func__);
		return -EINVAL;
	}
#if !defined(TR_CHECK_THREAD)
	tr_check_timeout();
#endif

	if(tr->requested || tr->busy)
		status = 1; /* busy */
	else if(tr->error)
		status = -1; /*error:timeout */
	else
		status = 0;/* finish */

	return status;

#if 0
	timeout = tr->start_time + msecs_to_jiffies(tr->timeout);
	if (time_after_eq(jiffies, timeout)) {
		pr_warn("%s, timeout(%d ms)\n", __func__, jiffies_to_msecs(jiffies - tr->start_time));
		de_tr_reset();
		tr->busy = false;
		return -1;
	}

	return (tr->busy?1:0);
#endif
}
EXPORT_SYMBOL_GPL(sunxi_tr_query);
/*
 * sunxi_tr_set_timeout - set transform timeout(ms)
 * @hdl: transform hdl
 * @timeout: time(ms)
 * On success, returns 0.  On failure, returns ERR_PTR(-errno).
 */
int sunxi_tr_set_timeout(unsigned long hdl, unsigned long timeout /* ms */)
{
	struct sunxi_transform* tr = (struct sunxi_transform*)hdl;

	if(NULL == tr) {
		pr_warn("%s, hdl is NULL!\n", __func__);
		return -EINVAL;
	}

	if(timeout == 0) {
		pr_warn("%s, para error(timeout=0)!\n", __func__);
		return -EINVAL;
	}
	tr->timeout = timeout;

	return 0;
}

static irqreturn_t sunxi_tr_interrupt(int irq, void *dev_id)
{
	int ret = 0;

	/* get irq status */
	ret = de_tr_irq_query();
	if(0 == ret) {
		sunxi_tr_finish_procss();
	}
	/* clear irq status */
	return IRQ_HANDLED;
}

static int tr_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int tr_release(struct inode *inode, struct file *file)
{
	return 0;
}
static ssize_t tr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t tr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return 0;
}

//static int __devinit tr_probe(struct platform_device *pdev)
static int tr_probe(struct platform_device *pdev)
{
#if !defined(CONFIG_OF)
	struct resource	*res;
#endif
	struct sunxi_trdev *sunxi_dev = NULL;
	int ret = 0;
	int irq;

	pr_info("enter %s\n", __func__);

	sunxi_dev = kzalloc(sizeof(struct sunxi_trdev), GFP_KERNEL);
	if (!sunxi_dev)
		return -ENOMEM;

	/* register base */
#if !defined(CONFIG_OF)
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
			dev_err(&pdev->dev, "get reource MEM fail\n");
	    ret = -EINVAL;
	    goto io_err;
	}
	sunxi_dev->base = (void __iomem *)res->start;
#else
	sunxi_dev->base = of_iomap(pdev->dev.of_node, 0);
	if (NULL == sunxi_dev->base) {
		dev_err(&pdev->dev, "unable to map transform registers\n");
		ret = -EINVAL;
		goto io_err;
	}
#endif
	/* irq */
#if !defined(CONFIG_OF)
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		ret = irq;
		goto io_err;
	}
#else
	irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (!irq) {
		dev_err(&pdev->dev, "irq_of_parse_and_map irq fail for transform\n");
	}
#endif
	ret = request_irq(irq, sunxi_tr_interrupt, IRQF_SHARED,
					dev_name(&pdev->dev), sunxi_dev);
	if (ret) {
		dev_err(&pdev->dev, "NO IRQ found!!!\n");
		goto iomap_err;
	}

	/* clk init */
	sunxi_dev->clk = of_clk_get(pdev->dev.of_node, 0);
	if (IS_ERR(sunxi_dev->clk)) {
		dev_err(&pdev->dev, "fail to get clk\n");
	}

	gsunxi_dev = sunxi_dev;
	platform_set_drvdata(pdev, sunxi_dev);
	INIT_LIST_HEAD(&sunxi_dev->trs);
	spin_lock_init(&sunxi_dev->slock);
	mutex_init(&sunxi_dev->mlock);
	sunxi_dev->dev = &pdev->dev;

	dev_init = true;
	/* init hw */
	de_tr_set_base((uintptr_t)sunxi_dev->base);

	pr_info("exit %s\n", __func__);
	return 0;

iomap_err:
	iounmap(sunxi_dev->base);
io_err:
	kfree(sunxi_dev);

	return ret;
}

static int tr_remove(struct platform_device *pdev)
{
	pr_info("tr_remove call\n");

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static int tr_suspend(struct platform_device *pdev, pm_message_t state)
{
	pr_info("enter %s\n", __func__);

	pr_info("exit %s\n", __func__);

	return 0;
}


static int tr_resume(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);

    if(gsunxi_dev->count != 0)
        de_tr_init();
	pr_info("exit %s\n", __func__);

	return 0;
}

static void tr_shutdown(struct platform_device *pdev)
{

	return ;
}

static long tr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long karg[4];
	unsigned long ubuffer[4] = {0};
	long ret = -1;

	if (copy_from_user((void*)karg,(void __user*)arg,4*sizeof(unsigned long))) {
		pr_warn("copy_from_user fail\n");
		return -EFAULT;
	}

	ubuffer[0] = *(unsigned long*)karg;
	ubuffer[1] = (*(unsigned long*)(karg+1));
	ubuffer[2] = (*(unsigned long*)(karg+2));
	ubuffer[3] = (*(unsigned long*)(karg+3));

	switch(cmd)	{
	case TR_REQUEST:
	{
		/* request a chan */
		struct sunxi_transform* tr = NULL;

		tr = (struct sunxi_transform*)sunxi_tr_request();
		if (copy_to_user((void __user *)ubuffer[0],&tr, sizeof(struct sunxi_transform*))) {
			pr_warn("copy_from_user fail\n");
			return  -EFAULT;
		}

		if(NULL == tr)
			ret = -EFAULT;
		else
			ret = 0;
		break;
	}

	case TR_COMMIT:
	{
		tr_info info;

		if(copy_from_user(&info, (void __user *)ubuffer[1],sizeof(tr_info)))	{
			pr_warn("%s, copy_from_user fail\n", __func__);
			return  -EFAULT;
		}
		ret = sunxi_tr_commit(ubuffer[0], &info);

		break;
	}

	case TR_RELEASE:
	{
		/* release a chan */
		ret = sunxi_tr_release(ubuffer[0]);
		break;
	}

	case TR_QUERY:
	{
		/* query status */
		ret = sunxi_tr_query(ubuffer[0]);
		break;
	}

	case TR_SET_TIMEOUT:
	{
		/* set timeout */
		ret = sunxi_tr_set_timeout(ubuffer[0], ubuffer[1]);
		break;
	}

	default:
		break;
	}

  return ret;
}

static const struct file_operations tr_fops = {
	.owner    = THIS_MODULE,
	.open     = tr_open,
	.release  = tr_release,
	.write    = tr_write,
	.read     = tr_read,
	.unlocked_ioctl = tr_ioctl,
};

#if !defined(CONFIG_OF)
static struct platform_device tr_device = {
	.name           = "transform",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(tr_resource),
	.resource       = tr_resource,
	.dev            =
	{
	},
};
#else
static const struct of_device_id sunxi_tr_match[] = {
	{ .compatible = "allwinner,sun50i-tr", },
	{},
};
#endif

static struct platform_driver tr_driver = {
	.probe    = tr_probe,
	.remove   = tr_remove,
	.suspend  = tr_suspend,
	.resume   = tr_resume,
	.shutdown = tr_shutdown,
	.driver   =
	{
		.name   = "transform",
		.owner  = THIS_MODULE,
		.of_match_table = sunxi_tr_match,
	},
};

static int __init tr_module_init(void)
{
	int ret = 0, err;

	alloc_chrdev_region(&devid, 0, 1, "transform");
	tr_cdev = cdev_alloc();
	cdev_init(tr_cdev, &tr_fops);
	tr_cdev->owner = THIS_MODULE;
	err = cdev_add(tr_cdev, devid, 1);
	if (err) {
		pr_warn("cdev_add fail\n");
		return -1;
	}

	tr_class = class_create(THIS_MODULE, "transform");
	if (IS_ERR(tr_class))	{
		pr_warn("class_create fail\n");
		return -1;
	}

	tr_dev = device_create(tr_class, NULL, devid, NULL, "transform");
#if !defined(CONFIG_OF)
	ret = platform_device_register(&tr_device);
#endif

	if (ret == 0) {
		ret = platform_driver_register(&tr_driver);
	}

	return ret;
}

static void __exit tr_module_exit(void)
{
	platform_driver_unregister(&tr_driver);
#if !defined(CONFIG_OF)
	platform_device_unregister(&tr_device);
#endif
	device_destroy(tr_class,  devid);
	class_destroy(tr_class);

	cdev_del(tr_cdev);
}

module_init(tr_module_init);
module_exit(tr_module_exit);

MODULE_AUTHOR("tyle");
MODULE_DESCRIPTION("transform driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:transform");


