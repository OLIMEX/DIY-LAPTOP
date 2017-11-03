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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/clock.h>
#include <asm/arch/sid.h>
#include <asm/arch/platform.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/key.h>
#include <asm/arch/dma.h>

#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <power/sunxi/pmu.h>
#include <smc.h>
#include <sunxi_board.h>




/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nannd flash, mmc2.
 * We check where we boot from by checking the config
 * of the gpio pin.
 */
DECLARE_GLOBAL_DATA_PTR;

extern void power_limit_init(void);

/* do some early init */
void s_init(void)
{
	watchdog_disable();
}

void reset_cpu(ulong addr)
{
	watchdog_enable();
	while(1);
}


void v7_outer_cache_enable(void)
{
	return ;
}

void v7_outer_cache_inval_all(void)
{
	return ;
}

void v7_outer_cache_flush_range(u32 start, u32 stop)
{
	return ;
}


void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}

void disable_caches(void)
{
	icache_disable();
	dcache_disable();
}

int display_inner(void)
{
	printf("version: %s\n", uboot_spare_head.boot_head.version);

	return 0;
}

int script_init(void)
{
	uint offset, length;
	char *addr;

	offset = uboot_spare_head.boot_head.uboot_length;
	length = uboot_spare_head.boot_head.length - uboot_spare_head.boot_head.uboot_length;

	addr   = (char *)CONFIG_SYS_TEXT_BASE + offset;

	if(length == 0)
	{
		script_parser_init(NULL);
		return 0;
	}
	
	if (!(gd->flags & GD_FLG_RELOC))
	{
		script_parser_init((char *)addr);
	}
	else
	{
		script_parser_init((char *)(gd->script_reloc_buf));
	}

	#if 0  //just for test
	{
	int dcdc_vol = 0;
	script_parser_fetch("power_sply", "dcdc2_vol", &dcdc_vol, 1);
	printf("----script test ---- dcdc2_vol = %d\n",dcdc_vol);
	}
	#endif

	

	return 0;
}


struct bias_set
{
	int  vol;
	int  index;
};

