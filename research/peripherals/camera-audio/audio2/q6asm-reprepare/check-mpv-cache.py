import socket,json
s=socket.socket(socket.AF_UNIX);s.settimeout(5);s.connect('/run/tcl-weston/mpv-sound-test.sock');f=s.makefile('rb')
for i,p in enumerate(['options/cache','options/demuxer-readahead-secs','options/demuxer-max-bytes','options/cache-secs','options/cache-pause-wait','demuxer-cache-state','cache-buffering-state','paused-for-cache','time-pos']):
 s.sendall((json.dumps({'command':['get_property',p],'request_id':i})+'\n').encode())
 while True:
  r=json.loads(f.readline())
  if r.get('request_id')==i:
   print(json.dumps({'property':p,'response':r}),flush=True);break
