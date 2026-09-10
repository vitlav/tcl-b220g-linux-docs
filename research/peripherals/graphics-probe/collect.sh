#!/bin/sh
# Read-only baseline collector. Output goes to stdout for capture over SSH.
set -u
section() { printf '\n## %s\n' "$1"; }
section identity
date -u
uname -a
cat /proc/sys/kernel/random/boot_id
cat /proc/cmdline
section framebuffer
cat /proc/fb
for f in /sys/class/graphics/fb0/name /sys/class/graphics/fb0/virtual_size /sys/class/graphics/fb0/bits_per_pixel /sys/class/graphics/fb0/stride; do
    if [ -r "$f" ]; then printf '%s: ' "$f"; cat "$f"; fi
done
section drm
ls -l /sys/class/drm /dev/dri 2>/dev/null || :
section modules
for module in msm panel_edp ti_sn65dsi86 lontium_lt8911exb; do
    printf '\nModule %s\n' "$module"
    modinfo "$module" 2>&1 | head -12
done
section i2c_adapter_names
for f in /sys/bus/i2c/devices/i2c-*/name; do
    if [ -r "$f" ]; then printf '%s: ' "$f"; cat "$f"; fi
done
section display_dt_status
for n in /sys/firmware/devicetree/base/soc@0/display-subsystem@ae00000 /sys/firmware/devicetree/base/soc@0/gpu@5000000 /sys/firmware/devicetree/base/soc@0/clock-controller@af00000; do
    printf '%s: ' "$n"
    if [ -r "$n/status" ]; then tr '\000' '\n' < "$n/status"; else printf 'status absent or node absent\n'; fi
done
section kernel_display_messages
dmesg | grep -Ei 'drm|dsi|mdss|display|framebuffer|efifb|adreno|gmu|iommu|smmu' || :
section storage
lsblk -dn -o NAME,SIZE,RO,MODEL
section route
ip -brief address show wlan0
ip route
