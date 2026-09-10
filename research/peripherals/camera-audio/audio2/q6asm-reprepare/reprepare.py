import ctypes as c
lib=c.CDLL('libasound.so.2')
P=c.c_void_p
lib.snd_pcm_open.argtypes=[c.POINTER(P),c.c_char_p,c.c_int,c.c_int]
lib.snd_pcm_set_params.argtypes=[P,c.c_int,c.c_int,c.c_uint,c.c_uint,c.c_int,c.c_uint]
for n in ['snd_pcm_prepare','snd_pcm_drop','snd_pcm_close']:
 getattr(lib,n).argtypes=[P]
lib.snd_pcm_writei.argtypes=[P,P,c.c_ulong]
lib.snd_pcm_writei.restype=c.c_long
lib.snd_strerror.argtypes=[c.c_int]
lib.snd_strerror.restype=c.c_char_p
pcm=P()
def check(tag,r):
 print(tag,r,flush=True)
 if r<0:raise RuntimeError(lib.snd_strerror(r).decode())
check('open',lib.snd_pcm_open(c.byref(pcm),b'hw:Test,0',0,0))
try:
 check('set_params',lib.snd_pcm_set_params(pcm,2,3,2,48000,0,100000))
 buf=c.create_string_buffer(4800*4)
 for cycle in range(3):
  for j in range(3):check(f'cycle {cycle} write {j}',lib.snd_pcm_writei(pcm,buf,4800))
  check(f'cycle {cycle} drop',lib.snd_pcm_drop(pcm))
  check(f'cycle {cycle} prepare',lib.snd_pcm_prepare(pcm))
 print('THREE_STOP_PREPARE_CYCLES_PASSED',flush=True)
finally:
 check('close',lib.snd_pcm_close(pcm))
