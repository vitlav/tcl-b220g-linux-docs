#!/usr/bin/env python3
"""Test the candidate's actual selector with a captured IORT; no MMIO or SMC."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--kernel',type=Path,required=True)
p.add_argument('--candidate',type=Path,required=True)
p.add_argument('--iort',type=Path,required=True)
a=p.parse_args()
b=a.iort.read_bytes()
assert b[:4]==b'IORT' and len(b)==struct.unpack_from('<I',b,4)[0] and sum(b)%256==0
assert b[10:16]==b'QCOM  ' and b[16:24]==b'QCOMEDK2' and struct.unpack_from('<I',b,24)[0]==0x7180
count,off=struct.unpack_from('<II',b,36);rows=[]
for _ in range(count):
    kind,length=struct.unpack_from('<BH',b,off)
    assert length>=16 and off+length<=len(b)
    if kind==3:
        assert length>=60
        address,size,model=struct.unpack_from('<QQI',b,off+16)
        rows.append((address,size,model))
    off+=length
assert rows==[(0x15000000,0x100000,3),(0x05040000,0x10000,1)],rows

def function(text,name):
    # Locate signature directly to avoid unrelated static declarations.
    m=re.search(r'static [^;{}]*\b'+name+r'\([^;{}]*\)\s*\{',text)
    assert m,name
    start=m.start();end=m.end();level=1
    while level:
        level+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]

kernel_dir=a.kernel/'drivers/iommu/arm/arm-smmu'
header=(kernel_dir/'arm-smmu.h').read_text()
enums='\n'.join(re.search(r'enum '+name+r'\s*\{[^}]*\};',header).group() for name in ('arm_smmu_arch_version','arm_smmu_implementation'))
constants='\n'.join(l for l in (a.kernel/'include/acpi/actbl2.h').read_text().splitlines() if re.match(r'#define ACPI_IORT_SMMU_(V1 |V2 |CORELINK_|CAVIUM_)',l))
convert=function((kernel_dir/'arm-smmu.c').read_text(),'acpi_smmu_get_data')
select=function(a.candidate.read_text(),'sc7180_acpi_smmu_create')
source='''#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef uint32_t u32;
'''+enums+'\n'+constants+'''
struct arm_smmu_device { int version,model; uint64_t ioaddr; unsigned numpage; void *dev; };
struct qcom_smmu_match_data { int tag; };
static const struct qcom_smmu_match_data qcom_smmu_500_impl0_data={1};
static const struct qcom_smmu_match_data sc7180_acpi_adreno_data={2};
#define ERR_PTR(x) ((struct arm_smmu_device *)(intptr_t)(x))
#define dev_err_probe(dev,err,...) (err)
static struct arm_smmu_device *qcom_smmu_create(struct arm_smmu_device *s, const struct qcom_smmu_match_data *d) {
    (void)s; return (struct arm_smmu_device *)(intptr_t)d->tag;
}
'''+convert+'\n'+select+'''
int main(int argc,char **argv) {
    if(argc!=5) return 2;
    struct arm_smmu_device s={0};
    s.ioaddr=strtoull(argv[1],0,0); s.numpage=strtoul(argv[2],0,0);
    int ret=acpi_smmu_get_data(strtoul(argv[3],0,0),&s);
    if(!ret) ret=(intptr_t)sc7180_acpi_smmu_create(&s);
    printf("selection=%d\\n",ret);
    return ret!=atoi(argv[4]);
}
'''
report={'iort_sha256':hashlib.sha256(b).hexdigest(),'candidate_sha256':hashlib.sha256(a.candidate.read_bytes()).hexdigest(),'tests':[]}
with tempfile.TemporaryDirectory(prefix='tcl-select-') as tmp:
    d=Path(tmp);(d/'check.c').write_text(source)
    subprocess.run(['cc','-Wall','-Wextra','-Werror',str(d/'check.c'),'-o',str(d/'check')],check=True)
    cases=[('live-apps',*rows[0],1),('live-adreno',*rows[1],2),
           ('apps-with-v2-model',rows[0][0],rows[0][1],1,-19),
           ('adreno-with-500-model',rows[1][0],rows[1][1],3,-19),
           ('apps-wrong-span',rows[0][0],0x10000,3,-19),
           ('adreno-wrong-span',rows[1][0],0x100000,1,-19),
           ('unknown-address',0x12340000,0x100000,3,-19),
           ('wrong-version',rows[1][0],rows[1][1],0,-19)]
    for name,address,size,model,expected in cases:
        run=subprocess.run([str(d/'check'),hex(address),hex(size),str(model),str(expected)],capture_output=True,text=True,check=True)
        report['tests'].append({'name':name,'result':run.stdout.strip(),'expected':expected})
print(json.dumps(report,indent=2))
