/*
 * MMC driver for allwinner sunxi platform.
 *
 */
#include <config.h>
#include <common.h>
#include <command.h>
#include <errno.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <linux/list.h>
#include <div64.h>
#include "mmc_private.h"
#include "mmc_def.h"

static ulong mmc_do_burn_boot(int dev_num, lbaint_t start, lbaint_t blkcnt, const void*src)
{
	s32 err = 0;
	ulong rblk, blocks_do = 0;
	void *dst = NULL;
	struct mmc *mmc = find_mmc_device(dev_num);

	if ((dst = (void *)malloc(blkcnt * mmc->write_bl_len)) == NULL) {
		MMCINFO("%s: request memory fail\n", __FUNCTION__);
		return 0;
	}

	/* write boot0 */
	blocks_do = mmc_bwrite(dev_num, start, blkcnt, src);
	if (blocks_do != blkcnt) {
		MMCINFO("%s: write boot0 fail\n", __FUNCTION__);
		err = -1;
		goto ERR_RET;
	}

	/* read and check */
	rblk = mmc_bread(dev_num, start, blkcnt, dst);
	if ((rblk<blkcnt) || memcmp(src, dst, blkcnt * mmc->write_bl_len)) {
		MMCINFO("%s: check boot0 fail\n", __FUNCTION__);
		err = -1;
	} else
		MMCDBG("%s: check boot0 ok\n", __FUNCTION__);

ERR_RET:
	free(dst);
	return (err ? 0 : blocks_do);
}

static ulong mmc_burn_boot(int dev_num, lbaint_t start, lbaint_t blkcnt, const void*src)
{
	lbaint_t blocks_do_user = 0, blocks_do_boot = 0;
	ulong start_todo = start;
	signed int err = 0;
	struct mmc *mmc = find_mmc_device(dev_num);

	if ((mmc->cfg->platform_caps.drv_burn_boot_pos & DRV_PARA_BURN_EMMC_BOOT_PART)
		&& mmc->boot_support)
	{
		/* enable boot mode */
		err = mmc_switch_boot_bus_cond(dev_num,
										MMC_SWITCH_BOOT_SDR_NORMAL,
										MMC_SWITCH_BOOT_RST_BUS_COND,
										MMC_SWITCH_BOOT_BUS_SDRx4_DDRx4);
		if (err) {
			MMCINFO("mmc switch boot bus condition failed\n");
			goto DISABLE_BOOT_MODE;
		}

		/* configure boot mode */
		err = mmc_switch_boot_part(dev_num,
									MMC_SWITCH_PART_BOOT_ACK_ENB,
									MMC_SWITCH_PART_BOOT_PART_1);
		if (err) {
			MMCINFO("mmc configure boot partition failed\n");
			goto DISABLE_BOOT_MODE;
		}

		err = mmc_switch_part(dev_num, MMC_SWITCH_PART_BOOT_PART_1);
		if (err) {
			MMCINFO("switch to boot1 partition failed\n");
			goto DISABLE_BOOT_MODE;
		}
		/*In eMMC boot partition,boot0.bin start from 0 sector,so we must change start form 16 to 0*/
		start_todo = 0;
		blocks_do_boot = mmc_do_burn_boot(dev_num, start_todo, blkcnt, src);
		if (blocks_do_boot != blkcnt)
			MMCINFO("burn boot to emmc boot1 partition failed" LBAF "\n", blocks_do_boot);

		/* switch back to user partiton */
		err = mmc_switch_part(dev_num, 0);
		if (err) {
			MMCINFO("switch back to user partition failed\n");
			return 0;
		}
	}

DISABLE_BOOT_MODE:
	if ((err || !(mmc->cfg->platform_caps.drv_burn_boot_pos & DRV_PARA_BURN_EMMC_BOOT_PART))
			&& mmc->boot_support)
	{
		/* disable boot mode */
		err = mmc_switch_boot_part(dev_num, 0, MMC_SWITCH_PART_BOOT_PART_NONE);
		if (err)
			MMCINFO("mmc disable boot mode failed\n");
	}

	/* burn boot to user partition */
	if (!(mmc->cfg->platform_caps.drv_burn_boot_pos & DRV_PARA_NOT_BURN_USER_PART))
	{
		blocks_do_user = mmc_do_burn_boot(dev_num, start, blkcnt, src);
		if (blocks_do_user != blkcnt)
			MMCINFO("burn boot to user partition failed," LBAFU "\n", blocks_do_user);
	}

	if (blocks_do_user == blkcnt || blocks_do_boot == blkcnt)
		return blkcnt;
	else
		return 0;
}

