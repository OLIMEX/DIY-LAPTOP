obj-$(CONFIG_DISP2_SUNXI) += disp.o

disp-objs :=  dev_disp.o dev_fb.o dev_disp_debugfs.o
disp-objs	+= disp_sys_intf.o
disp-objs += dev_composer.o
disp-objs	+= de/disp_display.o de/disp_features.o de/disp_device.o de/disp_lcd.o de/disp_manager.o de/disp_private.o \
							de/disp_smart_backlight.o de/disp_enhance.o de/disp_capture.o de/disp_hdmi.o de/disp_tv.o de/disp_vdevice.o
disp-objs += lcd/panels.o lcd/lcd_source.o lcd/default_panel.o lcd/lt070me05000.o lcd/wtq05027d01.o lcd/t27p06.o lcd/dx0960be40a1.o lcd/tft720x1280.o lcd/S6D7AA0X01.o  lcd/inet_dsi_panel.o \
	lcd/mb709_mipi.o lcd/lcd_edp_anx9804.o
disp-$(CONFIG_ARCH_SUN50I) += de/lowlevel_sun50iw1/de_fcc.o de/lowlevel_sun50iw1/de_gsu.o de/lowlevel_sun50iw1/de_hal.o de/lowlevel_sun50iw1/disp_al.o \
						de/lowlevel_sun50iw1/de_peak.o de/lowlevel_sun50iw1/de_rtmx.o de/lowlevel_sun50iw1/de_scaler_table.o\
						de/lowlevel_sun50iw1/de_vep_table.o de/lowlevel_sun50iw1/de_vsu.o de/lowlevel_sun50iw1/de_feat.o \
						de/lowlevel_sun50iw1/de_lcd.o de/lowlevel_sun50iw1/de_lti.o de/lowlevel_sun50iw1/de_fce.o \
						de/lowlevel_sun50iw1/de_ase.o de/lowlevel_sun50iw1/de_enhance.o \
						de/lowlevel_sun50iw1/de_smbl.o de/lowlevel_sun50iw1/de_ccsc.o de/lowlevel_sun50iw1/de_dcsc.o \
						de/lowlevel_sun50iw1/de_wb.o de/lowlevel_sun50iw1/de_dsi.o de/lowlevel_sun50iw1/de_clock.o

