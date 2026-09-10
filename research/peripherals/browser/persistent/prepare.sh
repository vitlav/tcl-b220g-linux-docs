#!/bin/sh
set -eu
cd /usr/local/lib/tcl-audio2
test "$(uname -r)" = 6.18.34-tcl-audio2
install -d -m700 -o tcl-browser -g tcl-browser /run/tcl-browser
n=0
until test -S /run/tcl-weston/wayland-tcl; do
 n=$((n+1)); test "$n" -lt 60; sleep 1
done
setfacl -m u:tcl-browser:--x /run/tcl-weston
setfacl -m u:tcl-browser:rw /run/tcl-weston/wayland-tcl
if ! test -d /sys/module/tcl_wcd_aggregate; then
 # Bootstrap only once on an unmodified audio2 boot. Its guards reject partial state.
 /bin/sh ./bootstrap-after-reboot.sh
fi
test -d /sys/module/tcl_wcd_sdw_irq_fixed
test ! -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver
