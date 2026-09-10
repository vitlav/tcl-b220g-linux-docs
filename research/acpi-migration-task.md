## Цель

Перевести Linux на TCL Book 14 Go B220G (Snapdragon 7c / SC7180) на загрузку и описание платформы через OEM ACPI, сохранив достигнутую функциональность DT-варианта. Подзадача #19084. Связанная задача телеметрии EC/батареи: #19490; не блокировать её реализацию для DT ожиданием полного ACPI-порта.

Нужен воспроизводимый ACPI-вариант, а не только достижение shell. Рабочий DT-вариант сохранять для сравнения и восстановления.

## Что установлено на 2026-09-09

- Штатная Windows на этом экземпляре использует ACPI и OEM-драйверы Qualcomm. Сохранены таблицы, PnP/ресурсы и драйверы, включая qcpep7180.sys. ACPI принципиально пригоден; невозможность ACPI-загрузки Linux не доказана.
- Текущий проверенный Linux работает через DT; ядро 6.18.34-tcl-audio2, boot c781c4d4-cd36-4a07-b734-40b059ac3e52. В исходной конфигурации этой сборки **# CONFIG_ACPI is not set**. Одного acpi=force для этого ядра недостаточно.
- Работают USB-root Ubuntu, клавиатура/тачпад, автоматические Wi-Fi/DHCP/SSH, доступ к UFS, CPUFreq, native DRM/Adreno618/Wayland. Телеметрия батареи читается скриптом EC, стандартного power_supply пока нет. Поддержка звука не завершена; не считать её регрессией будущего ACPI-варианта.
- UEFI продолжает использоваться при DT-загрузке. DT и ACPI — способы описания аппаратуры, UEFI — отдельная часть загрузки. Для ARM64 Linux обычно выбирает один способ описания при старте; нельзя просто включить ACPI-батарею поверх всей текущей DT-платформы и считать это стандартным смешанным режимом.
- Qualcomm PEP в Windows обрабатывает вендорские пакеты ресурсов: питание, clocks, GPIO, interconnect, последовательности D0/D3. Наличие _CRS и стандартного Windows-драйвера устройства не доказывает достаточность этих данных для Linux.

## Конкретные ограничения драйверов, проверенные по локальному коду

В /tmp/tcl-audio2-linux (исходники текущей сборки):
- drivers/i2c/busses/i2c-qcom-geni.c уже имеет ACPI-путь, IDs QCOM0220/QCOM0411 и обработку has_acpi_companion. В DSDT TCL I2C3/I2C5 имеют **QCOM0811**, _DEP на PEP0. Этого ID в таблице GENI нет. Проверить совместимость регистров/ресурсов, DMA, тактирования и питания; добавление одного ID без проверки не считать решением.
- drivers/pinctrl/qcom/pinctrl-sc7180.c использует OF match, ACPI match в нём не найден. GIO0 имеет **QCOM080D**, ACPI GeneralPurposeIo OperationRegion. Нужен аудит GPIO IRQ, OpRegion и pin configuration через ACPI.
- Отдельно проверить ACPI match/fwnode assumptions и ресурсы всех остальных драйверов: clocks, RPMh/PMIC, power domains/interconnect, SMMU, USB/PHY, UFS/PHY/ICE, remoteproc/Wi-Fi, display/GPU, CPUFreq, аудио. Их отсутствие не утверждается без аудита.

## Таблицы и конкретные несогласованности

Сняты через штатные Windows EnumSystemFirmwareTables/GetSystemFirmwareTable; MSDM с ключом Windows исключена. DSDT около189КБ AML /1.9МБ декомпилированного текста: это не «пустая таблица».

| Объект | Проверенные сведения |
|---|---|
| APIC/MADT | CPU MPIDR0x000..0x700; GICv3 distributor0x17a00000, redistributor0x17a60000/0x100000 |
| FADT/FACP | PSCI via SMC; остальные требования ARM64 ACPI, включая HW-reduced режим, перепроверить перед тестом |
| GTDT | Timer INTID17/18/19/16; polarity high отличается от mainline DT low |
| MADT maintenance | IRQ24 отличается от mainline DT25; ранее не использовали для тестов без KVM |
| IORT | SMMU0x15000000/0x100000, node offset0x30, 80 context interrupts |
| UFS IORT | output SID0xa0; Input base0x81030000 не является SID |
| USB IORT | output SID0x540; путь _SB.USB0 в IORT не совпадает с вложенным _SB.URS0.USB0 в Windows/DSDT |
| DBG2 | UART _SB.UARD0xa88000 |
| SPCR | serial0xa90000, IRQ0x182, baud code7; адрес занят IC11/i2c10 дисплея, IRQ совпадает с I2C8. Автоматический earlycon из SPCR ненадёжен |
| Ошибки декодера | Unknown Width Encoding0x20 в DBG2/SPCR; предупреждение iasl о коротком SPCR не считать само по себе доказательством повреждения firmware |

