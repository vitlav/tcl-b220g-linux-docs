#!/usr/bin/env python3
"""Check Linux IORT helpers against a captured table, without device access."""
import argparse,json,re,struct,subprocess,tempfile,hashlib
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--kernel',type=Path,required=True);p.add_argument('--iort',type=Path,required=True);a=p.parse_args()
s=(a.kernel/'drivers/acpi/arm64/iort.c').read_text();b=a.iort.read_bytes()
assert b[:4]==b'IORT' and sum(b)%256==0 and struct.unpack_from('<I',b,4)[0]==len(b)
def function(name):
 m=re.search(r'static [^;{}]*\b'+name+r'\([^;{}]*\)\s*\{',s);assert m
 end=m.end();n=1
 while n:n+=(s[end]=='{')-(s[end]=='}');end+=1
 return s[m.start():end]
source='''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
typedef uint32_t u32; typedef uint8_t u8;
#pragma pack(push,1)
struct acpi_iort_node { u8 type; uint16_t length; u8 revision; u32 identifier,mapping_count,mapping_offset; };
struct acpi_iort_id_mapping { u32 input_base,id_count,output_base,output_reference,flags; };
#pragma pack(pop)
#define ACPI_ADD_PTR(t,p,o) ((t *)((char *)(p)+(o)))
#define ACPI_IORT_ID_SINGLE_MAPPING 1
#define ACPI_IORT_NODE_NAMED_COMPONENT 1
#define ACPI_IORT_NODE_PCI_ROOT_COMPLEX 2
#define ACPI_IORT_NODE_SMMU_V3 4
#define ACPI_IORT_NODE_PMCG 5
#define pr_err(...) ((void)0)
#define pr_warn(...) ((void)0)
#define FW_BUG ""
static void *iort_table;
'''+function('iort_id_map')+'\n'+function('iort_node_get_id')+'''
int main(int argc,char **argv) {
 if(argc!=3)return 2;
 FILE *f=fopen(argv[1],"rb");if(!f)return 2;
 fseek(f,0,SEEK_END);long len=ftell(f);rewind(f);
 iort_table=malloc(len);if(!iort_table)return 2;
 if(fread(iort_table,1,len,f)!=(size_t)len)return 2;fclose(f);
 struct acpi_iort_node *n=ACPI_ADD_PTR(struct acpi_iort_node,iort_table,strtoul(argv[2],0,0));
 struct acpi_iort_id_mapping *m=ACPI_ADD_PTR(struct acpi_iort_id_mapping,n,n->mapping_offset);
 u32 id=0, explicit_id=0;
 struct acpi_iort_node *parent=iort_node_get_id(n,&id,0);
 int ret=iort_id_map(m,n->type,m->input_base,&explicit_id,false);
 printf("no_input_parent=%d explicit_ret=%d explicit_sid=%u\\n",parent!=NULL,ret,explicit_id);
 int failed=parent!=NULL || ret || explicit_id!=m->output_base; free(iort_table);return failed;
}
'''
count,off=struct.unpack_from('<II',b,36);rows=[]
with tempfile.TemporaryDirectory(prefix='tcl-iort-host-') as tmp:
 d=Path(tmp);(d/'main.c').write_text(source)
 subprocess.run(['cc','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-sign-compare',str(d/'main.c'),'-o',str(d/'check')],check=True)
 for _ in range(count):
  kind,length=struct.unpack_from('<BH',b,off);assert length>=16 and off+length<=len(b)
  if kind==1:
   name=b[off+29:off+length].split(b'\0')[0].decode();n,mo=struct.unpack_from('<II',b,off+8)
   assert n and mo+n*20<=length
   mappings=[struct.unpack_from('<IIIII',b,off+mo+i*20) for i in range(n)]
   assert all(m[4]==0 and m[3]+16<=len(b) for m in mappings)
   run=subprocess.run([str(d/'check'),str(a.iort),str(off)],capture_output=True,text=True,check=True)
   rows.append({'name':name,'mapping_count':n,'result_first_mapping':run.stdout.strip()})
  off+=length
print(json.dumps({'iort_sha256':hashlib.sha256(b).hexdigest(),'helpers_sha256':hashlib.sha256(s.encode()).hexdigest(),'nodes':rows},indent=2))
