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
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <sys_config.h>
#include <smc.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;


//#define GPIO_REG_READ(reg)              smc_readl((reg))
//#define GPIO_REG_WRITE(reg, value)      smc_writel((value), (reg))

#define GPIO_REG_READ(reg)              (smc_readl((ulong)(reg)))
#define GPIO_REG_WRITE(reg, value)      (smc_writel((value), (ulong)(reg)))


/**#############################################################################################################
 *
 *                           GPIO(PIN) Operations
 *
-##############################################################################################################*/
#define _PIO_REG_CFG(n, i)               ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00))
#define _PIO_REG_DLEVEL(n, i)            ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14))
#define _PIO_REG_PULL(n, i)              ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C))
#define _PIO_REG_DATA(n)                 ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10))

#define _PIO_REG_CFG_VALUE(n, i)          readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00)
#define _PIO_REG_DLEVEL_VALUE(n, i)       readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14)
#define _PIO_REG_PULL_VALUE(n, i)         readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C)
#define _PIO_REG_DATA_VALUE(n)            readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10)
#define _PIO_REG_BASE(n)                    ((volatile unsigned int *)(SUNXI_PIO_BASE +((n)-1)*24))

#ifdef SUNXI_RPIO_BASE
#define _R_PIO_REG_CFG(n, i)               ((volatile unsigned int *)( SUNXI_RPIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00))
#define _R_PIO_REG_DLEVEL(n, i)            ((volatile unsigned int *)( SUNXI_RPIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14))
#define _R_PIO_REG_PULL(n, i)              ((volatile unsigned int *)( SUNXI_RPIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C))
#define _R_PIO_REG_DATA(n)                 ((volatile unsigned int *)( SUNXI_RPIO_BASE + ((n)-12)*0x24 + 0x10))

#define _R_PIO_REG_CFG_VALUE(n, i)          smc_readl( SUNXI_RPIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00)
#define _R_PIO_REG_DLEVEL_VALUE(n, i)       smc_readl( SUNXI_RPIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14)
#define _R_PIO_REG_PULL_VALUE(n, i)         smc_readl( SUNXI_RPIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C)
#define _R_PIO_REG_DATA_VALUE(n)            smc_readl( SUNXI_RPIO_BASE + ((n)-12)*0x24 + 0x10)
#define _R_PIO_REG_BASE(n)                    ((volatile unsigned int *)(SUNXI_RPIO_BASE +((n)-12)*24))


volatile void* PIO_REG_CFG(int port, int port_num)
{
	if(port < 12)
	{
		return _PIO_REG_CFG(port, port_num);
	}
	else
	{
		return _R_PIO_REG_CFG(port, port_num);
	}
}

volatile void* PIO_REG_PULL(int port, int port_num)
{
	if(port < 12)
	{
		return _PIO_REG_PULL(port, port_num);  
	}
	else
	{
		return _R_PIO_REG_PULL(port, port_num);  
	}
}

volatile void* PIO_REG_DLEVEL(int port,int port_num)
{
	if(port < 12)
	{
		return _PIO_REG_DLEVEL(port, port_num);
	}
	else
	{
		return _R_PIO_REG_DLEVEL(port, port_num);
	}
}

volatile void* PIO_REG_DATA(int port)
{
	if(port < 12)
	{
		return _PIO_REG_DATA(port);
	}
	else
	{
		return _R_PIO_REG_DATA(port);
	}
}

u32 PIO_REG_CFG_VALUE(uint port, uint i)  
{
	u32 value = 0;
	if( port < 12) 
	{
		value =  _PIO_REG_CFG_VALUE(port,i);
	}
	else
	{
		value = _R_PIO_REG_CFG_VALUE(port,i);
	}
	return value;
}

u32 PIO_REG_DLEVEL_VALUE(uint port, uint i)  
{
	u32 value = 0;
	if( port < 12) 
	{
		value =  _R_PIO_REG_DLEVEL_VALUE(port,i);
	}
	else
	{
		value = _R_PIO_REG_DLEVEL_VALUE(port,i);
	}
	return value;
}

u32 PIO_REG_PULL_VALUE(uint port, uint i)  
{
	u32 value = 0;
	if( port < 12) 
	{
		value =  _PIO_REG_PULL_VALUE(port,i);
	}
	else
	{
		value = _R_PIO_REG_PULL_VALUE(port,i);
	}
	return value;
}

u32 PIO_REG_DATA_VALUE(uint port)  
{
	u32 value = 0;
	if( port < 12) 
	{
		value =  _PIO_REG_DATA_VALUE(port);
	}
	else
	{
		value = _R_PIO_REG_DATA_VALUE(port);
	}
	return value;
}
#else
volatile void* PIO_REG_CFG(int port, int port_num)
{
	return _PIO_REG_CFG(port, port_num);
}

volatile void* PIO_REG_PULL(int port, int port_num)
{
	return _PIO_REG_PULL(port, port_num);  
}

volatile void* PIO_REG_DLEVEL(int port,int port_num)
{
	
	return _PIO_REG_DLEVEL(port, port_num);
	
}

volatile void* PIO_REG_DATA(int port)
{
	return _PIO_REG_DATA(port);
}

u32 PIO_REG_CFG_VALUE(uint port, uint i)  
{
	return _PIO_REG_CFG_VALUE(port,i);
}

u32 PIO_REG_DLEVEL_VALUE(uint port, uint i)  
{
	return _PIO_REG_DLEVEL_VALUE(port,i);
}

u32 PIO_REG_PULL_VALUE(uint port, uint i)  
{
	return _PIO_REG_PULL_VALUE(port,i);
}

u32 PIO_REG_DATA_VALUE(uint port)  
{
        return _PIO_REG_DATA_VALUE(port);
}
#endif




#define PIO_REG_BASE(n)                    ((volatile unsigned int *)(SUNXI_PIO_BASE +((n)-1)*24))

