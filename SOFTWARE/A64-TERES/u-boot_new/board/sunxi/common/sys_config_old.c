
#include <common.h>
#include <sys_config_old.h>
#include <sys_config.h>
#include <malloc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

script_sub_key_t *sw_cfg_get_subkey(const char *script_buf, const char *main_key, const char *sub_key)
{
    script_head_t *hd = (script_head_t *)script_buf;
    script_main_key_t *mk = (script_main_key_t *)(hd + 1);
    script_sub_key_t *sk = NULL;
    int i, j;

    for (i = 0; i < hd->main_key_count; i++) {

        if (strcmp(main_key, mk->main_name)) {
            mk++;
            continue;
        }

        for (j = 0; j < mk->lenth; j++) {
            sk = (script_sub_key_t *)(script_buf + (mk->offset<<2) + j * sizeof(script_sub_key_t));
            if (!strcmp(sub_key, sk->sub_name)) return sk;
        }
    }
    return NULL;
}

int sw_cfg_get_int(const char *script_buf, const char *main_key, const char *sub_key)
{
    script_sub_key_t *sk = NULL;
    char *pdata;
    int value;

    sk = sw_cfg_get_subkey(script_buf, main_key, sub_key);
    if (sk == NULL) {
        return -1;
    }

    if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) {
        pdata = (char *)(script_buf + (sk->offset<<2));
        value = *((int *)pdata);
        return value;
    }

    return -1;
}

char *sw_cfg_get_str(const char *script_buf, const char *main_key, const char *sub_key, char *buf)
{
    script_sub_key_t *sk = NULL;
    char *pdata;

    sk = sw_cfg_get_subkey(script_buf, main_key, sub_key);
    if (sk == NULL) {
        return NULL;
    }

    if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_STRING) {
        pdata = (char *)(script_buf + (sk->offset<<2));
        memcpy(buf, pdata, ((sk->pattern >> 0) & 0xffff));
        return (char *)buf;
    }

    return NULL;
}


  int   _test_str_length(char *str)
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

int script_parser_init(char *script_buf)
{
    script_head_t   *script_head;

	if(script_buf)
    {
        gd->script_mod_buf = script_buf;
        script_head = (script_head_t *)script_buf;

        gd->script_main_key_count = script_head->main_key_count;

        return SCRIPT_PARSER_OK;
    }
    else
    {
        return SCRIPT_PARSER_EMPTY_BUFFER;
    }
}

unsigned script_get_length(void)
{
	script_head_t *orign_head = (script_head_t *)gd->script_mod_buf;

	return orign_head->length;
}

int script_parser_exit(void)
{
    gd->script_mod_buf = NULL;
    gd->script_main_key_count = 0;

    return SCRIPT_PARSER_OK;
}

ulong script_parser_subkey( script_main_key_t* main_name,char *subkey_name , uint *pattern)
{
    script_main_key_t *main_key = NULL;
    script_sub_key_t  *sub_key  = NULL;
    int i = 0;

    if((!gd->script_mod_buf)||(gd->script_main_key_count<=0))
    {
        return 0;
    }
    if((main_name == NULL)||(subkey_name == NULL))
    {
        printf("main_name is invalid \n");
        return 0;
    }

    main_key = main_name;
    for(i = 0;i<main_key->lenth;i++)
    {
        sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (i * sizeof(script_sub_key_t)));
        if(strcmp(sub_key->sub_name,subkey_name))
            continue;
        *pattern = (sub_key->pattern>>16)&0xffff;
        return (ulong)sub_key;
    }
    return 0;
}

ulong script_parser_fetch_subkey_start(char *main_name)
{
	char   main_bkname[32];
    char   *main_char;
    script_main_key_t  *main_key = NULL;
    int    i;
    /* check params */
    if((!gd->script_mod_buf) || (gd->script_main_key_count <= 0))
    {
        return 0;
    }

    if(main_name == NULL)
    {
		return 0;
    }

    /* truncate string if size >31 bytes */
    main_char = main_name;
    if(_test_str_length(main_name) > 31)
    {
        memset(main_bkname, 0, 32);
        strncpy(main_bkname, main_name, 31);
        main_char = main_bkname;
    }

    for(i=0;i<gd->script_main_key_count;i++)
    {
        main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
        if(strcmp(main_key->main_name, main_char))
        {
            continue;
        }

		return (ulong)main_key;
	}

	return 0;
}


