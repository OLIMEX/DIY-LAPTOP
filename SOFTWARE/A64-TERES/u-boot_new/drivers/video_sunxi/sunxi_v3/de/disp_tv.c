#include "disp_tv.h"


#if defined(SUPPORT_TV)

#define TVE_COUNT 	1
#if defined(__LINUX_PLAT__)
static spinlock_t g_tv_data_lock;
#else
static int g_tv_data_lock;
#endif

static struct disp_device *g_ptv_devices = NULL;
static struct disp_device_private_data *g_ptv_private = NULL;
static bool g_tv_used = 0;

struct disp_device* disp_get_tv(u32 disp)
{
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();
	if(disp >= num_screens || !bsp_disp_feat_is_supported_output_types(disp, DISP_OUTPUT_TYPE_TV)) {
		DE_WRN("disp %d not support TV output\n", disp);
		return NULL;
	}

	return &g_ptv_devices[disp];
}

static struct disp_device_private_data *disp_tv_get_priv(struct disp_device *ptv)
{
	if(NULL == ptv) {
		DE_WRN("NULL hdl!\n");
		return NULL;
	}

	if(!bsp_disp_feat_is_supported_output_types(ptv->disp, DISP_OUTPUT_TYPE_TV)) { //modify  1
	    DE_WRN("screen %d do not support TV TYPE!\n", ptv->disp);
	    return NULL;
	}

	return &g_ptv_private[ptv->disp];
}

extern void sync_event_proc(u32 disp);

#if defined(__LINUX_PLAT__)
static s32 disp_tv_event_proc(int irq, void *parg)
#else
static s32 disp_tv_event_proc(void *parg)
#endif
{
	u32 disp = (u32)parg;
	struct disp_device *ptv = disp_get_tv(disp);
	struct disp_manager *mgr = NULL;

	mgr = ptv->manager;
	if(disp_al_device_query_irq(disp)) {		//modify
		int cur_line = disp_al_device_get_cur_line(disp);
		int start_delay = disp_al_device_get_start_delay(disp);
		if((NULL == ptv) || (NULL == mgr))
			return DISP_IRQ_RETURN;

		if(cur_line <= (start_delay-4)) {
			sync_event_proc(mgr->disp);
		} else {
			//skip a frame
		}
	}

	return DISP_IRQ_RETURN;
}

#if defined(CONFIG_ARCH_SUN8IW6)
static s32 tv_clk_init(struct disp_device*  ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if(!ptv || !ptvp) {
	    DE_WRN("tv init null hdl!\n");
	    return DIS_FAIL;
	}
	return disp_sys_clk_set_parent(ptvp->clk, ptvp->clk_parent);
}

static s32 tv_clk_config(struct disp_device*  ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if(!ptv || !ptvp) {
		DE_WRN("tv init null hdl!\n");
		return DIS_FAIL;
	}
	return disp_sys_clk_set_rate(ptvp->clk, ptv->timings.pixel_clk);
}

#endif

static s32 tv_clk_enable(struct disp_device*  ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if(!ptv || !ptvp) {
	    DE_WRN("tv init null hdl!\n");
	    return DIS_FAIL;
	}
	return disp_sys_clk_enable(ptvp->clk);

}

static s32 tv_clk_disable(struct disp_device*  ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if(!ptv || !ptvp) {
	    DE_WRN("tv init null hdl!\n");
	    return DIS_FAIL;
	}
	return disp_sys_clk_disable(ptvp->clk);
}

