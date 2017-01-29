/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 *
 * SPDX-License-Identifier:	GPL-2.0+
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


extern int mmc_send_ext_csd(struct mmc *mmc, char *ext_csd);
extern int mmc_decode_ext_csd(struct mmc *mmc,struct mmc_ext_csd *dec_ext_csd, char *ext_csd);
extern int mmc_do_switch(struct mmc *mmc, u8 set, u8 index, u8 value, u32 timeout);

static const unsigned int tacc_exp[] = {
	1,	10,	100,	1000,	10000,	100000,	1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};


#if 0
static ulong mmc_erase_t(struct mmc *mmc, ulong start, lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	ulong end;
	int err, start_cmd, end_cmd;

	if (mmc->high_capacity) {
		end = start + blkcnt - 1;
	} else {
		end = (start + blkcnt - 1) * mmc->write_bl_len;
		start *= mmc->write_bl_len;
	}

	if (IS_SD(mmc)) {
		start_cmd = SD_CMD_ERASE_WR_BLK_START;
		end_cmd = SD_CMD_ERASE_WR_BLK_END;
	} else {
		start_cmd = MMC_CMD_ERASE_GROUP_START;
		end_cmd = MMC_CMD_ERASE_GROUP_END;
	}

	cmd.cmdidx = start_cmd;
	cmd.cmdarg = start;
	cmd.resp_type = MMC_RSP_R1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = end_cmd;
	cmd.cmdarg = end;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = MMC_CMD_ERASE;
	cmd.cmdarg = SECURE_ERASE;  ///???? for sd
	cmd.resp_type = MMC_RSP_R1b;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	return 0;

err_out:
	puts("mmc erase failed\n");
	return err;
}

unsigned long mmc_berase(int dev_num, lbaint_t start, lbaint_t blkcnt)
{
	int err = 0;
	struct mmc *mmc = find_mmc_device(dev_num);
	lbaint_t blk = 0, blk_r = 0;
	int timeout = 1000;

	if (!mmc)
		return -1;

	if ((start % mmc->erase_grp_size) || (blkcnt % mmc->erase_grp_size))
		printf("\n\nCaution! Your devices Erase group is 0x%x\n"
		       "The erase range would be change to "
		       "0x" LBAF "~0x" LBAF "\n\n",
		       mmc->erase_grp_size, start & ~(mmc->erase_grp_size - 1),
		       ((start + blkcnt + mmc->erase_grp_size)
		       & ~(mmc->erase_grp_size - 1)) - 1);

	while (blk < blkcnt) {
		blk_r = ((blkcnt - blk) > mmc->erase_grp_size) ?
			mmc->erase_grp_size : (blkcnt - blk);
		err = mmc_erase_t(mmc, start + blk, blk_r);
		if (err)
			break;

		blk += blk_r;

		/* Waiting for the ready status */
		if (mmc_send_status(mmc, timeout))
			return 0;
	}

	return blk;
}
#endif


int mmc_set_erase_start_addr(struct mmc *mmc, unsigned int address)
{
	struct mmc_cmd cmd;
	int err = 0;
	int timeout = 300; //ms

	cmd.cmdidx = MMC_CMD_ERASE_GROUP_START;
	cmd.resp_type = MMC_RSP_R1;
	cmd.flags = 0;

	if (mmc->high_capacity)
		cmd.cmdarg = address;
	else
		cmd.cmdarg = address * mmc->write_bl_len;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		MMCINFO("%s: send erase start addr failed\n", __FUNCTION__);
		goto ERR_RET;
	}

	err = mmc_send_status(mmc, timeout); //ms

ERR_RET:
	return err;
}

int mmc_set_erase_end_addr(struct mmc *mmc, unsigned int address)
{
	struct mmc_cmd cmd;
	int err = 0;
	int timeout = 300; //ms


	cmd.cmdidx = MMC_CMD_ERASE_GROUP_END;
	cmd.resp_type = MMC_RSP_R1;
	cmd.flags = 0;

	if (mmc->high_capacity)
		cmd.cmdarg = address;
	else
		cmd.cmdarg = address * mmc->write_bl_len;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		MMCINFO("%s: send erase end addr failed\n", __FUNCTION__);
		goto ERR_RET;
	}

	err = mmc_send_status(mmc, timeout); //ms

