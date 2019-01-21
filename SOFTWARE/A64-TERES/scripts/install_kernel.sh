#!/bin/sh
#
# Simple script to put the Kernel image into a destination folder
# to be booted. The script also copies the a initrd and the conmpiled device
# tree. Usually the destination is a location which can be read while booting
# with U-Boot.
#
# Use this script to populate the first partition of disk images created with
# the simpleimage script of this project.
#

set -e

DEST="$1"

if [ -z "$DEST" ]; then
	echo "Usage: $0 <destination-folder> [linux-folder]"
	exit 1
fi

BLOBS="../blobs"
LINUX="../linux-a64"
INITRD="./initrd.gz"
BOOTLOGO="../blobs/bootlogo.bmp"
BATTERY="../blobs/bat"

# Targets file names as loaded by U-Boot.
SUBFOLDER="a64"
KERNEL="$SUBFOLDER/Image"
INITRD_IMG="initrd.img"
BOOTLOGO_TARGET="bootlogo.bmp"
BATTERY_TARGET="bat"

if [ "$DEST" = "-" ]; then
	DEST="../build"
fi

if [ -n "$2" ]; then
	LINUX="$2"
fi

echo "Using Linux from $LINUX ..."

VERSION=$(strings $LINUX/arch/arm64/boot/Image |grep "Linux version"|awk '{print $3}')
echo "Kernel build version $VERSION ..."
if [ -z "$VERSION" ]; then
	echo "Failed to get build version, correct <linux-folder>?"
	exit 1
fi

# Clean up
mkdir -p "$DEST/$SUBFOLDER"
rm -vf "$DEST/$KERNEL"
rm -vf "$DEST/"*.dtb

# Create and copy Kernel
echo -n "Copying Kernel ..."
cp -vf "$LINUX/arch/arm64/boot/Image" "$DEST/$KERNEL"
echo "$VERSION" > "$DEST/Image.version"
echo " OK"

# Copy initrd
echo -n "Copying initrd ..."
cp -vf "$INITRD" "$DEST/$INITRD_IMG"
echo " OK"

# Create and copy binary device tree
#	if grep -q sunxi-drm "$LINUX/arch/arm64/boot/Image"; then
#		echo "Kernel with DRM driver!"
#		basename="pine64drm"
#	fi

	# Not found, use device tree from BSP.
	echo "Compiling device tree from $BLOBS/${basename}.dts"
	dtc -Odtb -o "$DEST/$SUBFOLDER/sun50i-a64-teres.dtb" "$BLOBS/sun50i-a64-teres.dts"

	# Add bootlogo.
	cp -v "$BOOTLOGO" "$DEST/$BOOTLOGO_TARGET"
	# Add battery icons.
	mkdir -p "$DEST/$BATTERY_TARGET"
	cp -v "$BATTERY/bempty.bmp" "$DEST/$BATTERY_TARGET"
	cp -v "$BATTERY/low_pwr.bmp" "$DEST/$BATTERY_TARGET"
	cp -v "$BATTERY/battery_charge.bmp" "$DEST/$BATTERY_TARGET"
	


if [ ! -e "$DEST/uEnv.txt" ]; then
	cat <<EOF > "$DEST/uEnv.txt"
console=ttyS0,115200n8
selinux=permissive
enforcing=0
optargs=no_console_suspend
kernel_filename=a64/Image
initrd_filename=initrd.img
recovery_initrd_filename=ramdisk-recovery.img
hardware=sun50iw1p1
debug=on
# INFO:
# To enable one of below options,
# uncomment them by removing # in front of name

# To use android recovery:
# Create empty file recovery.txt in root of this partition

# To enable LCD or HDMI, if not changed it will use default (experimental)
# disp_screen0=lcd or hdmi
# disp_screen1=lcd or hdmi
# disp_mode=screen0 or screen1 or dualhead or xinerama or clone or disabled

# USB OTG port mode (experimental)
# otg_mode=device or host or otg
otg_mode=host

# Configure contiguous memory allocation
# This maybe required to be enlarged for 4K displays
cma=384M

# To change HDMI display mode:
# hdmi_mode=480i
# hdmi_mode=576i
# hdmi_mode=480p
# hdmi_mode=576p
# hdmi_mode=720p50
# hdmi_mode=720p60
# hdmi_mode=1080i50
# hdmi_mode=1080i60
# hdmi_mode=1080p24
# hdmi_mode=1080p50
# hdmi_mode=1080p60
# hdmi_mode=2160p30
# hdmi_mode=2160p25
# hdmi_mode=2160p24
# hdmi_mode=800x480p
# hdmi_mode=1024x600p

# To enable DVI compatibilty:
# disp_dvi_compat=on

# To enable CSI camera, if not enabled it will use default:
# camera_type=s5k4ec
# camera_type=ov5640

# Configure ethernet speed (Android-only)
eth0_speed=auto
# eth0_speed=1000
# eth0_speed=100
# eth0_speed=10

# If you are having problems with running from eMMC, like Sandisk eMMC
# It forces to use SDR-mode instead of HS-mode.
# Enable eMMC compatibility mode:
# emmc_compat=on

# Enable enhanced eMMC speed (might not work), the HS200/150MHz:
# emmc_compat=150mhz

# Enable enhanced eMMC speed (might not work), the HS200/200MHz:
# emmc_compat=200mhz

# Disable HDMI CEC
# hdmi_cec=0

# Enable experimental HDMI CEC driver
hdmi_cec=2


# Allow to execute user command
user_cmd=
EOF
fi

sync
echo "Done - boot files in $DEST"
