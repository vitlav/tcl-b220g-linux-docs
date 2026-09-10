# ALT Sisyphus: конфигурации для TCL

Проверено 2026-09-08 по полным конфигурациям бинарных RPM, без установки.

| CONFIG | Рабочее qc7 6.18.34 | ALT 7.1.13 | ALT SC7280 7.2.0 |
|---|---|---|---|
| NVMEM_QCOM_QFPROM | y | m | y |
| HWSPINLOCK_QCOM | y | y | y |
| QCOM_APCS_IPC | y | m | y |
| PINCTRL_SC7180 | y | y | y |
| SC_GCC_7180 | y | y | y |
| SC_DISPCC_7180 | y | m | off |
| SC_GPUCC_7180 | y | m | off |
| INTERCONNECT_QCOM_SC7180 | y | y | y |
| PHY_QCOM_QUSB2 | y | m | y |
| PHY_QCOM_QMP_COMBO | y | m | y |
| USB_DWC3_QCOM | y | m | y |
| SCSI_UFS_QCOM | y | m | y |
| QCOM_Q6V5_PAS | m | off | m |
| QCOM_RMTFS_MEM | y | m | y |
| ATH10K_SNOC | m | off | m |
| ARM_QCOM_CPUFREQ_HW | y | m | y |
| DRM_MSM | m | m | y |
| DRM_MSM_DSI | y | y | y |
| DRM_MSM_DSI_10NM_PHY | y | y | y |
| BTRFS_FS | y | m | y |
| BLK_DEV_LOOP | y | y | y |
| FB_EFI | y | off | y |
| DRM_SIMPLEDRM | off | y | y |

## Вывод

Общее ALT ядро содержит QFPROM, но ATH10K_SNOC и QCOM_Q6V5_PAS выключены. SC7280 содержит их, однако SC_DISPCC_7180 и SC_GPUCC_7180 выключены. Ни один пакет не признан готовым для TCL. Для разработки дисплея можно сохранить рабочую6.18.34: MSM DRM/DSI10nm в ней есть. Нужен адаптированный LT8911EXB и проверенный дисплейный DT. Новизна версии сама по себе не решает эту задачу.

## Рецепт SC7280

Исходный RPM содержит spec и единый большой patch. Spec использует BuildRequires kernel-source-7.1, применяет patch, проверяет итоговую версию7.2.0. Конфигурация: make defconfig misc.config pmos.config altlinux.config sc7280.config. ExclusiveArch=aarch64; maintainer первого ALT релиза Vasiliy Doylov (neko), 2026-08-26. Это ещё не установленная upstream provenance всех изменений; нельзя называть весь patch специфичным для SC7280.

Полезно использовать аналогичную структуру пакета с отдельным sc7180/tcl.config и минимальным обоснованным набором патчей. Сохранить проверенный TCL DT, firmware и порядок remoteproc/RMTFS. В полном patch поиск LT8911/B220G совпадений не дал; готовой реализации нашей панели не обнаружено. Сборку поручать alt-packaging-agents. Никакая сборка на этом шаге не запускалась.

Источники:
- https://ftp.altlinux.org/pub/distributions/ALTLinux/Sisyphus/aarch64/RPMS.classic/
- https://ftp.altlinux.org/pub/distributions/ALTLinux/Sisyphus/files/SRPMS/kernel-image-qualcomm-sc7280-7.2.0-alt1.src.rpm
