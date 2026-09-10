# Exact OEM audio voltage test — 2026-09-09

## Correction: software voltage table is not proof of hardware quantization

0x00324b00 equals3300000 decimal microvolts exactly. Binary representation does not cause a different voltage. Linux pmic5_bob lists3000000+32000*n, which excludes3300000; this restriction is in the Linux voltage-selector table. It does not establish the PMIC request resolution.

OEM PmicDxe SHA e28eb7f6a6e3c6d41a627e478e7a124ab55057a590952084801920dfe3c02401 confirms BOB initialization at RVAad34 by pm_bob_driver.c string references. RVAae38..ae64 reads a subtype-dependent upper limit: two-byte limit times1000, or (af6c..af80) one-byte limit times32000. Crucially both paths converge ataf84: RangeMin=0 (afac), RangeMax=decoded limit (afcc), VStep=1000 (afa0,afdc). Thus32000 in this function scales the upper-limit encoding, not the request step. No firmware was executed.

Related Qualcomm source fetched via gh at pinned commit20bb8ae36c93fc16bbadda0e0a83f930c0c8a271 agrees: pm_bob_driver.c sets VStep1000; pm_pwr_alg.c converts requested microvolts to Vset by integer division. Source URLs in sources.txt. This supports issuing the original3300mV request rather than rounding it to3320mV. It does not measure the physical output or its accuracy.

## Controlled runtime tests

Fixed-purpose module uses the already-bound RPMh regulator bank devices and verifies resource addresses and absence of competing ldo15/bob DT providers. Exact active-only OEM requests:

| Resource | RPMh base | Voltage request (mV) | Mode | Enable |
|---|---|---:|---:|---:|
| ldoa15 | 0x42200 | 1800 | 7 (LDO HPM) | 1 |
| bobc1 | 0x40400 | 3300 | 6 (BOB AUTO) | 1 |

Requests bypass the Linux regulator selector table for this bounded diagnostic only. They are not a production regulator driver or persistent DT solution. Delayed work automatically sends enable0 for both resources after20seconds; script and normal module exit also perform cleanup. Voltage/mode request values remain cached; only enable votes are cleared. Other firmware clients may have their own votes, so do not describe this as measured physical power-off or full restoration of all register state. No sleep/wake votes, reserved GPIO, internal disk, or boot configuration changed.

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52. First test uptime2602s: all six requests ret0; after20s both disable requests ret0 and disable_complete1. No SoundWire slaves. Before test COMP_STATUS1; during at2605s COMP_STATUS8 (frame-generator bit0 clear). First sample coincides with3s autosuspend, so repeated with power/control=on, trap restoresauto.

Second test uptime2753s: runtime_statusactive before/during; COMP_STATUS8 already BEFORE supply enable and remains8 DURING. Therefore autosuspend alone does not explain the current register state, and the first change cannot confidently be attributed to rail power. All requests/disable ret0 again. Both modules normally unloaded. Taint4096 unchanged; Westonactive; SSHaccessible. No sound confirmed.

Important limitation: a negative slave result with frame-generator bit0 clear is not a decisive power/reset test. Next investigate the SoundWire active-but-no-frame status and missing clock/reset dependencies before testing GPIO58. Do not blindly rebind the driver: its remove() only deletes bus and disables clock, with no explicit runtime-PM teardown in this source. Reserved58..62 remain untouched. No Bugzilla publication pending sound validation.
