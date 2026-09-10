#!/usr/bin/env python3
import gzip
import sys
from pathlib import Path
root = Path(__file__).resolve().parent
busybox = Path(sys.argv[1]).read_bytes()
archive = bytearray()
def add(name, mode, data=b'', rmajor=0, rminor=0):
    name = name.encode()+b'\0'
    fields=[1,mode,0,0,1,0,len(data),0,0,rmajor,rminor,len(name),0]
    archive.extend(b'070701'+''.join(f'{x:08x}' for x in fields).encode()+name)
    archive.extend(b'\0'*(-len(archive)%4)); archive.extend(data)
    archive.extend(b'\0'*(-len(archive)%4))
for d in ['bin','dev','proc','sys']: add(d,0o040755)
add('dev/console',0o020600,rmajor=5,rminor=1)
add('bin/busybox',0o100755,busybox)
add('init',0o100755,(root/'init').read_bytes())
add('TRAILER!!!',0)
(root/'initramfs.cpio.gz').write_bytes(gzip.compress(archive,mtime=0))
