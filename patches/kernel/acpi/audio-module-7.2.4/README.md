# TCL B220G ACPI audio modules

This directory contains four external kernel modules for the matching TCL ACPI kernel `7.2.4-tcl-acpi-display1+`:

- `tcl_acpi_card.ko`: ASoC playback card, Q6ASM `MultiMedia1` to WCD9385 RX, with DAPM-controlled internal speaker PA GPIO46/47.
- `tcl_acpi_audio_power_hold.ko`: board-guarded OEM RPMh votes for LDO15_A (1.8 V/HPM7) and BOB_C (3.3 V/AUTO6), held until module removal.
- `tcl_acpi_codec_reset_hold.ko`: board-guarded GPIO58 reset pulse (low 5 ms, then high), with the reset deasserted until module removal and the original input state restored on unload.
- `tcl_lpi_provider.ko`: DMI-guarded ACPI/software-node provider for the SC7280 LPASS LPI pinctrl driver. It supplies the checked MMIO resources and enables the SoundWire LPI mux/configuration used by the RX/TX buses.

These are board-specific external modules, not upstream kernel patches or a generic regulator topology. Their DMI/CMD DB/GPIO guards are required. Build against the exact configured kernel tree and its `Module.symvers`:

```sh
make KERNEL_SRC=/path/to/matching/linux-7.2.4 \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1
```

Install all four modules under `/lib/modules/7.2.4-tcl-acpi-display1+/extra/tcl-audio/`, run `depmod -a`, and install the files from `root-overlay/` preserving paths. The prepare service loads the audio coordinator, then registers the SC7280 LPI driver and this board's LPI provider before ADSP startup. Enable `tcl-acpi-audio-prepare.service` and `tcl-acpi-audio-start.service`. The start service starts ADSP, waits for APR `q6adm`, holds the OEM power/reset sequence, resumes the SoundWire buses and waits for both WCD9385 slaves to attach, registers the card, and restores the saved ALSA profile. Its stop script refuses cleanup while any PCM is running, then unloads the card/codec and releases reset/rail votes.

A full service stop/start and audible playback have been verified without reboot on the target. Cold-boot audio still requires a reboot validation. See [the ACPI audio status](../../../../docs/hardware/audio-acpi.md).
