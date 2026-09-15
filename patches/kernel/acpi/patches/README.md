# Состав ACPI-серии

Порядок применения задан в [series](series). База, конфигурация и границы проверки — в [описании комплекта](../README.md). В каждом файле есть подробное описание изменения.

| № | Изменение |
|---|---|
| 01 | [ACPI: omit PCI default address space when PCI is disabled](0001-ACPI-omit-PCI-default-address-space-when-PCI-is-disa.patch) |
| 02 | [software node: guard unnamed nodes during graph traversal](0002-software-node-guard-unnamed-nodes-during-graph-trave.patch) |
| 03 | [remoteproc: coordinate deletion with crash and subdevice shutdown](0003-remoteproc-coordinate-deletion-with-crash-and-subdev.patch) |
| 04 | [drm/msm: initialize pagetable ownership before fallible allocation](0004-drm-msm-initialize-pagetable-ownership-before-fallib.patch) |
| 05 | [drm/msm: identify the vmap fbdev buffer as system memory](0005-drm-msm-identify-the-vmap-fbdev-buffer-as-system-mem.patch) |
| 06 | [interconnect: expose validated named path lookup to consumers](0006-interconnect-expose-validated-named-path-lookup-to-c.patch) |
| 07 | [interconnect: qcom: reject partial BCM initialization](0007-interconnect-qcom-reject-partial-BCM-initialization.patch) |
| 08 | [ACPI: resolve namespace devices and physical companions uniformly](0008-ACPI-resolve-namespace-devices-and-physical-companio.patch) |
| 09 | [ACPI: IORT: validate TCL USB WLAN display GPU and audio DMA mappings](0009-ACPI-IORT-validate-TCL-USB-WLAN-display-GPU-and-audi.patch) |
| 10 | [firmware: qcom: initialize SCM and its reset controller through ACPI](0010-firmware-qcom-initialize-SCM-and-its-reset-controlle.patch) |
| 11 | [iommu: arm-smmu: select and power the TCL Qualcomm implementations](0011-iommu-arm-smmu-select-and-power-the-TCL-Qualcomm-imp.patch) |
| 12 | [soc: qcom: define board-owned ACPI PAS and regulator resources](0012-soc-qcom-define-board-owned-ACPI-PAS-and-regulator-r.patch) |
| 13 | [soc: qcom: describe SMEM locks mailboxes and SMP2P with firmware nodes](0013-soc-qcom-describe-SMEM-locks-mailboxes-and-SMP2P-wit.patch) |
| 14 | [soc: qcom: support ACPI RPMh AOSS and memory suppliers](0014-soc-qcom-support-ACPI-RPMh-AOSS-and-memory-suppliers.patch) |
| 15 | [rpmsg: qcom: construct GLINK transports and channels from firmware nodes](0015-rpmsg-qcom-construct-GLINK-transports-and-channels-f.patch) |
| 16 | [remoteproc: qcom: support firmware-described TCL modem and ADSP](0016-remoteproc-qcom-support-firmware-described-TCL-modem.patch) |
| 17 | [soc: qcom: enumerate APR services from firmware nodes](0017-soc-qcom-enumerate-APR-services-from-firmware-nodes.patch) |
| 18 | [wifi: ath10k: validate TCL ACPI SNOC resources before probe](0018-wifi-ath10k-validate-TCL-ACPI-SNOC-resources-before-.patch) |
| 19 | [pinctrl: qcom: describe bounded TCL ACPI input and speaker GPIOs](0019-pinctrl-qcom-describe-bounded-TCL-ACPI-input-and-spe.patch) |
| 20 | [i2c: qcom-geni: support the TCL keyboard and bridge without a QUP wrapper](0020-i2c-qcom-geni-support-the-TCL-keyboard-and-bridge-wi.patch) |
| 21 | [clk: qcom: provide bounded TCL display and GPU clock trees](0021-clk-qcom-provide-bounded-TCL-display-and-GPU-clock-t.patch) |
| 22 | [interconnect: qcom: support firmware-described TCL fabric suppliers](0022-interconnect-qcom-support-firmware-described-TCL-fab.patch) |
| 23 | [soc: qcom: select SC7180 UBWC parameters on TCL ACPI](0023-soc-qcom-select-SC7180-UBWC-parameters-on-TCL-ACPI.patch) |
| 24 | [drm: discover bridges and DSI devices through firmware nodes](0024-drm-discover-bridges-and-DSI-devices-through-firmwar.patch) |
| 25 | [drm/msm: bind ACPI display components with explicit resources and OPPs](0025-drm-msm-bind-ACPI-display-components-with-explicit-r.patch) |
| 26 | [drm/msm/dsi: support the TCL firmware-described host and PHY](0026-drm-msm-dsi-support-the-TCL-firmware-described-host-.patch) |
| 27 | [drm: bridge: support the existing TCL LT8911 ACPI handoff device](0027-drm-bridge-support-the-existing-TCL-LT8911-ACPI-hand.patch) |
| 28 | [drm/msm/adreno: consume validated TCL ACPI GPU and GMU resources](0028-drm-msm-adreno-consume-validated-TCL-ACPI-GPU-and-GM.patch) |
| 29 | [ASoC: qdsp6: populate firmware-described TCL audio services](0029-ASoC-qdsp6-populate-firmware-described-TCL-audio-ser.patch) |
| 30 | [ASoC: qcom: support firmware-described LPASS macros and SoundWire pins](0030-ASoC-qcom-support-firmware-described-LPASS-macros-an.patch) |
| 31 | [soundwire: enumerate TCL firmware nodes and handle controller rebinding](0031-soundwire-enumerate-TCL-firmware-nodes-and-handle-co.patch) |
| 32 | [ASoC: wcd938x: bind TCL firmware nodes and clean aggregate IRQ state](0032-ASoC-wcd938x-bind-TCL-firmware-nodes-and-clean-aggre.patch) |
| 33 | [usb: dwc3: add the guarded TCL ACPI host handoff driver](0033-usb-dwc3-add-the-guarded-TCL-ACPI-host-handoff-drive.patch) |
| 34 | [soc: qcom: wire TCL ACPI SMEM modem and WLAN suppliers](0034-soc-qcom-wire-TCL-ACPI-SMEM-modem-and-WLAN-suppliers.patch) |
| 35 | [soc: qcom: wire TCL ACPI display and GPU with ordered firmware handoff](0035-soc-qcom-wire-TCL-ACPI-display-and-GPU-with-ordered-.patch) |
| 36 | [arm64: record TCL ACPI diagnostic and display configurations](0036-arm64-record-TCL-ACPI-diagnostic-and-display-configu.patch) |
