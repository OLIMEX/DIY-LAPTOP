/* drivers/input/touchscreen/gt818x.c
 * 
 * 2010 - 2012 Goodix Technology.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be a reference 
 * to you, when you are integrating the GOODiX's CTP IC into your system, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 * 
 * Version:1.4
 * Author:scott@goodix.com
 * Release Date:2012/09/20
 * Revision record:
 *      V1.0:2012/06/08,create file
 *      V1.2:2012/08/06,modify to support GT868&GT968M
 *      V1.4:2012/09/20,G868 sensor ID & coor key suppoert
 */

#include <linux/irq.h>
#include "gt818.h"
#include <linux/gpio.h>
#include <linux/init-input.h>


#if GTP_ICS_SLOT_REPORT
#include <linux/input/mt.h>
#endif

static const char *goodix_ts_name = "gt818";
static struct workqueue_struct *goodix_wq;
static struct i2c_client * i2c_connect_client = NULL; 
static u8 config[GTP_CONFIG_LENGTH + GTP_ADDR_LENGTH]
                = {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};

#if GTP_HAVE_TOUCH_KEY
    static const u16 touch_key_array[] = GTP_KEY_TAB
    #define GTP_MAX_KEY_NUM	 (sizeof(touch_key_array)/sizeof(touch_key_array[0]))
#endif

static s8 gtp_i2c_test(struct i2c_client *client);	

#ifdef CONFIG_HAS_EARLYSUSPEND
static void goodix_ts_early_suspend(struct early_suspend *h);
static void goodix_ts_late_resume(struct early_suspend *h);
#endif


//#if GTP_CREATE_WR_NODE
//extern s32 init_wr_node(struct i2c_client*);
//extern void uninit_wr_node(void);
//#endif



//#if GTP_AUTO_UPDATE
//extern u8 gup_init_update_proc(struct goodix_ts_data *);
//#endif

#if GTP_ESD_PROTECT
static struct delayed_work gtp_esd_check_work;
static struct workqueue_struct * gtp_esd_check_workqueue = NULL;
static void gtp_esd_check_func(struct work_struct *);
#endif

#define CTP_IRQ_NUMBER          (config_info.int_number)
#define CTP_IRQ_MODE		(IRQF_TRIGGER_FALLING)
#define CTP_NAME			"gt818_ts"	//GOODIX_I2C_NAME
#define SCREEN_MAX_HEIGHT		(screen_max_x)
#define SCREEN_MAX_WIDTH		(screen_max_y)
#define PRESS_MAX			(255)

static void goodix_init_events(struct work_struct *work);
static void goodix_resume_events(struct work_struct *work);
static struct workqueue_struct *goodix_init_wq;
static struct workqueue_struct *goodix_resume_wq;
static DECLARE_WORK(goodix_init_work, goodix_init_events);
static DECLARE_WORK(goodix_resume_work, goodix_resume_events);

static struct goodix_ts_data *ts_init;


static int screen_max_x = 0;
static int screen_max_y = 0;
static int revert_x_flag = 0;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;
//static char mach_name[20]={0};
static __u32 twi_id = 0;
static char irq_pin_name[8];

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
        pr_debug("***CTP***"fmt, ## arg)
module_param_named(debug_mask,debug_mask,int,S_IRUGO | S_IWUSR | S_IWGRP);

static struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
	.name = NULL,
	.int_number = 0,
};


static const unsigned short normal_i2c[2] = {0x5d,I2C_CLIENT_END};
static const int chip_id_value[3] = {0x13,0x27,0x28};
//static uint8_t read_chip_value[3] = {0x0f,0x7d,0};

s32 gtp_i2c_read(struct i2c_client *client, u8 *buf, s32 len);
s32 gtp_i2c_end_cmd(struct i2c_client *client);
/**
 * ctp_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
    int ret;  
    u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};
        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
        	pr_err("i2c_check_functionality err\n======return=====\n");
                return -ENODEV;
        }


        if(twi_id == adapter->nr){
                pr_debug("%s: addr= %x\n",__func__,client->addr);
                msleep(50);
    			ret = gtp_i2c_read(client, buf, 6);
    			gtp_i2c_end_cmd(client);
				if(buf[3] != 0x18)
				{
					pr_debug("%s:IC is not gt818\n",__func__);
					return -ENODEV;
				}
    			GTP_INFO("IC VERSION:%02x%02x_%02x%02x", buf[3], buf[2], buf[5], buf[4]);
            	strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
        	//printk("%s:I2C connection might be something wrong ! \n",__func__);
        	return 0;
	}else{
	        return -ENODEV;
	}
}

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

void gtp_set_int_value(int status)
{
        long unsigned int	config;
		
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	    pin_config_get(SUNXI_PINCTRL,irq_pin_name,&config);

		if (1 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		      config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,1);
			  pin_config_set(SUNXI_PINCTRL,irq_pin_name,config);
	    }
        
        __gpio_set_value(CTP_IRQ_NUMBER, status);   
}

static void gtp_set_io_int(void)
{
        long unsigned int	config;
		
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	    pin_config_get(SUNXI_PINCTRL,irq_pin_name,&config);

		if (4 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		      config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,4);
			  pin_config_set(SUNXI_PINCTRL,irq_pin_name,config);
	    }
        
}

static void gtp_io_init(int ms)
{       
        ctp_wakeup(0, 0);
        msleep(ms);
        
        gtp_set_int_value(1);
        msleep(5);
        
        ctp_wakeup(1, 0);
        msleep(5);

        
#if GTP_ESD_PROTECT
        gtp_init_ext_watchdog(client);
#endif
        
}


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
s32 gtp_i2c_read(struct i2c_client *client, u8 *buf, s32 len)
{
    struct i2c_msg msgs[2];
    s32 ret=-1;
    s32 retries = 0;

    GTP_DEBUG_FUNC();

    msgs[0].flags = !I2C_M_RD;
    msgs[0].addr  = client->addr;
    msgs[0].len   = GTP_ADDR_LENGTH;
    msgs[0].buf   = &buf[0];

    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = client->addr;
    msgs[1].len   = len - GTP_ADDR_LENGTH;
    msgs[1].buf   = &buf[GTP_ADDR_LENGTH];

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, msgs, 2);
        if (ret == 2)break;
        retries++;
    }
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
s32 gtp_i2c_write(struct i2c_client *client,u8 *buf,s32 len)
{
    struct i2c_msg msg;
    s32 ret=-1;
    s32 retries = 0;

    GTP_DEBUG_FUNC();

    msg.flags = !I2C_M_RD;
    msg.addr  = client->addr;
    msg.len   = len;
    msg.buf   = buf;

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret == 1)break;
        retries++;
    }
    return ret;
}

/*******************************************************	
Function:
	write i2c end cmd.

Input:
	client:	i2c device.
	
Output:
	numbers of i2c_msgs to transfer.
*********************************************************/
s32 gtp_i2c_end_cmd(struct i2c_client *client)
{
    s32 ret = -1;
    u8 end_cmd_data[2]={0x80, 0x00}; 
    
    GTP_DEBUG_FUNC();

    ret = gtp_i2c_write(client, end_cmd_data, 2);

    return ret;
}

