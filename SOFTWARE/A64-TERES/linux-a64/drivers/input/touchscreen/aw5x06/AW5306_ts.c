/**************************************************************************
*  AW5306_ts.c
* 
*  AW5306 ALLWIN sample code version 1.0
* 
*  Create Date : 2012/06/07
* 
*  Modify Date : 
*
*  Create by   : wuhaijun
* 
**************************************************************************/

#include <linux/i2c.h>
#include <linux/input.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
    #include <linux/pm.h>
    #include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/async.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/file.h>
#include <linux/proc_fs.h>

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/irqs.h>
//#include <mach/system.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>
//#include "ctp_platform_ops.h"

#include "AW5306_Drv.h"
#include "AW5306_userpara.h"
#define CONFIG_AW5306_MULTITOUCH     (1)
#define FOR_TSLIB_TEST
//#define PRINT_INT_INFO
//#define PRINT_POINT_INFO
//#define DEBUG
//#define TOUCH_KEY_SUPPORT
#ifdef TOUCH_KEY_SUPPORT
//#define TOUCH_KEY_LIGHT_SUPPORT
#define TOUCH_KEY_FOR_EVB13
//#define TOUCH_KEY_FOR_ANGDA
#ifdef TOUCH_KEY_FOR_ANGDA
#define TOUCH_KEY_X_LIMIT	(60000)
#define TOUCH_KEY_NUMBER	(4)
#endif
#ifdef TOUCH_KEY_FOR_EVB13
#define TOUCH_KEY_LOWER_X_LIMIT	(848)
#define TOUCH_KEY_HIGHER_X_LIMIT	(852)
#define TOUCH_KEY_NUMBER	(5)
#endif
#endif

#define TOUCH_KEY_NUMBER	(5)
#define AW5306_NAME	"aw5306_ts"//"synaptics_i2c_rmi"//"synaptics-rmi-ts"// 
#define I2C_CTPM_ADDRESS        (0x39)
#define CTP_NAME			AW5306_NAME

struct i2c_dev{
struct list_head list;	
struct i2c_adapter *adap;
struct device *dev;
};

static struct class *i2c_dev_class;
static LIST_HEAD (i2c_dev_list);
//static DEFINE_SPINLOCK(i2c_dev_list_lock);

static struct i2c_client *this_client;
#ifdef TOUCH_KEY_LIGHT_SUPPORT
static int gpio_light_hdle = 0;
#endif
#ifdef TOUCH_KEY_SUPPORT
static int key_tp  = 0;
static int key_val = 0;
#endif

