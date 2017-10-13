/* 
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x TouchScreen driver. 
 *
 * Copyright (c) 2010  Focal tech Ltd.
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
 *
 *	note: only support mulititouch	Wenfs 2010-10-01
 *  for this touchscreen to work, it's slave addr must be set to 0x7e | 0x70
 */
#include <linux/i2c.h>
#include <linux/input.h>
#include "ft5x_ts.h"
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
#include <linux/init-input.h>
#include <linux/gpio.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/irqs.h>
#include <mach/hardware.h>


#define CONFIG_SUPPORT_FTS_CTP_UPG

#define FOR_TSLIB_TEST
//#define TOUCH_KEY_SUPPORT
#ifdef TOUCH_KEY_SUPPORT
#define TOUCH_KEY_FOR_EVB13
//#define TOUCH_KEY_FOR_ANGDA
#ifdef TOUCH_KEY_FOR_ANGDA
#define TOUCH_KEY_X_LIMIT	        (60000)
#define TOUCH_KEY_NUMBER	        (4)
#endif
#ifdef TOUCH_KEY_FOR_EVB13
#define TOUCH_KEY_LOWER_X_LIMIT	        (848)
#define TOUCH_KEY_HIGHER_X_LIMIT	(852)
#define TOUCH_KEY_NUMBER	        (5)
#endif
#endif


//FT5X02_CONFIG
#define FT5X02_CONFIG_NAME "fttpconfig_5x02public.ini"
extern int ft5x02_Init_IC_Param(struct i2c_client * client);
extern int ft5x02_get_ic_param(struct i2c_client * client);
extern int ft5x02_Get_Param_From_Ini(char *config_name);
        
struct i2c_dev{
        struct list_head list;	
        struct i2c_adapter *adap;
        struct device *dev;
};

static struct class *i2c_dev_class;
static LIST_HEAD (i2c_dev_list);
static DEFINE_SPINLOCK(i2c_dev_list_lock);


struct Upgrade_Info {
	u16 delay_aa;		/*delay of write FT_UPGRADE_AA */
	u16 delay_55;		/*delay of write FT_UPGRADE_55 */
	u8 upgrade_id_1;	/*upgrade id 1 */
	u8 upgrade_id_2;	/*upgrade id 2 */
	u16 delay_readid;	/*delay of read id */
};

#define FT5X_NAME	"ft5x_ts"

static struct i2c_client *this_client;

#ifdef TOUCH_KEY_SUPPORT
static int key_tp  = 0;
static int key_val = 0;
#endif

/*********************************************************************************************/
#define CTP_IRQ_NUMBER                  (config_info.int_number)
#define CTP_IRQ_MODE			(IRQF_TRIGGER_FALLING)
#define SCREEN_MAX_X			(screen_max_x)
#define SCREEN_MAX_Y			(screen_max_y)
#define CTP_NAME			 FT5X_NAME
#define PRESS_MAX			(255)

static int screen_max_x = 0;
static int screen_max_y = 0;
static int revert_x_flag = 0;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;
static __u32 twi_id = 0;

static struct ctp_config_info config_info = {
	.input_type = CTP_TYPE,
	.name = NULL,
	.int_number = 0,
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
module_param_named(debug_mask,debug_mask,int,S_IRUGO | S_IWUSR | S_IWGRP);
/*********************************************************************************************/
/*------------------------------------------------------------------------------------------*/        
/* Addresses to scan */
static const unsigned short normal_i2c[2] = {0x38,I2C_CLIENT_END};
static const int chip_id_value[] = {0x55,0x06,0x08,0x02,0xa3};
static int chip_id = 0;

static void ft5x_resume_events(struct work_struct *work);
struct workqueue_struct *ft5x_resume_wq;
static DECLARE_WORK(ft5x_resume_work, ft5x_resume_events);

static void ft5x_init_events(struct work_struct *work);
struct workqueue_struct *ft5x_wq;
static DECLARE_WORK(ft5x_init_work, ft5x_init_events);
/*------------------------------------------------------------------------------------------*/ 

static int ctp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
        int ret = 0; 

        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;

	if(twi_id == adapter->nr){

	        ret = i2c_smbus_read_byte_data(client,0xA3);
		if(ret == -70) {
			msleep(10);
			ret = i2c_smbus_read_byte_data(client,0xA3);
		}
		
        dprintk(DEBUG_INIT,"addr:0x%x,chip_id_value:0x%x\n",client->addr,ret);
        if(ret < 0){
            printk("%s:I2C connection might be something wrong ! \n",__func__);
        	return -ENODEV;
		} else {

            strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
			chip_id = ret;			
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

int fts_ctpm_fw_upgrade_with_i_file(void);

static struct i2c_dev *i2c_dev_get_by_minor(unsigned index)
{
	struct i2c_dev *i2c_dev;
	spin_lock(&i2c_dev_list_lock);
	
	list_for_each_entry(i2c_dev,&i2c_dev_list,list){
		dprintk(DEBUG_OTHERS_INFO,"--line = %d ,i2c_dev->adapt->nr = %d,index = %d.\n",\
		        __LINE__,i2c_dev->adap->nr,index);
		if(i2c_dev->adap->nr == index){
		     goto found;
		}
	}
	i2c_dev = NULL;
	
found: 
	spin_unlock(&i2c_dev_list_lock);
	
	return i2c_dev ;
}

static struct i2c_dev *get_free_i2c_dev(struct i2c_adapter *adap) 
{
	struct i2c_dev *i2c_dev;

	if (adap->nr >= I2C_MINORS){
		dprintk(DEBUG_OTHERS_INFO,"i2c-dev:out of device minors (%d) \n",adap->nr);
		return ERR_PTR (-ENODEV);
	}

	i2c_dev = kzalloc(sizeof(*i2c_dev), GFP_KERNEL);
	if (!i2c_dev){
		return ERR_PTR(-ENOMEM);
	}
	i2c_dev->adap = adap;

	spin_lock(&i2c_dev_list_lock);
	list_add_tail(&i2c_dev->list, &i2c_dev_list);
	spin_unlock(&i2c_dev_list_lock);
	
	return i2c_dev;
}


static int ft5x_i2c_rxdata(char *rxdata, int length);

struct ts_event {
	u16	x1;
	u16	y1;
	u16	x2;
	u16	y2;
	u16	x3;
	u16	y3;
	u16	x4;
	u16	y4;
	u16	x5;
	u16	y5;
	u16	pressure;
	s16     touch_ID1;
	s16     touch_ID2;
	s16     touch_ID3;
	s16     touch_ID4;
	s16     touch_ID5;
        u8      touch_point;
};

struct ft5x_ts_data {
	struct input_dev	*input_dev;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct workqueue_struct *ts_workqueue;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
    bool is_suspended;
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

#define I2C_CTPM_ADDRESS        (0x70>>1)

static void delay_ms(FTS_WORD  w_ms)
{
	//platform related, please implement this function
	msleep( w_ms );
}

void delay_qt_ms(unsigned long  w_ms)
{
	unsigned long i;
	unsigned long j;

	for (i = 0; i < w_ms; i++)
	{
		for (j = 0; j < 1000; j++)
		{
			 udelay(1);
		}
	}
}
/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
int i2c_read_interface(u8 bt_ctpm_addr, u8* pbt_buf, u16 dw_lenth)
{
	int ret;

	ret = i2c_master_recv(this_client, pbt_buf, dw_lenth);

	if(ret != dw_lenth){
		printk("ret = %d. \n", ret);
		printk("i2c_read_interface error\n");
		return FTS_FALSE;
	}

	return FTS_TRUE;
}

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
int i2c_write_interface(u8 bt_ctpm_addr, u8* pbt_buf, u16 dw_lenth)
{
	int ret;
	ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
	if(ret != dw_lenth){
		printk("i2c_write_interface error\n");
		return FTS_FALSE;
	}

	return FTS_TRUE;
}


/***************************************************************************************/

/*
[function]: 
    read out the register value.
[parameters]:
    e_reg_name[in]    :register name;
    pbt_buf[out]    :the returned register value;
    bt_len[in]        :length of pbt_buf, should be set to 2;        
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
u8 fts_register_read(u8 e_reg_name, u8* pbt_buf, u8 bt_len)
{
	u8 read_cmd[3]= {0};
	u8 cmd_len     = 0;

	read_cmd[0] = e_reg_name;
	cmd_len = 1;    

	/*call the write callback function*/
	//    if(!i2c_write_interface(I2C_CTPM_ADDRESS, &read_cmd, cmd_len))
	//    {
	//        return FTS_FALSE;
	//    }


	if(!i2c_write_interface(I2C_CTPM_ADDRESS, read_cmd, cmd_len))	{//change by zhengdixu
		return FTS_FALSE;
	}

	/*call the read callback function to get the register value*/        
	if(!i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len)){
		return FTS_FALSE;
	}
	return FTS_TRUE;
}

/*
[function]: 
    write a value to register.
[parameters]:
    e_reg_name[in]    :register name;
    pbt_buf[in]        :the returned register value;
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
int fts_register_write(u8 e_reg_name, u8 bt_value)
{
	FTS_BYTE write_cmd[2] = {0};

	write_cmd[0] = e_reg_name;
	write_cmd[1] = bt_value;

	/*call the write callback function*/
	//return i2c_write_interface(I2C_CTPM_ADDRESS, &write_cmd, 2);
	return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, 2); //change by zhengdixu
}

