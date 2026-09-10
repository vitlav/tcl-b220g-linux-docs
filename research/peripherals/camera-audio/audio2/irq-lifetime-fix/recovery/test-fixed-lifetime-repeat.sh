#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
sw=/sys/bus/platform/devices/62610000.soundwire/power
rx=/sys/bus/platform/devices/62600000.rxmacro/power
va=/sys/bus/platform/devices/62770000.codec/power
tx=/sys/bus/platform/devices/62620000.txmacro/power
swtx=/sys/bus/platform/devices/62630000.soundwire/power
test "$(cat /proc/sys/kernel/tainted)" -eq 4096
original=$(modinfo -n soundwire_qcom)
test -f "$original"
test "$(cat /proc/sys/kernel/random/boot_id)" = d82e1f9e-de49-4a4d-9c36-edc5c1266009
for p in "$va" "$rx" "$tx" "$sw" "$swtx"; do test "$(cat "$p/control")" = auto; done
test -d /sys/module/tcl_wcd_sdw_irq_fixed
test -d /sys/module/tcl_wcd_aggregate
test ! -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver
cleanup() {
 set +e
 if test -d /sys/module/tcl_wcd_watchdog; then rmmod tcl_wcd_watchdog; fi
 for m in tcl_wcd_irq_fixed tcl_reset_provider tcl_audio_ldo tcl_bob_provider; do
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
# Provider absent: old consumer cannot enable BOB before this override.
echo tcl-disabled > /sys/bus/platform/devices/soc@0:tcl-bob-consumer/driver_override
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
echo REPEATED_LIFETIME_TEST
insmod ./tcl_reset_provider.ko
insmod ./tcl_audio_ldo.ko
insmod ./tcl_bob_provider.ko
for cycle in 1 2; do
 echo BIND_CYCLE="$cycle"
 insmod ./tcl_wcd_watchdog.ko
 insmod ./tcl-irq-recovery/tcl_wcd_irq_fixed.ko
 sleep 5
 test -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver
 cat /sys/kernel/debug/asoc/components
 if test -d /sys/kernel/debug/irq/domains; then ls /sys/kernel/debug/irq/domains; fi
 rmmod tcl_wcd_watchdog
 test ! -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver
 rmmod tcl_wcd_irq_fixed
 test "$(cat /proc/sys/kernel/tainted)" -eq 4096
 echo UNLOADED_CYCLE="$cycle"
 if test -d /sys/kernel/debug/irq/domains; then ls /sys/kernel/debug/irq/domains; fi
done
echo TWO_LIFECYCLES_DONE
