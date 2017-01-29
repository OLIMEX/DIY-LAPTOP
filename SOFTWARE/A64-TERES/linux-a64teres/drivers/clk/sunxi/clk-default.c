/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/clk-private.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/clk/sunxi.h>
#include <mach/sys_config.h>
#include "clk-sunxi.h"
#include "clk-factors.h"
#include "clk-periph.h"
#include "clk-sun8iw1.h"

#ifdef CONFIG_ARCH_SUN8IW3
#define SYS_CONFIG_PAT_EN       1
static struct sunxi_clk_pat_item clkpat_table[]=
{
	{"pll3","pll3pat"},
	{"pll_mipi","pll_mipipat"},
};
#endif
#ifdef CONFIG_ARCH_SUN8IW5
#define SYS_CONFIG_PAT_EN       1
static struct sunxi_clk_pat_item clkpat_table[]=
{
	{"pll_cpu","pll_cpupat"},
	{"pll_video","pll_videopat"},
	{"pll_ve","pll_vepat"},
	{"pll_gpu","pll_gpupat"},
	{"pll_hsic","pll_hsicpat"},
	{"pll_de","pll_depat"},
	{"pll_mipi","pll_mipipat"},
};
#endif

#ifdef CONFIG_ARCH_SUN8IW6
#define SYS_CONFIG_PAT_EN       1
static struct sunxi_clk_pat_item clkpat_table[]=
{
	{"pll_video0","pll_video0pat"},
};
#endif
#ifdef CONFIG_ARCH_SUN9IW1
#define SYS_CONFIG_PAT_EN       1
static struct sunxi_clk_pat_item clkpat_table[]=
{
	{"pll7","pll7pat"},
	{"pll8","pll8pat"},
	{"pll9","pll9pat"},
};
#endif
static void sunxi_clk_default_source(void)
{
	script_item_u   script_item;
	struct clk *clk = NULL;
	struct clk *parent_clk = NULL;
	unsigned int org_rate;
#if defined(CONFIG_SUNXI_CLK_AHB_FROM_PLL6) && !defined(CONFIG_ARCH_SUN9IW1)
	clk = clk_get(NULL,"ahb1");
	if(!clk || IS_ERR(clk))
		printk("Error not get clk ahb1\n");
	else
	{
		parent_clk = clk_get(NULL,"pll6ahb1");
		if(!parent_clk || IS_ERR(parent_clk))
		{
			clk_put(clk);
			printk("Error not get clk pll6ahb1");
		}
		else
		{
			printk("try to set ahb clk source to pll6ahb1\n");
			clk_set_parent(clk,parent_clk);
			printk("set ahb clk source to pll6ahb1\n");
			clk_put(parent_clk);
			clk_put(clk);
		}
	}
#else
	if((script_get_item("clock", "ahb1_parent", &script_item) == SCIRPT_ITEM_VALUE_TYPE_STR) && script_item.str)
	{
		clk = clk_get(NULL,"ahb1");
		if(!clk || IS_ERR(clk))
			printk("Error not get clk ahb1\n");
		else
		{
			parent_clk = clk_get(NULL,script_item.str);
			if(!parent_clk || IS_ERR(parent_clk))
			{
				clk_put(clk);
				printk("Error not get clk %s\n",script_item.str);
			}
			else
			{
				clk_set_parent(clk,parent_clk);
				printk("set ahb1 clk source to %s\n",script_item.str);
				clk_put(parent_clk);
				clk_put(clk);
			}
		}
	}
#endif

	if((script_get_item("clock", "apb1_parent", &script_item) == SCIRPT_ITEM_VALUE_TYPE_STR) && script_item.str)
	{
		clk = clk_get(NULL,"apb1");
		if(!clk || IS_ERR(clk))
			printk("Error not get clk apb1\n");
		else
		{
			parent_clk = clk_get(NULL,script_item.str);
			if(!parent_clk || IS_ERR(parent_clk))
			{
				clk_put(clk);
				printk("Error not get clk %s\n",script_item.str);
			}
			else
			{
				// switch to 1.2 M
				org_rate = clk_get_rate(clk);
				printk(KERN_INFO "set apb1 to low freq 1.2 Mhz\n");
				clk_set_rate(clk, 1200000);
				clk_set_parent(clk,parent_clk);
				printk("set apb1 clk source to %s\n",script_item.str);
				printk(KERN_INFO "recover apb1 to pre freq %d\n",org_rate);
				clk_set_rate(clk, org_rate);
				clk_put(parent_clk);
				clk_put(clk);
			}
		}
	}

	if((script_get_item("clock", "apb2_parent", &script_item) == SCIRPT_ITEM_VALUE_TYPE_STR) && script_item.str)
	{
		clk = clk_get(NULL,"apb2");
		if(!clk || IS_ERR(clk))
			printk("Error not get clk apb2\n");
		else
		{
			parent_clk = clk_get(NULL,script_item.str);
			if(!parent_clk || IS_ERR(parent_clk))
			{
				clk_put(clk);
				printk("Error not get clk %s\n",script_item.str);
			}
			else
			{
				// switch to 1.2 M
				org_rate = clk_get_rate(clk);
				printk(KERN_INFO "set apb2 to low freq 1.2 Mhz\n");
				clk_set_rate(clk, 1200000);
				clk_set_parent(clk,parent_clk);
				printk("set ahb clk source to %s\n",script_item.str);
				printk(KERN_INFO "recove apb2 to pre freq %d\n",org_rate);
				clk_set_rate(clk, org_rate);
				clk_put(parent_clk);
				clk_put(clk);
			}
		}
	}
}


