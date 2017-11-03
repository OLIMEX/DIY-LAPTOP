/*
 *  arch/arm/mach-sunxi/arisc/message_manager/message_manager.c
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-05-01 Written by sunny (sunny@allwinnertech.com).
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

#include <linux/hwspinlock.h>
#include "message_manager_i.h"

/* the start and end of message pool */
static struct arisc_message *message_start;
static struct arisc_message *message_end;

/* spinlock for this module */
static spinlock_t    msg_mgr_lock;

/* message cache manager */
static struct arisc_message_cache message_cache;

/* semaphore cache manager */
static struct arisc_semaphore_cache sem_cache;

static atomic_t sem_used_num;

static void *arisc_message_pool_base;
static u32 arisc_message_pool_size;

static struct hwspinlock *msg_mgr_hwlock;

/**
 * initialize message manager.
 * @para:  none.
 *
 * returns:  0 if initialize succeeded, others if failed.
 */
int arisc_message_manager_init(void *addr, u32 size)
{
	int i;

	msg_mgr_hwlock = hwspin_lock_request_specific(SUNXI_MSG_HWSPINLOCK);
	if (!msg_mgr_hwlock)
		pr_err("%s,%d request hwspinlock faild!\n", __func__, __LINE__);

	arisc_message_pool_base = addr;
	arisc_message_pool_size = size;

	/* initialize message pool start and end */
	message_start = (struct arisc_message *)(arisc_message_pool_base);
	message_end   = (struct arisc_message *)(arisc_message_pool_base + arisc_message_pool_size);

	memset((void *)message_start, 0, arisc_message_pool_size);

	/* initialize message_cache */
	for (i = 0; i < ARISC_MESSAGE_CACHED_MAX; i++) {
		message_cache.cache[i] = NULL;
	}
	atomic_set(&(message_cache.number), 0);

	/* initialzie semaphore allocator */
	for (i = 0; i < ARISC_SEM_CACHE_MAX; i++) {
		sem_cache.cache[i] = NULL;
	}
	atomic_set(&(sem_cache.number), 0);
	atomic_set(&sem_used_num, 0);

	/* initialize message manager spinlock */
	spin_lock_init(&(msg_mgr_lock));

	return 0;
}

/**
 * exit message manager.
 * @para:  none.
 *
 * returns:  0 if exit succeeded, others if failed.
 */
int arisc_message_manager_exit(void)
{
	int ret;

	ret = hwspin_lock_free(msg_mgr_hwlock);
	if (ret)
		pr_err("%s,%d free hwlock faild!\n", __func__, __LINE__);

	return 0;
}

static int arisc_semaphore_invalid(struct semaphore *psemaphore)
{
	/* semaphore use system kmalloc, valid range check */
	//if ((psemaphore >= ((struct semaphore *)(0xC0000000))) &&
	//  (psemaphore <  ((struct semaphore *)(0xF0000000))))
	if (psemaphore) {
		/* valid arisc semaphore */
		return 0;
	}
	/* invalid arisc semaphore */
	return 1;
}

static struct semaphore *arisc_semaphore_allocate(void)
{
	struct semaphore *sem = NULL;
	unsigned long msg_flags;

	/* try to allocate from cache first */
	spin_lock_irqsave(&msg_mgr_lock, msg_flags);
	if (atomic_read(&(sem_cache.number))) {
		atomic_dec(&(sem_cache.number));
		sem = sem_cache.cache[atomic_read(&(sem_cache.number))];
		sem_cache.cache[atomic_read(&(sem_cache.number))] = NULL;
		if (arisc_semaphore_invalid(sem)) {
			ARISC_ERR("allocate cache semaphore [%p] invalid\n", sem);
		}
	}
	spin_unlock_irqrestore(&msg_mgr_lock, msg_flags);

	if (arisc_semaphore_invalid(sem)) {
		/* cache allocate fail, allocate from kmem */
		sem = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	}
	/* check allocate semaphore valid or not */
	if (arisc_semaphore_invalid(sem)) {
		ARISC_ERR("allocate semaphore [%p] invalid\n", sem);
		return NULL;
	}

	/* initialize allocated semaphore */
	sema_init(sem, 0);
	atomic_inc(&sem_used_num);

	return sem;
}

