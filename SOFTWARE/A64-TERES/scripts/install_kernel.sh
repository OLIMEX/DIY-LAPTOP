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
	dtc -Odtb -o "$DEST/$SUBFOLDER/a64-olinuxino.dtb" "$BLOBS/a64-teres.dts"

	# Add bootlogo.
	cp -v "$BOOTLOGO" "$DEST/$BOOTLOGO_TARGET"
	# Add battery icons.
	mkdir -p "$DEST/$BATTERY_TARGET"
	cp -v "$BATTERY/bempty.bmp" "$DEST/$BATTERY_TARGET"
	cp -v "$BATTERY/low_pwr.bmp" "$DEST/$BATTERY_TARGET"
	cp -v "$BATTERY/battery_charge.bmp" "$DEST/$BATTERY_TARGET"


if [ ! -e "$DEST/uEnv.txt" ]; then
	cat <<EOF > "$DEST/uEnv.txt"
root=/dev/mmcblk1p2
debug=off
console=tty0 console=ttyS0,115200n8 no_console_suspend
kernel_filename=$KERNEL
initrd_filename=$INITRD_IMG
EOF
fi

sync
echo "Done - boot files in $DEST"
