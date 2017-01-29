#ifndef DEV_COMPOSER_C_C
#define DEV_COMPOSER_C_C

#include <linux/sw_sync.h>
#include <linux/sync.h>
#include <linux/file.h>
#include "dev_disp.h"
#include <video/sunxi_display2.h>
#include <linux/sunxi_tr.h>

#define DBG_TIME_TYPE 3
#define DBG_TIME_SIZE 100
#define WB_CHECK_SIZE  5
#define DISP_NUMS_SCREEN 2
enum {
    HWC_SYNC_NEED = -2,
    HWC_SYNC_INIT = -1,
    HWC_DISP0 = 0,
    HWC_DISP1 = 1,
    HWC_OTHER0 = 2,
    HWC_OTHER1 = 3,
    CNOUTDISPSYNC = 4,
};

enum COMMIT_ACTIVE{
    HWC_VSYNC_DEACTIVE = 0,
    HWC_VSYNC_ACTIVE_1 = 1,
    HWC_VSYNC_ACTIVE_2 = 2,
    HWC_VSYNC_ACTIVE_FINISH = 3,
};

enum HWC_IOCTL{
    HWC_IOCTL_FENCEFD = 0,
    HWC_IOCTL_COMMIT = 1,
    HWC_IOCTL_CKWB = 2,
    HWC_IOCTL_SETPRIDIP,
};
struct hwc_ioctl_arg {
    enum HWC_IOCTL cmd;
    void *arg;
};

struct hwc_compat_ioctl_arg {
    enum HWC_IOCTL cmd;
    compat_uptr_t arg;
};

struct hwc_commit_layer{
    int aquirefencefd;
    struct disp_layer_config hwc_layer_info;
};

struct hwc_dispc_data {
    struct disp_layer_config *hwc_layer_info[DISP_NUMS_SCREEN];
    int releasefencefd[CNOUTDISPSYNC];
    struct disp_capture_info *data;
    bool force_flip[DISP_NUMS_SCREEN];
};

struct hwc_compat_dispc_data {
    compat_uptr_t hwc_layer_info[DISP_NUMS_SCREEN];
    int releasefencefd[CNOUTDISPSYNC];
    compat_uptr_t data;
    bool force_flip[DISP_NUMS_SCREEN];
};
struct display_sync {
    unsigned int timeline_count;
    struct sw_sync_timeline *timeline;
    enum COMMIT_ACTIVE  active;
};

struct write_back {
    unsigned int  sync;
    bool success;
};

struct composer_health_info {
	unsigned long time[DBG_TIME_TYPE][DBG_TIME_SIZE];
	unsigned int time_index[DBG_TIME_TYPE];
	unsigned int count[DBG_TIME_TYPE];
};

struct composer_private_data {
	struct work_struct post2_cb_work;
	unsigned int cur_write_cnt[CNOUTDISPSYNC];
	unsigned int cur_disp_cnt[CNOUTDISPSYNC];
    unsigned int last_diplay_cnt[CNOUTDISPSYNC];
    int primary_disp;
	bool b_no_output;
    bool wb_status;
    struct mutex sync_lock;
    int disp_hotplug_cnt[CNOUTDISPSYNC];
    struct display_sync display_sync[CNOUTDISPSYNC];//0 is de0 ,1 is de1 , 2 is writeback,3 is FB
    struct disp_layer_config *tmp_hw_lyr;
    disp_drv_info *psg_disp_drv;
    wait_queue_head_t commit_wq;
    struct write_back check_wb[WB_CHECK_SIZE];
    struct composer_health_info health_info;
};

static struct composer_private_data composer_priv;

static s32 composer_get_frame_fps(u32 type)
{
	__u32 pre_time_index, cur_time_index;
	__u32 pre_time, cur_time;
	__u32 fps = 0xff;

	pre_time_index = composer_priv.health_info.time_index[type];
	cur_time_index = (pre_time_index == 0) ? (DBG_TIME_SIZE -1) : (pre_time_index-1);

	pre_time = composer_priv.health_info.time[type][pre_time_index];
	cur_time = composer_priv.health_info.time[type][cur_time_index];

	if(pre_time != cur_time) {
		fps = 1000 * 100 / (cur_time - pre_time);
	}

	return fps;
}

//type: 0:acquire, 1:release; 2:display
static void composer_frame_checkin(u32 type)
{
	u32 index = composer_priv.health_info.time_index[type];
	composer_priv.health_info.time[type][index] = jiffies;
	index++;
	index = (index >= DBG_TIME_SIZE) ? 0: index;
	composer_priv.health_info.time_index[type] = index;
	composer_priv.health_info.count[type]++;
}

