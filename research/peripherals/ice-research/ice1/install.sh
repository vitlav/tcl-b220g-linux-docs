#!/bin/sh
set -eu
cd /var/tmp/tcl-ice1
sha256sum -c SHA256SUMS
usb=/run/initramfs/usb
mountpoint -q "$usb"
[ "$(cat "$usb/tcl-diag/TCL-RAM-TEST")" = 'TCL B220G experimental RAM-only kernel diagnostic' ]
echo '96dadb1cdbd9452869fc07b994943d0085702235c16791ac2a0342843b207038  /run/initramfs/usb/EFI/BOOT/BOOTAA64.EFI' | sha256sum -c -
test ! -d /lib/modules/6.18.34-tcl-ice1
test ! -e "$usb/EFI/BOOT/BOOTAA64-before-ice1.bak"
mkdir -p root-stage
tar -xzf modules.tar.gz -C root-stage
(cd root-stage && sha256sum -c ../MODULES.SHA256 > ../modules-verify.log)
mv root-stage/lib/modules/6.18.34-tcl-ice1 /lib/modules/
mkdir -p "$usb/tcl-ice1"
cp Image ice-probe.dtb initramfs.cpio.gz "$usb/tcl-ice1/"
cp "$usb/EFI/BOOT/BOOTAA64.EFI" "$usb/EFI/BOOT/BOOTAA64-before-ice1.bak"
cp BOOTAA64-ice1.EFI "$usb/EFI/BOOT/BOOTAA64-ice1.new"
for f in Image ice-probe.dtb initramfs.cpio.gz; do cmp "$f" "$usb/tcl-ice1/$f"; done
cmp BOOTAA64-ice1.EFI "$usb/EFI/BOOT/BOOTAA64-ice1.new"
sync
mv "$usb/EFI/BOOT/BOOTAA64-ice1.new" "$usb/EFI/BOOT/BOOTAA64.EFI"
sync
echo ICE1_INSTALLED