/*******************************************************
Function:
	Send config Function.

Input:
	client:	i2c client.

Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
s32 gtp_send_cfg(struct i2c_client *client)
{
    s32 ret = -1;
#if GTP_DRIVER_SEND_CFG
    s32 retry = 0;
	msleep(100);
    for (retry = 0; retry < 5; retry++)
    {
        ret = gtp_i2c_write(client, config , GTP_CONFIG_LENGTH + GTP_ADDR_LENGTH);        
        gtp_i2c_end_cmd(client);

        if (ret > 0)
        {
            break;
        }
    }
#endif

    return ret;
}

/*******************************************************
Function:
	Disable IRQ Function.

Input:
	ts:	i2c client private struct.
	
Output:
	None.
*******************************************************/
void gtp_irq_disable(struct goodix_ts_data *ts)
{
        unsigned long irqflags;
		int ret;

        dprintk(DEBUG_INT_INFO, "%s ---start!---\n", __func__);

        spin_lock_irqsave(&ts->irq_lock, irqflags);
        if (!ts->irq_is_disable) {
               ts->irq_is_disable = 1; 
               ret = input_set_int_enable(&(config_info.input_type), 0);
			   if (ret < 0)		          
			   	  dprintk(DEBUG_OTHERS_INFO,"%s irq disable failed\n", goodix_ts_name);
        }
        spin_unlock_irqrestore(&ts->irq_lock, irqflags);
}

/*******************************************************
Function:
	Disable IRQ Function.

Input:
	ts:	i2c client private struct.
	
Output:
	None.
*******************************************************/
void gtp_irq_enable(struct goodix_ts_data *ts)
{
        unsigned long irqflags = 0;
		int ret;

        dprintk(DEBUG_INT_INFO, "%s ---start!---\n", __func__);
    
        spin_lock_irqsave(&ts->irq_lock, irqflags);
        if (ts->irq_is_disable) {
                ts->irq_is_disable = 0; 
                ret = input_set_int_enable(&(config_info.input_type), 1);	
				if (ret < 0)		            
					dprintk(DEBUG_OTHERS_INFO,"%s irq enable failed\n", goodix_ts_name);
        }
        spin_unlock_irqrestore(&ts->irq_lock, irqflags);
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
static void gtp_touch_down(struct goodix_ts_data* ts,s32 id,s32 x,s32 y,s32 w)
{

/****************************************
	if(x > 18 && x < 28)
		x =x + 8;
	if(x > 4 && x < 13)
		x = x+13;

	if(x > 775 && x < 790)
		x = x - 5; 
	if(x > 790 && x < 799)
		x = x - 13;
	
	
	if(y > 15 && y < 25)
		y =y + 8;
	if(y > 4 && y < 10)
		y = y+13;

	if(y > 455 && y < 470)
		y = y - 5; 
	if(y > 470 && y < 480)
		y = y - 10;

*********************************************/

#if GTP_ICS_SLOT_REPORT
    input_mt_slot(ts->input_dev, id);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input_dev, ABS_MT_PRESSURE, w);
#else
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(ts->input_dev);
#endif

    GTP_DEBUG("ID:%d, X:%d, Y:%d, W:%d", id, x, y, w);
}

/*******************************************************
Function:
	Touch up report function.

Input:
	ts:private data.
	
Output:
	None.
*******************************************************/
static void gtp_touch_up(struct goodix_ts_data* ts, s32 id)
{
#if GTP_ICS_SLOT_REPORT
    input_mt_slot(ts->input_dev, id);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, -1);
    GTP_DEBUG("Touch id[%2d] release!", id);
#else
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
    input_mt_sync(ts->input_dev);
#endif
}

