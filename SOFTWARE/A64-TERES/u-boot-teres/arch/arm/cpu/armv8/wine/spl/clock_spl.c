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
#include "asm/arch/ccmu.h"
#include "asm/arch/timer.h"
#include "asm/arch/archdef.h"



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
	__u32 reg_val;

	//select CPUX  clock src: OSC24M，AXI divide ratio is 2, system apb clk ratio is 4
	//cpu/axi /sys apb  clock ratio
	writel((1<<16) | (3<<8) | (2<<0), CCMU_CPUX_AXI_CFG_REG);
    
    //set PLL_CPUX, the  default  clk is 408M  ,PLL_OUTPUT= 24M*N*K/( M*P)
	writel(0x001000, CCMU_PLL_CPUX_CTRL_REG);
    //wait PLL_CPUX stable
#ifndef FPGA_PLATFORM
	while(!(readl(CCMU_PLL_CPUX_CTRL_REG) & (0x1<<28)));
#endif

    //enable pll_hsic, default is 480M
    writel(0x03001300 | (1<<31), CCMU_PLL_HSIC_CTRL_REG);  //set default value
    //cci400 clk src is pll_hsic: (2<<24) , div is 1:(0<<0)
#ifndef FPGA_PLATFORM
    while(!(readl(CCMU_PLL_HSIC_CTRL_REG) & (0x1<<28)));
#endif

    //change ahb src before set pll6
    writel((0x01 << 12) | (readl(CCMU_AHB1_APB1_CFG_REG)&(~(0x3<<12))), CCMU_AHB1_APB1_CFG_REG);
    //enable PLL6:  600M(1X)  1200M(2x)
    writel( 0x41811 | (1U << 31), CCMU_PLL_PERIPH0_CTRL_REG);
    __usdelay(1000);
	//set AHB1/APB1 clock  divide ratio
	//ahb1 clock src is PLL6,                           (0x03<< 12)
	//apb1 clk src is ahb1 clk src, divide  ratio is 2  (1<<8)
	//ahb1 pre divide  ratio is 2:    0:1  , 1:2,  2:3,   3:4 (2<<6)
    //PLL6:AHB1:APB1 = 600M:200M:100M ,
    writel((0x03 << 12) | (1<<8) | (2<<6) | (0<<4), CCMU_AHB1_APB1_CFG_REG);

	//set and change cpu clk src to PLL_CPUX,  PLL_CPUX:AXI0 = 408M:136M
	reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
	reg_val &=  ~(3 << 16);
	reg_val |=  (2 << 16);    
	writel(reg_val, CCMU_CPUX_AXI_CFG_REG);
	__usdelay(1000);
   
	//----DMA function--------
	//dma reset
	writel(readl(CCMU_BUS_SOFT_RST_REG0)  | (1 << 6), CCMU_BUS_SOFT_RST_REG0);
	__usdelay(20);
	//gating clock for dma pass
	writel(readl(CCMU_BUS_CLK_GATING_REG0) | (1 << 6), CCMU_BUS_CLK_GATING_REG0);
    //auto gating disable
	writel(7, (SUNXI_DMA_BASE+0x20));

    //MBUS function
	//reset mbus domain
	writel(0x80000000, CCMU_MBUS_RST_REG);
    //open MBUS,clk src is pll6(2x) , pll6/(m+1) = 400M
	writel((1<<31) | (1<<24) | (2<<0), CCMU_MBUS_CLK_REG);
 
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
void reset_pll( void )
{
    //切换CPU时钟源为24M
    writel(0x10300, CCMU_CPUX_AXI_CFG_REG);

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

}

