## ACPI: дополнительный аудит USB и GENI I²C перед первым запуском

Проверено по локальному Linux 6.18.34-tcl-audio2 и сохранённой DSDT. Аппаратной ACPI-загрузки не было; следующие выводы относятся к коду и описанию ресурсов.

### USB

| Проверка | Результат и необходимая работа |
|---|---|
| URS0 | QCOM0897 / PNP0CA1, MMIO 0x0a600000 длиной 0xfffff, IRQ в самом узле нет |
| USB0, дочерний _ADR=0 | IRQ 0xa5=165, 0xa2=162, 0x206=518, 0x208=520, 0x209=521 |
| Сравнение рабочего DT | Основной DWC3 SPI 0x85=133 → GIC INTID165, совпадает с первым OEM IRQ. Power event SPI130 → INTID162, совпадает со вторым. Wake IRQ не назначать по позиции без отдельной сверки |
| Linux dwc3_host_get_irq | Ищет host/dwc_usb3, затем IRQ0 у platform_device самого DWC3. Автоматического чтения _CRS дочернего USB0 в этом пути нет |
| Возможный glue для теста | Должен получить MMIO родителя и подходящие IRQ ребёнка, правильно передать ACPI companion/DMA/IORT и host role. Одного добавления QCOM0897 в match table недостаточно |
| QSCRATCH | Новый dwc3-qcom вычисляет его от MMIO; legacy DT имеет отдельный узел 0xa6f8800. Нельзя механически скопировать структуру DT-ресурсов в ACPI |
| PHY/clocks/reset/ICC | Рабочий DT явно задаёт их. Отсутствие поставщиков в ACPI не означает, что ресурсы не нужны: опциональный clk/reset getter может вернуть пустой набор. Нужна проверка состояния, оставленного UEFI, и план питания |
| UCS0 | QCOM08A9, GPIO24; URS0 имеет _DEP на UCS0 и PEP0. В просмотренном drivers/ нет QCOM08A9/QCOM0897/PNP0CA1. В acpi_honor_dep_ids нет QCOM08A9, PNP0D80 уже игнорируется. Не объявлять этот _DEP доказанным вечным deferred probe |
| DMA/IORT | xhci-plat выбирает ближайшего родителя с firmware node для sysdev; ранее найденное несовпадение пути IORT _SB.USB0 и реального _SB.URS0.USB0 остаётся существенным. Не обходить SMMU вслепую |

Первый минимальный ACPI initramfs должен оставаться независимым от USB. USB-root и сохранение логов можно обещать только после фактического host probe и чтения флешки.

### GENI I²C

1. При ACPI отсутствие se clock допускается. clk_map_idx выбирает таблицу32MHz только при точном совпадении, иначе19.2MHz. Нужно проверить фактическую исходную частоту, а не считать default правильным.
2. clock-frequency читается через device_property_read_u32; при отсутствии default100kHz. Частота400kHz в I2cSerialBusV2 устройства сама по себе не доказывает настройку адаптера в400kHz.
3. Probe сначала читает protocol. Если GENI_SE_INVALID_PROTO, вызывает geni_load_se_firmware. Там используется se->wrapper->dev без NULL-проверки. Поэтому прежний вывод о FIFO fallback при wrapper=NULL применим лишь к DMA-пути уже настроенного SE, а не ко всем веткам probe.
4. При FIFO_IF_DISABLE требуется GPI DMA; без корректных DMA-ресурсов probe не сможет просто перейти на FIFO. Принудительное no_dma_support не является доказательством работоспособного FIFO железа.
5. При включённом FIFO и уже загруженном I²C protocol существующий SE DMA fallback остаётся полезным: rx_dma_prep возвращает ошибку при wrapper=NULL и драйвер выбирает FIFO.
6. До записи регистров в ACPI тесте нужны ограниченные диагностические чтения protocol/FIFO-depth/FIFO_IF_DISABLE, проверка parent drvdata и IRQ. Не запускать firmware loader с непроверенным wrapper.

### Приоритет первого эксперимента

- Сохранённый DT fallback, отдельное ACPI-ядро/initramfs, acpi=force и отдельный acpi=nospcr.
- Подтвердить CPU/timer/GIC и реально доступный канал диагностики.
- Проверить USB resource glue и DMA/IORT; затем I²C с ограниченными ожиданиями, GPIO и ввод.
- Полноценное управление PEP/PHY/power и suspend/resume считать отдельными незавершёнными этапами; успешный запуск на состоянии UEFI не докажет их поддержку.

### Исходники

drivers/usb/dwc3/{dwc3-qcom.c,host.c,core.c}; drivers/usb/host/xhci-plat.c; drivers/i2c/busses/i2c-qcom-geni.c; drivers/soc/qcom/qcom-geni-se.c; drivers/acpi/scan.c. DSDT URS0 около45249, UCS0 около45849.

Дополнение к существующим комментариям181236/181239/181247/181334, не замена прежнему аудиту. Патчи ACPI и новая загрузка в этой сессии не выполнялись.
