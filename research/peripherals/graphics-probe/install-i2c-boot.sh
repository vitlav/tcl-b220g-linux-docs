#!/bin/sh
# Install only EFI/DTB files on the already mounted USB; do not touch rootfs.img.
set -eu
usb=/run/initramfs/usb
[ "$(cat /proc/sys/kernel/random/boot_id)" = '15a44890-c95f-4081-8cab-3d71212e63bb' ]
[ "$(findmnt -n -o FSTYPE --target "$usb")" = vfat ]
[ "$(cat "$usb/tcl-diag/TCL-RAM-TEST")" = 'TCL B220G experimental RAM-only kernel diagnostic' ]
echo "16bfa183c9d687ff54b8e136d8ab51ab9a08da863fef0eb68e4ae9ad21118f3e  $usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
echo "f88a6854d0e1085b32b5f92cbca01e391b61248d512bcf00003ee080a83f7713  $usb/tcl-cpufreq-probe/sc7180-tcl-cpufreq.dtb" | sha256sum -c -
echo "81f3a80c0c8fea648d705016fcfd055ea249f064ab872d769603763f845d863c  $usb/tcl-ubuntu/initramfs.cpio.gz" | sha256sum -c -
echo 'c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52  /run/BOOTAA64.EFI' | sha256sum -c -
echo '1929faac529880bcf7bd87d83ce30cccf0bf28c074141b61657af81fb44cc71c  /run/sc7180-tcl-i2c10.dtb' | sha256sum -c -
backup=$usb/EFI/BOOT/BOOTAA64-before-i2c10.bak
if [ ! -e "$backup" ]; then cp "$usb/EFI/BOOT/BOOTAA64.EFI" "$backup"; fi
echo "16bfa183c9d687ff54b8e136d8ab51ab9a08da863fef0eb68e4ae9ad21118f3e  $backup" | sha256sum -c -
mkdir -p "$usb/tcl-graphics-probe"
cp /run/sc7180-tcl-i2c10.dtb "$usb/tcl-graphics-probe/sc7180-tcl-i2c10.dtb.new"
echo "1929faac529880bcf7bd87d83ce30cccf0bf28c074141b61657af81fb44cc71c  $usb/tcl-graphics-probe/sc7180-tcl-i2c10.dtb.new" | sha256sum -c -
mv "$usb/tcl-graphics-probe/sc7180-tcl-i2c10.dtb.new" "$usb/tcl-graphics-probe/sc7180-tcl-i2c10.dtb"
sync
cp /run/BOOTAA64.EFI "$usb/EFI/BOOT/BOOTAA64.EFI.new"
echo "c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52  $usb/EFI/BOOT/BOOTAA64.EFI.new" | sha256sum -c -
mv "$usb/EFI/BOOT/BOOTAA64.EFI.new" "$usb/EFI/BOOT/BOOTAA64.EFI"
sync
echo "c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52  $usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
echo "1929faac529880bcf7bd87d83ce30cccf0bf28c074141b61657af81fb44cc71c  $usb/tcl-graphics-probe/sc7180-tcl-i2c10.dtb" | sha256sum -c -
printf 'I2C10 boot files installed. No reboot performed.\n'
