#!/bin/sh
set -eu
[ -f /run/final-save-complete ]
! ps | grep -q "[b]in/autoreboot"
expected_init=84b5858185f35a2c6922da5befc47c912d793b655f45e44d11494f63d689d856
expected_efi=133d0eae4c4475910cfaf4f8d3db989ea3d0485c1f290cafe9020e0149864e2c
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
 echo "727e5b17a06dd35f83741a084a242c12ab6ba6900c96a4b26cf59d7b87836501  /mnt/usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
 mount -o remount,rw /mnt/usb
 mkdir -p /mnt/usb/tcl-ufs-probe
 cp /run/ufs-update/sc7180-tcl-ufs.dtb /mnt/usb/tcl-ufs-probe/sc7180-tcl-ufs.dtb.new
 echo "$expected_init  /mnt/usb/tcl-ufs-probe/sc7180-tcl-ufs.dtb.new" | sha256sum -c -
 mv /mnt/usb/tcl-ufs-probe/sc7180-tcl-ufs.dtb.new /mnt/usb/tcl-ufs-probe/sc7180-tcl-ufs.dtb
 backup=/mnt/usb/EFI/BOOT/BOOTAA64-before-ufs.bak
 [ -f "$backup" ] || cp /mnt/usb/EFI/BOOT/BOOTAA64.EFI "$backup"
 cp /run/ufs-update/BOOTAA64.EFI /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new
 echo "$expected_efi  /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new" | sha256sum -c -
 mv /mnt/usb/EFI/BOOT/BOOTAA64.EFI.new /mnt/usb/EFI/BOOT/BOOTAA64.EFI
 sync
 umount /mnt/usb
 trap - EXIT
 mount -t vfat -o ro,nosuid,nodev,noexec "$node" /mnt/usb
 trap 'umount /mnt/usb' EXIT
 echo "$expected_init  /mnt/usb/tcl-ufs-probe/sc7180-tcl-ufs.dtb" | sha256sum -c -
 echo "$expected_efi  /mnt/usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
 echo 'USB UFS test installed and read-back verified; no reboot.'
 exit 0
done
exit 1
