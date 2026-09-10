

## 2026-09-09 codec-presence and Windows GPIO addressing

Clean boot c781c4d4-cd36-4a07-b734-40b059ac3e52 remains, taint4096, Weston active. Selected read-only SoundWire registers collected via module using pm_runtime_resume_and_get on bound controller; FIFO registers excluded. Result at uptime536.452168:

| Register | Value | Meaning |
|---|---|---|
| CFG 0x004 | 0x00000003 | controller enabled, pulse IRQ |
| COMP_STATUS 0x014 | 0x00000001 | frame generator enabled |
| IRQ_STATUS 0x200 | 0x00004000 | raw status; not decoded as a new failure |
| ENUM_CFG 0x500 | 0x00000001 | auto enumeration enabled |
| BUS_CTRL 0x1044 | 0 | snapshot |
| MCP_STATUS 0x104c | 0 | bank0 |
| SLAVE_STATUS 0x1090 | 0 | no attached slave status |
| ID1/ID2 slot1 0x538/0x53c | 0 / 0 | no enumerated identity |

Linux qcom_swrm_enumerate calls sdw_slave_add(bus,&id,NULL) for unlisted devices. Therefore missing codec DT alone does not explain absent slave sysfs entry: hardware slave-status is zero with active frame generator. This narrows next investigation to physical codec power/reset/link, but does not identify the faulty member.

Windows qcgpio.sys static analysis: functionVA140002430 probes three tile candidates at resourcebase+tileoffset+(pin<<12)+0x10, selects bit0 and caches tileoffset atVA1400092a0. TableVA140007158 has0x100000,0x500000,0x900000. With ACPI GIO0 base0x03400000 these yield0x03500000/0x03900000/0x03d00000, exactly Linux tile bases. GPIO routines e.g.VA1400024a8 directly read/write MMIO using this cache. No evidence of a simple GPIO58 renumbering or special secure path was found; this is not proof of its absence everywhere. GPIO58 is WEST in Linux and thus predicted control0x0353a000, NOT accessed. Reserved58..62 unchanged; prior boot hang was localized only to group, not exact pin. Codec reset role/polarity remains unproven. No reboot or GPIO writes this turn.

Evidence/source in codec-presence/. OEM power rail mapping remains unresolved; do not copy Atoll supply names/voltages to TCL. No sound and no new Bugzilla report.
