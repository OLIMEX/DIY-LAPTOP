/*
 * sound\soc\sunxi\sunxi_dma.c
 * (C) Copyright 2014-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@Reuuimllatech.com>
 * Liu shaohua <liushaohua@allwinnertech.com>
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma/sunxi-dma.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/dmaengine_pcm.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <asm/dma.h>
#include "sunxi_dma.h"
#include "sunxi_tdm_utils.h"

static int raw_flag = 1;
static dma_addr_t hdmiraw_dma_addr = 0;
static dma_addr_t hdmipcm_dma_addr = 0;
static unsigned char *hdmiraw_dma_area;	/* DMA area */
static unsigned int channel_status[192];

static u64 sunxi_pcm_mask = DMA_BIT_MASK(32);

typedef struct headbpcuv{
	unsigned other:3;
    unsigned V:1;
    unsigned U:1;
    unsigned C:1;
    unsigned P:1;
    unsigned B:1;
} headbpcuv;

union head61937
{
 headbpcuv head0;
 unsigned char head1;
}head;

typedef union word
{
	struct
	{
		unsigned int bit0:1;
		unsigned int bit1:1;
		unsigned int bit2:1;
		unsigned int bit3:1;
		unsigned int bit4:1;
		unsigned int bit5:1;
		unsigned int bit6:1;
		unsigned int bit7:1;
		unsigned int bit8:1;
		unsigned int bit9:1;
		unsigned int bit10:1;
		unsigned int bit11:1;
		unsigned int bit12:1;
		unsigned int bit13:1;
		unsigned int bit14:1;
		unsigned int bit15:1;
		unsigned int rsvd:16;
	}bits;
	unsigned int wval;
}word_format;
static const struct snd_pcm_hardware sunxi_pcm_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_BLOCK_TRANSFER |
				      SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID |
				      SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME,
	.formats		= SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
	.rates			= SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT,
	.rate_min		= 8000,
	.rate_max		= 192000,
	.channels_min		= 1,
	.channels_max		= 8,
	.buffer_bytes_max	= 1024*1024,    /* value must be (2^n)Kbyte size */
	.period_bytes_min	= 256,
	.period_bytes_max	= 1024*256,
	.periods_min		= 1,
	.periods_max		= 8,
	.fifo_size		= 128,
};
int hdmi_transfer_format_61937_to_60958(int *out,short* temp, int samples)
{
	int ret =0;
	int i;
	static int numtotal = 0;
	union word w1;

	samples>>=1;
	head.head0.other = 0;
	head.head0.B = 1;
	head.head0.P = 0;
	head.head0.C = 0;
	head.head0.U = 0;
	head.head0.V = 1;

	for (i=0 ; i<192; i++)
	{
		channel_status[i] = 0;
	}
	channel_status[1] = 1;
	//sample rates
	channel_status[24] = 0;
	channel_status[25] = 1;
	channel_status[26] = 0;
	channel_status[27] = 0;

	for (i = 0 ;i<samples;i++,numtotal++) {
		if( (numtotal%384 == 0) || (numtotal%384 == 1) )
		{
			head.head0.B = 1;
		}
		else
		{
			head.head0.B = 0;
		}
		head.head0.C = channel_status[(numtotal%384)/2];

		if(numtotal%384 == 0)
		{
			numtotal = 0;
		}

		w1.wval = (*temp)&(0xffff);

		head.head0.P = w1.bits.bit15 ^ w1.bits.bit14 ^ w1.bits.bit13 ^ w1.bits.bit12
		              ^w1.bits.bit11 ^ w1.bits.bit10 ^ w1.bits.bit9 ^ w1.bits.bit8
		              ^w1.bits.bit7 ^ w1.bits.bit6 ^ w1.bits.bit5 ^ w1.bits.bit4
		              ^w1.bits.bit3 ^ w1.bits.bit2 ^ w1.bits.bit1 ^ w1.bits.bit0;

		ret = (int)(head.head1)<<24;
		ret |= (int)((w1.wval)&(0xffff))<<11;//8 or 12
		*out = ret;
		out++;
		temp++;
	}
	return 0;
}