/* user same memory, if get a good boot0, exit.*/
static ulong mmc_get_boot(int dev_num, lbaint_t start, lbaint_t blkcnt, void *dst)
{
	lbaint_t blocks_do = 0;
	ulong start_todo = start;
	struct mmc *mmc = find_mmc_device(dev_num);

	if (!(mmc->cfg->platform_caps.drv_burn_boot_pos & DRV_PARA_NOT_BURN_USER_PART))
	{
		blocks_do = mmc_bread(dev_num, start_todo, blkcnt, dst);
		if (blocks_do != blkcnt) {
			MMCINFO("%s: get boot0 from user part err\n", __FUNCTION__);
		} else {
			MMCDBG("%s: get boot0 from user part ok, return..\n", __FUNCTION__);
			goto RET;
		}
	}

	if (!(mmc->cfg->platform_caps.drv_burn_boot_pos & DRV_PARA_BURN_EMMC_BOOT_PART)
		&& mmc->boot_support)
	{
		if (mmc_switch_part(dev_num, MMC_SWITCH_PART_BOOT_PART_1))
		{
			MMCINFO("%s: switch to boot1 part failed\n", __FUNCTION__);
			goto RET;
		}

		/*In eMMC boot partition,boot0.bin start from 0 sector,so we must change start form 16 to 0*/
		start_todo= 0;

		blocks_do = mmc_bread(dev_num, start_todo, blkcnt, dst);
		if (blocks_do != blkcnt) {
			MMCINFO("%s: get boot0 from boot part err\n", __FUNCTION__);
		} else {
			MMCDBG("%s: get boot0 from boot part ok\n", __FUNCTION__);
		}

		if (mmc_switch_part(dev_num, 0)) {
			MMCINFO("%s: switch back to user part failed,"
				"it maybe still in boot part!!!\n", __FUNCTION__);
			goto RET;
		}
	}

RET:
	return blocks_do;
}

static ulong mmc_bwrite_mass_pro(int dev_num, ulong start, lbaint_t blkcnt, const void*src)
{
	MMCDBG("%s: dev %d, start %ld, blkcnt %ld src 0x%x\n", __FUNCTION__, dev_num, start, blkcnt, src);

	if ((start == BOOT0_SDMMC_START_ADDR)
		&& ((start+blkcnt) < CONFIG_MMC_LOGICAL_OFFSET))
	{
		MMCDBG("%s: start burn boot data...\n", __FUNCTION__);
		return mmc_burn_boot(dev_num, start, blkcnt, src);
	}
	else /*if (start > CONFIG_MMC_LOGICAL_OFFSET)*/
	{
		MMCDBG("%s: start burn user data...\n", __FUNCTION__);
		return mmc_bwrite(dev_num, start, blkcnt, src);
	}
	/*
	else
	{
		MMCINFO("%s: input parameter error!\n", __FUNCTION__);
	}
	*/

	return 0;
}

static ulong mmc_bread_mass_pro(int dev_num, ulong start, lbaint_t blkcnt, void *dst)
{
	MMCDBG("%s: dev %d, start %ld, blkcnt %ld src 0x%x\n", __FUNCTION__, dev_num, start, blkcnt, dst);

	if ((start == BOOT0_SDMMC_START_ADDR)
		&& ((start+blkcnt) < CONFIG_MMC_LOGICAL_OFFSET))
	{
		MMCDBG("%s: start read boot data...\n", __FUNCTION__);
		return mmc_get_boot(dev_num, start, blkcnt, dst);
	}
	else /*if (start > CONFIG_MMC_LOGICAL_OFFSET)*/
	{
		MMCDBG("%s: start read user data...\n", __FUNCTION__);
		return mmc_bread(dev_num, start, blkcnt, dst);
	}
	/*
	else
	{
		MMCINFO("%s: input parameter error!\n", __FUNCTION__);
	}
	*/

	return 0;
}

#ifdef CONFIG_SUNXI_SECURE_STORAGE

static int
check_secure_area(ulong start, lbaint_t blkcnt)
{
	u32 sta_add = start;
	u32 end_add = start + blkcnt -1;
	u32 se_sta_add = SDMMC_SECURE_STORAGE_START_ADD;
	u32 se_end_add = SDMMC_SECURE_STORAGE_START_ADD +(SDMMC_ITEM_SIZE*2*MAX_SECURE_STORAGE_MAX_ITEM)-1;
	if(blkcnt<=(SDMMC_ITEM_SIZE*2*MAX_SECURE_STORAGE_MAX_ITEM)){
		if(((sta_add >= se_sta_add)&&(sta_add <= se_end_add))
			||((end_add >= se_sta_add)&&(end_add <= se_end_add))){
			return 1;
		}
	}else{
		if(((se_sta_add >= sta_add)&&(se_sta_add <= end_add))
			||((se_end_add >= sta_add)&&(se_end_add <= end_add))){
			return 1;
		}
	}

	return 0;
}

