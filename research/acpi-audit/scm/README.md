# SCM ACPI: QCOM080B и DMA

**Статус:** SCM v2 включён в полностью собранный ACPI2 Image; аппаратно не проверен. [Спецификация ACPI](../../../docs/acpi.md#scm-и-dma).

OEM SCM0 не содержит _CCA и _CRS. При ACPI_CCA_REQUIRED отсутствие _CCA блокирует DMA support; успешный probe без реальной allocation недостаточен. Безусловный devm_of_icc_get также не применим к устройству без OF-node.

[scm-acpi-draft-v2.patch](../../../patches/kernel/acpi/scm-acpi-draft-v2.patch) добавляет match QCOM080B, условный OF ICC, DMA guard, initial ACPI TZMEM pool PAGE_SIZE и acpi_dev_clear_dependencies после успеха. Предыдущий [v1](../../../patches/kernel/acpi/scm-acpi-draft.patch) заменён v2; вместе не применяются.

[SSDT SCM0._CCA=0 и early CPIO](early-ssdt/README.md) проверены компиляцией, AML evaluation и kernel parser. _CCA требуется до создания ACPI platform device; позднее добавление свойства не пересоздаёт его DMA mask автоматически.

Рабочий DT: coherent=0, mask/coherent_mask=0xffffffff. Выделение/освобождение страницы с CPU pattern прошло на оборудовании без SMC. GENERIC TZMEM использует dma_alloc_coherent, DMA API обслуживает noncoherent device. Передача буфера TrustZone этим не проверена.

SCM probe включает __get_convention, настройку download mode/SDI и QSEECOM/QTEE в зависимости от конфигурации. Optional clocks, IRQ и reserved-memory не устраняют необходимость проверить эти побочные действия. [Состав полной сборки](../../acpi-boot/acpi2/README.md).
