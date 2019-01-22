#!/bin/bash
# by @ayufan-pine64/linux-build
# https://github.com/ayufan-pine64/linux-build/blob/master/LICENSE.txt
# adopted hehopmajieh@debian.bg
set -e

DESKTOP="$1"

if [ -z "$DESKTOP" ]; then
	echo "Usage: $0 <mate|i3|gnome|xfce4|lxde>"
	exit 1
fi

DISTRO=""
if hash apt-get 2>/dev/null; then
	DISTRO=$(lsb_release -i -s)
fi

if [ -z "$DISTRO" ]; then
	echo "This script requires a Debian based Linux distribution."
	exit 1
fi

if [ "$(id -u)" -ne "0" ]; then
	echo "This script requires root."
	exit 1
fi

# Default packages.
PACKAGES=(
	xserver-xorg-video-fbturbo
#	we are not ready for drm yet :)
#	xserver-xorg-video-armsoc-sunxi
#	libmali-sunxi-utgard0-r6p0
#	mesa-utils-extra
#	glmark2-es2
	libvdpau-sunxi1
	vdpauinfo
	mplayer
	smplayer
	smplayer-themes
	smtube
	chromium-browser
)

# Additional packages
PACKAGES+=(
	xserver-xorg-input-all
	xfonts-base
	rxvt-unicode-lite
	suckless-tools
	network-manager
	pulseaudio
)

case $DISTRO in
	Ubuntu)
		PACKAGES+=(
			chromium-browser
			firefox
		)
		;;

	Debian)
		PACKAGES+=(
			chromium
			chromium-widevine
		)
		;;

	*)
		echo "Error: unsupported desktop environment $DESKTOP-$DISTRO"
		exit 2
		;;
esac

# Add packages based on desktop selection.
case $DESKTOP-$DISTRO in
	mate-Ubuntu)
		PACKAGES+=(
			ubuntu-mate-core
			lightdm
		)
		;;

	mate-Debian)
		PACKAGES+=(
			mate-desktop-environment
			mate-desktop-environment-extras
			desktop-base
			lightdm
		)
		;;

	gnome-Ubuntu)
		PACKAGES+=(
			ubuntu-gnome-desktop
			ubuntu-gnome-wallpapers-xenial
		)
		;;

	gnome-Debian)
		PACKAGES+=(
			gnome
			desktop-base
		)
		;;

	i3-Ubuntu|i3-Debian)
		PACKAGES+=(
			i3
			i3status
			i3lock
			slim
		)
		;;

	xfce4-Ubuntu|xfce4-Debian)
		PACKAGES+=(
			xfce4
			xfce4-goodies
			lightdm
			lightdm-gtk-greeter
		)
		;;

	lxde-Ubuntu|lxde-Debian)
		PACKAGES+=(
			lxde
			lxdm
		)
		;;

	*)
		echo "Error: unsupported desktop environment $DESKTOP"
		exit 2
		;;
esac

# Install.
apt -y update
apt -y --install-recommends install ${PACKAGES[@]}

# Kill parport module loading, not available on arm64.
if [ -e "/etc/modules-load.d/cups-filters.conf" ]; then
	echo "" >/etc/modules-load.d/cups-filters.conf
fi

# Disable Pulseaudio timer scheduling which does not work with sndhdmi driver.
if [ -e "/etc/pulse/default.pa" ]; then
	sed -i 's/load-module module-udev-detect$/& tsched=0/g' /etc/pulse/default.pa
fi

# Desktop dependent post installation.
#case $DESKTOP in
#	mate)
#		# Change default wallpaper
#		dpkg-divert --divert /usr/share/backgrounds/ubuntu-mate-common/Ubuntu-Mate-Cold-stock.jpg --rename /usr/share/backgrounds/ubuntu-mate-common/Ubuntu-Mate-Cold.jpg
#		ln -s /usr/share/backgrounds/ubuntu-mate-pinebook/Pinebook-Wallpaper-6.jpg /usr/share/backgrounds/ubuntu-mate-common/Ubuntu-Mate-Cold.jpg
#		;;
#
#	i3|i3wm)
#		if [ ! -d /usr/share/slim/themes/pine64 ]; then
#			cp -ra /usr/share/slim/themes/default /usr/share/slim/themes/pine64
#			wget -O /usr/share/slim/themes/pine64/background.png \
#				https://github.com/longsleep/build-pine64-image/raw/master/bootlogo/bootlogo-pine64-1366x768.png
#			sed -i "s/^current_theme(.*)/current_theme pine64/g" /etc/slim.conf
#		fi
#		;;
#
#	*)
#		;;
#esac
mkdir -p /etc/X11/xorg.conf.d

# Make X11 use fbturbo driver.
cat > "/etc/X11/xorg.conf.d/99-teres-disp.conf" <<EOF
Section "ServerLayout"
	Identifier	"MultyDisplay"
	Screen 0 "LCD"
	Screen 1 "HDMI" RightOf "LCD"
	Option "Xinerama" "on"
#	Option "Clone" "off"
EndSection
EOF

cat > "/etc/X11/xorg.conf.d/20-teres-lcd.conf" <<EOF
Section "Device"
	Identifier      "TERES-LCD"
	Driver          "fbturbo"
	Option          "fbdev" "/dev/fb0"
	Option          "SwapbuffersWait" "true"
EndSection
Section "Monitor"
	Identifier "LCD-TERES"
EndSection
Section "Screen"
	Identifier	"LCD"
	Device	"TERES-LCD"
	Monitor "LCD-TERES"
	Option "Primary" "true"
SubSection "Display"
	Modes "1366x768"
	ViewPort 0 0
	Virtual 1366 768
EndSubSection
EndSection
EOF

cat > "/etc/X11/xorg.conf.d/30-teres-hdmi.conf" <<EOF
Section "Device"
	Identifier	"TERES-HDMI"
	Driver		"fbturbo"
	Option		"fbdev" "/dev/fb1"
	Option          "SwapbuffersWait" "true"
EndSection
Section "Monitor"
	Identifier "HDMI-TERES"
EndSection
Section	"Screen"
	Identifier	"HDMI"
	Device	"TERES-HDMI"
	Monitor "HDMI-TERES"
SubSection "Display"
	Modes "1920x1080"
	ViewPort 0 1
	Virtual 1920 1080
EndSubSection
EndSection
EOF

echo "Enabling vdpau via Xsession"

cat > "/etc/X11/Xsession.d/30teres-vdpau-sunxi" <<EOF
if [ -d /sys/module/disp ]; then
  export VDPAU_DRIVER=sunxi
fi
end
EOF

# Create directory if not present
[ -d /etc/chromium-browser ] || mkdir /etc/chromium-browser

# Set some default parameters for chromium.
if [ ! -e "/etc/chromium-browser/default" ]; then
	cat > "/etc/chromium-browser/default" <<EOF
# Default settings for chromium-browser. This file is sourced by /bin/sh from
# /usr/bin/chromium-browser
# Options to pass to chromium-browser
CHROMIUM_FLAGS="\
--disable-smooth-scrolling \
--disable-low-res-tiling \
--enable-low-end-device-mode \
--num-raster-threads=4 \
--profiler-timing=0 \
--disable-seccomp-filter-sandbox \
--disable-composited-antialiasing \
"
EOF
fi

echo
echo "Done - $DESKTOP installed - you should reboot now."

