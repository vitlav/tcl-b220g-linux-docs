#!/usr/bin/env python3
"""Decode a single 32-byte TCL EC read, using the saved OEM DSDT fields."""
import json
import re
import sys
from pathlib import Path

text = Path(sys.argv[1]).read_text()
tokens = text.split()
if len(tokens) != 32 or any(not re.fullmatch(r'0x[0-9a-fA-F]{2}', t) for t in tokens):
    raise SystemExit('Expected exactly 32 hexadecimal bytes from i2ctransfer; retain errors separately.')
data = bytes(int(t, 16) for t in tokens)

def word(offset):
    return int.from_bytes(data[offset:offset + 2], 'little')

fields = {'ELID': data[1], 'ECWR': data[2], 'B1DC': word(6),
          'B1FV': word(8), 'B1FC': word(10), 'B1ST': data[14],
          'B1CR': word(15), 'B1RC': word(17), 'B1VT': word(19),
          'BPCN': data[21]}
result = {'raw_hex': data.hex(' '), 'fields': fields,
          'ac_online_by_dsdt': bool(fields['ECWR'] & 1),
          'battery_state_by_dsdt': fields['B1ST'] & 7,
          'present_rate_dsdt_formula': fields['B1CR'] * fields['B1FV'] // 10000,
          'note': 'Field mapping from DSDT; current scaling and BPCN meaning require hardware validation.'}
print(json.dumps(result, indent=2))
