import socket,json,time,os
assert os.path.getsize('/dev/shm/tcl-bbb.mov')==725106140
s=socket.socket(socket.AF_UNIX);s.settimeout(10);s.connect('/run/tcl-weston/mpv-sound-test.sock');f=s.makefile('rb');seq=0
def call(cmd):
 global seq
 seq+=1;s.sendall((json.dumps({'command':cmd,'request_id':seq})+'\n').encode())
 while True:
  r=json.loads(f.readline())
  if r.get('request_id')==seq:
   print(json.dumps({'command':cmd,'response':r}),flush=True)
   if r.get('error')!='success':raise RuntimeError(r)
   return r.get('data')
pos=call(['get_property','time-pos'])
call(['set_property','volume',50])
call(['loadfile','/dev/shm/tcl-bbb.mov','replace',-1,{'start':str(pos)}])
time.sleep(2)
assert call(['get_property','path'])=='/dev/shm/tcl-bbb.mov'
call(['set_property','mute',False])
call(['set_property','pause',False])
time.sleep(2)
for prop in ['time-pos','audio-out-params','volume','mute','paused-for-cache']:
 call(['get_property',prop])
