#include "disp_vdevice.h"

struct disp_vdevice_private_data {
	u32 enabled;

	disp_tv_mode mode;

	disp_device_func func;
	disp_video_timings *video_info;
	disp_vdevice_interface_para intf;

	u32 irq_no;

	char *clk;
	char *clk_parent;
#if defined(__LINUX_PLAT__)
	struct mutex mlock;
#else
	int mlock;
#endif
};

static struct disp_vdevice_private_data *disp_vdevice_get_priv(struct disp_device *vdevice)
{
	if(NULL == vdevice) {
		DE_WRN("NULL hdl!\n");
		return NULL;
	}

	return (struct disp_vdevice_private_data *)vdevice->priv_data;
}

static s32 vdevice_clk_init(struct disp_device *vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if(!vdevice || !vdevicep) {
	    DE_WRN("null hdl!\n");
	    return DIS_FAIL;
	}
	disp_sys_clk_set_parent(vdevicep->clk, vdevicep->clk_parent);

	return 0;
}

static s32 vdevice_clk_exit(struct disp_device *vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if(!vdevice || !vdevicep) {
		DE_WRN("null hdl!\n");
		return DIS_FAIL;
	}

	return 0;
}

static s32 vdevice_clk_config(struct disp_device *vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	disp_panel_para *para;
	struct lcd_clk_info clk_info;
	unsigned long pll_rate, lcd_rate, dclk_rate;//hz
	unsigned long pll_rate_set, lcd_rate_set, dclk_rate_set;//hz

	if(!vdevice || !vdevicep) {
	    DE_WRN("null hdl!\n");
	    return DIS_FAIL;
	}

	memset(&clk_info, 0, sizeof(struct lcd_clk_info));
	para = (disp_panel_para*)disp_sys_malloc(sizeof(disp_panel_para));
	dclk_rate = vdevicep->video_info->pixel_clk * (vdevicep->video_info->pixel_repeat + 1);
	para->lcd_if = vdevicep->intf.intf;
	para->lcd_dclk_freq = dclk_rate;
	disp_al_lcd_get_clk_info(vdevice->disp, &clk_info, para);
	disp_sys_free((void*)para);
	clk_info.tcon_div = 11;//fixme
	lcd_rate = dclk_rate * clk_info.tcon_div;
	pll_rate = lcd_rate * clk_info.lcd_div;
	disp_sys_clk_set_rate(vdevicep->clk_parent, pll_rate);
	pll_rate_set = disp_sys_clk_get_rate(vdevicep->clk_parent);
	lcd_rate_set = pll_rate_set / clk_info.lcd_div;
	disp_sys_clk_set_rate(vdevicep->clk, lcd_rate_set);
	lcd_rate_set = disp_sys_clk_get_rate(vdevicep->clk);
	dclk_rate_set = lcd_rate_set / clk_info.tcon_div;
	if(dclk_rate_set != dclk_rate)
		DE_WRN("pclk=%ld, cur=%ld\n", dclk_rate, dclk_rate_set);

	return 0;
}

static s32 vdevice_clk_enable(struct disp_device *vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if(!vdevice || !vdevicep) {
	    DE_WRN("null hdl!\n");
	    return DIS_FAIL;
	}

	vdevice_clk_config(vdevice);
	disp_sys_clk_enable(vdevicep->clk);

	return 0;
}

static s32 vdevice_clk_disable(struct disp_device *vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if(!vdevice || !vdevicep) {
	    DE_WRN("null hdl!\n");
	    return DIS_FAIL;
	}

	disp_sys_clk_disable(vdevicep->clk);

	return 0;
}

//FIXME
extern void sync_event_proc(u32 disp);
#if defined(__LINUX_PLAT__)
static s32 disp_vdevice_event_proc(int irq, void *parg)
#else
static s32 disp_vdevice_event_proc(void *parg)
#endif
{
	u32 disp = (u32)parg;

	struct disp_device *vdevice = (struct disp_device*)parg;
	struct disp_manager *mgr = NULL;

	if(NULL == vdevice)
		return DISP_IRQ_RETURN;
	mgr = vdevice->manager;
	if(NULL == mgr)
		return DISP_IRQ_RETURN;
	disp = vdevice->disp;

	if(disp_al_device_query_irq(disp)) {
		int cur_line = disp_al_device_get_cur_line(disp);
		int start_delay = disp_al_device_get_start_delay(disp);

		if(cur_line <= (start_delay-4)) {
			sync_event_proc(mgr->disp);
		} else {
			/* skip a frame */
		}
	}

	return DISP_IRQ_RETURN;
}

static s32 disp_vdevice_init(struct disp_device*  vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if(!vdevice || !vdevicep) {
	    DE_WRN("null hdl!\n");
	    return DIS_FAIL;
	}

	vdevice_clk_init(vdevice);
	return 0;
}