/*
[function]: 
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;    
    btPara2[in]    :parameter 2;    
    btPara3[in]    :parameter 3;    
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
int cmd_write(u8 btcmd,u8 btPara1,u8 btPara2,u8 btPara3,u8 num)
{
	FTS_BYTE write_cmd[4] = {0};

	write_cmd[0] = btcmd;
	write_cmd[1] = btPara1;
	write_cmd[2] = btPara2;
	write_cmd[3] = btPara3;
	//return i2c_write_interface(I2C_CTPM_ADDRESS, &write_cmd, num);
	return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);//change by zhengdixu
}

/*
[function]: 
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
int byte_write(u8* pbt_buf, u16 dw_len)
{
	return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]: 
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
int byte_read(u8* pbt_buf, u8 bt_len)
{
	return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
	//ft5x_i2c_rxdata
}


/*
[function]: 
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);    
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/


#define    FTS_PACKET_LENGTH       128 //2//4//8//16//32//64//128//256

static unsigned char CTPM_FW[]=
{
        #include "ft_app.i"
};
unsigned char fts_ctpm_get_i_file_ver(void)
{
        unsigned int ui_sz;
        ui_sz = sizeof(CTPM_FW);
        if (ui_sz > 2){
                return CTPM_FW[ui_sz - 2];
        }else{
                //TBD, error handling?
                return 0xff; //default value
        }
}

/*
*get upgrade information depend on the ic type
*/
static void fts_get_upgrade_info(struct Upgrade_Info *upgrade_info)
{
	switch (chip_id) {
	case 0x55:    //IC_FT5X06:
		upgrade_info->delay_55 = FT5X06_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X06_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X06_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X06_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X06_UPGRADE_READID_DELAY;
		break;
	case 0x08:    //IC_FT5606或者IC_FT5506
		upgrade_info->delay_55 = FT5606_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5606_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5606_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5606_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5606_UPGRADE_READID_DELAY;
		break;
	case 0x00:    //IC FT5316
	case 0x0a:    //IC FT5316
		upgrade_info->delay_55 = FT5316_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5316_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5316_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5316_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5316_UPGRADE_READID_DELAY;
		break;
	default:
		break;
	}
}

