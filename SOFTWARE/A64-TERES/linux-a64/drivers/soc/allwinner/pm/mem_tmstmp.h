/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2011-2015, gq.yang China
*                                             All Rights Reserved
*
* File    : mem_tmstmp.h
* By      : gq.yang
* Version : v1.0
* Date    : 2012-11-31 15:23
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_TMSTMP_H__
#define __MEM_TMSTMP_H__

typedef struct __MEM_TMSTMP_REG
{
	// offset:0x00
	volatile __u32   Ctl;
	volatile __u32   reserved0[7];
	// offset:0x20
	volatile __u32   Cluster0CtrlReg1;

} __mem_tmstmp_reg_t;

//for super standby;
__s32 mem_tmstmp_save(__mem_tmstmp_reg_t *ptmstmp_state);
__s32 mem_tmstmp_restore(__mem_tmstmp_reg_t *ptmstmp_state);

#endif  //__MEM_TMSTMP_H__

