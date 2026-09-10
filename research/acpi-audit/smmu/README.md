# SMMU: различие DT и ACPI TCL, 2026-09-10

Живая IORT QCOM / QCOMEDK2 / OEMrevision0x7180. qcom_acpi_platlist в Linux6.18.34 принимает только0x8180. CONFIG_ARM_SMMU_QCOM=y не гарантирует выбора Qualcomm implementation.

| Адрес | IORT | DT implementation |
|---|---|---|
|0x15000000/1MiB|model3 MMU-500|qcom_smmu_500_impl0_data|
|0x05040000/64KiB|model1 SMMUv2|qcom_adreno_smmu_v2_impl|

Код: arm-smmu-qcom.c732/779 — ACPI match; arm-smmu.c1968/2118 — выбор/проба;1658 — reset; qcom_smmu_cfg_probe414 — firmware quirk и перенос валидныхSMR; qcom_smmu_create624 — зависимостьSCM. IORT18узлов:2SMMUv1/v2,16NamedComponent,нетRMR. iort_init_platform_devices создаётконтроллеры до наличия клиентскихдрайверов.

apps_smmu обслуживает дисплей SID0x800 mask0x2. Общий reset переписываетSMR/S2CR/contextbanks; Qualcomm probe сохраняет известные прошивкеSMR и учитывает особенностиBYPASS. ACPI1 без совпаденияOEMревизии теряет эту реализацию. Это кандидат причины чёрного экрана, но не аппаратное доказательство. Не делать вывод, чтоgenericreset точно был последней операцией.

Однострочное добавление0x7180 недостаточно: ACPIпуть сейчас выбирает500implementation для всехконтроллеров, а Adreno требуетV2. qcom_scmдрайверOF-only; qcom_smmu_create безSCM возвращаетEPROBE_DEFER. Требуется решить SCM/ACPI, разделение двухSMMU, clientmappingиfirmwarehandover вместе. Глобальныйpassthrough и runtimeunbind/reset не выполнялись.

IORT также содержит NamedComponent\_SB.GPU0.AVS0: отсутствующий объект затрагивает не толькоTZ7, но иDMAописание. См.19512. На этой стадии таблицы/код ядра/драйверы не менялись, новых перезагрузок не было.
