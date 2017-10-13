/* extended_standby.c
 *
 * Copyright (C) 2013-2014 allwinner.
 *
 *  By      : liming
 *  Version : v1.0
 *  Date    : 2013-4-17 09:08
 */
#include <linux/module.h>
#include <linux/power/aw_pm.h>
#include <linux/power/scenelock.h>
#include "pm.h"

#define AW_EXSTANDBY_DBG   1
#undef EXSTANDBY_DBG
#if(AW_EXSTANDBY_DBG)
#define EXSTANDBY_DBG(format,args...)   printk("[exstandby]"format,##args)
#else
#define EXSTANDBY_DBG(format,args...)   do{}while(0)
#endif

static DEFINE_SPINLOCK(data_lock);

static extended_standby_t temp_standby_data = {
	.id = 0,
};

static extended_standby_manager_t extended_standby_manager = {
	.pextended_standby = NULL,
	.event = 0,
	.wakeup_gpio_map = 0,
	.wakeup_gpio_group = 0,
};

#ifdef CONFIG_ARCH_SUN8IW3P1
static bool calculate_pll(int index, scene_extended_standby_t *standby_data)
{
	__u32 standby_rate;
	__u32 temp_standby_rata;
	__u32 dividend;
	__u32 divisor;

	switch (index) {
	case 0: /* PLL1 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n * standby_data->extended_standby_data.pll_factor[index].k;
		divisor = standby_data->extended_standby_data.pll_factor[index].m * standby_data->extended_standby_data.pll_factor[index].p;
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n * temp_standby_data.pll_factor[index].k;
		divisor = temp_standby_data.pll_factor[index].m * temp_standby_data.pll_factor[index].p;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 1: /* PLL2 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = standby_data->extended_standby_data.pll_factor[index].m * standby_data->extended_standby_data.pll_factor[index].p;
		standby_rate =  do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = temp_standby_data.pll_factor[index].m * temp_standby_data.pll_factor[index].p;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 4: /* PLL5 */
	case 8: /* MIPI */
		dividend = standby_data->extended_standby_data.pll_factor[index].n * standby_data->extended_standby_data.pll_factor[index].k;
		divisor = standby_data->extended_standby_data.pll_factor[index].m;
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n * temp_standby_data.pll_factor[index].k;
		divisor = temp_standby_data.pll_factor[index].m;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 5: /* PLL6 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n * standby_data->extended_standby_data.pll_factor[index].k;
		divisor = 2;
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n * temp_standby_data.pll_factor[index].k;
		divisor = 2;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 2: /* PLL3 */
	case 3: /* PLL4 */
	case 7: /* PLL8 */
	case 9: /* PLL9 */
	case 10: /* PLL10 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = standby_data->extended_standby_data.pll_factor[index].m;
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = temp_standby_data.pll_factor[index].m;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	default:
		return true;
	}
}

static bool calculate_bus(int index, scene_extended_standby_t *standby_data)
{
	switch (index) {
	case 0: /* APB2 */
		if(standby_data->extended_standby_data.bus_factor[index].src > temp_standby_data.bus_factor[index].src)
			return true;
		else
			return false;
	case 2: /* AHB1 */
		if(standby_data->extended_standby_data.bus_factor[index].src > temp_standby_data.bus_factor[index].src)
			return true;
		else
			return false;
		break;
	default:
		return true;
	}
}
#elif defined CONFIG_ARCH_SUN8IW5P1
static bool calculate_pll(int index, scene_extended_standby_t *standby_data)
{
	__u32 standby_rate;
	__u32 temp_standby_rata;
	__u32 dividend;
	__u32 divisor;

	switch (index) {
	case 1: /* PLL2 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = standby_data->extended_standby_data.pll_factor[index].m * standby_data->extended_standby_data.pll_factor[index].p;
		standby_rate =  do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = temp_standby_data.pll_factor[index].m * temp_standby_data.pll_factor[index].p;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 0: /* PLL1 */
	case 4: /* PLL5 */
	case 8: /* MIPI */
		dividend = standby_data->extended_standby_data.pll_factor[index].n * standby_data->extended_standby_data.pll_factor[index].k;
		divisor = standby_data->extended_standby_data.pll_factor[index].m;
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n * temp_standby_data.pll_factor[index].k;
		divisor = temp_standby_data.pll_factor[index].m;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 5: /* PLL6 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n * standby_data->extended_standby_data.pll_factor[index].k;
		divisor = 2;
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n * temp_standby_data.pll_factor[index].k;
		divisor = 2;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 2: /* PLL3 */
	case 3: /* PLL4 */
	case 7: /* PLL8 */
	case 9: /* PLL9 */
	case 10: /* PLL10 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = standby_data->extended_standby_data.pll_factor[index].m;
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = temp_standby_data.pll_factor[index].m;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	default:
		return true;
	}
}

static bool calculate_bus(int index, scene_extended_standby_t *standby_data)
{
	switch (index) {
	case 0: /* APB2 */
		if(standby_data->extended_standby_data.bus_factor[index].src > temp_standby_data.bus_factor[index].src)
			return true;
		else
			return false;
	case 2: /* AHB1 */
		if(standby_data->extended_standby_data.bus_factor[index].src > temp_standby_data.bus_factor[index].src)
			return true;
		else
			return false;
		break;
	default:
		return true;
	}
}
#elif defined CONFIG_ARCH_SUN9IW1P1
static bool calculate_pll(int index, scene_extended_standby_t *standby_data)
{
	__u32 standby_rate;
	__u32 temp_standby_rata;
	__u32 dividend;
	__u32 divisor;

	switch (index) {
	case 0: /* PLL1 PLL_C0CPUX=24M*N/P */
	case 1: /* PLL2 PLL_C1CPUX */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = standby_data->extended_standby_data.pll_factor[index].p;
		standby_rate =  do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = temp_standby_data.pll_factor[index].p;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 2: /* PLL3 PLL_Audio=24M*N/(input_div+1)/(output_div+1)/(P+1) */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = (standby_data->extended_standby_data.pll_factor[index].divi+1) * \
			(standby_data->extended_standby_data.pll_factor[index].divo+1) * (standby_data->extended_standby_data.pll_factor[index].p+1);
		standby_rate =  do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = (temp_standby_data.pll_factor[index].divi+1) * \
			(temp_standby_data.pll_factor[index].divo+1) * (temp_standby_data.pll_factor[index].p+1);
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 3:  /* PLL4  PLL_peri0=24M*N/(input_div+1)/(output_div+1) */
	case 4:  /* PLL5  PLL_VE    */
	case 5:  /* PLL6  PLL_DDR   */
	case 8:  /* PLL9  PLL_GPU   */
	case 9:  /* PLL10 PLL_DE    */
	case 10: /* pLL11 PLL_ISP   */
	case 11: /* PLL12 PLL_peri1 */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = (standby_data->extended_standby_data.pll_factor[index].divi+1) * (standby_data->extended_standby_data.pll_factor[index].divo+1);
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = (temp_standby_data.pll_factor[index].divi+1) * (temp_standby_data.pll_factor[index].divo+1);
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 6: /* PLL7 PLL_Video0=24M*N/(input_div+1) */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = (standby_data->extended_standby_data.pll_factor[index].divi+1);
		standby_rate = do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = (temp_standby_data.pll_factor[index].divi+1);
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	case 7: /* PLL8 PLL_Video1=24M*N/(input_div+1)/P */
		dividend = standby_data->extended_standby_data.pll_factor[index].n;
		divisor = (standby_data->extended_standby_data.pll_factor[index].divi+1) * standby_data->extended_standby_data.pll_factor[index].p;
		standby_rate =  do_div(dividend, divisor);

		dividend = temp_standby_data.pll_factor[index].n;
		divisor = (temp_standby_data.pll_factor[index].divi+1) * temp_standby_data.pll_factor[index].p;
		temp_standby_rata = do_div(dividend, divisor);
		if (standby_rate > temp_standby_rata)
			return true;
		else
			return false;
	default:
		return true;
	}
}

