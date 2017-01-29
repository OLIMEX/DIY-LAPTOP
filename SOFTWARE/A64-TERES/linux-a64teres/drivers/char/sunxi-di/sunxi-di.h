#ifndef _SUNXI_DI_H
#define _SUNXI_DI_H

#include <linux/types.h>
#include "di.h"

//#define DI_RESERVED_MEM

#define DI_MODULE_NAME "deinterlace"
#define DI_TIMEOUT                      30                    /* DI-Interlace 30ms timeout */
#define DI_MODULE_TIMEOUT               0x1055
#define FLAG_WIDTH                      (2048)
#define FLAG_HIGH                       (1100)

typedef struct {
	void __iomem *base_addr;
	__di_mem_t mem_in_params;
	__di_mem_t mem_out_params;
	atomic_t     di_complete;
	atomic_t     enable;
	wait_queue_head_t wait;
	void *in_flag_phy;
	void *out_flag_phy;
	size_t  flag_size;
	u32  irq_number;
	u32  time_value;
#ifdef CONFIG_PM
	struct dev_pm_domain di_pm_domain;
#endif
}di_struct, *pdi_struct;

#define	DI_IOC_MAGIC		'D'
#define	DI_IOCSTART		_IOWR(DI_IOC_MAGIC, 0, __di_para_t *)

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_INT = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
	DEBUG_TEST = 1U << 4,
};

#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	 printk(KERN_DEBUG fmt , ## arg)

#endif

