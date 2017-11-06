// update.cpp : Defines the entry point for the console application.
//

#include "common.h"

__asm__(".symver memcpy ,memcpy@GLIBC_2.2.5");
void *script_file_decode(char *script_name);
int update_for_toc0(char *toc0_name);

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
	printf("update_toc0 script file path para file path\n\n");
}

int main(int argc, char* argv[])
{
	char   source_toc0_name[MAX_PATH];
	char   script_file_name[MAX_PATH];
	char   *script_buf = NULL;
	int    ret = -1;

	GetFullPath(source_toc0_name,  argv[1]);
	GetFullPath(script_file_name,   argv[2]);

	printf("\n");
	printf("toc0 file Path=%s\n", source_toc0_name);
	printf("script file Path=%s\n", script_file_name);
	printf("\n");
	//初始化配置脚本
	script_buf = (char *)script_file_decode(script_file_name);
	if(!script_buf)
	{
		printf("update toc0 error: unable to get script data\n");

		goto _err_out;
	}
	script_parser_init(script_buf);
	//读取原始toc0
	if(update_for_toc0(source_toc0_name))
	{
		printf("script update toc0 fail\n");

		goto _err_out;
	}
    //获取原始脚本长度
	printf("script update toc0 ok\n");

	ret = 0;
_err_out:
	if(script_buf)
	{
		free(script_buf);
	}

	return ret;
}