static s32 disp_vdevice_exit(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if(!vdevice || !vdevicep) {
	    DE_WRN("null hdl!\n");
	    return DIS_FAIL;
	}

	vdevice_clk_exit(vdevice);

  return 0;
}

static s32 disp_vdevice_enable(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	if(vdevicep->enabled == 1) {
		DE_WRN("%s%d is already enabled\n", vdevice->name, vdevice->disp);
		return DIS_FAIL;
	}

	disp_sys_lock((void*)&vdevicep->mlock);
	if(NULL != vdevicep->func.enable)
		vdevicep->func.enable();
	else
		DE_WRN("vdevice_enable is NULL\n");
	disp_sys_unlock((void*)&vdevicep->mlock);

	return 0;
}


static s32 disp_vdevice_disable(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);

	if((NULL == vdevice) || (NULL == vdevicep)) {
	    DE_WRN("null  hdl!\n");
	    return DIS_FAIL;
	}

	if(vdevicep->enabled == 0) {
		DE_WRN("%s%d is already disabled\n", vdevice->name, vdevice->disp);
		return DIS_FAIL;
	}

	disp_sys_lock((void*)&vdevicep->mlock);
	if(vdevicep->func.disable)
		vdevicep->func.disable();
	else
		DE_WRN("vdevice_disable is NULL\n");
	disp_sys_unlock((void*)&vdevicep->mlock);

	return 0;
}

static s32 disp_vdevice_is_enabled(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	return vdevicep->enabled;
}


static s32 disp_vdevice_set_mode(struct disp_device* vdevice, u32 mode)
{
	s32 ret = 0;
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	if(vdevicep->func.set_mode == NULL) {
		DE_WRN("vdevice_set_mode is null!\n");
		return -1;
	}

	ret = vdevicep->func.set_mode((disp_tv_mode)mode);

	if(ret == 0)
		vdevicep->mode = mode;

	return ret;
}

static s32 disp_vdevice_get_mode(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	return vdevicep->mode;
}

static s32 disp_vdevice_detect(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}
	if (vdevicep->func.get_HPD_status)
		return vdevicep->func.get_HPD_status();
	return DIS_FAIL;
}

static s32 disp_vdevice_check_support_mode(struct disp_device* vdevice, u32 mode)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	if(vdevicep->func.mode_support == NULL)
		return -1;

	return vdevicep->func.mode_support(mode);
}

static s32 disp_vdevice_get_input_csc(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	if(vdevicep->func.get_input_csc == NULL)
		return 0;

	return vdevicep->func.get_input_csc();
}

static s32 disp_vdevice_get_input_color_range(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	return DISP_COLOR_RANGE_0_255;
}

static s32 disp_vdevice_suspend(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	DE_WRN("\n");
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	if(vdevicep->func.suspend != NULL) {
		vdevicep->func.suspend();
	}

	return 0;
}

static s32 disp_vdevice_resume(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	DE_WRN("\n");
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}

	if(vdevicep->func.resume != NULL) {
		vdevicep->func.resume();
	}

	return 0;
}

static s32 disp_vdevice_tcon_enable(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	struct disp_manager *mgr = NULL;
	if((NULL == vdevice) || (NULL == vdevicep)) {
		DE_WRN("null  hdl!\n");
		return DIS_FAIL;
	}
	mgr = vdevice->manager;
	if(!mgr) {
		DE_WRN("%s%d's mgr is NULL\n", vdevice->name, vdevice->disp);
		return DIS_FAIL;
	}

	if(vdevicep->func.get_video_timing_info == NULL) {
		DE_WRN("vdevice_get_video_timing_info func is null\n");
		return DIS_FAIL;
	}

	vdevicep->func.get_video_timing_info(&(vdevicep->video_info));

	if(vdevicep->video_info == NULL) {
		DE_WRN("video info is null\n");
		return DIS_FAIL;
	}

	if(vdevicep->func.get_interface_para == NULL) {
		DE_WRN("get_interface_para func is null\n");
		return DIS_FAIL;
	}
	vdevicep->func.get_interface_para((void*)&(vdevicep->intf));

	memcpy(&vdevice->timings, vdevicep->video_info, sizeof(disp_video_timings));
	if(mgr->enable)
		mgr->enable(mgr);

	disp_sys_register_irq(vdevicep->irq_no,0,disp_vdevice_event_proc,(void*)vdevice,0,0);
	disp_sys_enable_irq(vdevicep->irq_no);

	vdevice_clk_enable(vdevice);
	disp_al_vdevice_cfg(vdevice->disp, &vdevice->timings, &vdevicep->intf);
	disp_al_vdevice_enable(vdevice->disp);

	vdevicep->enabled = 1;

	return 0;
}

