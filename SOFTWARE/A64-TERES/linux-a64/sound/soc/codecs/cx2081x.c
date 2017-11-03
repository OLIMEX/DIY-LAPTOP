/*
 * cx2081x.c  --  cx2081x ALSA Soc Audio driver
 *
 * Copyright(c) 2015-2018 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liu shaohua <liushaohua@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <linux/of.h>
#include <sound/tlv.h>
#include <linux/regulator/consumer.h>
#include <linux/io.h>


#define I2C_CHANNEL_NUM 1
#define REGULATOR_NAME  "voltage_enable"

struct i2c_client *i2c0;
struct i2c_client *i2c1;
int  regulator_en ;

struct voltage_supply {
	struct regulator *vcc3v3;
};
struct cx2081x_priv {
	struct i2c_client *i2c;
	struct snd_soc_codec *codec;
	struct voltage_supply vol_supply;
};

static const struct snd_soc_dapm_widget cx2081x_dapm_widgets[] = {

};

/* Target, Path, Source */
static const struct snd_soc_dapm_route cx2081x_audio_map[] = {
};

static const struct snd_kcontrol_new cx2081x_controls[] = {
};
#ifdef CONFIG_PM

static void hw_config(struct i2c_client *client);
static int cx2081x_suspend(struct snd_soc_codec *codec)
{
	struct cx2081x_priv *cx2081 = dev_get_drvdata(codec->dev);

	if ((regulator_en) && !(IS_ERR(cx2081->vol_supply.vcc3v3))) {
		regulator_disable(cx2081->vol_supply.vcc3v3);
		regulator_en = 0;
	}
	return 0;
}

static int cx2081x_resume(struct snd_soc_codec *codec)
{
	struct cx2081x_priv *cx2081 = dev_get_drvdata(codec->dev);

	if (!(regulator_en) && !(IS_ERR(cx2081->vol_supply.vcc3v3))) {
		regulator_enable(cx2081->vol_supply.vcc3v3);
		regulator_en = 1;
	}
	hw_config(i2c0);
	hw_config(i2c1);
	return 0;
}
#else
#define cx2081x_suspend NULL
#define cx2081x_resume NULL
#endif

static int cx2081x_probe(struct snd_soc_codec *codec)
{
	struct cx2081x_priv *cx2081x = dev_get_drvdata(codec->dev);
	int ret = 0;

	ret = snd_soc_codec_set_cache_io(codec, 8, 8, CONFIG_REGMAP_I2C);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to set cache I/O: %d\n", ret);
		return ret;
	}
	cx2081x->codec = codec;

	return 0;
}

static int cx2081x_remove(struct snd_soc_codec *codec)
{

	return 0;
}

static int cx2081x_write_i2c(u8 reg, unsigned char value,
				struct i2c_client *client);
