## 2026-09-09 — базовые ACPI-таблицы, USB и конфигурация первого теста

### Проверки таблиц
Для FACP, APIC, GTDT, IORT, SPCR, DBG2 и DSDT длина файла совпала с полем length, сумма байтов modulo256=0. Машиночитаемый результат: acpi-audit/table-validation.json. Корректная checksum не доказывает корректность ресурсов.

FADT: revision5/minor0, length276, flags0x300000 (HW_REDUCED и LOW_POWER_S0), arm_boot_flags1 (PSCI SMC).
arch/arm64/kernel/acpi.c:acpi_fadt_sanity_check в текущем Linux принимает старую revision при ненулевых ARM boot flags с предупреждением «assuming5.1». Следовательно, версия5.0 здесь сама по себе не требует патча FADT. Это статический вывод, а не успешная ACPI-загрузка.

ARM64 Kconfig автоматически выбирает ACPI_SPCR_TABLE. Для первого теста не полагаться на отключение этого символа: использовать acpi=nospcr и не задавать bare earlycon, иначе ошибочный OEM SPCR может затронуть адрес I²C дисплея. acpi=force и acpi=nospcr — отдельные параметры; parser не разбирает строку force,nospcr.

### USB — ограничение загрузки и сохранения логов
OEM URS0 имеет HID QCOM0897/CID PNP0CA1, MMIO0xa600000/0xfffff; IRQ находятся в дочернем USB0 (_ADR0), не в URS0. Linux source drivers/ не содержит этих USB IDs; dwc3-qcom и legacy имеют OF match, core DWC3 ACPI match содержит Intel808622B7; generic xhci ACPI match не совпадает с TCL. Поэтому наличие CONFIG_USB_XHCI_PLATFORM=y не гарантирует доступ к флешке через ACPI.

OpenBSD xhci_acpi.c на том же pinned commit содержит SC7180 USB QCOM0826 и несколько Qualcomm URS IDs, но НЕ QCOM0897. Для известных URS-контроллеров берёт IRQ из дочернего USB-узла. Это полезная схема для сравнения, не готовое соответствие TCL. Также есть отключение конкретного USB2 на ThinkPad X13s из-за reset при attach: не переносить эту аппаратную особенность на TCL без доказательства.

Источник: https://github.com/openbsd/src/blob/3b0fc0544407d328d2ed444734fe0a20422c1db0/sys/dev/acpi/xhci_acpi.c

### Подготовленная конфигурация
Отдельная acpi-audit/acpi.config получена из конфигурации6.18.34-tcl-audio2:
CONFIG_ACPI=y, ACPI_I2C_OPREGION=y, ACPI_AC/BATTERY/BUTTON=y, ACPI_DEBUG=y, ACPI_TABLE_UPGRADE=y.
Проверка make olddefconfig завершилась успешно с KCONFIG_CONFIG=/tmp/tcl-acpi-audit/acpi.config. Производные GPIO_ACPI, ACPI_IORT/GTDT/PPTT и HW_REDUCED_ONLY включились. Diff сохранён. Исходная /tmp/tcl-audio2-linux/.config по-прежнему CONFIG_ACPI=n. Это только проверка Kconfig, ядро с новой конфигурацией НЕ собрано и НЕ загружено.

### Уточнение первого теста
Минимальный userspace должен целиком находиться в initramfs и не зависеть от USB-root. Ранний экран/GOP необходимо проверить отдельно; клавиатуру/Wi-Fi также нельзя считать уже доступными при ACPI. Нельзя обещать сохранение логов на флешку, пока USB не заработал; RAM-логи пропадут после перезагрузки, pstore/EFI variable storage пока не проверены. До загрузочного эксперимента подготовить реально проверяемый способ наблюдения и запасной DT-пункт.

Рабочая система/GRUB/DT не менялись, перезагрузки не было. Предыдущий отчёт аудита опубликован в этой баге: comment181239.
