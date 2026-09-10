#!/usr/bin/env python3
"""Build a deterministic, uncompressed early ACPI archive; never install it."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess


def entry(name, data=b'', mode=0o100644, ino=1):
    name = name.encode('ascii') + b'\0'
    fields = [ino, mode, 0, 0, 1, 0, len(data), 0, 0, 0, 0, len(name), 0]
    result = b'070701' + b''.join(f'{n:08x}'.encode() for n in fields) + name
    result += b'\0' * (-len(result) % 4)
    result += data
    return result + b'\0' * (-len(result) % 4)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--asl', type=Path, default=Path(__file__).with_name('scm-cca0.asl'))
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    prefix = args.output / 'scm-cca0'
    run = subprocess.run(['iasl', '-p', str(prefix), str(args.asl)], capture_output=True, text=True)
    (args.output / 'iasl.log').write_text(run.stdout + run.stderr)
    run.check_returncode()
    aml = prefix.with_suffix('.aml').read_bytes()
    if len(aml) < 36 or aml[:4] != b'SSDT' or struct.unpack_from('<I', aml, 4)[0] != len(aml) or sum(aml) % 256:
        raise ValueError('Invalid SSDT signature, size or checksum')
    if aml[10:16] != b'TCLLAB' or aml[16:24] != b'SCMCCA0\0':
        raise ValueError('Unexpected OEM table identity')
    result = b''.join(entry(n, mode=0o40755, ino=i) for i, n in enumerate(
        ['kernel', 'kernel/firmware', 'kernel/firmware/acpi'], 1))
    result += entry('kernel/firmware/acpi/scm-cca0.aml', aml, ino=4)
    result += entry('TRAILER!!!', mode=0, ino=5)
    result += b'\0' * (-len(result) % 512)
    archive = args.output / 'scm-cca0-early.cpio'
    archive.write_bytes(result)
    manifest = {'format': 'uncompressed newc', 'member': 'kernel/firmware/acpi/scm-cca0.aml',
                'files': {p.name: {'bytes': p.stat().st_size, 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()}
                          for p in (args.asl, prefix.with_suffix('.aml'), archive)}}
    (args.output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(json.dumps(manifest, indent=2))


if __name__ == '__main__':
    main()
