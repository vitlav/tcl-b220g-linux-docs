import os,mmap,time,random,statistics,json
path='/run/initramfs/usb/tcl-venus1/big_buck_bunny_1080p_h264.mov'
fd=os.open(path,os.O_RDONLY|os.O_DIRECT)
buf=mmap.mmap(-1,1024*1024)
try:
 start=time.monotonic();total=0
 for i in range(64):
  n=os.readv(fd,[buf]);assert n==len(buf);total+=n
 dt=time.monotonic()-start
 print(json.dumps({'test':'sequential O_DIRECT','bytes':total,'seconds':dt,'MB_s':total/dt/1e6}),flush=True)
 size=os.fstat(fd).st_size;block=128*1024;rng=random.Random(7180);lat=[]
 for i in range(32):
  os.lseek(fd,rng.randrange(size//block)*block,os.SEEK_SET)
  t=time.monotonic()
  with memoryview(buf)[:block] as part:n=os.readv(fd,[part])
  assert n==block;lat.append(time.monotonic()-t)
 print(json.dumps({'test':'32 random O_DIRECT 128KiB reads','MB_s':32*block/sum(lat)/1e6,'median_ms':statistics.median(lat)*1000,'max_ms':max(lat)*1000,'latencies_ms':[round(x*1000,2) for x in lat]}),flush=True)
finally:
 os.close(fd);buf.close()
