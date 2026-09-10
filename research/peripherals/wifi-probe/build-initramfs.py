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
for d in ['bin','sbin','dev','proc','sys','run','tmp','mnt','mnt/usb','lib','lib/modules']: add(d,0o040755)
add('dev/console',0o020600,rmajor=5,rminor=1)
add('bin/busybox',0o100755,busybox)
add('init',0o100755,(root/'init').read_bytes())
for name in ['dhcp', 'diagnose', 'snapshot']:
    add('bin/'+name,0o100755,(root/name).read_bytes())
# Scripts referenced by PID1 live at the root as well.
for name in ['dhcp', 'diagnose']:
    add(name,0o100755,(root/name).read_bytes())
modules=Path(sys.argv[2])
for path in sorted(modules.rglob('*')):
    if path.is_symlink():
        continue
    name='lib/modules/'+str(path.relative_to(modules))
    if path.is_dir(): add(name,0o040755)
    elif path.is_file(): add(name,0o100644,path.read_bytes())
add('wifi-probe',0o100755,(root/'wifi-probe').read_bytes())
add('lib/firmware',0o040755)
for path in sorted((root/'firmware').rglob('*')):
    name='lib/firmware/'+str(path.relative_to(root/'firmware'))
    if path.is_dir(): add(name,0o040755)
    elif path.is_file(): add(name,0o100644,path.read_bytes())
add('TRAILER!!!',0)
(root/'initramfs.cpio.gz').write_bytes(gzip.compress(archive,mtime=0))
