/*
 * arch/arch/mach-sun6i/sys_config.c
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * sys_config utils (porting from 2.6.36)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <linux/string.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/sys_config.h>
#include <linux/slab.h>
#include <linux/bootmem.h>

#define SCRIPT_MALLOC(size)        alloc_bootmem((unsigned long)size)
#define SCRIPT_FREE(addr, size)    free_bootmem((unsigned long)addr, (unsigned long)size)


#define ITEM_TYPE_TO_STR(type)	((SCIRPT_ITEM_VALUE_TYPE_INT == (type)) ?  "int"    :	\
				((SCIRPT_ITEM_VALUE_TYPE_STR == (type))  ?  "string" :	\
				((SCIRPT_ITEM_VALUE_TYPE_PIO == (type))  ?   "gpio" : "invalid")))

/*
 * define origin main key data structure in cript buffer
 * @name: main key name, defined left of "="
 * @items: count of sub key
 * @offset: position of the value of main key, in dword
 */
#pragma pack(1)
typedef struct
{
    char name[32];
    int  sub_cnt;
    int  offset;
} script_origin_main_key_t;
#pragma pack()

/*
 * define origin sub key data structure in cript buffer
 * @name: sub key name, defined left of "="
 * @offset: sub key value position, in dword
 * @type: type of sub key, int / string / gpio
 * @cnt:  length of the value area, in dword
 */
#pragma pack(1)
typedef struct
{
    char name[32];
    int  offset;
    struct {
        u32 cnt : 16;
        u32 type: 16;
    }pattern;
} script_origin_sub_key_t;
#pragma pack()

/*
 * define origin header of the script in cript buffer
 * @main_cnt: count of main keys
 * @version: script version
 * @main_key: fist main key
 */
#pragma pack(1)
typedef struct
{
    unsigned int main_cnt;
    unsigned int length;
    unsigned int version[2];
    script_origin_main_key_t    main_key;
} script_origin_head_t;
#pragma pack()

/*
 * define origin gpio data structure in cript buffer
 * @gpio_name: gpio name, defined left of '='
 * @port: gpio port number, 1-PA, 2-PB, 3-PC, ...
 * @port_num: pin number in port, 0-Px0, 1-Px1, ...
 * @mul_sel: multi-function select
 * @pull: pin status,
 * @drv_level: drive level
 * @data: gpio data value
 */
#pragma pack(1)
typedef struct {
    char    gpio_name[32];
    int     port;
    int     port_num;
    int     mul_sel;
    int     pull;
    int     drv_level;
    int     data;
} script_origin_gpio_t;
#pragma pack()

/*
 *===========================================================================================================
 * origin script defined as follows:
 *
 * |-----------------------|
 * |@main-cnt |@version[3] |
 * |-----------------------|
 * | origin main key 0:    |
 * | @name[32]             |
 * | @sub_cnt              |
 * | @offset   ------------|-----
 * |-----------------------|    |
 * | origin main key 1:    |    |
 * | @name[32]             |    |
 * | @sub_cnt              |    |
 * | @offset               |    |
 * |-----------------------|    |
 * | origin main key 2:    |    |
 * |      ......           |    |
 *                              |
 *                              |
 *                              |
 * |-----------------------|    |
 * | origin sub key 0:     |<---|
 * | @name                 |
 * | @offset   ------------|----|
 * | @type                 |    |
 * |-----------------------|    |
 * | origin sub key 0:     |    |
 * | @name                 |    |
 * | @offset               |    |
 * | @type                 |    |
 * |-----------------------|    |
 * | origin sub key 0:     |    |
 * |    ......             |    |
 *                              |
 *                              |
 *                              |
 * |-----------------------|    |
 * | origin sub key value: |<---|
 * |    ......             |
 *
 *
 *
 *===========================================================================================================
 * script parse result organized as follows:
 *
 * |-----------------------|
 * | script_main_key_t     |
 * | @name[]               |
 * | @subkey      ---------|-------------------------------------------->|-----------------------|
 * | @subkey_val  ---------|------->|-----------------------|            | script_sub_key_t      |
 * | @gpio    -------------|------->| script_item_u         |<-----|     | @name[]               |
 * | @gpio_cnt             |        | @val                  |      ------|-@value                |
 * | @hash                 |        | @str                  |            | @type                 |
 * | @next      -----------|----    | @gpio                 |            | @hash                 |
 * |-----------------------|   |    |-----------------------|            | @next    -------------|---|
 * | script_main_key_t     |<--|    | script_item_u         |<----|      |-----------------------|   |
 * | @name[]               |        | @val                  |     |      | script_sub_key_t      |<--|
 * | @subkey               |        | @str                  |--|  |      | @name[]               |
 * | @subkey_val           |        | @gpio                 |  |  -------|-@value                |
 * | @gpio                 |        |-----------------------|  |         | @type                 |
 * | @gpio_cnt             |        | script_item_u         |  |         | @hash                 |
 * | @hash                 |        | @val                  |  |         | @next                 |
 * | @next                 |        | @str                  |  |         |-----------------------|
 * |-----------------------|        | @gpio                 |  |         | script_sub_key_t      |
 * | main key 2:           |        |-----------------------|  |         |  ......               |
 * |    ......             |        | script_item_u         |  |
 *                                  | ......                |  |
 *                                                             |--------->|-----------------------|
 *                                                                        | string                |
 *                                                                        |-----------------------|
 *===========================================================================================================
 */

