#!/usr/bin/env python3
"""Verify a decrypted streaming tar.gz without writing its private contents."""
import hashlib
import sys
import tarfile
from pathlib import Path

expected = {}
for line in Path(sys.argv[1]).read_text().splitlines():
    digest, name = line.split(None, 1)
    expected[name.removeprefix('/mnt/usb/')] = digest
seen = {}
with tarfile.open(fileobj=sys.stdin.buffer, mode='r|gz') as archive:
    for member in archive:
        if not member.isfile():
            continue
        name = member.name.removeprefix('./')
        digest = hashlib.sha256()
        stream = archive.extractfile(member)
        while block := stream.read(1024 * 1024):
            digest.update(block)
        value = digest.hexdigest()
        seen[name] = value
        print(f'{value}  {name}')
# Drain stdin so that GnuPG can verify the complete encrypted stream/MDC.
while sys.stdin.buffer.read(1024 * 1024):
    pass
for name, digest in expected.items():
    if seen.get(name) != digest:
        raise SystemExit(f'Archive checksum mismatch: {name}')
print(f'Verified {len(seen)} files and {len(expected)} reference hashes', file=sys.stderr)
