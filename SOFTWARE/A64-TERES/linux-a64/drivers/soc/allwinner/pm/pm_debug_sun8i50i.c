#include "pm_i.h"

#define CPUX_STATUS_CODE_INDEX (1)
#define CPUX_IRQ_STATUS_CODE_INDEX (2)
#define CPUS_STATUS_CODE_INDEX (3)
static u32 *base;
static u32 len;
static u32 mem_status_init_done = 0;
void mem_status_init(char *name)
{
    u32 gpr_offset;
    struct device_node *np;
    
    pm_get_dev_info(name, 0, &base, &len);
    np = of_find_node_by_type(NULL, name);
    if(NULL == np){
	printk(KERN_ERR "can not find np for %s. \n", name);
    }
    else{
	printk(KERN_INFO "np name = %s. \n", np->full_name);
	if(!of_property_read_u32(np, "gpr_offset", &gpr_offset)){
	    if(!of_property_read_u32(np, "gpr_len", &len)){
		base = (gpr_offset/sizeof(u32) + base);
		printk("base = %p, len = %x.\n", base, len);

	    }
	}
    }
	mem_status_init_done = 1;
	return  ;
}

void mem_status_init_nommu(void)
{
    return  ;
}

void mem_status_clear(void)
{
	int i = 0;

	while(i < len){
		*(volatile int *)((phys_addr_t)(base + i)) = 0x0;
		i++;
	}
	return	;

}

void mem_status_exit(void)
{
	return ;
}

void save_irq_status(volatile __u32 val)
{
	if(likely(1 == mem_status_init_done)){
		*(volatile __u32 *)((phys_addr_t)(base  + CPUX_IRQ_STATUS_CODE_INDEX)) = val;
		//	asm volatile ("dsb");
		//	asm volatile ("isb");
	}
	return;
}

void save_mem_status(volatile __u32 val)
{
	*(volatile __u32 *)((phys_addr_t)(base  + CPUX_STATUS_CODE_INDEX)) = val;
//	asm volatile ("dsb");
//	asm volatile ("isb");
	return;
}

__u32 get_mem_status(void)
{
	return *(volatile __u32 *)((phys_addr_t)(base  + CPUX_STATUS_CODE_INDEX));
}

void parse_cpux_status_code(__u32 code)
{
    printk("%s. \n", pm_errstr(code));

}

void parse_cpus_status_code(__u32 code)
{

}

void parse_status_code(__u32 code, __u32 index)
{
	switch(index){
		case CPUX_STATUS_CODE_INDEX:
			parse_cpux_status_code(code);
			break;
		case CPUS_STATUS_CODE_INDEX: 
			parse_cpus_status_code(code);
			break;
		default:
			printk(KERN_INFO "notice: para err, index = %x.\n", index);
	}

	return ;
}

void show_mem_status(void)
{
	int i = 0;
	__u32 status_code = 0;

	while(i < len){
		status_code = *(volatile int *)((phys_addr_t)(base + i));
		printk(KERN_INFO "addr %p, value = %x. \n", (base + i), status_code);
		parse_status_code(status_code, i);
		i++;
	}
}



