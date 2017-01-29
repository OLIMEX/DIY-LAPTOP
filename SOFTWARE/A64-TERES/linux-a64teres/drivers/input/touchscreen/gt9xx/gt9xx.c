/* drivers/input/touchscreen/gt9xx.c
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
 * Author:andrew@goodix.com
 * Release Date:2012/12/12
 * Revision record:
 *      V1.0:2012/08/31,first Release
 *      V1.2:2012/10/15,modify gtp_reset_guitar,slot report,tracking_id & 0x0F
 *      V1.4:2012/12/12,modify gt9xx_update.c
 *      
 */

#include <linux/irq.h>
#include "gt9xx_ts.h"
#include <linux/pm.h>

#if GTP_ICS_SLOT_REPORT
    #include <linux/input/mt.h>
#endif

static const char *goodix_ts_name = "gt9xx";
static struct workqueue_struct *goodix_wq;
struct i2c_client * i2c_connect_client = NULL; 
static u8 config[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH]
                = {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};

#if GTP_HAVE_TOUCH_KEY
	static const u16 touch_key_array[] = GTP_KEY_TAB;
	#define GTP_MAX_KEY_NUM	 (sizeof(touch_key_array)/sizeof(touch_key_array[0]))
#endif

static s8 gtp_i2c_test(struct i2c_client *client);
void gtp_reset_guitar(struct i2c_client *client, s32 ms);
void gtp_int_sync(s32 ms);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void goodix_ts_early_suspend(struct early_suspend *h);
static void goodix_ts_late_resume(struct early_suspend *h);
#endif
 
#if GTP_CREATE_WR_NODE
extern s32 init_wr_node(struct i2c_client*);
extern void uninit_wr_node(void);
#endif

#if GTP_AUTO_UPDATE
extern u8 gup_init_update_proc(struct goodix_ts_data *);
#endif

#if GTP_ESD_PROTECT
static struct delayed_work gtp_esd_check_work;
static struct workqueue_struct * gtp_esd_check_workqueue = NULL;
static void gtp_esd_check_func(struct work_struct *);
s32 gtp_init_ext_watchdog(struct i2c_client *client);
#endif

///////////////////////////////////////////////
//specific tp related macro: need be configured for specific tp

#define CTP_IRQ_NUMBER          (config_info.int_number)
#define CTP_IRQ_MODE		(IRQF_TRIGGER_FALLING)
#define CTP_NAME		("gt9xx_ts")
#define SCREEN_MAX_X	(screen_max_x)
#define SCREEN_MAX_Y	(screen_max_y)
#define PRESS_MAX		(255)


static int screen_max_x = 0;
static int screen_max_y = 0;
static int revert_x_flag = 0;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;
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
        printk("***CTP***"fmt, ## arg)
module_param_named(debug_mask,debug_mask,int,S_IRUGO | S_IWUSR | S_IWGRP);

static const unsigned short normal_i2c[2] = {0x5d, I2C_CLIENT_END};
//static const int chip_id_value[3] = {57};
//static uint8_t read_chip_value[3] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff,0};
struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
	.name = NULL,
	.int_number = 0,
};

//static void goodix_init_events(struct work_struct *work);
static void goodix_resume_events(struct work_struct *work);
static struct workqueue_struct *goodix_wq;
//static struct workqueue_struct *goodix_init_wq;
static struct workqueue_struct *goodix_resume_wq;
//static DECLARE_WORK(goodix_init_work, goodix_init_events);
static DECLARE_WORK(goodix_resume_work, goodix_resume_events);

/**
 * ctp_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
    int  ret = -1;
      
    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
        	printk("======return=====\n");
                return -ENODEV;
    }
        
    if(twi_id == adapter->nr){
            dprintk(DEBUG_INIT,"%s: addr = %x\n", __func__, client->addr);
            ret = gtp_i2c_test(client);
			printk("detect ret %d\n",ret);
            if(!ret){
        		printk("%s:I2C connection might be something wrong \n", __func__);
        		return -ENODEV;
        	}else{           	    
            	strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
				printk("======detect ok !=====\n");
				return 0;	
	        }
	}else{
	        return -ENODEV;
	}
}

/**
 * ctp_print_info - sysconfig print function
 * return value:
 *
 */
