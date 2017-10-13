/*
 * Hibernation support specific for ARM
 *
 * Copyright (C) 2010 Nokia Corporation
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Copyright (C) 2006 Rafael J. Wysocki <rjw <at> sisk.pl>
 *
 * Contact: Hiroshi DOYU <Hiroshi.DOYU <at> nokia.com>
 *
 * License terms: GNU General Public License (GPL) version 2
 */
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/major.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/power/aw_pm.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/pgtable.h>
#include "pm_o.h"

/*
 * "Linux" PTE definitions.
 *
 * We keep two sets of PTEs - the hardware and the linux version.
 * This allows greater flexibility in the way we map the Linux bits
 * onto the hardware tables, and allows us to have YOUNG and DIRTY
 * bits.
 *
 * The PTE table pointer refers to the hardware entries; the "Linux"
 * entries are stored 1024 bytes below.
 */

#define L_PTE_WRITE		(1 << 7)
#define L_PTE_EXEC		(1 << 9)
#define PAGE_TBL_ADDR 		(0xc0004000)

struct saved_mmu_level_one {
	u32 vaddr;
	u32 entry_val;
};

static struct saved_mmu_level_one backup_tbl[1];

/*
 * Create the page directory entries for 0x0000,0000 <-> 0x0000,0000
 */
void create_mapping(void)
{
	*((volatile __u32 *)(PAGE_TBL_ADDR)) = 0xc4a;
	
	/** 
	 * clean dcache,, invalidat icache&invalidate tlb,
	 *
	 * function: 
	 *	to make sure the correct PA will be access.
	 * 
	 * cache:
	 * 	clean cache unit: is cache line size;
	 * 	whether the end addr will be flush? exclusive,not including.
	 *
	 * tlb:
	 * 	invalidate tlb unit: PAGE_SIZE;
	 * 	Not including end addr.
	 *
	 * Note: 
	 * 	actually, because the PA will be used at resume period time, 
	 * 	mean, not use immediately, 
	 * 	and, the cache will be clean at the end.
	 * 	so, the clean&invalidate is not necessary.
	 * 	do this here, just in case testing. like: jump to resume code for testing.
	**/

	//Note: 0xc000,0000,is device area; not need to flush cache.
	//ref: ./arch/arm/kernel/head.S
	//__cpuc_coherent_kern_range((unsigned long)(PAGE_TBL_ADDR), (unsigned long)(PAGE_TBL_ADDR + (sizeof(u32))));
	//local_flush_tlb_kernel_range((unsigned long)(PAGE_TBL_ADDR), (unsigned long)(PAGE_TBL_ADDR + (sizeof(u32))));
	return;
}

/**save the va: 0x0000,0000 mapping. 
*@vaddr: the va of mmu mapping to save;
*/
void save_mapping(unsigned long vaddr)
{
	unsigned long addr;

	//busy_waiting();
	addr = vaddr & PAGE_MASK;

	//__cpuc_flush_kern_all();
	backup_tbl[0].vaddr = addr;
	backup_tbl[0].entry_val = *((volatile __u32 *)(PAGE_TBL_ADDR));
	//flush_tlb_all();

	return;
}

/**restore the va: 0x0000,0000 mapping. 
*@vaddr: the va of mmu mapping to restore.
*
*/
void restore_mapping(unsigned long vaddr)
{
	unsigned long addr;
	
	addr = vaddr & PAGE_MASK;
	
	if(addr != backup_tbl[0].vaddr){
		while(1);
	}

	*((volatile __u32 *)(PAGE_TBL_ADDR)) = backup_tbl[0].entry_val;
	//clean dcache, invalidat icache
	//__cpuc_coherent_kern_range((unsigned long)(PAGE_TBL_ADDR), (unsigned long)(PAGE_TBL_ADDR + (sizeof(u32))));
	//	flust tlb after change mmu mapping.
//	local_flush_tlb_kernel_range((unsigned long)(PAGE_TBL_ADDR), (unsigned long)(PAGE_TBL_ADDR + (sizeof(u32))));

	return;
}

