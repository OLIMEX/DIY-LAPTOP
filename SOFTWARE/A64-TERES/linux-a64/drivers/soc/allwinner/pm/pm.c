/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : pm.c
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-27 14:08
* Descript: power manager for allwinners chips platform.
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/

#include <linux/module.h>
#include <linux/suspend.h>
#include <asm/suspend.h>
#include <linux/cpufreq.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/major.h>
#include <linux/device.h>
#include <linux/console.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/cpu_pm.h>
#include <asm/system_misc.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/tlbflush.h>
#include <linux/power/aw_pm.h>
#include <asm/cacheflush.h>
#include "pm_o.h"
#include <linux/arisc/arisc.h>

#include <linux/power/scenelock.h>
#include <linux/kobject.h>
#include <linux/ctype.h>
#include <linux/regulator/consumer.h>
#include <linux/power/axp_depend.h>
#include "../../../../kernel/power/power.h"

static struct kobject *aw_pm_kobj;
static standby_space_cfg_t standby_space;

//#define CROSS_MAPPING_STANDBY

#define AW_PM_DBG   1
//#undef PM_DBG
#if(AW_PM_DBG)
    #define PM_DBG(format,args...)   printk("[pm]"format,##args)
#else
    #define PM_DBG(format,args...)   do{}while(0)
#endif

#ifdef RETURN_FROM_RESUME0_WITH_NOMMU
#define PRE_DISABLE_MMU    //actually, mean ,prepare condition to disable mmu
#endif

#ifdef ENTER_SUPER_STANDBY
#undef PRE_DISABLE_MMU
#endif

#ifdef ENTER_SUPER_STANDBY_WITH_NOMMU
#define PRE_DISABLE_MMU    //actually, mean ,prepare condition to disable mmu
#endif

#ifdef RETURN_FROM_RESUME0_WITH_MMU
#undef PRE_DISABLE_MMU
#endif

#ifdef WATCH_DOG_RESET
#define PRE_DISABLE_MMU    //actually, mean ,prepare condition to disable mmu
#endif

//#define VERIFY_RESTORE_STATUS

/* define major number for power manager */
#define AW_PMU_MAJOR    267

__u32 debug_mask = PM_STANDBY_TEST; // | PM_STANDBY_PRINT_STANDBY | PM_STANDBY_PRINT_RESUME| PM_STANDBY_ENABLE_JTAG;
static int suspend_freq = SUSPEND_FREQ;
static int suspend_delay_ms = SUSPEND_DELAY_MS;
static unsigned long time_to_wakeup = 0;

extern char *standby_bin_start;
extern char *standby_bin_end;
extern char *suspend_bin_start;
extern char *suspend_bin_end;

#ifdef RESUME_FROM_RESUME1
extern char *resume1_bin_start;
extern char *resume1_bin_end;
#endif

/*mem_cpu_asm.S*/
extern int mem_arch_suspend(void);
extern int mem_arch_resume(void);
extern int disable_prefetch(void);
extern asmlinkage int mem_clear_runtime_context(void);
extern void save_runtime_context(__u32 *addr);
extern void clear_reg_context(void);

#ifdef CONFIG_CPU_FREQ_USR_EVNT_NOTIFY
extern void cpufreq_user_event_notify(void);
#endif

#if (defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW6P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1) || defined(CONFIG_ARCH_SUN50IW1P1)) && defined(CONFIG_AW_AXP)
static int config_sys_pwr(void);
#endif

#ifdef GET_CYCLE_CNT
static int start = 0;
static int resume0_period = 0;
static int resume1_period = 0;

static int pm_start = 0;
static int invalidate_data_time = 0;
static int invalidate_instruct_time = 0;
static int before_restore_processor = 0;
static int after_restore_process = 0;
//static int restore_runtime_peroid = 0;

//late_resume timing
static int late_resume_start = 0;
static int backup_area_start = 0;
static int backup_area1_start = 0;
static int backup_area2_start = 0;
static int clk_restore_start = 0;
static int gpio_restore_start = 0;
static int twi_restore_start = 0;
static int int_restore_start = 0;
static int tmr_restore_start = 0;
static int sram_restore_start = 0;
static int late_resume_end = 0;
#endif

struct aw_mem_para mem_para_info;
struct super_standby_para super_standby_para_info;
static const extended_standby_manager_t *extended_standby_manager_id = NULL;

standby_type_e standby_type = NON_STANDBY;
EXPORT_SYMBOL(standby_type);
standby_level_e standby_level = STANDBY_INITIAL;
EXPORT_SYMBOL(standby_level);

//static volatile int enter_flag = 0;
static int standby_mode = 0;
static int suspend_status_flag = 0;

static __mem_tmr_reg_t saved_tmr_state;
#ifdef	CONFIG_AW_AXP
extern void axp_powerkey_set(int value);
#endif

static struct aw_pm_info standby_info = {
    .standby_para = {
	.event = CPU0_WAKEUP_MSGBOX,
	.axp_event = CPUS_MEM_WAKEUP,
	.timeout = 0,
    },
    .pmu_arg = {
	.twi_port = 0,
	.dev_addr = 10,
    },
};


#define pm_printk(mask, format, args... )   do	{   \
    if(unlikely(debug_mask&mask)){		    \
	printk(KERN_INFO format, ##args);   \
    }					    \
}while(0)

static void aw_pm_dev_status(char *name, int mask)
{
    ptrdiff_t i = 0;
    u32 *base = 0;
    u32 len = 0;

    if(unlikely(debug_mask&mask)){
	printk(KERN_INFO "%s status as follow:", name);
	pm_get_dev_info(name, 0, &base, &len);
	len = len/sizeof(u32);
	for(i=0; (i)<(len); i++){
	    printk(KERN_INFO "ADDR = %p, value = %x .\n", \
		    (base + i), *(volatile __u32 *)(base + i));
	}
    }

    return ;
}

static void aw_pm_show_dev_status(void)
{

    aw_pm_dev_status("pio", PM_STANDBY_PRINT_IO_STATUS);
    aw_pm_dev_status("r_pio", PM_STANDBY_PRINT_CPUS_IO_STATUS);
    aw_pm_dev_status("clocks", PM_STANDBY_PRINT_CCU_STATUS);

    return ;
}

