# Public70 combined ACPI qualification

The combined public70 build includes the ACPI display, SMMU runtime-PM, standalone-DPU guard, and Venus GDSC/genpd changes.

The tested Image SHA256 is `d1047672b1ab75159aa13fe4c822ec59929e07371ec3ff56f01535edc3e4b2ef`.

The standalone ACPI DPU is registered with the KMS-only `msm-kms` DRM driver. This deliberately leaves the render node to Adreno; otherwise the DPU and Adreno expose two render nodes and compositors can select the DPU accidentally. The tested `msm.ko` SHA256 is `0cd1e10106148e26ae8432dba147b0e166116a5c1e0fd6371020d266f59b5927`.

Hardware results on TCL B220G:

- DPU/card0 and eDP-1 bind successfully at 1920x1080@60.
- The only render node is Adreno `/dev/dri/renderD128`.
- Niri selects Adreno, creates its Wayland socket, and drives eDP.
- `vulkaninfo --summary` detects Adreno 618 and exits successfully.
- No `no GPU device`, `drm_sched`, or SMMU runtime-PM failure occurs in this boot.

Venus remains a separate unresolved issue: its ACPI platform device is created, but `qcom-venus` probe returns `-5` and decoder/encoder nodes `/dev/video2` and `/dev/video3` are not registered. Do not treat the Venus failure as a display or GPU failure.