ERR_RET:
	return err;
}

int mmc_launch_erase(struct mmc *mmc, unsigned int erase_arg)
{
	struct mmc_cmd cmd;

	cmd.cmdidx = MMC_CMD_ERASE;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = erase_arg;
	cmd.flags = 0;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

unsigned int mmc_sd_erase_timeout(struct mmc *mmc, unsigned int erase_arg,
	unsigned int qty)
{
	return 0xffffff;
}

/* calculate erase timeout based on CSD and current card clock frequency */
unsigned int mmc_mmc_def_erase_timeout(struct mmc *mmc)
{
	unsigned int erase_timeout = 0;
	unsigned int r2w_factor = (mmc->csd[3]>>26)&0x7;      //28:26
	unsigned int tacc_clks = ((mmc->csd[0]>>8)&0xFF)*100; //111:104
	unsigned int e = (mmc->csd[3]>>16)&0x7;
	unsigned int m = (mmc->csd[3]>>19)&0xF;
	unsigned int tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
	unsigned int mult = 10 << r2w_factor;

	unsigned int timeout_clks = tacc_clks * mult;
	unsigned int timeout_us;


	/* Avoid overflow: e.g. tacc_ns=80000000 mult=1280 */
	if (tacc_ns < 1000000)
		timeout_us = (tacc_ns * mult) / 1000;
	else
		timeout_us = (tacc_ns / 1000) * mult;

	/*
	 * ios.clock is only a target.  The real clock rate might be
	 * less but not that much less, so fudge it by multiplying by 2.
	 */
	timeout_clks <<= 1;
	timeout_us += (timeout_clks * 1000) / (mmc->clock / 1000);

	erase_timeout = timeout_us / 1000;

	/*
	 * Theoretically, the calculation could underflow so round up
	 * to 1ms in that case.
	 */
	if (!erase_timeout)
		erase_timeout = 1;

	return erase_timeout;
}

unsigned int mmc_mmc_update_timeout(struct mmc *mmc)
{
	int ret = 0;
	ALLOC_CACHE_ALIGN_BUFFER(char, ext_csd, MMC_MAX_BLOCK_LEN); //char ext_csd[512];
	struct mmc_ext_csd mmc_ext_csd;

	MMCDBG("+++%s\n", __FUNCTION__);

	ret = mmc_send_ext_csd(mmc, ext_csd);
	if (ret) {
		MMCINFO("send ext_csd failed\n");
		goto ERR_RET;
	}

	ret = mmc_decode_ext_csd(mmc, &mmc_ext_csd, ext_csd);
	if (ret) {
		MMCINFO("decode ext_csd failed\n");
		goto ERR_RET;
	}

	if (mmc_ext_csd.rev >= 4)
	{
		if (mmc_ext_csd.erase_group_def && mmc_ext_csd.hc_erase_timeout)
			mmc->erase_timeout = mmc_ext_csd.hc_erase_timeout;
		else
			mmc->erase_timeout = mmc_mmc_def_erase_timeout(mmc);

		mmc->trim_discard_timeout = mmc_ext_csd.trim_timeout;
		mmc->secure_erase_timeout = mmc->erase_timeout * mmc_ext_csd.sec_erase_mult;
		mmc->secure_trim_timeout  = mmc->erase_timeout * mmc_ext_csd.sec_trim_mult;
	}

ERR_RET:
	MMCDBG("---%s %d\n", __FUNCTION__, ret);
	return ret;
}

unsigned int mmc_mmc_erase_timeout(struct mmc *mmc, unsigned int arg,
	unsigned int qty)
{
	unsigned int erase_timeout = 0;

	if (arg == MMC_DISCARD_ARG || arg == MMC_TRIM_ARG)
		erase_timeout = mmc->trim_discard_timeout;
	else if (arg == MMC_ERASE_ARG)
		erase_timeout = mmc->erase_timeout;
	else if (arg == MMC_SECURE_ERASE_ARG)
		erase_timeout = mmc->secure_erase_timeout;
	else if (arg == MMC_SECURE_TRIM1_ARG || arg == MMC_SECURE_TRIM2_ARG)
		erase_timeout = mmc->secure_trim_timeout;
	else {
		MMCINFO("Unknown erase argument 0x%x\n", arg);
		goto ERR_RET;
	}

	erase_timeout *= qty;

	return erase_timeout;

ERR_RET:
	return 0;
}

unsigned int mmc_erase_timeout(struct mmc *mmc, unsigned int erase_arg,
	unsigned int qty)
{
	if (IS_SD(mmc))
		return mmc_sd_erase_timeout(mmc, erase_arg, qty);
	else {
		return mmc_mmc_erase_timeout(mmc, erase_arg, qty);
	}
}

int mmc_do_erase(struct mmc *mmc, unsigned int from,
	unsigned int to, unsigned int erase_arg)
{
	int err = 0;
	unsigned int timeout = 0;
	unsigned int qty = 0;

	MMCDBG("+++%s\n", __FUNCTION__);

	mmc_mmc_update_timeout(mmc);

	err = mmc_set_erase_start_addr(mmc, from);
	if (err) {
		MMCINFO("set erase start addr failed\n");
		goto ERR_RET;
	}

	err = mmc_set_erase_end_addr(mmc, to);
	if (err) {
		MMCINFO("set erase end addr failed\n");
		goto ERR_RET;
	}

	err = mmc_launch_erase(mmc, erase_arg);
	if (err) {
		MMCINFO("launch erase failed\n");
		goto ERR_RET;
	}

	if (IS_SD(mmc)) {
		qty = to - from + 1;
	} else {
		qty = (to - from)/mmc->erase_grp_size + 1;
	}
	timeout = mmc_erase_timeout(mmc, erase_arg, qty);
	if (!timeout) {
		MMCINFO("calculate timeout failed\n");
		err = -1;
		goto ERR_RET;
	}

	err = mmc_send_status(mmc, timeout); //ms

ERR_RET:

	MMCDBG("---%s %d\n", __FUNCTION__, err);
	return err;
}

int mmc_erase_group_aligned(struct mmc *mmc, unsigned int from,
	unsigned int nr)
{
	if (!mmc->erase_grp_size)
		return 0;
	if (from % mmc->erase_grp_size || nr % mmc->erase_grp_size)
		return 0;
	return 1;
}

void mmc_align_erase_group(struct mmc *mmc, unsigned int from,
	unsigned int nr, unsigned int *align_from, unsigned int *align_nr)
{
	unsigned int rem, start, cnt;

	MMCDBG("---start erase addr adjust... \n");
	MMCDBG("--1-- from: %d, nr: %d, erase_group: %d\n", from, nr, mmc->erase_grp_size);
	start = from;
	cnt = nr;

	rem = start % mmc->erase_grp_size;
	if (rem) {
		rem = mmc->erase_grp_size - rem;
		start += rem;
		if (cnt > rem)
			cnt -= rem;
		else {
			MMCINFO("after adjust start addr, no more space need to erase!!\n");
			goto RET;
		}
	}
	rem = cnt % mmc->erase_grp_size;
	if (rem)
		cnt -= rem;

	if (cnt == 0) {
		MMCINFO("after adjust nr, no more space need to erase!!\n");
	}

RET:
	MMCDBG("--2-- from: %d, nr: %d, erase_group: %d\n", start, cnt, mmc->erase_grp_size);
	*align_from = start;
	*align_nr = cnt;
	return ;
}

int mmc_erase(struct mmc *mmc, unsigned int from,
	unsigned int nr, unsigned int erase_arg)
{
	int ret;

	MMCDBG("+++%s\n", __FUNCTION__);

	if (nr == 0) {
		ret = 0;
		MMCINFO("No space need to be erased !\n");
		goto ERR_RET;
	}

	if (!IS_SD(mmc))
	{
		if ((erase_arg != MMC_ERASE_ARG)
			&& (erase_arg != MMC_SECURE_ERASE_ARG)
			&& (erase_arg != MMC_TRIM_ARG)
			&& (erase_arg != MMC_DISCARD_ARG)
 			&& (erase_arg != MMC_SECURE_TRIM1_ARG)) {
			ret = -1;
			MMCINFO("Unknown erase type!\n");
			goto ERR_RET;
		}

		if ((erase_arg == MMC_ERASE_ARG)
			||(erase_arg == MMC_SECURE_ERASE_ARG)) {
			ret = mmc_erase_group_aligned(mmc, from, nr);
			if (!ret) {
				ret = -1;
				MMCINFO("Erase addr is not erase group alignment!\n");
				goto ERR_RET;
			}
		}

		MMCINFO("erase from: %d, to: %d, cnt: %d, erase_group: %d\n",
			from, from+nr-1, nr, mmc->erase_grp_size);
		ret = mmc_do_erase(mmc, from, from+nr-1, erase_arg);
		if (ret) {
			ret = -1;
			MMCINFO("Do erase failed!\n");
			goto ERR_RET;
		}

		if (erase_arg == MMC_SECURE_TRIM1_ARG)
		{
			ret = mmc_do_erase(mmc, from, from+nr-1, MMC_SECURE_TRIM2_ARG);
			if (ret) {
				ret = -1;
				MMCINFO("Do secure trim step 2 failed!\n");
				goto ERR_RET;
			}
		}
	}
	else
	{
		MMCINFO("Don't support to erase SD card\n");
		ret = -1;
	}

ERR_RET:

	MMCDBG("--%s  ret%d\n", __FUNCTION__, ret);
	return ret;
}

#define MMC_SANITIZE_REQ_TIMEOUT 240000
int mmc_do_sanitize(struct mmc *mmc)
{
	int ret;

	MMCINFO("%s: start emmc sanitize...\n", __FUNCTION__);
	ret = mmc_do_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_SANITIZE_START, 1,
					MMC_SANITIZE_REQ_TIMEOUT);
	MMCINFO("%s end emmc sanitzie, ret %d\n", __FUNCTION__, ret);
	return ret;
}

