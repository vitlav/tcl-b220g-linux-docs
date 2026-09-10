# USB ACPI: ресурсы родителя и дочернего контроллера

2026-09-10. Сверены сохранённые OEM DSDT и живой DT; запуск нового USB-драйвера не выполнялся.

| Ресурс | OEM ACPI | Рабочий DT |
|---|---|---|
| Контроллер | URS0 HID QCOM0897 / CID PNP0CA1 | qcom,sc7180-dwc3 + snps,dwc3 |
| MMIO | URS0: 0x0a600000, **длина 0x000fffff** | core 0x0a600000/0xe000; glue 0x0a6f8800/0x400 |
| xHCI IRQ | Первый IRQ дочернего USB0: GSI165, Level/ActiveHigh/Shared | SPI133 +32 =165 |
| Остальные IRQ USB0 | 162,518,520,521; часть wake-capable | power/PHY/PDC IRQ; требуется отдельная сверка маршрутизации |
| DMA | URS0._CCA=0; IORT address limit36, properties0, output SID0x540 | Apps SMMU SID0x540 |
| Зависимости | URS0→PEP0,UCS0; UCS0 QCOM08A9, GPIO24 PullDown | clocks, power domain, reset, ICC, USB2/3 PHY описаны через DT |

Длину OEM MMIO нельзя молча округлять до 1MiB; для размещения контроллера сравнивать реальные окна регистров. Число IRQ само по себе не объясняет роль линии.

Linux6.18.34 Qualcomm DWC3 имеет OF-match; QCOM0897 не сопоставляется. Generic xhci ACPI поддерживает PNP0D10/PNP0D15. IRQ находится в `_CRS` дочернего USB0 с `_ADR=0`, а память у URS0. Добавление одного HID не формирует необходимые ресурсы.

Для DMA нужен firmware parent с подходящим IORT именем. Таблица содержит `\_SB.URS0` и `\_SB.USB0`, а фактический дочерний USB-объект — `\_SB.URS0.USB0`. Нельзя механически переносить companion на дочерний объект и считать IORT lookup решённым. Дополнительно требуется явный input ID0x80030000: см. [аудит mappings](README.md).

`xhci_generic_plat_probe()` ищет ближайший firmware-backed sysdev по родителям, `xhci_plat_probe()` настраивает DMA mask этому sysdev. Будущий glue должен сохранить этот путь и правильную IOMMU-связь. DWC3/PHY reset и xHCI start выполняют аппаратные операции; создание устройства не является read-only проверкой.

Наличие _DEP на UCS0 пока не доказывает блокировку enumeration: Linux учитывает honor_deps, а QCOM08A9 не входит в проверенный acpi_honor_dep_ids; PEP0/PNP0D80 в ignore-list. Это не означает, что питание и переключение USB Type-C можно игнорировать.

Критерий готовности эксперимента: USB-накопитель обнаружен, STARTED/RESULT/COMPLETE реально записаны с sync и прочитаны обратно. До этого новый ACPI2 Image не обеспечивает надёжное сохранение логов. Отдельная задача USB: [#19530](https://bugs.etersoft.ru/19530).
