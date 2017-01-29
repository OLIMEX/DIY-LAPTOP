/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
*
* Copyright (c) 2014
*
* ChangeLog
*
*
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#if defined(CONFIG_PM)
#include <linux/pm.h>
#endif
#include "sunxi-keyboard.h"

static unsigned char keypad_mapindex[64] = {
	0,0,0,0,0,0,0,0,0,            	/* key 1, 0-8 */
	1,1,1,1,1,                 	/* key 2, 9-13 */
	2,2,2,2,2,2,                 	/* key 3, 14-19 */
	3,3,3,3,3,3,                   	/* key 4, 20-25 */
	4,4,4,4,4,4,4,4,4,4,4,          /* key 5, 26-36 */
	5,5,5,5,5,5,5,5,5,5,5,          /* key 6, 37-39 */
	6,6,6,6,6,6,6,6,6,           	/* key 7, 40-49 */
	7,7,7,7,7,7,7  			/* key 8, 50-63 */
};

#ifdef CONFIG_PM
static struct dev_pm_domain keyboard_pm_domain;
#endif

#define VOL_NUM KEY_MAX_CNT
#define KEY_BASSADDRESS (key_data->reg_base)

static u32 key_vol[VOL_NUM];
static u32 key_num = 0;
static u32 scankeycodes[KEY_MAX_CNT];
static u32 adc_measure;
static u32 adc_resol;
static struct sunxi_adc_disc disc_1350 = {
	.measure = 1350,
	.resol = 21,
};
static struct sunxi_adc_disc disc_2000 = {
	.measure = 2000,
	.resol = 31,
};

static volatile u32 key_val;
static struct sunxi_key_data *key_data;
static struct input_dev *sunxikbd_dev;
static u8 scancode;

static u8 key_cnt = 0;
static u8 compare_buffer[REPORT_START_NUM] = {0};
static u8 transfer_code = INITIAL_VALUE;

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_INT = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
};
static u32 debug_mask = 0;
#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	printk(fmt , ## arg)

module_param_named(debug_mask, debug_mask, int, 0644);

#ifdef CONFIG_OF
/*
 * Translate OpenFirmware node properties into platform_data
 */
static struct of_device_id sunxi_keyboard_of_match[] = {
	{ .compatible = "allwinner,keyboard_1350mv",
		.data = &disc_1350 },
	{ .compatible = "allwinner,keyboard_2000mv",
		.data = &disc_2000 },
	{ },
};
MODULE_DEVICE_TABLE(of, sunxi_keyboard_of_match);
#else /* !CONFIG_OF */
#endif

static void sunxi_keyboard_ctrl_set(enum key_mode key_mode, u32 para)
{
	u32 ctrl_reg = 0;

	if (0 != para)
		ctrl_reg = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_CTRL));

	if (CONCERT_DLY_SET & key_mode)
		ctrl_reg |= (FIRST_CONCERT_DLY & para);
	if (ADC_CHAN_SET & key_mode)
		ctrl_reg |= (ADC_CHAN_SELECT & para);
	if (KEY_MODE_SET & key_mode)
		ctrl_reg |= (KEY_MODE_SELECT & para);
	if (LRADC_HOLD_SET & key_mode)
		ctrl_reg |= (LRADC_HOLD_EN & para);
	if (LEVELB_VOL_SET & key_mode)
		ctrl_reg |= (LEVELB_VOL & para);
	if (LRADC_SAMPLE_SET & key_mode)
		ctrl_reg |= (LRADC_SAMPLE_250HZ & para);
	if (LRADC_EN_SET & key_mode)
		ctrl_reg |= (LRADC_EN & para);

	writel(ctrl_reg, (volatile void __iomem *)(KEY_BASSADDRESS + LRADC_CTRL));
}

static void sunxi_keyboard_int_set(enum int_mode int_mode, u32 para)
{
	u32 ctrl_reg = 0;

	if (0 != para)
		ctrl_reg = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INTC));

	if (ADC0_DOWN_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC0_DOWN_EN & para);
	if (ADC0_UP_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC0_UP_EN & para);
	if (ADC0_DATA_INT_SET & int_mode)
		ctrl_reg |= (LRADC_ADC0_DATA_EN & para);

	writel(ctrl_reg, (volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INTC));
}

