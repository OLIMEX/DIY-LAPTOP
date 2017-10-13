/* capacivite multi-touch device driver.*/


#include <linux/i2c.h>
#include <linux/input.h>
#include "tu_ts.h"
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/miscdevice.h>

#include <linux/cdev.h>
#include <linux/uaccess.h>

#include <linux/kthread.h>
#include <mach/irqs.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>
#include <linux/init-input.h>
#include <linux/gpio.h>


//#define REG_DATA_DEBUG
#ifdef CONFIG_HAS_EARLYSUSPEND
    #include <linux/pm.h>
    #include <linux/earlysuspend.h>
#endif

#define BUF_SIZE 		REPORT_BUF_SIZE
#define COORD_INTERPRET(MSB_BYTE, LSB_BYTE) \
		(MSB_BYTE << 8 | LSB_BYTE)

static void tu_early_suspend(struct early_suspend *h);
static void tu_late_resume(struct early_suspend *h);


static struct i2c_client *touch_i2c_client;

static struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
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
        printk("[CTP]: "fmt, ## arg)

module_param_named(debug_mask,debug_mask,int,S_IRUGO | S_IWUSR | S_IWGRP);

static const unsigned short normal_i2c[2] = {0x5F,I2C_CLIENT_END};

// --------------------------------------------------------------
struct tu_data{
	u16 x, y, w, p, id;
	struct i2c_client *client;
	/* capacivite device*/
	struct input_dev *dev;
	/* digitizer */
	struct timer_list timer;
	struct input_dev *dig_dev;
	struct mutex lock;
	
	struct work_struct work;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	struct workqueue_struct *tu_wq;

	int fw_ver;
	struct miscdevice firmware;
};

///////////////////////////////////////////////
//specific tp related macro: need be configured for specific tp
//#define CTP_IRQ_NO			(gpio_int_info[0].port_num)
//#define CTP_IRQ_MODE			(NEGATIVE_EDGE)
#define CTP_IRQ_NUMBER          (config_info.int_number)
#define CTP_IRQ_MODE			(IRQF_TRIGGER_FALLING)
#define CTP_NAME			TU_I2C_NAME
#define TS_RESET_LOW_PERIOD		(15)
#define TS_INITIAL_HIGH_PERIOD		(15)
#define TS_WAKEUP_LOW_PERIOD	(100)
#define TS_WAKEUP_HIGH_PERIOD	(100)
#define TS_POLL_DELAY				(10)	/* ms delay between samples */
#define TS_POLL_PERIOD			(10)	/* ms delay between samples */
#define SCREEN_MAX_X			(screen_max_x)
#define SCREEN_MAX_Y  		(screen_max_y)
#define PRESS_MAX				(255)
static struct workqueue_struct *tu_wq;

#define READ_TOUCH_ADDR_H  		0x0F
#define READ_TOUCH_ADDR_L   		0x40


static int screen_max_x = 0;
static int screen_max_y = 0;
static int revert_x_flag = 0;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;
static __u32 twi_id = 0;


static int i2c_write_bytes(struct i2c_client *client, uint8_t *data, uint16_t len)
{
	struct i2c_msg msg;
	int ret=-1;
	
	msg.flags = !I2C_M_RD;
	msg.addr = client->addr;
	msg.len = len;
	msg.buf = data;		
	
	ret=i2c_transfer(client->adapter, &msg,1);
	return ret;
}

static bool i2c_test(struct i2c_client * client)
{
        int ret,retry;
        uint8_t test_data[1] = { 0 };	//only write a data address.
        
        for(retry=0; retry < 5; retry++)
        {
                ret = i2c_write_bytes(client, test_data, 1);	//Test i2c.
        	if (ret == 1)
        	        break;
        	msleep(10);
        }
        
        return ret==1 ? true : false;
} 


/**
 * ctp_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	int ret = 0;
	struct i2c_adapter *adapter = client->adapter;
	
	if(twi_id == adapter->nr)
	{
		ret = i2c_test(client);
       		if(!ret){
        			printk("%s:I2C connection might be something wrong \n",__func__);
        			return -ENODEV;
		}else{
		         dprintk(DEBUG_INIT,"***CTP***  I2C connection sucess !\n");
			strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
			return 0;
	    	}
	}else{
		return -ENODEV;
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


/**
 * ctp_print_info - sysconfig print function
 * return value:
 *
 */
void ctp_print_info(struct ctp_config_info info,int debug_level)
{
	if(debug_level == DEBUG_INIT)
	{
		printk("info.ctp_used:%d\n",info.ctp_used);
		printk("info.twi_id:%d\n",info.twi_id);
		printk("info.screen_max_x:%d\n",info.screen_max_x);
		printk("info.screen_max_y:%d\n",info.screen_max_y);
		printk("info.revert_x_flag:%d\n",info.revert_x_flag);
		printk("info.revert_y_flag:%d\n",info.revert_y_flag);
		printk("info.exchange_x_y_flag:%d\n",info.exchange_x_y_flag);
		printk("info.irq_gpio_number:%d\n",info.irq_gpio.gpio);
		printk("info.wakeup_gpio_number:%d\n",info.wakeup_gpio.gpio);
	}
}


unsigned int kbc_i2c_read_reg(unsigned int reg);
unsigned int kbc_i2c_write_reg(unsigned int reg,unsigned int value);

unsigned int kbc_i2c_read_reg(unsigned int reg) 
{
	return i2c_smbus_read_byte_data(touch_i2c_client, reg);
}

unsigned int kbc_i2c_write_reg(unsigned int reg,unsigned int value) 
{
	return i2c_smbus_write_byte_data(touch_i2c_client, reg, value);
}

irqreturn_t tu_irq(int irq, void *dev_id)
{     
         int ret = 0;
         struct tu_data *tu = (struct tu_data *)dev_id;
	dprintk(DEBUG_INT_INFO,"==========tu_ts Interrupt============\n"); 
         ret = input_set_int_enable(&(config_info.input_type), 0);
	if (ret < 0)
		dprintk(DEBUG_INT_INFO,"%s irq disable failed\n", __func__);
	
	queue_work(tu_wq, &tu->work);
	return 0;
}

static inline void tu_report(struct tu_data *tu)
{
         dprintk(DEBUG_X_Y_INFO,"source data:ID:%d,x=%d,y=%d,w=%d\n", tu->id,tu->x,tu->y,tu->w);
	if(1 == exchange_x_y_flag){
		swap(tu->x, tu->y);
	}
	if(1 == revert_x_flag){
		tu->x = SCREEN_MAX_X - tu->x;
	}
	if(1 == revert_y_flag){
		tu->y= SCREEN_MAX_Y - tu->y;
	}
	dprintk(DEBUG_X_Y_INFO," report data:ID:%d,x=%d,y=%d,w=%d\n",tu->id,tu->x,tu->y,tu->w);	
	input_report_abs(tu->dev, ABS_MT_POSITION_X, tu->x);
	input_report_abs(tu->dev, ABS_MT_POSITION_Y, tu->y);
	input_report_abs(tu->dev, ABS_MT_TOUCH_MAJOR, tu->w);
	input_report_abs(tu->dev, ABS_MT_WIDTH_MAJOR, tu->w);
	input_report_abs(tu->dev, ABS_MT_TRACKING_ID, tu->id);
	
	input_mt_sync(tu->dev);
}

static void tu_i2c_work(struct work_struct *work)
{
	int i = 0;
	u_int8_t ret = 0;
	u_int8_t idx_x_low;
	u_int8_t idx_x_hi;
	u_int8_t idx_y_low;
	u_int8_t idx_y_hi;
	u_int8_t idx_id_st;
	u_int8_t touchcnt;
	u_int8_t touchstatus;
         u8 read_buf[REPORT_BUF_SIZE+1]={0};
		 
#ifdef HAVE_TOUCH_KEY
	static unsigned int prev_key = 0;
#endif

	struct tu_data *tu = container_of(work, struct tu_data, work);

	ret = i2c_smbus_read_i2c_block_data(tu->client, 0, BUF_SIZE,read_buf);
	
	if(ret < 0)
	{
		printk("Read error!!!!!\n");
		goto report_work_fail;
	}

#ifdef REG_DATA_DEBUG
	printk("\nRead: "); 
	for(i = 0; i < BUF_SIZE; i ++) 
	{
		printk("%4x", read_buf[i]);
	}
	printk("\n");
#endif
    
	if (read_buf[TU_RMOD] == 0xb1)		//Point Report
	{
		touchcnt = read_buf[TU_POINTS] & 0x0f;
		dprintk(DEBUG_OTHERS_INFO,"touchcnt = %d  !!\n",touchcnt);
		if(touchcnt>5)
			goto report_work_fail;
                 touchstatus = read_buf[TU_1_ID_STATUS];
		if( touchstatus==0x00) 
		{
		  	dprintk(DEBUG_OTHERS_INFO,"touch  up   !!\n");
			input_report_abs(tu->dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(tu->dev, ABS_MT_WIDTH_MAJOR, 0);
			input_mt_sync(tu->dev);		
		}
		else
		{
			idx_x_low = TU_1_POS_X_LOW;
			idx_x_hi  = TU_1_POS_X_HI;
			idx_y_low = TU_1_POS_Y_LOW;
			idx_y_hi  = TU_1_POS_Y_HI;
			idx_id_st = TU_1_ID_STATUS;

			for( i=0; i<touchcnt; i++ )
			{
				tu->x = COORD_INTERPRET(read_buf[idx_x_hi], read_buf[idx_x_low]);
				tu->y = COORD_INTERPRET(read_buf[idx_y_hi], read_buf[idx_y_low]);

				//tu->w = (read_buf[idx_id_st]&0x0f);
				tu->w = 10;
				tu->id = (read_buf[idx_id_st]>>4)&0x0f;

				tu->x = 	(tu->x * SCREEN_MAX_X) /AA_X_SIZE;
				tu->y = 	(tu->y * SCREEN_MAX_Y ) /AA_Y_SIZE;
				tu_report(tu);		

				idx_x_low  += MAX_POINT_SIZE;
				idx_x_hi   += MAX_POINT_SIZE;
				idx_y_low  += MAX_POINT_SIZE;
				idx_y_hi   += MAX_POINT_SIZE;
				idx_id_st  += MAX_POINT_SIZE;
			}
		}
	} else if(read_buf[TU_RMOD] == 0x00){
                dprintk(DEBUG_OTHERS_INFO,"touch  up   !!\n");
		input_report_abs(tu->dev, ABS_MT_TOUCH_MAJOR, 0);
		input_report_abs(tu->dev, ABS_MT_WIDTH_MAJOR, 0);
		input_mt_sync(tu->dev);	
	}
#ifdef HAVE_TOUCH_KEY   
	else if (read_buf[TU_RMOD] == 0xb2) //Key Detect
	{
	        dprintk(DEBUG_KEY_INFO,"tu: have touch key !\n");
		switch (read_buf[TU_KEY_CODE]) 
		{
			case TOUCH_KEY_HOME:
				input_event(tu->dev, EV_KEY, KEY_HOME, !!read_buf[TU_KEY_CODE]);
				prev_key = KEY_HOME;
				break;
			case TOUCH_KEY_BACK:
				input_event(tu->dev, EV_KEY, KEY_BACK, !!read_buf[TU_KEY_CODE]);
				prev_key = KEY_BACK;
				break;
			case TOUCH_KEY_MENU:
				input_event(tu->dev, EV_KEY, KEY_MENU, !!read_buf[TU_KEY_CODE]);
				prev_key = KEY_MENU;
				break;
			case TOUCH_KEY_REL:
				input_event(tu->dev, EV_KEY, prev_key, !!read_buf[TU_KEY_CODE]);
				break;
			case TOUCH_KEY_VOL_UP:
				input_event(tu->dev, EV_KEY, KEY_VOLUMEUP, !!read_buf[TU_KEY_CODE]);
				prev_key = KEY_VOLUMEUP;
				break;
			case TOUCH_KEY_VOL_DOWN:
				input_event(tu->dev, EV_KEY, KEY_VOLUMEDOWN, !!read_buf[TU_KEY_CODE]);
				prev_key = KEY_VOLUMEDOWN;
				break;
			case TOUCH_KEY_CALL:
				input_event(tu->dev, EV_KEY, KEY_SEND, !!read_buf[TU_KEY_CODE]);
				prev_key = KEY_SEND;
				break;
			default:
				dev_dbg(&tu->client->dev, "Unknown Android Key %02x", read_buf[TU_KEY_CODE]);
				break;
		}	
	
	}
#endif
	input_sync(tu->dev);
	dprintk(DEBUG_OTHERS_INFO,"in tu_i2c_work!!!!!\n");
report_work_fail:
         ret = input_set_int_enable(&(config_info.input_type), 1);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq enable failed\n", __func__);
	
}


static int tu_probe(struct i2c_client *client, const struct i2c_device_id *ids)
{
	int err = 0;
	struct tu_data *tu;
	struct input_dev *input_dev;

	dprintk(DEBUG_INIT,"====%s begin=====.  \n", __func__);
	tu = kzalloc(sizeof(struct tu_data), GFP_KERNEL);
	if (!tu)
		return -ENOMEM;

	tu->client = client;
	touch_i2c_client=client;

	dev_info(&tu->client->dev, "device probing\n");
	i2c_set_clientdata(client, tu);
	mutex_init(&tu->lock);

	//Open Device Node for APK Update
   	tu->firmware.minor = MISC_DYNAMIC_MINOR;
   	tu->firmware.name = "TU_Update";
    	tu->firmware.mode = S_IRWXUGO; 
    	if (misc_register(&tu->firmware) < 0)
        		printk("[tu]misc_register failed!!");
   	 else
        		printk("[tu]misc_register finished!!"); 
		
	/* allocate input device for capacitive */
	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&tu->client->dev, "failed to allocate input device \n");
		goto exit_kfree;
	}
	
	//Initial Device Parameters
	input_dev->name = CTP_NAME;
	input_dev->phys = "input/tu-ts";
	input_dev->id.bustype = BUS_I2C;
	input_dev->id.vendor = 0x0EFF;
	input_dev->id.product = 0x0020;
	tu->dev = input_dev;

	//Key bit initialize
	input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) |BIT_MASK(EV_KEY);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	
