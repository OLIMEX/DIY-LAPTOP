/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Young <guoyingyang@allwinnertech.com>
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

#include <common.h>
#include <config.h>
#include <command.h>
#include <sys_config.h>
#include "linux/ctype.h"
#include <malloc.h>
#include <cli.h>

#undef CFGDEBUG
#ifdef CFGDEBUG
#define CFGINFO(fmt...)	printf("[CFG]: "fmt)
#else
#define CFGINFO(fmt...)
#endif


DECLARE_GLOBAL_DATA_PTR;

#define FORCE_EXIT -1
#define CONTINUE    0
#define WRITE_FLAG  1

extern char console_buffer[];
int sunxi_sprite_download_uboot(void *buffer, int production_media, int mode);


/************************************************************************************************************
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

/*judge user data */
static inline uint judge_num(int *num_addr)
{
    int nbytes = 0;
    int value = 0;
    int write_flag = 0;
    char* endp;
    nbytes = cli_readline("?");
    if(nbytes == -1)
        return FORCE_EXIT;
    else
    {
	while((write_flag == 0)&&(nbytes != 0))
        {
            value = simple_strtol(console_buffer, &endp, 10);
            nbytes = endp - console_buffer ;
            if(nbytes)
            {
                *num_addr = value;
                write_flag = 1;
                return WRITE_FLAG;
            }
            else
            {
                printf("you pressed num is not invail ,press again ");
                nbytes =  cli_readline("?");
            }
            if(nbytes == -1)
                return FORCE_EXIT;
        }
    }
    return CONTINUE;
}
/************************************************************************************************************
*
*                                             function
*
*    name          :deal_with_gpio_data
*
*    parmeters     :script_sub_key_t  script_gpio_set_t
*
*    return        :FORCE_EXIT : ctrl+C   CONTINUE: no change    WRITE_FLAG :change
*
*    note          :guoyingyang<guoyingyang@allwinnertech.com>
*
*
************************************************************************************************************
*/
/*display information if data_mode is GPIO*/
static uint deal_with_gpio_data(script_sub_key_t *sub_key,script_gpio_set_t* point)
{
    int nbytes = 0;
    int value = 0;
    int write_flag = 0;
    if(sub_key == NULL)
    {
        printf("gpio_err: sub_key is invail\n");
        return FORCE_EXIT;
    }
    
    memcpy(&point->port, gd->script_mod_buf + (sub_key->offset<<2), sizeof(script_gpio_set_t) - 32);
    printf("\n======================================\n"); 
    printf("PORT : %c",(char)(point->port+'A'-1));
    nbytes = cli_readline("?");
    CFGINFO("nbytes = %d \n",nbytes);
    if(nbytes == -1)
        return FORCE_EXIT;

    else if(nbytes == 1)
    {
        while((write_flag == 0)&&(nbytes == 1))
        {
            if(islower(console_buffer[0]))
            {
                point->port = (int)(console_buffer[0]-'a'+1);
                write_flag ++;
            }
            else if(isupper(console_buffer[0]))
            {
                point->port = (int)(console_buffer[0]-'A'+1);
                write_flag ++;
            }
            else
            {
                printf("gpio_err : you pressed gpio port is invalid,press again \n");
                nbytes = cli_readline("?");
            }
            if(nbytes == -1)
                goto end_err;
        }
    }
    printf("PORT_NUM :%d",point->port_num);
    value = judge_num(&point->port_num);
    if(value == FORCE_EXIT) 
        goto end_err;
    write_flag += value;
    
    printf("MUL_SEL :%d",point->mul_sel);
    value = judge_num(&point->mul_sel);
    if(value == FORCE_EXIT) 
        goto end_err;
    write_flag += value;
    
    printf("PULL :%d",point->pull);
    value = judge_num(&point->pull);
    if(value == FORCE_EXIT) 
        goto end_err;
    write_flag += value;
    
    printf("DRV_LEVEL :%d",point->drv_level);
    if(judge_num(&point->drv_level) == FORCE_EXIT) 
    value = judge_num(&point->pull);
    if(value == FORCE_EXIT) 
        goto end_err;
    write_flag += value;
    
    printf("DATA :%d",point->data);
    value = judge_num(&point->data);
    if(value == FORCE_EXIT) 
        goto end_err;
    write_flag += value;
    printf("====================================\n");
    if(write_flag)
    {
        CFGINFO("copy gpio info to sys_config\n");
        memcpy(gd->script_mod_buf+(sub_key->offset<<2),&point->port,24);
        return WRITE_FLAG;
    }
    return CONTINUE;
end_err:
    return FORCE_EXIT;
}
/************************************************************************************************************
*
*                                             function
*
*    name          :deal_with_data
*
*    parmeters     :sub_key  data_mode
*
*    return        :FORCE_EXIT : ctrl+C   CONTINUE: no change    WRITE_FLAG :change
*
*    note          :guoyingyang<guoyingyang@allwinnertech.com>
*
*
************************************************************************************************************
*/
/*judge data which user pressed*/
static uint deal_with_data(script_sub_key_t * sub_key ,uint data_mode)
{
    int value = 0;
    int value_input = 0;
    int nbytes = 0;
    int write_flag = 0;
    script_gpio_set_t user_input ;
    //deal different pattern
    if((data_mode == SCIRPT_PARSER_VALUE_TYPE_INVALID) ||(sub_key == NULL))
    {
        printf("[config err] :pattern or sub_key is invaild\n");
        return -1;
    }

    switch(data_mode)
    {
        case SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD:
            CFGINFO("SINGLE_WORD");
            printf("%d",*(int *)(gd->script_mod_buf+(sub_key->offset<<2)));
            value = *(int *)(gd->script_mod_buf+(sub_key->offset<<2));
            write_flag =  judge_num(&value);
            if(write_flag == WRITE_FLAG)
                *(int*)(gd->script_mod_buf+(sub_key->offset<<2)) = value;
            break;

        case SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD:
            CFGINFO("TYPE_GPIO");
            return (deal_with_gpio_data(sub_key,&user_input));
            break;
        
        case SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD:
            break;
        
        case SCIRPT_PARSER_VALUE_TYPE_STRING:
            CFGINFO("TYPE_STRING");
            printf("%s\n",(char *)(gd->script_mod_buf+(sub_key->offset<<2)));
            nbytes = cli_readline("?");
            if(nbytes==-1)
                return FORCE_EXIT;
            else if(nbytes == 0)
                return CONTINUE;
            else
            {
                value = strlen((char*)gd->script_mod_buf+(sub_key->offset<<2));
                value = ((value>>2)<<2)+4; //four byte align
                CFGINFO("string : %d\n",value);
                value_input = strlen(console_buffer);
                CFGINFO("value_input:%d\n",value_input+1);
                if(value_input+1 > value)
                {
                    printf("\nstring is too loog\n");
                    return 0;
                }
                console_buffer[value_input] = '\0';
                memcpy((char*)gd->script_mod_buf+(sub_key->offset<<2),console_buffer,value_input+1);
                write_flag = 1;
            }
            break;
        
        default:
            printf("default\n");
    }

    return write_flag;
}
/************************************************************************************************************
*
*                                             function
*
*    name          :do_modify_cfg
*
*    parmeters     :
*
*    return        :FORCE_EXIT : ctrl+C   CONTINUE: no change    WRITE_FLAG :change
*
*    note          :guoyingyang<guoyingyang@allwinnertech.com>
*					set sys_config para if you want to change sub_key value
*
************************************************************************************************************
*/