#ifdef PRINT_POINT_INFO 
#define print_point_info(fmt, args...)   \
        do{                              \
                pr_info(fmt, ##args);     \
        }while(0)
#else
#define print_point_info(fmt, args...)   //
#endif

#ifdef PRINT_INT_INFO 
#define print_int_info(fmt, args...)     \
        do{                              \
                pr_info(fmt, ##args);     \
        }while(0)
#else
#define print_int_info(fmt, args...)   //
#endif


#define CTP_IRQ_MODE			(NEGATIVE_EDGE)

#define TS_RESET_LOW_PERIOD		(1)
#define TS_INITIAL_HIGH_PERIOD		(30)
#define TS_WAKEUP_LOW_PERIOD	(20)
#define TS_WAKEUP_HIGH_PERIOD	(20)
#define TS_POLL_DELAY			(10)	/* ms delay between samples */
#define TS_POLL_PERIOD			(10)	/* ms delay between samples */
//#define SCREEN_MAX_X			(800)
//#define SCREEN_MAX_Y			(480)
#define PRESS_MAX			(255)

//#define AUTO_RUDUCEFRAME	
#if 0
static void* __iomem gpio_addr = NULL;
static int gpio_int_hdle = 0;
static int gpio_wakeup_hdle = 0;
static int gpio_reset_hdle = 0;
static int gpio_wakeup_enable = 1;
static int gpio_reset_enable = 1;
static int  store_pin_num;
#endif
int screen_max_x = 0;
int screen_max_y = 0;
#define SCREEN_MAX_X			(screen_max_x)
#define SCREEN_MAX_Y			(screen_max_y)


static int revert_x_flag = 0;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;
//static int	int_cfg_addr[]={PIO_INT_CFG0_OFFSET,PIO_INT_CFG1_OFFSET,
//			PIO_INT_CFG2_OFFSET, PIO_INT_CFG3_OFFSET};
/* Addresses to scan */
//static union{
//	unsigned short dirty_addr_buf[2];
//	const unsigned short normal_i2c[2];
//}u_i2c_addr = {{0x00},};
static __u32 twi_id = 0;


//lkj====begin==================

#include <mach/irqs.h>
//#include <mach/system.h>
#include <mach/hardware.h>
#include <mach/gpio.h> 
#include <linux/init-input.h>
#include <linux/gpio.h>

extern struct ctp_config_info config_info;

static struct i2c_client *this_client;

struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
	.name = NULL,
	.int_number = 0,
};


#ifdef TOUCH_KEY_SUPPORT
static int key_tp  = 0;
static int key_val = 0;
#endif

#define CTP_IRQ_NUMBER                  (config_info.irq_gpio_number)
//#define CTP_IRQ_MODE			(TRIG_EDGE_NEGATIVE)
#define CTP_NAME			 AW5306_NAME
//#define SCREEN_MAX_X			(screen_max_x)
//#define SCREEN_MAX_Y			(screen_max_y)
//#define PRESS_MAX			(255)

//static int screen_max_x = 0;
//static int screen_max_y = 0;
//static int revert_x_flag = 0;
//static int revert_y_flag = 0;
//static int exchange_x_y_flag = 0;
//static u32 int_handle = 0;

/* Addresses to scan */
static const unsigned short normal_i2c[2] = {0x38,I2C_CLIENT_END};
static const int chip_id_value[2] = {0xA8,0x0};
//static __u32 twi_id = 0;



static int ctp_get_system_config(void)
{   
       
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


/**
 * ctp_wakeup - function
 *
 */
int ctp_wakeup(int status,int ms)
{
	printk("***CTP*** %s:status:%d,ms = %d\n",__func__,status,ms);

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


static int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
        
        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;


	if(twi_id == adapter->nr)
	{
		pr_info("%s: Detected chip %s at adapter %d, address 0x%02x\n",
			 __func__, CTP_NAME, i2c_adapter_id(adapter), client->addr);

		strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
		return 0;
	}else{
		return -ENODEV;
	}

#if 0
	if(twi_id == adapter->nr){
        	int ret = 0, i = 0;
		if (client->addr == 0x38){
	        ret = i2c_smbus_read_byte_data(client,0x01);
                printk("addr:0x%x,ret:0x%x\n",client->addr,ret);
                while(chip_id_value[i++]){
                        if(ret == chip_id_value[i - 1]){
                		printk("found 0xa8\n");
            	                strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
    		                return 0;
                        }                   
                }
		}
	/*       
		else if (client->addr == 0x39) {
		                printk("addr is 0x39 \n");
            	                strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
    		                return 0;
		}
*/

        	printk("%s:I2C connection might be something wrong ! \n",__func__);
        	return -ENODEV;
	}else{
		return -ENODEV;
	}

#endif
}


void __aeabi_unwind_cpp_pr0(void)
{
};

void __aeabi_unwind_cpp_pr1(void)
{
};

//lkj=======end===============



#if 0

///////////////////////////////////////////////
//specific tp related macro: need be configured for specific tp
#define CTP_IRQ_NO			(gpio_int_info[0].port_num)
static user_gpio_set_t  gpio_int_info[1];


/*
 * ctp_get_pendown_state  : get the int_line data state, 
 * 
 * return value:
 *             return PRESS_DOWN: if down
 *             return FREE_UP: if up,
 *             return 0: do not need process, equal free up.
 */
static int ctp_get_pendown_state(void)
{
	unsigned int reg_val;
	static int state = FREE_UP;

	//get the input port state
	reg_val = readl(gpio_addr + PIOH_DATA);
	//pr_info("reg_val = %x\n",reg_val);
	if(!(reg_val & (1<<CTP_IRQ_NO))){
		state = PRESS_DOWN;
		print_int_info("pen down. \n");
	}else{ //touch panel is free up
		state = FREE_UP;
		print_int_info("free up. \n");
	}
	return state;
}

/**
 * ctp_clear_penirq - clear int pending
 *
 */
static void ctp_clear_penirq(void)
{
	int reg_val;
	//clear the IRQ_EINT29 interrupt pending
	//pr_info("clear pend irq pending\n");
	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);
	//writel(reg_val,gpio_addr + PIO_INT_STAT_OFFSET);
	//writel(reg_val&(1<<(IRQ_EINT21)),gpio_addr + PIO_INT_STAT_OFFSET);
	if((reg_val = (reg_val&(1<<(CTP_IRQ_NO))))){
		print_int_info("==CTP_IRQ_NO=\n");              
		writel(reg_val,gpio_addr + PIO_INT_STAT_OFFSET);
	}
	return;
}

/**
 * ctp_set_irq_mode - according sysconfig's subkey "ctp_int_port" to config int port.
 * 
 * return value: 
 *              0:      success;
 *              others: fail; 
 */
static int ctp_set_irq_mode(char *major_key , char *subkey, ext_int_mode int_mode)
{
	int ret = 0;
	__u32 reg_num = 0;
	__u32 reg_addr = 0;
	__u32 reg_val = 0;
	//config gpio to int mode
	pr_info("%s: config gpio to int mode. \n", __func__);
#ifndef SYSCONFIG_GPIO_ENABLE
#else
	if(gpio_int_hdle){
		gpio_release(gpio_int_hdle, 2);
	}
	gpio_int_hdle = gpio_request_ex(major_key, subkey);
	if(!gpio_int_hdle){
		pr_info("request tp_int_port failed. \n");
		ret = -1;
		goto request_tp_int_port_failed;
	}
	gpio_get_one_pin_status(gpio_int_hdle, gpio_int_info, subkey, 1);
	pr_info("%s, %d: gpio_int_info, port = %d, port_num = %d. \n", __func__, __LINE__, \
		gpio_int_info[0].port, gpio_int_info[0].port_num);
#endif

#ifdef AW_GPIO_INT_API_ENABLE
#else
	pr_info(" INTERRUPT CONFIG\n");
	reg_num = (gpio_int_info[0].port_num)%8;
	reg_addr = (gpio_int_info[0].port_num)/8;
	reg_val = readl(gpio_addr + int_cfg_addr[reg_addr]);
	reg_val &= (~(7 << (reg_num * 4)));
	reg_val |= (int_mode << (reg_num * 4));
	writel(reg_val,gpio_addr+int_cfg_addr[reg_addr]);
                                                               
	ctp_clear_penirq();
                                                               
	reg_val = readl(gpio_addr+PIO_INT_CTRL_OFFSET); 
	reg_val |= (1 << (gpio_int_info[0].port_num));
	writel(reg_val,gpio_addr+PIO_INT_CTRL_OFFSET);

	udelay(1);
#endif

request_tp_int_port_failed:
	return ret;  
}

/**
 * ctp_set_gpio_mode - according sysconfig's subkey "ctp_io_port" to config io port.
 *
 * return value: 
 *              0:      success;
 *              others: fail; 
 */
static int ctp_set_gpio_mode(void)
{
	//int reg_val;
	int ret = 0;
	//config gpio to io mode
	pr_info("%s: config gpio to io mode. \n", __func__);
#ifndef SYSCONFIG_GPIO_ENABLE
#else
	if(gpio_int_hdle){
		gpio_release(gpio_int_hdle, 2);
	}
	gpio_int_hdle = gpio_request_ex("ctp_para", "ctp_io_port");
	if(!gpio_int_hdle){
		pr_info("request ctp_io_port failed. \n");
		ret = -1;
		goto request_tp_io_port_failed;
	}
#endif
	return ret;

request_tp_io_port_failed:
	return ret;
}

/**
 * ctp_judge_int_occur - whether interrupt occur.
 *
 * return value: 
 *              0:      int occur;
 *              others: no int occur; 
 */
static int ctp_judge_int_occur(void)
{
	//int reg_val[3];
	int reg_val;
	int ret = -1;

	reg_val = readl(gpio_addr + PIO_INT_STAT_OFFSET);
	if(reg_val&(1<<(CTP_IRQ_NO))){
		ret = 0;
	}
	return ret; 	
}

/**
 * ctp_free_platform_resource - corresponding with ctp_init_platform_resource
 *
 */
static void ctp_free_platform_resource(void)
{
	if(gpio_addr){
		iounmap(gpio_addr);
	}
	
	if(gpio_int_hdle){
		gpio_release(gpio_int_hdle, 2);
	}
	
	if(gpio_wakeup_hdle){
		gpio_release(gpio_wakeup_hdle, 2);
	}
	
	if(gpio_reset_hdle){
		gpio_release(gpio_reset_hdle, 2);
	}

	return;
}


/**
 * ctp_init_platform_resource - initialize platform related resource
 * return value: 0 : success
 *               -EIO :  i/o err.
 *
 */
static int ctp_init_platform_resource(void)
{
	int ret = 0;

	gpio_addr = ioremap(PIO_BASE_ADDRESS, PIO_RANGE_SIZE);
	//pr_info("%s, gpio_addr = 0x%x. \n", __func__, gpio_addr);
	if(!gpio_addr) {
		ret = -EIO;
		goto exit_ioremap_failed;	
	}
	//    gpio_wakeup_enable = 1;
	gpio_wakeup_hdle = gpio_request_ex("ctp_para", "ctp_wakeup");
	if(!gpio_wakeup_hdle) {
		pr_warning("%s: tp_wakeup request gpio fail!\n", __func__);
		gpio_wakeup_enable = 0;
	}

	gpio_reset_hdle = gpio_request_ex("ctp_para", "ctp_reset");
	if(!gpio_reset_hdle) {
		pr_warning("%s: tp_reset request gpio fail!\n", __func__);
		gpio_reset_enable = 0;
	}

	return ret;

exit_ioremap_failed:
	ctp_free_platform_resource();
	return ret;
}


/**
 * ctp_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ctp_fetch_sysconfig_para(void)
{
	int ret = -1;
	int ctp_used = -1;
	char name[I2C_NAME_SIZE];
	__u32 twi_addr = 0;
	//__u32 twi_id = 0;
	script_parser_value_type_t type = SCIRPT_PARSER_VALUE_TYPE_STRING;

	pr_info("%s. \n", __func__);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_used", &ctp_used, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	if(1 != ctp_used){
		pr_err("%s: ctp_unused. \n",  __func__);
		//ret = 1;
		return ret;
	}

	if(SCRIPT_PARSER_OK != script_parser_fetch_ex("ctp_para", "ctp_name", (int *)(&name), &type, sizeof(name)/sizeof(int))){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	if(strcmp(CTP_NAME, name)){
		pr_err("%s: name %s does not match CTP_NAME. \n", __func__, name);
		pr_err(CTP_NAME);
		//ret = 1;
		return ret;
	}

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_twi_addr", &twi_addr, sizeof(twi_addr)/sizeof(__u32))){
		pr_err("%s: script_parser_fetch err. \n", name);
		goto script_parser_fetch_err;
	}
	//big-endian or small-endian?
	//pr_info("%s: before: ctp_twi_addr is 0x%x, dirty_addr_buf: 0x%hx. dirty_addr_buf[1]: 0x%hx \n", __func__, twi_addr, u_i2c_addr.dirty_addr_buf[0], u_i2c_addr.dirty_addr_buf[1]);
	u_i2c_addr.dirty_addr_buf[0] = twi_addr;
	u_i2c_addr.dirty_addr_buf[1] = I2C_CLIENT_END;
	pr_info("%s: after: ctp_twi_addr is 0x%x, dirty_addr_buf: 0x%hx. dirty_addr_buf[1]: 0x%hx \n", __func__, twi_addr, u_i2c_addr.dirty_addr_buf[0], u_i2c_addr.dirty_addr_buf[1]);
	//pr_info("%s: after: ctp_twi_addr is 0x%x, u32_dirty_addr_buf: 0x%hx. u32_dirty_addr_buf[1]: 0x%hx \n", __func__, twi_addr, u32_dirty_addr_buf[0],u32_dirty_addr_buf[1]);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_twi_id", &twi_id, sizeof(twi_id)/sizeof(__u32))){
		pr_err("%s: script_parser_fetch err. \n", name);
		goto script_parser_fetch_err;
	}
	pr_info("%s: ctp_twi_id is %d. \n", __func__, twi_id);
	
	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_x", &screen_max_x, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: screen_max_x = %d. \n", __func__, screen_max_x);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_y", &screen_max_y, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: screen_max_y = %d. \n", __func__, screen_max_y);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_revert_x_flag", &revert_x_flag, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: revert_x_flag = %d. \n", __func__, revert_x_flag);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_revert_y_flag", &revert_y_flag, 1)){
		pr_err("%s: script_parser_fetch err. \n", __func__);
		goto script_parser_fetch_err;
	}
	pr_info("%s: revert_y_flag = %d. \n", __func__, revert_y_flag);

	if(SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_exchange_x_y_flag", &exchange_x_y_flag, 1)){
		pr_err("AW5306_ts: script_parser_fetch err. \n");
		goto script_parser_fetch_err;
	}
	pr_info("%s: exchange_x_y_flag = %d. \n", __func__, exchange_x_y_flag);

	return 0;

script_parser_fetch_err:
	pr_notice("=========script_parser_fetch_err============\n");
	return ret;
}
static  void ft_ctp_reset(void)
{
      if(1 == gpio_wakeup_enable){  
		pr_info("%s. \n", __func__);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 0, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_WAKEUP_LOW_PERIOD);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_WAKEUP_HIGH_PERIOD);

	}
	return;
}

/**
 * ctp_reset - function
 *
 */
static void ctp_reset(void)
{
	if(gpio_reset_enable){
		pr_info("%s. \n", __func__);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_reset_hdle, 1, "ctp_reset")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_RESET_LOW_PERIOD);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_reset_hdle, 0, "ctp_reset")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_INITIAL_HIGH_PERIOD);
	}
}