s32 disp_tv_enable( struct disp_device* ptv)
{
	struct disp_manager *mgr = NULL;
	unsigned long flags;
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	printf("disp_tv_enable\n");
	if (!ptv || !ptvp) {
		DE_WRN("tv init null hdl!\n");
		return DIS_FAIL;
	}

	if ((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}
	mgr = ptv->manager;
	if (!mgr) {
		DE_WRN("tv%d's mgr is NULL\n", ptv->disp);
		return DIS_FAIL;
	}

	if(ptvp->enabled == 1) {
		DE_WRN("tv%d is already open\n", ptv->disp);
		return DIS_FAIL;
	}
	
	if (ptvp->tv_func.tv_get_video_timing_info == NULL) {
		DE_WRN("tv_get_video_timing_info func is null\n");
		return DIS_FAIL;
	}

	ptvp->tv_func.tv_get_video_timing_info(ptv->disp, &(ptvp->video_info));

	if (ptvp->video_info == NULL) {
		DE_WRN("video info is null\n");
		return DIS_FAIL;
	}
	memcpy(&ptv->timings, ptvp->video_info, sizeof(disp_video_timings));
	if (mgr->enable)
		mgr->enable(mgr);
#if defined(CONFIG_ARCH_SUN8IW6)
	tv_clk_config(ptv);  			//no need tcon clk for 1680
	tv_clk_enable(ptv);
#endif
	if(NULL == ptvp->tv_func.tv_enable) {
		DE_WRN("tv_enable is NULL\n");
		return -1;
	}
	DE_INF("tv enable before %d\n", ptv->disp);
	ptvp->tv_func.tv_enable(ptv->disp);
	tv_clk_enable(ptv);
	disp_al_tv_cfg(ptv->disp, ptvp->video_info);
	disp_al_tv_enable(ptv->disp);
	disp_sys_register_irq(ptvp->irq_no,0,disp_tv_event_proc,(void*)ptv->disp,0,0);
	disp_sys_enable_irq(ptvp->irq_no);
	disp_sys_irqlock((void*)&g_tv_data_lock, &flags);
	ptvp->enabled = 1;
	disp_sys_irqunlock((void*)&g_tv_data_lock, &flags);
	return 0;
}

s32 disp_tv_disable(struct disp_device* ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	unsigned long flags;
	struct disp_manager *mgr = NULL;
	printf("enter disp_tv_disable\n");
	if((NULL == ptv) || (NULL == ptvp)) {
	    DE_WRN("tv set func null  hdl!\n");
	    return DIS_FAIL;
	}

	mgr = ptv->manager;
	if(!mgr) {
		DE_WRN("tv%d's mgr is NULL\n", ptv->disp);
		return DIS_FAIL;
	}

	if(ptvp->enabled == 0) {
		DE_WRN("tv%d is already closed\n", ptv->disp);
		return DIS_FAIL;
	}
	
	if(ptvp->tv_func.tv_disable== NULL) {
		DE_WRN("tv_func.tv_disable is NULL\n");
		return -1;
	}

	ptvp->tv_func.tv_disable(ptv->disp);
	disp_al_tv_disable(ptv->disp);
	tv_clk_disable(ptv);
	ptvp->video_info = NULL;
	if(mgr->disable)
		mgr->disable(mgr);

	disp_sys_irqlock((void*)&g_tv_data_lock, &flags);
	ptvp->enabled = 0;
	disp_sys_irqunlock((void*)&g_tv_data_lock, &flags);
	//disp_delay_ms(900);
	/*
	disp_sys_disable_irq(ptvp->irq_no);
	disp_sys_unregister_irq(ptvp->irq_no, disp_tv_event_proc,(void*)ptv->disp);*/
	return 0;
}

static s32 tv_clk_exit(struct disp_device*  ptv)
{
	return 0;
}

static s32 disp_tv_init(struct disp_device*  ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if(!ptv || !ptvp) {
	    DE_WRN("tv init null hdl!\n");
	    return DIS_FAIL;
	}
#if defined(CONFIG_ARCH_SUN8IW6)
	tv_clk_init(ptv);
#endif
	return 0;
}


s32 disp_tv_exit(struct disp_device* ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if(!ptv || !ptvp) {
	    DE_WRN("tv init null hdl!\n");
	    return DIS_FAIL;
	}
	disp_tv_disable(ptv);
	tv_clk_exit(ptv);
	disp_sys_free(ptv);
	disp_sys_free(ptvp);
	ptv = NULL;
	ptvp = NULL;
	return 0;
}

s32 disp_tv_is_enabled(struct disp_device* ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}

	return ptvp->enabled;

}


s32 disp_tv_suspend(struct disp_device* ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(ptvp->tv_func.tv_suspend != NULL) {
		ptvp->tv_func.tv_suspend();
	}
	return 0;
}

s32 disp_tv_resume(struct disp_device* ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(ptvp->tv_func.tv_resume != NULL) {
		ptvp->tv_func.tv_resume();
	}
	return 0;
}

s32 disp_tv_set_mode(struct disp_device* ptv, disp_tv_mode tv_mode)
{
	s32 ret = 0;
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(ptvp->tv_func.tv_set_mode == NULL) {
		DE_WRN("tv_set_mode is null!\n");
		return DIS_FAIL;
	}

	ret = ptvp->tv_func.tv_set_mode(ptv->disp, tv_mode);
	if(ret == 0)
		ptvp->tv_mode = tv_mode;

	return ret;
}


s32 disp_tv_get_mode(struct disp_device* ptv)
{
	disp_tv_mode  tv_mode;
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(ptvp->tv_func.tv_get_mode == NULL) {
		DE_WRN("hdmi_set_mode is null!\n");
		return -1;
	}

	tv_mode = ptvp->tv_func.tv_get_mode(ptv->disp);

	if(tv_mode != ptvp->tv_mode)
		ptvp->tv_mode = tv_mode;

	return ptvp->tv_mode;

}

s32 disp_tv_get_input_csc(struct disp_device* ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}

	if(ptvp->tv_func.tv_get_input_csc == NULL)
		return -1;

	return ptvp->tv_func.tv_get_input_csc();			//0 or 1.
}


