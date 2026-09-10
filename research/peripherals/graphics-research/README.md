# TCL display bridge: offline investigation, 2026-09-08

Initial sections describe offline analysis. Later live validation is documented below and in ../graphics-probe/README.md: a separate I2C DTB was booted and only bridge bank-selector writes were used for bounded reads. No display reset, timing, PLL or power programming has been performed.

## Evidence

OEM EDPBridge.sys has PDB path `D:\JLQCode\SC7180\EdpbridgeQ329_328_Emdoor\ARM64\Release\EDPBridge.pdb`. INF version16.25.12.295 (2022-11-07). This identifies the OEM project, not the bridge chip.

ACPI EDP1: I2C7-bit0x29 on IC11 at100kHz, GPIO resources23/20/51/26/13/21. GPIO roles remain unresolved.

Windows function0x140003c48 repeatedly calls0x140003988 with register/value arguments. Its initial sequence includes FF=81,08=7F,49=FF,FF=82,5A=0E. The public LT8911EXB driver starts with FF=81,49=FF,FF=82,5A=0E (without the extra08=7F write). This is concrete register-sequence evidence supporting LT8911EXB-family identification, beyond the matching I2C address. It is not yet a chip-ID measurement or proof of exact revision.

Reference source (downloaded using gh): https://github.com/aystshen/lontium_lt8911exb_driver/blob/master/lt8911exb/lt8911exb.c . Saved as reference-lt8911exb.c. Its identification routine selects bank81 with FF=81 and reads00/01/02. The source comment shows example chipid0xe0517. This procedure includes writes (and additional enable operations); do not treat it as a passive read-only probe.

prepare-hardware.asm captures Windows function0x1400025e0. At0x1400027bc–27c4 it invokes MmMapIoSpaceEx on the supplied memory resource; saves the pointer at context+0x90, then reads a16-bit value at the mapping start (0x140002840). Existing strings include `CmResourceTypeMemory RebootState %d`. This supports investigation of0x9ff90000 as a firmware state handoff region, but the exact semantics and producer are not established.

## Next analysis

1. Trace0x140003988/0x140003ae8 to verify register-write/read transport.
2. Extract bank-aware register sequences and compare with the reference.
3. Map resource ordinal to GPIO handles and trace writes during monitor power on/off.
4. Confirm lane configuration and timing fields; retain actual TCL EDID timing rather than reference-board defaults.
5. Only after the above prepare a separate boot test; no blind SN65DSI86 node or live reset.

Reference drivers are evidence, not a tested drop-in MSM DRM bridge. Kernel integration and exact-chip support remain unverified.

## GPIO mapping recovered from OEM code

PrepareHardware stores successive GPIO connection IDs at context+0x30+8*n (0x14000275c–277c). These map to ACPI resource order, as documented by Microsoft: https://learn.microsoft.com/en-us/windows-hardware/drivers/gpio/gpio-based-hardware-resources . This establishes the following mapping without inventing electrical signal names:

| Ordinal | Context offset | GPIO | Observed initialization behavior |
|---|---|---|---|
| 0 | 0x30 | 23 | write1 |
| 1 | 0x38 | 20 | write1 |
| 2 | 0x40 | 51 | write1 |
| 3 | 0x48 | 26 | write1, wait20ms, write0, wait150ms, write1 |
| 4 | 0x50 | 13 | write1 in loop; then write1/wait10ms/write0/wait30ms/write1/wait10ms |
| 5 | 0x58 | 21 | separate boolean control function0x140003080 |

Evidence: initialization0x140002d40 loops indices0..4. Special branch compares index3 at0x140002e20; delay constant0x249f0 at0x140002f50 is150000 microseconds passed to KeStallExecutionProcessor. Final local descriptor var_558h is64 bytes after var_598h, selecting index4. Function0x140003080 explicitly loads context+0x58/+0x5c, converts its second argument to(value>0), and writes one byte.