__s32  gpio_get_all_pin_status(ulong p_handler, user_gpio_set_t *gpio_status, __u32 gpio_count_max, __u32 if_get_from_hardware)
{
    char               *tmp_buf;
    __u32               group_count_max, first_port;      
    system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
    user_gpio_set_t  *script_gpio;
    __u32               port_num_func, port_num_pull;
    volatile __u32     *tmp_group_func_addr = NULL, *tmp_group_pull_addr;
    volatile __u32     *tmp_group_data_addr, *tmp_group_dlevel_addr;
    __u32               port, port_num;
    __u32               pre_port = 0x7fffffff, pre_port_num_func = 0x7fffffff, pre_port_num_pull = 0x7fffffff;
    __u32               i;

    if((!p_handler) || (!gpio_status))
    {
        return EGPIO_FAIL;
    }
    if(gpio_count_max <= 0)
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    if(group_count_max <= 0)
    {
        return EGPIO_FAIL;
    }
    user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
    if(group_count_max > gpio_count_max)
    {
        group_count_max = gpio_count_max;
    }
 
    if(!if_get_from_hardware)
    {
        for(i = 0; i < group_count_max; i++)
        {
            tmp_sys_gpio_data = user_gpio_set + i;
            script_gpio       = gpio_status + i; 

            script_gpio->port      = tmp_sys_gpio_data->port;
            script_gpio->port_num  = tmp_sys_gpio_data->port_num;     
            script_gpio->pull      = tmp_sys_gpio_data->user_gpio_status.pull;
            script_gpio->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;
            script_gpio->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level;
            script_gpio->data      = tmp_sys_gpio_data->user_gpio_status.data;
            strcpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);
        }
    }
    else
    {
        for(first_port = 0; first_port < group_count_max; first_port++)
        {
            tmp_sys_gpio_data  = user_gpio_set + first_port;
            port     = tmp_sys_gpio_data->port;
            port_num = tmp_sys_gpio_data->port_num;
            if(!port)
            {
                continue;
            }
            port_num_func = (port_num >> 3);
            port_num_pull = (port_num >> 4);
            tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);
            tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);
            tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);
            tmp_group_data_addr    = PIO_REG_DATA(port);
            break;
        }
        if(first_port >= group_count_max)
        {
            return 0;
        }
        for(i = first_port; i < group_count_max; i++)
        {
            tmp_sys_gpio_data = user_gpio_set + i; 
            script_gpio       = gpio_status + i;  

            port     = tmp_sys_gpio_data->port;
            port_num = tmp_sys_gpio_data->port_num;

            script_gpio->port = port; 
            script_gpio->port_num  = port_num;
            strcpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);

            port_num_func = (port_num >> 3);
            port_num_pull = (port_num >> 4);

            if((port_num_pull != pre_port_num_pull) || (port != pre_port))
            {
                tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);
                tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);
                tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);
                tmp_group_data_addr    = PIO_REG_DATA(port);               
            }
            else if(pre_port_num_func != port_num_func)   
            {
                tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);
            }

            pre_port_num_pull = port_num_pull;
            pre_port_num_func = port_num_func;
            pre_port          = port;
          
            script_gpio->pull      = (GPIO_REG_READ(tmp_group_pull_addr)   >> ((port_num - (port_num_pull<<4))<<1)) & 0x03; 
            script_gpio->drv_level = (GPIO_REG_READ(tmp_group_dlevel_addr) >> ((port_num - (port_num_pull<<4))<<1)) & 0x03;
            script_gpio->mul_sel   = (GPIO_REG_READ(tmp_group_func_addr)   >> ((port_num - (port_num_func<<3))<<2)) & 0x07; 
            if(script_gpio->mul_sel <= 1)
            {
                script_gpio->data  = (GPIO_REG_READ(tmp_group_data_addr)   >>   port_num) & 0x01;  
            }
            else
            {
                script_gpio->data = -1;
            }
        }
    }

    return EGPIO_SUCCESS;
}



