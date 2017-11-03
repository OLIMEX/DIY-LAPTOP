/*
 *  TI Bluesleep driver
 *	Kernel module responsible for Wake up of Host
 *  Copyright (C) 2009-2010 Texas Instruments


 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Copyright (C) 2006-2007 - Motorola
 * Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 *  Date         Author           Comment
 * -----------  --------------   --------------------------------
 * 2006-Apr-28  Motorola         The kernel module for running the Bluetooth(R)
 *                               Sleep-Mode Protocol from the Host side
 * 2006-Sep-08  Motorola         Added workqueue for handling sleep work.
 * 2007-Jan-24  Motorola         Added mbm_handle_ioi() call to ISR.
 * 2009-Aug-10  Motorola         Changed "add_timer" to "mod_timer" to solve
 *                               race when flurry of queued work comes in.
*/

#include <linux/module.h>       /* kernel module definitions */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>

#include <linux/gpio.h>
#include <linux/sys_config.h>
#include <linux/of_gpio.h>

#include <net/bluetooth/bluetooth.h>

#define BT_SLEEP_DBG
#undef  BT_DBG
#undef  BT_ERR
#ifdef  BT_SLEEP_DBG
#define BT_DBG(fmt, arg...) printk(KERN_DEBUG "[BT_LPM] %s: " fmt "\n" , __func__ , ## arg)
#else
#define BT_DBG(fmt, arg...)
#endif
#define BT_ERR(fmt, arg...) printk(KERN_ERR "[BT_LPM] %s: " fmt "\n" , __func__ , ## arg)

/*
 * Defines
 */
#define VERSION	 "1.1"
#define PROC_DIR	"bluetooth/sleep"

#define DEFAULT_UART_INDEX   1


static void bluesleep_stop(void);
static int bluesleep_start(void);

struct bluesleep_info {
	unsigned host_wake;
	unsigned host_wake_irq;
	struct uart_port *uport;

	unsigned host_wake_assert;
};


/* state variable names and bit positions */
#define BT_PROTO      	0x00
#define BT_ACTIVE		0x02
#define BT_SUSPEND		0x04

/* variable use indicate lpm modle */
static bool has_lpm_enabled = false;

/* struct use save platform_device from uart */
static struct platform_device *bluesleep_uart_dev;

static struct bluesleep_info *bsi;

/* module usage */
static atomic_t open_count = ATOMIC_INIT(1);

/*
 * Global variables
 */

/** Global state flags */
static unsigned long flags;

struct proc_dir_entry *bluetooth_dir, *sleep_dir;

static struct uart_port *bluesleep_get_uart_port(void)
{
	struct uart_port *uport = NULL;
	if (bluesleep_uart_dev){
		uport = platform_get_drvdata(bluesleep_uart_dev);
		if(uport)
			BT_DBG("%s get uart_port from blusleep_uart_dev: %s, port irq: %d", 
					__FUNCTION__, bluesleep_uart_dev->name, uport->irq);
	}

	return uport;
}

static int bluesleep_lpm_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "lpm enable: %d\n", has_lpm_enabled);
	return 0;
}

static int bluesleep_lpm_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, bluesleep_lpm_proc_show, NULL);
}

static ssize_t bluesleep_write_proc_lpm(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)

{
	char b;

	if (count < 1)
		return -EINVAL;

	if (copy_from_user(&b, buffer, 1))
		return -EFAULT;

	if (b == '0') {
		/* HCI_DEV_UNREG */
		bluesleep_stop();
		has_lpm_enabled = false;
		bsi->uport = NULL;
	} else {
		/* HCI_DEV_REG */
		if (!has_lpm_enabled) {
			has_lpm_enabled = true;
			if (bluesleep_uart_dev)
				bsi->uport = bluesleep_get_uart_port();

			/* if bluetooth started, start bluesleep*/
			bluesleep_start();
		}
	}

	return count;
}

