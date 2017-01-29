
#ifndef _LINUX_SCENELOCK_DATA_H
#define _LINUX_SCENELOCK_DATA_H

#include <linux/power/axp_depend.h>
#ifdef CONFIG_ARCH_SUN50IW1P1
#include "scenelock_data_sun50iw1p1.h"
#elif defined(CONFIG_ARCH_SUN8IW6P1)
#include "scenelock_data_sun8iw6p1.h"
#elif defined(CONFIG_ARCH_SUN8IW8P1)
#include "scenelock_data_sun8iw8p1.h"
#elif defined(CONFIG_ARCH_SUN8IW10P1)
#include "scenelock_data_sun8iw10p1.h"
#elif defined(CONFIG_ARCH_SUN9IW1P1)
#include "scenelock_data_sun9iw1p1.h"
#endif

int extended_standby_cnt = sizeof(extended_standby)/sizeof(extended_standby[0]);

#endif

