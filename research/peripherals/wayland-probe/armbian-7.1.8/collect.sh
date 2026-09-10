#!/bin/sh
# Read-only status collection after boot; no I2C transactions or mode setting.
set -u
date -u
uname -a
cat /proc/sys/kernel/random/boot_id
cat /proc/cmdline
uptime
df -h / /run/initramfs/usb
printf '\n--- SERVICES ---\n'
systemctl --failed --no-legend
systemctl is-active ssh.socket
printf '\n--- NETWORK ---\n'
ip -brief address
ip route
iw dev wlan0 link
printf '\n--- STORAGE ---\n'
lsblk -o NAME,SIZE,RO,TYPE,MOUNTPOINTS
printf '\n--- CPUFREQ ---\n'
for p in /sys/devices/system/cpu/cpufreq/policy*; do
 echo "$p"
 for f in scaling_driver scaling_governor cpuinfo_min_freq cpuinfo_max_freq scaling_cur_freq; do
  printf '%s=' "$f"; cat "$p/$f"
 done
done
printf '\n--- DISPLAY AND I2C ---\n'
cat /proc/fb
ls -l /dev/dri /dev/i2c-* 2>/dev/null
cat /sys/kernel/debug/devices_deferred 2>/dev/null
printf '\n--- REMOTEPROC ---\n'
for p in /sys/class/remoteproc/remoteproc*; do
 echo "$p"; cat "$p/name" "$p/state"
done
printf '\n--- KERNEL LOG ---\n'
dmesg
