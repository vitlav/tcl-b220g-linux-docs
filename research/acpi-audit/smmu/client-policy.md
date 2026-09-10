# ACPI SMMU: выбор реализации и политика клиентов

2026-09-10. Аудит Linux 6.18.34; изменений SMMU-кода и аппаратных операций в этом этапе нет.

| Уровень | Рабочий DT | Что теряется при простом добавлении ACPI платформы |
|---|---|---|
| Выбор SoC | sc7180-smmu-500 / sc7180-smmu-v2 | OEM revision 0x7180 отсутствует в списке ACPI |
| Apps SMMU | 0x15000000, MMU500, qcom_smmu_500_impl0_data | Нужны cfg_probe/write_s2cr с Qualcomm quirk, а не только generic MMU500 reset |
| Adreno SMMU | 0x05040000, SMMUv2, qcom_adreno_smmu_v2_impl | qcom_smmu_create выбирает adreno_impl **только через OF compatible**; даже qcom_smmu_v2_data через ACPI выберет обычный v2 impl |
| Default domain клиента | qcom_smmu_def_domain_type сопоставляет OF compatible, включая sc7180-mdss и GPU | При ACPI OF-match пуст, функция возвращает 0 — выбор общей политики IOMMU, а не явно IOMMU_DOMAIN_IDENTITY. Фактический итог зависит от конфигурации/параметров ядра |
| ACTLR клиента | qcom_smmu_set_actlr_dev использует OF match | ACPI-клиент не получает эти настройки; не все они применяются к SC7180: отдельного sc7180-mdss ACTLR-entry в списке нет |
| GPU context bank / TTBR1 | qcom_adreno_smmu_is_gpu_device проверяет SID0 в iommu_fwspec | Эта проверка сама не требует OF; её можно сохранить при корректном IORT mapping и выборе Adreno impl. Нельзя объявлять весь GPU-path зависимым от DT |

Следовательно, будущий патч должен отдельно задавать реализацию для Apps и Adreno, а политику доменов выбирать по подтверждённой идентичности конкретного клиента. Не применять IOMMU identity глобально и не считать один platform-match достаточным для сохранения дисплея.

`GPU0.AVS0` отсутствует в текущем AML namespace, но указан в IORT; исправление SMMU не создаст этот ACPI device автоматически. Связанные задачи: #19515 (SMMU), #19517 (SCM), #19512 (AVS0), #19492 (ACPI).

Причина чёрного экрана по-прежнему не доказана последними строками ядра. Изложенное — конкретные различия пути инициализации, а не утверждение, что каждое из них уже проявилось аппаратно.