static ulong
mmc_bread_secure(int dev_num, ulong start, lbaint_t blkcnt, void *dst)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %ld,end %ld\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}

	return mmc_bread(dev_num, start, blkcnt, dst);
}

static ulong
mmc_bwrite_secure(int dev_num, ulong start, lbaint_t blkcnt, const void*src)
{
	if(check_secure_area(start,blkcnt)){
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %ld,end %ld\n",\
			__FUNCTION__,__LINE__ ,start,start + blkcnt-1);
		return -1;
	}

	return mmc_bwrite(dev_num, start, blkcnt, src);
}

static ulong
mmc_berase_secure(int dev_num, unsigned long start, lbaint_t blkcnt)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %ld,end %ld\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}

	return mmc_berase(dev_num, start, blkcnt);
}

static ulong
mmc_bwrite_mass_pro_secure(int dev_num, ulong start, lbaint_t blkcnt, const void*src)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %ld,end %ld\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}

	return mmc_bwrite_mass_pro(dev_num, start, blkcnt, src);
}

static ulong
mmc_bread_mass_pro_secure(int dev_num, ulong start, lbaint_t blkcnt, void *dst)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %ld,end %ld\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}

	return mmc_bread_mass_pro(dev_num, start, blkcnt, dst);
}


static int
mmc_secure_wipe_secure(int dev_num, unsigned int start, unsigned int blkcnt, unsigned int *skip_space)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %d,end %d\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}

	return mmc_secure_wipe(dev_num, start, blkcnt, skip_space);
}

static int
mmc_mmc_erase_secure(int dev_num, unsigned int start, unsigned int blkcnt, unsigned int *skip_space)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %d,end %d\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}
	return mmc_mmc_erase(dev_num, start, blkcnt, skip_space);
}

static int
mmc_mmc_trim_secure(int dev_num, unsigned int start, unsigned int blkcnt)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %d,end %d\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}
	return mmc_mmc_trim(dev_num, start, blkcnt);
}

static int
mmc_mmc_discard_secure(int dev_num, unsigned int start, unsigned int blkcnt)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %d,end %d\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}
	return mmc_mmc_discard(dev_num, start, blkcnt);
}

static int
mmc_mmc_sanitize_secure(int dev_num)
{
	return mmc_mmc_sanitize(dev_num);
}
static int
mmc_mmc_secure_erase_secure(int dev_num, unsigned int start, unsigned int blkcnt, unsigned int *skip_space)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %d,end %d\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}
	return mmc_mmc_secure_erase(dev_num, start, blkcnt, skip_space);
}

static int
mmc_mmc_secure_trim_secure(int dev_num, unsigned int start, unsigned int blkcnt)
{
	if (check_secure_area(start,blkcnt)) {
		MMCINFO("Should not w/r secure area in fun %s,line,%d in start %d,end %d\n",\
			__FUNCTION__,__LINE__,start,start + blkcnt-1);
		return -1;
	}
	return mmc_mmc_secure_trim(dev_num, start, blkcnt);
}

static int
get_sdmmc_secure_storage_max_item(void)
{
	return MAX_SECURE_STORAGE_MAX_ITEM;
}

static int
sdmmc_secure_storage_write(s32 dev_num,u32 item,u8 *buf , lbaint_t blkcnt)
{
	s32 ret = 0;

	if(buf == NULL){
		MMCINFO("intput buf is NULL \n");
		return -1;
	}

	if(item > MAX_SECURE_STORAGE_MAX_ITEM){
		MMCINFO("item exceed %d\n",MAX_SECURE_STORAGE_MAX_ITEM);
		return -1;
	}

	if(blkcnt > SDMMC_ITEM_SIZE){
		MMCINFO("block count exceed %d\n",SDMMC_ITEM_SIZE);
		return -1;
	}
	/* first backups*/
	ret = mmc_bwrite(dev_num,SDMMC_SECURE_STORAGE_START_ADD+SDMMC_ITEM_SIZE*2*item,blkcnt,buf);
	if(ret != blkcnt){
		MMCINFO("Write first backup failed\n");
		return -1;
	}
	/*second backups*/
	ret = mmc_bwrite(dev_num,SDMMC_SECURE_STORAGE_START_ADD+SDMMC_ITEM_SIZE*2*item+SDMMC_ITEM_SIZE,blkcnt,buf);
	if(ret != blkcnt){
		MMCINFO("Write second backup failed\n");
		return -1;
	}
	return blkcnt;
}

