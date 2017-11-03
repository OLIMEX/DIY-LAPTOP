/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, gq.yang China
*                                             All Rights Reserved
*
* File    : mem_mapping.h
* By      : 
* Version : v1.0
* Date    : 2012-5-31 14:34
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_MAPPING_H__
#define __MEM_MAPPING_H__

/*mem_mapping.c*/
extern void create_mapping(void);
extern void save_mapping(unsigned long vaddr);
extern void restore_mapping(unsigned long vaddr);

#endif  /* __MEM_MAPPING_H__ */