Transport0x140003618 uses IOCTL0x00480004 (GPIO write); input byte comes from x4. See gpio-transport.asm and Microsoft GPIO write semantics: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/gpio/ni-gpio-ioctl_gpio_write_pins . Function0x140002f58 writes zero to the first five resources (loop starts at5, advances IDs from context+0x30).

GPIO21 is driven low before display initialization and high in delayed worker0x140004fc0. This strongly suggests a display/backlight enable role, but do not yet assign a definitive backlight compatible or PWM behavior. GPIO13 has a reset-like pulse, GPIO26 a separate longer pulse; their exact physical functions and rail voltages are still unknown.

Register write wrapper0x140003988 truncates arguments w1/w2 to bytes, forms register/value, and calls transport0x1400032a0 at0x140003a68. Read wrapper0x140003ae8 calls0x140003428. Thus interpretation of the matched bank/register init sequence now has direct argument-level evidence.

## DSI/eDP lane and timing findings

- At0x140003eb4–3ed0 OEM selects bankD0 and writes register00=00. Reference lt8911exb.c:617 decodes this as4 MIPI input lanes. OEM also selects MIPI Rx with85:C0=01 at0x140003ea0.
- At0x140004858–4878 OEM selects bank85 and writes register1A from global0x14000705c. Reference lt8911exb_link_train uses this register for eDP lane_cnt.
- PrepareHardware0x140002cc0–2d04 compares timing structure field+0x10 against1080; sets global0x14000705c to2 when>=1080, else1. The field maps to vertical active in the timing write path; for the1920x1080 panel this predicts2 eDP lanes. This remains a code-derived configuration, not measurement of an active link.
- Timing is not entirely hardcoded: on first preparation, guarded by context+0xc8, OEM reads bridge registers0F/10 etc into a timing structure at0x140007040. Initial reads do not locally select a bank; subsequent reads explicitly select85 andD0. This creates a firmware-state/bank assumption to resolve before a cold-init Linux implementation.
-85:B0 (dithering-related reference field) is copied from timing structure+0x16, not a proven constant. Preserve uncertainty about color depth; do not infer all link parameters from EFI32bpp.
- The OEM writes differ materially from the reference (e.g.82:35=22 rather than42;82:3A/3B=77 rather than44;D0:0C=80 rather than40). The public driver is not a safe drop-in initialization recipe.

register-call-sites.tsv indexes189 write call sites with addresses, literal arguments or dynamic expressions. Generated by extract-register-calls.py. It is a static index, not control-flow simulation, a full bank-aware trace, or a script for writing hardware. Verify each relevant bank and branch in assembly.

## Timing structure and startup order

The structure at0x140007040 has16-bit fields:

| Offset | Meaning | Expected EDID value | D0 source register(s) |
|---|---|---|---|
|00|H front porch|58|19–1A|
|02|H sync|42|16|
|04|H back porch|60|derived|
|06|H active|1920|13–14|
|08|H total|2080|11–12|
|0A|V front porch|3|17–18|
|0C|V sync|5|15|
|0E|V back porch|54|derived|
|10|V active|1080|0F–10|
|12|V total|1142|0D–0E|

Verified by pairing the read path0x140002afc–2c68 with the write path0x140004088–4174 and the reference register meanings. Expected values are derived from saved EDID, NOT measured bridge values. Machine-readable expected-timing.json records this distinction. H/V back porches are totals minus active, front porch and sync. At+14 OEM derives a clock-like value from Htotal*Vtotal*60 using constant division; the multiplier60 is explicit. Do not assume this formula supports arbitrary refresh rates.

Correction to the earlier brief summary: after reading85:B0 into+16, OEM selects bankA8 (NOT D0) at0x140002c7c–2c90 and reads17/18 into+18/+1A. Their meaning remains unresolved. The initial timing reads are consistent with D0, but local bank selection is absent in that path, so the inherited bank still requires verification.