static const struct file_operations lpm_fops = {
	.owner		= THIS_MODULE,
	.open		= bluesleep_lpm_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= bluesleep_write_proc_lpm,
};

/**
 * Schedules a tasklet to run when receiving an interrupt on the
 * <code>HOST_WAKE</code> GPIO pin.
 * @param irq Not used.
 * @param dev_id Not used.
 */
static irqreturn_t bluesleep_hostwake_isr(int irq, void *dev_id)
{
	BT_DBG("%s", __func__);
	
	return 0;
}

/**
 * Starts the Sleep-Mode Protocol on the Host.
 * @return On success, 0. On error, -1, and <code>errno</code> is set
 * appropriately.
 */
static int bluesleep_start(void)
{
	int retval;
	
	BT_DBG("bluesleep_acquire irq\n");
	if (test_bit(BT_PROTO, &flags)) {
		return 0;
	}

	if (!atomic_dec_and_test(&open_count)) {
		atomic_inc(&open_count);
		return -EBUSY;
	}

	if(bsi->host_wake_assert == 0){
		retval = request_irq(bsi->host_wake_irq, bluesleep_hostwake_isr, IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "bluetooth hostwake", NULL);
	}else{
		retval = request_irq(bsi->host_wake_irq, bluesleep_hostwake_isr, IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND, "bluetooth hostwake", NULL);
	}
	if (retval < 0) {
		BT_ERR("Couldn't acquire bt_host_wake IRQ or enable it");
		goto fail;
	}

	set_bit(BT_PROTO, &flags);

	return 0;
fail:
	atomic_inc(&open_count);
	return retval;
}

/**
 * Stops the Sleep-Mode Protocol on the Host.
 */
static void bluesleep_stop(void)
{
	BT_DBG("%s", __func__);
	if (!test_bit(BT_PROTO, &flags)) {
		return;
	}
	clear_bit(BT_PROTO, &flags);
	atomic_inc(&open_count);

	disable_irq_wake(bsi->host_wake_irq);
	free_irq(bsi->host_wake_irq, NULL);
}

extern struct platform_device *sw_uart_get_pdev(int uart_id);
static int __init bluesleep_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct gpio_config config;
	int ret,uart_index;
	u32 val;

	bsi = devm_kzalloc(&pdev->dev,sizeof(struct bluesleep_info), GFP_KERNEL);
	if (!bsi)
		return -ENOMEM;

	bsi->host_wake = of_get_named_gpio_flags(np, "bt_hostwake", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(bsi->host_wake)) {
		BT_ERR("get gpio bt_hostwake failed\n");
		return -EINVAL;
	}

	BT_DBG("bt_hostwake gpio=%d  mul-sel=%d  pull=%d  drv_level=%d  data=%d\n",
			config.gpio,
			config.mul_sel,
			config.pull,
			config.drv_level,
			config.data);

	ret = devm_gpio_request(dev, bsi->host_wake, "bt_hostwake");
	if (ret < 0) {
		BT_ERR("can't request bt_hostwake gpio %d\n",
			bsi->host_wake);
		return ret;
	}
	ret = gpio_direction_input(bsi->host_wake);
	if (ret < 0) {
		BT_ERR("can't request input direction bt_wake gpio %d\n",
			bsi->host_wake);
		return ret;
	}

	//set ext_wake_assert and host_wake_assert
	bsi->host_wake_assert = config.data;

	//2.get bt_host_wake gpio irq
	bsi->host_wake_irq = gpio_to_irq(bsi->host_wake);
	if (IS_ERR_VALUE(bsi->host_wake_irq)) {
		BT_ERR("map gpio [%d] to virq failed, errno = %d\n",bsi->host_wake, bsi->host_wake_irq);
		ret = -ENODEV;
		return ret;
	}

	uart_index = DEFAULT_UART_INDEX;
	if (!of_property_read_u32(np, "uart_index", &val)) {
		switch (val) {
		case 0:
		case 1:
		case 2:
			uart_index = val;
			break;
		default:
			BT_ERR("unsupported uart_index (%u)\n", val);
		}
	}
	BT_DBG("uart_index (%u)\n", uart_index);
	bluesleep_uart_dev = sw_uart_get_pdev(uart_index);

	clear_bit(BT_SUSPEND, &flags);
	set_bit(BT_ACTIVE, &flags);

	return 0;
}

