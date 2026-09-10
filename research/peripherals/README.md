# TCL B220G peripheral test v1

Prepared 2026-09-07. Experimental; no successful peripheral boot recorded yet.

Uses unchanged ready Linux 6.18.34-stb-qc7+ Image from kernel-diag. DT is derived from torvalds/linux v6.18 sc7180.dtsi; dependencies are archived in source/. All CPU, EFI and console command-line parameters match the successful eight-CPU baseline.

Enabled experiment: RPMh/SCM/SMEM/clock/interconnect infrastructure, USB DWC3 host and its USB2/combo PHYs, I2C4 (0x890000) and HID-over-I2C keyboard/touchpad. UFS, GPU, modem, Wi-Fi, CPU frequency scaling and idle are not tested here.

| Device | Address / IRQ | GPIO |
|---|---|---|
| Keyboard | I2C 0x05, HID descriptor 0x20 | IRQ 33 low/pull-up; enable 32 high |
| Touchpad | I2C 0x2c, HID descriptor 0x20 | IRQ 94 low; enable 25 high |
| USB wrapper / DWC3 | 0x0a6f8800 / 0x0a600000, SPI 133 | mainline PHY configuration |

PEP/DSDT USB D0 votes: L11_A 1,800,000 uV; L17_A 3,088,000 uV; L3_C 1,200,000 uV; L4_A 880,000 uV. Supply mapping follows the earlier TCL DTS and SC7180 USB combo driver. QUSB2 vdd=L4_A, pll=L11_A, dpdm=L17_A; combo phy=L3_C, pll=L4_A. No guessed touchpad supply voltage or unconfirmed USB hub node.

The full SoC include has disabled alternative QUP personalities with duplicate unit addresses; dtc warns about those and an unused DSI address-cell declaration. Compilation has no errors. Disabled modem/video/Wi-Fi memory references are removed because no corresponding carveouts are declared. SMP2P instances are disabled. These static checks do not establish correct hardware operation.

Initramfs contains the previously used static ARM64 BusyBox and all modules from the exact same kernel archive (dependency closure verified). It explicitly loads common USB network drivers; DHCP runs on available non-wireless interfaces. There is no SSH daemon and no Wi-Fi firmware setup in this test.

## Run

Choose `EXPERIMENTAL: USB + keyboard + touchpad + logs (no UFS)` in GRUB. Leave Kingston inserted. After USERSPACE, move the touchpad and press Shift a few times for the first minute. Raw input events are captured for 60 seconds beginning 15 seconds after userspace (up to 4096 events per device); do not enter secrets.

At 30 seconds and every 30 seconds thereafter, the test takes a snapshot. It writes only to a FAT partition labelled TCLDIAG whose USB ancestor serial equals 001CC0EC34E4FBB085C323F2. Logs: `tcl-logs/<boot UUID>/report.txt`, `dmesg.txt`, `event*.bin`. It syncs and unmounts the stick each time. If USB does not work, the RAM copies remain under /run and a photograph is needed. An early hang cannot be logged to USB by this initramfs.

Wait at least 90 seconds to preserve input captures. `USB saved=yes` means the report and dmesg were copied and sync was attempted; inspect files after returning to Windows. An interactive RAM root shell is available if the keyboard works. `snapshot` refreshes RAM reports. `reboot -f` reboots when entered at the shell; prefer waiting for a completed save first.

Previous one/eight-CPU menu entries and files are retained. EFI backup: EFI/BOOT/BOOTAA64-smp.bak. The update script checks the Kingston serial, size, non-system/non-boot status, label, previous EFI and all new payload hashes. No partitioning or formatting.
