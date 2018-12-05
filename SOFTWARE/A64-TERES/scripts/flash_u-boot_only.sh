#!/bin/sh
#
# Simple script to replace u-boot-with-dtb.bin in an existing image.
#

set -e

out="$1"
uboot="$2"

if [ -z "$out" ]; then
	echo "Usage: $0 /dev/sdX [<u-boot-with-dtb.bin>]"
	exit 1
fi

if [ -z "$uboot" ]; then
    uboot="../build/u-boot-with-dtb.bin"
fi
uboot_position=19096  # KiB

if [ ! -e "$out" ]; then
    echo "Error: $out not found"
    exit 1
fi

pv "$uboot" | dd conv=notrunc bs=1k seek=$uboot_position of="$out"

sync