static int arisc_semaphore_free(struct semaphore *sem)
{
	struct semaphore *free_sem = sem;
	unsigned long msg_flags;

	if (arisc_semaphore_invalid(free_sem)) {
		ARISC_ERR("free semaphore [%p] invalid\n", free_sem);

		return -1;
	}

	/* try to free semaphore to cache */
	spin_lock_irqsave(&msg_mgr_lock, msg_flags);
	if (atomic_read(&(sem_cache.number)) < ARISC_SEM_CACHE_MAX) {
		sem_cache.cache[atomic_read(&(sem_cache.number))] = free_sem;
		atomic_inc(&(sem_cache.number));
		free_sem = NULL;
	}
	spin_unlock_irqrestore(&msg_mgr_lock, msg_flags);

	/* try to free semaphore to kmem if free to cache fail */
	if (free_sem) {
		/* free to kmem */
		kfree(free_sem);
	}

	atomic_dec(&sem_used_num);

	return 0;
}

int arisc_semaphore_used_num_query(void)
{
	return atomic_read(&sem_used_num);
}

static int arisc_message_invalid(struct arisc_message *pmessage)
{
	if ((pmessage >= message_start) &&
		(pmessage < message_end)) {
		/* valid arisc message */
		return 0;
	}
	/* invalid arisc message */
	return 1;
}

/**
 * allocate one message frame. mainly use for send message by message-box,
 * the message frame allocate form messages pool shared memory area.
 * @para:  none.
 *
 * returns:  the pointer of allocated message frame, NULL if failed;
 */
struct arisc_message *arisc_message_allocate(unsigned int msg_attr)
{
	struct arisc_message *pmessage = NULL;
	struct arisc_message *palloc   = NULL;
	unsigned long hwflags;
	unsigned long msg_flags;

	if (arisc_suspend_flag_query() && (msg_attr == ARISC_MESSAGE_ATTR_SOFTSYN)) {
		msg_attr = ARISC_MESSAGE_ATTR_HARDSYN;
	}

	/* first find in message_cache */
	spin_lock_irqsave(&msg_mgr_lock, msg_flags);
	if (atomic_read(&(message_cache.number))) {
		ARISC_INF("arisc message_cache.number = 0x%x.\n", atomic_read(&(message_cache.number)));
		atomic_dec(&(message_cache.number));
		palloc = message_cache.cache[atomic_read(&(message_cache.number))];
		ARISC_INF("message [%p] allocate from message_cache\n", palloc);
		if (arisc_message_invalid(palloc)) {
			ARISC_ERR("allocate cache message [%p] invalid\n", palloc);
		}
	}
	spin_unlock_irqrestore(&msg_mgr_lock, msg_flags);
	if (arisc_message_invalid(palloc)) {
		/*
		 * cached message_cache finded fail,
		 * use spinlock 0 to exclusive with arisc.
		 */
		hwspin_lock_timeout_irqsave(msg_mgr_hwlock, ARISC_SPINLOCK_TIMEOUT, &hwflags);

		/* search from the start of message pool every time. */
		pmessage = message_start;
		while (pmessage < message_end) {
			if (pmessage->state == ARISC_MESSAGE_FREED) {
				/* find free message in message pool, allocate it */
				palloc = pmessage;
				palloc->state = ARISC_MESSAGE_ALLOCATED;
				ARISC_INF("message [%p] allocate from message pool\n", palloc);
				break;
			}
			/* next message frame */
			pmessage++;
		}
		/* unlock hwspinlock 0 */
		hwspin_unlock_irqrestore(msg_mgr_hwlock, &hwflags);
	}
	if (arisc_message_invalid(palloc)) {
		ARISC_ERR("allocate message [%p] frame is invalid\n", palloc);
		return NULL;
	}
	/* initialize messgae frame */
	palloc->next = NULL;
	palloc->attr = msg_attr;

	if (msg_attr & ARISC_MESSAGE_ATTR_SOFTSYN) {
		/* syn message,allocate one semaphore for private */
		palloc->private = arisc_semaphore_allocate();
	} else {
		palloc->private = NULL;
	}
	return palloc;
}

/**
 * free one message frame. mainly use for process message finished,
 * free it to messages pool or add to free message queue.
 * @pmessage:  the pointer of free message frame.
 *
 * returns:  none.
 */
