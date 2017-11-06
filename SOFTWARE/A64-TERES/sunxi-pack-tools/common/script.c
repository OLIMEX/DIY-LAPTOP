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
#include "common.h"


static  char  *script_mod_buf = NULL;           //指向第一个主键
static  int    script_main_key_count = 0;       //保存主键的个数

static  int   _test_str_length(char *str)
{
	int length = 0;

	while(str[length++])
	{
		if(length > 32)
		{
			length = 32;
			break;
		}
	}

	return length;
}
/*
************************************************************************************************************
*
*                                             script_parser_init
*
*    函数名称：
*
*    参数列表：script_buf: 脚本数据池
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int script_parser_init(char *script_buf)
{
	script_head_t   *script_head;

	if(script_buf)
	{
		script_mod_buf = script_buf;
		script_head = (script_head_t *)script_mod_buf;

		script_main_key_count = script_head->main_key_count;

		return SCRIPT_PARSER_OK;
	}
	else
	{
		return SCRIPT_PARSER_EMPTY_BUFFER;
	}
}
/*
************************************************************************************************************
*
*                                             script_parser_exit
*
*    函数名称：
*
*    参数列表：NULL
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int script_parser_exit(void)
{
	script_mod_buf = NULL;
	script_main_key_count = 0;

	return SCRIPT_PARSER_OK;
}

/*
************************************************************************************************************
*
*                                             script_parser_fetch
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：根据传进的主键，子键，取得对应的数值
*
*
************************************************************************************************************
*/
int script_parser_sunkey_all(char *main_name, void *buffer)
{
	char   main_char[32], sub_char[32];
	script_main_key_t  *main_key;
	script_sub_key_t   *sub_key;
	int    i, j;
	int    pattern, word_count;
	script_gpio_set_t  *user_gpio_cfg;
	unsigned int   *data;

	//检查脚本buffer是否存在
	if(!script_mod_buf)
	{
		return SCRIPT_PARSER_EMPTY_BUFFER;
	}
	//检查主键名称和子键名称是否为空
	if(main_name == NULL)
	{
		return SCRIPT_PARSER_KEYNAME_NULL;
	}
	//检查数据buffer是否为空
	if(buffer == NULL)
	{
		return SCRIPT_PARSER_DATA_VALUE_NULL;
	}
	//保存主键名称和子键名称，如果超过16字节则截取16字节
	memset(main_char, 0, sizeof(main_char));
	memset(sub_char, 0, sizeof(sub_char));
	if(_test_str_length(main_name) <= 32)
	{
		strcpy(main_char, main_name);
	}
	else
	{
		strncpy(main_char, main_name, 31);
	}

	data = (unsigned int *)buffer;
	for(i=0;i<script_main_key_count;i++)
	{
		main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
		if(strcmp(main_key->main_name, main_char))    //如果主键不匹配，寻找下一个主键
		{
			continue;
		}
		//printf("find main key named %s\n", main_char);
		//主键匹配，寻找子键名称匹配
		for(j=0;j<main_key->lenth;j++)
		{
			sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
//			if(strcmp(sub_key->sub_name, sub_char))    //如果主键不匹配，寻找下一个主键
//			{
//				continue;
//			}
			//printf("index %d name : %s  value = ", j, sub_key->sub_name);
			pattern    = (sub_key->pattern>>16) & 0xffff;             //获取数据的类型
			word_count = (sub_key->pattern>> 0) & 0xffff;             //获取所占用的word个数
			//取出数据
			switch(pattern)
			{
				case DATA_TYPE_SINGLE_WORD:                           //单word数据类型
					*data = *(int *)(script_mod_buf + (sub_key->offset<<2));
					//printf("%d\n", *data);

					data ++;
					continue;

				case DATA_TYPE_STRING:     							  //字符串数据类型
					memcpy((char *)data, script_mod_buf + (sub_key->offset<<2), word_count << 2);
					data += word_count;
					continue;

				case DATA_TYPE_GPIO_WORD:							 //多word数据类型
					user_gpio_cfg = (script_gpio_set_t *)data;
                    //发现是GPIO类型，检查是否足够存放用户数据
					strcpy( user_gpio_cfg->gpio_name, sub_char);
					memcpy(&user_gpio_cfg->port, script_mod_buf + (sub_key->offset<<2),  sizeof(script_gpio_set_t) - 32);
					data += (sizeof(script_gpio_set_t) - 32)/4;

					continue;
			}
		}
		return SCRIPT_PARSER_OK;
	}

	return SCRIPT_PARSER_KEY_NOT_FIND;
}
/*
************************************************************************************************************
*
*                                             script_parser_fetch
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：根据传进的主键，子键，取得对应的数值
*
*
************************************************************************************************************
*/
int script_parser_fetch(char *main_name, char *sub_name, int value[])
{
	char   main_char[32], sub_char[32];
	script_main_key_t  *main_key;
	script_sub_key_t   *sub_key;
	int    i, j;
	int    pattern, word_count;
	script_gpio_set_t  *user_gpio_cfg;

	//检查脚本buffer是否存在
	if(!script_mod_buf)
	{
		return SCRIPT_PARSER_EMPTY_BUFFER;
	}
	//检查主键名称和子键名称是否为空
	if((main_name == NULL) || (sub_name == NULL))
	{
		return SCRIPT_PARSER_KEYNAME_NULL;
	}
	//检查数据buffer是否为空
	if(value == NULL)
	{
		return SCRIPT_PARSER_DATA_VALUE_NULL;
	}
	//保存主键名称和子键名称，如果超过16字节则截取16字节
	memset(main_char, 0, sizeof(main_char));
	memset(sub_char, 0, sizeof(sub_char));
	if(_test_str_length(main_name) <= 32)
	{
		strcpy(main_char, main_name);
	}
	else
	{
		strncpy(main_char, main_name, 31);
	}

	if(_test_str_length(sub_name) <= 32)
	{
		strcpy(sub_char, sub_name);
	}
	else
	{
		strncpy(sub_char, sub_name, 31);
	}

	for(i=0;i<script_main_key_count;i++)
	{
		main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
		if(strcmp(main_key->main_name, main_char))    //如果主键不匹配，寻找下一个主键
		{
			continue;
		}
		//主键匹配，寻找子键名称匹配
		for(j=0;j<main_key->lenth;j++)
		{
			sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
			if(strcmp(sub_key->sub_name, sub_char))    //如果主键不匹配，寻找下一个主键
			{
				continue;
			}
			pattern    = (sub_key->pattern>>16) & 0xffff;             //获取数据的类型
			word_count = (sub_key->pattern>> 0) & 0xffff;             //获取所占用的word个数
			//取出数据
			switch(pattern)
			{
				case DATA_TYPE_SINGLE_WORD:                           //单word数据类型
					value[0] = *(int *)(script_mod_buf + (sub_key->offset<<2));
					break;

				case DATA_TYPE_STRING:     							  //字符串数据类型
					memcpy((char *)value, script_mod_buf + (sub_key->offset<<2), word_count << 2);
					break;

				case DATA_TYPE_GPIO_WORD:							 //多word数据类型
					user_gpio_cfg = (script_gpio_set_t *)value;
                    //发现是GPIO类型，检查是否足够存放用户数据
					strcpy( user_gpio_cfg->gpio_name, sub_char);
					memcpy(&user_gpio_cfg->port, script_mod_buf + (sub_key->offset<<2),  sizeof(script_gpio_set_t) - 32);

					break;
			}

			return SCRIPT_PARSER_OK;
		}
	}

	return SCRIPT_PARSER_KEY_NOT_FIND;
}


