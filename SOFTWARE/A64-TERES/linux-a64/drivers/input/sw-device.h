#ifndef __LINUX_SW_DEVICE_H__
#define __LINUX_SW_DEVICE_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/input-polldev.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/errno.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <linux/sys_config.h>
//#include <mach/hardware.h>
//#include <mach/gpio.h>
//#include <mach/sys_config.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>



#include<linux/fs.h>
#include<linux/string.h>
#include<asm/uaccess.h>
#include <linux/regulator/consumer.h>

enum{
        DEBUG_INIT              = 1U << 0,
        DEBUG_SUSPEND           = 1U << 1,
        DEBUG_INT_INFO          = 1U << 2,
        DEBUG_X_Y_INFO          = 1U << 3,
        DEBUG_KEY_INFO          = 1U << 4,
        DEBUG_WAKEUP_INFO       = 1U << 5,
        DEBUG_OTHERS_INFO       = 1U << 6,
};

#define NOTE_INFO1              ";Behind the equals sign said detected equipment corresponding to the name of the driver\n"
#define NOTE_INFO2              ";Note: don't change the file format!\n"
#define GSENSOR_DEVICE_KEY_NAME "gsensor_module_name"
#define CTP_DEVICE_KEY_NAME     "ctp_module_name"
#define LSENSOR_DEVICE_KEY_NAME "light sensor_module_name"
#define GYR_SENSOR_DEVICE_KEY_NAME "gyr sensor_module_name"
#define COMPASS_SENSOR_DEVICE_KEY_NAME "compass sensor_module_name"
#define FILE_DIR                "sensors_cache/device.info"


#define STRING_LENGTH           (128)
#define FILE_LENGTH             (512)
#define NAME_LENGTH             (32)
#define ADDRESS_NUMBER          (5)
#define REG_VALUE_NUMBER        (10)
#define DEFAULT_TOTAL_ROW       (7)

/*
* sw_write_info - The contents of the device.info file
*
* @str_info:    Record contents.
* @str_id:      Record the line Numbers.
*/
struct sw_write_info{
        char str_info[STRING_LENGTH];
        int str_id;
};

/*
* sw_device_info - Record related information of the device
*
* @name:        Device driver name.
* @i2c_address: Device I2C address.
* @chip_id_reg: Device chip id register.
* @id_value:    Device chip id value.
* @same_flag:   When the device without chip id,Used to identify the same i2c address.
*
* When the device without chip id,Chip_id_reg & id_value should be set to 0x00.
* and when a device is the same as the address of the device, the device has the chip id,
* then same_flag should set to 1 without chip id of the device.
*/
struct sw_device_info{
	char name[NAME_LENGTH];
	unsigned short is_support;

	unsigned short i2c_address[ADDRESS_NUMBER];
	unsigned short chip_id_reg;
	unsigned short id_value[REG_VALUE_NUMBER];

	int same_flag;
};
/*
 * para_power_name - Used to parse the power elevant key word
 *
 * @keyname:             sysconfig.fex para key name, format:xxx_para ,For example: ctp_para
 * @power_ldo_name:      sysconfig.fex ldo name,For example: ctp_power_ldo
 * @power_ldo_vol_name:  sysconfig.fex ldo vol para key name, For example: ctp_power_ldo_vol
 * @power_io_name:       sysconfig.fex para power gpio name, For example: ctp_power_io
 * @reset_pin_name:      sysconfig.fex reset pin name, For example: ctp_wakeup
 * @power_ldo:           ldo name ,For example : "axp22_dldo4"
 * @power_ldo_vol        The voltage need to be set for ctp
 * @power_io             power gpio pin
 * @reset_pin            The reset pin of the device
 * @ldo                  axp ldo
*/
struct para_power{
        char* keyname;
        char* power_ldo_name;
        char* power_ldo_vol_name;
        char* power_io_name;
        char* reset_pin_name;
        const char *power_ldo;
        int    power_ldo_vol;
        struct gpio_config power_io;
        struct gpio_config reset_pin;
        struct regulator *ldo;
};

/*
* para_name - Used to parse the key word
*
* @used_keyname:        sysconfig.fex para key name, format:xxx_para ,For example: ctp_para
* @used_subname:        sysconfig.fex para used name, format:xxx_used ,For example: ctp_used
* @detected_keyname:    sysconfig.fex detect list para key name, format:xxx_list_para,
*                       For example: ctp_list_para
* @detected_subname:    sysconfig.fex detect list para detect used name, format:xxx_det_used,
*                       For example: ctp_det_used
* @twi_id_name:         sysconfig.fex para twi id name, format:xxx_twi_id ,For example: ctp_twi_id
* @write_key_name:      Device.info file Identifies the device driver keyword
*                       format:xxx_module_name,For example:ctp_module_name
*/
struct para_name{
        char* used_keyname;
        char* used_subname;
        char* detect_keyname;
        char* detect_subname;
        char* twi_id_name;
        char* write_key_name;
};

/*
* sw_device - The information of the device.
*
* @info:        sw_device_info structure.
* @temp_client: Contain device address of the i2c_client structure.
* @filp:        Read/write file structure.
* @name:        para_name sw_device_info.
* @write_info:  Store device.info information.
* @device_name: Device driver name
* @check_addr:  Have tested the i2c address.
* @support_number: The array size of info,total number of devices need to test.
* @current_number: The array subscript of info,the positions of the detected device in the info.
* @detect_used: sysconfig.fex file xxx_det_used value.
* @write_flag:  Identifies whether the contents of the need to update the device.info file.
* @total_raw:   The total number of the device.info file
* @write_id:    The number of rows in the device.info file of device
* @twi_id:      Twi id of the device
* @response_addr: The address of the communication success.
*/
struct sw_device{
        struct sw_device_info   *info;
        struct i2c_client       *temp_client;
        struct file             *filp;
        struct para_name        *name;
        struct sw_write_info write_info[NAME_LENGTH];

        char    device_name[NAME_LENGTH];
        char    check_addr[NAME_LENGTH];

        int     support_number;
        int     current_number;
        int     detect_used;
        int     write_flag;
        int     total_raw;
        int     write_id;

        __u32   twi_id; 

        unsigned short response_addr;
};

/*
*
* sw_device_name - The information of the device.
*
* @g_name: gsensor device name.
* @c_name: ctp device name
* @g_addr: gsensor device address.
* @c_addr: ctp device address.
*/

struct sw_device_name{
        char g_name[NAME_LENGTH];
        char c_name[NAME_LENGTH];
        char gy_name[NAME_LENGTH];
        char ls_name[NAME_LENGTH];

        unsigned short g_addr;
        unsigned short c_addr;
        unsigned short gy_addr;
        unsigned short ls_addr;
};

struct node_pointer{
	const char *name;
	struct device_node *np; 
};

struct device_node *find_np_by_name(const char *name);


#endif
