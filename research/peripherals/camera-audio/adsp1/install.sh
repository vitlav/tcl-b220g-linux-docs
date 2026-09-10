#!/bin/sh
set -eu
cd /var/tmp/tcl-adsp1
sha256sum -c SHA256SUMS
usb=/run/initramfs/usb
mountpoint -q "$usb"
test "$(uname -r)" = 6.18.34-tcl-ice1
echo 'badd55296979ec7f8064149bba77a3a22658f1de0134508efab5dd508834fc0c  /run/initramfs/usb/EFI/BOOT/BOOTAA64.EFI' | sha256sum -c -
test ! -e "$usb/EFI/BOOT/BOOTAA64-before-adsp1.bak"
test ! -e /lib/firmware/qcom/sc7180/tcl/b220g/qcadsp7180.mbn
for d in sda sdb sdc sdd sde sdf; do test "$(cat /sys/block/$d/ro)" = 1; done
systemd-analyze verify ./tcl-adsp1-collect.service
mkdir -p "$usb/tcl-adsp1"
cp adsp-probe.dtb "$usb/tcl-adsp1/"
cmp adsp-probe.dtb "$usb/tcl-adsp1/adsp-probe.dtb"
install -m644 tcl-adsp1-collect.service /etc/systemd/system/
systemctl daemon-reload
systemctl enable tcl-adsp1-collect.service
cp "$usb/EFI/BOOT/BOOTAA64.EFI" "$usb/EFI/BOOT/BOOTAA64-before-adsp1.bak"
cp BOOTAA64-adsp1.EFI "$usb/EFI/BOOT/BOOTAA64-adsp1.new"
cmp BOOTAA64-adsp1.EFI "$usb/EFI/BOOT/BOOTAA64-adsp1.new"
sync
mv "$usb/EFI/BOOT/BOOTAA64-adsp1.new" "$usb/EFI/BOOT/BOOTAA64.EFI"
sync
echo ADSP1_INSTALLED_FIRMWARE_STILL_STAGED_ONLY