E_UPGRADE_ERR_TYPE  ft5x06_ctpm_fw_upgrade(u8* pbt_buf, u16 dw_lenth)
{
        u8 reg_val[2] = {0};
        FTS_BOOL i_ret = 0;
        u16 i = 0;
        
        
        u16  packet_number;
        u16  j;
        u16  temp;
        u16  lenght;
        u8  packet_buf[FTS_PACKET_LENGTH + 6];
        //u8  auc_i2c_write_buf[10];
        u8 bt_ecc;

        struct  Upgrade_Info upgradeinfo = {0, 0, 0, 0 , 0};

	
	fts_get_upgrade_info(&upgradeinfo);

        /*********Step 1:Reset  CTPM *****/
        /*write 0xaa to register 0xfc*/
        //delay_ms(100);//最新的源码去掉延时
        fts_register_write(0xfc,0xaa);
        delay_ms(upgradeinfo.delay_aa);

        /*write 0x55 to register 0xfc*/
        fts_register_write(0xfc,0x55);
        printk("Step 1: Reset CTPM test\n");
        delay_ms(upgradeinfo.delay_55);

        /*********Step 2:Enter upgrade mode *****/
        //auc_i2c_write_buf[0] = 0x55;
        //auc_i2c_write_buf[1] = 0xaa;
        i = 0;
        do{
                i++;
                //i_ret = i2c_write_interface(I2C_CTPM_ADDRESS, auc_i2c_write_buf, 2);
		cmd_write(0x55,0xaa,0x00,0x00,2);
                printk("Step 2: Enter update mode. \n");
                delay_ms(5);
        }while((FTS_FALSE == i_ret) && i<5);

        /*********Step 3:check READ-ID***********************/
        /*send the opration head*/
	msleep(upgradeinfo.delay_readid);
	cmd_write(0x90,0x00,0x00,0x00,4);
	byte_read(reg_val,2);
	if (reg_val[0] == upgradeinfo.upgrade_id_1&& reg_val[1] == upgradeinfo.upgrade_id_2) {
		printk("Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
	} 
	else {
		printk("Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
		return ERR_READID;
	}
	cmd_write(0xcd,0x00,0x00,0x00,1);
	byte_read(reg_val,1);

        /*Step 4:erase app and panel paramenter area*/
        cmd_write(0x61,0x00,0x00,0x00,1);
        msleep(2000);
	cmd_write(0x63,0x00,0x00,0x00,1);
        msleep(100);
        printk("Step 4: erase. \n");
        
        /*********Step 5:write firmware(FW) to ctpm flash*********/
        bt_ecc = 0;
        printk("Step 5: start upgrade. \n");
        dw_lenth = dw_lenth - 8;
        packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
        packet_buf[0] = 0xbf;
        packet_buf[1] = 0x00;
        for (j=0;j<packet_number;j++){
                temp = j * FTS_PACKET_LENGTH;
                packet_buf[2] = (FTS_BYTE)(temp>>8);
                packet_buf[3] = (FTS_BYTE)temp;
                lenght = FTS_PACKET_LENGTH;
                packet_buf[4] = (FTS_BYTE)(lenght>>8);
                packet_buf[5] = (FTS_BYTE)lenght;
        
                for (i=0;i<FTS_PACKET_LENGTH;i++){
                        packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
                        bt_ecc ^= packet_buf[6+i];
                }
        
                byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
                //delay_ms(FTS_PACKET_LENGTH/6 + 1);
		msleep(FTS_PACKET_LENGTH/6 + 1);
                if ((j * FTS_PACKET_LENGTH % 1024) == 0){
                        printk("upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
                }
        }

        if ((dw_lenth) % FTS_PACKET_LENGTH > 0){
                temp = packet_number * FTS_PACKET_LENGTH;
                packet_buf[2] = (FTS_BYTE)(temp>>8);
                packet_buf[3] = (FTS_BYTE)temp;
        
                temp = (dw_lenth) % FTS_PACKET_LENGTH;
                packet_buf[4] = (FTS_BYTE)(temp>>8);
                packet_buf[5] = (FTS_BYTE)temp;
        
                for (i=0;i<temp;i++){
                        packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
                        bt_ecc ^= packet_buf[6+i];
                }
        
                byte_write(&packet_buf[0],temp+6);    
                //delay_ms(20);
		msleep(20);
        }

        //send the last six byte
        for (i = 0; i<6; i++){
                temp = 0x6ffa + i;
                packet_buf[2] = (FTS_BYTE)(temp>>8);
                packet_buf[3] = (FTS_BYTE)temp;
                temp =1;
                packet_buf[4] = (FTS_BYTE)(temp>>8);
                packet_buf[5] = (FTS_BYTE)temp;
                packet_buf[6] = pbt_buf[ dw_lenth + i]; 
                bt_ecc ^= packet_buf[6];
        
                byte_write(&packet_buf[0],7);  
                //delay_ms(20);
		msleep(20);
        }

        /*********Step 6: read out checksum***********************/
        /*send the opration head*/
        //cmd_write(0xcc,0x00,0x00,0x00,1);//把0xcc当作寄存器地址，去读出一个字节
        // byte_read(reg_val,1);//change by zhengdixu

	fts_register_read(0xcc, reg_val,1);	
        printk("Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
        if(reg_val[0] != bt_ecc){
                //cmd_write(0x07,0x00,0x00,0x00,1);
		printk("ecc error! \n");
		return ERR_ECC;
        }

        /*********Step 7: reset the new FW***********************/
        cmd_write(0x07,0x00,0x00,0x00,1);
        msleep(300);
        return ERR_OK;
}

E_UPGRADE_ERR_TYPE  ft5x02_ctpm_fw_upgrade(u8* pbt_buf, u32 dw_lenth)
{
	
	u8 reg_val[2] = {0};
	u32 i = 0;

	u32  packet_number;
	u32  j;
	u32  temp;
	u32  lenght;
	u8	packet_buf[FTS_PACKET_LENGTH + 6];
	//u8	auc_i2c_write_buf[10];
	u8	bt_ecc;

	//struct timeval begin_tv, end_tv;
	//do_gettimeofday(&begin_tv);

	for (i=0; i<16; i++) {
		/*********Step 1:Reset	CTPM *****/
		/*write 0xaa to register 0xfc*/
		fts_register_write(0xfc,0xaa);
		msleep(30);
		/*write 0x55 to register 0xfc*/
		fts_register_write(0xfc,0x55);
		//delay_qt_ms(18);
		delay_qt_ms(25);
		/*********Step 2:Enter upgrade mode *****/
		#if 0
		//auc_i2c_write_buf[0] = 0x55;
		//auc_i2c_write_buf[1] = 0xaa;
		do
		{
			i ++;
			//i_ret = ft5x02_i2c_Write(client, auc_i2c_write_buf, 2);
			//i_ret = i2c_write_interface(I2C_CTPM_ADDRESS, auc_i2c_write_buf, 2);
			cmd_write(0x55,0xaa,0x00,0x00,2);
			delay_qt_ms(5);
		}while(i_ret <= 0 && i < 5 );
		#else
		//auc_i2c_write_buf[0] = 0x55;
		//ft5x02_i2c_Write(client, auc_i2c_write_buf, 1);
		cmd_write(0x55,0x00,0x00,0x00,1);
		delay_qt_ms(1);
		//auc_i2c_write_buf[0] = 0xaa;
		//ft5x02_i2c_Write(client, auc_i2c_write_buf, 1);
		cmd_write(0xaa,0x00,0x00,0x00,1);
		#endif

		/*********Step 3:check READ-ID***********************/	 
		delay_qt_ms(1);
	
		//ft5x02_upgrade_send_head(client);
		cmd_write(0xFA,0xFA,0x00,0x00,2);//ft5x02_upgrade_send_head
		//auc_i2c_write_buf[0] = 0x90;
		//auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;
		//ft5x02_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
		cmd_write(0x90,0x00,0x00,0x00,4);
		byte_read(reg_val,2);

		if (reg_val[0] == 0x79
			&& reg_val[1] == 0x02) {
			//dev_dbg(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
			printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
			break;
		} else {
			printk("[FTS] Step 3 ERROR: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
			//delay_qt_ms(1);
		}
	}
	if (i >= 6)
		return ERR_READID;
	/********Step 4:enable write function*/
	//ft5x02_upgrade_send_head(client);
	cmd_write(0xFA,0xFA,0x00,0x00,2);//ft5x02_upgrade_send_head
	//auc_i2c_write_buf[0] = 0x06;
	//ft5x02_i2c_Write(client, auc_i2c_write_buf, 1);
	cmd_write(0x06,0x00,0x00,0x00,1);

	/*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;
	
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;

	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;
	for (j=0; j<packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8)(lenght>>8);
		packet_buf[5] = (u8)lenght;
		if(temp>=0x4c00 && temp <(0x4c00+512))
			continue;

		for (i=0; i<FTS_PACKET_LENGTH; i++) {
			packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
			bt_ecc ^= packet_buf[6+i];
		}
		//ft5x02_upgrade_send_head(client);
		cmd_write(0xFA,0xFA,0x00,0x00,2);//ft5x02_upgrade_send_head
		//ft5x02_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH+6);
		byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
		delay_qt_ms(2);
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;

		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8)(temp>>8);
		packet_buf[5] = (u8)temp;

		for (i=0; i<temp; i++) {
			packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
			bt_ecc ^= packet_buf[6+i];
		}
		//ft5x02_upgrade_send_head(client);
		cmd_write(0xFA,0xFA,0x00,0x00,2);//ft5x02_upgrade_send_head
		//ft5x02_i2c_Write(client, packet_buf, temp+6);
		byte_write(&packet_buf[0],temp + 6);
		delay_qt_ms(2);
	}

	/********Disable write function*/
	//ft5x02_upgrade_send_head(client);
	cmd_write(0xFA,0xFA,0x00,0x00,2);//ft5x02_upgrade_send_head
	//auc_i2c_write_buf[0] = 0x04;
	//ft5x02_i2c_Write(client, auc_i2c_write_buf, 1);
	cmd_write(0x04,0x00,0x00,0x00,1);

	delay_qt_ms(1);
	/*********Step 6: read out checksum***********************/
	//ft5x02_upgrade_send_head(client);
	cmd_write(0xFA,0xFA,0x00,0x00,2);//ft5x02_upgrade_send_head
	//auc_i2c_write_buf[0] = 0xcc;
	//ft5x02_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1); 
	cmd_write(0xcc,0x00,0x00,0x00,1);
	byte_read(reg_val,1);

	if (reg_val[0] != bt_ecc) {
		printk("[FTS]--ecc error! FW=%02x bt_ecc=%02x\n", reg_val[0], bt_ecc);
		//return -EIO;
		return ERR_READID;
	}

	/*********Step 7: reset the new FW***********************/
	//ft5x02_upgrade_send_head(client);
	cmd_write(0xFA,0xFA,0x00,0x00,2);//ft5x02_upgrade_send_head
	//auc_i2c_write_buf[0] = 0x07;
	//ft5x02_i2c_Write(client, auc_i2c_write_buf, 1);
	cmd_write(0x07,0x00,0x00,0x00,1);
	msleep(200);  /*make sure CTP startup normally*/
	//DBG("-------upgrade successful-----\n");

	//do_gettimeofday(&end_tv);
	//DBG("cost time=%lu.%lu\n", end_tv.tv_sec-begin_tv.tv_sec, 
	//		end_tv.tv_usec-begin_tv.tv_usec);	
	return ERR_OK;
}

int fts_ctpm_auto_clb(void)
{
        unsigned char uc_temp;
        unsigned char i ;

        printk("[FTS] start auto CLB.\n");
        msleep(200);
        fts_register_write(0, 0x40);  
        //delay_ms(100);                       //make sure already enter factory mode
	msleep(100);
        fts_register_write(2, 0x4);               //write command to start calibration
        //delay_ms(300);
	msleep(300);
        for(i=0;i<100;i++){
                fts_register_read(0,&uc_temp,1);
                if (((uc_temp&0x70)>>4) == 0x0){    //return to normal mode, calibration finish
                        break;
                }
                //delay_ms(200);
		msleep(200);
                printk("[FTS] waiting calibration %d\n",i);
        }
        
        printk("[FTS] calibration OK.\n");
        
        msleep(300);
        fts_register_write(0, 0x40);          //goto factory mode
        delay_ms(100);                       //make sure already enter factory mode
        fts_register_write(2, 0x5);          //store CLB result
        delay_ms(300);
        fts_register_write(0, 0x0);          //return to normal mode 
        msleep(300);
        printk("[FTS] store CLB result OK.\n");
        return 0;
}
void getVerNo(u8* buf, int len)
{
	u8 start_reg=0x0;
	int ret = -1;
	//int status = 0;
	int i = 0;
	start_reg = 0xa6;

#if 0
	printk("read 0xa6 one time. \n");
	if(FTS_FALSE == fts_register_read(0xa6, buf, len)){
                return ;
	}
	
	for (i=0; i< len; i++) {
		printk("=========buf[%d] = 0x%x \n", i, buf[i]);
	}
	
	printk("read 0xa8. \n");
	if(FTS_FALSE == fts_register_read(0xa8, buf, len)){
                return ;
	}
	for (i=0; i< len; i++) {
		printk("=========buf[%d] = 0x%x \n", i, buf[i]);
	}

	ft5x_i2c_rxdata(buf, len);

        for (i=0; i< len; i++) {
                printk("=========buf[%d] = 0x%x \n", i, buf[i]);
        }

        byte_read(buf, len);
        for (i=0; i< len; i++) {
                printk("=========buf[%d] = 0x%x \n", i, buf[i]);
        }
          
#endif

	ret =fts_register_read(0xa6, buf, len);
	//et = ft5406_read_regs(ft5x0x_ts_data_test->client,start_reg, buf, 2);
	if (ret < 0) {
		printk("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		return;
	}
	for (i=0; i<2; i++) {
		printk("=========buf[%d] = 0x%x \n", i, buf[i]);
	}
	return;
}

int fts_ctpm_fw_upgrade_with_i_file(void)
{
	FTS_BYTE*     pbt_buf = FTS_NULL;
	int i_ret = 0;
	unsigned char a;
	unsigned char b;
#define BUFFER_LEN (2)            //len == 2 
	unsigned char buf[BUFFER_LEN] = {0};
   
	//=========FW upgrade========================*/
	printk("%s. \n", __func__);

	pbt_buf = CTPM_FW;
	//msleep(200);
        // cmd_write(0x07,0x00,0x00,0x00,1);
	msleep(100);
	getVerNo(buf, BUFFER_LEN);
	a = buf[0];
	b = fts_ctpm_get_i_file_ver();
	printk("a == %hu,  b== %hu \n",a, b);
	/*
	 * when the firmware in touch panel maybe corrupted,
	 * or the firmware in host flash is new, need upgrade
	 */
	if ( 0xa6 == a || a != b ){
		/*call the upgrade function*/
		if(chip_id == 0x55 || chip_id == 0x08 || chip_id == 0x00 || chip_id == 0x0a){
			i_ret =  ft5x06_ctpm_fw_upgrade(&pbt_buf[0],sizeof(CTPM_FW));
			if (i_ret != 0){
				printk("[FTS] upgrade failed i_ret = %d.\n", i_ret);
			} 
			else {
				printk("[FTS] upgrade successfully.\n");
#ifdef AUTO_CLB
				fts_ctpm_auto_clb();  //start auto CLB
#endif
			}
		}
	}	
	return i_ret;
	
}

unsigned char fts_ctpm_get_upg_ver(void)
{
	unsigned int ui_sz;
	ui_sz = sizeof(CTPM_FW);
	if (ui_sz > 2){
		return CTPM_FW[0];
	}
	else{
		return 0xff; //default value
	}
}

static int ft5x_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		printk("msg %s i2c read error: %d\n", __func__, ret);
	
	return ret;
}

static int ft5x_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

   	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}

static int ft5x_set_reg(u8 addr, u8 para)
{
	u8 buf[3];
	int ret = -1;

	buf[0] = addr;
	buf[1] = para;
	ret = ft5x_i2c_txdata(buf, 2);
	if (ret < 0) {
		pr_err("write reg failed! %#x ret: %d", buf[0], ret);
		return -1;
	}

	return 0;
}

static void ft5x_ts_release(void)
{
	struct ft5x_ts_data *data = i2c_get_clientdata(this_client);
#ifdef CONFIG_FT5X0X_MULTITOUCH	
#ifdef TOUCH_KEY_SUPPORT
	if(1 == key_tp){
		input_report_key(data->input_dev, key_val, 0);
		dprintk(DEBUG_KEY_INFO,"Release Key = %d\n",key_val);		
	} else{
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	}
#else
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 0);
#endif

#else
	input_report_abs(data->input_dev, ABS_PRESSURE, 0);
	input_report_key(data->input_dev, BTN_TOUCH, 0);
#endif
	
	input_sync(data->input_dev);
	return;

}

static int ft5x_read_data(void)
{
	struct ft5x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	unsigned char buf[32]={0};
	int ret = -1;
        
#ifdef CONFIG_FT5X0X_MULTITOUCH
	ret = ft5x_i2c_rxdata(buf, 31);
#else
	ret = ft5x_i2c_rxdata(buf, 31);
#endif
	if (ret < 0) {
		dprintk(DEBUG_X_Y_INFO,"%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		return ret;
	}

	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = buf[2] & 0x07;// 000 0111
	dprintk(DEBUG_X_Y_INFO,"touch point = %d\n",event->touch_point);

	if (event->touch_point == 0) {
		ft5x_ts_release();
		return 1; 
	}

	switch (event->touch_point) {
	case 5:
		event->x5 = (s16)(buf[0x1b] & 0x0F)<<8 | (s16)buf[0x1c];
		event->y5 = (s16)(buf[0x1d] & 0x0F)<<8 | (s16)buf[0x1e];
		dprintk(DEBUG_X_Y_INFO,"source data:event->x5 = %d, event->y5 = %d. \n", event->x5, event->y5);
		if(1 == exchange_x_y_flag){
			swap(event->x5, event->y5);
		}
		if(1 == revert_x_flag){
			event->x5 = SCREEN_MAX_X - event->x5;
		}
		if(1 == revert_y_flag){
			event->y5 = SCREEN_MAX_Y - event->y5;
		}
		event->touch_ID5=(s16)(buf[0x1d] & 0xF0)>>4;
		
		dprintk(DEBUG_X_Y_INFO,"touch id : %d. \n",event->touch_ID5);
	case 4:
		event->x4 = (s16)(buf[0x15] & 0x0F)<<8 | (s16)buf[0x16];
		event->y4 = (s16)(buf[0x17] & 0x0F)<<8 | (s16)buf[0x18];
		dprintk(DEBUG_X_Y_INFO,"source data:event->x4 = %d, event->y4 = %d. \n", event->x4, event->y4);
		if(1 == exchange_x_y_flag){
			swap(event->x4, event->y4);
		}
		if(1 == revert_x_flag){
			event->x4 = SCREEN_MAX_X - event->x4;
		}
		if(1 == revert_y_flag){
			event->y4 = SCREEN_MAX_Y - event->y4;
		}	
		event->touch_ID4=(s16)(buf[0x17] & 0xF0)>>4;
		
		dprintk(DEBUG_X_Y_INFO,"touch id : %d. \n",event->touch_ID4);
	case 3:
		event->x3 = (s16)(buf[0x0f] & 0x0F)<<8 | (s16)buf[0x10];
		event->y3 = (s16)(buf[0x11] & 0x0F)<<8 | (s16)buf[0x12];
		dprintk(DEBUG_X_Y_INFO,"source data:event->x3 = %d, event->y3 = %d. \n", event->x3, event->y3);
		if(1 == exchange_x_y_flag){
			swap(event->x3, event->y3);
		}
		if(1 == revert_x_flag){
			event->x3 = SCREEN_MAX_X - event->x3;
		}
		if(1 == revert_y_flag){
			event->y3 = SCREEN_MAX_Y - event->y3;
		}
		event->touch_ID3=(s16)(buf[0x11] & 0xF0)>>4;
		dprintk(DEBUG_X_Y_INFO,"touch id : %d. \n",event->touch_ID3);
	case 2:
		event->x2 = (s16)(buf[9] & 0x0F)<<8 | (s16)buf[10];
		event->y2 = (s16)(buf[11] & 0x0F)<<8 | (s16)buf[12];
		dprintk(DEBUG_X_Y_INFO,"source data:event->x2 = %d, event->y2 = %d. \n", event->x2, event->y2);
		if(1 == exchange_x_y_flag){
			swap(event->x2, event->y2);
		}
		if(1 == revert_x_flag){
			event->x2 = SCREEN_MAX_X - event->x2;
		}
		if(1 == revert_y_flag){
			event->y2 = SCREEN_MAX_Y - event->y2;
		}
		event->touch_ID2=(s16)(buf[0x0b] & 0xF0)>>4;
		
		dprintk(DEBUG_X_Y_INFO,"touch id : %d. \n",event->touch_ID2);
	case 1:
		event->x1 = (s16)(buf[3] & 0x0F)<<8 | (s16)buf[4];
		event->y1 = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
		dprintk(DEBUG_X_Y_INFO,"source data:event->x1 = %d, event->y1 = %d. \n", event->x1, event->y1);
		if(1 == exchange_x_y_flag){
			swap(event->x1, event->y1);
		}
		if(1 == revert_x_flag){
			event->x1 = SCREEN_MAX_X - event->x1;
		}
		if(1 == revert_y_flag){
			event->y1 = SCREEN_MAX_Y - event->y1;
		}
		event->touch_ID1=(s16)(buf[0x05] & 0xF0)>>4;
		dprintk(DEBUG_X_Y_INFO,"touch id : %d. \n",event->touch_ID1);
		break;
	default:
		return -1;
	}
	event->pressure = 20;
        return 0;
}

#ifdef TOUCH_KEY_LIGHT_SUPPORT
static void ft5x_lighting(void)
{
        ctp_key_light(1,15);
	return;
}
#endif

static void ft5x_report_multitouch(void)
{
	struct ft5x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;

#ifdef TOUCH_KEY_SUPPORT
	if(1 == key_tp){
		return;
	}
#endif

	switch(event->touch_point) {
	case 5:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID5);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x5);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y5);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 30);
		input_mt_sync(data->input_dev);
		dprintk(DEBUG_X_Y_INFO,"report data:===x5 = %d,y5 = %d ====\n",event->x5,event->y5);
	case 4:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID4);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x4);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y4);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 30);
		input_mt_sync(data->input_dev);
		dprintk(DEBUG_X_Y_INFO,"report data:===x4 = %d,y4 = %d ====\n",event->x4,event->y4);
	case 3:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID3);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x3);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y3);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 30);
		input_mt_sync(data->input_dev);
		dprintk(DEBUG_X_Y_INFO,"report data:===x3 = %d,y3 = %d ====\n",event->x3,event->y3);
	case 2:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID2);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x2);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y2);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 30);
		input_mt_sync(data->input_dev);
		dprintk(DEBUG_X_Y_INFO,"report data:===x2 = %d,y2 = %d ====\n",event->x2,event->y2);
	case 1:
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->touch_ID1);	
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x1);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y1);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 30);
		input_mt_sync(data->input_dev);
		dprintk(DEBUG_X_Y_INFO,"report data:===x1 = %d,y1 = %d ====\n",event->x1,event->y1);
		break;
	default:
		dprintk(DEBUG_X_Y_INFO,"report data:==touch_point default =\n");
		break;
	}
	
	input_sync(data->input_dev);
	return;
}