int update_for_toc0(char *toc0_name)
{
	FILE *toc0_file = NULL;
	char *toc0_buf = NULL;
	int   length = 0;
	int   i, j;
	int   ret = -1;
	int   value[8];
    script_gpio_set_t   gpio_set[32];
    sbrom_toc0_head_info_t *toc0_head;
    sbrom_toc0_config_t *toc0_config;

	toc0_file = fopen(toc0_name, "rb+");
	if(toc0_file == NULL)
	{
		printf("update:unable to open toc0 file\n");
		goto _err_toc0_out;
	}
	fseek(toc0_file, 0, SEEK_END);
	length = ftell(toc0_file);
	fseek(toc0_file, 0, SEEK_SET);
	if(!length)
	{
		goto _err_toc0_out;
	}
	toc0_buf = (char *)malloc(length);
	if(!toc0_buf)
	{
		goto _err_toc0_out;
	}
	fread(toc0_buf, length, 1, toc0_file);
	rewind(toc0_file);

	toc0_head = (sbrom_toc0_head_info_t *)toc0_buf;
	toc0_config = (sbrom_toc0_config_t *)(toc0_buf + 0x80);
	//检查toc0的数据结构是否完整
//    ret = check_file( (unsigned int *)toc0_buf, toc0_head->boot_head.length, TOC0_MAGIC );
//    if( ret != CHECK_IS_CORRECT )
//    {
//		goto _err_toc0_out;
//	}
	//取出数据进行修正,DRAM参数
	if(script_parser_sunkey_all("dram_para", (void *)toc0_config->dram_para))
	{
		printf("script fetch dram para failed\n");
		goto _err_toc0_out;
	}
	//update power_mode para
	if(!script_parser_fetch("target", "power_mode", value))
	{
		toc0_config->power_mode = value[0];
		printf("toc0_config->power_mode %d\n",toc0_config->power_mode);
	}
	//取出数据进行修正,UART参数
	if(!script_parser_fetch("uart_para", "uart_debug_port", value))
	{
		toc0_config->uart_port = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("uart_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			toc0_config->uart_ctrl[i].port      = gpio_set[i].port;
			toc0_config->uart_ctrl[i].port_num  = gpio_set[i].port_num;
			toc0_config->uart_ctrl[i].mul_sel   = gpio_set[i].mul_sel;
			toc0_config->uart_ctrl[i].pull      = gpio_set[i].pull;
			toc0_config->uart_ctrl[i].drv_level = gpio_set[i].drv_level;
			toc0_config->uart_ctrl[i].data      = gpio_set[i].data;
		}
	}
	//取出数据进行修正,debugenable参数
	if(!script_parser_fetch("jtag_para", "jtag_enable", value))
	{
		toc0_config->enable_jtag = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("jtag_para", gpio_set, 32))
	{
		for(i=0;i<5;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			toc0_config->jtag_gpio[i].port      = gpio_set[i].port;
			toc0_config->jtag_gpio[i].port_num  = gpio_set[i].port_num;
			toc0_config->jtag_gpio[i].mul_sel   = gpio_set[i].mul_sel;
			toc0_config->jtag_gpio[i].pull      = gpio_set[i].pull;
			toc0_config->jtag_gpio[i].drv_level = gpio_set[i].drv_level;
			toc0_config->jtag_gpio[i].data      = gpio_set[i].data;
		}
	}
	//取出数据进行修正，NAND参数
	if(!script_parser_mainkey_get_gpio_cfg("nand_para", gpio_set, 32))
	{
		for(i=NAND0_GPIO_START, j=0; i<NAND0_GPIO_START+NAND0_GPIO_MAX;i++, j++)
		{
			if(!gpio_set[j].port)
			{
				break;
			}
			toc0_config->storage_gpio[i].port      = gpio_set[j].port;
			toc0_config->storage_gpio[i].port_num  = gpio_set[j].port_num;
			toc0_config->storage_gpio[i].mul_sel   = gpio_set[j].mul_sel;
			toc0_config->storage_gpio[i].pull      = gpio_set[j].pull;
			toc0_config->storage_gpio[i].drv_level = gpio_set[j].drv_level;
			toc0_config->storage_gpio[i].data      = gpio_set[j].data;
		}
	}
	//取得卡0参赛
	if(!script_parser_mainkey_get_gpio_cfg("card0_boot_para", gpio_set, 32))
	{
		for(i=CARD0_GPIO_START, j=0;i<CARD0_GPIO_START+CARD0_GPIO_MAX;i++, j++)
		{
			if(!gpio_set[j].port)
			{
				break;
			}
			toc0_config->storage_gpio[i].port      = gpio_set[j].port;
			toc0_config->storage_gpio[i].port_num  = gpio_set[j].port_num;
			toc0_config->storage_gpio[i].mul_sel   = gpio_set[j].mul_sel;
			toc0_config->storage_gpio[i].pull      = gpio_set[j].pull;
			toc0_config->storage_gpio[i].drv_level = gpio_set[j].drv_level;
			toc0_config->storage_gpio[i].data      = gpio_set[j].data;
		}
	}
	//取得卡2参赛
	if(!script_parser_mainkey_get_gpio_cfg("card2_boot_para", gpio_set, 32))
	{
		for(i=CARD2_GPIO_START, j=0;i<CARD2_GPIO_START+CARD2_GPIO_MAX;i++, j++)
		{
			if(!gpio_set[j].port)
			{
				break;
			}
			toc0_config->storage_gpio[i].port      = gpio_set[j].port;
			toc0_config->storage_gpio[i].port_num  = gpio_set[j].port_num;
			toc0_config->storage_gpio[i].mul_sel   = gpio_set[j].mul_sel;
			toc0_config->storage_gpio[i].pull      = gpio_set[j].pull;
			toc0_config->storage_gpio[i].drv_level = gpio_set[j].drv_level;
			toc0_config->storage_gpio[i].data      = gpio_set[j].data;
		}
	}
	//取得spi0参数
	if(!script_parser_mainkey_get_gpio_cfg("card2_boot_para", gpio_set, 32))
	{
		for(i=SPI0_GPIO_START, j=0;i<SPI0_GPIO_START+SPI0_GPIO_MAX;i++, j++)
		{
			if(!gpio_set[j].port)
			{
				break;
			}
			toc0_config->storage_gpio[i].port      = gpio_set[j].port;
			toc0_config->storage_gpio[i].port_num  = gpio_set[j].port_num;
			toc0_config->storage_gpio[i].mul_sel   = gpio_set[j].mul_sel;
			toc0_config->storage_gpio[i].pull      = gpio_set[j].pull;
			toc0_config->storage_gpio[i].drv_level = gpio_set[j].drv_level;
			toc0_config->storage_gpio[i].data      = gpio_set[j].data;
		}
	}
	//取出secure参数
	if(!script_parser_fetch("secure", "dram_region_mbytes", value))
	{
		toc0_config->secure_dram_mbytes = value[0];
	}
	if(!script_parser_fetch("secure", "drm_start_mbytes", value))
	{
		toc0_config->drm_start_mbytes = value[0];
	}
	if(!script_parser_fetch("secure", "drm_size_mbytes", value))
	{
		toc0_config->drm_size_mbytes = value[0];
	}
	if(!script_parser_fetch("cpu_logical_map", "cpu0", value))
	{
		toc0_config->boot_cpu = value[0];
	}
        if(!script_parser_fetch("platform","secure_without_OS",value))
        {
                toc0_config->secure_without_OS = value[0];
        }
        if(!script_parser_fetch("platform","debug_mode",value))
        {
                toc0_config->debug_mode = value[0];
        }
        else
        {
                toc0_config->debug_mode = 1;
        }
	//取 A15 外挂电源使能引脚参数
	memset(&(toc0_config->a15_power_gpio), 0, sizeof(special_gpio_cfg));
	if(!script_parser_fetch("external_power", "a15_pwr_en", (int *)&gpio_set[0]))
	{
		toc0_config->a15_power_gpio.port = gpio_set[0].port;
		toc0_config->a15_power_gpio.port_num = gpio_set[0].port_num;
		toc0_config->a15_power_gpio.mul_sel = gpio_set[0].mul_sel;
		toc0_config->a15_power_gpio.data = gpio_set[0].data;
	}

	//数据修正完毕
	//重新计算校验和
	gen_check_sum_toc0( (void *)toc0_buf );
	fwrite(toc0_buf, length, 1, toc0_file);

	ret = 0;

_err_toc0_out:
	if(toc0_buf)
	{
		free(toc0_buf);
	}
	if(toc0_file)
	{
		fclose(toc0_file);
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
