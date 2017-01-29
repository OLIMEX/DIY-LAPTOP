#include "pm_i.h"

static struct errstr_t {
    int last_err;
    const char **error_array;	    //save string info for corresponding err code.
}pm_errstr_array[MOD_MAX_COUNT];

static const char *pm_errstr_suspend_cpu0[ERR_SUSPEND_CPUXSYS_MAX_];
static void suspend_cpu0_errstr_init(void)
{
    pm_errstr_suspend_cpu0[error_get_module_error(ERR_SUSPEND_CPUXSYS_SHOW_DEVICES_STATE_DONE)]	= "ERR_SUSPEND_CPUXSYS_SHOW_DEVICES_STATE_DONE";
    pm_errstr_suspend_cpu0[error_get_module_error(ERR_SUSPEND_CPUXSYS_CONFIG_MEM_PARA_DONE)]	= "ERR_SUSPEND_CPUXSYS_CONFIG_MEM_PARA_DONE";
    pm_errstr_suspend_cpu0[error_get_module_error(ERR_SUSPEND_CPUXSYS_CONFIG_SUPER_PARA_DONE)]	= "ERR_SUSPEND_CPUXSYS_CONFIG_SUPER_PARA_DONE";
    pm_errstr_suspend_cpu0[error_get_module_error(ERR_SUSPEND_CPUXSYS_CONFIG_WAKEUP_SRC_DONE)]	= "ERR_SUSPEND_CPUXSYS_CONFIG_WAKEUP_SRC_DONE";
    pm_errstr_array[MOD_SUSPEND_CPUXSYS].last_err = error_get_module_error(ERR_SUSPEND_CPUXSYS_MAX_) -1;
    pm_errstr_array[MOD_SUSPEND_CPUXSYS].error_array = pm_errstr_suspend_cpu0;

}

static const char *pm_errstr_resume_cpu0[ERR_RESUME_CPUXSYS_MAX_];
static void resume_cpu0_errstr_init(void)
{
    pm_errstr_resume_cpu0[error_get_module_error(ERR_RESUME_CPUXSYS_CONFIG_WAKEUP_SRC_DONE)]	= "ERR_RESUME_CPUXSYS_CONFIG_WAKEUP_SRC_DONE";
    pm_errstr_resume_cpu0[error_get_module_error(ERR_RESUME_CPUXSYS_RESUME_DEVICES_DONE)]	= "ERR_RESUME_CPUXSYS_RESUME_DEVICES_DONE";
    pm_errstr_resume_cpu0[error_get_module_error(ERR_RESUME_CPUXSYS_RESUME_CPUXSYS_DONE)]	= "ERR_RESUME_CPUXSYS_RESUME_CPUXSYS_DONE";
    pm_errstr_resume_cpu0[error_get_module_error(ERR_RESUME_CPUXSYS_CONFIG_WAKEUP_SRC_DONE)]	= "ERR_RESUME_CPUXSYS_CONFIG_WAKEUP_SRC_DONE";
    pm_errstr_array[MOD_RESUME_CPUXSYS].last_err = error_get_module_error(ERR_RESUME_CPUXSYS_MAX_) -1;
    pm_errstr_array[MOD_RESUME_CPUXSYS].error_array = pm_errstr_resume_cpu0;

}


static const char *pm_errstr_first_boot_flag[ERR_FIRST_BOOT_FLAG_MAX_];
static void first_boot_flag_errstr_init(void)
{
    pm_errstr_first_boot_flag[error_get_module_error(ERR_FIRST_BOOT_FLAG_DONE)]	= "ERR_FIRST_BOOT_FLAG_DONE";
    pm_errstr_array[MOD_FIRST_BOOT_FLAG].last_err = error_get_module_error(ERR_FIRST_BOOT_FLAG_MAX_) -1;
    pm_errstr_array[MOD_FIRST_BOOT_FLAG].error_array = pm_errstr_first_boot_flag;

}

