# ICE1 reproduction commands

Base tree is reconstructed qc7-based Linux6.18.34 plus the previously saved DPU diagnostic patch. Preserve the original source tree and modules.

```sh
cp -a --reflink=auto /tmp/tcl-qc7-build/linux-6.18.34 /tmp/tcl-ice1-linux
cd /tmp/tcl-ice1-linux
scripts/config --set-str LOCALVERSION -tcl-ice1 --enable BLK_INLINE_ENCRYPTION --enable SCSI_UFS_CRYPTO --enable QCOM_INLINE_CRYPTO_ENGINE
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olddefconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j8 Image modules
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=/tmp/tcl-ice1-stage INSTALL_MOD_STRIP=1 DEPMOD=/sbin/depmod modules_install
# Copy handoff v2 source/Makefile into /tmp/tcl-ice1-handoff first.
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=/tmp/tcl-ice1-handoff W=1 modules
```

Then install stripped handoff module into stage extra/, depmod, run this directory's build-initramfs.py with known ARM64 BusyBox, stage/lib/modules, firmware root, output gzip. The three firmware files and source provenance are documented in native-display/kms1-gpu/. Use ice-probe.dts in parent directory with its original relative include tree. Scripts are adapted from the working GPU initramfs only for new release/log paths and additional ICE capability collection.

Boot with matching Image/modules/initramfs only. Current install.sh verifies transferred archives and every installed module/metadata checksum, then installs USB boot files and atomically replaces EFI last. It assumes the saved expected previous EFI SHA; do not bypass mismatches.
