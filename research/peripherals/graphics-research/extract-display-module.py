#!/usr/bin/env python3
"""Extract DisplayDxe from the specifically verified TCL XBL; never execute firmware."""
import hashlib,lzma,struct,sys
from pathlib import Path
source=Path(sys.argv[1]).read_bytes()
assert hashlib.sha256(source).hexdigest()=='82dd0864eeb1e8c1289abeed38ae540323e61dd95f0e2ff2ab38a4f33a32136f','Unexpected XBL'
d=lzma.LZMADecompressor(format=lzma.FORMAT_ALONE)
fv=d.decompress(source[0xc4438:],max_length=20000000)
assert d.eof and len(fv)==8859528
start=0x3fae38
pe=start+struct.unpack_from('<I',fv,start+60)[0]
assert fv[start:start+2]==b'MZ' and fv[pe:pe+4]==b'PE\0\0'
n=struct.unpack_from('<H',fv,pe+6)[0]
opt=struct.unpack_from('<H',fv,pe+20)[0]
end=0
for j in range(n):
    size,off=struct.unpack_from('<II',fv,pe+24+opt+40*j+16)
    end=max(end,size+off)
module=fv[start:start+end]
assert len(module)==507904
assert hashlib.sha256(module).hexdigest()=='76bfce3fe980a8c8130849628eee097b8c11af63073e98509652bfc1a5b492f0'
Path(sys.argv[2]).write_bytes(module)
print('Extracted verified ARM64 DisplayDxe, 507904 bytes')