/*******************************************************
Function:
	Goodix touchscreen work function.

Input:
	work:	work_struct of goodix_wq.
	
Output:
	None.
*******************************************************/
static void goodix_ts_work_func(struct work_struct *work)
{
    u8  index_data[3] = {(u8)(GTP_REG_INDEX>>8),(u8)GTP_REG_INDEX,0};
    u8  point_data[2 + 1 + 8 * GTP_MAX_TOUCH] = {GTP_READ_COOR_ADDR>>8, GTP_READ_COOR_ADDR & 0xFF};  
    u8  touch_num = 0;
    static u8 pre_touch = 0;
   // static u8 pre_key = 0;
    u8  key_value = 0;
    u8* coor_data = NULL;
    s32 input_x = 0;
    s32 input_y = 0;
    s32 input_w = 0;
    s32 input_id = 0;
    s32 idx = 0;
    s32 ret = -1;
    
#if GTP_USE_868_968M
    s32 key_pnt_x[GTP_MAX_KEY_NUM] = KEY_CNTR_PNT_X;
    s32 key_pnt_y[GTP_MAX_KEY_NUM] = KEY_CNTR_PNT_Y; 
#endif

    struct goodix_ts_data *ts = NULL;

    GTP_DEBUG_FUNC();
    
    ts = container_of(work, struct goodix_ts_data, work);
    if (ts->enter_update)
    {
        goto exit_work_func;
    }

    ret = gtp_i2c_read(ts->client, index_data, 3);
    gtp_i2c_end_cmd(ts->client);
    if (ret < 0)
    {
        GTP_ERROR("I2C transfer error. errno:%d\n ", ret);
        goto exit_work_func;
    }

    if ((index_data[GTP_ADDR_LENGTH] & 0x0f) == 0x0f)
    {
        ret = gtp_send_cfg(ts->client);
        if (ret < 0)
        {
            GTP_DEBUG("Reload config failed!\n");
        }
        goto exit_work_func;
    }
    
    if ((index_data[GTP_ADDR_LENGTH] & 0x30) != 0x20)
    {
        GTP_INFO("Data not ready!");
        goto exit_work_func;
    }
    
    touch_num = index_data[GTP_ADDR_LENGTH] & 0x0f;
    if(touch_num > 5)
    {
        touch_num = 5;
    }
    ret = gtp_i2c_read(ts->client, point_data, 2 + 8 * touch_num + 1);
    if(ret < 0)	
    {
        GTP_ERROR("I2C transfer error. Number:%d\n ", ret);
        goto exit_work_func;
    }
    gtp_i2c_end_cmd(ts->client);

    GTP_DEBUG_ARRAY(index_data, 3);
    GTP_DEBUG("touch num:%x", touch_num);
    GTP_DEBUG_ARRAY(point_data, 2 + 8 * touch_num + 1);

    coor_data = &point_data[3];

#if GTP_ICS_SLOT_REPORT
    if (pre_touch || touch_num)
    {
        s32 pos = 0;

        for (idx = 0; idx < GTP_MAX_TOUCH; idx++)
        {
            input_id = coor_data[pos] - 1;
            if (input_id == idx)
            {
                input_x = (coor_data[pos + 2] << 8) | coor_data[pos + 1];
                input_y = (coor_data[pos + 4] << 8) | coor_data[pos + 3];
                input_w = 20;

                pos += 8;

                if (ts->coor_div_2)
                {
                    input_x /= 2;
                    input_y /= 2;
                }
	    	GTP_SWAP(input_x,input_y);
	    	if(revert_x_flag)
				input_x = screen_max_x - input_x;
	    	if(revert_y_flag)
				input_y = screen_max_y - input_y;
				
                gtp_touch_down(ts, idx, input_x, input_y, input_w);
                pre_touch |= 0x01 << idx;
            }
            else if (pre_touch & (0x01 << idx))
            {
                gtp_touch_up(ts, idx);
                pre_touch &= ~(0x01 << idx);
            }
        }
    }
    
#else

    if (touch_num)
    {
        s32 pos = 0;
        
        for (idx = 0; idx < touch_num; idx++)
        {
            input_id = coor_data[pos] - 1;
            input_x = (coor_data[pos + 2] << 8) | coor_data[pos + 1];
            input_y = (coor_data[pos + 4] << 8) | coor_data[pos + 3];
            input_w = 20;

            pos += 8;
            
            if (ts->coor_div_2)
            {
                input_x /= 2;
                input_y /= 2;
            }
            if ((input_x > ts->abs_x_max)||(input_y > ts->abs_y_max))
            {
                continue;
            }
	    	
			GTP_SWAP(input_x,input_y);
	    	if(revert_x_flag)
				input_x = screen_max_x - input_x;
	    	if(revert_y_flag)
				input_y = screen_max_y - input_y;
				
            gtp_touch_down(ts, input_id, input_x, input_y, input_w);
        }
    }
    else if (pre_touch)
    {
        GTP_DEBUG("Touch Release!");
        gtp_touch_up(ts, 0);
    }
#if GTP_HAVE_TOUCH_KEY
    #if GTP_USE_868_968M
        if (touch_num == 1)
        {
            // key report as coordinates,judge key or ts point
            if (input_y > ts->abs_y_max)
            {
                for (idx = 0; idx < GTP_MAX_KEY_NUM; idx++)
                {
                    if ((ABS_VAL(input_x - key_pnt_x[idx]) <= KEY_AREA_WIDTH_H) &&
                        (ABS_VAL(input_y - key_pnt_y[idx]) <= KEY_AREA_HEIGHT_H))
                    {
                        GTP_DEBUG("key_x = %d, key_y = %d", input_x, input_y);
                        key_value |= (0x01<<idx);
                    }
                    else
                    {
                        key_value &= (~(0x01<<idx));
                    }
                }
            }
        }
    #else
        key_value = point_data[GTP_ADDR_LENGTH] & 0x0f;
    #endif
        if(key_value || pre_key)
        {
            for (idx = 0; idx < GTP_MAX_KEY_NUM; idx++)
            {
                input_report_key(ts->input_dev, touch_key_array[idx], key_value & (0x01<<idx));   
            }
        }
        
        pre_key = key_value;
#endif

    pre_touch = touch_num;
    input_report_key(ts->input_dev, BTN_TOUCH, (touch_num || key_value));
    
#endif
    input_sync(ts->input_dev);

exit_work_func:
    if (ts->use_irq)
    {
        gtp_irq_enable(ts);
    }
}

#if 0
/*******************************************************
Function:
	Timer interrupt service routine.

Input:
	timer:	timer struct pointer.
	
Output:
	Timer work mode. HRTIMER_NORESTART---not restart mode
*******************************************************/
static enum hrtimer_restart goodix_ts_timer_handler(struct hrtimer *timer)
{
    struct goodix_ts_data *ts = container_of(timer, struct goodix_ts_data, timer);
	
    GTP_DEBUG_FUNC();