void mmc_add_skip_space(unsigned int *skip_space, int index,
	unsigned int from, unsigned int nr)
{
	MMCDBG("%s: %d from %d  nr %d\n", __FUNCTION__, index, from, nr);
	skip_space[0] |= 0x1<<index;
	skip_space[index*2 + 1] = from;
	skip_space[index*2 + 2] = nr;
}

int mmc_insecure_secure_erase(struct mmc *mmc, unsigned int from,
	unsigned int nr, unsigned int *skip_space, int secure)
{
	int ret = 0;
	unsigned int align_from = from, align_nr = nr;
	int skip;
	unsigned int last_skip_len;
	unsigned arg;

	skip = 0;
	last_skip_len = 0;
	skip_space[0] = 0x0;

	MMCDBG("%d %s: start erase, seucre %d...\n", __LINE__, __FUNCTION__, secure);
	ret = mmc_erase_group_aligned(mmc, from, nr);
	if (!ret) {
		mmc_align_erase_group(mmc, from, nr, &align_from, &align_nr);
	}

	if (align_nr == 0)
	{
		MMCINFO("after align erase group, no space need to erase, erase failed\n");
		mmc_add_skip_space(skip_space, skip, from, nr);
		ret = -1;
	}
	else
	{
		if (secure)
			arg = MMC_SECURE_ERASE_ARG;
		else
			arg = MMC_ERASE_ARG;

		ret = mmc_erase(mmc, align_from, align_nr, arg);
		if (ret) {
			MMCINFO("erase failed, range %d - %d \n",
				align_from, (align_from+align_nr));
			mmc_add_skip_space(skip_space, skip, from, nr);
		} else {
			if (align_from - from)
				mmc_add_skip_space(skip_space, skip++, from, (align_from-from));

			if (skip)
				last_skip_len = skip_space[2];
			else
				last_skip_len = 0;
			if (nr - align_nr - last_skip_len)
				mmc_add_skip_space(skip_space, skip++, (align_from+align_nr), (nr-align_nr-last_skip_len));
		}
	}

	return ret;
}