static char *parse_debug_mask(unsigned int bitmap, char *s, char *end)
{
    int i = 0;
    int counted = 0;
    int count = 0;
    unsigned int bit_event = 0;
    
    for(i=0; i<32; i++){
	bit_event = (1<<i & bitmap);
	switch(bit_event){
	case 0				  : break;
	case PM_STANDBY_PRINT_STANDBY	  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_STANDBY         ", PM_STANDBY_PRINT_STANDBY	  ); count++; break;
	case PM_STANDBY_PRINT_RESUME	  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_RESUME          ", PM_STANDBY_PRINT_RESUME	  ); count++; break;
	case PM_STANDBY_ENABLE_JTAG		  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_ENABLE_JTAG           ", PM_STANDBY_ENABLE_JTAG	  ); count++; break;
	case PM_STANDBY_PRINT_PORT		  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_PORT            ", PM_STANDBY_PRINT_PORT	  ); count++; break;
	case PM_STANDBY_PRINT_IO_STATUS 	  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_IO_STATUS       ", PM_STANDBY_PRINT_IO_STATUS  ); count++; break;
	case PM_STANDBY_PRINT_CACHE_TLB_MISS  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_CACHE_TLB_MISS  ", PM_STANDBY_PRINT_CACHE_TLB_MISS); count++; break;
	case PM_STANDBY_PRINT_CCU_STATUS 	  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_CCU_STATUS      ", PM_STANDBY_PRINT_CCU_STATUS  ); count++; break;
	case PM_STANDBY_PRINT_PWR_STATUS 	  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_PWR_STATUS      ", PM_STANDBY_PRINT_PWR_STATUS  ); count++; break;
	case PM_STANDBY_PRINT_CPUS_IO_STATUS  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_CPUS_IO_STATUS  ", PM_STANDBY_PRINT_CPUS_IO_STATUS); count++; break;
	case PM_STANDBY_PRINT_CCI400_REG	  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_CCI400_REG      ", PM_STANDBY_PRINT_CCI400_REG   ); count++; break;
	case PM_STANDBY_PRINT_GTBUS_REG	  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_GTBUS_REG       ", PM_STANDBY_PRINT_GTBUS_REG  ); count++; break;
	case PM_STANDBY_TEST		  : s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_TEST                  ", PM_STANDBY_TEST		  ); count++; break;
	case PM_STANDBY_PRINT_RESUME_IO_STATUS: s += scnprintf(s, end-s, "%-34s bit 0x%x\t", "PM_STANDBY_PRINT_RESUME_IO_STATUS", PM_STANDBY_PRINT_RESUME_IO_STATUS); count++; break;
	default				  : break;
	
	}
	if(counted != count && 0 == count%2){
	    counted = count;
	    s += scnprintf(s, end-s,"\n");
	}
    }
    
    s += scnprintf(s, end-s,"\n");

    return s;
}
static ssize_t debug_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	char *s = buf;
	char *end = buf + PAGE_SIZE;
	
	s += sprintf(buf, "0x%x\n", debug_mask);
	s = parse_debug_mask(debug_mask, s, end);

	s += sprintf(s, "%s\n", "debug_mask usage help info:");
	s += sprintf(s, "%s\n", "target: for enable checking the io, ccu... suspended status.");
	s += sprintf(s, "%s\n", "bitmap: each bit corresponding one module, as follow:");
	s = parse_debug_mask(0xffff, s, end);

	return (s-buf);
}

#define TMPBUFLEN 22
static int get_long(const char **buf, size_t *size, unsigned long *val, bool *neg)
{
	size_t len;
	char *p, tmp[TMPBUFLEN];

	if (!*size)
		return -EINVAL;

	len = *size;
	if (len > TMPBUFLEN - 1)
		len = TMPBUFLEN - 1;

	memcpy(tmp, *buf, len);

	tmp[len] = 0;
	p = tmp;
	if (*p == '-' && *size > 1) {
		*neg = true;
		p++;
	} else
		*neg = false;
	if (!isdigit(*p))
		return -EINVAL;

	*val = simple_strtoul(p, &p, 0);

	len = p - tmp;

	/* We don't know if the next char is whitespace thus we may accept
	 * invalid integers (e.g. 1234...a) or two integers instead of one
	 * (e.g. 123...1). So lets not allow such large numbers. */
	if (len == TMPBUFLEN - 1)
		return -EINVAL;

	return 0;
}


static ssize_t debug_mask_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
    unsigned long data = 0;
    bool neg = false;

    if(!get_long(&buf, &count, &data, &neg)){
	if(true != neg){
	    debug_mask = (unsigned int)data;
	}else{
	    printk("%s\n", "minus is Illegal. ");
	    return -EINVAL;
	}
    }else{
	printk("%s\n", "non-digital is Illegal. ");
	return -EINVAL;
    }

    return count;
}

static ssize_t parse_status_code_store(struct device *dev, 
	struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned int status_code = 0;
    unsigned int index = 0;

    sscanf(buf, "%x %x\n", &status_code, &index);
    parse_status_code(status_code, index);
    return size;
}

ssize_t parse_status_code_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *s = buf;

    show_mem_status();
    return (s - buf);
}

static void init_wakeup_src(unsigned int event)
{
#ifdef CONFIG_ARCH_SUN8IW10P1
	//config int src.
	/* initialise standby modules */
	if(unlikely(debug_mask&PM_STANDBY_PRINT_STANDBY)){
		//don't need init serial ,depend kernel?
		//serial_init(0);
		printk("normal standby wakeup src config = 0x%x. \n", event);
	}
	
	/* init some system wake source */
	if(event & CPU0_WAKEUP_MSGBOX){
	    if(unlikely(standby_info.standby_para.debug_mask&PM_STANDBY_PRINT_STANDBY)){
		printk("enable CPU0_WAKEUP_MSGBOX. \n");
	    }
	    mem_enable_int(INT_SOURCE_MSG_BOX);
	}
	
	if(event & CPU0_WAKEUP_EXINT){
	    printk("enable CPU0_WAKEUP_EXINT. \n");
	    mem_enable_int(INT_SOURCE_EXTNMI);
	}

	if(event & CPU0_WAKEUP_TIMEOUT){
	    printk("enable CPU0_WAKEUP_TIMEOUT. \n");
	    /* set timer for power off */
	    if(standby_info.standby_para.timeout) {
		mem_tmr_set(standby_info.standby_para.timeout);
		mem_enable_int(INT_SOURCE_TIMER0);
	    }
	}

	if(event & CPU0_WAKEUP_ALARM){
	    mem_enable_int(INT_SOURCE_ALARM);
	}

	if(event & CPU0_WAKEUP_KEY){
	    mem_key_init();
	    mem_enable_int(INT_SOURCE_LRADC);
	}
	if(event & CPU0_WAKEUP_IR){
	    mem_ir_init();
	    mem_enable_int(INT_SOURCE_IR0);
	    mem_enable_int(INT_SOURCE_IR1);
	}
	
	if(event & CPU0_WAKEUP_USB){
	    mem_usb_init();
	    mem_enable_int(INT_SOURCE_USBOTG);
	    mem_enable_int(INT_SOURCE_USBEHCI0);
	    mem_enable_int(INT_SOURCE_USBEHCI1);
	    mem_enable_int(INT_SOURCE_USBEHCI2);
	    mem_enable_int(INT_SOURCE_USBOHCI0);
	    mem_enable_int(INT_SOURCE_USBOHCI1);
	    mem_enable_int(INT_SOURCE_USBOHCI2);
	}
	
	if(event & CPU0_WAKEUP_PIO){
	    mem_enable_int(INT_SOURCE_GPIOA);
	    mem_enable_int(INT_SOURCE_GPIOB);
	    mem_enable_int(INT_SOURCE_GPIOC);
	    mem_enable_int(INT_SOURCE_GPIOD);
	    mem_enable_int(INT_SOURCE_GPIOE);
	    mem_enable_int(INT_SOURCE_GPIOF);
	    mem_enable_int(INT_SOURCE_GPIOG);
	    mem_enable_int(INT_SOURCE_GPIOH);
	    mem_enable_int(INT_SOURCE_GPIOI);
	    mem_enable_int(INT_SOURCE_GPIOJ);
	    mem_pio_clk_src_init();
	}
#else
	//config int src.
	mem_int_init();
	mem_tmr_init();
	mem_tmr_save(&(saved_tmr_state));
	if(event & CPU0_WAKEUP_TIMEOUT){
	    printk("enable CPUS_WAKEUP_TIMEOUT. \n");
	    /* set timer for power off */
	    if(super_standby_para_info.timeout) {
		mem_tmr_set(super_standby_para_info.timeout);
		mem_enable_int(INT_SOURCE_TIMER1);
	    }
	}
#endif
	return ;
}