int script_parser_fetch_subkey_next(ulong hd, char *sub_name, int value[], int *index)
{
    script_main_key_t  *main_key;
    script_sub_key_t   *sub_key = NULL;
    int    j;
    int    pattern;

	if(!hd)
	{
		return -1;
	}

	main_key = (script_main_key_t *)hd;
    /* now find sub key */
    for(j = *index; j < main_key->lenth; j++)
    {
    	sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));

        pattern    = (sub_key->pattern>>16) & 0xffff; /* get datatype */
		if(pattern == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD)
		{
			value[0] = *(int *)(gd->script_mod_buf + (sub_key->offset<<2));
			strcpy(sub_name, sub_key->sub_name);

			*index = j + 1;

			return SCRIPT_PARSER_OK;
		}
		else if(pattern == SCIRPT_PARSER_VALUE_TYPE_STRING)
		{
			strcpy((void *)value, gd->script_mod_buf + (sub_key->offset<<2));
			strcpy(sub_name, sub_key->sub_name);

			*index = j + 1;

			return SCRIPT_PARSER_OK;
		}
    }

    return SCRIPT_PARSER_KEY_NOT_FIND;
}

int script_parser_fetch(char *main_name, char *sub_name, int value[], int count)
{
    char   main_bkname[32], sub_bkname[32];
    char   *main_char, *sub_char;
    script_main_key_t  *main_key = NULL;
    script_sub_key_t   *sub_key = NULL;
    int    i, j;
    int    pattern, word_count;
    /* check params */
    if((!gd->script_mod_buf) || (gd->script_main_key_count <= 0))
    {
        return SCRIPT_PARSER_EMPTY_BUFFER;
    }

    if((main_name == NULL) || (sub_name == NULL))
    {
		return SCRIPT_PARSER_KEYNAME_NULL;
    }

    if(value == NULL)
    {
		return SCRIPT_PARSER_DATA_VALUE_NULL;
    }

    /* truncate string if size >31 bytes */
    main_char = main_name;
    if(_test_str_length(main_name) > 31)
    {
        memset(main_bkname, 0, 32);
        strncpy(main_bkname, main_name, 31);
        main_char = main_bkname;
    }
    sub_char = sub_name;
    if(_test_str_length(sub_name) > 31)
    {
        memset(sub_bkname, 0, 32);
        strncpy(sub_bkname, sub_name, 31);
        sub_char = sub_bkname;
    }
    for(i=0;i<gd->script_main_key_count;i++)
    {
        main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
        if(strcmp(main_key->main_name, main_char))
        {
            continue;
        }

        /* now find sub key */
        for(j=0;j<main_key->lenth;j++)
        {
            sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
            if(strcmp(sub_key->sub_name, sub_char))
            {
                continue;
            }
            pattern    = (sub_key->pattern>>16) & 0xffff; /* get datatype */
            word_count = (sub_key->pattern>> 0) & 0xffff; /*get count of word */

            switch(pattern)
            {
                case SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD:
                    value[0] = *(int *)(gd->script_mod_buf + (sub_key->offset<<2));
                    break;

                case SCIRPT_PARSER_VALUE_TYPE_STRING:
                    if(count < word_count)
                    {
                        word_count = count;
                    }
                    memcpy((char *)value, gd->script_mod_buf + (sub_key->offset<<2), word_count << 2);
                    break;

                case SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD:
                    break;
                case SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD:
                {
                    script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)value;
                    /* buffer space enough? */
                    if(sizeof(script_gpio_set_t) > (count<<2))
                    {
                        return SCRIPT_PARSER_BUFFER_NOT_ENOUGH;
                    }
                    strcpy( user_gpio_cfg->gpio_name, sub_char);
                    memcpy(&user_gpio_cfg->port, gd->script_mod_buf + (sub_key->offset<<2),  sizeof(script_gpio_set_t) - 32);
                    break;
                }
            }

            return SCRIPT_PARSER_OK;
        }
    }

    return SCRIPT_PARSER_KEY_NOT_FIND;
}