__s32  gpio_get_one_pin_status(ulong p_handler, user_gpio_set_t *gpio_status, const char *gpio_name, __u32 if_get_from_hardware)
{
    char               *tmp_buf;                                     
    __u32               group_count_max;                               
    system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
    __u32               port_num_func, port_num_pull;
    __u32               port, port_num;
    __u32               i, tmp_val1, tmp_val2;

   
    if((!p_handler) || (!gpio_status))
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    if(group_count_max <= 0)
    {
        return EGPIO_FAIL;
    }
    else if((group_count_max > 1) && (!gpio_name))
    {
        return EGPIO_FAIL;
    }
    user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);

    for(i = 0; i < group_count_max; i++)
    {
        tmp_sys_gpio_data = user_gpio_set + i; 
        if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
        {
            continue;
        }
        strcpy(gpio_status->gpio_name, tmp_sys_gpio_data->gpio_name);
        port                   = tmp_sys_gpio_data->port;
        port_num               = tmp_sys_gpio_data->port_num;
        gpio_status->port      = port;                                         
        gpio_status->port_num  = port_num;                                     

        if(!if_get_from_hardware)                                                
        {
            gpio_status->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;  
            gpio_status->pull      = tmp_sys_gpio_data->user_gpio_status.pull;   
            gpio_status->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level;
            gpio_status->data      = tmp_sys_gpio_data->user_gpio_status.data;  
        }
        else                                                                  
        {
			port_num_func = (port_num >> 3);
			port_num_pull = (port_num >> 4);

			tmp_val1 = ((port_num - (port_num_func << 3)) << 2);
			tmp_val2 = ((port_num - (port_num_pull << 4)) << 1);
			gpio_status->mul_sel   = (PIO_REG_CFG_VALUE(port, port_num_func)>>tmp_val1) & 0x07;      
			gpio_status->pull      = (PIO_REG_PULL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;    
			gpio_status->drv_level = (PIO_REG_DLEVEL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;    
			if(gpio_status->mul_sel <= 1)
			{
				gpio_status->data = (PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;  
			}
			else
			{
				gpio_status->data = -1;
			}
        }

        break;
    }

    return EGPIO_SUCCESS;
}

__s32  gpio_set_one_pin_status(ulong p_handler, user_gpio_set_t *gpio_status, const char *gpio_name, __u32 if_set_to_current_input_status)
{
    char               *tmp_buf;                                   
    __u32               group_count_max;                            
    system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
    user_gpio_set_t     script_gpio;
    volatile __u32     *tmp_addr;
    __u32               port_num_func, port_num_pull;
    __u32               port, port_num;
    __u32               i, reg_val, tmp_val;

  
    if((!p_handler) || (!gpio_name))
    {
        return EGPIO_FAIL;
    }
    if((if_set_to_current_input_status) && (!gpio_status))
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    if(group_count_max <= 0)
    {
        return EGPIO_FAIL;
    }
    user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
 
    for(i = 0; i < group_count_max; i++)
    {
        tmp_sys_gpio_data = user_gpio_set + i;  
        if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
        {
            continue;
        }

        port          = tmp_sys_gpio_data->port;            
        port_num      = tmp_sys_gpio_data->port_num;      
        port_num_func = (port_num >> 3);
        port_num_pull = (port_num >> 4);

        if(if_set_to_current_input_status)                                
        {
            
            script_gpio.mul_sel   = gpio_status->mul_sel;
            script_gpio.pull      = gpio_status->pull;
            script_gpio.drv_level = gpio_status->drv_level;
            script_gpio.data      = gpio_status->data;
        }
        else
        {
            script_gpio.mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;
            script_gpio.pull      = tmp_sys_gpio_data->user_gpio_status.pull;
            script_gpio.drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level;
            script_gpio.data      = tmp_sys_gpio_data->user_gpio_status.data;
        }

        if(script_gpio.mul_sel >= 0)
        {
            tmp_addr = PIO_REG_CFG(port, port_num_func);
            reg_val = GPIO_REG_READ(tmp_addr);                                                      
            tmp_val = (port_num - (port_num_func<<3))<<2;
            reg_val &= ~(0x07 << tmp_val);
            reg_val |=  (script_gpio.mul_sel) << tmp_val;
            GPIO_REG_WRITE(tmp_addr, reg_val);
        }
       
        if(script_gpio.pull >= 0)
        {
            tmp_addr = PIO_REG_PULL(port, port_num_pull);
            reg_val = GPIO_REG_READ(tmp_addr);                                                 
            tmp_val = (port_num - (port_num_pull<<4))<<1;
            reg_val &= ~(0x03 << tmp_val);
            reg_val |=  (script_gpio.pull) << tmp_val;
            GPIO_REG_WRITE(tmp_addr, reg_val);
        }
      
        if(script_gpio.drv_level >= 0)
        {
            tmp_addr = PIO_REG_DLEVEL(port, port_num_pull);
            reg_val = GPIO_REG_READ(tmp_addr);                                                
            tmp_val = (port_num - (port_num_pull<<4))<<1;
            reg_val &= ~(0x03 << tmp_val);
            reg_val |=  (script_gpio.drv_level) << tmp_val;
            GPIO_REG_WRITE(tmp_addr, reg_val);
        }

        if(script_gpio.mul_sel == 1)
        {
            if(script_gpio.data >= 0)
            {
                tmp_addr = PIO_REG_DATA(port);
                reg_val = GPIO_REG_READ(tmp_addr);                                                 
                reg_val &= ~(0x01 << port_num);
                reg_val |=  (script_gpio.data & 0x01) << port_num;
                GPIO_REG_WRITE(tmp_addr, reg_val);
            }
        }

        break;
    }

    return EGPIO_SUCCESS;
}

__s32  gpio_set_one_pin_io_status(ulong p_handler, __u32 if_set_to_output_status, const char *gpio_name)
{
    char               *tmp_buf;                                       
    __u32               group_count_max;                             
    system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
    volatile __u32      *tmp_group_func_addr = NULL;
    __u32               port, port_num, port_num_func;
    __u32                i, reg_val;

   
    if(!p_handler)
    {
        return EGPIO_FAIL;
    }
    if(if_set_to_output_status > 1)
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);
    if(group_count_max == 0)
    {
        return EGPIO_FAIL;
    }
    else if(group_count_max == 1)
    {
        user_gpio_set = tmp_sys_gpio_data;
    }
    else if(gpio_name)
    {
        for(i=0; i<group_count_max; i++)
        {
            if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
            {
                tmp_sys_gpio_data ++;
                continue;
            }
            user_gpio_set = tmp_sys_gpio_data;
            break;
        }
    }
    if(!user_gpio_set)
    {
        return EGPIO_FAIL;
    }

    port     = user_gpio_set->port;
    port_num = user_gpio_set->port_num;
    port_num_func = port_num >> 3;

    tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
    reg_val = GPIO_REG_READ(tmp_group_func_addr);
    reg_val &= ~(0x07 << (((port_num - (port_num_func<<3))<<2)));
    reg_val |=   if_set_to_output_status << (((port_num - (port_num_func<<3))<<2));
    GPIO_REG_WRITE(tmp_group_func_addr, reg_val);

    return EGPIO_SUCCESS;
}

__s32  gpio_set_one_pin_pull(ulong p_handler, __u32 set_pull_status, const char *gpio_name)
{
    char               *tmp_buf;                                      
    __u32               group_count_max;                             
    system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
    volatile __u32      *tmp_group_pull_addr = NULL;
    __u32               port, port_num, port_num_pull;
    __u32                i, reg_val;
    
    if(!p_handler)
    {
        return EGPIO_FAIL;
    }
    if(set_pull_status >= 4)
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);
    if(group_count_max == 0)
    {
        return EGPIO_FAIL;
    }
    else if(group_count_max == 1)
    {
        user_gpio_set = tmp_sys_gpio_data;
    }
    else if(gpio_name)
    {
        for(i=0; i<group_count_max; i++)
        {
            if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
            {
                tmp_sys_gpio_data ++;
                continue;
            }
            user_gpio_set = tmp_sys_gpio_data;
            break;
        }
    }
    if(!user_gpio_set)
    {
        return EGPIO_FAIL;
    }

    port     = user_gpio_set->port;
    port_num = user_gpio_set->port_num;
    port_num_pull = port_num >> 4;

    tmp_group_pull_addr = PIO_REG_PULL(port, port_num_pull);
    reg_val = GPIO_REG_READ(tmp_group_pull_addr);
    reg_val &= ~(0x03 << (((port_num - (port_num_pull<<4))<<1)));
    reg_val |=  (set_pull_status << (((port_num - (port_num_pull<<4))<<1)));
    GPIO_REG_WRITE(tmp_group_pull_addr, reg_val);

    return EGPIO_SUCCESS;
}

