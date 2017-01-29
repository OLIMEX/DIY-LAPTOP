/*
Copyright (c) 2012-2022 ChipOne Technology (Beijing) Co., Ltd. All Rights Reserved.
This PROPRIETARY SOFTWARE is the property of ChipOne Technology (Beijing) Co., Ltd.
and may contains trade secrets and/or other confidential information of ChipOne
Technology (Beijing) Co., Ltd. This file shall not be disclosed to any third party,
in whole or in part, without prior written consent of ChipOne.
THIS PROPRIETARY SOFTWARE & ANY RELATED DOCUMENTATION ARE PROVIDED AS IS,
WITH ALL FAULTS, & WITHOUT WARRANTY OF ANY KIND. CHIPONE DISCLAIMS ALL EXPRESS OR
IMPLIED WARRANTIES.

File Name:    icn83xx.c
Abstract:
              input driver.
Author:       Zhimin Tian
Date :        01,17,2013
Version:      1.0
History :
2012,10,30, V0.1 first version
*/

#include "icn83xx.h"

static struct i2c_client *this_client;
short log_rawdata[28][16]  = {{0},};
short log_diffdata[28][16] = {{0},};

#define CTP_IRQ_NUMBER          (config_info.int_number)
#define CTP_IRQ_MODE		    (IRQF_TRIGGER_FALLING)

#define SCREEN_MAX_X			(screen_max_x)
#define SCREEN_MAX_Y			(screen_max_y)
#define PRESS_MAX			(255)

static int screen_max_x = 0;
static int screen_max_y = 0;
static int revert_x_flag = 0;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;
static u32 int_handle = 0;
static __u32 twi_id = 0;