    queue_work(goodix_wq, &ts->work);
    hrtimer_start(&ts->timer, ktime_set(0, (GTP_POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
    return HRTIMER_NORESTART;
}

#endif

/*******************************************************
Function:
	External interrupt service routine.

Input:
	irq:	interrupt number.
	dev_id: private data pointer.
	
Output:
	irq execute status.
*******************************************************/

static irqreturn_t goodix_ts_irq_handler(int irq, void *dev_id)
{	    
     struct goodix_ts_data *ts = (struct goodix_ts_data *)dev_id;
	 dprintk(DEBUG_INT_INFO, "==========------TS Interrupt-----============\n");  

	 GTP_DEBUG_FUNC();
     gtp_irq_disable(ts);
	 queue_work(goodix_wq, &ts->work);
	 return 0;
}


/*******************************************************
Function:
	Reset chip Function.

Input:
	ms:reset time.
	
Output:
	None.
*******************************************************/
void gtp_reset_guitar(s32 ms)
{
#if 0 //gandy
    GTP_DEBUG_FUNC();
    GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
    msleep(ms);

    GTP_GPIO_AS_INPUT(GTP_RST_PORT);
    msleep(50);

    return;
#endif

}

/*******************************************************
Function:
	Eter sleep function.

Input:
	ts:private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_enter_sleep(struct goodix_ts_data * ts)
{
    s8 ret = -1;
    s8 retry = 0;
    u8 i2c_control_buf[3] = {(u8)(GTP_REG_SLEEP >> 8), (u8)GTP_REG_SLEEP, 0x01};

    GTP_DEBUG_FUNC();
	gtp_set_int_value(0);

    while(retry++ < 5)
    {
        ret = gtp_i2c_write(ts->client, i2c_control_buf, 3);
        gtp_i2c_end_cmd(ts->client);
        if (ret > 0)
        {
            GTP_DEBUG("GTP enter sleep!");
            return ret;
        }
        msleep(10);
    }
    GTP_ERROR("GTP send sleep cmd failed.");
    return ret;
}

/*******************************************************
Function:
	Wakeup from sleep mode Function.

Input:
	ts:	private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_wakeup_sleep(struct goodix_ts_data * ts)
{
    u8 retry = 0;
    s8 ret = -1;

    GTP_DEBUG_FUNC();

	//ctp_wakeup(0,50);
    gtp_io_init(20);
    gtp_set_io_int();
#if GTP_POWER_CTRL_SLEEP
    while(retry++ < 5)
    {
       // gtp_reset_guitar(20);
        ret = gtp_send_cfg(ts->client);
        if (ret > 0)
        {
            GTP_DEBUG("Wakeup sleep send config success.");
            return ret;
        }
    }
#else
    while(retry++ < 10)
    {
        ret = gtp_i2c_test(ts->client);
        if (ret > 0)
        {
            GTP_DEBUG("GTP wakeup sleep.");
            return ret;
        }
        //gtp_reset_guitar(20);
    }
#endif

    GTP_ERROR("GTP wakeup sleep failed.");
    return ret;
}

/*******************************************************
Function:
	GTP read sensor id function.

Input:
	client:	i2c client private struct.
	
Output:
	sensor ID.
*******************************************************/
/* static u8 gtp_get_sensor_id(struct i2c_client *client)
 * {
 *     u8 buf[8] = {0};
 *     u8 sensor_id = 0;
 *     u8 i = 0;
 *     u8 count = 0;
 *     
 *     // step 1: setup sensorID port as input
 *     buf[0] = 0x16;
 *     buf[1] = 0x00;
 *     gtp_i2c_read(client, buf, 3);
 *     buf[2] &= 0xfd;
 *     gtp_i2c_write(client, buf, 3);
 * 
 *     // step2: setup SensorID as pullup, shutdown SensorID pulldown
 *     buf[0] = 0x16;
 *     buf[1] = 0x06;
 *     gtp_i2c_read(client, buf, 4);
 *     buf[2] |= 0x02;
 *     buf[3] &= 0xfd;
 *     gtp_i2c_write(client, buf, 4);
 *     
 *     msleep(1);
 *     // step3: read 0x1602, result and 0x02, test equal 0, repeat 200 times
 *     count = 0;
 *     for (i = 0; i < 200; i++)
 *     {
 *         buf[0] = 0x16;
 *         buf[1] = 0x02;
 *         gtp_i2c_read(client, buf, 3);
 *         buf[2] &= 0x02;
 *         if (buf[2] == 0)
 *         {
 *             ++count;
 *         }
 *     }
 *     // if count greater than 100, then assign sensorid as 2
 *     if (count >= 100)
 *     {
 *         GTP_DEBUG("count = %d", count);
 *         sensor_id = 2;
 *         goto SENSOR_ID_NONC;
 *     }
 *     
 *     // step4: setup SensorID as pulldown, shutdown SensorID pullup
 *     buf[0] = 0x16;
 *     buf[1] = 0x06;
 *     gtp_i2c_read(client, buf, 4);
 *     buf[2] &= 0xfd;
 *     buf[3] |= 0x02;
 *     gtp_i2c_write(client, buf, 4);
 *     
 *     msleep(1);
 *     count = 0;
 *     // step 5: do the same as step 3
 *     for (i = 0; i < 200; ++i)
 *     {
 *         buf[0] = 0x16;
 *         buf[1] = 0x02;
 *         gtp_i2c_read(client, buf, 3);
 *         buf[2] &= 0x02;
 *         if (buf[2] != 0)
 *         {
 *             ++count;
 *         }	
 *     }
 *     if (count >= 100)
 *     {
 *         GTP_DEBUG("count = %d", count);
 *         sensor_id = 1;
 *         goto SENSOR_ID_NONC;
 *     }
 *     
 *     sensor_id = 0;
 *     goto SENSOR_ID_NC;
 *     
 * SENSOR_ID_NONC:
 *     buf[0] = 0x16;
 *     buf[1] = 0x06;
 *     gtp_i2c_read(client, buf, 4);
 *     buf[2] &= 0xfd;
 *     buf[3] &= 0xfd;
 *     gtp_i2c_write(client, buf, 4);
 * 
 * SENSOR_ID_NC:
 *     return sensor_id;
 * }
 */

/*******************************************************
Function:
	GTP initialize function.

Input:
	ts:	i2c client private struct.
	
Output:
	Executive outcomes.0---succeed.
*******************************************************/
static s32 gtp_init_panel(struct goodix_ts_data *ts)
{
    s32 ret = -1;
	int index;
  
#if GTP_DRIVER_SEND_CFG
    u8 rd_cfg_buf[16];

    u8 cfg_info_group1[] = CTP_CFG_GROUP1;//yiju dpt
    u8 cfg_info_group2[] = CTP_CFG_GROUP2;//chuanzuoyue
    u8 cfg_info_group3[] = CTP_CFG_GROUP3;//chuyi
    u8 cfg_info_group4[] = CTP_CFG_GROUP4;//yiju dpt HD
    u8 cfg_info_group5[] = CTP_CFG_GROUP5;//chuanzuoyue  HD
    u8 cfg_info_group6[] = CTP_CFG_GROUP6;//chuyi  HD

    u8 *send_cfg_buf[6] = {cfg_info_group1,cfg_info_group2,cfg_info_group3,cfg_info_group4,cfg_info_group5,cfg_info_group6};

    u8 cfg_info_len[6] = {sizeof(cfg_info_group1)/sizeof(cfg_info_group1[0]),
    						sizeof(cfg_info_group2)/sizeof(cfg_info_group2[0]),
    						sizeof(cfg_info_group3)/sizeof(cfg_info_group3[0]),
                                                sizeof(cfg_info_group4)/sizeof(cfg_info_group4[0]),
                                                sizeof(cfg_info_group5)/sizeof(cfg_info_group5[0]),
                                                sizeof(cfg_info_group6)/sizeof(cfg_info_group6[0]),
                                                };

    GTP_DEBUG("len1=%d,len2=%d,len3=%d",cfg_info_len[0],cfg_info_len[1],cfg_info_len[2]);
#if 0 //gandy
	if ((!cfg_info_len[1]) && (!cfg_info_len[2]))
    {
        rd_cfg_buf[GTP_ADDR_LENGTH] = 0; 
    }
    else
#endif
    {
    #if GTP_USE_868_968M
        //GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
        //gtp_reset_guitar(20);
        //rd_cfg_buf[GTP_ADDR_LENGTH] = gtp_get_sensor_id(ts->client);
        //GTP_GPIO_OUTPUT(GTP_INT_PORT, 1);
        //gtp_reset_guitar(20);
        //GTP_GPIO_AS_INT(GTP_INT_PORT);
    #else
        rd_cfg_buf[0] = GTP_REG_SENSOR_ID >> 8;
        rd_cfg_buf[1] = GTP_REG_SENSOR_ID & 0xff;
        ret = gtp_i2c_read(ts->client, rd_cfg_buf, 3);
        gtp_i2c_end_cmd(ts->client);
        if (ret < 0)
        {
            GTP_ERROR("Read SENSOR ID failed,default use group1 config!");
            rd_cfg_buf[GTP_ADDR_LENGTH] = 0;
        }
        rd_cfg_buf[GTP_ADDR_LENGTH] &= 0x03;
    #endif
    }
    GTP_DEBUG("SENSOR ID:%d", rd_cfg_buf[GTP_ADDR_LENGTH]);
    pr_debug("SENSOR ID:%d", rd_cfg_buf[GTP_ADDR_LENGTH]);
    if (rd_cfg_buf[GTP_ADDR_LENGTH] > 2)
    {
        GTP_ERROR("Invalid Sensor ID.");
        rd_cfg_buf[GTP_ADDR_LENGTH] = 0;
    }
	
	index =0;
    if(screen_max_x==800)
    {
	if(rd_cfg_buf[GTP_ADDR_LENGTH] == 0)
		index = 0;
	else if(rd_cfg_buf[GTP_ADDR_LENGTH] == 1)
		index = 1;
	else
		index = 2;
    }
    else
    {
        if(rd_cfg_buf[GTP_ADDR_LENGTH] == 0)
		index = 3;
	else if(rd_cfg_buf[GTP_ADDR_LENGTH] == 1)
		index = 4;
	else
		index = 5;

    }
    memcpy(&config[GTP_ADDR_LENGTH], send_cfg_buf[index], GTP_CONFIG_LENGTH);

#if GTP_USE_868_968M
    ts->coor_div_2 = 0;
    if (config[GTP_ADDR_LENGTH + DRVCNT_LOC] == 0x0f)
    {
        ts->coor_div_2 = 1;
    }
    GTP_DEBUG("ts drv = 0x%x, coor_div_2 = %d", config[GTP_ADDR_LENGTH + DRVCNT_LOC], ts->coor_div_2);
#endif

#if (GTP_CUSTOM_CFG || GTP_USE_868_968M)
    config[RESOLUTION_LOC]     = (u8)(GTP_MAX_WIDTH);
    config[RESOLUTION_LOC + 1] = (u8)(GTP_MAX_WIDTH>>8);
    config[RESOLUTION_LOC + 2] = (u8)GTP_MAX_HEIGHT;
    config[RESOLUTION_LOC + 3] = (u8)(GTP_MAX_HEIGHT>>8);
    if (GTP_INT_TRIGGER == 0)  //FALLING
    {
        config[TRIGGER_LOC] &= 0xf7; 
    }
    else if (GTP_INT_TRIGGER == 1)  //RISING
    {
        config[TRIGGER_LOC] |= 0x08;
    }
#endif  //endif GTP_CUSTOM_CFG

#else //else DRIVER NEED NOT SEND CONFIG

    ret = gtp_i2c_read(ts->client, config, GTP_CONFIG_LENGTH + GTP_ADDR_LENGTH);
    gtp_i2c_end_cmd(ts->client);
    if (ret < 0)
    {
        GTP_ERROR("GTP read resolution & max_touch_num failed, use default value!");
        ts->abs_x_max = GTP_MAX_WIDTH;
        ts->abs_y_max = GTP_MAX_HEIGHT;
        ts->int_trigger_type = GTP_INT_TRIGGER;
    }
#endif //endif GTP_DRIVER_SEND_CFG

    GTP_DEBUG_FUNC();

    if (!ts->coor_div_2)
    {
        ts->abs_x_max = (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC];
        ts->abs_y_max = (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2];
    }
    else // enlarged height, assign touchscreen height the correct one
    {
        ts->abs_x_max = GTP_MAX_WIDTH;
        ts->abs_y_max = GTP_MAX_HEIGHT;
    }
    
    ts->int_trigger_type = (config[TRIGGER_LOC] >> 3) & 0x01;
    GTP_DEBUG("config_x = %d, config_y = %d",  (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC], (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2]);
    GTP_DEBUG("abs_x_max = %d, abs_y_max = %d", ts->abs_x_max, ts->abs_y_max);   

    if ( (!ts->abs_x_max) || (!ts->abs_y_max))
    {
        GTP_ERROR("GTP resolution & max_touch_num invalid, use default value!");
        ts->abs_x_max = GTP_MAX_WIDTH;
        ts->abs_y_max = GTP_MAX_HEIGHT;
    }

    ret = gtp_send_cfg(ts->client);
    if (ret < 0)
    {
        GTP_ERROR("Send config error. ret = %d", ret);
    }

    GTP_DEBUG("X_MAX = %d,Y_MAX = %d,TRIGGER = 0x%02x",ts->abs_x_max,ts->abs_y_max,ts->int_trigger_type);
    msleep(10);

    return 0;
}

/*******************************************************
Function:
	Read goodix touchscreen version function.

Input:
	client:	i2c client struct.
	version:address to store version info
	
Output:
	Executive outcomes.0---succeed.
*******************************************************/
s32 gtp_read_version(struct i2c_client *client, u16* version)
{
    s32 ret = -1;
    u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};
    struct goodix_ts_data* ts = i2c_get_clientdata(client);

    GTP_DEBUG_FUNC();

    ret = gtp_i2c_read(client, buf, 6);
    gtp_i2c_end_cmd(client);
    if (ret < 0)
    {
        GTP_ERROR("GTP read version failed"); 
        return ret;
    }

    if (version)
    {
        *version = (buf[5] << 8) | buf[4];
    }

    GTP_INFO("IC VERSION:%02x%02x_%02x%02x", buf[3], buf[2], buf[5], buf[4]);
    if(0x68 == buf[3])
    {
        ts->chip_type = GT868;
    }
    else
    {
        ts->chip_type = GT818X;
    }

    return ret;
}

/*******************************************************
Function:
	I2c test Function.

Input:
	client:i2c client.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_i2c_test(struct i2c_client *client)
{
    u8 retry = 0;
    s8 ret = -1;
  
    GTP_DEBUG_FUNC();
  
    while(retry++ < 5)
    {
        ret = gtp_i2c_end_cmd(client);
        if (ret > 0)
        {
            return ret;
        }
        GTP_ERROR("GTP i2c test failed time %d.",retry);
        msleep(10);
    }
    return ret;
}


#if 0
/*******************************************************
Function:
	Request gpio Function.

Input:
	ts:private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_request_io_port(struct goodix_ts_data *ts)
{
#if 0
    s32 ret = 0;

    ret = GTP_GPIO_REQUEST(GTP_INT_PORT, "GTP_INT_IRQ");
    if (ret < 0) 
    {
        GTP_ERROR("Failed to request GPIO:%d, ERRNO:%d", (s32)GTP_INT_PORT, ret);
        ret = -ENODEV;
    }
    else
    {
        GTP_GPIO_AS_INT(GTP_INT_PORT);	
        ts->client->irq = GTP_INT_IRQ;
    }

    ret = GTP_GPIO_REQUEST(GTP_RST_PORT, "GTP_RST_PORT");
    if (ret < 0) 
    {
        GTP_ERROR("Failed to request GPIO:%d, ERRNO:%d",(s32)GTP_RST_PORT,ret);
        ret = -ENODEV;
    }

    GTP_GPIO_AS_INPUT(GTP_RST_PORT);
    //gtp_reset_guitar(20);
    
    if(ret < 0)
    {
        GTP_GPIO_FREE(GTP_RST_PORT);
        GTP_GPIO_FREE(GTP_INT_PORT);
    }

    return ret;
#endif
	return 0;
}

/*******************************************************
Function:
	Request irq Function.

Input:
	ts:private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_request_irq(struct goodix_ts_data *ts)
{
#if 0
    s32 ret = -1;
    const u8 irq_table[2] = GTP_IRQ_TAB;

    GTP_DEBUG("INT trigger type:%x", ts->int_trigger_type);

    ret  = request_irq(ts->client->irq, 
                       goodix_ts_irq_handler,
                       irq_table[ts->int_trigger_type],
                       ts->client->name,
                       ts);
    if (ret)
    {
        GTP_ERROR("Request IRQ failed!ERRNO:%d.", ret);
        GTP_GPIO_AS_INPUT(GTP_INT_PORT);
        GTP_GPIO_FREE(GTP_INT_PORT);

        hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        ts->timer.function = goodix_ts_timer_handler;
        hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
        return -1;
    }
    else 
    {
        gtp_irq_disable(ts);
        ts->use_irq = 1;
        return 0;
    }
#endif
	return 0;
}
#endif


/*******************************************************
Function:
	Request input device Function.

Input:
	ts:private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_request_input_dev(struct goodix_ts_data *ts)
{
    s8 ret = -1;
    u16 report_max_x = 0;
    u16 report_max_y = 0;
#if GTP_HAVE_TOUCH_KEY
    u8 index = 0;
#endif
  
    GTP_DEBUG_FUNC();
  
    ts->input_dev = input_allocate_device();
    if (ts->input_dev == NULL)
    {
        GTP_ERROR("Failed to allocate input device.");
        return -ENOMEM;
    }

    ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
 //   ts->input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
#if GTP_ICS_SLOT_REPORT
    __set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
    input_mt_init_slots(ts->input_dev, 255);
#else
    ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
#endif

#if GTP_HAVE_TOUCH_KEY
    for (index = 0; index < GTP_MAX_KEY_NUM; index++)
    {
        input_set_capability(ts->input_dev,EV_KEY,touch_key_array[index]);	
    }
#endif

    report_max_x = ts->abs_x_max;
    report_max_y = ts->abs_y_max;
#if GTP_CHANGE_X2Y
    GTP_SWAP(report_max_x, report_max_y);
#endif
    set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);//gandy
    /*
    input_set_abs_params(ts->input_dev, ABS_X, 0, screen_max_x, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_Y, 0, screen_max_y, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, screen_max_x, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, screen_max_y, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);	
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
    */
    ts->input_dev->absbit[0] = BIT_MASK(ABS_MT_TRACKING_ID) |
                 BIT_MASK(ABS_MT_TOUCH_MAJOR)| BIT_MASK(ABS_MT_WIDTH_MAJOR) |
                 BIT_MASK(ABS_MT_POSITION_X) | BIT_MASK(ABS_MT_POSITION_Y); 	// for android
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, screen_max_x, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, screen_max_y, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 5, 0, 0);

