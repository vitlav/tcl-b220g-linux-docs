#!/bin/sh
set -eu
p=/sys/kernel/tracing/instances/tcl-swr-pm
sw=/sys/bus/platform/devices/62610000.soundwire/power
mkdir "$p"
trap 'echo auto > "$sw/control"; echo 0 > "$p/tracing_on"; echo nop > "$p/current_tracer"; rmdir "$p"' EXIT
printf '%s\n' swrm_runtime_resume swrm_runtime_suspend swrm_wait_for_frame_gen_enabled qcom_swrm_cpu_reg_write > "$p/set_ftrace_filter"
echo function > "$p/current_tracer"
echo 1 > "$p/tracing_on"
echo on > "$sw/control"
sleep 1
echo auto > "$sw/control"
sleep 4
echo 0 > "$p/tracing_on"
cat "$p/trace"