#ifndef CONFIG_FT5X0X_MULTITOUCH
static void ft5x_report_singletouch(void)
{
	struct ft5x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	
	if (event->touch_point == 1) {
		input_report_abs(data->input_dev, ABS_X, event->x1);
		input_report_abs(data->input_dev, ABS_Y, event->y1);
		input_report_abs(data->input_dev, ABS_PRESSURE, event->pressure);
	}
	dprintk(DEBUG_X_Y_INFO,"report:===x1 = %d,y1 = %d ====\n",event->x1,event->y1);
	input_report_key(data->input_dev, BTN_TOUCH, 1);
	input_sync(data->input_dev);
	return;
}
#endif

#ifdef TOUCH_KEY_SUPPORT
static void ft5x_report_touchkey(void)
{
	struct ft5x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;

#ifdef TOUCH_KEY_FOR_ANGDA
	if((1==event->touch_point)&&(event->x1 > TOUCH_KEY_X_LIMIT)){
		key_tp = 1;
		if(event->y1 < 40){
			key_val = 1;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);  
			dprintk(DEBUG_KEY_INFO,"===KEY 1====\n");
		}else if(event->y1 < 90){
			key_val = 2;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);     
			dprintk(DEBUG_KEY_INFO,"===KEY 2 ====\n");
		}else{
			key_val = 3;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);     
			dprintk(DEBUG_KEY_INFO,"===KEY 3====\n");	
		}
	} else{
		key_tp = 0;
	}
