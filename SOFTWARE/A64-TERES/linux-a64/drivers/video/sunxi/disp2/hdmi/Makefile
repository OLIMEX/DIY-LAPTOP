obj-$(CONFIG_HDMI_DISP2_SUNXI) += hdmi.o
obj-m += hdmi_cec.o

hdmi-y := drv_hdmi.o hdmi_core.o hdmi_edid.o
hdmi-$(CONFIG_ARCH_SUN50IW1P1) += libhdmi_sun50iw1.o
hdmi-$(CONFIG_ARCH_SUN8IW11) += libhdmi_sun8iw11

$(obj)/libhdmi_sun50iw1.o:
	cp $(srctree)/drivers/video/sunxi/disp2/hdmi/libhdmi_sun50iw1 $(obj)/libhdmi_sun50iw1.o
