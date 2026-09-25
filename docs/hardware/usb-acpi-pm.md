# USB ACPI: зависимости питания и пробуждения

Проверено по OEM DSDT, SC7180 DT и работающей ACPI-системе public70
25–26 сентября 2026 года. USB host работает с состоянием PHY и тактов,
оставленным firmware. Полноценное управление питанием USB ещё не реализовано.

## IRQ: не путать индекс ACPI, GSI и PDC pin

| Назначение | Индекс IRQ в URS0.USB0 `_CRS` | GSI / GIC INTID | DT источник |
|---|---:|---:|---|
| xHCI | 0 | 165 | GIC SPI133, level-high |
| Power event | 1 | 162 | GIC SPI130, level-high |
| HS PHY | отсутствует | 163 по DT | GIC SPI131, level-high |
| SuperSpeed wake | 2 | 518 | PDC6, level-high |
| DM wake | 3 | 520 | PDC8, edge |
| DP wake | 4 | 521 | PDC9, edge |

SC7180 PDC range `0 480 94` задаёт родительские **SPI**, поэтому для pin6
GIC INTID равен480+6+32=518. В ACPI DP/DM заданы как Edge/ActiveHigh;
драйвер выбирает полярность по подключённому устройству: LS — DM falling,
HS/FS — DP falling, без устройства — обе линии rising.

Включение GIC IRQ само по себе не настраивает PDC. Текущий ACPI USB glue
не запрашивает эти wake IRQ: все четыре индекса в pdata равны-1.
Нельзя назначать индекс1 на HS PHY: там находится power event.

## Ресурсы, необходимые Linux

| Блок | DT-эталон | Текущее состояние ACPI |
|---|---|---|
| DWC3 / xHCI | core0xa600000, IRQ165, SMMU SID0x540 | Работает; IORT adaptation уже имеется |
| Wrapper QSCRATCH | 0xa6f8800/0x400 | Отображён текущим драйвером |
| Wrapper clocks | GCC17 cfg_noc, GCC109 core, GCC8 iface, GCC113 sleep, GCC111 mock_utmi | `clk_init()` пропускает non-OF; Linux не получает эти clocks |
| Assigned rates | core150MHz, mock_utmi19,2MHz | DT-настройки, не новое измерение ACPI |
| Reset / GDSC | GCC_USB30_PRIM_BCR, USB30_PRIM_GDSC | Полные ACPI reset/genpd зависимости не представлены |
| DDR interconnect | aggre2_noc MASTER_USB3 → mc_virt SLAVE_EBI1 | ACPI ветка пропускает получение путей |
| Register interconnect | gem_noc MASTER_APPSS_PROC → config_noc SLAVE_USB3 | ACPI ветка пропускает получение путей |
| USB2 PHY | 0x88e3000/0x400; GCC119, RPMh CXO; GCC reset0 | Linux PHY device в проверенной системе не зарегистрирован |
| USB3/DP PHY | 0x88e8000/0x3000; GCC115/114/117/118/119; resets6/4 | Linux PHY device в проверенной системе не зарегистрирован |
| PHY supplies / calibration | PMIC regulators и QFPROM trim | Значения приведены в [USB DT](usb-input.md#phy-и-питание); перенос ещё необходим |

Текущий урезанный GCC provider обслуживает дисплей/GPU. Его таблица разрешённых
записей не включает USB-регистры; нельзя считать его полным GCC provider.
Отсутствие USB clock в CCF и USB domain в genpd не означает, что аппаратно
они выключены: работающий host использует firmware handoff.

## Ограничение системного сна

`dwc3_qcom_pm_suspend()` и runtime suspend в `tcl_usb_acpi.c` явно возвращают
`-EBUSY` для ACPI. Это подтверждённый блокирующий фактор s2idle: ядро
указывает `QCOM0897:00`, стадию suspend и errno-16. После отмены перехода
Wi-Fi/SSH восстановились, но успешный сон и пробуждение не подтверждены.
Также обнаружен `dpu_encoder_virt_atomic_disable: timeout pending` при
отменённом переходе; восстановление дисплея требует отдельной проверки.

Для снятия ограничения нужны:

1. Сбалансированное владение clocks/reset/GDSC, PHY, regulators и ICC;
   приём исходного firmware-состояния без неожиданного сброса USB.
2. Правильные PDC wake routes и обработка ошибок IRQ с откатом уже
   включённых линий. Ошибка `irq_set_irq_type()` или `enable_irq_wake()`
   должна отменять переход, а не скрываться.
3. Проверка probe/remove и неудачного suspend без потери устройств;
   затем реальный цикл с возвратом ввода, USB, сети, изображения и звука.

Удаление одного `return -EBUSY` не выполняет этих требований.
Подготовленный патч обработки ошибок IRQ ещё не квалифицирован на оборудовании
и не является частью заявленной рабочей поддержки сна.

## Частоты работающего firmware handoff

На работающем public70 прочитаны только перечисленные регистры через уже
существующий GCC regmap. Записей в регистры не было; диагностический модуль
после чтения выгружен. База GCC — `0x100000`.

| Регистр / источник | Readback | Вывод |
|---|---|---|
| GPLL0 USER_CTL, offset0x0c | 0x103 | even-divider encoding1, делитель2 |
| GPLL0 через CCF | 600000000 Hz | even-выход300MHz |
| USB MASTER CFG, offset0xf020 | 0x105 | source1=GPLL0, HID5: 600MHz × 2/6 = **200MHz** |
| USB MASTER M/N/D, offsets0xf024/28/2c | 0/0/0 | M/N mode в CFG выключен |
| USB MOCK UTMI CFG, offset0xf038 | 0x1 | source0=XO, HID1: **19,2MHz** |
| USB PHY AUX CFG, offset0xf064 | 0x1 | source0=XO, HID1: **19,2MHz** |
| USB GDSCR, offset0xf004 | 0xf8222000 | PWR_ON=1, SW_COLLAPSE=0 |

Частоты декодированы по регистрам и CCF, не измерены осциллографом.
DT `assigned-clock-rates=150000000` — запрос, а не доказательство работы
на150MHz. Таблица частот текущего GCC предлагает66,666667/133,333333/200/240MHz;
перенос управления не должен без необходимости менять подтверждённые200MHz.