#define SCRIPT_NAME_SIZE_MAX    (32)

/*
 * define script item management data
 * @name: item name, which is defined left of '='
 * @value: value of the item
 * @type: type of the item, interge / string / gpio?
 * @hash: hash value of sub key name, for searching quickly
 * @next: pointer for list
 */
typedef struct {
    char                        name[SCRIPT_NAME_SIZE_MAX];
    script_item_u               *value;
    script_item_value_type_e    type;
    int                         hash;
    void                        *next;
} script_sub_key_t;

/*
 * define script main key management data
 * @name: main key name, which is defined by '[]'
 * @subkey: sub key list
 * @subkey_val: buffer for save sub keys
 * @gpio: gpio list pointer
 * @gpio_cnt: gpio conter
 * @hash: hash value of sub key name, for searching quickly
 * @next: pointer for list
 */
typedef struct {
    char                name[SCRIPT_NAME_SIZE_MAX];
    script_sub_key_t    *subkey;
    script_item_u       *subkey_val;
    script_item_u       *gpio;
    int                 gpio_cnt;
    int                 hash;
    void                *next;
} script_main_key_t;

/*
 * define script sub key type, raw from sys_config.bin
 * @SCIRPT_PARSER_VALUE_TYPE_INVALID: invalid type
 * @SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD: int item type
 * @SCIRPT_PARSER_VALUE_TYPE_STRING: string item type
 * @SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD: multi int type, not used currently
 * @SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD: gpio item type
 */
typedef enum {
	SCIRPT_PARSER_VALUE_TYPE_INVALID = 0,
	SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD,
	SCIRPT_PARSER_VALUE_TYPE_STRING,
	SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD,
	SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD
} script_parser_value_type_t;

static script_main_key_t   *g_script;

static int hash(char *string)
{
    int     hash = 0;

    if(!string) {
        return 0;
    }

    while(*string){
        hash += *string;
	string++;
    }

    return hash;
}

/**
 * port_to_index - gpio port to global index, port is from script
 * @port: gpio port group index, eg: 1 for PA, 2 for PB..., -1(0xFFFF) for axp pin
 * @port_num: port index in gpio group, eg: 0 for PA0, 1 for PA1...
 *
 * return the gpio index for the port, GPIO_INDEX_INVALID indicate err
 */
u32 port_to_index(u32 port, u32 port_num)
{
	/* sunxi system config pin name space and
	 * pinctrl name space map table like this:
	 * pin define group  pin-index  pinctrl-number
	 * PA0        1      0          0
	 * PB0        2      0          32
	 * PC0        3      0          64
	 * PC1        3      1          65
	 * ...
	 * pinctrl-number = (group - 1) * 32 + pin-index
	 */
	u32 index;

	if (port == AXP_CFG_GRP) {
		/* valid axp gpio */
		index = AXP_PIN_BASE + port_num;
	} else {
		/* sunxi pinctrl pin */
		index = (port - 1) * 32 + port_num;
	}
	return index;
}