int power_config_gpio_bias(void)
{
	char gpio_bias[32], gpio_name[32];
	char *gpio_name_const="pa_bias";
	char port_index;
	char *axp=NULL, *supply=NULL, *vol=NULL;
	uint main_hd;
	uint bias_vol_set;
	int  index, ret, i;
	uint port_bias_addr;
	uint vol_index, config_type;
	int  pmu_vol;
	struct bias_set bias_vol_config[8] =
		{ {1800, 0}, {2500, 6}, {2800, 9}, {3000, 0xa}, {3300, 0xd}, {0, 0} };

	main_hd = script_parser_fetch_subkey_start("gpio_bias");
	if(main_hd == 0)
	{
		printf("gpio_bias not exist\n");
		return 0;
	}

	index = 0;
	while(1)
	{
		memset(gpio_bias, 0, 32);
		memset(gpio_name, 0, 32);
		ret = script_parser_fetch_subkey_next(main_hd, gpio_name, (int *)gpio_bias, &index);
		if(!ret)
		{
			lower(gpio_name);
			lower(gpio_bias);

			port_index = gpio_name[1];
			gpio_name[1] = 'a';
			if(strcmp(gpio_name_const, gpio_name))
			{
				printf("invalid gpio bias name %s\n", gpio_name);

				continue;
			}
			gpio_name[1] = port_index;
			i=0;
			axp = gpio_bias;
			while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
			{
				i++;
			}
			gpio_bias[i++]='\0';

			if(!strcmp(axp, "constant"))
			{
				config_type = 1;
			}
			else if(!strcmp(axp, "floating"))
			{
				printf("ignore %s bias config\n", gpio_name);

				continue;
			}
			else
			{
				config_type = 0;
			}

			if(config_type == 0)
			{
				supply = gpio_bias + i;
				while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
				{
					i++;
				}
				gpio_bias[i++]='\0';
			}

			printf("supply=%s\n", supply);
			vol = gpio_bias + i;
			while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
			{
				i++;
			}

			bias_vol_set = simple_strtoul(vol, NULL, 10);
			for(i=0;i<5;i++)
			{
				if(bias_vol_config[i].vol == bias_vol_set)
				{
					break;
				}
			}
			if(i==5)
			{
				printf("invalid gpio bias set vol %d, at name %s\n", bias_vol_set, gpio_name);

				break;
			}
			vol_index = bias_vol_config[i].index;

			if((port_index >= 'a') && (port_index <= 'h'))
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_PIO_BASE + 0x300 + 0x4 * (port_index - 'a');
			}
			else if(port_index == 'j')
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_PIO_BASE + 0x300 + 0x4 * (port_index - 'a');
			}
			else if((port_index == 'l') || (port_index == 'm'))
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_RPIO_BASE + 0x300 + 0x4 * (port_index - 'l');
			}
			else
			{
				printf("invalid gpio port at name %s\n", gpio_name);

				continue;
			}
			printf("axp=%s, supply=%s, vol=%d\n", axp, supply, bias_vol_set);
			if(config_type == 1)
			{
				smc_writel(vol_index, port_bias_addr);
			}
			else
			{
				pmu_vol = axp_probe_supply_status_byname(axp, supply);
				if(pmu_vol < 0)
				{
					printf("sunxi board read %s %s failed\n", axp, supply);

					continue;
				}

				if(pmu_vol > bias_vol_set)	//pmu实际电压超过需要设置的电压
				{
					//电压降低到需要电压
					axp_set_supply_status_byname(axp, supply, bias_vol_set, 1);
					//设置寄存器
					smc_writel(vol_index, port_bias_addr);
				}
				else if(pmu_vol < bias_vol_set)	//pmu实际电压低于需要设置的电压
				{
					//设置寄存器
					smc_writel(vol_index, port_bias_addr);
					//把pmu电压调整到需要的电压
					axp_set_supply_status_byname(axp, supply, bias_vol_set, 1);
				}
				else
				{
					//如果实际电压等于需要设置电压，直接设置即可
					smc_writel(vol_index, port_bias_addr);
				}
			}
			printf("reg addr=0x%x, value=0x%x, pmu_vol=%d\n", port_bias_addr, smc_readl(port_bias_addr), bias_vol_set);
		}
		else
		{
			printf("config gpio bias voltage finish\n");

			break;
		}
	}

	return 0;
}