    ts->input_dev->name = goodix_ts_name;
    ts->input_dev->phys = "input/goodix-ts";
    ts->input_dev->id.bustype = BUS_I2C;
    ts->input_dev->id.vendor = 0xDEAD;
    ts->input_dev->id.product = 0xBEEF;
    ts->input_dev->id.version = 10427;
	
    ret = input_register_device(ts->input_dev);
    if (ret)
    {
        GTP_ERROR("Register %s input device failed", ts->input_dev->name);
        return -ENODEV;
    }
    
#ifdef CONFIG_HAS_EARLYSUSPEND
    ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    ts->early_suspend.suspend = goodix_ts_early_suspend;
    ts->early_suspend.resume = goodix_ts_late_resume;
    register_early_suspend(&ts->early_suspend);
#endif

    return 0;
}


static void goodix_init_events (struct work_struct *work)
{
	int ret = 0;
        u16 version_info;
	ctp_wakeup(0,20);
	ret = gtp_init_panel(ts_init);
	if(ret < 0) {
		pr_err("init panel fail!\n");
		return;
	}else {
		dprintk(DEBUG_INIT,"init panel succeed!\n");
        }

	 ret = gtp_read_version(ts_init->client, &version_info);
          if (ret < 0)
         {
                GTP_ERROR("Read version failed.");
         }
         GTP_INFO("Chip type:%s.", ts_init->chip_type==GT818X ? "GT818X" : "GT868");

		config_info.dev = &(ts_init->input_dev->dev);
	ret = input_request_int(&(config_info.input_type), goodix_ts_irq_handler,
				CTP_IRQ_MODE, ts_init);
        ts_init->use_irq = 1;
         gtp_irq_enable(ts_init);
	if (ret) {
		pr_info( "goodix_probe: request irq failed\n");
		return;
	}
	return;
}



