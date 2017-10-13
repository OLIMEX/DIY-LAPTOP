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
#include <power/sunxi/pmu.h>
#include <asm/arch/timer.h>
#include <asm/arch/key.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config_old.h>
#include <asm/arch/dma.h>

/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nannd flash, mmc2.
 * We check where we boot from by checking the config
 * of the gpio pin.
 */
DECLARE_GLOBAL_DATA_PTR;

extern int sunxi_clock_get_axi(void);
extern int sunxi_clock_get_ahb(void);
extern int sunxi_clock_get_apb1(void);
extern int sunxi_clock_get_pll6(void);


u32 get_base(void)
{

	u32 val;

	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xF0000000;
	val >>= 28;
	return val;
}

/* do some early init */
void s_init(void)
{
	watchdog_disable();
}

void reset_cpu(ulong addr)
{
	watchdog_enable();
#ifndef CONFIG_A81_FPGA
loop_to_die:
	goto loop_to_die;
#endif
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
	tick_printf("version: %s\n", uboot_spare_head.boot_head.version);

	return 0;
}

int script_init(void)
{
    uint offset, length;
	char *addr;

	offset = uboot_spare_head.boot_head.uboot_length;
	length = uboot_spare_head.boot_head.length - uboot_spare_head.boot_head.uboot_length;
	addr   = (char *)CONFIG_SYS_TEXT_BASE + offset;

    debug("script offset=%x, length = %x\n", offset, length);

	if(length)
	{
		memcpy((void *)SYS_CONFIG_MEMBASE, addr, length);
		script_parser_init((char *)SYS_CONFIG_MEMBASE);
	}
	else
	{
		script_parser_init(NULL);
	}
#if defined(CONFIG_SUNXI_SCRIPT_REINIT)
	{
		void *tmp_target_buffer = (void *)(CONFIG_SYS_TEXT_BASE - 0x01000000);

		memset(tmp_target_buffer, 0, 1024 * 1024);
		memcpy(tmp_target_buffer, (void *)CONFIG_SYS_TEXT_BASE, uboot_spare_head.boot_head.length);
	}
#endif
	return 0;
}

int power_source_init(void)
{
	int pll1;
	int dcdc3_vol;

	if(script_parser_fetch("power_sply", "dcdc3_vol", &dcdc3_vol, 1))
	{
		dcdc3_vol = 1200;
	}
	if(axp_probe() > 0)
	{
		axp_probe_factory_mode();
		if(!axp_probe_power_supply_condition())
		{
			if(!axp_set_supply_status(0, PMU_SUPPLY_DCDC3, dcdc3_vol, -1))
			{
				tick_printf("PMU: dcdc3 %d\n", dcdc3_vol);
				sunxi_clock_set_corepll(uboot_spare_head.boot_data.run_clock, 0);
			}
			else
			{
				printf("axp_set_dcdc3 fail\n");
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

	pll1 = sunxi_clock_get_corepll();

	tick_printf("PMU: pll1 %d Mhz,PLL6=%d Mhz\n", pll1,sunxi_clock_get_pll6());
    printf("AXI=%d Mhz,AHB=%d Mhz, APB1=%d Mhz \n", sunxi_clock_get_axi(),sunxi_clock_get_ahb(),sunxi_clock_get_apb1());


    axp_set_charge_vol_limit();
    axp_set_all_limit();
    axp_set_hardware_poweron_vol();

	axp_set_power_supply_output();

	//power_limit_init();

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

int sunxi_probe_securemode(void)
{
	gd->securemode = SUNXI_NORMAL_MODE;
	printf("normal mode\n");
	return 0;
}

int sunxi_get_securemode(void)
{
	return gd->securemode;
}

int sunxi_probe_secure_monitor(void)
{
	return uboot_spare_head.boot_data.secureos_exist == SUNXI_SECURE_MODE_USE_SEC_MONITOR?1:0;
}

void sunxi_flush_allcaches(void)
{
	icache_disable();
	flush_dcache_all();
	dcache_disable();
}

void sunxi_board_close_source(void)
{
	//timer_exit should be call after sunxi_flash,because delay function
	sunxi_flash_exit(1);
	sunxi_sprite_exit(1);
	sunxi_dma_exit();
	timer_exit();
	sunxi_key_exit();
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

void sunxi_set_fel_flag(void)
{
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = SUNXI_RUN_EFEX_FLAG;
	asm volatile("DMB SY");
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



