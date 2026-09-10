#!/usr/bin/env python3
"""Reproducible autonomous ARM64 ACPI diagnostic initramfs."""
import gzip
import sys
from pathlib import Path
base = Path(__file__).resolve().parent
archive = bytearray()
def add(name, mode, data=b'', major=0, minor=0):
    name = name.encode() + b'\0'
    fields = [1, mode, 0, 0, 1, 0, len(data), 0, 0, major, minor, len(name), 0]
    archive.extend(b'070701' + ''.join(f'{v:08x}' for v in fields).encode() + name)
    archive.extend(b'\0' * (-len(archive) % 4))
    archive.extend(data)
    archive.extend(b'\0' * (-len(archive) % 4))
for d in ['bin', 'sbin', 'dev', 'proc', 'sys', 'run', 'tmp', 'mnt', 'mnt/usb', 'lib']:
    add(d, 0o040755)
add('dev/console', 0o020600, major=5, minor=1)
add('bin/busybox', 0o100755, Path(sys.argv[1]).read_bytes())
add('init', 0o100755, (base / 'init').read_bytes())
runtime = Path(sys.argv[3])
add('sbin/blkid', 0o100755, (runtime / 'blkid').read_bytes())
for lib in ['libblkid.so.1', 'libc.so.6', 'ld-linux-aarch64.so.1']:
    add('lib/' + lib, 0o100755, (runtime / lib).read_bytes())
add('TRAILER!!!', 0)
Path(sys.argv[2]).write_bytes(gzip.compress(archive, mtime=0))
