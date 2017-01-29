/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

 /*-------------------------------------------------------------------------------
|                  |                        |              |                      |
|     ATF          |     (atf&scp)share     | Trusted OS   |  Linux               |
|                  |                        |              |                      |
 ----------------------------------------------------------------------------------
|                  |                        |              |                      |
|     1M           |      1M                |  14M         |  other               |
|                  |                        |              |                      |
----------------------------------------------------------------------------------*/

#ifndef __SUNXI_DEF_H__
#define __SUNXI_DEF_H__

#include "sun50iw1p1.h"
/* Firmware Image Package */
#define FIP_IMAGE_NAME			"fip.bin"
#define SUNXI_PRIMARY_CPU			0x0

/* Memory location options for Shared data and TSP in sunxi */
#define SUNXI_IN_TRUSTED_SRAM		0
#define SUNXI_IN_TRUSTED_DRAM		1

/*******************************************************************************
 * sunxi memory map related constants
 ******************************************************************************/

#define SUNXI_MAX_DRAM_SIZE           (2ull<<30)     /*2G*/

//monitor area(atf+scp)
#define SUNXI_TRUSTED_MONITOR_BASE	0x40000000
#define SUNXI_TRUSTED_MONITOR_SIZE	(2<<20)     //2MB

//sec os area
#define SUNXI_TRUSTED_DRAM_BASE	0x40200000
#define SUNXI_TRUSTED_DRAM_SIZE	(14<<20)       //14 MB

//monitor area + sec os area
#define SUNXI_TRUSTED_RAM_SIZE  0x01000000        //total 16M

//atf code limit
#define SUNXI_TRUSTED_MONITOR_LIMIT	(SUNXI_TRUSTED_MONITOR_BASE + (1<<20)) //1M


/* 4KB shared memory */
#define SUNXI_SHARED_RAM_SIZE	0x1000

/* Shared memory at the base of Trusted DRAM */
#define SUNXI_SHARED_RAM_BASE		SUNXI_TRUSTED_DRAM_BASE


#define DRAM1_BASE		0x40000000ull
#define DRAM1_SIZE		0x40000000ull   //1G
#define DRAM1_END		(DRAM1_BASE + DRAM1_SIZE - 1)
#define DRAM1_SEC_SIZE		0x01000000ull

#define DRAM_BASE		DRAM1_BASE
#define DRAM_SIZE		DRAM1_SIZE

#define MEMRES_BASE             SUNXI_TRUSTED_MONITOR_LIMIT //0x40100000
#define MEMRES_SIZE             0x100000 //1M

/* Load address of BL33 in the sunxi */
#define NS_IMAGE_OFFSET		(DRAM1_BASE + 0xA000000) /* DRAM + 128MB */

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define SUNXI_BL31_PLAT_PARAM_VAL	0x12345678 //0x0f1e2d3c4b5a6978ULL


/*******************************************************************************
 * PL011 related constants
 ******************************************************************************/
#define UART0_BAUDRATE  115200

#define UART0_CLK_IN_HZ 24000000

/*******************************************************************************
 *  Shared Data
 ******************************************************************************/

/* Entrypoint mailboxes */
#define TRUSTED_MAILBOXES_BASE		SUNXI_SHARED_RAM_BASE
#define TRUSTED_MAILBOXES_SIZE		0x200
#define TRUSTED_MAILBOX_SHIFT 		4


/* Base address where parameters to BL31 are stored */
#define PARAMS_BASE		(TRUSTED_MAILBOXES_BASE + TRUSTED_MAILBOXES_SIZE)

#define MHU_SECURE_BASE         0x10000
#define MHU_SECURE_SIZE         0x1000

#define MHU_PAYLOAD_CACHED    0

#endif /* __SUNXI_DEF_H__ */
