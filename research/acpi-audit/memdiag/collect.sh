#!/bin/sh
set -eu
usb=/run/initramfs/usb
test "$(findmnt -n -o UUID --target "$usb")" = 405E-8AB3
test -f "$usb/tcl-acpi1/TCL-ACPI1"
boot=$(cat /proc/sys/kernel/random/boot_id)
dest="$usb/tcl-memdiag/logs/$boot"
mkdir -p "$dest"
date -Is > "$dest/STARTED"
uname -a > "$dest/uname.txt"
cat /proc/cmdline > "$dest/cmdline.txt"
cat /proc/iomem > "$dest/iomem.txt"
cat /sys/firmware/efi/systab > "$dest/efi-systab.txt"
dmesg > "$dest/dmesg-before.txt"
sync
if timeout 30 python3 /usr/local/lib/tcl-memdiag/read-live-acpi.py > "$dest/tables.json.tmp" 2> "$dest/reader-error.txt"; then
    mv "$dest/tables.json.tmp" "$dest/tables.json"
    echo success > "$dest/RESULT"
else
    code=$?
    echo "reader_failed=$code" > "$dest/RESULT"
fi
dmesg > "$dest/dmesg-after.txt"
date -Is > "$dest/COMPLETE"
sync
