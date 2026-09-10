## 2026-09-09 — аудит ACPI I²C/GPIO/PEP без перезагрузки

### Подтверждено по исходникам

| Узел | Linux 6.18.34-tcl-audio2 | OpenBSD / вывод |
|---|---|---|
| GENI I²C | ACPI IDs QCOM0220/QCOM0411; OEM TCL QCOM0811 отсутствует | qciic.c содержит именно QCOM0811, регистрирует GenericSerialBus handler |
| Питание GENI | geni_se_resources_on/off и geni_icc_get возвращают0 при ACPI companion; DT clocks/pinctrl/ICC не выполняются | В просмотренном qciic attach нет эквивалента полного Qualcomm PEP; рабочее исходное состояние оборудования остаётся условием |
| PEP _DEP | DSDT PEP0 HID QCOM0819, CID PNP0D80. Linux scan.c игнорирует PNP0D80; acpi_info_matches_ids проверяет и CID | _DEP на PEP0 само по себе НЕ означает вечный deferred probe. Обход зависимости не реализует управление питанием |
| SC7180 GPIO | OEM GIO0 QCOM080D; pinctrl-sc7180 OF-only | OpenBSD qcgpio принимает QCOM080D,119pins, но поддерживает только ограниченную карту |
| Карта OpenBSD SC7180 | Не готовый шаблон всех pin | GPIO30 SOUTH,33 NORTH (также alias0x180),94 SOUTH (alias0x1c0). GPIO32/alias0x140 выключены #if0 из-за interrupt storm; default возвращает-1 |
| Audio GPIO58 | Зарезервирован в рабочем Linux DT; ранее исключение58–62 устранило boot hang | OpenBSD mapping58 возвращает-1, write_pin выходит без записи. Это не доказательство безопасного доступа и не готовое исправление звука |

### Практические последствия

1. Не добавлять обход _DEP PEP0 вслепую: Linux уже игнорирует этот CID.
2. Добавление QCOM0811 — возможный минимальный диагностический шаг, но предварительно нужны проверка питания/clock и аудит FIFO/DMA. При ACPI нет прежней DT-гарантии ресурсов.
3. Linux уже имеет i2c_acpi_install_space_handler под CONFIG_ACPI_I2C_OPREGION; battery OpRegion не требует заново писать весь GenericSerialBus механизм. Нужны совместимый адаптер и рабочие зависимости.
4. OpenBSD qciic использует FIFO/polling с ограниченными ожиданиями. В Linux GENI есть fallback FIFO при невозможности подготовки SE DMA; его достижимость нужно проверить, особенно без wrapper у ACPI parent.
5. OpenBSD специально исключает GenericSerialBus registration для QCOM0610 из-за неудачных AML I²C-транзакций с удержанием kernel lock; QCOM0811 не исключён. Это предупреждение о полноте реализации, не установленная проблема TCL.
6. Изученный OpenBSD код не обеспечивает полный PEP-порт или все GPIO SC7180. Его нельзя выдавать за готовое решение для ACPI TCL.

### Источники и воспроизводимость

OpenBSD commit3b0fc0544407d328d2ed444734fe0a20422c1db0, получен через gh:
- https://github.com/openbsd/src/blob/3b0fc0544407d328d2ed444734fe0a20422c1db0/sys/dev/acpi/qciic.c
- https://github.com/openbsd/src/blob/3b0fc0544407d328d2ed444734fe0a20422c1db0/sys/dev/acpi/qcgpio.c

Локальный Linux: drivers/i2c/busses/i2c-qcom-geni.c; drivers/soc/qcom/qcom-geni-se.c; drivers/acpi/scan.c (acpi_info_matches_ids/acpi_ignore_dep_ids/acpi_scan_dep_init); drivers/i2c/i2c-core-acpi.c; drivers/pinctrl/qcom/pinctrl-sc7180.c.
OEM DSDT: PEP0 около799, I2C3 около32034, GIO0 около42997.

Копии OpenBSD и SHA256 лежат рядом с этим отчётом в acpi-audit/. Никаких модификаций ядра/DT/GRUB, транзакций EC/GPIO или перезагрузки при этом аудите не было.

Следующий этап: завершить FIFO/DMA и ресурсный аудит GENI, подготовить минимальную конфигурацию ACPI-ядра и тестовый маршрут сбора логов без опоры на ошибочный SPCR. Рабочую DT-загрузку сохранить.


### Дополнение: GENI FIFO fallback

В текущем Linux geni_se_tx/rx_dma_prep возвращает -EINVAL при wrapper=NULL. geni_i2c_rx_one_msg/tx_one_msg при неуспехе подготовки переключаются на FIFO. Поэтому отсутствие wrapper само по себе не доказывает невозможность I²C ACPI; нужно подтвердить реальный parent drvdata, FIFO_IF_DISABLE и прохождение транзакции. Для чтения32байта уже достигается порог попытки DMA (i2c_get_dma_safe_msg_buf(msg,32)), если получен буфер. Путь FIFO fallback подтверждён по коду, на ACPI-загрузке не проверен.

Отчёт в #19492 с work_time5 отправлен один раз; pending Lavtomate confirmation6ea164f8-91d2-4c28-a92a-f75f8c03997e. Не повторять отправку.
