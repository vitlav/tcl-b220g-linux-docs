import socket,json,time
s=socket.socket(socket.AF_UNIX);s.settimeout(5)
s.connect('/run/tcl-weston/mpv-sound-test.sock')
f=s.makefile('rb');seq=0
def call(cmd):
 global seq
 seq+=1;s.sendall((json.dumps({'command':cmd,'request_id':seq})+'\n').encode())
 while True:
  r=json.loads(f.readline())
  if r.get('request_id')==seq:
   print(json.dumps({'command':cmd,'response':r}),flush=True)
   return r
call(['get_property','time-pos'])
call(['seek',-5,'relative+exact'])
time.sleep(3)
for prop in ['time-pos','aid','audio-out-params','volume','pause']:
 call(['get_property',prop])
