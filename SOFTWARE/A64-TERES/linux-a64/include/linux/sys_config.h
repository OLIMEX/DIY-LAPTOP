/*
 * arch/arch/mach-sunxi/include/mach/sys_config.h
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Kevin <Kevin@allwinnertech.com>
 *
 * sys_config utils (porting from 2.6.36)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __SW_SYS_CONFIG_H
#define __SW_SYS_CONFIG_H

//#include <mach/gpio.h>
/* pin group base number name space,
 * the max pin number : 26*32=832.
 */
 #if defined(CONFIG_ARCH_SUN50IW1P1)
#define SUNXI_PINCTRL 	"1c20800.pinctrl"
#define SUNXI_R_PINCTRL "1f02c00.pinctrl"
#else
#define SUNXI_PINCTRL 	"1c20800.pinctrl"
#define SUNXI_R_PINCTRL "1f02c00.pinctrl"
#endif

#define SUNXI_BANK_SIZE 32
#define SUNXI_PA_BASE	0
#define SUNXI_PB_BASE	32
#define SUNXI_PC_BASE	64
#define SUNXI_PD_BASE	96
#define SUNXI_PE_BASE	128
#define SUNXI_PF_BASE	160
#define SUNXI_PG_BASE	192
#define SUNXI_PH_BASE	224
#define SUNXI_PI_BASE	256
#define SUNXI_PJ_BASE	288
#define SUNXI_PK_BASE	320
#define SUNXI_PL_BASE	352
#define SUNXI_PM_BASE	384
#define SUNXI_PN_BASE	416
#define SUNXI_PO_BASE	448
#define AXP_PIN_BASE	1024

#define SUNXI_PIN_NAME_MAX_LEN	8

/* sunxi gpio name space */
#define GPIOA(n)	(SUNXI_PA_BASE + (n))
#define GPIOB(n)	(SUNXI_PB_BASE + (n))
#define GPIOC(n)	(SUNXI_PC_BASE + (n))
#define GPIOD(n)	(SUNXI_PD_BASE + (n))
#define GPIOE(n)	(SUNXI_PE_BASE + (n))
#define GPIOF(n)	(SUNXI_PF_BASE + (n))
#define GPIOG(n)	(SUNXI_PG_BASE + (n))
#define GPIOH(n)	(SUNXI_PH_BASE + (n))
#define GPIOI(n)	(SUNXI_PI_BASE + (n))
#define GPIOJ(n)	(SUNXI_PJ_BASE + (n))
#define GPIOK(n)	(SUNXI_PK_BASE + (n))
#define GPIOL(n)	(SUNXI_PL_BASE + (n))
#define GPIOM(n)	(SUNXI_PM_BASE + (n))
#define GPION(n)	(SUNXI_PN_BASE + (n))
#define GPIOO(n)	(SUNXI_PO_BASE + (n))
#define GPIO_AXP(n)	(AXP_PIN_BASE  + (n))

/* sunxi specific input/output/eint functions */
#define SUNXI_PIN_INPUT_FUNC	(0)
#define SUNXI_PIN_OUTPUT_FUNC	(1)
#define SUNXI_PIN_EINT_FUNC	(6)
#define SUNXI_PIN_IO_DISABLE	(7)

/* axp group base number name space,
 * axp pinctrl number space coherent to sunxi-pinctrl.
 */
#define AXP_PINCTRL 	        "axp-pinctrl"
#define AXP_CFG_GRP 		(0xFFFF)
#define AXP_PIN_INPUT_FUNC	(0)
#define AXP_PIN_OUTPUT_FUNC	(1)
#define IS_AXP_PIN(pin)         (pin >= AXP_PIN_BASE)


/* sunxi specific pull up/down */
enum sunxi_pull_up_down {
	SUNXI_PULL_DISABLE = 0,
	SUNXI_PULL_UP,
	SUNXI_PULL_DOWN,
};

/* sunxi specific data types */
enum sunxi_data_type {
	SUNXI_DATA_LOW = 0,
	SUNXI_DATA_HIGH = 0,
};

/* sunxi specific pull status */
enum sunxi_pin_pull {
	SUNXI_PIN_PULL_DISABLE 	= 0x00,
	SUNXI_PIN_PULL_UP	= 0x01,
	SUNXI_PIN_PULL_DOWN	= 0x02,
	SUNXI_PIN_PULL_RESERVED	= 0x03,
};

/* sunxi specific driver levels */
enum sunxi_pin_drv_level {
	SUNXI_DRV_LEVEL0 = 10,
	SUNXI_DRV_LEVEL1 = 20,
	SUNXI_DRV_LEVEL2 = 30,
	SUNXI_DRV_LEVEL3 = 40,
};

/* sunxi specific data bit status */
enum sunxi_pin_data_status {
    SUNXI_PIN_DATA_LOW  = 0x00,
    SUNXI_PIN_DATA_HIGH = 0x01,
};