struct ctp_config_info config_info = {
	    .input_type = CTP_TYPE,
	    .name = NULL,
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
        printk("[CTP]:"fmt, ## arg)
module_param_named(debug_mask,debug_mask,int,S_IRUGO | S_IWUSR | S_IWGRP);

static const unsigned short normal_i2c[2] = {0x40,I2C_CLIENT_END};

#if SUPPORT_FW_UPDATE
static char firmware[128] = {"/system/vendor/modules/fw.bin"};
#endif

#if SUPPORT_SYSFS
static enum hrtimer_restart chipone_timer_func(struct hrtimer *timer);
static ssize_t icn83xx_show_update(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t icn83xx_store_update(struct device* cd, struct device_attribute *attr, const char* buf, size_t len);
static ssize_t icn83xx_show_process(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t icn83xx_store_process(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);

static DEVICE_ATTR(update, S_IRUGO | S_IWUSR, icn83xx_show_update, icn83xx_store_update);
static DEVICE_ATTR(process, S_IRUGO | S_IWUSR, icn83xx_show_process, icn83xx_store_process);


static ssize_t icn83xx_show_process(struct device* cd,struct device_attribute *attr, char* buf)
{
        ssize_t ret = 0;
        sprintf(buf, "icn83xx process\n");
        ret = strlen(buf) + 1;
        return ret;
}

static ssize_t icn83xx_store_process(struct device* cd, struct device_attribute *attr,
               const char* buf, size_t len)
{
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        unsigned long on_off = simple_strtoul(buf, NULL, 10);
        if(on_off == 0) {
                icn83xx_ts->work_mode = on_off;
        } else if((on_off == 1) || (on_off == 2)){
                if((icn83xx_ts->work_mode == 0) && (icn83xx_ts->use_irq == 1)) {
                        hrtimer_init(&icn83xx_ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
                        icn83xx_ts->timer.function = chipone_timer_func;
                        hrtimer_start(&icn83xx_ts->timer, ktime_set(CTP_START_TIMER/1000, (CTP_START_TIMER%1000)*1000000), HRTIMER_MODE_REL);
                }
                icn83xx_ts->work_mode = on_off;
        }
        return len;
}

static ssize_t icn83xx_show_update(struct device* cd,
                     struct device_attribute *attr, char* buf)
{
        ssize_t ret = 0;
        sprintf(buf, "icn83xx firmware\n");
        ret = strlen(buf) + 1;
        return ret;
}

static ssize_t icn83xx_store_update(struct device* cd, struct device_attribute *attr, const char* buf, size_t len)
{
        int err=0;
        unsigned long on_off = simple_strtoul(buf, NULL, 10);
        return len;
}

static int icn83xx_create_sysfs(struct i2c_client *client)
{
        int err;
        struct device *dev = &(client->dev);
        printk("%s: \n",__func__);
        err = device_create_file(dev, &dev_attr_update);
        err = device_create_file(dev, &dev_attr_process);
        return err;
}

#endif

#if SUPPORT_PROC_FS

pack_head cmd_head;
static struct proc_dir_entry *icn83xx_proc_entry;
int  DATA_LENGTH = 0;
static int icn83xx_tool_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
{
        int ret = 0;

        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        proc_info("%s \n",__func__);
        if(down_interruptible(&icn83xx_ts->sem)) {
                return -1;
        }
        ret = copy_from_user(&cmd_head, buff, CMD_HEAD_LENGTH);
        if(ret)  {
                proc_error("copy_from_user failed.\n");
                goto write_out;
        } else {
                ret = CMD_HEAD_LENGTH;
        }

        proc_info("wr  :0x%02x.\n", cmd_head.wr);
        proc_info("flag:0x%02x.\n", cmd_head.flag);
        proc_info("circle  :%d.\n", (int)cmd_head.circle);
        proc_info("times   :%d.\n", (int)cmd_head.times);
        proc_info("retry   :%d.\n", (int)cmd_head.retry);
        proc_info("data len:%d.\n", (int)cmd_head.data_len);
        proc_info("addr len:%d.\n", (int)cmd_head.addr_len);
        proc_info("addr:0x%02x%02x.\n", cmd_head.addr[0], cmd_head.addr[1]);
        proc_info("len:%d.\n", (int)len);
        proc_info("data:0x%02x%02x.\n", buff[CMD_HEAD_LENGTH], buff[CMD_HEAD_LENGTH+1]);
        if (1 == cmd_head.wr)  {
                if(1 == cmd_head.addr_len) {
                        ret = copy_from_user(&cmd_head.data[0], &buff[CMD_HEAD_LENGTH], cmd_head.data_len);
                        if(ret) {
                                proc_error("copy_from_user failed.\n");
                                goto write_out;
                        }
                        ret = icn83xx_i2c_txdata(cmd_head.addr[0], &cmd_head.data[0], cmd_head.data_len);
                        if (ret < 0) {
                                proc_error("write iic failed! ret: %d\n", ret);
                                goto write_out;
                        }
                        ret = cmd_head.data_len + CMD_HEAD_LENGTH;
                        goto write_out;
                }
        } else if(3 == cmd_head.wr) {
                ret = copy_from_user(&cmd_head.data[0], &buff[CMD_HEAD_LENGTH], cmd_head.data_len);
                if(ret) {
                        proc_error("copy_from_user failed.\n");
                        goto write_out;
                }
                ret = cmd_head.data_len + CMD_HEAD_LENGTH;
                memset(firmware, 0, 128);
                memcpy(firmware, &cmd_head.data[0], cmd_head.data_len);
                proc_info("firmware : %s\n", firmware);
        } else if(5 == cmd_head.wr) {
                icn83xx_update_status(1);
                ret = kernel_thread(icn83xx_fw_update,firmware,CLONE_KERNEL);
                icn83xx_trace("the kernel_thread result is:%d\n", ret);
        } else if(7 == cmd_head.wr) {
                if(2 == cmd_head.addr_len) {
                        ret = copy_from_user(&cmd_head.data[0], &buff[CMD_HEAD_LENGTH], cmd_head.data_len);
                        if(ret) {
                                proc_error("copy_from_user failed.\n");
                                goto write_out;
                        }
                        ret = icn83xx_writeReg((cmd_head.addr[0]<<8)|cmd_head.addr[1], cmd_head.data[0]);
                        if (ret < 0) {
                                proc_error("write reg failed! ret: %d\n", ret);
                                goto write_out;
                        }
                        ret = cmd_head.data_len + CMD_HEAD_LENGTH;
                        goto write_out;

                }
        }

write_out:
        up(&icn83xx_ts->sem);
        return len;

}
static int icn83xx_tool_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
        int i;
        int ret = 0;
        int data_len = 0;
        int len = 0;
        int loc = 0;
        char retvalue;
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        if(down_interruptible(&icn83xx_ts->sem))
                return -1;
        proc_info("%s: count:%d, off:%d, cmd_head.data_len: %d\n",__func__, count, off, cmd_head.data_len);
        if (cmd_head.wr % 2) {
                ret = 0;
                goto read_out;
        } else if (0 == cmd_head.wr) {
                if(1 == cmd_head.addr_len) {
                        data_len = cmd_head.data_len;
                        if(cmd_head.addr[0] == 0xff) {
                                page[0] = 83;
                                proc_info("read ic type: %d\n", page[0]);
                        } else {
                                while(data_len>0) {
                                        if (data_len > DATA_LENGTH)
                                                len = DATA_LENGTH;
                                        else
                                                len = data_len;
                                        data_len -= len;
                                        memset(&cmd_head.data[0], 0, len+1);
                                        ret = icn83xx_i2c_rxdata(cmd_head.addr[0]+loc, &cmd_head.data[0], len);

                                        if(ret < 0) {
                                                icn83xx_error("read iic failed: %d\n", ret);
                                                goto read_out;
                                        } else
                                                memcpy(&page[loc], &cmd_head.data[0], len);
                                        loc += len;
                                }
                                proc_info("page[0]: 0x%02x\n", page[0]);
                                proc_info("page[1]: 0x%02x\n", page[1]);
                        }
                }
        } else if(2 == cmd_head.wr) {
                //scan tp rawdata
                icn83xx_write_reg(4, 0x20);
                mdelay(cmd_head.times);
                icn83xx_read_reg(2, &retvalue);
                while(retvalue != 1) {
                        mdelay(cmd_head.times);
                        icn83xx_read_reg(2, &retvalue);
                }

                if(2 == cmd_head.addr_len) {
                        for(i=0; i<cmd_head.addr[1]; i++) {
                                icn83xx_write_reg(3, i);
                                mdelay(cmd_head.times);
                                ret = icn83xx_i2c_rxdata(128, &cmd_head.data[0], cmd_head.addr[0]*2);
                                if (ret < 0) {
                                        icn83xx_error("read rawdata failed: %d\n", ret);
                                        goto read_out;
                                } else {
                                        //proc_info("read rawdata out %d bytes, loc: %d\n", cmd_head.addr[0]*2, loc);
                                        memcpy(&page[loc], &cmd_head.data[0], cmd_head.addr[0]*2);
                                }
                                loc += cmd_head.addr[0]*2;
                        }
                        for(i=0; i<cmd_head.data_len; i=i+2)
                                swap_ab(page[i], page[i+1]);
                    //icn83xx_rawdatadump(&page[0], cmd_head.data_len/2, cmd_head.addr[0]);
                }

                //finish scan tp rawdata
                icn83xx_write_reg(2, 0x0);

        }else if(4 == cmd_head.wr)  //get update status
                page[0] = icn83xx_get_status();
        else if(6 == cmd_head.wr)  {
                if(2 == cmd_head.addr_len) {
                        ret = icn83xx_readReg((cmd_head.addr[0]<<8)|cmd_head.addr[1], &cmd_head.data[0]);
                        if (ret < 0) {
                                proc_error("reg reg failed! ret: %d\n", ret);
                                goto read_out;
                        }
                        page[0] = cmd_head.data[0];
                        goto read_out;
                }
        }
read_out:
        up(&icn83xx_ts->sem);
        proc_info("%s out: %d, cmd_head.data_len: %d\n\n",__func__, count, cmd_head.data_len);
        return cmd_head.data_len;
}

int init_proc_node()
{
        int i;
        memset(&cmd_head, 0, sizeof(cmd_head));
        cmd_head.data = NULL;

        i = 5;
        while ((!((int)cmd_head.data) )&& i){
                cmd_head.data = kzalloc(i * DATA_LENGTH_UINT, GFP_KERNEL);
                if (NULL != cmd_head.data)
                        break;
                i--;
        }
        if (i) {
                //DATA_LENGTH = i * DATA_LENGTH_UINT + GTP_ADDR_LENGTH;
                DATA_LENGTH = i * DATA_LENGTH_UINT;
                proc_info("alloc memory size:%d.\n", DATA_LENGTH);
        } else {
                printk("alloc for memory failed.\n");
                return 0;
        }

        icn83xx_proc_entry = create_proc_entry(ICN83XX_ENTRY_NAME, 0666, NULL);
        if (icn83xx_proc_entry == NULL) {
                printk("Couldn't create proc entry!\n");
                return 0;
        } else {
                proc_info("Create proc entry success!\n");
                icn83xx_proc_entry->write_proc = icn83xx_tool_write;
                icn83xx_proc_entry->read_proc = icn83xx_tool_read;
        }

        return 1;
}

void uninit_proc_node(void)
{
        kfree(cmd_head.data);
        cmd_head.data = NULL;
        remove_proc_entry(ICN83XX_ENTRY_NAME, NULL);
}

#endif


#if TOUCH_VIRTUAL_KEYS
static ssize_t virtual_keys_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf,
         __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":100:1030:50:60"
         ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":280:1030:50:60"
         ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":470:1030:50:60"
         ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":900:1030:50:60"
         "\n");
}

static struct kobj_attribute virtual_keys_attr = {
        .attr = {
                .name = "virtualkeys.chipone-ts",
                .mode = S_IRUGO,
        },
        .show = &virtual_keys_show,
};

static struct attribute *properties_attrs[] = {
        &virtual_keys_attr.attr,
        NULL
};

static struct attribute_group properties_attr_group = {
        .attrs = properties_attrs,
};

static void icn83xx_ts_virtual_keys_init(void)
{
        int ret;
        struct kobject *properties_kobj;
        properties_kobj = kobject_create_and_add("board_properties", NULL);
        if (properties_kobj)
                ret = sysfs_create_group(properties_kobj,
                         &properties_attr_group);
        if (!properties_kobj || ret)
                pr_err("failed to create board_properties\n");
}
#endif



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
                dprintk(DEBUG_INIT,"\n##############%s: Detected chip %s at adapter %d, address 0x%02x ##############\n",
                     __func__, CTP_NAME, i2c_adapter_id(adapter), client->addr);

                strlcpy(info->type, CTP_NAME, I2C_NAME_SIZE);
                return 0;
        }else{
                return -ENODEV;
        }
}


