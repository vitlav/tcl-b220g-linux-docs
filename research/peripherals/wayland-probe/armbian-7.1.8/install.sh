#!/bin/sh
# Install a separate kernel/module set; do not run Armbian package hooks.
set -eu
usb=/run/initramfs/usb
stage=/var/tmp/tcl-armbian-718-stage
release=7.1.8-edge-arm64
[ "$(cat /proc/sys/kernel/random/boot_id)" = 192b0902-e812-4c64-9c93-ef4b64ebb903 ]
[ "$(uname -r)" = 6.18.34-stb-qc7+ ]
[ "$(findmnt -n -o FSTYPE --target "$usb")" = vfat ]
[ "$(cat "$usb/tcl-diag/TCL-RAM-TEST")" = 'TCL B220G experimental RAM-only kernel diagnostic' ]
[ ! -e "/lib/modules/$release" ]
[ ! -e "$stage" ]
[ ! -e "$usb/tcl-armbian-7.1.8" ]
echo '930cf69a90cbb906afb07324715961f38fd089611dac720fd860f1cb7979a6f8  /var/tmp/tcl-armbian-718-deploy.tar.gz' | sha256sum -c -
echo "c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52  $usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
echo "1929faac529880bcf7bd87d83ce30cccf0bf28c074141b61657af81fb44cc71c  $usb/tcl-graphics-probe/sc7180-tcl-i2c10.dtb" | sha256sum -c -
mkdir -m 700 "$stage"
tar --no-same-owner -xzf /var/tmp/tcl-armbian-718-deploy.tar.gz -C "$stage"
cd "$stage"
sha256sum --quiet -c SHA256SUMS
echo 'All 6648 deployment file checksums verified.'
backup=$usb/EFI/BOOT/BOOTAA64-before-armbian-718.bak
[ ! -e "$backup" ]
cp "$usb/EFI/BOOT/BOOTAA64.EFI" "$backup"
sync
echo "c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52  $backup" | sha256sum -c -
mkdir "$usb/tcl-armbian-7.1.8"
cp Image initramfs.cpio.gz "$usb/tcl-armbian-7.1.8/"
echo "234870ec642388cd341dfc2c7144bf8d1e97d27c73525c8bfc52122269f158f5  $usb/tcl-armbian-7.1.8/Image" | sha256sum -c -
echo "dd9368b711b5e1ab6746fafb3270c0b2888de52e9704e906d5eaa8ea11e0102f  $usb/tcl-armbian-7.1.8/initramfs.cpio.gz" | sha256sum -c -
mv "modules/$release" /lib/modules/
sed -n 's@  modules/@  /lib/modules/@p' SHA256SUMS > installed-modules.sha256
sha256sum --quiet -c installed-modules.sha256
echo 'Installed module checksums verified.'
sync
cp BOOTAA64.EFI "$usb/EFI/BOOT/BOOTAA64.EFI.new"
echo "d321a865be1fb4d71ee4dd29c45d24a76b21062ca3367377b071615c7a5135f9  $usb/EFI/BOOT/BOOTAA64.EFI.new" | sha256sum -c -
sync
mv "$usb/EFI/BOOT/BOOTAA64.EFI.new" "$usb/EFI/BOOT/BOOTAA64.EFI"
sync
echo "d321a865be1fb4d71ee4dd29c45d24a76b21062ca3367377b071615c7a5135f9  $usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
echo 'Armbian installed, default=tcl-armbian-718. No reboot yet.'
