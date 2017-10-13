/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: MMC driver for General mmc operations
 * Author: Aaron <leafy.myeh@allwinnertech.com>
 * Date: 2012-2-3 14:18:18
 */
#include "mmc_def.h"
#include "mmc_bsp.h"
#include "mmc.h"
#include <private_boot0.h>

/* Set block count limit because of 16 bit register limit on some hardware*/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

extern const boot0_file_head_t BT0_head;
static struct mmc* mmc_devices[MAX_MMC_NUM];

int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	return mmc->send_cmd(mmc, cmd, data);
}

int mmc_send_status(struct mmc *mmc, int timeout)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;
	cmd.flags = 0;

	do {
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			mmcinfo("mmc %d Send status failed\n",mmc->control_num);
			return err;
		}
		else if (cmd.response[0] & MMC_STATUS_RDY_FOR_DATA)
			break;

		__msdelay(1);

		if (cmd.response[0] & MMC_STATUS_MASK) {
			mmcinfo("mmc %d Status Error: 0x%08X\n",mmc->control_num, cmd.response[0]);
			return COMM_ERR;
		}
	} while (timeout--);

	if (!timeout) {
		mmcinfo("mmc %d Timeout waiting card ready\n",mmc->control_num);
		return TIMEOUT;
	}

	return 0;
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;

	/* don't set blocklen at ddr mode */
	if ((mmc->speed_mode == HSDDR52_DDR50) || (mmc->speed_mode == HS400)) {
		return 0;
	}

	cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = len;
	cmd.flags = 0;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

struct mmc *find_mmc_device(int dev_num)
{
	if (mmc_devices[dev_num] != NULL)
		return mmc_devices[dev_num];

	mmcinfo("MMC Device %d not found\n", dev_num);

	return NULL;
}
#if 0
static unsigned long mmc_erase_t(struct mmc *mmc, unsigned long start, unsigned blkcnt)
{
	struct mmc_cmd cmd;
	unsigned long end;
	int err, start_cmd, end_cmd;

	if (mmc->high_capacity)
		end = start + blkcnt - 1;
	else {
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
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = end_cmd;
	cmd.cmdarg = end;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = MMC_CMD_ERASE;
	cmd.cmdarg = SECURE_ERASE;
	cmd.resp_type = MMC_RSP_R1b;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	return 0;

err_out:
	mmcdbg("mmc erase failed\n");
	return err;
}
#endif
#if 0
int
mmc_berase(int dev_num, unsigned long start, unsigned blkcnt)
{
	int err = 0;
	struct mmc *mmc = find_mmc_device(dev_num);
//	unsigned blk = 0, blk_r = 0;
	void* src = (void*)0x41000000;

	if (!mmc){
		mmcinfo("Can not find mmc dev %d\n",dev_num);
		return -1;
	}

	memset(src, 0, 512*blkcnt);
	mmcinfo("mmc %d erase blk %d ~ %d\n",mmc->control_num, start, start + blkcnt - 1);
	err = mmc_bwrite(dev_num, start, blkcnt, src);
	//mmcinfo("erase flag%d",err);
	if(!err)
	{
		mmcinfo("mmc %d erase failed\n",mmc->control_num);
	}

	return err;
	/*
	if ((start % mmc->erase_grp_size) || (blkcnt % mmc->erase_grp_size))
		mmcdbg("\n\nCaution! Your devices Erase group is 0x%x\n"
			"The erase range would be change to 0x%x~0x%x\n\n",
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
	}
	return blk;
	*/
}

static unsigned long
mmc_write_blocks(struct mmc *mmc, unsigned long start, unsigned blkcnt, const void*src)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout = 1000;

	if ((start + blkcnt) > mmc->lba) {
		mmcinfo("mmc %d: block number 0x%lx exceeds max(0x%lx)\n",mmc->control_num,
			start + blkcnt, mmc->lba);
		return 0;
	}

	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->write_bl_len;

	cmd.resp_type = MMC_RSP_R1;
	cmd.flags = 0;

	data.b.src = src;
	data.blocks = blkcnt;
	data.blocksize = mmc->write_bl_len;
	data.flags = MMC_DATA_WRITE;

	if (mmc_send_cmd(mmc, &cmd, &data)) {
		mmcinfo("mmc %d mmc write failed\n",mmc->control_num);
		return 0;
	}

	/* SPI multiblock writes terminate using a special
	 * token, not a STOP_TRANSMISSION request.
	 */
	if (!mmc_host_is_spi(mmc) && blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		cmd.flags = 0;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			mmcinfo("mmc %d fail to send stop cmd\n",mmc->control_num);
			return 0;
		}


	}

	/* Waiting for the ready status */
	mmc_send_status(mmc, timeout);
	return blkcnt;
}