static int sunxi_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct sunxi_dma_params *dmap;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct device *dev = rtd->platform->dev;
	struct dma_chan *chan = snd_dmaengine_pcm_get_chan(substream);
	struct dma_slave_config slave_config;
	int ret;

	dmap = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	ret = snd_hwparams_to_dma_slave_config(substream, params, &slave_config);
	if (ret) {
		dev_err(dev, "hw params config failed with err %d\n", ret);
		return ret;
	}
		if (SNDRV_PCM_FORMAT_S8 == params_format(params)) {
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	} else if (SNDRV_PCM_FORMAT_S16_LE == params_format(params)) {
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	} else {
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	}
	slave_config.dst_maxburst = dmap->dst_maxburst;
	slave_config.src_maxburst = dmap->src_maxburst;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		slave_config.dst_addr = dmap->dma_addr;
		slave_config.slave_id = sunxi_slave_id(dmap->dma_drq_type_num, DRQSRC_SDRAM);
	} else {
		slave_config.src_addr =	dmap->dma_addr;
		slave_config.slave_id = sunxi_slave_id(DRQDST_SDRAM, dmap->dma_drq_type_num);
	}

	ret = dmaengine_slave_config(chan, &slave_config);
	if (ret < 0) {
		dev_err(dev, "dma slave config failed with err %d\n", ret);
		return ret;
	}

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	return 0;
}


static int sunxi_pcm_hdmi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{

	struct sunxi_dma_params *dmap;
	struct snd_soc_dai *cpu_dai 	= NULL;
	struct sunxi_tdm_info  *sunxi_tdmhdmi = NULL;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct device *dev = rtd->platform->dev;
	struct dma_chan *chan = snd_dmaengine_pcm_get_chan(substream);
	struct dma_slave_config slave_config;
	int ret;
	cpu_dai 	= rtd->cpu_dai;
	sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);

	raw_flag = sunxi_tdmhdmi->others;
	dmap = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	ret = snd_hwparams_to_dma_slave_config(substream, params, &slave_config);
	if (ret) {
		dev_err(dev, "hw params config failed with err %d\n", ret);
		return ret;
	}
	if (SNDRV_PCM_FORMAT_S8 == params_format(params)) {
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	} else if (SNDRV_PCM_FORMAT_S16_LE == params_format(params)) {
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	} else {
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	}
	slave_config.dst_addr = dmap->dma_addr;
	slave_config.dst_maxburst = dmap->dst_maxburst;
	slave_config.src_maxburst = dmap->src_maxburst;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		slave_config.slave_id = sunxi_slave_id(dmap->dma_drq_type_num, DRQSRC_SDRAM);
	} else {
		slave_config.slave_id = sunxi_slave_id(DRQDST_SDRAM, dmap->dma_drq_type_num);
	}
	/*raw_flag>1. rawdata*/
	if (raw_flag > 1) {
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		strcpy(substream->pcm->card->id, "sndhdmiraw");
		if (!dev->dma_mask)
			dev->dma_mask = &sunxi_pcm_mask;
		if (!dev->coherent_dma_mask)
			dev->coherent_dma_mask = 0xffffffff;

		hdmiraw_dma_area = dma_alloc_coherent(dev, (2*params_buffer_bytes(params)), &hdmiraw_dma_addr, GFP_KERNEL);
		if (hdmiraw_dma_area == NULL) {
			pr_err("hdmi:raw:get mem failed...\n");
			return -ENOMEM;
		}
		hdmipcm_dma_addr = substream->dma_buffer.addr;
		substream->dma_buffer.addr = (dma_addr_t)hdmiraw_dma_addr;
	} else {
		strcpy(substream->pcm->card->id, "sndhdmi");
	}
	ret = dmaengine_slave_config(chan, &slave_config);
	if (ret < 0) {
		dev_err(dev, "dma slave config failed with err %d\n", ret);
		return ret;
	}

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	return 0;
}

static int sunxi_pcm_hdmi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct device *dev = rtd->platform->dev;
	if (snd_pcm_lib_buffer_bytes(substream)&& (raw_flag > 1)) {
		dma_free_coherent(dev, (2*snd_pcm_lib_buffer_bytes(substream)),
					      hdmiraw_dma_area, hdmiraw_dma_addr);
		substream->dma_buffer.addr = hdmipcm_dma_addr;
		hdmiraw_dma_area = NULL;
	}
	snd_pcm_set_runtime_buffer(substream, NULL);

	return 0;
}
static int sunxi_pcm_hw_free(struct snd_pcm_substream *substream)
{
	snd_pcm_set_runtime_buffer(substream, NULL);

	return 0;
}

static int sunxi_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			snd_dmaengine_pcm_trigger(substream, SNDRV_PCM_TRIGGER_START);
		return 0;
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			snd_dmaengine_pcm_trigger(substream, SNDRV_PCM_TRIGGER_STOP);
		return 0;
		}
	} else {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			snd_dmaengine_pcm_trigger(substream, SNDRV_PCM_TRIGGER_START);
		return 0;
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			snd_dmaengine_pcm_trigger(substream, SNDRV_PCM_TRIGGER_STOP);
		return 0;
		}
	}
	return 0;
}

