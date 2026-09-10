from pathlib import Path
import struct,hashlib,json,sys
r=Path(sys.argv[1]);b=(r/'ath10k/WCN3990/hw1.0/board-2.bin').read_bytes();magic=b'QCA-ATH10K-BOARD\0';assert b.startswith(magic);header=(len(magic)+3)&~3
def tlvs(b):
 off=0
 while off<len(b):
  assert off+8<=len(b)
  ident,n=struct.unpack_from('<II',b,off);off+=8;assert off+n<=len(b)
  yield ident,b[off:off+n]
  off+=(n+3)&~3
 assert off==len(b)
rows=[]
for ident,payload in tlvs(b[header:]):
 assert ident==0
 names=[];data=[]
 for kind,v in tlvs(payload):
  if kind==0:names.append(v.decode())
  elif kind==1:data.append({'bytes':len(v),'sha256':hashlib.sha256(v).hexdigest()})
  else:raise ValueError(kind)
 rows.append({'names':names,'data':data})
(r/'board-records.json').write_text(json.dumps(rows,indent=2)+'\n')
needle='e937bf4779ea30d588c3c9dab82913cd9e7fd31faaa45bde76b9fca876c0ad3c'
print('Records',len(rows),'names',sum(len(x['names']) for x in rows));print('OEM bdwlan.bin payload matches:',sum(d['sha256']==needle for x in rows for d in x['data']))
print('ECS entries:',[x for x in rows if any('ECS' in n for n in x['names'])])
print('TCL entries:',[x for x in rows if any('TCL' in n.upper() for n in x['names'])])