unsigned long
mmc_bwrite(int dev_num, unsigned long start, unsigned blkcnt, const void*src)
{
	unsigned cur, blocks_todo = blkcnt;

	struct mmc *mmc = find_mmc_device(dev_num);

	if (blkcnt == 0){
		mmcinfo("mmc %d blkcnt should not be 0\n",dev_num);
		return 0;
	}
	if (!mmc){
		mmcinfo("Can not found device %d\n",dev_num);
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->write_bl_len)){
		mmcinfo("mmc %d set block len failed\n",mmc->control_num);
//		sunxi_mmc_exit(dev_num);
//		if(sunxi_mmc_init(dev_num,4)<0){
//			mmcinfo("re init failed\n");
//			return 0;
//		}
		return 0;
	}

	do {
		cur = (blocks_todo > mmc->b_max) ?  mmc->b_max : blocks_todo;
		if(mmc_write_blocks(mmc, start, cur, src) != cur){
			mmcinfo("mmc %d write block failed\n",mmc->control_num);
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
//		src += cur * mmc->write_bl_len;
		src = (char*)src + cur * mmc->write_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}
#endif

int mmc_read_blocks(struct mmc *mmc, void *dst, unsigned long start, unsigned blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout = 1000;

	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->read_bl_len;

	cmd.resp_type = MMC_RSP_R1;
	cmd.flags = 0;

	data.b.dest = dst;
	data.blocks = blkcnt;
	data.blocksize = mmc->read_bl_len;
	data.flags = MMC_DATA_READ;

	if (mmc_send_cmd(mmc, &cmd, &data)){
		mmcinfo("mmc %d  read blcok failed\n",mmc->control_num);
		return 0;
	}

	if (blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		cmd.flags = 0;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			mmcinfo("mmc %d fail to send stop cmd\n",mmc->control_num);
			return 0;
		}

		/* Waiting for the ready status */
		mmc_send_status(mmc, timeout);
	}

	return blkcnt;
}