/**
 * ctp_wakeup - function
 *
 */
static void ctp_wakeup(void)
{
	if(1 == gpio_wakeup_enable){  
		pr_info("%s. \n", __func__);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_WAKEUP_LOW_PERIOD);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 0, "ctp_wakeup")){
			pr_info("%s: err when operate gpio. \n", __func__);
		}
		mdelay(TS_WAKEUP_HIGH_PERIOD);

	}
	return;
}
/**
 * ctp_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if(twi_id == adapter->nr)
	{
		pr_info("%s: Detected chip %s at adapter %d, address 0x%02x\n",
			 __func__, CTP_NAME, i2c_adapter_id(adapter), client->addr);

		strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
		return 0;
	}else{
		return -ENODEV;
	}
}
////////////////////////////////////////////////////////////////

static struct ctp_platform_ops ctp_ops = {
	.get_pendown_state = ctp_get_pendown_state,
	.clear_penirq	   = ctp_clear_penirq,
	.set_irq_mode      = ctp_set_irq_mode,
	.set_gpio_mode     = ctp_set_gpio_mode,	
	.judge_int_occur   = ctp_judge_int_occur,
	.init_platform_resource = ctp_init_platform_resource,
	.free_platform_resource = ctp_free_platform_resource,
	.fetch_sysconfig_para = ctp_fetch_sysconfig_para,
	.ts_reset =          ctp_reset,
	.ts_wakeup =         ctp_wakeup,
	.ts_detect = ctp_detect,
};
#endif
int fts_ctpm_fw_upgrade_with_i_file(void);

#if 0
static struct i2c_dev *i2c_dev_get_by_minor(unsigned index)
{
	struct i2c_dev *i2c_dev;
	spin_lock(&i2c_dev_list_lock);
	
	list_for_each_entry(i2c_dev,&i2c_dev_list,list){
		pr_info("--line = %d ,i2c_dev->adapt->nr = %d,index = %d.\n",__LINE__,i2c_dev->adap->nr,index);
		if(i2c_dev->adap->nr == index){
		     goto found;
		}
	}
	i2c_dev = NULL;
	
found: 
	spin_unlock(&i2c_dev_list_lock);
	
	return i2c_dev ;
}
#endif

struct ts_event {
	int	x[5];
	int	y[5];
	int	pressure;
	int touch_ID[5];
	int touch_point;
	int pre_point;
};

struct AW5306_ts_data {
	struct input_dev	*input_dev;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct workqueue_struct *ts_workqueue;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
	struct timer_list touch_timer;
};


/* ---------------------------------------------------------------------
*
*   Focal Touch panel upgrade related driver
*
*
----------------------------------------------------------------------*/

