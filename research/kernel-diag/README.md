# TCL B220G experimental RAM kernel test

Manual GRUB menu entry, no automatic boot. Linux6.18.34-stb-qc7+ from hexdump0815/linux-mainline-qcom-kernel (upstream experimental/not yet tested), static ARM64 BusyBox1.37.0-6+b9 from Debian, custom minimal DTB and diagnostic initramfs. Kernel was downloaded, not rebuilt or installed.

DT contains eight CPU MPIDRs0x000..0x700, PSCI SMC, GICv3 GICD0x17a00000 and GICR0x17a60000/1MiB, and architectural timer. Addresses/MPIDRs/SMC checked against local MADT/FADT, timer INTIDs17/18/19/16 against GTDT. Polarity follows mainline sc7180.dtsi (level-low); GTDT says level-high. This discrepancy remains untested. GIC maintenance interrupt is omitted: MADT24 differs from mainline25 and this test does not use KVM. No board peripherals, remoteproc, UFS, regulator or clock-controller nodes.

Memory and framebuffer are supplied by EFI. Do not copy the GRUB allocation snapshot as fixed memory/reserved-memory. maxcpus=1 limits first test to boot CPU; cpuidle.off=1 avoids idle-state probing. Existing research parameters efi=novamap,noruntime, clk_ignore_unused and pd_ignore_unused retained. earlycon=efifb and console=tty0 request screen output. Success is not guaranteed.

Expected success banner: TCL LINUX RAM DIAGNOSTIC REACHED USERSPACE. Screen remains stable for a photo. No persistent log is written. No disk is mounted. No automatic reboot. If stuck, photograph the last output before holding power; remove USB to return to Windows. Keyboard/USB/UFS drivers are not enabled by this DT.

Validation: dtc1.8.1 compiles without warnings; Image ARM64 magic checked; BusyBox ELF machine183 and absence of PT_INTERP checked; cpio listing confirms executable init/BusyBox and console device5:1. Runtime not tested yet.

Sources: https://github.com/hexdump0815/linux-mainline-qcom-kernel/releases/tag/6.18.34-stb-qc7%2B ; https://deb.debian.org/debian/pool/main/b/busybox/busybox-static_1.37.0-6+b9_arm64.deb . Linux EFI and GCC source snapshots and hardware ACPI dumps are stored one directory above.

Rebuild: dtc -I dts -O dtb -o sc7180-tcl-minimal-efi.dtb sc7180-tcl-minimal-efi.dts; python3 build-initramfs.py /path/to/static-arm64-busybox. GRUB uses matching Debian2.12 tool/modules; modules normal configfile echo sleep reboot part_gpt part_msdos fat lsefimmap videoinfo efi_gop linux fdt gzio search search_fs_file, embedded boot/grub/grub.cfg=grub.cfg. devicetree command is provided by fdt.mod, not a devicetree.mod.
