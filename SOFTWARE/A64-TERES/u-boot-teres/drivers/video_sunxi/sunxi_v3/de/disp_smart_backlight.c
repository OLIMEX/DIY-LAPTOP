#include "disp_smart_backlight.h"

struct disp_smbl_private_data
{
	struct disp_smbl_info     info;
	bool                      applied;

	s32 (*shadow_protect)(u32 sel, bool protect);

	u32                       enabled;
#if defined(__LINUX_PLAT__)
	struct mutex              mlock;
#else
	int                       mlock;
#endif
};

#if defined(__LINUX_PLAT__)
static spinlock_t smbl_data_lock;
#else
static int smbl_data_lock;
#endif

//#define SMBL_NO_AL
static struct disp_smbl *smbls = NULL;
static struct disp_smbl_private_data *smbl_private;

struct disp_smbl* disp_get_smbl(u32 disp)
{
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();
	if(disp >= num_screens) {
		DE_WRN("disp %d out of range\n", disp);
		return NULL;
	}
	DE_INF("get smbl%d ok\n", disp);

	return &smbls[disp];
}
static struct disp_smbl_private_data *disp_smbl_get_priv(struct disp_smbl *smbl)
{
	if(NULL == smbl) {
		DE_INF("NULL hdl!\n");
		return NULL;
	}

	return &smbl_private[smbl->disp];
}

static s32 disp_smbl_update_regs(struct disp_smbl* smbl)
{
	unsigned long flags;
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	disp_sys_irqlock((void*)&smbl_data_lock, &flags);
	if(true == smblp->applied) {
		disp_al_smbl_update_regs(smbl->disp);
		smblp->applied = false;
	}
	disp_sys_irqunlock((void*)&smbl_data_lock, &flags);

	return 0;
}

//should protect width @mlock
static s32 disp_smbl_update_backlight(struct disp_smbl* smbl, unsigned int bl)
{
	if(NULL == smbl) {
		DE_INF("NULL hdl!\n");
		return -1;
	}

	smbl->backlight = bl;

	return 0;
}


//should protect width @mlock
static s32 disp_smbl_apply(struct disp_smbl* smbl)
{
	unsigned long flags;
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);
	struct disp_smbl_info info;

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	DE_INF("smbl %d apply, en=%d, win=<%d,%d,%d,%d>\n", smbl->disp,
	smblp->info.enable, smblp->info.window.x, smblp->info.window.y,
	smblp->info.window.width, smblp->info.window.height);

	memset(&info, 0, sizeof(struct disp_smbl_info));

	//disp_sys_lock((void *)&smblp->mlock);
	if(smbl->backlight != smblp->info.backlight) {
		smblp->info.backlight = smbl->backlight;
		smblp->info.flags |= SMBL_DIRTY_BL;
	}
	if(SMBL_DIRTY_NONE != smblp->info.flags) {
		memcpy(&info, &smblp->info, sizeof(struct disp_smbl_info));
		smblp->info.flags = SMBL_DIRTY_NONE;
	}
	//disp_sys_unlock((void *)&smblp->mlock);

	if(SMBL_DIRTY_NONE != info.flags) {
		disp_smbl_shadow_protect(smbl, true);
		disp_al_smbl_apply(smbl->disp, &info);
		disp_smbl_shadow_protect(smbl, false);
		disp_sys_irqlock((void*)&smbl_data_lock, &flags);
		smblp->applied = true;
		disp_sys_irqunlock((void*)&smbl_data_lock, &flags);
	}

	return 0;
}

s32 disp_smbl_force_update_regs(struct disp_smbl* smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	DE_INF("disp_smbl_force_update_regs, smbl %d\n", smbl->disp);
	disp_sys_lock((void *)&smblp->mlock);
	smblp->info.flags = SMBL_DIRTY_ALL;
	disp_smbl_apply(smbl);
	disp_sys_unlock((void *)&smblp->mlock);

	return 0;
}

static s32 disp_smbl_sync(struct disp_smbl* smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);
	struct disp_manager *mgr;
	unsigned int dimming;
	bool dimming_update = false;

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}

	//disp_smbl_apply(smbl);
	disp_smbl_update_regs(smbl);

	disp_al_smbl_sync(smbl->disp);
	dimming = disp_al_smbl_get_status(smbl->disp);
	if(smblp->info.backlight_dimming != dimming) {
		smblp->info.backlight_dimming = disp_al_smbl_get_status(smbl->disp);
		dimming_update = true;
	}

	mgr = smbl->manager;
	if(mgr && mgr->device) {
		struct disp_device *dispdev = mgr->device;
		if(DISP_OUTPUT_TYPE_LCD == dispdev->type && dimming_update) {
			if(dispdev->set_bright_dimming)
				dispdev->set_bright_dimming(dispdev, smblp->info.backlight_dimming);
		}
	}

	return 0;
}

static bool disp_smbl_is_enabled(struct disp_smbl* smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return false;
	}

	return (smblp->info.enable == 1);
}

