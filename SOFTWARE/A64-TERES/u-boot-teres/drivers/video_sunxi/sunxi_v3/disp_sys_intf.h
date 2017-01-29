#ifndef _DISP_SYS_INTF_
#define _DISP_SYS_INTF_

#include <common.h>

/* cache flush flags */
#define  CACHE_FLUSH_I_CACHE_REGION       0
#define  CACHE_FLUSH_D_CACHE_REGION       1
#define  CACHE_FLUSH_CACHE_REGION         2
#define  CACHE_CLEAN_D_CACHE_REGION       3
#define  CACHE_CLEAN_FLUSH_D_CACHE_REGION 4
#define  CACHE_CLEAN_FLUSH_CACHE_REGION   5

typedef struct
{
	char  gpio_name[32];
	int port;
	int port_num;
	int mul_sel;
	int pull;
	int drv_level;
	int data;
	int gpio;
} disp_gpio_set_t;

void disp_sys_cache_flush(void*address, u32 length, u32 flags);

int disp_sys_register_irq(u32 IrqNo, u32 Flags,void* Handler,void *pArg,u32 DataSize,u32 Prio);
void disp_sys_unregister_irq(u32 IrqNo, void * Handler, void *pArg);
void disp_sys_disable_irq(u32 IrqNo);
void disp_sys_enable_irq(u32 IrqNo);

void disp_sys_irqlock(void* lock, unsigned long *cpu_sr);
void disp_sys_irqunlock(void* lock, unsigned long *cpu_sr);

void disp_sys_lock(void* lock);
void disp_sys_unlock(void* lock);

void * disp_sys_malloc(u32 Size);
void disp_sys_free(void *Addr);

/* returns: 0:invalid, 1: int; 2:str, 3: gpio */
int disp_sys_script_get_item(char *main_name, char *sub_name, int value[], int count);

int disp_sys_get_ic_ver(void);

int disp_sys_gpio_request(disp_gpio_set_t *gpio_list, u32 group_count_max);
int disp_sys_gpio_release(int p_handler, s32 if_release_to_default_status);

/* direction: 0:input, 1:output */
int disp_sys_gpio_set_direction(u32 p_handler, u32 direction, const char *gpio_name);
int disp_sys_gpio_get_value(u32 p_handler, const char *gpio_name);
int disp_sys_gpio_set_value(u32 p_handler, u32 value_to_gpio, const char *gpio_name);
int disp_sys_power_enable(char *name);
int disp_sys_power_disable(char *name);

int disp_sys_pwm_request(u32 pwm_id);
int disp_sys_pwm_free(int p_handler);
int disp_sys_pwm_enable(int p_handler);
int disp_sys_pwm_disable(int p_handler);
int disp_sys_pwm_config(int p_handler, int duty_ns, int period_ns);
int disp_sys_pwm_set_polarity(int p_handler, int polarity);

/* clock */
int disp_sys_clk_set_rate(const char *id, unsigned long rate);
unsigned long disp_sys_clk_get_rate(const char *id);
int disp_sys_clk_set_parent(const char *id, const char *parent);
int disp_sys_clk_enable(const char *id);
int disp_sys_clk_disable(const char *id);
int disp_sys_clk_init(void);

#endif
