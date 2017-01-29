/*
 *  arch/arm/mach-sunxi/arisc/hwmsgbox/hwmsgbox.c
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

#include "hwmsgbox_i.h"

/* spinlock for syn and asyn channel */
static spinlock_t syn_channel_lock;
static spinlock_t asyn_channel_lock;
static uintptr_t base;

/**
 * initialize hwmsgbox.
 * @para:  none.
 *
 * returns:  0 if initialize hwmsgbox succeeded, others if failed.
 */
int arisc_hwmsgbox_init(void)
{
	base = dts_cfg.msgbox.base;

	/* initialize syn and asyn spinlock */
	//writel(0xffffffff, base + AW_MSGBOX_IRQ_STATUS_REG(AW_HWMSG_QUEUE_USER_AC327));
	//writel(0x0, base + AW_MSGBOX_IRQ_EN_REG(AW_HWMSG_QUEUE_USER_AC327));

	return 0;
}

/**
 * exit hwmsgbox.
 * @para:  none.
 *
 * returns:  0 if exit hwmsgbox succeeded, others if failed.
 */
int arisc_hwmsgbox_exit(void)
{
	return 0;
}

/**
 * send one message to another processor by hwmsgbox.
 * @pmessage:  the pointer of sended message frame.
 * @timeout:   the wait time limit when message fifo is full,                             it is valid only when parameter mode = HWMSG_SEND_WAIT_TIMEOUT.
 *
 * returns:   0 if send message succeeded, other if failed.
 */
int arisc_hwmsgbox_send_message(struct arisc_message *pmessage, unsigned int timeout)
{
	volatile uint32_t value;

	if (pmessage == NULL) {
		return -EINVAL;
	}
	if (pmessage->attr & ARISC_MESSAGE_ATTR_HARDSYN) {
		/* use ac327 hwsyn transmit channel */
		spin_lock(&syn_channel_lock);
		while (readl(base + AW_MSGBOX_FIFO_STATUS_REG(ARISC_HWMSGBOX_AC327_SYN_TX_CH)) == 1);

		value = arisc_message_map_to_cpus(pmessage);
		ARISC_INF("ac327 send hard syn message : %x\n", (unsigned int)value);
		writel(value, base + AW_MSGBOX_MSG_REG(ARISC_HWMSGBOX_AC327_SYN_TX_CH));

		/* hwsyn messsage must feedback use syn rx channel */
		while (readl(base + AW_MSGBOX_MSG_STATUS_REG(ARISC_HWMSGBOX_AC327_SYN_RX_CH)) == 0);

		/* check message valid */
		if (value != (readl(base + AW_MSGBOX_MSG_REG(ARISC_HWMSGBOX_AC327_SYN_RX_CH)))) {
			ARISC_ERR("hard syn message error [%x, %x]\n", (uint32_t)value, (uint32_t)(readl(base + AW_MSGBOX_MSG_REG(ARISC_HWMSGBOX_AC327_SYN_RX_CH))));
			spin_unlock(&syn_channel_lock);
			return -EINVAL;
		}
		ARISC_INF("ac327 hard syn message [%x, %x] feedback\n", (unsigned int)value, (unsigned int)pmessage->type);
		/* if error call the callback function. by superm */
		if(pmessage->result != 0) {
			ARISC_ERR("message process error\n");
			ARISC_ERR("message addr   : %llx\n", pmessage);
			ARISC_ERR("message state  : %x\n", pmessage->state);
			ARISC_ERR("message attr   : %x\n", pmessage->attr);
			ARISC_ERR("message type   : %x\n", pmessage->type);
			ARISC_ERR("message result : %x\n", pmessage->result);
			if (pmessage->cb.handler == NULL) {
				ARISC_WRN("callback not install\n");
			} else {
				/* call callback function */
				ARISC_WRN("call the callback function\n");
				(*(pmessage->cb.handler))(pmessage->cb.arg);
			}
		}
		spin_unlock(&syn_channel_lock);
		return 0;
	}

	/* use ac327 asyn transmit channel */
	spin_lock(&asyn_channel_lock);
	while (readl(base + AW_MSGBOX_FIFO_STATUS_REG(ARISC_HWMSGBOX_ARISC_ASYN_RX_CH)) == 1);

	/* write message to message-queue fifo */
	value = arisc_message_map_to_cpus(pmessage);
	ARISC_INF("ac327 send message : %x\n", (unsigned int)value);
	writel(value, base + AW_MSGBOX_MSG_REG(ARISC_HWMSGBOX_ARISC_ASYN_RX_CH));
	spin_unlock(&asyn_channel_lock);

	return 0;
}

int arisc_hwmsgbox_feedback_message(struct arisc_message *pmessage, unsigned int timeout)
{
	volatile unsigned long value;

	if (pmessage->attr & ARISC_MESSAGE_ATTR_HARDSYN) {
		/* use ac327 hard syn receiver channel */
		spin_lock(&syn_channel_lock);
		while (readl(base + AW_MSGBOX_FIFO_STATUS_REG(ARISC_HWMSGBOX_ARISC_SYN_RX_CH)) == 1);

		value = arisc_message_map_to_cpus(pmessage);
		ARISC_INF("arisc feedback hard syn message : %x\n", (unsigned int)value);
		writel(value, base + AW_MSGBOX_MSG_REG(ARISC_HWMSGBOX_ARISC_SYN_RX_CH));
		spin_unlock(&syn_channel_lock);

		return 0;
	}

	/* invalid syn message */
	return -EINVAL;
}

/**
 * enbale the receiver interrupt of message-queue.
 * @queue:  the number of message-queue which we want to enable interrupt.
 * @user:   the user which we want to enable interrupt.
 *
 * returns:  0 if enable interrupt succeeded, others if failed.
 */
