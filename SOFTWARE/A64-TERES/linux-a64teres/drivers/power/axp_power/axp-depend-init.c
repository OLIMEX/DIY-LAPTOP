/* extended_standby.c
 *
 * Copyright (C) 2013-2014 allwinner.
 *
 *  By      : liming
 *  Version : v1.0
 *  Date    : 2013-4-17 09:08
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/regulator/consumer.h>
#include <linux/power/axp_depend.h>

static u32 debug_mask = 0;
/*
 *  a33_sys_mask: +
 *  a80_sys_mask: *
 *  a83_sys_mask: x: 0x7bd5;
 *
 * bit0-bit7
 * bit7		bit6	    bit5	bit4	    bit3	bit2	    bit1	bit0
 * vdd_drampll	vdd_cpus    vdd_vpu	vdd_sys	    vdd_gpu	vcc_dram    vdd_cpub	vdd_cpua
 * +		+           +		+           +
 * *		*	    *           *	    *
 * x		x	    N		x	    N		x	    N		x
 * bit8-bit15
 * bit7		bit6	    bit5	bit4	    bit3	bit2	    bit1	bit0
 * vcc-lpddr	vcc_pll	    vcc_ldoin	vcc_cpvdd   vcc_io	vcc_pm	    vcc_pl	vdd_adc
 *  +
 *              *           *
 * 		x	    x		x	    x		N	    x		x
 */

//pwr_dm_bitmap -> pwr_dm_name
const bitmap_name_mapping_t pwr_dm_bitmap_name_mapping[] = {
    {VDD_CPUA_BIT   ,    "vdd-cpua"         },
    {VDD_CPUB_BIT   ,    "vdd-cpub"         },
    {VCC_DRAM_BIT   ,    "vcc-dram"         },
    {VDD_GPU_BIT    ,    "vdd-gpu"          },
    {VDD_SYS_BIT    ,    "vdd-sys"          },
    {VDD_VPU_BIT    ,    "vdd-vpu"          },
    {VDD_CPUS_BIT   ,    "vdd-cpus"         },
    {VDD_DRAMPLL_BIT,    "vdd-drampll"      },
    {VCC_ADC_BIT    ,    "vcc-adc"          },
    {VCC_PL_BIT     ,    "vcc-pl"           },
    {VCC_PM_BIT     ,    "vcc-pm"           },
    {VCC_IO_BIT     ,    "vcc-io"           },
    {VCC_CPVDD_BIT ,     "vcc-cpvdd"        },
    {VCC_LDOIN_BIT  ,    "vcc-ldoin"        },
    {VCC_PLL_BIT    ,    "vcc-pll"          },
    {VCC_LPDDR_BIT  ,    "vcc-lpddr"        },
    {VDD_TEST_BIT  ,     "vdd-test"        },
};
s32 pwr_dm_bitmap_name_mapping_cnt = sizeof(pwr_dm_bitmap_name_mapping)/sizeof(pwr_dm_bitmap_name_mapping[0]);

static DEFINE_SPINLOCK(data_lock);
/*
 * sys_pwr_dm_mask:
 *	function: is used for mark: whether a pwr_dm belongs to sys_pwr_dm or not.
 * */
static u32 sys_pwr_dm_mask = 0;

static u32 power_regu_tree[VCC_MAX_INDEX] = {0};

/*
 * function: init sys_pwr_dm_mask.
 * input: mask, get from sysconfig or other modules.
 * return: null
 *
 */
void set_sys_pwr_dm_mask(u32 bitmap, u32 enable)
{
	unsigned long irqflags;

	spin_lock_irqsave(&data_lock, irqflags);
	if (enable)
		sys_pwr_dm_mask |= (0x1 << bitmap);
	else
		sys_pwr_dm_mask &= ~(0x1 << bitmap);
	spin_unlock_irqrestore(&data_lock, irqflags);

	if (unlikely(debug_mask != 0))
		printk("%s: sys_pwr_dm_mask = 0x%x\n", __func__, sys_pwr_dm_mask);

	return;
}

