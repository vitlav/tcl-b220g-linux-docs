# ACPI audio startup overlay

Install these files into the matching Ubuntu root filesystem, preserving their paths. Build/install all four matching kernel modules first, run `depmod -a`, then enable both units:

```sh
systemctl enable tcl-acpi-audio-prepare.service tcl-acpi-audio-start.service
```

The prepare unit loads Qualcomm audio transport/providers and the SC7280 LPI pinctrl driver. The start unit starts ADSP, waits for APR `q6adm` and its Q6AFE clock aliases, then registers the board-guarded LPI provider to configure SoundWire pins. It next loads the RPMh rail and GPIO58 reset owners, resumes RX/TX SoundWire runtime-PM if required, waits for both WCD9385 slaves to attach, then registers the WCD aggregate/playback card and restores ALSA state. `ExecStop` checks that no PCM is running before releasing audio resources. The corrected sequence has been exercised in the current boot; one cold-boot validation of this final order remains.