__s32  gpio_set_one_pin_driver_level(ulong p_handler, __u32 set_driver_level, const char *gpio_name)
{
    char               *tmp_buf;                                      
    __u32               group_count_max;                            
    system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
    volatile __u32      *tmp_group_dlevel_addr = NULL;
    __u32               port, port_num, port_num_dlevel;
    __u32                i, reg_val;
   
    if(!p_handler)
    {
        return EGPIO_FAIL;
    }
    if(set_driver_level >= 4)
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

    if(group_count_max == 0)
    {
        return EGPIO_FAIL;
    }
    else if(group_count_max == 1)
    {
        user_gpio_set = tmp_sys_gpio_data;
    }
    else if(gpio_name)
    {
        for(i=0; i<group_count_max; i++)
        {
            if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
            {
                tmp_sys_gpio_data ++;
                continue;
            }
            user_gpio_set = tmp_sys_gpio_data;
            break;
        }
    }
    if(!user_gpio_set)
    {
        return EGPIO_FAIL;
    }

    port     = user_gpio_set->port;
    port_num = user_gpio_set->port_num;
    port_num_dlevel = port_num >> 4;

    tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_dlevel);
    reg_val = GPIO_REG_READ(tmp_group_dlevel_addr);
    reg_val &= ~(0x03 << (((port_num - (port_num_dlevel<<4))<<1)));
    reg_val |=  (set_driver_level << (((port_num - (port_num_dlevel<<4))<<1)));
    GPIO_REG_WRITE(tmp_group_dlevel_addr, reg_val);

    return EGPIO_SUCCESS;
}

__s32  gpio_read_one_pin_value(ulong p_handler, const char *gpio_name)
{
    char               *tmp_buf;                                      
    __u32               group_count_max;                               
    system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
    __u32               port, port_num, port_num_func, func_val;
    __u32                i, reg_val;
   
    if(!p_handler)
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

    if(group_count_max == 0)
    {
        return EGPIO_FAIL;
    }
    else if(group_count_max == 1)
    {
        user_gpio_set = tmp_sys_gpio_data;
    }
    else if(gpio_name)
    {
        for(i=0; i<group_count_max; i++)
        {
            if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
            {
                tmp_sys_gpio_data ++;
                continue;
            }
            user_gpio_set = tmp_sys_gpio_data;
            break;
        }
    }
    if(!user_gpio_set)
    {
        return EGPIO_FAIL;
    }

    port     = user_gpio_set->port;
    port_num = user_gpio_set->port_num;
    port_num_func = port_num >> 3;

    reg_val  = PIO_REG_CFG_VALUE(port, port_num_func);

    func_val = (reg_val >> ((port_num - (port_num_func<<3))<<2)) & 0x07;
    if(func_val == 0)
    {
        reg_val = (PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;
        return reg_val;
    }

    return EGPIO_FAIL;
}

__s32  gpio_write_one_pin_value(ulong p_handler, __u32 value_to_gpio, const char *gpio_name)
{
    char               *tmp_buf;                                       
    __u32               group_count_max;                             
    system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
    volatile __u32     *tmp_group_data_addr = NULL;
    __u32               port, port_num, port_num_func, func_val;
    __u32                i, reg_val;
   
    if(!p_handler)
    {
        return EGPIO_FAIL;
    }
    if(value_to_gpio >= 2)
    {
        return EGPIO_FAIL;
    }
    tmp_buf = (char *)p_handler;
    group_count_max = *(int *)tmp_buf;
    tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

    if(group_count_max == 0)
    {
        return EGPIO_FAIL;
    }
    else if(group_count_max == 1)
    {
        user_gpio_set = tmp_sys_gpio_data;
    }
    else if(gpio_name)
    {
        for(i=0; i<group_count_max; i++)
        {
            if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
            {
                tmp_sys_gpio_data ++;
                continue;
            }
            user_gpio_set = tmp_sys_gpio_data;
            break;
        }
    }
    if(!user_gpio_set)
    {
        return EGPIO_FAIL;
    }

    port     = user_gpio_set->port;
    port_num = user_gpio_set->port_num;
    port_num_func = port_num >> 3;

    reg_val  = PIO_REG_CFG_VALUE(port, port_num_func);
    func_val = (reg_val >> ((port_num - (port_num_func<<3))<<2)) & 0x07;
    if(func_val == 1)
    {
        tmp_group_data_addr = PIO_REG_DATA(port);
        reg_val = GPIO_REG_READ(tmp_group_data_addr);
        reg_val &= ~(1 << port_num);
        reg_val |=  (value_to_gpio << port_num);
        GPIO_REG_WRITE(tmp_group_data_addr, reg_val);

        return EGPIO_SUCCESS;
    }

    return EGPIO_FAIL;
}

int gpio_request_early(void  *user_gpio_list, __u32 group_count_max, __s32 set_gpio)
{
	user_gpio_set_t    *tmp_user_gpio_data, *gpio_list;
	__u32				first_port;                      
	__u32               tmp_group_func_data;
	__u32               tmp_group_pull_data;
	__u32               tmp_group_dlevel_data;
	__u32               tmp_group_data_data;
	__u32               data_change = 0;
	//	__u32			   *tmp_group_port_addr;
	volatile __u32     *tmp_group_func_addr,   *tmp_group_pull_addr;
	volatile __u32     *tmp_group_dlevel_addr, *tmp_group_data_addr;
	__u32  				port, port_num, port_num_func, port_num_pull;
	__u32  				pre_port, pre_port_num_func;
	__u32  				pre_port_num_pull;
	__s32               i, tmp_val;


	debug("gpio: conut=%d, set gpio = %d\n", group_count_max, set_gpio);
		gpio_list = (user_gpio_set_t *)user_gpio_list;

	for(first_port = 0; first_port < group_count_max; first_port++)
	{
		tmp_user_gpio_data = gpio_list + first_port;
		port     = tmp_user_gpio_data->port;                        
		port_num = tmp_user_gpio_data->port_num;                  
		if(!port)
		{
			continue;
		}
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);

		tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);  
		tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull); 
		tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);
		tmp_group_data_addr    = PIO_REG_DATA(port);

		tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
		tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
		tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
		tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);

		pre_port          = port;
		pre_port_num_func = port_num_func;
		pre_port_num_pull = port_num_pull;

		tmp_val = (port_num - (port_num_func << 3)) << 2;
		tmp_group_func_data &= ~(0x07 << tmp_val);
		if(set_gpio)
		{
			tmp_group_func_data |= (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
		}

		tmp_val = (port_num - (port_num_pull << 4)) << 1;
		if(tmp_user_gpio_data->pull >= 0)
		{
			tmp_group_pull_data &= ~(                           0x03  << tmp_val);
			tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
		}

		if(tmp_user_gpio_data->drv_level >= 0)
		{
			tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
			tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
		}

		if(tmp_user_gpio_data->mul_sel == 1)
		{
		    if(tmp_user_gpio_data->data >= 0)
		    {
		    	tmp_val = tmp_user_gpio_data->data & 1;
		        tmp_group_data_data &= ~(1 << port_num);
		        tmp_group_data_data |= tmp_val << port_num;
		        data_change = 1;
		    }
		}
		debug("---name = %s, port = %d,portnum=%d,mul_sel=%d,pull=%d drive= %d, data=%d\n",
					tmp_user_gpio_data->gpio_name,
					tmp_user_gpio_data->port,
					tmp_user_gpio_data->port_num,
					tmp_user_gpio_data->mul_sel,
					tmp_user_gpio_data->pull,
					tmp_user_gpio_data->drv_level,
					tmp_user_gpio_data->data);
	

		break;
	}

	if(first_port >= group_count_max)
	{
	    return -1;
	}

	for(i = first_port + 1; i < group_count_max; i++)
	{
		tmp_user_gpio_data = gpio_list + i;
		port     = tmp_user_gpio_data->port;
		port_num = tmp_user_gpio_data->port_num;
		debug("+++name = %s, port = %d,portnum=%d,mul_sel=%d,pull=%d drive= %d, data=%d\n",
				tmp_user_gpio_data->gpio_name,
				tmp_user_gpio_data->port,
				tmp_user_gpio_data->port_num,
				tmp_user_gpio_data->mul_sel,
				tmp_user_gpio_data->pull,
				tmp_user_gpio_data->drv_level,
				tmp_user_gpio_data->data);
		if(!port)
		{
			break;
		}
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);

		if((port_num_pull != pre_port_num_pull) || (port != pre_port))
		{
			GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
			GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data); 
			GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
			if(data_change)
			{
				data_change = 0;
				GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
			}

			tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func); 
			tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);
			tmp_group_data_addr    = PIO_REG_DATA(port);  


			tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
			tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
			tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
			tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);
		}
		else if(pre_port_num_func != port_num_func)                    
		{
			GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);  
			tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func); 
			tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);		
		}

		pre_port_num_pull = port_num_pull;                    
		pre_port_num_func = port_num_func;
		pre_port          = port;

	
		tmp_val = (port_num - (port_num_func << 3)) << 2;
		if(tmp_user_gpio_data->mul_sel >= 0)
		{
			tmp_group_func_data &= ~(0x07  << tmp_val);
			if(set_gpio)
			{
				tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
			}
		}

		tmp_val = (port_num - (port_num_pull << 4)) << 1;
		if(tmp_user_gpio_data->pull >= 0)
		{
			tmp_group_pull_data &= ~(                           0x03  << tmp_val);
			tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
		}
       
		if(tmp_user_gpio_data->drv_level >= 0)
		{
			tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
			tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
		}
      
		if(tmp_user_gpio_data->mul_sel == 1)
		{
		    if(tmp_user_gpio_data->data >= 0)
		    {
		    	tmp_val = tmp_user_gpio_data->data & 1;
		        tmp_group_data_data &= ~(1 << port_num);
		        tmp_group_data_data |= tmp_val << port_num;
		        data_change = 1;
		    }
		}
      
	}
        
	if(tmp_group_func_addr)                        
	{                                              
		GPIO_REG_WRITE(tmp_group_func_addr,   tmp_group_func_data); 
		GPIO_REG_WRITE(tmp_group_pull_addr,   tmp_group_pull_data); 
		GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
		if(data_change)
		{
		    GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
		}
	}

    return 0;
}

