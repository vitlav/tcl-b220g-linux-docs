# Hardware-qualified ACPI SPMI/PWM supplement

These files document the next ACPI-only step beyond the numbered 72-patch
series. The standalone resource adapter registers the normal Qualcomm SPMI
arbiter, describes PMIC SID4/SID5 with software nodes, creates the normal LPG
platform device, and reads channel 3 through the generic PWM API. The adapter
is opt-in (`enable=1`) and is not enabled automatically at boot. It contains no
private SPMI transfer implementation.

`0001-spmi-pmic-software-node.patch` extends the kernel's SPMI/MFD core for
software-node compatible matching, typed controller/peripheral lookup and
PMIC revision lookup. `qcom-spmi-acpi-pwm.c` is the separately built TCL board
adapter. These are kept separate from the numbered stable patch replay because
the adapter is a board-specific module and the core patch has not included in the numbered 72-patch replay yet. The core patch was checked
to apply to working source commit `4fe575fdd970551c498fb9cc824a5dc0c5ae5dc2`;
the complete clean-base replay and upstream submission remain future work.

The tested Image SHA256 is `8f42e58218445ff6109246c09dfb03661037a18f4c7801643a5f675363cb94bc`; the adapter module SHA256 is `31b9fa4b3c0909181e23e3f09f67ef8052cc5f20a88ab5c0a320723295ce4999`.

Hardware check on 2026-09-28: standard `pmic-spmi` bound to SIDs4/5, standard
`qcom-spmi-lpg` created `pwmchip0`, and `pwm_get_state_hw()` read channel3 as
851667ns period, 426667ns duty, enabled, normal polarity. Adapter unload removed
the complete device graph and restored PDC/GIC state; reload succeeded. No
PWM/GPIO writes were made. This does not qualify a permanent backlight device,
optical blanking, or suspend/wake.