int mmc_do_secure_wipe(struct mmc *mmc, unsigned int from, unsigned int nr,
	unsigned int *skip_space)
{
	int ret = 0;
	int skip, not_support_wipe = 0;


	MMCINFO("+++%s\n", __FUNCTION__);

	if (nr == 0) {
		MMCINFO("%s: on space need to erase, nr %d\n", __FUNCTION__, nr);
		return 0;
	}

	if (IS_SD(mmc)) {
		MMCINFO("%s: no mmc, do nothing\n", __FUNCTION__);
		ret = -2;
		goto ERR_RET;
	}

	skip = 0;
	skip_space[0] = 0x0;

	if (mmc->version <= MMC_VERSION_4_41) //ver 4.41 or older
	{
		if (mmc->secure_feature & EXT_CSD_SEC_ER_EN)
		{
			//support secure purge operation
			if (mmc->secure_feature & EXT_CSD_SEC_GB_CL_EN) {
				//support trim
				MMCDBG("%d %s: start secure trim...\n", __LINE__, __FUNCTION__);
				ret = mmc_erase(mmc, from, nr, MMC_SECURE_TRIM1_ARG);
				if (ret) {
					MMCINFO("secure trim failed, range %d - %d \n",
						from, (from+nr));
					mmc_add_skip_space(skip_space, skip, from, nr);
				}
			} else {
				ret = mmc_insecure_secure_erase(mmc, from, nr, skip_space, 1);
			}
		}
		else if (mmc->secure_feature & EXT_CSD_SEC_GB_CL_EN)
		{
			//support insecure trim operation
			MMCDBG("%d %s: start trim...\n", __LINE__, __FUNCTION__);
			ret = mmc_erase(mmc, from, nr, MMC_TRIM_ARG);
			if (ret) {
				MMCINFO("trim failed, range %d - %d \n", from, (from+nr));
				mmc_add_skip_space(skip_space, skip, from, nr);
			}
		}
		else
		{
			//is currently not an acceptable solution. writing of zeroes to the user data partition as a third option
			MMCINFO("no method to wipe data (emmc <= v4.41)!\n ");
			not_support_wipe = 1;
			mmc_add_skip_space(skip_space, skip, from, nr);
		}
	}
	else if (mmc->version <= MMC_VERSION_5_0) //v4.5 or newer
	{
		if (mmc->secure_feature & EXT_CSD_SEC_SANITIZE)
		{
			//support sanitize
			if (mmc->secure_feature & EXT_CSD_SEC_GB_CL_EN) {
				//support trim
				MMCDBG("%d %s: start trim...\n", __LINE__, __FUNCTION__);
				ret = mmc_erase(mmc, from, nr, MMC_TRIM_ARG);
				if (ret) {
					MMCINFO("trim failed, range %d - %d \n", from, (from+nr));
					mmc_add_skip_space(skip_space, skip, from, nr);
					goto ERR_RET;
				}
			} else {
				ret = mmc_insecure_secure_erase(mmc, from, nr, skip_space, 0);
				if (ret) {
					MMCINFO("erase failed, range %d - %d \n", from, (from+nr));
					goto ERR_RET;
				}
			}

			MMCDBG("%d %s: start sanitize...\n", __LINE__, __FUNCTION__);
			ret = mmc_do_sanitize(mmc);
			if (ret) {
				MMCINFO("do sanitize failed!!\n");
				skip = 0;
				skip_space[0] = 0x0;
				mmc_add_skip_space(skip_space, skip, from, nr);
			}
		}
		else
		{
			//If the eMMC 4.5 part does not expose the required command set, there is currently not an acceptable solution to sanitize this part for re-use
			not_support_wipe = 1;
			MMCINFO("no method to wipe data for current (emmc >v4.5)!\n ");
			mmc_add_skip_space(skip_space, skip, from, nr);
		}
	}
	else
	{
		MMCINFO("Unknown mmc version 0x%x\n", mmc->version);
		ret = -3;
	}

	if (not_support_wipe)
		ret = -2;

ERR_RET:

	MMCINFO("---%s ret %d\n", __FUNCTION__, ret);
	return ret;
}