typedef enum
{
	ERR_OK,
	ERR_MODE,
	ERR_READID,
	ERR_ERASE,
	ERR_STATUS,
	ERR_ECC,
	ERR_DL_ERASE_FAIL,
	ERR_DL_PROGRAM_FAIL,
	ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit 

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE               0x0


//extern char AW5306_CLB();
//extern void AW5306_CLB_GetCfg();
extern STRUCTCALI       AW_Cali;
extern AW5306_UCF   AWTPCfg;
extern STRUCTBASE		AW_Base;
extern short	Diff[NUM_TX][NUM_RX];
extern short	adbDiff[NUM_TX][NUM_RX];
extern short	AWDeltaData[32];

static unsigned char suspend_flag=0; //0: sleep out; 1: sleep in
static short tp_idlecnt = 0;
static char tp_SlowMode = 1;


int AW_nvram_read(char *filename, char *buf, ssize_t len, int offset)
{	
    struct file *fd;
    //ssize_t ret;
    int retLen = -1;
    
    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    fd = filp_open(filename, O_RDONLY, 0);
    
    if(IS_ERR(fd)) {
        printk("[AW5306][nvram_read] : failed to open!!\n");
        return -1;
    }
    do{
        if ((fd->f_op == NULL) || (fd->f_op->read == NULL))
    		{
            printk("[AW5306][nvram_read] : file can not be read!!\n");
            break;
    		} 
    		
        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        		    if(fd->f_op->llseek(fd, offset, 0) != offset) {
						printk("[AW5306][nvram_read] : failed to seek!!\n");
					    break;
        		    }
        	  } else {
        		    fd->f_pos = offset;
        	  }
        }    		
        
    		retLen = fd->f_op->read(fd,
    									  buf,
    									  len,
    									  &fd->f_pos);			
    		
    }while(false);
    
    filp_close(fd, NULL);
    
    set_fs(old_fs);
    
    return retLen;
}

int AW_nvram_write(char *filename, char *buf, ssize_t len, int offset)
{	
    struct file *fd;
    //ssize_t ret;
    int retLen = -1;
        
    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    fd = filp_open(filename, O_WRONLY|O_CREAT, 0666);
    
    if(IS_ERR(fd)) {
        printk("[AW5306][nvram_write] : failed to open!!\n");
        return -1;
    }
    do{
        if ((fd->f_op == NULL) || (fd->f_op->write == NULL))
    		{
            printk("[AW5306][nvram_write] : file can not be write!!\n");
            break;
    		} /* End of if */
    		
        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        	    if(fd->f_op->llseek(fd, offset, 0) != offset) {
				    printk("[AW5306][nvram_write] : failed to seek!!\n");
                    break;
                }
            } else {
                fd->f_pos = offset;
            }
        }       		
        
        retLen = fd->f_op->write(fd,
                                 buf,
                                 len,
                                 &fd->f_pos);			
    		
    }while(false);
    
    filp_close(fd, NULL);
    
    set_fs(old_fs);
    
    return retLen;
}

int AW_I2C_WriteByte(u8 addr, u8 para)
{
	int ret;
	u8 buf[3];
	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		},
	};

	buf[0] = addr;
	buf[1] = para;

	ret = i2c_transfer(this_client->adapter, msg, 1);

	return ret;
}

unsigned char AW_I2C_ReadByte(u8 addr)
{
	int ret;
	u8 buf[2] = {0};
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};

	buf[0] = addr;

	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);

	return buf[0];
  
}

unsigned char AW_I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len)
{
	int ret,i;
	u8 rdbuf[512] = {0};
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rdbuf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= len,
			.buf	= rdbuf,
		},
	};

	rdbuf[0] = addr;
	

	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	for(i = 0; i < len; i++)
	{
		buf[i] = rdbuf[i];
	}

    return ret;
}

unsigned char AW_I2C_WriteXByte( unsigned char *buf, unsigned char addr, unsigned short len)
{
	int ret,i;
	u8 wdbuf[512] = {0};
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= len+1,
			.buf	= wdbuf,
		}
	};

	wdbuf[0] = addr;
	for(i = 0; i < len; i++)
	{
		wdbuf[i+1] = buf[i];
	}
	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 1);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	

    return ret;
}

void AW_Sleep(unsigned int msec)
{
	msleep(msec);
}

static ssize_t AW5306_get_Cali(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_set_Cali(struct device* cd,struct device_attribute *attr, const char *buf, size_t count);
static ssize_t AW5306_get_reg(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_write_reg(struct device* cd,struct device_attribute *attr, const char *buf, size_t count);
static ssize_t AW5306_get_Base(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_Diff(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_adbBase(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_adbDiff(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_FreqScan(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_Set_FreqScan(struct device* cd, struct device_attribute *attr, const char* buf, size_t len);



static DEVICE_ATTR(cali,  S_IRUGO | S_IWUSR, AW5306_get_Cali, AW5306_set_Cali);
static DEVICE_ATTR(readreg,  S_IRUGO | S_IWUSR, AW5306_get_reg, AW5306_write_reg);
static DEVICE_ATTR(base,  S_IRUGO | S_IWUSR, AW5306_get_Base, NULL);
static DEVICE_ATTR(diff, S_IRUGO | S_IWUSR, AW5306_get_Diff, NULL);
static DEVICE_ATTR(adbbase,  S_IRUGO | S_IWUSR, AW5306_get_adbBase, NULL);
static DEVICE_ATTR(adbdiff, S_IRUGO | S_IWUSR, AW5306_get_adbDiff, NULL);
static DEVICE_ATTR(freqscan, S_IRUGO | S_IWUSR, AW5306_get_FreqScan, AW5306_Set_FreqScan);


static ssize_t AW5306_get_Cali(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;
	
	len += snprintf(buf+len, PAGE_SIZE-len,"*****AW5306 Calibrate data*****\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"TXOFFSET:");
	
	for(i=0;i<11;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXOFFSET[i]);
	}
	
	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXOFFSET:");

	for(i=0;i<6;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXOFFSET[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXCAC:");

	for(i=0;i<21;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXCAC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXCAC:");

	for(i=0;i<12;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXCAC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXGAIN:");

	for(i=0;i<21;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXGAIN[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");

	for(i=0;i<AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d ", AW_Cali.SOFTOFFSET[i][j]);
		}
		len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	}
	return len;
	
}

static ssize_t AW5306_set_Cali(struct device* cd,struct device_attribute *attr, const char *buf, size_t count)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	
	unsigned long on_off = simple_strtoul(buf, NULL, 10);

	if(on_off == 1)
	{
		suspend_flag = 1;
		AW_Sleep(50);
		
		TP_Force_Calibration();
		
		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		data->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		add_timer(&data->touch_timer);
	}
	
	return count;
}


static ssize_t AW5306_get_adbBase(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "base: \n");
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]);
		}
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}
	
	return len;
}

static ssize_t AW5306_get_Base(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	*(buf+len) = AWTPCfg.TX_LOCAL;
	len++;
	*(buf+len) = AWTPCfg.RX_LOCAL;
	len++;
	
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			*(buf+len) = (char)(((AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]) & 0xFF00)>>8);
			len++;
			*(buf+len) = (char)((AW_Base.Base[i][j]+AW_Cali.SOFTOFFSET[i][j]) & 0x00FF);
			len++;
		}
	}
	return len;

}

static ssize_t AW5306_get_adbDiff(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "Diff: \n");
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",adbDiff[i][j]);
		}
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}
	
	return len;
}

