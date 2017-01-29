#include "pm.h"


int pm_get_dev_info(char *name, int index, u32 **base, u32 *len)
{
	struct device_node *np;
	u32 reg[4*(index+1)];

	np = of_find_node_by_type(NULL, name);
	if(NULL == np){
	    printk(KERN_ERR "can not find np for %s. \n", name);
	}
	else{
	    //printk(KERN_INFO "np name = %s. \n", np->full_name);
	    of_property_read_u32_array(np, "reg", reg, ARRAY_SIZE(reg));
	    *base = (u32 *)((phys_addr_t)reg[1 + index*4]);
	    printk(KERN_INFO "%s physical base = 0x%p . \n", name, *base);
	    *len = reg[3 + index*4];
	    *base = of_iomap(np, index);
	    //printk(KERN_INFO "virtual base = 0x%p. \n", *base);
	}


	return 0;

}
