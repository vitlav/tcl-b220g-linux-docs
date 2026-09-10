from pathlib import Path
import subprocess,tarfile,hashlib,json,sys
r=Path(sys.argv[1]);repo=Path(sys.argv[2])
refs=json.loads((repo/'artifacts/firmware-manifest.json').read_text())['files'];wanted={}
for x in refs:wanted.setdefault(x['sha256'],[]).append(x['name'])
packages=[];matches=[]
for kind in ['graphics','wireless','misc']:
 proc=subprocess.Popen(['dpkg-deb','--fsys-tarfile',str(r/(kind+'.deb'))],stdout=subprocess.PIPE);count=total=0
 with tarfile.open(fileobj=proc.stdout,mode='r|') as tar:
  for m in tar:
   if not m.isfile() or not m.name.startswith('./usr/lib/firmware/'):continue
   b=tar.extractfile(m).read()
   if m.name.endswith('.zst'):b=subprocess.check_output(['zstd','-q','-d','-c'],input=b)
   elif m.name.endswith('.xz'):b=subprocess.check_output(['xz','-d','-c'],input=b)
   h=hashlib.sha256(b).hexdigest();count+=1;total+=len(b)
   if h in wanted:matches.append({'package_group':kind,'path':m.name,'bytes':len(b),'sha256':h,'tcl_files':wanted[h]})
 assert proc.wait()==0
 packages.append({'group':kind,'regular_firmware_files_checked':count,'uncompressed_bytes_hashed':total});print(kind,count,total)
matched={x['sha256'] for x in matches};out={'scope':'All regular files under usr/lib/firmware in the three inspected Ubuntu Qualcomm packages; symlinks not independently hashed; containers not recursively unpacked','packages':packages,'matches':matches,'unmatched_tcl_files':[{'name':x['name'],'sha256':x['sha256']} for x in refs if x['sha256'] not in matched]}
(r/'all-payload-matches.json').write_text(json.dumps(out,indent=2)+'\n');print('Matches:',json.dumps(matches));print('Unmatched:',[x['name'] for x in out['unmatched_tcl_files']])
