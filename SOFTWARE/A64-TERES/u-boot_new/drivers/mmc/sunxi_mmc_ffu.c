/*
 * MMC driver for allwinner sunxi platform.
 * Field Firmware Update(FFU)
 *
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <mmc.h>
#include <malloc.h>
#include <div64.h>
#include <sys_config.h>

#include <libfdt.h>
#include <fdt_support.h>

#include <private_toc.h>
#include <sunxi_board.h>

#include "sunxi_mmc.h"
#include "mmc_def.h"

#ifdef SUPPORT_SUNXI_MMC_FFU

extern int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value);
extern int mmc_send_ext_csd(struct mmc *mmc, char *ext_csd);
extern int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
extern int mmc_set_blocklen(struct mmc *mmc, int len);
extern int mmc_switch_ffu(struct mmc *mmc, u8 set, u8 index, u8 value, u32 timeout, u8 check_status);

struct ffu_caps {
	u32 fw_cfg;   //[169] FW_CONFIG
	u32 supported_mode;  //[493] SUPPORTED_MODES
	u32 ffu_feature; //[492] FFU_FEATURES
	u32 ffu_arg;
	u32 ffu_op_code_timeout_us; //[491] OPERATION_CODES_TIMEOUT
};

static int mmc_power_cycle(struct mmc *mmc)
{
	MMCINFO("--- start %s ----1\n", __FUNCTION__);
	__msdelay(1000);
	MMCINFO("--- start %s ----2\n", __FUNCTION__);
	sunxi_board_restart(0);
	return 0;
}

/*
* return
* 	1: support FFU
*      others: error
*/
int mmc_support_ffu(struct mmc *mmc)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, ext_csd, MMC_MAX_BLOCK_LEN);
	int ret = 1, err;
	u32 supported_mode = 0;
	u32 fw_cfg = 0;

	/* send ext_csd */
	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err) {
		MMCINFO("%s: mmc get ext csd failed\n", __FUNCTION__);
		return 0;
	}

	/* get [493] SUPPORTED_MODES, [169] FW_CONFIG */
	fw_cfg = ext_csd[EXT_CSD_FW_CONFIG];
	supported_mode = ext_csd[EXT_CSD_SUPPORTED_MODES];

	/* check */
	if (!(supported_mode & 0x1)) {
		MMCINFO("mmc don't support FFU mode\n");
		ret = 0;
	}

	if (fw_cfg & 0x1) {
		MMCINFO("FW updates disabled permanently\n");
		ret = 0;
	}

	return ret;
}

static int mmc_ffu_get_paras(struct mmc *mmc, struct ffu_caps *ffu_caps)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, ext_csd, MMC_MAX_BLOCK_LEN);
	int err = 0;
	u32 tmp = 0;

	/* send ext_csd */
	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err) {
		MMCINFO("mmc get ext csd failed\n");
		return err;
	}

	/* get [493] SUPPORTED_MODES, [169] FW_CONFIG,  [492] FFU_FEATURES, [490:487] FFU_ARG  */
	ffu_caps->fw_cfg = ext_csd[EXT_CSD_FW_CONFIG];
	ffu_caps->supported_mode = ext_csd[EXT_CSD_SUPPORTED_MODES];
	ffu_caps->ffu_feature = ext_csd[EXT_CSD_FFU_FEATURES];
	ffu_caps->ffu_arg = ext_csd[EXT_CSD_FFU_ARG] << 0
					| ext_csd[EXT_CSD_FFU_ARG + 1] << 8
					| ext_csd[EXT_CSD_FFU_ARG + 2] << 16
					| ext_csd[EXT_CSD_FFU_ARG + 3] << 24;

	tmp = ext_csd[EXT_CSD_OPERATION_CODES_TIMEOUT];
	if ((tmp > 0) && (tmp <0x18)) {
		ffu_caps->ffu_op_code_timeout_us = 100 * (1U<<tmp); //ms, 100us * (2^EXT_CSD_OPERATION_CODES_TIMEOUT)
	} else {
		MMCINFO("invalid mmc operation codes timeout, set timeout to 800s\n");
		ffu_caps->ffu_op_code_timeout_us = 800*1000*1000; //800s
	}

	return 0;
}

