## Задача

Реализовать или адаптировать Linux-драйвер телеметрии EC/батареи TCL Book 14 Go B220G (Snapdragon 7c / SC7180) для загрузки через Device Tree. Связать как блокирующую подзадачу основной баги #19084.

## Проблема и проверенное состояние — 2026-09-09

Ubuntu ARM64, ядро 6.18.34-tcl-audio2, boot c781c4d4-cd36-4a07-b734-40b059ac3e52. /sys/class/power_supply пуст, /proc/acpi/battery отсутствует. I²C и EC работают: установленная команда читает заряд напрямую.

```sh
sudo /usr/local/sbin/battery-status
sudo watch -n 30 /usr/local/sbin/battery-status
```

Фактический вывод на uptime12507.75:
```text
ac=0 state=discharging percent_field=63 remaining_mAh=3402 voltage_mV=7763 current_raw=569
raw=0x00 0x00 0x02 0x01 0x00 0x00 0x18 0x15 0x14 0x1e 0x18 0x15 0x00 0x00 0x01 0x39 0x02 0x4a 0x0d 0x53 0x1e 0x3f 0x00 0x07 0x00 0xaa 0x00 0x00 0x00 0x00 0x00 0x00
```

Зарядка в Linux ранее подтверждена ростом ёмкости 4158→4212 мА·ч и процента 77→78 после подключения адаптера. Реализация мониторинга не должна подменять уже работающую автономную зарядку EC.

## Транспорт и протокол

OEM DSDT: \\_SB.I2C3, EC адрес 0x07, 7-bit, 400кГц; Linux контроллер 0x888000, в проверенной загрузке i2c-2. Не привязывать драйвер к динамическому номеру адаптера.

Проверенное чтение: запись указателя 0x00, затем 32 байта чтения одной I²C-транзакцией:
```sh
timeout 5 i2ctransfer -a -y 2 w1@0x07 0x00 r32
```
Это не запись настроек EC. Выполнять диагностическую команду только при отсутствии связанного драйвера адреса 2-0007, без -f; battery-status уже проверяет путь шины и занятость.

Смещения ниже относятся к сырым 32 байтам. В ECRB из ACPI перед данными есть двухбайтовый заголовок GenericSerialBus, поэтому там смещения на 2 больше.

| Смещение | Поле | Интерпретация |
|---|---|---|
| 1 | ELID | состояние крышки, отдельная валидация |
| 2 | ECWR | bit0: адаптер подключён |
| 6–7 | B1DC | проектная ёмкость, LE16 |
| 8–9 | B1FV | номинальное напряжение, LE16 |
| 10–11 | B1FC | полная ёмкость, LE16 |
| 14 | B1ST | биты0–2: ACPI battery state |
| 15–16 | B1CR | сырое значение скорости/тока, единицы требуют проверки |
| 17–18 | B1RC | оставшаяся ёмкость, LE16 |
| 19–20 | B1VT | напряжение, LE16 |
| 21 | BPCN | процент, согласуется с B1RC/B1DC в измерениях |

DSDT _BST вычисляет present rate как B1CR*B1FV/10000. Не объявлять current_raw миллиамперами и не переносить формулу в current_now без проверки единиц, знака и крайних значений. _BIF также требует критического сравнения: OEM записывает B1DC и в design, и в last-full поле.

## Что реализовать и проверить

1. Проверить существующие EC/power_supply драйверы на совместимость; модель EC и универсальность протокола пока не установлены.
2. Добавить описание устройства в DT и binding; реализовать минимальный I²C-драйвер чтения с таймаутами, сериализацией и проверкой ответа. MFD нужен только если обосновано разделение функций EC.
3. Зарегистрировать Battery и источник внешнего питания через power_supply: capacity, status, voltage_now, charge_now, charge_full/design и online с правильными единицами ABI. Неподтверждённые свойства не экспортировать как достоверные.
4. Обеспечить обновление/уведомления power_supply_changed; сначала ограниченный polling, IRQ только после проверки источника. Обрабатывать I²C-ошибки и suspend/resume, прекращать опрос при удалении.
5. Проверить разряд, подключение/отключение адаптера, заряд, полный заряд, восстановление после suspend/resume и отображение UPower/рабочим столом. Не доводить ноутбук до глубокого разряда ради теста.
6. Добавить драйвер, DT и нужную конфигурацию в воспроизводимый комплект Linux TCL; после привязки отключить прямой опрос userspace.
7. Не добавлять управление зарядными напряжениями или произвольные записи EC в рамках этой задачи.

## Почему отдельный драйвер для DT

DSDT содержит BAT0 (PNP0C0A), ADP1 (ACPI0003), GenericSerialBus OperationRegion и зависимости PEP0/GIO0/I2C3. Текущая рабочая платформа описана DT; простое включение ACPI не обеспечивает Qualcomm PEP и драйверы всей платы. Полноценная ACPI-загрузка не доказана невозможной, но является отдельной задачей, а не условием реализации телеметрии батареи.

## Исходные материалы

Репозиторий etersoft-admin-essential:
- .claude/docs/tcl-b220g-data/DSDT.dsl (Scope I2C3, BAT0, ADP1);
- .claude/docs/tcl-b220g-data/peripherals/power-storage-video/ (README, decode-ec.py, battery-status, логи);
- .claude/docs/tcl-b220g-data/peripherals/ubuntu-base/battery-status;
- .claude/docs/tcl-b220g-linux.md.

Каталог комплекта: /var/ftp/tmp/lav/tcl/.
ARM64 ACPI/DT: https://www.kernel.org/doc/html/latest/arch/arm64/arm-acpi.html

Критерий готовности: стандартные power_supply/UPower показывают подтверждённые данные без прямого i2ctransfer и не нарушают автономную зарядку EC.

Creation requested; Lavtomate confirmation pending: 3554d500-c12c-41e0-82e2-6458023dbd93. Do not retry bug_create. After approval obtain bug ID and set blocks=[19084], then verify relation. User explicitly authorized this battery bug; previous pause before sound validation still applies to audio reports.

Бага создана: https://bugs.etersoft.ru/show_bug.cgi?id=19490 — TCL B220G: реализовать драйвер EC/батареи с интерфейсом power_supply для Linux DT. Создание подтверждено (3554d500-c12c-41e0-82e2-6458023dbd93); blocks=[19084] применено, результат подтверждён (2a4a16ba-6063-47aa-b11b-56dfe73440c7). Основная задача19084 зависит от19490. Комментарий о связи/подготовке с work_time5 отправлен один раз; pending confirmation634e3a7b-78c4-4332-941f-e5940b65d2aa, не дублировать.
