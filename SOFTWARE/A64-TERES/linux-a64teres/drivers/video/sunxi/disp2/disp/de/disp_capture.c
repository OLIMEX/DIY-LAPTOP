#include "disp_capture.h"

struct disp_capture_info_list
{
	struct disp_capture_info info;
	struct list_head list;
};

struct disp_capture_private_data
{
	u32 reg_base;
	u32 enabled;
	u32 applied;
	s32 status;//0: finish; other: fail/err

	struct list_head req_list;
	s32 (*shadow_protect)(u32 sel, bool protect);

	struct clk *clk;
#if defined(__LINUX_PLAT__)
	struct mutex              mlock;
	spinlock_t                data_lock;
#else
	int                       mlock;
	int                       data_lock;
#endif
};

static struct disp_capture *captures = NULL;
static struct disp_capture_private_data *capture_private = NULL;

struct disp_capture* disp_get_capture(u32 disp)
{
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();
	if (disp >= num_screens) {
		DE_WRN("disp %d out of range\n", disp);
		return NULL;
	}

	if (!bsp_disp_feat_is_support_capture(disp)) {
		DE_INF("screen %d not support capture\n", disp);
		return NULL;
	}

	return &captures[disp];
}

static struct disp_capture_private_data *disp_capture_get_priv(struct disp_capture *cptr)
{
	if (NULL == cptr) {
		DE_WRN("NULL hdl!\n");
		return NULL;
	}

	if (!bsp_disp_feat_is_support_capture(cptr->disp)) {
		DE_WRN("screen %d not support capture\n", cptr->disp);
		return NULL;
	}

	return &capture_private[cptr->disp];
}

s32 disp_capture_shadow_protect(struct disp_capture *capture, bool protect)
{
	struct disp_capture_private_data *capturep = disp_capture_get_priv(capture);

	if ((NULL == capture) || (NULL == capturep)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	if (capturep->shadow_protect)
		return capturep->shadow_protect(capture->disp, protect);

	return -1;
}

static s32 disp_capture_clk_init(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);

	if ((NULL == cptr) || (NULL == cptrp)) {
		DE_WRN("NULL hdl!\n");
		return 0;
	}
	//todo: int the clock

	return 0;
}

static s32 disp_capture_clk_exit(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);

	if ((NULL == cptr) || (NULL == cptrp)) {
		DE_WRN("NULL hdl!\n");
		return 0;
	}

	//todo: colse the clock

	return 0;
}

static s32 disp_capture_clk_enable(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);

	if ((NULL == cptr) || (NULL == cptrp)) {
		DE_WRN("NULL hdl!\n");
		return 0;
	}

	clk_prepare_enable(cptrp->clk);

	return 0;
}

static s32 disp_capture_clk_disable(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);

	if ((NULL == cptr) || (NULL == cptrp)) {
		DE_WRN("NULL hdl!\n");
		return 0;
	}

	clk_disable(cptrp->clk);

	return 0;
}

s32 disp_capture_apply(struct disp_capture *cptr)
{
	return 0;
}

s32 disp_capture_force_apply(struct disp_capture *cptr)
{
	return 0;
}

s32 disp_capture_start(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);

	if (NULL == cptr || NULL == cptrp)
	{
		DE_WRN("NULL hdl!\n");
		return -1;
	}
	DE_INF("cap %d\n", cptr->disp);

	mutex_lock(&cptrp->mlock);
	if (1 == cptrp->enabled) {
		DE_WRN("capture %d already started!\n", cptr->disp);
		mutex_unlock(&cptrp->mlock);
		return -1;
	}
	disp_capture_clk_enable(cptr);
	disp_al_capture_init(cptr->disp);
	cptrp->enabled = 1;
	mutex_unlock(&cptrp->mlock);

	return 0;
}

s32 disp_capture_stop(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);

	if (NULL == cptr || NULL == cptrp)
	{
		DE_WRN("NULL hdl!\n");
		return -1;
	}
	DE_INF("cap %d\n", cptr->disp);

	mutex_lock(&cptrp->mlock);
	if (1 == cptrp->enabled) {
		disp_al_capture_exit(cptr->disp);
		disp_capture_clk_disable(cptr);
		cptrp->enabled = 0;
	}
	mutex_unlock(&cptrp->mlock);

	return 0;
}