unsigned long
mmc_bread(int dev_num, unsigned long start, unsigned blkcnt, void *dst)
{
	unsigned cur, blocks_todo = blkcnt;
	struct mmc *mmc = find_mmc_device(dev_num);

	if (blkcnt == 0){
		mmcinfo("mmc %d blkcnt should not be 0\n",mmc->control_num);
		return 0;
	}
	if (!mmc){
		mmcinfo("Can not find mmc dev %d\n",dev_num);
		return 0;
	}

	if ((start + blkcnt) > mmc->lba) {
		mmcinfo("mmc %d: block number 0x%x exceeds max(0x%x)\n",mmc->control_num,
			(unsigned int)(start + blkcnt), (unsigned int)mmc->lba);
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->read_bl_len)){
		mmcinfo("mmc %d Set block len failed\n",mmc->control_num);
		return 0;
	}

	do {
		cur = (blocks_todo > mmc->b_max) ?  mmc->b_max : blocks_todo;
		if(mmc_read_blocks(mmc, dst, start, cur) != cur){
			mmcinfo("mmc %d block read failed\n",mmc->control_num);
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
//		dst += cur * mmc->read_bl_len;
		dst = (char*)dst + cur * mmc->read_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

int mmc_go_idle(struct mmc* mmc)
{
	struct mmc_cmd cmd;
	int err;

	__msdelay(1);

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err){
		mmcinfo("mmc %d go idle failed\n",mmc->control_num);
		return err;
	}

	__msdelay(2);

	return 0;
}

int
sd_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	do {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;
		cmd.flags = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err){
			mmcinfo("mmc %d send app cmd failed\n",mmc->control_num);
			return err;
		}

		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;

		/*
		 * Most cards do not answer if some reserved bits
		 * in the ocr are set. However, Some controller
		 * can set bit 7 (reserved for low voltages), but
		 * how to manage low voltages SD card is not yet
		 * specified.
		 */
		cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 :
			(mmc->voltages & 0xff8000);

		if (mmc->version == SD_VERSION_2)
			cmd.cmdarg |= OCR_HCS;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err){
			mmcinfo("mmc %d send cmd41 failed\n",mmc->control_num);
			return err;
		}

		__msdelay(1);
	} while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

	if (timeout <= 0){
		mmcinfo("mmc %d wait card init failed\n",mmc->control_num);
		return UNUSABLE_ERR;
	}

	if (mmc->version != SD_VERSION_2)
		mmc->version = SD_VERSION_1_0;

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;
		cmd.flags = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err){
			mmcinfo("mmc %d spi read ocr failed\n",mmc->control_num);
			return err;
		}
	}

	mmc->ocr = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 0;

	return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
	int timeout = 10000;
	struct mmc_cmd cmd;
	int err;

	/* Some cards seem to need this */
	mmc_go_idle(mmc);

 	/* Asking to the card its capabilities */
 	cmd.cmdidx = MMC_CMD_SEND_OP_COND;
 	cmd.resp_type = MMC_RSP_R3;
 	cmd.cmdarg = 0x40ff8000;//foresee
 	cmd.flags = 0;

  //mmcinfo("mmc send op cond arg not zero !!!\n");
 	err = mmc_send_cmd(mmc, &cmd, NULL);

 	if (err){
 		mmcinfo("mmc %d send op cond failed\n",mmc->control_num);
 		return err;
 	}

 	__msdelay(1);

	do {
		cmd.cmdidx = MMC_CMD_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = (mmc_host_is_spi(mmc) ? 0 :
				(mmc->voltages &
				(cmd.response[0] & OCR_VOLTAGE_MASK)) |
				(cmd.response[0] & OCR_ACCESS_MODE));

		if (mmc->host_caps & MMC_MODE_HC)
			cmd.cmdarg |= OCR_HCS;

		cmd.flags = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err){
			mmcinfo("mmc %d send op cond failed\n",mmc->control_num);
			return err;
		}

		__msdelay(1);
	} while (!(cmd.response[0] & OCR_BUSY) && timeout--);

	if (timeout <= 0){
		mmcinfo("mmc %d wait for mmc init failed\n",mmc->control_num);
		return UNUSABLE_ERR;
	}

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;
		cmd.flags = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err)
			return err;
	}

	mmc->version = MMC_VERSION_UNKNOWN;
	mmc->ocr = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 1;

	return 0;
}


int mmc_send_ext_csd(struct mmc *mmc, char *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	/* Get the Card Status Register */
	cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;
	cmd.flags = 0;

	data.b.dest = ext_csd;
	data.blocks = 1;
	data.blocksize = 512;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);
	if(err)
		mmcinfo("mmc %d send ext csd failed\n",mmc->control_num);

	return err;
}

int mmc_update_phase(struct mmc *mmc)
{
	return mmc->update_phase(mmc);
}

int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	int ret;

	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				 (index << 16) |
				 (value << 8);
	cmd.flags = 0;

	ret = mmc_send_cmd(mmc, &cmd, NULL);
	if(ret){
		mmcinfo("mmc %d switch failed\n",mmc->control_num);
	}

	/* for re-update sample phase */
	ret = mmc_update_phase(mmc);
	if (ret) {
		mmcinfo("mmc_switch: update clock failed after send cmd6\n");
		return ret;
	}

	/* Waiting for the ready status */
	mmc_send_status(mmc, timeout);

	return ret;

}

int mmc_change_freq(struct mmc *mmc)
{
	char ext_csd[512];
	char cardtype;
	int err;
	int retry = 5;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Only version 4 supports high-speed */
	if (mmc->version < MMC_VERSION_4)
		return 0;

	mmc->card_caps |= MMC_MODE_4BIT|MMC_MODE_8BIT;

	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err){
		mmcinfo("mmc %d get ext csd failed\n",mmc->control_num);
		return err;
	}

	cardtype = ext_csd[196] & 0xff;

	//retry for Toshiba emmc,for the first time Toshiba emmc change to HS
	//it will return response crc err,so retry
	do{
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);
		if(!err){
			break;
		}
		mmcinfo("retry mmc switch(cmd6)\n");
	}while(retry--);

	if (err){
		mmcinfo("mmc %d change to hs failed\n",mmc->control_num);
		return err;
	}

	/* Now check to see that it worked */
	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err){
		mmcinfo("mmc %d send ext csd faild\n",mmc->control_num);
		return err;
	}

	/* No high-speed support */
	if (!ext_csd[185])
		return 0;

	/* High Speed is set, there are two types: 52MHz and 26MHz */
	if (cardtype & EXT_CSD_CARD_TYPE_HS) { //EXT_CSD_CARD_TYPE_52
		if (cardtype & EXT_CSD_CARD_TYPE_DDR_52) {
			mmcdbg("%s: get ddr OK!\n", __FUNCTION__);
			mmc->card_caps |= MMC_MODE_DDR_52MHz;
			mmc->speed_mode = HSDDR52_DDR50;
		} else
			mmc->speed_mode = HSSDR52_SDR25;
		mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	} else {
		mmc->card_caps |= MMC_MODE_HS;
		mmc->speed_mode = DS26_SDR12;
	}

	return 0;
}