unsigned int composer_dump(char *buf)
{
	u32 fps0, fps1, fps2;
	u32 cnt0, cnt1, cnt2;

	fps0 = composer_get_frame_fps(0);
	fps1 = composer_get_frame_fps(1);
	fps2 = composer_get_frame_fps(2);
	cnt0 = composer_priv.health_info.count[0];
	cnt1 = composer_priv.health_info.count[1];
	cnt2 = composer_priv.health_info.count[2];

	return sprintf(buf, "acquire: %d, %d.%d fps\nrelease: %d, %d.%d fps\ndisplay: %d, %d.%d fps\n",
		cnt0, fps0/10, fps0%10, cnt1, fps1/10, fps1%10, cnt2, fps2/10, fps2%10);
}

static void imp_finish_cb(void)
{
    int i = 0;
    for(i = 0; i < CNOUTDISPSYNC; i++)
    {
        while(composer_priv.display_sync[i].active == HWC_VSYNC_ACTIVE_FINISH
                && composer_priv.display_sync[i].timeline != NULL
                && composer_priv.display_sync[i].timeline->value != composer_priv.cur_write_cnt[i])
        {
            if(!composer_priv.b_no_output
                && composer_priv.display_sync[i].timeline->value + 1 == composer_priv.cur_disp_cnt[i])
            {
                break;
            }
            sw_sync_timeline_inc(composer_priv.display_sync[i].timeline, 1);
            composer_frame_checkin(1);
        }
        if(composer_priv.cur_write_cnt[composer_priv.primary_disp]
            == composer_priv.cur_disp_cnt[composer_priv.primary_disp])
        {
            wake_up(&composer_priv.commit_wq);
        }
    }
}

extern s32  bsp_disp_shadow_protect(u32 disp, bool protect);

int dispc_gralloc_queue(struct disp_layer_config *commit_layer,
        unsigned int disp, unsigned int hwc_sync, struct disp_capture_info *data)
{
    disp_drv_info *psg_disp_drv = composer_priv.psg_disp_drv;
    struct disp_manager *disp_mgr = NULL;
    struct disp_capture *write_back = NULL;

    disp_mgr = psg_disp_drv->mgr[disp];
    if( disp_mgr != NULL )
    {
        bsp_disp_shadow_protect(disp,true);
        if(data != NULL)
        {
            write_back = disp_mgr->cptr;
            if(composer_priv.wb_status != 1)
            {
                write_back->start(write_back);
                composer_priv.wb_status = 1;
            }
            write_back->commmit(write_back, data);
        }else{
            if(composer_priv.wb_status == 1)
            {
                write_back->stop(write_back);
                composer_priv.wb_status = 0;
            }
        }
        disp_mgr->set_layer_config(disp_mgr, commit_layer, disp ? 8 : 16);
        bsp_disp_shadow_protect(disp, false);
    }
    composer_frame_checkin(2);
    composer_priv.cur_write_cnt[disp] = hwc_sync;
    if(composer_priv.display_sync[disp].active == HWC_VSYNC_ACTIVE_1)
    {
        composer_priv.display_sync[disp].active = HWC_VSYNC_ACTIVE_2;
    }
    return 0;
}

inline bool hwc_pridisp_sync(void)
{
    return composer_priv.cur_write_cnt[composer_priv.primary_disp]
            == composer_priv.cur_disp_cnt[composer_priv.primary_disp];
}

unsigned int hwc_get_sync(int sync_disp, int fd)
{
    struct sync_fence *fence = NULL;
    struct sync_pt *pt = NULL;
    fence = sync_fence_fdget(fd);
    if(fence == NULL)
    {
        printk(KERN_ERR "hwc get relesefence(%d) err.\n",fd);
        goto err;
    }
    sync_fence_put(fence);
    if(!list_empty(&fence->pt_list_head))
    {
        pt = list_entry(fence->pt_list_head.next, struct sync_pt, pt_list);
    }else{
        printk(KERN_ERR "hwc get sw_pt err\n");
        goto err;
    }

    return  pt != NULL ? ((struct sw_sync_pt*)pt)->value : 0;
err:
    return composer_priv.cur_write_cnt[sync_disp] + 1;
}