ulong gpio_request(user_gpio_set_t *gpio_list, __u32 group_count_max)
{
    char               *user_gpio_buf;                                     
    system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
    user_gpio_set_t  *tmp_user_gpio_data;
    __u32                real_gpio_count = 0, first_port;
    __u32               tmp_group_func_data = 0;
    __u32               tmp_group_pull_data = 0;
    __u32               tmp_group_dlevel_data = 0;
    __u32               tmp_group_data_data = 0;
    __u32               func_change = 0, pull_change = 0;
    __u32               dlevel_change = 0, data_change = 0;
    volatile __u32  *tmp_group_func_addr = NULL, *tmp_group_pull_addr = NULL;
    volatile __u32  *tmp_group_dlevel_addr = NULL, *tmp_group_data_addr = NULL;
    __u32  port, port_num, port_num_func, port_num_pull;
    __u32  pre_port = 0x7fffffff, pre_port_num_func = 0x7fffffff;
    __u32  pre_port_num_pull = 0x7fffffff;
    __s32  i, tmp_val;
    if((!gpio_list) || (!group_count_max))
    {
        return (u32)0;
    }
    for(i = 0; i < group_count_max; i++)
    {
        tmp_user_gpio_data = gpio_list + i;        
        if(!tmp_user_gpio_data->port)
        {
            continue;
        }
        real_gpio_count ++;
    }

 
    user_gpio_buf = (char *)malloc(16 + sizeof(system_gpio_set_t) * real_gpio_count);  
    if(!user_gpio_buf)
    {
        return (u32)0;
    }
    memset(user_gpio_buf, 0, 16 + sizeof(system_gpio_set_t) * real_gpio_count);        
    *(int *)user_gpio_buf = real_gpio_count;                                          
    user_gpio_set = (system_gpio_set_t *)(user_gpio_buf + 16);                    
   
    for(first_port = 0; first_port < group_count_max; first_port++)
    {
        tmp_user_gpio_data = gpio_list + first_port;
        port     = tmp_user_gpio_data->port;                       
        port_num = tmp_user_gpio_data->port_num;                    
        if(!port)
        {
            continue;
        }
        port_num_func = (port_num >> 3);
        port_num_pull = (port_num >> 4);

        tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func); 
        tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);
        tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);
        tmp_group_data_addr    = PIO_REG_DATA(port);               

        tmp_group_func_data    = *tmp_group_func_addr;
        tmp_group_pull_data    = *tmp_group_pull_addr;
        tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
        tmp_group_data_data    = *tmp_group_data_addr;
        break;
    }
    if(first_port >= group_count_max)
    {
        return 0;
    }
   
    for(i = first_port; i < group_count_max; i++)
    {
        tmp_sys_gpio_data  = user_gpio_set + i;            
        tmp_user_gpio_data = gpio_list + i;                
        port     = tmp_user_gpio_data->port;             
        port_num = tmp_user_gpio_data->port_num;
        if(!port)
        {
            continue;
        }
      
        strcpy(tmp_sys_gpio_data->gpio_name, tmp_user_gpio_data->gpio_name);
        tmp_sys_gpio_data->port                       = port;
        tmp_sys_gpio_data->port_num                   = port_num;
        tmp_sys_gpio_data->user_gpio_status.mul_sel   = tmp_user_gpio_data->mul_sel;
        tmp_sys_gpio_data->user_gpio_status.pull      = tmp_user_gpio_data->pull;
        tmp_sys_gpio_data->user_gpio_status.drv_level = tmp_user_gpio_data->drv_level;
        tmp_sys_gpio_data->user_gpio_status.data      = tmp_user_gpio_data->data;

        port_num_func = (port_num >> 3);
        port_num_pull = (port_num >> 4);

        if((port_num_pull != pre_port_num_pull) || (port != pre_port))  
        {
            if(func_change)
            {
                *tmp_group_func_addr   = tmp_group_func_data;  
                func_change = 0;
            }
            if(pull_change)
            {
                pull_change = 0;
                *tmp_group_pull_addr   = tmp_group_pull_data;  
            }
            if(dlevel_change)
            {
                dlevel_change = 0;
                *tmp_group_dlevel_addr = tmp_group_dlevel_data;
            }
            if(data_change)
            {
                data_change = 0;
                *tmp_group_data_addr   = tmp_group_data_data;
            }

            tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func); 
            tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);
            tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);
            tmp_group_data_addr    = PIO_REG_DATA(port); 

			tmp_group_func_data    = *tmp_group_func_addr;
            tmp_group_pull_data    = *tmp_group_pull_addr;
            tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
            tmp_group_data_data    = *tmp_group_data_addr;

        }
        else if(pre_port_num_func != port_num_func)            
        {
            *tmp_group_func_addr   = tmp_group_func_data;

           tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);

            tmp_group_func_data    = *tmp_group_func_addr;
        }
       
        pre_port_num_pull = port_num_pull;   
        pre_port_num_func = port_num_func;
        pre_port          = port;

    
        if(tmp_user_gpio_data->mul_sel >= 0)
        {
            tmp_val = (port_num - (port_num_func<<3)) << 2;
            tmp_sys_gpio_data->hardware_gpio_status.mul_sel = (tmp_group_func_data >> tmp_val) & 0x07;
            tmp_group_func_data &= ~(                              0x07  << tmp_val);
            tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
            func_change = 1;
        }
     

        tmp_val = (port_num - (port_num_pull<<4)) << 1;

        if(tmp_user_gpio_data->pull >= 0)
        {
            tmp_sys_gpio_data->hardware_gpio_status.pull = (tmp_group_pull_data >> tmp_val) & 0x03;
            if(tmp_user_gpio_data->pull >= 0)
            {
                tmp_group_pull_data &= ~(                           0x03  << tmp_val);
                tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
                pull_change = 1;
            }
        }
     
        if(tmp_user_gpio_data->drv_level >= 0)
        {
            tmp_sys_gpio_data->hardware_gpio_status.drv_level = (tmp_group_dlevel_data >> tmp_val) & 0x03;
            if(tmp_user_gpio_data->drv_level >= 0)
            {
                tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
                tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
                dlevel_change = 1;
            }
        }
      
        if(tmp_user_gpio_data->mul_sel == 1)
        {
            if(tmp_user_gpio_data->data >= 0)
            {
                tmp_val = tmp_user_gpio_data->data;
                tmp_val &= 1;
                tmp_group_data_data &= ~(1 << port_num);
                tmp_group_data_data |= tmp_val << port_num;
                data_change = 1;
            }
        }
    }

   
    if(tmp_group_func_addr)                        
    {                                              
        *tmp_group_func_addr   = tmp_group_func_data;     
        if(pull_change)
        {
            *tmp_group_pull_addr   = tmp_group_pull_data;  
        }
        if(dlevel_change)
        {
            *tmp_group_dlevel_addr = tmp_group_dlevel_data;
        }
        if(data_change)
        {
            *tmp_group_data_addr   = tmp_group_data_data;   
        }
    }
    return (ulong)user_gpio_buf;
}

       
//note : just free malloc memory
__s32 gpio_release(ulong p_handler, __s32 unused_para)
{
	char               *tmp_buf;                                     
	__u32               group_count_max;             

	if(!p_handler)
	{
		return EGPIO_FAIL;
	}
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	if(!group_count_max)
	{
		return EGPIO_FAIL;
	}

	free((char *)p_handler);

	return EGPIO_SUCCESS;
}