static const char *pm_errstr_freezer_thread[ERR_FREEZER_THREAD_MAX_];
static void freezer_thread_errstr_init(void)
{
    pm_errstr_freezer_thread[error_get_module_error(ERR_FREEZER_THREAD_DONE)]	= "ERR_FREEZER_THREAD_DONE";
    pm_errstr_array[MOD_FREEZER_THREAD].last_err = error_get_module_error(ERR_FREEZER_THREAD_MAX_) -1;
    pm_errstr_array[MOD_FREEZER_THREAD].error_array = pm_errstr_freezer_thread;

}

static const char *pm_errstr_suspend_devices[ERR_SUSPEND_DEVICES_MAX_];
static void suspend_devices_errstr_init(void)
{
    pm_errstr_suspend_devices[error_get_module_error(ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_DONE)]	= "ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_DONE";
    pm_errstr_suspend_devices[error_get_module_error(ERR_SUSPEND_DEVICES_LATE_SUSPEND_DEVICES_DONE)]	= "ERR_SUSPEND_DEVICES_LATE_SUSPEND_DEVICES_DONE";
    pm_errstr_suspend_devices[error_get_module_error(ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_FAILED)]	= "ERR_SUSPEND_DEVICES_SUSPEND_DEVICES_FAILED";
    pm_errstr_array[MOD_SUSPEND_DEVICES].last_err = error_get_module_error(ERR_SUSPEND_DEVICES_MAX_) -1;
    pm_errstr_array[MOD_SUSPEND_DEVICES].error_array = pm_errstr_suspend_devices;

}

static const char *pm_errstr_suspend_platform[ERR_SUSPEND_PLATFORM_MAX_];
static void suspend_platform_errstr_init(void)
{
    pm_errstr_suspend_platform[error_get_module_error(ERR_SUSPEND_PLATFORM_DONE)]	= "ERR_SUSPEND_PLATFORM_DONE";
    pm_errstr_array[MOD_SUSPEND_PLATFORM].last_err = error_get_module_error(ERR_SUSPEND_PLATFORM_MAX_) -1;
    pm_errstr_array[MOD_SUSPEND_PLATFORM].error_array = pm_errstr_suspend_platform;

}

static const char *pm_errstr_suspend_processors[ERR_SUSPEND_PROCESSORS_MAX_];
static void suspend_processors_errstr_init(void)
{
    pm_errstr_suspend_processors[error_get_module_error(ERR_SUSPEND_PROCESSORS_DONE)]	= "ERR_SUSPEND_PROCESSORS_DONE";
    pm_errstr_array[MOD_SUSPEND_PROCESSORS].last_err = error_get_module_error(ERR_SUSPEND_PROCESSORS_MAX_) -1;
    pm_errstr_array[MOD_SUSPEND_PROCESSORS].error_array = pm_errstr_suspend_processors;

}


static const char *pm_errstr_suspend_core[ERR_SUSPEND_CORE_MAX_];
static void suspend_core_errstr_init(void)
{
    pm_errstr_suspend_core[error_get_module_error(ERR_SUSPEND_CORE_DONE)]	= "ERR_SUSPEND_CORE_DONE";
    pm_errstr_array[MOD_SUSPEND_CORE].last_err = error_get_module_error(ERR_SUSPEND_CORE_MAX_) -1;
    pm_errstr_array[MOD_SUSPEND_CORE].error_array = pm_errstr_suspend_core;

}

static const char *pm_errstr_resume_devices[ERR_RESUME_DEVICES_MAX_];
static void resume_devices_errstr_init(void)
{
    pm_errstr_resume_devices[error_get_module_error(ERR_RESUME_DEVICES_EARLY_RESUME_DEVICES_DONE)]	= "ERR_RESUME_DEVICES_EARLY_RESUME_DEVICES_DONE";
    pm_errstr_resume_devices[error_get_module_error(ERR_RESUME_DEVICES_RESUME_DEVICES_DONE)]		= "ERR_RESUME_DEVICES_RESUME_DEVICES_DONE";
    pm_errstr_array[MOD_RESUME_DEVICES].last_err = error_get_module_error(ERR_RESUME_DEVICES_MAX_) -1;
    pm_errstr_array[MOD_RESUME_DEVICES].error_array = pm_errstr_resume_devices;

}

