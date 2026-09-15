# ACPI audio on TCL B220G

Status checked 15 September 2026 on the separate ACPI boot, Linux `7.2.4-tcl-acpi-display1+`. The DT audio path is documented separately in [the DT audio specification](audio.md).

## Playback path

The registered ALSA card is `TCL B220G ACPI Audio` (ID `Audio`). Playback uses Q6ASM `MultiMedia1`, Q6 routing, Q6AFE backend `RX_CODEC_DMA_RX_0`, LPASS RX macro, SoundWire RX and WCD9385. The backend runs stereo, 48 kHz, `S16_LE`. The user has heard the 440 Hz test tone through the internal speakers.

The speaker PA uses GPIO46/47, controlled by a DAPM speaker widget. During playback both lines go high; they return low when the PCM closes. For orderly mpv stop, fade volume, switch off `Internal Speaker` while PCM remains `RUNNING`, then close the PCM. The ordering and GPIO transitions were verified; acoustic stop-pop absence has not been confirmed.

The persisted moderate ALSA profile is HPH 20/24 and RX digital 78/124, with headphone/RDAC, Class-H, backend and speaker switches enabled. The state is saved in `/var/lib/alsa/asound.state` and restored by the ACPI start unit. The mixer maximum is HPH 24/24 and RX digital 124/124; a short maximum-level test read back these maximum values, then restored and saved the moderate profile. Future short sound checks should use the maximum and verify with `amixer`; the saved normal profile remains moderate. The installed start script uses alsactl --no-ucm because alsaucm finds no matching TCL B220G card profile. This was tested on the target: the UCM lookup warning disappeared, the raw saved mixer state restored correctly, and only three writes to read-only HPH impedance/type controls remain.

## Modules and service lifecycle

The external modules and build instructions are in [`patches/kernel/acpi/audio-module-7.2.4/`](../../patches/kernel/acpi/audio-module-7.2.4/). They are built for the exact kernel above and are not upstream patches:

- `tcl_acpi_card.ko` registers the ASoC card and manages speaker PA through DAPM.
- `tcl_acpi_audio_power_hold.ko` checks the TCL B220G DMI identity and CMD DB addresses, then holds the OEM RPMh votes LDO15_A 1.8 V/HPM7 and BOB_C 3.3 V/AUTO6 for its module lifetime.
- `tcl_acpi_codec_reset_hold.ko` checks the initial GPIO58 state, applies the OEM reset pulse (low 5 ms, then high), keeps reset deasserted, and restores the initial input state on unload.
- `tcl_lpi_provider.ko` creates the board-guarded software node/resources consumed by the SC7280 LPASS LPI pinctrl driver; this applies the verified SoundWire GPIO mux and pad configuration.

The enabled `tcl-acpi-audio-prepare.service` loads the Qualcomm audio transport and the SC7280 LPI driver. `tcl-acpi-audio-start.service` starts ADSP, waits for APR `q6adm` and the Q6AFE clock aliases, then loads the board LPI provider and verifies pinmux setup. It next loads the guarded rail/reset owners, resumes RX/TX SoundWire runtime-PM when needed, waits for both WCD9385 slaves to attach, registers the WCD aggregate and playback card, then restores and checks the mixer profile with alsactl --no-ucm. No matching TCL B220G UCM profile is present, so this bypasses an unnecessary UCM lookup while preserving raw mixer-state restore. Its stop script refuses to release resources while a PCM is running; otherwise it unloads the card/aggregate, returns SoundWire runtime-PM to `auto`, and releases reset and rail votes.

The start/stop lifecycle was exercised by restarting the systemd unit without reboot: the service remained enabled and active afterward, the ALSA card registered, and RX/TX WCD9385 slaves reported `Attached` during startup. The user heard the test tone in the current session. The prepare unit loads the SC7280 LPI pinctrl driver; audio-start loads and verifies the board LPI provider only after ADSP/q6adm creates the required Q6AFE clock aliases. Cold-boot validation passed on 2026-09-15 (boot ID 63536663-9696-4529-a9b0-4b30e79d5d11): both units enabled and successful, LPI pinmux configured, and the TCL ACPI ALSA card registered. An 880 Hz stereo speaker-test completed at read-back maximum HPH 24/24 and RX digital 124/124; the saved moderate profile HPH 20/24 and RX 78/124 was restored and stored afterward. The UCM-free restore was deployed and the service was restarted without reboot; mixer readback remained correct and the UCM warning disappeared. After each of two audible PCM tests, RX was Attached and TX showed Alert. Restarting the audio service re-enumerated both as Attached; both remained Attached at a two-minute idle check. A temporary dynamic-debug filter on the SoundWire alert handler showed no implementation-defined interrupt log or bus/parity/alert-handling errors. Linux defines SDW_SLAVE_ALERT as an alert condition, but the specific TX cause remains unconfirmed; do not label this normal or a playback failure. The WCD probe also reports that qcom,mbhc-buttons-vthreshold-microvolt is absent. Mainline SC7280 WCD9385 board data supplies MBHC thresholds ([example](https://github.com/torvalds/linux/blob/master/arch/arm64/boot/dts/qcom/sc7280-idp.dtsi)); the TCL ACPI node has not been given copied values because they have not been validated for this laptop. Headset-button behavior remains untested. Only non-fatal writes to read-only HPH impedance/type controls remain during ALSA restore.

## Capture

Physical microphone capture and routing are not confirmed. Existing Q6ASM capture experiments did not produce a validated acoustic recording.

## ACPI property boundary

The live codec platform device is backed by the software-node `tcl-acpi-wcd9385-test`; no separate ACPI or DT node is exported in sysfs. Upstream `wcd938x` still initializes through `wcd938x_populate_dt_data()` and resolves `qcom,rx-device`/`qcom,tx-device` as OF phandles. The TCL ACPI path therefore depends on board-specific software-node glue. The missing MBHC button thresholds should be added only through that glue after headset hardware validation; values from another SC7280 board must not be copied blindly. This is not currently an explanation for the observed TX SoundWire `Alert` transition.

## Current TX status observation

After an idle period with no PCM open, RX remained `Attached` while TX was
`Alert`. Restarting `tcl-acpi-audio-start.service` without reboot re-enumerated
both slaves and returned them to `Attached` within four seconds; the ALSA card
remained registered. The transition is therefore reproducible after PCM use and
cleared by the existing service lifecycle. Its interrupt source is still not
identified, so no SoundWire or MBHC workaround is being asserted.
