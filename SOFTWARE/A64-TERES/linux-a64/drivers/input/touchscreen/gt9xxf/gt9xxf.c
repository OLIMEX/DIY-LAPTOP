/* drivers/input/touchscreen/gt9xx.c
 * 
 * 2010 - 2013 Goodix Technology.
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
 * Version: 2.0
 * Authors: andrew@goodix.com, meta@goodix.com
 * Release Date: 2013/04/25
 * Revision record:
 *      V1.0:   
 *          first Release. By Andrew, 2012/08/31 
 *      V1.2:
 *          modify gtp_reset_guitar,slot report,tracking_id & 0x0F. By Andrew, 2012/10/15
 *      V1.4:
 *          modify gt9xx_update.c. By Andrew, 2012/12/12
 *      V1.6: 
 *          1. new heartbeat/esd_protect mechanism(add external watchdog)
 *          2. doze mode, sliding wakeup 
 *          3. 3 more cfg_group(GT9 Sensor_ID: 0~5) 
 *          3. config length verification
 *          4. names & comments
 *                  By Meta, 2013/03/11
 *      V1.8:
 *          1. pen/stylus identification 
 *          2. read double check & fixed config support
 *          3. new esd & slide wakeup optimization
 *                  By Meta, 2013/06/08
 *      V2.0:
 *          1. compatible with GT9XXF
 *          2. send config after resume
 *                  By Meta, 2013/08/06
 */

#include <linux/irq.h>
#include "gt9xxf.h"

#if GTP_ICS_SLOT_REPORT
#include <linux/input/mt.h>
#endif

static void goodix_init_events(struct work_struct *work);
static void goodix_resume_events(struct work_struct *work);
struct workqueue_struct *goodix_init_wq;
struct workqueue_struct *goodix_resume_wq;
static DECLARE_WORK(goodix_init_work, goodix_init_events);
static DECLARE_WORK(goodix_resume_work, goodix_resume_events);
struct goodix_ts_data *ts_init;

static struct workqueue_struct *goodix_wq;
struct i2c_client * i2c_connect_client = NULL; 
u8 config[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH]
= {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};

#if GTP_HAVE_TOUCH_KEY
static const u16 touch_key_array[] = GTP_KEY_TAB;
#define GTP_MAX_KEY_NUM  (sizeof(touch_key_array)/sizeof(touch_key_array[0]))

#if GTP_DEBUG_ON
static const int  key_codes[] = {KEY_HOME, KEY_BACK, KEY_MENU, KEY_SEARCH};
static const char *key_names[] = {"Key_Home", "Key_Back", "Key_Menu", "Key_Search"};
#endif

#endif

static s8 gtp_i2c_test(struct i2c_client *client);
void gtp_reset_guitar(struct i2c_client *client, s32 ms);
s32 gtp_send_cfg(struct i2c_client *client);
void gtp_int_sync(s32 ms);

#include <linux/pm_runtime.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
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
static s32 gtp_init_ext_watchdog(struct i2c_client *client);
void gtp_esd_switch(struct i2c_client *, s32);
#endif

#define CTP_IRQ_NUMBER          (config_info.int_number)
#define CTP_RST_NUMBER          (config_info.wakeup_gpio.gpio)
#define CTP_IRQ_MODE		(IRQF_TRIGGER_FALLING)
#define CTP_NAME			GTP_I2C_NAME
#define SCREEN_MAX_HEIGHT		(screen_max_x)
#define SCREEN_MAX_WIDTH		(screen_max_y)
#define PRESS_MAX			(255)
#define PRINT_POINT_INFO        

struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
	.name = NULL,
	.int_number = 0,
};


static const unsigned short normal_i2c[2] = {0x5d,I2C_CLIENT_END};
const char *goodix_ts_name="gt9xxf_ts";

//static int gtp_ref_retries = 0;

static int screen_max_x = 0;
static int screen_max_y = 0;
static int revert_x_flag = 0;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;
//static int ctp_firm = 0;
static __u32 twi_id = 0;

//static u32 io_gpio_number=0;
static char irq_pin_name[8];
static char rst_pin_name[8];

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


//*********** For GT9XXF Start **********//
#if GTP_COMPATIBLE_MODE
extern s32 i2c_read_bytes(struct i2c_client *client, u16 addr, u8 *buf, s32 len);
extern s32 i2c_write_bytes(struct i2c_client *client, u16 addr, u8 *buf, s32 len);
extern s32 gup_clk_calibration(void);
extern s32 gup_fw_download_proc(void *dir, u8 dwn_mode);
extern u8 gup_check_fs_mounted(char *path_name);

void gtp_recovery_reset(struct i2c_client *client);
static s32 gtp_esd_recovery(struct i2c_client *client);
s32 gtp_fw_startup(struct i2c_client *client);
static s32 gtp_main_clk_proc(struct goodix_ts_data *ts);
static s32 gtp_bak_ref_proc(struct goodix_ts_data *ts, u8 mode);
#endif
//********** For GT9XXF End **********//

#if GTP_SLIDE_WAKEUP
typedef enum
{
	DOZE_DISABLED = 0,
	DOZE_ENABLED = 1,
	DOZE_WAKEUP = 2,
}DOZE_T;
static DOZE_T doze_status = DOZE_DISABLED;
static s8 gtp_enter_doze(struct goodix_ts_data *ts);
#endif
static u8 chip_gt9xxs = 0;  // true if ic is gt9xxs, like gt915s
u8 grp_cfg_version = 0;
/**
 * ctp_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    
	struct i2c_adapter *adapter = client->adapter;
	//u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
		printk("======return=====\n");
		return -ENODEV;
	}
	if(twi_id == adapter->nr){
		strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
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
			  pin_config_set(SUNXI_PINCTRL,irq_pin_name,config);
	    }
        
        __gpio_set_value(CTP_IRQ_NUMBER, status);   
}

void gtp_set_io_int(void)
{
        long unsigned int	config;
		
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	    pin_config_get(SUNXI_PINCTRL,irq_pin_name,&config);

		if (6 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		      config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,6);
			  pin_config_set(SUNXI_PINCTRL,irq_pin_name,config);
	    }
        
}

void gtp_set_rst_in(void)
{
        long unsigned int	config;
		
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	    pin_config_get(SUNXI_PINCTRL,rst_pin_name,&config);

		if (0 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		      config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0);
			  pin_config_set(SUNXI_PINCTRL,rst_pin_name,config);
	    }
        
}


void gtp_io_init(int ms)
{       
        ctp_wakeup(0, 0);
        msleep(ms);
        
        gtp_set_int_value(1);
        msleep(2);
        
        ctp_wakeup(1, 0);
        msleep(6);

        
#if GTP_ESD_PROTECT
     //   gtp_init_ext_watchdog(i2c_connect_client);
#endif
        
}


/*******************************************************
Function:
Read data from the i2c slave device.
Input:
client:     i2c device.
buf[0~1]:   read start address.
buf[2~len-1]:   read data buffer.
len:    GTP_ADDR_LENGTH + read bytes count
Output:
numbers of i2c_msgs to transfer: 
2: succeed, otherwise: failed
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
	//msgs[0].scl_rate = 300 * 1000;    // for Rockchip, etc.

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr  = client->addr;
	msgs[1].len   = len - GTP_ADDR_LENGTH;
	msgs[1].buf   = &buf[GTP_ADDR_LENGTH];
	//msgs[1].scl_rate = 300 * 1000;

	while(retries < 5)
	{
		ret = i2c_transfer(client->adapter, msgs, 2);
		if(ret == 2)break;
		retries++;
	}
	if((retries >= 5))
	{
#if GTP_COMPATIBLE_MODE
		struct goodix_ts_data *ts = i2c_get_clientdata(client);
#endif

#if GTP_SLIDE_WAKEUP
		// reset chip would quit doze mode
		if (DOZE_ENABLED == doze_status)
		{
			return ret;
		}
#endif
		GTP_ERROR("I2C Read: 0x%04X, %d bytes failed, errcode: %d! Process reset.", (((u16)(buf[0] << 8)) | buf[1]), len-2, ret);
#if GTP_COMPATIBLE_MODE
		if (CHIP_TYPE_GT9F == ts->chip_type)
		{
			gtp_recovery_reset(client);
		}
		else
#endif
		{
			gtp_reset_guitar(client, 10);  
		}
	}
	return ret;
}



/*******************************************************
Function:
Write data to the i2c slave device.
Input:
client:     i2c device.
buf[0~1]:   write start address.
buf[2~len-1]:   data buffer
len:    GTP_ADDR_LENGTH + write bytes count
Output:
numbers of i2c_msgs to transfer: 
1: succeed, otherwise: failed
 *********************************************************/
s32 gtp_i2c_write(struct i2c_client *client,u8 *buf,s32 len)
{
	struct i2c_msg msg;
	s32 ret = -1;
	s32 retries = 0;

	GTP_DEBUG_FUNC();

	msg.flags = !I2C_M_RD;
	msg.addr  = client->addr;
	msg.len   = len;
	msg.buf   = buf;
	//msg.scl_rate = 300 * 1000;    // for Rockchip, etc

	while(retries < 5)
	{
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret == 1)break;
		retries++;
	}
	if((retries >= 5))
	{
#if GTP_COMPATIBLE_MODE
		struct goodix_ts_data *ts = i2c_get_clientdata(client);
#endif

#if GTP_SLIDE_WAKEUP
		if (DOZE_ENABLED == doze_status)
		{
			return ret;
		}
#endif
		GTP_ERROR("I2C Write: 0x%04X, %d bytes failed, errcode: %d! Process reset.", (((u16)(buf[0] << 8)) | buf[1]), len-2, ret);
#if GTP_COMPATIBLE_MODE
		if (CHIP_TYPE_GT9F == ts->chip_type)
		{
			gtp_recovery_reset(client);
		}
		else
#endif
		{
			gtp_reset_guitar(client, 10);  
		}
	}
	return ret;
}


