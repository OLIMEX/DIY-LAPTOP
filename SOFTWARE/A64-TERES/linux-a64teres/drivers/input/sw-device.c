/*
 *  sw_device.c - Linux kernel modules for  Detection i2c device.
 */
 

#include "sw-device.h"

static int ctp_mask = 0x0;
static u32 debug_mask = 0x0;
#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	printk(KERN_DEBUG fmt , ## arg)

module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(ctp_mask, ctp_mask, int , S_IRUGO | S_IWUSR | S_IWGRP);

/*gsensor info*/
static struct sw_device_info gsensors[] = {
        {   "lsm9ds0_acc_mag", 0, {0x1e, 0x1d            }, 0x0f, {0x49          }, 0},    
        {    "bma250", 0, {0x18, 0x19, 0x08, 0x38}, 0x00, {0x02,0x03,0xf9,0xf8}, 0},
        {   "stk831x", 0, {0x3d, 0x22            }, 0x00, {0x00               }, 1},
        {   "mma8452", 0, {0x1c, 0x1d            }, 0x0d, {0x2A          }, 0},
        {   "mma7660", 0, {0x4c                  }, 0x00, {0x00          }, 0},
        {   "mma865x", 0, {0x1d                  }, 0x0d, {0x4A,0x5A     }, 0},
        {   "mc32x0" , 0, {0x4c                  }, 0x00, {0x00               }, 1},
        {    "afa750", 0, {0x3d                  }, 0x37, {0x3d,0x3c     }, 0},
        {"lis3de_acc", 0, {0x28, 0x29            }, 0x0f, {0x33          }, 0},
        {"lis3dh_acc", 0, {0x18, 0x19            }, 0x0f, {0x33          }, 0},
        {     "kxtik", 0, {0x0f                  }, 0x0f, {0x05,0x08     }, 0},
        {   "dmard10", 0, {0x18                  }, 0x00, {0x00          }, 1},
        {   "dmard06", 0, {0x1c                  }, 0x0f, {0x06          }, 0},
        {   "mxc622x", 0, {0x15                  }, 0x00, {0x00          }, 0},    
        {  "fxos8700", 0, {0x1c, 0x1d, 0x1e, 0x1f}, 0x0d, {0xc7          }, 0},
        {   "lsm303d", 0, {0x1e, 0x1d            }, 0x0f, {0x49          }, 0},    
        {   "sc7a30",  0, {0x1D                  }, 0x2A, {0x00          }, 0},
};
/*ctp info*/
static struct sw_device_info ctps[] = {
        { "ft5x_ts", 0, {      0x38},   0xa3, {0x3,0x0a,0x55,0x06,0x08,0x02,0xa3}, 0},
        {   "gt82x", 0, {      0x5d},  0xf7d, {0x13,0x27,0x28          }, 0},
        { "gslX680", 0, {      0x40},   0x00, {0x00                    }, 1},
        {"gslX680new", 0, {    0x40},   0x00, {0x00                    }, 1},
        {"gt9xx_ts", 0, {0x14, 0x5d}, 0x8140, {0x39                    }, 0},
	{"gt9xxnew_ts", 0, {0x14, 0x5d}, 0x8140, {0x39,0x60,0xe0,0x10,                    }, 0},
        {"gt9xxf_ts", 0, { 0x14,0x5d},   0x00, {0x00                    }, 1},
        {   "tu_ts", 0, {      0x5f},   0x00, {0x00                    }, 1},
        {"gt818_ts", 0, {      0x5d},  0x715, {0xc3                    }, 0},
        { "zet622x", 0, {      0x76},   0x00, {0x00                    }, 0},
        {"aw5306_ts", 0, {     0x38},   0x01, {0xA8                    }, 0},
        {"icn83xx_ts", 0,{     0x40},   0x00, {0x00                    }, 1},
};

static struct sw_device_info lsensors[] = {
        {"ltr_501als", 0, {0x23   },    0x87, {0x05                    }, 0},
        {"jsa1212",    0, {0x39,0x29,0x44},  0x00, {0x00                    }, 1},
        {"jsa1127",    0, {0x39,0x29,0x44},  0x00, {0x00                    }, 1},
        {"stk3x1x",    0, {0x48        },    0x00, {0x00                    }, 1},
};

static struct sw_device_info gyr_sensors[] = {
        {"lsm9ds0_gyr", 0, {0x6a, 0x6b}, 0x0F, {0x00D4                  }, 0},
        {"l3gd20_gyr", 0, {0x6a, 0x6b}, 0x0F, {0x00D4                  }, 0},
	{"bmg160_gyr", 0,{0x68      }, 0x00, {0x00                    }, 0},
};

static struct sw_device_info compass_sensors[] = {
        {"lsm9ds0_acc_mag", 0, {0x1e, 0x1d            }, 0x0f, {0x49          }, 0}, 
        {"lsm303d", 0, {0x1e, 0x1d            }, 0x0f, {0x49          }, 0}, 
};

