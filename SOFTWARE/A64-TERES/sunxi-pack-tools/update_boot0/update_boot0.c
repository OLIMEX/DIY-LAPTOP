// update.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include "boot0_v2.h"
#include "check.h"
#include "script.h"
#include <ctype.h>
#include <unistd.h>

#define  MAX_PATH             (260)


__asm__(".symver memcpy ,memcpy@GLIBC_2.2.5");
void *script_file_decode(char *script_name);
int update_for_boot0(char *boot0_name, int storage_type);
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int IsFullName(const char *FilePath)
{
    if (isalpha(FilePath[0]) && ':' == FilePath[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void GetFullPath(char *dName, const char *sName)
{
    char Buffer[MAX_PATH];

	if(IsFullName(sName))
	{
	    strcpy(dName, sName);
		return ;
	}

   /* Get the current working directory: */
   if(getcwd(Buffer, MAX_PATH ) == NULL)
   {
        perror( "getcwd error" );
        return ;
   }
   sprintf(dName, "%s/%s", Buffer, sName);
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("update.exe script file path para file path\n\n");
}

int main(int argc, char* argv[])
{
	char   str1[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\eGon\\boot1.bin";
	char   str2[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\wboot\\bootfs\\script.bin";
	char   source_boot0_name[MAX_PATH];
	char   script_file_name[MAX_PATH];
	FILE   *src_file = NULL;
//	FILE   *script_file;
//	int    source_length, script_length;
//	int    total_length;
//	char   *pbuf_source, *pbuf_script;
	int    storage_type = 0;
	char   *script_buf = NULL;


#if 1
	if(argc == 4)
	{
		if((argv[1] == NULL) || (argv[2] == NULL) || (argv[3] == NULL))
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
		if((!strcmp(argv[3], "sdmmc_card")) || (!strcmp(argv[3], "SDMMC_CARD")))
		{
			storage_type = 1;
		}
		else if((!strcmp(argv[3], "spinor_flash")) || (!strcmp(argv[3], "SPINOR_FLASH")))
		{
			storage_type = 2;
		}
	}
	else
	{
		Usage();

		return __LINE__;
	}
	GetFullPath(source_boot0_name,  argv[1]);
	GetFullPath(script_file_name,   argv[2]);
#else
	strcpy(source_boot0_name, str1);
	strcpy(script_file_name, str2);
#endif

	printf("\n");
	printf("boot0 file Path=%s\n", source_boot0_name);
	printf("script file Path=%s\n", script_file_name);
	printf("\n");
	//初始化配置脚本
	script_buf = (char *)script_file_decode(script_file_name);
	if(!script_buf)
	{
		printf("update boot0 error: unable to get script data\n");

		goto _err_out;
	}
	script_parser_init(script_buf);
	//读取原始boot0
	update_for_boot0(source_boot0_name, storage_type);
    //获取原始脚本长度
	printf("script update boot0 ok\n");
_err_out:
	if(script_buf)
	{
		free(script_buf);
	}

	return 0;
}


int update_sdcard_info(char  *buf)
{
	int    i, value[8];
	int    card_no;
	char  *tmp_buf = buf;
	script_gpio_set_t       gpio_set;
	normal_gpio_cfg        *storage_gpio;
	boot_sdcard_info_t     *card_info;

	card_info = (boot_sdcard_info_t *)(tmp_buf + 32 * sizeof(normal_gpio_cfg));
	//填写SDCARD参数
	for(i=0;i<4;i++)
	{
		char  card_str[32];

		card_info->card_no[i] = -1;
		card_no = i;
		memset(card_str, 0, 32);
		strcpy(card_str, "card0_boot_para");
		card_str[4] = '0' + card_no;
		storage_gpio = (normal_gpio_cfg *)tmp_buf + i * 8;

		if(!script_parser_fetch(card_str, "card_ctrl", value))
		{
			card_info->card_no[i] = value[0];
		}
		else
		{
			card_info->card_no[i] = -1;
			continue;
		}
		if(!script_parser_fetch(card_str, "card_high_speed", value))
		{
			card_info->speed_mode[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "card_line", value))
		{
			card_info->line_sel[i] = value[0];
			if(value[0] == 8)
			{
				printf("card line num is 8 \n");
			}
		}
		if(!script_parser_fetch(card_str, "sdc_2xmode", value))
		{
			card_info->sdc_2xmode[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "sdc_ddrmode", value))
		{
			card_info->sdc_ddrmode[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "sdc_f_max", value))
		{
			card_info->sdc_f_max[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "sdc_ex_dly_used", value))
		{
			card_info->sdc_ex_dly_used[i] = value[0];
			printf("sdc_ex_dly_used ok \n");
		}
		else
		{
			printf("sdc_ex_dly_used fail \n");
		}
		if(!script_parser_fetch(card_str, "sdc_odly_25M", value))
		{
			card_info->sdc_odly_25M[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "sdc_sdly_25M", value))
		{
			card_info->sdc_sdly_25M[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "sdc_odly_50M", value))
		{
			card_info->sdc_odly_50M[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "sdc_sdly_50M", value))
		{
			card_info->sdc_sdly_50M[i] = value[0];
		}
		//获取CLK
		memset(&gpio_set, 0, sizeof(script_gpio_set_t));
		if(!script_parser_fetch(card_str, "SDC_CLK", (int *)&gpio_set))
		{
			storage_gpio[0].port      = gpio_set.port;
			storage_gpio[0].port_num  = gpio_set.port_num;
			storage_gpio[0].mul_sel   = gpio_set.mul_sel;
			storage_gpio[0].pull      = gpio_set.pull;
			storage_gpio[0].drv_level = gpio_set.drv_level;
			storage_gpio[0].data      = gpio_set.data;
		}
		else if(!script_parser_fetch(card_str, "sdc_clk", (int *)&gpio_set))
		{
			storage_gpio[0].port      = gpio_set.port;
			storage_gpio[0].port_num  = gpio_set.port_num;
			storage_gpio[0].mul_sel   = gpio_set.mul_sel;
			storage_gpio[0].pull      = gpio_set.pull;
			storage_gpio[0].drv_level = gpio_set.drv_level;
			storage_gpio[0].data      = gpio_set.data;
		}
		else
		{
			printf("update error: unable to find SDC%d CLOCK PIN\n", i);

			return -1;
		}
		//获取CMD
		memset(&gpio_set, 0, sizeof(script_gpio_set_t));
		if(!script_parser_fetch(card_str, "SDC_CMD", (int *)&gpio_set))
		{
			storage_gpio[1].port      = gpio_set.port;
			storage_gpio[1].port_num  = gpio_set.port_num;
			storage_gpio[1].mul_sel   = gpio_set.mul_sel;
			storage_gpio[1].pull      = gpio_set.pull;
			storage_gpio[1].drv_level = gpio_set.drv_level;
			storage_gpio[1].data      = gpio_set.data;
		}
		else if(!script_parser_fetch(card_str, "sdc_cmd", (int *)&gpio_set))
		{
			storage_gpio[1].port      = gpio_set.port;
			storage_gpio[1].port_num  = gpio_set.port_num;
			storage_gpio[1].mul_sel   = gpio_set.mul_sel;
			storage_gpio[1].pull      = gpio_set.pull;
			storage_gpio[1].drv_level = gpio_set.drv_level;
			storage_gpio[1].data      = gpio_set.data;
		}
		else
		{
			printf("update error: unable to find SDC%d CLOCK CMD\n", i);

			return -1;
		}
		//获取DATA0
		memset(&gpio_set, 0, sizeof(script_gpio_set_t));
		if(!script_parser_fetch(card_str, "SDC_D0", (int *)&gpio_set))
		{
			storage_gpio[2].port      = gpio_set.port;
			storage_gpio[2].port_num  = gpio_set.port_num;
			storage_gpio[2].mul_sel   = gpio_set.mul_sel;
			storage_gpio[2].pull      = gpio_set.pull;
			storage_gpio[2].drv_level = gpio_set.drv_level;
			storage_gpio[2].data      = gpio_set.data;
		}
		if(!script_parser_fetch(card_str, "sdc_d0", (int *)&gpio_set))
		{
			storage_gpio[2].port      = gpio_set.port;
			storage_gpio[2].port_num  = gpio_set.port_num;
			storage_gpio[2].mul_sel   = gpio_set.mul_sel;
			storage_gpio[2].pull      = gpio_set.pull;
			storage_gpio[2].drv_level = gpio_set.drv_level;
			storage_gpio[2].data      = gpio_set.data;
		}
		else
		{
			printf("update error: unable to find SDC%d CLOCK DATA0\n", i);

			return -1;
		}
		if(1 != card_info->line_sel[i])
		{
			//获取DATA1
			memset(&gpio_set, 0, sizeof(script_gpio_set_t));
			if(!script_parser_fetch(card_str, "SDC_D1", (int *)&gpio_set))
			{
				storage_gpio[3].port      = gpio_set.port;
				storage_gpio[3].port_num  = gpio_set.port_num;
				storage_gpio[3].mul_sel   = gpio_set.mul_sel;
				storage_gpio[3].pull      = gpio_set.pull;
				storage_gpio[3].drv_level = gpio_set.drv_level;
				storage_gpio[3].data      = gpio_set.data;
			}
			else if(!script_parser_fetch(card_str, "sdc_d1", (int *)&gpio_set))
			{
				storage_gpio[3].port      = gpio_set.port;
				storage_gpio[3].port_num  = gpio_set.port_num;
				storage_gpio[3].mul_sel   = gpio_set.mul_sel;
				storage_gpio[3].pull      = gpio_set.pull;
				storage_gpio[3].drv_level = gpio_set.drv_level;
				storage_gpio[3].data      = gpio_set.data;
			}
			else
			{
				printf("update error: unable to find SDC%d CLOCK DATA1\n", i);

				return -1;
			}
			//获取DATA2
			memset(&gpio_set, 0, sizeof(script_gpio_set_t));
			if(!script_parser_fetch(card_str, "SDC_D2", (int *)&gpio_set))
			{
				storage_gpio[4].port      = gpio_set.port;
				storage_gpio[4].port_num  = gpio_set.port_num;
				storage_gpio[4].mul_sel   = gpio_set.mul_sel;
				storage_gpio[4].pull      = gpio_set.pull;
				storage_gpio[4].drv_level = gpio_set.drv_level;
				storage_gpio[4].data      = gpio_set.data;
			}
			else if(!script_parser_fetch(card_str, "sdc_d2", (int *)&gpio_set))
			{
				storage_gpio[4].port      = gpio_set.port;
				storage_gpio[4].port_num  = gpio_set.port_num;
				storage_gpio[4].mul_sel   = gpio_set.mul_sel;
				storage_gpio[4].pull      = gpio_set.pull;
				storage_gpio[4].drv_level = gpio_set.drv_level;
				storage_gpio[4].data      = gpio_set.data;
			}
			else
			{
				printf("update error: unable to find SDC%d CLOCK DATA2\n", i);

				return -1;
			}
			//获取DATA3
			memset(&gpio_set, 0, sizeof(script_gpio_set_t));
			if(!script_parser_fetch(card_str, "SDC_D3", (int *)&gpio_set))
			{
				storage_gpio[5].port      = gpio_set.port;
				storage_gpio[5].port_num  = gpio_set.port_num;
				storage_gpio[5].mul_sel   = gpio_set.mul_sel;
				storage_gpio[5].pull      = gpio_set.pull;
				storage_gpio[5].drv_level = gpio_set.drv_level;
				storage_gpio[5].data      = gpio_set.data;
			}
			else if(!script_parser_fetch(card_str, "sdc_d3", (int *)&gpio_set))
			{
				storage_gpio[5].port      = gpio_set.port;
				storage_gpio[5].port_num  = gpio_set.port_num;
				storage_gpio[5].mul_sel   = gpio_set.mul_sel;
				storage_gpio[5].pull      = gpio_set.pull;
				storage_gpio[5].drv_level = gpio_set.drv_level;
				storage_gpio[5].data      = gpio_set.data;
			}
			else
			{
				printf("update error: unable to find SDC%d CLOCK DATA3\n", i);

				return -1;
			}
			//support to init 8 line 
			if(8 == card_info->line_sel[i])
			{
				//获取DATA4
				memset(&gpio_set, 0, sizeof(script_gpio_set_t));
				if(!script_parser_fetch(card_str, "SDC_D4", (int *)&gpio_set))
				{
					storage_gpio[6].port      = gpio_set.port;
					storage_gpio[6].port_num  = gpio_set.port_num;
					storage_gpio[6].mul_sel   = gpio_set.mul_sel;
					storage_gpio[6].pull      = gpio_set.pull;
					storage_gpio[6].drv_level = gpio_set.drv_level;
					storage_gpio[6].data      = gpio_set.data;
				}
				else if(!script_parser_fetch(card_str, "sdc_d4", (int *)&gpio_set))
				{
					storage_gpio[6].port      = gpio_set.port;
					storage_gpio[6].port_num  = gpio_set.port_num;
					storage_gpio[6].mul_sel   = gpio_set.mul_sel;
					storage_gpio[6].pull      = gpio_set.pull;
					storage_gpio[6].drv_level = gpio_set.drv_level;
					storage_gpio[6].data      = gpio_set.data;
				}
				else
				{
					printf("update error: unable to find SDC%d CLOCK DATA4\n", i);

					return -1;
				}
				//获取DATA5
				memset(&gpio_set, 0, sizeof(script_gpio_set_t));
				if(!script_parser_fetch(card_str, "SDC_D5", (int *)&gpio_set))
				{
					storage_gpio[7].port      = gpio_set.port;
					storage_gpio[7].port_num  = gpio_set.port_num;
					storage_gpio[7].mul_sel   = gpio_set.mul_sel;
					storage_gpio[7].pull      = gpio_set.pull;
					storage_gpio[7].drv_level = gpio_set.drv_level;
					storage_gpio[7].data      = gpio_set.data;
				}
				else if(!script_parser_fetch(card_str, "sdc_d5", (int *)&gpio_set))
				{
					storage_gpio[7].port      = gpio_set.port;
					storage_gpio[7].port_num  = gpio_set.port_num;
					storage_gpio[7].mul_sel   = gpio_set.mul_sel;
					storage_gpio[7].pull      = gpio_set.pull;
					storage_gpio[7].drv_level = gpio_set.drv_level;
					storage_gpio[7].data      = gpio_set.data;
				}
				else
				{
					printf("update error: unable to find SDC%d CLOCK DATA5\n", i);

					return -1;
				}
				//获取DATA6
				memset(&gpio_set, 0, sizeof(script_gpio_set_t));
				if(!script_parser_fetch(card_str, "SDC_D6", (int *)&gpio_set))
				{
					storage_gpio[8].port      = gpio_set.port;
					storage_gpio[8].port_num  = gpio_set.port_num;
					storage_gpio[8].mul_sel   = gpio_set.mul_sel;
					storage_gpio[8].pull      = gpio_set.pull;
					storage_gpio[8].drv_level = gpio_set.drv_level;
					storage_gpio[8].data      = gpio_set.data;
				}
				else if(!script_parser_fetch(card_str, "sdc_d6", (int *)&gpio_set))
				{
					storage_gpio[8].port      = gpio_set.port;
					storage_gpio[8].port_num  = gpio_set.port_num;
					storage_gpio[8].mul_sel   = gpio_set.mul_sel;
					storage_gpio[8].pull      = gpio_set.pull;
					storage_gpio[8].drv_level = gpio_set.drv_level;
					storage_gpio[8].data      = gpio_set.data;
				}
				else
				{
					printf("update error: unable to find SDC%d CLOCK DATA6\n", i);

					return -1;
				}
				//获取DATA7
				memset(&gpio_set, 0, sizeof(script_gpio_set_t));
				if(!script_parser_fetch(card_str, "SDC_D7", (int *)&gpio_set))
				{
					storage_gpio[9].port      = gpio_set.port;
					storage_gpio[9].port_num  = gpio_set.port_num;
					storage_gpio[9].mul_sel   = gpio_set.mul_sel;
					storage_gpio[9].pull      = gpio_set.pull;
					storage_gpio[9].drv_level = gpio_set.drv_level;
					storage_gpio[9].data      = gpio_set.data;
				}
				else if(!script_parser_fetch(card_str, "sdc_d7", (int *)&gpio_set))
				{
					storage_gpio[9].port      = gpio_set.port;
					storage_gpio[9].port_num  = gpio_set.port_num;
					storage_gpio[9].mul_sel   = gpio_set.mul_sel;
					storage_gpio[9].pull      = gpio_set.pull;
					storage_gpio[9].drv_level = gpio_set.drv_level;
					storage_gpio[9].data      = gpio_set.data;
				}
				else
				{
					printf("update error: unable to find SDC%d CLOCK DATA7\n", i);

					return -1;
				}
			}
		}
	}

	return 0;
}


int update_for_boot0(char *boot0_name, int storage_type)
{
	FILE *boot0_file = NULL;
	boot0_file_head_t  *boot0_head;
	char *boot0_buf = NULL;
	int   length = 0;
	int   i;
	int   ret = -1;
	int   value[8];
    script_gpio_set_t   gpio_set[32];

	boot0_file = fopen(boot0_name, "rb+");
	if(boot0_file == NULL)
	{
		printf("update:unable to open boot0 file\n");
		goto _err_boot0_out;
	}
	fseek(boot0_file, 0, SEEK_END);
	length = ftell(boot0_file);
	fseek(boot0_file, 0, SEEK_SET);
	if(!length)
	{
		goto _err_boot0_out;
	}
	boot0_buf = (char *)malloc(length);
	if(!boot0_buf)
	{
		goto _err_boot0_out;
	}
	fread(boot0_buf, length, 1, boot0_file);
	rewind(boot0_file);

	boot0_head = (boot0_file_head_t *)boot0_buf;
	//检查boot0的数据结构是否完整
    ret = check_file( (unsigned int *)boot0_buf, boot0_head->boot_head.length, BOOT0_MAGIC );
    if( ret != CHECK_IS_CORRECT )
    {
		goto _err_boot0_out;
	}
	if(!script_parser_fetch("platform", "debug_mode", value))
	{
                printf("debug_mode \n");
		boot0_head->prvt_head.debug_mode = value[0];
	}
        else
        {
            boot0_head->prvt_head.debug_mode = 1;
        }
	//取出数据进行修正,DRAM参数
	if(script_parser_sunkey_all("dram_para", (void *)boot0_head->prvt_head.dram_para))
	{
		printf("script fetch dram para failed\n");
		goto _err_boot0_out;
	}
	//取出数据进行修正,UART参数
	if(!script_parser_fetch("uart_para", "uart_debug_port", value))
	{
		boot0_head->prvt_head.uart_port = value[0];
	}
	//取出bootcpu进行修正
	if(!script_parser_fetch("cpu_logical_map", "cpu0", value))
	{
		boot0_head->boot_head.boot_cpu = value[0];
	}
	//取出A15 使能引脚配置
	memset(&(boot0_head->boot_head.a15_power_gpio), 0, sizeof(special_gpio_cfg));
	if(!script_parser_fetch("external_power", "a15_pwr_en", (int *)&gpio_set[0]))
	{
		boot0_head->boot_head.a15_power_gpio.port			= gpio_set[0].port;
		boot0_head->boot_head.a15_power_gpio.port_num		= gpio_set[0].port_num;
		boot0_head->boot_head.a15_power_gpio.mul_sel		= gpio_set[0].mul_sel;
		boot0_head->boot_head.a15_power_gpio.data			= gpio_set[0].data;
	}
	if(!script_parser_mainkey_get_gpio_cfg("uart_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			boot0_head->prvt_head.uart_ctrl[i].port      = gpio_set[i].port;
			boot0_head->prvt_head.uart_ctrl[i].port_num  = gpio_set[i].port_num;
			boot0_head->prvt_head.uart_ctrl[i].mul_sel   = gpio_set[i].mul_sel;
			boot0_head->prvt_head.uart_ctrl[i].pull      = gpio_set[i].pull;
			boot0_head->prvt_head.uart_ctrl[i].drv_level = gpio_set[i].drv_level;
			boot0_head->prvt_head.uart_ctrl[i].data      = gpio_set[i].data;
		}
	}
	//取出数据进行修正,debugenable参数
	if(!script_parser_fetch("jtag_para", "jtag_enable", value))
	{
		boot0_head->prvt_head.enable_jtag = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("jtag_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			boot0_head->prvt_head.jtag_gpio[i].port      = gpio_set[i].port;
			boot0_head->prvt_head.jtag_gpio[i].port_num  = gpio_set[i].port_num;
			boot0_head->prvt_head.jtag_gpio[i].mul_sel   = gpio_set[i].mul_sel;
			boot0_head->prvt_head.jtag_gpio[i].pull      = gpio_set[i].pull;
			boot0_head->prvt_head.jtag_gpio[i].drv_level = gpio_set[i].drv_level;
			boot0_head->prvt_head.jtag_gpio[i].data      = gpio_set[i].data;
		}
	}
	//取出数据进行修正，NAND参数
	if(!storage_type)
	{
		if(!script_parser_mainkey_get_gpio_cfg("nand_para", gpio_set, 32))
		{
			for(i=0;i<32;i++)
			{
				if(!gpio_set[i].port)
				{
					break;
				}
				boot0_head->prvt_head.storage_gpio[i].port      = gpio_set[i].port;
				boot0_head->prvt_head.storage_gpio[i].port_num  = gpio_set[i].port_num;
				boot0_head->prvt_head.storage_gpio[i].mul_sel   = gpio_set[i].mul_sel;
				boot0_head->prvt_head.storage_gpio[i].pull      = gpio_set[i].pull;
				boot0_head->prvt_head.storage_gpio[i].drv_level = gpio_set[i].drv_level;
				boot0_head->prvt_head.storage_gpio[i].data      = gpio_set[i].data;
			}
		}
	}
	else if(1 == storage_type) //取得卡参赛
	{
		if(update_sdcard_info((char *)boot0_head->prvt_head.storage_gpio))
		{
			goto _err_boot0_out;
		}
	}
	else if(2 == storage_type)
	{
		if(!script_parser_mainkey_get_gpio_cfg("spi0_para", gpio_set, 32))
		{
			for(i=0;i<32;i++)
			{
				if(!gpio_set[i].port)
				{
					break;
				}
				boot0_head->prvt_head.storage_gpio[i].port      = gpio_set[i].port;
				boot0_head->prvt_head.storage_gpio[i].port_num  = gpio_set[i].port_num;
				boot0_head->prvt_head.storage_gpio[i].mul_sel   = gpio_set[i].mul_sel;
				boot0_head->prvt_head.storage_gpio[i].pull      = gpio_set[i].pull;
				boot0_head->prvt_head.storage_gpio[i].drv_level = gpio_set[i].drv_level;
				boot0_head->prvt_head.storage_gpio[i].data      = gpio_set[i].data;
			}
		}
	}
	//数据修正完毕
	//重新计算校验和
	gen_check_sum( (void *)boot0_buf );
	//再检查一次
    ret = check_file( (unsigned int *)boot0_buf, boot0_head->boot_head.length, BOOT0_MAGIC );
    if( ret != CHECK_IS_CORRECT )
    {
		goto _err_boot0_out;
	}
	fwrite(boot0_buf, length, 1, boot0_file);

_err_boot0_out:
	if(boot0_buf)
	{
		free(boot0_buf);
	}
	if(boot0_file)
	{
		fclose(boot0_file);
	}

	return ret;
}


void *script_file_decode(char *script_file_name)
{
	FILE  *script_file;
	void  *script_buf = NULL;
	int    script_length;
	//读取原始脚本
	script_file = fopen(script_file_name, "rb");
	if(!script_file)
	{
        printf("update error:unable to open script file\n");
		return NULL;
	}
    //获取原始脚本长度
    fseek(script_file, 0, SEEK_END);
	script_length = ftell(script_file);
	if(!script_length)
	{
		fclose(script_file);
		printf("the length of script is zero\n");

		return NULL;
	}
	//读取原始脚本
	script_buf = (char *)malloc(script_length);
	if(!script_buf)
	{
		fclose(script_file);
		printf("unable malloc memory for script\n");

		return NULL;;
	}
    fseek(script_file, 0, SEEK_SET);
	fread(script_buf, script_length, 1, script_file);
	fclose(script_file);

	return script_buf;
}
