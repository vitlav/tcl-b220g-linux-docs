import os, struct, json, base64, hashlib
from pathlib import Path
LOW,HIGH=0xfff22000,0xffffe000
s=Path('/sys/firmware/efi/systab').read_text()
addr=int(next(x.split('=')[1] for x in s.splitlines() if x.startswith('ACPI20=')),16)
fd=os.open('/dev/mem',os.O_RDONLY)
def read(a,n):
    if not (LOW<=a and 0<n<=0x100000 and a+n<=HIGH):
        raise ValueError(f'Outside verified ACPI reserved range: {a:#x}/{n}')
    b=os.pread(fd,n,a)
    if len(b)!=n: raise ValueError('Short read')
    return b
rows=[]
def table(a):
    h=read(a,36); sig=h[:4].decode('ascii'); n=struct.unpack_from('<I',h,4)[0]
    if sig=='MSDM':
        rows.append({'address':hex(a),'signature':sig,'skipped':True}); return sig,None
    b=read(a,n)
    if sum(b)%256: raise ValueError(f'Bad checksum {sig}')
    rows.append({'address':hex(a),'signature':sig,'length':n,'sha256':hashlib.sha256(b).hexdigest(),'base64':base64.b64encode(b).decode()})
    return sig,b
r=read(addr,36)
assert r[:8]==b'RSD PTR ' and sum(r[:20])%256==0 and sum(r)%256==0
x=struct.unpack_from('<Q',r,24)[0]
sig,b=table(x); assert sig=='XSDT' and (len(b)-36)%8==0
entries=struct.unpack('<'+'Q'*((len(b)-36)//8),b[36:])
assert len(entries)<=64
for a in entries:
    sig,b=table(a)
    if sig=='FACP':
        dsdt=struct.unpack_from('<Q',b,140)[0] if len(b)>=148 else 0
        dsdt=dsdt or struct.unpack_from('<I',b,40)[0]
        dsig,_=table(dsdt); assert dsig=='DSDT'
os.close(fd)
print(json.dumps({'boot_id':Path('/proc/sys/kernel/random/boot_id').read_text().strip(),'rsdp':hex(addr),'tables':rows}))
