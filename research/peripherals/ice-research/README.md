# Qualcomm ICE investigation, TCL B220G

Current hardware result (2026-09-09): ICE v3.1.75 probed successfully on kernel6.18.34-tcl-ice1, AES-256-XTS/32 slots, all UFS readonly. Encrypted I/O not tested. LUN4 initial-MiB hash changed; older archive comparison isolates four bytes in limits, cause unproven. See [hardware report](ice1/hardware-report.md).

ICE = Inline Crypto Engine, not ICU. SC7180 node /soc@0/crypto@1d90000, reg0x1d90000+0x8000, compatible qcom,sc7180-inline-crypto-engine/qcom,inline-crypto-engine. It processes UFS I/O encryption in-line; it is not a separate disk controller or a BitLocker key-recovery mechanism.

## Historical state before ICE1

Running kernel6.18.34-tcl-kms1: BLK_INLINE_ENCRYPTION=n, FS_ENCRYPTION=n, DM_CRYPT=y. SCSI_UFS_CRYPTO depends on BLK_INLINE_ENCRYPTION; UFS_QCOM selects QCOM_INLINE_CRYPTO_ENGINE only when SCSI_UFS_CRYPTO. Therefore ICE support cannot simply be enabled with modprobe in this build. DT node disabled and UFS qcom,ice reference removed during original UFS bring-up to avoid deferred probe. Plain UFS I/O works with ICE absent.

## Source review

Exact local kernel sources: drivers/soc/qcom/ice.c, drivers/ufs/host/ufs-qcom.c, drivers/ufs/core/Kconfig, block/Kconfig, Documentation/block/inline-encryption.rst.

qcom_ice_create checks SCM availability, SCM ICE service, enables the ICE clock, reads ICE version/fuse settings. Driver supports ICE major3/4 and rejects certain fuse modes. Runtime hardware version/fuse state on this TCL has NOT been read yet. Raw AES-256-XTS keys are programmed via SCM; wrapped keys require HWKM and supported secure-world calls. Source says earliest effectively supported wrapped-key SoC is SM8650, so no claim of wrapped-key support on SC7180.

Kernel dm-crypt implementation uses crypto_skcipher API. This ICE driver serves blk-crypto keyslots, not a crypto_skcipher provider; existing LUKS/dm-crypt does not automatically gain ICE acceleration. A normal user is fscrypt on a supported filesystem with inline encryption enabled. Existing Btrfs root-on-USB loop image is not an appropriate demonstration of UFS ICE.

## Next safe hardware step

Prepare separate kernel with BLK_INLINE_ENCRYPTION, SCSI_UFS_CRYPTO and QCOM_INLINE_CRYPTO_ENGINE; if testing fscrypt later also FS_ENCRYPTION and relevant filesystem encryption options. Enable the ICE DT node and restore UFS qcom,ice link only in the new test DT. Audit clocks/SCM dependencies before reboot.

First test is probe/capability discovery with all UFS LUNs still readonly, no mounting Windows partitions rw and no key programming requested by test software. Probe may enable ICE hardware but plain I/O must remain usable. Compare UFS identities/readonly state and capability logs. If supported, design a separate data-path experiment on explicitly allocated disposable UFS storage; no such storage has been allocated or formatted. No disk writes, partition changes, key extraction or encryption tests performed during this investigation.

Prepared, not installed: probe-config.fragment and ice-probe.dts. First-stage capability check targets /sys/block/<confirmed-UFS-LUN>/queue/crypto/ (blk-crypto sysfs interface); do not infer support merely from a node name. The original user request was investigation, so no ICE kernel or disk changes were made.

The ICE driver does not call request_firmware; it relies on secure-world SCM. qcom_scm_ice_available queries availability of ES_INVALIDATE_ICE_KEY and ES_CONFIG_SET_ICE_KEY, not actual key programming. No Qualcomm firmware download is indicated by this driver path.