#endif
#ifdef TOUCH_KEY_FOR_EVB13
	if((1==event->touch_point)&&((event->x1 > TOUCH_KEY_LOWER_X_LIMIT)&&(event->x1<TOUCH_KEY_HIGHER_X_LIMIT))){
		key_tp = 1;
		if(event->y1 < 5){
			key_val = 1;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);  
			dprintk(DEBUG_KEY_INFO,"===KEY 1====\n");     
		}else if((event->y1 < 45)&&(event->y1>35)){
			key_val = 2;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);     
			dprintk(DEBUG_KEY_INFO,"===KEY 2 ====\n");
		}else if((event->y1 < 75)&&(event->y1>65)){
			key_val = 3;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);     
			dprintk(DEBUG_KEY_INFO,"===KEY 3====\n");
		}else if ((event->y1 < 105)&&(event->y1>95))	{
			key_val = 4;
			input_report_key(data->input_dev, key_val, 1);
			input_sync(data->input_dev);     
			dprintk(DEBUG_KEY_INFO,"===KEY 4====\n");	
		}
	}else{
		key_tp = 0;
	}
#endif

#ifdef TOUCH_KEY_LIGHT_SUPPORT
	ft5x_lighting();
#endif
	return;
}
#endif

static void ft5x_report_value(void)
{

#ifdef TOUCH_KEY_SUPPORT
	ft5x_report_touchkey();
#endif

#ifdef CONFIG_FT5X0X_MULTITOUCH
	ft5x_report_multitouch();
#else	/* CONFIG_FT5X0X_MULTITOUCH*/
	ft5x_report_singletouch();
#endif	/* CONFIG_FT5X0X_MULTITOUCH*/
	return;
}	