void ctp_print_info(struct ctp_config_info info,int debug_level)
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
int ctp_wakeup(int status,int ms)
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
			  pin_config_set(SUNXI_PINCTRL,irq_pin_name,config);;
	    }

        __gpio_set_value(CTP_IRQ_NUMBER, status);   
}

void gtp_set_io_int(void)
{
        long unsigned int	config;
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	    pin_config_get(SUNXI_PINCTRL,irq_pin_name,&config);

		if (4 != SUNXI_PINCFG_UNPACK_VALUE(config)){		
		      config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,4);		 
			  pin_config_set(SUNXI_PINCTRL,irq_pin_name,config);
	    }
        
}

void gtp_io_init(int ms)
{       
        ctp_wakeup(0, 0);
        msleep(ms);
        
        gtp_set_int_value(0);
        msleep(2);
        
        ctp_wakeup(1, 0);
        msleep(6);

        
#if GTP_ESD_PROTECT
     //   gtp_init_ext_watchdog(client);
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
        s32 ret = -1;
        s32 retries = 0;
               
        msgs[0].flags = !I2C_M_RD;
        msgs[0].addr  = client->addr;
        msgs[0].len   = GTP_ADDR_LENGTH;
        msgs[0].buf   = &buf[0];
        
        msgs[1].flags = I2C_M_RD;
        msgs[1].addr  = client->addr;
        msgs[1].len   = len - GTP_ADDR_LENGTH;
        msgs[1].buf   = &buf[GTP_ADDR_LENGTH];

        while(retries < 2) {
                ret = i2c_transfer(client->adapter, msgs, 2);
                if(ret == 2) 
                        break;
                retries++;
        }

        if(retries >= 2) {
                printk("%s:I2C retry timeout, reset chip.", __func__);
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
        s32 ret = -1;
        s32 retries = 0;
        
        msg.flags = !I2C_M_RD;
        msg.addr  = client->addr;
        msg.len   = len;
        msg.buf   = buf;
        
        while(retries < 2) {
                ret = i2c_transfer(client->adapter, &msg, 1);
                if (ret == 1) 
                        break;
                retries++;
        }

        if(retries >= 2) {
                printk("%s:I2C retry timeout, reset chip.", __func__);
        }
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
    s32 ret = 0;
    
#if GTP_DRIVER_SEND_CFG
    s32 retry = 0;

    for (retry = 0; retry < 5; retry++)
    {
        ret = gtp_i2c_write(client, config , GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH);
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
        dprintk(DEBUG_X_Y_INFO, "source data:ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);
        
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

#if GTP_ICS_SLOT_REPORT
        input_mt_slot(ts->input_dev, id);
        input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
        input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
        input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
        input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
        input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
#else
        input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
        input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
        input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
        input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
        input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
        input_mt_sync(ts->input_dev);
#endif

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
        dprintk(DEBUG_X_Y_INFO, "Touch id[%2d] release!", id);
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
        u8  end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
        u8  point_data[2 + 1 + 8 * GTP_MAX_TOUCH + 1]={GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
        u8  touch_num = 0;
        u8  finger = 0;
        static u16 pre_touch = 0;
        static u8 pre_key = 0;
        u8  key_value = 0;
        u8* coor_data = NULL;
        s32 input_x = 0;
        s32 input_y = 0;
        s32 input_w = 0;
        s32 id = 0;
        s32 i  = 0;
        s32 ret = -1;
        struct goodix_ts_data *ts = NULL;

        dprintk(DEBUG_X_Y_INFO,"===enter %s===\n",__func__);

        ts = container_of(work, struct goodix_ts_data, work);
        if (ts->enter_update){
                return;
        }

        ret = gtp_i2c_read(ts->client, point_data, 12);
        if (ret < 0){
                printk("I2C transfer error. errno:%d\n ", ret);
                goto exit_work_func;
        }

        finger = point_data[GTP_ADDR_LENGTH];    
        if((finger & 0x80) == 0) {
                goto exit_work_func;
        }

        touch_num = finger & 0x0f;
        if (touch_num > GTP_MAX_TOUCH) {
                goto exit_work_func;
        }

        if (touch_num > 1) {
                u8 buf[8 * GTP_MAX_TOUCH] = {(GTP_READ_COOR_ADDR + 10) >> 8, (GTP_READ_COOR_ADDR + 10) & 0xff};

                ret = gtp_i2c_read(ts->client, buf, 2 + 8 * (touch_num - 1)); 
                memcpy(&point_data[12], &buf[2], 8 * (touch_num - 1));
        }

#if GTP_HAVE_TOUCH_KEY
        key_value = point_data[3 + 8 * touch_num];
    
        if(key_value || pre_key) {
                for (i = 0; i < GTP_MAX_KEY_NUM; i++) {
                        input_report_key(ts->input_dev, touch_key_array[i], key_value & (0x01<<i));   
                }
                touch_num = 0;
                pre_touch = 0;
        }
#endif
        pre_key = key_value;

        dprintk(DEBUG_X_Y_INFO, "pre_touch:%02x, finger:%02x.", pre_touch, finger);

#if GTP_ICS_SLOT_REPORT
        if (pre_touch || touch_num) {
                s32 pos = 0;
                u16 touch_index = 0;
                coor_data = &point_data[3];
                
                if(touch_num) {
                        id = coor_data[pos] & 0x0F;
                        touch_index |= (0x01<<id);
                }

                dprintk(DEBUG_X_Y_INFO, 
                       "id=%d, touch_index=0x%x, pre_touch=0x%x\n", id, touch_index, pre_touch);
                
                for (i = 0; i < GTP_MAX_TOUCH; i++) {
                        if (touch_index & (0x01<<i)) {
                                input_x  = coor_data[pos + 1] | coor_data[pos + 2] << 8;
                                input_y  = coor_data[pos + 3] | coor_data[pos + 4] << 8;
                                input_w  = coor_data[pos + 5] | coor_data[pos + 6] << 8;

                                gtp_touch_down(ts, id, input_x, input_y, input_w);
                                pre_touch |= 0x01 << i;

                                pos += 8;
                                id = coor_data[pos] & 0x0F;
                                touch_index |= (0x01<<id);
                        }else {// if (pre_touch & (0x01 << i))
            
                                gtp_touch_up(ts, i);
                                pre_touch &= ~(0x01 << i);
                        }
                }
        }

#else
        if (touch_num ) {
                for (i = 0; i < touch_num; i++) {
                        coor_data = &point_data[i * 8 + 3];

                        id = coor_data[0] & 0x0F;
                        input_x  = coor_data[1] | coor_data[2] << 8;
                        input_y  = coor_data[3] | coor_data[4] << 8;
                        input_w  = coor_data[5] | coor_data[6] << 8;

                        gtp_touch_down(ts, id, input_x, input_y, input_w);
                }
        }else if(pre_touch){
                dprintk(DEBUG_X_Y_INFO, "Touch Release!");
                gtp_touch_up(ts, 0);
        }
        
        pre_touch = touch_num;

#endif

        input_sync(ts->input_dev);

exit_work_func:
        if(!ts->gtp_rawdiff_mode) {
                ret = gtp_i2c_write(ts->client, end_cmd, 3);
                if (ret < 0) {
                        printk("I2C write end_cmd  error!"); 
                }
        }
        return ;
}

/*******************************************************
Function:
	External interrupt service routine.

Input:
	irq:	interrupt number.
	dev_id: private data pointer.
	
Output:
	irq execute status.
*******************************************************/

irqreturn_t goodix_ts_irq_handler(int irq, void *dev_id)
{	    
     struct goodix_ts_data *ts = (struct goodix_ts_data *)dev_id;
	 dprintk(DEBUG_INT_INFO, "==========------TS Interrupt-----============\n");  

	 queue_work(goodix_wq, &ts->work);
	 return 0;
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
        u8 i2c_control_buf[3] = {(u8)(GTP_REG_SLEEP >> 8), (u8)GTP_REG_SLEEP, 5};
        
        dprintk(DEBUG_SUSPEND, "%s start!\n", __func__);
        
        gtp_set_int_value(0);

        while(retry++ < 2) {
                ret = gtp_i2c_write(ts->client, i2c_control_buf, 3);
                if (ret > 0) {
                        dprintk(DEBUG_SUSPEND, "GTP enter sleep!");
                        return ret;
                }
                msleep(10);
        }
        dprintk(DEBUG_SUSPEND, "GTP send sleep cmd failed.");
        
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
        
        gtp_io_init(20);
        gtp_set_io_int();
       
#if GTP_POWER_CTRL_SLEEP
    while(retry++ < 5)
    {
        ret = gtp_send_cfg(ts->client);
        if (ret > 0)
        {
            dprintk(DEBUG_SUSPEND, "Wakeup sleep send config success.");
            return ret;
        }
    }

    printk("GTP wakeup sleep failed.");
    return ret;
#endif

}


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
	  
#if GTP_DRIVER_SEND_CFG
		s32 i;
		u8 check_sum = 0;
		u8 rd_cfg_buf[16];
		int index =0;
	
		static u8 cfg_info_group1[] = CTP_CFG_GROUP1; 
		static u8 cfg_info_group2[] = CTP_CFG_GROUP2; 
		static u8 cfg_info_group3[] = CTP_CFG_GROUP3; 
		static u8 cfg_info_group4[] = CTP_CFG_GROUP4;
		static u8 cfg_info_group5[] = CTP_CFG_GROUP5;
		static u8 cfg_info_group6[] = CTP_CFG_GROUP6;
		static u8 cfg_info_group7[] = CTP_CFG_GROUP7;
		u8 *send_cfg_buf[] = {cfg_info_group1,cfg_info_group2,cfg_info_group3,cfg_info_group4,cfg_info_group5,cfg_info_group6,cfg_info_group7};
		u8 cfg_info_len[] = {sizeof(cfg_info_group1)/sizeof(cfg_info_group1[0]),\
				  sizeof(cfg_info_group2)/sizeof(cfg_info_group2[0]),\
				  sizeof(cfg_info_group3)/sizeof(cfg_info_group3[0]),\
				  sizeof(cfg_info_group4)/sizeof(cfg_info_group4[0]),\
				  sizeof(cfg_info_group5)/sizeof(cfg_info_group5[0]),\
				  sizeof(cfg_info_group6)/sizeof(cfg_info_group6[0]),\
				  sizeof(cfg_info_group7)/sizeof(cfg_info_group7[0])};
#if 0 //gandy
		for(i=0; i<3; i++)
		{
			if(cfg_info_len[i] > ts->gtp_cfg_len)
			{
				ts->gtp_cfg_len = cfg_info_len[i];
			}
		}
#endif
		
		GTP_DEBUG("len1=%d,len2=%d,len3=%d,send_len:%d",cfg_info_len[0],cfg_info_len[1],cfg_info_len[2],ts->gtp_cfg_len);
#if 0
		if ((!cfg_info_len[1]) && (!cfg_info_len[2]))
		{
			rd_cfg_buf[GTP_ADDR_LENGTH] = 0; 
		}
		else
#endif
		{
			rd_cfg_buf[0] = GTP_REG_SENSOR_ID >> 8;
			rd_cfg_buf[1] = GTP_REG_SENSOR_ID & 0xff;
			ret = gtp_i2c_read(ts->client, rd_cfg_buf, 3);
			if (ret < 0)
			{
				GTP_ERROR("Read SENSOR ID failed,default use group1 config!");
				rd_cfg_buf[GTP_ADDR_LENGTH] = 0;
			}
			rd_cfg_buf[GTP_ADDR_LENGTH] &= 0x07;
		}

#if 0		
		if(screen_max_x == 800 && screen_max_y == 480)
		{
			if(rd_cfg_buf[GTP_ADDR_LENGTH] == 3)
				index = 0;
			else if(rd_cfg_buf[GTP_ADDR_LENGTH] == 4)
				index = 1;
			else if(rd_cfg_buf[GTP_ADDR_LENGTH] == 5)
				index = 3;
			else if(rd_cfg_buf[GTP_ADDR_LENGTH] == 0)
				index = 6;
		}
		else if(screen_max_x == 1024 && screen_max_y == 600)
		{
			if(rd_cfg_buf[GTP_ADDR_LENGTH] == 0)
				index = 5;
			else if(rd_cfg_buf[GTP_ADDR_LENGTH] == 3)
				index = 2;
		}
#endif  
        GTP_DEBUG("CTP name : %s\n",config_info.name);
        if (!strcmp(config_info.name,"gt911_805d5")){
            index = 0;
            GTP_DEBUG("gt9xx:index = %d\n",index);
			
		} else if (!strcmp(config_info.name,"gt911_g912")){
            index = 2;
            GTP_DEBUG("gt9xx:index = %d\n",index);
			
		} else if (!strcmp(config_info.name,"gt911_xw785")){
            index = 3;
            GTP_DEBUG("gt9xx:index = %d\n",index);
			
		} else {		    
            index = 1; //default  p4
            GTP_DEBUG("gt9xx:index = %d\n",index);
		}
        
        //index = rd_cfg_buf[GTP_ADDR_LENGTH];
		ts->gtp_cfg_len = cfg_info_len[index];
		GTP_DEBUG("gandy---SENSOR ID:%d\n", rd_cfg_buf[GTP_ADDR_LENGTH]);
		memset(&config[GTP_ADDR_LENGTH], 0, GTP_CONFIG_MAX_LENGTH);
		memcpy(&config[GTP_ADDR_LENGTH], send_cfg_buf[index], ts->gtp_cfg_len);
	
#if GTP_CUSTOM_CFG
		config[RESOLUTION_LOC]	   = (u8)GTP_MAX_WIDTH;
		config[RESOLUTION_LOC + 1] = (u8)(GTP_MAX_WIDTH>>8);
		config[RESOLUTION_LOC + 2] = (u8)GTP_MAX_HEIGHT;
		config[RESOLUTION_LOC + 3] = (u8)(GTP_MAX_HEIGHT>>8);
		
		if (GTP_INT_TRIGGER == 0)  //RISING
		{
			config[TRIGGER_LOC] &= 0xfe; 
		}
		else if (GTP_INT_TRIGGER == 1)	//FALLING
		{
			config[TRIGGER_LOC] |= 0x01;
		}
#endif  //endif GTP_CUSTOM_CFG
		
		check_sum = 0;
		for (i = GTP_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
		{
			check_sum += config[i];
		}
		config[ts->gtp_cfg_len] = (~check_sum) + 1;
		
#else //else DRIVER NEED NOT SEND CONFIG
	
		if(ts->gtp_cfg_len == 0)
		{
			ts->gtp_cfg_len = GTP_CONFIG_MAX_LENGTH;
		}
		ret = gtp_i2c_read(ts->client, config, ts->gtp_cfg_len + GTP_ADDR_LENGTH);
		if (ret < 0)
		{
			GTP_ERROR("GTP read resolution & max_touch_num failed, use default value!");
			ts->abs_x_max = GTP_MAX_WIDTH;
			ts->abs_y_max = GTP_MAX_HEIGHT;
			ts->int_trigger_type = GTP_INT_TRIGGER;
		}
#endif //endif GTP_DRIVER_SEND_CFG
	
		GTP_DEBUG_FUNC();
	
		ts->abs_x_max = (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC];
		ts->abs_y_max = (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2];
		ts->int_trigger_type = (config[TRIGGER_LOC]) & 0x03;
		if ((!ts->abs_x_max)||(!ts->abs_y_max))
		{
			GTP_ERROR("GTP resolution & max_touch_num invalid, use default value!");
			ts->abs_x_max = GTP_MAX_WIDTH;
			ts->abs_y_max = GTP_MAX_HEIGHT;
		}
		
		msleep(100);
		ret = gtp_send_cfg(ts->client);
		if (ret < 0)
		{
			printk("\ngandy-----send config error.ret=%d\n",ret);
			GTP_ERROR("Send config error.");
		}
		printk("X_MAX = %d,Y_MAX = %d,TRIGGER = 0x%02x",
				 ts->abs_x_max,ts->abs_y_max,ts->int_trigger_type);
	
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

        dprintk(DEBUG_INIT, "%s ---start!.---\n", __func__);

        ret = gtp_i2c_read(client, buf, sizeof(buf));
        if (ret < 0) {
                printk("GTP read version failed");
                return ret;
        }

        if (version) {
                *version = (buf[7] << 8) | buf[6];
        }

        if (buf[5] == 0x00) {
                printk("IC Version: %c%c%c_%02x%02x", buf[2], buf[3], buf[4], buf[7], buf[6]);
        }
        else {
                printk("IC Version: %c%c%c%c_%02x%02x", buf[2], buf[3], buf[4], buf[5], buf[7], buf[6]);
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
        u8 test[3] = {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};
        u8 retry = 0;
        s8 ret = -1;
  
        while(retry++ < 2) {
                ret = gtp_i2c_read(client, test, 3);
                if (ret > 0) {
                        return ret;
                }
                printk("GTP i2c test failed time %d.",retry);
                msleep(10);
        }
        return ret;
}


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
#if GTP_HAVE_TOUCH_KEY
        u8 index = 0;
#endif
  
        ts->input_dev = input_allocate_device();
        if (ts->input_dev == NULL) {
                GTP_ERROR("Failed to allocate input device.");
                return -ENOMEM;
        }

        ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
#if GTP_ICS_SLOT_REPORT
        __set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
        input_mt_init_slots(ts->input_dev, 255);
#else
        ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
#endif

#if GTP_HAVE_TOUCH_KEY
        for (index = 0; index < GTP_MAX_KEY_NUM; index++) {
                input_set_capability(ts->input_dev,EV_KEY,touch_key_array[index]);	
        }
#endif

//#if GTP_CHANGE_X2Y
//        GTP_SWAP(ts->abs_x_max, ts->abs_y_max);
//#endif

        input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
        input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
        input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
        input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);	
        input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
		set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
    
        ts->input_dev->name = CTP_NAME;
        ts->input_dev->phys = "input/goodix-ts";
        ts->input_dev->id.bustype = BUS_I2C;
        ts->input_dev->id.vendor = 0xDEAD;
        ts->input_dev->id.product = 0xBEEF;
        ts->input_dev->id.version = 10427;
        ret = input_register_device(ts->input_dev);
        if (ret) {
                printk("Register %s input device failed", ts->input_dev->name);
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
        u16 version_info;   
    
        dprintk(DEBUG_INIT, "GTP Driver Version:%s\n",GTP_DRIVER_VERSION);
        dprintk(DEBUG_INIT, "GTP Driver build@%s,%s\n", __TIME__,__DATE__);
        printk("GTP I2C Address:0x%02x\n", client->addr);

        i2c_connect_client = client;
        if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
                printk("I2C check functionality failed.\n");
                return -ENODEV;
        }
        
        ts = kzalloc(sizeof(*ts), GFP_KERNEL);
        if (ts == NULL) {
                printk("Alloc GFP_KERNEL memory failed.\n");
                return -ENOMEM;
        }
   
        memset(ts, 0, sizeof(*ts));
        INIT_WORK(&ts->work, goodix_ts_work_func);
        ts->client = client;
        i2c_set_clientdata(client, ts);
        //ts->irq_lock = SPIN_LOCK_UNLOCKED;
        ts->gtp_rawdiff_mode = 0;


        ret = gtp_i2c_test(client);
        if (ret < 0){
                printk("I2C communication ERROR!\n");
		        goto exit_device_detect;
        }

	    goodix_resume_wq = create_singlethread_workqueue("goodix_resume");
	    if (goodix_resume_wq == NULL) {
		        printk("create goodix_resume_wq fail!\n");
		        return -ENOMEM;
	    }

	    goodix_wq = create_singlethread_workqueue("goodix_wq");
	    if (!goodix_wq) {
		        printk(KERN_ALERT "Creat goodix_wq workqueue failed.\n");
				return -ENOMEM;
	    }

#if GTP_AUTO_UPDATE
        ret = gup_init_update_proc(ts);
        if (ret < 0) {
                printk("Create update thread error.");
        }
#endif

        ret = gtp_init_panel(ts);
        if (ret < 0) {
                printk("GTP init panel failed.\n");
        }

        
		
	    ret = gtp_request_input_dev(ts);
        if (ret < 0) {
                printk("GTP request input dev failed\n");
		goto exit_device_detect;
        }

		ret = gtp_read_version(client, &version_info);
		if (ret < 0) {
						printk("Read version failed.");
		}
		
		config_info.dev = &(ts->input_dev->dev);
		
        ret = input_request_int(&(config_info.input_type), goodix_ts_irq_handler,CTP_IRQ_MODE, ts);	   
        if (ret) {
                printk("Request irq fail!.\n");
        }
        	
        
        spin_lock_init(&ts->irq_lock);

#if GTP_CREATE_WR_NODE
        init_wr_node(client);
#endif

#if GTP_ESD_PROTECT
        INIT_DELAYED_WORK(&gtp_esd_check_work, gtp_esd_check_func);
        gtp_esd_check_workqueue = create_workqueue("gtp_esd_check");
        queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE); 
#endif
	dprintk(DEBUG_INIT, "gt9xx probe success!\n");
        return 0;
exit_device_detect:
	i2c_set_clientdata(client, NULL);
	kfree(ts);
	return ret;
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
	
	dprintk(DEBUG_INIT,"%s start!\n", __func__);
#ifdef CONFIG_HAS_EARLYSUSPEND
        unregister_early_suspend(&ts->early_suspend);
#endif

#if GTP_CREATE_WR_NODE
        uninit_wr_node();
#endif

#if GTP_ESD_PROTECT
        flush_workqueue(gtp_esd_check_workqueue);
    if(gtp_esd_check_workqueue)
        destroy_workqueue(gtp_esd_check_workqueue);
#endif
    input_free_int(&(config_info.input_type), ts);
	flush_workqueue(goodix_wq);
	//cancel_work_sync(&goodix_init_work);
  	cancel_work_sync(&goodix_resume_work);
	if(goodix_wq)
		destroy_workqueue(goodix_wq);
  	//destroy_workqueue(goodix_init_wq);
  	if(goodix_resume_wq)
  		destroy_workqueue(goodix_resume_wq);
  	i2c_set_clientdata(ts->client, NULL);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	
    return 0;
}

static void goodix_resume_events (struct work_struct *work)
{
	int ret;
        struct goodix_ts_data *ts = i2c_get_clientdata(i2c_connect_client);
        
		ret = gtp_wakeup_sleep(ts);
		if (ret < 0)
			printk("resume power on failed\n");    	
	   gtp_irq_enable(ts);
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

#if GTP_ESD_PROTECT
        ts->gtp_is_suspend = 1;
        cancel_delayed_work_sync(&gtp_esd_check_work);
#endif

        gtp_irq_disable(ts);
        
       
        cancel_work_sync(&goodix_resume_work);
      	flush_workqueue(goodix_resume_wq);
        ret = cancel_work_sync(&ts->work);
        flush_workqueue(goodix_wq);
      
		   ret = gtp_enter_sleep(ts);
        if (ret < 0) {
                printk("GTP early suspend failed.");
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
        struct goodix_ts_data *ts;
        ts = container_of(h, struct goodix_ts_data, early_suspend);
	
        queue_work(goodix_resume_wq, &goodix_resume_work);//gandy

#if GTP_ESD_PROTECT
        ts->gtp_is_suspend = 0;
        queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE);
#endif
}
#else
#ifdef CONFIG_PM
static void goodix_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
        struct goodix_ts_data *ts;
        s8 ret = -1;	
        ts = i2c_get_clientdata(client);
        printk("%s goodix_ts_suspend\n", goodix_ts_name);
#if GTP_ESD_PROTECT
        ts->gtp_is_suspend = 1;
        cancel_delayed_work_sync(&gtp_esd_check_work);
#endif

        ret = input_set_int_enable(&(config_info.input_type), 0);
	    if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq disable failed\n", goodix_ts_name);
        cancel_work_sync(&goodix_resume_work);
    	flush_workqueue(goodix_resume_wq);
        ret = cancel_work_sync(&ts->work);
        flush_workqueue(goodix_wq);
    
	   ret = gtp_enter_sleep(ts);
        if (ret < 0) {
                printk("GTP suspend failed.");
        }
}

static void goodix_ts_resume(struct i2c_client *client)
{
        struct goodix_ts_data *ts;
        ts = i2c_get_clientdata(client);
	    printk("%s goodix_ts_resume\n", goodix_ts_name);
        queue_work(goodix_resume_wq, &goodix_resume_work);//gandy

#if GTP_ESD_PROTECT
        ts->gtp_is_suspend = 0;
        queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE);
#endif
}
#endif
#endif