Worker0x140004ee0: obtain lock; GPIO21=0 via0x140003080; GPIO initialization0x140002d40; bridge initialization0x1400044a8 (calls0x140003c48 at0x1400046d8); create worker0x140004fc0; release lock. Delayed worker waits context+2C microseconds, then GPIO21=1. Default context+2C=120000 at0x140003260–3268 (other writes to this field still need auditing before treating delay as universal).

Unresolved before a safe display boot test: chip/revision identity, mapping GPIO rails to physical functions/voltages, inherited bank and unknown A8:17/18 fields, DSI host mode/clock/polarity, and DRM bridge integration. A list of register writes alone does not supply these requirements.

## Current XBL copied from the actual machine

Read /dev/sdb1 (XBL,3670016bytes) and /dev/sde7 (PLAT,2097152bytes) while both parent UFS devices remain read-only. Local copies /tmp/tcl-graphics-private/{xbl.bin,plat.bin}, parent mode700. SHA256 verified against independently read source devices:

- XBL:82dd0864eeb1e8c1289abeed38ae540323e61dd95f0e2ff2ab38a4f33a32136f
- PLAT:50cb58e67533228903cea9b3e384e91ac82fa32fa5359672f539c8c2f707336d

XBL itself contains explicit LT8911EXB strings at file offsets:

| Offset | String |
|---|---|
|2e735d|Reset_LT8911|
|2e7480|EDPBridgeDriver_EDIDRead_LT8911|
|2e75ab|EDPBridgeDriver_SetMode_LT8911|
|2e7d18|LT8911EXB edp bridge chip id %x %x %x|
|2e7dfc|LT8911 tx pll locked|

This establishes presence of LT8911EXB-specific implementation in the machine's installed XBL slot. Together with OEM register patterns and ACPI0x29 it is strong evidence for the bridge family. A runtime chip-ID value still has not been read. Boot-slot selection is not yet independently established; backup XBL should be compared if needed.

The older2021 update capsule contains generic SN65DSI86 panel templates and must not be treated as a dump of the current firmware. The current XBL also retains generic panel templates, so mere presence of a PanelName string does not prove selection. The relevant next step is extracting the DisplayDxe code and tracing its LT8911-specific functions and board-selection path. 7z does not recognize raw XBL as an archive; no executable from the firmware was run.

Memory handoff clarification: OEM0x140001ed0–1edc compares the first16-bit mapped value against0xA0 and clears it to0 on match, adjacent to RebootState logging. This confirms state-marker use of the mapped region; it is not a basis for treating the region as bridge MMIO.

## DisplayDxe extracted from current XBL

XBL LZMA stream at0xC4438 expands to8859528bytes. PE at decompressed offset0x3FAE38 is DisplayDxe (PDB QcomPkg/Drivers/DisplayDxe); extracted507904bytes, SHA25676bfce3fe980a8c8130849628eee097b8c11af63073e98509652bfc1a5b492f0. Reproducible extraction: extract-display-module.py verifies the exact input hash and output hash. Firmware binaries remain private /tmp/tcl-graphics-private. Analysis addresses below use PE image base0.

- Reset_LT8911 string0x46BDD references function0x2E1A4. It configures GPIO encoded0x200FC0D0 and logs GPIO13 on errors. Output high,10ms delay, low,100ms delay, high,100ms delay. Delay wrapper0x95BC dispatches through Boot Services+0xF8 (Stall). This differs from Windows GPIO13 high10ms/low30ms/high10ms. Do not blindly combine the two sequences.
- The generic error text says BL_EN_MSM even in Reset_LT8911. Treat this label as potentially copied debug text; the named reset routine and GPIO13 control are stronger evidence than the generic label, but the physical schematic remains unavailable.
- Chip identification function0x2E640: writeFF=81,08=7F, read00,01,02 through0x2F8B4; log bytes in that order at0x2E698. Actual device ID still unknown. This includes writes and is not a passive read-only probe.
- Bridge write helper0x2F81C takes register in w0, value in w1, unlike Windows wrapper's w1/w2. Do not reuse the Windows extraction convention for UEFI.
- eDP video configuration0x2E6A8 selectsA8 and sets2D=88, then emits timing from structure0x5225A. Further tracing of its population/EDID path remains necessary.