int do_modify_cfg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	script_main_key_t * main_key = NULL;
	script_sub_key_t  * sub_key  = NULL;
	uint    data_mode = 0;
        uint write_flag = 0;
        int i = 0;

	if ((argc != 3)&&(argc !=2))
		return cmd_usage(cmdtp);
	
	//find main_key
        main_key = (script_main_key_t *)script_parser_fetch_subkey_start(argv[1]);
	if(main_key == NULL)
        {
            printf("[config_err]: can not find main_key \n");
            return -1;
        }
        
        //find sub key
        printf("--%s--\n",main_key->main_name);
        if(argc == 3)
        {
            sub_key = (script_sub_key_t *)script_parser_subkey(main_key,argv[2],&data_mode);
            if(sub_key == NULL)
            {
                printf("[config_err]: can not find sub_key \n");
                return -1;
            }
            printf(" %s : ",sub_key->sub_name);
        
            //deal with data_mode
            write_flag = deal_with_data(sub_key,data_mode);
            if(write_flag == FORCE_EXIT)
                return FORCE_EXIT;
            gd->force_download_uboot += write_flag;
        }
        else if(argc == 2)
        {
            CFGINFO("find sub_key from beginning to end \n");
            for(i = 0; i < main_key->lenth ; i++)
            {
                sub_key = (script_sub_key_t *)(gd->script_mod_buf + (main_key->offset<<2) + (i * sizeof(script_sub_key_t)));
                printf("%s :",sub_key->sub_name);
                data_mode = (sub_key->pattern>>16)&0xffff;
                write_flag = deal_with_data(sub_key,data_mode);
                if(write_flag == FORCE_EXIT)
                    return FORCE_EXIT;
                gd->force_download_uboot += write_flag;
            }
        }
        return 0;
}

U_BOOT_CMD(
	setcfg, 3, 0, do_modify_cfg,
	"modify sys_config.fex ",
	"modify_cfg main_key sub_key or modify_cfg main_key"
);

/************************************************************************************************************
*
*                                             function
*
*    name          :do_savecfg
*
*    parmeters     :
*
*    return        :FORCE_EXIT : ctrl+C   CONTINUE: no change    WRITE_FLAG :change
*
*    note          :guoyingyang<guoyingyang@allwinnertech.com>
*					save sys_config.fex to flash if you set para to sys_config
*
*
************************************************************************************************************
*/
int do_savecfg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char *tmp_target_buffer = NULL;
    int script_offset = 0;
    int script_length = 0;
    if(argc != 1)
        return cmd_usage(cmdtp);

    if(gd->force_download_uboot >= 1)
    {
        //get script_offset and script_length
        script_offset = uboot_spare_head.boot_head.uboot_length;
        script_length = uboot_spare_head.boot_head.length - script_offset;
       // tmp_target_buffer = (void *)(CONFIG_SYS_TEXT_BASE -0x01000000);
        tmp_target_buffer = (char*)malloc(1024*1024);
        if(tmp_target_buffer == NULL)
        {
            printf("config err: no enough memory to malloc \n");
            return FORCE_EXIT;
        }
        //copy uboot.bin to CONFIG_SYS_TEXT_BASE -0x01000000
        memcpy(tmp_target_buffer,(char *)CONFIG_SYS_TEXT_BASE,uboot_spare_head.boot_head.uboot_length);
        //copy sys_config.bin to tmp_buf
        memcpy(tmp_target_buffer+script_offset,(char*)gd->script_mod_buf,script_length);

        //download uboot
        gd->force_download_uboot = 0;
        sunxi_sprite_download_uboot(tmp_target_buffer,uboot_spare_head.boot_data.storage_type,1);
        free(tmp_target_buffer);
        tmp_target_buffer = NULL;
    }
    return 0;
}


U_BOOT_CMD(
	savecfg, 1, 0, do_savecfg,
	"save sys_config into flash if you execute command setcfg",
	"savecfg"
);