static int bluesleep_remove(struct platform_device *pdev)
{
	BT_DBG("%s", __func__);
	if (!test_bit(BT_PROTO, &flags)) {
		return 0;
	}

	disable_irq_wake(bsi->host_wake_irq);
	free_irq(bsi->host_wake_irq, NULL);

	return 0;
}

static int bluesleep_resume(struct platform_device *pdev)
{
	BT_DBG("%s", __func__);
	if (test_bit(BT_SUSPEND, &flags)) {
		disable_irq_wake(bsi->host_wake_irq);
    
		clear_bit(BT_SUSPEND, &flags);
		set_bit(BT_ACTIVE, &flags);
	}

	return 0;
}

static int bluesleep_suspend(struct platform_device *pdev, pm_message_t state)
{
	BT_DBG("%s", __func__); 
	if(test_bit(BT_ACTIVE, &flags)) {
		enable_irq_wake(bsi->host_wake_irq);
		
		clear_bit(BT_ACTIVE, &flags);
		set_bit(BT_SUSPEND, &flags);
	}
	
	return 0;
}

static const struct of_device_id sunxi_btlpm_ids[] = {
	{ .compatible = "allwinner,sunxi-btlpm" },
	{ /* Sentinel */ }
};

static struct platform_driver bluesleep_driver = {
	.remove	= bluesleep_remove,
	.suspend = bluesleep_suspend,
	.resume = bluesleep_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "sunxi-btlpm",
		.of_match_table	= sunxi_btlpm_ids,
	},
};

/**
 * Initializes the module.
 * @return On success, 0. On error, -1, and <code>errno</code> is set
 * appropriately.
 */
static int __init bluesleep_init(void)
{
	int retval;
	struct proc_dir_entry *ent;

	BT_DBG("BlueSleep Mode Driver Ver %s", VERSION);

	/* clear all status bits */
	flags = 0;

	retval = platform_driver_probe(&bluesleep_driver, bluesleep_probe);
	if (retval)
		return retval;

	bluetooth_dir = proc_mkdir("bluetooth", NULL);
	if (bluetooth_dir == NULL) {
		BT_ERR("Unable to create /proc/bluetooth directory");
		goto unreg_drv;
	}

	sleep_dir = proc_mkdir("sleep", bluetooth_dir);
	if (sleep_dir == NULL) {
		BT_ERR("Unable to create /proc/%s directory", PROC_DIR);
		goto rm_bt_dir;
	}
	
	ent = proc_create("lpm", 0, sleep_dir,&lpm_fops);
	if (ent == NULL) {
		BT_ERR("Unable to create /proc/%s/lpm entry", PROC_DIR);
		goto rm_sleep_dir;
	}

	return 0;

rm_sleep_dir:
	remove_proc_entry("sleep", bluetooth_dir);
rm_bt_dir:
	remove_proc_entry("bluetooth", 0);
unreg_drv:
	platform_driver_unregister(&bluesleep_driver);

	return retval;
}

/**
 * Cleans up the module.
 */
static void __exit bluesleep_exit(void)
{ 
	platform_driver_unregister(&bluesleep_driver);
	
	remove_proc_entry("lpm", sleep_dir);
	remove_proc_entry("sleep", bluetooth_dir);
	remove_proc_entry("bluetooth", 0);
}

module_init(bluesleep_init);
module_exit(bluesleep_exit);

MODULE_DESCRIPTION("Bluetooth Sleep Mode Driver ver %s " VERSION);
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
