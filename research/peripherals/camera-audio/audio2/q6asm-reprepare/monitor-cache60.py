import socket,json,time,pathlib
s=socket.socket(socket.AF_UNIX);s.settimeout(5);s.connect('/run/tcl-weston/mpv-sound-test.sock');f=s.makefile('rb');seq=0
def get(p):
 global seq
 seq+=1;s.sendall((json.dumps({'command':['get_property',p],'request_id':seq})+'\n').encode())
 while True:
  r=json.loads(f.readline())
  if r.get('request_id')==seq:return r
for i in range(7):
 out={'sample':i,'time':time.time()}
 for p in ['path','time-pos','options/cache','options/demuxer-readahead-secs','demuxer-cache-state','paused-for-cache','audio-out-params','frame-drop-count']:
  out[p]=get(p)
 out['io_pressure']=pathlib.Path('/proc/pressure/io').read_text()
 out['usb_stat']=pathlib.Path('/sys/class/block/sdg/stat').read_text()
 print(json.dumps(out),flush=True)
 if i<6:time.sleep(5)
