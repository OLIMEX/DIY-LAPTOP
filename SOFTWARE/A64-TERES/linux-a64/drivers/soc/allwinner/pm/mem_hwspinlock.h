#ifndef _MEM_HWSPINLOCK_H
#define _MEM_HWSPINLOCK_H

//#if defined CONFIG_ARCH_SUN9IW1P1
#include "pm_config.h"

#define	MEM_MSG_HWSPINLOCK         (0)
#define	MEM_AUDIO_HWSPINLOCK       (1)
#define	MEM_RTC_REG_HWSPINLOCK     (2)

//state of spinlock: taken or not taken.
#define   MEM_SPINLOCK_NOTTAKEN      (0)
#define   MEM_SPINLOCK_TAKEN         (1)

//
//hardware spinlock register list
#define	MEM_SPINLOCK_SYS_STATUS_REG		(AW_SPINLOCK_BASE + 0x0000)
#define	MEM_SPINLOCK_STATUS_REG			(AW_SPINLOCK_BASE + 0x0010)
#define	MEM_SPINLOCK_IRQ_EN_REG			(AW_SPINLOCK_BASE + 0x0020)
#define MEM_SPINLOCK_IRQ_PEND_REG		(AW_SPINLOCK_BASE + 0x0040)
#define MEM_SPINLOCK_LOCK_REG(id)		(AW_SPINLOCK_BASE + 0x0100 + id * 4)

__s32 hwspinlock_init(__u32 mmu_flag);
__s32 hwspin_lock_timeout_nommu(__u32 hwid, __u32 timeout);
__s32 hwspin_unlock_nommu(__u32 hwid);
__s32 hwspin_lock_timeout(__u32 hwid, __u32 timeout);
__s32 hwspin_unlock(__u32 hwid);

#endif /*_MEM_HWSPINLOCK_H*/

