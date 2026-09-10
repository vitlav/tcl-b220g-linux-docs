# TCL: ACPI против рабочего DT перед ACPI1

Сверка 2026-09-09. База: сохранённые OEM таблицы Windows, текущий DT audio2-va и живые проверки Linux 6.18.34-tcl-audio2. Это карта подтверждений и пробелов, не заявление о равнофункциональном ACPI-порте.

| Подсистема | OEM ACPI | Рабочий DT / опыт | Вывод для ACPI1 |
|---|---|---|---|
| CPU | MADT MPIDR 0x000..0x700 | Восемь CPU работают | Топология адресов совпадает; проверить online в ACPI |
| PSCI | FADT ARM flags=1, SMC | psci-1.0, method=smc | Способ вызова совпадает |
| FADT | Rev5.0, длина276, HW_REDUCED/LOW_POWER_S0 | DT не использует FADT | Код Linux допускает с warning assuming5.1; заранее не исправляем |
| GICv3 | GICD17a00000; GICR17a60000/1MiB | Те же адреса и размер GICR | Основные адреса совпадают |
| Timer | INTID17/18/19/16, level high | PPI1/2/3/0 = INTID17/18/19/16, level low; живой arch_timer INTID19 | Номера совпадают; полярность различается. Проверить продвижение времени/IRQ, OEM пока без override |
| KVM maintenance | MADT IRQ24 | В mainline DT ранее отмечался25; в текущем flattened DT GIC maintenance IRQ не задан | Не объявлять проверенным ресурсом. Сейчас KVM=y, но HYP mode not available и /dev/kvm нет |
| Память | EFI memory map + ACPI tables | EFI handoff + DT reserved-memory для DSP/модема | Динамические EFI allocation не превращать в постоянный DT; carveouts перед remoteproc потребуют отдельной сверки |
| Ранний экран | EFI GOP, отдельный от ACPI display driver | efifb/earlycon ранее работали, затем native DRM | ACPI1 использует явный earlycon=efifb; наличие картинки надо проверить загрузкой |
| UART/SPCR | SPCR a90000/IRQ386; DBG2 UART a88000 | a90000 — действующий I2C10 дисплея | acpi=nospcr; без bare earlycon, без записи в подозрительный UART |
| I2C EC | I2C3 888000, INTID635, 0x07/400kHz | Linux i2c2, DT SPI603+32 | MMIO/IRQ совпадают; GENI QCOM0811 match пока отсутствует |
| I2C HID/SAR | I2C5 890000, INTID637 | Linux i2c4, DT SPI605+32, устройства05/2c/12 | Нумерация ACPI отличается от Linux; ресурсы совпадают |
| I2C display | IC11 a90000, INTID389, bridge0x29 | Linux i2c10, SPI357+32, LT8911 | Совпадает с рабочим контроллером |
| TLMM | GIO0 QCOM080D, GPIO OpRegion | pinctrl-sc7180 через OF; известные ограничения GPIO | Нужен ACPI attachment и корректные ресурсы/IRQ/OpRegion; не включён по одному ID |
| SAR | SAR1 AWDZ9610X, I2C0x12/GPIO34, DSM AW96105 | CHIPID a9610b00; IIO уже работает с OEM892байта/138пар | Данные ACPI практически подтверждены; источник vcc ещё неизвестен |
| Клавиатура/тачпад | I2C HID описания | Работают на i2c4 | I2C_HID_ACPI включён в ACPI1, но без адаптера ввод не обещаем |
| USB | URS0 a600000, дочерний USB0 IRQ165/162 и wake IRQ | DWC3 SPI133+32=165, power130+32=162 | Основные IRQ совпадают; glue требует объединить ресурсы родителя/ребёнка, PEP/PHY пока отсутствуют |
| IORT USB | SID540, путь _SB.USB0 | Реальный namespace _SB.URS0.USB0 | Несовпадение пути, DMA attachment надо исправить/проверить; обход SMMU не включали |
| SMMU | 15000000/1MiB,80 context IRQ | Работает в DT | Не считать совпадение базы подтверждением всех SID/потоков |
| UFS/ICE | IORT SIDa0; PEP питание/clocks сохранены | UFS работает; возможности ICE исследованы отдельно | ACPI1 без UFS-драйвера, чтобы тест не зависел от внутреннего накопителя |
| Wi-Fi/MPSS | OEM modem/PEP/firmware ресурсы | Проверены MPSS NM, carveouts, RMTFS/TFTP, DHCP/SSH | ACPI remoteproc+питание ещё не реализованы; сеть в ACPI1 не обещаем |
| CPUFreq/питание | OEM PEP | qcom cpufreq-hw и DT ресурсы | Стандартный Linux-интерфейс нужен, но одного ACPI_ENABLE мало |
| Батарея | BAT0/ADP1, GSBus I2C3/0x07, _BIF/_BST | EC читается адресными запросами | ACPI_BATTERY/AC/OpRegion включены; нужен GENI и уведомления, см.19490 |
| Native GPU/display | OEM Qualcomm/LT8911 ресурсы и PEP | DRM/MSM/Adreno/Wayland работают в DT | ACPI путь не реализован; EFI framebuffer — только диагностика |
| Звук | OEM WCD9385/LPASS/PEP, PA GPIO46/47 | Реально работает ALSA/PulseAudio/Firefox | DT fixes/machine topology и управление питанием ещё нужно перенести; не регрессия тестового RAM-userspace |

Основные исходники и разборы: ../../acpi-audit/{boot-readiness.md,preflight-usb-geni.md,table-validation.json}; ../../acpi-migration-task.md; ../../peripherals/sensors/README.md.

Первый опыт проверяет: OEM tables → ACPICA/MADT/GTDT → CPU/time/console → автономный userspace → перечень устройств и доступный USB. Это даст фактический список ошибок вместо дальнейших предположений.
