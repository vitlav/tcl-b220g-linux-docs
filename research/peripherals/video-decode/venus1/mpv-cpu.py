import os,time,json,subprocess,pathlib
out=pathlib.Path('/var/tmp/tcl-venus1/mpv-cpu');out.mkdir(exist_ok=True)
hz=os.sysconf('SC_CLK_TCK');env=dict(os.environ,XDG_RUNTIME_DIR='/run/tcl-weston',WAYLAND_DISPLAY='wayland-tcl',SDL_VIDEODRIVER='wayland')
movie='/run/initramfs/usb/tcl-venus1/big_buck_bunny_1080p_h264.mov'
def ticks(pid):
 try:
  a=pathlib.Path(f'/proc/{pid}/stat').read_text().rsplit(')',1)[1].split();return int(a[11])+int(a[12])
 except FileNotFoundError:return 0
def snapshot(pid):
 ws=[int(p.name) for p in pathlib.Path('/proc').iterdir() if p.name.isdigit() and (p/'comm').exists() and (p/'comm').read_text().strip()=='weston']
 c=list(map(int,pathlib.Path('/proc/stat').read_text().splitlines()[0].split()[1:9]))
 return {'time':time.monotonic(),'player':ticks(pid),'weston':sum(ticks(p) for p in ws),'total':sum(c),'busy':sum(c)-c[3]-c[4], 'freq_khz':{p.parent.name:p.read_text().strip() for p in pathlib.Path('/sys/devices/system/cpu/cpufreq').glob('policy*/scaling_cur_freq')}}
results=[]
p=None
subprocess.run(['systemctl','stop','tcl-bbb-resume'],check=True)
try:
 for name,codec in [('hardware1','h264_v4l2m2m'),('software','h264'),('hardware2','h264_v4l2m2m')]:
  with (out/(name+'.log')).open('w') as log:
   p=subprocess.Popen(['mpv','--no-config','--vo=gpu-next','--gpu-api=opengl','--gpu-context=wayland','--fullscreen','--no-audio','--start=60','--hwdec='+('v4l2m2m-copy' if codec=='h264_v4l2m2m' else 'no'),'--hwdec-software-fallback=no','--msg-level=all=v','--term-status-msg=time=${time-pos} decoder=${video-codec} dropped=${decoder-frame-drop-count} vo-drop=${frame-drop-count}','--title=CPU-mpv-'+name,movie],env=env,stdout=log,stderr=log)
   time.sleep(5)
   if p.poll() is not None:raise RuntimeError(f'{name} exited {p.returncode}')
   a=snapshot(p.pid);samples=[a]
   for i in range(5):
    time.sleep(5)
    if p.poll() is not None:raise RuntimeError(f'{name} exited early')
    samples.append(snapshot(p.pid))
   b=samples[-1];dt=b['time']-a['time']
   r={'mode':name,'seconds':dt,'player_pct_one_core':100*(b['player']-a['player'])/hz/dt,'weston_pct_one_core':100*(b['weston']-a['weston'])/hz/dt,'system_busy_pct_all_cores':100*(b['busy']-a['busy'])/(b['total']-a['total']),'samples':samples}
   results.append(r);(out/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps({k:v for k,v in r.items() if k!='samples'}),flush=True)
   p.terminate();p.wait(timeout=10)
finally:
 if p is not None and p.poll() is None:
  p.terminate();p.wait(timeout=10)
 subprocess.run(['systemd-run','--unit=tcl-bbb-after-mpv','--setenv=XDG_RUNTIME_DIR=/run/tcl-weston','--setenv=WAYLAND_DISPLAY=wayland-tcl','--setenv=SDL_VIDEODRIVER=wayland','/usr/bin/ffplay','-hide_banner','-loglevel','info','-nostats','-fs','-an','-vcodec','h264_v4l2m2m','-i',movie],check=True)
