#obj-$(CONFIG_CSI_VFE) += vfe_os.o
#obj-$(CONFIG_CSI_VFE) += vfe_subdev.o
obj-$(CONFIG_CSI_VFE) += device/
obj-$(CONFIG_CSI_VFE) += actuator/
#obj-$(CONFIG_CSI_VFE) += csi_cci/
#obj-$(CONFIG_CSI_VFE) += flash_light/
#obj-$(CONFIG_CSI_VFE) += camera_detector/
obj-$(CONFIG_CSI_VFE) += vfe_io.o
vfe_io-y					:= csi_cci/csi_cci_reg.o 
vfe_io-y					+= csi_cci/bsp_cci.o 
vfe_io-y					+= csi_cci/cci_helper.o 
vfe_io-y					+= csi_cci/cci_platform_drv.o
vfe_io-y					+= vfe_subdev.o
vfe_io-y					+= vfe_os.o
vfe_io-y					+= device/sensor_helper.o

obj-$(CONFIG_CSI_VFE) += vfe_v4l2.o
vfe_v4l2-y					:= csi/sunxi_csi.o
vfe_v4l2-y					+= mipi_csi/sunxi_mipi.o
vfe_v4l2-y					+= flash_light/flash.o
vfe_v4l2-y					+= sunxi_isp.o
ifneq ($(strip $(CONFIG_ARCH_SUN8IW11)),)
vfe_v4l2-y					+= csi/csi_reg_a20.o
else
vfe_v4l2-y					+= csi/csi_reg.o
endif
vfe_v4l2-y					+= csi/bsp_csi.o 
vfe_v4l2-y					+= bsp_common.o
vfe_v4l2-y					+= config.o
vfe_v4l2-y					+= utility/sensor_info.o
vfe_v4l2-y					+= isp_cfg/isp_cfg.o
vfe_v4l2-y					+= utility/cfg_op.o
vfe_v4l2-y					+= vfe.o
ifneq ($(strip $(CONFIG_ARCH_SUN8IW10)),)
vfe_v4l2-y					+= lib/bsp_isp_null.o
else ifneq ($(strip $(CONFIG_ARCH_SUN8IW11)),)
vfe_v4l2-y					+= lib/bsp_isp_null.o
else
vfe_v4l2-y					+= lib/libisp.o
endif

$(obj)/lib/libisp.o:
	mkdir -p $(obj)/lib/
	cp $(srctree)/drivers/media/platform/sunxi-vfe/lib/libisp $(obj)/lib/libisp.o

ifneq ($(strip $(CONFIG_ARCH_SUN9I)),)
vfe_v4l2-y					+= lib/lib_mipicsi2_v2
else ifneq ($(strip $(CONFIG_ARCH_SUN8IW6)),)
obj-$(CONFIG_CSI_VFE) += vfe_v4l2.o
vfe_v4l2-y					+= mipi_csi/bsp_mipi_csi.o
else ifneq ($(strip $(CONFIG_ARCH_SUN8IW8)),)
obj-$(CONFIG_CSI_VFE) += vfe_v4l2.o
vfe_v4l2-y					+= mipi_csi/bsp_mipi_csi_v1.o \
							mipi_csi/protocol/protocol_reg.o \
							mipi_csi/dphy/dphy_reg.o
else
obj-$(CONFIG_CSI_VFE) += vfe_v4l2.o
vfe_v4l2-y					+= mipi_csi/bsp_mipi_csi_null.o
endif