/*
************************************************************************************************************
*
*                                             script_parser_mainkey_get_gpio_cfg
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：根据传进的主键，取得主键下GPIO的配置信息
*
*
************************************************************************************************************
*/
int script_parser_mainkey_get_gpio_cfg(char *main_name, void *gpio_cfg, int gpio_count)
{
	char   main_bkname[32];
	char   *main_char;
	script_main_key_t  *main_key = NULL;
	script_sub_key_t   *sub_key = NULL;
	script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)gpio_cfg;
	int    i, j;
	int    pattern, user_index;

	//检查脚本buffer是否存在
	if(!script_mod_buf)
	{
		return SCRIPT_PARSER_EMPTY_BUFFER;
	}
	//检查主键名称和子键名称是否为空
	if(main_name == NULL)
	{
		return SCRIPT_PARSER_KEYNAME_NULL;
	}
	//首先清空用户buffer
	memset(user_gpio_cfg, 0, sizeof(script_gpio_set_t) * gpio_count);
	//保存主键名称和子键名称，如果超过31字节则截取31字节
	main_char = main_name;
	if(_test_str_length(main_name) > 31)
	{
	    memset(main_bkname, 0, 32);
		strncpy(main_bkname, main_name, 31);
		main_char = main_bkname;
	}

	for(i=0;i<script_main_key_count;i++)
	{
		main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
		if(strcmp(main_key->main_name, main_char))    //如果主键不匹配，寻找下一个主键
		{
			continue;
		}
		//主键匹配，寻找子键名称匹配
		user_index = 0;
		for(j=0;j<main_key->lenth;j++)
		{
			sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
			pattern    = (sub_key->pattern>>16) & 0xffff;             //获取数据的类型
			//取出数据
			if(DATA_TYPE_GPIO_WORD == pattern)
			{
			    strcpy( user_gpio_cfg[user_index].gpio_name, sub_key->sub_name);
				memcpy(&user_gpio_cfg[user_index].port, script_mod_buf + (sub_key->offset<<2), sizeof(script_gpio_set_t) - 32);
				user_index++;
				if(user_index >= gpio_count)
				{
					break;
				}
			}
		}
		return SCRIPT_PARSER_OK;
	}

	return SCRIPT_PARSER_KEY_NOT_FIND;
}