int script_parser_fetch_ex(char *main_name, char *sub_name, int value[], script_parser_value_type_t *type, int count)
{
    char   main_bkname[32], sub_bkname[32];
    char   *main_char, *sub_char;
    script_main_key_t  *main_key = NULL;
    script_sub_key_t   *sub_key = NULL;
    int    i, j;
    int    pattern, word_count;
    /* check params */
    if((!gd->script_mod_buf) || (gd->script_main_key_count <= 0))
    {
        return SCRIPT_PARSER_EMPTY_BUFFER;
    }

    if((main_name == NULL) || (sub_name == NULL))
    {
		return SCRIPT_PARSER_KEYNAME_NULL;
    }

    if(value == NULL)
    {
		return SCRIPT_PARSER_DATA_VALUE_NULL;
    }

    /* truncate string if size >31 bytes */
    main_char = main_name;
    if(_test_str_length(main_name) > 31)
    {
        memset(main_bkname, 0, 32);
        strncpy(main_bkname, main_name, 31);
        main_char = main_bkname;
    }
    sub_char = sub_name;
    if(_test_str_length(sub_name) > 31)
    {
        memset(sub_bkname, 0, 32);
        strncpy(sub_bkname, sub_name, 31);
        sub_char = sub_bkname;
    }
    for(i=0;i<gd->script_main_key_count;i++)
    {
        main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
        if(strcmp(main_key->main_name, main_char))
        {
            continue;
        }

        /* now find sub key */
        for(j=0;j<main_key->lenth;j++)
        {
            sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
            if(strcmp(sub_key->sub_name, sub_char))
            {
                continue;
            }
            pattern    = (sub_key->pattern>>16) & 0xffff; /* get datatype */
            word_count = (sub_key->pattern>> 0) & 0xffff; /*get count of word */

            switch(pattern)
            {
                case SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD:
                    value[0] = *(int *)(gd->script_mod_buf + (sub_key->offset<<2));
                    *type = SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD;
                    break;

                case SCIRPT_PARSER_VALUE_TYPE_STRING:
                    if(count < word_count)
                    {
                        word_count = count;
                    }
                    memcpy((char *)value, gd->script_mod_buf + (sub_key->offset<<2), word_count << 2);
                    *type = SCIRPT_PARSER_VALUE_TYPE_STRING;
                    break;

                case SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD:
					*type = SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD;
                    break;
                case SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD:
                {
                    script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)value;
                    /* buffer space enough? */
                    if(sizeof(script_gpio_set_t) > (count<<2))
                    {
                        return SCRIPT_PARSER_BUFFER_NOT_ENOUGH;
                    }
                    strcpy( user_gpio_cfg->gpio_name, sub_char);
                    memcpy(&user_gpio_cfg->port, gd->script_mod_buf + (sub_key->offset<<2),  sizeof(script_gpio_set_t) - 32);
                    *type = SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD;
                    break;
                }
            }

            return SCRIPT_PARSER_OK;
        }
    }

    return SCRIPT_PARSER_KEY_NOT_FIND;
}

int script_parser_patch_all(char *main_name, void *str, uint data_count)
{
	script_main_key_t  *main_key = NULL;
	script_sub_key_t   *sub_key = NULL;
	int    i, j;
	int    data_max;
	int    pattern;
	uint   *data = (uint *)str;


	if(!gd->script_mod_buf)
	{
		return SCRIPT_PARSER_EMPTY_BUFFER;
	}

	if(main_name == NULL)
	{
		return SCRIPT_PARSER_KEYNAME_NULL;
	}

	if(str == NULL)
	{
		return SCRIPT_PARSER_DATA_VALUE_NULL;
	}

	for(i=0;i<gd->script_main_key_count;i++)
	{
		main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
		if(strcmp(main_key->main_name, main_name))  
		{
			continue;
		}
		if(data_count > main_key->lenth)
		{
			data_max = main_key->lenth;
		}
		else
		{
			data_max = data_count;
		}
	
		for(j=0;j<data_max;j++)
		{
			sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));

			pattern    = (sub_key->pattern>>16) & 0xffff;
		
			if(pattern == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) 
			{
				*(int *)(gd->script_mod_buf + (sub_key->offset<<2)) = *(int *)data;
				data ++;
    		}
		}

		return 0;
	}

	return -1;
}

