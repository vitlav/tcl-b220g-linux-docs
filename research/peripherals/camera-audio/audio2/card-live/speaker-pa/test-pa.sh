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
 if test -d /sys/module/tcl_speaker_pa; then rmmod tcl_speaker_pa; fi
 if test -d /sys/module/tcl_audio_card; then rmmod tcl_audio_card; fi
 if test -d /sys/module/tcl_wcd_watchdog; then rmmod tcl_wcd_watchdog; fi
 for m in tcl_wcd_irq_fixed tcl_reset_provider tcl_audio_ldo tcl_bob_provider; do
  if test -d /sys/module/$m; then rmmod "$m"; fi
 done
 for d in /sys/bus/soundwire/devices/sdw-master-*; do echo auto > "$d/power/control"; done
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
echo LOGICAL_MASTER_PM_TEST
for d in /sys/bus/soundwire/devices/sdw-master-*; do
 echo on > "$d/power/control"
 test "$(cat "$d/power/runtime_status")" = active
done
echo REPEATED_LIFETIME_TEST
insmod ./tcl_reset_provider.ko
insmod ./tcl_audio_ldo.ko
insmod ./tcl_bob_provider.ko
modprobe snd_soc_sm8250
insmod ./tcl_wcd_watchdog.ko
insmod ./tcl-irq-recovery/tcl_wcd_irq_fixed.ko
sleep 2
test -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver
insmod ./tcl_audio_card.ko
sleep 3
echo CARD_RESULT
cat /proc/asound/cards
cat /proc/asound/pcm
cat /sys/kernel/debug/devices_deferred
if test -d /proc/asound/card0; then
 amixer -c 0 contents
fi
echo QUIET_TONE_TEST
amixer -c 0 cset name='HPHL Volume' 12
amixer -c 0 cset name='HPHR Volume' 12
amixer -c 0 cset name='RX_RX0 Digital Volume' 78
amixer -c 0 cset name='RX_RX1 Digital Volume' 78
amixer -c 0 cset name='RX_MACRO RX0 MUX' AIF1_PB
amixer -c 0 cset name='RX_MACRO RX1 MUX' AIF1_PB
amixer -c 0 cset name='RX INT0_1 MIX1 INP0' RX0
amixer -c 0 cset name='RX INT1_1 MIX1 INP0' RX1
amixer -c 0 cset name='RX INT0_1 INTERP' 'RX INT0_1 MIX1'
amixer -c 0 cset name='RX INT1_1 INTERP' 'RX INT1_1 MIX1'
amixer -c 0 cset name='RX INT0 DEM MUX' CLSH_DSM_OUT
amixer -c 0 cset name='RX INT1 DEM MUX' CLSH_DSM_OUT
amixer -c 0 cset name='RX HPH Mode' CLS_H_LP
amixer -c 0 cset name='HPHL Switch' on
amixer -c 0 cset name='HPHR Switch' on
amixer -c 0 cset name='CLSH Switch' on
amixer -c 0 cset name='HPHL_RDAC Switch' on
amixer -c 0 cset name='HPHR_RDAC Switch' on
amixer -c 0 cset name='RX_CODEC_DMA_RX_0 Audio Mixer MultiMedia1' on
timeout 10 aplay -D hw:0,0 -t raw -f S16_LE -r 48000 -c 2 -d 6 /var/tmp/tcl-audio2/quiet-tone-s16le.raw > /var/tmp/tcl-audio2/card-pa-aplay.txt 2>&1 &
playpid=$!
sleep 1
cat /proc/asound/card0/pcm0p/sub0/status
cat /proc/asound/card0/pcm0p/sub0/hw_params
grep -q 'state: RUNNING' /proc/asound/card0/pcm0p/sub0/status
insmod ./tcl_speaker_pa.ko
wait "$playpid"
rmmod tcl_speaker_pa
cat /var/tmp/tcl-audio2/card-pa-aplay.txt
echo CARD_TEST_END