static bool hwc_fence_get(void *user_fence)
{
    struct sync_fence *fence = NULL;
	struct sync_pt *pt = NULL;
	int fd[CNOUTDISPSYNC] = {-1, -1, -1, -1}, ret = -1;
    char buf[20];
    int i = 0;

    if(copy_from_user(&fd, (void __user *)user_fence, sizeof(int) * CNOUTDISPSYNC))
    {
        printk(KERN_ERR "copy_from_user hwc_fence_get err.\n");
        goto fecne_ret;
    }
    for(i = 0; i < CNOUTDISPSYNC; i++)
    {
        if(fd[i] == HWC_SYNC_NEED)
        {
            if(composer_priv.display_sync[i].active == HWC_VSYNC_DEACTIVE)
            {
                ret = sprintf(buf, "sunxi_%d_%d", i, composer_priv.disp_hotplug_cnt[i]);
                composer_priv.display_sync[i].timeline = sw_sync_timeline_create(buf);
                if(composer_priv.display_sync[i].timeline == NULL)
                {
                    printk("creat timeline err.\n");
                    continue;
                }
                composer_priv.cur_disp_cnt[i] = 0;
                composer_priv.cur_disp_cnt[i] = 0;
                composer_priv.cur_write_cnt[i] = 0;
                composer_priv.display_sync[i].timeline_count = 0;
                composer_priv.display_sync[i].active = HWC_VSYNC_ACTIVE_1;
                composer_priv.disp_hotplug_cnt[i]++;
            }
            composer_priv.display_sync[i].timeline_count++;
            ret = sprintf(buf, "sunxi_%d_%d", i, composer_priv.display_sync[i].timeline_count);
            fd[i] = get_unused_fd();
            if(fd[i] < 0)
            {
                printk(KERN_ERR "get unused fd faild\n");
                continue;
            }
            pt = sw_sync_pt_create(composer_priv.display_sync[i].timeline,
                                composer_priv.display_sync[i].timeline_count);
            if(pt == NULL)
            {
                put_unused_fd(fd[i]);
                printk(KERN_ERR "creat display pt faild\n");
                continue;
            }
            fence = sync_fence_create(buf, pt);
            if(fence == NULL)
            {
                put_unused_fd(fd[i]);
                printk(KERN_ERR "creat dispay fence faild\n");
                continue;
            }
            sync_fence_install(fence, fd[i]);
        }else{
            if(composer_priv.display_sync[i].active != HWC_VSYNC_DEACTIVE)
            {
                composer_priv.display_sync[i].active = HWC_VSYNC_DEACTIVE;
                mutex_lock(&composer_priv.sync_lock);
                if(composer_priv.display_sync[i].timeline != NULL)
                {
                    while(composer_priv.display_sync[i].timeline->value
                            != composer_priv.display_sync[i].timeline_count)
                    {
                        sw_sync_timeline_inc(composer_priv.display_sync[i].timeline, 1);
                    }
                    sync_timeline_destroy(&composer_priv.display_sync[i].timeline->obj);
                    composer_priv.display_sync[i].timeline = NULL;
                }
                mutex_unlock(&composer_priv.sync_lock);
                composer_priv.display_sync[i].timeline_count = 0;
            }
        }
    }
fecne_ret:
    if(copy_to_user((void __user *)user_fence, fd, sizeof(int) * 4))
    {
	    printk(KERN_ERR "copy_to_user fail\n");
	}
	return 0;
}

