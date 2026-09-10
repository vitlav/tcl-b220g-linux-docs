#!/usr/bin/env python3
"""Borg content-from-command source: SSH -> zstd -> verified raw LUN bytes."""
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

lun = int(sys.argv[1])
root = Path(sys.argv[2])
boot = '15a44890-c95f-4081-8cab-3d71212e63bb'
expected = [251331084288, 8388608, 8388608, 134217728, 4294967296, 134217728][lun]
os.umask(0o077)
log = root / 'logs' / f'lun{lun}-source.log'
ssh = ['ssh', '-i', '/home/lav/.ssh/id_ed25519', '-o', 'IdentitiesOnly=yes',
       '-o', 'BatchMode=yes', '-o', 'StrictHostKeyChecking=yes',
       '-o', 'UserKnownHostsFile=/tmp/tcl-wifi-known-hosts', '-o', 'ConnectTimeout=8',
       '-o', 'ServerAliveInterval=15', '-o', 'ServerAliveCountMax=4',
       'root@192.168.8.177', f'python3 /run/tcl-read-lun.py {lun} {boot}']
digest = hashlib.sha256()
total = 0
with log.open('wb') as errors:
    remote = subprocess.Popen(ssh, stdout=subprocess.PIPE, stderr=errors)
    decoder = subprocess.Popen(['zstd', '-q', '-d', '-c'], stdin=remote.stdout,
                               stdout=subprocess.PIPE, stderr=errors)
    remote.stdout.close()
    try:
        while data := decoder.stdout.read(4 * 1024 ** 2):
            total += len(data)
            if total > expected:
                raise RuntimeError('More source bytes than expected')
            digest.update(data)
            sys.stdout.buffer.write(data)
        sys.stdout.buffer.flush()
        decode_rc = decoder.wait()
        remote_rc = remote.wait()
        if decode_rc or remote_rc or total != expected:
            raise RuntimeError(f'Incomplete stream: SSH={remote_rc}, zstd={decode_rc}, bytes={total}/{expected}')
    except BaseException:
        for process in [remote, decoder]:
            if process.poll() is None:
                process.terminate()
        for process in [remote, decoder]:
            process.wait()
        raise
complete = [line.removeprefix('TCL_BACKUP_COMPLETE ') for line in log.read_text().splitlines()
            if line.startswith('TCL_BACKUP_COMPLETE ')]
if len(complete) != 1:
    raise SystemExit('Missing unique source checksum report')
report = json.loads(complete[0])
if report['bytes'] != total or report['sha256'] != digest.hexdigest() or report['boot_id'] != boot:
    raise SystemExit('Source and received stream checksums differ')
report['received_sha256'] = digest.hexdigest()
(root / 'logs' / f'lun{lun}-received.json').write_text(json.dumps(report, indent=2) + '\n')
print(f'LUN{lun}: received {total} bytes, SHA256 matches source', file=sys.stderr)