static bool calculate_bus(int index, scene_extended_standby_t *standby_data)
{
	switch (index) {
	case 0: /* APB2 */
		if(standby_data->extended_standby_data.bus_factor[index].src > temp_standby_data.bus_factor[index].src)
			return true;
		else
			return false;
	case 2: /* AHB1 */
		if(standby_data->extended_standby_data.bus_factor[index].src > temp_standby_data.bus_factor[index].src)
			return true;
		else
			return false;
		break;
	default:
		return true;
	}
}

#else
static bool calculate_pll(int index, scene_extended_standby_t *standby_data)
{
	return true;
}

static bool calculate_bus(int index, scene_extended_standby_t *standby_data)
{
	return true;
}
#endif


static int copy_extended_standby_data(scene_extended_standby_t *standby_data)
{
	int i = 0;

	if (!standby_data) {
		temp_standby_data.id = 0;
		temp_standby_data.pwr_dm_en = 0;
		temp_standby_data.osc_en = 0;
		temp_standby_data.init_pll_dis = 0;
		temp_standby_data.exit_pll_en = 0;
		temp_standby_data.pll_change = 0;
		temp_standby_data.bus_change = 0;
		memset(&temp_standby_data.pll_factor, 0, sizeof(temp_standby_data.pll_factor));
		memset(&temp_standby_data.bus_factor, 0, sizeof(temp_standby_data.bus_factor));
	} else {
		if ((0 != temp_standby_data.id) && (!((standby_data->extended_standby_data.id) & (temp_standby_data.id)))) {
			temp_standby_data.id |= standby_data->extended_standby_data.id;
			temp_standby_data.pwr_dm_en |= standby_data->extended_standby_data.pwr_dm_en;
			temp_standby_data.osc_en |= standby_data->extended_standby_data.osc_en;
			temp_standby_data.init_pll_dis &= standby_data->extended_standby_data.init_pll_dis;
			temp_standby_data.exit_pll_en |= standby_data->extended_standby_data.exit_pll_en;
			if (0 != standby_data->extended_standby_data.pll_change) {
				for (i=0; i<PLL_NUM; i++) {
					if (standby_data->extended_standby_data.pll_change & (0x1<<i)) {
						if (!(temp_standby_data.pll_change & (0x1<<i)))
							temp_standby_data.pll_factor[i] = standby_data->extended_standby_data.pll_factor[i];
						else if(calculate_pll(i, standby_data))
							temp_standby_data.pll_factor[i] = standby_data->extended_standby_data.pll_factor[i];
					}
				}
				temp_standby_data.pll_change |= standby_data->extended_standby_data.pll_change;
			}
			if (0 != standby_data->extended_standby_data.bus_change) {
				for (i=0; i<BUS_NUM; i++) {
					if (standby_data->extended_standby_data.bus_change & (0x1<<i)) {
						if (!(temp_standby_data.bus_change & (0x1<<i)))
							temp_standby_data.bus_factor[i] = standby_data->extended_standby_data.bus_factor[i];
						else if(calculate_bus(i, standby_data))
							temp_standby_data.bus_factor[i] = standby_data->extended_standby_data.bus_factor[i];
					}
				}
				temp_standby_data.bus_change |= standby_data->extended_standby_data.bus_change;
			}
		} else if ((0 == temp_standby_data.id)) {

			temp_standby_data.id = standby_data->extended_standby_data.id;
			temp_standby_data.pwr_dm_en = standby_data->extended_standby_data.pwr_dm_en;
			temp_standby_data.osc_en = standby_data->extended_standby_data.osc_en;
			temp_standby_data.init_pll_dis = standby_data->extended_standby_data.init_pll_dis;
			temp_standby_data.exit_pll_en = standby_data->extended_standby_data.exit_pll_en;
			temp_standby_data.pll_change = standby_data->extended_standby_data.pll_change;
			if (0 != standby_data->extended_standby_data.pll_change) {
				for (i=0; i<PLL_NUM; i++) {
					temp_standby_data.pll_factor[i] = standby_data->extended_standby_data.pll_factor[i];
				}
			} else
				memset(&temp_standby_data.pll_factor, 0, sizeof(temp_standby_data.pll_factor));

			temp_standby_data.bus_change = standby_data->extended_standby_data.bus_change;
			if (0 != standby_data->extended_standby_data.bus_change) {
				for (i=0; i<BUS_NUM; i++) {
					temp_standby_data.bus_factor[i] = standby_data->extended_standby_data.bus_factor[i];
				}
			} else
				memset(&temp_standby_data.bus_factor, 0, sizeof(temp_standby_data.bus_factor));
		}
	}
	return 0;
}
/**
 *	get_extended_standby_manager - get the extended_standby_manager pointer
 *
 *	Return	: if the extended_standby_manager is effective, return the extended_standby_manager pointer;
 *		  else return NULL;
 *	Notes	: you can check the configuration from the pointer.
 */
