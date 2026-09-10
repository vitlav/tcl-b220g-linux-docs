#!/usr/bin/env python3
"""Static call-site index, NOT an executable init sequence or control-flow emulator."""
import re
from pathlib import Path
# graphics-research -> peripherals -> data
source=Path(__file__).resolve().parents[2]/'EDPBridge-disassembly.txt'
rows=[]
args={}
for line in source.read_text().splitlines():
    m=re.match(r'([0-9a-f]+):\s+[0-9a-f]+\s+([a-z0-9.]+)\s*(.*)',line)
    if not m: continue
    addr,op,tail=m.groups()
    dest=re.match(r'w([12]),\s*(.*)',tail)
    if dest:
        n,val=dest.groups()
        imm=re.match(r'#(0x[0-9a-f]+|[0-9]+)',val)
        args[n]=f'0x{int(imm[1],0):02x}' if op=='mov' and imm else f'{op} {val.split("//")[0].strip()}'
    if op=='bl':
        if tail.startswith('0x140003988 '):
            rows.append(f'0x{addr}\t{args.get("1","unknown")}\t{args.get("2","unknown")}')
        args={}
    elif op in ('b','ret','blr') or op.startswith('b.') or op in ('cbz','cbnz','tbz','tbnz'):
        args={}
print('call_address\tregister_argument\tvalue_argument')
print('\n'.join(rows))
