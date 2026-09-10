#!/bin/sh
set -eu
p=/sys/kernel/tracing/instances/tcl-swr-stop
sw=/sys/bus/platform/devices/62610000.soundwire/power
mkdir "$p"
cleanup() {
 echo auto > "$sw/control"
 echo 0 > "$p/tracing_on"
 if [ -d "$p/events/tcl_swr_pm" ]; then echo 0 > "$p/events/tcl_swr_pm/enable"; fi
 rmdir "$p"
 echo '-:tcl_swr_pm/write' >> /sys/kernel/tracing/kprobe_events
 echo '-:tcl_swr_pm/frame' >> /sys/kernel/tracing/kprobe_events
 echo '-:tcl_swr_pm/stop' >> /sys/kernel/tracing/kprobe_events
 echo '-:tcl_swr_pm/suspend' >> /sys/kernel/tracing/kprobe_events
}
trap cleanup EXIT
echo 'p:tcl_swr_pm/write soundwire_qcom:qcom_swrm_cpu_reg_write reg=$arg2:u32 val=$arg3:u32' >> /sys/kernel/tracing/kprobe_events
echo 'r:tcl_swr_pm/frame soundwire_qcom:swrm_wait_for_frame_gen_enabled result=$retval:u8' >> /sys/kernel/tracing/kprobe_events
echo 'r:tcl_swr_pm/stop sdw_bus_clk_stop result=$retval:s32' >> /sys/kernel/tracing/kprobe_events
echo 'r:tcl_swr_pm/suspend soundwire_qcom:swrm_runtime_suspend result=$retval:s32' >> /sys/kernel/tracing/kprobe_events
echo 1 > "$p/events/tcl_swr_pm/enable"
echo 1 > "$p/tracing_on"
for cycle in 1 2; do
echo on > "$sw/control"
sleep 1
insmod /var/tmp/tcl-audio2/tcl_swr_status.ko
rmmod tcl_swr_status
echo auto > "$sw/control"
sleep 5
done
echo 0 > "$p/tracing_on"
cat "$p/trace"
dmesg | tail -n 3