static int hwc_commit(void *user_display, bool compat)
{
    struct hwc_dispc_data disp_data;
    struct hwc_compat_dispc_data disp_compat_data;
    struct disp_capture_info wb_data;
    bool need_wb = 0;
    int ret = 0, i = 0;
    unsigned int sync[CNOUTDISPSYNC];
    unsigned long  hwc_layer_info[DISP_NUMS_SCREEN];
    int releasefencefd[CNOUTDISPSYNC];
    unsigned long wb_ptr_data;
    bool force_flip[DISP_NUMS_SCREEN];

    composer_frame_checkin(0);
    if(compat)
    {
        ret = copy_from_user(&disp_compat_data,
            (void __user *)user_display, sizeof(struct hwc_compat_dispc_data));
        if(ret)
        {
            printk(KERN_ERR "hwc copy_from_user hwc_compat_dispc_data err.\n");
            goto commit_ok;
        }
        for(i = 0; i < DISP_NUMS_SCREEN; i++)
        {
            hwc_layer_info[i] = disp_compat_data.hwc_layer_info[i];
            force_flip[i] = disp_compat_data.force_flip[i];
        }
        for(i = 0; i < CNOUTDISPSYNC; i++)
        {
            releasefencefd[i] = disp_compat_data.releasefencefd[i];
        }
        wb_ptr_data = ((unsigned long)disp_compat_data.data);
    }else{
        ret = copy_from_user(&disp_data, (void __user *)user_display, sizeof(struct hwc_dispc_data));
        if(ret)
        {
            printk(KERN_ERR"hwc copy_from_user hwc_dispc_data err.\n");
            goto commit_ok;
        }
        for(i = 0; i < DISP_NUMS_SCREEN; i++)
        {
            hwc_layer_info[i] = (unsigned long)disp_data.hwc_layer_info[i];
            force_flip[i] = disp_data.force_flip[i];
        }
        for(i = 0; i < CNOUTDISPSYNC; i++)
        {
            releasefencefd[i] = disp_data.releasefencefd[i];
        }
        wb_ptr_data = (unsigned long)disp_data.data;
    }

    for(i = 0; i < CNOUTDISPSYNC; i++)
    {
        if(releasefencefd[i] >= 0)
        {
            sync[i] = hwc_get_sync(i, releasefencefd[i]);
            if(force_flip[i])
            {
                printk(KERN_INFO "hwc force flip disp[%d]:%d \n", i, sync[i]);
                if(composer_priv.display_sync[i].active != HWC_VSYNC_DEACTIVE
                    && composer_priv.display_sync[i].timeline != NULL)
                {
                    composer_priv.display_sync[i].active = HWC_VSYNC_ACTIVE_FINISH;
                    composer_priv.cur_write_cnt[i] = sync[i];
                }
                schedule_work(&composer_priv.post2_cb_work);
            }
        }
    }
    if(releasefencefd[composer_priv.primary_disp] >= 0 && !hwc_pridisp_sync())
    {
	    wait_event_interruptible_timeout(composer_priv.commit_wq,
			    			   hwc_pridisp_sync(),
					    	   msecs_to_jiffies(16));
        /* for vsync shadow protected */
        //usleep_range(100, 200);
    }
    if(NULL != (void *)wb_ptr_data && !force_flip[0])
    {
         if(copy_from_user(&wb_data, (void __user *)wb_ptr_data, sizeof(struct disp_capture_info)))
        {
            printk(KERN_ERR"hwc copy_from_user write back data err.\n");
        }
        need_wb = 1;
    }
    for(i = 0; i < DISP_NUMS_SCREEN; i++)
    {
		if(releasefencefd[i] >= 0 && !force_flip[i])
		{
            if(copy_from_user(composer_priv.tmp_hw_lyr,
                (void __user *)hwc_layer_info[i], sizeof(struct disp_layer_config) * (i?8:16)))
            {
                printk(KERN_ERR"hwc copy_from_user disp_layer_config err.\n");
                ret = -1;
                continue;
            }
            dispc_gralloc_queue(composer_priv.tmp_hw_lyr, i, sync[i], (need_wb && (0==i))? &wb_data : NULL);
		}
    }
commit_ok:
    if(composer_priv.b_no_output)
    {
        schedule_work(&composer_priv.post2_cb_work);
    }
	return ret;
}

static int hwc_check_wb(int disp, int fd)
{
    unsigned int pt_value = 0;
    pt_value = hwc_get_sync(disp, fd);
    if(composer_priv.check_wb[pt_value % WB_CHECK_SIZE].sync == pt_value)
    {
        return composer_priv.check_wb[pt_value % WB_CHECK_SIZE].success;
    }
    return 0;
}

static int hwc_compat_ioctl(unsigned int cmd, unsigned long arg)
{
    int ret = -EFAULT;
	if(DISP_HWC_COMMIT == cmd)
    {
        unsigned long *ubuffer;
        struct hwc_compat_ioctl_arg hwc_ctl;
        unsigned long addr_cmd;

        ubuffer = (unsigned long *)arg;
        if(copy_from_user(&hwc_ctl,
                (void __user *)ubuffer[1], sizeof(struct hwc_compat_ioctl_arg)))
        {
            printk(KERN_ERR "copy_from_user fail\n");
            return  -EFAULT;
		}
        addr_cmd = (unsigned long)hwc_ctl.arg;
        switch(hwc_ctl.cmd)
        {
            case HWC_IOCTL_FENCEFD:
                ret = hwc_fence_get((void *)addr_cmd);
            break;
            case HWC_IOCTL_COMMIT:
                ret = hwc_commit((void *)addr_cmd, 1);
            break;
            case HWC_IOCTL_CKWB:
                get_user(ret, (int *)(addr_cmd));
                ret = hwc_check_wb(0, ret);
            break;
            case HWC_IOCTL_SETPRIDIP:
                get_user(ret, (int *)(addr_cmd));
                if(ret < DISP_NUMS_SCREEN)
                {
                    composer_priv.primary_disp = ret;
                }
            break;
            default:
                printk(KERN_ERR "hwc give a err iotcl.\n");
        }
	}
	return ret;
}

