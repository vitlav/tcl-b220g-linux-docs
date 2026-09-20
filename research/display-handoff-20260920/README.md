# ACPI display firmware handoff validation

Validated on TCL B220G with Linux `7.2.4-tcl-acpi-standard3+`, boot ID `65437f4c-5466-4de4-99a2-26a660e067d2`.

The coordinator stops firmware DPU scanout before creation of the DPU platform device and attachment of SID `0x800` to Linux's IOMMU domain. It then adopts and disables the firmware-enabled DSI byte, byte-interface and pixel clock branches through balanced CCF calls before changing the shared DSI VCO.

The saved diagnostic boot proves that the branches changed from enabled (`0x1`) to root-off (`0x80000000`) before clock programming, then returned to enabled under Linux DRM. The boot contained neither the former SID `0x800` context fault nor `disp_cc_mdss_pclk0_clk_src: rcg didn't update its configuration`. DRM registered `msm-kmsdrmfb` and eDP was connected.

`kernel-success.log` contains only the relevant ordered handoff messages from the successful boot. The register dumps came from a read-only diagnostic revision and are intentionally absent from the published kernel patch.
