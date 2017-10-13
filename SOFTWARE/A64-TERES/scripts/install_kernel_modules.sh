#!/bin/sh

set -e

DEST="$1"

if [ -z "$DEST" ]; then
	echo "Usage: $0 <destination-folder> [linux-folder]"
	exit 1
fi

LINUX="../linux-a64"

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

# Install Kernel modules
make -C $LINUX ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules_install INSTALL_MOD_PATH="$DEST"
# Install Kernel firmware
make -C $LINUX ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- firmware_install INSTALL_MOD_PATH="$DEST"

# Fix symbolic links
rm -f "$DEST/lib/modules/$VERSION/source"
rm -f "$DEST/lib/modules/$VERSION/build"
ln -s "/usr/src/linux-headers-$VERSION" "$DEST/lib/modules/$VERSION/build"

# Install extra mali module if found in Kernel tree.
if [ -e $LINUX/modules/gpu/mali400/kernel_mode/driver/src/devicedrv/mali/mali.ko ]; then
	v=
	mkdir "$DEST/lib/modules/$VERSION/kernel/extramodules"
	cp -v $LINUX/modules/gpu/mali400/kernel_mode/driver/src/devicedrv/mali/mali.ko $DEST/lib/modules/$VERSION/kernel/extramodules
	depmod -b $DEST $VERSION
fi

echo "Done - installed Kernel modules to $DEST"
