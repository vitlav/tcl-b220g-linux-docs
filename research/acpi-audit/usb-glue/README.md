# ACPI USB: Qualcomm glue для TCL B220G

**USB host проверен на оборудовании** в диагностическом ядре `6.18.34-tcl-acpi6`: DWC3/xHCI, USB2 hub, накопитель, два снимка логов с SHA256 readback. [Подтверждение и ограничения](validation.md), [ресурсы USB](../../../docs/hardware/usb-input.md).

## Проверенная реализация

[Текущий исходник модуля](tcl_usb_acpi.c) соответствует использованному ядру. Он требует сопутствующих SCM/SMMU/IORT изменений; один этот файл не является полной переносимой серией патчей.

- ACPI HID QCOM0897; отдельное имя драйвера tcl-dwc3-acpi, OF-устройства отвергаются.
- Ресурсы памяти берутся у URS0, IRQ — у дочернего USB0; соответствие узлов проверяется через ACPI handles.
- До MMIO проверяются память 0x0a600000/0x000fffff, _CCA=0, IORT SID0x540 и DMA domain.
- IORT назначает input ID0x80030000 до USB probe. DWC3 child использует linux,sysdev_is_parent для DMA через URS0.
- Core window 0xe000, QSCRATCH 0x0a6f8800/0x400; включены пять quirks из рабочего SC7180 DT.
- Wake IRQ не включены; suspend/runtime suspend не поддержаны. Полный cold-start PHY/clocks/power ещё не проверен: используется firmware-состояние.

Сообщение xHCI о поддержке SuperSpeed не доказывает USB3-передачу. Измеренный в DT USB2 throughput не переносится на ACPI без отдельного замера.

## Основа драйвера

[Upstream ACPI URS glue](https://github.com/torvalds/linux/blob/12fc84e8c4288cc8ed5f14a35e077130c2cfece2/drivers/usb/dwc3/dwc3-qcom.c) задаёт схему объединения ресурсов родителя и ребёнка. [Удаление старой поддержки](https://github.com/torvalds/linux/commit/41717b88abf1cacd953e9ea2ace2f62eaf763c48) не является доказательством невозможности ACPI USB.

Для текущего аппаратного результата используйте исходник выше и спецификацию SCM/IORT/SMMU в [docs/acpi.md](../../../docs/acpi.md). Версия проверенного ядра указана в [условиях проверки](validation.md); она не задаёт структуру разделов документации.
