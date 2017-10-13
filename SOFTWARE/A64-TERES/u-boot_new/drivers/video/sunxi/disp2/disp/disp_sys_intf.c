
#include "de/bsp_display.h"
#include "disp_sys_intf.h"
#include "asm/io.h"


/* cache flush flags */
#define  CACHE_FLUSH_I_CACHE_REGION       0
#define  CACHE_FLUSH_D_CACHE_REGION       1
#define  CACHE_FLUSH_CACHE_REGION         2
#define  CACHE_CLEAN_D_CACHE_REGION       3
#define  CACHE_CLEAN_FLUSH_D_CACHE_REGION 4
#define  CACHE_CLEAN_FLUSH_CACHE_REGION   5

/*
*******************************************************************************
*                     OSAL_CacheRangeFlush
*
* Description:
*    Cache flush
*
* Parameters:
*    address    :  start address to be flush
*    length     :  size
*    flags      :  flush flags
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_cache_flush(void*address, u32 length, u32 flags)
{
	if (address == NULL || length == 0) {
		return;
	}

	switch(flags) {
	case CACHE_FLUSH_I_CACHE_REGION:
	break;

	case CACHE_FLUSH_D_CACHE_REGION:
	break;

	case CACHE_FLUSH_CACHE_REGION:
	break;

	case CACHE_CLEAN_D_CACHE_REGION:
	break;

	case CACHE_CLEAN_FLUSH_D_CACHE_REGION:
	break;

	case CACHE_CLEAN_FLUSH_CACHE_REGION:
	break;

	default:
	break;
	}
	return;
}

/*
*******************************************************************************
*                     disp_sys_register_irq
*
* Description:
*    irq register
*
* Parameters:
*    irqno    	    ��input.  irq no
*    flags    	    ��input.
*    Handler  	    ��input.  isr handler
*    pArg 	        ��input.  para
*    DataSize 	    ��input.  len of para
*    prio	        ��input.    priority

*
* Return value:
*
*
* note:
*    typedef s32 (*ISRCallback)( void *pArg)��
*
*******************************************************************************
*/
int disp_sys_register_irq(u32 IrqNo, u32 Flags, void* Handler,void *pArg,u32 DataSize,u32 Prio)
{
	__inf("%s, irqNo=%d, Handler=0x%p, pArg=0x%p\n", __func__, IrqNo, Handler, pArg);
	irq_install_handler(IrqNo, (interrupt_handler_t *)Handler,  pArg);

	return 0;
}