static s32 disp_smbl_enable(struct disp_smbl* smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);
	struct disp_device *dispdev = NULL;

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	if(smbl->manager)
		dispdev = smbl->manager->device;
	if(dispdev)
		dispdev->get_resolution(dispdev, &smblp->info.size.width,
			&smblp->info.size.height);
	if((0 == smblp->info.window.width) || (0 == smblp->info.window.height)) {
		smblp->info.window.width = smblp->info.size.width;
		smblp->info.window.height = smblp->info.size.height;
	}

	DE_INF("smbl %d enable\n", smbl->disp);
	disp_sys_lock((void *)&smblp->mlock);
	smblp->info.enable = 1;
	smblp->info.flags |= SMBL_DIRTY_ENABLE;
	disp_smbl_apply(smbl);
	disp_sys_unlock((void *)&smblp->mlock);

	return 0;
}

static s32 disp_smbl_disable(struct disp_smbl* smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	DE_INF("smbl %d disable\n", smbl->disp);

	disp_sys_lock((void *)&smblp->mlock);
	smblp->info.enable = 0;
	smblp->info.flags |= SMBL_DIRTY_ENABLE;
	disp_smbl_apply(smbl);
	disp_sys_unlock((void *)&smblp->mlock);

	return 0;
}

s32 disp_smbl_shadow_protect(struct disp_smbl *smbl, bool protect)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}

	if(smblp->shadow_protect)
		return smblp->shadow_protect(smbl->disp, protect);

	return -1;
}

static s32 disp_smbl_set_window(struct disp_smbl* smbl, disp_rect *window)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	disp_sys_lock((void *)&smblp->mlock);
	memcpy(&smblp->info.window, window, sizeof(disp_rect));
	smblp->info.flags |= SMBL_DIRTY_WINDOW;
	disp_smbl_apply(smbl);
	disp_sys_unlock((void *)&smblp->mlock);

	return 0;
}

static s32 disp_smbl_get_window(struct disp_smbl* smbl, disp_rect *window)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	disp_sys_lock((void *)&smblp->mlock);
	memcpy(window, &smblp->info.window, sizeof(disp_rect));
	disp_sys_unlock((void *)&smblp->mlock);

	return 0;
}

static s32 disp_smbl_set_manager(struct disp_smbl* smbl, struct disp_manager *mgr)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == mgr) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}

	DE_INF("smbl %d -> mgr %d\n", smbl->disp, mgr->disp);
	disp_sys_lock((void *)&smblp->mlock);
	smbl->manager = mgr;
	mgr->smbl = smbl;
	disp_sys_unlock((void *)&smblp->mlock);

	return 0;
}

static s32 disp_smbl_unset_manager(struct disp_smbl* smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}
	disp_sys_lock((void *)&smblp->mlock);
	if(smbl->manager)
		smbl->manager->smbl = NULL;
	smbl->manager = NULL;
	disp_sys_unlock((void *)&smblp->mlock);

	return 0;
}


static s32 disp_smbl_init(struct disp_smbl *smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}

	return 0;
}

static s32 disp_smbl_exit(struct disp_smbl *smbl)
{
	struct disp_smbl_private_data *smblp = disp_smbl_get_priv(smbl);

	if((NULL == smbl) || (NULL == smblp)) {
		DE_INF("NULL hdl!\n");
		return -1;
	}

	return 0;
}

s32 disp_init_smbl(disp_bsp_init_para * para)
{
	u32 num_smbls;
	u32 disp;
	struct disp_smbl *smbl;
	struct disp_smbl_private_data *smblp;

	DE_INF("disp_init_smbl\n");

#if defined(__LINUX_PLAT__)
	spin_lock_init(&smbl_data_lock);
#endif
	num_smbls = bsp_disp_feat_get_num_screens();
	smbls = (struct disp_smbl *)disp_sys_malloc(sizeof(struct disp_smbl) * num_smbls);
	if(NULL == smbls) {
		DE_WRN("malloc memory fail!\n");
		return DIS_FAIL;
	}
	smbl_private = (struct disp_smbl_private_data *)disp_sys_malloc(sizeof(struct disp_smbl_private_data) * num_smbls);
	if(NULL == smbl_private) {
		DE_WRN("malloc memory fail!\n");
		return DIS_FAIL;
	}

	for(disp=0; disp<num_smbls; disp++) {
		smbl = &smbls[disp];
		smblp = &smbl_private[disp];
#if defined(__LINUX_PLAT__)
		mutex_init(&smblp->mlock);
#endif

		switch(disp) {
		case 0:
			smbl->name = "smbl0";
			smbl->disp = 0;

			break;
		case 1:
			smbl->name = "smbl1";
			smbl->disp = 1;

			break;
		case 2:
			smbl->name = "smbl2";
			smbl->disp = 2;

			break;
		}
		smblp->shadow_protect = para->shadow_protect;

		smbl->enable = disp_smbl_enable;
		smbl->disable = disp_smbl_disable;
		smbl->is_enabled = disp_smbl_is_enabled;
		smbl->init = disp_smbl_init;
		smbl->exit = disp_smbl_exit;
		smbl->apply = disp_smbl_apply;
		smbl->update_regs = disp_smbl_update_regs;
		smbl->sync = disp_smbl_sync;
		smbl->set_manager = disp_smbl_set_manager;
		smbl->unset_manager = disp_smbl_unset_manager;
		smbl->set_window = disp_smbl_set_window;
		smbl->get_window = disp_smbl_get_window;
		smbl->update_backlight = disp_smbl_update_backlight;

		smbl->init(smbl);
	}

	return 0;
}

