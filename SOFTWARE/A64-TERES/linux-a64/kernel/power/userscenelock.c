/* kernel/power/userscenelock.c
 *
 * Copyright (C) 2013-2014 allwinner.
 *
 *  By      : liming
 *  Version : v1.0
 *  Date    : 2013-4-17 09:08
 */

#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/power/scenelock.h>
#include <linux/slab.h>
#include "power.h"
#include "linux/power/aw_pm.h"

enum {
	DEBUG_FAILURE	= BIT(0),
	DEBUG_ERROR	= BIT(1),
	DEBUG_NEW	= BIT(2),
	DEBUG_ACCESS	= BIT(3),
	DEBUG_LOOKUP	= BIT(4),
};
static int debug_mask = 0xff; //DEBUG_FAILURE;
module_param_named(debug_mask, debug_mask, int, 0644);

static DEFINE_MUTEX(tree_lock);

struct user_scene_lock {
	struct rb_node		node;
	struct scene_lock	scene_lock;
	char			name[0];
};
static struct rb_root user_scene_locks;

static struct user_scene_lock *lookup_scene_lock_name(
	const char *buf, int allocate)
{
	struct rb_node **p = &user_scene_locks.rb_node;
	struct rb_node *parent = NULL;
	struct user_scene_lock *l;
	int diff;
	int name_len;
	int i;
	const char *arg;
	aw_power_scene_e type = SCENE_MAX;

	/* Find length of lock name */
	arg = buf;
	while (*arg && !isspace(*arg))
		arg++;
	name_len = arg - buf;
	if (!name_len)
		goto bad_arg;
	while (isspace(*arg))
		arg++;

	/* Lookup scene lock in rbtree */
	while (*p) {
		parent = *p;
		l = rb_entry(parent, struct user_scene_lock, node);
		diff = strncmp(buf, l->name, name_len);
		if (!diff && l->name[name_len])
			diff = -1;
		if (debug_mask & DEBUG_ERROR)
			pr_info("lookup_scene_lock_name: compare %.*s %s %d\n",
				name_len, buf, l->name, diff);

		if (diff < 0)
			p = &(*p)->rb_left;
		else if (diff > 0)
			p = &(*p)->rb_right;
		else
			return l;
	}

	/* Allocate and add new scenelock to rbtree */
	if (!allocate) {
		if (debug_mask & DEBUG_ERROR)
			pr_info("lookup_scene_lock_name: %.*s not found\n",
				name_len, buf);
		return ERR_PTR(-EINVAL);
	}
	l = kzalloc(sizeof(*l) + name_len + 1, GFP_KERNEL);
	if (l == NULL) {
		if (debug_mask & DEBUG_FAILURE)
			pr_err("lookup_scene_lock_name: failed to allocate "
				"memory for %.*s\n", name_len, buf);
		return ERR_PTR(-ENOMEM);
	}
	memcpy(l->name, buf, name_len);

	for (i = 0; i < extended_standby_cnt; i++) {
	    //first, judge the extended standby initialized.
	    if (extended_standby[i].scene_type){ 
		if (name_len == strlen(extended_standby[i].name) && !strncmp(l->name, extended_standby[i].name, strlen(extended_standby[i].name))) {
		    type = extended_standby[i].scene_type;
		    break;
		}
	    }
	}

	if (SCENE_MAX == type) {
		printk(KERN_ERR "scene name: %s is not supported.\n", l->name);
		return ERR_PTR(-EINVAL);
	}

	if (debug_mask & DEBUG_NEW)
		pr_info("lookup_scene_lock_name: new scene lock %s\n", l->name);
	scene_lock_init(&l->scene_lock, type, l->name);
	rb_link_node(&l->node, parent, p);
	rb_insert_color(&l->node, &user_scene_locks);
	return l;

bad_arg:
	if (debug_mask & DEBUG_ERROR)
		pr_info("lookup_wake_lock_name: wake lock, %.*s, bad arg, %s\n",
			name_len, buf, arg);
	return ERR_PTR(-EINVAL);
}

static char *show_scene_state(char *s, char *end)
{
    int i = 0;
    for (i = 0; i < extended_standby_cnt; i++) {
	//first, judge the extended standby initialized.
	if (extended_standby[i].scene_type){ 
	    if (!check_scene_locked(extended_standby[i].scene_type)) {
		s += scnprintf(s, end - s, "[%s] ", extended_standby[i].name);
	    }else{
		s += scnprintf(s, end - s, "%s ", extended_standby[i].name);
	    }
	}
    }
   
    s += scnprintf(s, end - s, "\n");

    return s;

}
ssize_t scene_lock_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *s = buf;
    char *end = buf + PAGE_SIZE;
    s = show_scene_state(s, end);

    return (s - buf);
}

ssize_t scene_lock_store(
	struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
	struct user_scene_lock *l;

	mutex_lock(&tree_lock);
	l = lookup_scene_lock_name(buf, 1);
	if (IS_ERR(l)) {
		n = PTR_ERR(l);
		goto bad_name;
	}

	scene_lock(&l->scene_lock);

	if (strlen(l->name) == strlen("usb_standby") && !strncmp(l->name, "usb_standby", strlen("usb_standby"))) {
		enable_wakeup_src(CPUS_USBMOUSE_SRC, 0);
	} else if (strlen(l->name) == strlen("talking_standby") && !strncmp(l->name, "talking_standby", strlen("talking_standby"))) {
		;
	} else if (strlen(l->name) == strlen("mp3_standby") && !strncmp(l->name, "mp3_standby", strlen("mp3_standby"))) {
		;
	}
bad_name:
	mutex_unlock(&tree_lock);
	return n;
}