static void exit_wakeup_src(unsigned int event)
{
#ifdef CONFIG_ARCH_SUN8IW10P1
	/* exit standby module */
	if(event & CPU0_WAKEUP_PIO){
	    mem_pio_clk_src_exit();
	}
	
	if(event & CPU0_WAKEUP_USB){
	    mem_usb_exit();
	}
	
	if(event & CPU0_WAKEUP_IR){
	    mem_ir_exit();
	}
	
	if(event & CPU0_WAKEUP_ALARM){
	}

	if(event & CPU0_WAKEUP_KEY){
	    mem_key_exit();
	}

	/* exit standby module */
	if(unlikely(debug_mask&PM_STANDBY_PRINT_STANDBY)){
		//restore serial clk & gpio config.
		//serial_exit();
	}
#else
	mem_tmr_restore(&(saved_tmr_state));
	mem_tmr_exit();
	mem_int_exit();
#endif
	return ;
}

#if defined(CONFIG_ARCH_SUN8IW10P1)
extern int axp_mem_save(void);
extern void axp_mem_restore(void);
static struct clk_state saved_clk_state;
static struct gpio_state saved_gpio_state;
static struct ccm_state  saved_ccm_state;
static struct sram_state saved_sram_state;

static void mem_device_save(void)
{
	mem_tmr_init();
	mem_gpio_init();
	mem_sram_init();
	mem_int_init();
	mem_clk_init(1);
	//backup device state
	mem_ccu_save(&(saved_ccm_state));
	mem_clk_save(&(saved_clk_state));
	mem_tmr_save(&(saved_tmr_state));
	mem_gpio_save(&(saved_gpio_state));
	mem_sram_save(&(saved_sram_state));
#ifdef CONFIG_AW_AXP
	axp_mem_save();
#endif

	return;
}

static void mem_device_restore(void)
{
#ifdef CONFIG_AW_AXP
	axp_mem_restore();
#endif
	mem_sram_restore(&(saved_sram_state));
	mem_gpio_restore(&(saved_gpio_state));
	mem_tmr_restore(&(saved_tmr_state));
	mem_clk_restore(&(saved_clk_state));
	mem_ccu_restore(&(saved_ccm_state));
	mem_int_exit();
	mem_tmr_exit();

	return;
}

/**
 * aw_standby_enter() - Enter the system sleep state
 *
 * @state: suspend state
 * @return: return 0 is process successed
 * @note: the core function for platform sleep
 */
static int aw_standby_enter(unsigned long arg)
{
	struct aw_pm_info *para =(struct aw_pm_info *)(arg);
	int ret = -1;
	int (*standby)(struct aw_pm_info *arg);

	standby = (int (*)(struct aw_pm_info *arg))SRAM_FUNC_START;

	ret = standby(para);
	if (ret == 0)
		soft_restart(virt_to_phys(cpu_resume));

	return ret;
}

static void query_wakeup_source(struct aw_pm_info *arg)
{
	arg->standby_para.event = 0;

	arg->standby_para.event |= mem_query_int(INT_SOURCE_EXTNMI)? 0:CPU0_WAKEUP_EXINT;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_USBOTG)? 0:CPU0_WAKEUP_USB;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_USBEHCI0)? 0:CPU0_WAKEUP_USB;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_USBEHCI1)? 0:CPU0_WAKEUP_USB;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_USBEHCI2)? 0:CPU0_WAKEUP_USB;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_USBOHCI0)? 0:CPU0_WAKEUP_USB;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_USBOHCI1)? 0:CPU0_WAKEUP_USB;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_USBOHCI2)? 0:CPU0_WAKEUP_USB;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_LRADC)? 0:CPU0_WAKEUP_KEY;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_IR0)? 0:CPU0_WAKEUP_IR;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_ALARM)? 0:CPU0_WAKEUP_ALARM;
	arg->standby_para.event |= mem_query_int(INT_SOURCE_TIMER0)? 0:CPU0_WAKEUP_TIMEOUT;
}
#endif

#if (defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW6P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1) || defined(CONFIG_ARCH_SUN50IW1P1)) && defined(CONFIG_AW_AXP)
static unsigned int pwr_dm_mask_saved = 0;
static int save_sys_pwr_state(const char *id)
{
	int bitmap = 0;

	bitmap = is_sys_pwr_dm_id(id);
	if ((-1) != bitmap) {
		pwr_dm_mask_saved |= (0x1 << bitmap);
	} else {
		pm_printk(PM_STANDBY_PRINT_PWR_STATUS, KERN_INFO "%s: is not sys \n", id);
	}
	return 0;

}

static int resume_sys_pwr_state(void)
{
	int i = 0, ret = -1;
	char *sys_name = NULL;

	for (i=0; i<32; i++) {
		if (pwr_dm_mask_saved & (0x1 << i)) {
			sys_name = get_sys_pwr_dm_id(i);
			if (NULL != sys_name) {
				ret = add_sys_pwr_dm(sys_name);
				if (ret < 0) {
					pm_printk(PM_STANDBY_PRINT_PWR_STATUS, KERN_INFO "%s: resume failed \n", sys_name);
				}
			}
		}
	}
	pwr_dm_mask_saved = 0;
	return 0;
}

static int check_sys_pwr_dm_status(char *pwr_dm)
{
    char  ldo_name[20] = "\0";
    char  enable_id[20] = "\0";
    int ret  = 0;
    int i = 0;

    ret = get_ldo_name(pwr_dm, ldo_name);
    if(ret < 0){
	printk(KERN_ERR "	    %s: get %s failed. ret = %d \n", __func__, pwr_dm, ret);
	return -1;
    }
    ret = get_enable_id_count(ldo_name);
    if(0 == ret){
	pm_printk(PM_STANDBY_PRINT_PWR_STATUS, KERN_INFO "%s: no child, use by %s, property: sys.\n", ldo_name, pwr_dm);
    }else{
	for(i = 0; i < ret; i++){
	    get_enable_id(ldo_name, i, (char *)enable_id);
	    printk(KERN_INFO "%s: active child id %d is: %s. ",  ldo_name,  i, enable_id);
	    //need to check all enabled id is belong to sys_pwr_dm.
	    if((-1) != is_sys_pwr_dm_id(enable_id)){
		pm_printk(PM_STANDBY_PRINT_PWR_STATUS, KERN_INFO "property: sys \n");
	    }else{
		pm_printk(PM_STANDBY_PRINT_PWR_STATUS, KERN_INFO "property: module \n");
		del_sys_pwr_dm(pwr_dm);
		save_sys_pwr_state(pwr_dm);
		break;
	    }
	}
    }
    return 0;

}