/*
*******************************************************************************
*                     disp_sys_unregister_irq
*
* Description:
*    irq unregister
*
* Parameters:
*    irqno    	��input.  irq no
*    handler  	��input.  isr handler
*    Argment 	��input.    para
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_unregister_irq(u32 IrqNo, void* Handler, void *pArg)
{
	irq_free_handler(IrqNo);
}

/*
*******************************************************************************
*                     disp_sys_enable_irq
*
* Description:
*    enable irq
*
* Parameters:
*    irqno ��input.  irq no
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_enable_irq(u32 IrqNo)
{
	irq_enable(IrqNo);
}

/*
*******************************************************************************
*                     disp_sys_disable_irq
*
* Description:
*    disable irq
*
* Parameters:
*     irqno ��input.  irq no
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_disable_irq(u32 IrqNo)
{
	irq_disable(IrqNo);
}
#if 1
void spin_lock_init(spinlock_t* lock)
{
	return;
}

void mutex_init(struct mutex* lock)
{
	return;
}

void mutex_destroy(struct mutex* lock)
{
	return;
}

void mutex_lock(struct mutex* lock)
{
	return;
}

void mutex_unlock(struct mutex* lock)
{
	return;
}

void tasklet_init(struct tasklet_struct *tasklet, void (*func), unsigned long data)
{
	if ((NULL == tasklet) || (NULL == func)) {
		__wrn("tasklet_init, para is NULL, tasklet=0x%p, func=0x%p\n", tasklet, func);
		return ;
	}
	tasklet->func = func;
	tasklet->data = data;

	return ;
}

void tasklet_schedule(struct tasklet_struct *tasklet)
{
	if (NULL == tasklet) {
		__wrn("tasklet_schedule, para is NULL, tasklet=0x%p\n", tasklet);
		return ;
	}
	tasklet->func(tasklet->data);
}

void * kmalloc(u32 Size, u32 flag)
{
	void * addr;

	addr = malloc(Size);
	if(addr)
		memset(addr, 0, Size);

	return addr;
}

void kfree(void *Addr)
{
	free(Addr);
}

typedef struct __disp_node_map
{
	char node_name[16];
	int  nodeoffset;
}disp_fdt_node_map_t;

static disp_fdt_node_map_t g_disp_fdt_node_map[] ={
	{FDT_DISP_PATH, -1},
	{FDT_HDMI_PATH, -1},
	{FDT_LCD0_PATH, -1},
	{FDT_LCD1_PATH, -1},
	{FDT_BOOT_DISP_PATH, -1},
	{"",-1}
};

void disp_fdt_init(void)
{
	int i = 0;
	while(strlen(g_disp_fdt_node_map[i].node_name))
	{
		g_disp_fdt_node_map[i].nodeoffset =
			fdt_path_offset(working_fdt,g_disp_fdt_node_map[i].node_name);
		i++;
	}
}

int  disp_fdt_nodeoffset(char *main_name)
{
	int i = 0;
	for(i = 0; ; i++)
	{
		if( 0 == strcmp(g_disp_fdt_node_map[i].node_name, main_name))
		{
			//find ok
			return g_disp_fdt_node_map[i].nodeoffset;
		}
		if( 0 == strlen(g_disp_fdt_node_map[i].node_name) )
		{
			//last
			return -1;
		}
	}
	return -1;
}

/* type: 0:invalid, 1: int; 2:str, 3: gpio */
int disp_sys_script_get_item(char *main_name, char *sub_name, int value[], int type)
{
	int node;
	int ret = 0;
	user_gpio_set_t  gpio_info;
	disp_gpio_set_t  *gpio_list;
	//int i, node_index = 0;
	//bool find_flag = false;

	node = disp_fdt_nodeoffset(main_name);
	if(node < 0 )
	{
		__inf("fdt get node offset faill: %s\n", main_name);
		return ret;
	}

	if (1 == type) {
		if (fdt_getprop_u32(working_fdt, node, sub_name, (uint32_t*)value) < 0)
			__inf("fdt_getprop_u32 %s.%s fail\n", main_name, sub_name);
		else
			ret = type;
	} else if (2 == type) {
		const char *str;

		if (fdt_getprop_string(working_fdt, node, sub_name, (char **)&str) < 0)
			__inf("fdt_getprop_string %s.%s fail\n", main_name, sub_name);
		else {
			ret = type;
			memcpy((void*)value, str, strlen(str)+1);
		}
	} else if (3 == type) {
		if(fdt_get_one_gpio_by_offset(node, sub_name, &gpio_info) < 0)
			__wrn("fdt_get_one_gpio %s.%s fail\n", main_name, sub_name);
		else {
			gpio_list = (disp_gpio_set_t  *)value;
			gpio_list->port = gpio_info.port;
			gpio_list->port_num = gpio_info.port_num;
			gpio_list->mul_sel = gpio_info.mul_sel;
			gpio_list->drv_level = gpio_info.drv_level;
			gpio_list->pull = gpio_info.pull;
			gpio_list->data = gpio_info.data;

			memcpy(gpio_info.gpio_name, sub_name, strlen(sub_name)+1);
			__inf("%s.%s gpio=%d,mul_sel=%d,data:%d\n",main_name, sub_name, gpio_list->gpio, gpio_list->mul_sel, gpio_list->data);
			ret = type;
		}
	}

	return ret;
}
EXPORT_SYMBOL(disp_sys_script_get_item);

int disp_sys_get_ic_ver(void)
{
    return 0;
}