/***********************************************************************************************
Name    :   icn83xx_prog_i2c_rxdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : read data from icn83xx, prog mode
***********************************************************************************************/
int icn83xx_prog_i2c_rxdata(unsigned short addr, char *rxdata, int length)
{
        int ret = -1;
        int retries = 0;
        unsigned char tmp_buf[2];
        struct i2c_msg msgs[] = {
                {
                        .addr   = ICN83XX_PROG_IIC_ADDR,//this_client->addr,
                        .flags  = 0,
                        .len    = 2,
                        .buf    = tmp_buf,
                },{
                        .addr   = ICN83XX_PROG_IIC_ADDR,//this_client->addr,
                        .flags  = I2C_M_RD,
                        .len    = length,
                        .buf    = rxdata,
                },
        };
        tmp_buf[0] = U16HIBYTE(addr);
        tmp_buf[1] = U16LOBYTE(addr);

        while(retries < IIC_RETRY_NUM) {
                ret = i2c_transfer(this_client->adapter, msgs, 2);
                if(ret == 2)break;
                retries++;
        }

        if (retries >= IIC_RETRY_NUM) {
                printk("%s i2c read error: %d\n", __func__, ret);
                //icn83xx_ts_reset();
        }


        return ret;
}
/***********************************************************************************************
Name    :   icn83xx_prog_i2c_txdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : send data to icn83xx , prog mode
***********************************************************************************************/
int icn83xx_prog_i2c_txdata(unsigned short addr, char *txdata, int length)
{
        int ret = -1;
        char tmp_buf[128];
        int retries = 0;
        struct i2c_msg msg[] = {
                {
                        .addr   = ICN83XX_PROG_IIC_ADDR,//this_client->addr,
                        .flags  = 0,
                        .len    = length + 2,
                        .buf    = tmp_buf,
                },
        };

        if (length > 125) {
                printk("%s too big datalen = %d!\n", __func__, length);
                return -1;
        }

        tmp_buf[0] = U16HIBYTE(addr);
        tmp_buf[1] = U16LOBYTE(addr);

        if (length != 0 && txdata != NULL)
                memcpy(&tmp_buf[2], txdata, length);


        while(retries < IIC_RETRY_NUM) {
                ret = i2c_transfer(this_client->adapter, msg, 1);
                if(ret == 1)break;
                retries++;
        }

        if (retries >= IIC_RETRY_NUM) {
                printk("%s i2c write error: %d\n", __func__, ret);
                //icn83xx_ts_reset();
        }

        return ret;
}
/***********************************************************************************************
Name    :   icn83xx_prog_write_reg
Input   :   addr -- address
            para -- parameter
Output  :
function    :   write register of icn83xx, prog mode
***********************************************************************************************/
int icn83xx_prog_write_reg(unsigned short addr, char para)
{
        char buf[3];
        int ret = -1;

        buf[0] = para;
        ret = icn83xx_prog_i2c_txdata(addr, buf, 1);
        if (ret < 0) {
                printk("write reg failed! %#x ret: %d\n", buf[0], ret);
                return -1;
        }

        return ret;
}


/***********************************************************************************************
Name    :   icn83xx_prog_read_reg
Input   :   addr
            pdata
Output  :
function    :   read register of icn83xx, prog mode
***********************************************************************************************/
int icn83xx_prog_read_reg(unsigned short addr, char *pdata)
{
        int ret = -1;
        ret = icn83xx_prog_i2c_rxdata(addr, pdata, 1);
        return ret;
}