#if GTP_ESD_PROTECT
/*******************************************************
Function:
    Initialize external watchdog for esd protect
Input:
    client:  i2c device.
Output:
    result of i2c write operation. 
        1: succeed, otherwise: failed
*********************************************************/
s32 gtp_init_ext_watchdog(struct i2c_client *client)
{
        u8 opr_buffer[4] = {0x80, 0x40, 0xAA, 0xAA};
        dprintk(DEBUG_INIT, "Init external watchdog...");
        return gtp_i2c_write(client, opr_buffer, 4);
}
/*******************************************************
Function:
    Esd protect function.
    Added external watchdog by meta, 2013/03/07
Input:
    work: delayed work
Output:
    None.
*******************************************************/
static void gtp_esd_check_func(struct work_struct *work)
{
        s32 i;
        s32 ret = -1;
        struct goodix_ts_data *ts = NULL;
        u8 test[4] = {0x80, 0x40};
        
        dprintk(DEBUG_INIT, "enter %s work!\n", __func__);

         ts = i2c_get_clientdata(i2c_connect_client);

        if (ts->gtp_is_suspend || ts->enter_update) {
                return;
        }
    
        for (i = 0; i < 3; i++) {
                ret = gtp_i2c_read(ts->client, test, 4);
        
                dprintk(DEBUG_INIT, "0x8040 = 0x%02X, 0x8041 = 0x%02X", test[2], test[3]);
                if ((ret < 0)) {
                        // IC works abnormally..
                        continue;
                }else { 
                        if ((test[2] == 0xAA) || (test[3] != 0xAA)) {
                                // IC works abnormally..
                                i = 3;
                                break;  
                        }else {
                                // IC works normally, Write 0x8040 0xAA
                                test[2] = 0xAA; 
                                gtp_i2c_write(ts->client, test, 3);
                                break;
                        }
                }
        }
        
        if (i >= 3) {
               GTP_DEBUG("IC Working ABNORMALLY, Resetting Guitar...");
              //  gtp_reset_guitar(ts->client, 50);
        }

        if(!ts->gtp_is_suspend) {
                queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, GTP_ESD_CHECK_CIRCLE);
        }

        return;
}
#endif

