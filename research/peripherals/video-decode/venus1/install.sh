#!/bin/sh
set -eu
cd /var/tmp/tcl-venus1
sha256sum -c SHA256SUMS
usb=/run/initramfs/usb
mountpoint -q "$usb"
test "$(uname -r)" = 6.18.34-tcl-ice1
echo '6d1f5e32b290eee59ffd523aca001a73c2a9c8d6a8e90be046b9fe9619ad7d41  /run/initramfs/usb/EFI/BOOT/BOOTAA64.EFI' | sha256sum -c -
test ! -e "$usb/EFI/BOOT/BOOTAA64-before-venus1.bak"
test ! -e /lib/firmware/qcom/venus-5.4/venus.mbn
mkdir -p /lib/firmware/qcom/venus-5.4 "$usb/tcl-venus1"
cp firmware/qcom/venus-5.4/venus.mbn /lib/firmware/qcom/venus-5.4/
cp venus.dtb "$usb/tcl-venus1/"
cmp firmware/qcom/venus-5.4/venus.mbn /lib/firmware/qcom/venus-5.4/venus.mbn
cmp venus.dtb "$usb/tcl-venus1/venus.dtb"
cp "$usb/EFI/BOOT/BOOTAA64.EFI" "$usb/EFI/BOOT/BOOTAA64-before-venus1.bak"
cp BOOTAA64-venus1.EFI "$usb/EFI/BOOT/BOOTAA64-venus1.new"
cmp BOOTAA64-venus1.EFI "$usb/EFI/BOOT/BOOTAA64-venus1.new"
sync
mv "$usb/EFI/BOOT/BOOTAA64-venus1.new" "$usb/EFI/BOOT/BOOTAA64.EFI"
sync
echo VENUS1_INSTALLED