static s32 disp_vdevice_tcon_disable(struct disp_device* vdevice)
{
	struct disp_vdevice_private_data *vdevicep = disp_vdevice_get_priv(vdevice);
	struct disp_manager *mgr = NULL;

	if((NULL == vdevice) || (NULL == vdevicep)) {
	    DE_WRN("null  hdl!\n");
	    return DIS_FAIL;
	}

	mgr = vdevice->manager;
	if(!mgr) {
		DE_WRN("%s%d's mgr is NULL\n", vdevice->name, vdevice->disp);
		return DIS_FAIL;
	}

	if(vdevicep->enabled == 0) {
		DE_WRN("%s%d is already closed\n", vdevice->name, vdevice->disp);
		return DIS_FAIL;
	}

	disp_al_vdevice_disable(vdevice->disp);
	vdevice_clk_disable(vdevice);

	if(mgr->disable)
		mgr->disable(mgr);

	vdevicep->enabled = 0;
	disp_sys_disable_irq(vdevicep->irq_no);
	disp_sys_unregister_irq(vdevicep->irq_no, disp_vdevice_event_proc,(void*)vdevice);

	return 0;
}

struct disp_device* disp_vdevice_register(disp_vdevice_init_data *data)
{
	struct disp_device *vdevice;
	struct disp_vdevice_private_data *vdevicep;

	vdevice = (struct disp_device *)disp_sys_malloc(sizeof(struct disp_device));
	if(NULL == vdevice) {
			DE_WRN("malloc memory fail!\n");
			return NULL;
	}
	vdevicep = (struct disp_vdevice_private_data *)disp_sys_malloc(sizeof(struct disp_vdevice_private_data));
	if(NULL == vdevicep) {
			DE_WRN("malloc memory fail!\n");
			disp_sys_free(vdevice);
			return NULL;
	}
#if defined(__LINUX_PLAT__)
	mutex_init(&vdevicep->mlock);
#endif
	memcpy(&vdevice->name, data->name, 32);
	vdevice->disp = data->disp;
	vdevice->fix_timing = data->fix_timing;
	vdevice->type = data->type;
	memcpy(&vdevicep->func, &data->func, sizeof(disp_device_func));

	vdevicep->irq_no = gdisp.init_para.irq_no[DISP_MOD_LCD0 + vdevice->disp];
	switch(vdevice->disp) {
	case 0:
		vdevicep->clk = DE_LCD_CLK0;
		break;
	case 1:
		vdevicep->clk = DE_LCD_CLK1;//TCON clk
		break;
	default:
		vdevicep->clk = DE_LCD_CLK0;
	}
	vdevicep->clk_parent = DE_LCD_CLK_SRC;

	vdevice->set_manager = disp_device_set_manager;
	vdevice->unset_manager = disp_device_unset_manager;
	vdevice->get_resolution = disp_device_get_resolution;
	vdevice->get_timings = disp_device_get_timings;
	vdevice->is_interlace = disp_device_is_interlace;

	vdevice->init = disp_vdevice_init;
	vdevice->exit = disp_vdevice_exit;

	vdevice->enable = disp_vdevice_enable;
//	vdevice->sw_enable = disp_vdevice_sw_enable;
	vdevice->disable = disp_vdevice_disable;
	vdevice->is_enabled = disp_vdevice_is_enabled;
	vdevice->set_mode = disp_vdevice_set_mode;
	vdevice->get_mode = disp_vdevice_get_mode;
	vdevice->check_support_mode = disp_vdevice_check_support_mode;
	vdevice->get_input_csc = disp_vdevice_get_input_csc;
	vdevice->get_input_color_range = disp_vdevice_get_input_color_range;
	vdevice->suspend = disp_vdevice_suspend;
	vdevice->resume = disp_vdevice_resume;
	vdevice->detect = disp_vdevice_detect;

	vdevice->priv_data = (void*)vdevicep;
	vdevice->init(vdevice);
	disp_device_register(vdevice);

	return vdevice;
}

s32 disp_vdevice_unregister(struct disp_device *vdevice)
{
	struct disp_vdevice_private_data *vdevicep;

	if(NULL == vdevice) {
		DE_WRN("null hdl\n");
		return DIS_FAIL;
	}
	disp_device_unset_manager(vdevice);
	disp_device_unregister(vdevice);
	vdevice->exit(vdevice);
	vdevicep = (struct disp_vdevice_private_data *)vdevice->priv_data;

	disp_sys_free((void*)vdevice);
	disp_sys_free((void*)vdevicep);

	return 0;
}

static disp_vdevice_source_ops vdev_source_ops =
{
	.tcon_enable = disp_vdevice_tcon_enable,
	.tcon_disable = disp_vdevice_tcon_disable,
};

s32 disp_vdevice_get_source_ops(disp_vdevice_source_ops *ops)
{
	if(ops)
		memcpy((void*)ops, (void*)&vdev_source_ops, sizeof(disp_vdevice_source_ops));

	return 0;
}

#if defined(__LINUX_PLAT__)
EXPORT_SYMBOL(disp_vdevice_register);
EXPORT_SYMBOL(disp_vdevice_unregister);
EXPORT_SYMBOL(disp_vdevice_get_source_ops);
#endif
