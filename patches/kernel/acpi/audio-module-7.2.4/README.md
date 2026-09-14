# TCL B220G ACPI playback module

This directory contains the external ASoC card module tested with the matching TCL ACPI Linux kernel `7.2.4-tcl-acpi-display1+`. It is not an upstream kernel patch. The card joins Q6ASM `MultiMedia1` to the WCD9385 SoundWire RX path and controls the internal speaker PA GPIO46/47 through a DAPM speaker widget.

Build against the exact kernel build tree and its `Module.symvers`:

```sh
make KERNEL_SRC=/path/to/matching/linux-7.2.4 \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1
```

Install the resulting `tcl_acpi_card.ko` under the matching kernel's `extra/tcl-audio/`, run `depmod -a`, then load it with `modprobe tcl_acpi_card`. The boot preparation/start units and scripts are included under `root-overlay/`; the start unit waits for ADSP `q6adm` before loading the card.

The card and DSP startup are automated. The WCD9385 regulator/reset mapping and automatic SoundWire attachment are still incomplete, so a registered ALSA card alone does not prove speaker playback is ready after cold boot. See [the ACPI audio status](../../../../docs/hardware/audio-acpi.md) for tested state and remaining hardware integration.
