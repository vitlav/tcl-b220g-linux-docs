#!/bin/sh
sample() {
 printf 'uptime=%s ' "$(cut -d ' ' -f1 /proc/uptime)"
 for n in 0 6; do
  printf 'policy%s_kHz=%s ' "$n" "$(cat /sys/devices/system/cpu/cpufreq/policy$n/scaling_cur_freq)"
 done
 echo
}
echo 'Idle baseline'
for n in 1 2 3; do sample; sleep 1; done
for cpu in 0 6; do
 echo "Short CPU$cpu load"
 taskset -c "$cpu" timeout 5 sha256sum /dev/zero >/dev/null 2>&1 &
 job=$!
 for n in 1 2 3 4 5; do sample; sleep 1; done
 wait "$job"
 echo 'Recovery'
 for n in 1 2 3; do sample; sleep 1; done
done
echo 'Time in state'
for p in /sys/devices/system/cpu/cpufreq/policy*; do
 echo "$p"; cat "$p/stats/time_in_state"
done