/*
 * function: get the sys_pwr_dm_mask config.
 * input: void;
 * return: current sys_pwr_dm_mask.
 *
 * */
u32 get_sys_pwr_dm_mask(void)
{
	unsigned long irqflags;
	u32 ret = 0;

	spin_lock_irqsave(&data_lock, irqflags);
	ret = sys_pwr_dm_mask;
	spin_unlock_irqrestore(&data_lock, irqflags);

	return ret;
}

void set_pwr_regu_tree(u32 value, u32 bitmap)
{
	unsigned long irqflags;

	spin_lock_irqsave(&data_lock, irqflags);
	power_regu_tree[bitmap] = value;
	spin_unlock_irqrestore(&data_lock, irqflags);

	if (unlikely(debug_mask != 0))
		printk("%s: power_regu_tree[%d] = 0x%x\n", __func__, bitmap, value);

	return;
}

void get_pwr_regu_tree(unsigned int *p)
{
	memcpy((void *)p, (void *)power_regu_tree, sizeof(power_regu_tree));
}

s32 axp_check_sys_id(const char *supply_id)
{
	s32 i = 0;

	for(i=0; i < VCC_MAX_INDEX; i++) {
		if (strcmp(pwr_dm_bitmap_name_mapping[i].id_name, supply_id) == 0) {
			return i;
		}
	}
	return -1;
}

char *axp_get_sys_id(u32 bitmap)
{
	if ((bitmap < 0) || (bitmap >= VCC_MAX_INDEX))
		return NULL;
	return (char *)&(pwr_dm_bitmap_name_mapping[bitmap].id_name);
}

static s32 axp81x_regu_dependence(const char *ldo_name)
{
	s32 axp81x_dependence = 0;

	if (strcmp("axp81x_dcdc1", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DCDC1;
	} else if (strcmp("axp81x_dcdc2", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DCDC2;
	} else if (strcmp("axp81x_dcdc3", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DCDC3;
	}else if (strcmp("axp81x_dcdc4", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DCDC4;
	}else if (strcmp("axp81x_dcdc5", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DCDC5;
	}else if (strcmp("axp81x_dcdc6", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DCDC6;
	}else if (strcmp("axp81x_dcdc7", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DCDC7;
	}else if (strcmp("axp81x_aldo1", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_ALDO1;
	}else if (strcmp("axp81x_aldo2", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_ALDO2;
	}else if (strcmp("axp81x_aldo3", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_ALDO3;
	}else if (strcmp("axp81x_dldo1", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DLDO1;
	}else if (strcmp("axp81x_dldo2", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DLDO2;
	}else if (strcmp("axp81x_dldo3", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DLDO3;
	}else if (strcmp("axp81x_dldo4", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DLDO4;
	}else if (strcmp("axp81x_eldo1", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_ELDO1;
	}else if (strcmp("axp81x_eldo2", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_ELDO2;
	}else if (strcmp("axp81x_eldo3", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_ELDO3;
	}else if (strcmp("axp81x_fldo1", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_FLDO1;
	}else if (strcmp("axp81x_fldo2", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_FLDO2;
	}else if (strcmp("axp81x_gpio0ldo", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_LDOIO0;
	}else if (strcmp("axp81x_gpio1ldo", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_LDOIO1;
	}else if (strcmp("axp81x_dc1sw", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_DC1SW;
	}else if (strcmp("axp81x_rtc", ldo_name) == 0) {
		axp81x_dependence |= AXP803_813_RTC;
	}else {
		return -1;
	}
	axp81x_dependence |= (0 << 30);
	return axp81x_dependence;
}

s32 get_ldo_dependence(const char *ldo_name, s32 count)
{
	s32 ret = -1;

	if((strncmp("axp81x", ldo_name, 6) == 0)){
		ret = axp81x_regu_dependence(ldo_name);
	} else {
		return 0;
	}
	if (ret < 0) {
		printk(KERN_ERR "%s: get regu dependence failed\n", __func__);
		return -1;
	} else {
		set_pwr_regu_tree(ret, count);
	}
	return 0;
}

