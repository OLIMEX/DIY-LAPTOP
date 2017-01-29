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

#include <assert.h>
#include <debug.h>
#include <plat_config.h>
#include <tzc400.h>
#include <mmio.h>
#include "sunxi_def.h"
#include "sunxi_private.h"

#define SPC_BASE (0x1c23400ull)

#define SPC_DECPORT0_STA_REG  (SPC_BASE+0x4)
#define SPC_DECPORT0_SET_REG  (SPC_BASE+0x8)
#define SPC_DECPORT0_CLR_REG  (SPC_BASE+0xc)

#define SPC_DECPORT1_STA_REG  (SPC_BASE+0x10)
#define SPC_DECPORT1_SET_REG  (SPC_BASE+0x14)
#define SPC_DECPORT1_CLR_REG  (SPC_BASE+0x18)

#define SPC_DECPORT2_STA_REG  (SPC_BASE+0x1c)
#define SPC_DECPORT2_SET_REG  (SPC_BASE+0x20)
#define SPC_DECPORT2_CLR_REG  (SPC_BASE+0x24)

#define SPC_DECPORT3_STA_REG  (SPC_BASE+0x28)
#define SPC_DECPORT3_SET_REG  (SPC_BASE+0x2c)
#define SPC_DECPORT3_CLR_REG  (SPC_BASE+0x30)

#define SPC_DECPORT4_STA_REG  (SPC_BASE+0x34)
#define SPC_DECPORT4_SET_REG  (SPC_BASE+0x38)
#define SPC_DECPORT4_CLR_REG  (SPC_BASE+0x3c)

#define SPC_DECPORT5_STA_REG  (SPC_BASE+0x40)
#define SPC_DECPORT5_SET_REG  (SPC_BASE+0x44)
#define SPC_DECPORT5_CLR_REG  (SPC_BASE+0x48)








/* Used to improve readability for configuring regions. */
#define FILTER_SHIFT(filter)	(1 << filter)

/*
 * For the moment we assume that all security programming is done by the
 * primary core.
 * TODO:
 * Might want to enable interrupt on violations when supported?
 */
void sunxi_security_setup(void)
{
	/*
	 *
	 * If the platform had additional peripheral specific security
	 * configurations, those would be configured here.
	 */

	//if (!(get_plat_config()->flags & CONFIG_HAS_TZC))
	//	return;

	INFO("Configuring SPC Controller\n");
	//set all peripherals to non-sec
	mmio_write_32(SPC_DECPORT0_SET_REG,0xff);
	mmio_write_32(SPC_DECPORT1_SET_REG,0xff);
	mmio_write_32(SPC_DECPORT2_SET_REG,0xff);
	mmio_write_32(SPC_DECPORT3_SET_REG,0xff);
	mmio_write_32(SPC_DECPORT4_SET_REG,0xff);
	mmio_write_32(SPC_DECPORT5_SET_REG,0xff);
	
	//set ccmu security switch: set mbus_sec bus_sec pll_sec to non-sec
	mmio_write_32(0x01c20000+0x2f0, 0x7);

	//set R_PRCM security switch: set power_sec  pll_sec cpus_clk to non-sec
	mmio_write_32(0x01f01400+0x1d0, 0x7);
	
	//set dma security switch: set DMA channel0-7 to non-sec
	mmio_write_32(0x01c02000+0x20, 0xff);

}
