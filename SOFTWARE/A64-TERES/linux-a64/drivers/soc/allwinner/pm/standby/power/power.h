#ifndef _POWER_H
#define _POWER_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
extern s32 axp19x_set_volt(u32 id, u32 voltage);
extern s32 axp19x_get_volt(u32 id);
extern s32 axp19x_set_state(u32 id, u32 state);
extern s32 axp19x_get_state(u32 id);
extern s32 axp19x_suspend(u32 id);
extern s32 axp152_set_volt(u32 id, u32 voltage);
extern s32 axp152_get_volt(u32 id);
extern s32 axp152_set_state(u32 id, u32 state);
extern s32 axp152_get_state(u32 id);
extern s32 axp152_suspend(u32 id);
#endif /*_PM_H*/

