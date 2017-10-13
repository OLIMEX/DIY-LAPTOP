#include "pm_i.h"


static volatile __r_prcm_pio_pad_hold *status_reg;
static __r_prcm_pio_pad_hold status_reg_tmp;
static volatile __r_prcm_pio_pad_hold *status_reg_pa; 
static __r_prcm_pio_pad_hold status_reg_pa_tmp; 
void mem_status_init(void)
{
	status_reg	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG);
	status_reg_pa	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG_PA);

	//init spinlock for sync
	hwspinlock_init(1);
}

void mem_status_init_nommu(void)
{
	status_reg	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG);
	status_reg_pa	= (volatile __r_prcm_pio_pad_hold *)(STANDBY_STATUS_REG_PA);

	//init spinlock for sync
	hwspinlock_init(0);
}

void mem_status_clear(void)
{
	int i = 1;

	status_reg_tmp.dwval = (*status_reg).dwval;
	if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		while(i < STANDBY_STATUS_REG_NUM){
			status_reg_tmp.bits.reg_sel = i;
			status_reg_tmp.bits.data_wr = 0;
			(*status_reg).dwval = status_reg_tmp.dwval;
			status_reg_tmp.bits.wr_pulse = 0;
			(*status_reg).dwval = status_reg_tmp.dwval;
			status_reg_tmp.bits.wr_pulse = 1;
			(*status_reg).dwval = status_reg_tmp.dwval;
			status_reg_tmp.bits.wr_pulse = 0;
			(*status_reg).dwval = status_reg_tmp.dwval;
			i++;
	    	}
		hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
	}
}

void mem_status_exit(void)
{
	return ;
}

void save_mem_status(volatile __u32 val)
{
	if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		status_reg_tmp.bits.reg_sel = 1;
		status_reg_tmp.bits.data_wr = val;
		(*status_reg).dwval = status_reg_tmp.dwval;
		status_reg_tmp.bits.wr_pulse = 0;
		(*status_reg).dwval = status_reg_tmp.dwval;
		status_reg_tmp.bits.wr_pulse = 1;
		(*status_reg).dwval = status_reg_tmp.dwval;
		status_reg_tmp.bits.wr_pulse = 0;
		(*status_reg).dwval = status_reg_tmp.dwval;
		hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
	}

//	asm volatile ("dsb");
//	asm volatile ("isb");
	return;
}

__u32 get_mem_status(void)
{
	int val = 0;
	status_reg_tmp.bits.reg_sel = 1;
    
	if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		(*status_reg).dwval = status_reg_tmp.dwval;
		//read
		status_reg_tmp.dwval = (*status_reg).dwval;
		hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
	}
    
	val = status_reg_tmp.bits.data_rd;
	return (val);
}

void show_mem_status(void)
{
	int i = 1;
	int val = 0;
	while(i < STANDBY_STATUS_REG_NUM) {
		status_reg_tmp.bits.reg_sel = i;
	    
	    //write
	    if (!hwspin_lock_timeout(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		    (*status_reg).dwval = status_reg_tmp.dwval;
		    //read
		    status_reg_tmp.dwval = (*status_reg).dwval;
		    hwspin_unlock(MEM_RTC_REG_HWSPINLOCK);
	    }
	   
	    val = status_reg_tmp.bits.data_rd;
	    printk("addr %x, value = %x. \n", \
		    (i), val);
	    i++;
	}
}

void save_mem_status_nommu(volatile __u32 val)
{
	if (!hwspin_lock_timeout_nommu(MEM_RTC_REG_HWSPINLOCK, 20000)) {
		status_reg_pa_tmp.bits.reg_sel = 1;
		status_reg_pa_tmp.bits.data_wr = val;
		(*status_reg_pa).dwval = status_reg_pa_tmp.dwval;
		status_reg_pa_tmp.bits.wr_pulse = 0;
		(*status_reg_pa).dwval = status_reg_pa_tmp.dwval;
		status_reg_pa_tmp.bits.wr_pulse = 1;
		(*status_reg_pa).dwval = status_reg_pa_tmp.dwval;
		status_reg_pa_tmp.bits.wr_pulse = 0;
		(*status_reg_pa).dwval = status_reg_pa_tmp.dwval;
		hwspin_unlock_nommu(MEM_RTC_REG_HWSPINLOCK);
	}

	return;
}

