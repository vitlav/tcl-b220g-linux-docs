#!/bin/sh
# The USB is already mounted and the Ubuntu root image must be unmounted.
set -eu
[ -f /run/final-save-complete ]
[ "$(cat /mnt/usb/tcl-diag/TCL-RAM-TEST)" = 'TCL B220G experimental RAM-only kernel diagnostic' ]
[ -f /run/ubuntu-usb-device ]
! grep -q ' /mnt/ubuntu ' /proc/mounts
echo 'ddf237a46c16f6921937f3fd9d83b92a4a63e90cbddb6b240360ff9c2d266485  /mnt/usb/EFI/BOOT/BOOTAA64.EFI' | sha256sum -c -
echo 'f88a6854d0e1085b32b5f92cbca01e391b61248d512bcf00003ee080a83f7713  /mnt/usb/tcl-cpufreq-probe/sc7180-tcl-cpufreq.dtb' | sha256sum -c -
cp /run/ubuntu-boot/initramfs.cpio.gz /mnt/usb/tcl-ubuntu/initramfs.cpio.gz.new
echo '81f3a80c0c8fea648d705016fcfd055ea249f064ab872d769603763f845d863c  /mnt/usb/tcl-ubuntu/initramfs.cpio.gz.new' | sha256sum -c -
mv /mnt/usb/tcl-ubuntu/initramfs.cpio.gz.new /mnt/usb/tcl-ubuntu/initramfs.cpio.gz
[ -e /mnt/usb/EFI/BOOT/BOOTAA64-before-ubuntu.bak ] || cp /mnt/usb/EFI/BOOT/BOOTAA64.EFI /mnt/usb/EFI/BOOT/BOOTAA64-before-ubuntu.bak
cp /run/ubuntu-boot/BOOTAA64.EFI /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new
echo '16bfa183c9d687ff54b8e136d8ab51ab9a08da863fef0eb68e4ae9ad21118f3e  /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new' | sha256sum -c -
mv /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new /mnt/usb/EFI/BOOT/BOOTAA64.EFI
sync
umount /run/ubuntu-bootstrap/mnt/usb
umount /mnt/usb
mount -t vfat -o ro,nosuid,nodev,noexec "$(cat /run/ubuntu-usb-device)" /mnt/usb
echo '16bfa183c9d687ff54b8e136d8ab51ab9a08da863fef0eb68e4ae9ad21118f3e  /mnt/usb/EFI/BOOT/BOOTAA64.EFI' | sha256sum -c -
echo '81f3a80c0c8fea648d705016fcfd055ea249f064ab872d769603763f845d863c  /mnt/usb/tcl-ubuntu/initramfs.cpio.gz' | sha256sum -c -
echo 'Ubuntu boot files read-back verified. USB remains read-only for backup.'