uefi-reset-and-id.asm retains the disassembly evidence. No firmware code executed and no reset/chip-ID writes were sent to TCL.

## EDID-to-timing path and confirmed firmware handoff

UEFI0x2E2F0 initializes I2C then invokes chip-ID, initial bridge setup and EDID acquisition. The helper0x2F36C sums128bytes and returns true when the low8bits of the sum are zero. Reset/retry occurs when validation fails; the eventual failure path logs EDIDValidateCheckSum and calls firmware shutdown/reset service. A zero/invalid-looking header can select a built-in temp_edid (branch0x2E408–2E430), so successful checksum alone does not prove actual panel data.

0x2D4AC parses the first18-byte detailed timing descriptor from EDID+0x36 (guarded by panel-config+0x9C). Its caller0x2E434–2E548 fills structure0x5225A from parsed panel fields. This confirms an EDID-derived path rather than unconditional use of the generic SN65DSI86 timing template. Exact branch selection on the currently booted machine is not yet traced.

0x2FA58 consumes EDID and the timing structure and formats XML-like fields (e.g. HorizontalActive) into a buffer. It first writes16-bit0xA0 to physical0x9FF90000 at0x2FA70–2FA80. Windows0x140001ED0–1EDC reads the same marker and clears it. Thus the producer and consumer of this state handoff are both identified in code. The generated XML destination is obtained through helper0x2F910 from a firmware memory-region protocol; do not guess its address from the0x9FF90000 marker location.

Evidence: uefi-edid-path.asm and uefi-state-marker.asm. No reads or writes to physical memory have been performed on the running TCL for this analysis.

## Embedded fallback EDID decoded (2026-09-08)

The actual constant is at DisplayDxe RVA0x47828, 128bytes, checksum0, SHA256ab685660002cc0d9f104499b0c2be7730236061d92430c1b66cf15cf829c1a04. Its first DTD is **1366x768**, pixel clock76.30MHz, H front/sync/back48/32/146, V3/5/22, refresh60.059Hz. See uefi-fallback-timing.json. This differs from the saved physical-panel EDID1920x1080 at142.52MHz.

0x2E3F4–2E404 copies the constant into stack+0x98 (128bytes, helper0x10E38). If live buffer byte1 is zero, 0x2E420–2E430 copies stack+0x98 into buffer0x79FA9, using length256. Copy direction verified through0x93EC→0xD864→0xDC4C (x0 destination, x1 source). Only128bytes of fallback are explicitly initialized here; do not treat the following128bytes as a validated EDID extension. Address0x79FA9 itself is a mutable readout buffer, not the embedded constant.

Thus a valid checksum and a generic firmware panel template alone cannot establish native panel timing. The fallback timing is not a candidate replacement for our measured panel EDID. No display hardware changes performed.

## DSI parameters linked to the LT8911-generated XML

At0x302D0–30304, function0x2FA58 concatenates54 strings from pointer table0x52D50. Table element39 points at0x478D6, the DSI Interface fragment. This establishes a code reference in the LT8911 EDID-to-XML path; it is stronger than finding an unrelated firmware string. It does not establish which runtime path this boot executed, nor prove all XML options are acted on by hardware.

| XML field | Value |
|---|---|
| DSIChannelId / DSIVirtualId | 1 / 0 |
| DSIColorFormat | 36 |
| DSITrafficMode | 1 |
| DSILanes | 4 |
| DSILP11AtInit | True |
| DSILowPowerModeInBLLPEOF / BLLP | True / True |
| DSIRefreshRate | 0x3C0000 |
| DSIControllerMapping | 00 |

