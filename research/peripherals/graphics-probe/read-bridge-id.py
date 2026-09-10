#!/usr/bin/env python3
"""Bounded TCL bridge probe; default only reads the current bank selector."""
import ctypes as C
import json
import os
from pathlib import Path
import sys

class Msg(C.Structure):
    _fields_ = [('addr', C.c_uint16), ('flags', C.c_uint16),
                ('length', C.c_uint16), ('buf', C.POINTER(C.c_ubyte))]
class Transfer(C.Structure):
    _fields_ = [('msgs', C.POINTER(Msg)), ('nmsgs', C.c_uint32)]

if sys.argv[1:] not in ([], ['--select-id-bank']):
    raise SystemExit('Usage: read-bridge-id.py [--select-id-bank]')
if Path('/proc/sys/kernel/random/boot_id').read_text().strip() != '192b0902-e812-4c64-9c93-ef4b64ebb903':
    raise SystemExit('Unexpected boot; revalidate adapter before probing')
adapter = Path('/sys/bus/i2c/devices/i2c-10').resolve()
if '/ac0000.geniqup/a90000.i2c/i2c-10' not in str(adapter):
    raise SystemExit('Unexpected adapter')
if Path('/sys/bus/i2c/devices/10-0029/driver').exists():
    raise SystemExit('Kernel driver owns bridge')
libc = C.CDLL(None, use_errno=True)
libc.ioctl.argtypes = [C.c_int, C.c_ulong, C.c_void_p]
libc.ioctl.restype = C.c_int
fd = os.open('/dev/i2c-10', os.O_RDWR)
def transfer(parts):
    buffers = [(C.c_ubyte * len(data))(*data) for flags, data in parts]
    messages = (Msg * len(parts))(*[Msg(0x29, flags, len(buf), buf)
                    for (flags, data), buf in zip(parts, buffers)])
    request = Transfer(messages, len(parts))
    rc = libc.ioctl(fd, 0x0707, C.byref(request))  # I2C_RDWR
    if rc != len(parts):
        raise OSError(C.get_errno(), 'I2C transfer failed or incomplete')
    return list(buffers[-1])
def read(reg):
    return transfer([(0, [reg]), (1, [0])])[0]
def write(reg, value):
    transfer([(0, [reg, value])])
def emit(**data):
    print(json.dumps(data), flush=True)

try:
    bank = read(0xff)
    repeat = read(0xff)
    emit(bank_before=hex(bank), repeated=hex(repeat))
    if sys.argv[1:] == ['--select-id-bank']:
        # The actual device returns 0x12 from FF; bank readback is unverified.
        # Use the documented OEM selector, never restore an inferred value.
        write(0xff, 0x81)
        emit(bank_select_written='0x81', restoration='not attempted: original bank unknown')
        access = read(0x08)
        emit(access_81_08=hex(access))
        if access != 0x7f:
            raise RuntimeError('Access differs from OEM state; no access-control write')
        values = [read(reg) for reg in (0, 1, 2)]
        emit(id_registers_00_01_02=[hex(x) for x in values])

finally:
    os.close(fd)
