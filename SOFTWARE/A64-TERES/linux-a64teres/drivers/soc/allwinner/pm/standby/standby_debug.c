#include "standby.h"
#include "../pm_config.h"

void mem_status_init(void)
{
    return  ;
}

void mem_status_init_nommu(void)
{
    return  ;
}

void mem_status_clear(void)
{
	int i = 0;

	while(i < STANDBY_STATUS_REG_NUM){
		*(volatile int *)(STANDBY_STATUS_REG + i*4) = 0x0;
		i++;
	}
	return	;

}

void mem_status_exit(void)
{
	return ;
}

void save_mem_status(volatile __u32 val)
{
	*(volatile __u32 *)(STANDBY_STATUS_REG  + 0x0c) = val;
	asm volatile ("dsb");
	asm volatile ("isb");
	return;
}

__u32 get_mem_status(void)
{
	return *(volatile __u32 *)(STANDBY_STATUS_REG  + 0x0c);
}

void show_mem_status(void)
{
	int i = 0;

	while(i < STANDBY_STATUS_REG_NUM){
		printk("addr %x, value = %x. \n", \
			(STANDBY_STATUS_REG + i*4), *(volatile int *)(STANDBY_STATUS_REG + i*4));
		i++;
	}
}

void save_mem_status_nommu(volatile __u32 val)
{
	*(volatile __u32 *)(STANDBY_STATUS_REG_PA  + 0x0c) = val;
	return;
}

void save_cpux_mem_status_nommu(volatile __u32 val)
{
	*(volatile __u32 *)(STANDBY_STATUS_REG_PA  + 0x04) = val;
	return;
}

