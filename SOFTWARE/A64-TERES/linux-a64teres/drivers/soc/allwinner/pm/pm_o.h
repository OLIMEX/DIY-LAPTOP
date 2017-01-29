#ifndef _PM_O_H
#define _PM_O_H

/*
 * Copyright (c) 2011-2015 njubie@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include "pm.h"
//#include "mem_mapping.h"
#define  print_call_info(...)({                                                                         \
                do{                                                                                                                     \
                        printk("%s, %s, %d. \n" , __FILE__, __func__, __LINE__);\
                }while(0);})

#endif /*_PM_O_H*/