/*******************************************************
Function:
i2c read twice, compare the results
Input:
client:  i2c device
addr:    operate address
rxbuf:   read data to store, if compare successful
len:     bytes to read
Output:
FAIL:    read failed
SUCCESS: read successful
 *********************************************************/
s32 gtp_i2c_read_dbl_check(struct i2c_client *client, u16 addr, u8 *rxbuf, int len)
{
	u8 buf[16] = {0};
	u8 confirm_buf[16] = {0};
	u8 retry = 0;

	while (retry++ < 3)
	{
		memset(buf, 0xAA, 16);
		buf[0] = (u8)(addr >> 8);
		buf[1] = (u8)(addr & 0xFF);
		gtp_i2c_read(client, buf, len + 2);

		memset(confirm_buf, 0xAB, 16);
		confirm_buf[0] = (u8)(addr >> 8);
		confirm_buf[1] = (u8)(addr & 0xFF);
		gtp_i2c_read(client, confirm_buf, len + 2);

		if (!memcmp(buf, confirm_buf, len+2))
		{
			memcpy(rxbuf, confirm_buf+2, len);
			return SUCCESS;
		}
	}    
	GTP_ERROR("I2C read 0x%04X, %d bytes, double check failed!", addr, len);
	return FAIL;
}

/*******************************************************
Function:
Send config.
Input:
client: i2c device.
Output:
result of i2c write operation. 
1: succeed, otherwise: failed
 *********************************************************/

s32 gtp_send_cfg(struct i2c_client *client)
{
	s32 ret = 2;

#if GTP_DRIVER_SEND_CFG
	s32 retry = 0;
	struct goodix_ts_data *ts = i2c_get_clientdata(client);

	if (ts->fixed_cfg)
	{
		GTP_INFO("Ic fixed config, no config sent!");
		return 0;
	}
	else if (ts->pnl_init_error)
	{
		GTP_INFO("Error occured in init_panel, no config sent");
		return 0;
	}

	GTP_INFO("Driver send config.");
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
Report touch point event 
Input:
ts: goodix i2c_client private data
id: trackId
x:  input x coordinate
y:  input y coordinate
w:  input pressure
Output:
None.
 *********************************************************/
static void gtp_touch_down(struct goodix_ts_data* ts,s32 id,s32 x,s32 y,s32 w)
{
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

	GTP_DEBUG("ID:%d, X:%d, Y:%d, W:%d", id, x, y, w);
}

/*******************************************************
Function:
Report touch release event
Input:
ts: goodix i2c_client private data
Output:
None.
 *********************************************************/
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
Goodix touchscreen work function
Input:
work: work struct of goodix_workqueue
Output:
None.
 *********************************************************/
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

#if GTP_COMPATIBLE_MODE
	u8 rqst_buf[3] = {0x80, 0x43};  // for GT9XXF
#endif

#if GTP_SLIDE_WAKEUP
	u8 doze_buf[3] = {0x81, 0x4B};
#endif

	GTP_DEBUG_FUNC();
	ts = container_of(work, struct goodix_ts_data, work);
	if (ts->enter_update)
	{
		return;
	}
#if GTP_SLIDE_WAKEUP
	if (DOZE_ENABLED == doze_status)
	{               
		ret = gtp_i2c_read(i2c_connect_client, doze_buf, 3);
		GTP_DEBUG("0x814B = 0x%02X", doze_buf[2]);
		if (ret > 0)
		{               
			if (doze_buf[2] == 0xAA)
			{
				GTP_INFO("Forward slide to light up the screen!");
				doze_status = DOZE_WAKEUP;
				input_report_key(ts->input_dev, KEY_POWER, 1);
				input_sync(ts->input_dev);
				input_report_key(ts->input_dev, KEY_POWER, 0);
				input_sync(ts->input_dev);
				// clear 0x814B
				doze_buf[2] = 0x00;
				gtp_i2c_write(i2c_connect_client, doze_buf, 3);
			}
			else if (doze_buf[2] == 0xBB)
			{
				GTP_INFO("Backward slide to light up the screen!");
				doze_status = DOZE_WAKEUP;
				input_report_key(ts->input_dev, KEY_POWER, 1);
				input_sync(ts->input_dev);
				input_report_key(ts->input_dev, KEY_POWER, 0);
				input_sync(ts->input_dev);
				// clear 0x814B
				doze_buf[2] = 0x00;
				gtp_i2c_write(i2c_connect_client, doze_buf, 3);
			}
			else if (0xC0 == (doze_buf[2] & 0xC0))
			{
				GTP_INFO("Double click to light up the screen!");
				doze_status = DOZE_WAKEUP;
				input_report_key(ts->input_dev, KEY_POWER, 1);
				input_sync(ts->input_dev);
				input_report_key(ts->input_dev, KEY_POWER, 0);
				input_sync(ts->input_dev);
				// clear 0x814B
				doze_buf[2] = 0x00;
				gtp_i2c_write(i2c_connect_client, doze_buf, 3);
			}
			else
			{
				gtp_enter_doze(ts);
			}
		}
		if (ts->use_irq)
		{
			gtp_irq_enable(ts);
		}
		return;
	}
#endif

	ret = gtp_i2c_read(ts->client, point_data, 12);
	if (ret < 0)
	{
		GTP_ERROR("I2C transfer error. errno:%d\n ", ret);
		goto exit_work_func;
	}

	finger = point_data[GTP_ADDR_LENGTH];    

#if GTP_COMPATIBLE_MODE
	// GT9XXF
	if ((finger == 0x00) && (CHIP_TYPE_GT9F == ts->chip_type))     // request arrived
	{
		ret = gtp_i2c_read(ts->client, rqst_buf, 3);
		if (ret < 0)
		{
			GTP_ERROR("Read request status error!");
			goto exit_work_func;
		} 

		switch (rqst_buf[2] & 0x0F)
		{
			case GTP_RQST_CONFIG:
				GTP_INFO("Request for config.");
				ret = gtp_send_cfg(ts->client);
				if (ret < 0)
				{
					GTP_ERROR("Request for config unresponded!");
				}
				else
				{
					rqst_buf[2] = GTP_RQST_RESPONDED;
					gtp_i2c_write(ts->client, rqst_buf, 3);
					GTP_INFO("Request for config responded!");
				}
				break;

			case GTP_RQST_BAK_REF:
				GTP_INFO("Request for backup reference.");
				ret = gtp_bak_ref_proc(ts, GTP_BAK_REF_SEND);
				if (SUCCESS == ret)
				{
					rqst_buf[2] = GTP_RQST_RESPONDED;
					gtp_i2c_write(ts->client, rqst_buf, 3);
					GTP_INFO("Request for backup reference responded!");
				}
				else
				{
					GTP_ERROR("Requeset for backup reference unresponed!");
				}
				break;

			case GTP_RQST_RESET:
				GTP_INFO("Request for reset.");
				gtp_recovery_reset(ts->client);
				break;

			case GTP_RQST_MAIN_CLOCK:
				GTP_INFO("Request for main clock.");
				ts->rqst_processing = 1;
				ret = gtp_main_clk_proc(ts);
				if (FAIL == ret)
				{
					GTP_ERROR("Request for main clock unresponded!");
				}
				else
				{
					GTP_INFO("Request for main clock responded!");
					rqst_buf[2] = GTP_RQST_RESPONDED;
					gtp_i2c_write(ts->client, rqst_buf, 3);
					ts->rqst_processing = 0;
					ts->clk_chk_fs_times = 0;
				}
				break;

			case GTP_RQST_IDLE:
			default:
				break;
		}
	}
#endif

	if((finger & 0x80) == 0)
	{
		goto exit_work_func;
	}

	touch_num = finger & 0x0f;
	if (touch_num > GTP_MAX_TOUCH)
	{
		goto exit_work_func;
	}

	if (touch_num > 1)
	{
		u8 buf[8 * GTP_MAX_TOUCH] = {(GTP_READ_COOR_ADDR + 10) >> 8, (GTP_READ_COOR_ADDR + 10) & 0xff};

		ret = gtp_i2c_read(ts->client, buf, 2 + 8 * (touch_num - 1)); 
		memcpy(&point_data[12], &buf[2], 8 * (touch_num - 1));
	}

#if GTP_HAVE_TOUCH_KEY
	key_value = point_data[3 + 8 * touch_num];

	if(key_value || pre_key)
	{
		for (i = 0; i < GTP_MAX_KEY_NUM; i++)
		{
			input_report_key(ts->input_dev, touch_key_array[i], key_value & (0x01<<i));   
		}
		touch_num = 0;
		pre_touch = 0;
	}
#endif
	pre_key = key_value;

	GTP_DEBUG("pre_touch:%02x, finger:%02x.", pre_touch, finger);

#if GTP_ICS_SLOT_REPORT

	if (pre_touch || touch_num)
	{
		s32 pos = 0;
		u16 touch_index = 0;
		u8 report_num = 0;
		coor_data = &point_data[3];

		if(touch_num)
		{
			id = coor_data[pos] & 0x0F;


			touch_index |= (0x01<<id);
		}

		GTP_DEBUG("id = %d,touch_index = 0x%x, pre_touch = 0x%x\n",id, touch_index,pre_touch);
		for (i = 0; i < GTP_MAX_TOUCH; i++)
		{

			if ((touch_index & (0x01<<i)))
			{
				input_x  = coor_data[pos + 1] | (coor_data[pos + 2] << 8);
				input_y  = coor_data[pos + 3] | (coor_data[pos + 4] << 8);
				input_w  = coor_data[pos + 5] | (coor_data[pos + 6] << 8);
				if(exchange_x_y_flag)
					GTP_SWAP(input_x,input_y);
				if(revert_x_flag)
					input_x = screen_max_x - input_x;
				if(revert_y_flag)
					input_y = screen_max_y - input_y;
				gtp_touch_down(ts, id, input_x, input_y, input_w);
				pre_touch |= 0x01 << i;

				report_num++;
				if (report_num < touch_num)
				{
					pos += 8;
					id = coor_data[pos] & 0x0F;
					touch_index |= (0x01<<id);
				}
			}
			else
			{
				gtp_touch_up(ts, i);
				pre_touch &= ~(0x01 << i);
			}
		}
	}
#else
	input_report_key(ts->input_dev, BTN_TOUCH, (touch_num || key_value));
	if (touch_num)
	{
		for (i = 0; i < touch_num; i++)
		{
			coor_data = &point_data[i * 8 + 3];

			id = coor_data[0] & 0x0F;
			input_x  = coor_data[1] | (coor_data[2] << 8);
			input_y  = coor_data[3] | (coor_data[4] << 8);
			input_w  = coor_data[5] | (coor_data[6] << 8);
			if(exchange_x_y_flag)
				GTP_SWAP(input_x,input_y);
			if(revert_x_flag)
				input_x = screen_max_x - input_x;
			if(revert_y_flag)
				input_y = screen_max_y - input_y;

			gtp_touch_down(ts, id, input_x, input_y, input_w);
		}
	}
	else if (pre_touch)
	{   

		GTP_DEBUG("Touch Release!");
		gtp_touch_up(ts, 0);
	}

	pre_touch = touch_num;
#endif

	input_sync(ts->input_dev);

exit_work_func:
	if(!ts->gtp_rawdiff_mode)
	{
		ret = gtp_i2c_write(ts->client, end_cmd, 3);
		if (ret < 0)
		{
			GTP_INFO("I2C write end_cmd error!");
		}
	}
	if (ts->use_irq)
	{
		gtp_irq_enable(ts);
	}
}