const extended_standby_manager_t *get_extended_standby_manager(void)
{
	unsigned long irqflags;
	extended_standby_manager_t *manager_data = NULL;

	spin_lock_irqsave(&data_lock, irqflags);
	manager_data = &extended_standby_manager;
	spin_unlock_irqrestore(&data_lock, irqflags);
	if ((NULL != manager_data) && (NULL != manager_data->pextended_standby))
		EXSTANDBY_DBG("leave %s : id 0x%lx\n", __func__, manager_data->pextended_standby->id);

	return manager_data;
}

/**
 *	set_extended_standby_manager - set the extended_standby_manager;
 *	manager@: the manager config.
 *
 *      return value: if the setting is correct, return true.
 *		      else return false;
 *      notes: the function will check the struct member: pextended_standby and event.
 *		if the setting is not proper, return false.
 */
bool set_extended_standby_manager(scene_extended_standby_t *local_standby)
{
	unsigned long irqflags;

	EXSTANDBY_DBG("enter %s\n", __func__);

	if (local_standby && 0 == local_standby->extended_standby_data.pwr_dm_en) {
	    return true;
	}

	if (!local_standby) {
		spin_lock_irqsave(&data_lock, irqflags);
		copy_extended_standby_data(NULL);
		extended_standby_manager.pextended_standby = NULL;
		spin_unlock_irqrestore(&data_lock, irqflags);
		return true;
	} else {
		spin_lock_irqsave(&data_lock, irqflags);
		copy_extended_standby_data(local_standby);
		extended_standby_manager.pextended_standby = &temp_standby_data;
		spin_unlock_irqrestore(&data_lock, irqflags);
	}

	if (NULL != extended_standby_manager.pextended_standby)
		EXSTANDBY_DBG("leave %s : id 0x%lx\n", __func__, extended_standby_manager.pextended_standby->id);
	return true;
}