static ssize_t AW5306_get_Diff(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	ssize_t len = 0;

	*(buf+len) = AWTPCfg.TX_LOCAL;
	len++;
	*(buf+len) = AWTPCfg.RX_LOCAL;
	len++;
	
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			*(buf+len) = (char)((adbDiff[i][j] & 0xFF00)>>8);
			len++;
			*(buf+len) = (char)(adbDiff[i][j] & 0x00FF);
			len++;
		}
	}
	return len;
}

static ssize_t AW5306_get_FreqScan(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i;
	ssize_t len = 0;

	for(i=0;i< 32;i++)
	{
		//*(buf+len) = (char)((AWDeltaData[i] & 0xFF00)>>8);
		//len++;
		//*(buf+len) = (char)(AWDeltaData[i] & 0x00FF);
		//len++;
		len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",AWDeltaData[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	return len;
}

static ssize_t AW5306_Set_FreqScan(struct device* cd, struct device_attribute *attr,
		       const char* buf, size_t len)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	unsigned long Basefreq = simple_strtoul(buf, NULL, 10);

	if(Basefreq < 10)
	{
		suspend_flag = 1;
		AW_Sleep(50);

		FreqScan(Basefreq);

		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		data->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		add_timer(&data->touch_timer);
	}
	return len;
}

static ssize_t AW5306_get_reg(struct device* cd,struct device_attribute *attr, char* buf)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	u8 reg_val[128];
	ssize_t len = 0;
	u8 i;

	suspend_flag = 1;
	
	AW_Sleep(50);
			
	AW_I2C_ReadXByte(reg_val,0,127);

	AW5306_TP_Reinit();
	tp_idlecnt = 0;
	tp_SlowMode = 0;
	suspend_flag = 0;
	data->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
	add_timer(&data->touch_timer);
	
	for(i=0;i<0x7F;i++)
	{
		reg_val[0] = AW_I2C_ReadByte(i);
		len += snprintf(buf+len, PAGE_SIZE-len, "reg%02X = 0x%02X, ", i,reg_val[0]);
	}

	return len;

}

static ssize_t AW5306_write_reg(struct device* cd,struct device_attribute *attr, const char *buf, size_t count)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	int databuf[2];
	
	if(2 == sscanf(buf, "%d %d", &databuf[0], &databuf[1]))
	{ 
		suspend_flag = 1;
		AW_Sleep(50);
		
		AW_I2C_WriteByte((u8)databuf[0],(u8)databuf[1]);

		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
		data->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		add_timer(&data->touch_timer);
	}
	else
	{
		printk("invalid content: '%s', length = %d\n", buf, count);
	}
	return count; 
}

static int AW5306_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	//TS_DBG("%s", __func__);
	
	err = device_create_file(dev, &dev_attr_cali);
	err = device_create_file(dev, &dev_attr_readreg);
	err = device_create_file(dev, &dev_attr_base);
	err = device_create_file(dev, &dev_attr_diff);
	err = device_create_file(dev, &dev_attr_adbbase);
	err = device_create_file(dev, &dev_attr_adbdiff);
	err = device_create_file(dev, &dev_attr_freqscan);
	return err;
}

static void AW5306_ts_release(void)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
#ifdef CONFIG_AW5306_MULTITOUCH	
	#ifdef TOUCH_KEY_SUPPORT
	if(1 == key_tp){
		if(key_val == 1){
			input_report_key(data->input_dev, KEY_MENU, 0);
			input_sync(data->input_dev);  
        }
        else if(key_val == 2){
			input_report_key(data->input_dev, KEY_BACK, 0);
			input_sync(data->input_dev);  
		//	printk("===KEY 2   upupupupupu===++=\n");     
        }
        else if(key_val == 3){
			input_report_key(data->input_dev, KEY_SEARCH, 0);
			input_sync(data->input_dev);  
		//	printk("===KEY 3   upupupupupu===++=\n");     
        }
        else if(key_val == 4){
			input_report_key(data->input_dev, KEY_HOMEPAGE, 0);
			input_sync(data->input_dev);  
		//	printk("===KEY 4   upupupupupu===++=\n");     
        }
        else if(key_val == 5){
			input_report_key(data->input_dev, KEY_VOLUMEDOWN, 0);
			input_sync(data->input_dev);  
		//	printk("===KEY 5   upupupupupu===++=\n");     
        }
        else if(key_val == 6){
			input_report_key(data->input_dev, KEY_VOLUMEUP, 0);
			input_sync(data->input_dev);  
		//	printk("===KEY 6   upupupupupu===++=\n");     
        }
//		input_report_key(data->input_dev, key_val, 0);
		//printk("Release Key = %d\n",key_val);		
		//printk("Release Keyi+++++++++++++++++++++++++++++\n");		
	} else{
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	}
	#else
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	#endif

#else
	input_report_abs(data->input_dev, ABS_PRESSURE, 0);
	input_report_key(data->input_dev, BTN_TOUCH, 0);
#endif
	
	input_sync(data->input_dev);
	return;

}

