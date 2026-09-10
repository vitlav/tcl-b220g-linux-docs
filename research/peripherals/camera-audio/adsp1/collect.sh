#!/bin/sh
set -eu
boot=$(cat /proc/sys/kernel/random/boot_id)
out=/var/log/tcl-adsp1/$boot
usb=/run/initramfs/usb/tcl-adsp1/logs/$boot
mkdir -p "$out" "$usb"
for i in $(seq 1 120); do
 dmesg > "$out/dmesg.txt"
 cat /sys/kernel/debug/devices_deferred > "$out/deferred.txt" 2>/dev/null || :
 cat /proc/iomem > "$out/iomem.txt"
 cat /proc/asound/cards > "$out/alsa.txt"
 { for r in /sys/class/remoteproc/remoteproc*; do [ -d "$r" ] || continue; echo "$r"; cat "$r/name" "$r/state" "$r/firmware" || :; done; } > "$out/remoteproc.txt"
 ls -l /sys/bus/rpmsg/devices /sys/bus/aprbus/devices > "$out/rpmsg-apr.txt" 2>&1 || :
 cp "$out/"*.txt "$usb/"
 sync
 sleep 3
done
