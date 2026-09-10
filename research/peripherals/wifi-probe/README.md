# Wi-Fi firmware and board-ID diagnostic

Next step after confirmed normal USB/HID boot with gpio-reserved-ranges58/5. Same Image6.18.34-stb-qc7+, corrected logging-v2, old menu entries retained.

## Changed hardware description

- Enable WCN3990 Wi-Fi MMIO0x18800000, MSA0x93900000/2MiB confirmed by TCL DSDT AMSS.QWLN.
- Enable SC7180 MPSS PAS and modem SMP2P, reserve0x86000000/32MiB following original TCL/upstream Acer. All PT_LOAD memory ranges of the actual exported qcmpss7180_nm.mbn verified to fit this area. BSS with filesz0 has no file payload to bounds-check. Both carveouts lie inside the previously verified EFI reserved0x85b00000–0x945fffff.
- Firmware path qcom/sc7180/tcl/qcmpss7180_nm.mbn contains the exported Windows file, not the Acer file. Linux v6.18 PAS descriptor for SC7180 MPSS has auto_boot=false, so script starts its exact firmware-matched remoteproc explicitly.
- Wi-Fi supply mapping follows TCL/Acer. L9_A664000uV from TCL COEX PEP, L1_C1800000/L2_C1304000/L10_C3304000 from shared Bluetooth PEP votes. These are shared-rail evidence, not a measured Wi-Fi D0 sequence. No numeric L11_C constraint is imposed because its TCL voltage vote was not established; retain firmware voltage. Initial HPM mode follows reference.
- UFS, GPU, other remoteprocs remain disabled. Existing TLMM reserved range preserved.

## Startup sequence and limits

Logger must first successfully save a USB snapshot (wait up to120seconds). Then /wifi-probe loads ath10k_snoc and qcom_q6v5_pas, starts only the remoteproc whose firmware matches the TCL MPSS file, waits30seconds and reports interfaces, QMI/firmware/board-ID logs. One start attempt; no forced retries. Logger runs independently and copies /run/wifi.log every30seconds.

No arbitrary bdwlan chosen, no board-2.bin generated, and no AP credentials/supplicant supplied. This step aims to establish MPSS/QMI readiness and board ID, not promise an Internet connection. Windows wlanmdsp/board variants remain archived for the subsequent selection stage. Missing board data may prevent wlan interface registration and should appear in dmesg. Firmware may also fail before QMI discovery; capture logs rather than infer success.

Choose NEXT: Wi-Fi firmware and board-ID probe. Wait about3minutes, photograph the screen, and return Windows for retrieval of tcl-logs/<boot UUID>/{dmesg.txt,report.txt,wifi.log}. If boot hangs before userspace/logger, photograph final lines. If USB logging never succeeds, Wi-Fi startup is deliberately postponed with a message in /run/wifi.log; existing shell/USB/HID remain available.

Static validation: dtc completed (same disabled-QUP/DSI warnings as baseline), shell syntax checked, CPIO parsed with no duplicate paths, firmware bytes and required modules verified. Hardware test pending.