static int AW5306_read_data(void)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	 int Pevent;
    int i = 0;
	
	AW5306_TouchProcess();
	
	//memset(event, 0, sizeof(struct ts_event));
	event->touch_point = AW5306_GetPointNum();

	for(i=0;i<event->touch_point;i++)
	{
		AW5306_GetPoint(&event->x[i],&event->y[i],&event->touch_ID[i],&Pevent,i);
		swap(event->x[i], event->y[i]);
//		printk("AW5306key%d = %d,%d,%d \n",i,event->x[i],event->y[i],event->touch_ID[i] );
	}
    	
	if (event->touch_point == 0) 
	{
		if(tp_idlecnt <= AWTPCfg.FAST_FRAME*5)
		{
			tp_idlecnt++;
		}
		if(tp_idlecnt > AWTPCfg.FAST_FRAME*5)
		{
			tp_SlowMode = 1;
		}
		
		if (event->pre_point != 0)
		{
		    AW5306_ts_release();
			event->pre_point = 0;
		}
		return 1; 
	}
	else
	{
		tp_SlowMode = 0;
		tp_idlecnt = 0;
    event->pre_point = event->touch_point;
     
#ifdef CONFIG_AW5306_MULTITOUCH
  //  printk("tmp =%d\n",store_pin_num);
  //    printk("touch_point=%d\n",event->touch_point);
	switch (event->touch_point) {
	case 5:
		if(1 == exchange_x_y_flag){
			swap(event->x[4], event->y[4]);
		}
		if(0 == revert_x_flag){
			event->x[4] = SCREEN_MAX_X - event->x[4];    
		}
		if(1 == revert_y_flag){
			event->y[4] = SCREEN_MAX_Y - event->y[4];   
		}
	
	case 4:
		if(1 == exchange_x_y_flag){
			swap(event->x[3], event->y[3]);
		}
		if(0 == revert_x_flag){
			event->x[3] = SCREEN_MAX_X - event->x[3];
		}
		if(1 == revert_y_flag){
			event->y[3] = SCREEN_MAX_Y - event->y[3];
		}
	
	case 3:
		if(1 == exchange_x_y_flag){
			swap(event->x[2], event->y[2]);
		}
		if(0 == revert_x_flag){
			event->x[2] = SCREEN_MAX_X - event->x[2];
		}
		if(1 == revert_y_flag){
			event->y[2] = SCREEN_MAX_Y - event->y[2];
		}
		
	case 2:
		if(1 == exchange_x_y_flag){
			swap(event->x[1], event->y[1]);
		}
		if(0== revert_x_flag){
			event->x[1] = SCREEN_MAX_X - event->x[1];
		}
		if(1 == revert_y_flag){
			event->y[1] = SCREEN_MAX_Y - event->y[1];
		}
		
	case 1:
#ifdef TOUCH_KEY_FOR_ANGDA
		if(event->x[0] < TOUCH_KEY_X_LIMIT)
		{
			if(0 == revert_x_flag){
				event->x[0] = SCREEN_MAX_X - event->x[0];
			}
			if(1 == revert_y_flag){
				event->y[0] = SCREEN_MAX_Y - event->y[0];
			}
			if(1 == exchange_x_y_flag){
				swap(event->x[0], event->y[0]);
			}
		}
#elif defined(TOUCH_KEY_FOR_EVB13)
		if((event->x[0] > TOUCH_KEY_LOWER_X_LIMIT)&&(event->x[0]<TOUCH_KEY_HIGHER_X_LIMIT))
		{
		    //   printk("event->x=%d\n",event->x[0]);
		   
		if(1 == exchange_x_y_flag){
				swap(event->x[0], event->y[0]);
			}
			 //  printk("event->x=%d,y=%d\n",event->x[0],event->y[0]);
		
			if(0 == revert_x_flag){
				event->x[0] = SCREEN_MAX_X - event->x[0];
			}
			if(1 == revert_y_flag){
				event->y[0] = SCREEN_MAX_Y - event->y[0];
			}
			}
			 
#else
            if(1 == exchange_x_y_flag){
			swap(event->x[0], event->y[0]);
		}
	   
		//printk("event->x=%d,y=%d\n",event->x[0],event->y[0]); 
		 
		if(0 == revert_x_flag){
			event->x[0] = SCREEN_MAX_X - event->x[0];
		}
		if(1== revert_y_flag){
			event->y[0] = SCREEN_MAX_Y - event->y[0];
		}
		
		//printk("x=%d\n",event->x[0]);
#endif

		break;
	default:
		return -1;
	}
#else
#endif
	event->pressure = 200;

	dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__,
	event->x[0], event->y[0], event->x[1], event->y[1]);
		

    return 0;
		}
}

#ifdef TOUCH_KEY_LIGHT_SUPPORT
static void AW5306_lighting(void)
{
	if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_light_hdle, 1, "ctp_light")){
		pr_info("AW5306_ts_light: err when operate gpio. \n");
	}    
	msleep(15);
	if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_light_hdle, 0, "ctp_light")){
		pr_info("AW5306_ts_light: err when operate gpio. \n");
	}         

	return;
}
#endif

static void AW5306_report_multitouch(void)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;

#ifdef TOUCH_KEY_SUPPORT
	if(1 == key_tp){
		return;
	}
#endif

	switch(event->touch_point) {
	case 5:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID[4]);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[4]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[4]);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
		input_mt_sync(data->input_dev);
	case 4:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID[3]);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[3]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[3]);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
		input_mt_sync(data->input_dev);
	case 3:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID[2]);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[2]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[2]);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
		input_mt_sync(data->input_dev);
	case 2:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID[1]);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[1]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[1]);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
		input_mt_sync(data->input_dev);
	case 1:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID[0]);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[0]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[0]);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
		input_mt_sync(data->input_dev);
		break;
	default:
		print_point_info("==touch_point default =\n");
		break;
	}
	
	input_sync(data->input_dev);
	dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__,
		event->x[0], event->y[0], event->x[1], event->y[1]);
	return;
}

#ifndef CONFIG_AW5306_MULTITOUCH
static void AW5306_report_singletouch(void)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	
	if (event->touch_point == 1) {
		input_report_abs(data->input_dev, ABS_X, event->x1);
		input_report_abs(data->input_dev, ABS_Y, event->y1);
		input_report_abs(data->input_dev, ABS_PRESSURE, event->pressure);
	}
	input_report_key(data->input_dev, BTN_TOUCH, 1);
	input_sync(data->input_dev);
	dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__,
		event->x1, event->y1, event->x2, event->y2);

	return;
}
#endif

#ifdef TOUCH_KEY_SUPPORT
static void AW5306_report_touchkey(void)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	//printk("x=%d===Y=%d\n",event->x[0],event->y[0]);

#ifdef TOUCH_KEY_FOR_ANGDA
	if((1==event->touch_point)&&(event->x1 > TOUCH_KEY_X_LIMIT)){
		key_tp = 1;
		if(event->x1 < 40){
			key_val = 1;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);  
			print_point_info("===KEY 1====\n");
		}else if(event->y1 < 90){
			key_val = 2;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);     
			print_point_info("===KEY 2 ====\n");
		}else{
			key_val = 3;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);     
			print_point_info("===KEY 3====\n");	
		}
	} else{
		key_tp = 0;
	}
