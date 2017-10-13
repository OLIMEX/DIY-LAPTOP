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
#include "../../../../kernel/power/power.h"

static int aw_ex_standby_debug_mask = 0;
#undef EXSTANDBY_DBG
#define EXSTANDBY_DBG(format,args...)  do { \
    if(aw_ex_standby_debug_mask){	    \
	printk("[exstandby]"format,##args); \
    }else{				    \
	do{}while(0);			    \
    }					    \
    }while(0)

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

static struct kobject *aw_ex_standby_kobj;

static bool calculate_pll(int index, scene_extended_standby_t *standby_data)
{
    __u32 standby_rate;
    __u32 tmp_standby_rate;
    __u32 dividend;
    __u32 divisor;

    switch (index) {
	case PM_PLL_C0:
	case PM_PLL_C1:
	case PM_PLL_AUDIO:
	case PM_PLL_VIDEO0:
	case PM_PLL_VE:
	case PM_PLL_DRAM:
	    printk("%s err: not ready. \n", __func__);
	    break;

	case PM_PLL_PERIPH:
	    dividend = standby_data->soc_pwr_dep.cpux_clk_state.pll_factor[index].factor1 * (standby_data->soc_pwr_dep.cpux_clk_state.pll_factor[index].factor2 + 1);
	    divisor = standby_data->soc_pwr_dep.cpux_clk_state.pll_factor[index].factor3 + 1;
	    standby_rate = do_div(dividend, divisor);

	    dividend = temp_standby_data.cpux_clk_state.pll_factor[index].factor1 * (temp_standby_data.cpux_clk_state.pll_factor[index].factor2 + 1);
	    divisor = temp_standby_data.cpux_clk_state.pll_factor[index].factor3 + 1;
	    tmp_standby_rate = do_div(dividend, divisor);
	    
	    if (standby_rate > tmp_standby_rate)
		return true;
	    else
		return false;

	case PM_PLL_GPU:
	case PM_PLL_HSIC:
	case PM_PLL_DE:
	case PM_PLL_VIDEO1:
	    printk("%s err: not ready. \n", __func__);
	    break;

	default:
	    printk("%s err: input para. \n", __func__);
	    break;
    }

    return false;
}

static bool calculate_bus(int index, scene_extended_standby_t *standby_data)
{
    if (BUS_NUM <= index){
	if(standby_data->soc_pwr_dep.cpux_clk_state.bus_factor[index].src > temp_standby_data.cpux_clk_state.bus_factor[index].src)
	    return true;
	else
	    return false;

    }else{
	printk("%s: input para err.\n", __func__);
    }

    return false;
}

/*
 * function: make dependency check, for make sure the pwr dependency is reasonable.
 * return 0: if reasonable.
 *        -1: if not reasonable.
 */
static int check_cfg(void)
{
    int ret = 0;
    int i = 0;

    //make sure bus parent is exist.
    if (0 != temp_standby_data.cpux_clk_state.bus_change) {
	for (i=0; i<BUS_NUM; i++) {
	    if((CLK_SRC_LOSC == temp_standby_data.cpux_clk_state.bus_factor[i].src) &&\
		    !(temp_standby_data.cpux_clk_state.osc_en & BITMAP(OSC_LOSC_BIT)) ){
		ret = -1;   
	    }
	    if((CLK_SRC_HOSC == temp_standby_data.cpux_clk_state.bus_factor[i].src) &&\
		    !(temp_standby_data.cpux_clk_state.osc_en & BITMAP(OSC_HOSC_BIT)) ){
		ret = -2;    
	    }
	    if((CLK_SRC_PLL6 == temp_standby_data.cpux_clk_state.bus_factor[i].src) &&\
		    !(temp_standby_data.cpux_clk_state.init_pll_dis & BITMAP(PM_PLL_PERIPH)) ){
		ret = -3;    
	    }
	}
    }
   
    //check hold_flag is reasonable.
    if(1 == temp_standby_data.soc_io_state.hold_flag && \
	    (temp_standby_data.soc_pwr_dm_state.state & temp_standby_data.soc_pwr_dm_state.sys_mask & BITMAP(VDD_SYS_BIT))){
	ret = -11; //when vdd_sys is ON, no need to set hold_flag; 
    }

    //make sure selfresh flag is reasonable
    if(0 == temp_standby_data.soc_dram_state.selfresh_flag){
	//when selfresh is disable, then VDD_SYS_BIT is needed
	if(!(temp_standby_data.soc_pwr_dm_state.state & temp_standby_data.soc_pwr_dm_state.sys_mask & BITMAP(VDD_SYS_BIT))){
	    ret = -21;
	}
    }

    if(-1 == ret){ 
	printk("func: %s, ret = %d. \n", __func__, ret);
	dump_stack();
    }

    return ret;
}