/**
 *	extended_standby_enable_wakeup_src   - 	enable the wakeup src.
 *
 *	function:		the device driver care about the wakeup src.
 *				if the device driver do want the system be wakenup while in standby state.
 *				the device driver should use this function to enable corresponding intterupt.
 *	@src:			wakeup src.
 *	@para:			if wakeup src need para, be the para of wakeup src, 
 *				else ignored.
 *	notice:			1. for gpio intterupt, only access the enable bit, mean u need care about other config,
 *				such as: int mode, pull up or pull down resistance, etc.
 *				2. At a31, only gpio��pa, pb, pe, pg, pl, pm��int wakeup src is supported. 
*/
int extended_standby_enable_wakeup_src(cpu_wakeup_src_e src, int para)
{
	unsigned long irqflags;

	spin_lock_irqsave(&data_lock, irqflags);
	extended_standby_manager.event |= src;
	if (CPUS_GPIO_SRC & src) {
		if ( para >= AXP_PIN_BASE) {
			extended_standby_manager.wakeup_gpio_map |= (WAKEUP_GPIO_AXP((para - AXP_PIN_BASE)));
		} else if ( para >= SUNXI_PM_BASE) {
			extended_standby_manager.wakeup_gpio_map |= (WAKEUP_GPIO_PM((para - SUNXI_PM_BASE)));
		} else if ( para >= SUNXI_PL_BASE) {
			extended_standby_manager.wakeup_gpio_map |= (WAKEUP_GPIO_PL((para - SUNXI_PL_BASE)));
		} else if ( para >= SUNXI_PH_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('H'));
		} else if ( para >= SUNXI_PG_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('G'));
		} else if ( para >= SUNXI_PF_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('F'));
		} else if ( para >= SUNXI_PE_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('E'));
		} else if ( para >= SUNXI_PD_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('D'));
		} else if ( para >= SUNXI_PC_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('C'));
		} else if ( para >= SUNXI_PB_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('B'));
		} else if ( para >= SUNXI_PA_BASE) {
			extended_standby_manager.wakeup_gpio_group |= (WAKEUP_GPIO_GROUP('A'));
		} else {
			pr_info("cpux need care gpio %d. but, notice, currently, \
				cpux not support it.\n", para);
		}
	}
	spin_unlock_irqrestore(&data_lock, irqflags);
	EXSTANDBY_DBG("leave %s : event 0x%lx\n", __func__, extended_standby_manager.event);
	EXSTANDBY_DBG("leave %s : wakeup_gpio_map 0x%lx\n", __func__, extended_standby_manager.wakeup_gpio_map);
	EXSTANDBY_DBG("leave %s : wakeup_gpio_group 0x%lx\n", __func__, extended_standby_manager.wakeup_gpio_group);
	return 0;
}

