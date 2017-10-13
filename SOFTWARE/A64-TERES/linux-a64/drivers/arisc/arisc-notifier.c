/*
 * linux/driver/arisc/arisc-notifer.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *         http://www.allwinnertech.com
 *
 * Author: sunny <sunny@allwinnertech.com>
 *
 * arisc notifier framework.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/arisc/arisc-notifier.h>

/* arisc notifer */
static RAW_NOTIFIER_HEAD(arisc_notifier);

/* Serializes the operations to arisc_notifier */
static DEFINE_MUTEX(arisc_notifier_mutex);

/*
 * The following two API's must be used when attempting
 * to serialize the operations to arisc_notifier.
 */
static void arisc_notifier_lock(void)
{
	mutex_lock(&arisc_notifier_mutex);
}

static void arisc_notifier_unlock(void)
{
	mutex_unlock(&arisc_notifier_mutex);
}

int arisc_register_notifier(struct notifier_block *nb)
{
	int ret;
	arisc_notifier_lock();
	ret = raw_notifier_chain_register(&arisc_notifier, nb);
	arisc_notifier_unlock();
	return ret;
}
EXPORT_SYMBOL(arisc_register_notifier);

int arisc_notify(unsigned long val, void *v)
{
	int ret;
	ret = __raw_notifier_call_chain(&arisc_notifier, val, v, -1, NULL);
	return notifier_to_errno(ret);
}
EXPORT_SYMBOL(arisc_notify);

void arisc_unregister_notifier(struct notifier_block *nb)
{
	arisc_notifier_lock();
	raw_notifier_chain_unregister(&arisc_notifier, nb);
	arisc_notifier_unlock();
}
EXPORT_SYMBOL(arisc_unregister_notifier);

