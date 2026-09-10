#!/bin/sh
set -eu
boot=$(cat /proc/sys/kernel/random/boot_id)
out=/var/log/tcl-venus1/$boot
usb=/run/initramfs/usb/tcl-venus1/logs/$boot
mkdir -p "$out" "$usb"
for i in $(seq 1 30); do
 dmesg > "$out/dmesg.txt"
 cat /sys/kernel/debug/devices_deferred > "$out/deferred.txt"
 v4l2-ctl --list-devices > "$out/devices.txt" 2>&1 || :
 cat /proc/iomem > "$out/iomem.txt"
 cp "$out/"*.txt "$usb/"
 sync
 sleep 3
done
