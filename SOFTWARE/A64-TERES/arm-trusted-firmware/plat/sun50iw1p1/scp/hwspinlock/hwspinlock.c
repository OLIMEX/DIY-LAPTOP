/*
 *  arch/arm/mach-sunxi/arisc/hwspinlock/hwspinlock.c
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

#include "hwspinlock_i.h"

static struct arisc_hwspinlock arisc_hwspinlocks[ARISC_HW_SPINLOCK_NUM];
static uintptr_t base;

/**
 * initialize hwspinlock.
 * @para:  none.
 *
 * returns:  0 if initialize hwspinlock succeeded, others if failed.
 */
int arisc_hwspinlock_init(void)
{
	base = dts_cfg.hwspinlock.base;

	return 0;
}

/**
 * exit hwspinlock.
 * @para:none.
 *
 * returns:  0 if exit hwspinlock succeeded, others if failed.
 */
int arisc_hwspinlock_exit(void)
{
	return 0;
}

/**
 * lock an hwspinlock with timeout limit,
 * and hwspinlock will be unlocked in arisc_hwspin_unlock().
 * @hwid: an hwspinlock id which we want to lock.
 *
 * returns:  0 if lock hwspinlock succeeded, other if failed.
 */
int arisc_hwspin_lock(int hwid)
{
	arisc_hwspinlock_t *spinlock;

	if (hwid >= ARISC_HW_SPINLOCK_NUM) {
		ARISC_ERR("invalid hwspinlock id [%d] for trylock\n", hwid);
		return -EINVAL;
	}
	spinlock = &(arisc_hwspinlocks[hwid]);

	/* is lock already taken by another context on the local cpu ? */
	spin_lock(&(spinlock->lock));

	/* try to take spinlock */
	while (readl(base + AW_SPINLOCK_LOCK_REG(hwid)) == AW_SPINLOCK_TAKEN);

	return 0;
}

/**
 * unlock a specific hwspinlock.
 * hwid:  an hwspinlock id which we want to unlock.
 *
 * returns:  0 if unlock hwspinlock succeeded, other if failed.
 */
int arisc_hwspin_unlock(int hwid)
{
	arisc_hwspinlock_t *spinlock;

	if (hwid >= ARISC_HW_SPINLOCK_NUM) {
		ARISC_ERR("invalid hwspinlock id [%d] for unlock\n", hwid);
		return -EINVAL;
	}
	spinlock = &(arisc_hwspinlocks[hwid]);

	/* untaken the spinlock */
	writel(0x0, base + AW_SPINLOCK_LOCK_REG(hwid));

	spin_unlock(&(spinlock->lock));

	return 0;
}

int arisc_hwspinlock_standby_suspend(void)
{
	return 0;
}

int arisc_hwspinlock_standby_resume(void)
{
	return 0;
}
