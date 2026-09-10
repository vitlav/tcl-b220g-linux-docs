#!/usr/bin/env python3
"""Inspect DTB supplier references without guessing provider cell counts."""
import json, struct, sys
from pathlib import Path
b=Path(sys.argv[1]).read_bytes()
h=struct.unpack_from('>10I',b)
assert h[0]==0xd00dfeed
pos,strings=h[2],h[3]
nodes={};stack=[]
def u32(data):
 assert len(data)%4==0
 return list(struct.unpack('>'+'I'*(len(data)//4),data))
def string(data): return data.rstrip(b'\0').decode()
while True:
 token=struct.unpack_from('>I',b,pos)[0];pos+=4
 if token==1:
  end=b.index(0,pos);name=b[pos:end].decode();pos=(end+4)&~3
  path=(stack[-1].rstrip('/')+'/' if stack else '/')+name
  stack.append(path);nodes[path]={}
 elif token==2:stack.pop()
 elif token==3:
  size,off=struct.unpack_from('>II',b,pos);pos+=8
  end=b.index(0,strings+off);name=b[strings+off:end].decode()
  nodes[stack[-1]][name]=b[pos:pos+size];pos=(pos+size+3)&~3
 elif token==4:pass
 elif token==9:break
 else:raise ValueError(token)
handles={u32(v['phandle'])[0]:p for p,v in nodes.items() if 'phandle' in v}
arrays={'qcom,smem-states':'#qcom,smem-state-cells','clocks':'#clock-cells','assigned-clocks':'#clock-cells',
 'assigned-clock-parents':'#clock-cells','resets':'#reset-cells',
 'power-domains':'#power-domain-cells','interconnects':'#interconnect-cells',
 'iommus':'#iommu-cells','phys':'#phy-cells','dmas':'#dma-cells',
 'mboxes':'#mbox-cells','interrupts-extended':'#interrupt-cells',
 'io-channels':'#io-channel-cells','thermal-sensors':'#thermal-sensor-cells','cooling-device':'#cooling-cells'}
single={'trip','interrupt-parent','operating-points-v2','required-opps','qcom,bcm-voters',
 'nvmem-cells','memory-region','qcom,remoteproc','qcom,smem','qcom,gmu','qcom,ice'}
edges=[];errors=[]
for p,props in nodes.items():
 for k,v in props.items():
  if k not in arrays and k not in single and not k.endswith('-supply') and not k.startswith('pinctrl-'):continue
  if k=='pinctrl-names' or (k=='qcom,smem' and props.get('compatible')==b'qcom,smp2p\0'):continue
  cells=u32(v);i=0
  while i<len(cells):
   ph=cells[i];i+=1
   if ph==0 and k in arrays:continue
   target=handles.get(ph)
   if target is None:errors.append([p,k,'missing phandle',hex(ph)]);break
   edges.append((p,k,target))
   if k in arrays:
    count=nodes[target].get(arrays[k])
    if count is None:errors.append([p,k,'missing provider cell count',target]);break
    i+=u32(count)[0]
   if i>len(cells):errors.append([p,k,'truncated specifier'])
roots=['/soc@0/remoteproc@62400000','/smp2p-lpass','/soc@0/video-codec@aa00000','/soc@0/clock-controller@ab00000','/soc@0/crypto@1d90000','/soc@0/ufshc@1d84000','/thermal-zones','/soc@0/thermal-sensor@c263000','/soc@0/thermal-sensor@c265000','/soc@0/gpu@5000000','/soc@0/gmu@506a000','/soc@0/iommu@5040000','/soc@0/clock-controller@5090000','/soc@0/display-subsystem@ae00000','/soc@0/clock-controller@af00000','/soc@0/interconnect@1740000','/soc@0/geniqup@ac0000/i2c@a90000']
def active_path(p):
 while p:
  if nodes[p].get('status') == b'disabled\0': return False
  p=p.rsplit('/',1)[0]
 return True
seen={p for p in nodes if any(p==r or p.startswith(r+'/') for r in roots) and active_path(p)}
# Follow providers plus ancestors, whose resources/status can gate their children.
while True:
 old=set(seen)
 for p,k,t in edges:
  if p in seen:seen.add(t)
 for p in list(seen):
  parent=p.rsplit('/',1)[0]
  if parent:seen.add(parent)
 if seen==old:break
report={'scope':'DT reference/status audit only; no live bind or hardware proof',
 'reachable_nodes':len(seen),
 'disabled_suppliers':sorted(p for p in seen if nodes[p].get('status')==b'disabled\0'),
 'reference_errors_in_closure':[x for x in errors if x[0] in seen],
 'edges':[list(e) for e in edges if e[0] in seen]}
print(json.dumps(report,indent=2))