Related Qualcomm source maps XML DSIColorFormat and DSITrafficMode directly to configuration enums: [MDPEDID.c](https://github.com/Rivko/android-firmware-qti-sdm670/blob/20bb8ae36c93fc16bbadda0e0a83f930c0c8a271/boot_images/QcomPkg/Library/MDPLib/MDPEDID.c). [HALdsi.h](https://github.com/Rivko/android-firmware-qti-sdm670/blob/20bb8ae36c93fc16bbadda0e0a83f930c0c8a271/boot_images/QcomPkg/Library/HALDSILib/HALdsi.h) enumerates36 as DSI_COLOR_RGB_888_24BPP and1 as DSI_Video_TrafficMode_NonBurst_VSEvent. These are inferred interpretations using a related SDM670 source tree, not exact matching source for the TCL binary. Do not copy Qualcomm enum numbers as Linux flags. DSIChannelId1 is also not sufficient to label the Linux controller dsi1; controller mapping is00 and needs separate interpretation.

The fragment also contains PMIC backlight configuration; this is not evidence that our panel uses that route instead of the GPIO21 sequence already found. Physical backlight wiring remains unresolved.

Evidence saved as uefi-panel-xml-fragments.json; extract-panel-fragments.py reproduces it with an exact-image SHA256 guard. It represents static templates before runtime EDID field substitutions. No hardware reconfiguration performed.

## Live timing validation (2026-09-08)

The earlier expected values are now confirmed by actual I2C reads on this TCL: D0 timing and A8 eDP MSA configuration both match all10 EDID-derived timing fields (1920x1080, totals2080x1142, H58/42/60, V3/5/54). A8:2D=88. See ../graphics-probe/{bridge-timing.jsonl,timing-comparison.json,decode-timing.py}. This validates register configuration, not measured pixel/link clock. The saved EDID clock142.52MHz has not been independently measured. No timing/reset/PLL registers were written; only bank selectorFF changed, ending at81 with ID17 05 E0 rechecked.


## OEM final TX PLL value confirmed by live read

Live87:19=33 differs from the reference initialization31, but matches this DisplayDxe:0x2EC50 writes87:19=31 and0x2ED48 writes87:19=33 after additional setup. The final OEM step must not be lost by adopting the reference sequence verbatim. Evidence uefi-txpll-final-config.asm; w23=19 from0x2EBF4, w24=87 from0x2EC38.

Other live fields: D0:00=00 (4DSI lanes),85:1A=02 (2eDP lanes),A8:17/18=10/20 (8bit/color),85:B0=00 (dither disabled),A8:27=10 (MIPI input),87:37=03 (TX PLL lock bit set). Exact line rate remains unconfirmed; do not infer measured2.7Gbps solely from reference comments. Full raw data and interpretation in ../graphics-probe/bridge-link.jsonl and README.md.

## Training result resolves link-rate uncertainty

UEFI0x2F590–2F5EC checks AC:82 bit5 and low5bits==1E for successful training, then labels AC:83/84 as panel link rate/count. Actual reads AC:82=3E,83=0A,84=82. Interpreting those OEM-labelled values with standard DP encoding from Linux v6.18 drm_dp.h gives2.7Gbps/lane,2lanes,enhanced framing. This is bridge-reported training configuration/status, not direct DPCD access or a physical frequency measurement. It resolves the earlier uncertainty based solely on PLL87:19.

UEFI output profile tables0x5227E and0x52284 indexed by0x7A0C1 feed82:22/23 and26/27; next bytes24/28=80,25/29=00. Live82:22–29=[82,00,80,00,82,00,80,00] matches table index0. Electrical swing/pre-emphasis labels remain unverified. See uefi-training-status.asm and ../graphics-probe/README.md.