static const char *pm_errstr_resume_platform[ERR_RESUME_PLATFORM_MAX_];
static void resume_platform_errstr_init(void)
{
    pm_errstr_resume_platform[error_get_module_error(ERR_RESUME_PLATFORM_DONE)]	= "ERR_RESUME_PLATFORM_DONE";
    pm_errstr_array[MOD_RESUME_PLATFORM].last_err = error_get_module_error(ERR_RESUME_PLATFORM_MAX_) -1;
    pm_errstr_array[MOD_RESUME_PLATFORM].error_array = pm_errstr_resume_platform;

}

static const char *pm_errstr_resume_processors[ERR_RESUME_PROCESSORS_MAX_];
static void resume_processors_errstr_init(void)
{
    pm_errstr_resume_processors[error_get_module_error(ERR_RESUME_PROCESSORS_DONE)]	= "ERR_RESUME_PROCESSORS_DONE";
    pm_errstr_array[MOD_RESUME_PROCESSORS].last_err = error_get_module_error(ERR_RESUME_PROCESSORS_MAX_) -1;
    pm_errstr_array[MOD_RESUME_PROCESSORS].error_array = pm_errstr_resume_processors;

}


static const char *pm_errstr_resume_core[ERR_RESUME_CORE_MAX_];
static void resume_core_errstr_init(void)
{
    pm_errstr_resume_core[error_get_module_error(ERR_RESUME_CORE_DONE)]	= "ERR_RESUME_CORE_DONE";
    pm_errstr_array[MOD_RESUME_CORE].last_err = error_get_module_error(ERR_RESUME_CORE_MAX_) -1;
    pm_errstr_array[MOD_RESUME_CORE].error_array = pm_errstr_resume_core;

}

static const char *pm_errstr_thaw_thread[ERR_THAW_THREAD_MAX_];
static void thaw_thread_errstr_init(void)
{
    pm_errstr_thaw_thread[error_get_module_error(ERR_THAW_THREAD_DONE)]	= "ERR_THAW_THREAD_DONE";
    pm_errstr_array[MOD_THAW_THREAD].last_err = error_get_module_error(ERR_THAW_THREAD_MAX_) -1;
    pm_errstr_array[MOD_THAW_THREAD].error_array = pm_errstr_thaw_thread;

}

static const char *pm_errstr_resume_complete_flag[ERR_RESUME_COMPLETE_FLAG_MAX_];
static void resume_complete_flag_errstr_init(void)
{
    pm_errstr_resume_complete_flag[error_get_module_error(ERR_RESUME_COMPLETE_FLAG_DONE)]	= "ERR_RESUME_COMPLETE_FLAG_DONE";
    pm_errstr_array[MOD_RESUME_COMPLETE_FLAG].last_err = error_get_module_error(ERR_RESUME_COMPLETE_FLAG_MAX_) -1;
    pm_errstr_array[MOD_RESUME_COMPLETE_FLAG].error_array = pm_errstr_resume_complete_flag;

}
static void errstr_init(void)
{
    suspend_cpu0_errstr_init();
    resume_cpu0_errstr_init();
    first_boot_flag_errstr_init();
    resume_complete_flag_errstr_init();
    suspend_devices_errstr_init();
    suspend_platform_errstr_init();
    suspend_processors_errstr_init();
    suspend_core_errstr_init();
    resume_devices_errstr_init();
    resume_platform_errstr_init();
    resume_processors_errstr_init();
    resume_core_errstr_init();
    resume_cpu0_errstr_init();
    freezer_thread_errstr_init();
    thaw_thread_errstr_init();

    return ;
}

const char *pm_errstr(error_t error)
{
    static bool inited = false;
    short module_id = error_get_module_id(error);
    short error_id = error_get_module_error(error);
    if(false == inited){
	errstr_init();
	inited = true;
    }

    if(error_id > pm_errstr_array[module_id].last_err){
	return "ERR_ERRSTR_EXCEED_MAX_ID"; 
    }

    if((NULL == pm_errstr_array[module_id].error_array) ||  (NULL == pm_errstr_array[module_id].error_array[error_id]))
	return "ERR_ERRSTR_NOT_DEFINED";

    return pm_errstr_array[module_id].error_array[error_id];

}