BootServices/Loader allocations из EFI memory map меняются между загрузками. Не превращать весь снимок памяти GRUB в постоянные резервации. Проверить EFI runtime/virtual mapping отдельно; старые диагностические efi=noruntime/novamap не переносить автоматически в итоговую конфигурацию.

## Карта устройств для начала аудита

| ACPI | MMIO | IRQ global / DT SPI | Назначение |
|---|---|---|---|
| I2C3 | 0x888000/0x4000 |635/603 | EC0x07,400кГц; Linux i2c2 |
| I2C5 | 0x890000/0x4000 |637/605 | HID; Linux i2c4 |
| I2C8 | 0xa84000/0x4000 |386/354 | Linux i2c7 |
| IC11 | 0xa90000/0x4000 |389/357 | мост дисплея0x29; Linux i2c10 |
| UAR4 | 0x88c000/0x4000 |636/604 | Linux uart3; не путать с DBG2 UART |
| UFS0 | 0x1d84000/0x14000 |297/265 | ACPI диапазон включает больше host0x3000; учитывать PHY/ICE |

- Клавиатура ECKB/QTEC0001: I2C0x05, HID descriptor0x20, GPIO33 level-low/pull-up; _PS0/_PS3 меняют GIO0.LISP (GPIO32).
- Тачпад TCPD/QTEC0002: I2C0x2c, descriptor0x20, GPIO94 level-low; PEP TPXC содержит GPIO25.
- EDP1/EDPBridge: I2C0x29, GPIO23/20/51/26/13/21 в порядке _CRS. Сохранены OEM-разбор последовательностей и рабочий Linux display handoff. EDID1920x1080 около60Гц; старые1366x768 не относятся к нашему экземпляру.
- UFS PEP BPCC: L19A2960000uV, L12A1800000uV, L3C1200000uV, L4A880000uV; AXI200МГц, UniPro150МГц, ICE300МГц, GDSC и bus votes. D0: interconnect→GDSC→regulators→delay→clocks. Не считать названия Linux supply или единицы PEP DELAY доказанными только по пакету.
- Wi-Fi зависит от MPSS/firmware и RemoteFS/RMTFS. Сохранены рабочие DT/carveouts/сервисные настройки; переход не должен терять эти зависимости. Личные modem storage и ключи не прикладывать открыто.

## Батарея и EC: возможная ранняя польза ACPI

DSDT _SB.I2C3 содержит BAT0/PNP0C0A, ADP1/ACPI0003, _BIF/_BST и GenericSerialBus OpRegion. Зависимости PEP0/GIO0/I2C3. EC читает32байта с offset0 по адресу0x07; в ACPI ECRB ещё2байта служебного заголовка.

Протокол, единицы и фактический вывод описаны в #19490. Зарядка в DT Linux подтверждена ростом77→78%; текущий пример63%,3402мА·ч,7763мВ. ACPI-путь потенциально позволит использовать стандартный control-method battery driver, но для этого должны работать I2C, OpRegion, зависимости и notifications. B1CR raw и формула _BST требуют проверки единиц; не обещать корректный current_now автоматически.

## Аналоги и источники

- Windows на нашем TCL — непосредственно подтверждённый пример ACPI с OEM PEP.
- OpenBSD ThinkPad X13s/SC8280XP: qcgpio/qciic и acpipci для NVMe: https://www.openbsd.org/plus72.html
- OpenBSD qcgpio умеет ACPI attachment: https://man.openbsd.org/qcgpio.4
- OpenBSD7.6: ACPI PCIO для Snapdragon X Elite, Asus Vivobook S15/Lenovo Yoga Slim7x: https://www.openbsd.org/76.html
- Практический отчёт разработчика OpenBSD, март2024: https://www.mail-archive.com/arm@openbsd.org/msg02847.html . Ограничения письма относятся к2024году, не текущему релизу.
- Linux ARM64 ACPI/DT: https://www.kernel.org/doc/html/latest/arch/arm64/arm-acpi.html
- ACPI enumeration: https://cdn.kernel.org/doc/html/latest/firmware-guide/acpi/enumeration.html
- Windows PEP API: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/pep_x/

