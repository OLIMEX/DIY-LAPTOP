# Adopted from:
# https://github.com/igorpecovnik/lib/blob/master/config/bootscripts/boot-pine64-default.cmd
# https://github.com/longsleep/u-boot-pine64/blob/55c9c8c8ac005b1c00ac948386c60c4a741ebaa9/include/configs/sun50iw1p1.h#L339
# hehopmajieh@debian.bg Adopted for Olimex Teres/A64-OLinuXino
# set_cmdline
setenv bootargs "console=${console} enforcing=${enforcing} cma=${cma} ${optargs} androidboot.serialno=${sunxi_serial} androidboot.hardware=${hardware} androidboot.selinux=${selinux} earlyprintk=sunxi-uart,0x01c28000 loglevel=8 root=${root} eth0_speed=${eth0_speed}"

env_set_debug

run load_dtb

# set display resolution from uEnv.txt or other environment file
# default to 1080p30
if test "${hdmi_mode}" = "480i"; then setenv fdt_hdmi_mode "<0x00000000>"
elif test "${hdmi_mode}" = "576i"; then setenv fdt_hdmi_mode "<0x00000001>"
elif test "${hdmi_mode}" = "480p"; then setenv fdt_hdmi_mode "<0x00000002>"
elif test "${hdmi_mode}" = "576p"; then setenv fdt_hdmi_mode "<0x00000003>"
elif test "${hdmi_mode}" = "720p50"; then setenv fdt_hdmi_mode "<0x00000004>"
elif test "${hdmi_mode}" = "720p60"; then setenv fdt_hdmi_mode "<0x00000005>"
elif test "${hdmi_mode}" = "1080i50"; then setenv fdt_hdmi_mode "<0x00000006>"
elif test "${hdmi_mode}" = "1080i60"; then setenv fdt_hdmi_mode "<0x00000007>"
elif test "${hdmi_mode}" = "1080p24"; then setenv fdt_hdmi_mode "<0x00000008>"
elif test "${hdmi_mode}" = "1080p50"; then setenv fdt_hdmi_mode "<0x00000009>"
elif test "${hdmi_mode}" = "1080p60"; then setenv fdt_hdmi_mode "<0x0000000a>"
elif test "${hdmi_mode}" = "2160p30"; then setenv fdt_hdmi_mode "<0x0000001c>"
elif test "${hdmi_mode}" = "2160p25"; then setenv fdt_hdmi_mode "<0x0000001d>"
elif test "${hdmi_mode}" = "2160p24"; then setenv fdt_hdmi_mode "<0x0000001e>"
elif test "${hdmi_mode}" = "800x480p"; then setenv fdt_hdmi_mode "<0x0000001f>"
elif test "${hdmi_mode}" = "1024x600p"; then setenv fdt_hdmi_mode "<0x00000020>"
else setenv fdt_hdmi_mode "<0x0000000a>"
fi

if test "${fdt_hdmi_mode}" != ""; then
	echo "HDMI mode: ${fdt_hdmi_mode}"
	fdt set /soc@01c00000/boot_disp output_mode "<0x00000000>"
	fdt set /soc@01c00000/disp@01000000 screen0_output_mode "${fdt_hdmi_mode}"
	fdt set /soc@01c00000/disp@01000000 screen1_output_mode "${fdt_hdmi_mode}"
fi

# set display for screen0
if test "${disp_screen0}" = "lcd"; then
	echo "Using LCD for main screen"
	fdt set /soc@01c00000/disp@01000000 screen0_output_type "<0x00000001>"
	fdt set /soc@01c00000/boot_disp output_mode "<0x00000000>"

	# enable LCD screen
	fdt set /soc@01c00000/lcd0@01c0c000 status "okay"
	fdt set /soc@01c00000/lcd0@01c0c000 lcd_used "<0x00000001>"

	# disable HDMI screen
	fdt set /soc@01c00000/hdmi@01ee0000 status "disabled"

	# enable touchpad
	fdt set /soc@01c00000/ctp status "okay"
	fdt set /soc@01c00000/ctp ctp_used "<0x00000001>"
	fdt set /soc@01c00000/ctp ctp_name "gt911_DB2"
