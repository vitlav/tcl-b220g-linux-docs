# Audio startup overlay

Copy the files into the matching Ubuntu root filesystem preserving their paths. Enable the two units with `systemctl enable tcl-acpi-audio-prepare.service tcl-acpi-audio-start.service` after installing the matching `tcl_acpi_card.ko` and running `depmod -a`.

The start unit launches ADSP, waits for APR `q6adm`, then loads the playback card. It does not claim or configure WCD9385 power/reset rails; automatic codec attachment remains an open integration item.