/*******************************************************
Function:
	Goodix touchscreen probe function.

Input:
	client:	i2c device struct.
	id:device id.
	
Output:
	Executive outcomes. 0---succeed.
*******************************************************/
static int goodix_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    s32 ret = -1;
    struct goodix_ts_data *ts;

    GTP_DEBUG_FUNC();
    client->addr = 0x5d;
    
    //do NOT remove these output log
    GTP_INFO("GTP Driver Version:%s",GTP_DRIVER_VERSION);
    GTP_INFO("GTP Driver build@%s,%s", __TIME__,__DATE__);
    GTP_INFO("GTP I2C address:0x%02x", client->addr);
	pr_debug("gandy-----start gt818 probe\n");
    i2c_connect_client = client;
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
    {
        GTP_ERROR("I2C check functionality failed.");
        return -ENODEV;
    }
    ts = kzalloc(sizeof(*ts), GFP_KERNEL);
    if (ts == NULL)
    {
        GTP_ERROR("Alloc GFP_KERNEL memory failed.");
        return -ENOMEM;
    }
    
    memset(ts, 0, sizeof(*ts));
    INIT_WORK(&ts->work, goodix_ts_work_func);
    ts->client = client;
    i2c_set_clientdata(client, ts);
    spin_lock_init(&ts->irq_lock);