/***********************************************************************************************
Name    :   icn83xx_i2c_rxdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : read data from icn83xx, normal mode
***********************************************************************************************/
int icn83xx_i2c_rxdata(unsigned char addr, char *rxdata, int length)
{
        int ret = -1;
        int retries = 0;
        unsigned char tmp_buf[1];

        struct i2c_msg msgs[] = {
                {
                        .addr   = this_client->addr,
                        .flags  = 0,
                        .len    = 1,
                        .buf    = tmp_buf,
                }, {
                        .addr   = this_client->addr,
                        .flags  = I2C_M_RD,
                        .len    = length,
                        .buf    = rxdata,
                },
        };
        tmp_buf[0] = addr;

        while(retries < IIC_RETRY_NUM) {
                ret = i2c_transfer(this_client->adapter, msgs, 2);
                if(ret == 2)break;
                retries++;
        }

        if (retries >= IIC_RETRY_NUM)
                printk("%s i2c read error: %d\n", __func__, ret);

        return ret;
}
/***********************************************************************************************
Name    :   icn83xx_i2c_txdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : send data to icn83xx , normal mode
***********************************************************************************************/
int icn83xx_i2c_txdata(unsigned char addr, char *txdata, int length)
{
        int ret = -1;
        unsigned char tmp_buf[128];
        int retries = 0;

        struct i2c_msg msg[] = {
                {
                        .addr   = this_client->addr,
                        .flags  = 0,
                        .len    = length + 1,
                        .buf    = tmp_buf,
                },
        };

        if (length > 125) {
                printk("%s too big datalen = %d!\n", __func__, length);
                return -1;
        }

        tmp_buf[0] = addr;

        if (length != 0 && txdata != NULL) {
                memcpy(&tmp_buf[1], txdata, length);
        }

        while(retries < IIC_RETRY_NUM) {
                ret = i2c_transfer(this_client->adapter, msg, 1);
                if(ret == 1)break;
                retries++;
        }

        if (retries >= IIC_RETRY_NUM)
                printk("%s i2c write error: %d\n", __func__, ret);

        return ret;
}

/***********************************************************************************************
Name    :   icn83xx_write_reg
Input   :   addr -- address
            para -- parameter
Output  :
function    :   write register of icn83xx, normal mode
***********************************************************************************************/
int icn83xx_write_reg(unsigned char addr, char para)
{
        char buf[3];
        int ret = -1;

        buf[0] = para;
        ret = icn83xx_i2c_txdata(addr, buf, 1);
        if (ret < 0) {
                printk("write reg failed! %#x ret: %d\n", buf[0], ret);
                return -1;
        }

        return ret;
}


/***********************************************************************************************
Name    :   icn83xx_read_reg
Input   :   addr
            pdata
Output  :
function    :   read register of icn83xx, normal mode
***********************************************************************************************/
int icn83xx_read_reg(unsigned char addr, char *pdata)
{
        int ret = -1;
        ret = icn83xx_i2c_rxdata(addr, pdata, 1);
        return ret;
}

#if SUPPORT_FW_UPDATE
/***********************************************************************************************
Name    :   icn83xx_log
Input   :   0: rawdata, 1: diff data
Output  :   err type
function    :   calibrate param
***********************************************************************************************/
int  icn83xx_log(char diff)
{
        char row = 0;
        char column = 0;
        int i, j;
        icn83xx_read_reg(160, &row);
        icn83xx_read_reg(161, &column);

        if(diff == 1) {
                icn83xx_readTP(row, column, (char *)&log_diffdata[0][0]);

                for(i=0; i<row; i++) {
                        for(j=0; j<column; j++) {
                                log_diffdata[i][j] = log_diffdata[i][j] - log_rawdata[i][j];
                        }
                }
                icn83xx_rawdatadump(&log_diffdata[0][0], row*16, 16);
        } else {
                icn83xx_readTP(row, column, (char *)&log_rawdata[0][0]);
                icn83xx_rawdatadump(&log_rawdata[0][0], row*16, 16);
        }
        return 0;
}
#endif

/***********************************************************************************************
Name    :   icn83xx_iic_test
Input   :   void
Output  :
function    : 0 success,
***********************************************************************************************/
static int icn83xx_iic_test(void)
{
        int  ret = -1;
        char value = 0;
        int  retry = 0;
        while(retry++ < 3) {
                ret = icn83xx_read_reg(0, &value);
                if(ret > 0)
                        return ret;
                printk("iic test error! %d\n", retry);
                msleep(3);
        }
        return ret;
}
/***********************************************************************************************
Name    :   icn83xx_ts_release
Input   :   void
Output  :
function    : touch release
***********************************************************************************************/
static void icn83xx_ts_release(void)
{
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        dprintk(DEBUG_X_Y_INFO,"==icn83xx_ts_release ==\n");
        input_report_abs(icn83xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
        input_sync(icn83xx_ts->input_dev);
}

/***********************************************************************************************
Name    :   icn83xx_report_value_A
Input   :   void
Output  :
function    : reprot touch ponit
***********************************************************************************************/
static void icn83xx_report_value_A(void)
{
        char buf[POINT_NUM*POINT_SIZE+3]={0};
        int ret = -1;
        int i;
#if TOUCH_VIRTUAL_KEYS
        unsigned char button;
        static unsigned char button_last;
#endif
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        dprintk(DEBUG_OTHERS_INFO,"==icn83xx_report_value_A ==\n");
        ret = icn83xx_i2c_rxdata(16, buf, POINT_NUM*POINT_SIZE+2);
        if (ret < 0) {
                printk("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
                return;
        }
#if TOUCH_VIRTUAL_KEYS
        button = buf[0];
        dprintk(DEBUG_OTHERS_INFO,"%s: button=%d\n",__func__, button);

        if((button_last != 0) && (button == 0)) {
                icn83xx_ts_release();
                button_last = button;
                return 1;
        }
        if(button != 0) {
                switch(button) {
                        case ICN_VIRTUAL_BUTTON_HOME:
                                dprintk(DEBUG_X_Y_INFO,"ICN_VIRTUAL_BUTTON_HOME down\n");
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 200);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_X, 280);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_Y, 1030);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
                                input_mt_sync(icn83xx_ts->input_dev);
                                input_sync(icn83xx_ts->input_dev);
                                break;
                        case ICN_VIRTUAL_BUTTON_BACK:
                                dprintk(DEBUG_X_Y_INFO,"ICN_VIRTUAL_BUTTON_BACK down\n");
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 200);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_X, 470);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_Y, 1030);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
                                input_mt_sync(icn83xx_ts->input_dev);
                                input_sync(icn83xx_ts->input_dev);
                                break;
                        case ICN_VIRTUAL_BUTTON_MENU:
                                dprintk(DEBUG_X_Y_INFO,"ICN_VIRTUAL_BUTTON_MENU down\n");
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 200);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_X, 100);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_Y, 1030);
                                input_report_abs(icn83xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
                                input_mt_sync(icn83xx_ts->input_dev);
                                input_sync(icn83xx_ts->input_dev);
                                break;
                        default:
                                dprintk(DEBUG_X_Y_INFO,"other gesture\n");
                                break;
                }
                button_last = button;
                return;
        }
