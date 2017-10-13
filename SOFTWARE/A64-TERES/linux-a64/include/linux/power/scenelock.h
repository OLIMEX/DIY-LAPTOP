/* include/linux/scenelock.h
 *
 * Copyright (C) 2013-2014 allwinner.
 *
 *  By      : liming
 *  Version : v1.0
 *  Date    : 2013-4-17 09:08
 */

#ifndef _LINUX_SCENELOCK_H
#define _LINUX_SCENELOCK_H

#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/power/aw_pm.h>

typedef enum AW_POWER_SCENE
{
	SCENE_TALKING_STANDBY = 1,
	SCENE_USB_STANDBY,
	SCENE_MP3_STANDBY,
	SCENE_BOOT_FAST,
	SCENE_SUPER_STANDBY,
	SCENE_GPIO_STANDBY,
	SCENE_GPIO_HOLD_STANDBY,
	SCENE_NORMAL_STANDBY,
	SCENE_MISC_STANDBY,
	SCENE_MISC1_STANDBY,
	SCENE_DYNAMIC_STANDBY,
	SCENE_USB_OHCI_STANDBY,
	SCENE_USB_EHCI_STANDBY,
	SCENE_MAX
} aw_power_scene_e;

typedef enum POWER_SCENE_FLAGS
{
	TALKING_STANDBY_FLAG           = (1<<0x0),
	USB_STANDBY_FLAG               = (1<<0x1),
	MP3_STANDBY_FLAG               = (1<<0x2),
	SUPER_STANDBY_FLAG             = (1<<0x3),
	NORMAL_STANDBY_FLAG            = (1<<0x4),
	GPIO_STANDBY_FLAG              = (1<<0x5),
	MISC_STANDBY_FLAG              = (1<<0x6),
	BOOT_FAST_STANDBY_FLAG         = (1<<0x7),
	MISC1_STANDBY_FLAG             = (1<<0x8),
	GPIO_HOLD_STANDBY_FLAG         = (1<<0x9),
	DYNAMIC_STANDBY_FLAG           = (1<<0xa),
	USB_OHCI_STANDBY_FLAG          = (1<<0xb),
	USB_EHCI_STANDBY_FLAG          = (1<<0xc)
} power_scene_flags;

struct scene_lock {
#ifdef CONFIG_SCENELOCK
	struct list_head    link;
	int                 flags;
	int                 count;
	const char         *name;
#endif
};

typedef enum CPU_WAKEUP_SRC {
/* the wakeup source of main cpu: cpu0, only used in normal standby */
	CPU0_MSGBOX_SRC		= CPU0_WAKEUP_MSGBOX,  /* external interrupt, pmu event for ex. */
	CPU0_KEY_SRC		= CPU0_WAKEUP_KEY,     /* key event, volume home menu enter */

/* the wakeup source of assistant cpu: cpus */
	CPUS_LOWBATT_SRC	= CPUS_WAKEUP_LOWBATT,  /* low battery event */
	CPUS_USB_SRC		= CPUS_WAKEUP_USB ,     /* usb insert event */
	CPUS_AC_SRC		= CPUS_WAKEUP_AC,       /* charger insert event */
	CPUS_ASCEND_SRC		= CPUS_WAKEUP_ASCEND,   /* power key ascend event */
	CPUS_DESCEND_SRC	= CPUS_WAKEUP_DESCEND,  /* power key descend event */
	CPUS_SHORT_KEY_SRC	= CPUS_WAKEUP_SHORT_KEY,/* power key short press event */
	CPUS_LONG_KEY_SRC	= CPUS_WAKEUP_LONG_KEY, /* power key long press event */
	CPUS_IR_SRC		= CPUS_WAKEUP_IR,       /* IR key event */
	CPUS_ALM0_SRC		= CPUS_WAKEUP_ALM0,     /* alarm0 event */
	CPUS_ALM1_SRC		= CPUS_WAKEUP_ALM1,     /* alarm1 event */
	CPUS_TIMEOUT_SRC	= CPUS_WAKEUP_TIMEOUT,  /* debug test event */
	CPUS_GPIO_SRC		= CPUS_WAKEUP_GPIO,     /* GPIO interrupt event, only used in extended standby */
	CPUS_USBMOUSE_SRC	= CPUS_WAKEUP_USBMOUSE, /* USBMOUSE key event, only used in extended standby */
	CPUS_LRADC_SRC		= CPUS_WAKEUP_LRADC ,   /* key event, volume home menu enter, only used in extended standby */
	CPUS_CODEC_SRC		= CPUS_WAKEUP_CODEC,    /* codec irq, only used in extended standby*/
	CPUS_BAT_TEMP_SRC	= CPUS_WAKEUP_BAT_TEMP, /* baterry temperature low and high */
	CPUS_FULLBATT_SRC	= CPUS_WAKEUP_FULLBATT, /* baterry full */
	CPUS_HMIC_SRC		= CPUS_WAKEUP_HMIC,     /* earphone insert/pull event, only used in extended standby */
	CPUS_KEY_SL_SRC		= CPUS_WAKEUP_KEY       /* power key short and long press event */
}cpu_wakeup_src_e;

