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
#include <asm/arch/gic.h>
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
static void gic_distributor_init(void)
{
	__u32 cpumask = 0x01010101;
	__u32 gic_irqs;
	__u32 i;

	writel(0, GIC_DIST_CON);

	/* check GIC hardware configutation */
	gic_irqs = ((readl(GIC_CON_TYPE) & 0x1f) + 1) * 32;
	if (gic_irqs > 1020)
	{
		gic_irqs = 1020;
	}
	if (gic_irqs < GIC_IRQ_NUM)
	{
		printf("GIC parameter config error, only support %d"
				" irqs < %d(spec define)!!\n", gic_irqs, GIC_IRQ_NUM);
		return ;
	}
	/*  Set ALL interrupts as group1(non-secure) interrupts */
	for (i=1; i<GIC_IRQ_NUM; i+=16)
	{
	    writel(0xffffffff, GIC_CON_IGRP(i>>4));
	}
	/* set trigger type to be level-triggered, active low */
	for (i=0; i<GIC_IRQ_NUM; i+=16)
	{
		writel(0, GIC_IRQ_MOD_CFG(i>>4));
	}
	/* set priority */
	for (i=0; i<GIC_IRQ_NUM; i+=4)
	{
		writel(0xa0a0a0a0, GIC_SPI_PRIO((i-32)>>2));
	}
	/* set processor target */
	for (i=32; i<GIC_IRQ_NUM; i+=4)
	{
		writel(cpumask, GIC_SPI_PROC_TARG((i-32)>>2));
	}
	/* disable all interrupts */
	for (i=32; i<GIC_IRQ_NUM; i+=32)
	{
		writel(0xffffffff, GIC_CLR_EN(i>>5));
	}
	/* clear all interrupt active state */
	for (i=32; i<GIC_IRQ_NUM; i+=32)
	{
		writel(0xffffffff, GIC_ACT_CLR(i>>5));
	}
	writel(3, GIC_DIST_CON);

	return ;
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
static void gic_cpuif_init(void)
{
	writel(0, GIC_CPU_IF_CTRL);
	/*
	 * Deal with the banked PPI and SGI interrupts - disable all
	 * PPI interrupts, ensure all SGI interrupts are enabled.
	*/
	writel(0xffffffff, GIC_CON_IGRP(0));
	writel(0xffff0000, GIC_CLR_EN(0));
	writel(0x0000ffff, GIC_SET_EN(0));

	writel(0xf0, GIC_INT_PRIO_MASK);
	writel(0xf, GIC_CPU_IF_CTRL);

	return ;
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
int gic_init (void)
{
	gic_distributor_init();
	gic_cpuif_init();

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
int gic_exit(void)
{
	int i;
	/*  Set ALL interrupts as group0(secure) interrupts */
	for (i=1; i<GIC_IRQ_NUM; i+=16)
	{
	    writel(0, GIC_CON_IGRP(i>>4));
	}
	writel(1, GIC_DIST_CON);
	writel(0x1, GIC_CPU_IF_CTRL);

	return 0;
}

