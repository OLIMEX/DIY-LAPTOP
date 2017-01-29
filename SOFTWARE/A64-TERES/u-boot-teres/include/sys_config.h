/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#ifndef __SW_SYS_CONFIG_H
#define __SW_SYS_CONFIG_H

#define   FDT_PATH_FEL_KEY                   "/soc/fel_key"
#define   FDT_PATH_RECOVERY_KEY              "/soc/recovery_key"
#define   FDT_PATH_FASTBOOT_KEY              "/soc/fastboot_key"
#define   FDT_PATH_KEY_DETECT                "/soc/key_detect_en"
#define   FDT_PATH_PLATFORM                  "/soc/platform"
#define   FDT_PATH_POWER_SPLY                "/soc/power_sply"
#define   FDT_PATH_TARGET                    "/soc/target"
#define   FDT_PATH_CARD_BOOT                 "/soc/card_boot"
#define   FDT_PATH_CARD0_BOOT_PARA           "/soc/card0_boot_para"
#define   FDT_PATH_CARD2_BOOT_PARA           "/soc/card2_boot_para"
#define   FDT_PATH_REGU                      "/soc/regu"

//for disp alias
#define   FDT_DISP_PATH  "disp"
#define   FDT_HDMI_PATH  "hdmi"
#define   FDT_LCD0_PATH  "lcd0"
#define   FDT_LCD1_PATH  "lcd1"
#define   FDT_BOOT_DISP_PATH  "boot_disp"





typedef struct
{
	char  gpio_name[32];
	int port;
	int port_num;
	int mul_sel;
	int pull;
	int drv_level;
	int data;
} script_gpio_set_t;


#define   EGPIO_FAIL             (-1)
#define   EGPIO_SUCCESS          (0)

typedef enum
{
	PIN_PULL_DEFAULT 	= 	0xFF,
	PIN_PULL_DISABLE 	=	0x00,
	PIN_PULL_UP			  =	0x01,
	PIN_PULL_DOWN	  	=	0x02,
	PIN_PULL_RESERVED	=	0x03
} pin_pull_level_t;

typedef	enum
{
	PIN_MULTI_DRIVING_DEFAULT	=	0xFF,
	PIN_MULTI_DRIVING_0			=	0x00,
	PIN_MULTI_DRIVING_1			=	0x01,
	PIN_MULTI_DRIVING_2			=	0x02,
	PIN_MULTI_DRIVING_3			=	0x03
} pin_drive_level_t;

typedef enum
{
	PIN_DATA_LOW,
	PIN_DATA_HIGH,
	PIN_DATA_DEFAULT = 0XFF
} pin_data_t;

#define	PIN_PHY_GROUP_A			0x00
#define	PIN_PHY_GROUP_B			0x01
#define	PIN_PHY_GROUP_C			0x02
#define	PIN_PHY_GROUP_D			0x03
#define	PIN_PHY_GROUP_E			0x04
#define	PIN_PHY_GROUP_F			0x05
#define	PIN_PHY_GROUP_G			0x06
#define	PIN_PHY_GROUP_H			0x07
#define	PIN_PHY_GROUP_I			0x08
#define	PIN_PHY_GROUP_J			0x09

typedef struct
{
    char  gpio_name[32];
    int port;
    int port_num;
    int mul_sel;
    int pull;
    int drv_level;
    int data;
} user_gpio_set_t;


typedef struct
{
    int mul_sel;
    int pull;
    int drv_level;
    int data;
} gpio_status_set_t;

typedef struct
{
    char    gpio_name[32];
    int     port;
    int     port_num;
    gpio_status_set_t user_gpio_status;
    gpio_status_set_t hardware_gpio_status;
} system_gpio_set_t;



/* gpio operations */
extern int gpio_init(void);
extern int gpio_exit(void);
extern int gpio_request_simple(char *main_name, const char *sub_name);
extern ulong gpio_request(user_gpio_set_t *gpio_list, unsigned group_count_max);
extern ulong  gpio_request_ex(char *main_name, const char *sub_name);
extern int gpio_release(ulong p_handler, int if_release_to_default_status);
extern int gpio_get_all_pin_status(ulong p_handler, user_gpio_set_t *gpio_status, unsigned gpio_count_max, unsigned if_get_from_hardware);
extern int gpio_get_one_pin_status(ulong p_handler, user_gpio_set_t *gpio_status, const char *gpio_name, unsigned if_get_from_hardware);
extern int gpio_set_one_pin_status(ulong p_handler, user_gpio_set_t *gpio_status, const char *gpio_name, unsigned if_set_to_current_input_status);
extern int gpio_set_one_pin_io_status(ulong p_handler, unsigned if_set_to_output_status, const char *gpio_name);
extern int gpio_set_one_pin_pull(ulong p_handler, unsigned set_pull_status, const char *gpio_name);
extern int gpio_set_one_pin_driver_level(ulong p_handler, unsigned set_driver_level, const char *gpio_name);
extern int gpio_read_one_pin_value(ulong p_handler, const char *gpio_name);
extern int gpio_write_one_pin_value(ulong p_handler, unsigned value_to_gpio, const char *gpio_name);
extern int gpio_request_early(void  *user_gpio_list, __u32 group_count_max, __s32 set_gpio);

extern void upper(char *str);
extern void lower(char *str);

//gpio for use fdt
int fdt_get_one_gpio_by_offset(int node_offset, const char* prop_name,user_gpio_set_t* gpio_list);
int fdt_get_one_gpio(const char* node_path, const char* prop_name,user_gpio_set_t* gpio_list);

int fdt_set_one_gpio(const char* node_path, const char* prop_name);
//pin for use fdt
//int fdt_get_pin_num(const char* node_path,const char* pinctrl_name);
//int fdt_get_all_pin(const char* node_path,const char* pinctrl_name,user_gpio_set_t* gpio_list);
int fdt_set_all_pin(const char * node_path,const char * pinctrl_name);
int fdt_set_all_pin_by_offset(int nodeoffset,const char* pinctrl_name);
int fdt_set_pin_byname(user_gpio_set_t  *pin_list,int pin_count, const char* pin_name);
//normal
int fdt_set_normal_gpio(user_gpio_set_t  *gpio_list, int gpio_count);


#endif