static void hw_config(struct i2c_client *client)
{
	struct i2c_client *i2c = client ;
	/*0x10=0x00 ; Disable ADC for Setup*/
	cx2081x_write_i2c(0x10, 0x00, i2c);
	/*0x11=0x00 ; Disable ADC for Setup*/
	cx2081x_write_i2c(0x11, 0x00, i2c);
	/*0x0E=0x00 ; Gate DAC/PWM clocks*/
	cx2081x_write_i2c(0x0E, 0x00, i2c);
	/*0x16=0x00 ; Use DC Filters for ADCs*/
	cx2081x_write_i2c(0x16, 0x00, i2c);
	/*0x80=0x03 ; MCLK is an input,enable pad*/
	cx2081x_write_i2c(0x80, 0x03, i2c);
	/*0x08=0x20 ; MCLK !gated*/
	cx2081x_write_i2c(0x08, 0x20, i2c);
	/*0x09=0x03 ; Use MLCK directly*/
	cx2081x_write_i2c(0x09, 0x03, i2c);
	/*0x0A=0x00 for TX_clk divisor  ;*/
	cx2081x_write_i2c(0x0A, 0x02, i2c);
	cx2081x_write_i2c(0x0A, 0x82, i2c);
	/*0x0C=0x0A ; Slave Mode, ADC3/4 FIFO and I2S TX Clocks !gated*/
	cx2081x_write_i2c(0x0C, 0x0A, i2c);

	/*
	0x78=0x2D ; Enable VREF @ 2.8V (5V) or 2.6V (3.3V)
	*/
	cx2081x_write_i2c(0x78, 0x2d, i2c);
	/*
	0x78=0x6D ; Enable Analog LDO
	*/
	cx2081x_write_i2c(0x78, 0x6d, i2c);
	/*
	0x7A=0x01 ; Enable VREFP
	*/
	cx2081x_write_i2c(0x7A, 0x01, i2c);

	/*0x30=0x0B ; 7-Wire Mode, 16-bit, DSP/TDM Mode*/
	cx2081x_write_i2c(0x30, 0x0B, i2c);
	/*0x31=0x1F : TX 256 clocks per frame */
	cx2081x_write_i2c(0x31, 0x1F, i2c);
	/*0x32=0x07 : RX 64 clocks per frame*/
	cx2081x_write_i2c(0x32, 0x07, i2c);
	/*0x39=0x0A ; DSP_DSTART_DLY, MSB First, DSP_TX_OUT_LINE_SEL (all data on TX_DATA_1)*/
	cx2081x_write_i2c(0x39, 0x0A, i2c);
	if (i2c0 == i2c) {
		/*0x3A=0x00 ; ADC1 starts at 0*/
		cx2081x_write_i2c(0x3A, 0x00, i2c);

		/*0x3B=0x03 ; ADC2 starts at 2*/
		cx2081x_write_i2c(0x3B, 0x02, i2c);

		/*0x3C=0x06 ; ADC3 starts at 4*/
		cx2081x_write_i2c(0x3C, 0x04, i2c);

		/*0x3D=0x09 ; ADC4 starts at 6*/
		cx2081x_write_i2c(0x3D, 0x06, i2c);

		/*0xBF=0x0f ; ADC4 PGA gain set */
		cx2081x_write_i2c(0xBF, 0x0f, i2c);

	} else if (i2c1 == i2c) {
		/*0x3A=0x00 ; ADC1 starts at 0*/
		cx2081x_write_i2c(0x3A, 0x08, i2c);

		/*0x3B=0x03 ; ADC2 starts at 3*/
		cx2081x_write_i2c(0x3B, 0x0a, i2c);

		/*0x3C=0x06 ; ADC3 starts at 6*/
		cx2081x_write_i2c(0x3C, 0x0c, i2c);

		/*0x3D=0x09 ; ADC4 starts at 9*/
		cx2081x_write_i2c(0x3D, 0x0e, i2c);

		/*0xBF = 0x3f ; ADC4 PGA gain set*/
		cx2081x_write_i2c(0xBF, 0x3f, i2c);
	}

	/*0x3E=0x6F ; Enable TX1-4 and Ch4 is Last of Frame*/
	cx2081x_write_i2c(0x3E, 0x6F, i2c);

	/*0x3F=0x00 ;*/
	cx2081x_write_i2c(0x3F, 0x00, i2c);
	/*0x83=0x0F ; TX_CLK, TX_LRCK are Inputs; TX_DATA_1 is Output*/
	cx2081x_write_i2c(0x83, 0x0F, i2c);
	/*0xBC=0x0C ; ADC1 6dB Gain*/
	cx2081x_write_i2c(0xBC, 0x2b, i2c);
	/*0xBD=0x0C ; ADC2 6dB Gain*/
	cx2081x_write_i2c(0xBD, 0x2b, i2c);
	/*0xBE=0x0C ; ADC3 6dB Gain*/
	cx2081x_write_i2c(0xBE, 0x2b, i2c);
	/*0xBF=0x0C ; ADC4 6dB Gain*/
	cx2081x_write_i2c(0xBF, 0x2b, i2c);
	/*0xA2=0x18 ; Invert Ch1 DSM Clock, Output on Rising Edge*/
	cx2081x_write_i2c(0xA2, 0x18, i2c);
	/*0xA9=0x18 ; Ch2*/
	cx2081x_write_i2c(0xA9, 0x18, i2c);
	/*0xB0=0x18 ; Ch3*/
	cx2081x_write_i2c(0xB0, 0x18, i2c);
	/*0xB7=0x18 ; Ch4*/
	cx2081x_write_i2c(0xB7, 0x18, i2c);

	/*0x11=0x1F ; Enable all ADCs and set 16kHz sample rate*/
	cx2081x_write_i2c(0x11, 0x1F, i2c);
	/*0x10=0x5F ; Enable all ADC clocks, ADC digital and ADC Mic Clock Gate*/
	cx2081x_write_i2c(0x10, 0x5F, i2c);
	/*0xA0=0x0E ; ADC1, Mute PGA, enable AAF/ADC/PGA*/
	cx2081x_write_i2c(0xA0, 0x0E, i2c);
	/*0xA7=0x0E ; ADC2, Mute PGA, enable AAF/ADC/PGA*/
	cx2081x_write_i2c(0xA7, 0x0E, i2c);
	/*0xAE=0x0E ; ADC3, Mute PGA, enable AAF/ADC/PGA*/
	cx2081x_write_i2c(0xAE, 0x0E, i2c);
	/*0xB5=0x0E ; ADC4, Mute PGA, enable AAF/ADC/PGA*/
	cx2081x_write_i2c(0xB5, 0x0E, i2c);
	/*0xA0=0x06 ; ADC1 !Mute*/
	cx2081x_write_i2c(0xA0, 0x07, i2c);
	/*0xA7=0x06 ; ADC2 !Mute*/
	cx2081x_write_i2c(0xA7, 0x07, i2c);
	/*0xAE=0x06 ; ADC3 !Mute*/
	cx2081x_write_i2c(0xAE, 0x07, i2c);
	/*0xB5=0x06 ; ADC4 !Mute*/
	cx2081x_write_i2c(0xB5, 0x07, i2c);

}
static int cx2081x_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	u16 blen;

	hw_config(i2c0);
	hw_config(i2c1);

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		blen = 0x0;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		blen = 0x1;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		blen = 0x2;
		break;
	default:
		dev_err(dai->dev, "Unsupported word length: %u\n",
			params_format(params));
		return -EINVAL;
	}

	return 0;
}