#endif
#ifdef TOUCH_KEY_FOR_EVB13
	if((1==event->touch_point)&&((event->y[0] > 510)&&(event->y[0]<530)))
	{
		if(key_tp != 1)
		{
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_sync(data->input_dev);
		}
		else
		{
			//printk("===KEY touch ++++++++++====++=\n");     
		
			if(event->x[0] < 90){
				key_val = 1;
				input_report_key(data->input_dev, KEY_MENU, 1);
				input_sync(data->input_dev);  
			//	printk("===KEY 1===++=\n");     
			}else if((event->x[0] < 230)&&(event->x[0]>185)){
				key_val = 2;
				input_report_key(data->input_dev, KEY_BACK, 1);
				input_sync(data->input_dev);     
			//	printk("===KEY 2 ====\n");
			}else if((event->x[0] < 355)&&(event->x[0]>305)){
				key_val = 3;
				input_report_key(data->input_dev, KEY_SEARCH, 1);
				input_sync(data->input_dev);     
			//	print_point_info("===KEY 3====\n");
			}else if ((event->x[0] < 497)&&(event->x[0]>445))	{
				key_val = 4;
				input_report_key(data->input_dev, KEY_HOMEPAGE, 1);
				input_sync(data->input_dev);     
			//	print_point_info("===KEY 4====\n");	
			}else if ((event->x[0] < 615)&&(event->x[0]>570))	{
				key_val = 5;
				input_report_key(data->input_dev, KEY_VOLUMEDOWN, 1);
				input_sync(data->input_dev);     
			//	print_point_info("===KEY 5====\n");	
			}else if ((event->x[0] < 750)&&(event->x[0]>705))	{
				key_val = 6;
				input_report_key(data->input_dev, KEY_VOLUMEUP, 1);
				input_sync(data->input_dev);     
			//	print_point_info("===KEY 6====\n");	
			}
		}
		key_tp = 1;
	}
	else
	{
		key_tp = 0;
	}
#endif

#ifdef TOUCH_KEY_LIGHT_SUPPORT
	AW5306_lighting();
#endif
	return;
}
#endif

static void AW5306_report_value(void)
{
#ifdef CONFIG_AW5306_MULTITOUCH
	AW5306_report_multitouch();
#else	/* CONFIG_AW5306_MULTITOUCH*/
	AW5306_report_singletouch();
#endif	/* CONFIG_AW5306_MULTITOUCH*/
#ifdef TOUCH_KEY_SUPPORT
	AW5306_report_touchkey();
#endif
	return;
}	/*end AW5306_report_value*/

static void AW5306_ts_pen_irq_work(struct work_struct *work)
{
	int ret = -1;
        //printk("AW5306____read_data_test11111111\n");	
	if(suspend_flag != 1)
	{

		ret = AW5306_read_data();
	        //printk("AW5306_read_Data_ret = %d\n",ret);
         	if (ret == 0) {
			AW5306_report_value();
		}
	}
	else
	{
			AW5306_Sleep(); 
	}
	//enable_irq(SW_INT_IRQNO_PIO);

}
#ifdef INTMODE
static irqreturn_t AW5306_ts_interrupt(int irq, void *dev_id)
{
	struct AW5306_ts_data *AW5306_ts = dev_id;
		
	print_int_info("==========------AW5306_ts TS Interrupt-----============\n"); 
	if(!ctp_ops.judge_int_occur()){
		print_int_info("==IRQ_EINT21=\n");
		ctp_ops.clear_penirq();
		if (!work_pending(&AW5306_ts->pen_event_work)) 
		{
			print_int_info("Enter work\n");
			queue_work(AW5306_ts->ts_workqueue, &AW5306_ts->pen_event_work);
		}
	}else{
		print_int_info("Other Interrupt\n");
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}
#else
void AW5306_tpd_polling(unsigned long data)
 {
	struct AW5306_ts_data *AW5306_ts = i2c_get_clientdata(this_client);

 	if (!work_pending(&AW5306_ts->pen_event_work)) {
    queue_work(AW5306_ts->ts_workqueue, &AW5306_ts->pen_event_work);
    }
	if(suspend_flag != 1)
	{
	#ifdef AUTO_RUDUCEFRAME
		if(tp_SlowMode)
		{  	
			AW5306_ts->touch_timer.expires = jiffies + HZ/AWTPCfg.SLOW_FRAME;
		}
		else
		{
			AW5306_ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		}
	#else
		AW5306_ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
	#endif
		add_timer(&AW5306_ts->touch_timer);
	}
 }
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND

static void AW5306_ts_suspend(struct early_suspend *handler)
{	
	if(suspend_flag != 1)
	{
		printk("AW5306 SLEEP!!!");
		suspend_flag = 1;
	}  
}

static void AW5306_ts_resume(struct early_suspend *handler)
{
	struct AW5306_ts_data *AW5306_ts = i2c_get_clientdata(this_client);
	
	if(suspend_flag != 0)
	{     
	        msleep(10);
		//AW5306_TP_Reinit();
                //AW5306_Sleep();
                //AW_sleep(100);
		//AW5306_TP_Init();
                AW5306_User_Cfg1();
		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 0;
		suspend_flag = 0;
         	printk("AW5306 WAKE UP!!!");
	        AW5306_read_data();
                AW5306_report_value();
                init_timer(&AW5306_ts->touch_timer);	
                AW5306_ts->touch_timer.expires = jiffies + HZ/AWTPCfg.FAST_FRAME;
		add_timer(&AW5306_ts->touch_timer);
	}
}

#endif  //CONFIG_HAS_EARLYSUSPEND

static int 
AW5306_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct AW5306_ts_data *AW5306_ts;
	struct input_dev *input_dev;
//	struct device *dev;
//	struct i2c_dev *i2c_dev;
	int err = 0;
    u8 reg_value;
#ifdef TOUCH_KEY_SUPPORT
	int i = 0;
#endif
	pr_info("====%s begin=====.  \n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}


	AW5306_ts = kzalloc(sizeof(*AW5306_ts), GFP_KERNEL);
	if (!AW5306_ts)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	this_client = client;
	
#if 1
	//pr_info("AW5306_ts_probe : client->addr = %d. \n", client->addr);
	client->addr =0x38;
	this_client->addr = client->addr;
	printk("client addr2 = %x", client->addr);

	reg_value = AW_I2C_ReadByte(0x01);
	printk(" product id= %d", reg_value);
	if(reg_value != 0xA8)
	{
		client->addr = 0x39;
		//ft_ctp_reset();
		ctp_wakeup(1,TS_WAKEUP_LOW_PERIOD);  
		dev_err(&client->dev, "AW5306_ts_probe: CHIP ID NOT CORRECT\n");
		goto exit_check_functionality_failed;
	}
#endif	

	i2c_set_clientdata(client, AW5306_ts);

#ifdef TOUCH_KEY_LIGHT_SUPPORT
	gpio_light_hdle = gpio_request_ex("ctp_para", "ctp_light");
#endif

	pr_info("==INIT_WORK=\n");
	INIT_WORK(&AW5306_ts->pen_event_work, AW5306_ts_pen_irq_work);
	AW5306_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));

	if (!AW5306_ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}
	
	AW5306_ts->input_dev = input_dev;

#ifdef CONFIG_AW5306_MULTITOUCH
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);	
#ifdef FOR_TSLIB_TEST
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
#endif
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_TRACKING_ID, 0, 4, 0, 0);
#ifdef TOUCH_KEY_SUPPORT
	input_set_capability(input_dev,EV_KEY,KEY_BACK);
	input_set_capability(input_dev,EV_KEY,KEY_MENU);
	input_set_capability(input_dev,EV_KEY,KEY_HOMEPAGE);
	input_set_capability(input_dev,EV_KEY,KEY_SEARCH);
	input_set_capability(input_dev,EV_KEY,KEY_VOLUMEDOWN);
	input_set_capability(input_dev,EV_KEY,KEY_VOLUMEUP);
	set_bit(KEY_MENU, input_dev->keybit);
	key_tp = 0;
	input_dev->evbit[0] = BIT_MASK(EV_KEY);
	for (i = 1; i < TOUCH_KEY_NUMBER; i++)
		set_bit(i, input_dev->keybit);
