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

#include "sunxi_mmc.h"
#include "mmc_def.h"

char *spd_name[] = {"DS26/SDR12", "HSSDR52/SDR25", "HSDDR52/DDR50", "HS200/SDR104", "HS400"};

static const char hs200_tuning_blk_4b[64] = {
	/*hs200/uhs*/
	0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
	0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
	0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
	0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
	0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
	0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
	0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
	0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const char hs200_tuning_blk_8b[128] = {
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

static const char extra_tuning_data_wifi[64] = {
	0x30, 0x43, 0x04, 0x16, 0x00, 0x90, 0x10, 0x18,
	0x01, 0x00, 0xf8, 0x4b, 0x11, 0x42, 0x00, 0x27,
	0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x18,
	0xc5, 0x00, 0x10, 0x18, 0x01, 0x12, 0xf8, 0x4b,
	0x11, 0x42, 0x00, 0x19, 0x03, 0x01, 0x00, 0x00,
	0x05, 0x10, 0x00, 0x18, 0xc5, 0x10, 0x10, 0x18,
	0x30, 0x43, 0x04, 0x16, 0x00, 0x90, 0x10, 0x18,
	0x01, 0x00, 0xf8, 0x4b, 0x11, 0x42, 0x00, 0x27,
};

static const char extra_tuning_data_rand[128] = {
	0xe4, 0x4f, 0x76, 0xbb, 0xf0, 0xb7, 0xe0, 0xdb,
	0xb9, 0x1f, 0x9f, 0xfb, 0x7e, 0x9b, 0x03, 0x7d,
	0x2e, 0x32, 0x8f, 0x29, 0x7a, 0x9b, 0xab, 0x16,
	0x2f, 0x44, 0x99, 0xce, 0xc3, 0x99, 0xaa, 0xad,
	0x2d, 0x82, 0xb2, 0x8a, 0xfa, 0x2d, 0xb9, 0x9a,
	0x9e, 0x0f, 0xf3, 0x90, 0x08, 0x25, 0xf3, 0x09,
	0x79, 0x80, 0x1b, 0x28, 0x95, 0x00, 0x57, 0x7d,
	0xbb, 0x60, 0x0b, 0x2c, 0x92, 0x72, 0x49, 0x4b,
	0xe4, 0xac, 0x48, 0x8b, 0xb0, 0xe4, 0x11, 0x1b,
	0x7a, 0x58, 0x7c, 0xc9, 0xe6, 0xf1, 0x5b, 0x6b,
	0x85, 0xc9, 0xf5, 0x7d, 0xef, 0xea, 0xb6, 0x0b,
	0x12, 0x59, 0x24, 0xd2, 0xc9, 0x53, 0x15, 0xa2,
	0xb1, 0xd6, 0x1f, 0x06, 0x38, 0x63, 0x51, 0x27,
	0xf6, 0x03, 0x20, 0xee, 0x41, 0x88, 0xa4, 0x69,
	0xfb, 0x15, 0x05, 0x70, 0xaf, 0xe0, 0x30, 0x88,
	0xdc, 0x37, 0xce, 0x07, 0x91, 0xc1, 0x76, 0x79,
};

__attribute__((section(".data")))
u8 *tuning_blk_4b = NULL;

__attribute__((section(".data")))
u8 *tuning_blk_8b = NULL;

__attribute__((section(".data")))
u32 tuning_blk_cnt_8b = 0;

__attribute__((section(".data")))
u32 tuning_blk_cnt_4b = 0;

u32 freq_def = 6*1000*1000;

u32 freq_ds26_sdr12[8] = {
	400*1000, 25*1000*1000, 50*1000*1000,
};
u32 freq_hssdr52_sdr25[8] = {
	400*1000, 25*1000*1000, 50*1000*1000,
};
u32 freq_hsddr52_ddr50[8] = {
	400*1000, 25*1000*1000, 50*1000*1000,
};
u32 freq_hs200_sdr104[8] = {
	400*1000, 25*1000*1000, 50*1000*1000, 100*1000*1000,
	150*1000*1000, 200*1000*1000,
};
u32 freq_hs400[8] = {
	400*1000, 25*1000*1000, 50*1000*1000, 100*1000*1000,
	150*1000*1000, 200*1000*1000,
};

u32 freq_range_ds26_sdr12[2]    = {400*1000, 26*1000*1000};
u32 freq_range_hssdr52_sdr25[2] = {50*1000*1000, 52*1000*1000};
u32 freq_range_hsddr52_ddr50[2] = {25*1000*1000, 52*1000*1000};
u32 freq_range_hs200_sdr104[2]  = {50*1000*1000, 200*1000*1000};
u32 freq_range_hs400[2]         = {50*1000*1000, 150*1000*1000};

u8 pat_seed_pair[8][2] = {
		{0xFE, 0x01},
		{0x01, 0xFE},
		{0x00, 0xFE},
		//{0xFE, 0x00},
		{0x01, 0xFF},
		//{0xFF, 0x01},
		//{0x00, 0xFF},
		//{0xFF, 0x00},
};

#define TUNING_PAT_CNT_EACH_DATA_LINE 4
#define TUNING_PAT_SIZE_8BIT 128
#define TUNING_PAT_SIZE_4BIT 64

int gen_pat_bus4_1bit(u8 *dat, int bit_no, int pat_size)
{
	int p;
	u8 tmp, d_1st, d_2nd;
	int i, cur_len = 0;
	int repeat_cnt = pat_size>>1;

	cur_len = 0;
	for (p=0; p<TUNING_PAT_CNT_EACH_DATA_LINE; p++)
	{
		tmp = pat_seed_pair[p][0];
		for (i=1; i<=bit_no; i++)
		{
			tmp = ((tmp<<1)&0xf) | ((tmp>>(4-1)) & 0x1) ;
		}
		d_1st = ((tmp&0xf)<<4) | (tmp&0xf);

		tmp = pat_seed_pair[p][1];
		for (i=1; i<=bit_no; i++)
		{
			tmp = ((tmp<<1)&0xf) | ((tmp>>(4-1)) & 0x1) ;
		}
		d_2nd = ((tmp&0xf)<<4) | (tmp&0xf);

		for (i=0; i<repeat_cnt; i++)
		{
			dat[cur_len++] = d_1st;
			dat[cur_len++] = d_2nd;
		}
	}

	return 0;
}


int gen_pat_bus8_1bit(u8 *dat, int bit_no, int pat_size)
{
	int p;
	u8 tmp, d_1st, d_2nd;
	int i, cur_len = 0;
	int repeat_cnt = pat_size>>1;  //each pattern 128 byte for bus width 8

	cur_len = 0;
	for (p=0; p<TUNING_PAT_CNT_EACH_DATA_LINE; p++)
	{
		tmp = pat_seed_pair[p][0];
		for (i=1; i<=bit_no; i++)
		{
			tmp = ((tmp<<1)&0xff) | ((tmp>>(8-1)) & 0x1) ;
		}
		d_1st = tmp & 0xff;

		tmp = pat_seed_pair[p][1];
		for (i=1; i<=bit_no; i++)
		{
			tmp = ((tmp<<1)&0xff) | ((tmp>>(8-1)) & 0x1) ;
		}
		d_2nd = tmp & 0xff;


		for (i=0; i<repeat_cnt; i++)
		{
			dat[cur_len++] = d_1st;
			dat[cur_len++] = d_2nd;
		}
	}

	return 0;
}

int gen_tuning_blk_bus8(u8 *buf, u32 *pat_blk_cnt)
{
	int i, tmp, bit = 0;
	u8 *pdata = (u8 *)buf;
	int pat_cnt = 0;
	u8 *cur = NULL;

	//hs200 tuning block
	pat_cnt = 0;
	for (i=0; i<512; i++)
	{
		*(pdata + i) = hs200_tuning_blk_8b[i%128];
	}

	//io data pattern
	pat_cnt++;
	for (bit=0; bit<8; bit++)
	{
		cur = pdata + pat_cnt*512 + bit*TUNING_PAT_CNT_EACH_DATA_LINE*TUNING_PAT_SIZE_8BIT;
		gen_pat_bus8_1bit(cur, bit, TUNING_PAT_SIZE_8BIT);
	}

	pat_cnt += ((8*TUNING_PAT_CNT_EACH_DATA_LINE*TUNING_PAT_SIZE_8BIT) >> 9);

	tmp = (8*TUNING_PAT_CNT_EACH_DATA_LINE*TUNING_PAT_SIZE_8BIT) % 512;
	if (tmp)
	{
		for (i=0; i<(512-tmp); i++)
		{
			*(pdata + pat_cnt*512 + tmp + i) = extra_tuning_data_rand[i%128];
		}
		pat_cnt++;
	}

	//rand, wifi, 0x00, 0xff
	for (i=0; i<128; i++)
	{
		*(pdata + pat_cnt*512 + i) = extra_tuning_data_rand[i%128];
	}
	for (i=128; i<256; i++)
	{
		*(pdata + pat_cnt*512 + i) = extra_tuning_data_wifi[i%64];
	}
	for (i=(256>>1); i<(384>>1); i++)
	{
		*(pdata + pat_cnt*512 + 2*i) = 0x00;
		*(pdata + pat_cnt*512 + 2*i + 1) = 0xFF;
	}
	for (i=(384>>1); i<(512>>1); i++)
	{
		*(pdata + pat_cnt*512 + 2*i) = 0xFF;
		*(pdata + pat_cnt*512 + 2*i + 1) = 0x00;
	}
	pat_cnt++;

	*pat_blk_cnt = pat_cnt;
	MMCINFO("%s: total blk %d\n", __FUNCTION__, *pat_blk_cnt);
	return 0;
}

int gen_tuning_blk_bus4(u8 *buf, u32 *pat_blk_cnt)
{
	int i, tmp, bit = 0;
	u8 *pdata = (u8 *)buf;
	int pat_cnt = 0;
	u8 *cur = NULL;

	//hs200 tuning block
	pat_cnt = 0;
	for (i=0; i<512; i++)
	{
		*(pdata + i) = hs200_tuning_blk_4b[i%64];
	}

	//io data pattern
	pat_cnt++;
	for (bit=0; bit<8; bit++)
	{
		cur = pdata + pat_cnt*512 + bit*TUNING_PAT_CNT_EACH_DATA_LINE*TUNING_PAT_SIZE_4BIT;
		gen_pat_bus4_1bit(cur, bit, TUNING_PAT_SIZE_8BIT);
	}

	pat_cnt += ((8*TUNING_PAT_CNT_EACH_DATA_LINE*TUNING_PAT_SIZE_4BIT) >> 9);

	tmp = (8*TUNING_PAT_CNT_EACH_DATA_LINE*TUNING_PAT_SIZE_4BIT) % 512;
	if (tmp)
	{
		for (i=0; i<(512-tmp); i++)
		{
			*(pdata + pat_cnt*512 + tmp + i) = extra_tuning_data_rand[i%64];
		}
		pat_cnt++;
	}

	//rand, wifi, 0x00, 0xff
	for (i=0; i<128; i++)
	{
		*(pdata + pat_cnt*512 + i) = extra_tuning_data_rand[i%128];
	}
	for (i=128; i<256; i++)
	{
		*(pdata + pat_cnt*512 + i) = extra_tuning_data_wifi[i%64];
	}
	for (i=(256>>1); i<(384>>1); i++)
	{
		*(pdata + pat_cnt*512 + 2*i) = 0x00;
		*(pdata + pat_cnt*512 + 2*i + 1) = 0xFF;
	}
	for (i=(384>>1); i<(512>>1); i++)
	{
		*(pdata + pat_cnt*512 + 2*i) = 0xFF;
		*(pdata + pat_cnt*512 + 2*i + 1) = 0x00;
	}
	pat_cnt++;

	*pat_blk_cnt = pat_cnt;
	MMCINFO("%s: total blk %d\n", __FUNCTION__, *pat_blk_cnt);

	return 0;
}

int sunxi_mmc_tuning_init(void)
{
	int ret = 0;

	if ( NULL == (tuning_blk_8b = (u8 *)malloc(TUNING_LEN*512))) {
		MMCINFO("%s: request memory for buf_bus8 fail\n", __FUNCTION__);
		return -1;
	}

	if ( NULL == (tuning_blk_4b = (u8 *)malloc(TUNING_LEN*512))) {
		MMCINFO("%s: request memory for buf_bus4 fail\n", __FUNCTION__);
		ret = -1;
		goto OUT;
	}

	gen_tuning_blk_bus8(tuning_blk_8b, &tuning_blk_cnt_8b);
	gen_tuning_blk_bus4(tuning_blk_4b, &tuning_blk_cnt_4b);

	if (tuning_blk_cnt_8b > TUNING_LEN)
		tuning_blk_cnt_8b = TUNING_LEN;
	if (tuning_blk_cnt_4b > TUNING_LEN)
		tuning_blk_cnt_4b = TUNING_LEN;

	return 0;

OUT:
	if (!tuning_blk_8b)
		free(tuning_blk_8b);

	return ret;
}

int sunxi_mmc_tuning_exit(void)
{
	if (!tuning_blk_4b)
		free(tuning_blk_4b);

	if (!tuning_blk_8b)
		free(tuning_blk_8b);

	return 0;
}

extern int mmc_mmc_switch_bus_mode(struct mmc *mmc, int spd_mode, int width);
static ulong sunxi_read_tuning(int dev_num, ulong start, lbaint_t blkcnt, void *dst);

unsigned int sunxi_select_freq(struct mmc *mmc, int speed_md, int freq_index)
{
	u32 freq = 0;
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;
	int i;
	u32 val;

	if (freq_index >= 8) {
		MMCINFO("freq_index error %d\n", freq_index);
		return 0;
	}

	for (i=0; i<MAX_EXT_FREQ_POINT_NUM; i++)
	{
		val = host->cfg.platform_caps.tm4_tune_ext_freq[i];
		if (val & (1U<<31)) {
			if ((((val>>8) & 0xff) == freq_index) && (((val>>16) & 0xff) == speed_md)) {
				freq = (val & 0xff)*1000*1000;
				MMCINFO("select ext freq point: %d-%d MHz\n", i, (val & 0xff));
				goto OUT;
			}
		}
	}

	if (speed_md == DS26_SDR12)
		freq = freq_ds26_sdr12[freq_index];
	else if (speed_md == HSSDR52_SDR25)
		freq = freq_hssdr52_sdr25[freq_index];
	else if (speed_md == HSDDR52_DDR50)
		freq = freq_hsddr52_ddr50[freq_index];
	else if (speed_md == HS200_SDR104)
		freq = freq_hs200_sdr104[freq_index];
	else if (speed_md == HS400)
		freq = freq_hs400[freq_index];
	else {
		MMCINFO("speed_md error %d\n", speed_md);
		freq = 0;
	}

OUT:
	return freq;
}

static int sunxi_tuning_method_0(struct mmc *mmc, int retry_times)
{
	ulong ret = 0;
	int j;
	const u8 *std_pat = NULL;
	char *rcv_pat = NULL;
	int pat_blk_cnt = 0;

	if (NULL == (rcv_pat = malloc(TUNING_LEN*512))) {
		MMCINFO("request memory for rcv_pat fail\n");
		return -1;
	}

	if (mmc->bus_width == 4) {
		std_pat = tuning_blk_4b;
		pat_blk_cnt = tuning_blk_cnt_4b;
	} else if (mmc->bus_width == 8) {
		std_pat = tuning_blk_8b;
		pat_blk_cnt = tuning_blk_cnt_8b;
	} else if(mmc->bus_width == 1) {
		MMCINFO("Not support 1 bit tuning now\n");
		ret = -1;
		goto OUT;
	}

	for(j=0; j<retry_times; j++)
	{
		ret = sunxi_read_tuning(mmc->cfg->host_no,
						TUNING_ADD,
						pat_blk_cnt,
						rcv_pat);
		if (ret != pat_blk_cnt) {
			MMCMSG(mmc, "read failed\n");
			#if 0
			//if read failedand block len>1,send stop for next try
			//no care if it is successed
			if (pat_blk_cnt>1){
				MMCINFO("Send stop\n");
				mmc_send_manual_stop(mmc);
			}
			#endif

			#if 1
				//MMCINFO("Send manual stop\n");
				mmc_send_manual_stop(mmc);
			#endif
			break;
		}
		ret = memcmp(std_pat, rcv_pat, pat_blk_cnt*512);
		if(ret) {
			MMCINFO("pattern compare fail\n");
			break;
		}
	}
	if (j != retry_times)
		ret = -1;
	else
		ret = 0;
OUT:
	free(rcv_pat);

	return ret;
}

static int sunxi_tuning_method_1(struct mmc *mmc, int retry_times)
{
	struct mmc_cmd cmd;
	int err, retry = 0;

	/* set cmd13 to get status */
	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	if (!mmc_host_is_spi(mmc))
		cmd.cmdarg = mmc->rca << 16;
	cmd.flags = 0;

	do {
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err)
			break;

		udelay(10);
		retry++;
	} while (retry < retry_times);

	return err;
}


static int _skip_curr_freq(struct mmc *mmc, int spd_md, int freq)
{
	u32 min, max;
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;

	if (spd_md == DS26_SDR12) {
		min = freq_range_ds26_sdr12[0];
		max = freq_range_ds26_sdr12[1];
	} else if (spd_md == HSSDR52_SDR25) {
		min = freq_range_hssdr52_sdr25[0];
		max = freq_range_hssdr52_sdr25[1];
	} else if (spd_md == HSDDR52_DDR50) {
		min = freq_range_hsddr52_ddr50[0];
		max = freq_range_hsddr52_ddr50[1];
	} else if (spd_md == HS200_SDR104) {
		min = freq_range_hs200_sdr104[0];

		if (host->cfg.platform_caps.tm4_tune_hs200_max_freq)
			max = host->cfg.platform_caps.tm4_tune_hs200_max_freq * 1000 * 1000;
		else
			max = freq_range_hs200_sdr104[1];
	} else if (spd_md == HS400) {
		min = freq_range_hs400[0];

		if (host->cfg.platform_caps.tm4_tune_hs400_max_freq)
			max = host->cfg.platform_caps.tm4_tune_hs400_max_freq * 1000 * 1000;
		else
			max = freq_range_hs400[1];
	} else {
		min = 400*1000;
		max = 100*1000*1000;
	}

	if ((freq < min) || (freq > max))
		return 1;
	else
		return 0;
}

static int _get_best_sdly(int sdly_cnt, u8 win_th, u8 *p)
{
	int i, j, max, group;
	int s[15]={0}; //start
	int e[15]={0}; //end
	int w[15]={0}; //window
	int in_group = 0;
	u8 best = 0;

	/* get window boundary */
	i = 0;
	group = 0; //group
	in_group = 0;
	while (i<sdly_cnt)
	{
		if (in_group) {
			if (p[i] == 0) {
				in_group = 0;
				group++;
			} else {
				e[group] = i;
				if (i == (sdly_cnt-1)) {
					in_group = 0;
					group++;
				}
			}
		} else {
			if (p[i] == 1) {
				in_group = 1;
				s[group] = i;
				e[group] = i;
			}
		}

		i++;
	};

	/* get window size */
	for (i=0; i<group; i++)
	{
		if (e[i] >= s[i])
			w[i] = e[i] - s[i] + 1;
		else
			w[i] = 0;
	}

	if (group)
		printf("[mmc]: ");
	for (i=0; i<group; i++)
	{
		printf("[%d-%d|%d] ", s[i], e[i], w[i]);
		if (group && (i==(group-1)))
			printf("\n");
	}

	/*
	* in theory, the first two sample delay sections should include a max or almost max sections.
	* we check and choose best sample delay from these two sections. it will reduce impact on sample delay
	* caused by temperature and voltage variation partly.
	*/
	if (group > 2)
		group = 2;

	/* get max window */
	j = 0;
	max = w[0];
	for (i=1; i<group; i++)
	{
		if (w[i] > max) {
			j = i;
			max = w[i];
		}
	}

	/*get best point */
	if (w[j] >= win_th)
		best = s[j] + (w[j]>>1);
	else
		best = 0xff;

	MMCDBG("---- %d-%d: %d - %d,  best: 0x%x\n", j, w[j], s[j], e[j], best);

	return best;
}

static int sunxi_tuning_speed_mode(struct mmc *mmc, int speed_mode, int tuning_mode, int retry_times)
{
	int freq_index=0, freq = 0;
	int i, ret = 0;
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;
	int tm = host->timing_mode;
	u8 tm4_win_th = host->cfg.platform_caps.tm4_timing_window_th;
	u8 *sdly_cfg = NULL;
	int sdly, sdly_cnt = 0;
	u8 *p = NULL;
	u8 best = 0;

	if (tm == SUNXI_MMC_TIMING_MODE_1) {
		sdly_cnt = host->tm1.sample_point_cnt;
		sdly_cfg = (u8 *)host->tm1.sdly;
	} else if (tm == SUNXI_MMC_TIMING_MODE_3) {
		sdly_cnt = host->tm3.sample_point_cnt;
		sdly_cfg = (u8 *)host->tm3.sdly;
	} else if (tm == SUNXI_MMC_TIMING_MODE_4) {
		sdly_cnt = host->tm4.sample_point_cnt;
		if (speed_mode == HS400) {
			sdly_cfg = (u8 *)host->tm4.dsdly;
			MMCDBG("%s: current is HS400 mode, dsdly:0x%x\n", __FUNCTION__, (u32)sdly_cfg);
		} else
			sdly_cfg = (u8 *)host->tm4.sdly;
	} else if (tm == SUNXI_MMC_TIMING_MODE_2) {
		if (speed_mode == HS400) {
			sdly_cnt = host->tm2.sample_point_cnt_hs400;
			sdly_cfg = (u8 *)host->tm2.dsdly;
			MMCDBG("%s: current is HS400 mode, dsdly:0x%x\n", __FUNCTION__, (u32)sdly_cfg);
		} else {
			sdly_cnt = host->tm2.sample_point_cnt;
			sdly_cfg = (u8 *)host->tm2.sdly;
		}
	}

	if (NULL == (p = malloc(sdly_cnt*MAX_CLK_FREQ_NUM))) {
		MMCINFO("%s: request memory fail\n", __FUNCTION__);
		return -1;
	}

	freq_index = 0;
	while ((freq = sunxi_select_freq(mmc, speed_mode, freq_index)) != 0)
	{
		/* change clock frequency */
		mmc->tran_speed = freq;

		if (!_skip_curr_freq(mmc, speed_mode, freq))
		{
			/* do tuning  */
			//MMCINFO("start tuning spd_md: %d-%s, freq: %d-%d\n", speed_mode, spd_name[speed_mode], freq_index, freq);
			MMCINFO("freq: %d-%d\n", freq_index, freq);
			for (sdly=0; sdly<sdly_cnt; sdly++)
			{
				/* modify sample point cfg*/
				if (speed_mode == HS400)
					sdly_cfg[freq_index] = sdly;
				else
					sdly_cfg[speed_mode*MAX_CLK_FREQ_NUM + freq_index] = sdly;

				/* update sample point cfg */
				mmc_set_clock(mmc, mmc->tran_speed);
				if (tuning_mode == 0)
				{
					if (!sunxi_tuning_method_0(mmc, retry_times))
						p[freq_index*sdly_cnt + sdly] = 1;
					else
						p[freq_index*sdly_cnt + sdly] = 0;
				}
			}
		}
		else
		{
			MMCINFO("skip freq %d\n", freq);
			for (sdly=0; sdly<sdly_cnt; sdly++)
				p[freq_index*sdly_cnt + sdly] = 0xFF;
		}

		freq_index++;
	}

	/* dump tuning result */
	MMCMSG(mmc, "speed mode: %s\n", spd_name[speed_mode]);
	for (i=0; i<freq_index; i++)
	{
		MMCMSG(mmc, "---%dHz: \n", sunxi_select_freq(mmc, speed_mode, i));
		#if 0
		for (j=0; j<sdly_cnt; j++)
		{
			if (j && (j%32==0))
				printf("\n");
			printf("%02x ", p[i*sdly_cnt + j]);
		}
		printf("\n");
		#endif

		if (speed_mode == HSDDR52_DDR50)
			tm4_win_th = 8;

		if (tm == SUNXI_MMC_TIMING_MODE_2) {
			if (speed_mode == HS400) {
				/* get best delay for hs400 data strobe delay chain */
				best = _get_best_sdly(sdly_cnt, tm4_win_th, (p + i*sdly_cnt));
			} else {
				if (p[i*sdly_cnt + 0] == 1)
					best = 0;
				else if (p[i*sdly_cnt + 1] == 1)
					best = 1;
				else
					best = 0xFF;
			}
		} else {
			best = _get_best_sdly(sdly_cnt, tm4_win_th, (p + i*sdly_cnt));
		}
		MMCMSG(mmc, "--best %d\n", best);

		/* record best sample point to result[] */
		if (speed_mode == HS400)
			sdly_cfg[i] = best;
		else
			sdly_cfg[speed_mode*MAX_CLK_FREQ_NUM + i] = best; //( ((speed_mode&0xf)<<4) | (i&0xF)); //best;
	}

	/* set proper sample cfg and clock to tune next speed mode */
	freq_index = 2;
	if (((freq = sunxi_select_freq(mmc, speed_mode, freq_index)) != 0)
		&& (sdly_cfg[speed_mode*MAX_CLK_FREQ_NUM + freq_index] != 0xFF)) {
		mmc->tran_speed = freq;
	} else {
		freq_index--;
		MMCINFO("try next freq...%d\n", freq_index);
		if (((freq = sunxi_select_freq(mmc, speed_mode, freq_index)) != 0)
			&& (sdly_cfg[speed_mode*MAX_CLK_FREQ_NUM + freq_index] != 0xFF))
			mmc->tran_speed = freq;
		else {
			ret = -ERR_NO_BEST_DLY;
			mmc->tran_speed = freq_def;
			MMCINFO("invalid freq, switch to %d\n", freq_def);
		}
	}
	/* update sample point cfg */
	mmc_set_clock(mmc, mmc->tran_speed);

	free(p);

	return ret;
}

static int sunxi_tuning_hs400_cmd(struct mmc *mmc, int speed_mode, int tuning_mode, int retry_times)
{
	int freq_index=0, freq = 0;
	int i, ret = 0;
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;
	int tm = host->timing_mode;
	u8 tm4_win_th = host->cfg.platform_caps.tm4_timing_window_th;
	u8 *sdly_cfg = NULL;
	int sdly, sdly_cnt = 0;
	u8 *p = NULL;
	u8 best = 0;

	if (speed_mode != HS400) {
		MMCINFO("%s: input speed mode error %d\n", __FUNCTION__, speed_mode);
		return 0;
	}

	if (tm == SUNXI_MMC_TIMING_MODE_4) {
		sdly_cnt = host->tm4.sample_point_cnt;
		sdly_cfg = (u8 *)host->tm4.sdly;
	} else if (tm == SUNXI_MMC_TIMING_MODE_2) {
		sdly_cnt = host->tm2.sample_point_cnt;
		sdly_cfg = (u8 *)host->tm2.sdly;
	}

	if (NULL == (p = malloc(sdly_cnt*MAX_CLK_FREQ_NUM))) {
		MMCINFO("%s: request memory fail\n", __FUNCTION__);
		return -1;
	}

	freq_index = 0;
	while ((freq = sunxi_select_freq(mmc, speed_mode, freq_index)) != 0)
	{
		/* change clock frequency */
		mmc->tran_speed = freq;

		if (!_skip_curr_freq(mmc, speed_mode, freq))
		{
			/* do tuning  */
			//MMCINFO("start tuning spd_md: %d-%s, freq: %d-%d\n", speed_mode, spd_name[speed_mode], freq_index, freq);
			MMCINFO("freq: %d-%d\n", freq_index, freq);
			for (sdly=0; sdly<sdly_cnt; sdly++)
			{
				/* modify sample point cfg*/
				sdly_cfg[speed_mode*MAX_CLK_FREQ_NUM + freq_index] = sdly;

				/* update sample point cfg */
				mmc_set_clock(mmc, mmc->tran_speed);
				if (tuning_mode == 1)
				{
					if (!sunxi_tuning_method_1(mmc, retry_times)) //use method 1
						p[freq_index*sdly_cnt + sdly] = 1;
					else
						p[freq_index*sdly_cnt + sdly] = 0;
				} else {
					MMCINFO("%s: error tuning method!\n", __FUNCTION__);
				}
			}
		}
		else
		{
			MMCINFO("skip freq %d\n", freq);
			for (sdly=0; sdly<sdly_cnt; sdly++)
				p[freq_index*sdly_cnt + sdly] = 0xFF;
		}

		freq_index++;
	}

	/* dump tuning result */
	MMCMSG(mmc, "speed mode: %s\n", spd_name[speed_mode]);
	for (i=0; i<freq_index; i++)
	{
		MMCMSG(mmc, "---%dHz: \n", sunxi_select_freq(mmc, speed_mode, i));
		#if 0
		for (j=0; j<sdly_cnt; j++)
		{
			if (j && (j%32==0))
				printf("\n");
			printf("%02x ", p[i*sdly_cnt + j]);
		}
		printf("\n");
		#endif
		if (tm == SUNXI_MMC_TIMING_MODE_2) {
			if (p[i*sdly_cnt + 0] == 1)
				best = 0;
			else if (p[i*sdly_cnt + 1] == 1)
				best = 1;
			else
				best = 0xFF;
		} else {
			best = _get_best_sdly(sdly_cnt, tm4_win_th, (p + i*sdly_cnt));
		}
		MMCMSG(mmc, "--best %d\n", best);

		/* record best sample point to result[] */
		sdly_cfg[speed_mode*MAX_CLK_FREQ_NUM + i] = best; //( ((speed_mode&0xf)<<4) | (i&0xF)); //best;
	}

	/* set proper sample cfg and clock to tune next speed mode */
	freq_index = 2;
	if (((freq = sunxi_select_freq(mmc, speed_mode, freq_index)) != 0)
		&& (sdly_cfg[speed_mode*MAX_CLK_FREQ_NUM + freq_index] != 0xFF)) {
		mmc->tran_speed = freq;
	} else {
		ret = -ERR_NO_BEST_DLY;
		mmc->tran_speed = freq_def;
		MMCINFO("invalid freq, switch to 6MHz\n");
	}

	/* update sample point cfg */
	mmc_set_clock(mmc, mmc->tran_speed);

	free(p);

	return ret;
}


int sunxi_need_rty(struct mmc *mmc)
{
	int ret = 0;
	u32 err_no = 0;

	if (mmc->cfg->ops->decide_retry)
	{
		err_no = mmc->cfg->ops->get_detail_errno(mmc);
		ret = mmc->cfg->ops->decide_retry(mmc, err_no, 0);
		if (!ret) {
			MMCINFO("need retry next clk %d\n", mmc->clock);
			return 0;
		}
	}

	return -1;
}

int write_tuning_try_freq(struct mmc *mmc,u32 clk)
{
	int ret = 0;
	char *rcv_pattern = NULL;
	char *std_pattern = NULL;
	u32 err_no = 0;
	int pat_blk_cnt = 0;

	if ( NULL == (rcv_pattern = (char *)malloc(TUNING_LEN*512))) {
		MMCINFO("%s: request memory for rcv_pattern fail\n", __FUNCTION__);
		return -1;
	}

	if ( NULL == (std_pattern = (char *)malloc(TUNING_LEN*512))) {
		MMCINFO("%s: request memory for std_pattern fail\n", __FUNCTION__);
		ret = -1;
		goto out1;
	}

	if (mmc->bus_width == 4) {
		//std_pattern = tuning_blk_4b;
		memcpy(std_pattern, tuning_blk_4b, (TUNING_LEN*512));
		pat_blk_cnt = tuning_blk_cnt_4b;
	} else if (mmc->bus_width == 8) {
		//std_pattern = tuning_blk_8b;
		memcpy(std_pattern, tuning_blk_8b, (TUNING_LEN*512));
		pat_blk_cnt = tuning_blk_cnt_8b;
	} else if (mmc->bus_width == 1) {
		MMCINFO("Don't support 1 bit tuning now\n");
		ret = -1;
		goto out;
	}

	do {
		mmc_set_clock(mmc, clk);
		ret = mmc_bwrite(mmc->cfg->host_no,
						TUNING_ADD,
						pat_blk_cnt,
						std_pattern);
		MMCDBG("Write pattern ret = %d\n",ret);
		if (ret != pat_blk_cnt) { //fail
			MMCINFO("write failed\n");
			err_no = mmc->cfg->ops->get_detail_errno(mmc);
			/* if write failed and block len>1,send stop for next try */
			if (pat_blk_cnt>1) {
				MMCINFO("send stop\n");
				mmc_send_manual_stop(mmc);
			}
		} else {//ok
			MMCINFO("write_tuning_try_freq: write ok\n");

			/* read pattern and compare with the pattern show sent*/
			ret = mmc_bread(mmc->cfg->host_no,
							TUNING_ADD,
							pat_blk_cnt,
							rcv_pattern);
			if (ret != pat_blk_cnt) {
				MMCMSG(mmc, "read failed\n");

				//MMCINFO("0x%08x 0x%08x 0x%08x 0x%08x\n", *(volatile uint *)(ulong)(0x1c1105c), *(volatile uint *)(ulong)(0x1c110140), *(volatile uint *)(ulong)(0x1c110144), *(volatile uint *)(ulong)(0x1c110148));
				err_no = mmc->cfg->ops->get_detail_errno(mmc);
				/*if read failed and block len>1,send stop for next try*/
				if (pat_blk_cnt>1) {
					MMCINFO("Send stop\n");
					mmc_send_manual_stop(mmc);
				}
			} else {
				ret = memcmp(std_pattern, rcv_pattern, pat_blk_cnt*512);
				if(ret) {
					MMCINFO("pattern compare fail\n");
					err_no = 0xffffffff; //force retry
				} else {
					MMCINFO("Pattern compare ok\n");
					MMCINFO("Write tuning pattern ok\n");
					goto out;
				}
			}
		}
	} while(!mmc->cfg->ops->decide_retry(mmc, err_no, 0));
	MMCINFO(" Write tuning pattern failded\n");
	ret = -1;

out:
	free(std_pattern);
out1:
	free(rcv_pattern);
	return ret;
}

int sunxi_write_tuning(struct mmc *mmc)
{
	u32 freqs[] = {/*400000, */25*1000*1000, 50*1000*1000};
	int i = 0;
	int ret = -1;
	int clk_bak = mmc->clock;

	if (mmc->cfg->ops->decide_retry == NULL) {
		MMCINFO("Don't support tuning\n");
		return 0;
	}

	/* reset all sample point */
	mmc->cfg->ops->decide_retry(mmc, 0, 1);
	for (i=0; i<sizeof(freqs)/sizeof(freqs[0]); i++) {
		ret = write_tuning_try_freq(mmc, freqs[i]);
		if (!ret) {
			/* recover the clock before write patten*/
			mmc_set_clock(mmc, clk_bak);
			return ret;
		}
	}
	return ret;
}

static ulong sunxi_read_tuning(int dev_num, ulong start, lbaint_t blkcnt, void *dst)
{
	lbaint_t cur, blocks_todo = blkcnt;
	struct mmc *mmc = find_mmc_device(dev_num);

	if (blkcnt == 0) {
		MMCINFO("blkcnt should not be 0\n");
		return 0;
	}

	if (!mmc) {
		MMCINFO("can not find mmc dev\n");
		return 0;
	}

	if ((start + blkcnt) > mmc->block_dev.lba) {
		MMCINFO("MMC: block number 0x%lx exceeds max(0x%lx)\n",
			start + blkcnt, mmc->block_dev.lba);
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->read_bl_len)) {
		MMCMSG(mmc, "Set block len failed\n");
		return 0;
	}

	do {
		cur = 1;//force to read 1 block a time
		if (mmc_bread(dev_num, start, cur, dst) != cur) {
			MMCMSG(mmc, "block read failed, %s %d\n", __FUNCTION__, __LINE__);
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		dst = (char*)dst + cur * mmc->read_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

int sunxi_execute_tuning(struct mmc *mmc, int speed_mode)
{
	int ret = 0;
	int bus_width = 0;
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;
	int r_cycle = host->cfg.platform_caps.tm4_tune_r_cycle;

	if (!IS_SD(mmc))
	{
		if (mmc->card_caps & MMC_MODE_8BIT)
			bus_width = 8;
		else if (mmc->card_caps & MMC_MODE_4BIT)
			bus_width = 4;
		else
			bus_width = 1;

		/* switch to specific speed mode */
		if (speed_mode == DS26_SDR12)
			ret = mmc_mmc_switch_bus_mode(mmc, DS26_SDR12, bus_width);
		else if (speed_mode == HSSDR52_SDR25)
			ret = mmc_mmc_switch_bus_mode(mmc, HSSDR52_SDR25, bus_width);
		else if (speed_mode == HSDDR52_DDR50)
			ret = mmc_mmc_switch_bus_mode(mmc, HSDDR52_DDR50, bus_width);
		else if (speed_mode == HS200_SDR104)
			ret = mmc_mmc_switch_bus_mode(mmc, HS200_SDR104, bus_width);
		else if (speed_mode == HS400) {
			/* firstly, switch to HS-DDR 8 bit */
			ret = mmc_mmc_switch_bus_mode(mmc, HSDDR52_DDR50, bus_width);
			if (ret) {
				MMCINFO("switch to %s fail\n", spd_name[speed_mode]);
				goto OUT;
			}

			/* then, switch to HS400 */
			ret = mmc_mmc_switch_bus_mode(mmc, HS400, bus_width);
		}
		if (ret) {
			MMCINFO("switch to %s fail\n", spd_name[speed_mode]);
			goto OUT;
		}
	}
	else
	{
		if (speed_mode >= HSDDR52_DDR50) {
			MMCINFO("don't spport %s for sd card\n", spd_name[speed_mode]);
			ret = -1;
			goto OUT;
		}
	}

	/* execute tuning for current speed mode */
	if (r_cycle == 0)
		r_cycle = 15;

	if (speed_mode == HS400) {
		ret = sunxi_tuning_hs400_cmd(mmc, speed_mode, 1, r_cycle);
		if (ret < 0) {
			MMCINFO("tuning hs400 cmd line fail\n");
			goto OUT;
		}
	}

	ret = sunxi_tuning_speed_mode(mmc, speed_mode, 0, r_cycle);
	if (ret) {
		MMCINFO("tuning for %s fail\n", spd_name[speed_mode]);
	}

OUT:
	return ret;
}

int sunxi_pack_tuning_result(struct mmc *mmc)
{
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;
	//char *spd_name[] = {"DS26/SDR12", "HSSDR52/SDR25", "HSDDR52/DDR50", "HS200/SDR104", "HS400"};
	u32 tm = host->timing_mode;
	int spd_md, freq, g, group;
	u32 t, val = 0;
	u8 *p = NULL;

#if 0
	for (spd_md=0; spd_md<MAX_SPD_MD_NUM; spd_md++)
	{

		p = host->tm4.sdly;

		for (freq=0; freq<MAX_CLK_FREQ_NUM; freq++)
		{
			printf("%02x ", p[spd_md*MAX_CLK_FREQ_NUM + freq]);
		}

		if (spd_md == MAX_SPD_MD_NUM-1)
		{
			printf("\n---------- ds delay\n");
			p = host->tm4.dsdly;
			for (freq=0; freq<MAX_CLK_FREQ_NUM; freq++)
			{
				printf("%02x ", p[freq]);
			}
		}

		printf("\n");
	}
	printf("\n");
#endif

	for (spd_md=0; spd_md<MAX_SPD_MD_NUM; spd_md++)
	{
		/* get timing info of current timing mode */
		if (spd_md == HS400)
			group = 2;
		else
			group = 1;

		for (g=0; g<group; g++)
		{
			/* get timing info of current timing mode */
			if (tm == SUNXI_MMC_TIMING_MODE_2) {
				if ((spd_md == HS400) && (g == 0))
					p = host->tm2.dsdly;
				else
					p = host->tm2.sdly;
			} else if (tm == SUNXI_MMC_TIMING_MODE_4) {
				if ((spd_md == HS400) && (g == 0))
					p = host->tm4.dsdly;
				else
					p = host->tm4.sdly;
			}

			val = 0;
			for (freq=0; freq<4; freq++) {
				if ((spd_md == HS400) && (g == 0))
					t = p[freq];
				else
					t = p[spd_md*MAX_CLK_FREQ_NUM+freq];
				val |= ((t & 0xFF) << (8*freq));
			}

			/* because it only use one timing mode and generate one group of timing info. even current
			timing mode is SUNXI_MMC_TIMING_MODE_2, but we also pack the timing info of
			SUNXI_MMC_TIMING_MODE_2 to cfg.platform_caps.sdly.tm4_smx_fx[].
			*/
			host->cfg.platform_caps.sdly.tm4_smx_fx[spd_md*2 + g*2 + 0] = val;

			val = 0;
			for (freq=4; freq<MAX_CLK_FREQ_NUM; freq++) {
				if ((spd_md == HS400) && (g == 0))
					t = p[freq];
				else
					t = p[spd_md*MAX_CLK_FREQ_NUM+freq];
				val |= ((t & 0xFF) << (8*(freq-4)));
			}
			host->cfg.platform_caps.sdly.tm4_smx_fx[spd_md*2 + g*2 + 1] = val;

			MMCINFO("%s: 0x%08x 0x%08x\n", spd_name[spd_md],
				host->cfg.platform_caps.sdly.tm4_smx_fx[spd_md*2 + g*2 + 0],
				host->cfg.platform_caps.sdly.tm4_smx_fx[spd_md*2 + g*2 + 1]);
		}
	}

	return 0;
}

int sunxi_bus_tuning(struct mmc *mmc)
{
	int err = 0, ret = 0;
	unsigned err_flag = 0;

#if 0
	MMCINFO("================== start tuning DS26_SDR12...\n");
	ret = sunxi_execute_tuning(mmc, DS26_SDR12);
	if (ret) {
		MMCINFO("tuning fail at DS26_SDR12\n");
		err = -1;
	}
#endif

	MMCINFO("================== HSSDR52_SDR25...\n");
	ret = sunxi_execute_tuning(mmc, HSSDR52_SDR25);
	if (ret) {
		MMCINFO("tuning fail at HSSDR52_SDR25\n");
		err = -1;
		goto ERR_RET;
	}

	if (mmc->cfg->host_caps & MMC_MODE_HS200)
	{
		MMCINFO("================== HS200_SDR104...\n");
		ret = sunxi_execute_tuning(mmc, HS200_SDR104);
		if (ret) {
			MMCINFO("tuning fail at HS200_SDR104\n");
			//err = -2;
			//goto ERR_RET;
			err_flag |= 0x1;
		}
	}

	if (mmc->cfg->host_caps & MMC_MODE_DDR_52MHz)
	{
		MMCINFO("================== HSDDR52_DDR50...\n");
		ret = sunxi_execute_tuning(mmc, HSDDR52_DDR50);
		if (ret) {
			MMCINFO("tuning fail at HSDDR52_DDR50\n");
			//err = -3;
			//goto ERR_RET;
			err_flag |= 0x2;
		}
	}

	if (((mmc->cfg->host_caps & (MMC_MODE_HS400|MMC_MODE_8BIT)) == (MMC_MODE_HS400|MMC_MODE_8BIT))
		&& !(err_flag & 0x3))
	{
		MMCINFO("================== HS400...\n");
		ret = sunxi_execute_tuning(mmc, HS400);
		if (ret) {
			MMCINFO("tuning fail at HS400\n");
			err = -4;
			goto ERR_RET;
		}

		ret = mmc_mmc_switch_bus_mode(mmc, HSDDR52_DDR50, mmc->bus_width);
		if (ret) {
			MMCINFO("switch back to HSDDR52_DDR50 8bit fail\n");
			err = -5;
			goto ERR_RET;
		}
	}

	ret = mmc_mmc_switch_bus_mode(mmc, HSSDR52_SDR25, mmc->bus_width);
	if (ret) {
		MMCINFO("switch back to HSSDR52_SDR25 8bit fail\n");
		err = -6;
		goto ERR_RET;
	}

	sunxi_pack_tuning_result(mmc);

ERR_RET:

	return err;
}

int sunxi_switch_to_best_bus(struct mmc *mmc)
{
	int ret = 0, err;
	struct sunxi_mmc_host *host = (struct sunxi_mmc_host *)mmc->priv;
	int ifreq, imd, freq;
	int sdly = 0, dsdly = 0;
	int bus_width = 0;
	int work_mode = uboot_spare_head.boot_data.work_mode;
	char caps_bak = 0;
	ALLOC_CACHE_ALIGN_BUFFER(char, ext_csd, MMC_MAX_BLOCK_LEN);
	u32 tm = host->timing_mode;
	u8 *p = NULL, *pds=NULL;

	if (IS_SD(mmc) || (host->mmc_no == 0))
	{
		return 0;
	}
	if ((work_mode == WORK_MODE_BOOT)
		|| ((work_mode != WORK_MODE_BOOT)
			&& (host->cfg.platform_caps.sample_mode != AUTO_SAMPLE_MODE)))
	{
		char cardtype;

		caps_bak = mmc->card_caps;

		mmc->card_caps = 0;
		if (mmc_host_is_spi(mmc))
			return 0;

		/* Only version 4 supports high-speed */
		if (mmc->version < MMC_VERSION_4)
			return 0;

		/* here we assume eMMC support 8 bit */
		mmc->card_caps |= MMC_MODE_4BIT | MMC_MODE_8BIT;
#ifdef FPGA_PLATFORM
		/*just for 1718 fpga, wangwei*/
		mdelay(100);
#endif
		err = mmc_send_ext_csd(mmc, ext_csd);
		if (err) {
			MMCINFO("mmc get ext csd failed\n");
			return err;
		}

		cardtype = ext_csd[EXT_CSD_CARD_TYPE] & 0xFF;

		MMCDBG("raw card caps 0x%x\n", cardtype);

		mmc->card_caps = caps_bak;
		if ((cardtype & EXT_CSD_CARD_TYPE_HS400)
			&& (mmc->card_caps & MMC_MODE_8BIT))
			mmc->card_caps |= MMC_MODE_HS400;
		if (cardtype & EXT_CSD_CARD_TYPE_HS200)
			mmc->card_caps |= MMC_MODE_HS200;
		if (cardtype & EXT_CSD_CARD_TYPE_HS) {
			if (cardtype & EXT_CSD_CARD_TYPE_DDR_52)
				mmc->card_caps |= MMC_MODE_DDR_52MHz;
		}
	}

	MMCDBG("card caps 0x%x\n", mmc->card_caps);

	if (tm == SUNXI_MMC_TIMING_MODE_2) {
		p = &host->tm2.sdly[0];
		pds = &host->tm2.dsdly[0];
	} else if (tm == SUNXI_MMC_TIMING_MODE_4) {
		p = &host->tm4.sdly[0];
		pds = &host->tm4.dsdly[0];
	} else {
		MMCINFO("%s: err timing mode %d\n", __FUNCTION__, tm);
		goto OUT;
	}

	if ((mmc->card_caps & (MMC_MODE_HS400|MMC_MODE_8BIT))
		== (MMC_MODE_HS400|MMC_MODE_8BIT))
	{
		imd = HS400;
		for (ifreq=5; ifreq>=2; ifreq--) /*1-25MHz; 2-50MHz; 3-100MHz; 4-150MHz; 5-200MHz*/
		{
			imd = HS200_SDR104;
			sdly = p[imd*MAX_CLK_FREQ_NUM+ifreq];
			imd = HS400;
			dsdly = pds[ifreq];
			if ((sdly != 0xff) && (dsdly != 0xff))
				goto START_SWITCH;
		}
	}
	if (mmc->card_caps & MMC_MODE_HS200)
	{
		imd = HS200_SDR104;
		for (ifreq=5; ifreq>=4; ifreq--) /*1-25MHz; 2-50MHz; 3-100MHz; 4-150MHz; 5-200MHz*/
		{
			sdly = p[imd*MAX_CLK_FREQ_NUM+ifreq];
			if (sdly != 0xFF)
				goto START_SWITCH;
		}
	}
	if (mmc->card_caps & MMC_MODE_DDR_52MHz)
	{
		imd = HSDDR52_DDR50;
		for (ifreq=2; ifreq>=2; ifreq--) /*1-25MHz; 2-50MHz; 3-100MHz; 4-150MHz; 5-200MHz*/
		{
			sdly = p[imd*MAX_CLK_FREQ_NUM+ifreq];
			if (sdly != 0xFF)
				goto START_SWITCH;
		}
	}
	if (mmc->card_caps & MMC_MODE_HS_52MHz)
	{
		imd = HSSDR52_SDR25;
		for (ifreq=2; ifreq>=1; ifreq--) /*1-25MHz; 2-50MHz; 3-100MHz; 4-150MHz; 5-200MHz*/
		{
			sdly = p[imd*MAX_CLK_FREQ_NUM+ifreq];
			if (sdly != 0xFF)
				goto START_SWITCH;
		}
	}

	//imd = DS26_SDR12;
	//ifreq = CLK_25M;
	imd = HSSDR52_SDR25;
	ifreq = CLK_50M;
	MMCINFO("use default speed mode: %d-%s, ifreq: %d\n", imd, spd_name[imd], ifreq);


	//MMCINFO("don't find best speed mode and freq\n");
	//ret = -1;
	//goto OUT;

START_SWITCH:
	if ((freq = sunxi_select_freq(mmc, imd, ifreq)) != 0)
		mmc->tran_speed = freq;
	else {
		MMCINFO("%s: err freq %d-%d\n", __func__, ifreq, freq);
		ret = -1;
		goto OUT;
	}

	MMCINFO("========best spd md: %d-%s, freq: %d-%d\n", imd, spd_name[imd], ifreq, freq);

	if (mmc->card_caps & MMC_MODE_8BIT)
		bus_width = 8;
	else if (mmc->card_caps & MMC_MODE_4BIT)
		bus_width = 4;
	else
		bus_width = 1;

	/* switch to specific speed mode */
	if (imd == DS26_SDR12)
		ret = mmc_mmc_switch_bus_mode(mmc, DS26_SDR12, bus_width);
	else if (imd == HSSDR52_SDR25)
		ret = mmc_mmc_switch_bus_mode(mmc, HSSDR52_SDR25, bus_width);
	else if (imd == HSDDR52_DDR50)
		ret = mmc_mmc_switch_bus_mode(mmc, HSDDR52_DDR50, bus_width);
	else if (imd == HS200_SDR104)
		ret = mmc_mmc_switch_bus_mode(mmc, HS200_SDR104, bus_width);
	else if (imd == HS400) {
		/* firstly, switch to HS-DDR 8 bit */
		ret = mmc_mmc_switch_bus_mode(mmc, HSDDR52_DDR50, bus_width);
		if (ret) {
			MMCINFO("switch to %s fail\n", spd_name[HSDDR52_DDR50]);
			goto OUT;
		}

		/* then, switch to HS400 */
		ret = mmc_mmc_switch_bus_mode(mmc, HS400, bus_width);
	}
	if (ret) {
		MMCINFO("switch to %s fail\n", spd_name[HS400]);
		goto OUT;
	}

	mmc_set_clock(mmc, mmc->tran_speed);

OUT:

	return ret;
}

int mmc_request_update_boot0(int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);

	if (mmc == NULL)
		return 0;

	if ((mmc->cfg->platform_caps.sample_mode == AUTO_SAMPLE_MODE)
		&& (mmc->tuning_end)) {
		MMCINFO("mmc request udpate boot0\n");
		return 1;
	} else
		return 0;
}

int mmc_write_info(int dev_num, void *buffer, u32 buffer_size)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	//int work_mode = uboot_spare_head.boot_data.work_mode;
	struct boot_sdmmc_private_info_t priv_info;

	if (mmc == NULL) {
		MMCINFO("%s:Can not find mmc\n", __FUNCTION__);
		return -1;
	}

	/* this function will be called by fastboot, so delete this limitation. */
	//if (work_mode != WORK_MODE_BOOT)
	{
		memset(&priv_info, 0x0, sizeof(priv_info));

		if (mmc->cfg->platform_caps.sample_mode == AUTO_SAMPLE_MODE) {
			if ((sizeof(struct tune_sdly)+64) > buffer_size) { /* 64byte is resvered for other information */
				MMCINFO("size of tuning_sdly over %d\n", buffer_size);
			} else
				memcpy(&priv_info.tune_sdly.tm4_smx_fx[0],
					(void *)&mmc->cfg->platform_caps.sdly.tm4_smx_fx[0], sizeof(struct tune_sdly));

			/* if tuning procedure is executed successfully, set this flag. uboot will check this flag and determine it is need to
			execute tuning when boot */
			priv_info.ext_para0 = EXT_PARA0_ID;
			if (mmc->tuning_end)
				priv_info.ext_para0 |= EXT_PARA0_TUNING_SUCCESS_FLAG;
		} else {
			/* fill invalid information "0xff" */
			memset(&priv_info.tune_sdly.tm4_smx_fx[0], 0xff, sizeof(struct tune_sdly));
		}
		priv_info.boot_mmc_cfg.boot0_para = mmc->cfg->platform_caps.boot0_para;
		priv_info.boot_mmc_cfg.boot_odly_50M = mmc->cfg->platform_caps.boot_odly_50M;
		priv_info.boot_mmc_cfg.boot_sdly_50M = mmc->cfg->platform_caps.boot_sdly_50M;
		priv_info.boot_mmc_cfg.boot_odly_50M_ddr = mmc->cfg->platform_caps.boot_odly_50M_ddr;
		priv_info.boot_mmc_cfg.boot_sdly_50M_ddr = mmc->cfg->platform_caps.boot_sdly_50M_ddr;
		priv_info.boot_mmc_cfg.boot_hs_f_max = mmc->cfg->platform_caps.boot_hs_f_max;

		if (IS_SD(mmc))
			priv_info.card_type = CARD_TYPE_SD;
		else
			priv_info.card_type = CARD_TYPE_MMC;

		/*
		----- normal
		offset 0~127: boot0 struct _boot_sdcard_info_t;
		offset 128~255: struct tune_sdly, timing parameter for speed mode and frequency
		----- secure
		offset 128 ~ (224=384-160): struct tune_sdly, timing parameter for speed mode and frequency

		sizeof(priv_info)  is about 60 bytes.
		*/
		memcpy((buffer+SDMMC_PRIV_INFO_ADDR_OFFSET), (void *)&priv_info, sizeof(priv_info));

#if 0
		{
			u32 i, *p;
			p = (u32 *)(buffer+SDMMC_PRIV_INFO_ADDR_OFFSET);
			for (i=0; i<sizeof(priv_info)/4; i++)
				MMCINFO("%d %x\n", i, p[i]);
		}
#endif

		MMCINFO("write mmc info ok\n");
		return 0;
	}

	return -1;
}