int mmc_switch_part(int dev_num, unsigned int part_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);

	if (!mmc)
		return -1;

	return mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			  (mmc->part_config & ~PART_ACCESS_MASK)
			  | (part_num & PART_ACCESS_MASK));
}

int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	/* Switch the frequency */
	cmd.cmdidx = SD_CMD_SWITCH_FUNC;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = (mode << 31) | 0xffffff;
	cmd.cmdarg &= ~(0xf << (group * 4));
	cmd.cmdarg |= value << (group * 4);
	cmd.flags = 0;

	data.b.dest = (char *)resp;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	return mmc_send_cmd(mmc, &cmd, &data);
}


int sd_change_freq(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	u32 scr[2];
	u32 switch_status[16];
	struct mmc_data data;
	int timeout;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Read the SCR to find out if this card supports higher speeds */
	cmd.cmdidx = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err){
		mmcinfo("mmc %d Send app cmd failed\n",mmc->control_num);
		return err;
	}

	cmd.cmdidx = SD_CMD_APP_SEND_SCR;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;
	cmd.flags = 0;

	timeout = 3;

retry_scr:
	data.b.dest = (char *)&scr;
	data.blocksize = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	if (err) {
		if (timeout--)
			goto retry_scr;

		mmcinfo("mmc %d Send scr failed\n",mmc->control_num);
		return err;
	}

	mmc->scr[0] = __mmc_be32_to_cpu(scr[0]);
	mmc->scr[1] = __mmc_be32_to_cpu(scr[1]);

	switch ((mmc->scr[0] >> 24) & 0xf) {
		case 0:
			mmc->version = SD_VERSION_1_0;
			break;
		case 1:
			mmc->version = SD_VERSION_1_10;
			break;
		case 2:
			mmc->version = SD_VERSION_2;
			break;
		default:
			mmc->version = SD_VERSION_1_0;
			break;
	}

	if (mmc->scr[0] & SD_DATA_4BIT)
		mmc->card_caps |= MMC_MODE_4BIT;

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0)
		return 0;

	timeout = 4;
	while (timeout--) {
		err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
				(u8 *)&switch_status);

		if (err){
			mmcinfo("mmc %d Check high speed status faild\n",mmc->control_num);
			return err;
		}

		/* The high-speed function is busy.  Try again */
		if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
			break;
	}

	/* If high-speed isn't supported, we return */
	if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
		return 0;

	err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *)&switch_status);

	if (err){
		mmcinfo("mmc %d switch to high speed failed\n",mmc->control_num);
		return err;
	}

	err = mmc_update_phase(mmc);
	if (err) {
		mmcinfo("update clock failed after send cmd6 to switch to sd high speed mode\n");
		return err;
	}

	if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000) {
		mmc->card_caps |= MMC_MODE_HS;
		mmc->speed_mode = HSSDR52_SDR25;
	}

	return 0;
}

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
	10000,
	100000,
	1000000,
	10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multipliers[] = {
	0,	/* reserved */
	10,
	12,
	13,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
	60,
	70,
	80,
};

void mmc_set_ios(struct mmc *mmc)
{
	mmc->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, u32 clock)
{
	if (clock > mmc->f_max)
		clock = mmc->f_max;

	if (clock < mmc->f_min)
		clock = mmc->f_min;

	mmc->clock = clock;

	mmc_set_ios(mmc);
}

#if 0
void mmc_set_bus_mode(struct mmc *mmc, u32 ddr)
{
	mmc->io_mode= ddr;
	mmc_set_ios(mmc);
}
#endif

void mmc_set_bus_width(struct mmc *mmc, u32 width)
{
	mmc->bus_width = width;

	mmc_set_ios(mmc);
}