#if 0
/*******************************************************
Function:
Timer interrupt service routine for polling mode.
Input:
timer: timer struct pointer
Output:
Timer work mode. 
HRTIMER_NORESTART: no restart mode
 *********************************************************/
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
External interrupt service routine for interrupt mode.
Input:
irq:  interrupt number.
dev_id: private data pointer
Output:
Handle Result.
IRQ_HANDLED: interrupt handled successfully
 *********************************************************/


irqreturn_t goodix_ts_irq_handler(int irq, void *dev_id)
{	    
     struct goodix_ts_data *ts = (struct goodix_ts_data *)dev_id;
	 dprintk(DEBUG_INT_INFO, "==========------TS Interrupt-----============\n");  
	 queue_work(goodix_wq, &ts->work);
	 return 0;
}

/*******************************************************
Function:
Synchronization.
Input:
ms: synchronization time in millisecond.
Output:
None.
 *******************************************************/
void gtp_int_sync(s32 ms)
{

	GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
	msleep(ms);
	GTP_GPIO_AS_INT(GTP_INT_PORT);

}


/*******************************************************
Function:
Reset chip.
Input:
ms: reset time in millisecond
Output:
None.
 *******************************************************/
void gtp_reset_guitar(struct i2c_client *client, s32 ms)
{
	GTP_DEBUG_FUNC();
	//sun4i_gpio_free(&gpio_wakeup_hdle);

	GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);   // begin select I2C slave addr
	//ctp_wakeup(0,0); 

	msleep(ms);                         // T2: > 10ms
	// HIGH: 0x28/0x29, LOW: 0xBA/0xBB
	GTP_GPIO_OUTPUT(GTP_INT_PORT, client->addr == 0x14);
	//ctp_wakeup((client->addr == 0x14),0);

	msleep(2);                          // T3: > 100us
    GTP_GPIO_OUTPUT(GTP_RST_PORT, 1); 
	//ctp_wakeup(1,0);

	msleep(6);                          // T4: > 5ms
	GTP_GPIO_AS_INPUT(GTP_RST_PORT);    // end select I2C slave addr

#if 0
#if GTP_COMPATIBLE_MODE
		if (CHIP_TYPE_GT9F == ts->chip_type)
		{
			return;
		}
#endif
	
		gtp_int_sync(50);  
#if GTP_ESD_PROTECT
		gtp_init_ext_watchdog(client);
#endif
#endif


}

#if GTP_SLIDE_WAKEUP
/*******************************************************
Function:
Enter doze mode for sliding wakeup.
Input:
ts: goodix tp private data
Output:
1: succeed, otherwise failed
 *******************************************************/
static s8 gtp_enter_doze(struct goodix_ts_data *ts)
{
	s8 ret = -1;
	s8 retry = 0;
	u8 i2c_control_buf[3] = {(u8)(GTP_REG_SLEEP >> 8), (u8)GTP_REG_SLEEP, 8};

	GTP_DEBUG_FUNC();

#if GTP_DBL_CLK_WAKEUP
	i2c_control_buf[2] = 0x09;
#endif

	gtp_irq_disable(ts);

	GTP_DEBUG("Entering doze mode.");
	while(retry++ < 5)
	{
		i2c_control_buf[0] = 0x80;
		i2c_control_buf[1] = 0x46;
		ret = gtp_i2c_write(ts->client, i2c_control_buf, 3);
		if (ret < 0)
		{
			GTP_DEBUG("failed to set doze flag into 0x8046, %d", retry);
			continue;
		}
		i2c_control_buf[0] = 0x80;
		i2c_control_buf[1] = 0x40;
		ret = gtp_i2c_write(ts->client, i2c_control_buf, 3);
		if (ret > 0)
		{
			doze_status = DOZE_ENABLED;
			GTP_INFO("GTP has been working in doze mode!");
			gtp_irq_enable(ts);
			return ret;
		}
		msleep(10);
	}
	GTP_ERROR("GTP send doze cmd failed.");
	gtp_irq_enable(ts);
	return ret;
}
#else 
/*******************************************************
Function:
Enter sleep mode.
Input:
ts: private data.
Output:
Executive outcomes.
1: succeed, otherwise failed.
 *******************************************************/
static s8 gtp_enter_sleep(struct goodix_ts_data * ts)
{
	s8 ret = -1;
	s8 retry = 0;
	u8 i2c_control_buf[3] = {(u8)(GTP_REG_SLEEP >> 8), (u8)GTP_REG_SLEEP, 5};

#if GTP_COMPATIBLE_MODE
	u8 status_buf[3] = {0x80, 0x44};
#endif

	GTP_DEBUG_FUNC();

#if GTP_COMPATIBLE_MODE
	if (CHIP_TYPE_GT9F == ts->chip_type)
	{
		// GT9XXF: host interact with ic
		ret = gtp_i2c_read(ts->client, status_buf, 3);
		if (ret < 0)
		{
			GTP_ERROR("failed to get backup-reference status");
		}

		if (status_buf[2] & 0x80)
		{
			ret = gtp_bak_ref_proc(ts, GTP_BAK_REF_STORE);
			if (FAIL == ret)
			{
				GTP_ERROR("failed to store bak_ref");
			}
		}
	}
#endif

	GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
	msleep(5);

	while(retry++ < 5)
	{
		ret = gtp_i2c_write(ts->client, i2c_control_buf, 3);
		if (ret > 0)
		{
			GTP_INFO("GTP enter sleep!");

			return ret;
		}
		msleep(10);
	}
	GTP_ERROR("GTP send sleep cmd failed.");
	return ret;
}
#endif 
/*******************************************************
Function:
Wakeup from sleep.
Input:
ts: private data.
Output:
Executive outcomes.
>0: succeed, otherwise: failed.
 *******************************************************/