static void ft5x_ts_pen_irq_work(struct work_struct *work)
{
	int ret = -1;
	ret = ft5x_read_data();
	if (ret == 0) {
		ft5x_report_value();
	}
	dprintk(DEBUG_INT_INFO,"%s:ret:%d\n",__func__,ret);
}

irqreturn_t ft5x_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x_ts_data *ft5x_ts = (struct ft5x_ts_data *)dev_id;
	dprintk(DEBUG_INT_INFO,"==========ft5x_ts TS Interrupt============\n"); 
	queue_work(ft5x_ts->ts_workqueue, &ft5x_ts->pen_event_work);
	return IRQ_HANDLED;
}

static void ft5x_resume_events (struct work_struct *work)
{
	int i = 0;
    int ret = 0;
	ctp_wakeup(0, 20);
   
#ifdef CONFIG_HAS_EARLYSUSPEND	
	if(STANDBY_WITH_POWER_OFF != standby_level){
		goto standby_with_power_on; 
	}
#endif

	if(chip_id == 0x02 ){
#ifdef FT5X02_CONFIG_INI
		if (ft5x02_Get_Param_From_Ini(FT5X02_CONFIG_NAME) >= 0)
			ft5x02_Init_IC_Param(this_client);
		else
			printk("Get ft5x02 param from INI file failed\n");
#else
		msleep(200);	/*wait...*/
		while(i<5){
			dprintk(DEBUG_INIT,"-----------------------------------------Init ic param\r\n");
			if (ft5x02_Init_IC_Param(this_client) >=0 ){
				dprintk(DEBUG_INIT,"---------------------------------------get ic param\r\n");
				if(ft5x02_get_ic_param(this_client) >=0)
					break;
			}
			i++;
		}
#endif
        }

#ifdef CONFIG_HAS_EARLYSUSPEND
standby_with_power_on:
#endif
	ret = input_set_int_enable(&(config_info.input_type), 1);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq disable failed\n", __func__);
}


static int ft5x_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct ft5x_ts_data *data = i2c_get_clientdata(client);
	int ret = 0;
	dprintk(DEBUG_SUSPEND,"==ft5x_ts_suspend=\n");
	dprintk(DEBUG_SUSPEND,"CONFIG_PM: write FT5X0X_REG_PMODE .\n");