typedef struct extended_standby_manager{
	extended_standby_t *pextended_standby;
	unsigned long event;
	unsigned long wakeup_gpio_map;
	unsigned long wakeup_gpio_group;
}extended_standby_manager_t;

typedef struct scene_extended_standby {
	/*
	 * scene type of extended standby
	 */
	aw_power_scene_e scene_type; //for scene_lock implement convinient;

	char * name;	//for user convinient;

	soc_pwr_dep_t soc_pwr_dep;

	struct list_head list; /* list of all extended standby */
} scene_extended_standby_t;
int extended_standby_set_pmu_id(unsigned int num, unsigned int pmu_id);

extern scene_extended_standby_t extended_standby[];
extern int extended_standby_cnt;

const extended_standby_manager_t *get_extended_standby_manager(void);

bool set_extended_standby_manager(scene_extended_standby_t *local_standby);

int extended_standby_enable_wakeup_src(cpu_wakeup_src_e src, int para);

int extended_standby_disable_wakeup_src(cpu_wakeup_src_e src, int para);

int extended_standby_check_wakeup_state(cpu_wakeup_src_e src, int para);

int extended_standby_show_state(void);

#ifdef CONFIG_SCENELOCK

void scene_lock_init(struct scene_lock *lock, aw_power_scene_e type, const char *name);
void scene_lock_destroy(struct scene_lock *lock);
void scene_lock(struct scene_lock *lock);
void scene_unlock(struct scene_lock *lock);

/* check_scene_locked returns a zero value if the scene_lock is currently
 * locked.
 */
int check_scene_locked(aw_power_scene_e type);

/* scene_lock_active returns 0 if no scene locks of the specified type are active,
 * and non-zero if one or more scene locks are held.
 */
int scene_lock_active(struct scene_lock *lock);

int enable_wakeup_src(cpu_wakeup_src_e src, int para);

int disable_wakeup_src(cpu_wakeup_src_e src, int para);

int check_wakeup_state(cpu_wakeup_src_e src, int para);

int standby_show_state(void);
int scene_set_volt(aw_power_scene_e scene_type, unsigned int bitmap,
        unsigned int volt_value);
#else

static inline void scene_lock_init(struct scene_lock *lock, int type,
					const char *name) {}
static inline void scene_lock_destroy(struct scene_lock *lock) {}
static inline void scene_lock(struct scene_lock *lock) {}
static inline void scene_unlock(struct scene_lock *lock) {}

static inline int check_scene_locked(aw_power_scene_e type) { return -1; }
static inline int scene_lock_active(struct scene_lock *lock) { return 0; }

static inline int enable_wakeup_src(cpu_wakeup_src_e src, int para) { return 0; }
static inline int disable_wakeup_src(cpu_wakeup_src_e src, int para) { return 0; }
static inline int check_wakeup_state(cpu_wakeup_src_e src, int para) { return 0; }
static inline int standby_show_state(void) { return 0; }
static inline int scene_set_volt(aw_power_scene_e scene_type, unsigned int bitmap,
        unsigned int volt_value) { return 0; };
#endif

#endif

