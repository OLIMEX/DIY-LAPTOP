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
#include <power/sunxi/axp81X_reg.h>
#include <power/sunxi/axp.h>
#include <power/sunxi/pmu.h>

extern int axp81_set_supply_status(int vol_name, int vol_value, int onoff);
extern int axp81_set_supply_status_byname(char *vol_name, int vol_value, int onoff);
extern int axp81_probe_supply_status(int vol_name, int vol_value, int onoff);
extern int axp81_probe_supply_status_byname(char *vol_name);
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
int axp81_probe(void)
{
	u8    pmu_type;

    axp_i2c_config(SUNXI_AXP_81X, AXP81X_ADDR);
	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_VERSION, &pmu_type))
	{
		printf("axp read error\n");

		return -1;
	}

    pmu_type &= 0xCF;
	if(pmu_type == 0x41)
	{
		/* pmu type AXP81X */
		tick_printf("PMU: AXP81X\n");

		return 0;
	}

	return -1;
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
int axp81_set_coulombmeter_onoff(int onoff)
{
	u8 reg_value;

	if(axp_i2c_read(AXP81X_ADDR,BOOT_POWER81X_FUEL_GAUGE_CTL, &reg_value))
    {
        return -1;
    }
    if(!onoff)
    	reg_value &= ~(0x01 << 7);
    else
    	reg_value |= (0x01 << 7);

    if(axp_i2c_write(AXP81X_ADDR,BOOT_POWER81X_FUEL_GAUGE_CTL,reg_value))
    {
        return -1;
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
int axp81_set_charge_control(void)
{
	u8 reg_value;
	//disable ts adc, enable all other adc
	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_ADC_EN, &reg_value))
    {
        return -1;
    }
    reg_value |= 0xC0;
    if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_ADC_EN, reg_value))
    {
        return -1;
    }
    //enable charge
	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, &reg_value))
    {
        return -1;
    }
    reg_value |= 0x80;
    if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, reg_value))
    {
        return -1;
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
int axp81_probe_battery_ratio(void)
{
	u8 reg_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_BAT_PERCEN_CAL, &reg_value))
    {
        return -1;
    }

	return reg_value & 0x7f;
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
int axp81_probe_power_status(void)
{
	u8 reg_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_STATUS, &reg_value))
    {
        return -1;
    }
	if(reg_value & 0x10)		//vbus exist
	{
		return AXP_VBUS_EXIST;
	}
	if(reg_value & 0x40)		//dc in exist
	{
		return AXP_DCIN_EXIST;
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
int axp81_probe_battery_exist(void)
{
	u8 reg_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_MODE_CHGSTATUS, &reg_value))
    {
        return -1;
    }

	if(reg_value & 0x10)
	{
		return (reg_value & 0x20);
	}

	return -1;
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
int axp81_probe_battery_vol(void)
{
	u8  reg_value_h, reg_value_l;
	int bat_vol, tmp_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_BAT_AVERVOL_H8, &reg_value_h))
    {
        return -1;
    }
    if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_BAT_AVERVOL_L4, &reg_value_l))
    {
        return -1;
    }
    tmp_value = (reg_value_h << 4) | reg_value_l;
    bat_vol = tmp_value * 11;
    bat_vol /= 10;

	return bat_vol;
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
int axp81_probe_key(void)
{
	u8  reg_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_INTSTS5, &reg_value))
    {
        return -1;
    }
    reg_value &= (0x03<<3);
	if(reg_value)
	{
		if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_INTSTS5, reg_value))
	    {
	        return -1;
	    }
	}

	return (reg_value>>3)&3;
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
int axp81_probe_pre_sys_mode(void)
{
	u8  reg_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_DATA_BUFFER11, &reg_value))
    {
        return -1;
    }

	return reg_value;
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
int axp81_set_next_sys_mode(int data)
{
	if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_DATA_BUFFER11, (u8)data))
    {
        return -1;
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
int axp81_probe_this_poweron_cause(void)
{
    uchar   reg_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_STATUS, &reg_value))
    {
        return -1;
    }

    return reg_value & 0x01;
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
int axp81_set_power_off(void)
{
    u8 reg_value;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_OFF_CTL, &reg_value))
    {
        return -1;
    }
    reg_value |= 1 << 7;
	if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_OFF_CTL, reg_value))
    {
        return -1;
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
int axp81_set_power_onoff_vol(int set_vol, int stage)
{
	u8 reg_value;

	if(!set_vol)
	{
		if(!stage)
		{
			set_vol = 3300;
		}
		else
		{
			set_vol = 2900;
		}
	}
	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_VOFF_SET, &reg_value))
    {
        return -1;
    }
	reg_value &= 0xf8;
	if(set_vol >= 2600 && set_vol <= 3300)
	{
		reg_value |= (set_vol - 2600)/100;
	}
	else if(set_vol <= 2600)
	{
		reg_value |= 0x00;
	}
	else
	{
		reg_value |= 0x07;
	}
	if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_VOFF_SET, reg_value))
    {
        return -1;
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
int axp81_set_charge_current(int current)
{
	u8   reg_value;
	int  step;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, &reg_value))
    {
        return -1;
    }
	reg_value &= ~0x0f;
	if(current > 2800)
	{
		current = 2800;
	}
	else if(current < 200)
	{
		current = 200;
	}
	step       = (current/200) - 1; //0-13 ---->200to 2800
	reg_value |= (step & 0x0f);

	if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, reg_value))
    {
        return -1;
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
int axp81_probe_charge_current(void)
{
	uchar  reg_value;
	int	  current;

	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, &reg_value))
    {
        return -1;
    }
	reg_value &= 0x0f;
	current = (reg_value + 1) * 200;

	return current;
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
int axp81_set_vbus_cur_limit(int current)
{
	uchar reg_value;

	//set bus current limit off
	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, &reg_value))
    {
        return -1;
    }
    reg_value &= 0xfC;
	if(!current)
	{
	    reg_value |= 0x03;
	}
	else if(current >= 2500) //limit to2500
	{
		reg_value |= 0x03;
	}
	else if(current >= 2000) //limit to 2000
	{
		reg_value |= 0x02;
	}
    else if(current >= 1500) //limit to 1500
	{
		reg_value |= 0x01;
	}
    else 				     //limit to 900
	{
		reg_value |= 0x00;
	}

	if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, reg_value))
    {
        return -1;
    }

    return 0;
}