static int check_pwr_status(void)
{
    check_sys_pwr_dm_status("vdd-cpua");
    check_sys_pwr_dm_status("vdd-cpub");
    check_sys_pwr_dm_status("vcc-dram");
    check_sys_pwr_dm_status("vdd-gpu");
    check_sys_pwr_dm_status("vdd-sys");
    check_sys_pwr_dm_status("vdd-vpu");
    check_sys_pwr_dm_status("vdd-cpus");
    check_sys_pwr_dm_status("vdd-drampll");
    check_sys_pwr_dm_status("vcc-lpddr");
    check_sys_pwr_dm_status("vcc-adc");
    check_sys_pwr_dm_status("vcc-pl");
    check_sys_pwr_dm_status("vcc-pm");
    check_sys_pwr_dm_status("vcc-io");
    check_sys_pwr_dm_status("vcc-cpvdd");
    check_sys_pwr_dm_status("vcc-ldoin");
    check_sys_pwr_dm_status("vcc-pll");

    return 0;
}

static int init_sys_pwr_dm(void)
{
    unsigned int sys_mask = 0;

    add_sys_pwr_dm("vdd-cpua");
    //add_sys_pwr_dm("vdd-cpub");
    add_sys_pwr_dm("vcc-dram");
    //add_sys_pwr_dm("vdd-gpu");
    add_sys_pwr_dm("vdd-sys");
    //add_sys_pwr_dm("vdd-vpu");
    add_sys_pwr_dm("vdd-cpus");
    //add_sys_pwr_dm("vdd-drampll");
    add_sys_pwr_dm("vcc-lpddr");
    //add_sys_pwr_dm("vcc-adc");
    add_sys_pwr_dm("vcc-pl");
    //add_sys_pwr_dm("vcc-pm");
    add_sys_pwr_dm("vcc-io");
    //add_sys_pwr_dm("vcc-cpvdd");
    //add_sys_pwr_dm("vcc-ldoin");
    add_sys_pwr_dm("vcc-pll");
    
    sys_mask = get_sys_pwr_dm_mask();
    printk(KERN_INFO "after inited: sys_mask config = 0x%x. \n", sys_mask);

    return 0;
}
#endif 

/*
*********************************************************************************************************
*                           aw_pm_valid
*
*Description: determine if given system sleep state is supported by the platform;
*
*Arguments  : state     suspend state;
*
*Return     : if the state is valid, return 1, else return 0;
*
*Notes      : this is a call-back function, registered into PM core;
*
*********************************************************************************************************
*/
static int aw_pm_valid(suspend_state_t state)
{
#ifdef CHECK_IC_VERSION
	enum sw_ic_ver version = MAGIC_VER_NULL;
#endif

    PM_DBG("valid\n");

    if(!((state > PM_SUSPEND_ON) && (state < PM_SUSPEND_MAX))){
        PM_DBG("state (%d) invalid!\n", state);
        return 0;
    }

#ifdef CHECK_IC_VERSION
	if(1 == standby_mode){
			version = sw_get_ic_ver();
			if(!(MAGIC_VER_A13B == version || MAGIC_VER_A12B == version || MAGIC_VER_A10SB == version)){
				pr_info("ic version: %d not support super standby. \n", version);
				standby_mode = 0;
			}
	}
#endif

	//if 1 == standby_mode, actually, mean mem corresponding with super standby
	if(PM_SUSPEND_STANDBY == state){
		if(1 == standby_mode){
			standby_type = NORMAL_STANDBY;
		}else{
			standby_type = SUPER_STANDBY;
		}
	}else if(PM_SUSPEND_MEM == state || PM_SUSPEND_BOOTFAST == state){
		if(1 == standby_mode){
			standby_type = SUPER_STANDBY;
		}else{
			standby_type = NORMAL_STANDBY;
		}
	}

#if defined(CONFIG_ARCH_SUN9IW1P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN50IW1P1)
	if(NORMAL_STANDBY == standby_type){
		printk("Notice: sun9i&sun8iw5&sun50i not need support normal standby, \
				change to super standby.\n");

		standby_type = SUPER_STANDBY;
	}
#elif defined(CONFIG_ARCH_SUN8IW8P1)
	if(SUPER_STANDBY == standby_type){
		printk("Notice: sun8iw8p1 not need support super standby, \
				change to normal standby.\n");
		standby_type = NORMAL_STANDBY;
	}
	/* default enter to extended standby's normal standby*/
	{
		static struct scene_lock  normal_lock;
		if (unlikely(check_scene_locked(SCENE_NORMAL_STANDBY))) {
			scene_lock_init(&normal_lock, SCENE_NORMAL_STANDBY,  "normal_standby");
			scene_lock(&normal_lock);
		}
	}

#endif


    return 1;

}


/*
*********************************************************************************************************
*                           aw_pm_begin
*
*Description: Initialise a transition to given system sleep state;
*
*Arguments  : state     suspend state;
*
*Return     : return 0 for process successed;
*
*Notes      : this is a call-back function, registered into PM core, and this function
*             will be called before devices suspened;
*********************************************************************************************************
*/
static int aw_pm_begin(suspend_state_t state)
{
	static __u32 suspend_status = 0;
	static int  suspend_result = 0;    //record last suspend status, 0/-1
	static bool backup_console_suspend_enabled = 0;
	static bool backup_initcall_debug = 0; 
	static int backup_console_loglevel = 0;
	static __u32 backup_debug_mask = 0;

	PM_DBG("%d state begin\n", state);
	//set freq max
#ifdef CONFIG_CPU_FREQ_USR_EVNT_NOTIFY
	//cpufreq_user_event_notify();
#endif

	/*must init perfcounter, because delay_us and delay_ms is depandant perf counter*/

	//check rtc status, if err happened, do sth to fix it.
	suspend_status = get_pm_secure_mem_status(); 
	if( (error_gen(MOD_FIRST_BOOT_FLAG, 0)!= suspend_status) &&	    \
		(BOOT_UPGRADE_FLAG !=  suspend_status) &&		    \
		(error_gen(MOD_RESUME_COMPLETE_FLAG, 0) != suspend_status)){
	    suspend_result = -1;
	    printk("suspend_err, rtc gpr as follow: \n");
	    show_mem_status();
	    //adjust: loglevel, console_suspend, initcall_debug, debug_mask config.
	    //disable console suspend.
	    backup_console_suspend_enabled = console_suspend_enabled;
	    console_suspend_enabled = 0;
	    //enable initcall_debug
	    backup_initcall_debug = initcall_debug; 
	    initcall_debug = 1;
	    //change loglevel to 8
	    backup_console_loglevel = console_loglevel;
	    console_loglevel = 8;
	    //change debug_mask to 0xff
	    backup_debug_mask = debug_mask;
	    debug_mask |= 0x07;
	}else{
	    if(-1 == suspend_result){
		//restore console suspend.
		console_suspend_enabled = backup_console_suspend_enabled;
		//restore initcall_debug
		initcall_debug = backup_initcall_debug; 
		//restore console_loglevel
		console_loglevel = backup_console_loglevel;
		//restore debug_mask
		debug_mask = backup_debug_mask;
	    }
	    suspend_result = 0; 
	}
	save_pm_secure_mem_status(error_gen(MOD_FREEZER_THREAD,0));
	
	return 0;

}


/*
*********************************************************************************************************
*                           aw_pm_prepare
*
*Description: Prepare the platform for entering the system sleep state.
*
*Arguments  : none;
*
*Return     : return 0 for process successed, and negative code for error;
*
*Notes      : this is a call-back function, registered into PM core, this function
*             will be called after devices suspended, and before device late suspend
*             call-back functions;
*********************************************************************************************************
*/
static int aw_pm_prepare(void)
{
    PM_DBG("prepare\n");
    save_pm_secure_mem_status(error_gen(MOD_SUSPEND_DEVICES, ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_DONE));

    return 0;
}


