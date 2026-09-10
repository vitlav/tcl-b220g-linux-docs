#!/bin/sh
set -eu
[ -f /run/final-save-complete ]
! ps | grep -q "[b]in/autoreboot"
expected_init=a64d18bbb1034575047736bf3d476766fff16e7c65e47aeb5b71750ef9026509
expected_efi=1994f6275ce291c6e28f7df711629e130babbc70678898c6ef010066f2099432
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
 echo "3abba529d5e6f2668bc3eed21aebb38ec01b094970e631540c370ac9ca3528c2  /mnt/usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
 mount -o remount,rw /mnt/usb
 mkdir -p /mnt/usb/tcl-ec-bus
 cp /run/ec-update/sc7180-tcl-ec-bus.dtb /mnt/usb/tcl-ec-bus/sc7180-tcl-ec-bus.dtb.new
 echo "$expected_init  /mnt/usb/tcl-ec-bus/sc7180-tcl-ec-bus.dtb.new" | sha256sum -c -
 mv /mnt/usb/tcl-ec-bus/sc7180-tcl-ec-bus.dtb.new /mnt/usb/tcl-ec-bus/sc7180-tcl-ec-bus.dtb
 backup=/mnt/usb/EFI/BOOT/BOOTAA64-before-ec.bak
 [ -f "$backup" ] || cp /mnt/usb/EFI/BOOT/BOOTAA64.EFI "$backup"
 cp /run/ec-update/BOOTAA64.EFI /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new
 echo "$expected_efi  /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new" | sha256sum -c -
 mv /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new /mnt/usb/EFI/BOOT/BOOTAA64.EFI
 sync
 umount /mnt/usb
 trap - EXIT
 mount -t vfat -o ro,nosuid,nodev,noexec "$node" /mnt/usb
 trap 'umount /mnt/usb' EXIT
 echo "$expected_init  /mnt/usb/tcl-ec-bus/sc7180-tcl-ec-bus.dtb" | sha256sum -c -
 echo "$expected_efi  /mnt/usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
 echo 'USB EC test installed and read-back verified; no reboot.'
 exit 0
done
exit 1