int axp81_probe_vbus_cur_limit(void)
{
    uchar reg_value;

    if(axp_i2c_read(AXP81X_ADDR,BOOT_POWER81X_VBUS_SET,&reg_value))
    {
        return -1;
    }
    reg_value &= 0x03;
    if(reg_value == 0x01)
    {
        printf("limit to 1500mA \n");
        return 1500;
    }
    else if(reg_value == 0x00)
    {
        printf("limit to 900 \n");
        return 900;
    }
	else if(reg_value == 0x02)
	{
		printf("limit to 2000mA \n");
		return 2000;
	}
	else
	{
		printf("limit to 2500mA \n");
		return 2500;
	}

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
int axp81_set_vbus_vol_limit(int vol)
{
	uchar reg_value;

	//set bus vol limit off
	if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, &reg_value))
    {
        return -1;
    }
    reg_value &= ~(7 << 3);
	if(!vol)
	{
	    reg_value &= ~(1 << 6);
	}
	else
	{
		if(vol < 4000)
		{
			vol = 4000;
		}
		else if(vol > 4700)
		{
			vol = 4700;
		}
		reg_value |= ((vol-4000)/100) << 3;
	}
	if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, reg_value))
    {
        return -1;
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
int axp81_probe_int_pending(uchar *addr)
{
	int   i;

	for(i=0;i<5;i++)
	{
	    if(axp_i2c_read(AXP81X_ADDR, BOOT_POWER81X_INTSTS1 + i, addr + i))
	    {
	        return -1;
	    }
	}

	for(i=0;i<5;i++)
	{
	    if(axp_i2c_write(AXP81X_ADDR, BOOT_POWER81X_INTSTS1 + i, 0xff))
	    {
	        return -1;
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
int axp81_probe_int_enable(uchar *addr)
{
	int   i;
	uchar  int_reg = BOOT_POWER81X_INTEN1;

	for(i=0;i<5;i++)
	{
		if(axp_i2c_read(AXP81X_ADDR, int_reg, addr + i))
	    {
	        return -1;
	    }
	    int_reg ++;
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
int axp81_set_int_enable(uchar *addr)
{
	int   i;
	uchar  int_reg = BOOT_POWER81X_INTEN1;

	for(i=0;i<5;i++)
	{
		if(axp_i2c_write(AXP81X_ADDR, int_reg, addr[i]))
	    {
	        return -1;
	    }
	    int_reg ++;
	}

	return 0;
}


sunxi_axp_module_init("axp81x", SUNXI_AXP_81X);


