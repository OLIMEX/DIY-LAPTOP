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
#include "common.h"
#include "asm/io.h"
#include "asm/armv7.h"
#include "asm/arch/cpu.h"
#include "asm/arch/ccmu.h"
#include "asm/arch/timer.h"
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
static int clk_set_divd(void)
{
	unsigned int reg_val;

	//config axi
	reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
	reg_val &= ~(0x03 << 8);
	reg_val |=  (0x01 << 8);
	reg_val |=  (1 << 0);
	writel(reg_val, CCMU_CPUX_AXI_CFG_REG);
	
	//config ahb
	reg_val = readl(CCMU_AHB1_APB1_CFG_REG);;
	reg_val &= ~((0x03 << 12) | (0x03 << 8) |(0x03 << 4));
	reg_val |=  (0x02 << 12);
	reg_val |=  (2 << 6);
	reg_val |=  (1 << 8);

	writel(reg_val, CCMU_AHB1_APB1_CFG_REG);

	return 0;
}
/*******************************************************************************
*函数名称: set_pll
*函数原型：void set_pll( void )
*函数功能: 调整CPU频率
*入口参数: void
*返 回 值: void
*备    注:
*******************************************************************************/
void set_pll( void )
{
    unsigned int reg_val;
    unsigned int i;
    //设置时钟为默认408M

    //切换到24M
    reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
    reg_val &= ~(0x01 << 16);
    reg_val |=  (0x01 << 16);
	reg_val |=  (0x01 << 0);
    writel(reg_val, CCMU_CPUX_AXI_CFG_REG);
    //延时，等待时钟稳定
    for(i=0; i<0x400; i++);
	//回写PLL1
    reg_val = (0x01<<12)|(0x01<<31);
    writel(reg_val, CCMU_PLL_CPUX_CTRL_REG);
    //延时，等待时钟稳定
#ifndef CONFIG_FPGA
	do
	{
		reg_val = readl(CCMU_PLL_CPUX_CTRL_REG);
	}
	while(!(reg_val & (0x1 << 28)));
#endif
    //修改AXI,AHB,APB分频
    clk_set_divd();
		//dma reset
	writel(readl(CCMU_BUS_SOFT_RST_REG0)  | (1 << 6), CCMU_BUS_SOFT_RST_REG0);
	for(i=0;i<100;i++);
	//gating clock for dma pass
	writel(readl(CCMU_BUS_CLK_GATING_REG0) | (1 << 6), CCMU_BUS_CLK_GATING_REG0);
	//打开MBUS,clk src is pll6
	writel(0x80000000, CCMU_MBUS_RST_REG);       //Assert mbus domain
	writel(0x81000002, CCMU_MBUS_CLK_REG);  //dram>600M, so mbus from 300M->400M
	//使能PLL6
	writel(readl(CCMU_PLL_PERIPH0_CTRL_REG) | (1U << 31), CCMU_PLL_PERIPH0_CTRL_REG);

    //切换时钟到COREPLL上
    reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
    reg_val &= ~(0x03 << 16);
    reg_val |=  (0x02 << 16);
    writel(reg_val, CCMU_CPUX_AXI_CFG_REG);

    return  ;
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
void reset_pll( void )
{
	writel(0x00010000, CCMU_CPUX_AXI_CFG_REG);
	writel(0x00001000, CCMU_PLL_CPUX_CTRL_REG);
	writel(0x00001010, CCMU_AHB1_APB1_CFG_REG);

	return ;
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
void set_gpio_gate(void)
{
	writel(readl(CCMU_BUS_CLK_GATING_REG2)   | (1 << 5), CCMU_BUS_CLK_GATING_REG2);
}
