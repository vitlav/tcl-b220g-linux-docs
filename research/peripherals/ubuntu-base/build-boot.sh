#!/bin/sh
# Run from the published bundle after extracting grub-2.12-build-tools.tar.gz.
set -eu
[ "$#" = 2 ] || { echo "Usage: $0 /path/to/rootfs/lib/modules /output/directory" >&2; exit 1; }
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
modules=$1
output=$2
mkdir -p "$output"
python3 "$base/build-initramfs.py" "$base/busybox-arm64" "$modules" "$output/initramfs.cpio.gz"
grubtools=$base/grub-tools
LD_LIBRARY_PATH="$grubtools/usr/lib/x86_64-linux-gnu" "$grubtools/usr/bin/grub-script-check" "$base/grub.cfg"
LD_LIBRARY_PATH="$grubtools/usr/lib/x86_64-linux-gnu" "$grubtools/usr/bin/grub-mkstandalone" \
 -O arm64-efi -d "$grubtools/usr/lib/grub/arm64-efi" --locales= --fonts= \
 --modules='normal configfile echo sleep reboot part_gpt part_msdos fat lsefimmap videoinfo efi_gop linux fdt gzio search search_fs_file' \
 -o "$output/BOOTAA64.EFI" "boot/grub/grub.cfg=$base/grub.cfg"
sha256sum "$output/initramfs.cpio.gz" "$output/BOOTAA64.EFI"
