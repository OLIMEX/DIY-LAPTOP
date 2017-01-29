/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : mem_ir.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-31 15:15
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/

#ifndef __MEM_IR_H__
#define __MEM_IR_H__

extern __s32 mem_ir_init(void);
extern __s32 mem_ir_exit(void);
extern __s32 mem_ir_detect(void);
extern __s32 mem_ir_verify(void);

#endif  /*__MEM_IR_H__*/
