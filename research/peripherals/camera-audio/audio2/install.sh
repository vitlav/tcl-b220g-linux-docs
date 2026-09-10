#!/bin/sh
set -eu
cd /var/tmp/tcl-audio2
sha256sum -c SHA256SUMS > artifacts-verify.log
usb=/run/initramfs/usb
mountpoint -q "$usb"
test "$(uname -r)" = 6.18.34-tcl-ice1
test "$(cat "$usb/tcl-diag/TCL-RAM-TEST")" = 'TCL B220G experimental RAM-only kernel diagnostic'
printf '%s  %s\n' eac9f87d7bad88dbc3bf1ddaf53f8ea1d80505da1f288fcd7d5f755294c122db "$usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
test ! -e /lib/modules/6.18.34-tcl-audio2
test ! -e "$usb/tcl-audio2"
test ! -e "$usb/EFI/BOOT/BOOTAA64-before-audio2.bak"
test "$(df -Pk / | awk 'NR==2 {print $4}')" -gt 200000
test "$(df -Pk "$usb" | awk 'NR==2 {print $4}')" -gt 200000
mkdir root-stage
tar -xzf modules.tar.gz -C root-stage
(cd root-stage && sha256sum -c ../MODULES.SHA256 > ../modules-verify.log)
mv root-stage/lib/modules/6.18.34-tcl-audio2 /lib/modules/
for mod in ath10k_snoc msm tcl_lt8911_handoff q6asm_dai q6afe_dai q6routing; do
 modprobe -S 6.18.34-tcl-audio2 --show-depends "$mod" > "depends-$mod.txt"
done
mkdir "$usb/tcl-audio2"
cp Image audio2.dtb initramfs.cpio.gz grub.cfg "$usb/tcl-audio2/"
for f in Image audio2.dtb initramfs.cpio.gz grub.cfg; do cmp "$f" "$usb/tcl-audio2/$f"; done
cp "$usb/EFI/BOOT/BOOTAA64.EFI" "$usb/EFI/BOOT/BOOTAA64-before-audio2.bak"
cp BOOTAA64-audio2.EFI "$usb/EFI/BOOT/BOOTAA64-audio2.new"
cmp BOOTAA64-audio2.EFI "$usb/EFI/BOOT/BOOTAA64-audio2.new"
sync
mv "$usb/EFI/BOOT/BOOTAA64-audio2.new" "$usb/EFI/BOOT/BOOTAA64.EFI"
sync
echo AUDIO2_INSTALLED_NO_REBOOT
