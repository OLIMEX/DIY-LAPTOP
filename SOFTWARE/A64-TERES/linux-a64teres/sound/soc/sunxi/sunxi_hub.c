#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/io.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <asm/dma.h>
#include <linux/dma/sunxi-dma.h>
#include "sunxi_dma.h"
#include "sunxi_tdm_utils.h"
#include "sunxi_hub.h"
#include "sunxi_spdif.h"
#include "sunxi_tdm_utils.h"

extern void sndhdmi_shutdown(struct snd_pcm_substream *substream,	struct snd_soc_dai *dai);
extern int sndhdmi_perpare(struct snd_pcm_substream *substream,	struct snd_soc_dai *dai);
extern int sndhdmi_hw_params(struct snd_pcm_substream *substream,	struct snd_pcm_hw_params *params,struct snd_soc_dai *dai);
static int sunxi_hubhdmi_hwfree(struct snd_pcm_substream *substream )
{
	int reg_val;
	struct snd_soc_pcm_runtime *rtd = NULL;
	struct snd_soc_dai *cpu_dai 	= NULL;
	struct sunxi_tdm_info  *sunxi_tdmhdmi = NULL;
	if (!substream) {
		pr_err("error:%s,line:%d\n", __func__, __LINE__);
		return -EAGAIN;
	}
	rtd 		= substream->private_data;
	cpu_dai 	= rtd->cpu_dai;
	sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);

	reg_val = readl(sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);
	reg_val &= ~SUNXI_DAUDIOFCTL_HUBEN;
	writel(reg_val, sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);

	return 0;
}

static int sunxi_hubhdmi_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	int ret = 0;
	int reg_val;
	struct snd_soc_pcm_runtime *rtd = NULL;
	struct snd_soc_dai *cpu_dai 	= NULL;
	struct sunxi_tdm_info  *sunxi_tdmhdmi = NULL;

	u32 freq = 22579200;
	unsigned long sample_rate = params_rate(params);
	if (!substream) {
		pr_err("error:%s,line:%d\n", __func__, __LINE__);
		return -EAGAIN;
	}
	rtd 		= substream->private_data;
	cpu_dai 	= rtd->cpu_dai;
	sunxi_tdmhdmi = snd_soc_dai_get_drvdata(cpu_dai);
	sunxi_tdmhdmi->others = 1;/*pcm*/
	switch (sample_rate) {
		case 8000:
		case 16000:
		case 32000:
		case 64000:
		case 128000:
		case 12000:
		case 24000:
		case 48000:
		case 96000:
		case 192000:
			freq = 24576000;
			break;
	}

	/*set system clock source freq and set the mode as i2s0 or pcm*/
	ret = snd_soc_dai_set_sysclk(cpu_dai, 0 , freq, 0);
	if (ret < 0) {
		return ret;
	}

	/*
	* I2S mode \normal bit clock + frame\codec clk & FRM slave
	*/
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(cpu_dai, 0, sample_rate);
	if (ret < 0) {
		return ret;
	}
	reg_val = readl(sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);
	reg_val |= SUNXI_DAUDIOFCTL_HUBEN;
	writel(reg_val, sunxi_tdmhdmi->regs + SUNXI_DAUDIOFCTL);

	sndhdmi_hw_params(substream,params,NULL);
	sndhdmi_perpare(substream,NULL);
	return 0;
}
void sunxi_hubhdmi_shutdown(struct snd_pcm_substream *substream)
{
	sndhdmi_shutdown(substream,NULL);
}
static struct snd_soc_ops sunxi_hubhdmi_ops = {
	.hw_params 	= sunxi_hubhdmi_hw_params,
	.hw_free	= sunxi_hubhdmi_hwfree,
	.shutdown	= sunxi_hubhdmi_shutdown,
};
static int sunxi_hubspdif_hwfree(struct snd_pcm_substream *substream )
{
	int reg_val;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(cpu_dai);

	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	reg_val |= SUNXI_SPDIFFCTL_HUBEN;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	return 0;
}
static int sunxi_hubspdif_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret = 0,reg_val = 0;
	unsigned long rate = params_rate(params);
	struct sunxi_spdif_info *sunxi_spdif = snd_soc_dai_get_drvdata(cpu_dai);
	unsigned int fmt = 1;
	u32 mclk_div=0, mpll=0, bclk_div=0, mult_fs=0;

	get_clock_divder(rate, 32, &mclk_div, &mpll, &bclk_div, &mult_fs);

	if (ret < 0)
		return ret;
	//fmt = 1;
	if(fmt > 1){
		fmt = 1;
	}else{
		fmt = 0;
	}

	ret = snd_soc_dai_set_fmt(cpu_dai, fmt);//0:pcm,1:raw data
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(cpu_dai, 0 , mpll, 0);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(cpu_dai, SUNXI_DIV_MCLK, mclk_div);
	if (ret < 0)
		return ret;
	reg_val = readl(sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	reg_val |= SUNXI_SPDIFFCTL_HUBEN;
	writel(reg_val, sunxi_spdif->regs + SUNXI_SPDIF_FCTL);
	return 0;
}
static struct snd_soc_ops sunxi_hubspdif_ops = {
	.hw_params 	= sunxi_hubspdif_hw_params,
	.hw_free	= sunxi_hubspdif_hwfree,
};

/*dailink for hub*/
struct snd_soc_dai_link sunxi_hub_dai_link[2] = {
	{
	.name = "hub-hdmi",
	.stream_name 	= "hub-hdmi",
	.cpu_dai_name 	= "1c22800.daudio",
	.codec_dai_name = "snd-soc-dummy-dai",
	.codec_name 	= "snd-soc-dummy",
    .ops = &sunxi_hubhdmi_ops,
    .ignore_suspend = 1,

	},
/**/
	{
	.name = "hub-spdif",
	.stream_name = "hub-spdif",
	.cpu_dai_name = "1c21000.spdif-controller",
	.codec_dai_name = "snd-soc-dummy-dai",
	.codec_name = "snd-soc-dummy",
	.ops = &sunxi_hubspdif_ops,
	.ignore_suspend = 1,
	},
/**/
};