#define FDT_INFO(fmt,args...) printf("FDT INFO:"fmt,##args);
#define FDT_ERROR(fmt,args...) printf("FDT ERROR:"fmt,##args);


/**
 * fdt_get_pin - Read all pin from node
 *
 * @node_path: fdt path or alians
 * @prop_name:  pin property name--ex "pinctrl-0"
 * @gpio_list:  recevice the gpio data
 */
int fdt_get_all_pin(int nodeoffset,const char* pinctrl_name,user_gpio_set_t* gpio_list)
{
	//int  nodeoffset;	/* node offset from libfdt */
	char *pins = NULL;
	char *function = NULL;
	int  ret;
	char path_tmp[128] = {0};
	char port_name[20][12]; //format PA0,PB6 ....
	char pin_name[20][64] = {{0}}; //
	u32 handle[10] = {0};
	int handle_num = 0;
	int i,j;
	int name_num = 0,port_num = 0;
	int gpio_list_index = 0;

	u32 pull;
	u32 drive;
	u32 muxsel;

	//get property vaule by handle
	/*nodeoffset = fdt_path_offset (working_fdt,node_path );
	if(nodeoffset < 0)
	{
		printf ("error:fdt err returned %s\n",fdt_strerror(nodeoffset));
		return -1;
	}*/
	handle_num = fdt_getprop_u32(working_fdt,nodeoffset,pinctrl_name,handle);
	if(handle_num < 0)
	{
		FDT_ERROR("%s:get property handle %s error:%s\n",__func__, pinctrl_name,fdt_strerror(handle_num) );
		return -1;
	}
	for(i = 0; i < handle_num; i++)
	{
		debug("%s:get property handle  %s ok, index %d,value is 0x%x\n",__func__, pinctrl_name,i,handle[i] );
		nodeoffset = fdt_node_offset_by_phandle(working_fdt,handle[i]);
		if(nodeoffset < 0 )
		{
			FDT_ERROR("%s:get property by handle error\n",__func__);
			return -1;
		}
		ret = fdt_get_path(working_fdt,nodeoffset,path_tmp,sizeof(path_tmp));
		if(ret < 0 )
		{
			FDT_ERROR("%s:get path by nodeoffset error\n",__func__);
			return -1;
		}
		debug("path for handle is %s\n", path_tmp);

		name_num = 0;
		port_num = 0;
		ret = fdt_getprop_string(working_fdt,nodeoffset,"allwinner,pins",&pins);
		if(ret >=0 )
		{
			int len = ret;
			char *data = pins;
			j = 0;
			while (j < len) {
				strcpy(port_name[port_num++],data);
				j    += strlen(data) + 1;
				data += strlen(data) + 1;
			}
		}

		ret = fdt_getprop_string(working_fdt,nodeoffset,"allwinner,pname",&pins);
		if(ret >=0 )
		{
			int len = ret;
			char *data = pins;
			j = 0;
			while (j < len) {
				strcpy(pin_name[name_num++],data);
				j    += strlen(data) + 1;
				data += strlen(data) + 1;
			}
		}
#if 0
		if(name_num != port_num)
		{
			printf ("error:pin name num(%d) != port num(%d)\n",name_num, port_num);
			return -1;
		}
#endif
		ret = fdt_getprop_string(working_fdt,nodeoffset,"allwinner,function",&function);
		if(ret < 0 )
		{
			FDT_ERROR("get function err returned %s\n",fdt_strerror(ret));
			return -1;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"allwinner,muxsel",&muxsel);
		if(ret < 0 )
		{
			FDT_ERROR("get muxsel err returned %s\n",fdt_strerror(ret));
			return -1;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"allwinner,drive",&drive);
		if(ret < 0 )
		{
			FDT_ERROR("get drive err returned %s\n",fdt_strerror(ret));
			return -1;
		}

		ret = fdt_getprop_u32(working_fdt,nodeoffset,"allwinner,pull",&pull);
		if(ret < 0)
		{
			FDT_ERROR("get pull err returned %s\n",  fdt_strerror(ret));
			return -1;
		}
		for(j = 0; j < port_num; j++)
		{
			char *src;
			char ch;
			int tmp_value = 0;
			int tmp_mul;

			src = &port_name[j][2];
			ch = *src++;
			while(ch != '\0')
			{
				if((ch >= '0') && (ch <= '9'))
				{
					tmp_value = tmp_value * 10 + (ch - '0');
					ch = *src++;
				}
				else
				{
					FDT_ERROR("format gpio %s\n",pin_name[j] );
					break;
				}
			}
			tmp_mul = muxsel;//get_mul_by_func_name(function);
			if(tmp_mul<0)
			{
				FDT_ERROR(" muxsel not configed\n");
				return -1;
			}
			strcpy(gpio_list[gpio_list_index].gpio_name, pin_name[j]);
			gpio_list[gpio_list_index].port = port_name[j][1] - 'A'+1;
			gpio_list[gpio_list_index].port_num = tmp_value;
			gpio_list[gpio_list_index].mul_sel = tmp_mul&0x7;
			gpio_list[gpio_list_index].pull = pull;
			gpio_list[gpio_list_index].drv_level = drive;
			gpio_list[gpio_list_index].data = 0;
			gpio_list_index++;
		}
	}

	for(i = 0; i < gpio_list_index; i++)
	{
		debug("name = %s, port = %d,portnum=%d,mul_sel=%d,pull=%d drive= %d, data=%d\n",
			gpio_list[i].gpio_name,
			gpio_list[i].port,
			gpio_list[i].port_num,
			gpio_list[i].mul_sel,
			gpio_list[i].pull,
			gpio_list[i].drv_level,
			gpio_list[i].data);

	}
	return gpio_list_index;
}

