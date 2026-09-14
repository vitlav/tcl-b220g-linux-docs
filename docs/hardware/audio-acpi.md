# ACPI audio on TCL B220G

Status checked 14 September 2026 on the separate ACPI boot, Linux `7.2.4-tcl-acpi-display1+`. The DT audio path is documented separately in [the DT audio specification](audio.md).

## Playback path

The registered ALSA card is `TCL B220G ACPI Audio` (ID `Audio`). Playback uses Q6ASM `MultiMedia1`, Q6 routing, Q6AFE backend `RX_CODEC_DMA_RX_0`, LPASS RX macro, SoundWire RX and WCD9385. The backend runs stereo, 48 kHz, `S16_LE`. The user has heard the 440 Hz test tone through the internal speakers.

The speaker PA uses GPIO46/47, controlled by a DAPM speaker widget. During playback both lines go high; they return low when the PCM closes. For orderly mpv stop, fade volume, switch off `Internal Speaker` while PCM remains `RUNNING`, then close the PCM. The ordering and GPIO transitions were verified; acoustic stop-pop absence has not been confirmed.

The persisted moderate ALSA profile is HPH 20/24 and RX digital 78/124, with headphone/RDAC, Class-H, backend and speaker switches enabled. The state is saved in `/var/lib/alsa/asound.state` and restored by the ACPI start unit. The mixer maximum is HPH 24/24 and RX digital 124/124; a short maximum-level test read back these maximum values, then restored and saved the moderate profile. Future short sound checks should use the maximum and verify with `amixer`; the saved normal profile remains moderate. Read-only impedance/HPH-type warnings and the missing UCM profile do not prevent direct ALSA playback.

## Modules and service lifecycle

The external modules and build instructions are in [`patches/kernel/acpi/audio-module-7.2.4/`](../../patches/kernel/acpi/audio-module-7.2.4/). They are built for the exact kernel above and are not upstream patches:

- `tcl_acpi_card.ko` registers the ASoC card and manages speaker PA through DAPM.
- `tcl_acpi_audio_power_hold.ko` checks the TCL B220G DMI identity and CMD DB addresses, then holds the OEM RPMh votes LDO15_A 1.8 V/HPM7 and BOB_C 3.3 V/AUTO6 for its module lifetime.
- `tcl_acpi_codec_reset_hold.ko` checks the initial GPIO58 state, applies the OEM reset pulse (low 5 ms, then high), keeps reset deasserted, and restores the initial input state on unload.

The enabled `tcl-acpi-audio-prepare.service` loads the Qualcomm audio transport and providers. `tcl-acpi-audio-start.service` starts ADSP, waits for APR `q6adm`, loads the guarded rail/reset owners, resumes RX/TX SoundWire runtime-PM when needed, waits for both WCD9385 slaves to attach, registers the WCD aggregate and playback card, then restores and checks the mixer profile. Its stop script refuses to release resources while a PCM is running; otherwise it unloads the card/aggregate, returns SoundWire runtime-PM to `auto`, and releases reset and rail votes.

The start/stop lifecycle was exercised by restarting the systemd unit without reboot: the service remained enabled and active afterward, the ALSA card registered, and RX/TX WCD9385 slaves reported `Attached` during startup. The tone was audible in the current session. This proves repeatable service startup in the running boot; a cold boot with this configuration has not yet been tested and still needs one reboot validation.

## Capture

Physical microphone capture and routing are not confirmed. Existing Q6ASM capture experiments did not produce a validated acoustic recording.
