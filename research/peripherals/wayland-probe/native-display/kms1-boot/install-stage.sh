#!/bin/sh
# Stage only: never reboot and never replace the running root image/kernel files.
set -eu
stage=$1
cd "$stage"
sha256sum -c SHA256SUMS
[ "$(uname -r)" = 6.18.34-stb-qc7+ ]
[ "$(cat /proc/sys/kernel/random/boot_id)" = e7269aed-4e6c-42d4-a2c7-fefdad4573cd ]
usb=/run/initramfs/usb
mountpoint -q "$usb"
[ "$(cat "$usb/tcl-diag/TCL-RAM-TEST")" = 'TCL B220G experimental RAM-only kernel diagnostic' ]
printf '%s  %s\n' c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52 "$usb/EFI/BOOT/BOOTAA64.EFI" | sha256sum -c -
[ ! -e "$usb/tcl-kms1" ]
[ ! -e /lib/modules/6.18.34-tcl-kms1 ]
[ "$(df -Pk / | awk 'NR==2 {print $4}')" -gt 150000 ]
[ "$(df -Pk "$usb" | awk 'NR==2 {print $4}')" -gt 150000 ]
/run/initramfs/bin/busybox sh -n init
/run/initramfs/bin/busybox sh -n shutdown
/bin/sh -n collect
python3 - <<'PY'
import tarfile
with tarfile.open('modules.tar.gz') as t:
 for m in t.getmembers():
  assert m.name=='lib/modules/6.18.34-tcl-kms1' or m.name.startswith('lib/modules/6.18.34-tcl-kms1/')
  assert '..' not in m.name.split('/') and (m.isfile() or m.isdir())
PY
mkdir /lib/modules/6.18.34-tcl-kms1
tar -xzf modules.tar.gz --strip-components=3 -C /lib/modules/6.18.34-tcl-kms1
depmod 6.18.34-tcl-kms1
# Verify auto-load suppression and explicit-load resolution without loading anything.
printf 'blacklist msm\nblacklist tcl_lt8911_handoff\n' > "$stage/defer.conf"
for module in msm tcl_lt8911_handoff; do
 modprobe -S 6.18.34-tcl-kms1 -C "$stage/defer.conf" -b -n -v "$module" > "$stage/blacklist-$module.txt"
 [ ! -s "$stage/blacklist-$module.txt" ]
 modprobe -S 6.18.34-tcl-kms1 -C "$stage/defer.conf" --show-depends "$module" > "$stage/explicit-$module.txt"
 grep -q '^insmod ' "$stage/explicit-$module.txt"
done
modprobe -S 6.18.34-tcl-kms1 --show-depends ath10k_snoc > "$stage/wifi-dependencies.txt"
mkdir "$usb/tcl-kms1"
cp Image initramfs.cpio.gz kms1.dtb "$usb/tcl-kms1/"
for file in Image initramfs.cpio.gz kms1.dtb; do
 cmp "$file" "$usb/tcl-kms1/$file"
done
sync
printf 'KMS1 staging complete; EFI unchanged, no reboot.\n'