int arisc_hwmsgbox_enable_receiver_int(int queue, int user)
{
	volatile unsigned int value;

	value  =  readl(base + AW_MSGBOX_IRQ_EN_REG(user));
	value &= ~(0x1 << (queue * 2));
	value |=  (0x1 << (queue * 2));
	writel(value, base + AW_MSGBOX_IRQ_EN_REG(user));

	return 0;
}

/**
 * disbale the receiver interrupt of message-queue.
 * @queue:  the number of message-queue which we want to enable interrupt.
 * @user:   the user which we want to enable interrupt.
 *
 * returns:  0 if disable interrupt succeeded, others if failed.
 */
int arisc_hwmsgbox_disable_receiver_int(int queue, int user)
{
	volatile unsigned int value;

	value  =  readl(base + AW_MSGBOX_IRQ_EN_REG(user));
	value &= ~(0x1 << (queue * 2));
	writel(value, base + AW_MSGBOX_IRQ_EN_REG(user));

	return 0;
}

/**
 * query the receiver interrupt pending of message-queue.
 * @queue:  the number of message-queue which we want to query.
 * @user:   the user which we want to query.
 *
 * returns:  0 if query pending succeeded, others if failed.
 */
int arisc_hwmsgbox_query_receiver_pending(int queue, int user)
{
	volatile unsigned long value;

	value  =  readl(base + (AW_MSGBOX_IRQ_STATUS_REG(user)));

	return value & (0x1 << (queue * 2));
}

/**
 * clear the receiver interrupt pending of message-queue.
 * @queue:  the number of message-queue which we want to clear.
 * @user:   the user which we want to clear.
 *
 * returns:  0 if clear pending succeeded, others if failed.
 */
int arisc_hwmsgbox_clear_receiver_pending(int queue, int user)
{
	writel((0x1 << (queue * 2)), base + AW_MSGBOX_IRQ_STATUS_REG(user));

	return 0;
}

/**
 * query message of hwmsgbox by hand, mainly for.
 * @para:  none.
 *
 * returns:  the point of message, NULL if timeout.
 */
struct arisc_message *arisc_hwmsgbox_query_message(void)
{
	struct arisc_message *pmessage = NULL;

	/* query ac327 asyn received channel */
	if (readl(base + AW_MSGBOX_MSG_STATUS_REG(ARISC_HWMSGBOX_ARISC_ASYN_TX_CH))) {
		volatile unsigned long value;
		value = readl(base + AW_MSGBOX_MSG_REG(ARISC_HWMSGBOX_ARISC_ASYN_TX_CH));
		pmessage = arisc_message_map_to_cpux(value);

		if (arisc_message_valid(pmessage)) {
			/* message state switch */
			if (pmessage->state == ARISC_MESSAGE_PROCESSED) {
				/* ARISC_MESSAGE_PROCESSED->ARISC_MESSAGE_FEEDBACKED */
				pmessage->state = ARISC_MESSAGE_FEEDBACKED;
			} else {
				/* ARISC_MESSAGE_INITIALIZED->ARISC_MESSAGE_RECEIVED */
				pmessage->state = ARISC_MESSAGE_RECEIVED;
			}
		} else {
			ARISC_ERR("invalid asyn message received: pmessage = 0x%llx. \n", pmessage);
			return NULL;
		}
		/* clear pending */
		arisc_hwmsgbox_clear_receiver_pending(ARISC_HWMSGBOX_ARISC_ASYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);
		return pmessage;
	}
	/* query ac327 syn received channel */
	if (readl(base + AW_MSGBOX_MSG_STATUS_REG(ARISC_HWMSGBOX_ARISC_SYN_TX_CH))) {
		volatile unsigned long value;
		value = readl(base + AW_MSGBOX_MSG_REG(ARISC_HWMSGBOX_ARISC_SYN_TX_CH));
		pmessage = arisc_message_map_to_cpux(value);
		if (arisc_message_valid(pmessage)) {
			/* message state switch */
			if (pmessage->state == ARISC_MESSAGE_PROCESSED) {
				/* ARISC_MESSAGE_PROCESSED->ARISC_MESSAGE_FEEDBACKED */
				pmessage->state = ARISC_MESSAGE_FEEDBACKED;
			} else {
				/* ARISC_MESSAGE_INITIALIZED->ARISC_MESSAGE_RECEIVED */
				pmessage->state = ARISC_MESSAGE_RECEIVED;
			}
		} else {
			ARISC_ERR("invalid syn message received: pmessage = 0x%llx. \n", pmessage);
			arisc_hwmsgbox_clear_receiver_pending(ARISC_HWMSGBOX_ARISC_SYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);
			return NULL;
		}
		arisc_hwmsgbox_clear_receiver_pending(ARISC_HWMSGBOX_ARISC_SYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);
		return pmessage;
	}

	/* no valid message now */
	return NULL;
}

int arisc_hwmsgbox_standby_suspend(void)
{
	/* enable arisc asyn tx interrupt */
	//arisc_hwmsgbox_disable_receiver_int(ARISC_HWMSGBOX_ARISC_ASYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);

	/* enable arisc syn tx interrupt */
	//arisc_hwmsgbox_disable_receiver_int(ARISC_HWMSGBOX_ARISC_SYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);

	return 0;
}

int arisc_hwmsgbox_standby_resume(void)
{
	/* enable arisc asyn tx interrupt */
	//arisc_hwmsgbox_enable_receiver_int(ARISC_HWMSGBOX_ARISC_ASYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);

	/* enable arisc syn tx interrupt */
	//arisc_hwmsgbox_enable_receiver_int(ARISC_HWMSGBOX_ARISC_SYN_TX_CH, AW_HWMSG_QUEUE_USER_AC327);

	return 0;
}