/**
 * fdt_get_pin_num - get pin num from  device node
 *
 * @node_path: device path or alians
 * @prop_name: ex "pinctrl-0"

 */
int fdt_get_pin_num(int nodeoffset,const char* pinctrl_name)
{
	//int  nodeoffset;	/* node offset from libfdt */
	char *pins = NULL;
	int  ret;
	u32 handle[10] = {0};
	int handle_num = 0;
	int i,j;
	int port_num = 0;

	//get property vaule by handle
	/*nodeoffset = fdt_path_offset (working_fdt,node_path );
	if(nodeoffset < 0)
	{
		printf ("error:fdt err returned %s\n",fdt_strerror(nodeoffset));
		return -1;
	}*/
	handle_num = fdt_getprop_u32(working_fdt,nodeoffset,pinctrl_name,handle);
	if(handle_num < 0)
	{
		FDT_ERROR("%s:get property handle %s error:%s\n",__func__, pinctrl_name,fdt_strerror(handle_num) );
		return -1;
	}
	port_num = 0;
	for(i = 0; i < handle_num; i++)
	{
		//printf("get property handle  %s ok, index %d,value is 0x%x\n", "pinctrl-0",i,handle[i] );
		nodeoffset = fdt_node_offset_by_phandle(working_fdt,handle[i]);
		if(nodeoffset < 0 )
		{
			FDT_ERROR("%s:get property by handle error\n",__func__);
			return -1;
		}
		ret = fdt_getprop_string(working_fdt,nodeoffset,"allwinner,pins",&pins);
		if(ret >=0 )
		{
			int len = ret;
			char *data = pins;
			j = 0;
			while (j < len) {
				port_num++;
				j    += strlen(data) + 1;
				data += strlen(data) + 1;
			}
		}
	}
	return port_num;
}


/**
 * fdt_config_all_pin - config all pin for device
 *
 * @node_path:  device tree path

 */
