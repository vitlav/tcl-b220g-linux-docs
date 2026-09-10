#!/bin/sh
set -eu
out=$1
printf 'utc,uptime,sensor,value\n' > "$out"
while systemctl is-active --quiet tcl-gpu-full; do
 now=$(date -u +%FT%TZ)
 read -r uptime rest < /proc/uptime
 for z in /sys/class/thermal/thermal_zone*; do
  [ -r "$z/temp" ] || continue
  type=$(cat "$z/type")
  value=$(cat "$z/temp")
  printf '%s,%s,%s,%s\n' "$now" "$uptime" "$type" "$value" >> "$out"
 done
 for f in /sys/class/devfreq/5000000.gpu/cur_freq /sys/devices/system/cpu/cpufreq/policy*/scaling_cur_freq; do
  [ -r "$f" ] || continue
  printf '%s,%s,%s,%s\n' "$now" "$uptime" "$f" "$(cat "$f")" >> "$out"
 done
 sleep 5
done
