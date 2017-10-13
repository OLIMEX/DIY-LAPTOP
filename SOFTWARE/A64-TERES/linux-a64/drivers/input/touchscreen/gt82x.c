/*---------------------------------------------------------------------------------------------------------
 * driver/input/touchscreen/goodix_touch.c
 *
 * Copyright(c) 2010 Goodix Technology Corp.     
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Change Date: 
 *		2010.11.11, add point_queue's definiens.     
 *                             
 * 		2011.03.09, rewrite point_queue's definiens.  
 *   
 * 		2011.05.12, delete point_queue for Android 2.2/Android 2.3 and so on.
 *                                                                                              
 *---------------------------------------------------------------------------------------------------------*/
#include <linux/i2c.h>
#include <linux/input.h>
#include "goodix_touch.h"
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/init-input.h>
#include <linux/gpio.h>
#include <linux/pm_runtime.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#define FOR_TSLIB_TEST
#define TEST_I2C_TRANSFER


#ifndef GUITAR_GT80X
#error The code does not match the hardware version.
#endif

static struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
	.name = NULL,
	.int_number = 0,
};

struct goodix_ts_data {
	int retry;
	int panel_type;
	uint8_t bad_data;
	char phys[32];		
	struct i2c_client *client;
	struct input_dev *input_dev;
	uint8_t use_irq;
	uint8_t use_shutdown;
	uint32_t gpio_shutdown;
	uint32_t gpio_irq;
	uint32_t screen_width;
	uint32_t screen_height;
	bool is_suspended;
	struct ts_event		event;
	struct hrtimer timer;
	struct work_struct  work;
	int (*power)(struct goodix_ts_data * ts, int on);
};

static const char *f3x_ts_name = "gt82x";
static struct workqueue_struct *goodix_wq;
#define X_DIFF (800)
static uint8_t config_info1[114];
static uint8_t data_info0[] = {
	0x0F,0x80,
	0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,
	0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,
	0x0A,0x19,0x0B,0x1A,0xFF,0x15,0x16,0x17,
	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x03,0x0D,
	0x04,0x0E,0x05,0x0F,0x06,0x10,0x07,0x11,
	0x08,0x12,0xFF,0x0D,0xFF,0x0F,0x10,0x11,
	0x12,0x13,0x0F,0x03,0x10,0x88,0x88,0x20,
	0x00,0x00,0x06,0x00,0x00,0x02,0x50,0x3C,
	0x35,0x03,0x00,0x05,0x00,0x03,0x20,0x05,
	0x00,0x5A,0x5A,0x46,0x46,0x08,0x00,0x03,
	0x19,0x05,0x14,0x10,0x00,0x07,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01
};
static uint8_t data_info1[] = {
	0x0F,0x80,
	0x00,0x0F,0x01,0x10,0x02,0x11,0x03,0x12,
	0x04,0x13,0x05,0x14,0x06,0x15,0x07,0x16,
	0x08,0x17,0x09,0x18,0x0A,0x19,0x0B,0x1A,
	0x0C,0x1B,0x0D,0x1C,0x0E,0x1D,0x13,0x09,
	0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,
	0x0E,0x04,0x0D,0x03,0x0C,0x02,0x0B,0x01,
	0x0A,0x00,0x0B,0x03,0x10,0x00,0x00,0x2C,
	0x00,0x00,0x03,0x00,0x00,0x02,0x40,0x30,
	0x60,0x03,0x00,0x05,0x00,0x03,0x20,0x05,
	0x00,0x66,0x4E,0x60,0x49,0x06,0x00,0x23,
	0x19,0x05,0x14,0x10,0x03,0xFC,0x01,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
};

