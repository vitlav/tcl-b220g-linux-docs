#!/usr/bin/env python3
from pathlib import Path
import subprocess,shutil,re,hashlib,difflib
root=Path(__file__).resolve().parent
base=root.parent/'ufs-probe/sc7180-tcl-ufs.dtb'
out=root/'sc7180-tcl-cpufreq.dtb'
shutil.copyfile(base,out)
def flat(path):
 return subprocess.run(['dtc','-I','dtb','-O','dts',str(path)],capture_output=True,text=True,check=True).stdout
before=flat(base)
ph=max(int(x,16) for x in re.findall(r'phandle = <0x([0-9a-f]+)>;',before))+1
node='/soc@0/cpufreq@18323000'
def put(n,p,*v,t='x'):
 subprocess.run(['fdtput','-t',t,str(out),n,p,*map(str,v)],check=True)
put(node,'phandle',f'{ph:x}')
put(node,'status','okay',t='s')
for cpu in range(8):
 n=f'/cpus/cpu@{cpu*256:x}'
 domain=0 if cpu<6 else 1
 put(n,'clocks',f'{ph:x}',domain)
 put(n,'qcom,freq-domain',f'{ph:x}',domain)
after=flat(out)
(root/'dtb.diff').write_text(''.join(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='ufs',tofile='cpufreq')))
print(hashlib.sha256(out.read_bytes()).hexdigest(),out.name)
