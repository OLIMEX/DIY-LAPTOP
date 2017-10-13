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

#ifndef _SUNXI_GPIO_H
#define _SUNXI_GPIO_H



#define GPIO_REG_READ(reg)              readl((reg))
#define GPIO_REG_WRITE(reg, value)      writel((value), (reg))


#define PIOC_REG_o_CFG0                 0x00
#define PIOC_REG_o_CFG1                 0x04
#define PIOC_REG_o_CFG2                 0x08
#define PIOC_REG_o_CFG3                 0x0C
#define PIOC_REG_o_DATA                 0x10
#define PIOC_REG_o_DRV0                 0x14
#define PIOC_REG_o_DRV1                 0x18
#define PIOC_REG_o_PUL0                 0x1C
#define PIOC_REG_o_PUL1                 0x20


/**#############################################################################################################
 *
 *                           GPIO(PIN) Operations
 *
-##############################################################################################################*/
#define PIO_REG_CFG(n, i)               ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00))
#define PIO_REG_DLEVEL(n, i)            ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14))
#define PIO_REG_PULL(n, i)              ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C))
#define PIO_REG_DATA(n)                   ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10))

#define PIO_REG_CFG_VALUE(n, i)          readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00)
#define PIO_REG_DLEVEL_VALUE(n, i)       readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14)
#define PIO_REG_PULL_VALUE(n, i)         readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C)
#define PIO_REG_DATA_VALUE(n)            readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10)
#define PIO_REG_BASE(n)                    ((volatile unsigned int *)(SUNXI_PIO_BASE +((n)-1)*24))

#ifdef SUNXI_R_PIO_BASE
#define R_PIO_REG_CFG(n, i)               ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00))
#define R_PIO_REG_DLEVEL(n, i)            ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14))
#define R_PIO_REG_PULL(n, i)              ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C))
#define R_PIO_REG_DATA(n)                 ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + 0x10))

#define R_PIO_REG_CFG_VALUE(n, i)          readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00)
#define R_PIO_REG_DLEVEL_VALUE(n, i)       readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14)
#define R_PIO_REG_PULL_VALUE(n, i)         readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C)
#define R_PIO_REG_DATA_VALUE(n)            readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + 0x10)
#define R_PIO_REG_BASE(n)                    ((volatile unsigned int *)(SUNXI_R_PIO_BASE +((n)-12)*24))
#endif
typedef struct
{
    int mul_sel;
    int pull;
    int drv_level;
    int data;
} gpio_status_set_t;

typedef struct
{
    char    gpio_name[32];
    int     port;
    int     port_num;
    gpio_status_set_t user_gpio_status;
    gpio_status_set_t hardware_gpio_status;
} system_gpio_set_t;

//通用的，和GPIO相关的数据结构
typedef struct
{
    unsigned char      port;                       //端口号
    unsigned char      port_num;                   //端口内编号
    char               mul_sel;                    //功能编号
    char               pull;                       //电阻状态
    char               drv_level;                  //驱动驱动能力
    char               data;                       //输出电平
    unsigned char      reserved[2];                //保留位，保证对齐
}
normal_gpio_set_t;

int boot_set_gpio(void  *user_gpio_list, __u32 group_count_max, __s32 set_gpio);

#endif /* _SUNXI_GPIO_H */