static u32 debug_mask = 0;
enum{
	DEBUG_INIT = 1U << 0,
	DEBUG_SUSPEND = 1U << 1,
	DEBUG_INT_INFO = 1U << 2,
	DEBUG_X_Y_INFO = 1U << 3,
	DEBUG_KEY_INFO = 1U << 4,
	DEBUG_WAKEUP_INFO = 1U << 5,
	DEBUG_OTHERS_INFO = 1U << 6,
};
#define dprintk(level_mask,fmt,arg...)    if(unlikely(debug_mask & level_mask)) \
        printk("***CTP***"fmt, ## arg)

#define CTP_IRQ_MODE		(IRQF_TRIGGER_FALLING)
#define CTP_NAME		"gt82x"
#define SCREEN_MAX_X		(screen_max_x)
#define SCREEN_MAX_Y		(screen_max_y)
#define PRESS_MAX		(255)


#define READ_TOUCH_ADDR_H   0x0F
#define READ_TOUCH_ADDR_L   0x40

static u32 screen_max_x = 0;
static u32 screen_max_y = 0;
static u32 revert_x_flag = 0;
static u32 revert_y_flag = 0;
static u32 exchange_x_y_flag = 0;

static __u32 twi_id = 0;

/* Addresses to scan */

static const unsigned short normal_i2c[2] = {0x5d,I2C_CLIENT_END};
static const int chip_id_value[3] = {0x13,0x27,0x28};
static uint8_t read_chip_value[3] = {0x0f,0x7d,0};

/*used by GT80X-IAP module */
static struct i2c_client * i2c_connect_client = NULL;

static void goodix_init_events(struct work_struct *work);
static void goodix_resume_events(struct work_struct *work);
static struct workqueue_struct *goodix_init_wq;
static struct workqueue_struct *goodix_resume_wq;
static DECLARE_WORK(goodix_init_work, goodix_init_events);
static DECLARE_WORK(goodix_resume_work, goodix_resume_events);
static struct goodix_ts_data *ts_init;

/*******************************************************	
Function:
	Read data from the i2c slave device.
Input:
	client:	i2c device.
	buf[0]:operate address.
	buf[1]~buf[len]:read data buffer.
	len:operate length.	
Output:
	numbers of i2c_msgs to transfer
*********************************************************/
static int i2c_read_bytes(struct i2c_client *client, uint8_t *buf, uint16_t len)
{
	struct i2c_msg msgs[2];
	int ret=-1;
	//����д��ַ
	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr = client->addr;
	msgs[0].len = 2;		//data address
	msgs[0].buf = buf;
	//��������
	msgs[1].flags = I2C_M_RD;//����Ϣ
	msgs[1].addr = client->addr;
	msgs[1].len = len-2;
	msgs[1].buf = buf+2;
	
	ret=i2c_transfer(client->adapter, msgs, 2);
	return ret;
}

/*******************************************************	
Function:
	write data to the i2c slave device.
Input:
	client:	i2c device.
	buf[0]:operate address.
	buf[1]~buf[len]:write data buffer.
	len:operate length.	
Output:
	numbers of i2c_msgs to transfer.
*********************************************************/
static int i2c_write_bytes(struct i2c_client *client, uint8_t *data, uint16_t len)
{
	struct i2c_msg msg;
	int ret=-1;
	
	msg.flags = !I2C_M_RD;//д��Ϣ
	msg.addr = client->addr;
	msg.len = len;
	msg.buf = data;		
	
	ret=i2c_transfer(client->adapter, &msg,1);
	return ret;
}


/*******************************************************
Function:
	write i2c end cmd.	
	ts:	client Private data structures
return��
    Successful returns 1
*******************************************************/
static s32 i2c_end_cmd(struct goodix_ts_data *ts)
{
	s32 ret;
	u8 end_cmd_data[2]={0x80, 0x00};    

	ret=i2c_write_bytes(ts->client,end_cmd_data,2);
	return ret;
}

/**
 * ctp_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	int  i = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
		pr_info("======return=====\n");
		return -ENODEV;
	}
	if(twi_id == adapter->nr){
		i2c_read_bytes(client,read_chip_value,3);
		dprintk(DEBUG_INIT,"addr:0x%x,chip_id_value:0x%x\n",client->addr,read_chip_value[2]);
		while(chip_id_value[i++]){
			if(read_chip_value[2] == chip_id_value[i - 1]){
				strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
				return 0;
			}                   
		}
		pr_err("%s:I2C connection might be something wrong ! \n",__func__);
		return -ENODEV;
	}else{
		return -ENODEV;
	}
}

static ssize_t gt82x_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct i2c_client *client = input_get_drvdata(input);
	return sprintf(buf, "%d\n", !pm_runtime_suspended(&client->dev));
}

static ssize_t gt82x_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct input_dev *input = to_input_dev(dev);
	struct i2c_client *client = input_get_drvdata(input);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (data == 0) {
		pm_runtime_put_sync(&client->dev);
	}else if (data == 1){
		pm_runtime_get_sync(&client->dev);
	}

	return count;
}

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR,
	gt82x_enable_show, gt82x_enable_store);

static struct attribute *gt82x_att_als[] = {
	&dev_attr_enable.attr,
	NULL
};

static struct attribute_group gt82x_als_gr = {
	.attrs = gt82x_att_als
};

/**
 * ctp_print_info - sysconfig print function
 * return value:
 *
 */
static void ctp_print_info(struct ctp_config_info info,int debug_level)
{
	if(debug_level == DEBUG_INIT)
	{
		dprintk(DEBUG_INIT,"info.ctp_used:%d\n",info.ctp_used);
		dprintk(DEBUG_INIT,"info.twi_id:%d\n",info.twi_id);
		dprintk(DEBUG_INIT,"info.screen_max_x:%d\n",info.screen_max_x);
		dprintk(DEBUG_INIT,"info.screen_max_y:%d\n",info.screen_max_y);
		dprintk(DEBUG_INIT,"info.revert_x_flag:%d\n",info.revert_x_flag);
		dprintk(DEBUG_INIT,"info.revert_y_flag:%d\n",info.revert_y_flag);
		dprintk(DEBUG_INIT,"info.exchange_x_y_flag:%d\n",info.exchange_x_y_flag);
		dprintk(DEBUG_INIT,"info.irq_gpio_number:%d\n",info.irq_gpio.gpio);
		dprintk(DEBUG_INIT,"info.wakeup_gpio_number:%d\n",info.wakeup_gpio.gpio);
	}
}

/**
 * ctp_wakeup - function
 *
 */
static int ctp_wakeup(int status,int ms)
{
	dprintk(DEBUG_INIT,"***CTP*** %s:status:%d,ms = %d\n",__func__,status,ms);

	if (status == 0) {

		if(ms == 0) {
			__gpio_set_value(config_info.wakeup_gpio.gpio, 0);
		}else {
			__gpio_set_value(config_info.wakeup_gpio.gpio, 0);
			msleep(ms);
			__gpio_set_value(config_info.wakeup_gpio.gpio, 1);
		}
	}
	if (status == 1) {
		if(ms == 0) {
			__gpio_set_value(config_info.wakeup_gpio.gpio, 1);
		}else {
			__gpio_set_value(config_info.wakeup_gpio.gpio, 1);
			msleep(ms);
			__gpio_set_value(config_info.wakeup_gpio.gpio, 0);
		}
	}
	msleep(5);

	return 0;
}

/*******************************************************
Foundation
	 i2c communication test
	 ts:i2c client structure
return��
	Successful :1 fail:0
*******************************************************/
//Test i2c to check device. Before it SHUTDOWN port Must be low state 30ms or more.
#ifdef CTP_STANDBY
static bool goodix_i2c_test(struct i2c_client * client)
{
	int ret, retry;
	uint8_t test_data[1] = { 0 };	//only write a data address.

	for(retry=0; retry < 5; retry++)
	{
		ret =i2c_write_bytes(client, test_data, 1);	//Test i2c.
		if (ret == 1)
			break;
		msleep(5);
	}

	return ret==1 ? true : false;
}
#endif
/*******************************************************
Function:
	GTP initialize function.
Input:
	ts:	i2c client private struct.	
Output:
	Executive outcomes.1---succeed.
*******************************************************/
static int goodix_init_panel(struct goodix_ts_data *ts)
{
	int ret=-1;
	int i = 0;

	dprintk(DEBUG_OTHERS_INFO,"init panel\n");

	if (read_chip_value[2] == 0x13) {			//813
		ret=i2c_write_bytes(ts->client,data_info0, 114);       
	}else if(read_chip_value[2] == 0x28){ //828
		ret=i2c_write_bytes(ts->client,data_info1, 114); 
	}
	i2c_end_cmd(ts);
	msleep(10);
	dprintk(DEBUG_OTHERS_INFO,"init panel ret = %d\n",ret);
	if (ret < 0)
		return ret;
	msleep(100);
	config_info1[0] = 0x0F;
	config_info1[1] = 0x80;
	ret=i2c_read_bytes(ts->client,config_info1,114);	
	for ( i = 0;i<114;i++) {
		dprintk(DEBUG_OTHERS_INFO,"i = %d config_info1[i] = %x \n",i,config_info1[i]);
	    
	}
	msleep(10);

	return 1;
}

static s32 goodix_ts_version(struct goodix_ts_data *ts)

{
	u8 buf[8];
	buf[0] = 0x0f;
	buf[1] = 0x7d;

	i2c_read_bytes(ts->client, buf, 5);
	i2c_end_cmd(ts);
	dprintk(DEBUG_INIT,"PID:%02x, VID:%02x%02x\n", buf[2], buf[3], buf[4]);
	return 1;
}

/*******************************************************
Function:
	Touch down report function.
Input:
	ts:private data.
	id:tracking id.
	x:input x.
	y:input y.
	w:input weight.	
Output:
	None.
*******************************************************/
static void goodix_touch_down(struct goodix_ts_data* ts,s32 id,s32 x,s32 y,s32 w)
{
	dprintk(DEBUG_X_Y_INFO,"source data:ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);
	if(1 == exchange_x_y_flag){
		swap(x, y);
	}
	if(1 == revert_x_flag){
		x = SCREEN_MAX_X - x;
	}
	if(1 == revert_y_flag){
		y = SCREEN_MAX_Y - y;
	}
	dprintk(DEBUG_X_Y_INFO,"report data:ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
	input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
	input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
	input_mt_sync(ts->input_dev);
}
/*******************************************************
Function:
	Touch up report function.
Input:
	ts:private data.	
Output:
	None.
*******************************************************/
static void goodix_touch_up(struct goodix_ts_data* ts)
{
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
	input_mt_sync(ts->input_dev);
}

static void goodix_ts_work_func(struct work_struct *work)
{
	u8* coor_data = NULL;
	u8  point_data[2 + 2 + 5 * MAX_FINGER_NUM + 1]={READ_TOUCH_ADDR_H,READ_TOUCH_ADDR_L};
	u8  check_sum = 0;
	u8  touch_num = 0;
	u8  finger = 0;
	u8  key_value = 0;
	s32 input_x = 0;
	s32 input_y = 0;
	s32 input_w = 0;
	s32 idx = 0;
	s32 ret = -1;
	struct goodix_ts_data *ts = NULL;

	dprintk(DEBUG_X_Y_INFO,"===enter %s===\n",__func__);

	ts = container_of(work, struct goodix_ts_data, work);

	ret = i2c_read_bytes(ts->client, point_data, 10); 
	finger = point_data[2];
	touch_num = (finger & 0x01) + !!(finger & 0x02) + !!(finger & 0x04) + !!(finger & 0x08) + !!(finger & 0x10);
	if (touch_num > 1){
		u8 buf[25];
		buf[0] = READ_TOUCH_ADDR_H;
		buf[1] = READ_TOUCH_ADDR_L + 8;
		ret = i2c_read_bytes(ts->client, buf, 5 * (touch_num - 1) + 2); 
		memcpy(&point_data[10], &buf[2], 5 * (touch_num - 1));
	}
	i2c_end_cmd(ts);

	if (ret <= 0){
		pr_err("%s:I2C read error!",__func__);
		goto exit_work_func;
	}

	if((finger & 0xC0) != 0x80){
		dprintk(DEBUG_INIT,"%s:Data not ready!",__func__);
		goto exit_work_func;
	}

	key_value = point_data[3]&0x0f; // 1, 2, 4, 8
	if ((key_value & 0x0f) == 0x0f){
		if (!goodix_init_panel(ts)){
			pr_err("%s:Reload config failed!\n",__func__);
		}
		goto exit_work_func;
	}

	coor_data = &point_data[4];
	check_sum = 0;
	for ( idx = 0; idx < 5 * touch_num; idx++){
		check_sum += coor_data[idx];
	}
	if (check_sum != coor_data[5 * touch_num]){
		pr_err("%s:Check sum error!",__func__);
		goto exit_work_func;
	}

	if (touch_num){
		s32 pos = 0;

		for (idx = 0; idx < MAX_FINGER_NUM; idx++){
			if (!(finger & (0x01 << idx))){
			        continue;
			}
			input_x  = coor_data[pos] << 8;
			input_x |= coor_data[pos + 1];

			input_y  = coor_data[pos + 2] << 8;
			input_y |= coor_data[pos + 3];

			input_w  = coor_data[pos + 4];

			pos += 5;

			goodix_touch_down(ts, idx, input_x, input_y, input_w);
		}
	}else{
		dprintk(DEBUG_X_Y_INFO,"Touch Release!");
		goodix_touch_up(ts);
	}

#if GTP_HAVE_TOUCH_KEY
	for (idx= 0; idx < GTP_MAX_KEY_NUM; idx++){
		input_report_key(ts->input_dev, touch_key_array[idx], key_value & (0x01<<idx));   
	}
#endif
	//input_report_key(ts->input_dev, BTN_TOUCH, (touch_num || key_value));
	input_sync(ts->input_dev);

exit_work_func:
	return;
}

static irqreturn_t goodix_ts_irq_hanbler(int irq, void *dev_id)
{
	struct goodix_ts_data *ts = (struct goodix_ts_data *)dev_id;
	
	dprintk(DEBUG_INT_INFO,"==========------TS Interrupt-----============\n");
	queue_work(goodix_wq, &ts->work);
	return IRQ_HANDLED;
}

static int goodix_ts_power(struct goodix_ts_data * ts, int on)
{
	s32 ret = -1;
	s32 success = 1;
	u8 i2c_control_buf1[3] = {0x0F,0xF2,0xc0};        //suspend cmd
	u8 i2c_control_buf2[3] = {0x0F,0xF2,0x00};

	switch (on)
	{
	case 0:
		ret = i2c_write_bytes(ts->client, i2c_control_buf1, 3);
		i2c_end_cmd(ts);
		return ret;        
	case 1:             
		ctp_wakeup(0,100);
		#ifdef CTP_STANDBY
		if(STANDBY_WITH_POWER_OFF == standby_level){
			ret = goodix_i2c_test(ts->client);
			if(!ret){
				pr_err("Warnning: I2C connection might be something wrong!\n");
				ctp_wakeup(0,50);
				ret = goodix_i2c_test(ts->client);
				if(!ret){
				        pr_err("retry fail!\n");
				        return -1;
				}
			}
			pr_info("===== goodix i2c test ok=======\n");
		}
		#endif
		ret = goodix_init_panel(ts);
		if( ret != 1){
			pr_err("init panel fail!\n");
			return -1;
		}
		ret = i2c_write_bytes(ts->client, i2c_control_buf2, 3);
		msleep(10);
		return success;

	 default:
	        pr_err("%s: Cant't support this command.",f3x_ts_name );
	        return -EINVAL;
	}	
}

static void goodix_resume_events (struct work_struct *work)
{
	int ret;
	u8 i2c_control_buf[3] = {0x0F,0xF2,0x00};

	if (ts_init->is_suspended == false) {
		ctp_wakeup(0,2);
		ret = i2c_write_bytes(ts_init->client, i2c_control_buf, 3);
		if( ret != 1){
                        pr_err("set active mode fail!\n");
                        return ;
		}
		msleep(10);
	} else if (ts_init->power) {
		ret = ts_init->power(ts_init, 1);
		if (ret < 0)
			dprintk(DEBUG_SUSPEND,"%s power on failed\n", f3x_ts_name);
	}
	ret = input_set_int_enable(&(config_info.input_type), 1);
	ts_init->is_suspended = false;
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq enable failed\n", f3x_ts_name);
}

//ͣ���豸
static int goodix_ts_suspend(struct device *dev)
{
	int ret;
	struct goodix_ts_data *ts = dev_get_drvdata(dev);

	dprintk(DEBUG_SUSPEND,"CONFIG_PM:enter earlysuspend: goodix_ts_suspend. \n");
	if (pm_runtime_suspended(dev))
		return 0;

	if (ts->is_suspended == false) {
		flush_workqueue(goodix_resume_wq);
		ret = input_set_int_enable(&(config_info.input_type), 0);
		if (ret < 0)
			dprintk(DEBUG_SUSPEND,"%s irq disable failed\n", f3x_ts_name);
		ret = cancel_work_sync(&ts->work);
		flush_workqueue(goodix_wq);
		if (ts->power) {
			ret = ts->power(ts,0);
			if (ret < 0)
				dprintk(DEBUG_SUSPEND,"%s power off failed\n", f3x_ts_name);
		}
		ts->is_suspended = true;
	}
	return 0 ;
}

//���»���
static int goodix_ts_resume(struct device *dev)
{
	struct goodix_ts_data *ts = dev_get_drvdata(dev);
	dprintk(DEBUG_SUSPEND,"CONFIG_PM:enter laterresume: goodix_ts_resume. \n");
	if (pm_runtime_suspended(dev))
		return 0;
	if (ts->is_suspended == true) {
		queue_work(goodix_resume_wq, &goodix_resume_work);
	}
	return 0;
}

static void goodix_init_events (struct work_struct *work)
{
	int ret = 0;

	ctp_wakeup(0,100);
	ret = goodix_init_panel(ts_init);
	if(!ret) {
		pr_err("init panel fail!\n");
		return;
	}else {
		dprintk(DEBUG_INIT,"init panel succeed!\n");
        }
        config_info.dev = &(ts_init->input_dev->dev);
	ret = input_request_int(&(config_info.input_type), goodix_ts_irq_hanbler,
				CTP_IRQ_MODE, ts_init);
	if (ret) {
		pr_info( "goodix_probe: request irq failed\n");
		return;
	}

	return;
}

/*******************************************************	
���ܣ�
	������̽�⺯��
	��ע������ʱ���ã�Ҫ����ڶ�Ӧ��client����
	����IO,�жϵ���Դ���룻�豸ע�᣻��������ʼ���ȹ���
������
	client�����������豸�ṹ��
	id���豸ID
return��
	ִ�н���룬0��ʾ����ִ��
********************************************************/
static int goodix_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct goodix_ts_data *ts;
	int ret = 0;
	dprintk(DEBUG_INIT,"=============GT82x Probe==================\n");
        
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)){
		dev_err(&client->dev, "System need I2C function.\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	
	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
		
	
	i2c_connect_client = client;				//used by Guitar Updating.
	
	INIT_WORK(&ts->work, goodix_ts_work_func);
	ts->client = client;
	i2c_set_clientdata(ts->client, ts);
	
	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) 
	{
		ret = -ENOMEM;
		dev_dbg(&client->dev,"Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
	
#ifndef GOODIX_MULTI_TOUCH	
	ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	ts->input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
	input_set_abs_params(ts->input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);	
	
#else
	ts->input_dev->absbit[0] = BIT_MASK(ABS_MT_TRACKING_ID) |
		BIT_MASK(ABS_MT_TOUCH_MAJOR)| BIT_MASK(ABS_MT_WIDTH_MAJOR) |
		BIT_MASK(ABS_MT_POSITION_X) | BIT_MASK(ABS_MT_POSITION_Y); 	// for android
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);	
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MAX_FINGER_NUM, 0, 0);	
#endif	

#ifdef FOR_TSLIB_TEST
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
#endif

	sprintf(ts->phys, "input/goodix-ts");
	ts->input_dev->name = CTP_NAME;
	ts->input_dev->phys = ts->phys;
	ts->input_dev->id.bustype = BUS_I2C;
	ts->input_dev->id.vendor = 0xDEAD;
	ts->input_dev->id.product = 0xBEEF;
	ts->input_dev->id.version = 0x1105;	

	ts->is_suspended = false;

	ret = input_register_device(ts->input_dev);
	if (ret) {
		dev_err(&client->dev,"Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}
	input_set_drvdata(ts->input_dev, client);
	ret = sysfs_create_group(&ts->input_dev->dev.kobj,
						 &gt82x_als_gr);
	if (ret < 0)
	{
		dev_err(&client->dev,"gt82x: sysfs_create_group err\n");
		goto err_input_register_device_failed;
	}
	
	goodix_wq = create_singlethread_workqueue("goodix_wq");
	if (!goodix_wq) {
		pr_err("Creat %s workqueue failed.\n", f3x_ts_name);
		return -ENOMEM;
		
	}
	flush_workqueue(goodix_wq);	
	ts->power = goodix_ts_power;

	ts_init = ts;

	goodix_init_wq = create_singlethread_workqueue("goodix_init");
	if (goodix_init_wq == NULL) {
		pr_err("create goodix_wq fail!\n");
		return -ENOMEM;
	}

	goodix_resume_wq = create_singlethread_workqueue("goodix_resume");
	if (goodix_resume_wq == NULL) {
		pr_err("create goodix_resume_wq fail!\n");
		return -ENOMEM;
	}

	queue_work(goodix_init_wq, &goodix_init_work);
	
	goodix_ts_version(ts);
	pm_runtime_set_active(&client->dev);
	pm_runtime_get(&client->dev);
	pm_runtime_enable(&client->dev);
		
	dprintk(DEBUG_INIT,"========Probe Ok================\n");
	return 0;

err_input_register_device_failed:	
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:	
	i2c_set_clientdata(client, NULL);
err_alloc_data_failed:	
err_check_functionality_failed:
	return ret;
}


/*******************************************************	
���ܣ�
	������Դ�ͷ�
������
	client���豸�ṹ��
return��
	ִ�н���룬0��ʾ����ִ��
********************************************************/
static int goodix_ts_remove(struct i2c_client *client)
{
	struct goodix_ts_data *ts = i2c_get_clientdata(client);
	dev_notice(&client->dev,"The driver is removing...\n");
	
	//free_irq(SW_INT_IRQNO_PIO, ts);
	input_free_int(&(config_info.input_type), ts_init);

	cancel_work_sync(&goodix_init_work);
	cancel_work_sync(&goodix_resume_work);
	flush_workqueue(goodix_wq);
	if (goodix_resume_wq)
		destroy_workqueue(goodix_resume_wq);
	if (goodix_init_wq)
		destroy_workqueue(goodix_init_wq);
	if (goodix_wq)
		destroy_workqueue(goodix_wq);
	pm_runtime_disable(&client->dev);
	pm_runtime_set_suspended(&client->dev);
	sysfs_remove_group(&ts->input_dev->dev.kobj, &gt82x_als_gr);
	input_unregister_device(ts->input_dev);
	i2c_set_clientdata(ts->client, NULL);
	kfree(ts);
	return 0;
}

//�����ڸ������� �豸�����豸ID �б�
//only one client
static const struct i2c_device_id goodix_ts_id[] = {
	{ CTP_NAME, 0 },
	{ }
};

static UNIVERSAL_DEV_PM_OPS(gt82x_pm_ops, goodix_ts_suspend,
	goodix_ts_resume, NULL);

#define GT82X_PM_OPS (&gt82x_pm_ops)

//�豸�����ṹ��
static struct i2c_driver goodix_ts_driver = {
	.class = I2C_CLASS_HWMON,
	.probe		= goodix_ts_probe,
	.remove		= goodix_ts_remove,
	.id_table	= goodix_ts_id,
	.driver = {
		.name	= CTP_NAME,
		.owner = THIS_MODULE,
		.pm = GT82X_PM_OPS,
	},
	.detect         = ctp_detect,
	.address_list	= normal_i2c,
};

static int ctp_get_system_config(void)
{   
	ctp_print_info(config_info,DEBUG_INIT);
	twi_id = config_info.twi_id;
	screen_max_x = config_info.screen_max_x;
	screen_max_y = config_info.screen_max_y;
	revert_x_flag = config_info.revert_x_flag;
	revert_y_flag = config_info.revert_y_flag;
	exchange_x_y_flag = config_info.exchange_x_y_flag;
	if ((screen_max_x == 0) || (screen_max_y == 0)){
		pr_err("%s:read config error!\n",__func__);
		return 0;
	}
	return 1;
}
//�������غ���
static int __init goodix_ts_init(void)
{
	int ret = -1;
	//int err = -1;
	dprintk(DEBUG_INIT,"****************************************************************\n");
	if (input_fetch_sysconfig_para(&(config_info.input_type))){
		pr_err("%s: ctp_fetch_sysconfig_para err.\n", __func__);
		return 0;
	}else{
		ret = input_init_platform_resource(&(config_info.input_type));
		if (0 != ret) {
			pr_err("%s:ctp_ops.init_platform_resource err. \n", __func__);    
			goto init_err;
		}
		input_set_power_enable(&(config_info.input_type),1);
	}

	if (config_info.ctp_used == 0) {
		pr_info("*** ctp_used set to 0 !\n");
		pr_info("*** if use ctp,please put the sys_config.fex ctp_used set to 1. \n");
		ret = 0;
		goto init_err;
	}
	if(!ctp_get_system_config()){
		pr_err("%s:read config fail!\n",__func__);
		goto init_err;
	}
	ctp_wakeup(0,2);

	ret = i2c_add_driver(&goodix_ts_driver);
	if(ret)
		goto init_err;

	dprintk(DEBUG_INIT,"****************************************************************\n");
	return ret;
init_err:
	input_set_power_enable(&(config_info.input_type),0);
	return ret;
}

//����ж�غ���
static void __exit goodix_ts_exit(void)
{
	i2c_del_driver(&goodix_ts_driver);
	input_free_platform_resource(&(config_info.input_type));
	input_set_power_enable(&(config_info.input_type),0);
	return;
}

late_initcall(goodix_ts_init);
module_exit(goodix_ts_exit);
module_param_named(debug_mask,debug_mask,int,0644);
MODULE_DESCRIPTION("Goodix Touchscreen Driver");
MODULE_LICENSE("GPL v2");
