#!/bin/sh

set -e

if [ ! -w /dev/cedar_dev ]; then
	# chmod 666 /dev/cedar_dev
	echo "Error: /dev/cedar_dev missing or no access"
	exit 1
fi

if [ ! -w /dev/ion ]; then
	# chmod 666 /dev/ion
	echo "Error: /dev/ion missing or no access"
	exit 1
fi

if [ ! -w /dev/disp ]; then
	# chmod 666 /dev/disp
	echo "Error: /dev/disp missing or no access"
	exit 1
fi

if [ -z "VDPAU_DRIVER" ]; then
	export VDPAU_DRIVER=sunxi
fi

exec mplayer -vo vdpau -vc ffmpeg12vdpau,ffh264vdpau,ffhevcvdpau, "$@"