/* sunxi pin interrupt trigger mode */
enum sunxi_pin_int_trigger_mode {
    SUNXI_PIN_EINT_POSITIVE_EDGE   =   0x0,
    SUNXI_PIN_EINT_NEGATIVE_EDGE   =   0x1,
    SUNXI_PIN_EINT_HIGN_LEVEL      =   0x2,
    SUNXI_PIN_EINT_LOW_LEVEL       =   0x3,
    SUNXI_PIN_EINT_DOUBLE_EDGE     =   0x4
};

/* the source clock of pin int */
enum sunxi_pin_int_source_clk {
    SUNXI_PIN_INT_SRC_CLK_32K = 0x0,
    SUNXI_PIN_INT_SRC_CLK_24M = 0x1
};

static inline int sunxi_gpio_to_name(int gpio, char *name)
{
	int bank, index;
	if (!name) {
		return -EINVAL;
	}
	if (IS_AXP_PIN(gpio)) {
		/* axp gpio name like this : GPIO0/GPIO1/.. */
		index = gpio - AXP_PIN_BASE;
		sprintf(name, "GPIO%d", index);
	} else {
		/* sunxi gpio name like this : PA0/PA1/PB0 */
		bank = gpio / SUNXI_BANK_SIZE;
		index = gpio % SUNXI_BANK_SIZE;
		sprintf(name, "P%c%d", ('A' + bank), index);
	}
	return 0;
}

/* pio end, invalid macro */
#define GPIO_INDEX_INVALID	(0xFFFFFFF0      )
#define GPIO_CFG_INVALID	(0xEEEEEEEE      )
#define GPIO_PULL_INVALID	(0xDDDDDDDD      )
#define GPIO_DRVLVL_INVALID	(0xCCCCCCCC      )
#define IRQ_NUM_INVALID		(0xFFFFFFFF      )
#define AXP_PORT_VAL		(0x0000FFFF      ) /* port val for axp pin in sys_config.fex */

/* pio default macro */
#define GPIO_PULL_DEFAULT	((u32)-1         )
#define GPIO_DRVLVL_DEFAULT	((u32)-1         )
#define GPIO_DATA_DEFAULT	((u32)-1         )

/* gpio config info */
struct gpio_config {
	u32	gpio;		/* gpio global index, must be unique */
	u32 	mul_sel;	/* multi sel val: 0 - input, 1 - output... */
	u32 	pull;		/* pull val: 0 - pull up/down disable, 1 - pull up... */
	u32 	drv_level;	/* driver level val: 0 - level 0, 1 - level 1... */
	u32	data;		/* data val: 0 - low, 1 - high, only vaild when mul_sel is input/output */
};

/*
 * define types of script item
 * @SCIRPT_ITEM_VALUE_TYPE_INVALID:  invalid item type
 * @SCIRPT_ITEM_VALUE_TYPE_INT: integer item type
 * @SCIRPT_ITEM_VALUE_TYPE_STR: strint item type
 * @SCIRPT_ITEM_VALUE_TYPE_PIO: gpio item type
 */
typedef enum {
	SCIRPT_ITEM_VALUE_TYPE_INVALID = 0,
	SCIRPT_ITEM_VALUE_TYPE_INT,
	SCIRPT_ITEM_VALUE_TYPE_STR,
	SCIRPT_ITEM_VALUE_TYPE_PIO,
} script_item_value_type_e;


/*
 * define data structure script item
 * @val: integer value for integer type item
 * @str: string pointer for sting type item
 * @gpio: gpio config for gpio type item
 */
typedef union {
    int                 val;
    char                *str;
    struct gpio_config  gpio;
} script_item_u;

int __init script_init(void);

/*
 * script_get_item
 *      get an item from script based on main_key & sub_key
 * @main_key    main key value in script which is marked by '[]'
 * @sub_key     sub key value in script which is left of '='
 * @item        item pointer for return value
 * @return      type of the item
 */
script_item_value_type_e script_get_item(char *main_key, char *sub_key, script_item_u *item);


/*
 * script_get_pio_list
 *      get gpio list from script baseed on main_key
 * @main_key    main key value in script which is marked by '[]'
 * @list        list pointer for return gpio list
 * @return      count of the gpios
 */
int script_get_pio_list(char *main_key, script_item_u **list);

/*
 * script_dump_mainkey
 *      dump main_key info
 * @main_key    main key value in script which is marked by '[]',
 *              if NULL, dump all main key info in script
 * @return      0
 */
int script_dump_mainkey(char *main_key);

/*
 * script_get_main_key_count
 *      get the count of main_key
 *
 * @return     the count of main_key
 */
unsigned int script_get_main_key_count(void);


/*
 * script_get_main_key_name
 *      get the name of main_key by index
 *
 * @main_key_index   the index of main_key
 * @main_key_name    the buffer to store target main_key_name
 * @return     the pointer of target mainkey name
 */
char *script_get_main_key_name(unsigned int main_key_index);

/*
 * script_is_main_key_exist
 *      check if the name of main_key exist
 *
 * @main_key    the buffer to store target main key name
 * @return      true if exist, false if not exist or script not initialized
 */
bool script_is_main_key_exist(char *main_key);

#endif
