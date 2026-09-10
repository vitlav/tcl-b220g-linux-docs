# Конфигурация Linux ACPI2 для TCL B220G

**Степень подтверждения: полная сборка Image и модулей; на оборудовании не проверено.**

## Требуемые изменения

База: Linux 6.18.34 с локальными изменениями поддержки TCL. Полная последовательная серия для чистой upstream-базы пока не сформирована.

| Изменение | Назначение |
|---|---|
| [ACPICA при PCI=n](../../../patches/kernel/acpi/acpica-default-spaces-without-pci.patch) | Условная регистрация обработчика PCI_CONFIG |
| [SCM ACPI v2](../../../patches/kernel/acpi/scm-acpi-draft-v2.patch) | QCOM080B, проверка DMA и ACPI-зависимости |
| [SC7180 SMMU](../../../patches/kernel/acpi/sc7180-acpi-smmu-selection.patch) | Раздельный выбор Apps и Adreno implementation |

Конфигурация: [kernel.config](kernel.config). Включены ACPI, ACPI_TABLE_UPGRADE, ACPI_CCA_REQUIRED, QCOM_SCM, QCOM_TZMEM_MODE_GENERIC и ARM_SMMU_QCOM. UFS host отключён в этой диагностической конфигурации. Release: `6.18.34-tcl-acpi2`.

## Проверка сборки

Кросс-сборка ARM64: `ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-`, цели `Image modules`. Полная сборка и установка модулей в staging успешны. Все 948 устанавливаемых модулей имеют соответствующий release. [Манифест и SHA256](manifest.json).

Image: 32811520 байт, SHA256 `b482eda71cbe4a7949d5cf220f3e4a0c1305efa2a79f47ebd6606d453b1845e1`.

## Условия применимости

SCM требует [ранней SSDT с _CCA=0](../../acpi-audit/scm/early-ssdt/README.md). Совместный initramfs ещё не подготовлен. Модули с другим kernel release несовместимы с этой сборкой.

Не решены [IORT input IDs и ACPI USB](../../acpi-audit/iort-mappings/README.md). Успешная компиляция не подтверждает работу DMA, USB, дисплея или сети при ACPI-загрузке.