static u32 sunxi_keyboard_read_ints(void)
{
	u32 reg_val;
	reg_val  = readl((const volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INT_STA));

	return reg_val;
}

static void sunxi_keyboard_clr_ints(u32 reg_val)
{
	writel(reg_val, (volatile void __iomem *)(KEY_BASSADDRESS + LRADC_INT_STA));
}

static u32 sunxi_keyboard_read_data(void)
{
	u32 reg_val;
	reg_val = readl(KEY_BASSADDRESS+LRADC_DATA0);

	return reg_val;
}

#ifdef CONFIG_PM
static int sunxi_keyboard_suspend(struct device *dev)
{
	dprintk(DEBUG_SUSPEND, "[%s] enter standby. \n", __FUNCTION__);

	disable_irq_nosync(key_data->irq_num);

	sunxi_keyboard_ctrl_set(0, 0);

	return 0;
}

static int sunxi_keyboard_resume(struct device *dev)
{
	unsigned long mode, para;

	dprintk(DEBUG_SUSPEND, "[%s] return from standby. \n", __FUNCTION__);

	mode = ADC0_DOWN_INT_SET | ADC0_UP_INT_SET | ADC0_DATA_INT_SET;
	para = LRADC_ADC0_DOWN_EN | LRADC_ADC0_UP_EN | LRADC_ADC0_DATA_EN;
	sunxi_keyboard_int_set(mode, para);
	mode = CONCERT_DLY_SET | ADC_CHAN_SET | KEY_MODE_SET | LRADC_HOLD_SET | LEVELB_VOL_SET \
			| LRADC_SAMPLE_SET | LRADC_EN_SET;
	para = FIRST_CONCERT_DLY|LEVELB_VOL|KEY_MODE_SELECT|LRADC_HOLD_EN|ADC_CHAN_SELECT \
			|LRADC_SAMPLE_250HZ|LRADC_EN;
	sunxi_keyboard_ctrl_set(mode, para);

	enable_irq(key_data->irq_num);

	return 0;
}
#endif

static irqreturn_t sunxi_isr_key(int irq, void *dummy)
{
	u32  reg_val = 0;
	int judge_flag = 0;

	dprintk(DEBUG_INT, "Key Interrupt\n");

	reg_val = sunxi_keyboard_read_ints();

	if (reg_val & LRADC_ADC0_DOWNPEND) {
		dprintk(DEBUG_INT, "key down\n");
	}

	if (reg_val & LRADC_ADC0_DATAPEND) {
		key_val = sunxi_keyboard_read_data();

		if (key_val < 0x3f) {

			compare_buffer[key_cnt] = key_val&0x3f;
		}

		if ((key_cnt + 1) < REPORT_START_NUM) {
			key_cnt++;
			/* do not report key message */

		} else {
			if(compare_buffer[0] == compare_buffer[1])
			{
			key_val = compare_buffer[1];
			scancode = keypad_mapindex[key_val&0x3f];
			judge_flag = 1;
			key_cnt = 0;
			} else {
				key_cnt = 0;
				judge_flag = 0;
			}

			if (1 == judge_flag) {
				dprintk(DEBUG_INT, "report data: key_val :%8d transfer_code: %8d , scancode: %8d\n", \
					key_val, transfer_code, scancode);

				if (transfer_code == scancode) {
					/* report repeat key value */
#ifdef REPORT_REPEAT_KEY_FROM_HW
					input_report_key(sunxikbd_dev, scankeycodes[scancode], 0);
					input_sync(sunxikbd_dev);
					input_report_key(sunxikbd_dev, scankeycodes[scancode], 1);
					input_sync(sunxikbd_dev);
#else
					/* do not report key value */
#endif
				} else if (INITIAL_VALUE != transfer_code) {
					/* report previous key value up signal + report current key value down */
					input_report_key(sunxikbd_dev, scankeycodes[transfer_code], 0);
					input_sync(sunxikbd_dev);
					input_report_key(sunxikbd_dev, scankeycodes[scancode], 1);
					input_sync(sunxikbd_dev);
					transfer_code = scancode;

				} else {
					/* INITIAL_VALUE == transfer_code, first time to report key event */
					input_report_key(sunxikbd_dev, scankeycodes[scancode], 1);
					input_sync(sunxikbd_dev);
					transfer_code = scancode;
				}
			}
		}
	}

	if (reg_val & LRADC_ADC0_UPPEND) {

		if(INITIAL_VALUE != transfer_code) {

			dprintk(DEBUG_INT, "report data: key_val :%8d transfer_code: %8d \n",key_val, transfer_code);

			input_report_key(sunxikbd_dev, scankeycodes[transfer_code], 0);
			input_sync(sunxikbd_dev);
		}

		dprintk(DEBUG_INT, "key up \n");

		key_cnt = 0;
		judge_flag = 0;
		transfer_code = INITIAL_VALUE;
	}

	sunxi_keyboard_clr_ints(reg_val);
	return IRQ_HANDLED;
}

