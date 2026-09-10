#!/usr/bin/env python3
"""Extract the 54 XML fragments referenced by this exact TCL DisplayDxe."""
import hashlib
import json
from pathlib import Path
import struct
import sys

image = Path(sys.argv[1]).read_bytes()
if hashlib.sha256(image).hexdigest() != '76bfce3fe980a8c8130849628eee097b8c11af63073e98509652bfc1a5b492f0':
    raise SystemExit('Unexpected DisplayDxe image')
rows = []
for index in range(54):
    address = struct.unpack_from('<Q', image, 0x52d50 + index * 8)[0]
    rows.append({'index': index, 'rva': hex(address),
                 'text': image[address:image.index(b'\0', address)].decode('ascii')})
print(json.dumps(rows, indent=2))
