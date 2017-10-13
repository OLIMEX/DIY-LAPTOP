#ifndef _PM_ERR_CODE_H
#define _PM_ERR_CODE_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#define MODULE_BITS	(4)
#define ERROR_BITS	(4)
typedef __u32 error_t;
#define ERROR_BEGIN_OF_MODULE(module_id) (0)
static inline error_t error_begin_of_module(const short module_id)
{
    return (module_id<<(ERROR_BITS));
}

static inline short error_get_module_error(const int error)
{
    return (error & ((1<<(ERROR_BITS)) - 1));
}
 
static inline short error_get_module_id(const int error)
{
    return ((error) >> ERROR_BITS);
}

static inline error_t error_gen(const short module_id, const short module_error)
{
    return (error_begin_of_module(module_id) | module_error);
}

enum module_ids {
	MOD_FIRST_BOOT_FLAG	    ,	
	MOD_FREEZER_THREAD	    ,
	MOD_SUSPEND_DEVICES	    ,
	MOD_SUSPEND_PLATFORM	    ,
	MOD_SUSPEND_PROCESSORS	    ,
	MOD_SUSPEND_CORE	    ,	//notice: 0x5a, is occupied for upgrade;
	MOD_SUSPEND_CPUXSYS	    ,	
	MOD_RESUME_CPUXSYS	    ,	
	MOD_RESUME_CORE	    ,	
	MOD_RESUME_PROCESSORS	    ,
	MOD_RESUME_PLATFORM	    ,
	MOD_RESUME_DEVICES	    ,
	MOD_THAW_THREAD	    ,
	MOD_RESUME_COMPLETE_FLAG    ,
	MOD_MAX_COUNT
};

#if defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN9IW1P1)
 #define BOOT_UPGRADE_FLAG		(0x5a)		     //Notice: 0x5a is occupied by boot for upgrade.
#else 
 #define BOOT_UPGRADE_FLAG		(0x5AA5A55A)	    //Notice: 0x5a is occupied by boot for upgrade.
#endif

enum err_first_boot_flag {
    ERR_FIRST_BOOT_FLAG_DONE = ERROR_BEGIN_OF_MODULE(MOD_FIRST_BOOT_FLAG),
    ERR_FIRST_BOOT_FLAG_MAX_
};

enum err_thaw_thread {
    ERR_THAW_THREAD_DONE = ERROR_BEGIN_OF_MODULE(MOD_THAW_THREAD),
    ERR_THAW_THREAD_MAX_
};

enum err_resume_complete_flag {
    ERR_RESUME_COMPLETE_FLAG_DONE = ERROR_BEGIN_OF_MODULE(MOD_RESUME_COMPLETE_FLAG),
    ERR_RESUME_COMPLETE_FLAG_MAX_
};

enum err_freezer_thread {
    ERR_FREEZER_THREAD_DONE = ERROR_BEGIN_OF_MODULE(MOD_FREEZER_THREAD),
    ERR_FREEZER_THREAD_MAX_
};

enum err_suspend_platform {
    ERR_SUSPEND_PLATFORM_DONE = ERROR_BEGIN_OF_MODULE(MOD_SUSPEND_PLATFORM),
    ERR_SUSPEND_PLATFORM_MAX_
};

enum err_suspend_processors {
    ERR_SUSPEND_PROCESSORS_DONE = ERROR_BEGIN_OF_MODULE(MOD_SUSPEND_PROCESSORS),
    ERR_SUSPEND_PROCESSORS_MAX_
};

enum err_suspend_core {
    ERR_SUSPEND_CORE_DONE = ERROR_BEGIN_OF_MODULE(MOD_SUSPEND_CORE),
    ERR_SUSPEND_CORE_MAX_
};

enum err_resume_core {
    ERR_RESUME_CORE_DONE = ERROR_BEGIN_OF_MODULE(MOD_RESUME_CORE),
    ERR_RESUME_CORE_MAX_
};

enum err_resume_processors {
    ERR_RESUME_PROCESSORS_DONE = ERROR_BEGIN_OF_MODULE(MOD_RESUME_PROCESSORS),
    ERR_RESUME_PROCESSORS_MAX_
};

enum err_resume_platform {
    ERR_RESUME_PLATFORM_DONE = ERROR_BEGIN_OF_MODULE(MOD_RESUME_PLATFORM),
    ERR_RESUME_PLATFORM_MAX_
};

enum err_suspend_devices {
    ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_DONE = ERROR_BEGIN_OF_MODULE(MOD_SUSPEND_DEVICES),
    ERR_SUSPEND_DEVICES_LATE_SUSPEND_DEVICES_DONE,
    ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_FAILED,
    ERR_SUSPEND_DEVICES_MAX_
};

enum err_resume_devices {
    ERR_RESUME_DEVICES_EARLY_RESUME_DEVICES_DONE = ERROR_BEGIN_OF_MODULE(MOD_RESUME_DEVICES),
    ERR_RESUME_DEVICES_RESUME_DEVICES_DONE,
    ERR_RESUME_DEVICES_MAX_
};


enum err_resume_cpu0 {
    ERR_RESUME_CPUXSYS_CONFIG_WAKEUP_SRC_DONE = ERROR_BEGIN_OF_MODULE(MOD_RESUME_CPUXSYS),
    ERR_RESUME_CPUXSYS_RESUME_DEVICES_DONE,  
    ERR_RESUME_CPUXSYS_RESUME_CPUXSYS_DONE,
    ERR_RESUME_CPUXSYS_MAX_
};

enum err_suspend_cpu0 {
    ERR_SUSPEND_CPUXSYS_SHOW_DEVICES_STATE_DONE = ERROR_BEGIN_OF_MODULE(MOD_SUSPEND_CPUXSYS),
    ERR_SUSPEND_CPUXSYS_CONFIG_MEM_PARA_DONE,
    ERR_SUSPEND_CPUXSYS_CONFIG_SUPER_PARA_DONE,
    ERR_SUSPEND_CPUXSYS_CONFIG_WAKEUP_SRC_DONE,
    ERR_SUSPEND_CPUXSYS_MAX_
};
const char *pm_errstr(error_t error);
#endif /*_PM_ERR_CODE_H*/

