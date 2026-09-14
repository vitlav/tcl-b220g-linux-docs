# ACPI audio startup overlay

Install these files into the matching Ubuntu root filesystem, preserving their paths. Build/install all four matching kernel modules first, run `depmod -a`, then enable both units:

```sh
systemctl enable tcl-acpi-audio-prepare.service tcl-acpi-audio-start.service
```

The prepare unit loads Qualcomm audio transport/providers and registers the LPI pinctrl driver/provider, which configures the SoundWire pins. The start unit starts ADSP, waits for APR `q6adm`, loads the board-guarded RPMh rail and GPIO58 reset owners, resumes RX/TX SoundWire runtime-PM if required, waits for both WCD9385 slaves to attach, then registers the WCD aggregate/playback card and restores ALSA state. `ExecStop` checks that no PCM is running before releasing audio resources. The service lifecycle has been verified by a successful stop/start without reboot; verify cold-boot behavior after the updated prepare path is installed.