void dump_mainkey(script_main_key_t *pmainkey)
{
	script_sub_key_t *psubkey = NULL;
	char gpio_name[8] = {0};

	if(NULL == pmainkey || NULL == pmainkey->subkey || NULL == pmainkey->subkey_val)
		return;

	printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __func__);
	printk("    name:      %s\n", pmainkey->name);
	printk("    sub_key:   name           type      value\n");
	psubkey = pmainkey->subkey;
	while(psubkey) {
		switch(psubkey->type) {
		case SCIRPT_ITEM_VALUE_TYPE_INT:
			printk("               %-15s%-10s%d\n", psubkey->name,
				ITEM_TYPE_TO_STR(psubkey->type), psubkey->value->val);
			break;
		case SCIRPT_ITEM_VALUE_TYPE_STR:
			printk("               %-15s%-10s\"%s\"\n", psubkey->name,
				ITEM_TYPE_TO_STR(psubkey->type), psubkey->value->str);
			break;
		case SCIRPT_ITEM_VALUE_TYPE_PIO:
			sunxi_gpio_to_name(psubkey->value->gpio.gpio, gpio_name);
			printk("               %-15s%-10s(gpio: %#x / %s, mul: %d, pull %d, drv %d, data %d)\n",
				psubkey->name, ITEM_TYPE_TO_STR(psubkey->type),
				psubkey->value->gpio.gpio, gpio_name,
				psubkey->value->gpio.mul_sel,
				psubkey->value->gpio.pull, psubkey->value->gpio.drv_level, psubkey->value->gpio.data);
			break;
		default:
			printk("               %-15sinvalid type!\n", psubkey->name);
			break;
		}
		psubkey = psubkey->next;
	}
	printk("--------------------------%s--------------------------\n", __func__);
}

int script_dump_mainkey(char *main_key)
{
	int     main_hash = 0;
	script_main_key_t *pmainkey = g_script;

	if(NULL != main_key) {
		printk("%s: dump main_key %s\n", __func__, main_key);
		main_hash = hash(main_key);
		while(pmainkey) {
			if((pmainkey->hash == main_hash) && !strcmp(pmainkey->name, main_key)) {
				dump_mainkey(pmainkey);
				return 0;
			}
			pmainkey = pmainkey->next;
		}
		printk(KERN_ERR "%s err: main key %s not found!\n", __func__, main_key);
	} else {
		printk("%s: dump all the mainkey, \n", __func__);
		while(pmainkey) {
			printk("%s: dump main key %s\n", __func__, pmainkey->name);
			dump_mainkey(pmainkey);
			pmainkey = pmainkey->next;
		}
	}
	printk("%s exit\n", __func__);
	return 0;
}
EXPORT_SYMBOL(script_dump_mainkey);

script_item_value_type_e
script_get_item(char *main_key, char *sub_key, script_item_u *item)
{
    int     main_hash, sub_hash;
    script_main_key_t   *mainkey = g_script;

    if(!main_key || !sub_key || !item || !g_script) {
        return SCIRPT_ITEM_VALUE_TYPE_INVALID;
    }

    main_hash = hash(main_key);
    sub_hash = hash(sub_key);

    /* try to look for the main key from main key list */
    while(mainkey) {
        if((mainkey->hash == main_hash) && !strcmp(mainkey->name, main_key)) {
            /* find the main key */
            script_sub_key_t    *subkey = mainkey->subkey;
            while(subkey) {
                if((subkey->hash == sub_hash) && !strcmp(subkey->name, sub_key)) {
                    /* find the sub key */
                    *item = *subkey->value;
                    return subkey->type;
                }
                subkey = subkey->next;
            }

            /* no sub key defined under the main key */
            return SCIRPT_ITEM_VALUE_TYPE_INVALID;
        }
        mainkey = mainkey->next;
    }

    return SCIRPT_ITEM_VALUE_TYPE_INVALID;
}
EXPORT_SYMBOL(script_get_item);

int script_get_pio_list(char *main_key, script_item_u **list)
{
    int     main_hash;
    script_main_key_t   *mainkey = g_script;

    if(!main_key || !list || !g_script) {
        return 0;
    }

    main_hash = hash(main_key);

    /* try to look for the main key from main key list */
    while(mainkey) {
        if((mainkey->hash == main_hash) && !strcmp(mainkey->name, main_key)) {
            /* find the main key */
            *list = mainkey->gpio;
            return mainkey->gpio_cnt;
        }
        mainkey = mainkey->next;
    }

    /* look for main key failed */
    return 0;
}
EXPORT_SYMBOL(script_get_pio_list);

/*
 * script_get_main_key_count
 *      get the count of main_key
 *
 * @return     the count of main_key
 */