s32 disp_capture_commit(struct disp_capture *cptr, struct disp_capture_info *info)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);
	int fd = -1;
	struct disp_manager *mgr;
	struct disp_capture_info_list *info_list;
	unsigned long flags;

	if (NULL == cptr || NULL == cptrp)
	{
		DE_WRN("NULL hdl!\n");
		return -1;
	}
	DE_INF("cap %d\n", cptr->disp);

	mgr = cptr->manager;
  if ((NULL == mgr) || (0 == mgr->is_enabled(mgr))){
		DE_WRN("manager disable!\n");
		return -1;
  }
	DE_INF("disp%d, format %d, stride<%d,%d,%d>, crop<%d,%d,%d,%d>, address<0x%llx,0x%llx,0x%llx>\n",
		cptr->disp, info->out_frame.format,
		info->out_frame.size[0].width, info->out_frame.size[1].width, info->out_frame.size[2].width,
		info->out_frame.crop.x, info->out_frame.crop.y, info->out_frame.crop.width, info->out_frame.crop.height,
		info->out_frame.addr[0], info->out_frame.addr[1], info->out_frame.addr[2]);

	mutex_lock(&cptrp->mlock);
	if (0 == cptrp->enabled) {
		DE_WRN("capture %d is disabled!\n", cptr->disp);
		mutex_unlock(&cptrp->mlock);
		return -1;
	}
	info_list = kmalloc(sizeof(struct disp_capture_info_list), GFP_KERNEL | __GFP_ZERO);
	if (NULL == info_list) {
		DE_WRN("malloc fail!\n");
		mutex_unlock(&cptrp->mlock);
		return -1;
	}
	memcpy(&info_list->info, info, sizeof(struct disp_capture_info));
	spin_lock_irqsave(&cptrp->data_lock, flags);
	list_add_tail(&info_list->list, &cptrp->req_list);
	spin_unlock_irqrestore(&cptrp->data_lock, flags);
	mutex_unlock(&cptrp->mlock);

	return fd;
}

s32 disp_capture_query(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);

	if (NULL == cptr || NULL == cptrp)
	{
		DE_WRN("NULL hdl!\n");
		return 0;
	}

	return cptrp->status;
}

s32 disp_capture_sync(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);
	struct disp_manager *mgr = NULL;
	struct disp_device *dispdev = NULL;
	s32 ret = 0;
	unsigned long flags;
	if ((NULL == cptr) || (NULL == cptrp)) {
		DE_WRN("NULL hdl!\n");
		return 0;
	}

	mgr = cptr->manager;
	if ((NULL == mgr) || (0 == mgr->is_enabled(mgr))){
		DE_WRN("mgr is disable!\n");
		return 0;
	}
	dispdev = mgr->device;
	if (NULL == dispdev){
		DE_WRN("disp device is NULL!\n");
		return 0;
	}

	if (1 == cptrp->enabled){
		struct disp_capture_info_list *info_list = NULL;
		bool find = false;

		ret = disp_al_capture_get_status(cptr->disp);
		cptrp->status = ret;
		if (0 == ret){

		}
		spin_lock_irqsave(&cptrp->data_lock, flags);
		list_for_each_entry(info_list, &cptrp->req_list, list) {
			list_del(&info_list->list);
			find = true;
			break;
		}
		spin_unlock_irqrestore(&cptrp->data_lock, flags);
		if (find) {
			struct disp_capture_config config;
			enum disp_csc_type cs = DISP_CSC_TYPE_RGB;
			u32 width = 0, height = 0;

			memset(&config, 0, sizeof(struct disp_capture_config));
			memcpy(&config.out_frame, &info_list->info.out_frame, sizeof(struct disp_s_frame));
			config.disp = cptr->disp;
			memcpy(&config.in_frame.crop, &info_list->info.window, sizeof(struct disp_rect));
			if (dispdev->get_input_csc) {
				cs = dispdev->get_input_csc(dispdev);
			}
			if (DISP_CSC_TYPE_RGB == cs)
				config.in_frame.format = DISP_FORMAT_ARGB_8888;
			else
				config.in_frame.format = DISP_FORMAT_YUV444_P;//FIXME, how to diff  TV/HDMI yuv
			if (dispdev->get_resolution) {
				dispdev->get_resolution(dispdev, &width, &height);
			}
			config.in_frame.size[0].width = width;
			config.in_frame.size[1].width = width;
			config.in_frame.size[2].width = width;
			config.in_frame.size[0].height = height;
			config.in_frame.size[1].height = height;
			config.in_frame.size[2].height = height;
			if ((0 == config.in_frame.crop.width) || (0 == config.in_frame.crop.height)) {
				config.in_frame.crop.width = width;
				config.in_frame.crop.height = height;
			}
			disp_al_capture_apply(cptr->disp, &config);
			disp_al_capture_sync(cptr->disp);
			kfree((void*)info_list);
		}
	}

	return 0;
}

