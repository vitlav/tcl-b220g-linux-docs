#!/usr/bin/env python3
"""Compare live bridge timing configuration with saved panel EDID expectations."""
import json
from pathlib import Path
root = Path(__file__).resolve().parent
rows = [json.loads(line) for line in (root/'bridge-timing.jsonl').read_text().splitlines()]
banks = {r['bank']: r['registers'] for r in rows if 'bank' in r}
def pair(b, hi):
    return banks[b][f'{hi:02x}'] * 256 + banks[b][f'{hi+1:02x}']
d0 = dict(htotal=pair('d0',0x11), hactive=pair('d0',0x13), hsync_len=banks['d0']['16'],
          hfront_porch=pair('d0',0x19), vtotal=pair('d0',0x0d), vactive=pair('d0',0x0f),
          vsync_len=banks['d0']['15'], vfront_porch=pair('d0',0x17))
for axis in ('h','v'):
    d0[axis+'back_porch'] = d0[axis+'total'] - d0[axis+'active'] - d0[axis+'sync_len'] - d0[axis+'front_porch']
a8 = dict(htotal=pair('a8',0x05), hactive=pair('a8',0x0b), hsync_len=pair('a8',0x09),
          vtotal=pair('a8',0x0d), vactive=pair('a8',0x15), vsync_len=banks['a8']['14'])
for axis, reg in [('h',0x07), ('v',0x11)]:
    start=pair('a8',reg)
    a8[axis+'back_porch']=start-a8[axis+'sync_len']
    a8[axis+'front_porch']=a8[axis+'total']-a8[axis+'active']-start
expected=json.loads((root.parent/'graphics-research/expected-timing.json').read_text())
comparison=[dict(field=f['name'], edid=f['expected_from_edid'], d0=d0[f['name']], a8=a8[f['name']]) for f in expected['fields']]
report=dict(provenance='Live configuration register reads, not measured signal frequency',
            comparison=comparison, all_match=all(r['edid']==r['d0']==r['a8'] for r in comparison),
            a8_2d=banks['a8']['2d'], final_id=rows[-1]['id_check'])
(root/'timing-comparison.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
