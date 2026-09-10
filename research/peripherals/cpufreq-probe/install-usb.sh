#!/bin/sh
set -eu
[ -f /run/final-save-complete ]
! ps | grep -q "[b]in/autoreboot"
expected_init=f88a6854d0e1085b32b5f92cbca01e391b61248d512bcf00003ee080a83f7713
expected_efi=ddf237a46c16f6921937f3fd9d83b92a4a63e90cbddb6b240360ff9c2d266485
for part in /sys/class/block/*; do
 [ -f "$part/partition" ] || continue
 ancestor=$(readlink -f "$part"); matched=no
 while [ "$ancestor" != / ] && [ -n "$ancestor" ]; do
  if [ -f "$ancestor/serial" ] && [ "$(cat "$ancestor/serial")" = 001CC0EC34E4FBB085C323F2 ]; then matched=yes; break; fi
  ancestor=${ancestor%/*}
 done
 [ "$matched" = yes ] || continue
 node=/dev/${part##*/}
 mount -t vfat -o ro,nosuid,nodev,noexec "$node" /mnt/usb
 trap 'umount /mnt/usb' EXIT
 [ "$(cat /mnt/usb/tcl-diag/TCL-RAM-TEST)" = 'TCL B220G experimental RAM-only kernel diagnostic' ]
 echo "133d0eae4c4475910cfaf4f8d3db989ea3d0485c1f290cafe9020e0149864e2c  /mnt/usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
 mount -o remount,rw /mnt/usb
 mkdir -p /mnt/usb/tcl-cpufreq-probe
 cp /run/cpufreq-update/sc7180-tcl-cpufreq.dtb /mnt/usb/tcl-cpufreq-probe/sc7180-tcl-cpufreq.dtb.new
 echo "$expected_init  /mnt/usb/tcl-cpufreq-probe/sc7180-tcl-cpufreq.dtb.new" | sha256sum -c -
 mv /mnt/usb/tcl-cpufreq-probe/sc7180-tcl-cpufreq.dtb.new /mnt/usb/tcl-cpufreq-probe/sc7180-tcl-cpufreq.dtb
 backup=/mnt/usb/EFI/BOOT/BOOTAA64-before-cpufreq.bak
 [ -f "$backup" ] || cp /mnt/usb/EFI/BOOT/BOOTAA64.EFI "$backup"
 cp /run/cpufreq-update/BOOTAA64.EFI /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new
 echo "$expected_efi  /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new" | sha256sum -c -
 mv /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new /mnt/usb/EFI/BOOT/BOOTAA64.EFI
 sync
 umount /mnt/usb
 trap - EXIT
 mount -t vfat -o ro,nosuid,nodev,noexec "$node" /mnt/usb
 trap 'umount /mnt/usb' EXIT
 echo "$expected_init  /mnt/usb/tcl-cpufreq-probe/sc7180-tcl-cpufreq.dtb" | sha256sum -c -
 echo "$expected_efi  /mnt/usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
 echo 'USB CPU frequency test installed and read-back verified; no reboot.'
 exit 0
done
exit 1