int mmc_startup(struct mmc *mmc)
{
	int err;
	u32 mult, freq;
	u64 cmult, csize, capacity;
	struct mmc_cmd cmd;
	char ext_csd[512];
	int timeout = 1000;
	char *spd_name[] = {"DS26/SDR12", "HSSDR52/SDR25", "HSDDR52/DDR50", "HS200/SDR104", "HS400"};

	/* Put the Card in Identify Mode */
	cmd.cmdidx = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID :
		MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = 0;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err){
		mmcinfo("mmc %d Put the Card in Identify Mode failed\n",mmc->control_num);
		return err;
	}

	memcpy(mmc->cid, cmd.response, 16);

	/*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 * This also puts the cards into Standby State
	 */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
		cmd.cmdarg = mmc->rca << 16;
		cmd.resp_type = MMC_RSP_R6;
		cmd.flags = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err){
			mmcinfo("mmc %d send rca failed\n",mmc->control_num);
			return err;
		}

		if (IS_SD(mmc))
			mmc->rca = (cmd.response[0] >> 16) & 0xffff;
	}

	/* Get the Card-Specific Data */
	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = mmc->rca << 16;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	/* Waiting for the ready status */
	mmc_send_status(mmc, timeout);

	if (err){
		mmcinfo("mmc %d get csd failed\n",mmc->control_num);
		return err;
	}

	mmc->csd[0] = cmd.response[0];
	mmc->csd[1] = cmd.response[1];
	mmc->csd[2] = cmd.response[2];
	mmc->csd[3] = cmd.response[3];

	if (mmc->version == MMC_VERSION_UNKNOWN) {
		int version = (cmd.response[0] >> 26) & 0xf;

		switch (version) {
			case 0:
				mmc->version = MMC_VERSION_1_2;
				break;
			case 1:
				mmc->version = MMC_VERSION_1_4;
				break;
			case 2:
				mmc->version = MMC_VERSION_2_2;
				break;
			case 3:
				mmc->version = MMC_VERSION_3;
				break;
			case 4:
				mmc->version = MMC_VERSION_4;
				break;
			default:
				mmc->version = MMC_VERSION_1_2;
				break;
		}
	}

	/* divide frequency by 10, since the mults are 10x bigger */
	freq = fbase[(cmd.response[0] & 0x7)];
	mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

	mmc->tran_speed = freq * mult;

	mmc->read_bl_len = 1 << ((cmd.response[1] >> 16) & 0xf);

	if (IS_SD(mmc))
		mmc->write_bl_len = mmc->read_bl_len;
	else
		mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);

	if (mmc->high_capacity) {
		csize = (mmc->csd[1] & 0x3f) << 16
			| (mmc->csd[2] & 0xffff0000) >> 16;
		cmult = 8;
	} else {
		csize = (mmc->csd[1] & 0x3ff) << 2
			| (mmc->csd[2] & 0xc0000000) >> 30;
		cmult = (mmc->csd[2] & 0x00038000) >> 15;
	}

	mmc->capacity = (csize + 1) << (cmult + 2);
	mmc->capacity *= mmc->read_bl_len;

	if (mmc->read_bl_len > 512)
		mmc->read_bl_len = 512;

	if (mmc->write_bl_len > 512)
		mmc->write_bl_len = 512;

	mmc_set_clock(mmc, 25000000);

	/* Select the card, and put it into Transfer Mode */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = MMC_CMD_SELECT_CARD;
		cmd.resp_type = MMC_RSP_R1b;
		cmd.cmdarg = mmc->rca << 16;
		cmd.flags = 0;
		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err){
			mmcinfo("Select the card failed\n");
			return err;
		}
	}

	/*
	 * For SD, its erase group is always one sector
	 */
	mmc->erase_grp_size = 1;
	mmc->part_config = MMCPART_NOAVAILABLE;
	if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4)) {
		/* check  ext_csd version and capacity */
		err = mmc_send_ext_csd(mmc, ext_csd);

		if(!err){
				/* update mmc version */
			switch (ext_csd[192]) {
				case 0:
					mmc->version = MMC_VERSION_4;
					break;
				case 1:
					mmc->version = MMC_VERSION_4_1;
					break;
				case 2:
					mmc->version = MMC_VERSION_4_2;
					break;
				case 3:
					mmc->version = MMC_VERSION_4_3;
					break;
				case 5:
					mmc->version = MMC_VERSION_4_41;
					break;
				case 6:
					mmc->version = MMC_VERSION_4_5;
					break;
				case 7:
					mmc->version = MMC_VERSION_5_0;
					break;
				case 8:
					mmc->version = MMC_VERSION_5_1;
					break;
			}
		}


		if (!err & (ext_csd[192] >= 2)) {
			/*
			 * According to the JEDEC Standard, the value of
			 * ext_csd's capacity is valid if the value is more
			 * than 2GB
			 */
			capacity = ext_csd[212] << 0 | ext_csd[213] << 8 |
				   ext_csd[214] << 16 | ext_csd[215] << 24;
			capacity *= 512;
			if ((capacity >> 20) > 2 * 1024)
				mmc->capacity = capacity;
		}

		/*
		 * Check whether GROUP_DEF is set, if yes, read out
		 * group size from ext_csd directly, or calculate
		 * the group size from the csd value.
		 */
		if (ext_csd[175])
			mmc->erase_grp_size = ext_csd[224] * 512 * 1024;
		else {
			int erase_gsz, erase_gmul;
			erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
			erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
			mmc->erase_grp_size = (erase_gsz + 1)
				* (erase_gmul + 1);
		}

		/* store the partition info of emmc */
		if (ext_csd[160] & PART_SUPPORT)
			mmc->part_config = ext_csd[179];
	}

	if (IS_SD(mmc))
		err = sd_change_freq(mmc);
	else
		err = mmc_change_freq(mmc);

	if (err){
		mmcinfo("mmc %d Change speed mode failed\n",mmc->control_num);
		return err;
	}

	/* for re-update sample phase */
	err = mmc_update_phase(mmc);
	if (err)
	{
		mmcinfo("update clock failed\n");
		return err;
	}

	mmcdbg("mmc->card_caps 0x%x, ddr caps:0x%x\n", mmc->card_caps, mmc->card_caps & MMC_MODE_DDR_52MHz);
	/* Restrict card's capabilities by what the host can do */
	mmc->card_caps &= mmc->host_caps;
	mmcdbg("mmc->card_caps 0x%x, ddr caps:0x%x\n", mmc->card_caps, mmc->card_caps & MMC_MODE_DDR_52MHz);
	if (!(mmc->card_caps & MMC_MODE_DDR_52MHz) && !IS_SD(mmc)) {
		if (mmc->speed_mode == HSDDR52_DDR50)
			mmc->speed_mode = HSSDR52_SDR25;
		else
			mmc->speed_mode = DS26_SDR12;
	}

	if (IS_SD(mmc)) {
		if (mmc->card_caps & MMC_MODE_4BIT) {
			cmd.cmdidx = MMC_CMD_APP_CMD;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = mmc->rca << 16;
			cmd.flags = 0;

			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (err){
				mmcinfo("mmc %d send app cmd failed\n",mmc->control_num);
				return err;
			}

			cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = 2;
			cmd.flags = 0;
			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (err){
				mmcinfo("mmc %d sd set bus width failed\n",mmc->control_num);
				return err;
			}

			mmc_set_bus_width(mmc, 4);
		}

		if (mmc->card_caps & MMC_MODE_HS)
			mmc->tran_speed = 50000000;
		else
			mmc->tran_speed = 25000000;
	} else {

		if (mmc->card_caps & MMC_MODE_8BIT) {

			/* Set the card to use 8 bit*/
			if( (mmc->card_caps & MMC_MODE_DDR_52MHz) ){
				mmcdbg("8bit ddr!!! \n");
				/* Set the card to use 8 bit ddr*/
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						EXT_CSD_BUS_WIDTH,
						EXT_CSD_BUS_DDR_8);
				if (err){
					mmcinfo("mmc %d switch bus width failed\n", mmc->control_num);
					return err;
				}
				//mmc_set_bus_mode(mmc,1);
				mmc_set_bus_width(mmc, 8);
			}else{
				/* Set the card to use 8 bit*/
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						EXT_CSD_BUS_WIDTH,
						EXT_CSD_BUS_WIDTH_8);

				if (err){
					mmcinfo("mmc %d switch bus width8 failed\n", mmc->control_num);
					return err;
				}
				mmc_set_bus_width(mmc, 8);
			}
		}else if (mmc->card_caps & MMC_MODE_4BIT) {
			if ( (mmc->card_caps & MMC_MODE_DDR_52MHz) ){
				mmcdbg("4bit bus ddr!!! \n");
				/* Set the card to use 4 bit ddr*/
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						EXT_CSD_BUS_WIDTH,
						EXT_CSD_BUS_DDR_4);
				if (err){
					mmcinfo("mmc %d switch bus width failed\n", mmc->control_num);
					return err;
				}
				//mmc_set_bus_mode(mmc,1);
				mmc_set_bus_width(mmc, 4);
			}else{
				/* Set the card to use 4 bit*/
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						EXT_CSD_BUS_WIDTH,
						EXT_CSD_BUS_WIDTH_4);
				if (err){
					mmcinfo("mmc %d switch bus width failed\n", mmc->control_num);
					return err;
				}
				mmc_set_bus_width(mmc, 4);
			}
		}

		if (mmc->card_caps & MMC_MODE_DDR_52MHz) {
			mmc->tran_speed = 52000000;
		} else if (mmc->card_caps & MMC_MODE_HS) {
			if (mmc->card_caps & MMC_MODE_HS_52MHz)
				mmc->tran_speed = 52000000;
			else
				mmc->tran_speed = 26000000;
		} else {
			mmc->tran_speed = 26000000;
		}
	}

	mmcdbg("%s: set clock %d\n", __FUNCTION__, mmc->tran_speed);
	mmc_set_clock(mmc, mmc->tran_speed);

	/* fill in device description */
	mmc->blksz = mmc->read_bl_len;
	//mmc->lba = mmc->capacity/mmc->read_bl_len;   //for compiler error
	mmc->lba = mmc->capacity>>9;

	if(!IS_SD(mmc)){
		switch(mmc->version)
		{
			case MMC_VERSION_1_2:
				mmcinfo("MMC 1.2\n");
				break;
			case MMC_VERSION_1_4:
				mmcinfo("MMC 1.4\n");
				break;
			case MMC_VERSION_2_2:
				mmcinfo("MMC 2.2\n");
				break;
			case MMC_VERSION_3:
				mmcinfo("MMC 3.0\n");
				break;
			case MMC_VERSION_4:
				mmcinfo("MMC 4.0\n");
				break;
			case MMC_VERSION_4_1:
				mmcinfo("MMC 4.1\n");
				break;
			case MMC_VERSION_4_2:
				mmcinfo("MMC 4.2\n");
				break;
			case MMC_VERSION_4_3:
				mmcinfo("MMC 4.3\n");
				break;
			case MMC_VERSION_4_41:
				mmcinfo("MMC 4.41\n");
				break;
			case MMC_VERSION_4_5:
				mmcinfo("MMC 4.5\n");
				break;
			case MMC_VERSION_5_0:
				mmcinfo("MMC 5.0\n");
				break;
			case MMC_VERSION_5_1:
				mmcinfo("MMC 5.1\n");
				break;
			default:
				mmcinfo("Unknow MMC ver\n");
				break;
		}
	}

