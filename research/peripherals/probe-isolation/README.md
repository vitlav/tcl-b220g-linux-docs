# Deferred probe isolation

First run `ISOLATE: infrastructure (initcall trace)`.

Three DT variants retain the same infrastructure as peripheral v1:
- infrastructure: I2C4, USB wrapper and both USB PHYs disabled;
- usb-only: I2C4 disabled;
- hid-only: USB wrapper and both USB PHYs disabled.

SCM/SMEM/RPMh/GCC/interconnect/TLMM/SMMU and QUP wrapper remain enabled in all variants. Thus infrastructure is not the previously successful minimal CPU DT. If it hangs, the problem is below the USB/HID leaf drivers; if it reaches userspace, test the other variants to isolate the peripheral branch. Shared infrastructure interactions still need consideration.

Same Image/initramfs and command line with initcall_debug; old menu entries retained. Infrastructure and HID-only variants cannot save logs to USB because USB is disabled; take photos. The USB-only variant requires an external USB keyboard for interactive input, but automatically saves logs if the stick works. No Wi-Fi or UFS enabled. No success recorded yet; deployed on Kingston disk6 D:, all four new files verified; see usb-update-result.json.
