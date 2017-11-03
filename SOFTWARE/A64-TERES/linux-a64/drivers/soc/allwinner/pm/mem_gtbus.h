/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2011-2015, gq.yang China
*                                             All Rights Reserved
*
* File    : mem_gtbus.h
* By      : gq.yang
* Version : v1.0
* Date    : 2013-08-31 15:23
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_GTBUS_H__
#define __MEM_GTBUS_H__
#include "pm.h"

//total size: 0x804 bytes size.
typedef struct gtbus_reg_list{	
	__u32 master_config_reg[36] 			;	//0x00 - 0x8c
	__u32 reserved0[28]				;	//0x90 - 0xfc
	__u32 band_win_config_reg 			;	//0x100, qos_hpr?
	__u32 master_rd_pri_config_reg_0 		;	//0x104
	__u32 master_rd_pri_config_reg_1 		;	//0x108
	__u32 config_reg				;	//0x10c, level2 arbiter?
	__u32 soft_clk_on_reg				;	//0x110, this two reg, which is major? 
	__u32 soft_clk_off_reg				;	//0x114, gtb_gmb?
	__u32 reserved1					;	//0x118
	__u32 pmu_en_reg				;	//0x11c
	__u32 pmu_counter_reg[19]			;	//0x120 - 0x168, r(ead,only)
	__u32 reserved2[37]				;	//0x16c-0x1fc
	__u32 cci400_config_reg_0			;	//0x200
	__u32 cci400_config_reg_1			;	//0x204, ac channel?
	__u32 cci400_config_reg_2			;	//0x208, qvn? prealloc wm?
	__u32 cci400_status_reg_0			;	//0x20c, r, awqos?
	__u32 cci400_status_reg_1			;	//0x210, r
	__u32 reserved3[379]				;	//0x214-0x7fc
	__u32 ram_bist_config				;	//0x800
} gtbus_reg_list_t;	

#define GTBUS_REG_BACKUP_LENGTH		(47)
struct gtbus_state{
	gtbus_reg_list_t		*gtbus_reg;
	__u32				gtbus_reg_backup[GTBUS_REG_BACKUP_LENGTH];	
};

__s32 mem_gtbus_save(struct gtbus_state *gtbus_state);
__s32 mem_gtbus_restore(struct gtbus_state *gtbus_state);

#endif  //__MEM_GTBUS_H__

