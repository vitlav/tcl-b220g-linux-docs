#!/usr/bin/env python3
"""Extract PEP string-pointer records; compare candidate MMIO with Linux GCC."""
import json
import re
import struct
from pathlib import Path
root = Path(__file__).resolve().parent
binary = next((root / 'windows-drivers/OEM').glob('qcpep*/qcpep7180.sys'))
b = binary.read_bytes()
pe = struct.unpack_from('<I', b, 0x3c)[0]
count = struct.unpack_from('<H', b, pe + 6)[0]
opt = pe + 24
optsize = struct.unpack_from('<H', b, pe + 20)[0]
base = struct.unpack_from('<Q', b, opt + 24)[0]
sections = []
for i in range(count):
    off = opt + optsize + i * 40
    _, va, size, raw = struct.unpack_from('<IIII', b, off + 8)
    sections.append((va, size, raw))
def address(off):
    for va, size, raw in sections:
        if raw <= off < raw + size:
            return base + va + off - raw
    raise ValueError(off)
source = (root / 'gcc-sc7180-research.c').read_text()
names = ['gcc_aggre_ufs_phy_axi_clk','gcc_ufs_phy_ahb_clk','gcc_ufs_phy_axi_clk','gcc_ufs_phy_ice_core_clk','gcc_ufs_phy_phy_aux_clk','gcc_ufs_phy_rx_symbol_0_clk','gcc_ufs_phy_tx_symbol_0_clk','gcc_ufs_phy_unipro_core_clk','gcc_ufs_mem_clkref_en','gcc_ufs_phy_gdsc']
rows = []
for name in names:
    linux_name = {'gcc_ufs_mem_clkref_en':'gcc_ufs_mem_clkref_clk','gcc_ufs_phy_gdsc':'ufs_phy_gdsc'}.get(name,name)
    match = re.search(r'static struct \w+ '+linux_name+r' = \{(.*?)\n\};', source, re.S)
    reg = re.search(r'\.(?:halt_reg|gdscr) = (0x[0-9a-f]+)',match[1])
    expected = 0x100000 + int(reg[1],16)
    off = b.index(name.encode()+b'\0')
    pointer = struct.pack('<Q', address(off))
    hits = [m.start() for m in re.finditer(re.escape(pointer), b)]
    refs=[]
    for h in hits:
        words=struct.unpack_from('<8Q',b,h)
        refs.append({'file_offset':hex(h),'virtual_address':hex(address(h)),'qwords':[hex(x) for x in words],'expected_mmio_present':expected in words})
    rows.append({'name':name,'linux_name':linux_name,'linux_register_offset':reg[1],'expected_mmio':hex(expected),'string_va':hex(address(off)),'records':refs})
(root/'pep-ufs-clock-records.json').write_text(json.dumps(rows,indent=2)+'\n')
md='# Windows PEP / Linux SC7180 UFS register comparison\n\nPE string-pointer records contain the listed MMIO constants. Structure fields and runtime usage are not fully decoded. Equality of constants does not prove identical sequencing. Linux GCC base: 0x100000.\n\n|PEP name|Linux name|MMIO|Record VA|Match in first 8 qwords|\n|---|---|---|---|---|\n'
for row in rows:
    for ref in row['records']:
        md+=f"|{row['name']}|{row['linux_name']}|{row['expected_mmio']}|{ref['virtual_address']}|{ref['expected_mmio_present']}|\n"
(root/'pep-ufs-clock-records.md').write_text(md)
print(md)