#if 0
	if (IS_SD(mmc)){
		sprintf(mmc->revision, "PRV %d.%d", mmc->cid[2] >> 28,
			(mmc->cid[2] >> 24) & 0xf);
	} else {
		sprintf(mmc->revision, "PRV %d.%d", (mmc->cid[2] >> 20) & 0xf,
			(mmc->cid[2] >> 16) & 0xf);
	}
	mmcinfo("%s\n", mmc->revision);

	if (IS_SD(mmc)) {
		mmcinfo("MDT m-%d y-%d\n", ((mmc->cid[3] >> 8) & 0xF), (((mmc->cid[3] >> 12) & 0xFF) + 2000));
	} else {
		if (ext_csd[192] > 4) {
			mmcinfo("MDT m-%d y-%d\n", ((mmc->cid[3] >> 12) & 0xF),
				(((mmc->cid[3] >> 8) & 0xF) < 13) ? (((mmc->cid[3] >> 8) & 0xF) + 2013) : (((mmc->cid[3] >> 8) & 0xF) + 1997));
		} else {
			mmcinfo("MDT m-%d y-%d\n", ((mmc->cid[3] >> 12) & 0xF), (((mmc->cid[3] >> 8) & 0xF) + 1997));
		}
	}
#endif

	mmcinfo("%s %d bit\n", spd_name[mmc->speed_mode], mmc->bus_width);
	mmcinfo("%d Hz\n", mmc->clock);
	mmcinfo("%d MB\n", mmc->lba >> 11);
	return 0;
}

