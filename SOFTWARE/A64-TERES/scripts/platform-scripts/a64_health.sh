#!/bin/sh
#
# Simple script to print some health data for Pine64. Some details were
# shamelessly stolen from http://pastebin.com/bSTYCQ5u. Thanks tkaiser.
#
# Run this script like `sudo watch -n2 pine64_health.sh` for a 2 sec update
# or call `sudo pine64_health.sh -w` to get updates every 0.5 seconds (default).
#

set -e

if [ "$(id -u)" -ne "0" ]; then
	echo "This script requires root."
	exit 1
fi

GPU_ADDR="1c40000.gpu"

print() {
	printf "%-15s: %s %s\n" "$1" "$2 $3" "$4"
}

cpu_frequency() {
	local cur=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq)
	local mhz=$(awk "BEGIN {printf \"%.2f\",$cur/1000}")
	print "CPU freq" $mhz "MHz"
}

scaling_govenor() {
	local gov=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)
	print "Governor" $gov
}

cpu_count() {
	local cpus=$(grep -c processor /proc/cpuinfo)
	print "CPU count" $cpus
}

vcore_voltage() {
	local uv=$(cat /sys/devices/platform/axp81x_board/axp-regulator.41/regulator/regulator.2/microvolts)
	local v=$(awk "BEGIN {printf \"%.2f\",$uv/1000000}")
	print "Core voltage" $v "V"
}

soc_temp() {
	local temp=$(cat /sys/devices/virtual/thermal/thermal_zone0/temp)
	print "SOC Temp" $temp "C"
}

pmic_temp() {
	local pmictemp=$(cat /sys/class/axppower/ic_temp 2>/dev/null)
	if [ "X$pmictemp" != "X" ]; then
		print "PMIC Temp" $pmictemp "C"
	fi
}

bat_capacity() {
	local bat_capacity=$(cat /sys/class/power_supply/battery/capacity 2>/dev/null)
	if [ "X$bat_capacity" != "X" ]; then
		print "Battery" $bat_capacity '%'
	fi
}

cooling_state() {
	local state=$(cat /sys/devices/virtual/thermal/cooling_device0/cur_state)
	print "Cooling state" $state
}

cooling_limit() {
	local budget=$(ls -1 -d  /sys/devices/soc.0/cpu_budget_cool.* |head -n1)
	local limit=$(cat $budget/roomage)
	print "Cooling limit" $limit
}

gpu_frequency() {
	if [ ! -e /sys/devices/$GPU_ADDR/dvfs/manual ]; then
		return
	fi
	local cur=$(cat /sys/devices/$GPU_ADDR/dvfs/manual|awk '{print $1}')
	local mhz=$(awk "BEGIN {printf \"%.2f\",$cur}")
	print "GPU freq" $mhz MHz
}

gpu_temp() {
	if [ ! -e /sys/devices/$GPU_ADDR/dvfs/tempctrl ]; then
		return
	fi
	local temp=$(cat /sys/devices/$GPU_ADDR/dvfs/tempctrl|awk '{print $6}')
	print "GPU Temp" $temp "C"
}

gpu_voltage() {
	if [ ! -e /sys/devices/$GPU_ADDR/dvfs/voltage ]; then
		return
	fi
	local mv=$(cat /sys/devices/$GPU_ADDR/dvfs/voltage|awk '{print $1}')
	local v=$(awk "BEGIN {printf \"%.2f\",$mv/1000}")
	print "GPU voltage" $v "V"
}

all() {
	cpu_frequency
	gpu_frequency
	cpu_count
	scaling_govenor
	vcore_voltage
	gpu_voltage
	soc_temp
	gpu_temp
	pmic_temp
	cooling_state
	cooling_limit
	bat_capacity
}

usage() {
	echo "Usage: $0 [-w] [-h]"
}

WATCH=""
for i in "$@"; do
	case $i in
	-w)
		WATCH=1
		shift
		;;
	-h|--help)
		usage
		exit 0
		;;
	*)
		usage
		exit 1
		;;
	esac
done

if [ -n "$WATCH" ]; then
	exec watch -n0.5 "$0"
else
	all
fi