/*
*********************************************************************************************************
*                           aw_pm_prepare_late
*
*Description: Finish preparing the platform for entering the system sleep state.
*
*Arguments  : none;
*
*Return     : return 0 for process successed, and negative code for error;
*
*Notes      : this is a call-back function, registered into PM core.
*             prepare_late is called before disabling nonboot CPUs and after
*              device drivers' late suspend callbacks have been executed;
*********************************************************************************************************
*/
static int aw_pm_prepare_late(void)
{
#ifdef CONFIG_CPU_FREQ	
    struct cpufreq_policy policy;
#endif
    PM_DBG("prepare_late\n");

#ifdef CONFIG_CPU_FREQ	
    if (cpufreq_get_policy(&policy, 0))
	goto out;
	
    cpufreq_driver_target(&policy, suspend_freq, CPUFREQ_RELATION_L);
#endif
    save_pm_secure_mem_status(error_gen(MOD_SUSPEND_DEVICES, ERR_SUSPEND_DEVICES_LATE_SUSPEND_DEVICES_DONE));
    return 0;

#ifdef CONFIG_CPU_FREQ	
out:
    return -1;
#endif
}


/*
*********************************************************************************************************
*                           aw_early_suspend
*
*Description: prepare necessary info for suspend&resume;
*
*Return     : return 0 is process successed;
*
*Notes      : -1: data is ok;
*			-2: data has been destory.
*********************************************************************************************************
*/
static int aw_early_suspend(void)
{

#ifdef ENTER_SUPER_STANDBY
    //print_call_info();
    if (check_scene_locked(SCENE_BOOT_FAST))
	super_standby_para_info.event = mem_para_info.axp_event;
    else
	super_standby_para_info.event = CPUS_BOOTFAST_WAKEUP;

    // the wakeup src is independent of the scene_lock.
    // the developer only need to care about: the scene support the wakeup src;
    if (NULL != extended_standby_manager_id) {
	super_standby_para_info.event |= extended_standby_manager_id->event;
	super_standby_para_info.gpio_enable_bitmap = extended_standby_manager_id->wakeup_gpio_map;
	super_standby_para_info.cpux_gpiog_bitmap = extended_standby_manager_id->wakeup_gpio_group;
    }
    if ((super_standby_para_info.event & (CPUS_WAKEUP_DESCEND | CPUS_WAKEUP_ASCEND)) == 0) {
#ifdef	CONFIG_AW_AXP
	axp_powerkey_set(1);
#endif
    }
#ifdef RESUME_FROM_RESUME1
    super_standby_para_info.resume_entry = SRAM_FUNC_START_PA;
    if ((NULL != extended_standby_manager_id) && (NULL != extended_standby_manager_id->pextended_standby))
	super_standby_para_info.pextended_standby = (unsigned int)(standby_space.extended_standby_mem_base);
    else
	super_standby_para_info.pextended_standby = 0;
#endif

    super_standby_para_info.timeout = time_to_wakeup;
    if(0 < super_standby_para_info.timeout){
	super_standby_para_info.event |= CPUS_WAKEUP_TIMEOUT;
#if defined(CONFIG_ARCH_SUN8IW10P1)
	super_standby_para_info.event |= CPU0_WAKEUP_TIMEOUT;
#endif
    }

    if(unlikely(debug_mask&PM_STANDBY_PRINT_STANDBY)){
	pr_info("total(def & dynamic config) standby wakeup src config: 0x%x.\n", super_standby_para_info.event);
	parse_wakeup_event(NULL, 0, super_standby_para_info.event, CPUS_ID);
    }

    if(standby_space.mem_size < sizeof(super_standby_para_info) + sizeof(*(extended_standby_manager_id->pextended_standby))){
	//judge the reserved space for mem para is enough or not.
	printk("ERR: reserved space is not enough for mem_para. \n");
	printk("need size: %lx. \n", (unsigned long)(sizeof(super_standby_para_info) + sizeof(*(extended_standby_manager_id->pextended_standby))));
        printk("reserved size: %lx. \n", (unsigned long)(standby_space.mem_size));
	return -1;
    }

    //clean all the data into dram
    memcpy((void *)phys_to_virt(standby_space.standby_mem_base), (void *)&super_standby_para_info, sizeof(super_standby_para_info));
    if ((NULL != extended_standby_manager_id) && (NULL != extended_standby_manager_id->pextended_standby))
	memcpy((void *)(phys_to_virt(standby_space.extended_standby_mem_base)),
		(void *)(extended_standby_manager_id->pextended_standby),
		sizeof(*(extended_standby_manager_id->pextended_standby)));
#if defined(CONFIG_ARCH_SUN8IW10P1)
     dmac_flush_range((void *)phys_to_virt(standby_space.standby_mem_base), (void *)(phys_to_virt(standby_space.standby_mem_base + standby_space.mem_size)));
#else
     __dma_flush_range((void *)phys_to_virt(standby_space.standby_mem_base), (void *)(phys_to_virt(standby_space.standby_mem_base + standby_space.mem_size)));
#endif
#if defined(CONFIG_ARCH_SUN8IW10P1)
       /* move standby code to sram */
       memcpy((void *)SRAM_FUNC_START, (void *)&standby_bin_start, (unsigned int)&standby_bin_end - (unsigned int)&standby_bin_start);
       dmac_flush_range((void *)SRAM_FUNC_START, (void *)(phys_to_virt(SRAM_FUNC_START + ((unsigned int)&standby_bin_end - (unsigned int)&standby_bin_start))));

       /* copy brom to dram address */
       //memcpy();

       mem_device_save();

       standby_info.standby_para.event = super_standby_para_info.event;
       standby_info.standby_para.gpio_enable_bitmap = super_standby_para_info.gpio_enable_bitmap;
       standby_info.standby_para.cpux_gpiog_bitmap = super_standby_para_info.cpux_gpiog_bitmap;
       standby_info.standby_para.pextended_standby =  super_standby_para_info.pextended_standby;
       standby_info.standby_para.timeout = super_standby_para_info.timeout;
       standby_info.standby_para.debug_mask = debug_mask;
#if defined(CONFIG_AW_AXP)
       get_pwr_regu_tree((unsigned int *)(standby_info.pmu_arg.soc_power_tree));
#endif
#endif
   save_pm_secure_mem_status(error_gen(MOD_SUSPEND_CPUXSYS, ERR_SUSPEND_CPUXSYS_CONFIG_SUPER_PARA_DONE));
   init_wakeup_src(super_standby_para_info.event);
   save_pm_secure_mem_status(error_gen(MOD_SUSPEND_CPUXSYS, ERR_SUSPEND_CPUXSYS_CONFIG_WAKEUP_SRC_DONE));
#ifdef CONFIG_CPU_OPS_SUNXI
   asm("wfi");
#elif defined(CONFIG_ARCH_SUN8IW10P1)
   cpu_pm_enter();
   //cpu_cluster_pm_enter();
   cpu_suspend((unsigned long)(&standby_info), aw_standby_enter);
   //cpu_cluster_pm_enter();
   cpu_pm_exit();

   /* report which wake source wakeup system */
   query_wakeup_source(&standby_info);
#else
   cpu_suspend(3);
#endif

    exit_wakeup_src(super_standby_para_info.event);
    save_pm_secure_mem_status(error_gen(MOD_RESUME_CPUXSYS, ERR_RESUME_CPUXSYS_CONFIG_WAKEUP_SRC_DONE));
#endif
#ifdef CONFIG_ARCH_SUN8IW10P1
    mem_device_restore();
#endif
    return 0;

}

