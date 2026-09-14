# ACPI audio startup overlay

Install these files into the matching Ubuntu root filesystem, preserving their paths. Build/install all four matching kernel modules first, run `depmod -a`, then enable both units:

```sh
systemctl enable tcl-acpi-audio-prepare.service tcl-acpi-audio-start.service
```

The prepare unit loads Qualcomm audio transport/providers and the SC7280 LPI pinctrl driver. The start unit starts ADSP, waits for APR `q6adm` and its Q6AFE clock aliases, then registers the board-guarded LPI provider to configure SoundWire pins. It next loads the RPMh rail and GPIO58 reset owners, resumes RX/TX SoundWire runtime-PM if required, waits for both WCD9385 slaves to attach, then registers the WCD aggregate/playback card and restores ALSA state with alsactl --no-ucm; no matching TCL B220G UCM profile is present. `ExecStop` checks that no PCM is running before releasing audio resources. The corrected sequence passed cold-boot validation on 2026-09-15: both units enabled and successful, board LPI pinmux confirmed, ALSA card registered, and the maximum-level 880 Hz stereo test completed. The saved moderate mixer profile was restored and stored after playback.
