# TCL B220G ACPI audio modules

This directory contains six external kernel modules for the matching TCL ACPI kernel. The currently qualified build uses `7.2.4-tcl-acpi-standard3+`; always build and install the modules for the exact target kernel release.

- `tcl_acpi_audio.ko`: ADSP/GLINK/APR and LPASS platform wiring.
- `tcl_acpi_wcd.ko`: firmware-described WCD9385 aggregate.
- `tcl_acpi_card.ko`: ASoC playback/capture card, Q6ASM `MultiMedia1` to WCD9385 RX/TX, with DAPM-controlled internal speaker PA GPIO46/47. Both amplifier GPIOs are changed in one array operation.
- `tcl_acpi_audio_power_hold.ko`: board-guarded OEM RPMh votes for LDO15_A (1.8 V/HPM7) and BOB_C (3.3 V/AUTO6), held until module removal.
- `tcl_acpi_codec_reset_hold.ko`: board-guarded GPIO58 reset pulse (low 5 ms, then high), with the reset deasserted until module removal and the original input state restored on unload.
- `tcl_lpi_provider.ko`: DMI-guarded ACPI/software-node provider for the SC7280 LPASS LPI pinctrl driver. It supplies the checked MMIO resources and enables the SoundWire LPI mux/configuration used by the RX/TX buses.

These are board-specific external modules, not upstream kernel patches or a generic regulator topology. Their DMI/CMD DB/GPIO guards are required. Build against the exact configured kernel tree and its `Module.symvers`:

```sh
make KERNEL_SRC=/path/to/matching/linux-7.2.4 \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1
```

The baseline `tcl_lpi_provider.c` in this directory matches the baseline kernel series. If the optional [LPASS runtime-PM series](../optional-patches/lpass-pm/README.md) is applied, use its matching `modules/tcl_lpi_provider.c` instead; that version provides the `core`/`audio` clkdev aliases consumed by the runtime-PM-aware LPI driver. Mixing the baseline provider with that driver fails with `LPI clocks unavailable`.

Install all six modules under `/lib/modules/$(uname -r)/extra/tcl-audio/` for the matching target release, run `depmod -a`, and install the files from [system/acpi/root-overlay](../../../../system/acpi/root-overlay/README.md) preserving paths. The prepare service loads the audio coordinator and SC7280 LPI pinctrl driver. The start service starts ADSP, waits for APR `q6adm` (which provides the required Q6AFE clock aliases), then registers this board's LPI provider, verifies pinmux setup, holds the OEM power/reset sequence, resumes the SoundWire buses and waits for both WCD9385 slaves to attach, registers the card, and restores the saved ALSA profile with alsactl --no-ucm; no matching TCL B220G UCM profile is present. Enable `tcl-acpi-audio-prepare.service` and `tcl-acpi-audio-start.service`. Before cleanup the unit stores the current raw ALSA mixer state, so `alsamixer` mute/volume choices survive a service restart or shutdown. The start script validates control readback without forcing fixed values. Its stop script refuses cleanup while any PCM is running, then unloads the card/codec and releases reset/rail votes.

The standard3 hardware run validated complete unload and reprobe of the card, WCD aggregate, SoundWire controller, and all three LPASS macro modules. Both WCD9385 slaves returned to `Attached`; the ALSA card returned; no Oops, refcount warning, or unbalanced runtime-PM warning appeared; direct speaker playback was audible. The tested `tcl_acpi_card.ko` and `tcl_acpi_wcd.ko` are byte-identical to modules rebuilt from these sources against the 42-patch tree. This validates those module sources, but the subsequent boot test proved that the kernel patch series itself is not yet equivalent to the working standard3 tree. Q6AFE devote replies with status `0x16` remain a separate firmware/protocol issue. See [the ACPI audio status](../../../../docs/hardware/audio-acpi.md).
