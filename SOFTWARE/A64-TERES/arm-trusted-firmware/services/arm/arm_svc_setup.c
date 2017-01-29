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

#include <debug.h>
#include <psci.h>
#include <runtime_svc.h>
#include <std_svc.h>
#include <stdint.h>
#include <uuid.h>
#include <assert.h>
#include <string.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <arisc.h>
#include <mmio.h>

#define ARM_NUM_CALLS 6

#define ARM_SVC_CALL_COUNT	0x8000ff00
#define ARM_SVC_UID		0x8000ff01
//0x8000ff02 reserved
#define ARM_SVC_VERSION		0x8000ff03
#define ARM_SVC_RUNNSOS		0x8000ff04
#define ARM_SVC_READ_SEC_REG    0x8000ff05
#define ARM_SVC_WRITE_SEC_REG   0x8000ff06

#define ARM_SVC_ARISC_STARTUP	 0x8000ff10
#define ARM_SVC_ARISC_WAIT_READY 0x8000ff11
#define ARM_SVC_ARISC_READ_PMU   0x8000ff12
#define ARM_SVC_ARISC_WRITE_PMU  0x8000ff13


/* ARM Standard Service Calls version numbers */
#define ARM_SVC_VERSION_MAJOR		0x0
#define ARM_SVC_VERSION_MINOR		0x1

/* Arm arch Service UUID */

DEFINE_SVC_UUID(arm_svc_uid,
		0x83B53A5B, 0xC594, 0x40B9, 0x53, 0x91,
		0xF2, 0xFC, 0x54, 0x29, 0x86, 0x48);
/*******************************************************************************
 * This function programs EL3 registers and performs other setup to enable entry
 * into the next image after BL31 at the next ERET.
 ******************************************************************************/
void prepare_nonsec_os_entry(uint64_t kernel_addr, uint64_t dtb_addr)
{
	entry_point_info_t next_image_info;
	uint32_t image_type;

	/* Determine which image to execute next */
	image_type = NON_SECURE;

	/* Program EL3 registers to enable entry into the next EL */
	memset(&next_image_info, 0, sizeof(next_image_info));
	SET_SECURITY_STATE(next_image_info.h.attr, NON_SECURE);
	next_image_info.spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	next_image_info.pc = kernel_addr;
	next_image_info.args.arg0 = dtb_addr;
	
	

	INFO("BL3-1: Next image address = 0x%llx\n",(unsigned long long) next_image_info.pc);
	INFO("BL3-1: Next image spsr = 0x%x\n", next_image_info.spsr);

	cm_init_context(read_mpidr_el1(), &next_image_info);
	cm_prepare_el3_exit(image_type);
}


/*
 * Top-level Standard Service SMC handler. This handler will in turn dispatch
 * calls to PSCI SMC handler
 */
