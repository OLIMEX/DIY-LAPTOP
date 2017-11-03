#include "pm_i.h"

volatile int  print_flag = 0;

void busy_waiting(void)
{
#if 1
	volatile __u32 loop_flag = 1;
	while(1 == loop_flag);
	
#endif
	return;
}

