#!/usr/bin/env python3
"""Show a reversible 30-second framebuffer test on the active text VT."""
import array
import fcntl
import mmap
import os
from pathlib import Path
import signal
import struct
import time

if Path('/proc/fb').read_text().strip() != '0 EFI VGA':
    raise SystemExit('Unexpected framebuffer')
vt = Path('/sys/class/tty/tty0/active').read_text().strip()
if not vt.startswith('tty') or not vt[3:].isdigit():
    raise SystemExit('Unexpected VT')
fb = os.open('/dev/fb0', os.O_RDWR)
tty = os.open('/dev/' + vt, os.O_RDWR)
v = bytearray(160)
fcntl.ioctl(fb, 0x4600, v, True)  # FBIOGET_VSCREENINFO
fields = struct.unpack('<40I', v)
x, y, xv, yv, xo, yo, bpp = fields[:7]
if (x, y, xv, yv, xo, yo, bpp) != (1920,1080,1920,1080,0,0,32):
    raise SystemExit('Unexpected framebuffer geometry')
if fields[8:10] != (16,8) or fields[11:13] != (8,8) or fields[14:16] != (0,8):
    raise SystemExit('Unexpected color format')
stride = int(Path('/sys/class/graphics/fb0/stride').read_text())
if stride != x * 4:
    raise SystemExit('Unexpected stride')
mode = array.array('i', [0])
fcntl.ioctl(tty, 0x4B3B, mode, True)  # KDGETMODE
if mode[0] != 0:
    raise SystemExit('VT is already in graphics mode')
colors = [(255,255,255),(255,255,0),(0,255,255),(0,255,0),(255,0,255),(255,0,0),(0,0,255),(32,32,32)]
row = b''.join(bytes((b,g,r,255)) * (x//8) for r,g,b in colors)
canvas = bytearray(row*y)
white = bytes((255,255,255,255))
for yy in range(y):
    for xx in range(x):
        if xx < 6 or xx >= x-6 or yy < 6 or yy >= y-6 or (yy > 170 and (xx % 120 == 0 or yy % 120 == 0)):
            pos=yy*stride+xx*4
            canvas[pos:pos+4]=white
for yy in range(25,140):
    canvas[yy*stride+25*4:yy*stride+(x-25)*4]=bytes((0,0,0,255))*(x-50)
font={'T':['11111','00100','00100','00100','00100','00100','00100'],
'C':['01111','10000','10000','10000','10000','10000','01111'],
'L':['10000','10000','10000','10000','10000','10000','11111'],
'I':['11111','00100','00100','00100','00100','00100','11111'],
'N':['10001','11001','11001','10101','10011','10011','10001'],
'U':['10001','10001','10001','10001','10001','10001','01110'],
'X':['10001','10001','01010','00100','01010','10001','10001'],
'E':['11111','10000','10000','11110','10000','10000','11111'],
'F':['11111','10000','10000','11110','10000','10000','10000'],
'S':['01111','10000','10000','01110','00001','00001','11110']}
text='TCL LINUX EFI TEST'
scale=10
start=(x-len(text)*6*scale)//2
for i,ch in enumerate(text):
    for gy,bits in enumerate(font.get(ch,[])):
        for gx,bit in enumerate(bits):
            if bit=='1':
                for dy in range(scale):
                    pos=(45+gy*scale+dy)*stride+(start+i*6*scale+gx*scale)*4
                    canvas[pos:pos+4*scale]=white*scale
screen=mmap.mmap(fb,stride*y,flags=mmap.MAP_SHARED,prot=mmap.PROT_READ|mmap.PROT_WRITE)
saved=screen[:]
def interrupted(signum, frame):
    raise RuntimeError('Interrupted; restoring VT')
for sig in (signal.SIGTERM,signal.SIGINT,signal.SIGHUP,signal.SIGALRM):
    signal.signal(sig,interrupted)
signal.alarm(45)
try:
    fcntl.ioctl(tty,0x4B3A,1)  # KDSETMODE KD_GRAPHICS
    screen[:]=canvas
    print('DISPLAYING TCL LINUX EFI TEST for 30 seconds',flush=True)
    time.sleep(30)
finally:
    screen[:]=saved
    fcntl.ioctl(tty,0x4B3A,mode[0])
    signal.alarm(0)
    screen.close()
    os.close(tty)
    os.close(fb)
    print('RESTORED framebuffer and text VT',flush=True)
