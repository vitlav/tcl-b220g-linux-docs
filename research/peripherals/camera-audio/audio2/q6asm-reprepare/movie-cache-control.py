import os,mmap,ctypes as c,json,time
path='/run/initramfs/usb/tcl-venus1/big_buck_bunny_1080p_h264.mov'
fd=os.open(path,os.O_RDONLY);size=os.fstat(fd).st_size
assert size==725106140
lib=c.CDLL(None,use_errno=True);lib.mincore.argtypes=[c.c_void_p,c.c_size_t,c.POINTER(c.c_ubyte)]
def resident():
 m=mmap.mmap(fd,size,flags=mmap.MAP_PRIVATE,prot=mmap.PROT_READ|mmap.PROT_WRITE)
 v=(c.c_ubyte*((size+mmap.PAGESIZE-1)//mmap.PAGESIZE))()
 addr=c.addressof(c.c_char.from_buffer(m))
 r=lib.mincore(addr,size,v)
 if r:raise OSError(c.get_errno(),os.strerror(c.get_errno()))
 n=sum(bool(x&1) for x in v);m.close()
 return {'resident_pages':n,'total_pages':len(v),'percent':100*n/len(v)}
try:
 print(json.dumps({'time':time.time(),'before':resident()}),flush=True)
 os.posix_fadvise(fd,0,0,os.POSIX_FADV_DONTNEED)
 print(json.dumps({'time':time.time(),'after':resident()}),flush=True)
finally:os.close(fd)
