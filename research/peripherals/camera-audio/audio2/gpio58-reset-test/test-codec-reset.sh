#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
sw=/sys/bus/platform/devices/62610000.soundwire/power
rx=/sys/bus/platform/devices/62600000.rxmacro/power
va=/sys/bus/platform/devices/62770000.codec/power
original=$(modinfo -n soundwire_qcom)
test -f "$original"
test "$(cat /proc/sys/kernel/random/boot_id)" = c781c4d4-cd36-4a07-b734-40b059ac3e52
for p in "$va" "$rx" "$sw"; do test "$(cat "$p/control")" = auto; done
test "$(ls /sys/bus/soundwire/devices | wc -l)" -eq 1
cleanup() {
 set +e
 echo on > "$sw/control"
 if test -d /sys/module/tcl_codec_reset; then rmmod tcl_codec_reset; fi
 if test -d /sys/module/tcl_audio_power_test; then rmmod tcl_audio_power_test; fi
 if test -d /sys/module/soundwire_qcom_resume_candidate; then rmmod soundwire_qcom_resume_candidate; fi
 if ! test -d /sys/module/soundwire_qcom; then modprobe soundwire_qcom; fi
 echo auto > "$sw/control"
 echo auto > "$rx/control"
 echo auto > "$va/control"
 echo RESTORED
 cat /proc/sys/kernel/tainted
 systemctl is-active tcl-weston
 ls /sys/bus/soundwire/devices
}
trap cleanup EXIT
for p in "$va" "$rx" "$sw"; do echo on > "$p/control"; done
echo BEFORE
cat "$sw/runtime_status"
rmmod soundwire_qcom
insmod ./soundwire_qcom_resume_candidate.ko
test -L /sys/bus/platform/devices/62610000.soundwire/driver
for n in 1; do
 echo CYCLE="$n"
 echo auto > "$sw/control"
 sleep 5
 cat "$sw/runtime_status"
 echo on > "$sw/control"
 sleep 1
 cat "$sw/runtime_status"
 insmod ./tcl_swr_status.ko
 rmmod tcl_swr_status
 sleep 1
 insmod ./tcl_swr_status.ko
 rmmod tcl_swr_status
 ls /sys/bus/soundwire/devices
done
echo SUPPLY_TEST
insmod ./tcl_audio_power_test.ko
insmod ./tcl_codec_reset.ko
for n in 1 2 3 4; do
 sleep 3
 insmod ./tcl_swr_status.ko
 rmmod tcl_swr_status
 ls /sys/bus/soundwire/devices
done
rmmod tcl_codec_reset
rmmod tcl_audio_power_test
dmesg | tail -n 60