/**
 *	extended_standby_disable_wakeup_src  - 	disable the wakeup src.
 *
 *	function:		if the device driver do not want the system be wakenup while in standby state again.
 *				the device driver should use this function to disable the corresponding intterupt.
 *
 *	@src:			wakeup src.
 *	@para:			if wakeup src need para, be the para of wakeup src, 
 *				else ignored.
 *	notice:			for gpio intterupt, only access the enable bit, mean u need care about other config,
 *				such as: int mode, pull up or pull down resistance, etc.
 */
int extended_standby_disable_wakeup_src(cpu_wakeup_src_e src, int para)
{
	unsigned long irqflags;
	spin_lock_irqsave(&data_lock, irqflags);
	extended_standby_manager.event &= (~src);
	if (CPUS_GPIO_SRC & src) {
		if ( para >= AXP_PIN_BASE) {
			extended_standby_manager.wakeup_gpio_map &= (~(WAKEUP_GPIO_AXP((para - AXP_PIN_BASE))));
		}else if ( para >= SUNXI_PM_BASE) {
			extended_standby_manager.wakeup_gpio_map &= (~(WAKEUP_GPIO_PM((para - SUNXI_PM_BASE))));
		}else if ( para >= SUNXI_PL_BASE) {
			extended_standby_manager.wakeup_gpio_map &= (~(WAKEUP_GPIO_PL((para - SUNXI_PL_BASE))));
		}else if ( para >= SUNXI_PH_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('H')));
		}else if ( para >= SUNXI_PG_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('G')));
		}else if ( para >= SUNXI_PF_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('F')));
		}else if ( para >= SUNXI_PE_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('E')));
		}else if ( para >= SUNXI_PD_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('D')));
		}else if ( para >= SUNXI_PC_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('C')));
		}else if ( para >= SUNXI_PB_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('B')));
		}else if ( para >= SUNXI_PA_BASE) {
			extended_standby_manager.wakeup_gpio_group &= (~(WAKEUP_GPIO_GROUP('A')));
		}else {
			pr_info("cpux need care gpio %d. but, notice, currently, \
				cpux not support it.\n", para);
		}
	}
	spin_unlock_irqrestore(&data_lock, irqflags);
	EXSTANDBY_DBG("leave %s : event 0x%lx\n", __func__, extended_standby_manager.event);
	EXSTANDBY_DBG("leave %s : wakeup_gpio_map 0x%lx\n", __func__, extended_standby_manager.wakeup_gpio_map);
	EXSTANDBY_DBG("leave %s : wakeup_gpio_group 0x%lx\n", __func__, extended_standby_manager.wakeup_gpio_group);
	return 0;
}

