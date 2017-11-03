#ifndef _PM_DEBUG_H
#define _PM_DEBUG_H

#include "pm_config.h"
/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
 



//#define GET_CYCLE_CNT
#define IO_MEASURE
extern volatile int print_flag;

enum counter_type_e{
	I_CACHE_MISS = 0X01,
	I_TLB_MISS = 0X02,
	D_CACHE_MISS = 0X03,
	D_TLB_MISS = 0X05,
};

typedef union{
    __u32 dwval;
    struct __r_pio_pad_hold
    {
	__u32   data_rd:8;        //bit0-7,  data read reg.
	__u32   data_wr:8;        //bit8-15, data write reg.
	__u32   reg_sel:2;        //bit16-17,  data reg sel.
	__u32   reserved:13;      //bit18-30, reserved
	__u32   wr_pulse:1;       //bit31, write ops pulse:
	// first write 1, delay 1us, then write 0, 
	// the dataWrite will be writen to RTC domain. 
    } bits; 
}__r_prcm_pio_pad_hold;

void set_event_counter(enum counter_type_e type);
int get_event_counter(enum counter_type_e type);
void init_event_counter (__u32 do_reset, __u32 enable_divider);

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})

/*
 * if return true, means a is after b;
 */	
#define counter_after(a,b) \
(typecheck(__u32, a) && \
typecheck(__u32, b) && \
((__s32)(b) - (__s32)(a) < 0))
#define counter_before(a,b) counter_after(b,a)

#define counter_after_eq(a,b) \
(typecheck(__u32, a) && \
typecheck(__u32, b) && \
((__s32)(a) - (__s32)(b) >= 0))
#define counter_before_eq(a,b) counter_after_eq(b,a)


void busy_waiting(void);
/*
 * notice: when resume, boot0 need to clear the flag, 
 * in case the data in dram be destoryed result in the system is re-resume in cycle.
*/
void mem_status_init(char *name);
void mem_status_init_nommu(void);
void mem_status_clear(void);
void mem_status_exit(void);
void save_mem_flag(void);
void clear_mem_flag(void);
void save_mem_status(volatile __u32 val);
void parse_status_code(__u32 code, __u32 index);
void save_mem_status_nommu(volatile __u32 val);
void save_cpux_mem_status_nommu(volatile __u32 val);

__u32 get_mem_status(void);
void show_mem_status(void);
__u32 save_sun5i_mem_status_nommu(volatile __u32 val);
__u32 save_sun5i_mem_status(volatile __u32 val);
void save_irq_status(volatile __u32 val);

/*for secure debug, add by huangshr
 *data: 2014-10-20
 */
void pm_secure_mem_status_init(char *name);
void pm_secure_mem_status_init_nommu(void);
void pm_secure_mem_status_clear(void);
void pm_secure_mem_status_exit(void);
void show_pm_secure_mem_status(void);
void save_pm_secure_mem_status(volatile __u32 val);
void save_pm_secure_mem_status_nommu(volatile __u32 val);
__u32 get_pm_secure_mem_status(void);



void io_init(void);
void io_init_high(void);
void io_init_low(void);
void io_high(int num);

#endif /*_PM_DEBUG_H*/