static int cx2081x_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	return 0;
}
static int cx2081x_set_sysclk(struct snd_soc_dai *dai,
			     int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int cx2081x_set_clkdiv(struct snd_soc_dai *dai,
			     int div_id, int div)
{
	return 0;
}

static int cx2081x_set_pll(struct snd_soc_dai *dai, int pll_id,
			  int source, unsigned int freq_in,
			  unsigned int freq_out)
{
	return 0;
}


static int cx2081x_hw_free(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	return 0;
}

static const struct snd_soc_dai_ops cx2081x_dai_ops = {
	.hw_params = cx2081x_hw_params,
	.set_fmt = cx2081x_set_fmt,
	.set_sysclk = cx2081x_set_sysclk,
	.set_clkdiv = cx2081x_set_clkdiv,
	.set_pll = cx2081x_set_pll,
	.hw_free = cx2081x_hw_free,
};

#define CX2081X_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE)

#define CX2081X_RATES SNDRV_PCM_RATE_8000_96000
static struct snd_soc_dai_driver cx2081x_dai0 = {
		.name = "cx2081x-pcm0",
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = CX2081X_RATES,
			.formats = CX2081X_FORMATS,
		},
		.ops = &cx2081x_dai_ops,
};

static struct snd_soc_dai_driver cx2081x_dai1 = {
		.name = "cx2081x-pcm1",
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = CX2081X_RATES,
			.formats = CX2081X_FORMATS,
		},
		.ops = &cx2081x_dai_ops,
};

static struct snd_soc_codec_driver soc_codec_dev_cx2081x = {
	.probe = cx2081x_probe,
	.remove = cx2081x_remove,
	.suspend = cx2081x_suspend,
	.resume = cx2081x_resume,

	.dapm_widgets = cx2081x_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(cx2081x_dapm_widgets),
	.dapm_routes = cx2081x_audio_map,
	.num_dapm_routes = ARRAY_SIZE(cx2081x_audio_map),
	.controls = cx2081x_controls,
	.num_controls = ARRAY_SIZE(cx2081x_controls),
};

/***************************************************************************/
static int cx2081x_read_i2c(u8 reg, unsigned char *rt_value,
			struct i2c_client *client)
{
	int ret;
	u8 read_cmd[3] = {0};
	u8 cmd_len = 0;
	read_cmd[0] = reg;
	cmd_len = 1;
	if (client->adapter == NULL)
		pr_err("cx2081x_read_i2c client->adapter==NULL\n");
	ret = i2c_master_send(client, read_cmd, cmd_len);
	if (ret != cmd_len) {
		pr_err("cx2081x_read_i2c error1\n");
		return -1;
	}
	ret = i2c_master_recv(client, rt_value, 1);
	if (ret != 1) {
		pr_err("cx2081x_read_i2c error2, ret = %d.\n", ret);
		return -1;
	}

	return 0;
}

static int cx2081x_write_i2c(u8 reg, unsigned char value,
				struct i2c_client *client)
{
	int ret = 0;
	u8 write_cmd[2] = {0};
	write_cmd[0] = reg;
	write_cmd[1] = value;
	ret = i2c_master_send(client, write_cmd, 2);
	if (ret != 2) {
		pr_err("cx2081x_write_i2c error\n");
		return -1;
	}
	return 0;
}

