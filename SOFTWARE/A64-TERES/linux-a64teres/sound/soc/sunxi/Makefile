# SUNXI platform Support

snd-sunxi-soc-objs := sunxi_dma.o
snd-sunxi-soc-objs += codec_utils.o
ifneq ($(CONFIG_SND_SOC_SUNXI_RW),)
snd-sunxi-soc-objs += sunxi_rw_func.o
endif
ifneq ($(CONFIG_SND_SOC_SUNXI_TDM),)
snd-sunxi-soc-objs += sunxi_tdm_utils.o
endif
ifneq ($(CONFIG_SND_SOC_HUB),)
snd-sunxi-soc-objs += sunxi_hub.o
endif
obj-y += snd-sunxi-soc.o

#audiocodec
#sun50i
obj-$(CONFIG_SND_SOC_INTERNAL_AUDIOCODEC) += sunxi_codec.o
obj-$(CONFIG_SND_SOC_INTERNAL_I2S) += sunxi_i2s.o

#sun8iw10
obj-$(CONFIG_SND_SOC_INTERNAL_SUN8IW10_AUDIOCODEC) += sun8iw10_codec.o
#sun8iw11
obj-$(CONFIG_SND_SOC_INTERNAL_SUN8IW11_AUDIOCODEC) += sun8iw11_codec.o
#sun8iw10 sun8iw11
obj-$(CONFIG_SND_SOC_SUNXI_CODEC_CPU_DAI) += sunxi_cpudai.o

#daudio
obj-$(CONFIG_SND_SOC_VIRCODEC) += snddaudio.o
obj-$(CONFIG_SND_SOC_DAUDIO_PLATFORM) += sunxi_daudio.o

#dmic
obj-$(CONFIG_SND_SOC_DMIC) += sunxi_dmic.o

#dsd
obj-$(CONFIG_SND_SOC_DSD) += sunxi_dsd.o
obj-$(CONFIG_SND_SOC_CS4385) += cs4385.o

#hdmi
obj-$(CONFIG_SND_SUNXI_SOC_HDMIAUDIO) += sunxi_tdmhdmi.o
obj-$(CONFIG_SND_SUNXI_SOC_HDMIAUDIO) += sndhdmi.o

#spdif
obj-$(CONFIG_SND_SUNXI_SOC_SPDIF) += sunxi_spdif.o

#for sound machine
#sun50i
obj-$(CONFIG_SND_SOC_AUDIO_CODEC_MACHINE) += sunxi_sndcodec.o
#sun8iw10
obj-$(CONFIG_SND_SOC_SUN8IW10_AUDIO_CODEC_MACHINE) += sun8iw10_sndcodec.o
#sun8iw11
obj-$(CONFIG_SND_SOC_SUN8IW10_AUDIO_CODEC_MACHINE) += sun8iw11_sndcodec.o

#daudio0
obj-$(CONFIG_SND_SOC_DAUDIO0_MACHINE) += sunxi-snddaudio0.o

#daudio1
obj-$(CONFIG_SND_SOC_DAUDIO1_MACHINE) += sunxi-snddaudio1.o

#dmic
obj-$(CONFIG_SND_SOC_DMIC) += sunxi-snddmic.o

#dsd
obj-$(CONFIG_SND_SOC_DSD) += sunxi-snddsd.o

#hdmi
obj-$(CONFIG_SND_SUNXI_SOC_HDMIAUDIO) += sunxi-sndhdmi.o

#spdif
obj-$(CONFIG_SND_SUNXI_SOC_SPDIF) += sunxi-sndspdif.o

##DEBUG
ifeq ($(CONFIG_SUNXI_AUDIO_DEBUG), y)
    EXTRA_CFLAGS += -DDEBUG
endif