int script_parser_patch(char *main_name, char *sub_name, void *str, int str_size)
{
	char   main_bkname[32], sub_bkname[32];
	char   *main_char, *sub_char;
	script_main_key_t  *main_key = NULL;
	script_sub_key_t   *sub_key = NULL;
	int    i, j;
	int    pattern, word_count;


	if(!gd->script_mod_buf)
	{
		return SCRIPT_PARSER_EMPTY_BUFFER;
	}

	if((main_name == NULL) || (sub_name == NULL))
	{
		return SCRIPT_PARSER_KEYNAME_NULL;
	}
	
	if(str == NULL)
	{
		return SCRIPT_PARSER_DATA_VALUE_NULL;
	}

	main_char = main_name;
	if(_test_str_length(main_name) > 31)
	{
	    memset(main_bkname, 0, 32);
		strncpy(main_bkname, main_name, 31);
		main_char = main_bkname;
	}
    sub_char = sub_name;
	if(_test_str_length(sub_name) > 31)
	{
		memset(sub_bkname, 0, 32);
		strncpy(sub_bkname, sub_name, 31);
		sub_char = sub_bkname;
	}
	for(i=0;i<gd->script_main_key_count;i++)
	{
		main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
		if(strcmp(main_key->main_name, main_char))
		{
			continue;
		}
	
		for(j=0;j<main_key->lenth;j++)
		{
			sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
			if(strcmp(sub_key->sub_name, sub_char))
			{
				continue;
			}
			pattern    = (sub_key->pattern>>16) & 0xffff;
			word_count = (sub_key->pattern>> 0) & 0xffff;
		
			if(pattern == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD)
			{
				*(int *)(gd->script_mod_buf + (sub_key->offset<<2)) = *(int *)str;

    			return SCRIPT_PARSER_OK;
    		}
    		else if(pattern == SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD)
    		{
    			script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)str;

    			memset(gd->script_mod_buf + (sub_key->offset<<2), 0, 24);
    			memcpy(gd->script_mod_buf + (sub_key->offset<<2), &user_gpio_cfg->port, 24);

    			return SCRIPT_PARSER_OK;
    		}
    		else if(pattern == SCIRPT_PARSER_VALUE_TYPE_STRING)
    		{
    			if(str_size > word_count)
				{
					str_size = word_count;
				}
				memset(gd->script_mod_buf + (sub_key->offset<<2), 0, word_count << 2);
				memcpy(gd->script_mod_buf + (sub_key->offset<<2), str, str_size << 2);

				return SCRIPT_PARSER_OK;
    		}
		}
	}

	return SCRIPT_PARSER_KEY_NOT_FIND;
}

int script_parser_subkey_count(char *main_name)
{
    char   main_bkname[32];
    char   *main_char;
    script_main_key_t  *main_key = NULL;
    int    i;

    if(!gd->script_mod_buf)
    {
        return SCRIPT_PARSER_EMPTY_BUFFER;
    }

    if(main_name == NULL)
    {
        return SCRIPT_PARSER_KEYNAME_NULL;
    }

    main_char = main_name;
    if(_test_str_length(main_name) > 31)
    {
        memset(main_bkname, 0, 32);
        strncpy(main_bkname, main_name, 31);
        main_char = main_bkname;
    }

    for(i=0;i<gd->script_main_key_count;i++)
    {
        main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
        if(strcmp(main_key->main_name, main_char))
        {
            continue;
        }

        return main_key->lenth;
    }

    return -1;
}

int script_parser_mainkey_count(void)
{
    if(!gd->script_mod_buf)
    {
        return SCRIPT_PARSER_EMPTY_BUFFER;
    }

    return  gd->script_main_key_count;
}

int script_parser_mainkey_get_gpio_count(char *main_name)
{
    char   main_bkname[32];
    char   *main_char;
    script_main_key_t  *main_key = NULL;
    script_sub_key_t   *sub_key = NULL;
    int    i, j;
    int    pattern, gpio_count = 0;

    if(!gd->script_mod_buf)
    {
        return SCRIPT_PARSER_EMPTY_BUFFER;
    }

    if(main_name == NULL)
    {
        return SCRIPT_PARSER_KEYNAME_NULL;
    }

    main_char = main_name;
    if(_test_str_length(main_name) > 31)
    {
        memset(main_bkname, 0, 32);
        strncpy(main_bkname, main_name, 31);
        main_char = main_bkname;
    }

    for(i=0;i<gd->script_main_key_count;i++)
    {
        main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
        if(strcmp(main_key->main_name, main_char))
        {
            continue;
        }

        for(j=0;j<main_key->lenth;j++)
        {
            sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));

            pattern    = (sub_key->pattern>>16) & 0xffff;

            if(SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD == pattern)
            {
                gpio_count ++;
            }
        }
    }

    return gpio_count;
}