static const struct i2c_device_id goodix_ts_id[] = {
        { CTP_NAME, 0 },
        { }
};

static struct i2c_driver goodix_ts_driver = {
        .class          = I2C_CLASS_HWMON,
        .probe          = goodix_ts_probe,
        .remove         = goodix_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_PM
        .suspend        = goodix_ts_suspend,
        .resume         = goodix_ts_resume,
#endif
#endif
        .id_table       = goodix_ts_id,
        .driver = {
                .name   = CTP_NAME,
                .owner  = THIS_MODULE,
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
                printk("%s:read config error!\n",__func__);
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
        s32 ret = -1;
        dprintk(DEBUG_INIT,"****************************************************************\n");
        if (input_fetch_sysconfig_para(&(config_info.input_type))) {
			printk("%s: ctp_fetch_sysconfig_para err.\n", __func__);
			return 0;
    	} else {
			ret = input_init_platform_resource(&(config_info.input_type));
			if (0 != ret) {
				printk("%s:ctp_ops.init_platform_resource err. \n", __func__);    
			}
		}
        
        if(config_info.ctp_used == 0){
	        printk("*** ctp_used set to 0 !\n");
	        printk("*** if use ctp,please put the sys_config.fex ctp_used set to 1. \n");
	        return 0;
		}
	
        if(!ctp_get_system_config()){
                printk("%s:read config fail!\n",__func__);
                return ret;
        }
        sunxi_gpio_to_name(CTP_IRQ_NUMBER,irq_pin_name);
        gtp_io_init(20);
		
		goodix_ts_driver.detect = ctp_detect;
        ret = i2c_add_driver(&goodix_ts_driver);
        
        dprintk(DEBUG_INIT,"****************************************************************\n");
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
        printk("GTP driver exited.\n");
        i2c_del_driver(&goodix_ts_driver);
        input_free_platform_resource(&(config_info.input_type));       
}

late_initcall(goodix_ts_init);
module_exit(goodix_ts_exit);

MODULE_DESCRIPTION("GTP Series Driver");
MODULE_LICENSE("GPL");