static ssize_t cx2081x_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int val = 0, flag = 0;
	u8 reg, num, i = 0;
	u8 value_w, value_r[256];
	struct cx2081x_priv *cx2081x = dev_get_drvdata(dev);
	val = simple_strtol(buf, NULL, 16);
	flag = (val >> 16) & 0xF;
	if (flag) {
		reg = (val >> 8) & 0xFF;
		value_w = val & 0xFF;
		cx2081x_write_i2c(reg, value_w, cx2081x->i2c);
		printk("write 0x%x to reg:0x%x\n", value_w, reg);
	} else {
		reg = (val >> 8) & 0xFF;
		num = val & 0xff;
		printk("\nread:start add:0x%x,count:0x%x\n", reg, num);
		do {
			cx2081x_read_i2c(reg, &value_r[i], cx2081x->i2c);
			printk("0x%x: 0x%x ", reg, value_r[i]);
			reg += 1;
			i++;
			if (i == num)
				printk("\n");
			if (i % 4 == 0)
				printk("\n");
		} while (i < num);
	}
	return count;
}
static ssize_t cx2081x_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE,
			"echo flag|reg|val > cx2081x\n"
			"eg read star addres=0x06,count 0x10:echo 0610 >cx2081x\n"
			"eg write value:0xfe to address:0x06 :echo 106fe > cx2081x\n");
}
static DEVICE_ATTR(cx2081x, 0644, cx2081x_show, cx2081x_store);

static struct attribute *audio_debug_attrs[] = {
	&dev_attr_cx2081x.attr,
	NULL,
};

static struct attribute_group audio_debug_attr_group = {
	.name   = "cx2081x_debug",
	.attrs  = audio_debug_attrs,
};

/*****************************************************/
static int cx2081x_i2c_probe(struct i2c_client *i2c,
				      const struct i2c_device_id *i2c_id)
{
	struct cx2081x_priv *cx2081x;
	struct device_node *np = i2c->dev.of_node;
	char *regulator_name = NULL;
	int ret = 0;
	cx2081x = devm_kzalloc(&i2c->dev, sizeof(struct cx2081x_priv),
			      GFP_KERNEL);
	if (cx2081x == NULL) {
		dev_err(&i2c->dev, "Unable to allocate private data\n");
		return -ENOMEM;
	} else
		dev_set_drvdata(&i2c->dev, cx2081x);
	cx2081x->i2c = i2c;
	if (!regulator_en) {
		ret = of_property_read_string(np, REGULATOR_NAME, &regulator_name);
		if (ret) {
			pr_err("get regulator name failed \n");
		} else {
			cx2081x->vol_supply.vcc3v3 = regulator_get(NULL, regulator_name);
			if (IS_ERR(cx2081x->vol_supply.vcc3v3)) {
				pr_err("get audio audio-3v3 failed\n");
				return -EFAULT;
			}
			regulator_set_voltage(cx2081x->vol_supply.vcc3v3, 3300000, 3300000);
			regulator_enable(cx2081x->vol_supply.vcc3v3);
			regulator_en = 1;
		}
	}
	if (i2c_id->driver_data == 0) {
		ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_cx2081x, &cx2081x_dai0, 1);
		i2c0 = i2c;
	} else if (i2c_id->driver_data == 1) {
		ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_cx2081x, &cx2081x_dai1, 1);
		i2c1 = i2c;
	} else
		pr_err("The wrong i2c_id number :%d\n", i2c_id->driver_data);
	if (ret < 0) {
		dev_err(&i2c->dev, "Failed to register cx2081x: %d\n", ret);
	}
	ret = sysfs_create_group(&i2c->dev.kobj, &audio_debug_attr_group);
	if (ret) {
		pr_err("failed to create attr group\n");
	}
	return ret;
}

static int cx2081x_i2c_remove(struct i2c_client *i2c)
{
	snd_soc_unregister_codec(&i2c->dev);
	return 0;
}

static struct i2c_board_info cx2081x_i2c_board_info[] = {
	{I2C_BOARD_INFO("cx2081x_0", 0x35),	},
	{I2C_BOARD_INFO("cx2081x_1", 0x3b),	},
};

static const struct i2c_device_id cx2081x_i2c_id[] = {
	{ "cx2081x_0", 0 },
	{ "cx2081x_1", 1 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, cx2081x_i2c_id);

static const struct of_device_id cx2081x_dt_ids[] = {
	{ .compatible = "cx2081x_0", },
	{ .compatible = "cx2081x_1",},
};
MODULE_DEVICE_TABLE(of, cx2081x_dt_ids);

static struct i2c_driver cx2081x_i2c_driver = {
	.driver = {
		.name = "cx2081x",
		.owner = THIS_MODULE,
		.of_match_table = cx2081x_dt_ids,
	},
	.probe = cx2081x_i2c_probe,
	.remove = cx2081x_i2c_remove,
	.id_table = cx2081x_i2c_id,
};

static int __init cx2081x_init(void)
{
	int ret ;
	ret = i2c_add_driver(&cx2081x_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register cx2081x i2c driver : &d \n", ret);

}
module_init(cx2081x_init);

static void __exit cx2081x_exit(void)
{
	i2c_del_driver(&cx2081x_i2c_driver);
}
module_exit(cx2081x_exit);

MODULE_DESCRIPTION("ASoC cx2081x driver");
MODULE_AUTHOR("liu shaohua <liushaohua@allwinnertech.com>");
MODULE_LICENSE("GPL");

