/*
 *  drivers/arisc/interfaces/arisc_axp.c
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

/* nmi isr node, record current nmi interrupt handler and argument */
nmi_isr_t nmi_isr_node[2];

/**
 * register call-back function, call-back function is for arisc notify some event to ac327,
 * axp/rtc interrupt for external interrupt NMI.
 * @type:  nmi type, pmu/rtc;
 * @func:  call-back function;
 * @para:  parameter for call-back function;
 *
 * @return: result, 0 - register call-back function successed;
 *                 !0 - register call-back function failed;
 * NOTE: the function is like "int callback(void *para)";
 *       this function will execute in system ISR.
 */
int arisc_nmi_cb_register(uint32_t type, arisc_cb_t func, void *para)
{
	if (nmi_isr_node[type].handler) {
		if(func == nmi_isr_node[type].handler) {
			ARISC_WRN("nmi interrupt handler register already\n");
			return 0;
		}
		/* just output warning message, overlay handler */
		ARISC_WRN("nmi interrupt handler register already\n");
		return -EINVAL;
	}
	nmi_isr_node[type].handler = func;
	nmi_isr_node[type].arg     = para;

	return 0;
}


/**
 * unregister call-back function.
 * @type:  nmi type, pmu/rtc;
 * @func:  call-back function which need be unregister;
 */
void arisc_nmi_cb_unregister(uint32_t type, arisc_cb_t func)
{
	if ((nmi_isr_node[type].handler) != (func)) {
		/* invalid handler */
		ARISC_WRN("invalid handler for unreg\n\n");
		return ;
	}
	nmi_isr_node[type].handler = NULL;
	nmi_isr_node[type].arg     = NULL;
}

int arisc_disable_nmi_irq(void)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_AXP_DISABLE_IRQ;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_enable_nmi_irq(void)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_AXP_ENABLE_IRQ;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_clear_nmi_status(void)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_CLR_NMI_STATUS;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_set_nmi_trigger(uint32_t type)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_SET_NMI_TRIGGER;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;
	pmessage->paras[0]   = type;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_axp_get_chip_id(unsigned char *chip_id)
{
	int                   i;
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_AXP_GET_CHIP_ID;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	memset((void *)pmessage->paras, 0, 16);

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* |paras[0]    |paras[1]    |paras[2]     |paras[3]      |
	 * |chip_id[0~3]|chip_id[4~7]|chip_id[8~11]|chip_id[12~15]|
	 */
	/* copy message readout data to user data buffer */
	for (i = 0; i < 4; i++) {
			chip_id[i] = (pmessage->paras[0] >> (i * 8)) & 0xff;
			chip_id[4 + i] = (pmessage->paras[1] >> (i * 8)) & 0xff;
			chip_id[8 + i] = (pmessage->paras[2] >> (i * 8)) & 0xff;
			chip_id[12 + i] = (pmessage->paras[3] >> (i * 8)) & 0xff;
	}

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_set_led_bln(uint32_t *paras)
{
	int result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_SET_LED_BLN;
	pmessage->paras[0]   = paras[0];
	pmessage->paras[1]   = paras[1];
	pmessage->paras[2]   = paras[2];
	pmessage->paras[3]   = paras[3];

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;

}

#if (defined CONFIG_ARCH_SUN8IW5P1) || (defined CONFIG_ARCH_SUN50IW1P1)
int arisc_adjust_pmu_chgcur(uint32_t max_chgcur, uint32_t chg_ic_temp, uint32_t flag)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_AXP_SET_PARAS;
	pmessage->paras[0]   = chg_ic_temp;
	pmessage->paras[1]   = max_chgcur;
	pmessage->paras[2]   = flag;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}
#endif

int arisc_set_pwr_tree(uint32_t *pwr_tree)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_SET_PWR_TREE;
	memcpy((void *)pmessage->paras, (void *)pwr_tree, sizeof(int)*DM_MAX);

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_axp_int_notify(struct arisc_message *pmessage)
{
	uint32_t type = pmessage->paras[0];
	uint32_t ret = 0;

	if (type & NMI_INT_TYPE_PMU_OFFSET) {
		/* call pmu interrupt handler */
		if (nmi_isr_node[NMI_INT_TYPE_PMU].handler == NULL) {
			ARISC_WRN("pmu irq handler not install\n");
			return 1;
		}

		ARISC_INF("call pmu interrupt handler\n");
		ret |= (*(nmi_isr_node[NMI_INT_TYPE_PMU].handler))(nmi_isr_node[NMI_INT_TYPE_PMU].arg);
	}
	if (type & NMI_INT_TYPE_RTC_OFFSET)
	{
		/* call rtc interrupt handler */
		if (nmi_isr_node[NMI_INT_TYPE_RTC].handler == NULL) {
			ARISC_WRN("rtc irq handler not install\n");
			return 1;
		}

		ARISC_INF("call rtc interrupt handler\n");
		ret |= (*(nmi_isr_node[NMI_INT_TYPE_RTC].handler))(nmi_isr_node[NMI_INT_TYPE_RTC].arg);
	}

	return ret;
}

int arisc_pmu_set_voltage(uint32_t type, uint32_t voltage)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_SET_PMU_VOLT;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;
	pmessage->paras[0]   = type;
	pmessage->paras[1]   = voltage;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_pmu_get_voltage(uint32_t type, uint32_t *voltage)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_GET_PMU_VOLT;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;
	pmessage->paras[0]   = type;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
	*voltage = pmessage->paras[1];

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}