#if 0
    ret = gtp_request_io_port(ts);
    if (ret < 0)
    {
        GTP_ERROR("GTP request IO port failed.");
        kfree(ts);
        return ret;
    }
#endif

    ret = gtp_i2c_test(client);
    if (ret < 0)
    {
        GTP_ERROR("I2C communication ERROR!");
    }

    goodix_wq = create_singlethread_workqueue("goodix_wq");
    if (!goodix_wq)
    {
        GTP_ERROR("Creat workqueue failed.");
		kfree(ts);
        return -ENOMEM;
    }
#if GTP_AUTO_UPDATE
    ret = gup_init_update_proc(ts);
    if (ret < 0)
    {
        GTP_ERROR("Create update thread error.");
    }
#endif

    ret = gtp_request_input_dev(ts);
    if (ret < 0)
    {
        GTP_ERROR("GTP request input dev failed");
    }
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

#if GTP_CREATE_WR_NODE
    init_wr_node(client);
#endif

#if GTP_ESD_PROTECT
    INIT_DELAYED_WORK(&gtp_esd_check_work, gtp_esd_check_func);
    gtp_esd_check_workqueue = create_workqueue("gtp_esd_check");
    queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE); 
#endif
	pr_debug("gt818--------probe success\n");

    return 0;
}


/*******************************************************
Function:
	Goodix touchscreen driver release function.

Input:
	client:	i2c device struct.
	
Output:
	Executive outcomes. 0---succeed.
*******************************************************/
static int goodix_ts_remove(struct i2c_client *client)
{
    struct goodix_ts_data *ts = i2c_get_clientdata(client);
	
    GTP_DEBUG_FUNC();
	
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&ts->early_suspend);
#endif

#if GTP_CREATE_WR_NODE
    uninit_wr_node();
#endif
    cancel_work_sync(&goodix_init_work);
    cancel_work_sync(&goodix_resume_work);
    if (goodix_init_wq)
	destroy_workqueue(goodix_init_wq);
    if (goodix_wq)
	destroy_workqueue(goodix_wq);
#if GTP_ESD_PROTECT
    destroy_workqueue(gtp_esd_check_workqueue);
#endif

    if (ts) 
    {
        if (ts->use_irq)
        {
            //GTP_GPIO_AS_INPUT(GTP_INT_PORT);
            //GTP_GPIO_FREE(GTP_INT_PORT);
            input_free_int(&(config_info.input_type), ts);
        }
        else
        {
            hrtimer_cancel(&ts->timer);
        }
    }	
	
    GTP_INFO("GTP driver is removing...");
    i2c_set_clientdata(client, NULL);
    if(ts)
    	input_unregister_device(ts->input_dev);
    kfree(ts);

    return 0;
}


