# SoundWire: resume traces and wider prior-art search

Date: 2026-09-09. No reboot or sound validation. No Bugzilla publication.

## Measurements

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52, kernel 6.18.34-tcl-audio2. Holding VA, RX and SoundWire runtime power on does not restore the frame generator. RX output clock is enabled at 9.6 MHz, parent 19.2 MHz. Hardware version register is 0x01050001, confirming v1.5.1. Selected status remains COMP_STATUS=8, slaves=0. This rules out merely reading an autosuspended controller, not every clock or reset dependency.

Function and register kprobes show CLK_START=2 on resume. In two consecutive cycles, frame-generator wait returned 1 then 0. Both preceding/succeeding clock-stop calls returned -61 (-ENODATA), about 117 ms after broadcast 0x02ff0044; runtime suspend returned 0. Trace does not prove that missing acknowledgement causes the failed resume. IRQ clear 0x80004000 occurs in both cycles; bit31 remains undecoded. Do not infer delayed special-command completion from this trace.

All temporary probes/instances removed and power/control restored to auto by scripts. Weston active, taint4096 unchanged at last check. No codec appeared and no audible sound.

Candidate patch switches an empty bus with -ENODATA to the existing reset/re-enumeration resume path. Compiled in an isolated directory; NOT loaded or validated, original kernel source and boot files unchanged. This is a hypothesis, not a fix. The patch comment describing a causal link is stronger than the evidence: only correlation currently established.

Correction to earlier unload caution: drivers/base/dd.c device_unbind_cleanup() calls pm_runtime_reinit(). Missing explicit runtime-PM cleanup in qcom remove() alone therefore does not establish that normal replacement is impossible. Clock balancing and recovery still need review before replacement.

## Wider search and applicability

| Source | Finding | Applicability |
|---|---|---|
| [Kernel SoundWire error handling](https://cdn.kernel.org/doc/html/latest/driver-api/soundwire/error_handling.html) | No response can mean nonexistent slave or electrical issue; recovery may reset and enumerate again. | -ENODATA alone is not proof of a controller bug. |
| [Qualcomm RFC, 2026-06-23](https://lists.openwall.net/linux-kernel/2026/06/23/434) | qcom resume ignores enumeration completion timeout in full-reset path; proposal returns an error and unwinds clock/IRQ state. | Same driver, different wait from our frame-generator failure. RFC status, acceptance not verified; not a demonstrated TCL fix. |
| [AMD maintainer reply, 2026-06-23](https://lists.openwall.net/linux-kernel/2026/06/23/1480) | Returning success after resume timeout was deliberate to avoid failing PM; bus recovery suggested. | Different hardware, but shows why simply returning a PM error may make recovery worse. |
| [Qualcomm downstream SM6150 driver](https://github.com/LineageOS/android_kernel_xiaomi_sm6150/blob/lineage-22.2/techpack/audio/soc/swr-mstr-ctrl.c) | Distinct clock-stop mode0 and reset/master-init resume sequences. | Relevant sequence comparison, not interchangeable board configuration. Source obtained using gh and saved beside this report. |
| [SOF issue 4619](https://github.com/thesofproject/linux/issues/4619) | Search found MTL clock-stop prep/deprep failure report. | Title/search lead only; resolution not inspected, different controller. |
| [SOF issue 3101](https://github.com/thesofproject/linux/issues/3101) | Search found TGL controller reset report. | Title/search lead only, not evidence of a TCL solution. |

GitHub searched through gh: `soundwire "clock stop"`, `soundwire "link failed to connect"`, `SC7180 soundwire`, `TCL B220G audio`. Last three returned no issues in these searches; this does not establish that no relevant report exists. Earlier research also covered OEM Windows ACPI/drivers and Qualcomm PMIC firmware. No exact TCL B220G audio fix found so far.

Further lead: https://lkml.iu.edu/hypermail/linux/kernel/2102.3/03033.html (original Qualcomm clock-stop discussion, 2021). Search excerpt found; full page fetch failed, do not treat detailed claims as verified.

Next: compare safe reset recovery with downstream, validate candidate only after normal module replacement and fallback review; then return to codec power/reset enumeration. GPIO58..62 remain untouched.

## 2026-09-09 — runtime candidate tested; frame generation recovered, codec absent

Same boot c781c4d4-cd36-4a07-b734-40b059ac3e52. Normal module replacement, no reboot or bootfile changes. Original soundwire_qcom restored after both tests; temporary supplies unloaded and power controls returned to auto. Weston active, taint4096 unchanged.

| Test | Observed result | Limitation |
|---|---|---|
| Candidate v1, uptime7594–7616s | Three reset-resume cycles; six selected snapshots all COMP_STATUS1, no slaves. | Normal replacement exposed Unbalanced pm_runtime_enable on entry and restoration. No sound. |
| Candidate v2, uptime9143–9178s | Two reset-resume cycles; four snapshots COMP_STATUS1. Explicit PM disable for inherited state and removal prevents new imbalance warnings. | Test-only PM replacement handling; not a production driver patch. |
| v2 plus 20s OEM supply requests | LDO15_A1800mV/mode7 and BOB_C3300mV/mode6 all RPMh requests ret0. Three snapshots while enabled and one after disable all COMP_STATUS1, slaves0. Automatic disable_complete1. | Requests acknowledged, physical voltages not measured; reset GPIO untouched. |

The reset recovery consistently restores frame-generator bit0 in these bounded tests. This is stronger than the previous compiled-only hypothesis, but not a complete audio fix or proof that absent ClockStopNow ACK is the underlying cause. Initial probe before the first reset still logs a failed frame wait. No codec enumeration, sound playback or sound confirmation.

### Correction to PM-core interpretation

Our preceding statement about generic cleanup was incomplete. drivers/base/power/runtime.c:pm_runtime_reinit returns immediately when pm_runtime_enabled(dev). Thus qcom remove lacking pm_runtime_disable really does leave runtime PM enabled across driver unbind; the next probe logs Unbalanced pm_runtime_enable. This warning is not WARN_ON and taint stayed4096. Candidate v2 disables inherited PM at probe and disables PM at remove. Both scripts force VA/RX/SWR power/control=on for replacement; the removal code is tested under that condition only, not generalized to arbitrary suspended unbind. Do not repeat v1 for future tests.

Sources and final v2 module/script/patch saved in validated-runtime-test/. Original initial candidate files in parent are historical compiled-only artifacts. resume-candidate-result.txt corresponds to v1 with diagnostic messages; resume-supplies-result.txt corresponds to v2. The exact v1 instrumented binary was overwritten in /tmp; historical parent binary predates those messages and must not be described as byte-identical to v1 tested. v2 archived module is the tested binary.

Next investigate safe access to codec reset GPIO58 and remaining physical-link prerequisites. Existing OEM static evidence requests low5ms/high2ms; it does not prove Linux can access the pin. Reserved58..62 remain untouched because their reservation previously cured boot hang. No Bugzilla publication before sound validation.
