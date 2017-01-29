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

#ifndef __SUNXI_PRIVATE_H__
#define __SUNXI_PRIVATE_H__

#include <bl_common.h>
#include <platform_def.h>


void gic_cpuif_deactivate(unsigned int gicc_base);
void gic_cpuif_setup(unsigned int gicc_base);
void gic_pcpu_distif_setup(unsigned int gicd_base);
void gic_setup(void);


typedef volatile struct mailbox {
	unsigned long value
	__attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));
} mailbox_t;

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31 e.g. while passing control to it from BL2 which is bl31_params
 * and bl31_plat_params and its elements
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	bl31_params_t bl31_params;
	image_info_t bl31_image_info;
	image_info_t bl32_image_info;
	image_info_t bl33_image_info;
	entry_point_info_t bl33_ep_info;
	entry_point_info_t bl32_ep_info;
	entry_point_info_t bl31_ep_info;
} bl2_to_bl31_params_mem_t;

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct meminfo;

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void sunxi_configure_mmu_el1(unsigned long total_base,
			   unsigned long total_size,
			   unsigned long,
			   unsigned long,
			   unsigned long,
			   unsigned long);
void sunxi_configure_mmu_el3(unsigned long total_base,
			   unsigned long total_size,
			   unsigned long,
			   unsigned long,
			   unsigned long,
			   unsigned long);
int sunxi_config_setup(void);

void sunxi_cci_init(void);
void sunxi_cci_enable(void);

void sunxi_gic_init(void);

/* Declarations for sunxi_topology.c */
int sunxi_setup_topology(void);

/* Declarations for sunxi_io_storage.c */
void sunxi_io_setup(void);

/* Declarations for sunxi_security.c */
void sunxi_security_setup(void);

/* Gets the SPR for BL32 entry */
uint32_t sunxi_get_spsr_for_bl32_entry(void);

/* Gets the SPSR for BL33 entry */
uint32_t sunxi_get_spsr_for_bl33_entry(void);


#endif /* __FVP_PRIVATE_H__ */