/**
 *	extended_standby_check_wakeup_state   -   to get the corresponding wakeup src intterupt state, enable or disable.
 *
 *	@src:			wakeup src.
 *	@para:			if wakeup src need para, be the para of wakeup src, 
 *				else ignored.
 *
 *	return value:		enable, 	return 1,
 *				disable,	return 2,
 *				error: 		return -1.
 */
int extended_standby_check_wakeup_state(cpu_wakeup_src_e src, int para)
{
	unsigned long irqflags;
	int ret = -1;
	spin_lock_irqsave(&data_lock, irqflags);
	if (extended_standby_manager.event & src)
		ret = 1;
	else
		ret = 2;
	spin_unlock_irqrestore(&data_lock, irqflags);

	return ret;
}

/**
 *	extended_standby_show_state  - 	show current standby state, for debug purpose.
 *
 *	function:		standby state including locked_scene, power_supply dependancy, the wakeup src.
 *
 *	return value:		succeed, return 0, else return -1.
 */
int extended_standby_show_state(void)
{
#ifdef CONFIG_ARCH_SUN8IW6P1
#else
	unsigned long irqflags;
	int i;

	standby_show_state();

	spin_lock_irqsave(&data_lock, irqflags);
	printk("wakeup_src 0x%lx\n", extended_standby_manager.event);
	printk("wakeup_gpio_map 0x%lx\n", extended_standby_manager.wakeup_gpio_map);
	printk("wakeup_gpio_group 0x%lx\n", extended_standby_manager.wakeup_gpio_group);
	if (NULL != extended_standby_manager.pextended_standby) {
		printk("extended_standby id = 0x%lx\n", extended_standby_manager.pextended_standby->id);
		if (0 != extended_standby_manager.pextended_standby->pll_change) {
			for (i=0; i<PLL_NUM; i++) {
#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || (defined CONFIG_ARCH_SUN8IW5P1)
				EXSTANDBY_DBG("pll%i: n=%d k=%d m=%d p=%d\n", i, \
						extended_standby_manager.pextended_standby->pll_factor[i].n, \
						extended_standby_manager.pextended_standby->pll_factor[i].k, \
						extended_standby_manager.pextended_standby->pll_factor[i].m, \
						extended_standby_manager.pextended_standby->pll_factor[i].p);
#elif (defined CONFIG_ARCH_SUN9IW1P1)
				EXSTANDBY_DBG("pll%i: n=%d p=%d divi=%d divo=%d\n", i, \
					extended_standby_manager.pextended_standby->pll_factor[i].n, \
					extended_standby_manager.pextended_standby->pll_factor[i].p, \
					extended_standby_manager.pextended_standby->pll_factor[i].divi, \
					extended_standby_manager.pextended_standby->pll_factor[i].divo);
#endif
			}
		}
		if (0 != extended_standby_manager.pextended_standby->bus_change) {
			for (i=0; i<BUS_NUM; i++) {
				EXSTANDBY_DBG("bus%i: src=%d pre_div=%d div_ratio=%d n=%d m=%d\n", i, \
						extended_standby_manager.pextended_standby->bus_factor[i].src, \
						extended_standby_manager.pextended_standby->bus_factor[i].pre_div, \
						extended_standby_manager.pextended_standby->bus_factor[i].div_ratio, \
						extended_standby_manager.pextended_standby->bus_factor[i].n, \
						extended_standby_manager.pextended_standby->bus_factor[i].m);
			}
		}
	}

	spin_unlock_irqrestore(&data_lock, irqflags);
#endif
	return 0;
}