int disp_sys_gpio_request(disp_gpio_set_t *gpio_list, u32 group_count_max)
{
	user_gpio_set_t gpio_info;
	gpio_info.port = gpio_list->port;
	gpio_info.port_num = gpio_list->port_num;
	gpio_info.mul_sel = gpio_list->mul_sel;
	gpio_info.drv_level = gpio_list->drv_level;
	gpio_info.data = gpio_list->data;

	__inf("disp_sys_gpio_request, port:%d, port_num:%d, mul_sel:%d, pull:%d, drv_level:%d, data:%d\n", gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
	 //gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
#if 0
	if(gpio_list->port == 0xffff) {
		__u32 on_off;
		on_off = gpio_list->data;
		//axp_set_dc1sw(on_off);
		axp_set_supply_status(0, PMU_SUPPLY_DC1SW, 0, on_off);

		return 0xffff;
	}
#endif
	return gpio_request(&gpio_info, group_count_max);
}
EXPORT_SYMBOL(disp_sys_gpio_request);

int disp_sys_gpio_request_simple(disp_gpio_set_t *gpio_list, u32 group_count_max)
{

	return 0;
}
int disp_sys_gpio_release(int p_handler, s32 if_release_to_default_status)
{
	if(p_handler != 0xffff)
	{
		gpio_release(p_handler, if_release_to_default_status);
	}
	return 0;
}
EXPORT_SYMBOL(disp_sys_gpio_release);

/* direction: 0:input, 1:output */
int disp_sys_gpio_set_direction(u32 p_handler, u32 direction, const char *gpio_name)
{
	return gpio_set_one_pin_io_status(p_handler, direction, gpio_name);
}

int disp_sys_gpio_get_value(u32 p_handler, const char *gpio_name)
{
	return gpio_read_one_pin_value(p_handler, gpio_name);
}

int disp_sys_gpio_set_value(u32 p_handler, u32 value_to_gpio, const char *gpio_name)
{
	return gpio_write_one_pin_value(p_handler, value_to_gpio, gpio_name);
}

extern int fdt_set_all_pin(const char* node_path,const char* pinctrl_name);
int disp_sys_pin_set_state(char *dev_name, char *name)
{
	char compat[32];
	u32 len = 0;
	int state = 0;
	int ret = -1;
	int nodeoffset;

	if (!strcmp(name, DISP_PIN_STATE_ACTIVE))
		state = 1;
	else
		state = 0;

	len = sprintf(compat, "%s", dev_name);
	if (len > 32)
		__wrn("disp_sys_set_state, size of mian_name is out of range\n");

	nodeoffset = disp_fdt_nodeoffset(compat);
	if(nodeoffset < 0)
	{
		return ret;
	}
	ret = fdt_set_all_pin_by_offset(nodeoffset, (1 == state)?"pinctrl-0":"pinctrl-1");
	if (0 != ret)
		__wrn("%s, fdt_set_all_pin, ret=%d\n", __func__, ret);

	return ret;
}
EXPORT_SYMBOL(disp_sys_pin_set_state);

int disp_sys_power_enable(char *name)
{
	int ret = 0;
	if(0 == strlen(name)) {
		return 0;
	}
	ret = axp_set_supply_status_byregulator(name, 1);
	printf("enable power %s, ret=%d\n", name, ret);

	return 0;
}
EXPORT_SYMBOL(disp_sys_power_enable);

int disp_sys_power_disable(char *name)
{
	int ret = 0;

	ret = axp_set_supply_status_byregulator(name, 0);
	printf("disable power %s, ret=%d\n", name, ret);
	return 0;
}
EXPORT_SYMBOL(disp_sys_power_disable);

uintptr_t disp_sys_pwm_request(u32 pwm_id)
{
#if defined(CONFIG_ARCH_SUN50IW1P1)
	pwm_request(pwm_id, "lcd");
#endif
	return (pwm_id + 0x100);
}

int disp_sys_pwm_free(uintptr_t p_handler)
{
	return 0;
}

int disp_sys_pwm_enable(uintptr_t p_handler)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
	ret = pwm_enable(pwm_id);
#else
	ret = sunxi_pwm_enable(pwm_id);
#endif

	return ret;

}

int disp_sys_pwm_disable(uintptr_t p_handler)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
	pwm_disable(pwm_id);
#else
	sunxi_pwm_disable(pwm_id);
#endif

	return ret;
}

int disp_sys_pwm_config(uintptr_t p_handler, int duty_ns, int period_ns)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
	ret = pwm_config(pwm_id, duty_ns, period_ns);
#else
	ret = sunxi_pwm_config(pwm_id, duty_ns, period_ns);
#endif
	return ret;
}

int disp_sys_pwm_set_polarity(uintptr_t p_handler, int polarity)
{
	int ret = 0;
	int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
	ret = pwm_set_polarity(pwm_id, polarity);
#else
	ret = sunxi_pwm_set_polarity(pwm_id, polarity);
#endif

	return ret;
}

#endif

#if 0
int clk_set_rate(struct clk* clk, unsigned long rate)
{
	int ret = 0;

	return ret;
}

unsigned long clk_get_rate(struct clk* clk)
{
	unsigned long rate = 0;

	return rate;
}
EXPORT_SYMBOL(clk_set_rate);
int clk_set_parent(struct clk* clk, struct clk *parent)
{
	int ret = 0;

	return ret;
}

struct clk* clk_get_parent(struct clk* clk)
{
	struct clk* ret = NULL;

	return ret;
}

int clk_enable(struct clk* clk)
{
		return 0;
}

int clk_prepare_enable(struct clk* clk)
{
	return clk_enable(clk);
}

int clk_disable(struct clk* clk)
{
	int ret = 0;

	return ret;
}

struct clk* clk_get(char* clk)
{
	struct clk* ret = NULL;

	return ret;
}

void clk_put(struct clk* clk)
{
	return;
}

EXPORT_SYMBOL(clk_get_rate);
EXPORT_SYMBOL(clk_set_parent);
EXPORT_SYMBOL(clk_enable);
EXPORT_SYMBOL(clk_disable);
#endif
