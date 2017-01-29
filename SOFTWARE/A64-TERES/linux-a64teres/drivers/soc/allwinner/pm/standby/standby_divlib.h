#ifndef _STANDBY_DIVLIBC_H
#define _STANDBY_DIVLIBC_H

void __mem_div0(void);
__u32 raw_lib_udiv(__u32 dividend, __u32 divisior);
void standby_delay(int cycle);
void standby_delay_cycle(int cycle);

#endif /*_MEM_DIVLIBC_H*/