elif test "${disp_screen0}" = "hdmi"; then
	echo "Using HDMI for main screen"
	fdt set /soc@01c00000/boot_disp output_mode "<0x00000000>"
	fdt set /soc@01c00000/disp@01000000 screen0_output_type "<0x00000003>"

	# enable HDMI screen
	fdt set /soc@01c00000/hdmi@01ee0000 status "okay"

	# disable LCD screen
	fdt set /soc@01c00000/lcd0@01c0c000 status "disabled"
	fdt set /soc@01c00000/lcd0@01c0c000 lcd_used "<0x00000000>"
fi

# set display for screen1
if test "${disp_screen1}" = "lcd"; then
	echo "Using LCD for secondary screen"
	fdt set /soc@01c00000/disp@01000000 screen1_output_type "<0x00000001>"

	# enable LCD screen
	fdt set /soc@01c00000/lcd0@01c0c000 status "okay"
	fdt set /soc@01c00000/lcd0@01c0c000 lcd_used "<0x00000001>"

	# enable touchpad
	fdt set /soc@01c00000/ctp status "okay"
	fdt set /soc@01c00000/ctp ctp_used "<0x00000001>"
	fdt set /soc@01c00000/ctp ctp_name "gt911_DB2"
elif test "${disp_screen1}" = "hdmi"; then
	echo "Using HDMI for secondary screen"
	fdt set /soc@01c00000/disp@01000000 screen1_output_type "<0x00000003>"

	# enable HDMI screen
	fdt set /soc@01c00000/hdmi@01ee0000 status "okay"
fi

# set disp_mode 
if test "${disp_mode}" = "screen0"; then
	echo "Using screen0 as display"
	fdt set /soc@01c00000/disp@01000000 disp_mode "<0x00000000>"
elif test "${disp_mode}" = "screen1"; then
	echo "Using screen1 as display"
	fdt set /soc@01c00000/disp@01000000 disp_mode "<0x00000001>"
elif test "${disp_mode}" = "dualhead"; then
	echo "Using screen0 and screen1 as separate displays"
	fdt set /soc@01c00000/disp@01000000 disp_mode "<0x00000002>"
elif test "${disp_mode}" = "xinerama"; then
	echo "Using screen0 and screen1 as one large screen"
	fdt set /soc@01c00000/disp@01000000 disp_mode "<0x00000003>"
elif test "${disp_mode}" = "clone"; then
	echo "Clonning screen0 and screen1"
	fdt set /soc@01c00000/disp@01000000 disp_mode "<0x00000004>"
elif test "${disp_mode}" = "disabled"; then
	echo "Disabling all screens"
	fdt set /soc@01c00000/disp@01000000 disp_mode "<0x00000000>"
	fdt set /soc@01c00000/hdmi@01ee0000 status "disabled"
	fdt set /soc@01c00000/lcd0@01c0c000 status "disabled"
fi

# HDMI CEC
if test "${hdmi_cec}" = "2"; then
	echo "Using experimental HDMI CEC driver"
	fdt set /soc@01c00000/hdmi@01ee0000 hdmi_cec_support "<0x00000002>"
else
	echo "HDMI CEC is disabled"
	fdt set /soc@01c00000/hdmi@01ee0000 hdmi_cec_support "<0x00000000>"
fi


# DVI compatibility
if test "${disp_dvi_compat}" = "on"; then
	fdt set /soc@01c00000/hdmi@01ee0000 hdmi_hdcp_enable "<0x00000000>"
	fdt set /soc@01c00000/hdmi@01ee0000 hdmi_cts_compatibility "<0x00000001>"
fi