#ifndef CONFIG_HAS_EARLYSUSPEND
	data->is_suspended = true;
#endif
	if (data->is_suspended == true) {
	//	ft5x_ts_release();
		flush_workqueue(ft5x_resume_wq);
		ret = input_set_int_enable(&(config_info.input_type), 0);
		if (ret < 0)
			dprintk(DEBUG_SUSPEND,"%s irq disable failed\n", __func__);
		cancel_work_sync(&data->pen_event_work);
		flush_workqueue(data->ts_workqueue);
		ft5x_set_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
		msleep(5);
		input_set_power_enable(&(config_info.input_type), 0);
	}
	return 0;
}

static int ft5x_ts_resume(struct i2c_client *client)
{
	struct ft5x_ts_data *data = i2c_get_clientdata(client);
	dprintk(DEBUG_SUSPEND,"==CONFIG_PM:ft5x_ts_resume== \n");
	data->is_suspended = true;
	input_set_power_enable(&(config_info.input_type), 1);
        msleep(5);
    queue_work(ft5x_resume_wq, &ft5x_resume_work);
	
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x_ts_early_suspend(struct early_suspend *handler)
{
    int ret = 0;
	struct ft5x_ts_data *data = i2c_get_clientdata(this_client);
	dprintk(DEBUG_SUSPEND,"==ft5x_ts_suspend=\n");
	dprintk(DEBUG_SUSPEND,"CONFIG_HAS_EARLYSUSPEND: write FT5X0X_REG_PMODE .\n");
	ft5x_ts_release();
    data->is_suspended = false;
	flush_workqueue(ft5x_resume_wq);
	ret = input_set_int_enable(&(config_info.input_type), 0);
	if (ret < 0)
		dprintk(DEBUG_SUSPEND,"%s irq disable failed\n", __func__);
	cancel_work_sync(&data->pen_event_work);
	flush_workqueue(data->ts_workqueue);
	ft5x_set_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
	msleep(5);	
	input_set_power_enable(&(config_info.input_type), 0);
}

static void ft5x_ts_late_resume(struct early_suspend *handler)
{
	struct ft5x_ts_data *data = container_of(handler, struct ft5x_ts_data, early_suspend);
	dprintk(DEBUG_SUSPEND,"==CONFIG_HAS_EARLYSUSPEND:ft5x_ts_resume== \n");
	if (data->is_suspended == false) {
		input_set_power_enable(&(config_info.input_type), 1);
		msleep(5);
		queue_work(ft5x_resume_wq, &ft5x_resume_work);
	}

	printk("ts->is_suspended:%d\n",data->is_suspended);
}

#endif

static void ft5x_init_events (struct work_struct *work)
{
	int i = 0;
	int ret; 
	dprintk(DEBUG_INIT,"====%s begin=====.  \n", __func__);

	while((chip_id == 0x00) || (chip_id == 0xa3)){
		delay_ms(5);
		ret = i2c_smbus_read_byte_data(this_client,0xA3);
        	dprintk(DEBUG_INIT,"addr:0x%x,chip_id_value:0x%x\n",this_client->addr,ret);
		if((ret != 0x00) && (ret != 0xa3)) {
			chip_id = ret;
			break;
		}
		if((i++)>10) {
			break;
		}	
	}
	dprintk(DEBUG_INIT,"read chip_id timers,timers=%d\n",i);
	i = 0;

#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
	fts_ctpm_fw_upgrade_with_i_file();
#endif

	if(chip_id == 0x02 ){
#ifdef FT5X02_CONFIG_INI
		if (ft5x02_Get_Param_From_Ini(FT5X02_CONFIG_NAME) >= 0)
			ft5x02_Init_IC_Param(this_client);
		else
			printk("Get ft5x02 param from INI file failed\n");
#else
		msleep(1000);	/*wait...*/
		while(i<5){
			dprintk(DEBUG_INIT,"-----------------------------------------Init ic param\r\n");
			if (ft5x02_Init_IC_Param(this_client) >=0 ){
				dprintk(DEBUG_INIT,"---------------------------------------get ic param\r\n");
				if(ft5x02_get_ic_param(this_client) >=0)
					break;
			}
			i++;
		}
#endif
        }

}

static int ft5x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ft5x_ts_data *ft5x_ts;
	struct input_dev *input_dev;
	struct device *dev;
	struct i2c_dev *i2c_dev;
	int err = 0;
        
#ifdef TOUCH_KEY_SUPPORT
	int i = 0;
#endif

	dprintk(DEBUG_INIT,"====%s begin=====.  \n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		printk("check_functionality_failed\n");
		goto exit_check_functionality_failed;
	}

	ft5x_ts = kzalloc(sizeof(*ft5x_ts), GFP_KERNEL);
	if (!ft5x_ts)	{
		err = -ENOMEM;
		printk("alloc_data_failed\n");
		goto exit_alloc_data_failed;
	}

	this_client = client;
	i2c_set_clientdata(client, ft5x_ts);

	ft5x_wq = create_singlethread_workqueue("ft5x_init");
	if (ft5x_wq == NULL) {
		printk("create ft5x_wq fail!\n");
		return -ENOMEM;
	}

	queue_work(ft5x_wq, &ft5x_init_work);

	INIT_WORK(&ft5x_ts->pen_event_work, ft5x_ts_pen_irq_work);
	ft5x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!ft5x_ts->ts_workqueue) {
		err = -ESRCH;
		printk("ts_workqueue fail!\n");
		goto exit_create_singlethread;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}
	
	ft5x_ts->input_dev = input_dev;