static int mmc_ffu_enter(struct mmc *mmc)
{
	int ret = 0;

	ret = mmc_switch_ffu(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_MODE_CONFIG, EXT_CSD_FFU_MODE, 1000, 1);
	if (ret){
		MMCINFO("enter FFU mode fail\n");
		return ret;
	}

	return 0;
}


static int mmc_ffu_download_firmware(struct mmc *mmc, struct ffu_caps *ffu_caps, const char *src, u32 blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	//int timeout = 1000;

	if (blkcnt == 0)
		return 0;
	else if (blkcnt == 1)
		cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

	/* the argument of ffu write is different from normal write */
	cmd.cmdarg = ffu_caps->ffu_arg;
	cmd.resp_type = MMC_RSP_R1;
	/* send cmd12 manually, auto send cmd12 may cause download fw error, like samsung's emmc */
	cmd.flags = MMC_CMD_MANUAL;

	data.src = src;
	data.blocks = blkcnt;
	data.blocksize = mmc->write_bl_len;
	data.flags = MMC_DATA_WRITE;

	if (mmc_send_cmd(mmc, &cmd, &data)) {
		MMCINFO("mmc downlaod fw failed\n");
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
	#if 0
	if (mmc_send_status(mmc, timeout))
		return 0;
	#endif

	return blkcnt;
}

static int mmc_ffu_install_firmware(struct mmc *mmc, struct ffu_caps *ffu_caps, u32 use_md_op_code)
{
	int ret = 0;
	u32 timeout = 0;

	/* If eMMC supports MODE_OPERATION_CODES, the host sets MODE_OPERATION_CODES
	* to FFU_INSTALL, eMMC shall set NUMBER_OF_FW_SECTORS_CORRECTLY_PROGRAMMED field to zero,
	* install the new firmware and set MODE_CONFIG to Normal state automaticially that would regain regular
	* operation of read and write commands.  It is not necessary to exit FFU mode manually.
	*/
	if (use_md_op_code)
	{
		timeout = ffu_caps->ffu_op_code_timeout_us / 1000; //ms
		ret = mmc_switch_ffu(mmc, EXT_CSD_CMD_SET_NORMAL, \
			EXT_CSD_MODE_OPERATION_CODES, EXT_CSD_FFU_INSTALL, timeout, 1);
	}
	else
	{
		/* switch back to Normal State */
		ret = mmc_switch_ffu(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_MODE_CONFIG, EXT_CSD_NORMAL_MODE, 1000, 1);
		if (ret){
			MMCINFO("exit FFU mode fail\n");
			return ret;
		}

		/* initiates a CMD0/HW_Reset/Power cycle to install the new firmware. */
		mmc_power_cycle(mmc);
	}

	return 0;
}


int mmc_ffu_check_sta(struct mmc *mmc, u32 *ext_csd_fw_ver)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, ext_csd, MMC_MAX_BLOCK_LEN);
	int ret = 0;
	u32 sta = 0;

	/* send ext_csd */
	ret = mmc_send_ext_csd(mmc, ext_csd);
	if (ret) {
		MMCINFO("mmc get ext csd failed\n");
		return ret;
	}

	//dumphex32("EXT_CSD", (char *)&ext_csd[0], 512);

	ext_csd_fw_ver[0] = ext_csd[EXT_CSD_FIRMWARE_VERSION] << 0
					| ext_csd[EXT_CSD_FIRMWARE_VERSION + 1] << 8
					| ext_csd[EXT_CSD_FIRMWARE_VERSION + 2] << 16
					| ext_csd[EXT_CSD_FIRMWARE_VERSION + 3] << 24;

	ext_csd_fw_ver[1] = ext_csd[EXT_CSD_FIRMWARE_VERSION + 4] << 0
					| ext_csd[EXT_CSD_FIRMWARE_VERSION + 5] << 8
					| ext_csd[EXT_CSD_FIRMWARE_VERSION + 6] << 16
					| ext_csd[EXT_CSD_FIRMWARE_VERSION + 7] << 24;

	MMCINFO("ext_csd fw ver: 0x%x 0x%x\n", ext_csd_fw_ver[0], ext_csd_fw_ver[1]);

	sta = ext_csd[EXT_CSD_FFU_STATUS];
	if (!sta) {
		MMCINFO("FFU success!\n");
	} else {
		if (sta == 0x10)
			MMCINFO("Gernal error!\n");
		else if (sta == 0x11)
			MMCINFO("Firmware install error!\n");
		else if (sta == 0x12)
			MMCINFO("Error in downloading firmware!\n");
		else
			MMCINFO("Reserved sta %x\n", sta);

		return -1;
	}

	return 0;
}