ssize_t scene_unlock_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return  1;
}

ssize_t scene_unlock_store(
	struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
	struct user_scene_lock *l;

	mutex_lock(&tree_lock);
	l = lookup_scene_lock_name(buf, 0);
	if (IS_ERR(l)) {
		n = PTR_ERR(l);
		goto not_found;
	}

	if (debug_mask & DEBUG_ACCESS)
		pr_info("scene_unlock_store: %s\n", l->name);

	scene_unlock(&l->scene_lock);

	if (0 == l->scene_lock.count) {
	    if (strlen(l->name) == strlen("usb_standby") && !strncmp(l->name, "usb_standby", strlen("usb_standby"))) {
		if (check_scene_locked(SCENE_USB_STANDBY))
		    disable_wakeup_src(CPUS_USBMOUSE_SRC, 0);
	    } else if (strlen(l->name) == strlen("talking_standby") && !strncmp(l->name, "talking_standby", strlen("talking_standby"))) {
		;
	    } else if (strlen(l->name) == strlen("mp3_standby") && !strncmp(l->name, "mp3_standby", strlen("mp3_standby"))) {
		;
	    }
	}
not_found:
	mutex_unlock(&tree_lock);
	return n;
}

ssize_t scene_state_store(struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
	return n;
}

ssize_t scene_state_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char *s = buf;
	char *end = buf + PAGE_SIZE;

	s = show_scene_state(s, end);

	return (s - buf);
}

ssize_t wakeup_src_store(struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
	cpu_wakeup_src_e src;
	__u32 para = 0;
	__u32 enable = 0;

	sscanf(buf, "%x %x %x\n", (__u32 *)&src, (__u32 *)&para, (__u32 *)&enable);
	if(enable){
	    enable_wakeup_src(src, para);
	}else{
	    disable_wakeup_src(src, para);
	}

	return n;
}

ssize_t wakeup_src_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char *s = buf;
	char *end = buf + PAGE_SIZE;
	const extended_standby_manager_t *manager = NULL;

	manager = get_extended_standby_manager();

	if (NULL != manager) {

		s += scnprintf(s, end - s, "%s\n", "dynamic wakeup src config:");
		s += scnprintf(s, end - s, "wakeup_src 0x%lx\n", manager->event);
		s += parse_wakeup_event(s, end - s, manager->event, CPUS_ID);	
		s += scnprintf(s, end - s, "wakeup_gpio_map 0x%lx\n", manager->wakeup_gpio_map);
		s += parse_wakeup_gpio_map(s, end -s, manager->wakeup_gpio_map);	
		s += scnprintf(s, end - s, "wakeup_gpio_group 0x%lx\n", manager->wakeup_gpio_group);
		s += parse_wakeup_gpio_group_map(s, end - s, manager->wakeup_gpio_group);
		if (NULL != manager->pextended_standby)
			s += scnprintf(s, end - s, "extended_standby id = 0x%x\n", manager->pextended_standby->id);
	}

	s += scnprintf(s, end - s, "%s\n", "==========================wakeup src setting usage help info========:");
	s += scnprintf(s, end - s, "%s\n", "echo wakeup_src_e para (1:enable)/(0:disable) > /sys/power/wakeup_src");
	s += scnprintf(s, end - s, "%s\n", "demo: echo 0x2000 0x200 1 > /sys/power/wakeup_src");
	s += scnprintf(s, end - s, "%s\n", "wakeup_src_e para info: ");
	s += parse_wakeup_event(s, end - s, 0xffffffff, CPUS_ID);
	s += scnprintf(s, end - s, "%s\n", "gpio para info: ");
	s += show_gpio_config(s, end - s);

	return (s - buf);
}

#if (defined CONFIG_AW_AXP)
ssize_t sys_pwr_dm_mask_show(struct kobject *kobj, struct kobj_attribute *attr,
	char *buf)
{
	char *s = buf;
	char *end = (char *)((ptrdiff_t)buf + (ptrdiff_t)PAGE_SIZE);
	__u32 dm = 0;

	dm = get_sys_pwr_dm_mask();
	s += scnprintf(s, end - s, "0x%x \n", dm);
	s += parse_pwr_dm_map(s, end - s, dm);
	s += scnprintf(s, end - s, "%s\n", "==========================sys pwr_dm mask setting usage help info========:");
	s += scnprintf(s, end - s, "%s\n", "echo pwr_dm (1:enable)/(0:disable) > /sys/power/sys_pwr_dm_mask");
	s += scnprintf(s, end - s, "%s\n", "demo: for add cpub to sys_pwr_dm, \n echo 0x2 1 > /sys/power/sys_pwr_dm_mask");
	s += scnprintf(s, end - s, "%s\n", "sys pwr dm info: ");
	s += parse_pwr_dm_map(s, end - s, 0xffffffff);

	return (s - buf);
}

ssize_t sys_pwr_dm_mask_store(struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
	int i = 0;
	__u32 dm = 0;
	__u32 enable = 0;

	sscanf(buf, "%x %x \n", (__u32 *)&dm, (__u32 *)&enable);
	
	for(i = 0; i < sizeof(dm)*8; i++){
	    if(dm & 0x1<<i){
		set_sys_pwr_dm_mask(i, enable);
	    }
	}

	return n;
}
#endif

static int __init userscene_lock_init(void)
{

#if defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN50IW1P1)
	printk(KERN_INFO "lock super standby defaultly!\n");
	scene_lock_store(NULL, NULL, "super_standby", 0);
#endif
	return 0;
}

static void __exit userscene_lock_exit(void)
{
    return  ;
}

module_init(userscene_lock_init);
module_exit(userscene_lock_exit);

