/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
#include <asm/io.h>
#include <spare_head.h>
#include <asm/arch/gpio.h>
//#include <sys_config.h>
/*
************************************************************************************************************
*
*                                             normal_gpio_cfg
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
__s32 boot_set_gpio(void  *user_gpio_list, __u32 group_count_max, __s32 set_gpio)
{
	normal_gpio_set_t    *tmp_user_gpio_data, *gpio_list;
	__u32				first_port;                      //保存真正有效的GPIO的个数
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


   	gpio_list = (normal_gpio_set_t *)user_gpio_list;

    for(first_port = 0; first_port < group_count_max; first_port++)
    {
        tmp_user_gpio_data = gpio_list + first_port;
        port     = tmp_user_gpio_data->port;                         //读出端口数值
	    port_num = tmp_user_gpio_data->port_num;                     //读出端口中的某一个GPIO
	    if(!port)
	    {
	    	continue;
	    }
	    port_num_func = (port_num >> 3);
        port_num_pull = (port_num >> 4);

        tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //更新功能寄存器地址
        tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);  //更新pull寄存器
        tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);//更新level寄存器
        tmp_group_data_addr    = PIO_REG_DATA(port);                 //更新data寄存器

        tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
        tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
        tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
        tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);

        pre_port          = port;
        pre_port_num_func = port_num_func;
        pre_port_num_pull = port_num_pull;
        //更新功能寄存器
        tmp_val = (port_num - (port_num_func << 3)) << 2;
        tmp_group_func_data &= ~(0x07 << tmp_val);
        if(set_gpio)
        {
        	tmp_group_func_data |= (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
        }
        //根据pull的值决定是否更新pull寄存器
        tmp_val = (port_num - (port_num_pull << 4)) << 1;
        if(tmp_user_gpio_data->pull >= 0)
        {
        	tmp_group_pull_data &= ~(                           0x03  << tmp_val);
        	tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
        }
        //根据driver level的值决定是否更新driver level寄存器
        if(tmp_user_gpio_data->drv_level >= 0)
        {
        	tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
        	tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
        }
        //根据用户输入，以及功能分配决定是否更新data寄存器
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

        break;
	}
	//检查是否有数据存在
	if(first_port >= group_count_max)
	{
	    return -1;
	}
	//保存用户数据
	for(i = first_port + 1; i < group_count_max; i++)
	{
		tmp_user_gpio_data = gpio_list + i;                 //gpio_set依次指向用户的每个GPIO数组成员
	    port     = tmp_user_gpio_data->port;                //读出端口数值
	    port_num = tmp_user_gpio_data->port_num;            //读出端口中的某一个GPIO
	    if(!port)
	    {
	    	break;
	    }
        port_num_func = (port_num >> 3);
        port_num_pull = (port_num >> 4);

        if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //如果发现当前引脚的端口不一致，或者所在的pull寄存器不一致
        {
            GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);     //回写功能寄存器
            GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);     //回写pull寄存器
            GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data); //回写driver level寄存器
            if(data_change)
            {
                data_change = 0;
                GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data); //回写data寄存器
            }

        	tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //更新功能寄存器地址
        	tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);  //更新pull寄存器
        	tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);//更新level寄存器
        	tmp_group_data_addr    = PIO_REG_DATA(port);                 //更新data寄存器

            tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
            tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
            tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
            tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);
        }
        else if(pre_port_num_func != port_num_func)                       //如果发现当前引脚的功能寄存器不一致
        {
            GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);    //则只回写功能寄存器
            tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //更新功能寄存器地址

            tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
        }
		//保存当前硬件寄存器数据
        pre_port_num_pull = port_num_pull;                      //设置当前GPIO成为前一个GPIO
        pre_port_num_func = port_num_func;
        pre_port          = port;

        //更新功能寄存器
        tmp_val = (port_num - (port_num_func << 3)) << 2;
        if(tmp_user_gpio_data->mul_sel >= 0)
        {
        	tmp_group_func_data &= ~(                              0x07  << tmp_val);
        	if(set_gpio)
        	{
        		tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
        	}
        }
        //根据pull的值决定是否更新pull寄存器
        tmp_val = (port_num - (port_num_pull << 4)) << 1;
        if(tmp_user_gpio_data->pull >= 0)
        {
        	tmp_group_pull_data &= ~(                           0x03  << tmp_val);
        	tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
        }
        //根据driver level的值决定是否更新driver level寄存器
        if(tmp_user_gpio_data->drv_level >= 0)
        {
        	tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
        	tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
        }
        //根据用户输入，以及功能分配决定是否更新data寄存器
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
    //for循环结束，如果存在还没有回写的寄存器，这里写回到硬件当中
    if(tmp_group_func_addr)                         //只要更新过寄存器地址，就可以对硬件赋值
    {                                               //那么把所有的值全部回写到硬件寄存器
        GPIO_REG_WRITE(tmp_group_func_addr,   tmp_group_func_data);   //回写功能寄存器
        GPIO_REG_WRITE(tmp_group_pull_addr,   tmp_group_pull_data);   //回写pull寄存器
        GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data); //回写driver level寄存器
        if(data_change)
        {
            GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data); //回写data寄存器
        }
    }

    return 0;
}




