#!/bin/sh
# Run manually after a clean boot. Does not enable codec supplies or play PCM.
set -eu
cd /var/tmp/tcl-audio2
[ "$(uname -r)" = 6.18.34-tcl-audio2 ]
[ "$(cat /proc/sys/kernel/random/boot_id)" != c781c4d4-cd36-4a07-b734-40b059ac3e52 ]
taint=$(cat /proc/sys/kernel/tainted)
[ "$((taint & ~4096))" -eq 0 ]
[ ! -d /sys/bus/platform/devices/62600000.rxmacro ]
[ ! -d /sys/bus/platform/devices/62630000.soundwire ]
# These overlays guard their required provider phandles before applying.
insmod ./tcl_rx_overlay.ko
sleep 1
test -L /sys/bus/platform/devices/62600000.rxmacro/driver
insmod ./tcl_lpi_fixed.ko
sleep 1
test -L /sys/bus/platform/devices/627c0000.pinctrl/driver
insmod ./tcl_swr_overlay.ko
sleep 1
test -L /sys/bus/platform/devices/62610000.soundwire/driver
insmod ./tcl_wcd_rx_overlay.ko
modprobe snd_soc_lpass_tx_macro
insmod ./tcl_tx_overlay.ko
sleep 1
test -L /sys/bus/platform/devices/62620000.txmacro/driver
insmod ./tcl_swr_tx_overlay.ko
sleep 1
test -L /sys/bus/platform/devices/62630000.soundwire/driver
ls /sys/bus/soundwire/devices
cat /sys/kernel/debug/devices_deferred
cat /proc/sys/kernel/tainted
systemctl is-active tcl-weston
# No rollback by removing overlays: preserve their live DT references.
# Stop and inspect a failure instead of retrying this script.
