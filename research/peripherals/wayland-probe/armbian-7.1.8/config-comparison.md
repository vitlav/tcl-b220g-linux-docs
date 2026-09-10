# Сравнение конфигураций

| Опция | Рабочее 6.18.34-stb-qc7+ | Armbian 7.1.8-edge-arm64 |
|---|---|---|
| CONFIG_EFI_STUB | y | y |
| CONFIG_ARM64_4K_PAGES | y | y |
| CONFIG_BLK_DEV_LOOP | y | y |
| CONFIG_BTRFS_FS | y | m |
| CONFIG_USB_XHCI_HCD | y | y |
| CONFIG_USB_DWC3 | y | m |
| CONFIG_USB_DWC3_QCOM | y | m |
| CONFIG_PHY_QCOM_QMP_COMBO | y | m |
| CONFIG_PHY_QCOM_QUSB2 | y | m |
| CONFIG_SCSI_UFS_QCOM | y | m |
| CONFIG_SCSI_UFS_CRYPTO | не задано | y |
| CONFIG_QCOM_INLINE_CRYPTO_ENGINE | не задано | m |
| CONFIG_PHY_QCOM_QMP_UFS | y | m |
| CONFIG_PINCTRL_SC7180 | y | m |
| CONFIG_INTERCONNECT_QCOM_SC7180 | y | m |
| CONFIG_SC_GCC_7180 | y | m |
| CONFIG_REGULATOR_QCOM_RPMH | y | m |
| CONFIG_QCOM_SMEM | y | m |
| CONFIG_QCOM_Q6V5_MSS | y | m |
| CONFIG_QCOM_RMTFS_MEM | y | m |
| CONFIG_ATH10K_SNOC | m | m |
| CONFIG_ARM_QCOM_CPUFREQ_HW | y | m |
| CONFIG_I2C_QCOM_GENI | y | m |
| CONFIG_DRM_MSM | m | m |
| CONFIG_DRM_MSM_DSI | y | y |
| CONFIG_DRM_MSM_DSI_10NM_PHY | y | y |
| CONFIG_SC_DISPCC_7180 | y | m |
| CONFIG_FB_EFI | y | y |