int mmc_send_if_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = ((mmc->voltages & 0xff8000) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err){
		mmcinfo("mmc %d send if cond failed\n",mmc->control_num);
		return err;
	}

	if ((cmd.response[0] & 0xff) != 0xaa)
		return UNUSABLE_ERR;
	else
		mmc->version = SD_VERSION_2;

	return 0;
}

int mmc_init(struct mmc *mmc)
{
	int err;
	struct boot_sdmmc_private_info_t *priv_info =
		(struct boot_sdmmc_private_info_t *)(BT0_head.prvt_head.storage_data+SDMMC_PRIV_INFO_ADDR_OFFSET);

	if (mmc->has_init){
		mmcinfo("mmc %d Has init\n",mmc->control_num);
		return 0;
	}

	err = mmc->init(mmc);
	if (err){
		mmcinfo("mmc %d host init failed\n",mmc->control_num);
		return err;
	}

	mmc_set_bus_width(mmc, 1);
	mmc_set_clock(mmc, 1);

	/* Reset the Card */
	err = mmc_go_idle(mmc);
	if (err){
		mmcinfo("mmc %d reset card failed\n",mmc->control_num);
		return err;
	}

	/* The internal partition reset to user partition(0) at every CMD0*/
	mmc->part_num = 0;

#if 0
	mmcinfo("***Try SD card %d***\n",mmc->control_num);
	/* Test for SD version 2 */
	err = mmc_send_if_cond(mmc);

	/* Now try to get the SD card's operating condition */
	err = sd_send_op_cond(mmc);

	/* If the command timed out, we check for an MMC card */
	if(err){
		mmcinfo("***Try MMC card %d***\n",mmc->control_num);
		err = mmc_send_op_cond(mmc);

		if (err) {
			mmcinfo("mmc %d Card did not respond to voltage select!\n",mmc->control_num);
			mmcinfo("***SD/MMC %d init error!!!***\n",mmc->control_num);
			return UNUSABLE_ERR;
		}
	}
#else
	if (priv_info->card_type == CARD_TYPE_SD)
	{
		mmcinfo("***Try SD card %d***\n",mmc->control_num);
		/* Test for SD version 2 */
		err = mmc_send_if_cond(mmc);

		/* Now try to get the SD card's operating condition */
		err = sd_send_op_cond(mmc);

		if (err) {
			mmcinfo("SD card %d Card did not respond to voltage select!\n",mmc->control_num);
			mmcinfo("***SD/MMC %d init error!!!***\n",mmc->control_num);
			return UNUSABLE_ERR;
		}
	}
	else if (priv_info->card_type == CARD_TYPE_MMC)
	{
		/* If the command timed out, we check for an MMC card */
		mmcinfo("***Try MMC card %d***\n",mmc->control_num);
		err = mmc_send_op_cond(mmc);

		if (err) {
			mmcinfo("MMC card %d Card did not respond to voltage select!\n",mmc->control_num);
			mmcinfo("***SD/MMC %d init error!!!***\n",mmc->control_num);
			return UNUSABLE_ERR;
		}
	}
	else
	{
		mmcinfo("Wrong media type 0x%x\n", priv_info->card_type);

		mmcinfo("***Try SD card %d***\n",mmc->control_num);
		/* Test for SD version 2 */
		err = mmc_send_if_cond(mmc);

		/* Now try to get the SD card's operating condition */
		err = sd_send_op_cond(mmc);

		/* If the command timed out, we check for an MMC card */
		if(err){
			mmcinfo("***Try MMC card %d***\n",mmc->control_num);
			err = mmc_send_op_cond(mmc);

			if (err) {
				mmcinfo("mmc %d Card did not respond to voltage select!\n",mmc->control_num);
				mmcinfo("***SD/MMC %d init error!!!***\n",mmc->control_num);
				return UNUSABLE_ERR;
			}
		}
	}
#endif

	err = mmc_startup(mmc);
	if (err){
		mmcinfo("***SD/MMC %d init error!!!***\n",mmc->control_num);
		mmc->has_init = 0;
	}
	else{
		mmc->has_init = 1;
		mmcinfo("***SD/MMC %d init OK!!!***\n",mmc->control_num);
	}

	return err;
}

int mmc_register(int dev_num, struct mmc *mmc)
{
	mmc_devices[dev_num] = mmc;

	if (!mmc->b_max)
		mmc->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	return mmc_init(mmc);
}

int mmc_unregister(int dev_num)
{
	mmc_devices[dev_num] = NULL;
	mmcdbg("mmc%d unregister\n",dev_num);
	return 0;
}
