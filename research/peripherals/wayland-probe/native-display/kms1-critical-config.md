# Критичные драйверы: рабочий qc7 → kms1

Значения проверены против исходного config рабочего qc7. Все перечисленные совпадают. `y` означает builtin, `m` — отдельный модуль.

| Группа | CONFIG | qc7 | kms1 |
|---|---|---|---|
| USB | CONFIG_USB_XHCI_HCD | y | y |
| USB | CONFIG_USB_XHCI_PLATFORM | y | y |
| USB | CONFIG_USB_DWC3 | y | y |
| USB | CONFIG_USB_DWC3_QCOM | y | y |
| USB | CONFIG_USB_STORAGE | y | y |
| USB | CONFIG_PHY_QCOM_QUSB2 | y | y |
| USB | CONFIG_PHY_QCOM_QMP_COMBO | y | y |
| USB | CONFIG_PHY_QCOM_QMP_USB | y | y |
| USB | CONFIG_NVMEM_QCOM_QFPROM | y | y |
| USB | CONFIG_HWSPINLOCK_QCOM | y | y |
| USB | CONFIG_QCOM_APCS_IPC | y | y |
| UFS | CONFIG_SCSI_UFSHCD | y | y |
| UFS | CONFIG_SCSI_UFSHCD_PLATFORM | y | y |
| UFS | CONFIG_SCSI_UFS_QCOM | y | y |
| UFS | CONFIG_PHY_QCOM_QMP_UFS | y | y |
| Wi-Fi | CONFIG_ATH10K_SNOC | m | m |
| Wi-Fi | CONFIG_ATH10K | m | m |
| Wi-Fi | CONFIG_ATH_COMMON | m | m |
| Wi-Fi | CONFIG_MAC80211 | m | m |
| Wi-Fi | CONFIG_CFG80211 | m | m |
| Wi-Fi | CONFIG_QCOM_Q6V5_MSS | y | y |
| Wi-Fi | CONFIG_QCOM_SYSMON | y | y |
| Wi-Fi | CONFIG_QCOM_RMTFS_MEM | y | y |
| Wi-Fi | CONFIG_QCOM_SMEM | y | y |
| Wi-Fi | CONFIG_QCOM_SMP2P | y | y |
| Wi-Fi | CONFIG_RPMSG_QCOM_GLINK_SMEM | y | y |
| Wi-Fi | CONFIG_QCOM_PDR_HELPERS | m | m |
| Suppliers | CONFIG_PINCTRL_SC7180 | y | y |
| Suppliers | CONFIG_SC_GCC_7180 | y | y |
| Suppliers | CONFIG_INTERCONNECT_QCOM_SC7180 | y | y |
| Suppliers | CONFIG_QCOM_RPMH | y | y |
| Suppliers | CONFIG_REGULATOR_QCOM_RPMH | y | y |
| Suppliers | CONFIG_QCOM_RPMHPD | y | y |
| Suppliers | CONFIG_ARM_SMMU_QCOM | y | y |

modules.builtin подтверждает qcom-apcs-ipc-mailbox, qcom_hwspinlock, QUSB2/QMP PHY, dwc3/qcom, xhci и UFS; QFPROM проверен по CONFIG_NVMEM_QCOM_QFPROM=y (drivers/nvmem/nvmem_qfprom.ko).
Фактически собраны ath10k_snoc.ko, ath10k_core.ko, ath.ko, mac80211.ko, cfg80211.ko и pdr_interface.ko.
По modinfo: ath10k_snoc → ath10k_core → mac80211,cfg80211,ath. Service/firmware remoteproc prerequisites остаются отдельной частью initramfs/userspace.