static s8 gtp_wakeup_sleep(struct goodix_ts_data * ts)
{
	u8 retry = 0;
	s8 ret = -1;

	GTP_DEBUG_FUNC();
	
 gtp_io_init(20);
 gtp_set_io_int();	

#if GTP_COMPATIBLE_MODE
	if (CHIP_TYPE_GT9F == ts->chip_type)
	{
		u8 opr_buf[3] = {0x41, 0x80};

		GTP_GPIO_OUTPUT(GTP_INT_PORT, 1);
		msleep(5);

		for (retry = 0; retry < 20; ++retry)
		{
			// hold ss51 & dsp
			opr_buf[2] = 0x0C;
			ret = gtp_i2c_write(ts->client, opr_buf, 3);
			if (FAIL == ret)
			{
				GTP_ERROR("failed to hold ss51 & dsp!");
				continue;
			}
			opr_buf[2] = 0x00;
			ret = gtp_i2c_read(ts->client, opr_buf, 3);
			if (FAIL == ret)
			{
				GTP_ERROR("failed to get ss51 & dsp status!");
				continue;
			}
			if (0x0C != opr_buf[2])
			{
				GTP_DEBUG("ss51 & dsp not been hold, %d", retry+1);
				continue;
			}
			GTP_DEBUG("ss51 & dsp confirmed hold");

			ret = gtp_fw_startup(ts->client);
			if (FAIL == ret)
			{
				GTP_ERROR("failed to startup GT9XXF, process recovery");
				gtp_esd_recovery(ts->client);
			}
			break;
		}
		if (retry >= 10)
		{
			GTP_ERROR("failed to wakeup, processing esd recovery");
			gtp_esd_recovery(ts->client);
		}
		else
		{
			GTP_INFO("GT9XXF gtp wakeup success");
		}
		return ret;
	}
#endif

#if GTP_POWER_CTRL_SLEEP
	while(retry++ < 5)
	{
		gtp_reset_guitar(ts->client, 20);

		GTP_INFO("GTP wakeup sleep.");
		return 1;
	}
#else
	while(retry++ < 10)
	{
#if GTP_SLIDE_WAKEUP
		if (DOZE_WAKEUP != doze_status)       // wakeup not by slide 
		{
			GTP_DEBUG("wakeup by power, reset guitar");
			doze_status = DOZE_DISABLED;   
			gtp_irq_disable(ts);
			gtp_reset_guitar(ts->client, 10);
			gtp_irq_enable(ts);
		}
		else              // wakeup by slide 
		{
			GTP_DEBUG("wakeup by slide/double-click, no reset guitar");
			doze_status = DOZE_DISABLED;
#if GTP_ESD_PROTECT
			gtp_init_ext_watchdog(ts->client);
#endif
		}

#else
		if (chip_gt9xxs == 1)
		{
			gtp_reset_guitar(ts->client, 10);
		}
		else
		{
			GTP_GPIO_OUTPUT(GTP_INT_PORT, 1);
			msleep(5);
		}
#endif

		ret = gtp_i2c_test(ts->client);
		if (ret > 0)
		{
			GTP_INFO("GTP wakeup sleep.");

#if (!GTP_SLIDE_WAKEUP)
			if (chip_gt9xxs == 0)
			{
				gtp_int_sync(25);
#if GTP_ESD_PROTECT
				gtp_init_ext_watchdog(ts->client);
#endif
			}
#endif

			return ret;
		}
		gtp_reset_guitar(ts->client, 20);
	}
#endif

	GTP_ERROR("GTP wakeup sleep failed.");
	return ret;
}
#if GTP_DRIVER_SEND_CFG
static s32 gtp_get_info(struct goodix_ts_data *ts)
{
	u8 opr_buf[6] = {0};
	s32 ret = 0;

	opr_buf[0] = (u8)((GTP_REG_CONFIG_DATA+1) >> 8);
	opr_buf[1] = (u8)((GTP_REG_CONFIG_DATA+1) & 0xFF);

	ret = gtp_i2c_read(ts->client, opr_buf, 6);
	if (ret < 0)
	{
		return FAIL;
	}

	ts->abs_x_max = (opr_buf[3] << 8) + opr_buf[2];
	ts->abs_y_max = (opr_buf[5] << 8) + opr_buf[4];

	opr_buf[0] = (u8)((GTP_REG_CONFIG_DATA+6) >> 8);
	opr_buf[1] = (u8)((GTP_REG_CONFIG_DATA+6) & 0xFF);

	ret = gtp_i2c_read(ts->client, opr_buf, 3);
	if (ret < 0)
	{
		return FAIL;
	}
	ts->int_trigger_type = opr_buf[2] & 0x03;

	GTP_INFO("X_MAX = %d, Y_MAX = %d, TRIGGER = 0x%02x",
			ts->abs_x_max,ts->abs_y_max,ts->int_trigger_type);

	return SUCCESS;    
}
#endif 

/*******************************************************
Function:
Initialize gtp.
Input:
ts: goodix private data
Output:
Executive outcomes.
0: succeed, otherwise: failed
 *******************************************************/