static void sunxikbd_map_init(void)
{
	int i = 0;
	unsigned char j = 0;
	for(i = 0; i < 64; i++){
		if(i * adc_resol > key_vol[j])
			j++;
		keypad_mapindex[i] = j;
	}
}

static int sunxi_keyboard_startup(struct platform_device *pdev)
{
	struct device_node *np =NULL;
	int ret = 0;
	np = pdev->dev.of_node;

	if (!of_device_is_available(np)) {
		pr_err("%s: sunxi keyboard is disable\n", __func__);
		return -EPERM;
	}

	key_data = kzalloc(sizeof(*key_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(key_data)) {
		pr_err("key_data: not enough memory for key data\n");
		return -ENOMEM;
	}

	key_data->reg_base= of_iomap(np, 0);
	if (NULL == key_data->reg_base) {
		pr_err("%s:Failed to ioremap() io memory region.\n",__func__);
		ret = -EBUSY;
	}else
		dprintk(DEBUG_INIT, "key base: %p !\n",key_data->reg_base);
	key_data->irq_num= irq_of_parse_and_map(np, 0);
	if (0 == key_data->irq_num) {
		pr_err("%s:Failed to map irq.\n", __func__);
		ret = -EBUSY;
	}else
		dprintk(DEBUG_INIT, "ir irq num: %d !\n",key_data->irq_num);
	key_data->pclk = of_clk_get(np, 0);
	key_data->mclk = of_clk_get(np, 1);
	if (NULL==key_data->pclk||IS_ERR(key_data->pclk)
		||NULL==key_data->mclk||IS_ERR(key_data->mclk)) {
		dprintk(DEBUG_INIT, "%s:keyboard has no clk.\n", __func__);
	}

	return ret;
}

static int sunxikbd_key_init(struct platform_device *pdev)
{
	struct device_node *np =NULL;
	const struct of_device_id *match;
	struct sunxi_adc_disc *disc;
	int i;
	u32 val[2] = {0, 0};
	char key_name[16];

	np = pdev->dev.of_node;

	match = of_match_node(sunxi_keyboard_of_match, np);
	disc = (struct sunxi_adc_disc *)match->data;
	adc_measure = disc->measure;
	adc_resol = disc->resol;

	if (of_property_read_u32(np, "key_cnt", &key_num)) {
		pr_err("%s: get key count failed", __func__);
		return -EBUSY;
	}
	dprintk(DEBUG_INT,"%s key number = %d.\n", __func__, key_num);
	if(key_num < 1 || key_num > VOL_NUM){
		pr_err("incorrect key number.\n");
		return -1;
	}
	for(i = 1; i <= key_num; i++){
		sprintf(key_name, "key%d", i);
		if (of_property_read_u32_array(np, key_name, val, ARRAY_SIZE(val))) {
			pr_err("%s: get %s err! \n", __func__, key_name);
			return -EBUSY;
		}
		key_vol[i-1] = val[0];
		scankeycodes[i-1] = val[1];
		dprintk(DEBUG_INT,"%s: key%d vol= %d code= %d\n", __func__, i,
						 key_vol[i-1], scankeycodes[i-1]);
	}
	sunxikbd_map_init();

	return 0;
}

static int sunxi_keyboard_probe(struct platform_device *pdev)

{
	int i;
	int err =0;
	unsigned long mode, para;

	dprintk(DEBUG_INIT, "sunxikbd_init \n");

	if (pdev->dev.of_node) {
		/* get dt and sysconfig */
		err = sunxi_keyboard_startup(pdev);
	}else{
		pr_err("sunxi keyboard device tree err!\n");
		return -EBUSY;
	}

	if( err < 0)
		goto fail1;

	if(sunxikbd_key_init(pdev)){
		err = -EFAULT;
		goto fail1;
	}

	sunxikbd_dev = input_allocate_device();
	if (!sunxikbd_dev) {
		pr_err("sunxikbd: not enough memory for input device\n");
		err = -ENOMEM;
		goto fail1;
	}

	sunxikbd_dev->name = INPUT_DEV_NAME;
	sunxikbd_dev->phys = "sunxikbd/input0";
	sunxikbd_dev->id.bustype = BUS_HOST;
	sunxikbd_dev->id.vendor = 0x0001;
	sunxikbd_dev->id.product = 0x0001;
	sunxikbd_dev->id.version = 0x0100;

#ifdef REPORT_REPEAT_KEY_BY_INPUT_CORE
	sunxikbd_dev->evbit[0] = BIT_MASK(EV_KEY)|BIT_MASK(EV_REP);
	pr_info("support report repeat key value. \n");
#else
	sunxikbd_dev->evbit[0] = BIT_MASK(EV_KEY);
#endif

	for (i = 0; i < KEY_MAX_CNT; i++)
		set_bit(scankeycodes[i], sunxikbd_dev->keybit);
	key_data->input_dev = sunxikbd_dev;

#ifdef ONE_CHANNEL
	mode = ADC0_DOWN_INT_SET | ADC0_UP_INT_SET | ADC0_DATA_INT_SET;
	para = LRADC_ADC0_DOWN_EN | LRADC_ADC0_UP_EN | LRADC_ADC0_DATA_EN;
	sunxi_keyboard_int_set(mode, para);
	mode = CONCERT_DLY_SET | ADC_CHAN_SET | KEY_MODE_SET \
		| LRADC_HOLD_SET | LEVELB_VOL_SET \
		| LRADC_SAMPLE_SET | LRADC_EN_SET;
	para = FIRST_CONCERT_DLY|LEVELB_VOL|KEY_MODE_SELECT \
		|LRADC_HOLD_EN|ADC_CHAN_SELECT \
		|LRADC_SAMPLE_250HZ|LRADC_EN;
	sunxi_keyboard_ctrl_set(mode, para);
#else
#endif
	if (request_irq(key_data->irq_num, sunxi_isr_key, 0, "sunxikbd", NULL)) {
		err = -EBUSY;
		pr_err("request irq failure. \n");
		goto fail2;
	}
#ifdef CONFIG_PM
	keyboard_pm_domain.ops.suspend = sunxi_keyboard_suspend;
	keyboard_pm_domain.ops.resume = sunxi_keyboard_resume;
	sunxikbd_dev->dev.pm_domain = &keyboard_pm_domain;
#endif
	err = input_register_device(sunxikbd_dev);
	if (err)
		goto fail3;

	dprintk(DEBUG_INIT, "sunxikbd_init end\n");

	return 0;

fail3:
	free_irq(key_data->irq_num, NULL);
fail2:
	input_free_device(sunxikbd_dev);
fail1:
	if(key_data)
		kfree(key_data);
	pr_err("sunxikbd_init failed. \n");

	return err;
}

static int sunxi_keyboard_remove(struct platform_device *pdev)

{
	free_irq(key_data->irq_num, NULL);
	input_unregister_device(sunxikbd_dev);
	if(key_data)
		kfree(key_data);
	return 0;
}

static struct platform_driver sunxi_keyboard_driver = {
	.probe  = sunxi_keyboard_probe,
	.remove = sunxi_keyboard_remove,
	.driver = {
		.name   = "sunxi-keyboard",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(sunxi_keyboard_of_match),
	},
};
module_platform_driver(sunxi_keyboard_driver);

MODULE_AUTHOR(" Qin");
MODULE_DESCRIPTION("sunxi-keyboard driver");
MODULE_LICENSE("GPL");