void arisc_message_free(struct arisc_message *pmessage)
{
	struct arisc_message *free_message = pmessage;
	unsigned long hwflags;
	unsigned long msg_flags;

	/* check this message valid or not */
	if (arisc_message_invalid(free_message)) {
		ARISC_ERR("free invalid arisc message [%p]\n", free_message);
		return;
	}
	if (free_message->attr & ARISC_MESSAGE_ATTR_SOFTSYN) {
		/* free message semaphore first */
		arisc_semaphore_free((struct semaphore *)(free_message->private));
		free_message->private = NULL;
	}
	/* try to free to free_list first */
	spin_lock_irqsave(&msg_mgr_lock, msg_flags);
	if (atomic_read(&(message_cache.number)) < ARISC_MESSAGE_CACHED_MAX) {
		ARISC_INF("insert message [%p] to message_cache\n", free_message);
		ARISC_INF("message_cache number : %d\n", atomic_read(&(message_cache.number)));
		/* cached this message, message state: ALLOCATED */
		message_cache.cache[atomic_read(&(message_cache.number))] = free_message;
		atomic_inc(&(message_cache.number));
		free_message->next = NULL;
		free_message->state = ARISC_MESSAGE_ALLOCATED;
		free_message = NULL;
	}
	spin_unlock_irqrestore(&msg_mgr_lock, msg_flags);

	/*  try to free message to pool if free to cache fail */
	if (free_message) {
		/* free to message pool,set message state as FREED. */
		hwspin_lock_timeout_irqsave(msg_mgr_hwlock, ARISC_SPINLOCK_TIMEOUT, &hwflags);
		ARISC_INF("insert message [%p] to message pool\n", free_message);
		free_message->state = ARISC_MESSAGE_FREED;
		free_message->next  = NULL;
		hwspin_unlock_irqrestore(msg_mgr_hwlock, &hwflags);
	}
}

/**
 * notify system that one message coming.
 * @pmessage:  the pointer of coming message frame.
 *
 * returns:  0 if notify succeeded, other if failed.
 */
int arisc_message_coming_notify(struct arisc_message *pmessage)
{
	int   ret;

	/* ac327 receive message to arisc */
	ARISC_INF("-------------------------------------------------------------\n");
	ARISC_INF("                MESSAGE FROM ARISC                           \n");
	ARISC_INF("message addr : %p\n", pmessage);
	ARISC_INF("message type : %x\n", pmessage->type);
	ARISC_INF("message attr : %x\n", pmessage->attr);
	ARISC_INF("-------------------------------------------------------------\n");

	/* message per-process */
	pmessage->state = ARISC_MESSAGE_PROCESSING;

	/* process message */
	switch (pmessage->type) {
		case ARISC_AXP_INT_COMING_NOTIFY: {
			ARISC_INF("pmu interrupt coming notify\n");
			ret = arisc_axp_int_notify(pmessage);
			pmessage->result = ret;
			break;
		}
		case ARISC_AUDIO_PERDONE_NOTIFY: {
			ARISC_INF("audio perdone notify\n");
			ret = arisc_audio_perdone_notify(pmessage);
			pmessage->result = ret;
			break;
		}
		case ARISC_REPORT_ERR_INFO: {
			ARISC_INF("arisc report error info\n");
			ret = arisc_report_error_info(pmessage);
			pmessage->result = ret;
			break;
		}

		default : {
			ARISC_ERR("invalid message type for ac327 process\n");
			ARISC_ERR("message addr   : %p\n", pmessage);
			ARISC_ERR("message state  : %x\n", pmessage->state);
			ARISC_ERR("message attr   : %x\n", pmessage->attr);
			ARISC_ERR("message type   : %x\n", pmessage->type);
			ARISC_ERR("message result : %x\n", pmessage->result);
			ret = -EINVAL;
			break;
		}
	}
	/* message post process */
	pmessage->state = ARISC_MESSAGE_PROCESSED;
	if ((pmessage->attr & ARISC_MESSAGE_ATTR_SOFTSYN) ||
		(pmessage->attr & ARISC_MESSAGE_ATTR_HARDSYN)) {
		/* synchronous message, should feedback process result */
		arisc_hwmsgbox_feedback_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
	} else {
		/*
		 * asyn message, no need feedback message result,
		 * free message directly.
		 */
		arisc_message_free(pmessage);
	}

	return ret;
}

struct arisc_message *arisc_message_map_to_cpux(u32 addr)
{
	struct arisc_message *message;
	message = (struct arisc_message *)((ptrdiff_t)addr + (ptrdiff_t)arisc_message_pool_base);

	return message;
}

u32 arisc_message_map_to_cpus(struct arisc_message *message)
{
	u32 value = (u32)((ptrdiff_t)message - (ptrdiff_t)arisc_message_pool_base);

	return value;
}

int arisc_message_valid(struct arisc_message *pmessage)
{
	if ((pmessage >= message_start) && (pmessage <  message_end)) {
		/* valid message */
		return 1;
	}

	return 0;
}