int mmc_ffu(struct mmc *mmc, const char *fw, u32 blkcnt)
{
	int ret = 0;
	struct ffu_caps ffu_caps;
	u32 use_md_op_code = 0;
	u32 cnt = 0;

	ret = mmc_ffu_get_paras(mmc, &ffu_caps);
	if (ret) {
		MMCINFO("get FFU para fail\n");
		return -1;
	}

	/* set block len to 512 bytes */
	if (mmc->write_bl_len != 512) {
		MMCINFO("force set write block to 512B, mmc->write_bl_len %d\n", mmc->write_bl_len);
	}
	if (mmc_set_blocklen(mmc, 512)) {
		MMCINFO("set block len fail\n");
		return -1;
	}

	ret = mmc_ffu_enter(mmc);
	if (ret) {
		MMCINFO("enter FFU mode fail\n");
		return -1;
	}
	udelay(2000);

	cnt = mmc_ffu_download_firmware(mmc, &ffu_caps, fw, blkcnt);
	if (cnt != blkcnt) {
		MMCINFO("FFU download fw fail\n");
		goto ERR_RET;
	}
	udelay(2000);

	ret = mmc_ffu_install_firmware(mmc, &ffu_caps, use_md_op_code);
	if (ret) {
		MMCINFO("FFU install fw fail\n");
		goto ERR_RET;
	}
	udelay(2000);

	return 0;

ERR_RET:
	//cmd0/hw reset/power cycle
	mmc_power_cycle(mmc);
	return -1;
}

