/* arch/arm/mach-sunxi/pm/pm_debug_secure.c
 * Copyright (C) 2013-2014 allwinner.
 *
 *  By      : huangshr
 *  Date    : 2014-10-20
 */
#include <linux/ctype.h>
#include <linux/module.h>
#include "pm_debug.h"
#include "mem_hwspinlock.h"


#if defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN9IW1P1)
static volatile __r_prcm_pio_pad_hold 	*pm_secure_status_reg;
static volatile __r_prcm_pio_pad_hold 	*pm_secure_status_reg_pa;
#ifndef CONFIG_SUNXI_TRUSTZONE
static __r_prcm_pio_pad_hold 		pm_secure_status_reg_tmp;
static __r_prcm_pio_pad_hold 		pm_secure_status_reg_pa_tmp;
#endif //for define status reg tmp

void pm_secure_mem_status_init(void)
{
	pm_secure_status_reg	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG);
	pm_secure_status_reg_pa	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG_PA);
	hwspinlock_init(1);
}

void pm_secure_mem_status_init_nommu(void)
{
	pm_secure_status_reg	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG);
	pm_secure_status_reg_pa	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG_PA);
	hwspinlock_init(0);

}

void pm_secure_mem_status_clear(void)
{
#ifndef CONFIG_SUNXI_TRUSTZONE
	int i = 1;
	pm_secure_status_reg_tmp.dwval = (*pm_secure_status_reg).dwval;
	if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		while(i < STANDBY_STATUS_REG_NUM){
			pm_secure_status_reg_tmp.bits.reg_sel = i;
			pm_secure_status_reg_tmp.bits.data_wr = 0;
			(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
			pm_secure_status_reg_tmp.bits.wr_pulse = 0;
			(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
			pm_secure_status_reg_tmp.bits.wr_pulse = 1;
			(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
			pm_secure_status_reg_tmp.bits.wr_pulse = 0;
			(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
			i++;
	    	}
		hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
	}
#else
	call_firmware_op(set_standby_status,TEE_SMC_PLAFORM_OPERATION, TE_SMC_STANDBY_STATUS_CLEAR, (u32)pm_secure_status_reg, 0);
#endif
}

void pm_secure_mem_status_exit(void)
{
	return ;
}

void save_pm_secure_mem_status(volatile __u32 val)
{
#ifndef CONFIG_SUNXI_TRUSTZONE
	if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		pm_secure_status_reg_tmp.bits.reg_sel = 1;
		pm_secure_status_reg_tmp.bits.data_wr = val;
		(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
		pm_secure_status_reg_tmp.bits.wr_pulse = 0;
		(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
		pm_secure_status_reg_tmp.bits.wr_pulse = 1;
		(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
		pm_secure_status_reg_tmp.bits.wr_pulse = 0;
		(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
		hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
	}
//	asm volatile ("dsb");
//	asm volatile ("isb");
	return;
#else
	call_firmware_op(set_standby_status,TEE_SMC_PLAFORM_OPERATION, TE_SMC_STANDBY_STATUS_SET, (u32)pm_secure_status_reg, val);
	return;
#endif
}

__u32 get_pm_secure_mem_status(void)
{
	int val = 0;

#ifndef CONFIG_SUNXI_TRUSTZONE
	pm_secure_status_reg_tmp.bits.reg_sel = 1;
	if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
		pm_secure_status_reg_tmp.dwval = (*pm_secure_status_reg).dwval;
		hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
	}

	val = pm_secure_status_reg_tmp.bits.data_rd;
	return (val);
#else
	val = call_firmware_op(set_standby_status,TEE_SMC_PLAFORM_OPERATION, TE_SMC_STANDBY_STATUS_GET, (u32)pm_secure_status_reg, 1);
	return (val);
#endif
}

void show_pm_secure_mem_status(void)
{
	int i = 1;
	int val = 0;
	
#ifndef CONFIG_SUNXI_TRUSTZONE
	while(i < STANDBY_STATUS_REG_NUM) {
		pm_secure_status_reg_tmp.bits.reg_sel = i;
		if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
			(*pm_secure_status_reg).dwval = pm_secure_status_reg_tmp.dwval;
			pm_secure_status_reg_tmp.dwval = (*pm_secure_status_reg).dwval;
			hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
		}
		val = pm_secure_status_reg_tmp.bits.data_rd;
		printk("addr %x, value = %x. \n", (i), val);
		i++;
	}
#else
	while(i < STANDBY_STATUS_REG_NUM){
		val = call_firmware_op(set_standby_status,TEE_SMC_PLAFORM_OPERATION, TE_SMC_STANDBY_STATUS_GET, (u32)pm_secure_status_reg, i);
		printk("addr %x, value = %x. \n", (i), val);
		i++;
	}
#endif

}

void save_pm_secure_mem_status_nommu(volatile __u32 val)
{
#ifndef CONFIG_SUNXI_TRUSTZONE
	if (!hwspin_lock_timeout_nommu(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		pm_secure_status_reg_pa_tmp.bits.reg_sel = 1;
		pm_secure_status_reg_pa_tmp.bits.data_wr = val;
		(*pm_secure_status_reg_pa).dwval = pm_secure_status_reg_pa_tmp.dwval;
		pm_secure_status_reg_pa_tmp.bits.wr_pulse = 0;
		(*pm_secure_status_reg_pa).dwval = pm_secure_status_reg_pa_tmp.dwval;
		pm_secure_status_reg_pa_tmp.bits.wr_pulse = 1;
		(*pm_secure_status_reg_pa).dwval = pm_secure_status_reg_pa_tmp.dwval;
		pm_secure_status_reg_pa_tmp.bits.wr_pulse = 0;
		(*pm_secure_status_reg_pa).dwval = pm_secure_status_reg_pa_tmp.dwval;
		hwspin_unlock_nommu(MEM_RTC_REG_HWSPINLOCK);
	}

	return;
#else
	call_firmware_op(set_standby_status,TEE_SMC_PLAFORM_OPERATION, TE_SMC_STANDBY_STATUS_SET,  (u32)pm_secure_status_reg_pa, val);
#endif

}
#elif defined(CONFIG_ARCH_SUN8IW1P1) || \
	defined(CONFIG_ARCH_SUN8IW3P1) || \
	defined(CONFIG_ARCH_SUN8IW5P1) || \
	defined(CONFIG_ARCH_SUN8IW7P1) || \
	defined(CONFIG_ARCH_SUN8IW8P1) || \
	defined(CONFIG_ARCH_SUN8IW10P1) || \
	defined(CONFIG_ARCH_SUN50IW1P1)	

void pm_secure_mem_status_init(char *name)
{
	mem_status_init(name);
    	return;
}

void pm_secure_mem_status_init_nommu(void)
{
	mem_status_init_nommu();
	return  ;
}

void pm_secure_mem_status_clear(void)
{
	mem_status_clear();
	return;

}

void pm_secure_mem_status_exit(void)
{
	mem_status_exit();
	return ;
}

void save_pm_secure_mem_status(volatile __u32 val)
{
	save_mem_status(val);
	return;
}

__u32 get_pm_secure_mem_status(void)
{
	u32 val = 0;
	val = get_mem_status();
	return val;
}

void show_pm_secure_mem_status(void)
{
	show_mem_status();
}

#endif



