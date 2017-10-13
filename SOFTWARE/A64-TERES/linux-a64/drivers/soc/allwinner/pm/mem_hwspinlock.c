#include "pm_i.h"

#if defined CONFIG_ARCH_SUN9IW1P1 || defined CONFIG_ARCH_SUN8IW6P1 
//#if defined CONFIG_ARCH_SUN9IW1P1

#define readb(addr)		(*((volatile unsigned char  *)(addr)))
#define readw(addr)		(*((volatile unsigned short *)(addr)))
#define readl(addr)		(*((volatile unsigned long  *)(addr)))
#define writeb(v, addr)		(*((volatile unsigned char  *)(addr)) = (unsigned char)(v))
#define writew(v, addr)		(*((volatile unsigned short *)(addr)) = (unsigned short)(v))
#define writel(v, addr)		(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

static __u32 cpsr_backup = 0;

/*
 * CPU interrupt mask handling.
 */

static inline unsigned long mem_local_irq_save(void)
{
	unsigned long flags;

	asm volatile(
		"	mrs	%0, cpsr	@ mem_local_irq_save\n"
		"	cpsid	i"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

static inline void mem_local_irq_enable(void)
{
	asm volatile(
		"	cpsie i			@ mem_local_irq_enable"
		:
		:
		: "memory", "cc");
}

static inline void mem_local_irq_disable(void)
{
	asm volatile(
		"	cpsid i			@ mem_local_irq_disable"
		:
		:
		: "memory", "cc");
}

/*
 * restore saved IRQ & FIQ state
 */
static inline void mem_local_irq_restore(unsigned long flags)
{
	asm volatile(
		"	msr	cpsr_c, %0	@ local_irq_restore"
		:
		: "r" (flags)
		: "memory", "cc");
}

/*
*********************************************************************************************************
*                                       INITIALIZE HWSPINLOCK
*
* Description:  initialize hwspinlock.
*
* Arguments  :  none.
*
* Returns    :  0 if initialize hwspinlock succeeded, others if failed.
*********************************************************************************************************
*/
__s32 hwspinlock_init(__u32 mmu_flag)
{
	if(1 == mmu_flag){
	    //enable SPINLOCK clock and set reset as de-assert state.
	    writel((CCU_CLK_NRESET | readl(IO_ADDRESS(CCU_MOD_CLK_AHB1_RESET_SPINLOCK))), IO_ADDRESS(CCU_MOD_CLK_AHB1_RESET_SPINLOCK));
	    writel((CCU_CLK_ON | readl(IO_ADDRESS(CCU_MOD_CLK_AHB1_GATING_SPINLOCK))), IO_ADDRESS(CCU_MOD_CLK_AHB1_GATING_SPINLOCK));
	
	}else{
	    //enable SPINLOCK clock and set reset as de-assert state.
	    writel((CCU_CLK_NRESET | readl(CCU_MOD_CLK_AHB1_RESET_SPINLOCK)), CCU_MOD_CLK_AHB1_RESET_SPINLOCK);
	    writel((CCU_CLK_ON | readl(CCU_MOD_CLK_AHB1_GATING_SPINLOCK)), CCU_MOD_CLK_AHB1_GATING_SPINLOCK);
	}
	return 0;
}

/*
*********************************************************************************************************
*                                               LOCK HWSPINLOCK WITH TIMEOUT
*
* Description:  lock an hwspinlock with timeout limit.
*
* Arguments  :  hwid : an hwspinlock id which we want to lock.
*
* Returns    :  0 if lock hwspinlock succeeded, other if failed.
*********************************************************************************************************
*/
__s32 hwspin_lock_timeout(__u32 hwid, __u32 timeout)
{
	//multipy 1024.
	__u32 expire = timeout<<10;

	//disable cpu interrupt, save cpsr to cpsr-table.
	cpsr_backup = mem_local_irq_save();

	//try to take spinlock
	while (readl(IO_ADDRESS(MEM_SPINLOCK_LOCK_REG(hwid))) == MEM_SPINLOCK_TAKEN)
	{
		//spinlock is busy
		if (expire == 0)
		{
			mem_local_irq_restore(cpsr_backup);
			printk("take hwspinlock timeout\n");
			return -1;
		}
		expire--;
	}

	return 0;
}

/*
*********************************************************************************************************
*                                               UNLOCK HWSPINLOCK
*
* Description:  unlock a specific hwspinlock.
*
* Arguments  :  hwid : an hwspinlock id which we want to unlock.
*
* Returns    :  0 if unlock hwspinlock succeeded, other if failed.
*********************************************************************************************************
*/
__s32 hwspin_unlock(__u32 hwid)
{
	//untaken the spinlock
	writel(0x0, IO_ADDRESS(MEM_SPINLOCK_LOCK_REG(hwid)));

	//restore cpsr
	mem_local_irq_restore(cpsr_backup);

	return 0;
}

/*
*********************************************************************************************************
*                                               LOCK HWSPINLOCK WITH TIMEOUT
*
* Description:  lock an hwspinlock with timeout limit.
*
* Arguments  :  hwid : an hwspinlock id which we want to lock.
*
* Returns    :  0 if lock hwspinlock succeeded, other if failed.
*********************************************************************************************************
*/
__s32 hwspin_lock_timeout_nommu(__u32 hwid, __u32 timeout)
{
	//multipy 1024.
	__u32 expire = timeout<<10;

	//disable cpu interrupt, save cpsr to cpsr-table.
	cpsr_backup = mem_local_irq_save();

	//try to take spinlock
	while (readl(MEM_SPINLOCK_LOCK_REG(hwid)) == MEM_SPINLOCK_TAKEN)
	{
		//spinlock is busy
		if (expire == 0)
		{
			mem_local_irq_restore(cpsr_backup);
			//printk_nommu("take hwspinlock timeout\n");
			return -1;
		}
		expire--;
	}

	return 0;
}

/*
*********************************************************************************************************
*                                               UNLOCK HWSPINLOCK
*
* Description:  unlock a specific hwspinlock.
*
* Arguments  :  hwid : an hwspinlock id which we want to unlock.
*
* Returns    :  0 if unlock hwspinlock succeeded, other if failed.
*********************************************************************************************************
*/
__s32 hwspin_unlock_nommu(__u32 hwid)
{
	//untaken the spinlock
	writel(0x0, MEM_SPINLOCK_LOCK_REG(hwid));

	//restore cpsr
	mem_local_irq_restore(cpsr_backup);

	return 0;
}

#endif

