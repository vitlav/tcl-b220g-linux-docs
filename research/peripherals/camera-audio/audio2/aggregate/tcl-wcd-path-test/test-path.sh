#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
sw=/sys/bus/platform/devices/62610000.soundwire/power
rx=/sys/bus/platform/devices/62600000.rxmacro/power
va=/sys/bus/platform/devices/62770000.codec/power
tx=/sys/bus/platform/devices/62620000.txmacro/power
swtx=/sys/bus/platform/devices/62630000.soundwire/power
original=$(modinfo -n soundwire_qcom)
test -f "$original"
test "$(cat /proc/sys/kernel/random/boot_id)" = c781c4d4-cd36-4a07-b734-40b059ac3e52
for p in "$va" "$rx" "$tx" "$sw" "$swtx"; do test "$(cat "$p/control")" = auto; done
test "$(ls /sys/bus/soundwire/devices | wc -l)" -eq 4
cleanup() {
 set +e
 if test -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver; then
  echo soc@0:tcl-wcd9385 > /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver/unbind
 fi
 for m in tcl_wcd_watchdog tcl_wcd_path_test tcl_reset_provider tcl_audio_ldo tcl_bob_provider; do
  if test -d /sys/module/$m; then rmmod "$m"; fi
 done
 echo "" > /sys/bus/platform/devices/soc@0:tcl-bob-consumer/driver_override
 echo on > "$sw/control"
 echo on > "$swtx/control"
 if test -d /sys/module/tcl_codec_reset; then rmmod tcl_codec_reset; fi
 if test -d /sys/module/tcl_audio_power_test; then rmmod tcl_audio_power_test; fi
 if test -d /sys/module/soundwire_qcom_resume_candidate; then rmmod soundwire_qcom_resume_candidate; fi
 if ! test -d /sys/module/soundwire_qcom; then modprobe soundwire_qcom; fi
 echo auto > "$sw/control"
 echo auto > "$swtx/control"
 echo auto > "$rx/control"
 echo auto > "$va/control"
 echo auto > "$tx/control"
 echo RESTORED
 cat /proc/sys/kernel/tainted
 systemctl is-active tcl-weston
 ls /sys/bus/soundwire/devices
}
trap cleanup EXIT
for p in "$va" "$rx" "$tx" "$sw" "$swtx"; do echo on > "$p/control"; done
echo BEFORE
cat "$sw/runtime_status"
rmmod soundwire_qcom
insmod ./soundwire_qcom_resume_candidate.ko
test -L /sys/bus/platform/devices/62610000.soundwire/driver
for n in 1; do
 echo CYCLE="$n"
 echo auto > "$sw/control"
 echo auto > "$swtx/control"
 sleep 5
 cat "$sw/runtime_status"
 echo on > "$sw/control"
 echo on > "$swtx/control"
 sleep 1
 cat "$sw/runtime_status"
 insmod ./tcl_swr_status.ko
 rmmod tcl_swr_status
 insmod ./tcl_swr_tx_status.ko
 rmmod tcl_swr_tx_status
 sleep 1
 insmod ./tcl_swr_status.ko
 rmmod tcl_swr_status
 insmod ./tcl_swr_tx_status.ko
 rmmod tcl_swr_tx_status
 ls /sys/bus/soundwire/devices
done
echo AGGREGATE_TEST
 echo tcl-disabled > /sys/bus/platform/devices/soc@0:tcl-bob-consumer/driver_override
 modprobe fixed
 modprobe snd_soc_wcd938x
 rmmod snd_soc_wcd938x
 insmod ./tcl_reset_provider.ko
 insmod ./tcl_audio_ldo.ko
 insmod ./tcl_bob_provider.ko
 insmod ./tcl_wcd_watchdog.ko
 insmod ./tcl_wcd_path_test.ko
 sleep 5
 echo DURING
 readlink /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver || true
 cat /sys/kernel/debug/devices_deferred
 cat /sys/kernel/debug/asoc/components
 cat /proc/asound/cards
 sleep 30
 echo AFTER_WATCHDOG
 dmesg | tail -100
