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
#include <linux/ctype.h>
#include <linux/types.h>

#include <power/sunxi/power.h>
#include <power/sunxi/pmu.h>
#include <sys_config.h>
#include <power/sunxi/axp.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

sunxi_axp_dev_t  *sunxi_axp_dev[SUNXI_AXP_DEV_MAX] = {(void *)(-1)};
extern int platform_axp_probe(sunxi_axp_dev_t* sunxi_axp_dev[], int max_dev);
static int pmu_nodeoffset = 0;

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_probe(void)
{
	int axp_num = 0;
	memset(sunxi_axp_dev, 0, SUNXI_AXP_DEV_MAX * 4);

	pmu_nodeoffset = fdt_path_offset(working_fdt,PMU_SCRIPT_NAME);
	if(pmu_nodeoffset < 0)
	{
		printf("axp: get node[%s] error\n",PMU_SCRIPT_NAME);
		return 0;
	}
	axp_num = platform_axp_probe((sunxi_axp_dev_t* *)&sunxi_axp_dev,SUNXI_AXP_DEV_MAX);

	
	return axp_num;
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
int axp_reinit(void)
{
	int i;

	for(i=0;i<SUNXI_AXP_DEV_MAX;i++)
	{
		if((sunxi_axp_dev[i] != NULL) && (sunxi_axp_dev[i] != (void *)(-1)))
		{
			sunxi_axp_dev[i] = (sunxi_axp_dev_t *)((ulong)sunxi_axp_dev[i] + gd->reloc_off);
		}
	}
	pmu_nodeoffset = fdt_path_offset(working_fdt,PMU_SCRIPT_NAME);
	if(pmu_nodeoffset < 0)
	{
		printf("axp: get node[%s] error\n",PMU_SCRIPT_NAME);
		return 0;
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_get_power_vol_level(void)
{
	return gd->power_step_level;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_probe_startup_cause(void)
{
	return sunxi_axp_dev[0]->probe_this_poweron_cause();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：axp_probe_startup_check_factory_mode
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：用于样机恢复出厂设置后，第一次启动系统要求，要在USB或火牛存在情况下才能按按键启动，启动后清除标志
*
*
************************************************************************************************************
*/
int axp_probe_factory_mode(void)
{
	int buffer_value, status;
	int poweron_reason;

	buffer_value = sunxi_axp_dev[0]->probe_pre_sys_mode();

	if(buffer_value == PMU_PRE_FACTORY_MODE)	//factory mode: need the power key and dc or vbus
	{
		printf("factory mode detect\n");
		status = sunxi_axp_dev[0]->probe_power_status();
		if(status > 0)  //has the dc or vbus
		{
			//获取 开机原因，是按键触发，或者插入电压触发
			poweron_reason = sunxi_axp_dev[0]->probe_this_poweron_cause();
			if(poweron_reason == AXP_POWER_ON_BY_POWER_KEY)
			{
				//set the system next powerom status as 0x0e(the system mode)
				printf("factory mode release\n");
				sunxi_axp_dev[0]->set_next_sys_mode(PMU_PRE_SYS_MODE);
			}
			else
			{
				printf("factory mode: try to poweroff without power key\n");
				axp_set_hardware_poweron_vol();  //poweroff
				axp_set_power_off();
				for(;;);
			}
		}
		else
		{
			printf("factory mode: try to poweroff without power in\n");
			axp_set_hardware_poweroff_vol();  //poweroff
			axp_set_power_off();
			for(;;);
		}
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_hardware_poweron_vol(void) //设置开机之后，PMU硬件关机电压为2.9V
{
	int vol_value = 0;

	//if(script_parser_fetch(PMU_SCRIPT_NAME, "pmu_pwron_vol", &vol_value, 1))
	if(fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_pwron_vol", (uint32_t*)&vol_value)<0)
	{
		puts("set power on vol to default\n");
	}

	return sunxi_axp_dev[0]->set_power_onoff_vol(vol_value, 1);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_hardware_poweroff_vol(void) //设置关机之后，PMU硬件下次开机电压为3.3V
{
	int vol_value = 0;

	//if(script_parser_fetch(PMU_SCRIPT_NAME, "pmu_pwroff_vol", &vol_value, 1))
	if(fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_pwroff_vol", (uint32_t*)&vol_value)<0)
	{
		puts("set power off vol to default\n");
	}

	return sunxi_axp_dev[0]->set_power_onoff_vol(vol_value, 0);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_set_power_off(void)
{
	return sunxi_axp_dev[0]->set_power_off();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_next_poweron_status(int value)
{
	return sunxi_axp_dev[0]->set_next_sys_mode(value);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_probe_pre_sys_mode(void)
{
	return sunxi_axp_dev[0]->probe_pre_sys_mode();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_power_get_dcin_battery_exist(int *dcin_exist, int *battery_exist)
{
	*dcin_exist    = sunxi_axp_dev[0]->probe_power_status();
	*battery_exist = sunxi_axp_dev[0]->probe_battery_exist();

	return 0;
}

int  axp_probe_battery_exist(void)
{
	
	return sunxi_axp_dev[0]->probe_battery_exist();
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：						axp_probe_power_source
*
*    参数列表：void
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_probe_power_source(void)
{
	int status = 0;
	status = sunxi_axp_dev[0]->probe_power_status();
	if(status == AXP_VBUS_EXIST)
		return 1;
	else if(status == AXP_DCIN_EXIST)
		return 2;
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_probe_battery_vol(void)
{
	return sunxi_axp_dev[0]->probe_battery_vol();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_probe_rest_battery_capacity(void)
{
	return sunxi_axp_dev[0]->probe_battery_ratio();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_probe_key(void)
{
	return sunxi_axp_dev[0]->probe_key();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_probe_charge_current(void)
{
	return sunxi_axp_dev[0]->probe_charge_current();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_set_charge_current(int current)
{
	return sunxi_axp_dev[0]->set_charge_current(current);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  axp_set_charge_control(void)
{
	return sunxi_axp_dev[0]->set_charge_control();
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_vbus_limit_dc(void)
{
	sunxi_axp_dev[0]->set_vbus_vol_limit(gd->limit_vol);
	sunxi_axp_dev[0]->set_vbus_cur_limit(gd->limit_cur);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_vbus_limit_pc(void)
{
	sunxi_axp_dev[0]->set_vbus_vol_limit(gd->limit_pcvol);
	sunxi_axp_dev[0]->set_vbus_cur_limit(gd->limit_pccur);

	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_all_limit(void)
{
	int usbvol_limit = 0;
	int usbcur_limit = 0;
#if 0
	script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbvol_limit", &usbvol_limit, 1);
	script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbcur_limit", &usbcur_limit, 1);
	script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbvol", (int *)&gd->limit_vol, 1);
	script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbcur", (int *)&gd->limit_cur, 1);
	script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbvol_pc", (int *)&gd->limit_pcvol, 1);
	script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbcur_pc", (int *)&gd->limit_pccur, 1);
#endif
	fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_usbvol_limit", (uint32_t*)&usbvol_limit);
	fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_usbcur_limit", (uint32_t*)&usbcur_limit);
	fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_usbvol", (uint32_t*)&gd->limit_vol);
	fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_usbcur", (uint32_t*)&gd->limit_cur);
	fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_usbvol_pc", (uint32_t*)&gd->limit_pcvol);
	fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_usbcur_pc", (uint32_t*)&gd->limit_pccur);

	debug("usbvol_limit = %d, limit_vol = %ld\n", usbvol_limit, gd->limit_vol);
	debug("usbcur_limit = %d, limit_cur = %ld\n", usbcur_limit, gd->limit_cur);

	if(!usbvol_limit)
	{
		gd->limit_vol = 0;

	}
	if(!usbcur_limit)
	{
		gd->limit_cur = 0;
	}

	axp_set_vbus_limit_pc();

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_suspend_chgcur(void)
{
	return sunxi_axp_dev[0]->set_charge_current(gd->pmu_suspend_chgcur);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_runtime_chgcur(void)
{
	return sunxi_axp_dev[0]->set_charge_current(gd->pmu_runtime_chgcur);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_charge_vol_limit(void)
{
	int ret1;
	int ret2;

	//ret1 = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_runtime_chgcur", (int *)&gd->pmu_runtime_chgcur, 1);
	//ret2 = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_suspend_chgcur", (int *)&gd->pmu_suspend_chgcur, 1);
	ret1 = fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_runtime_chgcur", (uint32_t*)&gd->pmu_runtime_chgcur);
	ret2 = fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_suspend_chgcur", (uint32_t*)&gd->pmu_suspend_chgcur);

	if(ret1)
	{
		gd->pmu_runtime_chgcur = 600;
	}
	if(ret2)
	{
		gd->pmu_suspend_chgcur = 1500;
	}

	debug("pmu_runtime_chgcur=%ld\n", gd->pmu_runtime_chgcur);
	debug("pmu_suspend_chgcur=%ld\n", gd->pmu_suspend_chgcur);

	axp_set_suspend_chgcur();

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_set_power_supply_output(void)
{
	int   onoff;
	//uint power_supply_hd;
	//int power_index = 0, ret;
	char power_name[32];
	int  power_vol;
	int  power_vol_d;
	int nodeoffset ,offset;
	const struct fdt_property *prop;
	const char *pname;
	const int* pdata;

	nodeoffset = fdt_path_offset(working_fdt,FDT_PATH_POWER_SPLY);
	for (offset = fdt_first_property_offset(working_fdt, nodeoffset);
			offset > 0;
			offset = fdt_next_property_offset(working_fdt, offset))

	{
		prop = fdt_get_property_by_offset(working_fdt,offset,NULL);
		pname = fdt_string(working_fdt, fdt32_to_cpu(prop->nameoff));
		pdata=(const int*)prop->data;
		power_vol = fdt32_to_cpu(pdata[0]);

		memset(power_name, 0, sizeof(power_name));
		strcpy(power_name,pname);

		onoff = -1;
		power_vol_d = 0;

#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
		if(power_vol > 10000)
		{
			onoff = 1;
			power_vol_d = power_vol%10000;
		}
		else if(power_vol >= 0)
		{
			onoff = 0;
			power_vol_d = power_vol;
		}
#else
		if(power_vol > 0 )
		{
			onoff = 1;
			power_vol_d = power_vol;
		}
		else if(power_vol == 0)
		{
			onoff = 0;
		}
#endif

#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
		printf("%s = %d, onoff=%d\n", power_name, power_vol_d, onoff);
#else
		printf("%s = %d\n", power_name, power_vol_d);
#endif
		if(sunxi_axp_dev[0]->set_supply_status_byname(power_name, power_vol_d, onoff))
		{
			printf("axp set %s to %d failed\n", power_name, power_vol_d);
		}
	}

#if 0
	power_supply_hd = script_parser_fetch_subkey_start("power_sply");
	if(!power_supply_hd)
	{
		printf("unable to set power supply\n");
		return -1;
	}
	do
	{
		memset(power_name, 0, 16);
		ret = script_parser_fetch_subkey_next(power_supply_hd, power_name, &power_vol, &power_index);
		if(ret < 0)
		{
			printf("find power_sply to end\n");
			return 0;
		}

	onoff = -1;
	power_vol_d = 0;

#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
	if(power_vol > 10000)
	{
	    onoff = 1;
	    power_vol_d = power_vol%10000;
	}
	else if(power_vol >= 0)
	{
		onoff = 0;
		power_vol_d = power_vol;
	}
#else
	if(power_vol > 0 )
	{
	    onoff = 1;
	    power_vol_d = power_vol;
	}
	else if(power_vol == 0)
	{
		onoff = 0;
	}
#endif

#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
	printf("%s = %d, onoff=%d\n", power_name, power_vol_d, onoff);
#else
	printf("%s = %d\n", power_name, power_vol_d);
#endif
	if(sunxi_axp_dev[0]->set_supply_status_byname(power_name, power_vol_d, onoff))
	{
	    printf("axp set %s to %d failed\n", power_name, power_vol_d);
	}
	}
	while(1);
#endif
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_slave_set_power_supply_output(void)
{
	int  onoff;
	char power_name[32];
	int  power_vol;
	int  index = -1;
	int  i;
	int  power_vol_d;

	int nodeoffset ,offset;
	const struct fdt_property *prop;
	const char *pname;
	const int* pdata;

	nodeoffset = fdt_path_offset(working_fdt,"slave_power_sply");
	if(nodeoffset < 0)
	{
		printf("unable to set slave power supply\n");
		return -1;
	}

	for(i=1;i<SUNXI_AXP_DEV_MAX;i++)
	{
		if(sunxi_axp_dev[i] != NULL)
		{
		    if(strcmp(sunxi_axp_dev[0]->pmu_name, sunxi_axp_dev[i]->pmu_name))
		    {
		        index = i;

		        break;
		    }
		}
	}
	if(index == -1)
	{
		printf("unable to find slave pmu\n");
		return -1;
	}
	printf("slave power\n");

	for (offset = fdt_first_property_offset(working_fdt, nodeoffset);
			offset > 0;
			offset = fdt_next_property_offset(working_fdt, offset))

	{
		prop = fdt_get_property_by_offset(working_fdt,offset,NULL);
		pname = fdt_string(working_fdt, fdt32_to_cpu(prop->nameoff));
		pdata=(const int*)prop->data;
		power_vol = fdt32_to_cpu(pdata[0]);

		memset(power_name, 0, sizeof(power_name));
		strcpy(power_name,pname);

		onoff = -1;
		power_vol_d = 0;
		if(power_vol > 10000)
		{
		    onoff = 1;
		    power_vol_d = power_vol%10000;
		}
#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
		else if(power_vol > 0)
		{
			onoff = 0;
			power_vol_d = power_vol;
		}
#endif
		else if(power_vol == 0)
		{
			onoff = 0;
		}
#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
		printf("%s = %d, onoff=%d\n", power_name, power_vol_d, onoff);
#else
		printf("%s = %d\n", power_name, power_vol_d);
#endif
		if(sunxi_axp_dev[index]->set_supply_status_byname(power_name, power_vol_d, onoff))
		{
		    printf("axp set %s to %d failed\n", power_name, power_vol_d);
		}
	}

    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_probe_power_supply_condition(void)
{
	int   dcin_exist, bat_vol;
	int   ratio;
	int   safe_vol;

	//
	dcin_exist = sunxi_axp_dev[0]->probe_power_status();
	bat_vol = sunxi_axp_dev[0]->probe_battery_vol();

	safe_vol = 0;
	fdt_getprop_u32(working_fdt,pmu_nodeoffset,"pmu_safe_vol", (uint32_t*)&safe_vol);
	if(safe_vol < 3000)
	{
		safe_vol = 3500;
	}

	ratio = sunxi_axp_dev[0]->probe_battery_ratio();
	if(ratio < 1)
	{
		sunxi_axp_dev[0]->set_coulombmeter_onoff(0);
		sunxi_axp_dev[0]->set_coulombmeter_onoff(1);
	}
	printf("bat_vol=%d, ratio=%d\n", bat_vol, ratio);
	if(ratio < 1)
	{
		if(dcin_exist)
		{
			if(bat_vol < safe_vol)
			{
				gd->power_step_level = BATTERY_RATIO_TOO_LOW_WITH_DCIN_VOL_TOO_LOW;
			}
			else
			{
				gd->power_step_level = BATTERY_RATIO_TOO_LOW_WITH_DCIN;
			}
		}
		else
		{
			gd->power_step_level = BATTERY_RATIO_TOO_LOW_WITHOUT_DCIN;
		}
	}
	else
	{
		gd->power_step_level = BATTERY_RATIO_ENOUGH;
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static __u8 power_int_value[8];

int axp_int_enable(__u8 *value)
{
	sunxi_axp_dev[0]->probe_int_enable(power_int_value);
	sunxi_axp_dev[0]->set_int_enable(value);
	//打开小cpu的中断使能
	//*(volatile unsigned int *)(0x01f00c00 + 0x10) |= 1;
	//*(volatile unsigned int *)(0x01f00c00 + 0x40) |= 1;

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_int_disable(void)
{
	//*(volatile unsigned int *)(0x01f00c00 + 0x10) |= 1;
	//*(volatile unsigned int *)(0x01f00c00 + 0x40) &= ~1;
	return sunxi_axp_dev[0]->set_int_enable(power_int_value);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp_int_query(__u8 *addr)
{
	int ret;

	ret = sunxi_axp_dev[0]->probe_int_pending(addr);
	//*(volatile unsigned int *)(0x01f00c00 + 0x10) |= 1;

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
*    note          :  如果pmu_type = 0, 表示操作主PMU
*                     如果合法的非零值，表示操作指定pmu
*                     如果非法值，不执行任何操作
*
************************************************************************************************************
*/
int axp_set_supply_status(int pmu_type, int vol_name, int vol_value, int onoff)
{
	//调用不同的函数指针
	return sunxi_axp_dev[pmu_type]->set_supply_status(vol_name, vol_value, onoff);
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
int axp_set_supply_status_byname(char *pmu_type, char *vol_type, int vol_value, int onoff)
{
	//调用不同的函数指针
	int   i;

	for(i=1;i<SUNXI_AXP_DEV_MAX;i++)
	{
		if(sunxi_axp_dev[i] != NULL)
		{
			if(!strcmp(sunxi_axp_dev[i]->pmu_name, pmu_type))
			{
				return sunxi_axp_dev[i]->set_supply_status_byname(vol_type, vol_value, onoff);
			}
		}
	}

	return -1;
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
int axp_probe_supply_status(int pmu_type, int vol_name, int vol_value)
{
	//调用不同的函数指针
	return sunxi_axp_dev[pmu_type]->probe_supply_status(vol_name, 0, 0);
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
int axp_probe_supply_status_byname(char *pmu_type, char *vol_type)
{
	//调用不同的函数指针
	int   i;

	for(i=1;i<SUNXI_AXP_DEV_MAX;i++)
	{
		if(sunxi_axp_dev[i] != NULL)
		{
			if(!strcmp(sunxi_axp_dev[i]->pmu_name, pmu_type))
			{
				return sunxi_axp_dev[i]->probe_supply_status_byname(vol_type);
			}
		}
	}

	return -1;
}

//example string  :    "axp81x_dcdc6 vdd-sys vdd-usb0-09 vdd-hdmi-09"
static int find_regulator_str(const char* src, const char* des)
{
    int i,len_src, len_des ;
    int token_index;
    len_src = strlen(src);
    len_des = strlen(des);

    if(len_des > len_src) return 0;

    token_index = 0;
    for(i =0 ; i < len_src+1; i++)
    {
        if(src[i]==' ' || src[i] == '\t' || src[i] == '\0')
        {
            if(i-token_index == len_des)
            {
                if(memcmp(src+token_index,des,len_des) == 0)
                {
                    return 1;
                }
            }
            token_index = i+1;
        }
    }
    return 0;
}

int axp_set_supply_status_byregulator(const char* id, int onoff)
{
	//char main_key[32];
	char pmu_type[32];
	char vol_type[32];
	char sub_key_name[32];
	//char sub_key_value[256];
	char *sub_key_value = NULL;
	//int main_hd = 0;
	unsigned int i = 0, j = 0, index = 0;
	int ldo_count = 0;
	int find_flag = 0;
	int ret = 0;
	//use static variable for save time,because fdt_path_offset will take a long time
	static int nodeoffset = -1;

	if(nodeoffset < 0 )
	{
		nodeoffset = fdt_path_offset(working_fdt,FDT_PATH_REGU);
	}
	
	if(nodeoffset < 0)
	{
		printf("axp: get node[%s] error\n",FDT_PATH_REGU);
		return -1;
	}

	do
	{
		if(fdt_getprop_u32(working_fdt,nodeoffset,"regulator_count", (uint32_t*)&ldo_count)<0)
		{
			break;
		}
	        for(index = 1; index <= ldo_count; index++)
	        {
			sprintf(sub_key_name, "regulator%d", index);
			sub_key_value = NULL;
			if(fdt_getprop_string(working_fdt,nodeoffset,sub_key_name, &sub_key_value)<0)
			{
				break;
			}
			if(find_regulator_str(sub_key_value, id))
			{
				find_flag = 1;
				break;
			}

	        }
	        if(find_flag)
	            break;
	} while(0);

    if(!find_flag)
    {
        printf("unable to find regulator %s from [pmu1_regu] or [pmu2_regu] \n",id);
        return -1;
    }

    //example :    ldo6      = "axp81x_dcdc6 vdd-sys vdd-usb0-09 vdd-hdmi-09"
    memset(pmu_type, 0, sizeof(pmu_type));
    memset(vol_type, 0, sizeof(vol_type));
    //get pmu type
    for(j = 0,i =0; i < strlen(sub_key_value); i++)
    {
        if(sub_key_value[i] == '_')
        {
            i++;
            break;
        }
        pmu_type[j++] = sub_key_value[i];
    }
    //get vol type
    j = 0;
    for(; i < strlen(sub_key_value); i++)
    {
        if(sub_key_value[i] == ' ')
        {
            break;
        }
        vol_type[j++]= sub_key_value[i];
    }

    //para vol = 0  indcate not set vol,only open or close  voltage switch
    ret = axp_set_supply_status_byname(pmu_type,vol_type,0, onoff);
    if(ret != 0)
    {
         printf("error: supply regelator %s=id, pmu_type=%s_%s onoff=%d fail\n", id,pmu_type,vol_type,onoff );
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
static int current_pmu = -1;
int axp_probe_supply_pmu_name(char *axpname)
{
	int i = 0;
	if(axpname == NULL)
	{
		return -1;
	}
	for(i=1;i<SUNXI_AXP_DEV_MAX;i++)
	{
		if((sunxi_axp_dev[i] != NULL) && (current_pmu < i))
		{
			current_pmu = i;
			strcpy(axpname, sunxi_axp_dev[i]->pmu_name);
			return 0;
		}
	}
	return -1;

}

int axp_probe_vbus_cur_limit(void)
{
    return sunxi_axp_dev[0]->probe_vbus_cur_limit();
}

int axp_set_coulombmeter_onoff(int onoff )
{
	return sunxi_axp_dev[0]->set_coulombmeter_onoff(onoff);
}


