#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
movie=/run/initramfs/usb/tcl-venus1/big_buck_bunny_1080p_h264.mov
test -s "$movie"
test -S /run/tcl-weston/wayland-tcl
playpid=
sw=/sys/bus/platform/devices/62610000.soundwire/power
rx=/sys/bus/platform/devices/62600000.rxmacro/power
va=/sys/bus/platform/devices/62770000.codec/power
tx=/sys/bus/platform/devices/62620000.txmacro/power
swtx=/sys/bus/platform/devices/62630000.soundwire/power
test "$(cat /proc/sys/kernel/tainted)" -eq 4096
original=$(modinfo -n soundwire_qcom)
test -f "$original"
test "$(cat /proc/sys/kernel/random/boot_id)" = faf038a3-aea7-46c4-92d4-3266a7ebbef4
for p in "$va" "$rx" "$tx" "$sw" "$swtx"; do test "$(cat "$p/control")" = auto; done
test -d /sys/module/tcl_wcd_sdw_irq_fixed
test -d /sys/module/tcl_wcd_aggregate
test ! -L /sys/bus/platform/devices/soc@0:tcl-wcd9385/driver
cleanup() {
 set +e
 if test -n "$playpid" && kill -0 "$playpid" 2>/dev/null; then kill -TERM "$playpid"; wait "$playpid"; fi
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
 if test -d /sys/module/tcl_q6asm_dai_fixed; then rmmod tcl_q6asm_dai_fixed; modprobe q6asm_dai; fi
 echo RESTORED
 cat /proc/sys/kernel/tainted
 systemctl is-active tcl-weston
 ls /sys/bus/soundwire/devices
}
trap cleanup EXIT
rmmod q6asm_dai
insmod ./tcl_q6asm_dai_fixed.ko
trap "exit 143" TERM INT HUP
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
insmod ./tcl_wcd_watchdog.ko hold_seconds=1000
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
amixer -c 0 cset name='HPHL Volume' 20
amixer -c 0 cset name='HPHR Volume' 20
amixer -c 0 cset name='RX_RX0 Digital Volume' 84
amixer -c 0 cset name='RX_RX1 Digital Volume' 84
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
XDG_RUNTIME_DIR=/run/tcl-weston WAYLAND_DISPLAY=wayland-tcl mpv --no-config --msg-level=all=v --fullscreen --vo=gpu-next --gpu-api=opengl --gpu-context=wayland --hwdec=no --ao=alsa --audio-device=alsa/hw:Test,0 --audio-samplerate=48000 --audio-format=s16 --audio-channels=stereo --volume=50 --volume-max=100 --input-ipc-server=/run/tcl-weston/mpv-sound-test.sock "$movie" > /var/tmp/tcl-audio2/movie-mpv.log 2>&1 &
playpid=$!
ready=no
n=0
while test "$n" -lt 60; do
 n=$((n + 1))
 sleep 1
 if grep -q 'state: RUNNING' /proc/asound/card0/pcm0p/sub0/status; then ready=yes; break; fi
 kill -0 "$playpid"
done
test "$ready" = yes
insmod ./tcl_speaker_pa.ko hold_seconds=900
echo MOVIE_AUDIO_RUNNING
cat /proc/asound/card0/pcm0p/sub0/status
wait "$playpid"
playpid=
echo MOVIE_FINISHED