unsigned int script_get_main_key_count(void)
{
	unsigned int      mainkey_count = 0;
	script_main_key_t *mainkey = g_script;
    if(!mainkey) {
    	/*  system config not initialize now */
        return 0;
    }

    /* count the total mainkey number */
    while(mainkey) {
        mainkey_count++;
        mainkey = mainkey->next;
    }
	return mainkey_count;
}
EXPORT_SYMBOL(script_get_main_key_count);

/*
 * script_get_main_key_name
 *      get the name of main_key by index
 *
 * @main_key_index   the index of main_key
 * @main_key_name    the buffer to store target main_key_name
 * @return     the pointer of target mainkey name
 */
char *script_get_main_key_name(unsigned int main_key_index)
{
	unsigned int      mainkey_count = 0;
	script_main_key_t *mainkey = g_script;
    if(!mainkey) {
    	/*  system config not initialize now */
        return 0;
    }

    /* try to find target mainkey */
    while(mainkey) {
        if (mainkey_count == main_key_index) {
        	/* find target mainkey */
        	return mainkey->name;
        }
        mainkey_count++;
        mainkey = mainkey->next;
    }
    /* invalid mainkey index for seach */
	return NULL;
}
EXPORT_SYMBOL(script_get_main_key_name);

bool script_is_main_key_exist(char *main_key)
{
    int     main_hash;
    script_main_key_t   *mainkey = g_script;

    if(!main_key || !g_script) {
    	pr_err("%s(%d) err: para err, main_key %s\n", __func__, __LINE__, main_key);
        return false;
    }

    main_hash = hash(main_key);

    /* try to look for the main key from main key list */
    while(mainkey) {
        if((mainkey->hash == main_hash) && !strcmp(mainkey->name, main_key)) {
            /* find the main key */
            return true;
        }
        mainkey = mainkey->next;
    }

    /* look for main key failed */
    return false;
}
EXPORT_SYMBOL(script_is_main_key_exist);


unsigned int script_get_length(void)
{
	script_origin_head_t         *orign_head = __va(0x41010000);

	return orign_head->length;
}
EXPORT_SYMBOL(script_get_length);
/*
 * init script
 */