#ifdef CONFIG_FT5X0X_MULTITOUCH
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);	
#ifdef FOR_TSLIB_TEST
	set_bit(BTN_TOUCH, input_dev->keybit);
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

	input_dev->name	= CTP_NAME;		//dev_name(&client->dev)
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,"ft5x_ts_probe: failed to register input device: %s\n",
		        dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

    ft5x_ts->is_suspended = false;

	ft5x_resume_wq = create_singlethread_workqueue("ft5x_resume");
	if (ft5x_resume_wq == NULL) {
		printk("create ft5x_resume_wq fail!\n");
		return -ENOMEM;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	printk("==register_early_suspend =\n");
	ft5x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ft5x_ts->early_suspend.suspend = ft5x_ts_early_suspend;
	ft5x_ts->early_suspend.resume	= ft5x_ts_late_resume;
	register_early_suspend(&ft5x_ts->early_suspend);
#endif

#ifdef CONFIG_FT5X0X_MULTITOUCH
	dprintk(DEBUG_INIT,"CONFIG_FT5X0X_MULTITOUCH is defined. \n");
#endif

    config_info.dev = &(ft5x_ts->input_dev->dev);
	err = input_request_int(&(config_info.input_type), ft5x_ts_interrupt,
				CTP_IRQ_MODE, ft5x_ts);
	if (err) {
		pr_info( "goodix_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}

    i2c_dev = get_free_i2c_dev(client->adapter);	
	if (IS_ERR(i2c_dev)){	
		err = PTR_ERR(i2c_dev);	
		printk("i2c_dev fail!");	
		return err;	
	}
	
	dev = device_create(i2c_dev_class, &client->adapter->dev, MKDEV(I2C_MAJOR,client->adapter->nr),
	         NULL, "aw_i2c_ts%d", client->adapter->nr);	
	if (IS_ERR(dev))	{		
			err = PTR_ERR(dev);
			printk("dev fail!\n");		
			return err;	
	}

	device_enable_async_suspend(&client->dev);
	dprintk(DEBUG_INIT,"==%s over =\n", __func__);

	return 0;

exit_irq_request_failed:
    cancel_work_sync(&ft5x_resume_work);
	destroy_workqueue(ft5x_resume_wq);	
exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
	input_free_int(&(config_info.input_type), ft5x_ts);
    i2c_set_clientdata(client, NULL);
    cancel_work_sync(&ft5x_ts->pen_event_work);
	destroy_workqueue(ft5x_ts->ts_workqueue);
exit_create_singlethread:
	kfree(ft5x_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	cancel_work_sync(&ft5x_init_work);
	destroy_workqueue(ft5x_wq);
        
	return err;
}

static int __devexit ft5x_ts_remove(struct i2c_client *client)
{

	struct ft5x_ts_data *ft5x_ts = i2c_get_clientdata(client);
	ft5x_set_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
	
	printk("==ft5x_ts_remove=\n");
	device_destroy(i2c_dev_class, MKDEV(I2C_MAJOR,client->adapter->nr));
	input_free_int(&(config_info.input_type), ft5x_ts);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ft5x_ts->early_suspend);
#endif
	cancel_work_sync(&ft5x_resume_work);
	destroy_workqueue(ft5x_resume_wq);
	input_unregister_device(ft5x_ts->input_dev);
	input_free_device(ft5x_ts->input_dev);
	cancel_work_sync(&ft5x_ts->pen_event_work);
	destroy_workqueue(ft5x_ts->ts_workqueue);
	input_set_power_enable(&(config_info.input_type), 0);
	kfree(ft5x_ts);
    
	i2c_set_clientdata(this_client, NULL);

	return 0;

}

static const struct i2c_device_id ft5x_ts_id[] = {
	{ CTP_NAME, 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, ft5x_ts_id);

static struct i2c_driver ft5x_ts_driver = {
	.class          = I2C_CLASS_HWMON,
	.probe		= ft5x_ts_probe,
	.remove		= __devexit_p(ft5x_ts_remove),
	.id_table	= ft5x_ts_id,
	.suspend        = ft5x_ts_suspend,
	.resume         = ft5x_ts_resume,
	.driver	= {
		.name	= CTP_NAME,
		.owner	= THIS_MODULE,
	},
	.address_list	= normal_i2c,

};

static int aw_open(struct inode *inode, struct file *file)
{
	int subminor;
	int ret = 0;	
	struct i2c_client *client;
	struct i2c_adapter *adapter;	
	struct i2c_dev *i2c_dev;	

	printk("====%s======.\n", __func__);
	dprintk(DEBUG_OTHERS_INFO,"enter aw_open function\n");
	subminor = iminor(inode);
	dprintk(DEBUG_OTHERS_INFO,"subminor=%d\n",subminor);
	
	i2c_dev = i2c_dev_get_by_minor(2);	
	if (!i2c_dev)	{	
		printk("error i2c_dev\n");		
		return -ENODEV;	
	}	
	adapter = i2c_get_adapter(i2c_dev->adap->nr);	
	if (!adapter)	{		
		return -ENODEV;	
	}	
	
	client = kzalloc(sizeof(*client), GFP_KERNEL);	
	
	if (!client)	{		
		i2c_put_adapter(adapter);		
		return -ENOMEM;	
	}	
	snprintf(client->name, I2C_NAME_SIZE, "pctp_i2c_ts%d", adapter->nr);
	client->driver = &ft5x_ts_driver;
	client->adapter = adapter;		
	file->private_data = client;
		
	return 0;
}

static long aw_ioctl(struct file *file, unsigned int cmd,unsigned long arg ) 
{
	dprintk(DEBUG_OTHERS_INFO,"====%s====\n",__func__);
	dprintk(DEBUG_OTHERS_INFO,"line :%d,cmd = %d,arg = %ld.\n",__LINE__,cmd,arg);
	
	switch (cmd) {
	case UPGRADE:
	        dprintk(DEBUG_OTHERS_INFO,"==UPGRADE_WORK=\n");
		fts_ctpm_fw_upgrade_with_i_file();
		// calibrate();
		break;
	default:
		break;			 
	}	
	return 0;
}

static int aw_release (struct inode *inode, struct file *file) 
{
	struct i2c_client *client = file->private_data;
	dprintk(DEBUG_OTHERS_INFO,"enter aw_release function.\n");		
	i2c_put_adapter(client->adapter);
	kfree(client);
	file->private_data = NULL;
	return 0;	  
}

static const struct file_operations aw_i2c_ts_fops ={	
	.owner = THIS_MODULE, 		
	.open = aw_open, 	
	.unlocked_ioctl = aw_ioctl,	
	.release = aw_release, 
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
static int __init ft5x_ts_init(void)
{ 
	int ret = -1;      
	dprintk(DEBUG_INIT,"***************************init begin*************************************\n");
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
	input_set_power_enable(&(config_info.input_type), 1);
	msleep(10);	
	ctp_wakeup(0, 10);

	ft5x_ts_driver.detect = ctp_detect;

	ret= register_chrdev(I2C_MAJOR,"aw_i2c_ts",&aw_i2c_ts_fops );	
	if(ret) {	
		printk("%s:register chrdev failed\n",__FILE__);	
		return ret;
	}
	
	i2c_dev_class = class_create(THIS_MODULE,"aw_i2c_dev");
	if (IS_ERR(i2c_dev_class)) {		
		ret = PTR_ERR(i2c_dev_class);		
		class_destroy(i2c_dev_class);	
	}
        ret = i2c_add_driver(&ft5x_ts_driver);
        
        dprintk(DEBUG_INIT,"****************************init end************************************\n");
	return ret;
}

static void __exit ft5x_ts_exit(void)
{
	printk("==ft5x_ts_exit==\n");
	i2c_del_driver(&ft5x_ts_driver);
	class_destroy(i2c_dev_class);
	unregister_chrdev(I2C_MAJOR, "aw_i2c_ts");
	input_free_platform_resource(&(config_info.input_type));
}

late_initcall(ft5x_ts_init);
module_exit(ft5x_ts_exit);
MODULE_AUTHOR("<wenfs@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x TouchScreen driver");
MODULE_LICENSE("GPL");

