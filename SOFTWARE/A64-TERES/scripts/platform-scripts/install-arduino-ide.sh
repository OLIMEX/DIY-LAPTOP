#!/bin/bash

#check for root priv
if [ "$(id -u)" != "0" ]; then
        echo -ne "This script must be executed as root. Exiting\n" >&2
        exit 1
fi

#check how being elevated
if [ -z $SUDO_COMMAND ]; then
	echo -ne "This script relies on being run via sudo for some operations.\n"
	echo -ne "Expect things to not work, or to have to do some extra stuff\n"
	echo -ne "after running it if you continue.\n"
	echo -ne "\nYou have been warned!\n\n"
fi

#download and unpack
echo -ne "Arduino IDE 1.8.5 Install Script\n\n"
echo -ne "Downloading Arduino IDE 1.8.5 ... "
wget -q -P /tmp/ https://downloads.arduino.cc/arduino-1.8.5-linuxarm.tar.xz 2>&1
if [ $? -ne 0 ]; then
   echo -ne "fail\n"
   echo -ne "Unable to successfully download package... please try again!\n\n"
   exit 1
else
   echo -ne "done\n"
fi

echo -ne "Unpacking to /opt/arduino-1.8.5/ ... "
tar xf /tmp/arduino-1.8.5-linuxarm.tar.xz --directory /opt/
echo -ne "done\n"

#enable armhf packages support
echo -ne "Enable armhf package support and update software repository ... "
dpkg --add-architecture armhf
apt-get -qq update
echo -ne "done\n"

#install required armhf dependencies
echo -ne "Installing required dependencies (this may take several minutes) ... "
apt-get -f -qq -y install libxtst6:armhf > /dev/null 2>&1
apt-get -f -qq -y install libxrender1:armhf > /dev/null 2>&1
apt-get -f -qq -y install libxi6:armhf > /dev/null 2>&1
apt-get -f -qq -y install openjdk-8-jre:armhf > /dev/null 2>&1
echo -ne " done\n"

#get rid of GTK errors
echo -ne "Install GTK2 engine and required theme ... "
apt-get -f -qq -y install gtk2-engines:armhf gtk2-engines-murrine:armhf > /dev/null 2>&1
echo -ne "done\n"

#fix serial monitor error caused by wrong ~/.jssc/linux/libjSSC-2.8_armsf.so
if [ -n $SUDO_USER ]; then
	echo -ne "Fixing up serial monitor bug ... "

	#rename old files if present
	[ -f "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armsf.so" ] && mv "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armsf.so" "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armsf.so.old"
	[ -f "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armhf.so" ] && mv "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armhf.so" "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armhf.so.old"

	#create directory if it doesn't actually exist, which it shouldn't on a clean system
	[ ! -d "/home/$SUDO_USER/.jssc/linux" ] && mkdir -p "/home/$SUDO_USER/.jssc/linux"

	unzip -p "/opt/arduino-1.8.5/lib/jssc-2.8.0.jar" "libs/linux/libjSSC-2.8_armhf.so" > "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armhf.so"
	ln -s "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armhf.so" "/home/$SUDO_USER/.jssc/linux/libjSSC-2.8_armsf.so"
	echo -ne "done\n"
else
	echo -ne "Not sudo, so you'll have to apply the serial monitor bug fix yourself!"
	echo -ne "As per post #21 of https://forum.arduino.cc/index.php?topic=400808.15"
fi

#add user to dialout group
if [ -n $SUDO_USER ]; then
	echo -ne "Add user to the dialout group ... "
	usermod -aG dialout $SUDO_USER
	echo -ne "done\n"
else
	echo -ne "Not running via sudo, can't determine username to add to dialout group!\n"
fi

#add desktop icon using provided install script
if [ -n $SUDO_USER ]; then
    echo -ne "Adding desktop shortcut, menu item and file associations for Arduino IDE ... "
	su $SUDO_USER /opt/arduino-1.8.5/install.sh > /dev/null 2>&1
    echo -ne "done\n"
else
	echo -ne "Not running as sudo, can't run install.sh as normal user\n"
	echo -ne "So you'll need to run /opt/arduino-1.8.5/install.sh yourself!\n"
fi

#completion messages
echo -ne "\nYou should now be able to see an 'Arduino' icon on your desktop ready"
echo -ne "\nfor you to use! Please note that the first launch will take a while,"
echo -ne "\nbut it will be pretty quick after that first run."

#notify user that they will need to log out and in again before will be able to load to a device to allow addition to dialout group to take effect
if [ -n $SUDO_USER ]; then
	echo -ne "\n\nYou will need to log out and back in again to allow the addition"
	echo -ne "\nof your username to the 'dialout' group to take effect. Failure"
	echo -ne "\nto do so will prevent you from being able to upload to any"
	echo -ne "\nserial programmed Arduino compatiable devices."
fi

#notify user they can also delete the downloaded arduino archive
echo -ne "\n\nAdditionally, you can delete /tmp/arduino-1.8.5-linuxarm.tar.xz"
echo -ne "\nif you wish to as it is no longer needed.\n"