static int
sdmmc_secure_storage_read(s32 dev_num,u32 item,u8 *buf , lbaint_t blkcnt)
{
	s32 ret = 0;
	s32 *fst_bak = NULL;
	s32 *sec_bak = NULL;

	if(buf == NULL){
		MMCINFO("intput buf is NULL\n");
		ret = -1;
		goto out;
	}

	if(item > MAX_SECURE_STORAGE_MAX_ITEM){
		MMCINFO("item exceed %d\n",MAX_SECURE_STORAGE_MAX_ITEM);
		ret = -1;
		goto out;
	}

	if(blkcnt > SDMMC_ITEM_SIZE){
		MMCINFO("block count exceed %d\n",SDMMC_ITEM_SIZE);
		ret = -1;
		goto out;
	}

	fst_bak = malloc(blkcnt*512);
	if(fst_bak == NULL){
		MMCINFO("malloc buff failed in fun %s line %d\n",__FUNCTION__,__LINE__);
		ret = -1;
		goto out;
	}
	sec_bak = malloc(blkcnt*512);
	if(sec_bak == NULL){
		MMCINFO("malloc buff failed in fun %s line %d\n",__FUNCTION__,__LINE__);
		ret = -1;
		goto out_fst;
	}

	/*first backups*/
	ret = mmc_bread(dev_num,SDMMC_SECURE_STORAGE_START_ADD+SDMMC_ITEM_SIZE*2*item,blkcnt,fst_bak);
	if(ret != blkcnt){
		MMCINFO("read first backup failed in fun %s line %d\n",__FUNCTION__,__LINE__);
		ret = -1;
		goto out_sec;
	}
	/*second backups*/
	ret = mmc_bread(dev_num,SDMMC_SECURE_STORAGE_START_ADD+SDMMC_ITEM_SIZE*2*item+SDMMC_ITEM_SIZE,blkcnt,sec_bak);
	if(ret != blkcnt){
		MMCINFO("read second backup failed fun %s line %d\n",__FUNCTION__,__LINE__);
		ret = -1;
		goto out_sec;
	}

	if(memcmp(fst_bak,sec_bak,blkcnt*512)){
		MMCINFO("first and second bak compare failed fun %s line %d\n",__FUNCTION__,__LINE__);
		ret = -1;
	}else{
		memcpy(buf,fst_bak,blkcnt*512);
		ret = blkcnt;
	}

out_sec:
	free(sec_bak);
out_fst:
	free(fst_bak);
out:
	return ret;
}
#endif


int mmc_init_blk_ops(struct mmc *mmc)
{

#ifndef CONFIG_SUNXI_SECURE_STORAGE
	mmc->block_dev.block_read 	= mmc_bread;
	mmc->block_dev.block_write 	= mmc_bwrite;
	mmc->block_dev.block_erase 	= mmc_berase;
	mmc->block_dev.block_read_mass_pro  = mmc_bread_mass_pro;
	mmc->block_dev.block_write_mass_pro = mmc_bwrite_mass_pro;

	mmc->block_dev.block_read_secure 	= NULL;
	mmc->block_dev.block_write_secure	= NULL;
	mmc->block_dev.block_get_item_secure= NULL;

	mmc->block_dev.block_secure_wipe = mmc_secure_wipe;
	mmc->block_dev.block_mmc_erase = mmc_mmc_erase;
	mmc->block_dev.block_mmc_trim = mmc_mmc_trim;
	mmc->block_dev.block_mmc_discard = mmc_mmc_discard;
	mmc->block_dev.block_mmc_sanitize = mmc_mmc_sanitize;
	mmc->block_dev.block_mmc_secure_erase = mmc_mmc_secure_erase;
	mmc->block_dev.block_mmc_secure_trim = mmc_mmc_secure_trim;
#else
	mmc->block_dev.block_read 	= mmc_bread_secure;
	mmc->block_dev.block_write 	= mmc_bwrite_secure;
	mmc->block_dev.block_erase 	= mmc_berase_secure;
	mmc->block_dev.block_read_mass_pro  = mmc_bread_mass_pro_secure;
	mmc->block_dev.block_write_mass_pro = mmc_bwrite_mass_pro_secure;

	mmc->block_dev.block_read_secure 	= sdmmc_secure_storage_read;
	mmc->block_dev.block_write_secure	= sdmmc_secure_storage_write;
	mmc->block_dev.block_get_item_secure= get_sdmmc_secure_storage_max_item;

	mmc->block_dev.block_secure_wipe    = mmc_secure_wipe_secure;
	mmc->block_dev.block_mmc_erase = mmc_mmc_erase_secure;
	mmc->block_dev.block_mmc_trim = mmc_mmc_trim_secure;
	mmc->block_dev.block_mmc_discard = mmc_mmc_discard_secure;
	mmc->block_dev.block_mmc_sanitize = mmc_mmc_sanitize_secure;
	mmc->block_dev.block_mmc_secure_erase = mmc_mmc_secure_erase_secure;
	mmc->block_dev.block_mmc_secure_trim = mmc_mmc_secure_trim_secure;
#endif


	return 0;
}