#endif

        icn83xx_ts->point_num = buf[1];
        dprintk(DEBUG_OTHERS_INFO,"point_num is :%d\n",icn83xx_ts->point_num);
        if (icn83xx_ts->point_num == 0) {
                icn83xx_ts_release();
                return;
        }
        for(i=0;i<icn83xx_ts->point_num;i++)
                if(buf[8 + POINT_SIZE*i]  != 4) break ;

        if(i == icn83xx_ts->point_num) {
                icn83xx_ts_release();
                return;
        }

        for(i=0; i<icn83xx_ts->point_num; i++) {
                icn83xx_ts->point_info[i].u8ID = buf[2 + POINT_SIZE*i];
                icn83xx_ts->point_info[i].u16PosX = (buf[3 + POINT_SIZE*i]<<8) + buf[4 + POINT_SIZE*i];
                icn83xx_ts->point_info[i].u16PosY = (buf[5 + POINT_SIZE*i]<<8) + buf[6 + POINT_SIZE*i];
                icn83xx_ts->point_info[i].u8Pressure = 30;//buf[7 + POINT_SIZE*i];
                icn83xx_ts->point_info[i].u8EventId = buf[8 + POINT_SIZE*i];
                dprintk(DEBUG_X_Y_INFO,"source point: %d ===x = %d,y = %d, press = %d ====\n",
					i, icn83xx_ts->point_info[i].u16PosX,icn83xx_ts->point_info[i].u16PosY, icn83xx_ts->point_info[i].u8Pressure);
               if((0 == icn83xx_ts->point_info[i].u16PosX) && (0 == icn83xx_ts->point_info[i].u16PosY))
	                return;
	       if(1 == exchange_x_y_flag) {
                        unsigned short x = 0;
		       x = icn83xx_ts->point_info[i].u16PosX;
		       icn83xx_ts->point_info[i].u16PosX = icn83xx_ts->point_info[i].u16PosY;
		       icn83xx_ts->point_info[i].u16PosY = x;
                }
                if(1 == revert_x_flag)
                        icn83xx_ts->point_info[i].u16PosX = SCREEN_MAX_X- icn83xx_ts->point_info[i].u16PosX;

                if(1 == revert_y_flag)
                        icn83xx_ts->point_info[i].u16PosY = SCREEN_MAX_Y- icn83xx_ts->point_info[i].u16PosY;

                input_report_abs(icn83xx_ts->input_dev, ABS_MT_TRACKING_ID, icn83xx_ts->point_info[i].u8ID);
                input_report_abs(icn83xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, icn83xx_ts->point_info[i].u8Pressure);
                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_X, icn83xx_ts->point_info[i].u16PosX);
                input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_Y, icn83xx_ts->point_info[i].u16PosY);
                input_report_abs(icn83xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
                input_mt_sync(icn83xx_ts->input_dev);

                dprintk(DEBUG_X_Y_INFO,"report point: %d ===x = %d,y = %d, press = %d ====\n",
					i, icn83xx_ts->point_info[i].u16PosX,icn83xx_ts->point_info[i].u16PosY, icn83xx_ts->point_info[i].u8Pressure);
        }

        input_sync(icn83xx_ts->input_dev);

}

/***********************************************************************************************
Name    :   icn83xx_report_value_B
Input   :   void
Output  :
function    : reprot touch ponit
***********************************************************************************************/
#if CTP_REPORT_PROTOCOL
static void icn83xx_report_value_B(void)
{
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        char buf[POINT_NUM*POINT_SIZE+3]={0};
        static unsigned char finger_last[POINT_NUM + 1]={0};
        unsigned char  finger_current[POINT_NUM + 1] = {0};
        unsigned int position = 0;
        int temp = 0;
        int ret = -1;
        dprintk(DEBUG_OTHERS_INFO,"==icn83xx_report_value_B ==\n");

        ret = icn83xx_i2c_rxdata(16, buf, POINT_NUM*POINT_SIZE+2);
        if (ret < 0) {
                printk("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
                return ret;
        }

        icn83xx_ts->point_num = buf[1];
        if(icn83xx_ts->point_num > 0) {
                for(position = 0; position<icn83xx_ts->point_num; position++) {
                        temp = buf[2 + POINT_SIZE*position] + 1;
                        finger_current[temp] = 1;
                        icn83xx_ts->point_info[temp].u8ID = buf[2 + POINT_SIZE*position];
                        icn83xx_ts->point_info[temp].u16PosX = (buf[3 + POINT_SIZE*position]<<8) + buf[4 + POINT_SIZE*position];
                        icn83xx_ts->point_info[temp].u16PosY = (buf[5 + POINT_SIZE*position]<<8) + buf[6 + POINT_SIZE*position];
                        icn83xx_ts->point_info[temp].u8Pressure = buf[7 + POINT_SIZE*position];
                        icn83xx_ts->point_info[temp].u8EventId = buf[8 + POINT_SIZE*position];

                        if(icn83xx_ts->point_info[temp].u8EventId == 4)
                                finger_current[temp] = 0;

                        if(1 == revert_x_flag)
                                icn83xx_ts->point_info[temp].u16PosX = screen_max_x- icn83xx_ts->point_info[temp].u16PosX;

                        if(1 == revert_y_flag)
                                icn83xx_ts->point_info[temp].u16PosY = screen_max_y- icn83xx_ts->point_info[temp].u16PosY;

                        icn83xx_info("temp %d\n", temp);
                        icn83xx_info("u8ID %d\n", icn83xx_ts->point_info[temp].u8ID);
                        icn83xx_info("u16PosX %d\n", icn83xx_ts->point_info[temp].u16PosX);
                        icn83xx_info("u16PosY %d\n", icn83xx_ts->point_info[temp].u16PosY);
                        icn83xx_info("u8Pressure %d\n", icn83xx_ts->point_info[temp].u8Pressure);
                        icn83xx_info("u8EventId %d\n", icn83xx_ts->point_info[temp].u8EventId);
                        //icn83xx_info("u8Pressure %d\n", icn83xx_ts->point_info[temp].u8Pressure*16);
                }
        } else {
                for(position = 1; position < POINT_NUM+1; position++)
                        finger_current[position] = 0;
                dprintk(DEBUG_OTHERS_INFO,"no touch\n");
        }

        for(position = 1; position < POINT_NUM + 1; position++) {
                if((finger_current[position] == 0) && (finger_last[position] != 0)) {
                        input_mt_slot(icn83xx_ts->input_dev, position-1);
                        input_mt_report_slot_state(icn83xx_ts->input_dev, MT_TOOL_FINGER, false);
                        dprintk(DEBUG_X_Y_INFO,"one touch up: %d\n", position);
                }
                else if(finger_current[position]) {
                        input_mt_slot(icn83xx_ts->input_dev, position-1);
                        input_mt_report_slot_state(icn83xx_ts->input_dev, MT_TOOL_FINGER, true);
                        input_report_abs(icn83xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);
                        //input_report_abs(icn83xx_ts->input_dev, ABS_MT_PRESSURE, icn83xx_ts->point_info[position].u8Pressure);
                        input_report_abs(icn83xx_ts->input_dev, ABS_MT_PRESSURE, 200);
                        input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_X, icn83xx_ts->point_info[position].u16PosX);
                        input_report_abs(icn83xx_ts->input_dev, ABS_MT_POSITION_Y, icn83xx_ts->point_info[position].u16PosY);
                        dprintk(DEBUG_X_Y_INFO,"===position: %d, x = %d,y = %d, press = %d ====\n",
							position, icn83xx_ts->point_info[position].u16PosX,icn83xx_ts->point_info[position].u16PosY, icn83xx_ts->point_info[position].u8Pressure);
                }

        }
        input_sync(icn83xx_ts->input_dev);

        for(position = 1; position < POINT_NUM + 1; position++)
                finger_last[position] = finger_current[position];
}
#endif

