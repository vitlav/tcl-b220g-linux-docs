# Windows audio GPIO control: partial static reconstruction

2026-09-09. Offline analysis of preserved qcauddev7180.sys, no device writes.

Registry reader VA14004f858 allocates 40-byte records. Confirmed fields by string references and store destinations:

| Offset | Field | Evidence VA |
|---|---|---|
| +0x00 | GPIOUID | 14004f9f4..14004fa04 |
| +0x04 | ACPI resource INDEX | 14004fa58..14004fa60 |
| +0x08 | INITIALVALUE | 14004fab8..14004fac0 |
| +0x10 | I/O target handle, initially NULL | used14005d4c0, initialized14004fb20 |
| +0x18 | active/valid marker, initialized1 | 14004fb08 |
| +0x1c | reference counter, initialized0 | increment14005d780, decrement14005d898 |
| +0x20 | cached/override state, exact initialization unresolved | 14005f2cc |

AUDD global table pointer VA1400237b0 and countVA1400242a8; MBHC has separate table. Lookup14005d380 takes group0=AUDD/group1=MBHC and searches GPIOUID, not INDEX. Read helper14005d3f8 sends I/O control0x480000 with output1byte. Write helper14005d560 sends0x480004 with input1byte through140047260. GPIO control14005d5e8 selects UID and writes byte1 for requested state1, byte0 for state0, with reference counting for AUDD. Thus the observed path is ordinary GPIO I/O target requests; it does not itself imply a direct secure-monitor operation.

Value dispatch around14005f26c supports requested0/1, requested2 loads INITIALVALUE, and requested-1 toggles cached state (xor1). Therefore INITIALVALUE is a default to restore, NOT an active-low flag. This corrects a possible misleading inference from INF alone. Actual GPIOUID0 caller and reset timing remain unresolved. No conclusion that GPIO58 can safely be accessed under Linux. Reserved GPIO58..62 stay excluded.

Firmware PMIC resource binaries29a524/2e257c contain LPASS register labels but string search did not identify a named external-codec power client. This negative search does not prove missing supplies or firmware support. Do not infer voltages from Atoll rails.

Next static work: trace UID0 use through the codec OS callback table and reconstruct GPIO target open from ACPI resource INDEX. Hardware remains at working RX/LPI/SoundWire master with no slave responses. No sound test or bug publication.

## 2026-09-09 — OEM codec GPIO58 reset request sequence recovered

Static analysis of qcauddev7180.sys SHA256 `923326aa1538fc699e52a11d6c5f594ea086d1abc31b76740005626deff6fe3e`; evidence in `windows-gpio-flow/codec-reset-chain.asm`, `codec-reset-pe-evidence.txt`, `qcauddev-imports.txt`.

| Stage | Confirmed addresses / values |
|---|---|
| Callback registration | 14003624c installs 14003d590 into init structure; 140051e6c reads offset0x58; API command0x601 reaches140064f88 and stores pointer at140022148 |
| GPIO backend | 14006d990(selector,state), static table14001c2a0 stride12; selectors0..7 have backend5; produces callback outer opcode4 |
| Board mapping | 14003d800 dispatcher: selector7 -> AUDD group0 UID0; selector0 -> MBHC group1 UID0; selectors1..4 -> AUDD correspondingUID |
| Board resource | OEM INF AUDD UID0 INDEX0 INITIALVALUE0; DSDT AUDD first GpioIo is GIO0 pin0x3a=58 in both BSID branches |
| Reset request | 14006c768 calls setter(7,0); 14006c770 delay(5); 14006c77c setter(7,1); 14006c784 delay(2) |
| Timing units | delay14000b030 ->1400088a8 multiplies input by -10000 and calls IAT140018188, independently resolved through PE import table as KeDelayExecutionThread: requested relative intervals5ms and2ms |
| Caller | 14006c9dc calls reset routine14006c6b0 in initialization branch w20==1, after two successful140081b48 calls, before14006c4f0 calls with0and1 |
| Disable path | 14006be9c requests setter(7,0) |

This supersedes the previous unresolved UID0 caller/timing note. The code requests low -> wait5ms -> high -> wait2ms, strongly supporting active-low codec reset. These are software-requested intervals, not measured electrical timing. GPIO writes go through the previously reconstructed IOCTL0x480004 path with reference-count/override gating; static calls alone do not prove that every invocation produces both electrical edges. INITIALVALUE remains a default value, not a polarity flag.

Live read-only verification: same boot c781c4d4-cd36-4a07-b734-40b059ac3e52, uptime21min, taint4096, Weston active, SoundWire only sdw-master-2-0, no newly logged kernel Oops. Existing selected register snapshot still shows no slave response. No GPIO58..62 access, voltage change, bootfile change, reboot, PCM test, or Bugzilla publication performed.

Remaining blocker: establish safe Linux access/ownership and exact tile for GPIO58, plus board-specific codec supplies. A prior boot hang was isolated only to group58..62, not to pin58 individually. Windows dynamically probes GPIO tiles; Linux SC7180 chooses a static tile. Do not infer a safe register write or remove the whole reservation from this reset-sequence evidence. Also decode the two140081b48 prerequisite calls and clock-vote request preceding the pulse before designing a hardware test.