static int sunxi_pcm_open(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct device *dev = rtd->platform->dev;
	/* Set HW params now that initialization is complete */
	snd_soc_set_runtime_hwparams(substream, &sunxi_pcm_hardware);
	ret = snd_pcm_hw_constraint_integer(substream->runtime,
					    SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0)
		return ret;
	ret = snd_dmaengine_pcm_open_request_chan(substream, NULL,
				NULL);
	if (ret) {
		dev_err(dev, "dmaengine pcm open failed with err %d\n", ret);
	}

	return 0;
}

static int sunxi_pcm_mmap(struct snd_pcm_substream *substream,
	struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = NULL;
	if (substream->runtime!=NULL) {
		runtime = substream->runtime;

		return dma_mmap_writecombine(substream->pcm->card->dev, vma,
					     runtime->dma_area,
					     runtime->dma_addr,
					     runtime->dma_bytes);
	} else {
		return -1;
	}

}

static int sunxi_pcm_copy(struct snd_pcm_substream *substream, int a,
	 snd_pcm_uframes_t hwoff, void __user *buf, snd_pcm_uframes_t frames)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, hwoff);
		if (copy_from_user(hwbuf, buf, frames_to_bytes(runtime, frames))) {
			return -EFAULT;
		}
		if (raw_flag > 1) {
			char* hdmihw_area = hdmiraw_dma_area + 2*frames_to_bytes(runtime, hwoff);
			hdmi_transfer_format_61937_to_60958((int*)hdmihw_area, (short*)hwbuf, frames_to_bytes(runtime, frames));
		}
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, hwoff);
		if (copy_to_user(buf, hwbuf, frames_to_bytes(runtime, frames))) {
			return -EFAULT;
		}
	}

	return ret;
}

static struct snd_pcm_ops sunxi_pcm_ops = {
	.open			= sunxi_pcm_open,
	.close			= snd_dmaengine_pcm_close_release_chan,
	.ioctl			= snd_pcm_lib_ioctl,
	.hw_params		= sunxi_pcm_hw_params,
	.hw_free		= sunxi_pcm_hw_free,
	.trigger		= sunxi_pcm_trigger,
	.pointer		= snd_dmaengine_pcm_pointer,
	.mmap			= sunxi_pcm_mmap,
};

static struct snd_pcm_ops sunxi_pcm_ops_no_residue = {
	.open			= sunxi_pcm_open,
	.close			= snd_dmaengine_pcm_close_release_chan,
	.ioctl			= snd_pcm_lib_ioctl,
	.hw_params		= sunxi_pcm_hdmi_hw_params,
	.hw_free		= sunxi_pcm_hdmi_hw_free,
	.trigger		= sunxi_pcm_trigger,
	.pointer		= snd_dmaengine_pcm_pointer_no_residue,
	.mmap			= sunxi_pcm_mmap,
	.copy			= sunxi_pcm_copy,
};
static int sunxi_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = 0;
	size = sunxi_pcm_hardware.buffer_bytes_max;

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_coherent(pcm->card->dev, size,
					   &buf->addr, GFP_KERNEL);
	if (!buf->area)
		return -ENOMEM;
	buf->bytes = size;

	return 0;
}

static void sunxi_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_coherent(pcm->card->dev, buf->bytes,
				      			buf->area, buf->addr);
		buf->area = NULL;
	}
}


static int sunxi_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &sunxi_pcm_mask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = 0xffffffff;

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = sunxi_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = sunxi_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
	}
 	out:
		return ret;
}

static struct snd_soc_platform_driver sunxi_soc_platform = {
	.ops		= &sunxi_pcm_ops,
	.pcm_new	= sunxi_pcm_new,
	.pcm_free	= sunxi_pcm_free_dma_buffers,
};

static const struct snd_soc_platform_driver sunxi_soc_platform_no_residue = {
	.ops		= &sunxi_pcm_ops_no_residue,
	.pcm_new	= sunxi_pcm_new,
	.pcm_free	= sunxi_pcm_free_dma_buffers,
};
int asoc_dma_platform_register(struct device *dev,unsigned int flags)
{
	if (flags & SND_DMAENGINE_PCM_FLAG_NO_RESIDUE)
		return snd_soc_register_platform(dev, &sunxi_soc_platform_no_residue);
	else
		return snd_soc_register_platform(dev, &sunxi_soc_platform);
}
EXPORT_SYMBOL_GPL(asoc_dma_platform_register);

void asoc_dma_platform_unregister(struct device *dev)
{
	snd_soc_unregister_platform(dev);
}
EXPORT_SYMBOL_GPL(asoc_dma_platform_unregister);

MODULE_AUTHOR("huangxin, liushaohua");
MODULE_DESCRIPTION("sunxi ASoC DMA Driver");
MODULE_LICENSE("GPL");