uint64_t arm_svc_smc_handler(uint32_t smc_fid,
			     uint64_t x1,
			     uint64_t x2,
			     uint64_t x3,
			     uint64_t x4,
			     void *cookie,
			     void *handle,
			     uint64_t flags)
{
	//INFO(" Arm Architechture Service Call: 0x%x \n", smc_fid);
	
	switch (smc_fid) {
	case ARM_SVC_CALL_COUNT:
		SMC_RET1(handle, ARM_NUM_CALLS);
	case ARM_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, arm_svc_uid);
	case ARM_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, ARM_SVC_VERSION_MAJOR, ARM_SVC_VERSION_MINOR);
	case ARM_SVC_RUNNSOS:
		prepare_nonsec_os_entry((uint32_t)x1,(uint32_t)x2);
		SMC_RET0(handle);
	case ARM_SVC_READ_SEC_REG:
		SMC_RET1(handle, mmio_read_32((uintptr_t)x1));
	case ARM_SVC_WRITE_SEC_REG:
		mmio_write_32((uintptr_t)x1,(uint32_t)x2);
		SMC_RET0(handle);

	//arise cmd begin
	//arise aa32 cmd
	case ARM_SVC_ARISC_STARTUP:
		SMC_RET1(handle, sunxi_arisc_probe((void *)x1));
	case ARM_SVC_ARISC_WAIT_READY:
		SMC_RET1(handle, sunxi_arisc_wait_ready());
	case ARM_SVC_ARISC_READ_PMU:
		SMC_RET1(handle, arisc_rsb_read_pmu_reg((uint32_t)x1));
	case ARM_SVC_ARISC_WRITE_PMU:
		SMC_RET1(handle,arisc_rsb_write_pmu_reg((uint32_t)x1,(uint32_t)x2));

	//arise aa64 cmd
	case ARM_SVC_ARISC_CPUX_DVFS_REQ:
		SMC_RET1(handle, arisc_dvfs_set_cpufreq((uint32_t)x1, (uint32_t)x2, (uint32_t)x3));
	case ARM_SVC_ARISC_RSB_READ_BLOCK_DATA:
		SMC_RET1(handle, arisc_rsb_read_block_data((uint32_t *)x1));
	case ARM_SVC_ARISC_RSB_WRITE_BLOCK_DATA:
		SMC_RET1(handle, arisc_rsb_write_block_data((uint32_t *)x1));
	case ARM_SVC_ARISC_RSB_BITS_OPS_SYNC:
		SMC_RET1(handle, rsb_bits_ops_sync((uint32_t *)x1));
	case ARM_SVC_ARISC_RSB_SET_INTERFACE_MODE:
		SMC_RET1(handle, arisc_rsb_set_interface_mode((uint32_t)x1, (uint32_t)x2, (uint32_t)x3));
	case ARM_SVC_ARISC_RSB_SET_RTSADDR:
		SMC_RET1(handle, arisc_rsb_set_rtsaddr((uint32_t)x1, (uint32_t)x2));
	case ARM_SVC_ARISC_SET_DEBUG_LEVEL:
		SMC_RET1(handle, arisc_set_debug_level((uint32_t)x1));
	case ARM_SVC_ARISC_SET_UART_BAUDRATE:
		SMC_RET1(handle, arisc_set_uart_baudrate((uint32_t)x1));
	case ARM_SVC_ARISC_MESSAGE_LOOPBACK:
		SMC_RET1(handle, arisc_message_loopback());
	case ARM_SVC_ARISC_SET_DEBUG_DRAM_CRC_PARAS:
		SMC_RET1(handle, arisc_set_dram_crc_paras((uint32_t)x1, (uint32_t)x2, (uint32_t)x3));
	case ARM_SVC_ARISC_AXP_DISABLE_IRQ:
		SMC_RET1(handle, arisc_disable_nmi_irq());
	case ARM_SVC_ARISC_AXP_ENABLE_IRQ:
		SMC_RET1(handle, arisc_enable_nmi_irq());
	case ARM_SVC_ARISC_CLR_NMI_STATUS:
		SMC_RET1(handle, arisc_clear_nmi_status());
	case ARM_SVC_ARISC_SET_NMI_TRIGGER:
		SMC_RET1(handle, arisc_set_nmi_trigger((uint32_t)x1));
	case ARM_SVC_ARISC_AXP_GET_CHIP_ID:
		SMC_RET1(handle, arisc_axp_get_chip_id((unsigned char *)x1));
	case ARM_SVC_ARISC_AXP_SET_PARAS:
		SMC_RET1(handle, arisc_adjust_pmu_chgcur((uint32_t)x1, (uint32_t)x2, (uint32_t)x3));
	case ARM_SVC_ARISC_SET_PWR_TREE:
		SMC_RET1(handle, arisc_set_pwr_tree((uint32_t *)x1));
	case ARM_SVC_ARISC_SET_PMU_VOLT:
		SMC_RET1(handle, arisc_pmu_set_voltage((uint32_t)x1, (uint32_t)x2));
	case ARM_SVC_ARISC_GET_PMU_VOLT:
		SMC_RET1(handle, arisc_pmu_get_voltage((uint32_t)x1, (uint32_t *)x2));
	case ARM_SVC_ARISC_SET_LED_BLN:
		SMC_RET1(handle, arisc_set_led_bln((uint32_t *)x1));
	case ARM_SVC_ARISC_QUERY_WAKEUP_SRC_REQ:
		SMC_RET1(handle, arisc_query_wakeup_source((uint32_t *)x1));
	case ARM_SVC_ARISC_STANDBY_INFO_REQ:
		SMC_RET1(handle, arisc_query_set_standby_info((standby_info_para_t *)x1, (arisc_rw_type_e)x2));
	default:
		WARN("Unimplemented Standard Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(
		arm_svc,

		OEN_ARM_START,
		OEN_ARM_END,
		SMC_TYPE_FAST,
		NULL,
		arm_svc_smc_handler
);