/***********************************************************************************************
Name    :   icn83xx_ts_pen_irq_work
Input   :   void
Output  :
function    : work_struct
***********************************************************************************************/
static void icn83xx_ts_pen_irq_work(struct work_struct *work)
{
        int ret = -1;
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
#if SUPPORT_PROC_FS
        if(down_interruptible(&icn83xx_ts->sem))
                return -1;
#endif
        if(icn83xx_ts->work_mode == 0) {
#if CTP_REPORT_PROTOCOL
                icn83xx_report_value_B();
#else
                icn83xx_report_value_A();
#endif
                ret = input_set_int_enable(&(config_info.input_type), 1);
	       if (ret < 0)
	               printk("%s irq ensable failed\n", ICN83XX_NAME);
        }

#if SUPPORT_FW_UPDATE
        else if(icn83xx_ts->work_mode == 1){
                printk("log raw data\n");
                icn83xx_log(0);   //raw data
        } else if(icn83xx_ts->work_mode == 2){
                printk("log diff data\n");
                icn83xx_log(1);   //diff data
        }
#endif

#if SUPPORT_PROC_FS
        up(&icn83xx_ts->sem);
#endif
        dprintk(DEBUG_INT_INFO,"%s:ret:\n",__func__);
}

/***********************************************************************************************
Name    :   chipone_timer_func
Input   :   void
Output  :
function    : Timer interrupt service routine.
***********************************************************************************************/
static enum hrtimer_restart chipone_timer_func(struct hrtimer *timer)
{
        struct icn83xx_ts_data *icn83xx_ts = container_of(timer, struct icn83xx_ts_data, timer);
        queue_work(icn83xx_ts->ts_workqueue, &icn83xx_ts->pen_event_work);

        if(icn83xx_ts->use_irq == 1){
                if((icn83xx_ts->work_mode == 1) || (icn83xx_ts->work_mode == 2)) {
                        hrtimer_start(&icn83xx_ts->timer, ktime_set(CTP_POLL_TIMER/1000, (CTP_POLL_TIMER%1000)*1000000), HRTIMER_MODE_REL);
                }
        } else {
                hrtimer_start(&icn83xx_ts->timer, ktime_set(CTP_POLL_TIMER/1000, (CTP_POLL_TIMER%1000)*1000000), HRTIMER_MODE_REL);
        }

        return HRTIMER_NORESTART;
}

/***********************************************************************************************
Name    :   icn83xx_ts_interrupt
Input   :   void
Output  :
function    : interrupt service routine
*
**********************************************************************************************/
irqreturn_t icn83xx_ts_interrupt(int irq, void *dev_id)
{
        int ret = -1;
        struct icn83xx_ts_data *icn83xx_ts = (struct icn83xx_ts_data *)dev_id;
        ret = input_set_int_enable(&(config_info.input_type), 0);
        if (ret < 0)
		printk("%s irq dissable failed\n", ICN83XX_NAME);
        dprintk(DEBUG_INT_INFO,"==========icn83xx TS Interrupt============\n");
        queue_work(icn83xx_ts->ts_workqueue, &icn83xx_ts->pen_event_work);

        return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
/***********************************************************************************************
Name    :   icn83xx_ts_suspend
Input   :   void
Output  :
function    : tp enter sleep mode
***********************************************************************************************/
static void icn83xx_ts_suspend(struct early_suspend *handler)
{
        int ret = -1;
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        dprintk(DEBUG_SUSPEND,"\n#######################icn83xx_ts_suspend: write ICN83XX_REG_PMODE .#######################\n");
        if (icn83xx_ts->use_irq){
                ret = input_set_int_enable(&(config_info.input_type), 0);
                if (ret < 0)
                        printk("%s irq dissable failed\n", ICN83XX_NAME);
        } else {
                hrtimer_cancel(&icn83xx_ts->timer);
        }
        cancel_work_sync(&icn83xx_ts->pen_event_work);
        flush_workqueue(icn83xx_ts->ts_workqueue);
        icn83xx_write_reg(ICN83XX_REG_PMODE, PMODE_HIBERNATE);
        dprintk(DEBUG_SUSPEND,"\n####################### icn83xx_ts_suspend OVER .#######################\n");
}

/***********************************************************************************************
Name    :   icn83xx_ts_resume
Input   :   void
Output  :
function    : wakeup tp or reset tp
***********************************************************************************************/
static void icn83xx_ts_resume(struct early_suspend *handler)
{
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        int ret = -1;
        dprintk(DEBUG_SUSPEND,"==icn83xx_ts_resume== \n");

#if CTP_REPORT_PROTOCOL
        int i;
        for(i = 0; i < POINT_NUM; i++){
                input_mt_slot(icn83xx_ts->input_dev, i);
                input_mt_report_slot_state(icn83xx_ts->input_dev, MT_TOOL_FINGER, false);
        }
#else
	       icn83xx_ts_release();
#endif
	       ctp_wakeup(0, CTP_WAKEUP_LOW_PERIOD);
                msleep(CTP_WAKEUP_HIGH_PERIOD);
        if (icn83xx_ts->use_irq) {
	       ret = input_set_int_enable(&(config_info.input_type), 1);
	       if (ret < 0)
	              dprintk(DEBUG_SUSPEND,"%s irq ensable failed\n", ICN83XX_NAME);
        } else {
                hrtimer_start(&icn83xx_ts->timer, ktime_set(CTP_START_TIMER/1000, (CTP_START_TIMER%1000)*1000000), HRTIMER_MODE_REL);
        }
}
#else
/***********************************************************************************************
Name    :   icn83xx_ts_suspend
Input   :   void
Output  :
function    : tp enter sleep mode
***********************************************************************************************/
static int ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
        int ret = -1;
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        dprintk(DEBUG_SUSPEND,"\n#######################icn83xx_ts_suspend: write ICN83XX_REG_PMODE .#######################\n");
        if (icn83xx_ts->use_irq){
                ret = input_set_int_enable(&(config_info.input_type), 0);
                if (ret < 0)
                        printk("%s irq dissable failed\n", ICN83XX_NAME);
        } else {
                hrtimer_cancel(&icn83xx_ts->timer);
        }
        cancel_work_sync(&icn83xx_ts->pen_event_work);
        flush_workqueue(icn83xx_ts->ts_workqueue);
        icn83xx_write_reg(ICN83XX_REG_PMODE, PMODE_HIBERNATE);
        dprintk(DEBUG_SUSPEND,"\n####################### icn83xx_ts_suspend OVER .#######################\n");
	return 0;
}

