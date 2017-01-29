/*
 * SUNXI suspend
 *
 * Copyright (C) 2015 AllWinnertech Ltd.
 * Author: xiafeng <xiafeng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __PM_RESUMES_ASSEM_H__
#define __PM_RESUMES_ASSEM_H__

/* paras and code offset in SRAM A1 */
#define SRAMA1_PARA_OFFSET  0x400
#define SRAMA1_CODE_OFFSET  0x800

/* NOTE: these define order same with struct arisc_para in resumes.h */
#define RESUME_ADDR 0x00    /* 00, CR0: Cache Size Selection            */
#define MONITOR_VEC 0x04    /* 04, CR1: Control                         */
#define CP15_CR0    0x08    /* 08, Coprocessor Access Control           */
#define CP15_CR1    0x0c    /* 12, CR2: Translation Table Base 0        */
#define CP15_CACR   0x10    /* 16, Translation Table Base 1             */
#define CP15_TTB0R  0x14    /* 20, Translation Talbe Base Control       */
#define CP15_TTB1R  0x18    /* 24, CR3: Domain Access Control           */
#define CP15_TTBCR  0x1c    /* 28, cr10: Primary Region Remap Register  */
#define CP15_DACR   0x20    /* 32, Normal Memory Remap Register         */
#define CP15_PRRR   0x24
#define CP15_NRRR   0x28
#define REGS_NUM    0x2c    /* 44, the number we should restore to regs */
#define REGS_OFFST  0x30

#endif  /* __PM_RESUMES_ASSEM_H__ */