/*lsensors para_name info*/
static struct para_name ls_name = {
        "lightsensor",
        "lightsensor_used",
        "ls_list",
        "ls_list_used",
        "ls_twi_id",
        LSENSOR_DEVICE_KEY_NAME,
};

/*compass sensors para_name info*/
static struct para_name compass_name = {
        "compasssensor",
        "compasssensor_used",
        "compass_list",
        "compass_list_used",
        "compass_twi_id",
        COMPASS_SENSOR_DEVICE_KEY_NAME,
};

/*gyr para_name info*/
static struct para_name gyr_name = {
        "gyroscopesensor",
        "gyroscopesensor_used",
        "gy_list",
        "gy_list_used",
        "gy_twi_id",
        GYR_SENSOR_DEVICE_KEY_NAME,
};

/*gsensor para_name info*/
static struct para_name g_name = {
        "gsensor",
        "gsensor_used",
        "gsensor_list",
        "gsensor_list__used",
        "gsensor_twi_id",
        GSENSOR_DEVICE_KEY_NAME,
};

/*ctp para_name info*/
static struct para_name c_name = {
        "ctp",
        "ctp_used",
        "ctp_list",
        "ctp_list_used",
        "ctp_twi_id",
        CTP_DEVICE_KEY_NAME,
};
static struct para_power c_power = {
	.keyname             = "ctp",
	.power_ldo_name      = "ctp_power_ldo",
	.power_ldo_vol_name  = "ctp_power_ldo_vol",
	.power_io_name       = "ctp_power_io",
	.reset_pin_name      = "ctp_wakeup",
	.power_ldo           = NULL,
	.ldo                 = NULL,
};
	
static struct para_name *all_device_name[] = {&g_name,&c_name,&ls_name,&gyr_name,&compass_name};
#define	ALL_DEVICE_NAME_SIZE	(sizeof(all_device_name)/sizeof(all_device_name[0]))

static struct node_pointer all_device_np[2*ALL_DEVICE_NAME_SIZE];
#define ALL_DEVICE_NP_SIZE		(sizeof(all_device_np)/sizeof(all_device_np[0]))

	
static struct sw_device_name d_name = {"", "", "", "", 0, 0, 0, 0};
static void sw_devices_events(struct work_struct *work);
static struct workqueue_struct *sw_wq;
static DECLARE_WORK(sw_work, sw_devices_events);

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

