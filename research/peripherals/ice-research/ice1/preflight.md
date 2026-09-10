# ICE1 preflight

Separate source/build tree /tmp/tcl-ice1-linux copied from the running DPU-diagnostic6.18.34-tcl-kms1 source. DPU diagnostic's three files byte-identical. New release6.18.34-tcl-ice1. Delta: BLK_INLINE_ENCRYPTION=y, SCSI_UFS_CRYPTO=y, QCOM_INLINE_CRYPTO_ENGINE=y; fallback n, MMC crypto n. FS_ENCRYPTION remains disabled (probe only).

DT audit240 nodes, includes qcom,ice reference and UFS suppliers; no missing/disabled references. DT adds ICE status okay and UFS phandle to previous thermal/GPU DT. Actual SCM/version/fuse support unverified until boot.

Read-only protection confirmed on current system: all six UFS LUNs RO=1; root loop image resides on USB. Existing udev rule /etc/udev/rules.d/99-tcl-ufs-readonly.rules applies blockdev --setro on add/change for devices under1d84000.ufshc, covering delayed block discovery too. Initramfs controller-based readonly loop retained.

Potential failure observed in source: qcom_ice_create returns NULL when SCM ICE calls unavailable; qcom_ice_probe can bind with NULL drvdata; consumer returns EPROBE_DEFER. Test root remains on USB so this should leave userspace diagnosis possible. Do not infer ICE working merely from platform driver link; require logs/version and UFS queue/crypto capabilities.

Collectors preserve kernel log, deferred devices, UFS readonly/size and crypto sysfs. No key programming by userspace, no encrypted filesystem or partition changes requested.

## Build and archive validation

make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j8 Image modules exited0. External handoffv2 rebuilt with W=1/MODPOST, exit0. Four pre-existing unused-variable warnings in unused Acer EC driver; no new warning category. Module tar/install tree/initramfs/BusyBox/scripts/three GPU firmware files byte-compared, 949 modules all have matching6.18.34-tcl-ice1 vermagic. All SHA256 in validation.json/SHA256SUMS.

New init/collector target ICE1 paths and record actual UFS crypto capability sysfs. GRUB script check and ARM64 EFI check passed with coherent Debian2.12 tools/modules. Old boot entries intact; ICE1 will be selected separately by default. Baseline first1MiB per UFS LUN was read while RO=1; six checksums saved on TCL /var/tmp/tcl-ice-before-read.sha256 for comparison after boot.
