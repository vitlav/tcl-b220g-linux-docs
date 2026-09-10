#!/bin/sh
set -eu
cd "$1"
sha256sum -c SHA256SUMS.install
[ "$(uname -r)" = 6.18.34-tcl-kms1 ]
[ "$(cat /proc/sys/kernel/random/boot_id)" = eb351fb4-4b70-454a-8425-1047c5c4c9aa ]
usb=/run/initramfs/usb
module=/lib/modules/6.18.34-tcl-kms1/extra/tcl_lt8911_handoff.ko
printf '%s  %s\n' eb879e9f9e4c021d49b4fd7fcd14d8040aef46199836227eb4eb83a78979e545 "$module" | sha256sum -c -
printf '%s  %s\n' d9fc63ce9c9d409b2133a5b3234492f06970c48b9757fd8d216e569062ca77df "$usb/tcl-kms1/initramfs.cpio.gz" | sha256sum -c -
[ ! -e /var/tmp/tcl-kms1-v1-handoff.ko ]
[ ! -e "$usb/tcl-kms1/initramfs-v1.cpio.gz" ]
cp "$module" /var/tmp/tcl-kms1-v1-handoff.ko
cp "$usb/tcl-kms1/initramfs.cpio.gz" "$usb/tcl-kms1/initramfs-v1.cpio.gz"
cp tcl_lt8911_handoff.ko "$module.new"
cmp tcl_lt8911_handoff.ko "$module.new"
cp initramfs.cpio.gz "$usb/tcl-kms1/initramfs-v2.new"
cmp initramfs.cpio.gz "$usb/tcl-kms1/initramfs-v2.new"
sync
mv "$module.new" "$module"
depmod 6.18.34-tcl-kms1
mv "$usb/tcl-kms1/initramfs-v2.new" "$usb/tcl-kms1/initramfs.cpio.gz"
sync
sha256sum "$module" "$usb/tcl-kms1/initramfs.cpio.gz"
echo 'V2 installed on disk; currently loaded V1 was not unloaded or changed.'
