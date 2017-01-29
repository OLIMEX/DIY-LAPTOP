/*
 *  arch/arm/mach-sunxi/arisc/include/arisc_includes.h
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-10-01 Written by superm (superm@allwinnertech.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ARISC_INCLUDES_H
#define __ARISC_INCLUDES_H

//#include <linux/kernel.h>
//#include <linux/module.h>
//#include <linux/string.h>
//#include <linux/spinlock.h>
//#include <linux/err.h>
//#include <linux/io.h>
//#include <linux/slab.h>
//#include <linux/semaphore.h>
//#include <linux/interrupt.h>
//#include <linux/jiffies.h>
//#include <linux/delay.h>
//#include <linux/arisc/hwmsgbox.h>
//#include <linux/arisc/hwspinlock.h>
#define	CONFIG_ARCH_SUN50IW1P1

#include <arch_helpers.h>
#include <platform.h>
#include <platform_def.h>
#include <bakery_lock.h>
#include <mmio.h>
#include <debug.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <psci.h>
#include <bakery_lock.h>
#include <spinlock.h>
#include <arisc.h>

#include "../../sunxi_private.h"
#include "../../sunxi_cpu_ops.h"
#include "../../sunxi_def.h"
#include "../../mhu.h"
#include "../../scpi.h"
#include "../../sun50iw1p1.h"

/* configure and debugger */
#include "../arisc_i.h"
#include "./arisc_cfgs.h"
#include "./arisc_dbgs.h"
#include "./arisc_para.h"

/* messages define */
#include "./arisc_messages.h"
#include "./arisc_message_manager.h"

/* driver headers */
#include "./arisc_hwmsgbox.h"
#include "./arisc_hwspinlock.h"

#define readl(x)     mmio_read_32((x))
#define writel(v, a) mmio_write_32((a), (v))

/* global functions */
extern int arisc_axp_int_notify(struct arisc_message *pmessage);
extern int arisc_audio_perdone_notify(struct arisc_message *pmessage);
extern int arisc_dvfs_cfg_vf_table(void);
extern int arisc_query_set_standby_info(struct standby_info_para *para, arisc_rw_type_e op);
extern int arisc_sysconfig_sstpower_paras(void);
extern int arisc_report_error_info(struct arisc_message *pmessage);

/* global vars */
extern unsigned long arisc_sram_a2_base;
extern struct dts_cfg dts_cfg;

#endif /* __ARISC_INCLUDES_H */
