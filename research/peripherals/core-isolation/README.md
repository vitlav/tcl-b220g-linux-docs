# Single-node core isolation

Based on the infrastructure variant, with USB/PHY and I2C4 disabled.
- CORE: no-qfprom: additionally disable only QFPROM. Try first.
- CORE: no-smmu: additionally disable only apps_smmu. All described DMA consumers are disabled; this is not a USB DMA bypass test.

Each is compared to infrastructure, not to the other variant. Image/initramfs/cmdline unchanged. No USB logging; photograph final output. QFPROM is a hypothesis because its probe reads security/version MMIO; GCC completed immediately before the previously photographed stop, but queue order has not been proven. SMMU is an independent candidate, not an established cause. DYNAMIC_DEBUG and FTRACE are unavailable in this kernel.

User reported hid-only also hangs, then "again the same" after being directed to infrastructure. Treat the latter as infrastructure by conversational context; exact final lines are not photo-verified. No success for these new variants yet.

Deployed to Kingston disk6 D:, all three files verified by read-back hash. Previous EFI saved to EFI/BOOT/BOOTAA64-probe-isolation.bak. Hardware tests pending.
