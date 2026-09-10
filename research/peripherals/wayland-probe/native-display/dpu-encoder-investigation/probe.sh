#!/bin/sh
set -eu
export XDG_RUNTIME_DIR=/run/tcl-weston WAYLAND_DISPLAY=wayland-tcl
out=/var/log/tcl-dpu-probe
mkdir -p "$out"
for mode in recreate-a reuse recreate-b; do
 dmesg > "$out/$mode-before.txt"
 cat /sys/kernel/debug/dri/0/state > "$out/$mode-state-before.txt"
 set -- --fullscreen
 [ "$mode" != reuse ] || set -- "$@" --reuse-context
 for scene in 1 2 3 4 5 6 7 8 9 10 11 12; do
  set -- "$@" --benchmark build:duration=1.0
 done
 date -u > "$out/$mode-start.txt"
 /usr/bin/glmark2-wayland "$@" > "$out/$mode-glmark.txt" 2>&1
 dmesg > "$out/$mode-after.txt"
 cat /sys/kernel/debug/dri/0/state > "$out/$mode-state-after.txt"
 printf '%s: ' "$mode"
 before=$(grep -c 'no encoder found for crtc' "$out/$mode-before.txt" || true)
 after=$(grep -c 'no encoder found for crtc' "$out/$mode-after.txt" || true)
 echo "$before -> $after"
done
