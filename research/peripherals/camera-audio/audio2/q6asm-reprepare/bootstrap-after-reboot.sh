#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
test "$(cat /proc/sys/kernel/random/boot_id)" != d82e1f9e-de49-4a4d-9c36-edc5c1266009
sh ./tcl-irq-recovery/restore-audio-buses.sh
modprobe snd_soc_wcd938x
rmmod snd_soc_wcd938x
rmmod snd_soc_wcd938x_sdw
insmod ./tcl-irq-recovery/tcl_wcd_sdw_irq_fixed.ko
insmod ./tcl_bob_test.ko
insmod ./tcl_bob_mode.ko
echo tcl-disabled > /sys/bus/platform/devices/soc@0:tcl-bob-consumer/driver_override
modprobe fixed
insmod ./tcl_wcd_aggregate.ko
# Let the overlay's one-shot watchdog expire before the separate PCM test.
sleep 32
test ! -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver
cat /proc/sys/kernel/tainted