/***********************************************************************************************
Name    :   icn83xx_ts_resume
Input   :   void
Output  :
function    : wakeup tp or reset tp
***********************************************************************************************/
static int ts_resume(struct i2c_client *client)
{
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(this_client);
        int ret = -1;
        dprintk(DEBUG_SUSPEND,"==icn83xx_ts_resume== \n");

#if CTP_REPORT_PROTOCOL
        int i;
        for(i = 0; i < POINT_NUM; i++){
                input_mt_slot(icn83xx_ts->input_dev, i);
                input_mt_report_slot_state(icn83xx_ts->input_dev, MT_TOOL_FINGER, false);
        }
#else
	       icn83xx_ts_release();
#endif
	       ctp_wakeup(0, CTP_WAKEUP_LOW_PERIOD);
                msleep(CTP_WAKEUP_HIGH_PERIOD);
        if (icn83xx_ts->use_irq) {
	       ret = input_set_int_enable(&(config_info.input_type), 1);
	       if (ret < 0)
	              dprintk(DEBUG_SUSPEND,"%s irq ensable failed\n", ICN83XX_NAME);
        } else {
                hrtimer_start(&icn83xx_ts->timer, ktime_set(CTP_START_TIMER/1000, (CTP_START_TIMER%1000)*1000000), HRTIMER_MODE_REL);
        }
	return 0;
}
#endif

