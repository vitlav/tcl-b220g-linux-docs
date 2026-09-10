#!/usr/bin/env python3
"""Compare restored Borg stream with the independently recorded source SHA256."""
import hashlib
import json
import sys
from pathlib import Path

report = json.loads(Path(sys.argv[1]).read_text())
digest = hashlib.sha256()
total = 0
while data := sys.stdin.buffer.read(4 * 1024 ** 2):
    total += len(data)
    digest.update(data)
if total != report['bytes'] or digest.hexdigest() != report['sha256']:
    raise SystemExit(f'Restored data differs: {total} bytes, {digest.hexdigest()}')
print(json.dumps({'lun': report['lun'], 'bytes': total, 'sha256': digest.hexdigest(), 'restored_verified': True}))
