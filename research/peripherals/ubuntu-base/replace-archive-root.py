#!/usr/bin/env python3
"""Replace only rootfs.img in a streaming decrypted USB tar.gz archive."""
import sys
import tarfile
from pathlib import Path

replacement = Path(sys.argv[1])
count = 0
with tarfile.open(fileobj=sys.stdin.buffer, mode='r|gz') as source:
    with tarfile.open(fileobj=sys.stdout.buffer, mode='w|gz', compresslevel=6) as target:
        for member in source:
            if member.name.removeprefix('./') == 'tcl-ubuntu/rootfs.img':
                if member.size != replacement.stat().st_size:
                    raise SystemExit('Replacement root image size differs')
                with replacement.open('rb') as stream:
                    target.addfile(member, stream)
                count += 1
            else:
                target.addfile(member, source.extractfile(member) if member.isfile() else None)
while sys.stdin.buffer.read(1024 * 1024):
    pass
if count != 1:
    raise SystemExit(f'Expected exactly one rootfs.img, found {count}')
print('Replaced exactly one rootfs.img; other archive members copied unchanged', file=sys.stderr)