/*
*********************************************************************************************************
*                           verify_restore
*
*Description: verify src and dest region is the same;
*
*Return     : 0: same;
*                -1: different;
*
*Notes      :
*********************************************************************************************************
*/
#ifdef VERIFY_RESTORE_STATUS
static int verify_restore(void *src, void *dest, int count)
{
	volatile char *s = (volatile char *)src;
	volatile char *d = (volatile char *)dest;

	while(count--){
		if(*(s+(count)) != *(d+(count))){
			//busy_waiting();
			return -1;
		}
	}

	return 0;
}
#endif

/*
*********************************************************************************************************
*                           aw_late_resume
*
*Description: prepare necessary info for suspend&resume;
*
*Return     : return 0 is process successed;
*
*Notes      :
*********************************************************************************************************
*/
static void aw_late_resume(void)
{
    aw_pm_dev_status("pio", PM_STANDBY_PRINT_RESUME_IO_STATUS);

    return;
}

/*
*********************************************************************************************************
*                           aw_super_standby
*
*Description: enter super standby;
*
*Return     : return 0 is process successed;
*
*Notes      :
*********************************************************************************************************
*/
static int aw_super_standby(suspend_state_t state)
{
	int result = 0;
	suspend_status_flag = 0;

	standby_level = STANDBY_WITH_POWER_OFF;
	mem_para_info.debug_mask = debug_mask;
	mem_para_info.suspend_delay_ms = suspend_delay_ms;

	/* config cpus wakeup event type */
	if(PM_SUSPEND_MEM == state || PM_SUSPEND_STANDBY == state){
		mem_para_info.axp_event = CPUS_MEM_WAKEUP;
	}else if(PM_SUSPEND_BOOTFAST == state){
		mem_para_info.axp_event = CPUS_BOOTFAST_WAKEUP;
	}
	
        save_pm_secure_mem_status(error_gen(MOD_SUSPEND_CPUXSYS, ERR_SUSPEND_CPUXSYS_CONFIG_MEM_PARA_DONE));
	result = aw_early_suspend();

	aw_late_resume();
	save_pm_secure_mem_status(error_gen(MOD_RESUME_CPUXSYS, ERR_RESUME_CPUXSYS_RESUME_DEVICES_DONE));

	return 0;

}


/*
 *********************************************************************************************************
 *                           aw_pm_enter
 *
 *Description: Enter the system sleep state;
 *
 *Arguments  : state     system sleep state;
 *
 *Return     : return 0 is process successed;
 *
 *Notes      : this function is the core function for platform sleep.
 *********************************************************************************************************
 */
static int aw_pm_enter(suspend_state_t state)
{
#ifdef CONFIG_ARCH_SUN8IW3P1
    int val = 0;
    int sram_backup = 0;
#endif

    PM_DBG("enter state %d\n", state);

    save_pm_secure_mem_status(error_gen(MOD_SUSPEND_CORE, 0));
    if(unlikely(0 == console_suspend_enabled)){
	debug_mask |= (PM_STANDBY_PRINT_RESUME | PM_STANDBY_PRINT_STANDBY);
    }else{
	debug_mask &= ~(PM_STANDBY_PRINT_RESUME | PM_STANDBY_PRINT_STANDBY);
    }

#ifdef CONFIG_ARCH_SUN8IW3P1
    //#if 0
    val = readl((const volatile void __iomem *)SRAM_CTRL_REG1_ADDR_VA);
    sram_backup = val;
    val |= 0x1000000;
    writel(val, (volatile void __iomem *)SRAM_CTRL_REG1_ADDR_VA);
#endif

    /* show device: cpux_io, cpus_io, ccu status */
    aw_pm_show_dev_status();

#if (defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW6P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1) || defined(CONFIG_ARCH_SUN50IW1P1)) && defined(CONFIG_AW_AXP)
    if(unlikely(debug_mask&PM_STANDBY_PRINT_PWR_STATUS)){
	printk(KERN_INFO "power status as follow:");
	axp_regulator_dump();	
    }

    /* check pwr usage info: change sys_id according module usage info.*/
    check_pwr_status();
    /* config sys_pwr_dm according to the sysconfig */
    config_sys_pwr();
#endif

    extended_standby_manager_id = get_extended_standby_manager();
    extended_standby_show_state();
    save_pm_secure_mem_status(error_gen(MOD_SUSPEND_CPUXSYS, ERR_SUSPEND_CPUXSYS_SHOW_DEVICES_STATE_DONE));

    aw_super_standby(state);

#ifdef CONFIG_SUNXI_ARISC
    arisc_query_wakeup_source(&mem_para_info.axp_event);
    PM_DBG("platform wakeup, super standby wakesource is:0x%x\n", mem_para_info.axp_event);	
#else
    mem_para_info.axp_event = standby_info.standby_para.event;
    PM_DBG("platform wakeup, super standby wakesource is:0x%x\n", mem_para_info.axp_event);
    parse_wakeup_event(NULL, 0, mem_para_info.axp_event, CPU0_ID);
#endif

    if (mem_para_info.axp_event & (CPUS_WAKEUP_LONG_KEY)) {
#ifdef	CONFIG_AW_AXP
	axp_powerkey_set(0);
#endif
    }
    parse_wakeup_event(NULL, 0, mem_para_info.axp_event, CPUS_ID);

    /* show device: cpux_io, cpus_io, ccu status */
    aw_pm_show_dev_status();


#if (defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW6P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1) || defined(CONFIG_ARCH_SUN50IW1P1)) && defined(CONFIG_AW_AXP)
    resume_sys_pwr_state();
#endif

#ifdef CONFIG_ARCH_SUN8IW3P1	
    //#if 0
    writel(sram_backup,(volatile void __iomem *)SRAM_CTRL_REG1_ADDR_VA);
#endif
    save_pm_secure_mem_status(error_gen(MOD_RESUME_CPUXSYS, ERR_RESUME_CPUXSYS_RESUME_CPUXSYS_DONE));
    return 0;
}


/*
 *********************************************************************************************************
 *                           aw_pm_wake
 *
 *Description: platform wakeup;
 *
 *Arguments  : none;
 *
 *Return     : none;
 *
 *Notes      : This function called when the system has just left a sleep state, right after
 *             the nonboot CPUs have been enabled and before device drivers' early resume
 *             callbacks are executed. This function is opposited to the aw_pm_prepare_late;
 *********************************************************************************************************
 */
static void aw_pm_wake(void)
{
    save_pm_secure_mem_status(error_gen(MOD_RESUME_PROCESSORS,0));
    PM_DBG("%s \n", __func__);
    return;
}