# default, only set status
if test "${camera_type}" = "s5k4ec"; then
	fdt set /soc@01c00000/vfe@0/ status "okay"
	fdt set /soc@01c00000/vfe@0/dev@0/ status "okay"
fi

# change name, i2c address and vdd voltage
if test "${camera_type}" = "ov5640"; then
	fdt set /soc@01c00000/vfe@0/dev@0/ csi0_dev0_mname "ov5640"
	fdt set /soc@01c00000/vfe@0/dev@0/ csi0_dev0_twi_addr "<0x00000078>"
	fdt set /soc@01c00000/vfe@0/dev@0/ csi0_dev0_iovdd_vol "<0x001b7740>"
	fdt set /soc@01c00000/vfe@0/ status "okay"
	fdt set /soc@01c00000/vfe@0/dev@0/ status "okay"
fi

# set otg mode
if test "${otg_mode}" = "device"; then
	echo "USB-OTG port is in device mode"
	fdt set /soc@01c00000/usbc0@0 usb_port_type "<0x00000000>"
elif test "${otg_mode}" = "host"; then
	echo "USB-OTG port is in host mode"
	fdt set /soc@01c00000/usbc0@0 usb_port_type "<0x00000001>"
elif test "${otg_mode}" = "otg"; then
	echo "USB-OTG port is in OTG mode"
	fdt set /soc@01c00000/usbc0@0 usb_port_type "<0x00000002>"
fi

if test "${emmc_compat}" = "on"; then
	echo "Enabling eMMC compatibility mode (Use SDR)..."
	fdt rm /soc@01c00000/sdmmc@01C11000 mmc-ddr-1_8v;
	fdt rm /soc@01c00000/sdmmc@01C11000 mmc-hs200-1_8v;
	fdt rm /soc@01c00000/sdmmc@01C11000 mmc-hs400-1_8v;
elif test "${emmc_compat}" = "150mhz"; then
	echo "Enabling eMMC HS200 150MHz mode..."
	fdt set /soc@01c00000/sdmmc@01C11000 max-frequency "<0x8F0D180>";
elif test "${emmc_compat}" = "200mhz"; then
	echo "Enabling eMMC HS200 200MHz mode..."
	fdt set /soc@01c00000/sdmmc@01C11000 max-frequency "<0xBEBC200>";
fi

# Execute user command
if test "${user_cmd}" != ""; then
	echo "Executing ${user_cmd}..."
	run user_cmd
fi

if test "${boot_part}" = ""; then
	setenv boot_part "0:1"
fi

# Re-order SD or eMMC to always be a first device when booting
if test "${boot_part}" = "0:1"; then
	echo "Booting from SD so moving eMMC definition..."
	fdt resize
	fdt dup /soc@01c00000/ sdmmc@01C11000 sdmmc@01C11001
	fdt rm /soc@01c00000/sdmmc@01C11000/
else
	echo "Booting from eMMC so moving SD definition..."
	fdt resize
	fdt dup /soc@01c00000/ sdmmc@01c0f000 sdmmc@01c0f001
	fdt rm /soc@01c00000/sdmmc@01c0f000/
fi

if test "${boot_filename}" = ""; then
	# boot regular kernel
	if fatload mmc ${boot_part} ${initrd_addr} recovery.txt; then
		echo Using recovery...
		setenv initrd_filename "${recovery_initrd_filename}"
	fi
	echo "Loading kernel and initrd..."
	run load_kernel load_initrd boot_kernel
else
	# check if recovery.txt is created and load recovery image
	if fatload mmc ${boot_part} ${initrd_addr} recovery.txt; then
		echo Loading recovery...
		fatload mmc ${boot_part} ${initrd_addr} ${recovery_filename}
	else
		echo Loading normal boot...
		fatload mmc ${boot_part} ${initrd_addr} ${boot_filename}
	fi

	# boot android image
	boota ${initrd_addr}
fi

if test "${olimex_model}" = "teres"; then
echo "Booting for Teres-I"
fi