#ifdef CONFIG_SUNXI_CLK_DEFAULT_INIT
#if defined (CONFIG_ARCH_SUN9IW1)
static char *init_clks[] ={"pll3","pll7","pll8","pll10","ahb1"};
#elif defined (CONFIG_ARCH_SUN8IW3) || defined (CONFIG_ARCH_SUN8IW5)
static char *init_clks[] = {"pll3", "pll4","pll6","pll8","pll9","pll10"};
#elif defined (CONFIG_ARCH_SUN8IW6)
static char *init_clks[] = {"pll_video1", "pll_ve","pll_periph","pll_gpu","pll_hsic","pll_de"};
#elif defined (CONFIG_ARCH_SUN8IW8)
static char *init_clks[] = {"pll_isp", "pll_video", "pll_ve","pll_periph0","pll_de"};
//add pll_isp for camera.
#elif defined (CONFIG_ARCH_SUN8IW7)
static char *init_clks[] = {"pll_video","pll_de","pll_ve"};
#else
static char *init_clks[] = {"pll3", "pll4","pll6","pll7","pll8","pll9","pll10"};
#endif
static int __init sunxi_clk_default_plls(void)
{
	int i;
	script_item_u script_item;
	struct clk *clk = NULL;


	for(i=0;i < ARRAY_SIZE(init_clks);i++)
	{
		if(script_get_item("clock", init_clks[i], &script_item) == SCIRPT_ITEM_VALUE_TYPE_INT)
		{
			if(script_item.val) {
					clk = clk_get(NULL,init_clks[i]);
					if(!clk || IS_ERR(clk))
					{
						clk = NULL;
						printk("Error not get clk %s\n",init_clks[i]);
						continue;
					}
					printk(KERN_INFO "script config %s to %d Mhz\n", init_clks[i],script_item.val);
					clk_set_rate(clk, script_item.val*1000000);
					clk_put(clk);
					clk=NULL;
			}
		}
		else
			printk(KERN_NOTICE "Not Found clk %s in script \n",init_clks[i]);
	}
	printk("sunxi_default_clk_init\n");

#if defined(CONFIG_PLL6AHB1_CLK_DFT_VALUE) && (CONFIG_PLL6AHB1_CLK_DFT_VALUE > 0)
	printk("try to set pll6ahb1 to %d\n",CONFIG_PLL6AHB1_CLK_DFT_VALUE);
#if defined (CONFIG_ARCH_SUN8IW6)
	clk = clk_get(NULL,"pll_periphahb1");
#else
	clk = clk_get(NULL,"pll6ahb1");
#endif
	if(!clk || IS_ERR(clk))
	{
		clk = NULL;
#if defined (CONFIG_ARCH_SUN8IW6)
		printk("Error not get clk pll_periphahb1\n");
#else
		printk("Error not get clk pll6ahb1\n");
#endif
		return 0;
	}
	clk_set_rate(clk,CONFIG_PLL6AHB1_CLK_DFT_VALUE);
	clk_put(clk);
#endif
	return 0;
}


