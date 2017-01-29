

ifdef CONFIG_ARCH_SUN8IW6P1
	LOWLEVEL_DIR = lowlevel_sun8iw6
	HDMI_BSP_FILE = hdmi_bsp_sun8iw6.o
endif

ifdef CONFIG_ARCH_SUN8IW7P1
	LOWLEVEL_DIR = lowlevel_sun8iw7
	HDMI_BSP_FILE = hdmi_bsp_sun8iw7.o
endif

ifdef CONFIG_ARCH_SUN50IW1P1
	LOWLEVEL_DIR = lowlevel_sun50iw1p1
	HDMI_BSP_FILE = hdmi_bsp_sun50iw1p1.o
endif


obj-y +=        dev_disp.o disp_sys_intf.o\
				lcd/panels.o lcd/lcd_source.o lcd/default_panel.o lcd/lt070me05000.o \
				de/disp_display.o de/disp_features.o de/disp_device.o de/disp_lcd.o de/disp_manager.o de/disp_private.o \
				de/disp_smart_backlight.o de/disp_enhance.o de/disp_capture.o de/disp_hdmi.o de/disp_vdevice.o\
				de/$(LOWLEVEL_DIR)/de_fcc.o de/$(LOWLEVEL_DIR)/de_gsu.o de/$(LOWLEVEL_DIR)/de_hal.o de/$(LOWLEVEL_DIR)/disp_al.o \
				de/$(LOWLEVEL_DIR)/de_peak.o de/$(LOWLEVEL_DIR)/de_rtmx.o de/$(LOWLEVEL_DIR)/de_scaler_table.o\
				de/$(LOWLEVEL_DIR)/de_vep_table.o de/$(LOWLEVEL_DIR)/de_vsu.o de/$(LOWLEVEL_DIR)/de_feat.o \
				de/$(LOWLEVEL_DIR)/de_lcd.o de/$(LOWLEVEL_DIR)/de_lti.o de/$(LOWLEVEL_DIR)/de_fce.o \
				de/$(LOWLEVEL_DIR)/de_ase.o de/$(LOWLEVEL_DIR)/de_bws.o de/$(LOWLEVEL_DIR)/de_enhance.o \
				de/$(LOWLEVEL_DIR)/de_smbl.o de/$(LOWLEVEL_DIR)/de_ccsc.o de/$(LOWLEVEL_DIR)/de_dcsc.o \
				de/$(LOWLEVEL_DIR)/de_wb.o de/$(LOWLEVEL_DIR)/de_dsi.o de/$(LOWLEVEL_DIR)/de_clock.o\
				hdmi/aw/hdmi_core.o hdmi/aw/hdmi_edid.o hdmi/aw/hdmi_interface.o hdmi/drv_hdmi.o\
				hdmi/aw/hdmi_hal.o hdmi/aw/$(HDMI_BSP_FILE)

