#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
va=/sys/bus/platform/devices/62770000.codec/power
rx=/sys/bus/platform/devices/62600000.rxmacro/power
sw=/sys/bus/platform/devices/62610000.soundwire/power
for p in "$va" "$rx" "$sw"; do test "$(cat "$p/control")" = auto; done
trap 'echo auto > "$sw/control"; echo auto > "$rx/control"; echo auto > "$va/control"' EXIT
echo on > "$va/control"
echo on > "$rx/control"
echo on > "$sw/control"
sleep 1
for p in "$va" "$rx" "$sw"; do printf '%s ' "$p"; cat "$p/runtime_status"; done
insmod ./tcl_swr_status.ko
rmmod tcl_swr_status
cat /sys/kernel/debug/clk/clk_summary | grep -E 'lpass-rx|fsgen|TX_CORE'
dmesg | tail -n 10