static int __init sunxi_clk_default_devices(void)
{
	script_item_u   script_item;
	struct clk *clk = NULL;

#if defined(CONFIG_AHB1_CLK_DFT_VALUE) && (CONFIG_AHB1_CLK_DFT_VALUE > 0)
	printk("try to set ahb1 to %d\n",CONFIG_AHB1_CLK_DFT_VALUE);
	clk = clk_get(NULL,"ahb1");
	if(!clk || IS_ERR(clk))
	{
		clk = NULL;
		printk("Error not get clk ahb1\n");
	}
	else
	{
		clk_set_rate(clk,CONFIG_AHB1_CLK_DFT_VALUE);
		clk_put(clk);
	}
#endif
#if defined(CONFIG_APB1_CLK_DFT_VALUE) && (CONFIG_APB1_CLK_DFT_VALUE > 0)
	printk("try to set apb1 to %d\n",CONFIG_APB1_CLK_DFT_VALUE);
	clk = clk_get(NULL,"apb1");
	if(!clk || IS_ERR(clk))
	{
		clk = NULL;
		printk("Error not get clk apb1\n");
	}
	else
	{
		clk_set_rate(clk,CONFIG_APB1_CLK_DFT_VALUE);
		clk_put(clk);
	}
#endif
	if((script_get_item("clock", "apb1", &script_item) == SCIRPT_ITEM_VALUE_TYPE_INT) && script_item.val)
	{
		clk = clk_get(NULL,"apb1");
		if(!clk || IS_ERR(clk))
		{
			clk = NULL;
			printk("Error not get clk %s\n","apb1");
		}
		else
		{
			printk(KERN_INFO "script config apb1 to %d Mhz\n",script_item.val);
			clk_set_rate(clk, script_item.val*1000000);
			clk_put(clk);
			clk=NULL;
		}
	}
	if((script_get_item("clock", "apb2", &script_item) == SCIRPT_ITEM_VALUE_TYPE_INT) && script_item.val)
	{
		clk = clk_get(NULL,"apb2");
		if(!clk || IS_ERR(clk))
		{
			clk = NULL;
			printk("Error not get clk %s\n","apb2");
		}
		else
		{
			printk(KERN_INFO "script config apb2 to %d Mhz\n",script_item.val);
			clk_set_rate(clk, script_item.val*1000000);
			clk_put(clk);
			clk=NULL;
		}
	}
	return 0;

}
static int __init sunxi_clk_default_sdm(void)
{
#ifdef SYS_CONFIG_PAT_EN
	struct clk *clk = NULL;
	script_item_u   script_item;
	struct sunxi_clk_factors *factor=NULL;
	int i;
	unsigned long reg;
	for(i=0;i<sizeof(clkpat_table)/sizeof(struct sunxi_clk_pat_item);i++)
	{
		if((script_get_item("clock", clkpat_table[i].patname, &script_item) == SCIRPT_ITEM_VALUE_TYPE_INT))
		{
			clk = clk_get(NULL,clkpat_table[i].name);
			if(!IS_ERR_OR_NULL(clk))
			{
				factor = to_clk_factor(clk->hw);
				if(script_item.val)
				{
					factor->config->sdmwidth = 1;
					if(script_item.val != 1)    //avoid old format usage to only enable
						factor->config->sdmval=script_item.val;

					//sync with already enable PLLs
					if (clk->enable_count && __clk_is_enabled(clk))
					{
						reg = readl(factor->reg);
						writel(factor->config->sdmval, (void __iomem *)factor->config->sdmpat);
						reg = SET_BITS(factor->config->sdmshift, factor->config->sdmwidth, reg, 1);
						writel(reg, factor->reg);
					}
				}
				else
					factor->config->sdmwidth = 0;
				clk_put(clk);
				clk=NULL;
			}
		}
	}
#endif
	return 0;
}
static int __init sunx_clk_default_value(void)
{
	sunxi_clk_default_plls();
	sunxi_clk_default_sdm();
	sunxi_clk_default_source();
	sunxi_clk_default_devices();
	return 0;
}
arch_initcall(sunx_clk_default_value);
#endif