int fdt_set_all_pin(const char* node_path,const char* pinctrl_name)
{
	user_gpio_set_t  *pin_set;
	int pin_count = 0;
	int ret = 0;
	int nodeoffset;

	//get property vaule by handle
	nodeoffset = fdt_path_offset (working_fdt,node_path );
	if(nodeoffset < 0)
	{
		FDT_ERROR("%s:fdt err returned %s\n",__func__,fdt_strerror(nodeoffset));
		return -1;
	}

	pin_count = fdt_get_pin_num(nodeoffset,pinctrl_name);
	if(pin_count < 0)
	{
		return -1;
	}
	pin_set = malloc(pin_count*sizeof(user_gpio_set_t));
	if(pin_set == NULL)
	{
		FDT_ERROR("%s:malloc fail\n", __func__);
		return -1;
	}
	ret = fdt_get_all_pin( nodeoffset, pinctrl_name,pin_set);
	if(ret < 0)
	{
		return -1;
	}
	ret =  gpio_request_early(pin_set,pin_count,1);
	free(pin_set);

	return ret;
}

int fdt_set_all_pin_by_offset(int nodeoffset,const char* pinctrl_name)
{
	user_gpio_set_t  *pin_set;
	int pin_count = 0;
	int ret = 0;

	if(nodeoffset < 0)
	{
		FDT_ERROR("%s:fdt bad nodeoffset %d\n",__func__,nodeoffset);
		return -1;
	}

	pin_count = fdt_get_pin_num(nodeoffset,pinctrl_name);
	if(pin_count < 0)
	{
		return -1;
	}
	pin_set = malloc(pin_count*sizeof(user_gpio_set_t));
	if(pin_set == NULL)
	{
		FDT_ERROR("%s:malloc fail\n", __func__);
		return -1;
	}
	ret = fdt_get_all_pin( nodeoffset, pinctrl_name,pin_set);
	if(ret < 0)
	{
		return -1;
	}
	ret =  gpio_request_early(pin_set,pin_count,1);
	free(pin_set);

	return ret;
}

/**
 * fdt_config_all_pin - config all pin for device
 *
 * @node_path:  device tree path

 */
int fdt_set_pin_byname(user_gpio_set_t  *pin_list,int pin_count, const char* pin_name)
{
	user_gpio_set_t  *pin;
	int ret = 0;
	int i = 0;
	for(i = 0; i < pin_count; i++)
	{
		pin = pin_list+i;
		if(strcmp(pin->gpio_name, pin_name) == 0)
		{
			break;
		}
	}
	//not find
	if(i >= pin_count)
	{
		return -1;
	}

	ret =  gpio_request_early(pin,1,1);

	return ret;
}


/**
 * fdt_get_gpio - Read one gpio from node
 *
 * @nodeoffset: fdt nodeoffset
 * @prop_name:  gpio property naem
 * @gpio_list:  recevice the gpio data
 */
int fdt_get_one_gpio_by_offset(int nodeoffset, const char* prop_name,user_gpio_set_t* gpio_list)
{
	int ret ;
	u32 data[10];

	memset(data, 0, sizeof(data));
	//get property vaule by handle
	if(nodeoffset < 0)
	{
		debug ("fdt err returned %s\n",fdt_strerror(nodeoffset));
		return -1;
	}

	ret = fdt_getprop_u32(working_fdt,nodeoffset,prop_name,data);
	if(ret < 0 )
	{
		debug("%s : err returned %s\n",__func__,fdt_strerror(ret));
		return -1;
	}

	strcpy(gpio_list->gpio_name,prop_name);

	gpio_list->port = data[1] + 1;  //0: PA
	gpio_list->port_num = data[2];
	gpio_list->mul_sel = data[3];
	gpio_list->pull = data[4];
	gpio_list->drv_level = data[5];
	gpio_list->data = data[6];

	debug("name = %s, port = %x,portnum=%x,mul_sel=%x,pull=%x drive= %x, data=%x\n",
			gpio_list->gpio_name,
			gpio_list->port,
			gpio_list->port_num,
			gpio_list->mul_sel,
			gpio_list->pull,
			gpio_list->drv_level,
			gpio_list->data);
	return 0;

}

int fdt_get_one_gpio(const char* node_path, const char* prop_name,user_gpio_set_t* gpio_list)
{
	int nodeoffset;	/* node offset from libfdt */
	int ret ;
	u32 data[10];

	memset(data, 0, sizeof(data));
	//get property vaule by handle
	nodeoffset = fdt_path_offset (working_fdt,node_path );
	if(nodeoffset < 0)
	{
		debug ("fdt err returned %s\n",fdt_strerror(nodeoffset));
		return -1;
	}

	ret = fdt_getprop_u32(working_fdt,nodeoffset,prop_name,data);
	if(ret < 0 )
	{
		debug("%s :%s|%s err returned %s\n",__func__,node_path,prop_name,fdt_strerror(ret));
		return -1;
	}

	strcpy(gpio_list->gpio_name,prop_name);

	gpio_list->port = data[1] + 1;  //0: PA
	gpio_list->port_num = data[2];
	gpio_list->mul_sel = data[3];
	gpio_list->pull = data[4];
	gpio_list->drv_level = data[5];
	gpio_list->data = data[6];

	debug("name = %s, port = %x,portnum=%x,mul_sel=%x,pull=%x drive= %x, data=%x\n",
			gpio_list->gpio_name,
			gpio_list->port,
			gpio_list->port_num,
			gpio_list->mul_sel,
			gpio_list->pull,
			gpio_list->drv_level,
			gpio_list->data);
	return 0;

}
/**
 * fdt_set_one_gpio - config one gpio for device
 *
 * @node_path:  device tree path
 * @prop_name: gpio name  ex "vdevice_gpio_1"   ---- vdevice_gpio_1 = <&r_pio PL 0 1 1 1 1>;
 */
int fdt_set_one_gpio(const char* node_path, const char* prop_name)
{
	user_gpio_set_t gpio_set[1];
	if(fdt_get_one_gpio(node_path, prop_name,gpio_set))
	{
		return -1;
	}
	return gpio_request_early(&gpio_set,1,1);
}


int fdt_set_normal_gpio(user_gpio_set_t  *gpio_set, int gpio_count)
{
	return gpio_request_early(&gpio_set,gpio_count,1);
}
