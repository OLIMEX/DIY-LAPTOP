#!/bin/bash
UENV_PATH="/boot/uEnv.txt"
set -e

if [ "$(id -u)" -ne "0" ]; then
	echo "This script requires root."
	exit 1
fi
case $1 in

	on) sed -i.bak '/debug=/c\debug=on' $UENV_PATH 
	    echo "Debug on headphone port enabled. Please reboot !"
	    ;;
       off) sed -i.bak '/debug=/c\debug=off' $UENV_PATH	
	    echo "Debug on headphone port disabled. Please reboot !"
            ;;
	*)
        echo "Unknown option! Usage: ./debug_swirch on|off "
	esac
