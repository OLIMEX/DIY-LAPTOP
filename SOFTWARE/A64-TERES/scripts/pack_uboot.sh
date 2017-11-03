#!/bin/sh
#
# Simple script to create a U-Boot with all the additional parts which are
# required to be accepted by th A64 boot0.
#
# This script requires build variants and tools from several other sources.
# See the variable definitions below. When all files can be found, a U-Boot
# file is created which can be loaded by A64 boot0 just fine.
#Useed parts from : https://github.com/longsleep/build-pine64-image/blob/master/u-boot-postprocess/u-boot-postprocess.sh

set -e

# Blobs as provided in the BSP
BLOBS="../blobs"
# https://github.com/longsleep/u-boot-pine64/tree/pine64-hacks
UBOOT="../u-boot"
# https://github.com/longsleep/arm-trusted-firmware-pine64
TRUSTED_FIRMWARE="../arm-trusted-firmware-a64"
TRUSTED_FIRMWARE_BUILD="release"
# https://github.com/longsleep/sunxi-pack-tools
SUNXI_PACK_TOOLS="../sunxi-pack-tools/bin"

BUILD="../build"
mkdir -p $BUILD

cp -avf $TRUSTED_FIRMWARE/build/sun50iw1p1/$TRUSTED_FIRMWARE_BUILD/bl31.bin $BUILD
cp -avf $UBOOT/u-boot-sun50iw1p1.bin $BUILD/u-boot.bin
cp -avf $BLOBS/scp.bin $BUILD
cp -avf $BLOBS/sys_config.fex $BUILD

# build binary device tree
dtc -Odtb -o $BUILD/a64-teres_uboot.dtb $BLOBS/a64-teres_uboot.dts

unix2dos $BUILD/sys_config.fex
$SUNXI_PACK_TOOLS/script $BUILD/sys_config.fex

# merge_uboot.exe u-boot.bin infile outfile mode[secmonitor|secos|scp]
$SUNXI_PACK_TOOLS/merge_uboot $BUILD/u-boot.bin $BUILD/bl31.bin $BUILD/u-boot-merged.bin secmonitor
$SUNXI_PACK_TOOLS/merge_uboot $BUILD/u-boot-merged.bin $BUILD/scp.bin $BUILD/u-boot-merged2.bin scp

# update_fdt.exe u-boot.bin xxx.dtb output_file.bin
$SUNXI_PACK_TOOLS/update_uboot_fdt $BUILD/u-boot-merged2.bin $BUILD/a64-teres_uboot.dtb $BUILD/u-boot-with-dtb.bin

# Add fex file to u-boot so it actually is accepted by boot0.
$SUNXI_PACK_TOOLS/update_uboot $BUILD/u-boot-with-dtb.bin $BUILD/sys_config.bin

echo "Done - created $BUILD/u-boot-with-dtb.bin"