int mmc_secure_wipe(int dev_num, unsigned int start,
	unsigned int blkcnt, unsigned int *skip_space)
{
	int ret = 0;
	struct mmc *mmc = find_mmc_device(dev_num);

	MMCINFO("========start %s\n", __FUNCTION__);
	if (mmc->cfg->platform_caps.drv_wipe_feature & DRV_PARA_DISABLE_SECURE_WIPE) {
		MMCINFO("driver do not support secure wipe operation\n");
		return -1;
	}

	ret = mmc_do_secure_wipe(mmc, start, blkcnt, skip_space);
	if (ret == -1) {
		MMCINFO("erase failed!!!!!!\n");
	} else if ((ret == -2) || (ret == -3)) {
		MMCINFO("do not erase!!!!!!\n");
		ret = -1;
	} else if (skip_space[0]){
		MMCINFO("skip some space when align erase group, need to write zeros.\n");
		ret = 1;
	} else {
		MMCINFO("erase ok\n");
		ret = 0;
	}

	MMCINFO("========end %s %d\n", __FUNCTION__, ret);
	return ret;
}

int mmc_mmc_erase(int dev_num, unsigned int start,
	unsigned int blkcnt, unsigned int *skip_space)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int i, ret1, ret = 0;

	MMCDBG("start %s ...\n", __FUNCTION__);
	if (IS_SD(mmc)) {
		MMCINFO("%s: sd card don't support erase\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (mmc->cfg->platform_caps.drv_erase_feature & DRV_PARA_DISABLE_EMMC_ERASE) {
		MMCINFO("%s: driver don't support erase\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (blkcnt == 0) {
		MMCINFO("%s: no space need to erase, from:%d nr:%d\n",
			__FUNCTION__, start, blkcnt);
		ret = 0;
		goto ERR_RET;
	}

	if ((start+blkcnt) > mmc->block_dev.lba) {
		MMCINFO("%s: input lenght error!!!\n", __FUNCTION__);
		blkcnt = mmc->block_dev.lba - start;
		MMCINFO("%s: after clip, from: %d, nr: %d\n",
			__FUNCTION__, start, blkcnt);
	}

	ret = mmc_insecure_secure_erase(mmc, start, blkcnt, skip_space, 0);
	if (ret) {
		MMCINFO("%s: erase emmc fail!\n", __FUNCTION__);
	}

	if (skip_space[0]) {
		ret = 1;

		MMCINFO("%s: some sectors in emmc are ignored!\n", __FUNCTION__);
		for (i=0; i<2; i++)
			if (skip_space[0] & (1<<i))
				MMCINFO("--%d: from%d  nr%d \n", i,
					(int)skip_space[i*2+1], (int)skip_space[i*2+2]);
	}

	if ((mmc->cfg->platform_caps.drv_erase_feature & DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE)
		&& (mmc->secure_feature & EXT_CSD_SEC_SANITIZE))
	{
		ret1 = mmc_do_sanitize(mmc);
		if (ret1) {
			MMCINFO("%s: emmc sanitize fail. ignore this error and continue...\n", __FUNCTION__);
		}
	}

ERR_RET:
	MMCDBG("end %s %d\n", __FUNCTION__, ret);
	return ret;
}

int mmc_mmc_sanitize(int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int ret = 0;

	MMCDBG("start %s ...\n", __FUNCTION__);
	if (IS_SD(mmc)) {
		MMCINFO("%s: sd card don't support erase\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (!(mmc->secure_feature & EXT_CSD_SEC_SANITIZE)) {
		MMCINFO("%s: driver don't support sanitize\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	ret = mmc_do_sanitize(mmc);
	if (ret) {
		ret = -1;
		MMCINFO("%s: sanitize fail!\n", __FUNCTION__);
	}

ERR_RET:
	MMCDBG("end %s %d\n", __FUNCTION__, ret);
	return ret;
}

int mmc_mmc_trim(int dev_num, unsigned int start, unsigned int blkcnt)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int ret = 0;

	MMCDBG("start %s ...\n", __FUNCTION__);
	if (IS_SD(mmc)) {
		MMCINFO("%s: sd card don't support trim\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (blkcnt == 0) {
		MMCINFO("%s: no space need to erase, from:%d nr:%d\n",
			__FUNCTION__, start, blkcnt);
		ret = 0;
		goto ERR_RET;
	}

	if ((start+blkcnt) > mmc->block_dev.lba) {
		MMCINFO("%s: input lenght error!!!\n", __FUNCTION__);
		blkcnt = mmc->block_dev.lba - start;
		MMCINFO("%s: after clip, from: %d, nr: %d\n",
			__FUNCTION__, start, blkcnt);
	}

	if (mmc->secure_feature & EXT_CSD_SEC_GB_CL_EN)
	{
		ret = mmc_erase(mmc, start, blkcnt, MMC_TRIM_ARG);
		if (ret) {
			MMCINFO("trim failed, range %d - %d\n",	start, (start+blkcnt));
		}
	} else {
		MMCINFO("%s: don't support trim!\n", __FUNCTION__);
		ret = -1;
	}

ERR_RET:
	MMCDBG("end %s %d\n", __FUNCTION__, ret);
	return ret;
}

int mmc_mmc_discard(int dev_num, unsigned int start, unsigned int blkcnt)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int ret = 0;

	MMCDBG("start %s ...\n", __FUNCTION__);
	if (IS_SD(mmc)) {
		MMCINFO("%s: sd card don't support discard\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (blkcnt == 0) {
		MMCINFO("%s: no space need to erase, from:%d nr:%d\n",
			__FUNCTION__, start, blkcnt);
		ret = 0;
		goto ERR_RET;
	}

	if ((start+blkcnt) > mmc->block_dev.lba) {
		MMCINFO("%s: input lenght error!!!\n", __FUNCTION__);
		blkcnt = mmc->block_dev.lba - start;
		MMCINFO("%s: after clip, from: %d, nr: %d\n",
			__FUNCTION__, start, blkcnt);
	}

	/* eMMC v4.5 or later */
	if (mmc->version >= MMC_VERSION_4_5)
	{
		ret = mmc_erase(mmc, start, blkcnt, MMC_DISCARD_ARG);
		if (ret) {
			MMCINFO("trim failed, range %d - %d\n",	start, (start+blkcnt));
		}
	} else {
		MMCINFO("%s: don't support discard!\n", __FUNCTION__);
		ret = -1;
	}

ERR_RET:
	MMCDBG("end %s %d\n", __FUNCTION__, ret);
	return ret;
}

int mmc_mmc_secure_erase(int dev_num, unsigned int start,
	unsigned int blkcnt, unsigned int *skip_space)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int i, ret = 0;

	MMCDBG("start %s ...\n", __FUNCTION__);
	if (IS_SD(mmc)) {
		MMCINFO("%s: sd card don't support secure erase!\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (blkcnt == 0) {
		MMCINFO("%s: no space need to erase, from:%d nr:%d\n",
			__FUNCTION__, start, blkcnt);
		ret = 0;
		goto ERR_RET;
	}

	if ((start+blkcnt) > mmc->block_dev.lba) {
		MMCINFO("%s: input lenght error!!!\n", __FUNCTION__);
		blkcnt = mmc->block_dev.lba - start;
		MMCINFO("%s: after clip, from: %d, nr: %d\n",
			__FUNCTION__, start, blkcnt);
	}

	if (!(mmc->secure_feature & EXT_CSD_SEC_ER_EN)) {
		MMCINFO("%s: don't support secure erase!\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	ret = mmc_insecure_secure_erase(mmc, start, blkcnt, skip_space,1);
	if (ret) {
		MMCINFO("%s: erase emmc fail!\n", __FUNCTION__);
	}

	if (skip_space[0]) {
		MMCDBG("%s: some sectors in emmc are ignored!\n\n", __FUNCTION__);
		for (i=0; i<2; i++)
			if (skip_space[0] & (1<<i))
				MMCDBG("--%d: from%d  nr%d \n",
					skip_space[i*2+1], skip_space[i*2+2]);
	}

ERR_RET:
	MMCDBG("end %s %d\n", __FUNCTION__, ret);
	return ret;
}

int mmc_mmc_secure_trim(int dev_num, unsigned int start, unsigned int blkcnt)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int ret = 0;

	MMCDBG("start %s ...\n", __FUNCTION__);
	if (IS_SD(mmc)) {
		MMCINFO("%s: sd card don't support secure trim!\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (blkcnt == 0) {
		MMCINFO("%s: no space need to erase, from:%d nr:%d\n",
			__FUNCTION__, start, blkcnt);
		ret = 0;
		goto ERR_RET;
	}

	if ((start+blkcnt) > mmc->block_dev.lba) {
		MMCINFO("%s: input lenght error!!!\n", __FUNCTION__);
		blkcnt = mmc->block_dev.lba - start;
		MMCINFO("%s: after clip, from: %d, nr: %d\n",
			__FUNCTION__, start, blkcnt);
	}

	if ((mmc->secure_feature & EXT_CSD_SEC_ER_EN)
		&& (mmc->secure_feature & EXT_CSD_SEC_GB_CL_EN))
	{
		ret = mmc_erase(mmc, start, blkcnt, MMC_SECURE_TRIM1_ARG);
		if (ret) {
			MMCINFO("secure trim failed, range %d - %d\n",	start, (start+blkcnt));
		}
	} else {
		MMCINFO("%s: don't support secure trim!\n", __FUNCTION__);
		ret = -1;
	}

ERR_RET:
	MMCDBG("end %s %d\n", __FUNCTION__, ret);
	return ret;
}

static ulong mmc_write_blocks(struct mmc *mmc, lbaint_t start,
		lbaint_t blkcnt, const void *src)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout = 1000;

	if ((start + blkcnt) > mmc->block_dev.lba) {
		MMCINFO("MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
		       start + blkcnt, mmc->block_dev.lba);
		return 0;
	}

	if (blkcnt == 0)
		return 0;
	else if (blkcnt == 1)
		cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->write_bl_len;

	cmd.resp_type = MMC_RSP_R1;

	data.src = src;
	data.blocks = blkcnt;
	data.blocksize = mmc->write_bl_len;
	data.flags = MMC_DATA_WRITE;

	if (mmc_send_cmd(mmc, &cmd, &data)) {
		MMCINFO("mmc write failed\n");
		return 0;
	}

	/* SPI multiblock writes terminate using a special
	 * token, not a STOP_TRANSMISSION request.
	 */
	if (!mmc_host_is_spi(mmc) && blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			MMCINFO("mmc fail to send stop cmd\n");
			return 0;
		}
	}

	/* Waiting for the ready status */
	if (mmc_send_status(mmc, timeout))
		return 0;

	return blkcnt;
}

ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt, const void *src)
{
	lbaint_t cur, blocks_todo = blkcnt;
	struct mmc *mmc = find_mmc_device(dev_num);

	if (blkcnt == 0) {
		MMCINFO("blkcnt should not be 0\n");
		return 0;
	}

	if (!mmc) {
		MMCINFO("can not found device\n");
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->write_bl_len))
		return 0;

	do {
		cur = (blocks_todo > mmc->cfg->b_max) ?
			mmc->cfg->b_max : blocks_todo;
		if (mmc_write_blocks(mmc, start, cur, src) != cur) {
			MMCINFO("write block failed\n");
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		src += cur * mmc->write_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

ulong mmc_berase(int dev_num, lbaint_t start, lbaint_t blkcnt)
{
	int err = 0;
	struct mmc *mmc = find_mmc_device(dev_num);
	void* src = NULL;

	src = malloc(blkcnt * mmc->write_bl_len);
	if (src == NULL){
		printf("%s: malloc failed\n", __FUNCTION__);
		return -1;
	}

	if (!mmc){
		MMCINFO("can not find mmc dev\n");
		free(src);
		return -1;
	}

	memset(src, 0, mmc->write_bl_len*blkcnt);
	MMCINFO("%s: blk %ld ~ %ld\n", __FUNCTION__, start, start + blkcnt - 1);
	err = mmc_bwrite(dev_num, start, blkcnt, src);
	if(!err){
		MMCINFO("%s: erase failed\n", __FUNCTION__);
	}

	free(src);
	return err;
}