int script_parser_mainkey_get_gpio_cfg(char *main_name, void *gpio_cfg, int gpio_count)
{
    char   main_bkname[32];
    char   *main_char;
    script_main_key_t  *main_key = NULL;
    script_sub_key_t   *sub_key = NULL;
    script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)gpio_cfg;
    int    i, j;
    int    pattern, user_index;

    if(!gd->script_mod_buf)
    {
        return SCRIPT_PARSER_EMPTY_BUFFER;
    }

    if(main_name == NULL)
    {
        return SCRIPT_PARSER_KEYNAME_NULL;
    }

    memset(user_gpio_cfg, 0, sizeof(script_gpio_set_t) * gpio_count);

    main_char = main_name;
    if(_test_str_length(main_name) > 31)
    {
        memset(main_bkname, 0, 32);
        strncpy(main_bkname, main_name, 31);
        main_char = main_bkname;
    }

    for(i=0;i<gd->script_main_key_count;i++)
    {
        main_key = (script_main_key_t *)(gd->script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
        if(strcmp(main_key->main_name, main_char))
        {
            continue;
        }

       /* printf("mainkey name = %s\n", main_key->main_name);*/
        user_index = 0;
        for(j=0;j<main_key->lenth;j++)
        {
            sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
          /*  printf("subkey name = %s\n", sub_key->sub_name);*/
            pattern    = (sub_key->pattern>>16) & 0xffff;
           /* printf("subkey pattern = %d\n", pattern);*/

            if(SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD == pattern)
            {
                strcpy( user_gpio_cfg[user_index].gpio_name, sub_key->sub_name);
                memcpy(&user_gpio_cfg[user_index].port, gd->script_mod_buf + (sub_key->offset<<2), sizeof(script_gpio_set_t) - 32);
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

ulong gpio_request_ex(char *main_name, const char *sub_name) 
{
	    user_gpio_set_t    *gpio_list=NULL;
    user_gpio_set_t     one_gpio;
       __u32               gpio_handle;
    __s32               gpio_count;

    if(!sub_name){
            gpio_count = script_parser_mainkey_get_gpio_count(main_name);
            if(gpio_count <= 0)
            {
                /*printf("err: gpio count < =0 ,gpio_count is: %d \n", gpio_count);*/
                return 0;
            }
            gpio_list = (user_gpio_set_t *)malloc(sizeof(system_gpio_set_t) * gpio_count);
            if(!gpio_list){
         /*   printf("malloc gpio_list error \n");*/
                return 0;
            }
        if(!script_parser_mainkey_get_gpio_cfg(main_name,gpio_list,gpio_count)){
            gpio_handle = gpio_request(gpio_list, gpio_count);
            free(gpio_list);

        }else{
            return 0;
        }
        }else{
            if(script_parser_fetch((char *)main_name, (char *)sub_name, (int *)&one_gpio, (sizeof(user_gpio_set_t) >> 2)) < 0){
           /* printf("script parser fetch err. \n");*/
            return 0;
            }

            gpio_handle = gpio_request(&one_gpio, 1);
        }

        return gpio_handle;

}


int gpio_request_simple(char *main_name, const char *sub_name)
{
    user_gpio_set_t     gpio_list[16];
    __s32               gpio_count;
    int ret = -1;

    if(!sub_name)
    {
        gpio_count = script_parser_mainkey_get_gpio_count(main_name);
        if(gpio_count <= 0)
        {
            printf("err: gpio count < =0 ,gpio_count is: %d \n", gpio_count);
            return -1;
        }
        memset(gpio_list, 0, 16 * sizeof(user_gpio_set_t));
        if(!script_parser_mainkey_get_gpio_cfg(main_name, gpio_list, gpio_count))
        {
            ret = gpio_request_early(gpio_list, gpio_count, 1);
        }
    }
    else
    {
        if(script_parser_fetch((char *)main_name, (char *)sub_name, (int *)gpio_list, (sizeof(user_gpio_set_t) >> 2)) < 0)
        {
       		printf("script parser fetch err. \n");
        	return 0;
        }

        ret = gpio_request_early(gpio_list, 1, 1);
    }

    return ret;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void upper(char *str)
{
	int i=0;
	char c;

	do
	{
		c=str[i];
		if(c=='\0')
		{
			return;
		}
		if((c>='a') && (c<='z'))
		{
			str[i]-=('a'-'A');
		}
		i++;
	}
	while(1);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void lower(char *str)
{
	int i=0;
	char c;

	do
	{
		c=str[i];
		if(c=='\0')
		{
			return;
		}
		if((c>='A') && (c<='Z'))
		{
			str[i]+=('a'-'A');
		}
		i++;
	}
	while(1);
}