Исходники OpenBSD полезны для изучения платформенного ACPI-пути, но не являются готовыми Linux-патчами. Подтверждённого равнофункционального Linux ACPI-порта именно TCL пока не найдено.

## План реализации

1. Сохранить точный рабочий baseline: kernel/config/modules, GRUB, DTB/initramfs, cmdline, boot logs, периферия, частоты/питание. Составить матрицу ACPI IDs→Linux drivers→необходимые ресурсы→пробелы.
2. Аудит ACPI tables, PEP semantics и драйверов без перезагрузки. Отдельно исследовать подход OpenBSD к Qualcomm power/GPIO/I2C.
3. Подготовить kernel с ACPI и нужными драйверами; исходники/патчи/.config/манифест сохранить. Проверить поддержку UEFI handoff, ACPICA OpRegion и зависимостей. Не смешивать поддержку декодирования AML с поддержкой всей платы.
4. Отдельный GRUB ACPI test с прежним DT fallback. Начать с USB/RAM-root и сохранения логов; не доверять SPCR earlycon. Перезагрузка только после согласования пользователя. До подтверждения не делать экспериментальный ACPI пункт единственным/основным.
5. Проверять по этапам: CPU/timer/GIC/EFI→USB/storage root→GPIO/I2C/input→сеть/SSH→EC battery→UFS→CPUFreq/thermal/idle→DRM/GPU/Wayland→остальные устройства. Логи и таблица сравнения обязательны.
6. Для каждого пробела отдельная связанная бага. ACPI table override допускается как диагностический инструмент с явным перечнем изменений, не как скрытая замена OEM firmware.
7. Проверить несколько загрузок и suspend/resume, батарею/питание и отсутствие регрессий. Только после этого согласовать переход по умолчанию.

## Ограничения и критерии готовности

- Не писать во внутренние Windows-разделы; проверенный архив Windows UFS находится /var/ftp/tmp/lav/tcl/windows-ufs-20260908/.
- GPIO58–62 ранее вызывали зависание при обходе TLMM; reserved-ranges устранил его. ACPI OpRegion тоже может обратиться к этим pin: не обходить ограничение вслепую.
- Сохранить рабочий экран/SSH и сбор логов; UART пока отсутствует. Пользователя привлекать для необходимой перезагрузки.
- Результат: подтверждённая ACPI enumeration, доступ после загрузки, функциональный паритет с зафиксированным DT baseline либо явные связанные блокеры. Каждое необходимое исправление таблиц/ядра документировано. acpi=force без рабочего оборудования не является завершением.

## Где все материалы

Репозиторий etersoft-admin-essential, .claude/docs/tcl-b220g-data/:
DSDT.dat/.dsl, FACP-1, APIC-2, GTDT-5, PPTT-4, MCFG-0, CSRT-3, DBG2-8, IORT-9, SPCR-10; acpi.ps1; details.json; inventory и driver-details; windows-drivers/OEM и системные драйверы; peripherals/ с журналами и рабочими конфигурациями.

Общий журнал .claude/docs/tcl-b220g-linux.md; индекс .claude/memory/reference_tcl_b220g_linux.md. Серверный комплект /var/ftp/tmp/lav/tcl/. Исторические README описывают состояние на дату записи: поздние проверенные результаты имеют приоритет над ранними гипотезами.


## 2026-09-09 — подзадача перехода на ACPI создана

https://bugs.etersoft.ru/show_bug.cgi?id=19492 — TCL B220G: перевести Linux на ACPI с сохранением функциональности DT-загрузки. blocks=[19084] применено; задача батареи19490 связана текстом, искусственная зависимость между ними не добавлялась. Создание approved08523be0-6232-49cf-ac1c-71f701932042, связь approved63b65480-c388-49d4-b08b-6473581aeccf, комментарий181238/work_time5 approved78ec3896-ef47-498f-8d7f-46e546cbe369.

Полное задание: `.claude/docs/tcl-b220g-data/acpi-migration-task.md`. Включены известные ACPI ресурсы, PEP, IORT/SPCR/GTDT расхождения, EC, OEM и OpenBSD источники, план/критерии/восстановление. Дополнительно по коду текущего ядра подтверждены CONFIG_ACPI=n; GENI имеет ACPI IDs QCOM0220/QCOM0411, но TCL I2C3/I2C5=QCOM0811 с _DEP PEP0; pinctrl-sc7180.c OF-only, OEM GIO0=QCOM080D. Добавление ID без проверки ресурсов/питания не считается исправлением. Рабочую систему не меняли, не перезагружали.