s32 disp_capture_set_manager(struct disp_capture *cptr, struct disp_manager *mgr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);
	if ((NULL == cptr) || (NULL == mgr)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}
	mutex_lock(&cptrp->mlock);
	cptr->manager = mgr;
	if (mgr)
		mgr->cptr = cptr;
	mutex_unlock(&cptrp->mlock);
	return 0;
}

s32 disp_capture_unset_manager(struct disp_capture *cptr)
{
	struct disp_capture_private_data *cptrp = disp_capture_get_priv(cptr);
	if (NULL == cptr) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}
	mutex_lock(&cptrp->mlock);
	if (cptr->manager)
		cptr->manager->cptr = NULL;
	cptr->manager = NULL;
	mutex_unlock(&cptrp->mlock);
	return 0;
}

s32 disp_capture_suspend(struct disp_capture *cptr)
{
	struct disp_capture_private_data* cptrp = disp_capture_get_priv(cptr);

	if ((NULL == cptr) || (NULL == cptrp)) {
		DE_WRN("capture NULL hdl!\n");
		return -1;
	}

	return 0;
}

s32 disp_capture_resume(struct disp_capture *cptr)
{
	struct disp_capture_private_data* cptrp = disp_capture_get_priv(cptr);

	if ((NULL == cptr) || (NULL == cptrp)) {
		DE_WRN("capture NULL hdl!\n");
		return -1;
	}

	return 0;

}

s32 disp_capture_init(struct disp_capture *cptr)
{
	struct disp_capture_private_data* capturep = disp_capture_get_priv(cptr);

	if ((NULL == cptr) || (NULL == capturep)) {
		DE_WRN("capture NULL hdl!\n");
		return -1;
	}

	if (!bsp_disp_feat_is_support_capture(cptr->disp)){
		DE_WRN("capture %d is not support\n", cptr->disp);
		return -1;
	}

	disp_capture_clk_init(cptr);
	return 0;
}

s32 disp_capture_exit(struct disp_capture *cptr)
{
	if (!bsp_disp_feat_is_support_capture(cptr->disp)){
		DE_WRN("capture %d is not support\n", cptr->disp);
		return -1;
	}
	disp_capture_clk_exit(cptr);

	return 0;
}

s32 disp_init_capture(disp_bsp_init_para *para)
{
	u32 num_screens;
	u32 capture_id;
	struct disp_capture *capture;
	struct disp_capture_private_data *capturep;

	num_screens = bsp_disp_feat_get_num_screens();
	captures = (struct disp_capture *)kmalloc(sizeof(struct disp_capture) * num_screens, GFP_KERNEL | __GFP_ZERO);
	if (NULL == captures) {
		DE_WRN("malloc memory fail!\n");
		return DIS_FAIL;
	}
	capture_private = (struct disp_capture_private_data *)kmalloc(sizeof(struct disp_capture_private_data)\
		* num_screens, GFP_KERNEL | __GFP_ZERO);
	if (NULL == capture_private) {
		DE_WRN("malloc memory fail!\n");
		return DIS_FAIL;
	}

	for (capture_id=0; capture_id < num_screens; capture_id++) {
		if (!bsp_disp_feat_is_support_capture(capture_id))
			continue;

		capture = &captures[capture_id];
		capturep = &capture_private[capture_id];
#if defined(__LINUX_PLAT__)
		mutex_init(&capturep->mlock);
		spin_lock_init(&(capturep->data_lock));
#endif

		capturep->clk = para->mclk[DISP_MOD_DE];
		switch(capture_id) {
		case 0:
			capture->disp = 0;
			capture->name = "capture0";
			break;

		case 1:
			capture->disp = 1;
			capture->name = "capture1";
			break;

		case 2:
			capture->disp = 2;
			capture->name = "capture2";
			break;
		}

		capturep->shadow_protect = para->shadow_protect;
		capture->set_manager = disp_capture_set_manager;
		capture->unset_manager = disp_capture_unset_manager;
		capture->start = disp_capture_start;
		capture->stop = disp_capture_stop;
		capture->sync = disp_capture_sync;
		capture->init = disp_capture_init;
		capture->exit = disp_capture_exit;
		capture->commmit = disp_capture_commit;
		capture->query = disp_capture_query;
		INIT_LIST_HEAD(&capturep->req_list);

		disp_capture_init(capture);
	}
	return 0;
}

