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
#include "mmc_def.h"
#include "mmc_test.h"

#ifdef MMC_INTERNAL_TEST

/*
	write, read and compare.
*/
int mmc_t_rwc(struct mmc *mmc, ulong start, ulong blkcnt)
{
	ulong blk;
	u32 *src=NULL, *dst=NULL;
	int i, ret=0;

	MMCINFO("%s: start test .......\n", __FUNCTION__);

	src = (unsigned int *)malloc(512*blkcnt);
	if (src == NULL) {
		MMCINFO("%s: malloc error!\n", __FUNCTION__);
		return -1;
	}
	else
		MMCINFO("%s: src 0x%x!\n", __FUNCTION__, src);

	dst = (unsigned int *)malloc(512*blkcnt);
	if (src == NULL) {
		MMCINFO("%s: malloc error!\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET1;
	}
	else
		MMCINFO("%s: dst 0x%x!\n", __FUNCTION__, dst);

	for (i=0; i<blkcnt*512>>2; i++)
	{
		src[i] = i;
		dst[i] = 0;
	}

	if ((mmc->block_dev.block_write == NULL)
		|| (mmc->block_dev.block_read == NULL)) {
		MMCINFO("%s: write and/or read func is null\n", __FUNCTION__);
	}

	blk = mmc->block_dev.block_write(mmc->block_dev.dev, start, blkcnt, src);
	if (blk != blkcnt) {
		MMCINFO("%s: write block fail\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	blk = mmc->block_dev.block_read(mmc->block_dev.dev, start, blkcnt, dst);
	if (blk != blkcnt) {
		MMCINFO("%s: read block fail\n", __FUNCTION__);
		ret = -1;
		goto ERR_RET;
	}

	if (memcmp(src, dst, blkcnt*512) != 0) {
		MMCINFO("%s: memory compare fail\n", __FUNCTION__);
		ret = -1;
	}

	for (i=0; i<100; i+=10)
	{
		MMCINFO("%d: %d %d\n", i, src[i], dst[i]);
	}

ERR_RET:
	free(dst);

ERR_RET1:
	free(src);

	MMCINFO("%s: test end......%s\n", __FUNCTION__, ret?"FAIL!":"OK");
	return ret;
}

int mmc_t_emmc_erase(struct mmc *mmc, ulong start, ulong blkcnt)
{
	return 0;
}

int mmc_t_emmc_trim(struct mmc *mmc, ulong start, ulong blkcnt)
{
	return 0;
}

int mmc_t_emmc_discard(struct mmc *mmc, ulong start, ulong blkcnt)
{
	return 0;
}

int mmc_t_emmc_secure_erase(struct mmc *mmc, ulong start, ulong blkcnt)
{
	return 0;
}

int mmc_t_emmc_secure_trim(struct mmc *mmc, ulong start, ulong blkcnt)
{
	return 0;
}

int mmc_t_emmc_sanitize(struct mmc *mmc)
{
	return 0;
}


#endif /*MMC_INTERNAL_TEST*/