int __init script_init(void)
{
    int     i, j, count;

    script_origin_head_t        *script_hdr = __va(0x41010000);

    script_origin_main_key_t    *origin_main;
    script_origin_sub_key_t     *origin_sub;

    script_main_key_t           *main_key;
    script_sub_key_t            *sub_key, *tmp_sub, swap_sub;

    script_item_u               *sub_val, *tmp_val, swap_val, *pval_temp;

    printk("%s enter!\n", __func__);
    if(!script_hdr || 1) {
        printk(KERN_ERR "script buffer is NULL!\n");
        return -1;
    }

    /* alloc memory for main keys */
    g_script = SCRIPT_MALLOC(script_hdr->main_cnt*sizeof(script_main_key_t));
    if(!g_script) {
        printk(KERN_ERR "try to alloc memory for main keys!\n");
        return -1;
    }

    origin_main = &script_hdr->main_key;
    for(i=0; i<script_hdr->main_cnt; i++) {
        main_key = &g_script[i];

        /* copy main key name */
        strncpy(main_key->name, origin_main[i].name, SCRIPT_NAME_SIZE_MAX);
        /* calculate hash value */
        main_key->hash = hash(main_key->name);

	if (origin_main[i].sub_cnt == 0) {
		/* this mainkey have no subkey, skip subkey initialize */
		main_key->subkey = NULL;
		main_key->subkey_val = NULL;
		count = 0;
		goto next_mainkey;
	}

        /* allock memory for sub-keys */
        main_key->subkey = SCRIPT_MALLOC(origin_main[i].sub_cnt*sizeof(script_sub_key_t));
        main_key->subkey_val = SCRIPT_MALLOC(origin_main[i].sub_cnt*sizeof(script_item_u));
        if(!main_key->subkey || !main_key->subkey_val) {
            printk(KERN_ERR "try alloc memory for sub keys failed!\n");
            goto err_out;
        }

        sub_key = main_key->subkey;
        sub_val = main_key->subkey_val;
        origin_sub = (script_origin_sub_key_t *)((ptrdiff_t)script_hdr + (origin_main[i].offset<<2));

        /* process sub keys */
        for(j=0; j<origin_main[i].sub_cnt; j++) {
            strncpy(sub_key[j].name, origin_sub[j].name, SCRIPT_NAME_SIZE_MAX);
            sub_key[j].hash = hash(sub_key[j].name);
            sub_key[j].type = (script_item_value_type_e)origin_sub[j].pattern.type;
            sub_key[j].value = &sub_val[j];
            if(origin_sub[j].pattern.type == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) {
                sub_val[j].val = *(int *)((ptrdiff_t)script_hdr + (origin_sub[j].offset<<2));
                sub_key[j].type = SCIRPT_ITEM_VALUE_TYPE_INT;
            } else if(origin_sub[j].pattern.type == SCIRPT_PARSER_VALUE_TYPE_STRING) {
                sub_val[j].str = SCRIPT_MALLOC((origin_sub[j].pattern.cnt<<2) + 1);
                memcpy(sub_val[j].str, (char *)((ptrdiff_t)script_hdr + (origin_sub[j].offset<<2)), origin_sub[j].pattern.cnt<<2);
                sub_key[j].type = SCIRPT_ITEM_VALUE_TYPE_STR;
            } else if(origin_sub[j].pattern.type == SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD) {
                script_origin_gpio_t    *origin_gpio = (script_origin_gpio_t *)((ptrdiff_t)script_hdr + (origin_sub[j].offset<<2) - 32);
		u32 gpio_tmp = port_to_index(origin_gpio->port, origin_gpio->port_num);

		if(GPIO_INDEX_INVALID == gpio_tmp)
			printk(KERN_ERR "%s:%s->%s gpio cfg invalid, please check sys_config.fex!\n",__func__,main_key->name,sub_key[j].name);
                sub_val[j].gpio.gpio = gpio_tmp;
                sub_val[j].gpio.mul_sel = (u32)origin_gpio->mul_sel;
                sub_val[j].gpio.pull = (u32)origin_gpio->pull;
                sub_val[j].gpio.drv_level = (u32)origin_gpio->drv_level;
		sub_val[j].gpio.data = (u32)origin_gpio->data;
                sub_key[j].type = SCIRPT_ITEM_VALUE_TYPE_PIO;
            } else {
                sub_key[j].type = SCIRPT_ITEM_VALUE_TYPE_INVALID;
            }
        }

        /* process gpios */
        tmp_sub = main_key->subkey;
        tmp_val = main_key->subkey_val;
        count = 0;
        for(j=0; j<origin_main[i].sub_cnt; j++) {
            if(sub_key[j].type == SCIRPT_ITEM_VALUE_TYPE_PIO) {
                /* swap sub key */
                swap_sub = *tmp_sub;
                *tmp_sub = sub_key[j];
                sub_key[j] = swap_sub;
		/* swap sub key value ptr */
		pval_temp = tmp_sub->value;
		tmp_sub->value = sub_key[j].value;
		sub_key[j].value = pval_temp;
                /* swap key value */
                swap_val = *tmp_val;
                *tmp_val = main_key->subkey_val[j];
                main_key->subkey_val[j] = swap_val;
                tmp_sub++;
                tmp_val++;
                count++;
            }
        }

        /* process sub key link */
        for(j=0; j<origin_main[i].sub_cnt-1; j++) {
            main_key->subkey[j].next = &main_key->subkey[j+1];
        }
        /* set gpio infermation */
next_mainkey:
        main_key->gpio = main_key->subkey_val;
        main_key->gpio_cnt = count;
	main_key->next = &g_script[i+1];
    }
    g_script[script_hdr->main_cnt-1].next = 0;

    /* dump all the item */
    //script_dump_mainkey(NULL);
    printk("%s exit!\n", __func__);
    return 0;

err_out:

    /* script init failed, release resource */
    printk(KERN_ERR "init sys_config script failed!\n");
    if(g_script) {
        for(i=0; i<script_hdr->main_cnt; i++) {
            main_key = &g_script[i];
            origin_sub = (script_origin_sub_key_t *)((ptrdiff_t)script_hdr + (origin_main[i].offset<<2));

            if(main_key->subkey_val) {
                for(j=0; j<origin_main[i].sub_cnt; j++) {
                    if(main_key->subkey[j].type == SCIRPT_ITEM_VALUE_TYPE_STR) {
                        if (main_key->subkey_val[j].str) {
				SCRIPT_FREE(main_key->subkey_val[j].str, (origin_sub[j].pattern.cnt<<2) + 1);
			}
                    }
                }
                SCRIPT_FREE(main_key->subkey_val, sizeof(script_item_u));
            }

            if(main_key->subkey) {
                SCRIPT_FREE(main_key->subkey, sizeof(script_sub_key_t));
            }
        }

        SCRIPT_FREE(g_script, script_hdr->main_cnt*sizeof(script_main_key_t));
        g_script = 0;
    }

    return -1;
}

