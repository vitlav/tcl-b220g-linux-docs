#!/usr/bin/env python3
"""Compare explicit decoders; failures remain failures, never fallback."""
import json,subprocess,time,resource,hashlib
from pathlib import Path
boot=Path('/proc/sys/kernel/random/boot_id').read_text().strip()
out=Path('/var/log/tcl-venus1')/boot/'benchmark';out.mkdir(parents=True,exist_ok=True)
sample=Path('/var/tmp/tcl-venus1/test-1080p30.h264')
results=[]
for index,decoder in enumerate(['h264','h264_v4l2m2m','h264_v4l2m2m','h264']):
 cmd=['ffmpeg','-hide_banner','-nostdin','-benchmark','-threads','2','-c:v',decoder,'-i',str(sample),'-an','-f','null','-']
 before=resource.getrusage(resource.RUSAGE_CHILDREN);t=time.monotonic()
 with (out/f'{index}-{decoder}.log').open('w') as log:
  run=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=90)
 after=resource.getrusage(resource.RUSAGE_CHILDREN)
 results.append(dict(decoder=decoder,command=cmd,exit_code=run.returncode,elapsed=time.monotonic()-t,user=after.ru_utime-before.ru_utime,system=after.ru_stime-before.ru_stime))
 (out/'timings.json').write_text(json.dumps(results,indent=2)+'\n')
 if run.returncode:break
if len(results)==4 and all(r['exit_code']==0 for r in results):
 for decoder in ['h264','h264_v4l2m2m']:
  cmd=['ffmpeg','-hide_banner','-nostdin','-threads','2','-c:v',decoder,'-i',str(sample),'-an','-pix_fmt','yuv420p','-f','framemd5','-y',str(out/f'{decoder}.framemd5')]
  with (out/f'{decoder}-hash.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=90,check=True)
print(out)