/***********************************************************************************************
Name    :   icn83xx_request_input_dev
Input   :   void
Output  :
function    : 0 success,
***********************************************************************************************/
static int icn83xx_request_input_dev(struct icn83xx_ts_data *icn83xx_ts)
{
        int ret = -1;
        struct input_dev *input_dev;

        input_dev = input_allocate_device();
        if (!input_dev) {
                printk("failed to allocate input device\n");
                return -ENOMEM;
        }
        icn83xx_ts->input_dev = input_dev;

        icn83xx_ts->input_dev->evbit[0] =  BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;//BIT_MASK(EV_SYN) |
#if CTP_REPORT_PROTOCOL
        __set_bit(INPUT_PROP_DIRECT, icn83xx_ts->input_dev->propbit);
        input_mt_init_slots(icn83xx_ts->input_dev, 255);
#else
        set_bit(ABS_MT_TOUCH_MAJOR, icn83xx_ts->input_dev->absbit);
        set_bit(ABS_MT_POSITION_X, icn83xx_ts->input_dev->absbit);
        set_bit(ABS_MT_POSITION_Y, icn83xx_ts->input_dev->absbit);
        set_bit(ABS_MT_WIDTH_MAJOR, icn83xx_ts->input_dev->absbit);
#endif
        input_set_abs_params(icn83xx_ts->input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
        input_set_abs_params(icn83xx_ts->input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
        input_set_abs_params(icn83xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
        input_set_abs_params(icn83xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
        input_set_abs_params(icn83xx_ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
        set_bit(INPUT_PROP_DIRECT, icn83xx_ts->input_dev->propbit);

        __set_bit(KEY_MENU,  input_dev->keybit);
        __set_bit(KEY_BACK,  input_dev->keybit);
        __set_bit(KEY_HOME,  input_dev->keybit);
        __set_bit(KEY_SEARCH,  input_dev->keybit);

        input_dev->name = CTP_NAME;
        ret = input_register_device(input_dev);
        if (ret) {
                printk("Register %s input device failed\n", input_dev->name);
                input_free_device(input_dev);
                return -ENODEV;
        }
		
        return 0;
}

char FbCap[4][16]={
        {0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14},
        {0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12},
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
        {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
        };

static int icn83xx_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
        struct icn83xx_ts_data *icn83xx_ts;
        int err = 0;
#if SUPPORT_FW_UPDATE
        short fwVersion = 0;
        short curVersion = 0;
        int retry;
#endif
        dprintk(DEBUG_INIT,"\n %s begin: addr = 0x%02x .  \n", __func__, client->addr);

        if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
                printk("\n %s I2C check functionality failed. \n",__func__);
                return -ENODEV;
        }

        icn83xx_ts = kzalloc(sizeof(*icn83xx_ts), GFP_KERNEL);
        if (!icn83xx_ts) {
                printk("Alloc icn83xx_ts memory failed.\n");
                return -ENOMEM;
        }
        memset(icn83xx_ts, 0, sizeof(*icn83xx_ts));

        this_client = client;
        this_client->addr = client->addr;
        i2c_set_clientdata(client, icn83xx_ts);

        icn83xx_ts->work_mode = 0;
        dprintk(DEBUG_INIT,"work_mode:%d\n",icn83xx_ts->work_mode);

        INIT_WORK(&icn83xx_ts->pen_event_work, icn83xx_ts_pen_irq_work);
        icn83xx_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
        if (!icn83xx_ts->ts_workqueue) {
                printk("create_singlethread_workqueue failed.\n");
                kfree(icn83xx_ts);
                return -ESRCH;
        }

        err = icn83xx_iic_test();
        if (err < 0) {
                printk("icn83xx_iic_test  failed.\n");
#if SUPPORT_FW_UPDATE

#if COMPILE_FW_WITH_DRIVER
                icn83xx_set_fw(sizeof(icn83xx_fw), &icn83xx_fw[0]);
#endif
                if(icn83xx_check_progmod() == 0) {
                        retry = 5;
                        dprintk(DEBUG_INIT,"in prog mode\n");
                        while(retry > 0) {
                                if(R_OK == icn83xx_fw_update(firmware)) {
                                        break;
                                }
                                retry--;
                                printk("icn83xx_fw_update failed.\n");
                        }
                } else {
                        printk("I2C communication failed.\n");
                        kfree(icn83xx_ts);
                        return -1;
                }
#endif
        } else {
                dprintk(DEBUG_INIT,"iic communication ok\n");
        }

#if SUPPORT_FW_UPDATE
        fwVersion = icn83xx_read_fw_Ver(firmware);
        curVersion = icn83xx_readVersion();
        dprintk(DEBUG_INIT,"fwVersion : 0x%x\n", fwVersion);
        dprintk(DEBUG_INIT,"current version: 0x%x\n", curVersion);

#if FORCE_UPDATA_FW
        retry = 5;
        while(retry > 0){
                if(R_OK == icn83xx_fw_update(firmware)){
                        break;
                }
                retry--;
                printk("icn83xx_fw_update failed.\n");
        }
#else
        if(fwVersion > curVersion) {
                retry = 5;
                while(retry > 0) {
                        if(R_OK == icn83xx_fw_update(firmware)) {
                                break;
                        }
                        retry--;
                        printk("icn83xx_fw_update failed.\n");
                 }
        }
#endif
#endif

#if SUPPORT_FW_CALIB
        err = icn83xx_read_reg(0, &value);
        if(err > 0) {
                //auto calib fw
                average = icn83xx_calib(0, NULL);
                //fix FbCap
                //average = icn83xx_calib(0, FbCap[1]);
                dprintk(DEBUG_INIT,"average : %d\n", average);
                icn83xx_setPeakGroup(250, 150);
                icn83xx_setDownUp(400, 300);
        }
#endif

       err= icn83xx_request_input_dev(icn83xx_ts);
       if (err < 0) {
	       printk("request input dev failed\n");
	       kfree(icn83xx_ts);
	       return err;
       }


#if TOUCH_VIRTUAL_KEYS
        icn83xx_ts_virtual_keys_init();
#endif
        config_info.dev = &(icn83xx_ts->input_dev->dev);
        int_handle = input_request_int(&(config_info.input_type), icn83xx_ts_interrupt,CTP_IRQ_MODE, icn83xx_ts);
        if (int_handle) {
                printk("icn83xx_ts_probe: request irq failed\n");
                input_free_int(&(config_info.input_type), icn83xx_ts);
                icn83xx_ts->use_irq = 0;
                hrtimer_init(&icn83xx_ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
                icn83xx_ts->timer.function = chipone_timer_func;
                hrtimer_start(&icn83xx_ts->timer, ktime_set(CTP_START_TIMER/1000, (CTP_START_TIMER%1000)*1000000), HRTIMER_MODE_REL);
        }else {
	        icn83xx_ts->use_irq = 1;
	        printk("icn83xx_ts_probe: request irq sucesse\n");
        }

#if SUPPORT_SYSFS
        icn83xx_create_sysfs(client);
#endif

#if SUPPORT_PROC_FS
        sema_init(&icn83xx_ts->sem, 1);
        init_proc_node();
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	printk("==register_early_suspend =\n");
	icn83xx_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	icn83xx_ts->early_suspend.suspend = icn83xx_ts_suspend;
	icn83xx_ts->early_suspend.resume  = icn83xx_ts_resume;
	register_early_suspend(&icn83xx_ts->early_suspend);
#endif

        printk("==%s over =\n", __func__);

        return 0;

}

static int __devexit icn83xx_ts_remove(struct i2c_client *client)
{
        struct icn83xx_ts_data *icn83xx_ts = i2c_get_clientdata(client);
        printk("==icn83xx_ts_remove==\n");
#ifdef CONFIG_HAS_EARLYSUSPEND
        unregister_early_suspend(&icn83xx_ts->early_suspend);
#endif

#if SUPPORT_PROC_FS
        uninit_proc_node();
#endif
        if(1 == icn83xx_ts->use_irq)
                input_free_int(&(config_info.input_type), icn83xx_ts);
        input_unregister_device(icn83xx_ts->input_dev);

        cancel_work_sync(&icn83xx_ts->pen_event_work);
        destroy_workqueue(icn83xx_ts->ts_workqueue);

        kfree(icn83xx_ts);
        i2c_set_clientdata(client, NULL);

        return 0;
}

static const struct i2c_device_id icn83xx_ts_id[] = {
        { CTP_NAME, 0 },
        {}
};
MODULE_DEVICE_TABLE(i2c, icn83xx_ts_id);

static struct i2c_driver icn83xx_ts_driver = {
        .class      = I2C_CLASS_HWMON,
        .probe      = icn83xx_ts_probe,
        .remove     = __devexit_p(icn83xx_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend    = ts_suspend,
	.resume     = ts_resume,
#endif
        .id_table   = icn83xx_ts_id,
        .driver = {
                .name   = CTP_NAME,
                .owner  = THIS_MODULE,
        },
        .address_list   = normal_i2c,
};

static int ctp_get_system_config(void)
{
        ctp_print_info(config_info,DEBUG_INIT);
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

static int __init icn83xx_ts_init(void)
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

        ctp_wakeup(0, CTP_WAKEUP_LOW_PERIOD);
        icn83xx_ts_driver.detect = ctp_detect;

        ret = i2c_add_driver(&icn83xx_ts_driver);
        return ret;
}

static void __exit icn83xx_ts_exit(void)
{
        printk("==icn83xx_ts_exit==\n");
        i2c_del_driver(&icn83xx_ts_driver);
        input_free_platform_resource(&(config_info.input_type));
}

module_init(icn83xx_ts_init);
module_exit(icn83xx_ts_exit);

MODULE_AUTHOR("<zmtian@chiponeic.com>");
MODULE_DESCRIPTION("Chipone icn83xx TouchScreen driver");
MODULE_LICENSE("GPL");

