#!/usr/bin/env python3
"""Prepare an I2C-only candidate from the verified working USB DTB."""
from pathlib import Path
import difflib
import hashlib
import shutil
import subprocess

root = Path(__file__).resolve().parent
base = root.parent / 'cpufreq-probe/sc7180-tcl-cpufreq.dtb'
out = root / 'sc7180-tcl-i2c10.dtb'
expected = 'f88a6854d0e1085b32b5f92cbca01e391b61248d512bcf00003ee080a83f7713'
if hashlib.sha256(base.read_bytes()).hexdigest() != expected:
    raise SystemExit('Working base DTB changed; recheck against USB before rebuilding')
def flat(path):
    return subprocess.run(['dtc', '-I', 'dtb', '-O', 'dts', str(path)],
                          capture_output=True, text=True, check=True).stdout
before = flat(base)
shutil.copyfile(base, out)
parent = '/soc@0/geniqup@ac0000'
for node in (parent, parent + '/i2c@a90000'):
    subprocess.run(['fdtput', '-t', 's', str(out), node, 'status', 'okay'], check=True)
subprocess.run(['fdtput', '-t', 'i', str(out), parent + '/i2c@a90000',
                'clock-frequency', '100000'], check=True)
after = flat(out)
(root / 'i2c10-dtb.diff').write_text(''.join(difflib.unified_diff(
    before.splitlines(True), after.splitlines(True), fromfile='working-cpufreq', tofile='i2c10-candidate')))
(root / 'i2c10-SHA256SUMS').write_text(hashlib.sha256(out.read_bytes()).hexdigest() + '  ' + out.name + '\n')
