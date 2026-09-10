#!/usr/bin/env python3
"""Plot recorded thermal and GPU-frequency telemetry; no interpolation claims."""
import csv, json, os
from collections import defaultdict
from pathlib import Path
os.environ.setdefault('MPLCONFIGDIR', '/tmp/tcl-matplotlib')
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
p = Path(__file__).resolve().parent
rows = list(csv.DictReader((p / 'results/telemetry.csv').open()))
start = float(rows[0]['uptime'])
series = defaultdict(list)
for r in rows:
    series[r['sensor']].append(((float(r['uptime'])-start)/60, float(r['value'])))
fig, axes = plt.subplots(2, 1, figsize=(10, 6), sharex=True, layout='constrained')
for sensor in ('gpuss0-thermal', 'gpuss1-thermal', 'cpu0-thermal', 'cpu6-thermal'):
    values = series[sensor]
    axes[0].plot([x for x,y in values], [y/1000 for x,y in values], label=sensor)
axes[0].set_ylabel('Temperature (°C)')
axes[0].legend(ncol=2)
values = series['/sys/class/devfreq/5000000.gpu/cur_freq']
axes[1].step([x for x,y in values], [y/1e6 for x,y in values], where='mid')
axes[1].set_ylabel('GPU frequency (MHz)')
axes[1].set_xlabel('Minutes since first sample')
for ax in axes: ax.grid(alpha=.25)
fig.suptitle('TCL B220G • glmark2 fullscreen 1920×1080 • samples every 5 s')
fig.savefig(p/'thermal-frequency.png', dpi=160)
fig.savefig(p/'thermal-frequency.svg')
temps = {k: {'min_c': min(y for x,y in v)/1000, 'max_c': max(y for x,y in v)/1000} for k,v in series.items() if not k.startswith('/')}
report = {'temperatures': temps, 'gpu_frequency_mhz': sorted({y/1e6 for x,y in values}), 'sample_span_s': (float(rows[-1]['uptime'])-start), 'sampling_caveat': 'Peaks between 5-second samples may be missed.'}
(p/'telemetry-summary.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