static void goodix_resume_events (struct work_struct *work)
{

         s8 ret = -1;

        ret = gtp_wakeup_sleep(ts_init);
        if (ret < 0)
        {
          GTP_ERROR("GTP later resume failed.");
        }

        if (ts_init->use_irq)
        {
            gtp_irq_enable(ts_init);
        }
         else
	{
              hrtimer_start(&ts_init->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
        }
}


/*******************************************************
Function:
	Early suspend function.

Input:
	h:early_suspend struct.
	
Output:
	None.
*******************************************************/
#ifdef CONFIG_HAS_EARLYSUSPEND
static void goodix_ts_early_suspend(struct early_suspend *h)
{
    struct goodix_ts_data *ts;
    s8 ret = -1;	
    ts = container_of(h, struct goodix_ts_data, early_suspend);
	
    GTP_DEBUG_FUNC();
#if GTP_ESD_PROTECT
    ts->gtp_is_suspend = 1;
    cancel_delayed_work_sync(&gtp_esd_check_work);
#endif

    if (ts->use_irq)
    {
        gtp_irq_disable(ts);
    }
    else
    {
        hrtimer_cancel(&ts->timer);
    }
    ret = gtp_enter_sleep(ts);
    if (ret < 0)
    {
        GTP_ERROR("GTP early suspend failed.");
    }
}

/*******************************************************
Function:
	Late resume function.

Input:
	h:early_suspend struct.
	
Output:
	None.
*******************************************************/
static void goodix_ts_late_resume(struct early_suspend *h)
{
    GTP_DEBUG_FUNC();

    queue_work(goodix_resume_wq, &goodix_resume_work);
#if GTP_ESD_PROTECT
    ts_init->gtp_is_suspend = 0;
    queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE);
#endif
}
#else
static int goodix_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct goodix_ts_data *ts = i2c_get_clientdata(client);
    s8 ret = -1;
	
    GTP_DEBUG_FUNC();
#if GTP_ESD_PROTECT
    ts->gtp_is_suspend = 1;
    cancel_delayed_work_sync(&gtp_esd_check_work);
#endif

    if (ts->use_irq)
    {
        gtp_irq_disable(ts);
    }
    else
    {
        hrtimer_cancel(&ts->timer);
    }
    ret = gtp_enter_sleep(ts);
    if (ret < 0)
    {
        GTP_ERROR("GTP early suspend failed.");
    }
    return 0;
}

/*******************************************************
Function:
	Late resume function.

Input:
	h:early_suspend struct.
	
Output:
	None.
*******************************************************/
static int goodix_ts_resume(struct i2c_client *client)
{
    GTP_DEBUG_FUNC();

    queue_work(goodix_resume_wq, &goodix_resume_work);
#if GTP_ESD_PROTECT
    ts_init->gtp_is_suspend = 0;
    queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE);
#endif
    return 0;
}
#endif

#if GTP_ESD_PROTECT
static void gtp_esd_check_func(struct work_struct *work)
{
    int i;
    int ret = -1;
    struct goodix_ts_data *ts = NULL;

    GTP_DEBUG_FUNC();
    
    ts = i2c_get_clientdata(i2c_connect_client);

    if (ts->gtp_is_suspend)
    {
        return;
    }

    for (i = 0; i < 3; i++)
    {
        ret = gtp_i2c_end_cmd(i2c_connect_client);
	    if (ret >= 0)
	    {
	        break;
	    }
	}
	
    if (i >= 3)
    {
       gtp_reset_guitar(50);
    }

    if(!ts->gtp_is_suspend)
    {
        queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE);
    }

    return;
}
#endif

static const struct i2c_device_id goodix_ts_id[] = {
    { GTP_I2C_NAME, 0 },
    { }
};

static struct i2c_driver goodix_ts_driver = {
    .class = I2C_CLASS_HWMON,
    .probe      = goodix_ts_probe,
    .remove     = goodix_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend    = goodix_ts_suspend,
    .resume     = goodix_ts_resume,
#endif
    .id_table   = goodix_ts_id,
    .driver = {
        .name     = GTP_I2C_NAME,
        .owner    = THIS_MODULE,
    },
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

		if((screen_max_x == 0) || (screen_max_y == 0)){
                pr_err("%s:read config error!\n",__func__);
                return 0;
        }
        return 1;
}

/*******************************************************	
Function:
	Driver Install function.
Input:
  None.
Output:
	Executive Outcomes. 0---succeed.
********************************************************/
static int __devinit goodix_ts_init(void)
{
    int ret = -1;

    GTP_DEBUG_FUNC();	
    GTP_INFO("GTP driver install.");
	pr_debug("gt818 init\n");

/**************add by gandy************/

/*************end gandy***************/

    dprintk(DEBUG_INIT,"****************************************************************\n");
    if (input_fetch_sysconfig_para(&(config_info.input_type))) {
	    pr_err("%s: ctp_fetch_sysconfig_para err.\n", __func__);
	return 0;
    } else {
		ret = input_init_platform_resource(&(config_info.input_type));
		if (0 != ret) {
			pr_err("%s:ctp_ops.init_platform_resource err. \n", __func__);    
		}
	}
        
    if(config_info.ctp_used == 0){
	    pr_err("*** ctp_used set to 0 !\n");
	    pr_debug("*** if use ctp,please put the sys_config.fex ctp_used set to 1. \n");
	    return 0;
    }
     
    if(!ctp_get_system_config()){
        pr_err("%s:read config fail!\n",__func__);
        return ret;
    }

    sunxi_gpio_to_name(CTP_IRQ_NUMBER,irq_pin_name);
    gtp_io_init(2);
	//ctp_wakeup(0,50);
	goodix_ts_driver.detect = ctp_detect;
    
	ret = i2c_add_driver(&goodix_ts_driver);
    return ret; 
}

/*******************************************************	
Function:
	Driver uninstall function.
Input:
  None.
Output:
	Executive Outcomes. 0---succeed.
********************************************************/
static void __exit goodix_ts_exit(void)
{
    GTP_DEBUG_FUNC();
    GTP_INFO("GTP driver exited.");
    i2c_del_driver(&goodix_ts_driver);
    if (goodix_wq)
    {
        destroy_workqueue(goodix_wq);
    }
}

late_initcall(goodix_ts_init);
module_exit(goodix_ts_exit);

MODULE_DESCRIPTION("GTP Series Driver");
MODULE_LICENSE("GPL");
