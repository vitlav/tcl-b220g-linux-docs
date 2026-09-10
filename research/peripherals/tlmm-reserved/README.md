# Restore TCL reserved GPIO range

Manual-probe photograph photo_2026-09-07_01-50-37.jpg shows USERSPACE, successful GCC and last BEGIN device=3500000.pinctrl without END. No watcher stack is visible; global firmware freeze is not proven.

Original TCL DTS and upstream SC7180 QC710 both specify gpio-reserved-ranges = <58 5>, describing possible TrustZone protection for fingerprint-related pins. This board restriction was accidentally omitted from our new DT. Linux v6.18 gpiolib initializes the valid mask from reserved ranges before calling get_direction for valid lines; MSM get_direction reads TLMM MMIO. Thus a concrete causal path exists, but exact protected pin and hardware success are not established yet.

New DTB differs from original peripheral DTB by exactly one decoded property: gpio-reserved-ranges = <58 5>. Same Image/initramfs for each respective menu baseline.

First run FIX TEST: TLMM reserved GPIO 58-62 (USB/HID), using normal deferred probe and the logging initramfs. If this hangs, TRACE: manual probes with TLMM reserved GPIO retains blacklist/manual probing to locate the next stop. Deployed on Kingston disk6 D:, both new files verified by read-back SHA256. Hardware result pending. See usb-update-result.json.
