# Initcall trace after peripheral boot hang

Same Image, DTB and initramfs as peripheral v1. Adds only `initcall_debug` in a new GRUB entry; the six previous entries remain intact. Deployed and read-back verified on Kingston disk6 D:. See usb-update-result.json.

No Windows update is necessary for a single test: in GRUB select the USB/HID entry, press `e`, append ` initcall_debug` to the line beginning `linux`, then boot with Ctrl+X or F10. This edit is temporary. Photograph the last messages, including `calling ...` and `initcall ... returned ...`.

This locates synchronous initcall stalls; asynchronous probes may require additional diagnostics. Last visible AppArmor message is not proof that AppArmor caused the hang.