static int copy_extended_standby_data(scene_extended_standby_t *standby_data)
{
	int i = 0;
	int j = 0;
	int new_config_flag = 0;

	if (!standby_data) {
		temp_standby_data.id = 0;
		temp_standby_data.soc_pwr_dm_state.state = 0;
		temp_standby_data.soc_pwr_dm_state.sys_mask = 0;
		memset(&temp_standby_data.soc_pwr_dm_state.volt, 0, sizeof(temp_standby_data.soc_pwr_dm_state.volt));

		temp_standby_data.cpux_clk_state.osc_en = 0;
		temp_standby_data.cpux_clk_state.init_pll_dis = 0;
		temp_standby_data.cpux_clk_state.exit_pll_en = 0;
		temp_standby_data.cpux_clk_state.pll_change = 0;
		temp_standby_data.cpux_clk_state.bus_change = 0;
		memset(&temp_standby_data.cpux_clk_state.pll_factor, 0, sizeof(temp_standby_data.cpux_clk_state.pll_factor));
		memset(&temp_standby_data.cpux_clk_state.bus_factor, 0, sizeof(temp_standby_data.cpux_clk_state.bus_factor));
		
		temp_standby_data.soc_io_state.hold_flag=0;
		memset(&temp_standby_data.soc_io_state.io_state, 0, sizeof(temp_standby_data.soc_io_state.io_state));
		
		temp_standby_data.soc_dram_state.selfresh_flag = 0;
	} else {
		if ((0 != temp_standby_data.id) && (!((standby_data->soc_pwr_dep.id) & (temp_standby_data.id)))) {
			temp_standby_data.id |= standby_data->soc_pwr_dep.id;
			temp_standby_data.soc_pwr_dm_state.state |= standby_data->soc_pwr_dep.soc_pwr_dm_state.state;
			
			//only update voltage when new config has pwr on info;	
			//for stable reason, remain the higher voltage;
			if (0 != (temp_standby_data.soc_pwr_dm_state.state & temp_standby_data.soc_pwr_dm_state.sys_mask )) {
				for (i=0; i<VCC_MAX_INDEX; i++) {
					    if(standby_data->soc_pwr_dep.soc_pwr_dm_state.volt[i] > temp_standby_data.soc_pwr_dm_state.volt[i])
						temp_standby_data.soc_pwr_dm_state.volt[i] = standby_data->soc_pwr_dep.soc_pwr_dm_state.volt[i];
				}
			}

			temp_standby_data.cpux_clk_state.osc_en |= standby_data->soc_pwr_dep.cpux_clk_state.osc_en;
			//0 is disable, enable have higher priority.
			temp_standby_data.cpux_clk_state.init_pll_dis |= standby_data->soc_pwr_dep.cpux_clk_state.init_pll_dis;
			temp_standby_data.cpux_clk_state.exit_pll_en |= standby_data->soc_pwr_dep.cpux_clk_state.exit_pll_en;
			if (0 != standby_data->soc_pwr_dep.cpux_clk_state.pll_change) {
				for (i=0; i<PLL_NUM; i++) {
					if (standby_data->soc_pwr_dep.cpux_clk_state.pll_change & (0x1<<i)) {
						if (!(temp_standby_data.cpux_clk_state.pll_change & (0x1<<i)) || calculate_pll(i, standby_data))
							temp_standby_data.cpux_clk_state.pll_factor[i] = standby_data->soc_pwr_dep.cpux_clk_state.pll_factor[i];
					}
				}
				temp_standby_data.cpux_clk_state.pll_change |= standby_data->soc_pwr_dep.cpux_clk_state.pll_change;
			}
			if (0 != standby_data->soc_pwr_dep.cpux_clk_state.bus_change) {
				for (i=0; i<BUS_NUM; i++) {
					if (standby_data->soc_pwr_dep.cpux_clk_state.bus_change & (0x1<<i)) {
						if (!(temp_standby_data.cpux_clk_state.bus_change & (0x1<<i)) || calculate_bus(i, standby_data))
							temp_standby_data.cpux_clk_state.bus_factor[i] = standby_data->soc_pwr_dep.cpux_clk_state.bus_factor[i];
					}
				}
				temp_standby_data.cpux_clk_state.bus_change |= standby_data->soc_pwr_dep.cpux_clk_state.bus_change;
			}
		
			//unhold_flag has higher level priority.
			temp_standby_data.soc_io_state.hold_flag &= standby_data->soc_pwr_dep.soc_io_state.hold_flag;
			
			//notice: how to merge io config?
			//not supprt add io config, this code just for checking io config.
			for (j=0; j<IO_NUM; j++) { //new added
			    if(0 == standby_data->soc_pwr_dep.soc_io_state.io_state[j].paddr){
				printk("io config is not in effect.\n");
				continue;
			    }
			    for (i=0; i<IO_NUM; i++) { //orig configed

				//when io has not been initialized.
				if(0 == temp_standby_data.soc_io_state.io_state[i].paddr){
				    temp_standby_data.soc_io_state.io_state[i].paddr = standby_data->soc_pwr_dep.soc_io_state.io_state[j].paddr;
				    temp_standby_data.soc_io_state.io_state[i].value_mask = standby_data->soc_pwr_dep.soc_io_state.io_state[j].value_mask;
				    temp_standby_data.soc_io_state.io_state[i].value = standby_data->soc_pwr_dep.soc_io_state.io_state[j].value;
				    break;
				}else{
				    if(temp_standby_data.soc_io_state.io_state[i].paddr == standby_data->soc_pwr_dep.soc_io_state.io_state[j].paddr && \
					    temp_standby_data.soc_io_state.io_state[i].value_mask == standby_data->soc_pwr_dep.soc_io_state.io_state[j].value_mask ){

					if(temp_standby_data.soc_io_state.io_state[i].value != standby_data->soc_pwr_dep.soc_io_state.io_state[j].value){
					    printk("NOTICE: io config conflict.\n");
					    dump_stack();
					}else{
					    printk("NOTICE: io config is the same. \n");
					    new_config_flag = 0;
					    break;
					}
				    }else{
					//new config?
					new_config_flag = 1;
					continue;
				    }

				}

			    }
			    if(1 == new_config_flag)
				printk("NOTICE: exist new io config. \n");
			}

			//un_selfresh_flag has higher level priority.
			temp_standby_data.soc_dram_state.selfresh_flag &= standby_data->soc_pwr_dep.soc_dram_state.selfresh_flag;
		} else if ((0 == temp_standby_data.id)) {
			//update sys_mask: when scene_unlock happend or scene_lock cnt > 0
#if defined(CONFIG_AW_AXP)
			temp_standby_data.soc_pwr_dm_state.sys_mask = get_sys_pwr_dm_mask();
#endif
			temp_standby_data.id = standby_data->soc_pwr_dep.id;
			temp_standby_data.soc_pwr_dm_state.state = standby_data->soc_pwr_dep.soc_pwr_dm_state.state;
			if (0 != (temp_standby_data.soc_pwr_dm_state.state&temp_standby_data.soc_pwr_dm_state.sys_mask)) {
				for (i=0; i<VCC_MAX_INDEX; i++) {
					temp_standby_data.soc_pwr_dm_state.volt[i] = standby_data->soc_pwr_dep.soc_pwr_dm_state.volt[i];
				}
			} else
				memset(&temp_standby_data.soc_pwr_dm_state.volt, 0, sizeof(temp_standby_data.soc_pwr_dm_state.volt));
			
			temp_standby_data.cpux_clk_state.osc_en = standby_data->soc_pwr_dep.cpux_clk_state.osc_en;
			temp_standby_data.cpux_clk_state.init_pll_dis = standby_data->soc_pwr_dep.cpux_clk_state.init_pll_dis;
			temp_standby_data.cpux_clk_state.exit_pll_en = standby_data->soc_pwr_dep.cpux_clk_state.exit_pll_en;
			temp_standby_data.cpux_clk_state.pll_change = standby_data->soc_pwr_dep.cpux_clk_state.pll_change;
			if (0 != standby_data->soc_pwr_dep.cpux_clk_state.pll_change) {
				for (i=0; i<PLL_NUM; i++) {
					temp_standby_data.cpux_clk_state.pll_factor[i] = standby_data->soc_pwr_dep.cpux_clk_state.pll_factor[i];
				}
			} else
				memset(&temp_standby_data.cpux_clk_state.pll_factor, 0, sizeof(temp_standby_data.cpux_clk_state.pll_factor));

			temp_standby_data.cpux_clk_state.bus_change = standby_data->soc_pwr_dep.cpux_clk_state.bus_change;
			if (0 != standby_data->soc_pwr_dep.cpux_clk_state.bus_change) {
			    for (i=0; i<BUS_NUM; i++) {
				temp_standby_data.cpux_clk_state.bus_factor[i] = standby_data->soc_pwr_dep.cpux_clk_state.bus_factor[i];
			    }
			} else
			    memset(&temp_standby_data.cpux_clk_state.bus_factor, 0, sizeof(temp_standby_data.cpux_clk_state.bus_factor));

			temp_standby_data.soc_io_state.hold_flag = standby_data->soc_pwr_dep.soc_io_state.hold_flag;
			for (i=0; i<IO_NUM; i++) {
			    temp_standby_data.soc_io_state.io_state[i] = standby_data->soc_pwr_dep.soc_io_state.io_state[i];
			}


			temp_standby_data.soc_dram_state.selfresh_flag = standby_data->soc_pwr_dep.soc_dram_state.selfresh_flag;
		}
	}

	return check_cfg();
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
	if ((NULL != manager_data) && (NULL != manager_data->pextended_standby)){
#if defined(CONFIG_AW_AXP)
	    //update sys_mask
	    manager_data->pextended_standby->soc_pwr_dm_state.sys_mask = get_sys_pwr_dm_mask();
#endif
	    EXSTANDBY_DBG("leave %s : id 0x%x\n", __func__, manager_data->pextended_standby->id);
	}
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

	if (local_standby && 0 == local_standby->soc_pwr_dep.soc_pwr_dm_state.state) {
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
		EXSTANDBY_DBG("leave %s : id 0x%x\n", __func__, extended_standby_manager.pextended_standby->id);
	return true;
}

/**
 *	extended_standby_set_pmu_id   -     set pmu_id for suspend modules.
 *
 *	@num:	pmu serial number;
 *	@pmu_id: corresponding pmu_id;
 */
int extended_standby_set_pmu_id(unsigned int num, unsigned int pmu_id)
{
    unsigned int tmp;

    if(num > 4 || num < 1)
	return -1;

    tmp = temp_standby_data.pmu_id;
    tmp &= ~(0xff << ((num - 1)*8));
    tmp |= (pmu_id << ((num - 1)*8));
    temp_standby_data.pmu_id = tmp;

    return 0;
}

/**
 *	extended_standby_get_pmu_id   -     get specific pmu_id for suspend modules.
 *
 *	@num:	pmu serial number;
 */
int extended_standby_get_pmu_id(unsigned int num)
{
    unsigned int tmp;

    if(num > 4 || num < 1)
	return -1;

    tmp = temp_standby_data.pmu_id;
    tmp >>= ((num -1)*8);
    tmp &= (0xff);
    
    return tmp;
}

/**
 *	extended_standby_store_dram_crc_paras   -     store dram_crc_paras for suspend modules.
 *
 *	@num:	pmu serial number;
 *	@dram_crc_paras: corresponding dram_crc_paras;
 */
static ssize_t extended_standby_dram_crc_paras_store(struct device *dev, 
	struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned int dram_crc_en;
    unsigned int dram_crc_start;
    unsigned int dram_crc_len;

    sscanf(buf, "%x %x %x\n", &dram_crc_en, &dram_crc_start, &dram_crc_len);
    if ((dram_crc_en != 0) && (dram_crc_en != 1)) {
	printk(KERN_ERR "invalid paras for dram_crc: [%x] [%x] [%x] \n", \
		dram_crc_en, dram_crc_start, dram_crc_len);
	return size;
    }

    temp_standby_data.soc_dram_state.crc_en = dram_crc_en;
    temp_standby_data.soc_dram_state.crc_start = dram_crc_start;
    temp_standby_data.soc_dram_state.crc_len = dram_crc_len;

    return size;
}

/**
 *	extended_standby_show_dram_crc_paras   -     show specific dram_crc_paras for suspend modules.
 *
 *	@num:	pmu serial number;
 */
ssize_t extended_standby_dram_crc_paras_show(struct device *dev, 
	struct device_attribute *attr, char *buf)
{
    char *s = buf;

    s += sprintf(buf, "dram_crc_paras: enable, start, len == [%x] [%x] [%x]\n", \
	    temp_standby_data.soc_dram_state.crc_en, \
	    temp_standby_data.soc_dram_state.crc_start, \
	    temp_standby_data.soc_dram_state.crc_len);
    
    return (s-buf);
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
 *
 *	function:		standby state including locked_scene, power_supply dependancy, the wakeup src.
 *
 *	return value:		succeed, return 0, else return -1.
 */
int extended_standby_show_state(void)
{
	unsigned long irqflags;
	int i = 0;
	unsigned int pwr_on_bitmap  = 0;
	unsigned int pwr_off_bitmap  = 0;

	standby_show_state();

	spin_lock_irqsave(&data_lock, irqflags);
	printk(KERN_INFO "dynamic config wakeup_src: 0x%16lx\n", extended_standby_manager.event);
	parse_wakeup_event(NULL, 0, extended_standby_manager.event, CPUS_ID);
	printk(KERN_INFO "wakeup_gpio_map 0x%16lx\n", extended_standby_manager.wakeup_gpio_map);
	parse_wakeup_gpio_map(NULL, 0, extended_standby_manager.wakeup_gpio_map);
	printk(KERN_INFO "wakeup_gpio_group 0x%16lx\n", extended_standby_manager.wakeup_gpio_group);
	parse_wakeup_gpio_group_map(NULL, 0, extended_standby_manager.wakeup_gpio_group);
	if (NULL != extended_standby_manager.pextended_standby) {
		printk(KERN_INFO "extended_standby id = 0x%16x\n", extended_standby_manager.pextended_standby->id);
		printk(KERN_INFO "extended_standby pmu_id = 0x%16x\n", extended_standby_manager.pextended_standby->pmu_id);
		printk(KERN_INFO "extended_standby soc_id = 0x%16x\n", extended_standby_manager.pextended_standby->soc_id);
		printk(KERN_INFO "extended_standby pwr dep as follow: \n");
	
		printk(KERN_INFO "pwr dm state as follow: \n");
		printk(KERN_INFO "\tpwr dm state = 0x%8x. \n", extended_standby_manager.pextended_standby->soc_pwr_dm_state.state);
		parse_pwr_dm_map(NULL, 0, extended_standby_manager.pextended_standby->soc_pwr_dm_state.state); 
		printk(KERN_INFO "\tpwr dm sys mask = 0x%8x. \n", extended_standby_manager.pextended_standby->soc_pwr_dm_state.sys_mask);
		parse_pwr_dm_map(NULL, 0, extended_standby_manager.pextended_standby->soc_pwr_dm_state.sys_mask); 

		pwr_on_bitmap = extended_standby_manager.pextended_standby->soc_pwr_dm_state.sys_mask & extended_standby_manager.pextended_standby->soc_pwr_dm_state.state;	
		printk(KERN_INFO "\tpwr on = 0x%x. \n", pwr_on_bitmap);
		parse_pwr_dm_map(NULL, 0, pwr_on_bitmap); 

		pwr_off_bitmap = (~extended_standby_manager.pextended_standby->soc_pwr_dm_state.sys_mask) | extended_standby_manager.pextended_standby->soc_pwr_dm_state.state;	
		printk(KERN_INFO "\tpwr off = 0x%x. \n", pwr_off_bitmap);
		parse_pwr_dm_map(NULL, 0, (~pwr_off_bitmap)); 
		
		EXSTANDBY_DBG("\tpwr on volt which need adjusted: \n");
		if (0 != (extended_standby_manager.pextended_standby->soc_pwr_dm_state.state&\
			    extended_standby_manager.pextended_standby->soc_pwr_dm_state.sys_mask)) {
				for (i=0; i<VCC_MAX_INDEX; i++) {
					if(0 != extended_standby_manager.pextended_standby->soc_pwr_dm_state.volt[i]){
					    printk(KERN_INFO "index = %d, volt[]= %d. \n", i, extended_standby_manager.pextended_standby->soc_pwr_dm_state.volt[i]);	
					}
				}
		} 

		EXSTANDBY_DBG("cpux clk state as follow: \n");
		EXSTANDBY_DBG("    cpux osc en: 0x%8x. \n", extended_standby_manager.pextended_standby->cpux_clk_state.osc_en);
		EXSTANDBY_DBG("    cpux pll init disabled config: 0x%8x. \n", extended_standby_manager.pextended_standby->cpux_clk_state.init_pll_dis);
		EXSTANDBY_DBG("    cpux pll exit enable config: 0x%8x. \n", extended_standby_manager.pextended_standby->cpux_clk_state.exit_pll_en);

		if (0 != extended_standby_manager.pextended_standby->cpux_clk_state.pll_change) {
			for (i=0; i<PLL_NUM; i++) {
				EXSTANDBY_DBG("pll%i: factor1=%d factor2=%d factor3=%d factor4=%d\n", i, \
						extended_standby_manager.pextended_standby->cpux_clk_state.pll_factor[i].factor1, \
						extended_standby_manager.pextended_standby->cpux_clk_state.pll_factor[i].factor2, \
						extended_standby_manager.pextended_standby->cpux_clk_state.pll_factor[i].factor3, \
						extended_standby_manager.pextended_standby->cpux_clk_state.pll_factor[i].factor4);
			}
		}else{
		    EXSTANDBY_DBG("pll_change == 0: no pll need change. \n");
		}

		if (0 != extended_standby_manager.pextended_standby->cpux_clk_state.bus_change) {
			for (i=0; i<BUS_NUM; i++) {
				EXSTANDBY_DBG("bus%i: src=%d pre_div=%d div_ratio=%d n=%d m=%d\n", i, \
						extended_standby_manager.pextended_standby->cpux_clk_state.bus_factor[i].src, \
						extended_standby_manager.pextended_standby->cpux_clk_state.bus_factor[i].pre_div, \
						extended_standby_manager.pextended_standby->cpux_clk_state.bus_factor[i].div_ratio, \
						extended_standby_manager.pextended_standby->cpux_clk_state.bus_factor[i].n, \
						extended_standby_manager.pextended_standby->cpux_clk_state.bus_factor[i].m);
			}
		}else{
		    EXSTANDBY_DBG("bus_change == 0: no bus need change. \n");
		}
		
		EXSTANDBY_DBG("cpux io state as follow: \n");
		EXSTANDBY_DBG("     hold_flag = %d. \n", extended_standby_manager.pextended_standby->soc_io_state.hold_flag);
		
		for (i=0; i<IO_NUM; i++){
		    if(0 != extended_standby_manager.pextended_standby->soc_io_state.io_state[i].paddr){
			printk(KERN_INFO "    count %4d io config: addr 0x%x, value_mask 0x%8x, value 0x%8x. \n", i,			   \
				extended_standby_manager.pextended_standby->soc_io_state.io_state[i].paddr,		    \
				extended_standby_manager.pextended_standby->soc_io_state.io_state[i].value_mask,		    \
				extended_standby_manager.pextended_standby->soc_io_state.io_state[i].value);
		    }
		}
		
		EXSTANDBY_DBG("soc dram state as follow: \n");
		EXSTANDBY_DBG("    selfresh_flag = %d. \n", extended_standby_manager.pextended_standby->soc_dram_state.selfresh_flag);

	}

	spin_unlock_irqrestore(&data_lock, irqflags);

	return 0;
}

static DEVICE_ATTR(dram_crc_paras, S_IRUGO|S_IWUSR|S_IWGRP,
		extended_standby_dram_crc_paras_show, extended_standby_dram_crc_paras_store);

static struct attribute * g[] = {
	&dev_attr_dram_crc_paras.attr,
	NULL,
};
static struct attribute_group attr_group = {
	.attrs = g,
};


static int __init aw_ex_standby_init(void)
{
    int error = 0;

    aw_ex_standby_kobj = kobject_create_and_add("aw_ex_standby", power_kobj);
    if (!aw_ex_standby_kobj)
	return -ENOMEM;
    error = sysfs_create_group(aw_ex_standby_kobj, &attr_group);

    return error ? error : 0;
}

/*
*********************************************************************************************************
*                           aw_ex_standby_exit
*
*Description: exit ex_standby sub-system on platform;
*
*Arguments  : none
*
*Return     : none
*
*Notes      :
*
*********************************************************************************************************
*/
static void __exit aw_ex_standby_exit(void)
{
    printk(KERN_INFO "aw_ex_standby_exit!\n");

    return;
}

module_param_named(aw_ex_standby_debug_mask, aw_ex_standby_debug_mask, int, S_IRUGO | S_IWUSR);
module_init(aw_ex_standby_init);
module_exit(aw_ex_standby_exit);