/*
*********************************************************************************************************
*                           aw_pm_finish
*
*Description: Finish wake-up of the platform;
*
*Arguments  : none
*
*Return     : none
*
*Notes      : This function is called right prior to calling device drivers' regular suspend
*              callbacks. This function is opposited to the aw_pm_prepare function.
*********************************************************************************************************
*/
static void aw_pm_finish(void)
{
#ifdef CONFIG_CPU_FREQ	
    struct cpufreq_policy policy;
#endif
    save_pm_secure_mem_status(error_gen(MOD_RESUME_DEVICES, ERR_RESUME_DEVICES_EARLY_RESUME_DEVICES_DONE));
    PM_DBG("platform wakeup finish\n");

#ifdef CONFIG_CPU_FREQ	
    if (cpufreq_get_policy(&policy, 0))
	goto out;

    cpufreq_driver_target(&policy, policy.max, CPUFREQ_RELATION_L);
#endif
    return ;

#ifdef CONFIG_CPU_FREQ	
out:
    return ;
#endif
}


/*
*********************************************************************************************************
*                           aw_pm_end
*
*Description: Notify the platform that system is in work mode now.
*
*Arguments  : none
*
*Return     : none
*
*Notes      : This function is called by the PM core right after resuming devices, to indicate to
*             the platform that the system has returned to the working state or
*             the transition to the sleep state has been aborted. This function is opposited to
*             aw_pm_begin function.
*********************************************************************************************************
*/
static void aw_pm_end(void)
{
    save_pm_secure_mem_status(error_gen(MOD_RESUME_DEVICES, ERR_RESUME_DEVICES_RESUME_DEVICES_DONE));
    PM_DBG("aw_pm_end!\n");

    save_pm_secure_mem_status(error_gen(MOD_RESUME_COMPLETE_FLAG, 0));
    return ;
}


/*
*********************************************************************************************************
*                           aw_pm_recover
*
*Description: Recover platform from a suspend failure;
*
*Arguments  : none
*
*Return     : none
*
*Notes      : This function alled by the PM core if the suspending of devices fails.
*             This callback is optional and should only be implemented by platforms
*             which require special recovery actions in that situation.
*********************************************************************************************************
*/
static void aw_pm_recover(void)
{
    save_pm_secure_mem_status(error_gen(MOD_SUSPEND_DEVICES, ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_FAILED ));
    PM_DBG("aw_pm_recover\n");
}


/*
    define platform_suspend_ops which is registered into PM core.
*/
static struct platform_suspend_ops aw_pm_ops = {
    .valid = aw_pm_valid,
    .begin = aw_pm_begin,
    .prepare = aw_pm_prepare,
    .prepare_late = aw_pm_prepare_late,
    .enter = aw_pm_enter,
    .wake = aw_pm_wake,
    .finish = aw_pm_finish,
    .end = aw_pm_end,
    .recover = aw_pm_recover,
};

static DEVICE_ATTR(debug_mask, S_IRUGO|S_IWUSR|S_IWGRP,
		debug_mask_show, debug_mask_store);

static DEVICE_ATTR(parse_status_code, S_IRUGO|S_IWUSR|S_IWGRP,
		parse_status_code_show, parse_status_code_store);

static struct attribute * g[] = {
	&dev_attr_debug_mask.attr,
	&dev_attr_parse_status_code.attr,
	NULL,
};
static struct attribute_group attr_group = {
	.attrs = g,
};

#if (defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW6P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1) || defined(CONFIG_ARCH_SUN50IW1P1)) && defined(CONFIG_AW_AXP)
static int config_pmux_para(unsigned num)
{
#define PM_NAME_LEN (25)
    char name[PM_NAME_LEN] = "\0";
    int enable = 0;
    int pmux_id = 0;
    int pmux_twi_id = 0;
    int pmux_twi_addr = 0;
    struct device_node *np;
    
    sprintf(name, "pmu%d", num);
    printk("pmu name: %s .\n", name);

    //get pmu config
    np = of_find_node_by_type(NULL, name);
    if(NULL == np){
	printk(KERN_ERR "Warning: can not find np for %s. \n", name);
    } else{
	if (!of_device_is_available(np)) {
	    enable = 0;
	} else{
	    enable = 1;
	}
	printk(KERN_INFO "%s_enable = 0x%x. \n", name, enable);

	if(1 == enable){
	    if(of_property_read_u32(np, "pmu_id", &pmux_id)){
		pr_err("Warning: %s fetch pmu_id err. \n", __func__);
		pmux_id = 0;
	    }
	    printk(KERN_INFO "pmux_id = 0x%x. \n", pmux_id);
	    extended_standby_set_pmu_id(num, pmux_id);

	    if(of_property_read_u32(np, "pmu_twi_id", &pmux_twi_id)){
		pr_err("Warning: %s fetch pmu_twi_id err. \n", __func__);
		standby_info.pmu_arg.twi_port = 0;
	    }else{
		standby_info.pmu_arg.twi_port = pmux_twi_id;
	    }
	    printk(KERN_INFO "pmux_twi_id = 0x%x. \n", standby_info.pmu_arg.twi_port);

	    if(of_property_read_u32(np, "pmu_twi_addr", &pmux_twi_addr)){
		pr_err("Warning: %s: fetch pmu_twi_addr err. \n", __func__);
		standby_info.pmu_arg.dev_addr = 0x34;
	    }else{
		standby_info.pmu_arg.dev_addr = pmux_twi_addr;
	    }
	    printk(KERN_INFO "pmux_twi_addr = 0x%x. \n", standby_info.pmu_arg.dev_addr);
	}

    }
    
    return 0;
}

static int config_pmu_para(void)
{
    config_pmux_para(0);
    config_pmux_para(1);
    return 0;
}



static int config_dynamic_standby(void)
{
    aw_power_scene_e type = SCENE_DYNAMIC_STANDBY;
    scene_extended_standby_t *local_standby;
    int enable = 0;
    int dram_selfresh_flag = 1;
    unsigned int vdd_cpua_vol = 0;
    unsigned int vdd_sys_vol = 0;
    struct device_node *np;
    char *name = "dynamic_standby_para";
    int i = 0;
    int ret = 0;

    //get customer customized dynamic_standby config
    np = of_find_node_by_type(NULL, name);
    if(NULL == np){
	printk(KERN_ERR "Warning: can not find np for %s. \n", name);
    }else{
	if (!of_device_is_available(np)) {
	    enable = 0;
	} else{
	    enable = 1;
	}
	printk(KERN_INFO "Warning: %s_enable = 0x%x. \n", name, enable);

	if(1 == enable){
	    for (i = 0; i < extended_standby_cnt; i++) {
		if (type == extended_standby[i].scene_type) {
		    //config dram_selfresh flag;
		    local_standby = &(extended_standby[i]);
		    if(of_property_read_u32(np, "dram_selfresh_flag", &dram_selfresh_flag)){
			printk(KERN_ERR "%s: fetch dram_selfresh_flag err. \n", __func__);
			dram_selfresh_flag = 1;
		    }
		    printk(KERN_INFO "dynamic_standby dram selfresh flag = 0x%x. \n", dram_selfresh_flag);
		    if(0 == dram_selfresh_flag){
			local_standby->soc_pwr_dep.soc_dram_state.selfresh_flag     = dram_selfresh_flag;
			local_standby->soc_pwr_dep.soc_pwr_dm_state.state |= BITMAP(VDD_SYS_BIT);
			local_standby->soc_pwr_dep.cpux_clk_state.osc_en         |= 0xf;	// mean all osc is on.
			//mean pll5 is shutdowned & open by dram driver. 
			//hsic can't closed.  
			//periph is needed.
			local_standby->soc_pwr_dep.cpux_clk_state.init_pll_dis   |= (BITMAP(PM_PLL_HSIC) | BITMAP(PM_PLL_PERIPH) | BITMAP(PM_PLL_DRAM));
		    }

		    //config other flag?
		}

		//config other extended_standby?
	    }


	    if(of_property_read_u32(np, "vdd_cpua_vol", &vdd_cpua_vol)){
	    }else{
		printk(KERN_INFO "vdd_cpua_vol = 0x%x. \n", vdd_cpua_vol);
		ret = scene_set_volt(SCENE_DYNAMIC_STANDBY, VDD_CPUA_BIT, vdd_cpua_vol);
		if (ret < 0)
		    printk(KERN_ERR "%s: set vdd_cpua volt failed\n", __func__);

	    }	

	    if(of_property_read_u32(np, "vdd_sys_vol", &vdd_sys_vol)){
	    }else{
		printk(KERN_INFO "vdd_sys_vol = 0x%x. \n", vdd_sys_vol);
		ret = scene_set_volt(SCENE_DYNAMIC_STANDBY, VDD_SYS_BIT, vdd_sys_vol);
		if (ret < 0)
		    printk(KERN_ERR "%s: set vdd_sys volt failed\n", __func__);
	    }
	    
	    printk(KERN_INFO "enable dynamic_standby by customer.\n");
	    scene_lock_store(NULL, NULL, "dynamic_standby", 0);

	}
    }

    
    return 0;
}

