#!/usr/bin/env python3
from pathlib import Path
import subprocess, shutil, re, hashlib
root = Path(__file__).resolve().parent
base = root.parent/'power-storage-video/sc7180-tcl-ec-bus.dtb'
out = root/'sc7180-tcl-ufs.dtb'
shutil.copyfile(base, out)
def get(node, prop):
    return subprocess.check_output(['fdtget','-t','x',str(out),node,prop],text=True).strip()
def put(node, prop, *values, kind='x'):
    subprocess.run(['fdtput','-t',kind,str(out),node,prop,*map(str,values)],check=True)
flat = subprocess.run(['dtc','-I','dtb','-O','dts',str(base)],capture_output=True,text=True,check=True).stdout
nextph = max(int(x,16) for x in re.findall(r'phandle = <0x([0-9a-f]+)>;',flat))+1
reg = '/soc@0/rsc@18200000/regulators-0'
mode = get(reg+'/ldo4','regulator-initial-mode')
for name,uv in [('ldo12',1800000),('ldo19',2960000)]:
    node=reg+'/'+name
    subprocess.run(['fdtput','-c',str(out),node],check=True)
    put(node,'phandle',f'{nextph:x}'); nextph+=1
    put(node,'regulator-min-microvolt',f'{uv:x}')
    put(node,'regulator-max-microvolt',f'{uv:x}')
    put(node,'regulator-initial-mode',mode)
host='/soc@0/ufshc@1d84000';phy='/soc@0/phy@1d87000'
put(host,'vcc-supply',get(reg+'/ldo19','phandle'))
put(host,'vccq2-supply',get(reg+'/ldo12','phandle'))
put(phy,'vdda-phy-supply',get(reg+'/ldo4','phandle'))
put(phy,'vdda-pll-supply',get('/soc@0/rsc@18200000/regulators-1/ldo3','phandle'))
subprocess.run(['fdtput','-d',str(out),host,'qcom,ice'],check=True)
put(host,'status','okay',kind='s');put(phy,'status','okay',kind='s')
print(hashlib.sha256(out.read_bytes()).hexdigest(),out.name)
