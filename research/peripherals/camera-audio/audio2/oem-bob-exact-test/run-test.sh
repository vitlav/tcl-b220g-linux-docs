#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
[ "$(cat /proc/sys/kernel/random/boot_id)" = c781c4d4-cd36-4a07-b734-40b059ac3e52 ]
printf '\n=== Before ===\n'
cat /proc/sys/kernel/tainted
ls /sys/bus/soundwire/devices
if [ -d /sys/module/tcl_swr_status ]; then rmmod tcl_swr_status; fi
insmod ./tcl_swr_status.ko
rmmod tcl_swr_status
trap 'if [ -d /sys/module/tcl_audio_power_test ]; then rmmod tcl_audio_power_test; fi' EXIT
insmod ./tcl_audio_power_test.ko
sleep 3
printf '\n=== During exact OEM supply votes ===\n'
ls /sys/bus/soundwire/devices
insmod ./tcl_swr_status.ko
rmmod tcl_swr_status
sleep 20
printf '\n=== After automatic disable ===\n'
rmmod tcl_audio_power_test
cat /proc/sys/kernel/tainted
systemctl is-active tcl-weston
ls /sys/bus/soundwire/devices
dmesg | tail -n 35
