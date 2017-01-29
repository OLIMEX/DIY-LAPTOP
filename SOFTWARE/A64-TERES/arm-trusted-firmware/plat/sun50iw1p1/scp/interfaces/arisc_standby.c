/*
 *  drivers/arisc/interfaces/arisc_standby.c
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-10-01 Written by superm (superm@allwinnertech.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../arisc_i.h"

/* record super-standby wakeup event */
static unsigned long wakeup_event = 0;
static unsigned long dram_crc_error = 0;
static unsigned long dram_crc_total_count = 0;
static unsigned long dram_crc_error_count = 0;

/**
 * cpu operations.
 * @mpidr: cpu id;
 * @entrypoint: cpu resume entrypoint;
 * @cpu_state: cpu state;
 * @cluster_state: cluster state;
 *
 * return: result, 0 - cpu operations successed,
 *                !0 - cpu operations failed;
 */
int arisc_cpu_op(uint32_t mpidr, uint32_t entrypoint, arisc_power_state_t cpu_state,
		arisc_power_state_t cluster_state)
{
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(0);
	if (pmessage == NULL) {
		ARISC_ERR("allocate message for cpu op request failed\n");
		return -ENOMEM;
	}

	pmessage->type       = ARISC_CPU_OP_REQ;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;
	pmessage->paras[0] = mpidr;
	pmessage->paras[1] = entrypoint;
	pmessage->paras[2] = cpu_state;
	pmessage->paras[3] = cluster_state;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;

	/* send enter cpu operations request to arisc */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	return 0;
}

 /**
 * system operations.
 * @state: system state;
 *
 * return: result, 0 - system operations successed,
 *                !0 - system operations failed;
 */
int arisc_system_op(arisc_system_state_t state)
{
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(0);
	if (pmessage == NULL) {
		ARISC_ERR("allocate message for sys op request failed\n");
		return -ENOMEM;
	}

	pmessage->type       = ARISC_SYS_OP_REQ;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;
	pmessage->paras[0] = state;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;

	/* send enter sys operations request to arisc */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	return 0;
}

/*
 * enter cpu idle.
 * @para:  parameter for enter cpu idle.
 *    para->flag: 0x01-clear pending, 0x10-enter cpuidle
 *    para->resume_addr: the address cpu0 will run when exit idle
 *
 * return: result, 0 - super standby successed,
 *                !0 - super standby failed;
 */
int arisc_enter_cpuidle(arisc_cb_t cb, void *cb_arg, struct sunxi_enter_idle_para *para)
{
	struct arisc_message *pmessage;	/* allocate a message frame */
	pmessage = arisc_message_allocate(0);
	if (pmessage == NULL) {
		ARISC_ERR("allocate message for super-standby request failed\n");
		return -ENOMEM;
	}
	pmessage->type  		= ARISC_CPUIDLE_ENTER_REQ;
	pmessage->cb.handler	= cb;
	pmessage->cb.arg		= cb_arg;
	pmessage->state			= ARISC_MESSAGE_INITIALIZED;
	pmessage->paras[0]  	= para->flags;
	pmessage->paras[1]  	= (unsigned long)para->resume_addr;
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
	return 0;
}

/**
 * query super-standby wakeup source.
 * @para:  point of buffer to store wakeup event informations.
 *
 * return: result, 0 - query successed,
 *                !0 - query failed;
 */
int arisc_query_wakeup_source(uint32_t *event)
{
	*event = wakeup_event;

	return 0;
}

/*
 * query super-standby infoation.
 * @para:  point of array to store power states informations during sst.
 * @op: 0:read, 1:set
 *
 * return: result, 0 - query successed,
 *                !0 - query failed;
 */
int arisc_query_set_standby_info(struct standby_info_para *para, arisc_rw_type_e op)
{
	struct arisc_message *pmsg;
	int result;

	/* allocate a message frame */
	pmsg = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmsg == NULL) {
		ARISC_ERR("allocate message for query standby info failed\n");
		return -ENOMEM;
	}

	/* check standby_info_para size valid or not */
	if (sizeof(struct standby_info_para) > sizeof(pmsg->paras)) {
		ARISC_ERR("standby info parameters number too long\n");
		return -EINVAL;
	}

	/* initialize message */
	pmsg->type       = ARISC_STANDBY_INFO_REQ;
	pmsg->cb.handler = NULL;
	pmsg->cb.arg     = NULL;
	pmsg->private = (void *)op;
	if (ARISC_WRITE == op) {
		memcpy((void *)pmsg->paras, (const void *)para, sizeof(struct standby_info_para));
	}
	pmsg->state      = ARISC_MESSAGE_INITIALIZED;

	/* send query sst info request to arisc */
	arisc_hwmsgbox_send_message(pmsg, ARISC_SEND_MSG_TIMEOUT);
	if (ARISC_READ == op)
		memcpy((void *)para, (void *)pmsg->paras, sizeof(struct standby_info_para));

	/* free message */
	result = pmsg->result;
	arisc_message_free(pmsg);

	return result;
}

/*
 * query super-standby dram crc result.
 * @para:  point of buffer to store dram crc result informations.
 *
 * return: result, 0 - query successed,
 *                !0 - query failed;
 */
int arisc_query_dram_crc_result(unsigned long *perror, unsigned long *ptotal_count,
	unsigned long *perror_count)
{
	*perror = dram_crc_error;
	*ptotal_count = dram_crc_total_count;
	*perror_count = dram_crc_error_count;

	return 0;
}

int arisc_set_dram_crc_result(unsigned long error, unsigned long total_count,
	unsigned long error_count)
{
	dram_crc_error = error;
	dram_crc_total_count = total_count;
	dram_crc_error_count = error_count;

	return 0;
}

/**
 * notify arisc cpux restored.
 * @para:  none.
 *
 * return: result, 0 - notify successed, !0 - notify failed;
 */
int arisc_cpux_ready_notify(void)
{
	struct arisc_message *pmessage;

	/* notify hwspinlock and hwmsgbox resume first */
	arisc_hwmsgbox_standby_resume();
	arisc_hwspinlock_standby_resume();

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type     = ARISC_SSTANDBY_RESTORE_NOTIFY;
	pmessage->state    = ARISC_MESSAGE_INITIALIZED;

	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* record wakeup event */
	wakeup_event   = pmessage->paras[0];
	if (arisc_debug_dram_crc_en) {
		dram_crc_error = pmessage->paras[1];
		dram_crc_total_count++;
		dram_crc_error_count += (dram_crc_error ? 1 : 0);
	}

	/* free message */
	arisc_message_free(pmessage);

	return 0;
}

int arisc_config_ir_paras(uint32_t ir_code, uint32_t ir_addr)
{
	int result = 0;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	/* initialize message */
	pmessage->type       = ARISC_SET_IR_PARAS;
	pmessage->paras[0]   = ir_code;
	pmessage->paras[1]   = ir_addr;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	ARISC_INF("ir power key:0x%x, addr:0x%x\n", ir_code, ir_addr);

	/* send request message */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
	if (pmessage->result) {
		ARISC_WRN("config ir power key code [%d] fail\n", pmessage->paras[0]);
		result = -EINVAL;
	}

	/* free allocated message */
	arisc_message_free(pmessage);

	return result;
}
