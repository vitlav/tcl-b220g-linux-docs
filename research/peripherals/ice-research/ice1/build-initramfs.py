#!/usr/bin/env python3
"""Package ready ARM64 BusyBox and kernel modules; no compilation."""
import gzip
import sys
from pathlib import Path

source = Path(__file__).resolve().parent
busybox, modules, firmware, output = map(Path, sys.argv[1:])
archive = bytearray()
seen = set()

def add(name, mode, data=b'', major=0, minor=0):
    if name in seen:
        return
    seen.add(name)
    name = name.encode() + b'\0'
    fields = [1, mode, 0, 0, 1, 0, len(data), 0, 0, major, minor, len(name), 0]
    archive.extend(b'070701' + ''.join(f'{x:08x}' for x in fields).encode() + name)
    archive.extend(b'\0' * (-len(archive) % 4))
    archive.extend(data)
    archive.extend(b'\0' * (-len(archive) % 4))

for directory in ['bin', 'sbin', 'dev', 'proc', 'sys', 'run', 'tmp', 'newroot', 'lib', 'lib/modules']:
    add(directory, 0o040755)
add('dev/console', 0o020600, major=5, minor=1)
add('bin/busybox', 0o100755, busybox.read_bytes())
for name in ['init', 'shutdown', 'collect']:
    add(name, 0o100755, (source / name).read_bytes())
for path in sorted(modules.rglob('*')):
    if path.is_symlink():
        continue
    name = 'lib/modules/' + str(path.relative_to(modules))
    if path.is_dir():
        add(name, 0o040755)
    elif path.is_file():
        add(name, 0o100644, path.read_bytes())
add('lib/firmware', 0o040755)
for path in sorted(firmware.rglob('*')):
    if path.is_symlink():
        raise ValueError(f'Unexpected firmware symlink: {path}')
    name = 'lib/firmware/' + str(path.relative_to(firmware))
    if path.is_dir():
        add(name, 0o040755)
    elif path.is_file():
        add(name, 0o100644, path.read_bytes())
add('TRAILER!!!', 0)
output.write_bytes(gzip.compress(archive, mtime=0))