int mmc_ffu_get_fw(struct mmc *mmc, char *fw, u32 fw_request_len, u32 *fw_real_len)
{
	int i;
	int len = 0;

	struct sbrom_toc1_head_info  *toc1_head = NULL;
	struct sbrom_toc1_item_info  *item_head = NULL;

	struct sbrom_toc1_item_info  *toc1_item = NULL;

	toc1_head = (struct sbrom_toc1_head_info *)CONFIG_BOOTPKG_STORE_IN_DRAM_BASE;
	item_head = (struct sbrom_toc1_item_info *)(CONFIG_BOOTPKG_STORE_IN_DRAM_BASE + sizeof(struct sbrom_toc1_head_info));

#ifdef BOOT_DEBUG
	printf("*******************TOC1 Head Message*************************\n");
	printf("Toc_name          = %s\n",   toc1_head->name);
	printf("Toc_magic         = 0x%x\n", toc1_head->magic);
	printf("Toc_add_sum           = 0x%x\n", toc1_head->add_sum);

	printf("Toc_serial_num    = 0x%x\n", toc1_head->serial_num);
	printf("Toc_status        = 0x%x\n", toc1_head->status);

	printf("Toc_items_nr      = 0x%x\n", toc1_head->items_nr);
	printf("Toc_valid_len     = 0x%x\n", toc1_head->valid_len);
	printf("TOC_MAIN_END      = 0x%x\n", toc1_head->end);
	printf("***************************************************************\n\n");
#endif

	//init
	toc1_item = item_head;
	for(i=0; i<toc1_head->items_nr; i++,toc1_item++)
	{
#ifdef BOOT_DEBUG
		printf("\n*******************TOC1 Item Message*************************\n");
		printf("Entry_name        = %s\n",   toc1_item->name);
		printf("Entry_data_offset = 0x%x\n", toc1_item->data_offset);
		printf("Entry_data_len    = 0x%x\n", toc1_item->data_len);

		printf("encrypt           = 0x%x\n", toc1_item->encrypt);
		printf("Entry_type        = 0x%x\n", toc1_item->type);
		printf("run_addr          = 0x%x\n", toc1_item->run_addr);
		printf("index             = 0x%x\n", toc1_item->index);
		printf("Entry_end         = 0x%x\n", toc1_item->end);
		printf("***************************************************************\n\n");
#endif

		if(strncmp(toc1_item->name, ITEM_EMMC_FW_NAME, sizeof(ITEM_EMMC_FW_NAME)) == 0)
		{
			//toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)CONFIG_SYS_TEXT_BASE);

			if (!fw_request_len) {
				len = (toc1_item->data_len+511)/512;
				if (!len) {
					MMCINFO("invalid fw length %d from package.\n", len*512 );
					return -1;
				}
			} else if (fw_request_len != 0xFFFFFFFF) {
				len = fw_request_len;
			} else {
				MMCINFO("invalid request fw length %d ---2 \n", fw_request_len);
				return -1;
			}

			memcpy(fw, (void *)(CONFIG_BOOTPKG_STORE_IN_DRAM_BASE + toc1_item->data_offset), len*512);
			MMCINFO("fw len: %d sector\n", len);
			break;
		}
	}

	if (i == toc1_head->items_nr) {
		MMCINFO("get emmc fw from toc0 fail\n");
		return -1;
	}

	*fw_real_len = len*512;

	return 0;
}

int sunxi_mmc_ffu(struct mmc *mmc)
{
	int err = 0;
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;
	u32 fw_ver[2] = {0};
	ALLOC_CACHE_ALIGN_BUFFER(char, fw, 512*1024);
	u32 len = 0;
	u8 prv = 0, mid = 0;

	if (!host->cfg.platform_caps.enable_ffu) {
		MMCINFO("don't enable ffu\n");
		goto OUT;
	}

	err = mmc_ffu_check_sta(mmc, &fw_ver[0]);
	MMCINFO("EXT_CSD FW VER: 0x%x 0x%x\n", fw_ver[0], fw_ver[1]);

	if (!IS_SD(mmc)) {
		mid = ((mmc->cid[0] >> 24) & 0xff);
		prv = ((mmc->cid[2] >> 16) & 0xff);
		MMCINFO("check emmc mid: 0x%x, prv: 0x%x\n", mid, prv);
		MMCINFO("cid: 0x%x, 0x%x 0x%x, 0x%x\n", mmc->cid[0], mmc->cid[1], mmc->cid[2], mmc->cid[3]);
	}

#if 0
	/* use emmc mid prv */
	if (prv == 0x26) {
		MMCINFO("old fw, need to update fw...\n");
	} else {
		MMCINFO("----------- new fw ---------\n");
		goto OUT;
	}
#else
	if ((fw_ver[0] == host->cfg.platform_caps.emmc_fw_ver0)
		&& (fw_ver[1] == host->cfg.platform_caps.emmc_fw_ver1)) {
		MMCINFO("old ext_csd fw ver, need to update fw...\n");
	} else {
		MMCINFO("----------- new ext_csd fw ver ---------\n");
		goto OUT;
	}
#endif

	if (mmc_ffu_get_fw(mmc, fw, host->cfg.platform_caps.emmc_fw_byte_len, &len)) {
		MMCINFO("get emmc fw fail\n");
		goto OUT;
	}

	if (mmc_support_ffu(mmc))
	{
		err = mmc_ffu(mmc, (const char *)fw, len>>9);
	}
	else
	{
		MMCINFO("don't support FFU\n");
	}

	err = mmc_ffu_check_sta(mmc, &fw_ver[0]);

OUT:
	return err;
}

#endif //SUPPORT_SUNXI_MMC_FFU