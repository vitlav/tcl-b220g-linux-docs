import socket,json,shutil,hashlib,os,time
s=socket.socket(socket.AF_UNIX);s.settimeout(5);s.connect('/run/tcl-weston/mpv-sound-test.sock')
s.sendall((json.dumps({'command':['set_property','pause',True]})+'\n').encode());print(s.recv(4096).decode(),flush=True)
s.close()
src='/run/initramfs/usb/tcl-venus1/big_buck_bunny_1080p_h264.mov'
dst='/dev/shm/tcl-bbb.mov'
assert shutil.disk_usage('/dev/shm').free > os.path.getsize(src)+64*1024**2
start=time.monotonic();h=hashlib.sha256()
with open(src,'rb') as a,open(dst+'.part','wb') as b:
 while chunk:=a.read(4*1024**2):
  h.update(chunk);b.write(chunk)
with open(dst+'.part','rb') as f:actual=hashlib.file_digest(f,'sha256').hexdigest()
assert actual==h.hexdigest()
os.rename(dst+'.part',dst)
print('RAM_COPY_VERIFIED',os.path.getsize(dst),actual,'seconds',time.monotonic()-start,flush=True)