static int i2c_read_bytes_addr16(struct i2c_client *client, uint8_t *buf, uint16_t len)
{
	struct i2c_msg msgs[2];
	int ret=-1;

	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr  = client->addr;
	msgs[0].len   = 2;		//data address
	msgs[0].buf   = buf;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr  = client->addr;
	msgs[1].len   = len-2;
	msgs[1].buf   = buf+2;
	
	ret=i2c_transfer(client->adapter, msgs, 2);
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

static int i2c_check_addr(char check_addr[], unsigned short address)
{
        int ret = 0;
        
        while((check_addr[ret]) && (ret < NAME_LENGTH)) {
                if(check_addr[ret] == address){
                        dprintk(DEBUG_INIT, "address:0x%x\n", check_addr[ret]);
                        return 1;
                }
                 ret++;              
        }
        
        return 0;
}

/**
 * ctp_wakeup - function
 *
 */
static int ctp_wakeup(struct gpio_config gpio, int ms)
{
	int status = 0;

	if(0 != gpio_request(gpio.gpio, NULL)) {
		printk(KERN_ERR "wakeup gpio_request is failed\n");
		return -1;
	}

	if (0 != gpio_direction_output(gpio.gpio, gpio.data)) {
		printk(KERN_ERR "wakeup gpio set err!");
		return -1;
	}

	printk("%s: !!!!gpio data %d\n", __func__, gpio.data);

	if(!(gpio.data))
		status = 1;

	if(status == 0){               
		if(ms == 0) { 
		        __gpio_set_value(gpio.gpio, 0);
		}else {
		        __gpio_set_value(gpio.gpio, 0);
		        msleep(ms);
		        __gpio_set_value(gpio.gpio, 1);
		}
	}
	if(status == 1){
		if(ms == 0) {
		        __gpio_set_value(gpio.gpio, 1);
		}else {
		        __gpio_set_value(gpio.gpio, 1); 
		        msleep(ms);
		        __gpio_set_value(gpio.gpio, 0); 
		}      
	}

	msleep(5);

	gpio_free(gpio.gpio);

	return 0;
}


static int get_power_para(struct para_power *pm)
{

	int ret = -1;

        
	struct device_node *np = NULL; 

	np = find_np_by_name(pm->keyname);
	if(!np){
		printk("get ctp_para failed, %d\n", ret);
		return -1;
	}
		
	ret = of_property_read_string(np, pm->power_ldo_name, &pm->power_ldo);
	if (ret) {
		 printk("get ctp_power_ldo failed, %d\n", ret);
	}

	ret = of_property_read_u32(np, pm->power_ldo_vol_name, &pm->power_ldo_vol);
	if (ret) {
		 printk("get ctp_power_ldo_vol failed, %d\n", ret);
	}

	pm->power_io.gpio = of_get_named_gpio_flags(np, pm->power_io_name, 0, (enum of_gpio_flags *)(&(pm->power_io)));	
	if (!gpio_is_valid(pm->power_io.gpio))
		printk("get ctp_power_io is invalid. \n");

	pm->reset_pin.gpio = of_get_named_gpio_flags(np, pm->reset_pin_name, 0, (enum of_gpio_flags *)(&(pm->reset_pin)));	 
	if (!gpio_is_valid(pm->reset_pin.gpio))
		printk("get ctp_power_io is invalid. \n");

	dprintk(DEBUG_INIT, "[sw_device]:%s: power_ldo = %s,power_ldo_vol = %d,"
			"power_io = %d,reset_pin = %d\n", __func__,
			pm->power_ldo,pm->power_ldo_vol,pm->power_io.gpio,pm->reset_pin.gpio);
	return 0;
}

/*
*********************************************************************************************************
*                                   i2c_get_chip_id_value_addr16
*
*Description: By reading chip_id register for 16 bit address, get chip_id value
*
*Arguments  :address     Register address
*Return     : result;
*             Chip_id value
*********************************************************************************************************
*/
static int i2c_get_chip_id_value_addr16(unsigned short address, struct i2c_client* temp_client)
{
        int value = -1;
        uint8_t buf[5] = {0};
        int retry = 2;
        
        if(address & 0xff00) {
                buf[0] = address >> 8;
                buf[1] = address & 0x00ff;
        }
        
        while(retry--) {
                value = i2c_read_bytes_addr16(temp_client, buf, 3);
                if(value != 2){
                        msleep(1);
                        printk("%s:read chip id error!", __func__);
                }else{
                        break;
                }
        }
        
        value = buf[2] & 0xffff;
        dprintk(DEBUG_INIT, "buf[0]:0x%x, buf[1]:0x%x, buf[2]:0x%x, value:0x%x\n",
                buf[0], buf[1], buf[2], value);
        
        return value;     
}
/*
*********************************************************************************************************
*                                   i2c_get_chip_id_value_addr8
*
*Description: By reading chip_id register for 8 bit address, get chip_id value
*
*Arguments  :address     Register address
*Return     : result;
*             Chip_id value
*********************************************************************************************************
*/
static int i2c_get_chip_id_value_addr8(unsigned short address, struct i2c_client* temp_client)
{
        int value = -1;
        int retry = 2;
        
        while(retry--) {
                value = i2c_smbus_read_byte_data(temp_client, address);
                if(value < 0){
                        msleep(1);
                }else { 
                        break;
                }
        }
        value = value & 0xffff;

        return value;
}


static int is_alpha(char chr)
{
        int ret = -1;
        
        ret = ((chr >= 'a') && (chr <= 'z') ) ? 0 : 1;
                
        return ret;
}

static int  get_device_name(char *buf, char * name)
{
        int s1 = 0, i = 0;
        int ret = -1;
        char ch = '\"';
        char tmp_name[64];
        char * str1;
        char * str2;
        
        memset(&tmp_name, 0, sizeof(tmp_name));
        if(!strlen (buf)){
                printk("%s:the buf is null!\n", __func__);
                return 0; 
        }
        
        str1 = strchr(buf, ch);
        str2 = strrchr(buf, ch);
        if((str1 == NULL) || (str2 == NULL)) {
                printk("the str1 or str2 is null!\n");
                return 1;
        }
                   
        ret = str1 - buf + 1;  
        s1 =  str2 - buf; 
        dprintk(DEBUG_INIT,"----ret : %d,s1 : %d---\n ", ret, s1);
        
        while(ret != s1)
        {
                tmp_name[i++] = buf[ret++];         
         
        }
        tmp_name[i] = '\0';
        strcpy(name, tmp_name);
        
        dprintk(DEBUG_INIT, "name:%s\n", name);
        return 1;
}  

static int get_device_name_info(struct sw_device *sw)
{
         int row_number = 0;
         
         
         row_number = sw->total_raw;
	 while(row_number--) {
	        dprintk(DEBUG_INIT, "config_info[%d].str_info:%s\n", row_number, 
	                sw->write_info[row_number].str_info);
	    
	        if(is_alpha(sw->write_info[row_number].str_info[0])) {
	                continue;
	        } else if(!strncmp(sw->write_info[row_number].str_info, sw->name->write_key_name, 
	                strlen(sw->name->write_key_name))){
	                        
	                dprintk(DEBUG_INIT, "info:%s, key_name:%s\n", sw->write_info[row_number].str_info,
	                         sw->name->write_key_name);
	                         
	                if(sw->write_info[0].str_id == sw->write_info[1].str_id)          
	                        sw->write_id = row_number;
	                else
	                        sw->write_id = sw->write_info[row_number].str_id;
	               
	                if(get_device_name(sw->write_info[row_number].str_info, sw->device_name)) {
	                        dprintk(DEBUG_INIT, "device_name:%s,write_id:%d\n", sw->device_name,
	                                 sw->write_id);
	                        return 0; 
	                }	                
	        }
	 }
	 return -1;
}
static int get_device_same_addr(struct sw_device *sw, int now_number)
{
        int ret = -1;
        int i = 0;
        int number = sw->support_number;
        int scan_number = 0;
               
        while(scan_number < number) {
                scan_number++;
                i = 0; 
                now_number = (now_number == number) ? 0 : now_number; 
                        
                while((sw->info[now_number].i2c_address[i++]) && (i < ADDRESS_NUMBER)) {
                        dprintk(DEBUG_INIT, "addr:0x%x, response_addr:0x%x\n", 
                                sw->info[now_number].i2c_address[i -1], sw->response_addr);	                
                        if( sw->info[now_number].i2c_address[i - 1] == sw->response_addr) {
	                        dprintk(DEBUG_INIT, "return number: %d \n", now_number);
                                return now_number;
	                }      
	                  
                } 
                
                now_number++;                  
        }
        dprintk(DEBUG_INIT, "-----not find !-----\n");
        return ret;
}

static int get_device_name_number(struct sw_device *sw)
{
        int ret = -1;
        int number = sw->support_number;
        if(strlen(sw->device_name)) {
                while((number)--) {
                        if (!strncmp(sw->device_name, sw->info[number].name,
                             strlen(sw->info[number].name))) {
                                dprintk(DEBUG_INIT, "number: %d \n", number);
                                return number;
                        }
                }
        }
		
        dprintk(DEBUG_INIT, "-----the name is null !-----\n");
        return ret;
}

static int get_device_para_value(char* keyname, char* subname)
{
        
		struct device_node *np = NULL;
		int val = 0;

		np = find_np_by_name(keyname);
		if(!np){
			printk("get node %s failed\n", keyname);
			goto script_get_item_err;
		}
			
		if(strstr(subname,"used")){
			if (!of_device_is_available(np)) {
			     goto script_get_item_err;
			}
			val = 1;

		}else{
			if (of_property_read_u32(np, subname, &val)) {
				 goto script_get_item_err;
			}
		}
		return val;


        
script_get_item_err:
        dprintk(DEBUG_INIT, "keyname:%s  subname:%s ,get error!\n", keyname, subname);
	return -1;
}


static void get_detect_list(struct sw_device *sw)
{
        int i = 0;
        int val = -1;
        int number = sw->support_number;
        while((number)--) {
                i = 0;
				printk("%s name = %s\n",__FUNCTION__,sw->info[number].name);
                val = get_device_para_value(sw->name->detect_keyname,  sw->info[number].name);
                if(val == -1) {
			        sw->info[number].is_support = 0;
	                printk("%s: script_get_item err.support_number = %d. \n", __func__, number);
	                continue;
	        } 
		
		if(val == 0) {
			sw->info[number].is_support = 0;
		}
		else {
			sw->info[number].is_support = 1;	
		}
        	dprintk(DEBUG_INIT, "number %d, module_name:%s  is_support:%u\n", number,sw->info[number].name, sw->info[number].is_support);
        } 
 }


static int sw_sysconfig_get_para(struct sw_device *sw) 
{
        int ret = -1;
        int device_used = 0;
		
	dprintk(DEBUG_INIT, "========%s===================\n", __func__);
	 
     device_used = get_device_para_value(sw->name->used_keyname, sw->name->used_subname); 
	if(1 == device_used ){               
        	sw->twi_id = get_device_para_value(sw->name->used_keyname, sw->name->twi_id_name);
        	dprintk(DEBUG_INIT, "%s: device_twi_id is %d. \n", __func__, sw->twi_id);
        
	        sw->detect_used = get_device_para_value(sw->name->detect_keyname, sw->name->detect_subname);
	        if(sw->detect_used >= 0) {
                  get_detect_list(sw);
                  ret = 0;
                } else {
                  ret = -1;
                }
	
	}else{
	        dprintk(DEBUG_INIT, "%s: device_unused. \n",  __func__);
		ret = -1;
	}

	return ret;
}
static int sw_analytic_write_info(char * src_string, struct sw_device *sw)
{
        int ret = -1;
        int i = 0, j = 0, k = 0;
        sw->total_raw = 0;
        
        dprintk(DEBUG_INIT, "%s:strlen(src_string):%zu\n", src_string, strlen(src_string));
        if(strlen(src_string) < 16 ) {
                sw->total_raw = DEFAULT_TOTAL_ROW;
                dprintk(DEBUG_INIT, "%s: the src string is null !\n", __func__);
                ret = 0;
                return ret;
        }  
               
        while(src_string[i++]) {  
                sw->write_info[k].str_info[j++] = src_string[i-1];
                
                if(src_string[i-1] == '\n') {
                        (sw->total_raw)++; 
                        sw->write_info[k].str_info[j] = '\0';
                        sw->write_info[k].str_id = k;         
                        k += 1;
                        j = 0;
                    
                }   
        } 
        
        if(src_string[strlen(src_string)] != '\n') {
                (sw->total_raw)++; 
                sw->write_info[k].str_id = k;
        } 
                      
        dprintk(DEBUG_INIT, "%s:total_raw:%d\n", __func__, sw->total_raw);
        ret = 1;
        
        return ret;

}
static int sw_get_write_info(char * tmp, struct sw_device *sw)
{
        mm_segment_t old_fs;
        int ret;
        sw->filp = filp_open(FILE_DIR,O_RDWR | O_CREAT, 0666);
        if(!sw->filp || IS_ERR(sw->filp)) {
                printk("%s:open error ....IS(filp):%ld\n", __func__, IS_ERR(sw->filp));
                return -1;
        } 
        
        old_fs = get_fs();
        set_fs(get_ds());
        sw->filp->f_op->llseek(sw->filp, 0, 0);
        ret = sw->filp->f_op->read(sw->filp, tmp, FILE_LENGTH, &sw->filp->f_pos);
        
        if(ret <= 0) {
                printk("%s:read erro or read null !\n", __func__);
        }
        
        set_fs(old_fs);
        filp_close(sw->filp, NULL);
        
        return ret;

}

static int sw_set_write_info(struct sw_device *sw)
{
        mm_segment_t old_fs;
        int ret = 0, i =0;
        
        sw->filp = filp_open(FILE_DIR, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if(!sw->filp || IS_ERR(sw->filp)) {
                printk("%s:open error ....IS(filp):%ld\n", __func__,IS_ERR(sw->filp));
                return -1;
        } 
        
        old_fs = get_fs();
        set_fs(get_ds());
        
        sw->filp->f_op->llseek(sw->filp, 0, 0);
        
        while(i < sw->total_raw ) {
                dprintk(DEBUG_INIT, "write info:%s\n", sw->write_info[i].str_info);
                ret = sw->filp->f_op->write(sw->filp, sw->write_info[i].str_info,
                        strlen(sw->write_info[i].str_info), &sw->filp->f_pos);
                i++;
        }
        
        set_fs(old_fs);
        filp_close(sw->filp, NULL);
        return ret;
}

static bool sw_match_chip_id(struct sw_device_info *info, int number, int value)
{
        int i = 0;
        while(info[number].id_value[i])
        {
                if(info[number].id_value[i] == value) {
                        dprintk(DEBUG_INIT, "Chip id value equal!\n");
                        return true;
                }
                i++;
        }
        dprintk(DEBUG_INIT, "Chip id value does not match!--value:%d--\n", value);
        return false;
}

static int sw_chip_id_gain(struct sw_device *sw, int now_number)
{
        unsigned short reg_addr;
        int match_value = 0;
        int id_value = -1;
        reg_addr = sw->info[now_number].chip_id_reg;        
        if(reg_addr & 0xff00)
                id_value = i2c_get_chip_id_value_addr16(reg_addr, sw->temp_client);
        else
                id_value = i2c_get_chip_id_value_addr8(reg_addr, sw->temp_client);
       
        dprintk(DEBUG_INIT, "-----%s:chip_id_reg:0x%x, chip_id_value = 0x%x-----\n",
                __func__, reg_addr, id_value);
        match_value = sw_match_chip_id(sw->info, now_number, id_value);
        
        return match_value;
}

static int sw_chip_id_detect(struct sw_device *sw, int now_number)
{
        int result = -1;
        int same_number = 0;
        while (!(sw->info[now_number].id_value[0])) {                
                if(sw->info[now_number].same_flag) {
                        same_number = get_device_same_addr(sw, now_number + 1);
                        while((same_number != now_number) && (sw->info[same_number].is_support==1) && (same_number != -1) &&
                              (sw->info[same_number].id_value[0])) {
                                result = sw_chip_id_gain(sw, same_number); 
                                if(result) {
                                        sw->current_number = same_number;
                                        return result; 
                                } 
                               same_number = get_device_same_addr(sw, same_number + 1);               
                        }  
                }              
                result = 1;
                dprintk(DEBUG_INIT, "-----%s:chip_id_reg value:0x%x",
                        __func__, sw->info[now_number].id_value[0]);
                return result;
        }        
        result = sw_chip_id_gain(sw, now_number);         
        return result;
}

static int sw_device_response_test(struct sw_device *sw, int now_number)
{       
        struct i2c_adapter *adap;
        int ret = 0;
        int addr_scan = 0;
        adap = i2c_get_adapter(sw->twi_id);
        sw->temp_client->adapter = adap;
        if (!i2c_check_functionality(sw->temp_client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;
          
        if(sw->twi_id == sw->temp_client->adapter->nr){
                while((sw->info[now_number].i2c_address[addr_scan++]) &&  
                      (addr_scan < (ADDRESS_NUMBER+1))) {
                        if(!sw->info[now_number].is_support) {
				continue;
			}
			
                        sw->temp_client->addr = sw->info[now_number].i2c_address[addr_scan - 1];
                        dprintk(DEBUG_INIT, "%s: name = %s, addr = 0x%x\n", __func__,
                                sw->info[now_number].name, sw->temp_client->addr); 
                                                        
                        if(i2c_check_addr(sw->check_addr, sw->temp_client->addr)) {
                                ret = 0;
                                continue;
                        }
   
                        ret = i2c_test(sw->temp_client);
                        if(!ret) {
                                dprintk(DEBUG_INIT, "sw->check_addr[%zu]:0x%x\n", strlen(sw->check_addr),
                                         sw->temp_client->addr);
                                sw->check_addr[strlen(sw->check_addr)] = sw->temp_client->addr;                 
                                ret = 0;
                        	continue;
                        } else {   
                                sw->response_addr = sw->temp_client->addr;      	    
                                dprintk(DEBUG_INIT, "I2C connection sucess!\n");
                                break;
                        }  
                }   
        }
        return ret;
}
static void sw_update_write_info(struct sw_device *sw)
{
        
        if((sw->device_name == NULL) && (sw->write_id < 0)) {
                dprintk(DEBUG_INIT, "%s:key_name is null or the id is error !\n", __func__);
                return ;
        }
                       
        memset(&sw->write_info[sw->write_id].str_info, 0, sizeof(sw->write_info[sw->write_id].str_info));
        sprintf(sw->write_info[sw->write_id].str_info, "%s=\"%s\"\n", sw->name->write_key_name, sw->device_name); 
        
}
static void sw_i2c_test(struct sw_device *sw)
{        
        int number = sw->support_number;
        int now_number = -1;
        int scan_number = 0;
        int is_find = 0;
        int ret = -1;
        
        now_number = get_device_name_number(sw);
        if(now_number < 0)
                now_number = 0;
                
        dprintk(DEBUG_INIT, "number:%d now_number:%d,scan_number:%d\n", number,
                now_number, scan_number);
        while(scan_number < number) {
                dprintk(DEBUG_INIT, "scan_number:%d, now_number:%d\n", 
                        scan_number, now_number);
                scan_number++;

                now_number = (now_number == number) ? 0 : now_number;
               
                ret = sw_device_response_test(sw, now_number);
                if(!ret) {
                        now_number++; 
        	        continue;
        	}   
        	sw->current_number = now_number;
                if(sw_chip_id_detect(sw, now_number)) {                        
                        is_find = 1;
                        break;
                }
                        
                now_number++; 
                is_find = 0;
         
        }
        
        if(is_find == 1) {
                dprintk(DEBUG_INIT, "from copy name:%s, strlen(name):%zu\n", 
                        sw->info[sw->current_number].name, strlen(sw->device_name));
                        
                if((strncmp(sw->info[sw->current_number].name, sw->device_name, 
                   strlen(sw->device_name))) || (!strlen(sw->device_name))) {
                        sw->write_flag = 1;
                        memset(&sw->device_name, 0, sizeof(sw->device_name));
                        strcpy(sw->device_name, sw->info[sw->current_number].name);
                        sw_update_write_info(sw);
                        
                }
                
                dprintk(DEBUG_INIT, "%s: write_key_name:%s\n", __func__, sw->name->write_key_name);
                if(!strncmp(sw->name->write_key_name, "ctp", 3)) {               
                        strcpy(d_name.c_name, sw->device_name);
                        d_name.c_addr = sw->response_addr;
                } else if(!strncmp(sw->name->write_key_name, "gsensor", 7)) {
                        strcpy(d_name.g_name, sw->device_name);
                        d_name.g_addr = sw->response_addr;
                }
        }
}

static int sw_device_detect_start(struct sw_device *sw)
{        
        char tmp[FILE_LENGTH];
       
        /*step1: Get sysconfig.fex profile information*/
        memset(&tmp, 0, sizeof(tmp));
        if(sw_sysconfig_get_para(sw) < 0) {
                printk("get sysconfig para erro!\n");
                return -1;
        }
        
        if(sw->detect_used) {
                /*step 2:Read the device.info file information ,get device name!*/
                if(ctp_mask != 1) {                       

                        if(sw_get_write_info(tmp, sw) <= 0) {
                                sw_set_write_info(sw);
                                printk("get write info erro!\n");
                        } else {
                                if(!sw_analytic_write_info(tmp, sw)) {
                	              sw_set_write_info(sw);
                                }                       
                        }
                        get_device_name_info(sw);
                }
                
                /*step 3: The i2c address detection equipment, find the device used at present.*/                
                sw_i2c_test(sw);
                
                /*step 4:Update the device.info file information*/
                if(ctp_mask != 1) {                        
                        dprintk(DEBUG_INIT, "write_flag:%d\n", sw->write_flag);
                        if(sw->write_flag) {
                                sw_set_write_info(sw);       
                        }
                }
        }    
        
        return 0;           
}

static int sw_register_device_detect(struct sw_device_info info[], struct para_name *name,
                                     int number)
{
        struct sw_device *sw;        
        struct i2c_client *client;
        
        dprintk(DEBUG_INIT, "[sw_device]:%s begin!\n", __func__);

        sw = kzalloc(sizeof(*sw), GFP_KERNEL);
	if (!sw){
	        printk("allocate data fail!\n");
		return -ENOMEM;
	}
	
	        
	client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!client){
		printk("allocate client fail!\n");
		kfree(sw);
                return -ENOMEM;        
	}
		
	sw->temp_client = client;      
	sw->info = info;
	sw->name = name; 
	sw->support_number = number;  
	sw->total_raw = DEFAULT_TOTAL_ROW;  
	
	strcpy(sw->write_info[0].str_info, NOTE_INFO1);
        strcpy(sw->write_info[1].str_info, NOTE_INFO2);
        sprintf(sw->write_info[2].str_info, "%s=\"\"\n", GSENSOR_DEVICE_KEY_NAME);
        sprintf(sw->write_info[3].str_info, "%s=\"\"\n", CTP_DEVICE_KEY_NAME);
        sprintf(sw->write_info[4].str_info, "%s=\"\"\n", LSENSOR_DEVICE_KEY_NAME);
        sprintf(sw->write_info[5].str_info, "%s=\"\"\n", GYR_SENSOR_DEVICE_KEY_NAME);
	sprintf(sw->write_info[6].str_info, "%s=\"\"\n", COMPASS_SENSOR_DEVICE_KEY_NAME);
        
        sw_device_detect_start(sw);
        
        kfree(sw);
        kfree(client);
        
        dprintk(DEBUG_INIT, "[sw_device]:%s end!\n", __func__);
	
	return 0; 
}

static void sw_devices_set_power(struct para_power *pm)
{
	if (pm->power_ldo) {
		pm->ldo = regulator_get(NULL, pm->power_ldo);
		if (!pm->ldo)
			printk("%s: could not get  ldo '%s' ,ignore firstly\n",
					__func__,pm->power_ldo);
		else {
			regulator_set_voltage(pm->ldo,
					(int)(pm->power_ldo_vol)*1000,
					(int)(pm->power_ldo_vol)*1000);
			regulator_enable(pm->ldo);
		}
	} else if(0 != pm->power_io.gpio) {
		if(0 != gpio_request(pm->power_io.gpio, NULL))
			printk("%s : %d gpio_request is failed,ignore firstly\n",
					__func__,pm->power_io.gpio);
		else
			gpio_direction_output(pm->power_io.gpio, 1);
	}
}

static void sw_devices_power_free(struct para_power *pm)
{
	if(pm->ldo) {
		regulator_disable(pm->ldo);
		regulator_put(pm->ldo);
		pm->ldo = NULL;
	} else if (0 != pm->power_io.gpio)
		gpio_free(pm->power_io.gpio);
}
static void sw_devices_events(struct work_struct *work)
{
        int ret = -1;
        int device_number = 0;
        
        
        get_power_para(&c_power);
        sw_devices_set_power(&c_power);
        dprintk(DEBUG_INIT, "[sw_device]:%s begin!\n", __func__);
        
        if(ctp_mask != 1) {
                device_number = (sizeof(gsensors)) / (sizeof(gsensors[0]));
                ret = sw_register_device_detect(gsensors, &g_name, device_number);
                if(ret < 0)
                        printk("gsensor detect fail!\n");

                device_number = (sizeof(lsensors)) / (sizeof(lsensors[0]));
                ret = sw_register_device_detect(lsensors, &ls_name, device_number);
                if(ret < 0)
                        printk("lsensor detect fail!\n");

                device_number = (sizeof(gyr_sensors)) / (sizeof(gyr_sensors[0]));
                ret = sw_register_device_detect(gyr_sensors, &gyr_name, device_number);
                if(ret < 0)
                        printk("gyr detect fail!\n");

				device_number = (sizeof(compass_sensors)) / (sizeof(compass_sensors[0]));
                ret = sw_register_device_detect(compass_sensors, &compass_name, device_number);
                if(ret < 0)
                        printk("compass detect fail!\n");
        }
        
        device_number = (sizeof(ctps)) / (sizeof(ctps[0]));
        ctp_wakeup(c_power.reset_pin, 20);
        msleep(50);
        ret = sw_register_device_detect(ctps, &c_name, device_number);        
        if(ret < 0)
                printk("ctp detect fail!\n"); 
        
        sw_devices_power_free(&c_power);
        dprintk(DEBUG_INIT, "[sw_device]:%s end!\n", __func__);       
}	

static ssize_t sw_device_gsensor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{   
        ssize_t cnt = 0;
        dprintk(DEBUG_INIT, "[sw_device]:%s: device name:%s\n", __func__, d_name.g_name);
        
        cnt += sprintf(buf + cnt,"device name:%s\n" ,d_name.g_name); 
        cnt += sprintf(buf + cnt,"device addr:0x%x\n" ,d_name.g_addr);
        return cnt;

}

static ssize_t sw_device_gsensor_store(struct device *dev,struct device_attribute *attr,
		const char *buf, size_t size)
{
        return 0;
}

static ssize_t sw_device_ctp_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{   
         ssize_t cnt = 0;
         dprintk(DEBUG_INIT, "[sw_device]:%s: write_key_name:%s\n", __func__, d_name.c_name);
         if(ctp_mask != 1) {
                cnt += sprintf(buf + cnt,"device name:%s\n" ,d_name.c_name); 
                cnt += sprintf(buf + cnt,"device addr:0x%x\n" ,d_name.c_addr);
        } else 
                return sprintf(buf, d_name.c_name);
                
        return cnt;

}

static ssize_t sw_device_ctp_store(struct device *dev,struct device_attribute *attr,
		const char *buf, size_t size)
{
        return 0;
}

static DEVICE_ATTR(gsensor, S_IRUGO|S_IWUSR|S_IWGRP, sw_device_gsensor_show, sw_device_gsensor_store);
static DEVICE_ATTR(ctp, S_IRUGO|S_IWUSR|S_IWGRP, sw_device_ctp_show, sw_device_ctp_store);




struct device_node *find_np_by_name(const char *name)
{
	int i;
	for(i=0; i<ALL_DEVICE_NP_SIZE && all_device_np[i].np; i++)
	{
		if(!strcmp(all_device_np[i].name, name))
			return all_device_np[i].np;
	}
	dprintk(DEBUG_INIT, "[sw_device] cat't find node in all_device_np\n");
	return NULL;
}

static int sw_input_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	static int count = 0;
	if(!np){
		printk("[sw_device]: ERROR get device tree node failed\n");
		return -1;
	}

	dprintk(DEBUG_INIT, "[sw_device] probe name:%s, node:%p\n", np->name, np);
	all_device_np[count].name = np->name;
	all_device_np[count].np   = np; 
	count++;
	return 0;
}

static int sw_input_remove(struct platform_device *pdev)
{
	printk("sw-device: sw_input_remove\n");
	return 0;
}

/*TODO: change the node name to real one in DTS*/
static const struct of_device_id sw_input_match[] = {
	{.compatible = "allwinner,sun50i-ctp-para", },		
	{.compatible = "allwinner,sun50i-ctp-list", },		
	{.compatible = "allwinner,sun50i-gsensor-para", },		
	{.compatible = "allwinner,sun50i-gsensor-list-para", },		
	{.compatible = "allwinner,sun50i-compass-para", },		
	{.compatible = "allwinner,sun50i-compass-list-para", },		
	{.compatible = "allwinner,sun50i-lsensors-para", },		
	{.compatible = "allwinner,sun50i-lsensors-list-para", },	
	{.compatible = "allwinner,sun50i-gyr_sensors-para", },		
	{.compatible = "allwinner,sun50i-gyr_sensors-list-para", },
	{},
};


static struct platform_driver sw_input_platform_driver = {
	.probe  = sw_input_probe,
	.remove = sw_input_remove,
	.driver = {
		.name  = "sw-input",
		//.pm    = ,
		.owner = THIS_MODULE,
		.of_match_table = sw_input_match,
	},
};




static struct attribute *sw_device_attributes[] = {
        &dev_attr_gsensor.attr,
        &dev_attr_ctp.attr,
        NULL
};

static struct attribute_group dev_attr_group = {
	.attrs = sw_device_attributes,
};

static const struct attribute_group *dev_attr_groups[] = {
	&dev_attr_group,
	NULL,
};

static void sw_device_release(struct device *dev)
{
    
}

static struct device sw_device_detect = {
        .init_name = "sw_device",
        .release = sw_device_release,
};	
		
static int __init sw_device_init(void)
{        	
	int err = 0;
	dprintk(DEBUG_INIT, "[sw_device]:%s begin!\n", __func__);
	sw_wq = create_singlethread_workqueue("sw_wq");
	if (sw_wq == NULL) {
		printk("create sw_wq fail!\n");
		return 0;
	}

	dprintk(DEBUG_INIT, "[sw_device]: register platform driver: sw_input_platform_driver\n");
	platform_driver_register(&sw_input_platform_driver);


	queue_work(sw_wq, &sw_work);
	
	sw_device_detect.groups = dev_attr_groups;
	err = device_register(&sw_device_detect);
	if (err) {
		printk("%s register camera detect driver as misc device error\n", __FUNCTION__);
	}
	dprintk(DEBUG_INIT, "[sw_device]:%s end!\n", __func__);
	return 0;
}

static void __exit sw_device_exit(void)
{
        printk(" sw device driver exit!\n"); 
        device_unregister(&sw_device_detect);   
		platform_driver_unregister(&sw_input_platform_driver);
    
}
/*************************************************************************************/

MODULE_AUTHOR("Ida yin");
MODULE_DESCRIPTION("Detection i2c device driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");

module_init(sw_device_init);
module_exit(sw_device_exit);