#endif
#else
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	input_set_abs_params(input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_PRESSURE, 0, PRESS_MAX, 0 , 0);
#endif

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);

	input_dev->name		= CTP_NAME;		//dev_name(&client->dev)
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
		"AW5306_ts_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	pr_info("==register_early_suspend =\n");
	AW5306_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	AW5306_ts->early_suspend.suspend = AW5306_ts_suspend;
	AW5306_ts->early_suspend.resume	= AW5306_ts_resume;
	register_early_suspend(&AW5306_ts->early_suspend);
#endif

#ifdef CONFIG_AW5306_MULTITOUCH
	pr_info("CONFIG_AW5306_MULTITOUCH is defined. \n");
#endif

#ifdef INTMODE
	err = ctp_ops.set_irq_mode("ctp_para", "ctp_int_port", CTP_IRQ_MODE);
	if(0 != err){
		pr_info("%s:ctp_ops.set_irq_mode err. \n", __func__);
		goto exit_set_irq_mode;
	}
	err = request_irq(SW_INT_IRQNO_PIO, AW5306_ts_interrupt, IRQF_TRIGGER_RISING | IRQF_SHARED, "AW5306_ts", AW5306_ts);
   
	if (err < 0) {
		dev_err(&client->dev, "AW5306_ts_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
#else
  
  AW5306_create_sysfs(client);
  
  AW5306_TP_Init();  

	AW5306_ts->touch_timer.function = AW5306_tpd_polling;
	AW5306_ts->touch_timer.data = 0;
	init_timer(&AW5306_ts->touch_timer);
	AW5306_ts->touch_timer.expires = jiffies + HZ*5;
	add_timer(&AW5306_ts->touch_timer);
#endif
    //	i2c_dev = get_free_i2c_dev(client->adapter);	
/*	if (IS_ERR(i2c_dev)){	
		err = PTR_ERR(i2c_dev);		
		return err;	
	}*/
/*	dev = device_create(i2c_dev_class, &client->adapter->dev, MKDEV(I2C_MAJOR,client->adapter->nr), NULL, "aw_i2c_ts%d", client->adapter->nr);	
	if (IS_ERR(dev))	{		
			err = PTR_ERR(dev);		
			return err;	
	}
*/
	pr_info("==%s over =\n", __func__);
	
	return 0;

#ifdef INTMODE
exit_irq_request_failed:
exit_set_irq_mode:
#endif
	cancel_work_sync(&AW5306_ts->pen_event_work);
	destroy_workqueue(AW5306_ts->ts_workqueue);
	//enable_irq(SW_INT_IRQNO_PIO);
exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
	//free_irq(SW_INT_IRQNO_PIO, AW5306_ts);
exit_create_singlethread:
	pr_info("==singlethread error =\n");
	i2c_set_clientdata(client, NULL);
	kfree(AW5306_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int __devexit AW5306_ts_remove(struct i2c_client *client)
{

	struct AW5306_ts_data *AW5306_ts = i2c_get_clientdata(client);
	
	pr_info("==AW5306_ts_remove=\n");
#ifdef INTMODE
	free_irq(SW_INT_IRQNO_PIO, AW5306_ts);
#else
	del_timer(&AW5306_ts->touch_timer);
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&AW5306_ts->early_suspend);
#endif
	input_unregister_device(AW5306_ts->input_dev);
	input_free_device(AW5306_ts->input_dev);
	cancel_work_sync(&AW5306_ts->pen_event_work);
	destroy_workqueue(AW5306_ts->ts_workqueue);
	kfree(AW5306_ts);
    
	i2c_set_clientdata(client, NULL);
	
//lkj	ctp_ops.free_platform_resource();

	return 0;

}

static const struct i2c_device_id AW5306_ts_id[] = {
	{ CTP_NAME, 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, AW5306_ts_id);

static struct i2c_driver AW5306_ts_driver = {
	.class = I2C_CLASS_HWMON,
	.probe		= AW5306_ts_probe,
	.remove		= __devexit_p(AW5306_ts_remove),
	.id_table	= AW5306_ts_id,
	.driver	= {
		.name	= CTP_NAME,
		.owner	= THIS_MODULE,
	},
	.address_list	= normal_i2c,
};

static int __init AW5306_ts_init(void)
{ 
	int ret = -1;
//	int err = -1;
    
//lkj =====begin============    
	printk("AW5306_ts_init:****************************************************************\n");

	if (input_fetch_sysconfig_para(&(config_info.input_type))){
		printk("%s: ctp_fetch_sysconfig_para err.\n", __func__);
		return 0;
	}else{
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
                printk("%s:read config fail!\n", __func__);
                return ret;
        }
        
	ctp_wakeup(1,TS_WAKEUP_LOW_PERIOD);  
		
	AW5306_ts_driver.detect = ctp_detect;
//==================end===================

#if 0
	pr_info("===========================%s=====================\n", __func__);

	if (ctp_ops.fetch_sysconfig_para)
	{
		if(ctp_ops.fetch_sysconfig_para()){
			pr_info("%s: err.\n", __func__);
			return -1;
		}
	}
	pr_info("%s: after fetch_sysconfig_para:  normal_i2c: 0x%hx. normal_i2c[1]: 0x%hx \n", \
	__func__, u_i2c_addr.normal_i2c[0], u_i2c_addr.normal_i2c[1]);

	err = ctp_ops.init_platform_resource();
	if(0 != err){
	    pr_info("%s:ctp_ops.init_platform_resource err. \n", __func__);    
	}

	//reset
	ctp_ops.ts_reset();
	//wakeup
	ctp_ops.ts_wakeup();  
	
	AW5306_ts_driver.detect = ctp_ops.ts_detect;

/*	if(ret) {	
		pr_info(KERN_ERR "%s:register chrdev failed\n",__FILE__);	
		return ret;
	}
*/	
#endif

	i2c_dev_class = class_create(THIS_MODULE,"aw_i2c_dev");
	if (IS_ERR(i2c_dev_class)) {		
		ret = PTR_ERR(i2c_dev_class);		
		class_destroy(i2c_dev_class);	
	}

	ret = i2c_add_driver(&AW5306_ts_driver);

	return ret;
}

static void __exit AW5306_ts_exit(void)
{
	pr_info("==AW5306_ts_exit==\n");
	i2c_del_driver(&AW5306_ts_driver);

	input_free_platform_resource(&(config_info.input_type));
}

late_initcall(AW5306_ts_init);
module_exit(AW5306_ts_exit);

MODULE_AUTHOR("<whj@AWINIC.com>");
MODULE_DESCRIPTION("AWINIC AW5306 TouchScreen driver");
MODULE_LICENSE("GPL");

