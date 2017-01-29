#ifndef _POWER_H
#define _POWER_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
extern void power_enter_super(struct aw_pm_info * config, extended_standby_t * extended_config);
extern void dm_suspend(struct aw_pm_info * config, extended_standby_t * extended_config);
extern void dm_resume(extended_standby_t * extended_config);
#endif /*_PM_H*/