s32 disp_tv_set_func(struct disp_device*  ptv, disp_tv_func * func)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);

	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		DE_WRN("in  disp_set_tv_func,point  ptv = %u, point  ptvp = %u\n", (s32)ptv, (s32)ptvp);
		return DIS_FAIL;
	}
	ptvp->tv_func.tv_enable = func->tv_enable;
	ptvp->tv_func.tv_disable = func->tv_disable;
	ptvp->tv_func.tv_suspend = func->tv_suspend;
	ptvp->tv_func.tv_resume = func->tv_resume;
	ptvp->tv_func.tv_get_mode = func->tv_get_mode;
	ptvp->tv_func.tv_set_mode = func->tv_set_mode;
	ptvp->tv_func.tv_get_input_csc = func->tv_get_input_csc;
	ptvp->tv_func.tv_get_video_timing_info = func->tv_get_video_timing_info;
	ptvp->tv_func.tv_mode_support = func->tv_mode_support;
	ptvp->tv_func.tv_get_dac_hpd = func->tv_get_dac_hpd;
	return 0;
}

s32 disp_tv_detect(struct disp_device* ptv)
{
	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}
	if(ptvp->tv_func.tv_get_dac_hpd)
		return ptvp->tv_func.tv_get_dac_hpd(ptv->disp);
	return -1;
}


s32 disp_tv_check_support_mode(struct disp_device*  ptv, disp_tv_mode tv_mode)
{

	struct disp_device_private_data *ptvp = disp_tv_get_priv(ptv);
	if((NULL == ptv) || (NULL == ptvp)) {
		DE_WRN("tv set func null  hdl!\n");
		return DIS_FAIL;
	}
	if(ptvp->tv_func.tv_get_input_csc == NULL)
		return DIS_FAIL;
	return ptvp->tv_func.tv_mode_support(tv_mode);
}


s32 disp_init_tv(disp_bsp_init_para * para)  //call by disp_display
{

	s32   val;
	s32  type;

	type = disp_sys_script_get_item("tv_para", "tv_used", &val, 1);
	if(1 == type)
		g_tv_used = val;
	if(g_tv_used ) {
		__u32 num_screens;
		__u32 disp;
		struct disp_device* p_tv;
		struct disp_device_private_data* p_tvp;

		DE_WRN("[boot]disp_init_tv\n");
#if defined(__LINUX_PLAT__)
		spin_lock_init(&g_tv_data_lock);
#endif
		DE_WRN("[DISP_TV] disp_init_tv enter g_tv_used\n");
		num_screens = bsp_disp_feat_get_num_screens();
		g_ptv_devices = (struct disp_device *)disp_sys_malloc(sizeof(struct disp_device) * num_screens);
		if(NULL == g_ptv_devices) {
			DE_WRN("malloc memory fail!\n");
			return DIS_FAIL;
		}

		g_ptv_private = (struct disp_device_private_data *)disp_sys_malloc(sizeof(struct disp_device_private_data) * num_screens);
		if(NULL == g_ptv_private) {
			DE_WRN("malloc memory fail!\n");
			return DIS_FAIL;
		}

		for(disp=0; disp<num_screens; disp++) {
			p_tv = &g_ptv_devices[disp];
			p_tvp = &g_ptv_private[disp];

			if(!bsp_disp_feat_is_supported_output_types(disp, DISP_OUTPUT_TYPE_TV)) { //modify 1
			    DE_WRN("screen %d do not support TV TYPE!\n", disp);
			    continue;
			}

			p_tv->disp = disp;
			sprintf(p_tv->name, "tv%d", disp);
			p_tv->type = DISP_OUTPUT_TYPE_TV;
			p_tvp->tv_mode = DISP_TV_MOD_PAL;						//modifyed
			p_tvp->irq_no = para->irq_no[DISP_MOD_LCD0 + disp];			//modify
			p_tvp->clk = "tcon1";
			p_tvp->clk_parent = "pll_video0";

			p_tv->set_manager = disp_device_set_manager;
			p_tv->unset_manager = disp_device_unset_manager;
			p_tv->get_resolution = disp_device_get_resolution;
			p_tv->get_timings = disp_device_get_timings;

			p_tv->init =  disp_tv_init;
			p_tv->exit =  disp_tv_exit;
			p_tv->set_tv_func = disp_tv_set_func;
			p_tv->enable = disp_tv_enable;
			p_tv->disable = disp_tv_disable;
			p_tv->is_enabled = disp_tv_is_enabled;
			p_tv->set_mode = disp_tv_set_mode;
			p_tv->get_mode = disp_tv_get_mode;
			p_tv->check_support_mode = disp_tv_check_support_mode;
			p_tv->get_input_csc = disp_tv_get_input_csc;
			p_tv->detect = disp_tv_detect;
			p_tv->suspend = disp_tv_suspend;
			p_tv->resume = disp_tv_resume;
			p_tv->init(p_tv);
			if(bsp_disp_feat_is_supported_output_types(disp, DISP_OUTPUT_TYPE_TV))
				disp_device_register(p_tv);
			DE_WRN("[BOOOT_DISP_TV] disp tv device_registered\n");
		}


	}
	return 0;
}

#endif
