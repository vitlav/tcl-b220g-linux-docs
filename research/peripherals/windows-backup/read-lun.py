#!/usr/bin/env python3
"""Read one verified, unmounted TCL UFS LUN; emit zstd, never write the device."""
import fcntl
import hashlib
import json
import os
import struct
import subprocess
import sys
import time
from pathlib import Path

SIZES = [251331084288, 8388608, 8388608, 134217728, 4294967296, 134217728]
lun = int(sys.argv[1])
if lun not in range(len(SIZES)):
    raise SystemExit('Invalid LUN')
boot = Path('/proc/sys/kernel/random/boot_id').read_text().strip()
if boot != sys.argv[2]:
    raise SystemExit('Boot ID changed; revalidate storage before backup')
matches = []
for block in Path('/sys/class/block').iterdir():
    if (block / 'partition').exists():
        continue
    resolved = str(block.resolve())
    if '/1d84000.ufshc/' in resolved and f'/0:0:0:{lun}/block/' in resolved:
        matches.append(block)
if len(matches) != 1:
    raise SystemExit(f'Ambiguous UFS LUN: {matches}')
block = matches[0]
if (block / 'device/model').read_text().strip() != 'KM8F9001JM-B813':
    raise SystemExit('Unexpected UFS model')
mounted = {line.split()[2] for line in Path('/proc/self/mountinfo').read_text().splitlines()}
for child in [block, *[p for p in block.iterdir() if (p / 'partition').exists()]]:
    if (child / 'dev').read_text().strip() in mounted:
        raise SystemExit(f'Source is mounted: {child.name}')
device = '/dev/' + block.name
expected = SIZES[lun]
digest = hashlib.sha256()
total = 0
started = time.monotonic()
with open(device, 'rb', buffering=0) as source:
    size = struct.unpack('Q', fcntl.ioctl(source, 0x80081272, bytes(8)))[0]
    readonly = struct.unpack('I', fcntl.ioctl(source, 0x125e, bytes(4)))[0]
    if size != expected or readonly != 1:
        raise SystemExit(f'Unexpected size/read-only state: {size}, {readonly}')
    compressor = subprocess.Popen(['zstd', '-q', '-T2', '-1', '-c'], stdin=subprocess.PIPE)
    next_report = 1024 ** 3
    try:
        while total < expected:
            data = source.read(min(4 * 1024 ** 2, expected - total))
            if not data:
                raise RuntimeError(f'Short read at {total}/{expected}')
            digest.update(data)
            compressor.stdin.write(data)
            total += len(data)
            if total >= next_report:
                print(f'TCL_PROGRESS lun={lun} bytes={total} total={expected} seconds={time.monotonic()-started:.1f}', file=sys.stderr, flush=True)
                next_report += 1024 ** 3
        if source.read(1):
            raise RuntimeError('Source larger than expected')
        compressor.stdin.close()
        if compressor.wait() != 0:
            raise RuntimeError('Compression failed')
    except BaseException:
        if compressor.poll() is None:
            compressor.terminate()
        compressor.wait()
        raise
result = {'lun': lun, 'bytes': total, 'sha256': digest.hexdigest(), 'device': device,
          'sysfs': str(block.resolve()), 'boot_id': boot, 'seconds': time.monotonic()-started}
out = Path('/run/tcl-windows-backup')
out.mkdir(mode=0o700, exist_ok=True)
(out / f'lun{lun}.json').write_text(json.dumps(result, indent=2) + '\n')
print('TCL_BACKUP_COMPLETE ' + json.dumps(result), file=sys.stderr, flush=True)