static int config_sys_pwr_dm(struct device_node *np, char *pwr_dm)
{
    int dm_enable = 0;

    if(of_property_read_u32(np, pwr_dm, &dm_enable)){
    }else{
	printk("%s: dm_enalbe: %d. \n", pwr_dm, dm_enable);
	if(0 == dm_enable) {
	    del_sys_pwr_dm(pwr_dm);	
	    save_sys_pwr_state(pwr_dm);
	}else{
	    add_sys_pwr_dm(pwr_dm);	
	}
    }

    return 0;
}

static int config_sys_pwr(void)
{
    unsigned int sys_mask = 0;
    struct device_node *np;
    char *name = "sys_pwr_dm_para";
    
    np = of_find_node_by_type(NULL, name);
    if(NULL == np){
	printk(KERN_INFO "info: can not find np for %s. \n", name);
    }else{
	config_sys_pwr_dm(np, "vdd-cpua");
	config_sys_pwr_dm(np, "vdd-cpub");
	config_sys_pwr_dm(np, "vcc-dram");
	config_sys_pwr_dm(np, "vdd-gpu");
	config_sys_pwr_dm(np, "vdd-sys");
	config_sys_pwr_dm(np, "vdd-vpu");
	config_sys_pwr_dm(np, "vdd-cpus");
	config_sys_pwr_dm(np, "vdd-drampll");
	config_sys_pwr_dm(np, "vcc-lpddr");
	config_sys_pwr_dm(np, "vcc-adc");
	config_sys_pwr_dm(np, "vcc-pl");
	config_sys_pwr_dm(np, "vcc-pm");
	config_sys_pwr_dm(np, "vcc-io");
	config_sys_pwr_dm(np, "vcc-cpvdd");
	config_sys_pwr_dm(np, "vcc-ldoin");
	config_sys_pwr_dm(np, "vcc-pll");
    }

    sys_mask = get_sys_pwr_dm_mask();
    printk(KERN_INFO "after customized: sys_mask config = 0x%x. \n", sys_mask);

    return 0;
}
#endif

/*
*********************************************************************************************************
*                           aw_pm_init
*
*Description: initial pm sub-system for platform;
*
*Arguments  : none;
*
*Return     : result;
*
*Notes      :
*
*********************************************************************************************************
*/
static int __init aw_pm_init(void)
{
	int ret = 0;
	u32 value[3] = {0, 0, 0};	
#if 0
	struct device_node *sram_a1_np;
	void __iomem *sram_a1_vbase;
	struct resource res;
	phys_addr_t pbase;
	size_t size;
#endif

	PM_DBG("aw_pm_init!\n");

	/*init debug state*/
	pm_secure_mem_status_init("rtc");
	/*for auto test reason.*/
	//*(volatile __u32 *)(STANDBY_STATUS_REG  + 0x08) = BOOT_UPGRADE_FLAG;

#if (defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW6P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1) || defined(CONFIG_ARCH_SUN50IW1P1)) && defined(CONFIG_AW_AXP)
	config_pmu_para();
	/*init sys_pwr_dm*/
	init_sys_pwr_dm();
	config_dynamic_standby();
#endif


	standby_space.np = of_find_compatible_node(NULL, NULL, "allwinner,standby_space");
	if (IS_ERR(standby_space.np)) {
	    printk(KERN_ERR "get [allwinner,standby_space] device node error\n");
	    return -EINVAL;
	}
	
	ret = of_property_read_u32_array(standby_space.np, "space1", value, ARRAY_SIZE(value));
	if (ret){
	    printk(KERN_ERR "get standby_space1 err. \n");	
	    return -EINVAL;
	}

#if 0
	sram_a1_np = of_find_compatible_node(NULL, NULL, "allwinner,sram_a1");
	if (IS_ERR(sram_a1_np)) {
	    printk(KERN_ERR "get [allwinner,sram_a1] device node error\n");
	    return -EINVAL;
	}

	ret = of_address_to_resource(sram_a1_np, 0, &res);
	if (ret || !res.start) {
		 printk(KERN_ERR "get sram_a1 pbase error\n");
		return -EINVAL;
	}
	pbase = res.start;
	size = resource_size(&res);
	printk("%s: sram_a1_pbase=0x%x , size=0x%x\n", __func__, (unsigned int)pbase, size);
	sram_a1_vbase = of_iomap(sram_a1_np, 0);
	if (!sram_a1_vbase)
		panic("Can't map sram_a1 registers");
	printk("%s: sram_a1_vbase=0x%x\n", __func__, (unsigned int)sram_a1_vbase);
#endif
	standby_space.standby_mem_base = (phys_addr_t)value[0];
	standby_space.extended_standby_mem_base = (phys_addr_t)value[0] + 0x400;    //1K bytes offset
	standby_space.mem_offset = (phys_addr_t)value[1];
	standby_space.mem_size = (size_t)value[2];

	suspend_set_ops(&aw_pm_ops);
	aw_pm_kobj = kobject_create_and_add("aw_pm", power_kobj);
	if (!aw_pm_kobj)
		return -ENOMEM;
	ret = sysfs_create_group(aw_pm_kobj, &attr_group);
	
	return ret ? ret : 0;
}

/*
*********************************************************************************************************
*                           aw_pm_exit
*
*Description: exit pm sub-system on platform;
*
*Arguments  : none
*
*Return     : none
*
*Notes      :
*
*********************************************************************************************************
*/
static void __exit aw_pm_exit(void)
{
	PM_DBG("aw_pm_exit!\n");
	suspend_set_ops(NULL);
}

module_param_named(suspend_freq, suspend_freq, int, S_IRUGO | S_IWUSR);
module_param_named(suspend_delay_ms, suspend_delay_ms, int, S_IRUGO | S_IWUSR);
module_param_named(standby_mode, standby_mode, int, S_IRUGO | S_IWUSR);
module_param_named(time_to_wakeup, time_to_wakeup, ulong, S_IRUGO | S_IWUSR);
subsys_initcall_sync(aw_pm_init);
module_exit(aw_pm_exit);

