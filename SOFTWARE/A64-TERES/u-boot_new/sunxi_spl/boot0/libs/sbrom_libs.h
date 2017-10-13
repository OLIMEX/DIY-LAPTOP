/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#ifndef  __SBORM_LIBS_H__
#define  __SBORM_LIBS_H__


extern void mmu_setup(void);
extern void mmu_turn_off(void);

extern int create_heap(unsigned int pHeapHead, unsigned int nHeapSize);

extern unsigned int go_exec (unsigned int run_addr, unsigned int para_addr, int out_secure);

void boot0_jump(unsigned int addr);

#endif

