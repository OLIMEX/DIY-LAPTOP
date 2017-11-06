/*
**********************************************************************************************************************
*											        eGon
*						                     the Embedded System
*									       script parser sub-system
*
*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
*                                           All Rights Reserved
*
* File    : script.c
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#ifndef  _SCRIPT_H_
#define  _SCRIPT_H_

#define   DATA_TYPE_SINGLE_WORD  (1)
#define   DATA_TYPE_STRING       (2)
#define   DATA_TYPE_MULTI_WORD   (3)
#define   DATA_TYPE_GPIO_WORD    (4)

#define   SCRIPT_PARSER_OK       (0)
#define   SCRIPT_PARSER_EMPTY_BUFFER   	(-1)
#define   SCRIPT_PARSER_KEYNAME_NULL   	(-2)
#define   SCRIPT_PARSER_DATA_VALUE_NULL	(-3)
#define   SCRIPT_PARSER_KEY_NOT_FIND    (-4)
#define   SCRIPT_PARSER_BUFFER_NOT_ENOUGH  (-5)

typedef struct
{
	unsigned  main_key_count;
	unsigned  length;
	unsigned  char name[8];
} script_head_t;

typedef struct
{
	char main_name[32];
	int  lenth;
	int  offset;
}
script_main_key_t;

typedef struct
{
	char sub_name[32];
	int  offset;
	int  pattern;
}
script_sub_key_t;

typedef struct
{
    char  gpio_name[32];
    int port;
    int port_num;
    int mul_sel;
    int pull;
    int drv_level;
    int data;
}
script_gpio_set_t;

extern  int script_parser_init(char *script_buf);
extern  int script_parser_exit(void);
extern  int script_parser_sunkey_all(char *main_name, void *buffer);
extern  int script_parser_fetch(char *main_name, char *sub_name, int value[]);
extern  int script_parser_subkey_count(char *main_name);
extern  int script_parser_mainkey_count(void);
extern  int script_parser_mainkey_get_gpio_count(char *main_name);
extern  int script_parser_mainkey_get_gpio_cfg(char *main_name, void *gpio_cfg, int gpio_count);


#endif