int power_source_init(void)
{
	int pll_cpux;
	int cpu_vol;
	int dcdc_vol;
	int axp_exist = 0;

	//PMU_SUPPLY_DCDC2 is for cpua
	if(script_parser_fetch("power_sply", "dcdc2_vol", &dcdc_vol, 1))
	{
		cpu_vol = 900;
	}
	else
	{
		cpu_vol = dcdc_vol%10000;
	}
	axp_exist =  axp_probe();
	if(axp_exist)
	{
		axp_probe_factory_mode();
		if(!axp_probe_power_supply_condition())
		{
			//PMU_SUPPLY_DCDC2 is for cpua
			if(!axp_set_supply_status(0, PMU_SUPPLY_DCDC2, cpu_vol, -1))
			{
				tick_printf("PMU: dcdc2 %d\n", cpu_vol);
				sunxi_clock_set_corepll(uboot_spare_head.boot_data.run_clock);
			}
			else
			{
				printf("axp_set_dcdc2 fail\n");
			}
		}
		else
		{
			printf("axp_probe_power_supply_condition error\n");
		}
	}
	else
	{
		printf("axp_probe error\n");
	}

	pll_cpux = sunxi_clock_get_corepll();
	tick_printf("PMU: cpux %d Mhz,AXI=%d Mhz\n", pll_cpux,sunxi_clock_get_axi());
	printf("PLL6=%d Mhz,AHB=%d Mhz, APB1=%d Mhz \n", sunxi_clock_get_pll6(),
		sunxi_clock_get_ahb(),
		sunxi_clock_get_apb());

	if(axp_exist)
	{
		axp_set_charge_vol_limit();
		axp_set_all_limit();
		axp_set_hardware_poweron_vol();
		axp_set_power_supply_output();
		power_config_gpio_bias();
		power_limit_init();
	}

	return 0;
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
void sunxi_set_fel_flag(void)
{
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = SUNXI_RUN_EFEX_FLAG;
	asm volatile("DMB SY");
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
void sunxi_clear_fel_flag(void)
{
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = 0;
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
void sunxi_set_rtc_flag(u8 flag)
{
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = flag;
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
int sunxi_probe_securemode(void)
{
//	uint reg_val;
//
//	reg_val = readl(CCM_PLL1_C0_CTRL);
//	if(!reg_val)  //读到数据全是0，那么只能是使能secure的normal模式
//	{
//		if(uboot_spare_head.boot_data.secureos_exist==1)	//如果是1，由sbromsw传递，表示存在安全系统，否则没有
//		{
//			gd->securemode = SUNXI_SECURE_MODE_WITH_SECUREOS;
//			printf("secure mode: with secureos\n");
//		}
//		else
//		{
//			gd->securemode = SUNXI_SECURE_MODE_NO_SECUREOS;		//不存在安全系统
//			printf("secure mode: no secureos\n");
//		}
//	}
//	else		 //读到数据非0，那么只能是未使能secure
//	{
//		gd->securemode = SUNXI_NORMAL_MODE;
//		printf("normal mode\n");
//	}

	return 0;
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
int sunxi_set_secure_mode(void)
{
//	int mode;
//
//	if(gd->securemode == SUNXI_NORMAL_MODE)
//	{
//		mode = sid_probe_security_mode();
//		if(!mode)
//		{
//			sid_set_security_mode();
//		}
//	}

	return 0;
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
int sunxi_get_securemode(void)
{
	return gd->securemode;
}


extern int mmc_exit(void);
void sunxi_board_close_source(void)
{
	mmc_exit();
	timer_exit();
	sunxi_key_exit();
	sunxi_flash_exit(1);
	sunxi_sprite_exit(1);
	sunxi_dma_exit();
	disable_interrupts();
	interrupt_exit();
	return ;
}

int sunxi_board_restart(int next_mode)
{
	if(!next_mode)
	{
		next_mode = PMU_PRE_SYS_MODE;
	}
	printf("set next mode %d\n", next_mode);
	axp_set_next_poweron_status(next_mode);
	
#ifdef CONFIG_SUNXI_DISPLAY
	board_display_set_exit_mode(0);
	drv_disp_exit();
#endif
	sunxi_board_close_source();
	reset_cpu(0);

	return 0;
}

int sunxi_board_shutdown(void)
{


	printf("set next system normal\n");
	axp_set_next_poweron_status(0x0);

#ifdef CONFIG_SUNXI_DISPLAY
	board_display_set_exit_mode(0);
	drv_disp_exit();
#endif
	sunxi_flash_exit(1);	
	sunxi_sprite_exit(1);
	disable_interrupts();
	interrupt_exit();

	tick_printf("power off\n");
	axp_set_hardware_poweroff_vol();
	axp_set_power_off();

	return 0;

}

void sunxi_flush_allcaches(void)
{
	icache_disable();
	flush_dcache_all();
	dcache_disable();
}


int sunxi_board_run_fel(void)
{
	sunxi_set_fel_flag();
	printf("set next system status\n");
	axp_set_next_poweron_status(PMU_PRE_SYS_MODE);
#ifdef CONFIG_SUNXI_DISPLAY
	board_display_set_exit_mode(0);
	drv_disp_exit();
#endif
	printf("sunxi_board_close_source\n");
	sunxi_board_close_source();
	sunxi_flush_allcaches();
	printf("reset cpu\n");
	reset_cpu(0);
	return 0;
}

int sunxi_board_run_fel_eraly(void)
{
	sunxi_set_fel_flag();
	printf("set next system status\n");
	axp_set_next_poweron_status(PMU_PRE_SYS_MODE);
	timer_exit();
	sunxi_key_exit();
	printf("reset cpu\n");
	reset_cpu(0);

	return 0;
}