#ifdef HAVE_TOUCH_KEY       
    	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(KEY_HOME, input_dev->keybit);
	set_bit(KEY_VOLUMEUP, input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, input_dev->keybit);
	set_bit(KEY_SEND, input_dev->keybit);
#endif

         set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	//Set Coordinate
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 8, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 8, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 4, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 256, 0, 0);


	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);  //896
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);  //576
	
	//Register Device Object
	err = input_register_device(input_dev);
	if (err)
		goto exit_input;

	//initialize I2C Work
	INIT_WORK(&tu->work, tu_i2c_work);
	tu_wq = create_singlethread_workqueue("tu_wq");
	if (!tu_wq) {
		printk(KERN_ALERT "Creat %s workqueue failed.\n", __func__);
		return -ENOMEM;	
	}

    	flush_workqueue(tu_wq);

	config_info.dev = &(tu->dev->dev);
	err = input_request_int(&(config_info.input_type), tu_irq,CTP_IRQ_MODE, tu);
	if (err) {
		pr_info( "tu_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	tu->early_suspend.level		= EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	tu->early_suspend.suspend	= tu_early_suspend;
	tu->early_suspend.resume	= tu_late_resume;
	register_early_suspend(&tu->early_suspend);
#endif

	return 0;

exit_irq_request_failed:
	input_free_int(&(config_info.input_type), tu);
exit_input:
	input_unregister_device(tu->dev);
exit_kfree:
	kfree(tu);
	return err;
}

static int __devexit tu_remove(struct i2c_client *client)
{
	struct tu_data *tu = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND	
	unregister_early_suspend(&tu->early_suspend);
#endif
	input_free_int(&(config_info.input_type), tu);

	flush_workqueue(tu_wq);
	if (tu_wq)
		destroy_workqueue(tu_wq);
        misc_deregister(&tu->firmware);
	input_unregister_device(tu->dev);
	kfree(tu);
	
	return 0;
}


static int tu_suspend(struct i2c_client *client, pm_message_t state)
{
	int ret;
	struct tu_data *ts = i2c_get_clientdata(touch_i2c_client);
	printk("[CTP]: %s         suspend!\n",__func__);
	
         ret = input_set_int_enable(&(config_info.input_type), 0);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq disable failed\n", __func__);
         ret = cancel_work_sync(&ts->work);
	ret = i2c_smbus_write_i2c_block_data(touch_i2c_client, 0, 4, command_list[0]);
	if (ret < 0)
		printk(KERN_ERR "tu_suspend: send i2c data failed\n");
	
	return 0;
}



static int tu_resume(struct i2c_client *client)
{
	int ret;
	
	printk("[CTP]: %s         resume!\n",__func__);
	ret = input_set_int_enable(&(config_info.input_type), 1);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq enable failed\n", __func__);
	ret = i2c_smbus_write_i2c_block_data(touch_i2c_client, 0, 4, command_list[1]);
	if (ret < 0)
		printk(KERN_ERR "tu_resume: send i2c data failed\n");
		
	return 0;
}


#ifdef CONFIG_HAS_EARLYSUSPEND
static void tu_early_suspend(struct early_suspend *h)
{

    	dprintk(DEBUG_SUSPEND,"\ntu_early_suspend\n"); 
	tu_suspend(touch_i2c_client, PMSG_SUSPEND);

}

static void tu_late_resume(struct early_suspend *h)
{
        dprintk(DEBUG_WAKEUP_INFO,"\tu_late_resume\n");
	tu_resume(touch_i2c_client);
}
#endif

static struct i2c_device_id tu_id_table[] = {
    	{CTP_NAME,0 },
   	{ }
};

static struct i2c_driver tu_driver = {
   	.class = I2C_CLASS_HWMON,
   	.probe = tu_probe,
	.remove = tu_remove,
#ifdef CONFIG_HAS_EARLYSUSPEND
#else
#ifdef CONFIG_PM
	.suspend = tu_suspend,
	.resume =  tu_resume,
#endif
#endif
	.id_table 	= tu_id_table,
	.driver = {
		.name = CTP_NAME,
        		.owner = THIS_MODULE,
	},
    	.address_list = normal_i2c,
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

static int __init tu_init(void)
{
	int ret = -1;      
	dprintk(DEBUG_INIT,"***************************init begin*************************************\n");
	if (input_fetch_sysconfig_para(&(config_info.input_type))) {
		printk("%s: ctp_fetch_sysconfig_para err.\n", __func__);
		return 0;
	} else {
		ret = input_init_platform_resource(&(config_info.input_type));
		if (0 != ret) {
			printk("%s:init_platform_resource err. \n", __func__);    
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
	ctp_wakeup(0, 50);
	tu_driver.detect = ctp_detect,
		
	ret = i2c_add_driver(&tu_driver);
	return ret;
}

static void tu_exit(void)
{
         printk("[CTP]: ctp driver exit !!     %s\n",__func__);
	i2c_del_driver(&tu_driver);
	input_free_platform_resource(&(config_info.input_type));
}

late_initcall(tu_init);
module_exit(tu_exit);

MODULE_DESCRIPTION("TU Touchscreen Driver");
MODULE_LICENSE("GPL v2");