static s32 gtp_init_panel(struct goodix_ts_data *ts)
{
	s32 ret = -1;

#if GTP_DRIVER_SEND_CFG
	s32 i = 0;
	u8 check_sum = 0;
	u8 opr_buf[16] = {0};
	u8 sensor_id = 0; 
	u8 index = 0;

	static u8 cfg_info_group1[] = CTP_CFG_GROUP1;
	static u8 cfg_info_group2[] = CTP_CFG_GROUP2;
	static u8 cfg_info_group3[] = CTP_CFG_GROUP3;
	static u8 cfg_info_group4[] = CTP_CFG_GROUP4;
	static u8 cfg_info_group5[] = CTP_CFG_GROUP5;
	static u8 cfg_info_group6[] = CTP_CFG_GROUP6;
	u8 *send_cfg_buf[] = {cfg_info_group1, cfg_info_group2, cfg_info_group3,
		cfg_info_group4, cfg_info_group5, cfg_info_group6};
	u8 cfg_info_len[] = { CFG_GROUP_LEN(cfg_info_group1),
		CFG_GROUP_LEN(cfg_info_group2),
		CFG_GROUP_LEN(cfg_info_group3),
		CFG_GROUP_LEN(cfg_info_group4),
		CFG_GROUP_LEN(cfg_info_group5),
		CFG_GROUP_LEN(cfg_info_group6)};

	GTP_DEBUG_FUNC();
	GTP_DEBUG("Config Groups\' Lengths: %d, %d, %d, %d, %d, %d", 
			cfg_info_len[0], cfg_info_len[1], cfg_info_len[2], cfg_info_len[3],
			cfg_info_len[4], cfg_info_len[5]);


#if GTP_COMPATIBLE_MODE
	if (CHIP_TYPE_GT9F == ts->chip_type)
	{
		ts->fw_error = 0;
	}
	else
#endif
	{
		ret = gtp_i2c_read_dbl_check(ts->client, 0x41E4, opr_buf, 1);
		if (SUCCESS == ret) 
		{
			if (opr_buf[0] != 0xBE)
			{
				ts->fw_error = 1;
				GTP_ERROR("Firmware error, no config sent!");
				return -1;
			}
		}
	}
#if 1
	if ((!cfg_info_len[0]))
	{
		sensor_id = 0; 
	}
	else
	{
#if GTP_COMPATIBLE_MODE
		msleep(50);
#endif
//		ret = gtp_i2c_read_dbl_check(ts->client, GTP_REG_SENSOR_ID, &sensor_id, 1);
//		if (SUCCESS == ret)
//		{
//			if (sensor_id >= 0x06)
//			{
//				GTP_ERROR("Invalid sensor_id(0x%02X), No Config Sent!", sensor_id);
//				ts->pnl_init_error = 1;
//#if GTP_COMPATIBLE_MODE
//				if (CHIP_TYPE_GT9F == ts->chip_type)
//				{
//					return -1;
//				}
//				else
//#endif
//				{
//					gtp_get_info(ts);
//				}
//				return 0;
//			}
//		}
//		else
//		{
//			GTP_ERROR("Failed to get sensor_id, No config sent!");
//			ts->pnl_init_error = 1;
//			return -1;
//		}
       GTP_DEBUG("CTP name : %s\n",config_info.name);
        if (!strcmp(config_info.name,"gt9271_1024_600")){
            index = 0;
            GTP_DEBUG("gt9xx:index = %d\n",index);
			
			}
        if (!strcmp(config_info.name,"gt9271_D116")){
            index = 1;
            GTP_DEBUG("gt9xx:index = %d\n",index);
			
			}
        if (!strcmp(config_info.name,"gt911_DB")){
            index = 3;
            GTP_INFO("gt9xx:index = %d\n",index);
			
			}
			if (!strcmp(config_info.name,"gt911_DB2")){
            index = 4;
            GTP_INFO("gt9xx:index = %d\n",index);
			
			}
		}
#endif

	sensor_id = index;

	
	ts->gtp_cfg_len = cfg_info_len[sensor_id];
	GTP_INFO("CTP_CONFIG_GROUP%d used, config length: %d", sensor_id + 1, ts->gtp_cfg_len);

	if (ts->gtp_cfg_len < GTP_CONFIG_MIN_LENGTH)
	{
		GTP_ERROR("Config Group%d is INVALID CONFIG GROUP(Len: %d)! NO Config Sent! You need to check you header file CFG_GROUP section!", sensor_id+1, ts->gtp_cfg_len);
		ts->pnl_init_error = 1;
		return -1;
	}

#if GTP_COMPATIBLE_MODE
	if (CHIP_TYPE_GT9F == ts->chip_type)
	{
		ts->fixed_cfg = 0;
	}
	else
#endif
	{
		ret = gtp_i2c_read_dbl_check(ts->client, GTP_REG_CONFIG_DATA, &opr_buf[0], 1);

		if (ret == SUCCESS)
		{
			GTP_DEBUG("CFG_GROUP%d Config Version: %d, 0x%02X; IC Config Version: %d, 0x%02X", index+1, 
					send_cfg_buf[index][0], send_cfg_buf[index][0], opr_buf[0], opr_buf[0]);

			if (opr_buf[0] < 90)    
			{
				grp_cfg_version = send_cfg_buf[index][0];       // backup group config version
				send_cfg_buf[index][0] = 0x00;
				ts->fixed_cfg = 0;
			}
			else        // treated as fixed config, not send config
			{
				GTP_INFO("Ic fixed config with config version(%d, 0x%02X)", opr_buf[0], opr_buf[0]);
				ts->fixed_cfg = 1;
				gtp_get_info(ts);
				return 0;
			}
		}
		else
		{
			GTP_ERROR("Failed to get ic config version!No config sent!");
			return -1;
		}
	}

	memset(&config[GTP_ADDR_LENGTH], 0, GTP_CONFIG_MAX_LENGTH);
	memcpy(&config[GTP_ADDR_LENGTH], send_cfg_buf[sensor_id], ts->gtp_cfg_len);

#if GTP_CUSTOM_CFG
	config[RESOLUTION_LOC]     = (u8)GTP_MAX_WIDTH;
	config[RESOLUTION_LOC + 1] = (u8)(GTP_MAX_WIDTH>>8);
	config[RESOLUTION_LOC + 2] = (u8)GTP_MAX_HEIGHT;
	config[RESOLUTION_LOC + 3] = (u8)(GTP_MAX_HEIGHT>>8);

	if (GTP_INT_TRIGGER == 0)  //RISING
	{
		config[TRIGGER_LOC] &= 0xfe; 
	}
	else if (GTP_INT_TRIGGER == 1)  //FALLING
	{
		config[TRIGGER_LOC] |= 0x01;
	}
#endif  // GTP_CUSTOM_CFG

	check_sum = 0;
	for (i = GTP_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
	{
		check_sum += config[i];
	}
	config[ts->gtp_cfg_len] = (~check_sum) + 1;

#else // driver not send config

	ts->gtp_cfg_len = GTP_CONFIG_MAX_LENGTH;
	ret = gtp_i2c_read(ts->client, config, ts->gtp_cfg_len + GTP_ADDR_LENGTH);
	if (ret < 0)
	{
		GTP_ERROR("Read Config Failed, Using Default Resolution & INT Trigger!");
		ts->abs_x_max = GTP_MAX_WIDTH;
		ts->abs_y_max = GTP_MAX_HEIGHT;
		ts->int_trigger_type = GTP_INT_TRIGGER;
	}

#endif // GTP_DRIVER_SEND_CFG

	if ((ts->abs_x_max == 0) && (ts->abs_y_max == 0))
	{
		ts->abs_x_max = (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC];
		ts->abs_y_max = (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2];
		ts->int_trigger_type = (config[TRIGGER_LOC]) & 0x03; 
	}

#if GTP_COMPATIBLE_MODE
	if (CHIP_TYPE_GT9F == ts->chip_type)
	{
		u8 sensor_num = 0;
		u8 driver_num = 0;
		u8 have_key = 0;

		have_key = (config[GTP_REG_HAVE_KEY - GTP_REG_CONFIG_DATA + 2] & 0x01);

		if (1 == ts->is_950)
		{
			driver_num = config[GTP_REG_MATRIX_DRVNUM - GTP_REG_CONFIG_DATA + 2];
			sensor_num = config[GTP_REG_MATRIX_SENNUM - GTP_REG_CONFIG_DATA + 2];
			if (have_key)
			{
				driver_num--;
			}
			ts->bak_ref_len = (driver_num * (sensor_num - 1) + 2) * 2 * 6;
		}
		else
		{
			driver_num = (config[CFG_LOC_DRVA_NUM] & 0x1F) + (config[CFG_LOC_DRVB_NUM]&0x1F);
			if (have_key)
			{
				driver_num--;
			}
			sensor_num = (config[CFG_LOC_SENS_NUM] & 0x0F) + ((config[CFG_LOC_SENS_NUM] >> 4) & 0x0F);
			ts->bak_ref_len = (driver_num * (sensor_num - 2) + 2) * 2;
		}

		GTP_INFO("Drv * Sen: %d * %d(key: %d), X_MAX: %d, Y_MAX: %d, TRIGGER: 0x%02x",
				driver_num, sensor_num, have_key, ts->abs_x_max,ts->abs_y_max,ts->int_trigger_type);
		return 0;
	}
	else
#endif
	{
#if GTP_DRIVER_SEND_CFG
		ret = gtp_send_cfg(ts->client);
		if (ret < 0)
		{
			GTP_ERROR("Send config error.");
		}
		// set config version to CTP_CFG_GROUP, for resume to send config
		config[GTP_ADDR_LENGTH] = grp_cfg_version;
		check_sum = 0;
		for (i = GTP_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
		{
			check_sum += config[i];
		}
		config[ts->gtp_cfg_len] = (~check_sum) + 1;
#endif
		GTP_INFO("X_MAX: %d, Y_MAX: %d, TRIGGER: 0x%02x", ts->abs_x_max,ts->abs_y_max,ts->int_trigger_type);
	}

	msleep(10);
	return 0;
}

/*******************************************************
Function:
Read chip version.
Input:
client:  i2c device
version: buffer to keep ic firmware version
Output:
read operation return.
2: succeed, otherwise: failed
 *******************************************************/
s32 gtp_read_version(struct i2c_client *client, u16* version)
{
	s32 ret = -1;
	u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};

	GTP_DEBUG_FUNC();

	ret = gtp_i2c_read(client, buf, sizeof(buf));
	if (ret < 0)
	{
		GTP_ERROR("GTP read version failed");
		return ret;
	}

	if (version)
	{
		*version = (buf[7] << 8) | buf[6];
	}

	if (buf[5] == 0x00)
	{
		GTP_INFO("IC Version: %c%c%c_%02x%02x", buf[2], buf[3], buf[4], buf[7], buf[6]);
	}
	else
	{
		if (buf[5] == 'S' || buf[5] == 's')
		{
			chip_gt9xxs = 1;
		}
		GTP_INFO("IC ), X_MAX: 480, Y_MAX: 800, TRIGGER: 0x01: %c%c%c%c_%02x%02x", buf[2], buf[3], buf[4], buf[5], buf[7], buf[6]);
	}
	return ret;
}

/*******************************************************
Function:
I2c test Function.
Input:
client:i2c client.
Output:
Executive outcomes.
2: succeed, otherwise failed.
 *******************************************************/
static s8 gtp_i2c_test(struct i2c_client *client)
{
	u8 test[3] = {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};
	u8 retry = 0;
	s8 ret = -1;

	GTP_DEBUG_FUNC();

	while(retry++ < 5)
	{
		ret = gtp_i2c_read(client, test, 3);
		if (ret > 0)
		{
			GTP_INFO("GTP i2c test OK.");
			return ret;
		}
		GTP_ERROR("GTP i2c test failed time %d.",retry);
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
Executive outcomes.
0: succeed, otherwise: failed.
 *******************************************************/
static s8 gtp_request_input_dev(struct goodix_ts_data *ts)
{
	s8 ret = -1;
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
#if GTP_ICS_SLOT_REPORT
	__set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
	input_mt_init_slots(ts->input_dev, 16);     // in case of "out of memory"
#else
	ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
#endif

#if GTP_HAVE_TOUCH_KEY
	for (index = 0; index < GTP_MAX_KEY_NUM; index++)
	{
		input_set_capability(ts->input_dev, EV_KEY, touch_key_array[index]);  
	}
#endif

#if GTP_SLIDE_WAKEUP
	input_set_capability(ts->input_dev, EV_KEY, KEY_POWER);
#endif 

#if GTP_WITH_PEN
	// pen support
	__set_bit(BTN_TOOL_PEN, ts->input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
	//__set_bit(INPUT_PROP_POINTER, ts->input_dev->propbit);
#endif

#if GTP_CHANGE_X2Y
	GTP_SWAP(ts->abs_x_max, ts->abs_y_max);
#endif
	set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0,screen_max_x , 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, screen_max_y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);

	ts->input_dev->name = CTP_NAME;
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
	input_set_drvdata(ts->input_dev, i2c_connect_client);
	return 0;
}

//************** For GT9XXF Start *************//
#if GTP_COMPATIBLE_MODE

s32 gtp_fw_startup(struct i2c_client *client)
{
	u8 opr_buf[4];
	s32 ret = 0;

	//init sw WDT
	opr_buf[0] = 0xAA;
	ret = i2c_write_bytes(client, 0x8041, opr_buf, 1);
	if (ret < 0)
	{
		return FAIL;
	}

	//release SS51 & DSP
	opr_buf[0] = 0x00;
	ret = i2c_write_bytes(client, 0x4180, opr_buf, 1);
	if (ret < 0)
	{
		return FAIL;
	}
	//int sync
	gtp_int_sync(25);  

	//check fw run status
	ret = i2c_read_bytes(client, 0x8041, opr_buf, 1);
	if (ret < 0)
	{
		return FAIL;
	}
	if(0xAA == opr_buf[0])
	{
		GTP_ERROR("IC works abnormally,startup failed.");
		return FAIL;
	}
	else
	{
		GTP_INFO("IC works normally, Startup success.");
		opr_buf[0] = 0xAA;
		i2c_write_bytes(client, 0x8041, opr_buf, 1);
		return SUCCESS;
	}
}

static s32 gtp_esd_recovery(struct i2c_client *client)
{
	s32 retry = 0;
	s32 ret = 0;
	struct goodix_ts_data *ts;

	ts = i2c_get_clientdata(client);

	gtp_irq_disable(ts);

	GTP_INFO("GT9XXF esd recovery mode");
	gtp_reset_guitar(client, 20);       // reset & select I2C addr
	for (retry = 0; retry < 5; ++retry)
	{
		ret = gup_fw_download_proc(NULL, GTP_FL_ESD_RECOVERY); 
		if (FAIL == ret)
		{
			GTP_ERROR("esd recovery failed %d", retry+1);
			continue;
		}
		ret = gtp_fw_startup(ts->client);
		if (FAIL == ret)
		{
			GTP_ERROR("GT9XXF start up failed %d", retry+1);
			continue;
		}
		break;
	}
	gtp_irq_enable(ts);

	if (retry >= 5)
	{
		GTP_ERROR("failed to esd recovery");
		return FAIL;
	}

	GTP_INFO("Esd recovery successful");
	return SUCCESS;
}

void gtp_recovery_reset(struct i2c_client *client)
{
#if GTP_ESD_PROTECT
	gtp_esd_switch(client, SWITCH_OFF);
#endif
	GTP_DEBUG_FUNC();

	gtp_esd_recovery(client); 

#if GTP_ESD_PROTECT
	gtp_esd_switch(client, SWITCH_ON);
#endif
}

static s32 gtp_bak_ref_proc(struct goodix_ts_data *ts, u8 mode)
{
	s32 ret = 0;
	s32 i = 0;
	s32 j = 0;
	u16 ref_sum = 0;
	u16 learn_cnt = 0;
	u16 chksum = 0;
	s32 ref_seg_len = 0;
	s32 ref_grps = 0;
	struct file *ref_filp = NULL;
	u8 *p_bak_ref;

	ret = gup_check_fs_mounted("/sensors_cache");
	if (FAIL == ret)
	{
		ts->ref_chk_fs_times++;
		GTP_DEBUG("Ref check /data times/MAX_TIMES: %d / %d", ts->ref_chk_fs_times, GTP_CHK_FS_MNT_MAX);
		if (ts->ref_chk_fs_times < GTP_CHK_FS_MNT_MAX)
		{
			msleep(50);
			GTP_INFO("/data not mounted.");
			return FAIL;
		}
		GTP_INFO("check /data mount timeout...");
	}
	else
	{
		GTP_INFO("/data mounted!!!(%d/%d)", ts->ref_chk_fs_times, GTP_CHK_FS_MNT_MAX);
	}

	p_bak_ref = (u8 *)kzalloc(ts->bak_ref_len, GFP_KERNEL);

	if (NULL == p_bak_ref)
	{
		GTP_ERROR("Allocate memory for p_bak_ref failed!");
		return FAIL;   
	}

	if (ts->is_950)
	{
		ref_seg_len = ts->bak_ref_len / 6;
		ref_grps = 6;
	}
	else
	{
		ref_seg_len = ts->bak_ref_len;
		ref_grps = 1;
	}
	ref_filp = filp_open(GTP_BAK_REF_PATH, O_RDWR | O_CREAT, 0666);
	if (IS_ERR(ref_filp))
	{
		GTP_INFO("%s is unavailable, default backup-reference used", GTP_BAK_REF_PATH);
		goto bak_ref_default;
	}

	switch (mode)
	{
		case GTP_BAK_REF_SEND:
			GTP_INFO("Send backup-reference");
			ref_filp->f_op->llseek(ref_filp, 0, SEEK_SET);
			ret = ref_filp->f_op->read(ref_filp, (char*)p_bak_ref, ts->bak_ref_len, &ref_filp->f_pos);
			if (ret < 0)
			{
				GTP_ERROR("failed to read bak_ref info from file, sending defualt bak_ref");
				goto bak_ref_default;
			}
			for (j = 0; j < ref_grps; ++j)
			{
				ref_sum = 0;
				for (i = 0; i < (ref_seg_len); i += 2)
				{
					ref_sum += (p_bak_ref[i + j * ref_seg_len] << 8) + p_bak_ref[i+1 + j * ref_seg_len];
				}
				learn_cnt = (p_bak_ref[j * ref_seg_len + ref_seg_len -4] << 8) + (p_bak_ref[j * ref_seg_len + ref_seg_len -3]);
				chksum = (p_bak_ref[j * ref_seg_len + ref_seg_len -2] << 8) + (p_bak_ref[j * ref_seg_len + ref_seg_len -1]);
				GTP_DEBUG("learn count = %d", learn_cnt);
				GTP_DEBUG("chksum = %d", chksum);
				GTP_DEBUG("ref_sum = 0x%04X", ref_sum & 0xFFFF);
				// Sum(1~ref_seg_len) == 1
				if (1 != ref_sum)
				{
					GTP_INFO("wrong chksum for bak_ref, reset to 0x00 bak_ref");
					memset(&p_bak_ref[j * ref_seg_len], 0, ref_seg_len);
					p_bak_ref[ref_seg_len + j * ref_seg_len - 1] = 0x01;
				}
				else
				{
					if (j == (ref_grps - 1))
					{
						GTP_INFO("backup-reference data in %s used", GTP_BAK_REF_PATH);
					}
				}
			}
			ret = i2c_write_bytes(ts->client, GTP_REG_BAK_REF, p_bak_ref, ts->bak_ref_len);
			if (FAIL == ret)
			{
				GTP_ERROR("failed to send bak_ref because of iic comm error");
				filp_close(ref_filp, NULL);
				return FAIL;
			}
			break;

		case GTP_BAK_REF_STORE:
			GTP_INFO("Store backup-reference");
			ret = i2c_read_bytes(ts->client, GTP_REG_BAK_REF, p_bak_ref, ts->bak_ref_len);
			if (ret < 0)
			{
				GTP_ERROR("failed to read bak_ref info, sending default back-reference");
				goto bak_ref_default;
			}
			ref_filp->f_op->llseek(ref_filp, 0, SEEK_SET);
			ref_filp->f_op->write(ref_filp, (char*)p_bak_ref, ts->bak_ref_len, &ref_filp->f_pos);
			break;

		default:
			GTP_ERROR("invalid backup-reference request");
			break;
	}
	filp_close(ref_filp, NULL);
	return SUCCESS;

bak_ref_default:

	for (j = 0; j < ref_grps; ++j)
	{
		memset(&p_bak_ref[j * ref_seg_len], 0, ref_seg_len);
		p_bak_ref[j * ref_seg_len + ref_seg_len - 1] = 0x01;  // checksum = 1     
	}
	ret = i2c_write_bytes(ts->client, GTP_REG_BAK_REF, p_bak_ref, ts->bak_ref_len);
	if (!IS_ERR(ref_filp))
	{
		GTP_INFO("write backup-reference data into %s", GTP_BAK_REF_PATH);
		ref_filp->f_op->llseek(ref_filp, 0, SEEK_SET);
		ref_filp->f_op->write(ref_filp, (char*)p_bak_ref, ts->bak_ref_len, &ref_filp->f_pos);
		filp_close(ref_filp, NULL);
	}
	if (ret == FAIL)
	{
		GTP_ERROR("failed to load the default backup reference");
		return FAIL;
	}
	return SUCCESS;
}


static s32 gtp_verify_main_clk(u8 *p_main_clk)
{
	u8 chksum = 0;
	u8 main_clock = p_main_clk[0];
	s32 i = 0;

	if (main_clock < 50 || main_clock > 120)    
	{
		return FAIL;
	}

	for (i = 0; i < 5; ++i)
	{
		if (main_clock != p_main_clk[i])
		{
			return FAIL;
		}
		chksum += p_main_clk[i];
	}
	chksum += p_main_clk[5];
	if ( (chksum) == 0)
	{
		return SUCCESS;
	}
	else
	{
		return FAIL;
	}
}

static s32 gtp_main_clk_proc(struct goodix_ts_data *ts)
{
	s32 ret = 0;
	s32 i = 0;
	s32 clk_chksum = 0;
	struct file *clk_filp = NULL;
	u8 p_main_clk[6] = {0};

	ret = gup_check_fs_mounted("/sensors_cache");
	if (FAIL == ret)
	{
		ts->clk_chk_fs_times++;
		GTP_DEBUG("Clock check /data times/MAX_TIMES: %d / %d", ts->clk_chk_fs_times, GTP_CHK_FS_MNT_MAX);
		if (ts->clk_chk_fs_times < GTP_CHK_FS_MNT_MAX)
		{
			msleep(50);
			GTP_INFO("/data not mounted.");
			return FAIL;
		}
		GTP_INFO("Check /data mount timeout!");
	}
	else
	{
		GTP_INFO("/data mounted!!!(%d/%d)", ts->clk_chk_fs_times, GTP_CHK_FS_MNT_MAX);
	}

	clk_filp = filp_open(GTP_MAIN_CLK_PATH, O_RDWR | O_CREAT, 0666);
	if (IS_ERR(clk_filp))
	{
		GTP_ERROR("%s is unavailable, calculate main clock", GTP_MAIN_CLK_PATH);
	}
	else
	{
		clk_filp->f_op->llseek(clk_filp, 0, SEEK_SET);
		clk_filp->f_op->read(clk_filp, (char *)p_main_clk, 6, &clk_filp->f_pos);

		ret = gtp_verify_main_clk(p_main_clk);
		if (FAIL == ret)
		{
			// recalculate main clock & rewrite main clock data to file
			GTP_ERROR("main clock data in %s is wrong, recalculate main clock", GTP_MAIN_CLK_PATH);
		}
		else
		{ 
			GTP_INFO("main clock data in %s used, main clock freq: %d", GTP_MAIN_CLK_PATH, p_main_clk[0]);
			filp_close(clk_filp, NULL);
			goto update_main_clk;
		}
	}

#if GTP_ESD_PROTECT
	gtp_esd_switch(ts->client, SWITCH_OFF);
#endif
	ret = gup_clk_calibration();
	gtp_esd_recovery(ts->client);

#if GTP_ESD_PROTECT
	gtp_esd_switch(ts->client, SWITCH_ON);
#endif

	GTP_INFO("calibrate main clock: %d", ret);
	if (ret < 50 || ret > 120)
	{
		GTP_ERROR("wrong main clock: %d", ret);
		goto exit_main_clk;
	}

	// Sum{0x8020~0x8025} = 0
	for (i = 0; i < 5; ++i)
	{
		p_main_clk[i] = ret;
		clk_chksum += p_main_clk[i];
	}
	p_main_clk[5] = 0 - clk_chksum;

	if (!IS_ERR(clk_filp))
	{
		GTP_DEBUG("write main clock data into %s", GTP_MAIN_CLK_PATH);
		clk_filp->f_op->llseek(clk_filp, 0, SEEK_SET);
		clk_filp->f_op->write(clk_filp, (char *)p_main_clk, 6, &clk_filp->f_pos);
		filp_close(clk_filp, NULL);
	}

update_main_clk:
	ret = i2c_write_bytes(ts->client, GTP_REG_MAIN_CLK, p_main_clk, 6);
	if (FAIL == ret)
	{
		GTP_ERROR("update main clock failed!");
		return FAIL;
	}
	return SUCCESS;

exit_main_clk:
	if (!IS_ERR(clk_filp))
	{
		filp_close(clk_filp, NULL);
	}
	return FAIL;
}

s32 gtp_gt9xxf_init(struct i2c_client *client)
{
	s32 ret = 0;

	ret = gup_fw_download_proc(NULL, GTP_FL_FW_BURN); 
	if (FAIL == ret)
	{
		return FAIL;
	}

	ret = gtp_fw_startup(client);
	if (FAIL == ret)
	{
		return FAIL;
	}
	return SUCCESS;
}

void gtp_get_chip_type(struct goodix_ts_data *ts)
{
	u8 opr_buf[10] = {0x00};
	s32 ret = 0;

	msleep(10);

	ret = gtp_i2c_read_dbl_check(ts->client, GTP_REG_CHIP_TYPE, opr_buf, 10);

	if (FAIL == ret)
	{
		GTP_ERROR("Failed to get chip-type, set chip type default: GOODIX_GT9");
		ts->chip_type = CHIP_TYPE_GT9;
		return;
	}

	if (!memcmp(opr_buf, "GOODIX_GT9", 10))
	{
		ts->chip_type = CHIP_TYPE_GT9;
	}
	else // GT9XXF
	{
		ts->chip_type = CHIP_TYPE_GT9F;
	}
	GTP_INFO("Chip Type: %s", (ts->chip_type == CHIP_TYPE_GT9) ? "GOODIX_GT9" : "GOODIX_GT9F");
}

#endif
//************* For GT9XXF End ************//

static void goodix_init_events (struct work_struct *work)
{
	int ret;
	u16 version_info;

#if GTP_COMPATIBLE_MODE
	gtp_get_chip_type(ts_init);

	if (CHIP_TYPE_GT9F == ts_init->chip_type)
	{
		ret = gtp_gt9xxf_init(ts_init->client);
		if (FAIL == ret)
		{
			GTP_INFO("Failed to init GT9XXF.");
		}
	}
#endif

#if GTP_ESD_PROTECT
	INIT_DELAYED_WORK(&gtp_esd_check_work, gtp_esd_check_func);
	gtp_esd_check_workqueue = create_workqueue("gtp_esd_check");
#endif

	ret = gtp_init_panel(ts_init);
	if (ret < 0)
	{
		GTP_ERROR("GTP init chip failed.");
	}

#if GTP_AUTO_UPDATE
	ret = gup_init_update_proc(ts_init);
	if (ret < 0)
	{
		GTP_ERROR("Create update thread error.");
	}
#endif

	ret = gtp_request_input_dev(ts_init);
	if (ret < 0)
	{
		GTP_ERROR("GTP request input dev failed");
	}

	ret = gtp_read_version(i2c_connect_client, &version_info);
	if (ret < 0)
	{
		GTP_ERROR("Read version failed.");
	}

    config_info.dev = &(ts_init->input_dev->dev);
    ret = input_request_int(&(config_info.input_type), goodix_ts_irq_handler,CTP_IRQ_MODE, ts_init);	   
    if (ret) {
          printk("Request irq fail!.\n");
    }

	ts_init->use_irq = 1;
	if (ts_init->use_irq)
	{
		gtp_irq_enable(ts_init);
	}

#if GTP_CREATE_WR_NODE
	init_wr_node(i2c_connect_client);
#endif

#if GTP_ESD_PROTECT
	gtp_esd_switch(i2c_connect_client, SWITCH_ON);
#endif
	return;
}

/*******************************************************
Function:
I2c probe.
Input:
client: i2c device struct.
id: device id.
Output:
Executive outcomes. 
0: succeed.
 *******************************************************/
static int goodix_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	s32 ret = -1;
	struct goodix_ts_data *ts;
	GTP_DEBUG_FUNC();
	//do NOT remove these logs
	GTP_INFO("GTP Driver Version: %s", GTP_DRIVER_VERSION);
	GTP_INFO("GTP Driver Built@%s, %s", __TIME__, __DATE__);
	GTP_INFO("GTP I2C Address: 0x%02x", client->addr);

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
	spin_lock_init(&ts->irq_lock);          // 2.6.39 later
	// ts->irq_lock = SPIN_LOCK_UNLOCKED;   // 2.6.39 & before
#if GTP_ESD_PROTECT
	ts->clk_tick_cnt = 2 * HZ;      // HZ: clock ticks in 1 second generated by system
	GTP_DEBUG("Clock ticks for an esd cycle: %d", ts->clk_tick_cnt);  
	spin_lock_init(&ts->esd_lock);
	// ts->esd_lock = SPIN_LOCK_UNLOCKED;
#endif
	i2c_set_clientdata(client, ts);

	ts->gtp_rawdiff_mode = 0;

	ret = gtp_i2c_test(client);
	if (ret < 0)
	{
		GTP_ERROR("I2C communication ERROR!");
		goto I2C_TEST_FAIL;
	}
	printk("======detect ok !=====");
	ts_init = ts;
	ts->is_suspended = false;

	goodix_wq = create_singlethread_workqueue("goodix_wq");
	if (!goodix_wq) {
		printk(KERN_ALERT "Creat %s workqueue failed.\n", goodix_ts_name);
		return -ENOMEM;

	}

	goodix_init_wq = create_singlethread_workqueue("goodix_init");
	if (goodix_init_wq == NULL) {
		printk("create goodix_wq fail!\n");
		return -ENOMEM;
	}

	goodix_resume_wq = create_singlethread_workqueue("goodix_resume");
	if (goodix_resume_wq == NULL) {
		printk("create goodix_resume_wq fail!\n");
		return -ENOMEM;
	}

	queue_work(goodix_init_wq, &goodix_init_work);
//	input_set_drvdata(ts->input_dev, client); 
	pm_runtime_set_active(&client->dev);
	pm_runtime_get(&client->dev);
	pm_runtime_enable(&client->dev);
	return 0;

I2C_TEST_FAIL:
	kfree(ts);
	return -1;
}


/*******************************************************
Function:
Goodix touchscreen driver release function.
Input:
client: i2c device struct.
Output:
Executive outcomes. 0---succeed.
 *******************************************************/
static int goodix_ts_remove(struct i2c_client *client)
{
	struct goodix_ts_data *ts = i2c_get_clientdata(client);

	GTP_DEBUG_FUNC();

	cancel_work_sync(&goodix_init_work);
	cancel_work_sync(&goodix_resume_work);

	if (goodix_resume_wq)
		destroy_workqueue(goodix_resume_wq);
	if (goodix_init_wq)
		destroy_workqueue(goodix_init_wq);

#if GTP_CREATE_WR_NODE
	uninit_wr_node();
#endif

#if GTP_ESD_PROTECT
	destroy_workqueue(gtp_esd_check_workqueue);
#endif

	if (ts) 
	{
		if (ts->use_irq)
		{
		//	GTP_GPIO_AS_INPUT(GTP_INT_PORT);
		//	GTP_GPIO_FREE(GTP_INT_PORT);
			input_free_int(&(config_info.input_type), ts);
		}
		else
		{
			hrtimer_cancel(&ts->timer);
		}
	}   

	GTP_INFO("GTP driver removing...");
	i2c_set_clientdata(client, NULL);
	pm_runtime_disable(&client->dev);
	pm_runtime_set_suspended(&client->dev);
	input_unregister_device(ts->input_dev);
	kfree(ts);

	return 0;
}

static void goodix_resume_events (struct work_struct *work)
{
	int ret;

	ret = gtp_wakeup_sleep(ts_init);

#if GTP_SLIDE_WAKEUP
	doze_status = DOZE_DISABLED;
#endif

	if (ret < 0)
	{
		GTP_ERROR("GTP later resume failed.");
	}
#if (GTP_COMPATIBLE_MODE)
	if (CHIP_TYPE_GT9F == ts_init->chip_type)
	{
		// do nothing
	}
	else
#endif
	{
		gtp_send_cfg(ts_init->client);
	}

	if (ts_init->use_irq)
	{
		gtp_irq_enable(ts_init);
	}
	else
	{
		hrtimer_start(&ts_init->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

	ts_init->gtp_is_suspend = 0;
#if GTP_ESD_PROTECT
	gtp_esd_switch(ts_init->client, SWITCH_ON);
#endif
	return;
}

/*******************************************************
Function:
suspend function.
Input:
h: early_suspend struct.
Output:
None.
 *******************************************************/
static int goodix_ts_suspend(struct device *dev)
{
	struct goodix_ts_data *ts = dev_get_drvdata(dev);
	s8 ret = -1;

	GTP_DEBUG_FUNC();

	pm_runtime_suspended(dev);

//	if (false == ts->is_suspended) {
#if GTP_ESD_PROTECT
		gtp_esd_switch(ts->client, SWITCH_OFF);
#endif
		ts->gtp_is_suspend = 1;

#if GTP_SLIDE_WAKEUP
		ret = gtp_enter_doze(ts);
#else
		if (ts->use_irq)
		{
			gtp_irq_disable(ts);
		}
		else
		{
			hrtimer_cancel(&ts->timer);
		}
		ret = gtp_enter_sleep(ts);
#endif
		if (ret < 0)
		{
			GTP_ERROR("GTP early suspend failed.");
		}
		// to avoid waking up while not sleeping
		//  delay 48 + 10ms to ensure reliability
		msleep(58);
//	}
	ts->is_suspended = true;
		input_set_power_enable(&(config_info.input_type), 0);		
	return 0;
}

/*******************************************************
Function:
resume function.
Input:
h: early_suspend struct.
Output:
None.
 *******************************************************/
static int goodix_ts_resume(struct device *dev)
{
	struct goodix_ts_data *ts = dev_get_drvdata(dev);
	input_set_power_enable(&(config_info.input_type), 1);	
	GTP_DEBUG_FUNC();

	pm_runtime_suspended(dev);
//	if (ts->is_suspended == true) {
		queue_work(goodix_resume_wq, &goodix_resume_work);
//	}
	ts->is_suspended = false;
	printk("ts->is_suspended:%d\n",ts->is_suspended);

	return 0;
}

#if GTP_ESD_PROTECT
s32 gtp_i2c_read_no_rst(struct i2c_client *client, u8 *buf, s32 len)
{
	struct i2c_msg msgs[2];
	s32 ret=-1;
	s32 retries = 0;

	GTP_DEBUG_FUNC();

	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr  = client->addr;
	msgs[0].len   = GTP_ADDR_LENGTH;
	msgs[0].buf   = &buf[0];
	//msgs[0].scl_rate = 300 * 1000;    // for Rockchip, etc.

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr  = client->addr;
	msgs[1].len   = len - GTP_ADDR_LENGTH;
	msgs[1].buf   = &buf[GTP_ADDR_LENGTH];
	//msgs[1].scl_rate = 300 * 1000;

	while(retries < 5)
	{
		ret = i2c_transfer(client->adapter, msgs, 2);
		if(ret == 2)break;
		retries++;
	}
	if ((retries >= 5))
	{    
		GTP_ERROR("I2C Read: 0x%04X, %d bytes failed, errcode: %d!", (((u16)(buf[0] << 8)) | buf[1]), len-2, ret);
	}
	return ret;
}

s32 gtp_i2c_write_no_rst(struct i2c_client *client,u8 *buf,s32 len)
{
	struct i2c_msg msg;
	s32 ret = -1;
	s32 retries = 0;

	GTP_DEBUG_FUNC();
	msg.flags = !I2C_M_RD;
	msg.addr  = client->addr;
	msg.len   = len;
	msg.buf   = buf;
	//msg.scl_rate = 300 * 1000;    // for Rockchip, etc

	while(retries < 5)
	{
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret == 1)break;
		retries++;
	}
	if((retries >= 5))
	{
		GTP_ERROR("I2C Write: 0x%04X, %d bytes failed, errcode: %d!", (((u16)(buf[0] << 8)) | buf[1]), len-2, ret);
	}
	return ret;
}
/*******************************************************
Function:
switch on & off esd delayed work
Input:
client:  i2c device
on:      SWITCH_ON / SWITCH_OFF
Output:
void
 *********************************************************/
void gtp_esd_switch(struct i2c_client *client, s32 on)
{
	struct goodix_ts_data *ts;

	ts = i2c_get_clientdata(client);
	spin_lock(&ts->esd_lock);

	if (SWITCH_ON == on)     // switch on esd 
	{
		if (!ts->esd_running)
		{
			ts->esd_running = 1;
			spin_unlock(&ts->esd_lock);
			GTP_INFO("Esd started");
			queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, ts->clk_tick_cnt);
		}
		else
		{
			spin_unlock(&ts->esd_lock);
		}
	}
	else    // switch off esd
	{
		if (ts->esd_running)
		{
			ts->esd_running = 0;
			spin_unlock(&ts->esd_lock);
			GTP_INFO("Esd cancelled");
			cancel_delayed_work_sync(&gtp_esd_check_work);
		}
		else
		{
			spin_unlock(&ts->esd_lock);
		}
	}
}

/*******************************************************
Function:
Initialize external watchdog for esd protect
Input:
client:  i2c device.
Output:
result of i2c write operation. 
1: succeed, otherwise: failed
 *********************************************************/
static s32 gtp_init_ext_watchdog(struct i2c_client *client)
{
	u8 opr_buffer[3] = {0x80, 0x41, 0xAA};
	GTP_DEBUG("[Esd]Init external watchdog");
	return gtp_i2c_write_no_rst(client, opr_buffer, 3);
}

/*******************************************************
Function:
Esd protect function.
External watchdog added by meta, 2013/03/07
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
	u8 esd_buf[4] = {0x80, 0x40};

	GTP_DEBUG_FUNC();

	ts = i2c_get_clientdata(i2c_connect_client);

	if (ts->gtp_is_suspend)
	{
		GTP_INFO("Esd suspended!");
		return;
	}

	for (i = 0; i < 3; i++)
	{
		ret = gtp_i2c_read_no_rst(ts->client, esd_buf, 4);

		GTP_DEBUG("[Esd]0x8040 = 0x%02X, 0x8041 = 0x%02X", esd_buf[2], esd_buf[3]);
		if ((ret < 0))
		{
			// IIC communication problem
			continue;
		}
		else
		{ 
			if ((esd_buf[2] == 0xAA) || (esd_buf[3] != 0xAA))
			{
				// IC works abnormally..
				u8 chk_buf[4] = {0x80, 0x40};

				gtp_i2c_read_no_rst(ts->client, chk_buf, 4);

				GTP_DEBUG("[Check]0x8040 = 0x%02X, 0x8041 = 0x%02X", chk_buf[2], chk_buf[3]);

				if ((chk_buf[2] == 0xAA) || (chk_buf[3] != 0xAA))
				{
					i = 3;
					break;
				}
				else
				{
					continue;
				}
			}
			else 
			{
				// IC works normally, Write 0x8040 0xAA, feed the dog
				esd_buf[2] = 0xAA; 
				gtp_i2c_write_no_rst(ts->client, esd_buf, 3);
				break;
			}
		}
	}
	if (i >= 3)
	{
#if GTP_COMPATIBLE_MODE
		if (CHIP_TYPE_GT9F == ts->chip_type)
		{        
			if (ts->rqst_processing)
			{
				GTP_INFO("Request processing, no esd recovery");
			}
			else
			{
				GTP_ERROR("IC working abnormally! Process esd recovery.");
				gtp_esd_recovery(ts->client);
			}
		}
		else
#endif
		{
			GTP_ERROR("IC working abnormally! Process reset guitar.");
			gtp_reset_guitar(ts->client, 50);
		}
	}

	if(!ts->gtp_is_suspend)
	{
		queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, ts->clk_tick_cnt);
	}
	else
	{
		GTP_INFO("Esd suspended!");
	}
	return;
}
#endif

static const struct i2c_device_id goodix_ts_id[] = {
	{ GTP_I2C_NAME, 0 },
	{ }
};

static UNIVERSAL_DEV_PM_OPS(gt9xx_pm_ops, goodix_ts_suspend,
	goodix_ts_resume, NULL);

#define GT9XX_PM_OPS (&gt9xx_pm_ops)

static struct i2c_driver goodix_ts_driver = {
	.class = I2C_CLASS_HWMON,
	.probe      = goodix_ts_probe,
	.remove     = goodix_ts_remove,
	.id_table   = goodix_ts_id,
	.driver = {
		.name     = GTP_I2C_NAME,
		.owner    = THIS_MODULE,
		.pm = GT9XX_PM_OPS,		
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
static int __init goodix_ts_init(void)
{
	s32 ret = -1;

	GTP_DEBUG_FUNC();   
	GTP_INFO("GTP driver installing...");

        dprintk(DEBUG_INIT,"****************************************************************\n");
        if (input_fetch_sysconfig_para(&(config_info.input_type))) {
			printk("%s: ctp_fetch_sysconfig_para err.\n", __func__);
			return 0;
    	} else {
			ret = input_init_platform_resource(&(config_info.input_type));
			if (0 != ret) {
				printk("%s:ctp_ops.init_platform_resource err. \n", __func__);
				goto init_err;    
			}
			input_set_power_enable(&(config_info.input_type),1);			
		}
        
        if(config_info.ctp_used == 0){
	        printk("*** ctp_used set to 0 !\n");
	        printk("*** if use ctp,please put the sys_config.fex ctp_used set to 1. \n");
					ret = 0;
					goto init_err;
		}
	
        if(!ctp_get_system_config()){
                printk("%s:read config fail!\n",__func__);
								goto init_err;
        }
        
        sunxi_gpio_to_name(CTP_IRQ_NUMBER,irq_pin_name);
				sunxi_gpio_to_name(CTP_RST_NUMBER,rst_pin_name);
        gtp_io_init(20);		
				goodix_ts_driver.detect = ctp_detect;
        ret = i2c_add_driver(&goodix_ts_driver);
				if(ret)
					goto init_err;
        
        dprintk(DEBUG_INIT,"****************************************************************\n");
        return ret; 
init_err:
	input_set_power_enable(&(config_info.input_type),0);
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
	input_set_power_enable(&(config_info.input_type), 0);
	input_free_platform_resource(&(config_info.input_type));
}

late_initcall(goodix_ts_init);
module_exit(goodix_ts_exit);

MODULE_DESCRIPTION("GTP Series Driver");
MODULE_LICENSE("GPL");