static int hwc_ioctl(unsigned int cmd, unsigned long arg)
{
    int ret = -EFAULT;
	if(DISP_HWC_COMMIT == cmd)
    {
        unsigned long *ubuffer;
        struct hwc_ioctl_arg hwc_ctl;
        ubuffer = (unsigned long *)arg;
        if(copy_from_user(&hwc_ctl,
                (void __user *)ubuffer[1], sizeof(struct hwc_ioctl_arg)))
        {
            printk(KERN_ERR "copy_from_user fail\n");
            return  -EFAULT;
		}
        switch(hwc_ctl.cmd)
        {
            case HWC_IOCTL_FENCEFD:
                ret = hwc_fence_get(hwc_ctl.arg);
            break;
            case HWC_IOCTL_COMMIT:
                ret = hwc_commit(hwc_ctl.arg, 0);
            break;
            case HWC_IOCTL_CKWB:
                get_user(ret, (int *)(hwc_ctl.arg));
                ret = hwc_check_wb(0, ret);
            break;
            case HWC_IOCTL_SETPRIDIP:
                get_user(ret, (int *)(hwc_ctl.arg));
                if(ret < DISP_NUMS_SCREEN)
                {
                    composer_priv.primary_disp = ret;
                }
            break;
            default:
                printk(KERN_ERR "hwc give a err iotcl.\n");
        }
	}
	return ret;
}

static void disp_composer_proc(u32 sel)
{
    disp_drv_info *psg_disp_drv = composer_priv.psg_disp_drv;
    struct disp_manager *disp_mgr = NULL;
    struct disp_capture *wb_back = NULL;
    struct write_back *wb_status = NULL;
    if(sel < 2)
    {
        if(sel == 0 && composer_priv.wb_status == 1)
        {
            disp_mgr = psg_disp_drv->mgr[sel];
            wb_back = disp_mgr->cptr;
            wb_status = &composer_priv.check_wb[composer_priv.cur_disp_cnt[sel] % WB_CHECK_SIZE];
            wb_status->sync = composer_priv.cur_disp_cnt[sel];
            wb_status->success = !wb_back->query(wb_back);
        }
        composer_priv.last_diplay_cnt[sel]= composer_priv.cur_disp_cnt[sel];
        composer_priv.cur_disp_cnt[sel] = composer_priv.cur_write_cnt[sel];
        if(composer_priv.display_sync[sel].active == HWC_VSYNC_ACTIVE_2)
        {
            composer_priv.display_sync[sel].active = HWC_VSYNC_ACTIVE_FINISH;
        }
    }
	schedule_work(&composer_priv.post2_cb_work);
	return;
}

static void post2_cb(struct work_struct *work)
{
	mutex_lock(&composer_priv.sync_lock);
    imp_finish_cb();
	mutex_unlock(&composer_priv.sync_lock);
}

static int hwc_suspend(void)
{
	composer_priv.b_no_output = 1;
	schedule_work(&composer_priv.post2_cb_work);
	printk("%s\n", __func__);
	return 0;
}

static int hwc_resume(void)
{
	composer_priv.b_no_output = 0;
	printk("%s\n", __func__);
	return 0;
}

s32 composer_init(disp_drv_info *psg_disp_drv)
{
	memset(&composer_priv, 0x0, sizeof(struct composer_private_data));
	INIT_WORK(&composer_priv.post2_cb_work, post2_cb);
    mutex_init(&composer_priv.sync_lock);
    init_waitqueue_head(&composer_priv.commit_wq);

	disp_register_ioctl_func(DISP_HWC_COMMIT, hwc_ioctl);
    disp_register_compat_ioctl_func(DISP_HWC_COMMIT, hwc_compat_ioctl);
    disp_register_sync_finish_proc(disp_composer_proc);
	disp_register_standby_func(hwc_suspend, hwc_resume);
    composer_priv.tmp_hw_lyr = kzalloc(sizeof(struct disp_layer_config) * 16, GFP_KERNEL);

    if(composer_priv.tmp_hw_lyr == NULL)
    {
        printk(KERN_ERR"hwc_composer init err when kzalloc,you need reboot\n");
    }
    composer_priv.psg_disp_drv = psg_disp_drv;
    return 0;
}
#endif
