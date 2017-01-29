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

#ifndef _SS_H_
#define _SS_H_

#include <asm/arch/platform.h>

#define SS_N_BASE			   SUNXI_SS_BASE      //non security
#define SS_S_BASE             (SUNXI_SS_BASE+0x800)      //security

#define SS_TDQ				  (SS_N_BASE + 0x00 + 0x800*ss_base_mode)
#define SS_CTR				  (SS_N_BASE + 0x04 + 0x800*ss_base_mode)
#define SS_ICR				  (SS_N_BASE + 0x08 + 0x800*ss_base_mode)
#define SS_ISR				  (SS_N_BASE + 0x0C + 0x800*ss_base_mode)
#define SS_TLR                (SS_N_BASE + 0x10 + 0x800*ss_base_mode)
#define SS_TSR                (SS_N_BASE + 0x14 + 0x800*ss_base_mode)
#define SS_ERR                (SS_N_BASE + 0x18 + 0x800*ss_base_mode)
#define SS_TPR                (SS_N_BASE + 0x1C + 0x800*ss_base_mode)
#define SS_PKEY               (SS_N_BASE + 0x30 + 0x800*ss_base_mode)
#define SS_PCTL               (SS_N_BASE + 0x34 + 0x800*ss_base_mode)


#define SS_S_TDQ				(SS_S_BASE + 0x00)
#define SS_S_CTR				(SS_S_BASE + 0x04)
#define SS_S_ICR				(SS_S_BASE + 0x08)
#define SS_S_ISR				(SS_S_BASE + 0x0C)
#define SS_S_TLR                (SS_S_BASE + 0x10)
#define SS_S_TSR                (SS_S_BASE + 0x14)
#define SS_S_ERR                (SS_S_BASE + 0x18)
#define SS_S_TPR                (SS_S_BASE + 0x1C)
#define SS_S_PKEY               (SS_S_BASE + 0x30)
#define SS_S_PCTL               (SS_S_BASE + 0x34)

#define SS_N_TDQ				(SS_N_BASE + 0x00)
#define SS_N_CTR				(SS_N_BASE + 0x04)
#define SS_N_ICR				(SS_N_BASE + 0x08)
#define SS_N_ISR				(SS_N_BASE + 0x0C)
#define SS_N_TLR                (SS_N_BASE + 0x10)
#define SS_N_TSR                (SS_N_BASE + 0x14)
#define SS_N_ERR                (SS_N_BASE + 0x18)
#define SS_N_TPR                (SS_N_BASE + 0x1C)
#define SS_N_PKEY               (SS_N_BASE + 0x30)
#define SS_N_PCTL               (SS_N_BASE + 0x34)


#define		SHA1_160_MODE	0
#define		SHA2_256_MODE	1

typedef struct sg
{
   uint addr;
   uint length;
}sg;

typedef struct descriptor_queue
{
	uint task_id;
	uint common_ctl;
	uint symmetric_ctl;
	uint asymmetric_ctl;
	uint key_descriptor;
	uint iv_descriptor;
	uint ctr_descriptor;
	uint data_len;
	sg   source[8];
	sg   destination[8];
	uint next_descriptor;
	uint reserved[3];
}task_queue;


void sunxi_ss_open(void);
void sunxi_ss_close(void);
int  sunxi_sha_calc(u8 *dst_addr, u32 dst_len,
					u8 *src_addr, u32 src_len);

s32 sunxi_rsa_calc(u8 * n_addr,   u32 n_len,
				   u8 * e_addr,   u32 e_len,
				   u8 * dst_addr, u32 dst_len,
				   u8 * src_addr, u32 src_len);

#endif    /*  #ifndef _SS_H_  */