/* string for dump all items */
#define DUMP_ALL_STR	"all"

typedef struct {
	char mainkey[64];
}sysfs_dump_t;

typedef struct {
	char mainkey[64];
	char subkey[64];
}sysfs_get_item_t;

static sysfs_dump_t dump_struct;
static sysfs_get_item_t get_item_struct;

int __sysfs_dump_mainkey(script_main_key_t *pmainkey, char *buf)
{
	script_sub_key_t *psubkey = NULL;
	int cnt = 0;
	char gpio_name[8] = {0};

	if(NULL == pmainkey || NULL == pmainkey->subkey || NULL == pmainkey->subkey_val)
		return 0;

	cnt += sprintf(buf + cnt, "++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __func__);
	cnt += sprintf(buf + cnt, "    name:      %s\n", pmainkey->name);
	cnt += sprintf(buf + cnt, "    sub_key:   name           type      value\n");
	psubkey = pmainkey->subkey;
	while(psubkey) {
		if (cnt > PAGE_SIZE - 100) {
			printk("%s err: cnt %d\n", __func__, cnt);
			return cnt;
		}
		switch(psubkey->type) {
		case SCIRPT_ITEM_VALUE_TYPE_INT:
			cnt += sprintf(buf + cnt, "               %-15s%-10s%d\n", psubkey->name,
				ITEM_TYPE_TO_STR(psubkey->type), psubkey->value->val);
			break;
		case SCIRPT_ITEM_VALUE_TYPE_STR:
			cnt += sprintf(buf + cnt, "               %-15s%-10s\"%s\"\n", psubkey->name,
				ITEM_TYPE_TO_STR(psubkey->type), psubkey->value->str);
			break;
		case SCIRPT_ITEM_VALUE_TYPE_PIO:
			sunxi_gpio_to_name(psubkey->value->gpio.gpio, gpio_name);
			cnt += sprintf(buf + cnt, "               %-15s%-10s(gpio: %#x / %s, mul: %d, pull %d, drv %d, data %d)\n",
				psubkey->name, ITEM_TYPE_TO_STR(psubkey->type),
				psubkey->value->gpio.gpio, gpio_name,
				psubkey->value->gpio.mul_sel,
				psubkey->value->gpio.pull, psubkey->value->gpio.drv_level, psubkey->value->gpio.data);
			break;
		default:
			cnt += sprintf(buf + cnt, "               %-15sinvalid type!\n", psubkey->name);
			break;
		}
		psubkey = psubkey->next;
	}
	cnt += sprintf(buf + cnt, "--------------------------%s--------------------------\n", __func__);
	return cnt;
}

/**
 * show func of dump attribute.
 * @dev:     class ptr.
 * @attr:    attribute ptr.
 * @buf:     the output buf which store the dump msg
 *
 * return size written to the buf, otherwise failed
 */
static ssize_t dump_show(struct class *class, struct class_attribute *attr, char *buf)
{
	script_main_key_t *pmainkey = g_script;
	int main_hash = 0;
	int cnt = 0;
#if 1
	if(0 == dump_struct.mainkey[0]) {
		pr_err("%s(%d) err: please input mainkey firstly\n", __func__, __LINE__);
		return -EINVAL;
	}
#endif
	if(!memcmp(dump_struct.mainkey, DUMP_ALL_STR, strlen(DUMP_ALL_STR))
		|| 0 == dump_struct.mainkey[0]) { /* dump all mainkey */
		pr_info("%s: dump all main keys\n", __func__);
		while(pmainkey) {
			pr_info("%s: dump main key for %s\n", __func__, pmainkey->name);
			cnt += __sysfs_dump_mainkey(pmainkey, buf + cnt);
			pmainkey = pmainkey->next;
		}
	} else {
		pr_info("%s: dump main key for %s\n", __func__, dump_struct.mainkey);
		main_hash = hash(dump_struct.mainkey);
		while(pmainkey) {
			if((pmainkey->hash == main_hash) && !strcmp(pmainkey->name, dump_struct.mainkey))
				return __sysfs_dump_mainkey(pmainkey, buf);
			pmainkey = pmainkey->next;
		}
		pr_err("%s(%d) err: main key %s not found!\n", __func__, __LINE__, dump_struct.mainkey);
	}
	return cnt;
}

/**
 * store func of dump attribute.
 * @class:   class ptr.
 * @attr:    attribute ptr.
 * @buf:     the input buf which contain the mainkey name. eg: "lcd0_para\n"
 * @size:    buf size.
 *
 * return size if success, otherwise failed
 */
static ssize_t dump_store(struct class *class, struct class_attribute *attr,
			const char *buf, size_t size)
{
	if(strlen(buf) >= sizeof(dump_struct.mainkey)) {
		pr_err("%s(%d) err: name \"%s\" too long\n", __func__, __LINE__, buf);
		return -EINVAL;
	}
	if(0 == buf[0]) {
		pr_err("%s(%d) err: invalid mainkey\n", __func__, __LINE__);
		return -EINVAL;
	}
	strcpy(dump_struct.mainkey, buf);
	if('\n' == dump_struct.mainkey[strlen(dump_struct.mainkey) - 1]) /* remove tail \n */
		dump_struct.mainkey[strlen(dump_struct.mainkey) - 1] = 0;
	pr_info("%s: get input mainkey \"%s\"\n", __func__, dump_struct.mainkey);
	return size;
}

ssize_t get_item_show(struct class *class, struct class_attribute *attr, char *buf)
{
	script_item_value_type_e type;
	script_item_u item;
	ssize_t outsize;

	type = script_get_item(get_item_struct.mainkey, get_item_struct.subkey, &item);
	if(SCIRPT_ITEM_VALUE_TYPE_INVALID == type) {
		pr_err("%s(%d) err: script_get_item failed for \"%s\"->\"%s\"\n", __func__, __LINE__,
			get_item_struct.mainkey, get_item_struct.subkey);
		return -EINVAL;
	} else {
		pr_info("%s(%d): script_get_item return type [%s] for %s->%s\n", __func__, __LINE__,
			ITEM_TYPE_TO_STR(type), get_item_struct.mainkey, get_item_struct.subkey);
		memcpy(buf, &item, sizeof(item));
		/* the extra 4bytes store item type, for sizeof(script_item_value_type_e) = 4 */
		*(u32 *)(buf + sizeof(item)) = (u32)type;
		outsize = sizeof(item) + sizeof(u32);
		/* copy string to user space */
		if(SCIRPT_ITEM_VALUE_TYPE_STR == type) {
			strcpy(buf + outsize, item.str);
			outsize += strlen(item.str);
		} else if(SCIRPT_ITEM_VALUE_TYPE_PIO == type) {
			/* convert gpio to name(eg: "PH5") and copy to user space */
			//WARN_ON(0 != sw_gpio_to_name(item.gpio.gpio, buf + outsize));
			outsize += strlen(buf + outsize);
		}
		return outsize;
	}
}

ssize_t get_item_store(struct class *class, struct class_attribute *attr,
			const char *buf, size_t size)
{
	char *last_char;

	pr_info("%s: input buf %s\n", __func__, buf);
	sscanf(buf, "%s %s", get_item_struct.mainkey, get_item_struct.subkey);
	if(0 != strlen(get_item_struct.subkey)) {
		last_char = get_item_struct.subkey + strlen(get_item_struct.subkey) - 1;
		if('\n' == *last_char)
			*last_char = 0;
	}
	pr_info("%s: get \"%s\"->\"%s\"\n", __func__, get_item_struct.mainkey, get_item_struct.subkey);
	return size;
}

static struct class_attribute script_class_attrs[] = {
	__ATTR(dump, 0644, dump_show, dump_store),
	__ATTR(get_item, 0644, get_item_show, get_item_store),
	__ATTR_NULL,
};

static struct class script_class = {
    .name = "script",
    .owner = THIS_MODULE,
    .class_attrs = script_class_attrs,
};

static int __init script_sysfs_init(void)
{
    int status;

    memset((void*)&dump_struct, 0, sizeof(dump_struct));
    memset((void*)&get_item_struct, 0, sizeof(get_item_struct));
    /* create /sys/class/script/ */
    status = class_register(&script_class);
    if (status < 0)
        pr_err("%s: status %d\n", __func__, status);
    else
        pr_info("%s success\n", __func__);

    return status;
}
postcore_initcall(script_sysfs_init);

