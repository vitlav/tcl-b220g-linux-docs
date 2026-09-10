#!/bin/sh
# Diagnostic session, run as root on validated KMS1 with handoff V2 already loaded.
# Not an installed display manager or automatic login setup.
set -eu
[ "$(uname -r)" = 6.18.34-tcl-kms1 ]
[ -e /dev/dri/card0 ]
[ "$(cat /sys/class/graphics/fb0/name)" = msmdrmfb ]
[ -f /var/tmp/tcl-weston.ini ]
bootid=$(cat /proc/sys/kernel/random/boot_id)
logdir=/var/log/tcl-kms1/$bootid
mkdir -p "$logdir"
systemd-run --unit=tcl-weston-pixman \
 --property=RuntimeDirectory=tcl-weston --property=RuntimeDirectoryMode=0700 \
 --setenv=XDG_RUNTIME_DIR=/run/tcl-weston --setenv=LIBSEAT_BACKEND=builtin \
 /usr/bin/weston --backend=drm --renderer=pixman --drm-device=card0 \
 --current-mode --socket=wayland-tcl --idle-time=0 \
 --config=/var/tmp/tcl-weston.ini --log="$logdir/weston.log"
