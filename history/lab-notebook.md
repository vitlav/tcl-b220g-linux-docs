# Исторический журнал исследований

**Хронология, не инструкция по установке.** Ранние выводы могут быть исправлены последующими записями. Чувствительные значения заменены. Актуальный вход — README.md.

# TCL B220G: подготовка данных для запуска Linux

Дата исследования: 2026-09-06. Задача: собрать проверенные данные именно этой платы для DTS (исходник Device Tree), из которого компилируется DTB. Linux пока не запускался. Рабочий файл заметок по поручению lav; новые результаты дописывать сюда.

Bugzilla: [Etersoft #19084 — Установка Linux на ARM-ноутбук TCL Book 14 Go (B220G)](https://bugs.etersoft.ru/show_bug.cgi?id=19084). Найдена поиском TCL/B220; NEW, P5, Отдел исследований / оборудование, исполнитель lav@etersoft.ru. Прочитан c#0 от 2026-05-13. Он описывает характеристики из публикации (4/128 ГБ), а не обследованный экземпляр. Пользователь поручил отчитываться в багу по завершённым шагам.

Оформление отчётов: по указанию пользователя писать в Markdown (заголовки, списки, inline code/блоки команд, ссылки). Уже отправленные комментарии автоматически не переписывать.

## Доступ и выполненные действия

- Windows: `ssh etersoft@192.168.8.143`, hostname B220G. Успешный вход возвращает `b220g\etersoft`.
- По явному поручению пользователя установлен публичный ключ `/home/lav/.ssh/id_ed25519.pub`, комментарий lav@etersoft.ru. Пароль в заметках и материалах не сохраняется.
- Учётная запись входит в Administrators. В sshd_config действует `Match Group administrators` с `AuthorizedKeysFile __PROGRAMDATA__/ssh/administrators_authorized_keys`.
- Ключ добавлен в `C:\ProgramData\ssh\administrators_authorized_keys`, существующее содержимое сохранялось (при наличии файла предусмотрена timestamp-копия). ACL проверен: только SYSTEM и Administrators, FullControl; наследование отключено. sshd не перезапускался.
- Проверено: `ssh -i /home/lav/.ssh/id_ed25519 -o IdentitiesOnly=yes -o BatchMode=yes -o ConnectTimeout=10 etersoft@192.168.8.143 whoami`.
- Запущен fastfetch; собраны CIM/PnP, версии драйверов, разделы, состояние Secure Boot/BitLocker/WinRE, UEFI boot entries, EDID, ACPI.
- Считывание ACPI через штатные `EnumSystemFirmwareTables` / `GetSystemFirmwareTable`, без драйвера доступа к физической памяти. MSDM (ключ Windows) исключена.
- Разделы, прошивка, загрузочная конфигурация и состояние сервисов не изменялись. Перезагрузки, сборка ядра и загрузка Linux не выполнялись.
- GitHub читать через `gh` — явное предпочтение пользователя.

## Этот экземпляр

| Параметр | Результат чтения Windows |
|---|---|
| Производитель / модель | TCL Communication Ltd. / B220G |
| Плата | TCL B220G, Version PIO; fastfetch Host B220G (1.1) |
| CPU | Snapdragon 7c @ 2.40 GHz, 8 ядер, ARM64; SC7180 подтверждается ACPI/Hardware IDs |
| RAM | 8 ГБ, ОС доступно 8125050880 байт (7.57 GiB) |
| UFS | SAMSUNG KM8F9001JM-B813; видимый диск 251331084288 байт, GPT |
| GPU | Qualcomm Adreno 618 |
| Windows | Windows 11 Home 25H2 ARM64, fastfetch 10.0.26200.9168 |
| BIOS | Qualcomm Technologies, Inc. 1.21; CIM ReleaseDate 2022-11-09 UTC |
| Панель по EDID | MDT003D, имя 140PM49D3014; preferred timing 1920×1080, около 60 Гц |
| Secure Boot | False (не менялся) |
| BitLocker C: | ProtectionStatus=0, VolumeStatus=0, EncryptionPercentage=0 |
| WinRE | Enabled, partition4, 10.0.26100.9168 |
| Сон | Modern Standby S0, гибернация, Fast Startup; S3 недоступен |

В исходной статье — 4 ГБ RAM, Micron MT128GASAO4U21 128 ГБ, панель 1366×768. Эти параметры нельзя переносить на нашу машину. Указанные fastfetch 1 MiB видеопамяти и Discrete не использовать как аппаратную характеристику Adreno.

Get-Partition показал только четыре раздела видимого Windows диска: ESP 260 MiB, MSR 16 MiB, C: 249991004160 байт, Recovery 1000 MiB. Это не доказывает отсутствие других UFS LUN/служебных областей Qualcomm; чтение Windows GPT не является полной резервной копией устройства.

## Материалы и воспроизводимость

Каталог [tcl-b220g-data/](../research/):

- `inventory.json` — подробная инвентаризация; `details.json` — PnP ACPI paths и EDID.
- `inventory.ps1`, `acpi.ps1`, `details.ps1` — выполненные команды сбора (через PowerShell stdin, без установки на диск Windows).
- `DSDT.dat` — 189125 байт, checksum mod 256 = 0; OEM QCOMM, table SDM7180, revision 3.
- `DSDT.dsl` — результат `iasl -d DSDT.dat`, iasl 20250807. Разбор успешен, один неразрешённый внешний метод `_SB.UCS0.USBR`; его аргументы iasl угадал. До работы с USB/UCSI этот участок требует дополнительной проверки.
- Остальные считанные таблицы: MCFG, FACP, APIC, CSRT, PPTT, GTDT, FPDT, BGRT, DBG2, IORT, SPCR, TPM2; все контрольные суммы корректны. Таблицы SSDT API не перечислило; это не доказательство их отсутствия во всех фазах загрузки.
- DSDT не вошла в результат EnumSystemFirmwareTables, но успешно прочиталась отдельным GetSystemFirmwareTable по сигнатуре DSDT. Нельзя останавливаться на одной enumeration.
- `display-edid.bin` — 256 байт из реестра Windows; checksum основного 128-байтного блока 0.
- `sc7180-tcl-book-14-go.dts` — неизменённое вложение AliceCyber 2023 года; BSD-3-Clause. Это исследовательский образец, не готовый DTS для нашего экземпляра.
- `sc7180-mainline.dtsi`, `phy-qcom-qmp-ufs.c`, `sc7180-ecs-liva-qc710.dts` — материалы из torvalds/linux/master, считаны 2026-09-06. HEAD при проверке: `1fc5a74b108fc90951890ec513ac81869f5eaff1` (сами файлы получены по master в соседних запросах).
- `upstream-readme.md` — исходные заметки MinimumLaw. Текст и код третьих лиц сохраняют исходное авторство/лицензию; локальное сохранение не означает лицензию на дальнейшую публикацию всех материалов.
- `SHA256SUMS` — контрольные суммы сохранённых материалов. Полные инвентаризации содержат идентификаторы устройств, использовать как внутренние материалы.

## Что действительно мешает загрузке

Процессор поддерживается Linux; проблема на уровне описания конкретной платы и соответствия драйверов/прошивки. Windows использует не только стандартные ACPI `_CRS`, но и вендорские пакеты Qualcomm PEP, которые обычная Linux ACPI-инициализация не воспроизводит автоматически.

Фразу из старых обсуждений «DSDT бедная» следует уточнить: наша DSDT содержит 189 КБ AML и около 1.9 МБ декомпилированного текста. В ней есть полезные параметры питания и тактирования, но их надо интерпретировать и сопоставлять с Linux bindings. Это не готовый Device Tree и не доказательство возможности полноценной ACPI-загрузки.

Исторически ранние ошибки UEFI обходили `efi=novamap,noruntime`, а сохранение настроенных firmware clocks/power domains — `clk_ignore_unused pd_ignore_unused`. Это параметры для исследования, не доказанное окончательное решение.

В отчёте AliceCyber от 2023-05-21 работали EFIFB, клавиатура, USB и Bluetooth; не работали тачпад, GPU/мост и UFS, Wi-Fi/WWAN не проверялись. В логе 2023-05-22: UFS NOP OUT failed -11 и `gcc_ufs_phy_axi_clk status stuck at 'off'`; TravMurav указывает на возможную проблему питания/остановки clocks, влияющую на GCC. В 2025 fixinit75 доходил до init/udev, затем зависание. Нельзя выдавать эти результаты за тест нашей машины.

Важное обновление: в проверенном современном `sc7180.dtsi` уже есть `ufs_mem_hc` и `ufs_mem_phy` с `qcom,sc7180-qmp-ufs-phy`. Драйвер `phy-qcom-qmp-ufs.c` сопоставляет этот compatible с `sm7150_ufsphy_cfg`. Утверждение из 2023 года об отсутствии поддержки PHY больше не описывает текущее дерево. При этом готового TCL DTS в проверенном каталоге qcom не найдено, и работоспособность UFS именно на TCL ещё не доказана.

## Проверенные исходные данные для DTS

Имена Windows I2C нельзя копировать как Linux label: соответствие установлено по MMIO и IRQ, а не по номеру в имени.

| ACPI | MMIO / длина | ACPI IRQ | Linux label / SPI |
|---|---|---|---|
| `_SB.I2C3` | 0x00888000 / 0x4000 | 0x27b = 635 | `&i2c2`, SPI603 |
| `_SB.I2C5` | 0x00890000 / 0x4000 | 0x27d = 637 | `&i2c4`, SPI605 |
| `_SB.I2C8` | 0x00a84000 / 0x4000 | 0x182 = 386 | `&i2c7`, SPI354 |
| `_SB.IC11` | 0x00a90000 / 0x4000 | 0x185 = 389 | `&i2c10`, SPI357 |
| `_SB.UAR4` | 0x0088c000 / 0x4000 | 0x27c = 636 | `&uart3`, SPI604 |
| `_SB.UFS0` | 0x01d84000 / 0x14000 | 0x129 = 297 | `&ufs_mem_hc`, SPI265 |

Для этих GIC SPI ACPI global interrupt ID = DT SPI + 32. Размер UFS `_CRS` охватывает больший диапазон, чем Linux host reg 0x3000: нельзя заменить размер host node на весь ACPI диапазон, перекрыв PHY/ICE.

### Клавиатура: подтверждено ACPI и работающим Windows PnP

- `_SB.ECKB`, ACPI QTEC0001, Windows hidi2c + kbdhid.
- `&i2c4`, 400000 Hz, адрес 0x05, `hid-descr-addr = <0x20>` из HID `_DSM`.
- IRQ GIO0 GPIO33, Level ActiveLow, PullUp; кандидат `interrupts-extended = <&tlmm 33 IRQ_TYPE_LEVEL_LOW>`.
- Дополнительно `_PS0` устанавливает `_SB.GIO0.LISP = 1`, `_PS3` сбрасывает в 0. LISP связан с GPIO32 (DSDT.dsl около 43026). Это подтверждённая последовательность управления, но электрическую роль pin (enable/reset/питание) надо установить до выбора DTS property/драйвера.
- В старом DTS IRQ и адрес совпадают, но управление GPIO32 отсутствует, а IRQ pin задан bias-disable вместо ACPI PullUp. Возможная причина различий; причинность на Linux не проверена.

### Тачпад: подтверждено ACPI и работающим Windows PnP

- `_SB.TCPD`, ACPI QTEC0002, HID VID_093A, Windows hidi2c.
- `&i2c4`, 400000 Hz, адрес 0x2c, HID descriptor 0x20.
- GPIO94, Level ActiveLow, PullDefault; waking IRQ.
- В `_SB.PEP0.TPXC` для D0 есть `TLMMGPIO` с pin 0x19 = GPIO25 и параметрами `(1,0,1,0,0)`; D3 без действий. Участие GPIO25 подтверждено; 3.3 В и точное значение каждого поля пакета не выводятся из одного этого текста.
- Опубликованный DTS уже содержит GPIO25 как active-high regulator 3.3 В; считать это гипотезой до проверки PEP semantics/схемы. Альтернативный тачпад 0x15, упомянутый в чужом DTS, не соответствует выбранному Windows устройству нашего экземпляра.

### Мостик и дисплей

- `_SB.EDP1`, ACPI EDPBridge, служба EDPBridge, INF oem48.inf.
- `&i2c10` (ACPI IC11), адрес 0x29, 100000 Hz.
- `_CRS` перечисляет GPIO23,20,51,26,13,21 (именно в таком порядке). Направление/полярность/назначение каждого pin из `_CRS` не следуют; не переносить подписи от Acer.
- Дополнительная memory resource 0x9ff90000 / 0x2000. Не считать автоматически MMIO чипа или framebuffer; требуется разбор назначения.
- LT8911EXB известен из UEFI-лога другого TCL. В Windows нашей машины пока подтверждено только EDPBridge; точную маркировку подтвердить драйвером/UEFI/платой.
- EDID: 1920×1080, pixel clock 142520000 Hz, около 59.9993 Hz. Тайминги 1366×768 из старой статьи к этому EDID не подходят. EDID из реестра может быть сохранённым описанием; фактический GOP mode перед Linux надо снять отдельно.
- Preferred timing из первого detailed descriptor: horizontal active/front/sync/back = 1920/58/42/60, vertical = 1080/3/5/54. Flags 0x1f, extension count 0 (вторые 128 байт файла — заполнение). Машиночитаемая расшифровка `display-timing.json`.
- В mainline среди Lontium bridge drivers LT8911EXB не найден (есть lt8713sx, lt8912b, lt9211, lt9611, lt9611uxc). Одно сходство имени не означает совместимость.
- Для первого запуска сохранять framebuffer UEFI, не включать неподтверждённую инициализацию MDSS/DSI/bridge. Это позволяет отделить запуск ядра от полноценного DRM/GPU.

### UFS: ключевая находка в Qualcomm PEP

DSDT.dsl: `_SB.PEP0.BPCC`, около 23270–23865. Есть не только `_CRS`, но и описание D0/D3, clocks, interconnect и regulator votes.

| Ресурс PEP для UFS D0 | Значение | Интерпретация |
|---|---|---|
| PPP_RESOURCE_ID_LDO19_A | 0x002d2a80 = 2960000 | кандидат 2.96 В |
| PPP_RESOURCE_ID_LDO12_A | 0x001b7740 = 1800000 | кандидат 1.8 В |
| PPP_RESOURCE_ID_LDO3_C | 0x00124f80 = 1200000 | кандидат 1.2 В |
| PPP_RESOURCE_ID_LDO4_A | 0x000d6d80 = 880000 | кандидат 0.88 В |
| gcc_ufs_phy_axi_clk | 0x0bebc200 = 200000000 | 200 MHz, совпадает с max mainline freq-table |
| gcc_ufs_phy_unipro_core_clk | 0x08f0d180 = 150000000 | 150 MHz, совпадает с max mainline freq-table |
| gcc_ufs_phy_ice_core_clk | 0x11e1a300 = 300000000 | 300 MHz, ICE описывается отдельно |

Также упомянуты `gcc_ufs_phy_gdsc`, aggre/ahb/phy_aux/tx_symbol_0/rx_symbol_0, `gcc_ufs_mem_clkref_en`; bus votes `MASTER_UFS_MEM→SLAVE_EBI1` и `MASTER_APPSS_PROC→SLAVE_UFS_MEM_CFG`.

Порядок D0 в PEP: PSTATE2 (interconnect), PSTATE0 (GDSC), четыре regulator votes, DELAY 0x23, PSTATE1 (clocks). D3 выполняет обратные действия; LDO12_A сохраняет vote 1.8 В с другим режимом. Единицы DELAY и смысл полей PEP требуют подтверждения, не писать «35 мс» без источника.

Набор линий поддерживает рабочую гипотезу `host vcc→L19A`, `host vccq2→L12A`, `phy vdda-pll→L3C`, `phy vdda-phy→L4A`. Это НЕ доказанная распиновка: DSDT группирует votes по устройству, не даёт им Linux supply names. Требуются сравнение с bindings/SC7180 reference и фактическим UFS/PHY. Не включать UFS до этой проверки и получения доступной отладочной консоли.

Текущее Linux SoC описание: host 0x1d84000/0x3000, PHY 0x1d87000/0x1000, одна lane, SMMU stream 0xa0, GDSC UFS_PHY_GDSC. Эти значения взяты из mainline и требуют сверки с IORT/CSRT нашей платы. Отсутствие exact TCL DTS не оправдывает применение PHY от произвольного SoC: теперь есть точный compatible SC7180.

## Следующие шаги и граница готовности

### Дополнительная сверка: UART, SMMU и USB

Разобраны `IORT-9.dat`, `SPCR-10.dat`, `DBG2-8.dat`, `CSRT-3.dat` командой iasl; результаты `.dsl` сохранены рядом.

- IORT: SMMU node offset 0x30, base 0x15000000 / span 0x100000, 80 context interrupts.
- IORT UFS0: Output Base 0xa0, Output Reference 0x30, ID Count 0, cache coherency=1. Подтверждает SC7180 UFS stream ID в mainline; не использовать в DTS значение Input base 0x81030000 как SID.
- IORT URS0 и USB0: Output Base 0x540, reference 0x30. Windows ACPI путь вложенного устройства `_SB.URS0.USB0`, в IORT строка `_SB.USB0`; это несовпадение имён также учитывать при анализе ACPI-загрузки.
- DSDT URS0: MMIO 0x0a600000, длина 0x000fffff; USB IRQ 0xa5/0xa2/0x206/0x208/0x209. Их назначения сверять с mainline usb_1; сам порядок без драйвера недостаточен.
- DBG2: отладочный UART `_SB.UARD` по адресу 0x00a88000, размер 0x1000, port subtype 0x11. Адрес соответствует Linux `uart8` и опубликованному DTS.
- **SPCR противоречит DBG2/DSDT:** serial address 0x00a90000, interrupt 0x182, baud code 7. Адрес 0xa90000 используется IC11/i2c10 для мостика дисплея, IRQ 0x182 совпадает с I2C8. Поэтому автоматический выбор earlycon из SPCR ненадёжен. У DBG2/SPCR также Encoded Access Width=0x20, который iasl помечает Unknown Width Encoding. Это конкретные дефекты/несогласованности описания, а не просто «мало данных».
- iasl для SPCR rev2/80 байт напечатал `table terminates in the middle of a data structure`; не трактовать отдельно как доказательство повреждения firmware, поскольку декодер может ожидать поля более новой структуры. Сами адреса проверены в двоичном дампе.
- GPIO/электрические уровни физического DEBUG UART ещё не измерялись; адрес DBG2 не подтверждает напряжение контактов или возможность безопасного подключения конкретного USB-UART.

### Windows driver metadata и firmware

Сохранены `driver-details.json`, `edpbridge.inf`, `EDPBridge.sys` (40880 байт), строки драйвера ASCII/UTF-16. SHA256 EDPBridge.sys: `d9a3aa29c7e71fc0d9e142012c483d56d700a66d858690690d10da56156a8023`. Это локальная исследовательская копия OEM-драйвера, не открытый исходный код.

- EDPBridge INF: Provider JLQ Technology, DriverVer 2022-11-07 / 16.25.12.295, KMDF 1.15, ARM64. GPIO roles и имя LT8911EXB в INF не указаны.
- Строка PDB: `D:\JLQCode\SC7180\EdpbridgeQ329_328_Emdoor\ARM64\Release\EDPBridge.pdb`. Это подсказка о происхождении драйвера (Emdoor/Q329_328), а не доказанная модель/схема платы. Поиск строк не дал назначения GPIO; требуется разбор машинного кода или исходники/схема.
- PEP: Qualcomm System Manager Power Engine Plug-in Device, `oem72.inf`, версия 1.0.800.0; бинарный путь `System32\DriverStore\FileRepository\qcpep.wd7180.inf_arm64_8c2bf704ae20b939\qcpep7180.sys`.
- UFS в Windows обслуживает `storufs.sys`, PEP участвует отдельно; наличие стандартного storufs не означает, что Linux обойдётся без корректного PMIC/GDSC/PHY.
- Найдены 12 firmware-файлов по маскам *.mbn/*.mdt/*.b00/*.elf. Полные пути/размеры в driver-details.json: qcdxkmsuc7180.mbn (GPU zap), qcvss7180.mbn, ipa_fws.elf, qcadsp7180.mbn, qccdsp7180.mbn, qcmpss7180.mbn, qcmpss7180_nm.mbn, hdcp1.mbn, hdcp2p2.mbn, hdcpsrm.mbn, pr_3_wp.mbn, wlanmdsp.mbn. Роли нескольких имён следуют из названия driver package, не проверены загрузкой Linux. Эти firmware пока не экспортированы; список не включает все возможные форматы Wi-Fi/BT board data и calibration.
- Технический нюанс сбора: PowerShell `Get-Content -Raw` вернул строку с provider metadata, и ConvertTo-Json развернул лишнюю метаинформацию. Из результата сохранено только значение текста INF; прочие поля нормализованного JSON сохранены. При повторном сборе использовать `[IO.File]::ReadAllText()` для получения обычной строки. Ошибку устранить в сборе, не интерпретировать большой JSON как аппаратные данные.

### Что уже можно передать автору DTS, а что нельзя обещать

Готовы: собственные AML/DSL, инвентаризация и версии OEM-драйверов; точная шина/адрес/HID descriptor/IRQ клавиатуры и тачпада; обнаруженное управление GPIO32; адрес/ресурсы мостика; EDID; UFS MMIO/IRQ/SID, PEP votes/clocks/порядок; USB MMIO/SID; UART DBG2 и конфликт SPCR; список firmware; старый DTS и современные SoC definitions.

Не установлены: электрические роли/полярности GPIO мостика, точная маркировка мостика на этой плате, окончательное соответствие regulator votes именам UFS/PHY supplies, runtime memory map/reserved-memory и текущий GOP framebuffer, pinmux/power sequencing всех периферийных устройств, источник полного набора board firmware, безопасная аппаратная UART-консоль. Значения в дампах описывают firmware, но сами по себе не доказывают электрическую схему и успешную инициализацию Linux.

Поэтому подготовка исходных данных и их первичная верификация выполнены; полноценный DTS/DTB и восстановление/испытательная загрузка — следующий отдельный этап. Ошибку запуска нашего ядра ещё не наблюдали. Публиковать или загружать чужой DTB как проверенный нельзя.

1. Сохранить и разобрать EDID, DSDT, IORT, SPCR/DBG2; сравнить reserved-memory, SMMU IDs, USB/PHY и UART. Номера из чужого DTS проверять по физическим адресам.
2. Прочитать конфигурацию EDPBridge/PEP и доступные firmware filenames Windows; установить GPIO roles/полярности, regulator mapping и pinmux. Не выполнять произвольные записи MMIO/PMIC ради угадывания.
3. Подготовить таблицу «подтверждено / гипотеза / неизвестно» и минимальный DTS только на проверенной базе. Сначала RAM/initramfs + framebuffer + UART/USB, UFS и remoteproc отключены. Наличие собравшегося DTB не равно корректности схемы.
4. До первой загрузки подготовить восстановление Windows и экспорт OEM-драйверов. Сейчас WinRE только проверена; резервная копия диска/драйверов ещё не сделана. Экспорт драйверов не заменяет резервную копию UFS.
5. Для эксперимента: ARM64 UEFI loader `EFI/BOOT/BOOTAA64.EFI`, GRUB с devicetree, совместимые Image/initramfs/DTB. F7/F5 известны из статьи, на этой машине в текущей работе не проверялись. Developer Mode ещё не проверен. Первый запуск проводить при наличии человека у ноутбука/консоли.
6. Исследовательские параметры из источников: `efi=novamap,noruntime clk_ignore_unused pd_ignore_unused`, сохранение `gfxpayload=keep`; UART `console=ttyMSM0` только после проверки aliases/адреса. Не считать их готовой универсальной cmdline.
7. Сборку пакетов ядра передавать alt-packaging-agents согласно правилам проекта. Сейчас задача — сбор данных, не сборка/установка/публикация ядра.

## Источники

- [MinimumLaw README](https://github.com/MinimumLaw/TCL-B220G-Linux): оборудование, UEFI/debug, восстановление Windows.
- [MinimumLaw issue #1](https://github.com/MinimumLaw/TCL-B220G-Linux/issues/1): история опытов, включая 2025. Последнее обновление issue в API 2025-09-18; 40 комментариев.
- [AliceCyber: результат и DTS](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1556219317), [само вложение](https://github.com/hexdump0815/imagebuilder/files/11525028/sc7180-tcl-book-14-go.dts.txt).
- [Лог UFS и UART](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1557691230), [объяснение TravMurav](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1558529478).
- [Советы по минимальному DT и EFIFB](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1554131693).
- [Mainline sc7180.dtsi](https://github.com/torvalds/linux/blob/1fc5a74b108fc90951890ec513ac81869f5eaff1/arch/arm64/boot/dts/qcom/sc7180.dtsi), [UFS PHY driver](https://github.com/torvalds/linux/blob/1fc5a74b108fc90951890ec513ac81869f5eaff1/drivers/phy/qualcomm/phy-qcom-qmp-ufs.c).
- [Внешний LT8911EXB driver](https://github.com/aystshen/lontium_lt8911exb_driver): ссылка из обсуждения, готовность к современному DRM не проверена.
- `gh api repos/foric27/kernel_tcl_b220g` вернул HTTP 404 на дату исследования: код/DTS из этого репозитория не получены. Не утверждать, что репозиторий удалён: 404 также возможен при приватности.
- Комментарии про Samsung Galaxy Book Go/Chromebook в общем issue #136, включая 2026 год, не доказывают работоспособность TCL.

## Отчётность по шагам

- Шаг 1: доступ, fastfetch, инвентаризация и ACPI — завершён. bug_add_comment с оценкой 10 минут: подтверждён пользователем, confirmation_check approved, comment_id **180892**. confirmation_id `13ee7b5e-bbb7-4657-8404-56206996279b`.
- Шаг 2: разбор DSDT, соответствие I2C/IRQ, GPIO32, UFS PEP, EDID и обновление сведений о mainline — завершён. Отчёт с оценкой 10 минут подтверждён: comment_id **180893**, confirmation_id `02c5eda9-91f6-4203-b439-ba6ef7928889`.
- Шаг 3: IORT/SPCR/DBG2/CSRT, EDID timings, OEM INF/driver strings и firmware index — завершён. Отчёт с оценкой 5 минут подтверждён: comment_id **180894**, confirmation_id `d40a2e9a-d3fe-423c-8567-b85deaf4fe9b`.
- Проверка сохранения: 36 файлов в SHA256SUMS, все контрольные суммы совпали; JSON разбирается, у текстовых заметок/скриптов есть завершающий перенос строки. Пароль в файлы не записывался. Изменения в git не коммитились и не публиковались.

## Шаг 4: аналоги, разбор мостика и флешка (2026-09-06)

Пользователь рядом с ноутбуком; UART пока нет. Разрешено полностью стереть предоставленную флешку от 8 ГБ. После подключения Windows определила USB **Kingston DT 101 G2**, serial `001CC0EC34E4FBB085C323F2`, **8010194944 байт**, диск **6**, GPT, IsBoot/IsSystem=false. Внутренний Samsung UFS — диск 0. Номер диска при записи перепроверять вместе с serial, BusType и размером. На USB уже четыре раздела: 3566909440 байт D:, ESP 6782976, 307200 E:, неизвестный 4435476480. Пока ничего не стирали и не записывали.

### Дополнительные доказательства для DTS

- Windows VideoController подтвердил текущий режим 1920×1080, 60 Гц, 32 bit. Это не физический адрес GOP framebuffer.
- Сохранён REG_RESOURCE_LIST физической памяти и декодированные диапазоны. Последний дескриптор MemoryLarge (type 7, flag 0x200) имеет длину с масштабом 256. Сумма диапазонов 8103067648 байт; отличается от ранее полученного TotalPhysicalMemory 8125050880. Причина различия пока не установлена. Это карта Windows, **не EFI GetMemoryMap** и не готовый memory/reserved-memory DTS.
- Galaxy Book Go DTS и mainline SM7125 Xiaomi используют UFS host vcc=L19A, vccq2=L12A, PHY vdda-phy=L4A, vdda-pll=L3C. Совпадает с группой PEP votes нашего TCL. Сильное подтверждение рабочей гипотезы, но не измерение схемы платы; пределы токов чужой платы не переносить.
- В pinctrl-sc7180.c UFS_RESET имеет номер **119**, ngpios=120; SM7125 DTS использует reset-gpios TLMM119 active low. Закомментированная строка TLMM150 в старом TCL DTS ошибочна для SC7180. Наличие линии SoC ещё не доказывает подключение reset на этой плате.
- Разбор локального ARM64 EDPBridge.sys показал последовательность банков/регистров ff=81,08=7f и чтение00/01/02, совпадающую с открытым LT8911EXB driver. Сильный признак семейства мостика; фактический chip ID нами не считан.
- Функция 0x140002d40 обрабатывает первые пять GPIO connection resources DSDT: 23,20,51,26,13. По машинному коду для четвёртой линии (26): запись1, ожидание20000, запись0, ожидание150000, запись1. Затем для пятой (13): 1, ожидание10000, 0, ожидание30000, 1, ожидание10000. Ожидания — вызовы KeStallExecutionProcessor (микросекунды). Электрические назначения пока неизвестны; GPIO21 в этом цикле не участвует. Декомпилятор выдаёт предупреждения о параметрах/стеке, поэтому вывод проверять по инструкциям ARM64.

### Источники аналогов

- [Galaxy Book Go DTS](https://gist.github.com/goodspeed34/620650946609ae07db82e9c83c724c3a), получен через gh api.
- [SM7125 Xiaomi common](https://github.com/torvalds/linux/blob/master/arch/arm64/boot/dts/qcom/sm7125-xiaomi-common.dtsi); локальная копия сохранена.
- [SC7180 pinctrl](https://github.com/torvalds/linux/blob/master/drivers/pinctrl/qcom/pinctrl-sc7180.c); локальная копия сохранена.
- [Обсуждение UFS reset SM7125](https://lkml.iu.edu/2407.3/09057.html).
- [SC8280XP UFS clock fix](https://patchew.org/linux/20220825163755.683843-1-bmasney%40redhat.com/): похожий stuck-off возникал из-за ref_aux clock. Это другой SoC, не готовый патч для TCL.
- [Исходник LT8911EXB](https://github.com/aystshen/lontium_lt8911exb_driver/blob/master/lt8911exb/lt8911exb.c); локальная исследовательская копия сохранена.
- [TCL различает B220G и B220G1](https://www.tcl.com/global/en/support-mobile/faq/11727); [официальные загрузки](https://www.tcl.com/global/en/support-mobile/model/tcl-book-14-go).

Обнаружен свежий [imagebuilder release 251220-01](https://github.com/velvet-os/imagebuilder/releases/tag/251220-01), asset snapdragon_7c_woa-aarch64-trixie-251028.img.gz, 1880690128 байт в сжатом виде. Совместимость с TCL и распакованный размер ещё не проверены; это кандидат для изучения загрузочного комплекта, не проверенный образ для записи.

Шаг 4 опубликован в Bugzilla, Markdown, 25 минут (оценка): comment_id **180895**, confirmation_id `fa1e1ad6-f996-44c1-97dd-0465c03ac088`, approved. На момент отчёта запись USB ещё не подтверждена.

Диагностический GRUB 2.12-9+deb13u2 собран отдельно, без ядра, сохранён в tcl-b220g-data/usb-diag/ вместе с меню и скриптом. SHA256 BOOTAA64.EFI: e444b7e1ad78d87d81f78a4eaec9027ee14ae846ed84dadb3986770c582de1f2. Вывод только на экран, потребуются фотографии; автоматического сохранения диагностики нет. Пользователь об этом уведомлён. USB-загрузка проверяет firmware/GRUB и даёт videoinfo/lsefimmap, но не диагностирует само ядро.

Первый вариант передачи большого скрипта через stdin не исполнился; второй с ReadToEnd завис до создания файла и до изменения USB. После проверки процесса/старой разметки остановлен только этот PowerShell. EFI и небольшой скрипт успешно переданы через scp; запущен вариант -File. Подробности в usb-diag/README.md.

### Шаг 5: USB записана и проверена

Kingston disk 6, serial 001CC0EC34E4FBB085C323F2: создан GPT/FAT32 том TCLDIAG, буква D:, размер ФС 7992246272 байт, свободно 7988666368. BOOTAA64.EFI записан в EFI/BOOT, SHA256 прочитанного с USB файла совпал с локальным: e444b7e1ad78d87d81f78a4eaec9027ee14ae846ed84dadb3986770c582de1f2. Скрипт завершился с кодом 0 и JSON отчётом (usb-diag/usb-write-result.json). Внутренний UFS не изменяли, ноутбук не перезагружали.

Исправления подготовки: Clear-Disk сохраняет GPT на этой Windows, Initialize-Disk нужен только при RAW. Большие скрипты передавать scp и запускать -File; ReadToEnd из stdin SSH здесь зависает. Эти исправления включены в сохранённый скрипт.

Следующий шаг требует действий пользователя у ноутбука: перезагрузка, F7 и USB Kingston (по процедуре из репозитория; на этом экземпляре ещё не проверена); при недоступности USB проверить Developer Mode в DEL Setup, Secure Boot ранее уже false. В меню GRUB сначала videoinfo, затем lsefimmap с постраничным выводом. Результат только на экране, нужны фотографии. Для возврата выбрать reboot и вынуть USB. Это ещё не тест Linux.

Архив ядра 6.18.34-stb-qc7+ скачан в /tmp для дальнейшего исследования; config и manifest с SHA256 сохранены в данных. В конфиге EFI, FB_EFI, I2C_HID_OF и BLK_DEV_INITRD встроены. На флешку ядро не записано.

Шаг 5 опубликован: comment_id **180896**, confirmation_id `b97fd0fe-e3aa-4f4f-be5c-cc91611fbee8`, approved; 15 минут (оценка).

### Первый запуск диагностического GRUB — подтверждён пользователем

Пользователь передал вывод первого пункта: `GRUB ARM64 is running. Linux has NOT been started.`, adapter `EFI GOP driver`. Это подтверждает загрузку подготовленного ARM64 GRUB с USB и работающий вывод через GOP. Ядро Linux не запускалось.

Режимы из переданного текста (строки перемешаны при копировании):

| GOP mode | Размер | Глубина | Bytes per scanline |
|---|---|---|---|
| 0x000 | 1920×1080 | 32 | 7680 |
| 0x001 | 640×480 | 32 | 2560 |
| 0x002 | 800×600 | 32 | 3200 |
| 0x003 | 1280×720 | 32 | 5120 |
| 0x004 | 1024×768 | 32 | 4096 |
| 0x005 | 1280×768 | 32 | 5120 |
| 0x006 | 1366×768 | 32 | 5464 |

Все режимы: Direct color, маски R/G/B/reserved 8/8/8/8, позиции16/8/0/24. Это описывает 32-битный пиксель 0xXXRRGGBB (в памяти little-endian B,G,R,X); reserved не считать доказанным alpha. Таблица перечисляет поддерживаемые режимы, переданный текст не содержит отметки текущего режима. Физический адрес framebuffer не получен.

Для выбранного 1920×1080 при stride7680 объём видимой области составит8294400 байт (0x7e9000); это расчёт, не фактический размер GOP allocation. Не создавать framebuffer reg без физического адреса.

Следующее действие: Escape, второй пункт lsefimmap, фотографии всех страниц; пробел для следующей страницы. Затем возврат в Windows нужен для экспорта драйверов. Передача export-drivers.ps1 по SSH не состоялась (No route to host, затем timeout); Export-WindowsDriver ещё не запускался, нового архива нет. Локально подготовлен скрипт экспорта OEM пакетов, firmware, системных UFS/HID/I2C файлов и manifest/SHA256.

### Получена карта UEFI lsefimmap от пользователя

Передан текст с распознаванием экрана. Колонки Type / Physical start-end / pages / Size / Attributes перемешаны, встречаются ошибки OCR (`9ffBffff`, `17-22fff`, пропущенная c в начале диапазона, `UT/UB/UP` вместо вероятных WT/WB/WP). Не считать это проверенным машинным дампом и не привязывать типы к диапазонам по порядку текстовых блоков.

Читаемые диапазоны верхней RAM: 0xfffff000–0x17fffffff (0x80001 страниц), 0x180000000–0x18001bfff (0x1c страниц), 0x18001c000–0x27dffffff, 0x27e000000–0x27fffffff (32 MiB). Верхняя исключающая граница0x280000000 совпадает с Windows resource map. Это подтверждает необходимость учесть вариант8ГБ; не переносить memory/reserved-memory от4ГБ без проверки. Конкретные allocation types верхних диапазонов требуют сверки по изображению.

Видны firmware carveouts, отсутствующие в списке Windows RAM: 0x80000000–0x805fffff (6MiB), 0x80600000–0x807fffff (2MiB), 0x80800000–0x8083ffff (256KiB), 0x80840000–0x8084ffff (64KiB), 0x80900000–0x80afffff (2MiB), 0x85b00000–0x945fffff (235MiB), 0x9bc00000–0x9cafffff (15MiB), 0x9dc00000–0x9e3fffff (8MiB), 0x9e400000–0x9f7fffff (20MiB). Назначения не установлены, не объявлять их framebuffer или remoteproc без доказательства.

Отдельная область0x9ff90000–0x9ff91fff (8KiB) совпадает с MMIO/memory resource мостика EDP1 в DSDT. Это совпадение адреса, не доказательство назначения или размера framebuffer. Ещё виден reserved0x9ffda000–0x9ffdcfff (12KiB).

Пользователю предложено вернуться в Windows через reboot с удалением USB, чтобы выполнить ранее запрошенный экспорт драйверов. До восстановления SSH экспорт невозможен. Для окончательной расшифровки lsefimmap предпочтительны исходные фотографии с выровненными колонками; текущий текст не достаточен для автоматического построения reserved-memory.

### Расхождение двух текстовых расшифровок videoinfo

Исходные пользовательские тексты сохранены отдельно: grub-videoinfo-user-transcript-1.txt и grub-videoinfo-user-transcript-2.txt. Во второй версии размеры/stride совпадают, но mask=0/0/0/8 и pos=16/0/0/24 для всех семи режимов; в первой mask=8/8/8/8 и pos=16/8/0/24. До сверки со скриншотами нельзя выбирать одну версию или утверждать формат пикселя0xXXRRGGBB. Ранее приведённая интерпретация формата условна и опиралась только на первую расшифровку. Нулевые RGB mask могут быть ошибкой распознавания/переписывания либо реальным выводом, причина не установлена.

Пользователь просит сохранить текст для сравнения с ожидаемыми скриншотами. Отчёты в Bugzilla приостановлены по прямому указанию пользователя. Последняя попытка scp export-drivers.ps1 после сообщения о восстановлении доступа снова завершилась timeout к192.168.8.143:22; экспорт ещё не запускался.

Получена вторая текстовая расшифровка lsefimmap, сохранена без исправлений в grub-lsefimmap-user-transcript-2.txt. Явные противоречия: первая строка end<start; BS-code pages0x401 означает4100KiB, а указан размер260KiB; ldr-code pages0x200 означает2MiB, а указан размер32MiB. Нижние reserved-диапазоны потеряли адреса. Эту версию нельзя использовать для DTS; типы и атрибуты также сверить со скриншотами. Не подменять исходные тексты реконструкцией.

### Сверка с исходными фотографиями завершена

Получены /home/lav/photo_2026-09-06_21-12-34.jpg (videoinfo) и photo_2026-09-06_21-12-48.jpg (lsefimmap); копии сохранены в каталоге данных. Более старые photo*.jpg не относятся к задаче и не читались.

**GOP:** фотография однозначно показывает mask8/8/8/8 и pos16/8/0/24 во всех семи режимах. Первая текстовая расшифровка по этим полям верна, вторая с нулями ошибочна. Разрешения и stride во второй версии верны. Исправленный проверенный текст: grub-videoinfo-photo-verified.txt. Reserved-байт не объявлять alpha; физический адрес framebuffer по-прежнему неизвестен.

**Карта памяти:** вручную восстановлены47 строк с типами/адресами/pages/размерами/атрибутами. Файлы grub-lsefimmap-photo-verified.md и .json. Для всех строк программно проверены end-start+1 = pages×4096 = отображаемый размер и отсутствие пересечений. Это проверка согласованности расшифровки, не валидация firmware.

Вторая текстовая версия lsefimmap сильно повреждена: 0x41 страниц BS-code переписано как0x401; верхний ldr-code32MiB имеет0x2000 страниц, не0x200; верхний conv-mem имеет0xfdfe4 страниц; runtime MMIO только RT UC, а не все атрибуты. У нижних reserved восстановлены точные адреса. Сохранённые исходные расшифровки не исправлялись.

Ключевые уточнения по фото:

- 0x27e000000–0x27fffffff: **ldr-code32MiB**, не доказанный постоянный резерв и не установленный framebuffer.
- 0xffed2000–0xfff21fff: ACPI-nvs320KiB; 0xfff22000–0xffffdfff: ACPI-rec880KiB.
- 0x9ff90000–0x9ff91fff: reserved8KiB, совпадает с ресурсом EDP1.
- 0x9ffda000–0x9ffdcfff: reserved12KiB WC, расположен между двумя RT-data областями.
- 0x9e400000–0x9f7fffff: reserved20MiB WC; это кандидат для дальнейшего выяснения назначения, **не подтверждённый framebuffer**.
- 0xc264000–0xc264fff и0x17c22000–0x17c22fff: runtime MMIO4KiB RT UC.

BootServices/Loader allocations меняются между загрузками; не превращать весь снимок GRUB в постоянный reserved-memory DTS. Сопоставить с EFI handoff, ACPI и кодом драйверов. В Bugzilla новые данные не отправлялись по указанию пользователя.

### Возобновление публикации и попытка экспорта

Пользователь явно разрешил снова писать расшифрованные данные в багу, включая текстовое вложение. Подготовлено tcl-b220g-grub-photo-verified-20260906.md с полной проверенной таблицей47 строк, videoinfo и ограничениями. bug_add_attachment ожидает подтверждения Lavtomate: f30c900b-91d9-43fb-95a2-975b0a3ed812. Ранее отложенный комментарий36246b94-3667-4115-a2be-986a7ca09523 также остаётся pending; повторно не отправлялся.

Очередная передача export-drivers.ps1 завершилась тайм-аутом SSH192.168.8.143:22. Пользователю задан вопрос о текущем IPv4 активного подключения Windows. Экспорт ещё не запускался и архива локально нет; не смешивать готовые ранее EDPBridge/ACPI данные с ещё не скопированным набором драйверов.

Вложение **7663** опубликовано в Etersoft#19084, confirmation_check approved. Ссылка: https://bugs.etersoft.ru/attachment.cgi?id=7663 .

### Восстановление SSH после перезагрузки

Пользователь сообщил: Start-Service sshd отвечает отсутствием службы; OpenSSH Server устанавливался через Add-WindowsCapability -Online -Name OpenSSH.Server~~~~0.0.1.0. Пока неизвестен текущий Capability State и результат установки. Следующая диагностика на Windows: Get-WindowsCapability -Online -Name OpenSSH.Server*, sc.exe query sshd, Test-Path C:\Windows\System32\OpenSSH\sshd.exe. Не регистрировать службу вручную и не переустанавливать до проверки состояния. Документация: https://learn.microsoft.com/en-us/windows-server/administration/openssh/openssh_install_firstuse .

### Автономная копия драйверов получена

Export-WindowsDriver успешно выгрузил86 OEM-пакетов. Добавлены12 системных драйверов: storufs, hidi2c, hidclass, hidparse, kbdhid, kbdclass, mouhid, mouclass, SpbCx, Wdf01000, ACPI, EDPBridge. Всего manifest содержит638 файлов/305604641 байт, включая индексы пакетов/PnP/services. Сами manifest.json в эти638 не входит. Архив306279424 байт, SHA256 f26911653b337c95f1510403f0c23fecdfd431b0345c31a27b0ce2f56c4dc8fa.

Архив скопирован на локальную машину, SHA256 совпал с Windows. Распаковано в .claude/docs/tcl-b220g-data/windows-drivers/; размер и SHA256 каждого из638 файлов проверены по manifest. Сохранены qcpep7180.sys, qci2c7180.sys, qcgpio.sys, EDPBridge, GPU драйверы и все12 ранее найденных firmware. Экспортированы INF/CAT и остальные файлы OEM-пакетов. Для анализа этого набора Windows больше не нужна. Это копия драйверов, не резервная копия UFS/Windows.

Архив на Windows: C:\Users\Etersoft\tcl-driver-export-20260906.tar, распакованный экспорт рядом. Локальный архив /tmp/tcl-driver-export-20260906.tar временный; постоянное хранилище — windows-drivers/ в проекте. Проприетарные binaries в публичные репозитории не публиковались.

### Причина недоступности SSH после перезагрузки

По диагностике пользователь вручную запустил C:\Program Files\OpenSSH\sshd.exe, listener PID15188; службы sshd не было. Компонент Windows отдельно показывал State4, но реально использовался бинарник из Program Files. Конфигурация/ключи в ProgramData\ssh; firewall OpenSSH-Server-In-TCP включён для Private. Подключение даёт повышенный администраторский токен.

sshd -t успешно проверил конфигурацию. ACL бинарника: SYSTEM/Administrators FullControl, Users/AppPackages только ReadAndExecute. Зарегистрирована служба sshd LocalSystem, Auto, с quoted BinaryPathName и привилегиями по официальному install-sshd.ps1. Ключи и sshd_config сохранены. Подготовлено одноразовое задание SYSTEM для остановки только проверенного ручного listener и запуска службы; окончательная проверка отдельного входа ещё ожидается.

Дополнение проверки SSH: отдельный вход проходит, но проверка равенства PID службы и listener не прошла. Это ещё не подтверждённый переход на службу; прочитываются состояние Task Scheduler и файл результата/ошибки. Не считать факт нового SSH-входа доказательством, что listener уже обслуживает служба.

Причина задержки переключения установлена: задача находилась в Queued с DisallowStartIfOnBatteries=true и StopIfGoingOnBatteries=true; файлов результата/ошибки не было, ручной listener15188 продолжал работу. Для одноразовой задачи разрешён запуск от батареи, снята остановка при переходе на батарею, установлен лимит2минуты.

После снятия ограничения батареи Task Scheduler выполнил скрипт, но Stop-Process из SYSTEM запросил подтверждение остановки процесса другого пользователя. NonInteractive не допускает Prompt; LastTaskResult=1, служба осталась Stopped. Ошибка прочитана из файла задания. Исправлен конкретный шаг остановки заранее проверенного listener: -Force -Confirm:$false, без затрагивания дочерних SSH-сессий. Это штатное переключение сервера, не попытка обойти неизвестное зависание.

**Финальная проверка SSH успешна:** отдельное подключение по ключу, служба sshd Running/Auto/LocalSystem, PID1872 совпадает с listener порта22. Задание завершилось успешно и удалено; JSON сохранён в ssh-service-final.json. Конфигурация и ключи не менялись, правило firewall Private сохранено. Фактическую перезагрузку для проверки автозапуска не выполняли.

Отчёт об экспорте и восстановлении SSH опубликован: comment_id180898, confirmation_id2bcf4555-9cc7-41e9-ba46-2aa1f6ae325f, approved;20минут (оценка).

## Разбор PEP и подготовка первого теста ядра

Из qcpep7180.sys извлечены строки и записи с указателями на имена UFS. В первых8 qword соответствующих записей найдены все10 ожидаемых MMIO констант Linux GCC SC7180: aggre_axi0x182024, ahb0x177014, axi0x177038, ice0x177090, aux0x177094, rx0x17701c, tx0x177018, unipro0x17708c, clkref0x18c000, gdsc0x177004. Результат/адреса записей в pep-ufs-clock-records.md/.json; воспроизводимый анализ analyze-pep-ufs.py. Два дополнительных указателя rx/tx входят в другую таблицу и не содержат MMIO в просмотренном окне; они не объявлены несовпадающими аппаратными ресурсами. Полная структура/семантика вызовов пока не декодированы. Совпадение адресов не доказывает одинаковый порядок включения, но не даёт оснований переносить clock-address fixes от других SoC.

Проверены исходники Linux EFI: libstub/fdt.c передаёт linux,uefi-mmap-* в chosen и удаляет reserve-map entries FDT; это НЕ удаление узлов /reserved-memory. efi-init.c использует EFI memory map вместо DT memory nodes, освобождая подходящие Loader/BootServices области после ExitBootServices. Поэтому предупреждение про4/8ГБ не означает необходимость вручную вписать47 областей GRUB в DT. Главная ценность фото — сверка реальных carveouts/типов/границ и обнаружение конфликтов. Снимки исходников сохранены в linux-efi-*.c.

Дополнительно разобраны MADT/APIC, GTDT и FADT. Подтверждены CPU MPIDR0x000..0x700, GICv3 distributor0x17a00000, redistributor0x17a60000/0x100000, PSCI via SMC. Timer INTID17/18/19/16 совпадают с mainline, но GTDT polarity0 (high) отличается от mainline low; в экспериментальном DT выбрано mainline low, расхождение сохраняется в README. MADT maintenance IRQ24 отличается от mainline25; maintenance property для теста без KVM не включена. Новые поля TRBE, которые современный iasl читает за концом старых GICC записей, не используются.

Подготовлен kernel-diag/: минимальные DTS/DTB, готовый Image6.18.34-stb-qc7+ (экспериментальный upstream, не пересобирался), initramfs со статическим ARM64 BusyBox1.37.0-6+b9, init, сборочный скрипт, GRUB и manifest. Установлены dtc/libfdt через epm; компиляция DTB без предупреждений, проверены ARM64Image magic, статический ELF BusyBox и состав cpio. Это технические проверки артефактов, не успешная загрузка.

Первый пункт Linux использует maxcpus=1, cpuidle.off=1, console=tty0, earlycon=efifb, efi=novamap,noruntime,debug, clk_ignore_unused, pd_ignore_unused, nokaslr, loglevel8, keep_bootcon, panic0. DT описывает только CPU/PSCI/GIC/timer; UFS/USB/клавиатура/GPU/remoteproc не включены. RAM и framebuffer ожидаются из EFI; адрес framebuffer не угадывается. При успехе init выводит баннер USERSPACE и данные CPU/RAM/FB, затем остаётся на экране. Никакие диски не монтирует; автоматической записи логов нет.

Перед обновлением USB снова подтверждены serialKingston001CC0EC34E4FBB085C323F2, disk6,8010194944байт, FAT32TCLDIAG D:. Скрипт обновления без форматирования проверяет старый EFI hash, сохраняет его как BOOTAA64-grub-only.bak, проверяет hashes всех новых файлов. Комплект передан на Windows; результат обновления USB ещё ожидается.

**Обновление USB успешно:** disk6,D:, все6 файлов проверены на флешке; EFI SHA2560f4c1f01ccef95c3701b80ec02f458343f5a3a9ea8999f8de4b6877f117c898d. Прежний EFI сохранён EFI/BOOT/BOOTAA64-grub-only.bak. Результат в kernel-diag/usb-update-result.json. Новый пункт меню EXPERIMENTAL: Linux RAM test - one CPU, no UFS запускается только вручную. Ноутбук не перезагружали; ждём первого запуска пользователем.

Отчёт этапа подготовлен для Bugzilla: confirmation_id622a9561-9fd5-4573-9c2d-eae04387e77c; первоначальный ответ pending,20минут (оценка). Не отправлять повторно, проверять confirmation_check.

## 2026-09-07: первый успешный запуск Linux до userspace

По указанию пользователя с lav@192.168.8.186:/tmp/ скопирована photo_2026-09-07_00-18-21.jpg. Ранее сохранённые фото6сентября повторно не копировались. Новое фото и ручная расшифровка kernel-first-boot-photo-transcript.txt сохранены в каталоге данных.

**Подтверждено:** Linux6.18.34-stb-qc7+ aarch64 запустил /init из подготовленного initramfs. Видны диагностический баннер REACHED USERSPACE и последующий вывод uname/CPU/meminfo/framebuffer. CPU online=0 соответствует maxcpus=1; это не проверка остальных7CPU. MemTotal7665264kB, MemFree7637392kB на момент фото. MemAvailable перекрыт другим выводом, не восстанавливался догадкой.

**Framebuffer установлен по фактическому Linux выводу:** base0x9bc00000, length8100KiB=8294400байт=0x7e9000; /proc/iomem9bc00000–9c3e8fff. Режим1920×1080×32, stride7680, pages1; EFI VGA. Размер совпадает со stride×height. Полностью находится в UEFI reserved0x9bc00000–0x9cafffff (15MiB). Прежняя гипотеза про20MiB WC0x9e400000 не подтвердилась как адрес текущего framebuffer. Адрес относится к этому экземпляру/этому запуску, его стабильность между режимами/загрузками не проверена.

Это доказывает возможность раннего запуска ядра и userspace на минимальном DT с EFI framebuffer, без UFS/USB/GPU/remoteproc. Конкретная причина прежних зависаний с полным DT по-прежнему не установлена. UFS, клавиатура, USB, SMP и графическое ускорение этим тестом не проверены.

Уточнение пользователя: на экране наложения текста НЕ было. Впечатление от фотографии не отражает наблюдаемое поведение консоли. Предположение о конфликте keep_bootcon/efifb/fbcon отозвано; оснований менять cmdline по этому снимку нет. Причина искажения фотографии не установлена.

Следующая последовательность: отдельно SMP; затем USB/клавиатура с проверенными ресурсами. UFS включать отдельным этапом после фиксации baseline. Никакой установки на UFS не было.

Пользователь опроверг наложение текста непосредственно после подготовки отчёта. Pending confirmation72f480d8-ad5a-46a4-8e26-020a6aadd2dd содержит ошибочную гипотезу; её не следует считать актуальной. Требуется отмена либо корректирующий комментарий при публикации.

Ошибочный отчёт72f480d8-ad5a-46a4-8e26-020a6aadd2dd отменён через confirmation_reject до публикации. Подготовлен исправленный отчёт с уточнением пользователя.

## 2026-09-07: подготовлен отдельный SMP-тест

После подтверждённого userspace на CPU0 добавлен новый ручной пункт GRUB `EXPERIMENTAL: Linux RAM test - eight CPUs, no UFS`. Единственное изменение kernel cmdline: maxcpus=1→maxcpus=8. Прежний пункт полностью сохранён. Image, DTB и initramfs не менялись; проверены hashes5 исходных payload-файлов на USB. Консоль/keep_bootcon/cpuidle.off не менялись; утверждение о конфликте консолей отозвано после уточнения пользователя.

USB disk6/TCLDIAG D: обновлена без форматирования; EFI SHA2560a575ca6269c6dd4afa278155f892e32dd57f2affb69ca7492044a2ee31a140f проверен чтением с USB. Предыдущий EFI сохранён как EFI/BOOT/BOOTAA64-onecpu.bak. Файлы и результат: kernel-smp/.

Критерий следующего запуска: REACHED USERSPACE, CPU online0-7. Это проверка запуска SMP, не стресс-тест, не проверка cpuidle/cpufreq/терморежима и не периферии. Ноутбук автоматически не перезагружался. Ожидается ручной запуск пользователем и фотография результата.

SMP-отчёт ожидает подтверждения Lavtomate:64fa1c18-f03c-4d2e-8432-595e70cec460,3минуты. Исправленный отчёт об успешном первом запуске88118559-705f-48a7-a714-6a9b211c234c при последней проверке также pending.

## 2026-09-07 00:28: SMP-тест успешен

Из /tmp на lav@192.168.8.186 получен новый снимок IMG_20260907_002809584.jpg (2613921байт); сохранён в данных. Файл назывался IMG_*, поэтому первоначальный поиск photo*.jpg его не показывал; найден при просмотре обычных файлов верхнего уровня /tmp. Текстовая расшифровка kernel-smp/photo-transcript.txt.

Подтверждены баннер userspace, выполненный /init, CPU online **0-7**. Linux6.18.34-stb-qc7+ успешно загрузил все8CPU на минимальном DT. MemTotal7665264kB совпадает с первым запуском, MemFree7619804kB на момент фото. Framebuffer прежний:0x9bc00000–0x9c3e8fff,1920×1080×32,stride7680. Адрес совпал в двух тестах, но это не гарантия для других режимов/версий firmware.

Изменение относительно первого запуска только maxcpus1→8; подтверждён запуск SMP и userspace. Не проверены нагрузочная стабильность, cpuidle/cpufreq/терморежим, USB/клавиатура/UFS/GPU. Искажения фото не трактуются как наложение на реальном экране с учётом уточнения пользователя. Следующий этап — поочерёдное подключение периферии к сохранённому рабочему SMP baseline.

Отчёт об успешном SMP ожидает подтверждения:19de9e93-d37b-41a9-93e7-a574e4c5a484,2минуты.

## 2026-09-07: объединённый тест USB/HID подготовлен

Пользователь подтвердил: встроенного Ethernet нет, есть Wi-Fi. Windows inventory (`peripherals/windows-network.json`) показывает активный Qualcomm QCMS DEV_082B и виртуальный ROOT/KDNIC с Not Present. USB Ethernet драйверы и DHCP добавлены для возможного внешнего адаптера; встроенный Wi-Fi требует отдельного согласования firmware/remoteproc/QMI/памяти и в этот DT не включён.

Собран `peripherals/sc7180-tcl-usb-hid.dtb` на основе Linux v6.18 SC7180 с USB DWC3/PHY, I2C4 и HID клавиатурой/тачпадом. UFS/GPU/модем отключены. Исправлены отсутствующие ссылки на carveouts отключённых устройств; SMP2P отключены. Dtc завершается без ошибок; предупреждения об альтернативных выключенных QUP и DSI описаны в README. Это техническая проверка, а не подтверждение работы периферии.

Питание USB по DSDT/PEP D0: L11_A=1800000, L17_A=3088000, L3_C=1200000, L4_A=880000 мкВ. Совместимость PM6150/PM6150L и имена регуляторов сверены через gh с upstream qcom-rpmh-regulator.c, исходник сохранён. Клавиатура I2C0x05/HID0x20/IRQgpio33LOW/enable32HIGH; тачпад0x2c/HID0x20/IRQgpio94LOW/enable25HIGH. Вымышленное напряжение GPIO-питания не задавалось.

Initramfs ~15МБ: прежний BusyBox ARM64 и модули точной версии ядра, все зависимости проверены. RAM shell, сводка USB/I2C/input/network/interrupts/deferred probes/regulators, dmesg; ограниченная запись событий ввода 60секунд. Каждые30секунд сохраняет данные только на TCLDIAG с подтверждённым USB serial в tcl-logs/<boot UUID>/, sync/umount. При раннем зависании/неработающем USB автосохранения не будет. SSH-сервера в Linux-тесте пока нет. Прежние два теста CPU сохранены; новая запись GRUB отдельная. README содержит процедуру и ограничения.

**USB обновлена успешно:** disk6 D:, проверены3 новых файла; EFI SHA2568c3df2be1b7500a7be30996751b809449f8a21e5ef2332f44d991036743e0707. Прежний EFI сохранён как EFI/BOOT/BOOTAA64-smp.bak. Результат peripherals/usb-update-result.json. Ожидается ручная загрузка нового пункта; успех периферии пока не установлен. SMP-отчёт опубликован comment180902, confirmation19de9e93 approved.

Отчёт подготовки USB/HID опубликован в Bugzilla19084: comment180903, confirmationf848344b-808d-471c-874a-45fbbaa28c93 approved,15минут (оценка).

## 2026-09-07 01:04: первый USB/HID тест завис до userspace

Получено с lav@192.168.8.186:/tmp фото photo_2026-09-07_01-04-58.jpg; сохранено в peripherals/, расшифровка hang-photo-transcript.txt. Пользователь сообщил о зависании. Последняя видимая строка `[4.749119] AppArmor: AppArmor sha256 policy hashing enabled`. На снимке нет panic/stack trace/баннера USERSPACE. Это не доказывает причину в AppArmor: последний напечатанный этап мог завершиться, а следующий зависнуть без сообщения.

Подтверждён прежний framebuffer0x9bc00000/8100KiB/1920x1080/stride7680. Регистрация usb-storage/usbhid — регистрация драйверов, не доказательство обнаружения USB устройств. Повторение блока в фотографии не объявляется причиной зависания или наложением на живом экране.

Подготовлен peripherals/initcall-debug/: отдельный пункт DEBUG: USB/HID initcall trace (no UFS), единственное изменение параметров initcall_debug. Image/DTB/initramfs прежние. В Linux v6.18 init/main.c через gh проверены сообщения calling/returned. Следующий снимок должен помочь локализовать незавершённый initcall; асинхронные probes могут потребовать дальнейшей трассировки. Windows SSH восстановлен пользователем, новый EFI передан; запись/проверка USB выполняется.

**Initcall debug записан:** disk6 D:, EFI c900b48d4b871ce7acfcb4e00c809415d304046f1ef20ae17d32ca590706d1de, оба прежних peripheral payload проверены; backup EFI/BOOT/BOOTAA64-peripheral-v1.bak. Ожидается ручной тест. Отчёт о зависании опубликован comment180904, confirmationac722daf-4cdd-4400-8e69-f393bd820676 approved.

## 2026-09-07 01:14: локализовано ожидание deferred probe

Новое фото photo_2026-09-07_01-14-26.jpg скопировано с lav@192.168.8.186:/tmp в peripherals/initcall-debug/. Ключевые строки вручную расшифрованы в probe-hang-transcript.txt.

`init_profile_hash` (AppArmor) вернул0. `sync_state_resume_initcall` также вернул0. Затем `[12.882829] calling deferred_probe_initcall+0x0/0xc0 @ 1`; его возврат на снимке отсутствует. Внутри видны успешные probes qcom-socinfo, smem, power-domain-cpu0..7, GDSC и последним `[12.953013] probe of 100000.clock-controller returned 0 after 21530 usecs`. Это указывает на остановку в стадии повторного probe/ожидания очереди, но не определяет зависший драйвер. GCC завершился успешно. ufs_phy_gdsc — регистрация power domain GCC, не включение контроллера UFS.

Исходник drivers/base/dd.c v6.18 проверен через gh: initcall_debug печатает probe только после возврата; строка перед probe — dev_dbg. CONFIG_DYNAMIC_DEBUG и CONFIG_DYNAMIC_DEBUG_CORE в готовом ядре отключены; параметр dyndbg не предлагается как рабочая диагностика.

Подготовлены три варианта peripherals/probe-isolation/: infrastructure (отключены I2C4/USB wrapper/обаPHY), usb-only (отключён I2C4), hid-only (отключены USBwrapper/PHY). Общая инфраструктура SCM/SMEM/RPMh/GCC/interconnect/TLMM/SMMU/QUP остаётся во всех вариантах. Это не минимальный CPU baseline. Все три DTB компилируются; Image/initramfs прежние, initcall_debug сохранён. Сначала infrastructure: если зависает — искать ниже leaf USB/HID; если userspace — сравнивать две ветки. При выключенном USB сохранения на флешку не будет, нужны фото. Пользователь восстановил SSH; комплект передан, обновление USB выполняется.

**Probe isolation записан:** disk6 D:, EFI8adf47b1c45e700dd6175b406bca9c1a02a60c764cd7bc9c7cd1d74c5d9d8c3c, проверены4файла, backupEFI/BOOT/BOOTAA64-initcall.bak. Ожидается ручной запуск infrastructure. Отчёт Bugzilla confirmation2c0ca0bd-ad65-438f-8811-ffb523626f56 approved, но result=null; публикацию/номер комментария следует проверить, не дублировать.

## 2026-09-07: hid-only и infrastructure также зависают (со слов пользователя)

Пользователь явно сообщил о зависании hid-only. После инструкции запустить infrastructure ответил «опять то же самое»; по контексту это результат infrastructure, но имя выбранного пункта и точные последние строки новым фото не проверены. Нельзя приписывать этим двум запускам точные времена/последний probe из предыдущего фото.

Если выбран infrastructure, зависание воспроизводится без USB/PHY и I2C4, поэтому подключённые leaf USB/HID не являются необходимым условием. Это не доказывает их исправность и не локализует конкретный общий драйвер.

Проверен upstream drivers/nvmem/qfprom.c v6.18 через gh: при дополнительных reg ресурсах probe читает QFPROM_VERSION_OFFSET из security MMIO. В общем DT QFPROM включён с четырьмя reg диапазонами и зависимостью GCC core clock. QFPROM — конкретная гипотеза для проверки после последнего успешного GCC probe; порядок очереди по фото не известен, виновник не объявляется установленным. Также CONFIG_FTRACE отключён, как и DYNAMIC_DEBUG.

Подготовлены core-isolation/ и два независимых изменения относительно infrastructure: no-qfprom (отключён только qfprom), no-smmu (отключён только apps_smmu). Во втором USB/UFS и другие DMA-потребители по-прежнему выключены: это не попытка пустить USB DMA в обход IOMMU. Обе DTB компилируются, готов EFI и скрипт обновления с serial/hash/backup проверками. Сначала no-qfprom. Журнал на USB не запишется, нужны фото. SSH сейчас timeout; комплект пока только локально, флешка не обновлена.

Отчёт этапа: confirmationaa52f7a5-4c53-4f1a-bfe9-85b83aead4ba approved, result=null; номер/факт публикации проверить позже, не дублировать.

**Core isolation записан после восстановления SSH:** disk6 D:, EFI7dbf5a3deb94546e4964313be23b2d3a3507fb8490e084cc1f35a282e429c14e; проверены3файла после записи; backup EFI/BOOT/BOOTAA64-probe-isolation.bak. Проверено: каждый новый DTS отличается от infrastructure одним status=disabled. Первый запуск CORE: no-qfprom, затем при необходимости CORE: no-smmu; USB-логирования в этих вариантах нет. Результат core-isolation/usb-update-result.json.

Проверена публикация прежних отчётов: confirmation2c0ca0bd → comment180905; confirmationaa52f7a5 → comment180906, оба approved.

Отчёт о записи CORE: confirmationd65c0d83-d73a-439a-9912-71094ea91443 pending в Lavtomate; не дублировать, проверить позже.

## 2026-09-07: оба CORE варианта зависли; подготовлен поочерёдный probe

Пользователь сообщает: no-qfprom и no-smmu оба зависли так же. Точные строки/таймстемпы этих запусков новым фото не подтверждены. Ни одно отдельное отключение не устранило симптом; это не исключает взаимодействия/несколько причин. Перестать трактовать QFPROM или SMMU как единственную установленную причину.

Вместо очередного исключения групп подготовлен manual-probe/: исходный полный USB/HID DT и прежний Image, добавлен initcall_blacklist=deferred_probe_initcall, новый небольшой initramfs. Исходники Linux v6.18 init/main.c, drivers/base/dd.c, bus.c проверены через gh: blacklist доступен при CONFIG_KALLSYMS=y, drivers_probe вызывает device_attach(...false), не разрешая async для непосредственного вызова. Дочерние устройства могут порождать собственный probe. В готовом ядре CONFIG_STACKTRACE=y, но DYNAMIC_DEBUG/FTRACE отсутствуют.

Идея: пропустить зависшую автоматическую очередь, достичь userspace и через10секунд выполнить до3проходов по непривязанным platform-устройствам, печатая BEGIN device=... до записи в /sys/bus/platform/drivers_probe и END после. Отдельный процесс каждые20секунд пытается показать последний запрос и /proc/<worker>/stack. При глобальной блокировке firmware он тоже может не работать. Порядок отличается от штатной очереди, автоматический deferred probe остаётся отключённым; успешный тест не является исправлением normal boot. Диски не монтирует, сеть/модули не запускает, вывод только на экран. Нужно фото последнего BEGIN и стека либо результата завершения.

Проверены shell syntax, CPIO/наличие init и manual-probe, ARM64 BusyBox. Собраны EFI, manifest, скрипт обновления с проверками serial/hash/backup. Windows SSH оказался доступен; комплект передан, запись выполняется. Новый пункт TRACE: manual platform probes (no UFS). Существующие меню сохранены.

**Manual probe записан:** disk6 D:, EFIffdd7d5be6d435fc4808e326239e1938377537576015a992f2ecf034e91f2a91; проверены2файла; backup EFI/BOOT/BOOTAA64-core-isolation.bak. Ждём ручной запуск TRACE: manual platform probes (no UFS). Отчёт опубликован comment180908, confirmation1b22ff48-f22a-482a-b24a-77be451ef24c approved.

## 2026-09-07 01:50: manual probe локализовал остановку на TLMM

Фото photo_2026-09-07_01-50-37.jpg скопировано с lav@192.168.8.186:/tmp в peripherals/manual-probe/, ключевые строки в photo-transcript.txt. Подтверждены USERSPACE и manual round1. GCC100000.clock-controller завершился rc0/driver gcc-sc7180; syscon1f60000/1fc0000 вернулись rc0still-unbound. Последнее BEGIN device=3500000.pinctrl, END отсутствует. На снимке нет watcher/стека; по одному фото не устанавливаем ни время ожидания, ни глобальную блокировку. Конкретный callback внутри TLMM ещё не известен.

**Обнаружен мой пропуск board restriction:** исходный TCL DTS и upstream QC710 задают gpio-reserved-ranges=<58 5> в TLMM с комментарием о возможной TZ-защите GPIO для fingerprint. В собранном мной peripheral DT это ограничение отсутствовало. Linuxv6.18 gpiolib.c (получен через gh) применяет reserved ranges до обхода get_direction разрешённых линий; pinctrl-msm.c get_direction читает MMIO control. Следовательно, пропуск имеет конкретный механизм, согласующийся с остановкой на TLMM. Точный GPIO/инструкция и исправление аппаратным запуском пока не доказаны.

Подготовлен peripherals/tlmm-reserved/: новый DTB, сравнение декомпилированных DTB подтвердило единственное отличие gpio-reserved-ranges=<58 5>. Пункт FIX TEST: TLMM reserved GPIO 58-62 (USB/HID) с нормальной очередью и прежним logging initramfs; запасной TRACE: manual probes with TLMM reserved GPIO с прежним manual initramfs. Image не менялся. EFI/manifest/update script собраны, но на флешке пока предыдущий комплект. Пользователь включает SSH.

Отчёт TLMM: confirmation41e8cb5f-63e2-4d1d-9059-d8b706850004 approved, result=null; номер комментария проверить позже. Пользователь сообщил SSH готов; исправление передано, запись выполняется.

**TLMM reserved test записан:** disk6 D:, EFI1c5029398c50a3a8b674b8fa830170a765c8d283ebebb8fac1d8351d47e65451;2файла проверены чтением после записи; backup EFI/BOOT/BOOTAA64-manual-probe.bak. Ждём запуск FIX TEST: TLMM reserved GPIO 58-62 (USB/HID). Отчёт опубликован comment180909. Успех исправления пока не подтверждён.

## 2026-09-07 01:59: manual probe с исправлением прошёл, появились HID и USB storage

Получено фото photo_2026-09-07_01-59-38.jpg из /tmp на lav@192.168.8.186, сохранено в peripherals/tlmm-reserved/. Расшифровка photo-transcript.txt. На фото rounds2/3 и `TCL MANUAL PROBE PASS FINISHED`; это **manual initramfs**, не обычный FIX TEST с logging initramfs. Точный выбранный GRUB пункт/cmdline не видны; corrected TRACE следует из последовательности. Поэтому отсутствие USB saved ожидаемо, логирования в этом initramfs нет. Предыдущую реплику об успехе не распространять на обычную deferred queue.

Подтверждено обнаружение I2C HID тачпада093A:0255 (Mouse/Touchpad) на4-002c и клавиатуры0CF2:9020 на4-0005 (также Wireless Radio Control). xhci обнаружил high-speed USB, hub4ports, usb-storage, Kingston DT101G2 как sda/sda1,15644912×512=8010194944байт. Это обнаружение block device, не проверка чтения/записи файлов или USB3скорости. supplyvdd dummy сообщения согласуются с намеренно неописанным отдельным регулятором HID, напряжение не выдумывалось.

Со слов пользователя работают ввод с клавиатуры и скроллинг; верхние5строк не меняются. Это новое реальное наблюдение, не переоценка прежних фото с опровергнутым наложением. Причина артефакта ещё не установлена; keep_bootcon/две консоли остаются гипотезой. Manual init не предоставляет shell, поэтому ввод не объявляется выполнением команд. Следующий тест — уже имеющийся FIX TEST: TLMM reserved GPIO58-62 (USB/HID), обычная очередь драйверов и запись логов. Новых файлов для этого не требуется.

Отчёт manual success: confirmation28bf71b7-188d-4b0e-9e1c-74dd359d22a7 approved, result=null; публикацию/номер проверить позже, не дублировать.

## 2026-09-07: обычная загрузка BusyBox; ошибка blkid и запрос Wi-Fi

Пользователь сообщил о запуске BusyBox после инструкции FIX TEST, затем `/diagnose: line 10: blkid: not found`, USB saved=no. Наличие /diagnose соответствует logging initramfs; обычная загрузка до userspace подтверждена сообщением пользователя, точная cmdline/фото ещё не получены. Ошибка логирования — мой дефект комплектации: в сохранённом ARM64 BusyBox applet blkid отсутствует (проверено в бинарной таблице имён); это не доказательство неисправности USB.

Подготовлен logging-v2/: из diagnose/snapshot убраны вызовы blkid. Поиск USB по serial001CC0EC34E4FBB085C323F2; раздел сначала монтируется readonly, проверяется точное содержимое tcl-diag/TCL-RAM-TEST, только затем remount rw. saved=yes после успешных copy/sync/umount; счётчик input исправлен. Проверены shell syntax и наличие требуемых applets в конкретном бинарнике; список сохранён. Новый initramfs/EFI/manifest/update script готовы локально, не записаны. Предложено вручную сохранить /run/dmesg.txt и /run/report.txt с текущего запуска на известную Kingston sda1, затем sync/umount.

Пользователь просит сеть. Текущий DT Wi-Fi disabled; в ядре ATH10K_SNOC=m, QMI/QRTR включены. OEM экспорт содержит wlanmdsp.mbn и множество bdwlan/bdwlanu, поэтому исходные файлы уже локально. DSDT AMSS.QWLN CRS подтверждает MMIO18800000/800000 и MSA93900000/200000; последняя входит в verified EFI reserved85b00000–945fffff. Совпадает с исходным TCL и upstream Acer SC7180. Исходник ath10k-snoc.c и Acer DTS v6.18 получены через gh, сохранены wifi-research/. Acer включает remoteproc_mpss с qcmpss7180_nm.mbn; путь firmware/QMI, board ID/файл и PEP питание ещё нужно согласовать. Не объявлять wlanmdsp достаточным, не выбирать bdwlan наугад. Wi-Fi image пока не подготовлен; интерфейс под Linux ещё не обнаружен.

Отчёт normal/logging/Wi-Fi подготовка: confirmation404e5ef5-f672-4452-8286-61fc6a7fd84f pending; не дублировать, проверить позже.

## 2026-09-07: получены полные журналы обычного успешного запуска

После возвращения Windows с D: скопированы dmesg.txt125421байт и report.txt11491байт в peripherals/normal-boot-logs/. Файлы были вручную записаны из Linux; их чтение из Windows подтверждает файловую запись на Kingston и последующее чтение, не только обнаружение block device.

Cmdline содержит initcall_debug, НЕ содержит initcall_blacklist. `[12.962464] probe of 3500000.pinctrl returned 0 after1089usecs`; `[13.174454] initcall deferred_probe_initcall returned0 after283258usecs`; `[13.761930] Run /init as init process`. Обычный deferred probe с исправленным DT успешно завершился. CPU online0-7, MemTotal7664780kB. Связь исправления gpio-reserved-ranges58/5 с устранением нашего зависания подтверждена сравнением вариантов; конкретное запрещённое MMIO не измерено.

Report: оба HID привязаны к i2c_hid_of; клавиатураGPIO33 IRQ384срабатывания, тачпадGPIO94 IRQ1 на момент снимка, I2C637 IRQ411. Это не полная проверка всех жестов тачпада. SMMU зарегистрирован, счётчики fault IRQ0. Deferred probes список пуст. USB: Kingston0951:1642/serialсовпадает/speed480, камераSunplusIT13d3:784b/speed480 (камера только обнаружена, видеозахват не проверен). Контроллер объявляет SuperSpeed, но флешка работает наUSB2. Сеть только lo; Wi-Fi DT выключен. Осталась ошибка aux_bridge -ENODEV failed to acquire drm_bridge, не блокирует userspace/efifb. Dummy HID supplies ожидаемы для текущего описания; отдельное питание ещё не измерено.

logging-v2 передан на Windows; обновление Kingston с проверкой хешей выполняется. Ядро/исправленный DT прежние, меняются только диагностический initramfs и добавленный пункт меню.

**logging-v2 записан:** disk6 D:, EFIaa16c5b6058643e543746446fc7b608cada89ea576a0a6dc90629cc8f22ca20a; проверены2файла, backup EFI/BOOT/BOOTAA64-tlmm-reserved.bak. Новый пункт USB/HID: fixed logging (TLMM reserved). Автоматическая запись v2 пока не проверена загрузкой.

Попытка приложить объединённые полные dmesg/report к Bugzilla19084 отклонена: tool bug_add_attachment вернул rejected, "Action was rejected by the user." Вложение не опубликовано; повторять без нового разрешения нельзя. Исходные файлы сохранены локально.

Комментарий по полным логам/logging-v2: confirmationd3686fbe-34a3-4ae6-beff-3b13371cd5bb pending; не дублировать.

## 2026-09-07: подготовлен NEXT Wi-Fi firmware / board-ID

Вложение полных normal-boot логов подтверждено: attachment7664, confirmation820d7b8f-b0b7-47e1-9ca8-9defa254bd26 approved. Пользователь поручил новый пункт следующего шага; уточнено, что клавиатура/тачпад I2C, USB — флешка/хаб/камера.

Собран peripherals/wifi-probe/: исправленный USB/HID DT дополнен Wi-Fi WCN3990, MPSS PAS/SMP2P и carveoutsMPSS86000000/32MiB, WLAN93900000/2MiB. Wlan адрес изACPI; MPSS изTCL/Acer и проверенных PT_LOAD фактического OEMqcmpss7180_nm.mbn. Все загружаемые сегменты/файловые данные помещаются в область/файл; у последнего BSSfilesz0 offset заEOF не требует чтения. Оба carveouts внутри verifiedEFIreserved85b00000–945fffff. Ядро не менялось; UFS/GPU/прочие remoteprocs отключены.

Питание mappingTCL/Acer: L9_A664000uV по COEXPEP, L1_C1800000/L2_C1304000/L10_C3304000 по shared BluetoothPEP. Это свидетельства общих шин, не измеренная последовательность Wi-Fi. Для L11_C численное напряжение не навязано — TCL vote пока не установлен. Упоминать эту ограниченность при оценке результата.

PAS descriptorSC7180MPSS auto_boot=false подтверждён исходникомv6.18 черезgh. Initramfs содержит точный экспортированный qcmpss7180_nm.mbn по путиqcom/sc7180/tcl/. Скрипт ждёт успешной записи USBлоговдо120сек, загружаетath10k_snoc/qcom_q6v5_pas, запускает единственныйremoteproc с точнымfirmwareимением, ждёт30сек и снимает QMI/firmware/boardID/интерфейсы. Logger независимо копируетwifi.log каждые30сек. Без успешной USBзаписи Wi-Fiстарт откладывается. Board-2.bin не угадывается, Windowsbdwlanварианты пока архивированы; ожидаемый результат — состояниеMPSS/QMI иboardID либо объяснимая ошибка, не обещание подключениякAP. Пароли/association/supplicant не настроены.

Проверены DTкомпиляция/shellsyntax/CPIOструктура/firmwareбайты/модули. Новый пункт NEXT: Wi-Fi firmware and board-ID probe, рабочие пункты сохранены. Комплект передан наWindows, обновлениеUSB выполняется. Ожидаемый ручнойтест около3минут, затемфото/логи.

**Wi-Fi probe записан:** disk6D:,EFI44f91e637bb908a51608e315eb85201d91cb02ad8d34c5315337f88d25ab6cf9; проверены3файла после записи; backupEFI/BOOT/BOOTAA64-logging-v2.bak. Ждём аппаратный тест, подключенияWi-Fi пока нет.

Отчёт Wi-Fi probe: confirmation32904d09-8970-47ca-9808-5a42a1afa0d1 pending; не дублировать.

## 2026-09-07: логи первого Wi-Fi probe получены, MPSS watchdog

С WindowsD:/tcl-logs/becedc85-2930-4686-ae61-5d50ec838ffc/ скопирован весь каталог в wifi-probe/logs/. Report19649байт,dmesg125554,wifi.log3061;event0..3.bin пусты (не доказательство отказа ввода: окно записи ограничено). Хеши сохранены. Таким образом автоматическое USBлогированиеv2 подтверждено, а не только ручная запись.

ath10k_snoc modprobe rc0; `[56.585238] probe of18800000.wifi returned0` и sysfs driver→ath10k_snoc. Сам драйвер успешно привязан, но phy/сетевой интерфейс не зарегистрирован (толькоlo), QMIboardID в журнале нет. Deferred probes список пуст. Regulatory.db отсутствует (-2) — отдельный пробел initramfs, не установленная причина MPSSwatchdog. Ошибка чтения /proc/net/qrtr отражает ошибочное ожидание этого procfile в диагностике; не доказательство отсутствия QRTR (IPCRTR endpoint зарегистрирован).

MPSS PASпрошивкаqcmpss7180_nm.mbn стартовала: `[59.907375] remote processor modem is now up`, IPCRTR иglink_ssr привязались. Затем `[99.919217] ... fatal error ... dog_hal_common.c:173:DOG detects stalled initialization, triage with IMAGE OWNER`. Через ~40сек после каждого старта повторяется сбой. В конце сохранённогоdmesg crash#17 (~747сек). Автовосстановление запускает ядро remoteproc, хотя наш shell делает лишьоднукомандуstart. Раннийwifi.log отражает running до первогоwatchdog; полноеdmesg устанавливает дальнейшую неустойчивость. Не объявлять MPSSготовым по state=running.

Следующий анализ — недостающие Qualcomm userspace services/запросыfirmware. README linux-msm/tqftpserv получен черезgh: обслуживает запросы другихDSP черезQRTR, переводит readonlyfirmware paths в/lib/firmware. В этомinitramfs tqftpserv отсутствует; его роль в этом зависании ещё гипотеза до логов запросов. qcom_common.pd-mapper.0 уже есть в ядре, не объявлять pdmapper отсутствующим. Следующийтест должен отключить автоматическое recovery доstart, чтобы получить один сбой и ясный журнал, а не цикл.

Пользователь разрешил для будущих тестов автоматическую перезагрузку после окончания диагностики. Требование принято: сначала сохранитьлоги,sync,размонтироватьфлешку; обратный отсчёт с отменой. В уже записанном Wi-Fi пункте этого ещё нет. Нового EFI в этом этапе не записывали.

Отчёт Wi-Fi результатов: confirmationa53ec0b0-891d-42bf-b1bf-e2ba4a1b81d7 pending. Попытка вложить объединённые Wi-Fiлоги отменена пользователем (tool: user cancelled MCP tool call); не повторять без нового разрешения, локальные файлы сохранены. На вопрос о firmware: возможно отсутствуют дополнительные firmware-файлы/служба их доставки; в initramfs только MPSSmbn, а wlanmdsp.mbn и bdwlan уже экспортированы локально. Недостающий конкретный файл ещё не установлен — нужен журнал запросов tqftpserv.

## 2026-09-07: подготовлен и записан Wi-Fi firmware services + auto reboot

По указанию «Так делай» собран новый `peripherals/wifi-services/`. В initramfs добавлены готовые ARM64 Debian tqftpserv 1.1-4, qrtr-tools/libqrtr 1.1-2+b1, libc6 2.41-12+deb13u4, libzstd1 1.5.7+dfsg-1, stdbuf/libstdbuf из coreutils 9.7-3. Службы не компилировались и не изменялись. Подробности и SHA256 пакетов/firmware — в README.md рядом с образом. stdbuf обеспечивает немедленную запись stdout, который tqftpserv иначе буферизует при перенаправлении в файл.

MPSS firmware дополнена wlanmdsp.mbn и всеми 14 OEM bdwlan/bdwlanu; выбранного board-2.bin пока нет. Сначала успешное USB-логирование, затем qrtr-ns -f 1 и tqftpserv с проверкой живых процессов; перед единственным стартом MPSS выставляется recovery=disabled с чтением обратно. Наблюдение 90 секунд, включая прежний 40-секундный watchdog; qrtr-lookup вместо отсутствующего /proc/net/qrtr. Отсутствие regulatory.db пока остаётся отдельным ограничением. Причина watchdog ещё не доказана.

После завершения теста — 30-секундный отсчёт с командой cancel-reboot. Перезагрузка разрешена только после отдельного финального сохранения логов, sync и успешного umount. При незавершённом тесте или отсутствии подтверждения записи перезагрузки нет. Mock-проверки четырёх условий прошли. Проверены синтаксис shell, ELF AArch64/интерпретатор/все DT_NEEDED, CPIO и точность firmware. Аппаратная проверка служб ещё впереди.

USB обновлён успешно: Disk6, D:, Kingston TCLDIAG; проверены два файла после записи. EFI SHA256 a284ce394c9f70b615a18c29eb8474d7ecac562351324bcf06e6f2e576e34630; initramfs SHA256 317bbe27d4f701a86d86ccdcc5b88030cecdb67e386263abefd6700479a89d88. Backup EFI/BOOT/BOOTAA64-wifi-probe.bak. Новый пункт `NEXT: Wi-Fi firmware services + auto reboot`; прежние пункты сохранены. DT и ядро прежнего Wi-Fi probe. Windows не перезагружался по SSH. Следующий ручной тест ~3–4 минуты; после reboot UEFI может снова выбрать USB, для Windows убрать флешку после размонтирования/перезагрузки.

## 2026-09-07: результат Wi-Fi firmware services

После возврата пользователя в Windows скопирован каталог D:/tcl-logs/47d3ae97-57e4-470b-934c-10600f557714 в peripherals/wifi-services/logs/, вычислены SHA256. dmesg125386 байт, report20029, wifi3441, qrtr-ns81, tqftpserv0, autoreboot458; input event файлы пусты.

MPSS up64.912522, watchdog104.924514 (40.012 секунды): прежний dog_hal_common.c:173 stalled initialization. Ровно crash#1, итог remoteproc crashed/recovery disabled. Wi-Fi phy отсутствует, только lo. Добавление OEM firmware и tqftpserv само по себе сбой не устранило.

qrtr-lookup: node0 service43 v2 instance18 (Subsystem control), service66 v1 instance180 (Service registry notification); node1 service4096 TFTP port16384, service64 locator port16387. Обмен QRTR частично работает, но WLAN QMI service не виден в конечном снимке. Журнал tqftpserv пуст, /tmp/tqftpserv пуст; зарегистрированный TFTP ещё не означает, что модем получил firmware. Нет зафиксированных RRQ/WRQ, конкретный отсутствующий файл пока не установлен.

qrtr-ns: nameserver already running, going dormant: Address already in use. Проверен исходник точно v1.1 через gh, сохранён qrtr-ns-v1.1.c: EADDRINUSE закрывает сокет и переводит процесс в бесконечный sleep. Значит прежняя проверка kill -0 подтверждала только существование процесса, а не работающий userspace nameserver. qrtr-ns -f 1 также вызывает qrtr_set_address(1) до bind; влияние изменения локального node на обмен требует отдельной проверки, не установленная причина watchdog. Источник https://github.com/linux-msm/qrtr/blob/v1.1/src/ns.c . Не следует запускать второй nameserver без необходимости; следующий диагностический вариант должен учитывать встроенный NS и снимать QRTR до старта/во время запуска, а не только после crash.

autoreboot.log содержит весь отсчёт и Requesting final USB save and unmount. Последняя строка после подтверждения umount по конструкции остаётся в RAM и в сохранённый лог не попадает; сам факт автоматического (а не ручного) reboot этим файлом не доказан. Пользователь сообщил возврат в Windows. EFI в этом этапе не менялся. Regulatory.db -2 остаётся отдельной ошибкой.

Отчёт этого результата в Bugzilla ожидает подтверждения Lavtomate: a3df134c-eaff-4c86-beb1-5d15a717dba2. Не дублировать. Предыдущий отчёт подготовки: aa4e3214-d9ac-4323-989e-552c80ef830b.

## 2026-09-07: код ядра и поиск аналогов после services-теста

Проверены upstream v6.18 (через gh): ath10k/snoc.c, ath10k/qmi.c, qcom_q6v5.c, qcom_pd_mapper.c, net/qrtr/ns.c и af_qrtr.c; qrtr/src/addr.c версииv1.1. Это upstream-база, не доказательство идентичности всем downstream-патчам экспериментального 6.18.34-stb-qc7+.

Цепочка: ath10k_snoc_probe -> ath10k_qmi_init -> qmi_add_lookup(WLFW). Probe0 подтверждает создание клиента, но не готовность Wi-Fi. При появлении сервиса: регистрация уведомлений -> host capabilities -> MSA memory info/permissions/ready -> capability (chip/board/fw IDs). После MSA_READY ядро получает board-файл через ath10k_core_fetch_board_file и передаёт его через QMI BDF download. FW_READY приводит к ath10k_core_register. Поэтому отсутствие phy/boardID согласуется с остановкой до этих событий; конечный QRTR-снимок не исключает кратковременное появление службы раньше.

Уточнение предыдущего объяснения: board data в этом ath10k передаёт ядро по QMI; это не обязательный TFTP-запрос. tqftpserv обслуживает запросы дополнительных файлов от DSP, в текущем тесте RRQ/WRQ отсутствуют. wlanmdsp специфичен для платформы, универсальная замена WCN3990 из другой SoC неверна: commit https://kernel.googlesource.com/pub/scm/linux/kernel/git/srini/linux-firmware/+/a0142c57045701b7557c3060af5c4246c420e4d8 .

qcom_q6v5.c q6v5_fatal_interrupt читает текст причины из SMEM и вызывает rproc_report_crash; dog_hal_common.c — исходник закрытой firmware, не Linux-драйвера. Ядро не содержит причину ожидания внутри DSP.

**QRTR адрес:** af_qrtr.c задаёт qrtr_local_nid=1. qrtr v1.1 addr.c делает getsockname и сразу возвращается, если адрес уже1. Следовательно прежняя гипотеза, что qrtr-ns -f1 изменил адрес и тем вызвал сбой, не подтверждена. EADDRINUSE/dormant ожидаем при встроенном NS; второй демон лишний, но не установленная причина watchdog. Проверка kill -0 не проверяет работу NS.

**PD mapper:** CONFIG_QCOM_PD_MAPPER=y; root compatible содержит qcom,sc7180; sc7180_domains включает msm/modem/root_pd и msm/modem/wlan_pd (wlan/fw). Лог probe qcom_common.pd-mapper.0=0 на64.840606 до MPSSup64.912522, конечная служба64 также видна. Нельзя говорить, что mapper отсутствует. Найден patch Loic Poulain от2026-09-01 с точно таким watchdog через40с на Agatti/Arduino UNO Q; исправление softdep для порядка загрузки модульного mapper не переносится автоматически на нашу builtin-конфигурацию. Источник https://www.mail-archive.com/linux-kernel%40vger.kernel.org/msg2653217.html (индекс первичной рассылки также https://lists.openwall.net/linux-kernel/2026/09/01/2243 ).

**Рабочий аналог QC710:** публикация Val Packett от2026-01-11 описывает SC7180/QSIP7180P, Windows package qcwlan7180.inf_arm64_38b56292be899426 и bdwlan.b67. Наш SHA256 полностью совпал: da2e615dea087b66889d09ab627e091710a35ce05167b0bd95cf4926196a62e1. В рабочем аналоге qmi-board-id=67/qmi-chip-id=320/variant=ECS_QC710. Это кандидат, но не доказательство ID TCL; variant другой платы в наш DT не добавлен. Источник https://www.mail-archive.com/ath10k%40lists.infradead.org/msg17504.html .

**RMTFS отдельное направление:** текущий DT явно отключает rmtfs_mem. В публикации об SC7180 описана зависимость networking от корректного rmtfs/SCM: https://lkml.rescloud.iu.edu/2102.2/06743.html . Это про иной сбой SCM, не готовое объяснение нашего watchdog. До включения нужны подтверждённые memory reservation и файлы modem storage именно TCL. Сейчас не включали и UFS не трогали.

CONFIG_ATH10K_DEBUG, ATH10K_DEBUGFS и DYNAMIC_DEBUG выключены: включение debug_mask не обеспечит отсутствующий в бинарнике диагностический код. Пока подготовлен сбор ранней последовательности через существующий qrtr-lookup и sysrq-w; вопрос отладочной сборки при необходимости передаётся сборщику пакетов.

Новый QMI timeline записан и проверен: Disk6D:, EFI e6cc4ce7f08913fb860b6d7bc0751d5c14e1211fe27a782ec5f005058c6ba67c, initramfs c299a556ecbdfaa98082ed521c87afa5ef3a983893f7045dd0d48ce9d20b9941, FilesVerified2. Backup BOOTAA64-wifi-services.bak. Пункт NEXT: Wi-Fi QMI timeline + auto reboot. Сбор ранних QRTR-снимков и sysrq-w; ядро/DT/firmware не менялись. Ожидается ручная загрузка ~3–5мин. Отчёт анализа в Bugzilla ожидает подтверждения1d77f759-c9ed-41da-8c3e-9614185da7b8, не дублировать.

## Результат QMI timeline, 2026-09-07

Boot ID a57596de-c8e6-46a0-8182-83820f2098f2. Каталог целиком скопирован с Windows D:/tcl-logs, хеши оригинальных файлов сохранены.

| Наблюдение | Результат |
|---|---|
| Снимки QRTR | 30, все lookup rc=0; период примерно2.03с |
| Первый снимок | MPSS offline; только TFTP4096 |
| Снимки2–20 | MPSS running; службы43/66 node0,64/4096 node1 |
| Снимки21–30 | MPSS crashed; прежние записи служб остаются видны |
| WLAN-служба | Не обнаружена ни в одном снимке; короткое событие между снимками не исключено |
| PD mapper | Probe0 на61.912393с; MPSSup61.985145с; locator64 виден со2снимка |
| Watchdog | На101.996935с, через40.011790с после MPSSup; dog_hal_common.c:173 |
| Recovery | disabled, ровно crash#1 |
| TFTP | Журнал0байт, /tmp/tqftpserv пуст; запросы не зафиксированы |
| SysRq-w | На91.961676/117.052465с; после заголовков нет списка blocked tasks |
| Сеть | Толькоlo, phy отсутствует |

Вывод: отсутствие userspace qrtr-ns не изменило отказ. Гипотезу о конфликте второго NS не подтверждает сравнительный тест. Mapper и служба доставки файлов зарегистрированы уже в ранней фазе, но наличие QRTR-анонса не доказывает успешные ответы на QMI-запросы. Нельзя считать модем рабочим после аварии только потому, что его записи ещё видны в lookup.

SysRq-w показывает лишь задачи в блокированном состоянии, а не все спящие потоки: отсутствие вывода не исключает логическое ожидание на Linux-стороне. Снимки показывают остановку до наблюдаемого WLAN QMI/board ID, конкретная причина внутри DSP пока неизвестна. Отсутствие board-2.bin и regulatory.db остаётся будущей задачей, но не доказанной причиной текущего watchdog.

Часы dmesg и /proc/uptime в этих логах имеют смещение: sample1 uptime57.31 с offline, хотя kernel MPSSstart61.890267. Сопоставлять порядок/интервалы и состояния, не смешивать абсолютные значения этих шкал. Причина смещения не установлена.

Лог autoreboot содержит отсчёт и запрос финального сохранения. Последняя строка после umount по конструкции не сохраняется; пользователь сообщил возврат в Windows. Пустые input event файлы не доказывают отказ HID.

Следующая содержательная диагностика — обмен запросами/ответами QMI и зависимости MPSS (включая проверку потребности в RMTFS и соответствия OEM firmware), а не ещё один тест только с перестановкой qrtr-ns. RMTFS пока не включать без проверки памяти и backing files TCL. Данные/EFI на USB в этом этапе не менялись.

Отчёт QMI timeline результата ожидает подтверждения Lavtomate: 2490c5d6-a0cb-4742-bd2f-345aa1914799. Не дублировать отправку.

## 2026-09-07: приоритет следующего шага — RMTFS TCL

Новые локальные подтверждения: OEM qcremotefs7180.inf привязан к ACPI QCOM0817; строки qcremotefs7180.sys содержат /boot/modem_fsg, /boot/modem_fsc и пути разделов диска. Наличие драйвера не доказывает, что конкретный запуск MPSS ожидает этот сервис.

DSDT.dsl: RMTB=0x80600000, RMTX=0x00200000 (строки128–129), RFMB/RFMS/RFAB/RFAS=0. RFS0._CRS подставляет эти значения в ресурсы QCOM0817. Это отличается от типового rmtfs_mem94600000 в sc7180.dtsi. Диапазон80600000–807fffff уже был виден как EFI reserved2MiB. Поэтому нельзя просто включить типовой узел94600000; требуется описание памяти TCL и проверка прав SCM/VMID. В этом этапе DT/USB не менялись.

Сверены modemr.jsn и modemuw.jsn OEM: root_pd/wlan_pd instance180, службы gps/gps_service, tms/pdr_enabled, kernel/elf_loader, wlan/fw совпадают с таблицей SC7180 kernel PD mapper. tms/servreg ядро добавляет каждому домену автоматически в qcom_pdm_add_domain. Простая замена таблицы mapper не имеет найденного обоснования.

Официальный репозиторий RMTFS https://github.com/andersson/rmtfs проверен через gh. README отсутствует, изучены rmtfs.c/sharedmem.c, локальные копии сохранены. rmtfs.c поддерживает -o storage_root, -r avoid writing to storage, -P partitions; точные semantics хранения нужно сверить по storage.c перед запуском. Предпочтительный тест — копии необходимых modem storage в RAM с корректной общей памятью; внутренние разделы Linux пока не подключать для записи. Сначала собрать read-only карту разделов и убедиться, какие файлы/разделы TCL реально использует; не создавать пустые данные взамен заводских вслепую.

Текущий kernel: KPROBES/FTRACE/ATH10K_DEBUG/DYNAMIC_DEBUG выключены. Полный захват внутренних kernel QMI через эти механизмы нельзя включить одной командой; нужен подходящий готовый отладочный kernel или задача сборщику пакетов. Обычный qrtr-lookup показывает объявления, не трафик запросов. Приоритет: проверка RMTFS по конкретным OEM/ACPI данным, затем отладочное ядро если причина не выявлена. UART полезен для раннего boot, но Linux уже сохраняет логи, поэтому сейчас не обязательное условие.

Отчёт RMTFS-находки ожидает подтверждения Lavtomate8f1e108c-3249-4a5f-b4b9-e894f331e6ad; не дублировать.

## 2026-09-07: RMTFS RAM-тест и Android USB tethering

Подробная документация, таблица разделов, ограничения и SHA256 пакетов записаны в [rmtfs-probe/README.md](../research/peripherals/rmtfs-probe/README.md). Скрытый PhysicalDrive5 содержит modemst1/modemst2/fsg/fsc; копии получены только чтением, два чтения каждого совпали. GPT CRC32 проверены. Данные находятся в tcl-rmtfs-export, в публичную багу не прикладывались.

DT использует RMTFS 0x80600000/2MiB по ACPI/EFI, без SCM-переназначения прав (qcom,vmid отсутствует). Доступ DSP должен подтвердить аппаратный тест. Linux UFS отключён. RMTFS1.1-4 использует -r и каталог копий /rmtfs-storage; записи обслуживаются в RAM shadow buffer. До MPSS проверяются адрес/размер и служба14. Сохраняются RMTFS/QRTR/network логи, одна попытка MPSS и прежняя автоперезагрузка после финального сохранения.

Пользователь подтвердил Android. Добавлен повторный поиск USBnet и DHCP; RNDIS/CDC-ECM/NCM уже включены. Кабель должен поддерживать данные; USB-модем включается на телефоне после Linux boot. Это отдельная сеть: DHCP не доказывает доступность SSH с нашей LAN. SSH-сервер пока не добавлялся. [Инструкция Google](https://support.google.com/android/answer/9059108/share-a-mobile-connection-by-hotspot-or-tethering-on-android?hl=en-GB).

Предложение Ubuntu ARM64 принято как следующий сравнительный вариант с нашим DT и firmware. Ubuntu userspace на рабочем ядре и Ubuntu kernel — разные тесты. Generic ISO не гарантирует поддержку TCL; сейчас Ubuntu не скачивали и не записывали. [Описание Canonical](https://discourse.ubuntu.com/t/what-s-new-for-generic-arm64-desktop-isos-in-25-10/66277).

Запись RMTFS-теста подтверждена: Disk6 D:, FilesVerified3, backup EFI/BOOT/BOOTAA64-wifi-timeline.bak. Новый пункт `NEXT: Wi-Fi RMTFS RAM test + auto reboot`. EFI SHA256 eca465223afdb78ce38205d21f7f73bfd975b08bd68386699255e4b02bb9ac5a; initramfs b79adf96fd4686e7599ca3855df267e4a9397311070ede9a9fa3d73c1c0bc8a9; DT 1629b4148a83d5dbd087b64965c5d481449dd52ef89d84e57e2a8490682ccdc0. Проверены shell, ELF, CPIO, DT и данные. Ожидается ручной запуск.

Отчёт подготовки RMTFS ожидает подтверждения Lavtomate: 075078cf-d3ec-415d-9bf1-bdbd2ce015f1. Не дублировать отправку.

## 2026-09-07: результат RMTFS: пройден прежний watchdog, WLAN QMI появился

Boot835e6cba-8379-4ffb-943c-03f792745b97, полный каталог сохранён локально с SHA256. MPSS up64.264117, после90с наблюдения running/recoverydisabled; прежнего fatal/crash в сохранённом dmesg нет. Это подтверждает устойчивость в окне теста, не длительный тест.

RMTFSservice14 зарегистрирован доMPSS. Модем открыл modem_fs1/modem_fs2/modem_fsg, запросил2MiB по80600000, прочитал через80600200/80600400, включая4094сектора modem_fs2. Ответы0:0. Корректная памятьTCL и данныеRMTFS позволили пройти прежний этап зависания. Роли отдельно исправленногоадреса/службы/данных этим комбинированным тестом не разделены.

WLANservice69 появился. На64.845405 ath10k получил chip_id0x320/family0x4001/board_id0xff/soc0x400c0000; firmware WLAN.HL.3.2.2.c10-00748-QCAHLSWMTPLZ-1. На65.057785 — failed to fetch board-2.bin or board.bin from ath10k/WCN3990/hw1.0. PHY пока нет. В кодеath10k board_info_invalid даёт sentinel0xff; нельзя автоматически назначитьb67 только по аналогииQC710. Следующая задача — обоснованныйOEMboardfile/путь Linux.

TFTP теперь получает запросы wlanmdsp.mbn: сначала два неподдерживаемыхvendorпути, потом /readonly/firmware/image/wlanmdsp.mbn. Есть stat failed и END OF TRANSFER, но также запросrsize3832108; WLANQMIдошёлдоcapabilities. Не объявлять всеTFTPзапросыуспешными или всёнеудачей по одномуstat: нужно сопоставить translate.c и обработку probe/размера/передачи. /readwrite/server_check.txt записанвRAM.

Android RNDIS зарегистрирован на118.251423; usb0=10.254.135.41/24, DHCPserver/router10.254.135.36. Пользователь подтвердил сеть иNAT. Внешнийинтернетпоэтимлогамнепроверен; SSHсервервinitramfsпокаотсутствует. WindowssshdнедоступенкогдаработаетLinux. Для входа нуженDropbear/OpenSSHвLinux; для доступачерезNAT — исходящийобратныйтуннельнаразрешённыйдоступныйсервер. Ничегоэтоговэтомэтапенеустанавливали.

На вопросType-C: обычныйкабельнеобеспечиваетhost-hostсетьсампосебе. НуженgadgetнаоднойсторонеилиThunderbolt/USB4сетьнаобоих. KernelDWC3dual-role/GADGETесть, но маршрутизацияпортаTCL/рольиодновременнаяработафлешкинепроверены. Не переключать контроллер с диагностической флешкой вслепую. Источники: https://www.kernel.org/doc/html/latest/driver-api/usb/gadget.html и https://docs.kernel.org/6.16/admin-guide/thunderbolt.html . Предпочтительна уже подтверждённаяUSBсетьтелефона сSSHтуннелем.

## 2026-09-07: подготовлены OEM boardfile и SSH

По указанию пользователя подготовлен peripherals/wifi-ssh. Подробнее: [wifi-ssh/README.md](../research/peripherals/wifi-ssh/README.md).

OEM bdwlan.bin скопирован в ath10k/WCN3990/hw1.0/board.bin как проверяемый default-кандидат. QMIboardff не трактуется как подтверждённыйID; b67/variantQC710 не назначались. Добавлены firmware-5.bin (Debian20250410-2) и regulatory.db/upstreamподпись. Родной TCL wlanmdsp.mbn сохранён. Отсутствие файла исправлено в образе, работоспособность Wi-Fi ещё не подтверждена.

TFTP stat failed относится к попытке найти сжатый вариант одного кандидата пути, а не обязательно к окончательному отказу; code open_maybe_compressed/translate_readonly проверен. Vendorпути не поддерживаются, модем затем пробует штатный /readonly/firmware/image. Служба не патчилась.

Добавлены Dropbear SSH-сервер и клиент (команда ssh), зависимости, devpts, ключи и логирование. Только root по ключам, без пароля. Личные приватные ключи lav не копировались: использованы их открытые ключи, а host/tunnelключи отдельные диагностические. Вinitramfs private0600/rootSSH0700 проверены.

На lavbook192.168.8.186 установлен ограниченный ключ обратного туннеля, backupauthorized_keys.before-tcl-20260907. Разрешён только reverse listener127.0.0.1:22220, shell запрещён. С текущего хоста проверены auth и remote forward; через Android/ARM64 ещё нет. КлючTCLхоста закреплён вlavbook:.ssh/tcl_known_hosts. Команда сlavbook: ssh -o UserKnownHostsFile=~/.ssh/tcl_known_hosts -p22220 root@127.0.0.1. TCLавтоматически попробует туннель при наличии маршрута. Требуется доступ из сети телефона к192.168.8.186; вопрос Wi-Fi или mobile пока без ответа, доступность не обещать.

Автоперезагрузка сохранена; для длительной работы выполнить cancel-reboot вLinux. Ядро/DT/RMTFS прежние. Проверены ELF/CPIO/permissions/shell/firmware. Образ содержит диагностические privatekeys иmodemstorage, не для публичного вложения.

Новый пункт NEXT: Wi-Fi board file + SSH. EFI35a04e79c7c550a834b021db079cb22bb542f4ee286ac73d6fcc1cacd48a9681, initramfs84c24bb38bcf7a96681bf8899ce0b49844f77434fc3772dd7d653f447ad48f3d. ЗаписьUSB выполняется.

Запись wifi-ssh подтверждена: Disk6D:, FilesVerified2, EFI35a04e79c7c550a834b021db079cb22bb542f4ee286ac73d6fcc1cacd48a9681, backup BOOTAA64-rmtfs-probe.bak. Ожидается запуск пользователем.

Отчёт wifi-ssh опубликован в Bugzilla19084, comment180927 (confirmation8d1bfa2a-c14b-41bc-a26a-747e69ac4520 approved).

### 2026-09-07 — пароль Wi-Fi в общем хранилище

По указанию пользователя пароль сети `eterwifi` сохранён штатной командой `admin-pass-add.sh` на `server` в запись `wifi-eterwifi`. Проверены успешная запись и права `600 root:root`; каталог хранилища имеет права `700 root:root`. Доступ для администраторов: `ssh rooter@server pass show wifi-eterwifi`. Сам пароль в заметки и багу не включать.

### 2026-09-07 — встроенный Wi-Fi работает; подготовлен постоянный запуск

В boot 6b74186a-61b4-4382-b18f-038ec218c8df подтверждены подключение WPA2-PSK/CCMP к eterwifi (5220 МГц), DHCP 192.168.8.177/24, прямой SSH и двусторонняя передача 4 МиБ с SHA256 ef668c2cc941ccc9f2c8d09e2732e7620be9382532a75b18b5de8ca02b8eb294. Затем usb0 отключён, его маршруты удалены: новое прямое SSH-соединение, ping шлюза/1.1.1.1 и DNS работают только через wlan0. MPSS running на 30-й минуте. Ранее было одно переподключение reason 6 и предупреждения ath10k invalid frequency 0; длительная устойчивость не установлена.

На Kingston записаны /tcl-wifi-persistent/initramfs.cpio.gz и новый EFI, выполнена проверка SHA256 после размонтирования/повторного монтирования. EFI 3b6b6046bb44e78fa492abde8e9a3e0ed7e2c7ab35b64689dc1945e90cac4886; initramfs 51c58b0f865f5a0b9d562b5b274de74d77ae370a138ec53fd88c1227fbed7556. Резервный EFI: EFI/BOOT/BOOTAA64-before-persistent.bak. Секретов в опубликованных скриптах нет; образ приватный. Подробности: tcl-b220g-data/peripherals/wifi-persistent/README.md.

Новый пункт GRUB Linux: automatic Wi-Fi + SSH (no auto reboot) выбран по умолчанию, таймер 5 секунд. Автоматические WPA/DHCP/SSH, стабильный локальный MAC, повторный обратный туннель; автоперезагрузка отключена. Холодный запуск пока не проверен.

Проверка накопителей/видео без изменения состояния: видна только Kingston /dev/sda, внутренний UFS отключён в DT (ufshc@1d84000 и phy@1d87000); драйвер UFS зарегистрирован, это не доказательство неисправности диска. GPU/GMU/MDSS также отключены в DT. Работает только EFI fb0: 1920x1080, 32 bpp, stride 7680; DRM-карт нет. Дальнейшая проверка внутренних устройств остаётся задачей.

Пользователь разрешил перезагрузку для проверки автономного Wi-Fi. EFI runtime отключён параметром efi=noruntime, BootOrder из этого сеанса не менялся. Пользователю предложен ручной выбор USB и постоянное изменение приоритета USB в UEFI; порядок Windows Boot Manager не изменён.

### Ошибка нового EFI после перезагрузки

Фото photo_2026-09-07_14-59-06.jpg и текст сохранены в peripherals/wifi-persistent/photos/. GRUB не находит файл-маркер, Image и DTB; ядро не запускалось. Пользователь подтвердил отказ всех пунктов. Ошибка сборки с моей стороны: системный grub-mkstandalone 2.14 использован с модулями Debian 2.12, без прежнего явного списка предзагружаемых модулей. Это обнаруженные отличия от рабочего рецепта; их отдельный вклад в сбой не разделён. Подготовлен EFI тем же комплектом Debian 2.12-9+deb13u2 и с прежним списком модулей; SHA256 3abba529d5e6f2668bc3eed21aebb38ec01b094970e631540c370ac9ca3528c2. Ещё не записан/не проверен загрузкой. Для доступа предложена Windows. Рабочая резервная копия: EFI/BOOT/BOOTAA64-before-persistent.bak.

### Питание — дальнейшие эксперименты приостановлены

Пользователь сообщил: после разряда TCL не включается, хотя подключён к зарядке. Причина не установлена; исправленный EFI пока не записан. Поддержка зарядки/индикации батареи в диагностическом Linux не проверялась; cpuidle отключён, поэтому энергопотребление не оптимизировано. Сначала восстановить включение со штатным питанием, затем проверить зарядку и состояние батареи в Windows. Не считать проблему питания доказанным следствием ошибки GRUB.

Питание восстановилось: по сообщению пользователя Windows включилась после полной зарядки, отключения зарядного кабеля и ожидания примерно 5 минут. Windows Win32_Battery затем сообщил EstimatedChargeRemaining=100, BatteryStatus=2. Не установлено, что для включения требуются именно 100% и 5 минут: влияние зарядки и переподключения питания не разделено. Зависание контроллера питания — гипотеза, не диагноз.

Исправленный EFI установлен из Windows на проверенную Kingston D:. Скрипт подтвердил EFI SHA256 3abba529d5e6f2668bc3eed21aebb38ec01b094970e631540c370ac9ca3528c2, прежний initramfs, наличие Image/DTB и сохранённую резервную копию. Перезагрузка после исправления пока не выполнялась; исправление проверено по файлам, не загрузкой.

### 2026-09-07 — автоматический Wi-Fi и SSH подтверждены после перезагрузки

Boot ID dfd5d164-3c99-47b9-bb19-ec78a734bb88, первый прямой SSH на uptime 102 секунды. WLAN автоматически подключён к eterwifi, WPA2-PSK/CCMP, 5220 МГц, DHCP снова выдал 192.168.8.177/24. В системе только lo и wlan0, USB-сети телефона нет. Работают wifi-start, wpa_supplicant, постоянный udhcpc и Dropbear; MPSS running. Автоперезагрузка не запущена. Исправленный EFI Debian 2.12 загрузил новый initramfs успешно. Логи без секретного конфига и SHA256 сохранены в peripherals/wifi-persistent/logs/dfd5d164-3c99-47b9-bb19-ec78a734bb88/. Длительная устойчивость и энергопотребление остаются непроверенными; порядок UEFI с приоритетом Windows не менялся.

### Питание, диск и видео после автономной загрузки

Собран текущий статус и разобран EC-протокол батареи по DSDT; см. [таблицы и следующий тест](../research/peripherals/power-storage-video/README.md). Подготовлен локальный DTB для включения только EC-шины i2c2@888000, без драйвера и транзакций; ещё не записан/не загружался. В текущем ядре OF_OVERLAY отключён. Телеметрия батареи/температуры отсутствует, UFS/GPU отключены в DT. Работающий Wi-Fi сохранён, перезагрузки и изменения питания устройств не выполнялись.

### Тест EC записан на флешку

Добавлен отдельный пункт TEST: battery EC bus + automatic Wi-Fi + SSH, рабочий tcl-online остаётся по умолчанию. Kingston проверена по serial и маркеру; DTB и EFI проверены повторным чтением после размонтирования. EFI SHA256 1994f6275ce291c6e28f7df711629e130babbc70678898c6ef010066f2099432, DTB a64d18bbb1034575047736bf3d476766fff16e7c65e47aeb5b71750ef9026509; резервный EFI BOOTAA64-before-ec.bak. Ядро/initramfs не менялись. Подготовлен декодер EC и подтверждена форма I²C-чтения по i2c-core-acpi.c v6.18. Сам EC ещё не опрашивался, новая загрузка требуется для появления шины. Перезагрузка не выполнялась.

### EDL Mode

EDL (Emergency Download Mode, Qualcomm 9008) — низкоуровневый режим восстановления/прошивки через USB; это не обычная UEFI-загрузка с флешки. Qualcomm описывает bootrom-инициируемый стек Sahara/Firehose: https://github.com/qualcomm/qdlrs/ . Для текущего теста Linux EDL не требуется, вход/прошивка не выполнялись. Сам вопрос пользователя не означает, что устройство находится в EDL.

### Контроллер батареи отвечает в Linux

Новая загрузка 749c411c-ad99-4caf-875e-d2b5f356b73e. i2c-2 соответствует 888000.i2c, DT status okay; адрес 2-0007 не занят драйвером. Однократное i2ctransfer -y 2 w1@0x07 0x00 r32 успешно, без -f и записи настроек. Raw и JSON сохранены в peripherals/power-storage-video/logs/749c411c-ad99-4caf-875e-d2b5f356b73e/. ECWR=3 (AC online), B1DC=5400, B1FV=7700, B1FC=5400, B1ST=0x42, B1CR=0, B1RC=5400, B1VT=8702, BPCN=100. По DSDT напряжение 8702 мВ и ёмкость 5400 мА·ч; поле BPCN согласуется с 100%, но динамика ещё не проверена. Battery state по DSDT=2 при rate=0: это не доказательство ненулевого тока зарядки.

По новой просьбе пользователя тестовый пункт tcl-ec теперь выбран по умолчанию; старые пункты сохранены. EFI 727e5b17a06dd35f83741a084a242c12ab6ba6900c96a4b26cf59d7b87836501 проверен после повторного монтирования, резервная копия BOOTAA64-before-ec-default.bak. Перезагрузка для изменения default не выполнялась.

### Тест внутреннего UFS подготовлен

На Kingston записан и проверен отдельный DTB UFS и EFI с default=tcl-ufs. Детали питания, ограничения и источники: peripherals/ufs-probe/README.md. EFI 133d0eae4c4475910cfaf4f8d3db989ea3d0485c1f290cafe9020e0149864e2c; DTB 84b5858185f35a2c6922da5befc47c912d793b655f45e44d11494f63d689d856. Ядро/initramfs прежние, рабочая загрузка сохранена. Пока не перезагружали и UFS не опрашивали.

Повторное чтение EC до теста: ECWR=2 (адаптер отключён), B1ST=1 (разряд по DSDT), B1CR=546 raw, B1RC=5346, B1VT=8554, BPCN=99. Это подтверждает динамические показания; физический масштаб тока ещё не измерен.

Обнаружена ошибка ручного возобновления логгера: nohup отсутствует в BusyBox, поэтому ранее выведенное logging-resumed не доказывало запуск. Перед записью UFS выполнен /diagnose в foreground с final-save-request, подтверждены USB saved=yes и размонтирование. Для последующего ручного запуска использовать setsid и проверять процесс. Автозапуск /diagnose из init не зависит от nohup.

### UFS заработал: подтверждённое чтение всех шести LUN

Boot 4bed92e8-2cfe-4a9e-99cd-b26cdd21a3e0, прямой Wi-Fi SSH доступен на uptime 53 секунды. Контроллер 1d84000.ufshc успешно прошёл probe на 13.25 секунды; начальный -EPROBE_DEFER сменился успехом после появления зависимостей. Модель SAMSUNG KM8F9001JM-B813, revision0700.

| Linux в этом boot | UFS LUN / Windows PhysicalDrive | Размер байт | GPT-разделов |
|---|---:|---:|---:|
| sda | 0 | 251331084288 | 4 |
| sdb | 1 | 8388608 | 2 |
| sdc | 2 | 8388608 | 2 |
| sdd | 3 | 134217728 | 2 |
| sde | 4 | 4294967296 | 35 |
| sdf | 5 | 134217728 | 4 |

Kingston теперь sdg, а не sda: определять устройства по sysfs-пути/serial, не по букве. Размер сектора UFS 4096 байт; /sys/block/*/size выражен в 512-байтных секторах. Все UFS-диски и разделы по sysfs-пути 1d84000.ufshc переведены blockdev --setro и проверены --getro=1. Это настройка текущей загрузки, не постоянная защита накопителя.

Прочитаны первые 128 КиБ каждого LUN. Для каждого проверены CRC32 primary GPT header и partition-entry array; все корректны. Все шесть буферов побайтно совпадают с disk0-gpt.bin ... disk5-gpt.bin, ранее снятыми из Windows. JSON сравнения, raw GPT и логи сохранены в peripherals/ufs-probe/logs/4bed92e8-2cfe-4a9e-99cd-b26cdd21a3e0/ с SHA256SUMS.

Windows-разделы не монтировались, команды записи на UFS не выполнялись. Проверена инициализация и чтение GPT; скорость, длительная устойчивость, чтение всего диска и работа файловых систем ещё не проверены. Предупреждения об отсутствии vdd-hba/vccq и max-microamp ожидаемы по изученному коду, probe завершился успешно. ICE остаётся отключён, содержимое зашифрованных разделов не расшифровывалось.

### 2026-09-08 — зарядка подтверждена в работающем Linux

Boot 4bed92e8-2cfe-4a9e-99cd-b26cdd21a3e0. До подключения адаптера: AC=0, state=discharging, BPCN=77, B1RC=4158 мА·ч, B1VT=8056 мВ, B1CR=577 raw. Пользователь подключил штатную зарядку. На uptime562: AC=1, state=charging, B1VT=8620 мВ, B1CR=2432 raw; на uptime599 ёмкость выросла до4212 мА·ч, BPCN=78. Рост ёмкости подтверждает фактическую зарядку в текущей конфигурации Linux, а не только флаг AC. Записей настроек EC не делали. Это не объясняет прежний отказ включения после разряда.

В текущую RAM-систему установлены /bin/battery-status и /bin/battery-monitor; монитор через setsid опрашивает EC каждые30с, проверен живой процесс. Скрипты проверяют путь шины и отсутствие связанного драйвера EC. Лог /run/battery.log скопирован в peripherals/power-storage-video/logs/4bed92e8-2cfe-4a9e-99cd-b26cdd21a3e0/. Скрипты сохранены рядом с таблицами. В загрузочный initramfs пока не добавлены, после перезагрузки эти RAM-команды исчезнут. Штатного power_supply-драйвера TCL пока нет.

Масштаб сырого тока отдельно не проверен: не объявлять его измеренными мА/Вт и не считать по нему точное время автономности. Изменения ёмкости имеют шаг54 мА·ч (1% от5400), что ограничивает точность коротких измерений.

CPU0–7 online; sysfs cpufreq пуст, cpu0/cpuidle отсутствует. В DT удалены CPU clocks/power-domains, в cmdline cpuidle.off=1. Глубокие состояния простоя и регулирование частоты ещё не проверены; отсутствие cpuidle driver не означает отсутствие базового ARM WFI. Во время проверки зарядки настройки CPU/регуляторов не меняли.

### Тест CPUFreq подготовлен и записан

Включён аппаратный cpufreq и связи CPU с двумя доменами, без восстановления глубокого сна и ICC scaling. Запрошен schedutil. Детали в peripherals/cpufreq-probe/README.md. EFI/DTB проверены после повторного монтирования. Пользователь явно разрешил перезагрузку для теста.

### CPUFreq работает после автоматической перезагрузки

Boot 2b0f6945-4e68-425a-8b0d-ebbf7de2af76, Wi-Fi SSH проверен на uptime50. Драйвер qcom-cpufreq-hw зарегистрирован, обе policy используют schedutil. CPU0–5:300000–1804800 кГц; CPU6–7:652800–2400000 кГц. scaling_min/max совпадают с cpuinfo_min/max.

Короткие независимые нагрузки taskset CPU0/CPU6 по5сек: policy0 поднялась до1804800, policy6 до2400000; после нагрузки значения снизились. Статистика time_in_state показывает основную долю времени на768000/825600 кГц. Это подтверждает изменение состояний драйвером; scaling_cur_freq и time_in_state не являются независимым физическим частотомером. Доступные минимумы300000/652800 в этом окне статистики не использовались.

MPSS running, Wi-Fi/SSH, UFS и i2c-2 сохранены. Read-only UFS после новой загрузки установлен повторно; автоматической постоянной защиты пока нет. Глубокий сон по-прежнему отключён, ICC scaling не включён. Экономия мощности количественно не измерена. Логи, ограничения policy и SHA256 сохранены в peripherals/cpufreq-probe/logs/2b0f6945-4e68-425a-8b0d-ebbf7de2af76/.

### 2026-09-08 — Ubuntu Base подготовлена на USB

Ubuntu Base26.04.1 ARM64 развернута native в chroot на работающем TCL, обновлена из подписанных репозиториев Ubuntu. Подробный полный перечень изменений и ограничения: [Ubuntu README](../research/peripherals/ubuntu-base/README.md). Сохранены прежние ядро6.18.34-stb-qc7+ и cpufreq DTB. Ext4 в ядре нет, поэтому на существующей FAT32 создан Btrfs loop rootfs.img3ГиБ (UUID b433988c-6473-450b-9e9a-b07598eac2a3), без переразметки USB/UFS.

OpenSSH Ubuntu на временном порту2222 проверен реальным входом с прежним ключом; listener остановлен. RMTFS1.1.1-2 и tqftpserv1.1.1-1 теперь штатные пакеты Ubuntu, режим RMTFS -r -v -o с прежними копиями modem storage. Настроены порядок запуска модема, Wi-Fi/networkd, root key-only SSH, локальный autologin, журнал, время, защита UFS. Батарея100%, AC1, voltage8699мВ, current raw0. Для i2c-tools Ubuntu нужен -a к уже проверенному адресу0x07.

После размонтирования btrfs check --readonly без ошибок. Новый default=tcl-ubuntu, backup EFI BOOTAA64-before-ubuntu.bak, старые меню сохранены. Новый EFI16bfa183c9d687ff54b8e136d8ab51ab9a08da863fef0eb68e4ae9ad21118f3e; initramfs81f3a80c0c8fea648d705016fcfd055ea249f064ab872d769603763f845d863c; rootfs235d00780b9f1795b8c6c3845bc6b28e7ae4253c696b7c5bae20bf445a61cc0e. SHA проверены после повторного монтирования USB read-only.

По просьбе пользователя комплект опубликован в /var/ftp/tmp/lav/tcl. Полный файловый архив USB tcl-ubuntu-usb-20260908.tar.gz.gpg зашифрован AES256/MDC, поскольку содержит закрытые ключи/профиль Wi-Fi/индивидуальные modem storage. Новый случайный пароль записан в admin store на server: tcl-ubuntu-usb-20260908. В заметки/багу пароль не включён. Проверены потоковая расшифровка,169 файлов и5 эталонных SHA. Открыто лежат Image/DTB/EFI/initramfs Ubuntu, исходный Ubuntu Base, GRUB2.12 tools, скрипты и README. Пока загрузка systemd не подтверждена; следующий этап — одна тестовая загрузка.

Уроки подготовки: BusyBox chroot с коротким именем команды может выбрать applet BusyBox; для native Ubuntu использовать /bin/bash или абсолютный путь бинарника. tqftpserv не обрабатывает -h как help и запускает службу: пробный процесс из chroot был корректно остановлен SIGTERM, штатный процесс176 и Wi-Fi остались работающими. Для проверки rootfs при копировании исключать сами proc/sys/dev/run/mnt, а не только их содержимое, иначе GNU tar возвращает1 на изменяющейся metadata sysfs. На builder gpg1.4.23: --pinentry-mode отсутствует, применены --batch --passphrase-file и --force-mdc.

Отдельный полный Markdown-отчёт об изменениях Ubuntu отправлен через bug_add_comment с work_time60мин; ожидает подтверждения Lavtomate70f27b17-eff4-4a7b-bcba-08b633c72136. Не повторять запрос. Комплект24файла и общий SHA256SUMS проверены. После полного резервирования выполнена одна reboot -f с предварительным umount USB для проверки systemd Ubuntu; результат ещё ожидается.

Комментарий Ubuntu принят в Bugzilla: comment_id180964, confirmation70f27b17-eff4-4a7b-bcba-08b633c72136 approved. Пользователь сообщил после загрузки: «всё работает: и диск и wifi». Это пользовательское подтверждение, проверка новой Ubuntu по SSH ещё выполняется.

### Ubuntu: найдена ошибка прав конфигурации DHCP

После загрузки пользователь подтвердил отсутствие IP и SSH. В зашифрованном предзагрузочном архиве офлайн проверены Btrfs INODE_ITEM: /etc/systemd/network/20-tcl-wifi.network и10-tcl-wifi.link имеют100600 root:root, каталог40755. Профиль wpa_supplicant-wlan0.conf присутствует,168байт,100600 root:root; содержимое не выводилось. Сведения без секретов сохранены в ubuntu-base/network-permissions-before.txt. Закрытые права файлов networkd ошибочны для службы systemd-network; профиль WPA должен оставаться600.

Источник в подготовке: ssh-start диагностики выставляет umask077, configure-root.sh не задавал свою umask. Исправлен источник: umask022 и явный chmod644 только .network/.link. Пользователю переданы chmod644, restart systemd-networkd и start ssh.socket. Подтверждение получения адреса после этих команд пока ожидается. Предзагрузочный архив ещё содержит старые права, обновление выполняется; не объявлять его исправленным раньше проверки.

### Подтверждение сети Ubuntu и новые задачи

SSH после ручного исправления подтверждён: boot15a44890-c95f-4081-8cab-3d71212e63bb, Ubuntu26.04.1, PID1systemd, running, failed0. Журнал подтвердил Permission denied у networkd; Wi-Fi сам подключился06:55:19, ssh.socket слушал06:55:06, после chmod/restart DHCP выдал192.168.8.177. Все radio/network/SSH службы enabled/active, права644 и WPA600 сохранены в Btrfs loop root rw, sync выполнен. Повторной перезагрузки после исправления не было; пользователь специально уточнил ручной запуск. Лог в ubuntu-base/logs/15a44890-c95f-4081-8cab-3d71212e63bb/status.txt.

Архивv2 опубликован и проверен: tcl-ubuntu-usb-20260908-v2.tar.gz.gpg, rootfs SHA5a7c8f191c5bbde431fae69316f419637c8758bd7e230aaaea1c3698f8d565f5. Все169файлов и5эталонных SHA проверены,168остальных файлов неизменны. Исходный архив безv2 содержит ошибку прав. Обновлены README, configure-root.sh(umask022+chmod644), манифесты и общий SHA256SUMS(30файлов). Отчёт bug19084 с25мин ожидает Lavtomate e2e1e7c6-b94a-4bbe-8f90-e8aa378904bb, не дублировать.

Уточнение ядра: это готовая Qualcomm-сборка hexdump0815 6.18.34-stb-qc7+, не штатный Ubuntu linux-generic. Мы её не пересобирали, чистоту upstream/отсутствие сторонних патчей не заявлять.

Пользователь поручил установить EPM, затем полный архивный снимок внутреннего диска для возврата Windows. Официальный универсальный epm.sh скачан с https://eepm.ru/epm.sh, инструкция проверена через gh в Etersoft/eepm. Попытка bootstrap install curl/ei не прошла из-за действующего пользовательского apt install inxi(PID1589); блокировку не удаляли, процесс не прерывали. Снимок пока не начат. План — полные raw-потоки всех6UFS LUN в Restic на сервере, не файловая копия Windows. Свободно около1.5ТБ в /var/ftp/tmp (NFS ftpserver:/tmp), зарядка подключена. GNU ddrescue рассмотрен для случая ошибок чтения, Restic stdin-from-command — для контроля завершения потока SSH.

Пользователь повторно закрепил EPM как обязательный интерфейс пакетных операций; память feedback_use_epm_for_packages.md дополнена. Затем уточнил: полный снимок Windows нужен БЕЗ шифрования. Это относится к новой резервной копии UFS; Restic с обязательным шифрованием не подходит. Рассматривается Borg encryption=none со сжатием и проверкой целостности, каталог резервной копии закрытый. Ранее опубликованный зашифрованный комплект Ubuntu пока не изменён этим уточнением.

### 2026-09-08: уточнение правильных DEB-зависимостей EPM

Повторно прочитан control официального eepm_3.64.66-alt1ubuntu_all.deb: Architecture: all, Multi-Arch: foreign, Depends содержит diffutils:amd64 и gzip:amd64. Для запуска обычных CLI из EPM нужны зависимости diffutils и gzip без архитектурного суффикса; замена amd64 на arm64 также неправильно привяжет универсальный пакет к одной архитектуре. Суффикс :any не является универсальной заменой: его разрешение зависит от Multi-Arch пакета-зависимости. Multi-Arch: foreign у самого eepm регулирует удовлетворение зависимостей других пакетов от eepm, а не снимает ограничения его собственного Depends.

Важно: сочетание Architecture: all и явно архитектурной зависимости само по себе не запрещено; ошибка здесь в отсутствии потребности EPM именно в amd64-утилитах. Control указывает конвертацию alien 8.95.9-1, но место появления суффиксов ещё не установлено. Разработчику нужно сравнить исходные RPM Requires, промежуточные зависимости конвертера и итоговый DEB. Возможный источник — перенос архитектурно квалифицированных имён из базы сборочного хоста; это гипотеза, не доказанный дефект alien.

Источники: https://www.debian.org/doc/debian-policy/ch-controlfields.html ; https://manpages.debian.org/unstable/dpkg-dev/deb-src-control.5.en.html ; https://manpages.debian.org/unstable/dpkg/dpkg-query.1.en.html . Проверка исправления: один и тот же all.deb штатно устанавливается на чистых arm64 и amd64, без foreign architecture и force-depends, пакетная база остаётся согласованной.

### Отдельные баги EPM по указанию пользователя

Созданы и подтверждены в Bugzilla Etersoft:
- https://bugs.etersoft.ru/show_bug.cgi?id=19451 — eepm all.deb требует diffutils:amd64 и gzip:amd64, компонент Package.
- https://bugs.etersoft.ru/show_bug.cgi?id=19452 — epm print info: частота ARM через CPUFreq и несколько доменов, компонент Разработка. Область задачи — процессор.

Предыдущий объединённый комментарий в TCL 19084 отменён до публикации: confirmation e6aa86f5-d927-45b7-a7cd-4ef3a00bbd5e успешно rejected по уточнению пользователя.

### 2026-09-08: текущая графика и дополнение CPUFreq

По SSH подтверждено: uname 6.18.34-stb-qc7+; /proc/fb = «0 EFI VGA»; fb0/device/driver → platform/drivers/efi-framebuffer; /sys/class/drm содержит только version, DRM card отсутствует. dmesg: framebuffer 0x9bc00000, 1920x1080x32, stride7680; console переключилась на framebuffer240x67. Отключение ранней bootconsole efifb0 не означает смены графического драйвера: затем работает обычная fbcon через efifb.

Проверен опубликованный комментарий #41 (180964) баги19084: прямо указаны готовая Qualcomm-сборка hexdump0815, отсутствие linux-generic Ubuntu, непроверенное штатное ядро Ubuntu и EFI framebuffer; GPU/MDSS/GMU отключены.

В багу19452 отправлены команда чтения CPUFreq и фактический вывод: policy0 1516800кГц, policy6 825600кГц, max1804800/2400000, schedutil; epm print info «8 /  MHz». Публикация ожидает Lavtomate confirmation83de55d0-6727-476d-90c1-4394fb692f28; не дублировать.

### 2026-09-08: успешный роуминг на 5 ГГц во время архивации

По разрешению пользователя выполнен `wpa_cli -i wlan0 roam 8c:de:f9:bf:2a:ba`. Перед командой создан одноразовый systemd timer возврата на прежнюю BSSID8c:de:f9:bf:2a:b9 через90с. После проверки новой SSH-сессии, прежнего TCP192.168.0.64:51674 и тех же процессов архивации6592/6593/6594 таймер остановлен (inactive).

Подключение: eterwifi,5220МГц/канал44,80МГц,VHT-NSS2,MCS4, RX bitrate390Мбит/с, сигнал−75…−76дБм. Прежний SSH/TCP сохранился; мгновенный delivery_rate вырос с~67 до162Мбит/с, RTT~11мс. Это моментальный замер, не итоговая средняя скорость. Power save не менялся. Постоянная привязка BSSID в WPA-конфиг не добавлялась; выбор5ГГц после перезагрузки не гарантирован. АрхивLUN0 продолжает расти, пройдено9ГиБ; пять служебныхLUN ранее проверены чтением из Borg.

### Предпочтение 5 ГГц при следующем старте WPA

В постоянном /etc/wpa_supplicant/wpa_supplicant-wlan0.conf на TCL сохранены два профиля eterwifi: priority10 с freq_list частот5ГГц из iw phy и priority0 без ограничения диапазона как fallback. Ключ скопирован внутри TCL без вывода, mode600, прежний файл сохранён как .before-prefer5. Выполнен sync. Работающий wpa_supplicant не перезапускался и reconfigure не вызывался, чтобы не прерывать архив; новая политика вступит при следующем старте службы. Выбор после перезагрузки пока не проверен. Скрипт без секретов сохранён ubuntu-base/prefer-5ghz.py; ранее опубликованный архивv2 эту настройку ещё не содержит.

### Предварительный отчёт по графике

Проверена конфигурация: DRM_MSM=m; KMS/DPU/DSI/10NM_PHY, PANEL_EDP, TI_SN65DSI86 включены. Старый DTS содержит неактивную заготовку SN65DSI86@2c, но реальные ACPI-ресурсы EDPBridge — i2c10@29. Просто включать чужую заготовку нельзя. LT8911EXB на нашем экземпляре пока не подтверждён. Сохранены OEM EDPBridge.sys, INF, частичная декомпиляция и qcdxkmsuc7180.mbn; пригодность последнего для GPU zap не проверена. Следующий этап — модель мостика, GPIO и power sequence из OEM/ACPI, отдельно GPU/GMU. Рабочий DTB не менялся, тестовый DRM DTB пока не готов. В bug19084 направлен отдельный Markdown-отчёт с10мин; статус публикации смотреть по результату Lavtomate, не повторять запрос.

### Графика: новые офлайн-находки

Сохранены graphics-research/README.md, prepare-hardware.asm и reference-lt8911exb.c (получен gh). В OEM функции0x140003c48 найдена последовательность FF=81,08=7F,49=FF,FF=82,5A=0E, совпадающая по ключевым операциям с открытым LT8911EXB init (кроме дополнительной08=7F). Это усиливает гипотезу LT8911EXB-family, но не заменяет chip-ID. OEM функция подготовки отображает memory resource через MmMapIoSpaceEx и читает16бит в начале; назначение области как RebootState handoff исследуется, точная семантика не подтверждена. GPIO и lanes пока не расшифрованы. Аппаратных записей/перезагрузок не было.

### OEM GPIO: восстановлено соответствие и импульсы

По PrepareHardware IDs GPIO хранятся context+0x30+8*n в ACPI-порядке:23,20,51,26,13,21. Init0x140002d40 ставит первые5 в1; для GPIO26 выполняет1/20мс/0/150мс/1; затем GPIO13 получает1/10мс/0/30мс/1/10мс. GPIO21 отдельно управляется boolean-функцией0x140003080:0 перед init,1 в отложенном worker. Предположительная роль21 — разрешение дисплея/подсветки,13 — reset-подобный сигнал; электрические названия и напряжения не доказаны. Транспорт использует IOCTL0x00480004. Подробные адреса, ссылки и asm сохранены graphics-research/README.md, gpio-sequences.asm, gpio-transport.asm. Аппаратных записей не выполнялось.

### DSI/eDP: параметры из OEM-кода

OEM D0:00=00 (call0x140003ed0) соответствует4 входным MIPI линиям по reference LT8911EXB. eDP85:1A (0x140004878) берёт global0x14000705c; PrepareHardware задаёт2 при высоте>=1080, иначе1. Для панели1080p ожидаются2 eDP линии; физический link ещё не измерялся. Драйвер при первом запуске читает timing из уже настроенного моста, первые чтения без локального выбора банка — зависимость от firmware-state требует уточнения. Dither-параметр динамический. Список189 call sites и скрипт извлечения сохранены graphics-research; это не готовая программа инициализации. Существенные различия OEM/reference запрещают слепой перенос исходника. АрхивLUN0 дошёл69/234ГиБ, продолжает выполняться.

### Таблица таймингов и порядок включения

Восстановлены10полей timing structure0x140007040 (H/V front,sync,back,active,total); expected-timing.json рассчитан из EDID: totals2080×1142,59.999326Гц. Это ожидаемые значения, не live dump. Чтение0x140002afc–2c68 сопоставлено с записью0x140004088–4174. Уточнение предыдущей заметки: после85:B0 драйвер выбираетA8 и читает17/18, неD0; назначение этих2полей пока неизвестно.

Worker0x140004ee0: lock → GPIO21=0 → init GPIO → init bridge → delayed worker → unlock. Delayed worker0x140004fc0 ждёт context+2Cмкс и пишетGPIO21=1; default120000мкс, универсальность задержки не доказана. Сохранены и опубликованы открытые README, expected-timing.json, register-call-sites.tsv,extract-register-calls.py в /var/ftp/tmp/lav/tcl/graphics-research. Обновление общего SHA256SUMS пока не выполнено, приватный Borg-каталог в рекурсивное хеширование не включать.

### Текущий XBL содержит LT8911EXB

С TCL read-only считаны XBL/dev/sdb1 и PLAT/dev/sde7 в /tmp/tcl-graphics-private (700), SHA256 совпали с независимым чтением устройств. XBL82dd0864eeb1e8c1289abeed38ae540323e61dd95f0e2ff2ab38a4f33a32136f; PLAT50cb58e67533228903cea9b3e384e91ac82fa32fa5359672f539c8c2f707336d. В XBL обнаружены Reset_LT8911, EDPBridgeDriver_SetMode_LT8911, EDIDRead_LT8911 и строка LT8911EXB chip id. Это прямое подтверждение реализации LT8911EXB в установленном XBL-слоте, но не измеренный chip-ID и не подтверждение выбора слота. Старую капсулу2021 с generic SN65DSI86 templates не считать актуальной конфигурацией. Следующий шаг — извлечение DisplayDxe и board-selection пути.

### Извлечён DisplayDxe и подтверждён GPIO13 в reset routine

LZMA XBL0xC4438 →8859528байт; PE DisplayDxe по0x3FAE38,507904байта,SHA76bfce3fe980a8c8130849628eee097b8c11af63073e98509652bfc1a5b492f0. Скрипт воспроизводимого извлечения и asm сохранены graphics-research. Reset_LT8911@0x2E1A4 управляетGPIO13: high10мс,low100мс,high100мс (Windows10/30/10мс). Generic label BL_EN_MSM в error text не считать точным названием цепи. ID routine0x2E640 пишетFF81/087F, читает00/01/02; live ID не читался. UEFI write helper0x2F81C используетw0/w1; eDP config0x2E6A8 читает timing structure0x5225A. Дальше проследить заполнение структуры из EDID/выбор панели. Аппаратных записей и перезагрузки нет.

### EDID и handoff UEFI→Windows подтверждены в коде

UEFI0x2F36C проверяет checksum128байт;0x2D4AC разбирает первый18-байтный DTD поEDID+0x36 с guard в panel-config+0x9C;0x2E434–2E548 переносит поля в0x5225A. Есть ветвь temp_edid, поэтому фактический выбор runtime ещё не доказан.0x2FA58 форматирует XML-поля, а0x2FA70–80 пишет16бит0xA0 по0x9FF90000; Windows0x140001ED0–1EDC читает/сбрасывает тот же маркер. Адрес XML-буфера получается отдельно через memory-region protocol, не равен автоматически адресу маркера. Сохраненыuefi-edid-path.asm иuefi-state-marker.asm. Аппаратную память не трогали.

### 2026-09-08: GitHub — адресаты будущего отчёта

По просьбе пользователя выполнен поиск через gh. Ссылки, участники, исторические результаты и план будущего сообщения сохранены отдельно: [tcl-b220g-github.md](../docs/github-research.md). Приоритет — MinimumLaw#1 и velvet-os/imagebuilder#136. Наши результаты UFS/Wi-Fi дополняют ранний DTS AliceCyber; LT8911EXB был известен участникам ранее, наше новое — подробности Windows/UEFI инициализации. Найден также firmware-репозиторий rainbyte для CX 27000W, совместимость пока не проверена. Публикаций на GitHub не делали.

Параллельная проверка архива: LUN0 принят полностью, 251331084288 байт, SHA256 совпал с источником; driver.log сообщает этап `Verifying restored stream of LUN0`. Завершение всего архива пока не подтверждено.

### 2026-09-08: запасной EDID и проверка архива

Из DisplayDxe декодирован встроенный fallback EDID (RVA0x47828): 1366x768, 76.30MHz, H48/32/146, V3/5/22, checksum0. Он отличается от сохранённого EDID нашего экрана1920x1080/142.52MHz. Уточнены направление копирования и адрес изменяемого EDID-буфера0x79FA9. Подробности в graphics-research/README.md и uefi-fallback-timing.json. Применять шаблон fallback к нашей панели оснований нет.

Подготовлена инструкция извлечения Windows/UFS: peripherals/windows-backup/README.md. LUN0 полностью принят, SHA256 c96610bd52b9c141a34ddfc440e707023687089cbe9c560f73b666bbee2b5a26 совпадает с источником. Проверка обратного потока активна; счётчик rchar проверяющего процесса примерно116.3GB (ориентир прогресса, не финальный результат). LUN1–5 ранее проверены. COMPLETE ещё отсутствует. Перезагрузок и аппаратных записей не выполняли.

Отчёт по fallback EDID/архиву/upstream отправлен в Lavtomate для Etersoft#19084, work_time15. Ожидает подтверждения в интерфейсе: `5893fbf8-6d73-4c01-8f10-a83ef4c523ea`; не повторять отправку. README восстановления скопирован рядом с приватным архивом, открытые заметки graphics-research обновлены в /var/ftp/tmp/lav/tcl/graphics-research/.

### 2026-09-08: DSI-конфигурация и завершение обратной проверки LUN

Подтверждена таблица54фрагментов XML в функции LT8911/EDID DisplayDxe: индекс39 указывает на DSI Interface. Значения: lanes4, color36, traffic1, LP11 true. По enum родственных исходников Qualcomm это RGB888/24bpp и NonBurst_VSEvent; перенос числовых значений непосредственно в Linux недопустим. Сохранены uefi-panel-xml-fragments.json и воспроизводимый extract-panel-fragments.py; подробности/ссылки graphics-research/README.md.

В 10:26:48UTC LUN0 прошёл полное обратное чтение с SHA256: теперь проверены все6LUN. Заключительный borg check ещё выполняется. SSH/TCL доступен, boot_id прежний, load0.00, Wi-Fi5220MHz/390MbpsRX/-72dBm, все6внутренних устройств RO1, framebuffer EFI VGA.

Предыдущий отчёт Etersoft#19084 подтверждён и опубликован, comment_id181004 (confirmation5893fbf8-6d73-4c01-8f10-a83ef4c523ea).

Отчёт о DSI и полной проверке потоков6LUN опубликован в Etersoft#19084, comment_id181011, work_time15, confirmationb25c8bef-4646-46e5-9ff5-9591721c64bd approved.

### 2026-09-08: архив завершён, подготовка графического теста

COMPLETE архива Windows/UFS:2026-09-08T10:33:14+00:00. Все6потоков извлечены с проверкой SHA256, borg check --repository-only успешен. Создан verified-manifest.json, инструкция обновлена. Восстановление на аппаратный диск не выполнялось.

Подготовлен graphics-probe/collect.sh, проверен sh -n и выполнен по SSH; исходное состояние в baseline.txt. README описывает поэтапный тест I2C/ID/DRM/KMS и сбор логов/возврат к рабочей загрузке. Это ещё не загрузочный DRM-тест: требуется драйвер мостика и проверка pinmux I2C. Текущий framebuffer EFI, mdss/gpu/display clocks disabled, msm доступен, LT8911EXB module не найден. Изменений USB/GRUB и перезагрузок нет.

Отчёт о завершении архива и подготовке graphics-probe ожидает подтверждения Lavtomate:42d94e7c-1156-41ad-a37f-346cbb204dc7, Etersoft#19084/work_time10. Не повторять отправку.

### 2026-09-08: подготовлен I2C10 DTB

Подтверждены ACPI IC11/a90000/IRQ389 и PEP GPIO86/87, согласующиеся с SC7180 qup14/IRQ_SPI357. Live pinmux86/87 UNCLAIMED. GENI I2C builtin. Рабочий USB DTB совпал с локальным SHA256f88a6854...83f7713.

Подготовлен graphics-probe/sc7180-tcl-i2c10.dtb (SHA2561929faac529880bcf7bd87d83ce30cccf0bf28c074141b61657af81fb44cc71c): включены только QUP1 и i2c10, частота100kHz. Проверен diff3свойств. Build-script и GRUB-фрагмент сохранены. В работающий GRUB изменения не внесены, аппаратного теста/перезагрузки ещё не было. Доступ к0x29 не выполнялся.

DTB-кандидат и сопровождающие файлы скопированы в /var/ftp/tmp/lav/tcl/graphics-probe/. Отчёт Etersoft#19084/work_time15 ожидает подтверждения Lavtomate:e2f245fa-0c17-4dd3-9169-491e19ace473; повторно не отправлять.

### 2026-09-08: I2C10 тест установлен на USB, без перезагрузки

Проверены текущие хеши EFI/initramfs и сеть: ssh.socket enabled/active (ssh.service disabled штатно при socket activation), WPA/networkd enabled, права сетевых файлов644. Собран новый EFI GRUB2.12 с прежним явным набором modules, grub-script-check успешен; SHA256c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52. Default=tcl-i2c10, tcl-ubuntu сохранён.

DTB/EFI установлены проверяющим install-i2c-boot.sh, sync и повторные SHA256 успешны. Backup USB EFI/BOOT/BOOTAA64-before-i2c10.bak (16bfa183...118f3e), локальная копия /tmp/tcl-i2c-boot-build/BOOTAA64-before-i2c10.EFI. Rootfs.img/Image/initramfs не заменялись. Перезагрузки нет; аппаратный результат I2C-теста ожидается. Подробности graphics-probe/README.md и install.log.

Отчёт об установке I2C10/EFI ожидает подтверждения Lavtomate:b040ec20-00f0-464a-a129-b099d4e89e81, Etersoft#19084/work_time15. Не повторять отправку.

### 2026-09-08: первая загрузка I2C10-теста начата

По указанию «продолжай» выполнены повторная проверка EFI/DTB SHA256, sync и systemctl reboot. SSH-команда завершилась0; прежний boot_id15a44890-c95f-4081-8cab-3d71212e63bb. Новый EFI/DTB совпали с установленными хешами. Два последующих SSH подключения к192.168.8.177 завершились таймаутом. ip neigh содержит прежнюю MAC-запись со статусом STALE — это не подтверждение живого устройства. Успех загрузки/I2C пока не установлен. Запрошено состояние экрана; нельзя пока отличить отсутствие сети от зависания ядра/сбоя до Linux.

### 2026-09-08: I2C10 аппаратный тест успешен

SSH вернулся, boot_id192b0902-e812-4c64-9c93-ef4b64ebb903. Новый DT активен, QUP1/i2c10 okay, a90000.i2c→geni_i2c, /dev/i2c-10 Geni-I2C. Deferred probe пуст. EFI VGA сохранён,6UFS RO1, HID keyboard/touchpad зарегистрированы. Первые SSH timeout не были доказательством зависания; причину задержки отдельно не установили.

Автозапуск сети теперь подтверждён журналом после reboot: WPA профиль eterwifi-5ghz/5220MHz, DHCP192.168.8.177, ssh.socket Listening. Логи graphics-probe/after-first-boot.txt и first-boot-network-and-i2c.txt. Исправлен путь перечисления адаптеров collector. Мостик0x29 ещё не опрашивали. Следующий этап готовить на текущей загрузке, без повторного reboot.

Отчёт успешного I2C10 теста ожидает подтверждения Lavtomate:3a686aa7-c72c-4f71-ba18-6dca939089ce, Etersoft#19084/work_time15; не повторять отправку. README/collector обновлены в серверном комплекте.

### 2026-09-08: реальный ID LT8911EXB прочитан

На I2C10/0x29 FF read дважды0x12, поэтому восстановление селектора по readback не обосновано. После проверки OEM последовательности выполнена единственная запись FF=81. Access81:08 уже7F (не меняли), ID81:00/01/02 =17 05 E0, совпадает с внешним reference driver. Банк оставлен81, восстановление исходного банка не заявляем. GPIO/reset/power/video не менялись. SSH доступен, EFI VGA зарегистрирован, boot_id тот же. Визуальная проверка картинки отдельно не выполнена. Логи/скрипт в graphics-probe; следующим шагом можно готовить whitelist timing/status read на текущей загрузке.

Отчёт ID ожидает подтверждения Lavtomate:2771c4b9-efec-47bd-ae6a-b19e309395ef, Etersoft#19084/work_time15. Не повторять отправку. Скрипт/ID/README скопированы в /var/ftp/tmp/lav/tcl/graphics-probe/.

### 2026-09-08: реальные тайминги LT8911EXB подтверждены

Прочитан whitelist D0:0D–1A и A8:05–0E,11/12/14/15/16/2D. Все10timing-полей в обоих банках совпали с EDID:1920x1080, totals2080x1142, Hfront/sync/back58/42/60, V3/5/54. A8 startH102/V59,2D88. Это конфигурационные регистры, не измерение частоты; pixelclock142.52MHz пока только EDID. Записи только FF, закончили bank81 с повторной проверкой ID17 05 E0. SSH/EFI доступны, reboot нет. Скрипт/сырые данные/декодер/сравнение в graphics-probe; таблица в README.

Отчёт timing ожидает подтверждения Lavtomate:cf9f8341-1aa8-40d9-9c56-4ada0c921303, Etersoft#19084/work_time15, повторно не отправлять. Серверный комплект обновлён.

### 2026-09-08: DSI/eDP параметры и отличие финального PLL

Live whitelist: D0:0000 (4DSI),85:1A02 (2eDP),A8:17/1810/20 (8bit/color),85:B000 dither off,C001 MIPI Rx,A8:2710 MIPI video. 87:3703 — PLL lock bit1 установлен. D0:2616,82:6E81 согласуются с EDID clock профилем, частоту не измеряли.

87:1933 отличается от reference31, но найден точный финальный write33 в текущем DisplayDxe0x2ED48 после исходного31@2EC50. Сохранён uefi-txpll-final-config.asm. Это OEM отличие нужно сохранить в будущей реализации; скорость eDP по нему пока не декодирована. Записи только FF, финальныйbank81/ID совпал, SSH/EFI доступны. Логи bridge-link.jsonl,after-link-health.txt, таблица graphics-probe/README.md.

Отчёт link/PLL в Etersoft#19084/work_time15: Lavtomate вернул success=true/status=approved, confirmation0036f809-dd6f-431a-8e3f-818c29e295ea, result=null (comment_id не предоставлен). Повторно не отправлять; при необходимости проверить результат confirmation_check. Серверный комплект обновлён.

### 2026-09-08: eDP training status и профиль выхода

Найден OEM статусный путь AC82(bit5done,low5==1Esuccess),AC83panel link rate,AC84panel link count. Live AC01=0A,82=3E,83=0A,84=82. По стандартным DP кодам из официального Linux v6.18 drm_dp.h это соответствует2.7Gbps/lane,2lanes,enhanced framing. Не direct DPCD и не измерение физической частоты; status инициализации не тест текущих ошибок канала.

Live82:22–29=82 00 80 00 82 00 80 00, совпадает с первым индексом таблиц OEM0x5227E/0x52284. Напряжения/битовая семантика профиля не установлены. Записи толькоFF, final81/IDсовпал, reset/retrain/AUX нет. Сохранены script/raw/uefi-training-status.asm; таблица graphics-probe/README.md.

Отчёт training status ожидает Lavtomate confirmation8d4a9b22-634f-4987-a006-5a8e5a56e92e, Etersoft#19084/work_time15; не повторять отправку. Комплект на сервере обновлён.

### 2026-09-08: цель Wayland и визуальная демонстрация

Пользователь уточнил конечную цель Wayland. Проверено: в saved kernel config6.18.34-stb-qc7+ CONFIG_DRM_SIMPLEDRM и CONFIG_SYSFB_SIMPLEFB выключены; modinfo simpledrm на TCL not found. Подготовлен wayland-probe/README.md с путём kernel(simpledrm)→Weston DRM/pixman, отдельно от nativeMSM/LT8911/GPU. Пакеты/ядро пока не менялись.

Подготовлен show-fb-test.py с проверками формата1920x1080/32bpp/stride7680 и текущей текстовойVT. По SSH запущен30секундный тест полос/сетки/надписи TCL LINUX EFI TEST. Получено сообщение DISPLAYING; ожидается автоматическое восстановление и визуальная обратная связь пользователя.

Визуальный тест завершён: RESTORED framebuffer and text VT, exit0. Пользователь подтвердил «всё отлично!». Фактический вывод полос/сетки/надписи проверен пользователем; консоль автоматически восстановлена. Это EFI software rendering, не Wayland/DRM/GPU. Изменений загрузки/ядра не выполняли.

Отчёт визуального теста/Wayland ожидает Lavtomate confirmation067d21c3-19d1-4a43-a131-72c25bb417ad, Etersoft#19084/work_time10. Не повторять отправку. Комплект /var/ftp/tmp/lav/tcl/wayland-probe/ сохранён.

### 2026-09-08: simpledrm отложен по уточнению пользователя

Пользователь считает simpledrm неинтересным; промежуточная сборка не запущена. Возвращаем приоритет native DRM MSM/DPU/DSI/LT8911EXB и Adreno для Wayland. Начат поиск существующих DRM bridge реализаций для сопоставления с OEM TCL.

### 2026-09-08: предложение Armbian и найденная DRM-основа

После отказа от simpledrm пользователь предложил новое ядро Armbian. Проверены uefi-arm64 configs: current6.18, edge7.2 (это настройки дерева, готовый пакет не проверен). В current включены основные Qualcomm drivers, часто modules. SpacemiT сборка RISC-V непригодна как бинарное ядро TCL, но найден настоящий LT8911EXB drm_panel/mipi_dsi driver chainsx/linux-spacemit и Armbian backlight patch. В драйвере SYNC_PULSE против OEM TCL event, GPIO/инициализация требуют адаптации. Оценка/ссылки в wayland-probe/armbian-assessment.md. Сборку/установку ядра не запускали.

## 2026-09-08: проверен готовый Armbian 7.1.8 ARM64

Официальный apt.armbian.com, resolute/main/binary-arm64, Release от 2026-09-07. Пакет linux-image-edge-arm64 26.8.3, kernel release 7.1.8-edge-arm64, ARM64, 87763228 байт. SHA256 08413acb920ef695c4042643dfa1d7f51eae562364e41bce640b1191ab7482b1 совпал с Packages.gz. Подпись Release на этом шаге не проверялась; до установки требуется проверка доверия репозиторию. Доступный current в этом индексе — 6.18.44, edge — 7.1.8; версии сборочных конфигов не равны опубликованным пакетам.

Пакет распакован только на builder: /tmp/tcl-armbian-candidate/edge.deb и edge/. Полная конфигурация и сравнение с рабочим qc7 сохранены в wayland-probe/armbian-7.1.8/. Основные SC7180 UFS/ICE, USB PHY, Wi-Fi/modem/RMTFS, cpufreq, I2C и MSM DRM/DSI опции включены. Страница памяти 4K. Все пути файлов в modules.dep существуют. Это проверка состава, не тест загрузки или доказательство совместимости DT/драйверов.

USB DT использует qcom,sc7180-qmp-usb3-dp-phy (его поддерживает phy_qcom_qmp_combo), поэтому выключенный PHY_QCOM_QMP_USB_LEGACY сам по себе не блокирует этот PHY. Старый двухуровневый DWC3 DT требует отдельной проверки с dwc3-qcom-legacy. PINCTRL_SC7180 и многие зависимости в новом пакете модульные: initramfs должен содержать полный набор для USB rootfs/loop/Btrfs и удалённого доступа.

Важное поведение preinst: при /boot на FAT удаляет /boot/System.map*, config*, vmlinuz*, Image и uImage. Postinst запускает kernel hooks и обновляет Image/symlinks. Поэтому первый тест готовить из распакованных файлов в отдельных путях, без выполнения этих скриптов на рабочем TCL. Пакет не устанавливался; GRUB, запущенное ядро и TCL не менялись, перезагрузки не было.

Задание сборщику подготовлено (packaging-task.md). Для DEB требуется Armbian workflow; ALT hasher/girar не заменяет его. Сначала проверка готового ядра, затем при необходимости сборка с портом LT8911EXB. Следующее: проверка подписи репозитория, DT compatibility и состава нового initramfs; после этого отдельный boot entry. Native DRM/Wayland ещё не получены.

Отчёт Armbian 7.1.8 в #19084: Lavtomate pending confirmation `3eee2b56-e554-4128-8f5f-e91f13ee0442`; не повторять отправку. Файлы скопированы в /var/ftp/tmp/lav/tcl/wayland-probe/armbian-7.1.8/.

## 2026-09-08: Armbian 7.1.8 boot kit подготовлен

Проверены подпись InRelease официальным ключом и цепочка SHA256 до DEB. Проверены USB legacy/PHY alias и28ранних модулей с зависимостями. Собраны новый initramfs со всеми модулями и EFI GRUB2.12 (default=tcl-armbian-718, старые пункты сохранены); cpio проверен побайтно. Комплект и ограничения: peripherals/wayland-probe/armbian-7.1.8/README.md. На TCL пока НЕ установлен, перезагрузки не было. Следующее: поставить отдельный каталог новых модулей в Ubuntu и отдельные boot-файлы на USB, проверить суммы/backup EFI, затем аппаратный тест.

Отчёт о готовом boot kit #19084 pending Lavtomate `c7f30d43-7a8e-4005-955c-3b42a02d9727`; повторно не отправлять. Комплект скопирован в /var/ftp/tmp/lav/tcl/wayland-probe/armbian-7.1.8/.

## 2026-09-08: Armbian 7.1.8 установлен для пробной загрузки

Установка завершена на boot_id192b0902-e812-4c64-9c93-ef4b64ebb903, kernel6.18.34-stb-qc7+. Все6648файлов deploy проверены SHA256 до установки; новые модули повторно проверены в /lib/modules/7.1.8-edge-arm64. Старый каталог модулей сохранён. Новый Image/initramfs в /run/initramfs/usb/tcl-armbian-7.1.8/.

EFI установлен SHA256 d321a865be1fb4d71ee4dd29c45d24a76b21062ca3367377b071615c7a5135f9, default=tcl-armbian-718. Backup EFI/BOOT/BOOTAA64-before-armbian-718.bak SHA c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52; тот же файл на builder /tmp/tcl-i2c-boot-build/BOOTAA64.EFI. Старые пункты сохранены, для возврата выбрать TCL: I2C10 controller probe - EFI display retained.

Скрипт install.sh завершился exit0, sync закончен, свежих ошибок ввода-вывода в хвосте dmesg нет. Существующее правило UFS RO и ssh.socket enabled/active проверены. Следующее — одна пробная перезагрузка и collect.sh. Пока аппаратный запуск7.1.8 не подтверждён.

Пробная перезагрузка Armbian выполнена через systemctl reboot. Две проверки SSH .177 после ожидания — timeout. Последние строки экрана запрошены через async-вопрос. Ядро7.1.8 не считаем успешно загруженным; стадия отказа ещё неизвестна. Новые фотографии локально не найдены.

Отчёт об установке и SSH timeout #19084 pending Lavtomate `40bb08c1-0917-425c-bd97-aca8e77644a2`; повторно не отправлять.

Пользователь сообщил: после теста нет Wi-Fi-интерфейса, доступна локальная консоль. Запрошены uname -r, modprobe ath10k_snoc, ip -br link и journalctl -b -u tcl-radio -n40. В существующей конфигурации qcom_q6v5_pas намеренно blacklist; tcl-radio загружает его после проверки RMTFS и QRTR TFTP service14. Не предлагать слепой ранний запуск модема в обход этих проверок. Точное запущенное ядро пока не подтверждено.

## 2026-09-08: фото Armbian уточнило стадию отказа

Ручная расшифровка ключевых строк photo_2026-09-08_18-01-35.jpg
Источник: lav@192.168.8.186:/tmp/, получен 2026-09-08.

~ # modprobe ath10k_snoc
[ 9034.024484] cfg80211: Loading compiled-in X.509 certificates for regulatory database
[ 9034.026234] Loaded X.509 cert 'sforshee: 00b28ddf47aef9cea7'
[ 9034.027681] Loaded X.509 cert 'wens: 61c038651aabdcf94bd0ac7ff06c7248db18c600'
[ 9034.028438] qcom_aoss_qmp c300000.power-management: failed to acquire ipc mailbox
[ 9034.028591] faux_driver regulatory: Direct firmware load for regulatory.db failed with error -2
[ 9034.030160] cfg80211: failed to load regulatory.db
[ 9034.077325] ath10k_snoc 18800000.wifi: failed to register wlfw qmi client: -517
~ # ip a
1: lo: <LOOPBACK> mtu 65536 qdisc noop qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
~ # df
Filesystem           1K-blocks  Used Available Use% Mounted on
devtmpfs               3742928     0   3742928   0% /dev
tmpfs                  3824120  1936   3822184   0% /run

Выше виден хвост lsmod: gcc_sc7180, clk_qcom, icc_clk, clk_spmi_pmic_div,
clk_rpmh, pinctrl_spmi_gpio, qcom_spmi_pmic, regmap_spmi, spmi_pmic_arb,
spmi_devres, spmi, pinctrl_sc7180, qcom_aoss, smem. Полный lsmod не виден.

Вывод: оболочка initramfs, USB и Ubuntu rootfs не смонтированы. Журнал раннего
modprobe ожидается в /run/early-modules.log в RAM. Код сохранения на USB расположен
после успешного обнаружения USB, поэтому в этом состоянии он не выполнялся.
Точная строка fail/init на фото отсутствует, uname -r также отсутствует.

Дополнительная локальная проверка: CONFIG_QCOM_APCS_IPC=y в рабочем6.18.34,
m в Armbian7.1.8. Файл qcom-apcs-ipc-mailbox.ko в новом initramfs есть, но его
нет в явном списке ранней загрузки. AOSS DT mboxes=<0x20 0>; APCS mailbox
нуждается в загрузке драйвера. Это конкретный пропуск подготовки, но пока
не доказано, что он единственная причина остановки и что он объясняет QMI -517.


## 2026-09-08: после ручной загрузки APCS USB не появился

Фото photo_2026-09-08_18-08-21.jpg, lav@192.168.8.186:/tmp.
Новые строки относительно предыдущего снимка:
~ # cat /run/early-modules.log
[вывода нет]
~ # modprobe qcom_apcs_ipc_mailbox
[ 9421.109167] ath10k_snoc 18800000.wifi: failed to register wlfw qmi client: -517
~ # ip a
1: lo: <LOOPBACK> mtu 65536 qdisc noop qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
~ # ls /sys/class/block
loop0 loop1 loop2 loop3 loop4 loop5 loop6 loop7

Ранний журнал пуст, ошибок modprobe в нём нет. Это не доказывает успешный probe устройств.
После ручной загрузки APCS USB block device не появился. Повторной ошибки AOSS mailbox
в видимом новом выводе нет, но успешный AOSS probe отдельно не подтверждён.
Запрошенные devices_deferred и dmesg tail на снимке отсутствуют; нужны для дальнейшей диагностики.

## 2026-09-08: подтверждён блокер USB Armbian — QFPROM выключен

Ручная расшифровка ключевых строк photo_2026-09-08_18-12-38.jpg.
Источник lav@192.168.8.186:/tmp.

# cat /sys/kernel/debug/devices_deferred
18800000.wifi
smem    qcom-smem: failed to retrieve hwlock
88e3000.phy    platform: wait for supplier /soc@0/efuse@784000/hstx-trim-primary@25b
a600000.usb    platform: supplier 88e3000.phy not ready

Ключевые строки dmesg (прочий вывод HID/raid6/Btrfs опущен):
phy-qcom-qmp-combo-phy 88e8000.phy: unable to determine orientation & mode from data-lanes
aux_bridge.aux_bridge aux_bridge.aux_bridge.0: error -ENODEV: failed to acquire drm_bridge
platform a600000.usb: Adding to iommu group 4
usbcore: registered new interface driver usb-storage
usbcore: registered new interface driver uas
HID: видны Touchpad, Keyboard, Wireless Radio Control через I2C.
Btrfs loaded, zoned=yes, fsverity=yes
qcom_aoss_qmp c300000.power-management: failed to acquire ipc mailbox
platform smem: deferred probe pending: qcom-smem: failed to retrieve hwlock
platform 88e3000.phy: deferred probe pending: platform: wait for supplier /soc@0/efuse@784000/hstx-trim-primary@25b
platform a600000.usb: deferred probe pending: platform: supplier 88e3000.phy not ready
ath10k_snoc 18800000.wifi: failed to register wlfw qmi client: -517

Проверка полной конфигурации скачанного Armbian 7.1.8:
# CONFIG_NVMEM_QCOM_QFPROM is not set
CONFIG_HWSPINLOCK_QCOM=m
CONFIG_QCOM_APCS_IPC=m
В рабочем6.18.34 все три=y. QFPROM модуля в пакете нет.
DT: efuse@784000 compatible qcom,sc7180-qfprom/qcom,qfprom; USB2 PHY
использует калибровочную ячейку hstx-trim-primary@25b. Это подтверждённый
блокер USB текущего готового ядра, modprobe его не устраняет.
qcom_hwspinlock есть, но пропущен ранним init; APCS ранее загружен вручную.
Отдельная проблема QMP combo/data-lanes/DRM bridge остаётся неразобранной;
включение QFPROM не гарантирует полного исправления USB3/Wi-Fi.

Ошибка подготовки: проверили dependencies из modules.dep, но не все зависимости
поставщиков DT и переходы встроенных драйверов в модульные/отключённые.
Не удалять калибровочные nvmem ссылки ради обхода. Требуется пакетная сборка
с QFPROM и исправленным early init, прежний рабочий GRUB I2C10 сохранён.

Урок по запросу пользователя сохранён в .claude/memory/lesson_tcl_kernel_dt_dependencies.md и добавлен в мастер-индекс. Отдельно зафиксирована неполнота логирования и неверное раннее предположение о запуске Ubuntu.

2026-09-08: по запросу пользователя проверен официальный aarch64 индекс ALT Sisyphus: общие kernel-image-7.1=7.1.13-alt1, kernel-image-6.18=6.18.49-alt1, kernel-image-6.12=6.12.108-alt1. Отдельно qualcomm-sc7280=7.2.0-alt1 (не наш SC7180). Состав/config пакетов не проверен, установка не выполнялась. Список и источник: peripherals/wayland-probe/alt-sisyphus-kernels-20260908.md.

## 2026-09-08: сравнение ALT общего7.1.13 и SC7280 7.2.0

Полные configs и исходный SRPM изучены без установки/сборки. Таблица и рецепт: peripherals/wayland-probe/alt-kernel-review/README.md. Общее: QFPROM=m, но ATH10K_SNOC/QCOM_Q6V5_PAS off. SC7280: они включены, но SC_DISPCC_7180/SC_GPUCC_7180 off. Рецепт defconfig+misc+pmos+altlinux+sc7280 полезен как модель отдельного TCL config, большой patch целиком не рекомендован. LT8911/B220G в patch не найдены. Рабочее6.18.34 достаточно как база для native display разработки, новейшее ядро не обязательно.

Пользователь вернулся на прежний пункт: SSH подтвердил6.18.34, boot e7269aed-4e6c-42d4-a2c7-fefdad4573cd, wlan0 .177 и EFI VGA.

2026-09-08: рабочий EFI восстановлен из проверенной копии, SHA c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52, default=tcl-i2c10. Неудачный EFI сохранён как EFI/BOOT/BOOTAA64-armbian-718.failed. Новые файлы Armbian и модули оставлены для исследования, перезагрузки при восстановлении не было.

2026-09-08: iw dev wlan0 link после возврата на6.18.34: eterwifi, BSSID8c:de:f9:bf:2a:ba,5220MHz (5GHz), signal−81dBm, RX bitrate390Mbit/s VHT80MHz NSS2; это скорость радиолинка, не замер передачи. Отчёт ALT #19084 Lavtomate approved113cccf2-cb3a-4525-b48e-9f4416e9ccab, result null — ID комментария не получен.

## 2026-09-08: native display — аудит glue и DT suppliers

Найден SpacemiT-specific force-attached+/lcds, неинициализированный lcd_name при отсутствии свойства, ранние переключения GPIO, поздняя отмена delayed work и неполный unwind. Регистровая часть кандидат, glue не переносить буквально. DT дополнительно требует включения mmss-noc1740000 (сейчас disabled), не только DISPCC/MDSS/DSI. План и аудит: peripherals/wayland-probe/native-display/{driver-review.md,dt-plan.md}. Аппаратное состояние не менялось, драйвер/DTB не собраны.

Отчёт native-display audit #19084 pending Lavtomate a1722d7b-6b57-4fe9-8ea2-327cd42079a9; не повторять. Материалы опубликованы в /var/ftp/tmp/lav/tcl/wayland-probe/native-display/.

## 2026-09-08: рецепт qc7 закреплён для отдельного модуля

# Подготовка совместимой сборки LT8911EXB для qc7

2026-09-08. Драйвер не собран и не загружен. На TCL нет /lib/modules/6.18.34-stb-qc7+/{build,source} и подготовленных заголовков в /usr/src. vermagic msm: `6.18.34-stb-qc7+ SMP preempt mod_unload aarch64`. В config MODVERSIONS выключен. Это облегчает отдельную сборку, но не доказывает ABI-совместимость по одному vermagic.

Рецепт закреплён по commit b626cbd286665293e0cf9d72b97d1a980f1a9fe2:
https://github.com/hexdump0815/linux-mainline-qcom-kernel/tree/b626cbd286665293e0cf9d72b97d1a980f1a9fe2
Конфигурация config.qc7-6.18.34-stb-qc7+ SHA2568d89d4b6f3558818b468f8bd919fb283d6b503c3ae94ff4005c53891e3a93f49 побайтно совпадает с ранее сохранённым конфигом рабочего релиза. В release только один tar.gz46MB; отдельного assets headers нет (содержимое tar на этом шаге повторно не проверялось).

Рецепт использует stable6.18.34 плюс travmurav-changes-v6.12.patch из каталога v6.18, add-new-dts-files-to-makefile.patch, add-gamma-lut-support.patch, внешний fix-kernel-version/v6.12.5.patch и draft Galaxy Book DTS. Первые три патча сохранены, внешний version patch и draft ещё не закреплены. Это не готовое восстановленное дерево исходников.

В travmurav patch найдены исправления DISPCC rot_clk/GDSC при runtime suspend и назначения DPU planes. Gamma patch меняет DPU. Следовательно, текущее ядро не чистый stable и не Ubuntu generic; замена на vanilla6.18 с тем же config не эквивалентна.

## Задание сборщику (alt-packaging-agents)

1. Восстановить точный исходный stable tag6.18.34, перечисленные патчи и generated config; зафиксировать все ревизии/хеши, kernelrelease и отличия итогового config.
2. Подготовить дерево для внешнего модуля, проверить требуемые exported DRM/I2C/DSI symbols. Отключённый MODVERSIONS не отменяет проверку ABI; modules_prepare сам по себе не является успешной сборкой драйвера.
3. После реализации driver+binding из ../driver-review.md собрать отдельный LT8911EXB модуль для qc7. Не менять модуль msm или запущенное ядро без обоснованной необходимости.
4. Передать исходники, config, команды, полный build log, .ko/vermagic/dependencies, контрольные суммы и список неразрешённых вопросов. Не устанавливать/не загружать на TCL в процессе сборки.

## Питание пока не подтверждено

В сохранённом upstream6.18 Acer Aspire1 DTS mdss_dsi0.vdda=vreg_l3c_1p2, mdss_dsi0_phy.vdds=vreg_l4a_0p8. Это родственная плата, не электрическая схема TCL. До первого теста установить TCL источники по OEM firmware/таблицам либо иному прямому свидетельству; не добавлять guessed supplies в загрузочный DT. На TCL DT DSI/PHY supplies сейчас отсутствуют, узлы отключены.


## 2026-09-08: OEM eDP power table и подготовленное дерево qc7

Найдена OEM resource record disp_edp с dependencies ldoc3/ldoa4/ldoc8 и таблицами, предположительно1200/880/1800mV. Назначение линий и выбор платформенной таблицы ещё не подтверждены. Отчёт/JSON: peripherals/wayland-probe/native-display/oem-power-review.md. qc7 дерево подготовлено /tmp/tcl-qc7-build/linux-6.18.34 (olddefconfig/modules_prepare OK), но toolchain/config отличаются, два hunks fuzz1, kernelrelease без+, Module.symvers отсутствует; модуль не готов. На TCL только read-only regulator_summary.

2026-09-08: PmicDxe кодом подтверждён формат OEM vreg [mode,mV,enable],stride12/3ключа; ldoc3/ldoa4/ldoc8=1200/880/1800mV в ресурсной таблице. Связанный SDM670 header имеет4поля, поэтому расшифровка уточнена по реальному бинарнику. Назначение линий и выбор режима DisplayDxe пока не доказаны, напряжения не меняли. Evidence: native-display/pmic-prm-format.asm, oem-power-review.md.

## 2026-09-08 — диагностический LT8911EXB handoff: исходник и compile-only

Подготовлен native-display/handoff/tcl_lt8911_handoff.c: firmware-state gate (ID/D0/A8), один EDID режим1920×1080@142.52MHz, DSI4/RGB888/non-burst sync event, DRM bridge API текущего qc7. Единственные I2C writes — FF bank selection, конечный81. Нет GPIO/reset/PLL/power/training writes. EDID DTD flags1f подтверждают +H/+V. Continuous clock остаётся экспериментальным выбором. Проверены порядок MSM component bind и NO_CONNECTOR; eDP detect без callback означает connected по умолчанию, не подтверждённый физический линк.

Packaging agent выполнил только .o compile W=1 на реконструированном qc7, exit0 без предупреждений; отчёт и лог сохранены рядом. .ko/полное ядро не собирались, на TCL ничего не установлено. Ограничения ABI прежние: нет Module.symvers, отличается toolchain/config/kernelrelease. MSM перезапускает DSI/PHY даже с пассивным мостом: изображение после handoff не гарантировано. Необходимы проверка питания/unused regulators, DT graph/suppliers и совместимый комплект сборки.

SSH read-only: boot e7269aed-4e6c-42d4-a2c7-fefdad4573cd,6.18.34-stb-qc7+,deferred пуст,fb0 EFI VGA. Wi-Fi eterwifi5220MHz/5GHz,BSSID8c:de:f9:bf:2a:ba,signal−74dBm,RX PHY520Mbps,VHT80MHz,NSS2 (не измеренная скорость передачи). Перезагрузки и переключения экрана не было.

Handoff исходник/Makefile/README/compile-report/log скопированы в /var/ftp/tmp/lav/tcl/wayland-probe/native-display/handoff/. Отчёт в #19084 запрошен с work_time20min; Lavtomate confirmation fb96679b-a0d8-4f21-94d7-714c1dee637a pending. Не считать опубликованным до подтверждения; не повторять bug_add_comment.

## 2026-09-08 — согласованная сборка KMS1 и DT-кандидат

Через gh повторно проверен release6.18.34-stb-qc7+: единственный asset46116924bytes tar.gz, отдельных headers нет. Packaging agent начал локальную полную сборку Image+modules с LOCALVERSION=-tcl-kms1, kernelrelease6.18.34-tcl-kms1, -j8, тем же согласованным GCC15/binutils2.46; это новый комплект, не модуль под старый qc7. Сборка ещё идёт; лог /tmp/tcl-qc7-build/logs/kms1-image-modules.log, отчёт kms1-build-report.md. Ничего из неё на ноут не установлено.

Подготовлен native-display/kms1-dt/: база соответствует сохранённому SHA рабочего DT; включены пять дисплейных узлов, DSI→C3/1.2V, PHY→A4/0.88V, I2C bridge/graph. Регуляторные напряжения и USB/UFS/Wi-Fi не менялись. dtc exit0, новых категорий предупреждений против базы нет. Рекурсивный phandle/supplier audit57узлов не обнаружил missing/disabled suppliers; это ограниченная статическая проверка перечисленных свойств, не hardware/schema validation. DT не установлен.

SSH read-only regulator_summary: C3 use1/1200mV USB PHY, A4 use2/880mV USB PHY; подтверждает существующих framework consumers, не физические измерения. C8 отсутствует в DT, его назначение остаётся вопросом. Рабочая cmdline clk_ignore_unused pd_ignore_unused, без regulator_ignore_unused. DRMcard нет, modetest отсутствовал; через epm найден libdrm-tests и запущена установка.

Установка libdrm-tests завершилась exit0: dpkg install ok installed2.4.131-1, /usr/bin/modetest присутствует. epm предупредил о deprecated --force-yes, а epm search — о недоступном play в single-file mode; основной apt поиск/установка отработали. Modeset не запускался. kms1-dt каталог скопирован в /var/ftp/tmp/lav/tcl/wayland-probe/native-display/. Сборка KMS1 пока продолжается; четыре unused-variable предупреждения acer-aspire1-ec.c из qc7, без заявленного конечного успеха.

Bug19084 KMS1 DT/build progress: confirmation4d673144-62d0-4a61-b942-679c3b660fd1 pending, work_time15min; не повторять отправку, публикация после подтверждения Lavtomate.

### KMS1 сборка завершена

make Image modules exit0: ARM64 Image25,541,120bytes,948in-tree modules,настоящий Module.symvers. External handoff module W=1 modules/MODPOST exit0 без предупреждений/неопределённых экспортов. Vermagic msm и handoff одинаковый6.18.34-tcl-kms1 SMP preempt mod_unload aarch64. Image SHA463ad3972650ac6d7e21d532b98325c149ba738f498850a523a5715411d5a383;handoff KO SHA166e078007888e6712398255cfe1cab5f88d8d8e32ec3b85ff96fe98038b3642. Отчёт native-display/kms1-build-report.md. Старые ABI blockers остаются для загрузки под qc7, но новый согласованный комплект их устраняет. Аппаратный запуск ещё не выполнен; нужны initramfs/early dependencies/deferred KMS/logging.

KMS1 modules archive готов: /tmp/tcl-kms1-artifacts/modules.tar.gz,12248510bytes,SHA5004e5ab8a157e6fa9cef2bff9cd8d83d16927ab8bffea327960330e2b0a593d.949ko (948+handoff),cross-strip,depmod PASS,13offline modprobe --show-depends PASS; критичные USB/UFS/Wi-Fi/QFPROM/APCS/hwspinlock config совпадают с рабочим. Archive без build/source links. Kernel Image/config/Module.symvers/отчёт/handoffKO сохранены на FTP в native-display/kms1-dt/. Новый initramfs ещё не создан, установка/загрузка не выполнялись.

Финальные modules.tar.gz/build-report/critical-config скопированы на FTP. Отчёт BUILD PASS в Bug19084: confirmation bf025a39-dda4-4a2a-953b-d9a0d665f322 pending,work_time5min. Не повторять bug_add_comment; публикация ожидает Lavtomate.

## 2026-09-08 — KMS1 initramfs установлен, EFI переключён

Рабочий initramfs скопирован с TCL; BusyBox/shutdown сохранены. Новый initramfs содержит949ko,963module/metadatafiles,все побайтно сверены со staging. SHA d9fc63ce9c9d409b2133a5b3234492f06970c48b9757fd8d216e569062ca77df,size13087832. /init сохраняет ранние логи в /run/kms1-log, после обнаружения USB в /tcl-kms1/logs/BOOTID/early; если USB не появился, они остаются только RAM. Runtime systemd collector сохраняет6снимков с шагом30s в /var/log/tcl-kms1/BOOTID и наUSB, без копирования credentials. Shutdown унаследован из рабочего initramfs.

В /run/modprobe.d runtime blacklist MSM/handoff; дополнительно cmdline modprobe.blacklist. Проверка на целевой Ubuntu `modprobe -S6.18.34-tcl-kms1 -b -n -v` даёт пустой вывод, explicit --show-depends показывает все insmod; реальные модули не загружались. Существующих явных загрузок MSM/handoff в startup scripts не найдено. Зарегистрированные unused regulators удерживаются параметром regulator_ignore_unused только в новых пунктах; напряжения не менялись.

Новый modules dir установлен отдельно; /tcl-kms1/{Image,initramfs.cpio.gz,kms1.dtb} записаны и SHA/cmp проверены. EFI собран Debian GRUB2.12, script-check и arm64-efi checkPASS. Новый SHA9617e47d43a1531b055af7acf5a783f77dae8f078a6c678cf95a453e2928159b, default=tcl-kms1-deferred. Второй контрольный пункт tcl-kms1-control использует новыйkernel+initramfs со старымDT. Все прежние пункты сохранены. Предыдущий EFI backup EFI/BOOT/BOOTAA64-before-kms1.bak SHA c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52. Готовится одна перезагрузка; успех загрузки ещё не заявлен.

## 2026-09-08 — первый native DRM с видимым изображением

KMS1 загрузился: boot eb351fb4-4b70-454a-8425-1047c5c4c9aa,Ubuntu running,SSH на uptime66.5s,Wi-Fi5GHz5220MHz без телефона/ручной настройки. Чёрный экран до MSM при зарегистрированном EFI VGA; точная причина пока не доказана, builtinDISPCC уже активен. Предпоследний GRUBпункт правильный tcl-kms1-deferred. CapsLock реагирует на нажатие, не мигает самостоятельно (уточнение пользователя).

Явный modprobe msm на uptime112.76s success; DSI/PHY bound. Явный handoff на222.67s → DRMcard0/eDP-1/msmdrmfb, пользователь подтвердил картинку. Modetest query показывает активный1920×1080@60Hz,142520kHz,+H/+V,connector34/CRTC65. GPU disabled, noGPUdevice; renderD128 не доказательство ускорения.

При bind обнаружена наша ошибкаv1: devm_kzalloc вместо обязательного devm_drm_bridge_alloc, неинициализированы container/kref. WARN refcount addition0/saturation. Основной исходник исправлен, v1 отдельно сохранён. V2 собран без warnings через MODPOST, SHA3b9a0e24090f76527a86142f858e98ca70b9b2bb72d3444ceddbbfb4f0340401, ещё не установлен/загружен. Рабочий v1 не выгружаем с насыщенным refcount; проверка исправления при чистой загрузке. Подробнее native-display/first-boot/README.md и handoff/v2-report.md. Логи успешно сохранены USB+rootfs и скопированы локально. Отдельно fbdev предупреждения FBINFO_VIRTFB/refgen dummy требуют разбора.

Отчёт о первой картинке и ошибкеv1 опубликован в Bug19084 comment181083,work_time30min,confirmationd2ad6010-ba81-4df9-a854-aab6fca852f0 approved. Native first-boot logs/handoffv2/source/report скопированы на FTP, текущий boot kit README явно помечен как v1. На TCL по-прежнему загружен v1, v2 только подготовлен.

## 2026-09-08 — подготовка чистой проверки handoff v2 и Weston

Согласованный v2 archive /tmp/tcl-kms1-v2-artifacts: modules.tar.gz12250923bytes SHAee7fa426b46e41d80af700910e259403ba5ce3756c0caccba92ab725407499ea;initramfs13087776bytes SHA38ab5885c44d7a33828b552520441bd87a91da1472254aeabc8bf19f24fb416a;stripped bridgeKO SHA cfff58ea5393414a835a741e589204c4bf2e58ae7d0868ad7777b03edf35ab0b. Полное сравнение963files/949ko междуstage/tar/cpioPASS; отv1 изменён толькоbridgeKO,scripts/BusyBoxпрежние. Материалы и установщик сохранены native-display/kms1-v2/ и FTP.

Текущий TCL перед изменениями всё ещё boot eb351fb4...,6.18.34-tcl-kms1,msmdrmfb,systemd running. Запущены установка v2 на диск с backupv1 (без выгрузки активного драйвера) и epm --auto install weston. Первый Wayland тест планируется Pixman на DRM, GPU пока disabled. Перезагрузка только после завершения обеих операций.

V2 установка на TCL завершена exit0,KO/initramfsSHA совпали. Backup module /var/tmp/tcl-kms1-v1-handoff.ko,backupinitramfs /tcl-kms1/initramfs-v1.cpio.gz. Активныйv1 не выгружался. Weston пакет14.0.2-5 ARM64, установка зависимостей ещё идёт. Прочитана man weston-drm7 из точногоDEB: поддерживает --current-mode и --drm-device=card0. Конфиг первого теста mode=current,idle-time0,xwaylandfalse,lockingfalse; рендерер Pixman.

epm install weston exit0: установлен Weston14.0.2-5, weston --version14.0.2. Параметры --backend=drm,--renderer=pixman,--drm-device=card0,--current-mode подтверждены --help точного бинарника. V2 KO/initramfs SHA перед reboot повторно проверены. Запрошена чистая перезагрузка в прежний KMS1 default с v2; ядро/DT/EFI не менялись. После возвращения SSH планируется явная загрузка MSM/bridge и контроль отсутствия refcount WARN перед Weston.

## 2026-09-08 — Wayland на родном DRM работает

Чистая загрузка2256e057-bbfd-4c35-9264-e150d76a2321: SSH/Wi-Fiauto на32.5s,MSM+handoffv2 на49.16s,msmdrmfb1920×1080. V1 refcount/container WARN устранены аппаратно: не повторились,taint4096 толькоexternalmodule. FBINFO_VIRTFB/refgen dummy остаются отдельными вопросами.

Weston14.0.2 запущен через transient systemd unit tcl-weston-pixman с --backend=drm --renderer=pixman --drm-device=card0 --current-mode --socket=wayland-tcl --idle-time=0; XDG_RUNTIME_DIR=/run/tcl-weston,LIBSEAT_BACKEND=builtin. Libseat builtin успешно запустилembeddedseatd/VT-bound seat0. Log: atomicmodesetting,Pixman,eDP1enabled1920×1080@60Hz,I2Ckeyboard/touchpad libinput. DRMclients weston master=y. Запущены weston-terminal и анимированный weston-simple-shm; всеunitsactive/success. Пользователь подтвердил «всё работает!!».

Это программный Waylandrenderer с роднымDPUвыводом; GPUdisabled,3Dacceleration не проверялся. Сеанс оставлен активным, permanent display-manager/login/автозапуск DRM/Weston не настроены. Логи/конфиг/команды: native-display/weston-pixman/,скопированы сrootfs/USB;описание ограничений там. Бинарные v2bootartifacts вnative-display/kms1-v2/. Следующие направления: постоянный userlogin,GPU,полный bridge cold-start/power lifecycle.

Отчёт о подтверждённом Wayland/v2 опубликован в Bug19084 comment181085,confirmation25497634-4d93-4ddf-bbb7-81cf70c33077 approved,work_time25min.

## Устранение чёрного экрана до ручного DRM

По просьбе пользователя подготовлен отдельный ранний initramfs с теми же v2, ядром и DT; 949 модулей побайтно сверены. Новый default `tcl-kms1-early` с `tcl.kms=early` вызывает modprobe MSM и bridge до поиска USB и запуска Ubuntu. Результат, uptime и /proc/fb пишутся в early-kms.txt и сохраняются существующим collector. При ошибке загрузки модулей продолжается путь Ubuntu/SSH. Прежние deferred/control пункты сохраняют старый initramfs и поведение.

SHA initramfs-early: 0a360fbe68a1de564bd1587e337f8099d56c346d3a8da0c4547802ce5153d7a7; EFI: 2fd1dc6746c0660e40447e1a31ad30a58705820b4c723d0dbb108623e62b1af0. Исходники и validation в native-display/kms1-early/. BusyBox sh -n на TCL прошёл; modprobe поддерживает -D и применяет blacklist к явным именам с -b. GRUB 2.12 script-check/arm64 check прошли. EFI установлен с backup BOOTAA64-before-early.bak (прежний SHA 9617e47d43a1531b055af7acf5a783f77dae8f078a6c678cf95a453e2928159b). Запрошена перезагрузка для аппаратной проверки ранней консоли.

Пользователь спросил о проверке 3D: сейчас GPU disabled, Pixman работает на CPU. После включения GPU проверять Wayland GL benchmark и renderer Adreno/Freedreno; llvmpipe/softpipe означают CPU. К аппаратному 3D пока не приступали, заканчиваем early console.

### Ранний native KMS подтверждён

Boot `0b07456b-9f44-48c9-a92e-8ea948e2441b`: initramfs сам загрузил MSM/bridge v2, dmesg зарегистрировал msmdrmfb на 1.566s, до Ubuntu и SSH. SSH/Wi-Fi автоматически работают, проверены на uptime35.94s. Пользователь подтвердил быстрое появление консоли («да, быстро»). Длительный чёрный экран устранён. Текущий default GRUB `tcl-kms1-early`, прежние диагностические пункты и EFI backup сохранены. Логи и описание: native-display/kms1-early/.

Weston и терминал восстановлены вручную после проверки, оба transient service active/success. Автозагрузка теперь даёт native-консоль; автоматический графический вход пока не настроен. GPU/аппаратное 3D ещё не проверены.

Отчёт об устранении чёрного экрана опубликован: Bug19084 comment181087, work_time15min, confirmation13c462f3-db31-440b-b5ca-ade87674e35a approved. Артефакты и логи раннего запуска сохранены на FTP.

### Подготовка GPU и вопрос о Vulkan (2026-09-08)

Текущий Weston — 14.0.2 (пакет Ubuntu 14.0.2-5), не последний upstream: официальный список https://wayland.freedesktop.org/releases.html содержит Weston 16.0.0 от 2026-07-14. Обновление Weston само по себе не включает GPU.

Mesa Turnip поддерживает Vulkan на Adreno 6xx: https://docs.mesa3d.org/drivers/freedreno.html . Для Adreno 618 это основание готовить аппаратный Vulkan, но успешного теста на TCL пока нет. Сейчас GPU отключён в DT, Weston использует Pixman. Критерий проверки: vulkaninfo определяет Adreno/Turnip, затем Vulkan-приложение успешно рисует; Lavapipe означает программное исполнение и не доказывает работу GPU.

Из официального linux-firmware через GitLab API скачаны qcom/a630_sqe.fw и qcom/a630_gmu.bin, соответствующие записи A618 в исходнике ядра a6xx_catalog.c. Ревизия 488ba7eca371313167ce5f15aad6b4b6d57787f1. SHA256: SQE 1c21b527d9183487cc550dabbb3f43e555df5a977a461934fc61f0635a9aa90c; GMU da8d9b1b1f5c1a0b311f32567093b4828f3c80031dd8435f91ac13c664e173a6. Суммы сверены с content_sha256 API. Файлы и provenance.json пока в /tmp/tcl-gpu-prep; на ноут не установлены. WHENCE скачан; запрос LICENSE.qcom вернул 404, правильный файл лицензии ещё предстоит определить по WHENCE.

OEM zap firmware уже сохранена из Windows: windows-drivers/OEM/qcdx7180.inf_arm64_8bf1f859236667d0/qcdxkmsuc7180.mbn (14256 байт, SHA256 34b3a965a2cfd98495f1b2f3f06ff0254032a293a321fd32b21518525a6ac618). Перед новым тестом нужны проверка reserved-memory и поставщиков GPU/GMU/SMMU/GPUCC, добавление прошивок в initramfs раннего KMS, установка средств тестирования через epm. Аутентификация zap и запуск GPU ещё не проверены.

### Тест GPU подготовлен и установлен

Новый default tcl-kms1-gpu включает GPU/GMU/GPU-SMMU/GPUCC и zap-shader с OEM прошивкой TCL. Ядро 6.18.34-tcl-kms1 и handoff v2 прежние. Запрос прошивок обеспечен в initramfs и Ubuntu root. DT-аудит 77 узлов прошёл, 949 модулей и 3 прошивки в initramfs побайтно сверены. ZAP требует 4096 байт загрузочной памяти, выделено 8192 байта в подтверждённой UEFI области 0x80840000. Подробнее: native-display/kms1-gpu/README.md, validation.json и zap-validation.json.

DT SHA256 7c5d236bc083d97d9f9f0c5f64f7be6c5ed90741669e953c81e4bd62dc1e25b3; initramfs 7e772374cf8e43d345a062678dd969de6dbba277ed8317f0bf911317fda8fe1f; EFI 39f3c8e43c706e55cf036f6efa9cbc2b8601ac4c8972112aa12bbf4abe8120cb. Запись завершена, cmp и суммы на USB прошли. Backup EFI/BOOT/BOOTAA64-before-gpu.bak; прежний пункт tcl-kms1-early сохранён. Инициирована одна перезагрузка для аппаратного теста.

Перед тестом через epm установлены vulkan-tools и glmark2-wayland. Исходный vulkaninfo: Turnip не получает GPU ID, единственный device — CPU llvmpipe. Вывод сохранён в kms1-gpu/vulkan-before.txt. Кратковременный SSH timeout не означал новую загрузку: повторное подключение показало прежний boot ID; Wi-Fi 5 GHz 5220 MHz, сигнал -87 dBm. Причина тайм-аута точно не установлена.

### Аппаратный GPU и Vulkan заработали

Boot ID 7bf9601b-61ad-465c-a4e3-ad6b8447ab05. Новый GPU DT загрузился, ранний framebuffer msmdrmfb и автоматический SSH/Wi-Fi сохранились. Vulkaninfo с принудительным freedreno_icd.json определяет Turnip Adreno (TM) 618, integrated GPU, vendor 0x5143, device 0x6010800, API 1.3.335, Mesa 26.0.8-1ubuntu0.3. До теста был только CPU llvmpipe.

Weston 14.0.2 запущен с --renderer=gl: GL_VENDOR freedreno, GL_RENDERER FD618, OpenGL ES 3.2. Wayland vkcube --wsi wayland --c 600 с VK_DRIVER_FILES=/usr/share/vulkan/icd.d/freedreno_icd.json выбрал Turnip Adreno 618 и завершился Result=success/ExecMainStatus=0. glmark2-wayland --size 800x600 --benchmark build:duration=10.0 определил FD618 / OpenGL 4.6 Compatibility и завершился успешно (3169 FPS, score3168). Это короткая проверка одной сцены, частично одновременно с vkcube, а НЕ полный сравнительный benchmark.

Поиск ошибок GPU hang/fault в dmesg после короткого теста их не выявил. Сохраняются сообщения dummy vdd/vddcx и sync_state pending GMU; это не мешало текущим тестам, управление питанием/длительная стабильность ещё не проверены. Полные логи в native-display/kms1-gpu/hardware-results, на TCL /var/log/tcl-gpu и USB /tcl-kms1/gpu-results. Для пользователя запущен vkcube-demo на 18000 кадров и терминал. Weston GL и демонстрация — transient units, постоянный графический автозапуск пока не настроен.

Подготовительный отчёт: Bug19084 comment181088 (20 минут).

Аппаратные результаты опубликованы в Bug19084 comment181089 (10 минут); логи скопированы и проверены локально.

Подготовлены 25 температурных зон SC7180; см. native-display/kms1-thermal/README.md. Новый default tcl-kms1-thermal установлен с backup и проверкой сумм; перезагрузка для проверки TSENS и автозапуска Weston. Полноэкранный benchmark с CSV и температурой — следующий шаг по просьбе пользователя.

## Hardware confirmation

Boot ab49eb3a-f0bc-47ce-94e6-b6662673243d: all 25 zones report plausible 33.7–35.0 °C after startup. Cooling devices cpufreq-cpu0 (max state9), cpufreq-cpu6 (max13), devfreq-5000000.gpu (max6) registered, current states0. power_allocator governor active; GPU sustainable_power is estimated, as logged by kernel. Temperature-triggered throttling/critical shutdown not exercised deliberately.

Persistent Weston autostart succeeded on this boot (service active). Full standard fullscreen glmark2 with --annotate and CSV launched; telemetry every5s. See ../kms1-gpu/full-benchmark/ for procedure/results. Raw sensor indices cpu8/cpu9 name thermal locations in the SoC reference, not extra logical CPU cores (machine remains 8-core).

## Completed baseline

Boot ab49eb3a-f0bc-47ce-94e6-b6662673243d, all 33 standard scenes successful, process exit0; duration5min37s. Fullscreen1920×1080 with annotation, score381. Examples: terrain55FPS, refract129FPS, jellyfish432FPS, shadow421FPS. CSV and logs in results/.

Sampled maximum GPU temperatures: gpuss0 54.9°C, gpuss1 57.2°C; maximum among CPU-named zones47.5°C. Sampling interval5s may miss brief peaks. GPU observed267–800MHz during recording; 180MHz and runtime suspend were separately observed at idle before this full test. Checked cooling states were0; logged temperatures are far below passive GPU threshold95°C. Deliberate thermal-throttling/critical-shutdown tests not performed.

No GPU hang/fault found in saved log, but four DPU messages report missing encoder for CRTC0 during the test (vblank counter/scanout position). These remain an open issue; successful completion is not a claim of a completely clean driver log.

thermal-frequency.png/.svg plots recorded temperatures and frequency. telemetry-summary.json contains per-zone minima/maxima. All results include annotation and telemetry overhead and must be compared with the same command/configuration.

Отчёт о подготовке датчиков и автозапуска подтверждён: comment181090, work_time15min.

Итоговый отчёт о TSENS/fullscreen benchmark направлен в Bug19084; ожидает подтверждения Lavtomate 3c6f2d92-1c42-4553-8cc8-cd80007d3398 (15 минут). Не отправлять повторно; проверить confirmation_check. Копия результатов и thermal DT на FTP обновлена успешно.

DPU investigation: source suggests unsynchronized encoder->crtc lookup versus atomic legacy-state reset; hypothesis not traced. Short A/B/A36scenes did not reproduce (4→4 each phase). Same helper still present in fetched mainline and v6.18.49. Developer task and evidence: native-display/dpu-encoder-investigation/. No reboot or driver patch. Prior full benchmark report confirmed as Bug19084 comment181094.

Разбор DPU опубликован в Bug19084 comment181096 (15 минут). Задание и логи скопированы на FTP в native-display/dpu-encoder-investigation/.

По просьбе пользователя изучен ICE (не ICU): см. peripherals/ice-research/README.md. В текущем ядре BLK_INLINE_ENCRYPTION=n, FS_ENCRYPTION=n, ICE DT disabled. Для hardware probe нужен отдельный kernel config и возврат UFS qcom,ice; обычный dm-crypt не использует этот blk-crypto engine автоматически. Диск и ключи не менялись.

Пользователь поручил продолжать DPU самостоятельно. Собран локальный diagnostic fallback на внутреннюю DPU assignment, READ_ONCE/WRITE_ONCE без повторного enc_spinlock в IRQ. Три исходника изменены в /tmp/tcl-qc7-build/linux-6.18.34; originals /tmp/tcl-dpu-fix/original. Kbuild/MODPOST прошли. Подробности, patch и SHA: native-display/dpu-encoder-investigation/fix/. Проверка на железе ещё предстоит; это не заявление об исправлении.

## Hardware validation completed

Boot80d53c86-78c1-4257-b68b-1310cdb831fc, native console/Wi-Fi/SSH/Weston autostart successful. Root module checksum matches candidate. Full33-scene glmark2 fullscreen+annotation run exit0, score347, duration5min37s. At uptime217.176541 the diagnostic logged legacy lookup miss recovered via assigned CRTC. No `no encoder found for crtc` messages in the saved log. This directly demonstrates the fallback handled the condition on hardware; exact writer/callback interleaving still was not traced. Code remains a local diagnostic recovery, not upstream-reviewed patch.

Sampled GPU max60.0°C. Score lower than prior381; runs were not frequency-controlled. Mean sampled big-cluster frequency1034240kHz versus1602880kHz previously; GPU mean651.9MHz versus594.1MHz. Thus a9% aggregate-score difference cannot be attributed to this patch from these two runs. Matched CPU scheduling/frequency conditions needed for performance comparison; do not claim no regression or a proven regression.

Refresh verified during600-frame Vulkan animation: encoder vsync delta250 over4.16s after initial startup sample, ~60.1Hz with SSH/sysfs sample timing uncertainty. Mode calculation142520000/(2080*1142)=59.9993Hz. Underrun counter0. Panel refresh is60Hz even when glmark2 renders hundreds of FPS.

Saved results/ includes CSV, telemetry, before/after dmesg, DRM state, desktop log and refresh samples. comparison.json contains measured frequencies/temperature and refresh calculation. Old thermal boot with original initramfs remains; root-original MSM backup /var/tmp/msm-before-dpu.ko.

ICE source-review report published in Bug19084 comment181104 (15min). No hardware ICE enable attempted.

Аппаратный DPU recovery и измерение60Hz опубликованы: Bug19084 comment181105 (20min). Патч/модуль/EFI/логи скопированы на FTP, initramfs-dpu.cpio.gz с OEM прошивкой сохранён там с правами0600.

ICE1 hardware probe prepared: separate6.18.34-tcl-ice1 kernel enables inline/UFS crypto, no software fallback; existing GPU/thermal/DPU recovery retained. Build/archive checks passed, details peripherals/ice-research/ice1/. Baseline1MiB read hashes collected for six protected UFS LUNs. Transfer/staging in progress; not yet booted or claimed operational.

ICE1 установлен с проверкой всех сумм и резервной копией BOOTAA64-before-ice1.bak. Инициирована перезагрузка, default=tcl-ice1. Root modules отдельные, прежний DPU пункт сохранён. Hardware probe пока ожидается.

2026-09-09 ICE1 hardware: boot7b25ec7a-df79-4fb3-aaa5-67ca093e4372, kernel6.18.34-tcl-ice1, defaulttcl-ice1, Wi-Fi/SSH/Weston FD618 auto. ICEv3.1.75, AES256XTS32slots, rawkeys only; sixUFS RO1. Encryption data path NOT tested. FirstMiB fiveLUN hashes match; LUN4 differs and stable with direct read. Versus earlier Borg archive only4bytes in limits differ, NOT immediate-before-byte comparison; cause unknown. Report/logs: peripherals/ice-research/ice1/hardware-report.md. Compiler existing ALT RPM gcc-aarch64-linux-gnu15.3.1-alt4, installedAug31, not self-built. No extra reboot.

ICE1 follow-up: after~3h50 sameboot, aligned mmap+O_DIRECT firstMiB LUN4 matches earlier currentboot hash75652c87…; dd(uutils0.8.0) direct read EINVAL, cause unproven. No extra reboot. limits writer unidentified. Added explanation of blk-crypto vs Crypto API, fscrypt vs whole-root LUKS, missing FS_ENCRYPTION/EXT4 in ICE1, Android dm-default-key absent in this tree. Detailed hardware-report.md updated.

2026-09-09: CPU AES/PMULL confirmed; OpenSSL AES256GCM CPU6 A/B/A1826/104/1833MBps (~17.6x, not VPN speed); OpenSSH libcrypto3. Camera UVC720p30 ffplay Weston shown and USER CONFIRMED, transient tcl-camera-preview, no recording. epm installed ffmpeg/v4l-utils/alsa-utils. Separate reports peripherals/cpu-crypto,video-decode,camera-audio. Venus+VIDEOCC disabled, firmware absent at expected path: hardware decode not operational. ALSA no cards, LPASS disabled and sound card undescribed: audio bring-up pending. No reboot.

Weston screenshot self-check SUCCESS: Super+S injected via identified keyboard, authorized compositor client saved1920x1080PNG; viewed locally, real camera frame present (dark/noisy). Direct screenshooter SSH launch unauthorized by design. Procedure in camera-audio/README.md; image private, not on public FTP/bug. Reports CPU/video/camera Bug19084 comment181117.

2026-09-09 Venus1 installed, NOT yet hardware tested: defaulttcl-venus1 uses same6.18.34-tcl-ice1 Image/initrd, newDT enablingVIDEOCC/Venus plus5MiB no-map0x85b00000. OEM qcpilext confirms poolbase0x85b00000 and Venus reservation5MiB, exact runtime placement still inference. DT audit250nodes PASS. Firmware VIDEO.VE.5.4-00064-PROD-1 upstream48d27ba...,sha256db2a2efe...; old EFI backupBOOTAA64-before-venus1.bak. Driver autoload blacklisted; manual probe after SSH and log collector. Teststream300frames1080p30 ready; compare software/hardware required. Package/install passed. Reboot planned.
Audio correction: matchingTCL SUBSYS27822202 selects AUDD_CLS_7180. Realtek settings in qcauddev_ext are QSP-only forQSIP7180, not evidence of TCL codec. CLS analog speakerPA usesGPIOUID1/2, ExtSpeaker/HeadsetCodec0; binaryPDB pathES_CODEC/Emdoor_QL328 hints customization but proves no chip model. Audio DT not changed.

2026-09-09 03:10 UTC: полный Big Buck Bunny скопирован на USB (725106140 bytes) и запущен с начала полноэкранно, unit tcl-bbb-preview, ffplay -an -vcodec h264_v4l2m2m. Unit active, журнал подтверждает /dev/video2 qcom-venus. В стартовом журнале один VIDIOC_G_FMT ioctl, corrupt decoded frame пока нет. Полный просмотр/оценка плавности ещё не выполнены. Venus1 artifacts и runtime-logs-8919be62.tar.gz опубликованы /var/ftp/tmp/lav/tcl/video-decode/venus1/.

2026-09-09: пользователь ответом «да» подтвердил, что контрольный mpv с программным H.264 (--hwdec=no) при прежнем GPU-выводе gpu-next/OpenGL/Wayland на FD618 показывает плавнее и без искажений. Аппаратный mpv ранее имел стартовые артефакты и менее плавное движение. Это сужает поиск к аппаратному пути Venus/V4L2/FFmpeg и его взаимодействию с mpv, но не доказывает дефект самого аппаратного блока либо конкретного компонента. Программный режим — подтверждённый визуальный контроль; аппаратный пока не считать полностью исправным. Просмотр не прерывался.

## ADSP1 hardware result: PASS transport, sound still absent

Boot3f72d7f5-839f-4deb-9d1a-afd403ce9cce. Wi-Fi/SSH returned, ADSP remoteproc1 offline until firmware supplied. OEM firmware hash24d665684966cab76eb0ca58536cd96f9e978267f52ffef1dd300d8e3dfe2860 verified then manual start succeeded. Modem remains running. Q6CORE/AFE/ASM/ADM drivers bound on aprbus; AFE DAIs/clocks/routing registered. One-shot module query confirmed API versions3=5,4=7,7=2,8=4 (ret0), then unloaded. readiness API1 alone not proof due unsupported-command fallback. Concrete versions provide live command/response evidence.

ALSA still no soundcard. Q6ASM frontend dai probe failed No dais found in DT (-22), expected absent board PCM child definitions. Exact physical codec/routes unresolved. Generic AFE DAI list is not board hardware enumeration. FASTRPC no reserved DMA memory remains separate issue. Logger initially failed before remoteproc existed; fixed empty-glob handling before firmware start. Correct APR sysfs bus is aprbus. Firmware now exists on root, so next boot may autostart once available; not yet reboot-tested.

Runtime archive /tmp/tcl-adsp1-runtime.tar.gz. Module sources/binary in query/. One diagnostic reboot, no amplifier GPIO or internal UFS writes. Next prepare PCM/machine/codec mapping; do not claim audible sound.


## 2026-09-09: live PCM и отдельная ошибка q6afe

Без перезагрузки модуль `live-q6asm/tcl_q6asm_live.ko` через OF changeset добавил dai@0 (reg=0) в Q6ASM; повторный bind q6asm-dai успешен. CONFIG_OF_OVERLAY=n этому не мешает: используется OF_DYNAMIC, не интерфейс DTBO. Модуль намеренно без module_exit, не выгружать принудительно: дерево используется потребителями.

`diagnostic-card/tcl_adsp_card.ko` создал TCL-ADSP-Diagnostic с dummy codec, MultiMedia1 → RX_CODEC_DMA_RX_0 (0xb030). Это искусственная диагностическая карта, физический кодек и усилитель ещё не настроены.

- Явная channel_mask=3: DSP отвергает AFE_PORT_CMD_SET_PARAM_V2 (0x100ef), error=2, hw_params -EINVAL.
- channel_mask=0: драйвер вычисляет 3, hw_params проходят; позднее aplay завершается write EIO. ADSP остаётся running. Звука пока нет.
- В q6afe_cdc_dma_port_prepare отсутствует перенос ненулевой входной маски. Отдельная бага https://bugs.etersoft.ru/19478 блокирует https://bugs.etersoft.ru/19084 (связь установлена). Подробности q6afe-channel-mask-bug.md. Исправленный модуль ещё не установлен.
- Текущая диагностическая карта оставлена с channel_mask=0; PCM закрыт, маршрут MultiMedia1 включён. GPIO усилителя не переключались.
- Логи на TCL: /var/tmp/tcl-adsp1/pcm-zero-test.log и pcm-zero-default-mask.log.

OEM ACDB: Speaker_cal device 0x45 = SPEAKER_OUT (совпадает с CLS INF), DEVICE_SPEC_INFO содержит путь 0x11103 и порт 0xb030. Codec_cal путь 0x11103 назван HEADSET_SPEAKER_STEREO. Строка CDCNAME WCD938x.1.0 присутствует также в QSP: это ещё не доказательство физического WCD938x на TCL. Остальные поля не расшифрованы окончательно.

2026-09-09: TODO следующего ядра вынесен в `.claude/docs/tcl-b220g-data/kernel-next-TODO.md`: DYNAMIC_DEBUG, FTRACE/function graph, KPROBES/KPROBE_EVENTS, OF_OVERLAY и отдельный загрузчик, q6afe fix19478, сохранение рабочих драйверов/логгера. Это план, не установленная конфигурация.


## Аппаратные проверки 2026-09-09

Boot 2109ce1f-a976-4457-b4f1-35f313948d45, kernel6.18.34-tcl-audio2, systemd running. Одна штатная перезагрузка вернула доступ после прежнего GLINK D-state. Wi-Fi автоматически на5220MHz/5GHz, Weston GL active, msmdrmfb, modem+ADSP running, все четыре APR сервиса. UFS sda..sdf readonly1, USBsdg rw.

1. Явная channel_mask3 теперь проходит hw_params; прежней AFE cmd100ef error2 нет. Q6ASM open10db3, mediafmt10d98, run10daa отвечают status0. PCMwrite по-прежнему EIO.
2. rmmod card/q6asm_dai/q6asm; новая загрузка q6asm с дополнительным WRITE_DONE dev_dbg; q6asm_dai+card mask0. APR service7 нормально отвечает, map-timeout отсутствует. При period12000frames, buffer48000: RUNNING, appl_ptr48000, hw_ptr0. Первый WRITE48000bytes адрес1:fff80000, map handleb0a06c88 принят send52/52. WRITE_DONE приходит через1.13s непосредственно перед ответом CLOSE, words fff80000,1,b0a06c88,0. По времени похоже на возврат при закрытии после таймаута, не доказательство нормального воспроизведения.
3. Закрыта карта, unbind только APR rpmsg-канала вернулся в ту же секунду. Канал и APR4сервиса автоматически восстановились через~2s. D-state не обнаружен, DSP не перезапускали.
4. После восстановления снова cardmask3, open/media/run status0, тот же поздний PCM EIO, таймаута map и ошибкиAFE нет. Карта оставлена подключённой, PCM закрыт; диагностическая печать q6asm выключена.

ЗВУКА НЕТ. Карта dummy-codec; физический кодек/выход/усилитель не настроены. Гипотеза: неработающий последующий аппаратный тракт/тактирование, пока не доказано. Linuxdummy ALSA не подтверждает физический WCD938x/ES83xx.

Пользователь попросил дождаться проверки настоящего звука перед новым отчётом в багу. Новые runtime-результаты пока сохраняются локально, баги не закрыты. Для визуально-звукового теста выбран прежний Big Buck Bunny в mpv с software decode и низкой громкостью; пока не запущен, так как PCMнеработает. Проверить громкость до включения.


## VA boot подтверждён после ручного выключения/включения

2026-09-09 boot ecc340af-a306-4585-b154-186000cdcc05, 6.18.34-tcl-audio2, systemd running. Пользователь видел чёрный экран после предыдущего штатного reboot из Oops; CapsLock реагировал, SysRqB нет. После аппаратного выключения и включения загрузился выбранный VA DT. Точный этап предыдущей остановки не установлен.

- AON62780000 → lpass-gfm-clk, VA62770000 → va_macro успешно bound.
- q6afe clock provider теперь дочерний APR service4 (проверено readlink), прежнего NULL-pointer Oops нет. Taint4096 только внешний handoff/диагностические модули, без DIE128.
- Wi-Fi/SSH/Weston и modem/ADSP автоматически работают.
- В журнале Unknown cmd0x100f6: это ответ на AFE_CMD_REMOTE_LPASS_CORE_HW_DEVOTE_REQUEST. Callback не имеет case для этой fire-and-forget команды; строка сама по себе не означает отказ DSP (ненулевой status печатался бы отдельно). Не считать первопричиной отсутствия звука.
- КартаTCLADSPDiagnost снова создана, явнаяmask3 проходитhwparams, PCM writeEIO сохраняется. PCM закрыт, маршрутMultiMedia1 включён. Ненулевых аудиоотсчётов и переключенийGPIO усилителя не было.
- Полный scontrols сохранён: Volume/Gain/Master/Headphone/Speaker регуляторов нет. Это всё ещё dummy-карта с маршрутамиDSP, не физический аудиокодек. Фильм не запущен; низкая громкость5/max20 только в черновике сервиса.
- Результаты va-boot/, архив /tmp/tcl-audio2-va-boot.tar.gz. В багу новый отчёт не отправлять до проверки реального звука, по указанию пользователя.

Следующий шаг: RXmacro62600000/SWR_RX62610000 (адреса подтверждены Windows кодом), корректные clocks/pinctrl/physical codec. В текущем bootDT phandle q6afecc и AON имеются, VA fsgen provider может не иметь phandle, так как на него пока нет ссылок. Не заменять APR clockprovider через обычный runtime-overlay повторно: OF notifier использует platform-parent lookup и теряет APR-parent. Новые SoC platform nodes этой проблемы не имеют. Проверить полный фрагмент и fsgen-ссылку до применения; не добавлять перезагрузки ради мелких изменений.


## 2026-09-09: RX macro and SoundWire identification, no reboot

Boot ecc340af-a306-4585-b154-186000cdcc05, kernel 6.18.34-tcl-audio2.
Runtime SoC overlay added RX macro at 0x62600000, compatible sm8250-lpass-rx-macro. Clocks q6afe phandle0xda IDs57/58/102/103, attribute1; VA fsgen supplied through clkdev alias because VA node has no phandle. APR provider unchanged. rx_macro driver bound; component visible in ALSA. LPASS codec version reported v1.0. RX output clock lpass-rx-mclk=9600000 Hz. Overlay module persistent until reboot; no forced removal.

Separate module enabled RX clock, read identification registers at 0x62610000, disabled clock again. No direct SoundWire register writes, no IRQ registration and no amplifier GPIO writes.

| Register | Value | Interpretation |
|---|---|---|
| HW_VERSION 0x000 | 0x01050001 | SoundWire 1.5.1 |
| COMP_PARAMS 0x100 | 0x02684026 | DOUT capacity6, DIN capacity1 (Linux masks bits4:0,9:5) |
| MASTER_ID 0x104 | 0x00000002 | controller ID2 |
| CFG 0x004 | 0 | controller not enabled |
| STATUS 0x014 | 0 | frame generator not enabled |

Taint4096 unchanged (OOT only), Weston active and SSH working after both probes. No audible sound yet. Next: identify IRQ ordering from OEM resources and actual port assignments; SM8250 template has5DOUT/0DIN and cannot be assumed exact from controller capacity alone. Windows disassembly immediate0x147..149 at tracing call sites are diagnostic IDs, NOT evidence of IRQ mapping. ACPI resource values remain evidence of IRQ set only. New bug publishing remains paused until sound test.
Sources in rx-live/. Boot files unchanged: runtime RX will disappear on reboot and must later be integrated into a verified boot DT.


## 2026-09-09 SoundWire probe and LPI experiment

Same boot ecc340af-a306-4585-b154-186000cdcc05. Found matching downstream resources in https://github.com/LineageOS/android_kernel_xiaomi_sm6150/blob/lineage-22.2/arch/arm64/boot/dts/qcom/atoll-audio-overlay.dtsi : RX62610000 ID2 SPI297, TX62630000 SPI296, WSA62650000 SPI295. Matches TCL ACPI329/328/327 after GIC offset32. Five RX output ports are used despite hardware capacity6. Applied mainline SoundWire1.5.1 RX overlay using actual RX phandle0xdd, SPI297 edge, SM8250 five-port timing template. Driver bound, sdw-master-2-0 created, actual GIC329 interrupt received. No slave detected, bus clash reported. This is not proof of codec failure.

Full qualcomm-registers debugfs dump reads FIFO registers and generated read FIFO underflow errors itself. Do not repeat whole-register dump as passive diagnostic.

Atoll LPI confirms GPIO3=RXclock and GPIO4/5=RXdata, func1, pin base627c0000, slew physical6295a000. Mainline SC7280 mux assignments match these three pins. Initial test hog on pinctrl itself failed because driver registers groups after devm_pinctrl_register; no groups available during hog application. Removed failed overlay3 and applied LPI plus separate consumer. Pins muxed, but slew configuration Oops in lpi_config_set+0x1ec. ROOT CAUSE OUR DT: mainline driver adds LPI_SLEW_RATE_CTL_REG=0xa000, while test supplied already-offset base6295a000 with length1000. Correct resource must be base62950000 length10000. Translation fault is beyond mapping; not evidence of unsupported peripheral. Corrected DT source saved, UNTESTED. No boot files changed.

Taint now4224 (DIE128+OOT4096); SSH still works. Stop hardware mutations pending user-authorized reboot. Existing boot default tcl-audio2-va does not include temporary RX/SWR/LPI overlays, so reboot clears them. Do not reload failed modules or force-remove in this boot. Full Oops and all source variants in swr-lpi-live/. No audible sound; no new Bugzilla publishing until sound test. Next clean boot: RX overlay, corrected LPI controller+separate consumer FIRST, then SoundWire; inspect enumeration before amplifier or playback.


## 2026-09-09 clean reboot and fixed LPI success

User authorized reboot. systemctl reboot completed; SSH/Weston returned automatically. New boot c781c4d4-cd36-4a07-b734-40b059ac3e52, kernel6.18.34-tcl-audio2, taint4096 only. Applied RX overlay id1, then corrected LPI overlay id2 with separate consumer and slew resource62950000/10000, then SoundWire overlay id3. RX pinctrl client active, GPIO3/4/5 mux and configuration applied without Oops. SoundWire master2 registered without previous bus clash, no slaves enumerated. No audio played, no PA GPIO writes. Boot files unchanged; these are runtime experiments.

Critical next constraint: OEM AUDD GPIOUID0 maps ACPI GIO0 pin58; PA uses46/47. GPIO58 is excluded by existing gpio-reserved-ranges=<58 5>, which previously cured reproducible TLMM boot hangs. Do NOT blindly remove exclusion or directly read/write reserved GPIO MMIO. Exact failing pin within58..62 was never isolated. GPIO58 role as codec reset remains hypothesis, not established fact. GIO0 ACPI resource03400000 size00c00000; Linux TLMM tile mapping must be compared with qcgpio.sys. Local disassembly saved /tmp/tcl-audio-research/qcgpio-disassembly.txt. Simple opcode search finds no SMC/HVC, which does not exclude an indirect firmware interface.

Current regulator list does not establish codec supplies. Atoll L10A/L15A/BOB assignments must NOT be copied blindly: TCL ldo10 visible WiFi3.3V, whereas Atoll codec wants1.8V. Need OEM board-specific power mapping. GPIO46/47 remain out-low. Whole SoundWire debugfs register dump avoided (FIFO read side effects). No new bugs published pending real sound test.


## 2026-09-09 codec-presence and Windows GPIO addressing

Clean boot c781c4d4-cd36-4a07-b734-40b059ac3e52 remains, taint4096, Weston active. Selected read-only SoundWire registers collected via module using pm_runtime_resume_and_get on bound controller; FIFO registers excluded. Result at uptime536.452168:

| Register | Value | Meaning |
|---|---|---|
| CFG 0x004 | 0x00000003 | controller enabled, pulse IRQ |
| COMP_STATUS 0x014 | 0x00000001 | frame generator enabled |
| IRQ_STATUS 0x200 | 0x00004000 | raw status; not decoded as a new failure |
| ENUM_CFG 0x500 | 0x00000001 | auto enumeration enabled |
| BUS_CTRL 0x1044 | 0 | snapshot |
| MCP_STATUS 0x104c | 0 | bank0 |
| SLAVE_STATUS 0x1090 | 0 | no attached slave status |
| ID1/ID2 slot1 0x538/0x53c | 0 / 0 | no enumerated identity |

Linux qcom_swrm_enumerate calls sdw_slave_add(bus,&id,NULL) for unlisted devices. Therefore missing codec DT alone does not explain absent slave sysfs entry: hardware slave-status is zero with active frame generator. This narrows next investigation to physical codec power/reset/link, but does not identify the faulty member.

Windows qcgpio.sys static analysis: functionVA140002430 probes three tile candidates at resourcebase+tileoffset+(pin<<12)+0x10, selects bit0 and caches tileoffset atVA1400092a0. TableVA140007158 has0x100000,0x500000,0x900000. With ACPI GIO0 base0x03400000 these yield0x03500000/0x03900000/0x03d00000, exactly Linux tile bases. GPIO routines e.g.VA1400024a8 directly read/write MMIO using this cache. No evidence of a simple GPIO58 renumbering or special secure path was found; this is not proof of its absence everywhere. GPIO58 is WEST in Linux and thus predicted control0x0353a000, NOT accessed. Reserved58..62 unchanged; prior boot hang was localized only to group, not exact pin. Codec reset role/polarity remains unproven. No reboot or GPIO writes this turn.

Evidence/source in codec-presence/. OEM power rail mapping remains unresolved; do not copy Atoll supply names/voltages to TCL. No sound and no new Bugzilla report.


# Windows audio GPIO control: partial static reconstruction

2026-09-09. Offline analysis of preserved qcauddev7180.sys, no device writes.

Registry reader VA14004f858 allocates 40-byte records. Confirmed fields by string references and store destinations:

| Offset | Field | Evidence VA |
|---|---|---|
| +0x00 | GPIOUID | 14004f9f4..14004fa04 |
| +0x04 | ACPI resource INDEX | 14004fa58..14004fa60 |
| +0x08 | INITIALVALUE | 14004fab8..14004fac0 |
| +0x10 | I/O target handle, initially NULL | used14005d4c0, initialized14004fb20 |
| +0x18 | active/valid marker, initialized1 | 14004fb08 |
| +0x1c | reference counter, initialized0 | increment14005d780, decrement14005d898 |
| +0x20 | cached/override state, exact initialization unresolved | 14005f2cc |

AUDD global table pointer VA1400237b0 and countVA1400242a8; MBHC has separate table. Lookup14005d380 takes group0=AUDD/group1=MBHC and searches GPIOUID, not INDEX. Read helper14005d3f8 sends I/O control0x480000 with output1byte. Write helper14005d560 sends0x480004 with input1byte through140047260. GPIO control14005d5e8 selects UID and writes byte1 for requested state1, byte0 for state0, with reference counting for AUDD. Thus the observed path is ordinary GPIO I/O target requests; it does not itself imply a direct secure-monitor operation.

Value dispatch around14005f26c supports requested0/1, requested2 loads INITIALVALUE, and requested-1 toggles cached state (xor1). Therefore INITIALVALUE is a default to restore, NOT an active-low flag. This corrects a possible misleading inference from INF alone. Actual GPIOUID0 caller and reset timing remain unresolved. No conclusion that GPIO58 can safely be accessed under Linux. Reserved GPIO58..62 stay excluded.

Firmware PMIC resource binaries29a524/2e257c contain LPASS register labels but string search did not identify a named external-codec power client. This negative search does not prove missing supplies or firmware support. Do not infer voltages from Atoll rails.

Next static work: trace UID0 use through the codec OS callback table and reconstruct GPIO target open from ACPI resource INDEX. Hardware remains at working RX/LPI/SoundWire master with no slave responses. No sound test or bug publication.

## 2026-09-09 — OEM codec GPIO58 reset request sequence recovered

Static analysis of qcauddev7180.sys SHA256 `923326aa1538fc699e52a11d6c5f594ea086d1abc31b76740005626deff6fe3e`; evidence in `windows-gpio-flow/codec-reset-chain.asm`, `codec-reset-pe-evidence.txt`, `qcauddev-imports.txt`.

| Stage | Confirmed addresses / values |
|---|---|
| Callback registration | 14003624c installs 14003d590 into init structure; 140051e6c reads offset0x58; API command0x601 reaches140064f88 and stores pointer at140022148 |
| GPIO backend | 14006d990(selector,state), static table14001c2a0 stride12; selectors0..7 have backend5; produces callback outer opcode4 |
| Board mapping | 14003d800 dispatcher: selector7 -> AUDD group0 UID0; selector0 -> MBHC group1 UID0; selectors1..4 -> AUDD correspondingUID |
| Board resource | OEM INF AUDD UID0 INDEX0 INITIALVALUE0; DSDT AUDD first GpioIo is GIO0 pin0x3a=58 in both BSID branches |
| Reset request | 14006c768 calls setter(7,0); 14006c770 delay(5); 14006c77c setter(7,1); 14006c784 delay(2) |
| Timing units | delay14000b030 ->1400088a8 multiplies input by -10000 and calls IAT140018188, independently resolved through PE import table as KeDelayExecutionThread: requested relative intervals5ms and2ms |
| Caller | 14006c9dc calls reset routine14006c6b0 in initialization branch w20==1, after two successful140081b48 calls, before14006c4f0 calls with0and1 |
| Disable path | 14006be9c requests setter(7,0) |

This supersedes the previous unresolved UID0 caller/timing note. The code requests low -> wait5ms -> high -> wait2ms, strongly supporting active-low codec reset. These are software-requested intervals, not measured electrical timing. GPIO writes go through the previously reconstructed IOCTL0x480004 path with reference-count/override gating; static calls alone do not prove that every invocation produces both electrical edges. INITIALVALUE remains a default value, not a polarity flag.

Live read-only verification: same boot c781c4d4-cd36-4a07-b734-40b059ac3e52, uptime21min, taint4096, Weston active, SoundWire only sdw-master-2-0, no newly logged kernel Oops. Existing selected register snapshot still shows no slave response. No GPIO58..62 access, voltage change, bootfile change, reboot, PCM test, or Bugzilla publication performed.

Remaining blocker: establish safe Linux access/ownership and exact tile for GPIO58, plus board-specific codec supplies. A prior boot hang was isolated only to group58..62, not to pin58 individually. Windows dynamically probes GPIO tiles; Linux SC7180 chooses a static tile. Do not infer a safe register write or remove the whole reservation from this reset-sequence evidence. Also decode the two140081b48 prerequisite calls and clock-vote request preceding the pulse before designing a hardware test.

## TCL OEM audio power prerequisites — 2026-09-09

Confirmed by preserved board ACPI and static OEM driver analysis, then read-only CMD DB probe on live Linux. No power votes or GPIO writes performed.

## OEM board tables

DSDT PEP0.APMD returns APCC in BOTH BSID branches; APCD is not selected by that method. APCC AUDD contains24 components. Do not use the pin-empty APCD component5/6 definitions as the active board configuration.

| AUDD component | APCC active PSTATE0 resource | Linux correspondence |
|---|---|---|
| 1 | PPP_RESOURCE_ID_BUCK_BOOST1_C, raw voltage0x324b00=3300000 | PM6150L id c, resource bobc1, node bob |
| 2 | PPP_RESOURCE_ID_LDO15_A, raw voltage0x1b7740=1800000 | PM6150 id a, resource ldoa15, node ldo15 |
| 5 | TLMMGPIO entries0,1,2,14 | TX LPI pin group; raw fields retained in APCC.dsl.txt |
| 6 | TLMMGPIO entries3,4,5 | RX LPI pin group, matches current Linux mux ownership |
| 3,4 | no explicit resource package | Empty table does not prove these components have no firmware-side effects |

APCC PSTATE1 clears enable in regulator tuples. Do not assume raw PMIC mode numbers map to Linux enum values. No audio LDO10_A vote appears in this APCC component list; do not copy Atoll LDO10 supply based on name alone.

## Driver path

Prerequisites140081b48 called with indices0and1 before reset construct separate bus contexts, synchronization objects and callbacks;14007ead0 then ORs masks0x3f and0x3f00 into register6295a000. It issues outer callback7 with payload(enable1,selector1) for index0, and(enable1,selector0) for index1. Callback14003d468 maps these to AUDD component5 and6. Helper14005f410 goes via140054d88 to IAT140018218=PoFxActivateComponent, then140018250=PoFxIssueComponentPerfStateChange. Import identities independently checked against saved PE IAT order. These are pin-group power-state requests, not external reset GPIO writes.

Immediately preceding the reset pulse,14006ac40 sends outer callback1 through14006a0c8. Handler14003d2e0 maps four change fields to components4,3,2,1 (offsets12,0,4,8), using values1=activate,-1=deactivate,0=no change. Thus the callback path reaches the board's LDO15/BOB components; exact which change fields are nonzero depends on prior runtime state. Do not claim every reset call necessarily enables both rails anew.

## Live comparison and CMD DB probe

Current DT regulator banks are pm6150 id a and pm6150l id c. Neither ldo15 nor bob registered in regulator_summary. This means Linux has no consumer votes for those providers in this configuration; it does NOT prove that firmware has electrically disabled the rails.

Read-only module compiled against6.18.34-tcl-audio2, loaded and normally unloaded on boot c781c4d4-cd36-4a07-b734-40b059ac3e52. Resources ldoa15=0x42200,bobc1=0x40400 are present, type4(VRM), auxiliary data4bytes. ldoa10=0x41d00 and ldoc10=0x41900 confirm A/C are different resources. No RPMh request issued. Taint remains4096. Code and exact output saved here.

Current mux ownership confirms RX gpio3/4/5 assigned to rx-pin-test. Pinconf debugfs displays empty strings, so it cannot independently verify every electrical setting. SoundWire still only master; no sound confirmed.

## Voltage constraint before hardware test

Kernel qcom-rpmh-regulator.c pmic5_pldo_lv table:1504000+8000*n;1800000 is representable (n37). pmic5_bob:3000000+32000*n;3300000 is NOT representable. Neighbours3288000/3320000. Exact min=max3300000 would reject voltage selection. Do not silently round this to3320000 or expand the allowed voltage range without establishing codec tolerance or OEM PMIC rounding behavior. CMD DB presence alone does not establish such tolerance.

Next: identify OEM BOB voltage rounding/range and remaining codec supply path; prepare regulator providers with confirmed constraints. Resolve GPIO58 ownership/tile independently before reset experiment. Reserved58..62 stay excluded. No reboot, bootfile edits, or Bugzilla publication.

## Exact OEM audio voltage test — 2026-09-09

## Correction: software voltage table is not proof of hardware quantization

0x00324b00 equals3300000 decimal microvolts exactly. Binary representation does not cause a different voltage. Linux pmic5_bob lists3000000+32000*n, which excludes3300000; this restriction is in the Linux voltage-selector table. It does not establish the PMIC request resolution.

OEM PmicDxe SHA e28eb7f6a6e3c6d41a627e478e7a124ab55057a590952084801920dfe3c02401 confirms BOB initialization at RVAad34 by pm_bob_driver.c string references. RVAae38..ae64 reads a subtype-dependent upper limit: two-byte limit times1000, or (af6c..af80) one-byte limit times32000. Crucially both paths converge ataf84: RangeMin=0 (afac), RangeMax=decoded limit (afcc), VStep=1000 (afa0,afdc). Thus32000 in this function scales the upper-limit encoding, not the request step. No firmware was executed.

Related Qualcomm source fetched via gh at pinned commit20bb8ae36c93fc16bbadda0e0a83f930c0c8a271 agrees: pm_bob_driver.c sets VStep1000; pm_pwr_alg.c converts requested microvolts to Vset by integer division. Source URLs in sources.txt. This supports issuing the original3300mV request rather than rounding it to3320mV. It does not measure the physical output or its accuracy.

## Controlled runtime tests

Fixed-purpose module uses the already-bound RPMh regulator bank devices and verifies resource addresses and absence of competing ldo15/bob DT providers. Exact active-only OEM requests:

| Resource | RPMh base | Voltage request (mV) | Mode | Enable |
|---|---|---:|---:|---:|
| ldoa15 | 0x42200 | 1800 | 7 (LDO HPM) | 1 |
| bobc1 | 0x40400 | 3300 | 6 (BOB AUTO) | 1 |

Requests bypass the Linux regulator selector table for this bounded diagnostic only. They are not a production regulator driver or persistent DT solution. Delayed work automatically sends enable0 for both resources after20seconds; script and normal module exit also perform cleanup. Voltage/mode request values remain cached; only enable votes are cleared. Other firmware clients may have their own votes, so do not describe this as measured physical power-off or full restoration of all register state. No sleep/wake votes, reserved GPIO, internal disk, or boot configuration changed.

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52. First test uptime2602s: all six requests ret0; after20s both disable requests ret0 and disable_complete1. No SoundWire slaves. Before test COMP_STATUS1; during at2605s COMP_STATUS8 (frame-generator bit0 clear). First sample coincides with3s autosuspend, so repeated with power/control=on, trap restoresauto.

Second test uptime2753s: runtime_statusactive before/during; COMP_STATUS8 already BEFORE supply enable and remains8 DURING. Therefore autosuspend alone does not explain the current register state, and the first change cannot confidently be attributed to rail power. All requests/disable ret0 again. Both modules normally unloaded. Taint4096 unchanged; Westonactive; SSHaccessible. No sound confirmed.

Important limitation: a negative slave result with frame-generator bit0 clear is not a decisive power/reset test. Next investigate the SoundWire active-but-no-frame status and missing clock/reset dependencies before testing GPIO58. Do not blindly rebind the driver: its remove() only deletes bus and disables clock, with no explicit runtime-PM teardown in this source. Reserved58..62 remain untouched. No Bugzilla publication pending sound validation.

## 2026-09-09 — SoundWire resume and wider search

Live trace: v1.5.1 hardware confirmed; full VA/RX/SWR clock chain active still leaves COMP_STATUS8 and no slaves. Two clock-stop calls return -ENODATA; subsequent frame-generator waits vary between success and failure. Causality not established. Isolated reset/re-enumeration candidate compiled, NOT loaded. No reboot, sound, or Bugzilla publication.

Broader search found a same-driver Qualcomm RFC about ignored enumeration timeout (2026-06-23), downstream SM6150 reset-vs-clock-stop paths, and AMD PM recovery discussion. No exact TCL fix found. Sources, applicability table, traces and candidate: `.claude/docs/tcl-b220g-data/peripherals/camera-audio/audio2/swr-resume-trace/README.md`. PM-core correction: device_unbind_cleanup calls pm_runtime_reinit; lack of explicit cleanup in qcom remove alone is not proof normal replacement is impossible.

## 2026-09-09 — runtime candidate tested; frame generation recovered, codec absent

Same boot c781c4d4-cd36-4a07-b734-40b059ac3e52. Normal module replacement, no reboot or bootfile changes. Original soundwire_qcom restored after both tests; temporary supplies unloaded and power controls returned to auto. Weston active, taint4096 unchanged.

| Test | Observed result | Limitation |
|---|---|---|
| Candidate v1, uptime7594–7616s | Three reset-resume cycles; six selected snapshots all COMP_STATUS1, no slaves. | Normal replacement exposed Unbalanced pm_runtime_enable on entry and restoration. No sound. |
| Candidate v2, uptime9143–9178s | Two reset-resume cycles; four snapshots COMP_STATUS1. Explicit PM disable for inherited state and removal prevents new imbalance warnings. | Test-only PM replacement handling; not a production driver patch. |
| v2 plus 20s OEM supply requests | LDO15_A1800mV/mode7 and BOB_C3300mV/mode6 all RPMh requests ret0. Three snapshots while enabled and one after disable all COMP_STATUS1, slaves0. Automatic disable_complete1. | Requests acknowledged, physical voltages not measured; reset GPIO untouched. |

The reset recovery consistently restores frame-generator bit0 in these bounded tests. This is stronger than the previous compiled-only hypothesis, but not a complete audio fix or proof that absent ClockStopNow ACK is the underlying cause. Initial probe before the first reset still logs a failed frame wait. No codec enumeration, sound playback or sound confirmation.

### Correction to PM-core interpretation

Our preceding statement about generic cleanup was incomplete. drivers/base/power/runtime.c:pm_runtime_reinit returns immediately when pm_runtime_enabled(dev). Thus qcom remove lacking pm_runtime_disable really does leave runtime PM enabled across driver unbind; the next probe logs Unbalanced pm_runtime_enable. This warning is not WARN_ON and taint stayed4096. Candidate v2 disables inherited PM at probe and disables PM at remove. Both scripts force VA/RX/SWR power/control=on for replacement; the removal code is tested under that condition only, not generalized to arbitrary suspended unbind. Do not repeat v1 for future tests.

Sources and final v2 module/script/patch saved in validated-runtime-test/. Original initial candidate files in parent are historical compiled-only artifacts. resume-candidate-result.txt corresponds to v1 with diagnostic messages; resume-supplies-result.txt corresponds to v2. The exact v1 instrumented binary was overwritten in /tmp; historical parent binary predates those messages and must not be described as byte-identical to v1 tested. v2 archived module is the tested binary.

Next investigate safe access to codec reset GPIO58 and remaining physical-link prerequisites. Existing OEM static evidence requests low5ms/high2ms; it does not prove Linux can access the pin. Reserved58..62 remain untouched because their reservation previously cured boot hang. No Bugzilla publication before sound validation.

## 2026-09-09 — команда просмотра батареи в Ubuntu audio2

На текущей загрузке c781c4d4-cd36-4a07-b734-40b059ac3e52 каталог /sys/class/power_supply пуст, /proc/acpi/battery отсутствует. Доступна установленная команда `sudo /usr/local/sbin/battery-status`; обновление `sudo watch -n 30 /usr/local/sbin/battery-status`. Она проверяет шину 888000.i2c/i2c-2 и отсутствие драйвера 2-0007, затем читает 32-байтный блок EC через i2ctransfer с таймаутом. Процент — percent_field.

Фактическое чтение uptime12507.75: ac0, discharging, percent_field63, remaining_mAh3402, voltage_mV7763, current_raw569. Это снимок на момент проверки, а не постоянное значение. Стандартный power_supply-драйвер остаётся TODO; пользователь просил команду, новые интерфейсы не добавлялись.

## 2026-09-09 — запрос баги EC и объяснение выбора DT

Пользователь явно поручил создать связанную багу драйвера батареи. Подготовлен Markdown: `.claude/docs/tcl-b220g-data/peripherals/power-storage-video/ec-driver-bug-draft.md`. bug_create для Etersoft / Отдел исследований / оборудование, P5, enhancement, aarch64, Linux, lav@etersoft.ru отправлен один раз. Lavtomate ожидает подтверждения `3554d500-c12c-41e0-82e2-6458023dbd93`. ID ещё нет. НЕ повторять создание; проверить confirmation_check, затем blocks=[19084] на новой баге и подтвердить зависимость. Пауза публикации звуковых отчётов остаётся в силе.

Выбор DT: рабочая поддержка SC7180 собрана с DT-описанием; OEM ACPI полагается на Qualcomm PEP. В DSDT батареи BAT0/ADP1 используют GenericSerialBus и зависимости PEP0/GIO0/I2C3. Нельзя обещать батарею простым acpi=force. Полноценная ACPI-загрузка не доказана невозможной; это отдельное направление. Дополнительно зафиксирован конфликт SPCR с DBG2/DSDT, но он не объявляется самостоятельным доказательством невозможности ACPI. На ARM64 при загрузке выбирается один способ описания согласно https://www.kernel.org/doc/html/latest/arch/arm64/arm-acpi.html . UEFI и DT совместимы; отказ от ACPI не означает отказ от UEFI.

Бага создана: https://bugs.etersoft.ru/show_bug.cgi?id=19490 — TCL B220G: реализовать драйвер EC/батареи с интерфейсом power_supply для Linux DT. Создание подтверждено (3554d500-c12c-41e0-82e2-6458023dbd93); blocks=[19084] применено, результат подтверждён (2a4a16ba-6063-47aa-b11b-56dfe73440c7). Основная задача19084 зависит от19490. Комментарий о связи/подготовке с work_time5 отправлен один раз; pending confirmation634e3a7b-78c4-4332-941f-e5940b65d2aa, не дублировать.

Комментарий к #19490 опубликован: comment_id181234, work_time5, confirmation634e3a7b-78c4-4332-941f-e5940b65d2aa approved. Задача создания и связывания баги EC завершена.

## 2026-09-09 — примеры ACPI на Snapdragon-ноутбуках

Штатная Windows на обследованном TCL использует ACPI; подтверждение — сохранённые PnP ACPI paths, OEM DSDT и qcpep7180.sys. Поэтому ACPI на этой плате не объявляется неработоспособной технологией. Для Linux отсутствует доказательство сопоставимой полноты ACPI-пути; наш рабочий путь — DT и Linux-драйверы.

Пример другой ОС: OpenBSD на ThinkPad X13s/SC8280XP, в рассылке разработчик Peter Hessler сообщает об использовании как ежедневного ноутбука в поездке (15.03.2024); обсуждение ссылается на qcgpio/qciic и исправления acpipci для NVMe. Источник https://www.mail-archive.com/arm@openbsd.org/msg02847.html . Поддержка модели официально перечислена https://www.openbsd.org/arm64.html . Ограничения Wi-Fi/звука в письме относятся к состоянию2024года, не выдавать за текущий статус. Это пример поддержки ACPI при наличии платформенных драйверов, не подтверждение готовности Linux ACPI на TCL.

## 2026-09-09 — подзадача перехода на ACPI создана

https://bugs.etersoft.ru/show_bug.cgi?id=19492 — TCL B220G: перевести Linux на ACPI с сохранением функциональности DT-загрузки. blocks=[19084] применено; задача батареи19490 связана текстом, искусственная зависимость между ними не добавлялась. Создание approved08523be0-6232-49cf-ac1c-71f701932042, связь approved63b65480-c388-49d4-b08b-6473581aeccf, комментарий181238/work_time5 approved78ec3896-ef47-498f-8d7f-46e546cbe369.

Полное задание: `.claude/docs/tcl-b220g-data/acpi-migration-task.md`. Включены известные ACPI ресурсы, PEP, IORT/SPCR/GTDT расхождения, EC, OEM и OpenBSD источники, план/критерии/восстановление. Дополнительно по коду текущего ядра подтверждены CONFIG_ACPI=n; GENI имеет ACPI IDs QCOM0220/QCOM0411, но TCL I2C3/I2C5=QCOM0811 с _DEP PEP0; pinctrl-sc7180.c OF-only, OEM GIO0=QCOM080D. Добавление ID без проверки ресурсов/питания не считается исправлением. Рабочую систему не меняли, не перезагружали.

2026-09-09: продолжен ACPI-аудит #19492. Отчёт и фиксированные OpenBSD исходники: `.claude/docs/tcl-b220g-data/acpi-audit/README.md`. В Linux PEP0 CID PNP0D80 уже игнорируется при _DEP, но ACPI GENI пропускает DT clocks/pinctrl/ICC. OpenBSD поддерживает QCOM0811 и QCOM080D, однако SC7180 GPIO-карта ограничена30/33/94;32 отключён из-за interrupt storm,58 отсутствует. Не считать OpenBSD готовым полным портом или решением reset аудио. Аппаратных изменений не было.

### Дополнение: GENI FIFO fallback

В текущем Linux geni_se_tx/rx_dma_prep возвращает -EINVAL при wrapper=NULL. geni_i2c_rx_one_msg/tx_one_msg при неуспехе подготовки переключаются на FIFO. Поэтому отсутствие wrapper само по себе не доказывает невозможность I²C ACPI; нужно подтвердить реальный parent drvdata, FIFO_IF_DISABLE и прохождение транзакции. Для чтения32байта уже достигается порог попытки DMA (i2c_get_dma_safe_msg_buf(msg,32)), если получен буфер. Путь FIFO fallback подтверждён по коду, на ACPI-загрузке не проверен.

Отчёт в #19492 с work_time5 отправлен один раз; pending Lavtomate confirmation6ea164f8-91d2-4c28-a92a-f75f8c03997e. Не повторять отправку.

# 2026-09-09 — первый аппаратный ответ WCD938x после GPIO58 reset

Активная задача — ЗВУК. ACPI19492 завели и отложили по явному уточнению пользователя; больше не продолжать его автоматически.

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52,6.18.34-tcl-audio2. Успешные единичные чтения GPIO58:0353a010=1;0353a000cfg=1;0353a004io=0. Это mux0,OE0,pull-down,drive0. GPIO58 WEST доступен в этой загрузке. GPIO59..62 NORTH не проверялись, reservation в DT сохранён.

OEM последовательность low5ms/high2ms проверена коротким модулем с guards machine=tcl,book14,presence1,cfg1,io0; записьlatch0,cfg200,5ms,latch2,2ms. Послеrelease cfg200/io3. Автовосстановление15s, питание20s; сценарий снимает reset/power раньше. Использованы ранее проверенные LDO15_A1800mV/mode7 и BOB_C3300mV/mode6, SoundWire reset-resume candidatev2.

Первый тест15103–15115s: четыре чтенияCOMP_STATUS1,slave_status4,ID1=01170224,ID2=0000000d. По qcom byte-order addr240217010d00: version2,unique4,mfg0217,part010d,class0. Соответствует драйверуWCD938x, не различает9380/9385. Slot1 ATTACHED. До reset при тех же запросах питания/работающемframegen ответы были0.

ИСПРАВЛЕНО 2026-09-09: sysfs slave ПОЯВЛЯЛСЯ во время теста; прежний вывод по списку после cleanup был ошибочным. Звук не воспроизводился. Все измененияGPIO восстановленыcfg1/io0, votesdisable_complete1,stock soundwire_qcom восстановлен,Westonactive,taint4096. Следующийv3 тест15958–15967s дал тот же аппаратныйответ. v3добавляетenumerate вreset-resume; послеGPIOrelease resume НЕ был вызван, поэтому hook не был проверен. Не утверждать, что явноеenumerate не помогло. ПроверитьIRQ/PM.

GPIO58 теперь адресно проверен; это не разрешение доступа ко всей58..62. Багу по звуку пока не обновлять до проверки слышимого результата. Логи первого теста и исходники рядом; второй лог сохранён рядом. На ноуте файлcandidate.ko сейчасv3,но загружен исходныйsoundwire_qcom. Перезагрузок не было.


2026-09-09 — IRQ и регистрация RX-кодека проверены полными логами.
В codec-irq-result.txt четыре выборки содержат sdw:2:0:0217:010d:00:4.
IRQ raw2/raw4, ENUM slot1 addr240217010d00: пропуска IRQ нет.
В codec-binding-result.txt (uptime22647–22660) устройство существует, но без driver,
status UNATTACHED, device_number N/A. Причина отказа привязки в
kernel drivers/soundwire/bus_type.c: sdw_drv_probe возвращает -ENODEV без fwnode
(при !CONFIG_ACPI также требует of_node). qcom dynamic discovery вызывает
sdw_slave_add(..., NULL), а dev_num новому slave остаётся0; DT pre-registration
должна обеспечить штатные match, dev_num и binding. Не добавлять polling/повторный
reset ради обхода отсутствующего DT. Новый RX-only overlay содержит compatible
sdw20217010d00, reg<0 4>, rx-port-mapping<1 2 3 4 5> по binding WCD938x.
Это ещё не проверка звука и не доказательство модели9380/9385. ACPI отложен.


## 2026-09-09 — RX WCD938x Attached: проверено без перезагрузки

| Проверка | Результат |
| --- | --- |
| RX DT overlay | Применён id4, /soc@0/soundwire@62610000/codec@0,4; остаётся до перезагрузки |
| DT + candidate v4, uptime22954–22966 | Driver wcd9380-codec привязан, но framegen8 и UNATTACHED |
| Причина второго отрицательного теста | v4 проверял list_empty(bus.slaves); DT создал slave заранее, поэтому reset-resume больше не выбирался |
| Candidate v5 | После sdw_bus_clk_stop свежий get_device_status; при ret0/-ENODATA и аппаратном slave_status0 выбирается существующий reset-resume |
| v5, uptime23080–23092 | Четыре выборки: driver wcd9380-codec, status Attached, device_number1; COMP_STATUS1, slave_status4, ID240217010d00 |
| Завершение | GPIO cfg1/io0, питание disable_complete1, stock soundwire_qcom восстановлен, Westonactive, taint4096, тот же boot |

Логи codec-dt-binding-result.txt и codec-dt-attached-result.txt; точные исходники и ko
в rx-codec-dt/ и unattached-candidate-v5/. На ноуте файл candidate.ko теперь v5,
но загружен исходный драйвер. После cleanup DT-устройство остаётся, поскольку overlay
сохранён; питание снято, нельзя утверждать, что сейчас оно Attached. Базовый список
SoundWire теперь содержит master и один DT slave (2 entries), что учтено новым скриптом.
Это подтверждает RX enumeration и привязку штатного WCD938x SoundWire driver,
но не воспроизведение, не ALSA card и не различие WCD9380/9385.

Следующее: TX macro/контроллер62630000 (OEM SPI296/global328), pin/clock dependencies,
TX codec UID3 сверить с аппаратным ответом. Затем aggregate WCD938x с управлением
питанием/reset через драйверы и sound-card/маршруты; только после этого тихая PCM-проверка.
RX mapping1..5 взят из binding, перенос аудиоданных пока не проверен.
Багу по звуку не обновляли: ждём проверки слышимого результата. ACPI отложен.


## 2026-09-09 — TX и RX аппаратно Attached одновременно

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52, kernel6.18.34-tcl-audio2.
TX macro62620000 подтверждён qcauddev7180 VA14000b524 и atoll-audio-overlay.dtsi.
Добавлен overlay5 с qcom,sm8250-lpass-tx-macro, clocks q6afe57/58/102/103,
fsgen alias от VA62770000 по образцу уже рабочегоRX. Driver tx_macro bound,
phandle e3. Overlay7 добавляет TX SoundWire62630000, SPI296 edge, clocke3,
LPI gpio0clk/gpio1,2,14data (10mA,slew3,nopull/hold), пятьTXпортов поSM8250,
codec@0,3 compatible sdw20217010d00, tx-port-mapping2/3/4/5.
Первая попытка overlay6 отклонена -EINVAL: live tree без __symbols__, dtc -@
экспортировал символы локальных labels. Пересборка без -@ сохранила
__local_fixups__, убрала __symbols__; overlay7 применён успешно.

| Канал | Контроллер | HW ENUM address | Статус в четырёх выборках |
| --- | --- | --- | --- |
| RX | 62610000, master2 | 240217010d00 | Attached, device_number1, driver wcd9380-codec |
| TX | 62630000, master3 | 230217010d00 | Attached, device_number1, driver wcd9380-codec |

Uptime23580–23593s: candidatev5 обслуживает оба контроллера, сначалаreset-resume
с аппаратнымstatus0, затем общие питание/reset. ОбаCOMP_STATUS1, slave_status4.
TX version01050001, ID1=01170223, ID2=0000000d; UID3 теперь подтверждён железом.
ПослеcleanupGPIOcfg1/io0,disable_complete1, исходныйsoundwire_qcom восстановлен,
Westonactive,taint4096. Никаких reboot/PCM/публикации аудиобаги.
Runtimeoverlays5/7 остаются до reboot, SoundWire baseline теперь4 entries;
старые скрипты с ожиданием1/2 entries неприменимы. Новый test-codec-rxtx.sh
держит VA/RX/TX и обаSWR включёнными, снимает питание/reset автоматически.

Следующее: aggregate WCD938x driver требует reset GPIO и supplies доcomponent binding.
GPIO58 остаётся reserved в TLMM — обычный devm_gpiod_get пока неприменим.
Не снимать вслепую reservation58..62: только58 проверен. Питание пока тестовыми
RPMh votes, ещё не regulator-framework. Нужна корректная интеграция этих ресурсов,
затем определение ревизии кодека, ALSA card, маршруты и тихая проверка звука.
TX/RX enumeration ещё не доказывает работоспособность PCM/аналогового выхода.


## 2026-09-09 — WCD9385 идентифицирован прямым чтением TX

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52, uptime23858s.
Модуль tcl_wcd_id читает sdw_read_no_pm на TX slave только при Attached/dev_num1/driver,
во время удержания контроллеров, macro clocks, питания и снятого reset тестовым сценарием.
Regmap cache не использован, запись регистров кодека не выполнялась.

| Адрес | Значение | Назначение |
| --- | --- | --- |
| 3401 | 00 | CHIP_ID0 |
| 3402 | 00 | CHIP_ID1 |
| 3403 | 0d | CHIP_ID2 |
| 3404 | 01 | CHIP_ID3 |
| 34b0 | 0a | DIGITAL_EFUSE_REG_0 |

В wcd938x.c variant = field EFUSE bits4:1. (0x0a & 0x1e)>>1 =5,
CHIPID_WCD9385=5, CHIPID_WCD9380=0. Таким образом установлен именно WCD9385,
и подтверждён прямой транспорт чтения регистров черезTX, не только enumeration.
После теста оба RX/TX Attached в трёх выборках; cleanup GPIOcfg1/io0,
питаниеdisable_complete1, stock soundwire_qcom restored, Westonactive,taint4096.
ALSA cards отсутствуют, PCM не запускался, Bugzilla не обновлялась.

### Следующая интеграция: ресурсы общего драйвера

wcd938x_populate_dt_data требует GPIO reset и supplies vdd-rxtx/vdd-io/vdd-buck/vdd-mic-bias.
Atoll reference связывает первые два с L10A1.8V, buck сL15A1.8V, mic сBOB3.3V.
ИСПРАВЛЕНО: WiFi использует L10C3304mV, а не L10A; см. power-topology.md.
Не менять L10C WiFi; соответствие L10A кодеку требует отдельной проверки. TCL APCC явно
содержит LDO15_A1.8V и BOB_C3.3V, компонентов3/4 ресурсы пустые; питания IO/RXTX
требуют дальнейшего доказательства. Успешные чтения показывают, что необходимые
питания во время теста доступны, но не устанавливают их физическую разводку.

GPIO58 проверен, но reserved58..62 в boot DT. gpiochip_apply_reserved_ranges
формирует gpiodev->valid_mask при регистрации gpiochip; простое изменение DT
runtime overlay не восстановит валидность58. Не перепривязывать системный TLMM
и не менять внутреннюю valid_mask. Для штатного gpiod пути при следующей
согласованной загрузке менять reservation только на59..62 (<59 4>).
Это не требование немедленно перезагрузиться: сначала подготовить все ресурсы.
Не выдавать временные power/reset модули за постоянное управление питанием.


## 2026-09-09 — исправление идентичности LDO10 и план ресурсов

Предыдущий вывод «LDO10_A используется WiFi при3304mV» ОШИБОЧЕН:
regulator_summary показывает имя ldo10 без банка. Проверка sysfs regulator.7:
parent18200000.rsc:regulators-1, of_node regulators-1/ldo10, pmic-id=c.
Это LDO10_C, ресурсldoc10=41900. LDO10_A — другой PMIC, ldoa10=41d00;
он отсутствует среди зарегистрированных Linux regulator providers.

| Ресурс | Доказательства | Вывод |
| --- | --- | --- |
| LDO10_C | live sysfs regulators-1;3304000uV; WiFiconsumer | Не относится к предполагаемому питанию кодека |
| LDO10_A | CMD DB41d00; DSDTG6MD EXIT содержит1800000uV/enable1/mode7 | TCL firmware содержит запрос1.8V; это GPU/display table, не прямое доказательство разводки codec |
| LDO15_A | APCC AUDDcomp2; CMD DB42200; успешные тесты1800mV | Подтверждённое управляемое питание аудио |
| BOB_C | APCC AUDDcomp1; CMD DB40400; успешные тесты3300mV/mode6 | Подтверждённое управляемое питание аудио |
| GPIO58 | прямые чтения/сброс успешно; TLMM reservation58..62 | Для штатного gpiod в boot DT исключить только58 изreservation |

Atoll LDO10_A для vdd-rxtx/vdd-io теперь остаётся рабочей гипотезой, а не
опровергнутым вариантом. Связь G6MD с текущим runtime PEP и физическая разводка
до кодека отдельно не проверены. Нельзя выдавать наличие таблицы за выполненный vote.

Важные ограничения интеграции:
* rpmh_regulator_probe перечисляет child nodes один раз. Добавление ldo15/bob
  в уже bound PMIC node runtimeoverlay само по себе не создаёт providers.
* Не перепривязывать целиком PMIC banks: они обслуживают текущие WiFi/USB/display.
* BOB pmic5_bob Linux32mV selector не представляет3300mV точно; ранее проверенный
  OEM request3300mV прошёл напрямуюRPMh. Не менять напряжение на3320mV молча.
* gpiochip valid_mask вычисляется на регистрации; runtime DT изменениеreservation
  само по себе не даёт штатный доступ GPIO58.
* Стандартный WCD driver запрашивает4supplies. Не подменять отсутствующие providers
  dummy regulators с утверждением, что получено постоянное управление питанием.

Подготовлен power-topology.dtsi.txt: НЕ готов к загрузке, BOB и codec намеренно
неактивны, LDO10_A явно помечен как гипотеза. До нового загрузочного варианта
нужно решить точное представлениеBOB3300mV и согласованное управление GPIO58;
проверить полный DT, сохранить известный рабочий GRUB пункт. Перезагрузки не было.


## 2026-09-09 — BOB3300 через regulator framework проверен

Использован изолированный OOT вариант qcom-rpmh-regulator.c из6.18.34-tcl-audio2.
Match только tcl,book14-bob-test, имяdriver tcl-bob-test, только resource bobc1.
Проверяет machine tcl,book14 и CMD DBbobc1=40400. Таблица напряжения — единственная
точка3300000uV, не общий диапазон с неподтверждённым шагом. Остальные PMIC
драйверы не заменялись. Это диагностический provider, не готовый upstreampatch.

Overlay8 добавил отдельный bank tcl-audio-bob-test под18200000.rsc, pmic-id=c,
и consumer. Первоначальный consumerprobe вернул-EPERM наset_mode: DT не разрешал
смену режима. До enable не дошёл. Overlay9 добавил allowed-modes2/initial-mode2
(AUTO вDT => REGULATOR_MODE_NORMAL => hardware6). Перепривязан только тестовый
bank; существующие regulators-0/-1 не перепривязывались.

| Uptime | Операция | Результат |
| --- | --- | --- |
|24402.525| RPMh40408=6 |ret0|
|24402.526| RPMh40400=3300 |ret0|
|24402.526| RPMh40404=1 |ret0|
|24402.526| regulator_get_voltage / get_mode |3300000 / NORMAL(2)|
|24407.684| автоматический regulator_disable, RPMh40404=0 |ret0, enabled0|

Таким образом обычный consumer API работает с точнымOEM3300mV, включая
refcount/enable/disable; проблема представляла собой Linux selector table,
а не отказRPMh. get_voltage показывает запрошенное/кэшированное значение,
не измеренное физическое напряжение. Данные подтверждают только проверенную
точку3300mV; не публиковать обобщение о всём диапазоне PM6150L.

После теста consumer и provider unbound, tcl_bob_provider выгружен.
Overlay8/9 и модули tcl_bob_test/tcl_bob_mode безexit остаются доreboot,
но потребителя и provider driver binding нет. Не повторять insmod этих
overlayмодулей; для нового теста использовать существующие DT nodes и явную
привязку после проверки конфликта владельцев. Не смешивать rawRPMh powertest
с активным regulator consumer. Подтверждениеcleanup вresult.txt.
Westonactive,taint4096, тот жеbootc781c4d4-cd36-4a07-b734-40b059ac3e52.
Безreboot/PCM/аудиобаги. Следующее — совместный цикл питания LDO15/BOB через
framework и GPIO58 для aggregate WCD9385, затем звуковая карта/маршруты.


## 2026-09-09 — GPIO descriptor API для сброса проверен

Временный OOT gpio_chip представляет исключительно проверенный GPIO58 WEST,
MMIO0353a000..13. Это диагностический adapter, НЕ штатный TLMM driver.
Machine tcl,book14 и presence1/cfg1/io0 проверяются до записи. DT reservation
и TLMM valid_mask не меняются. GPIO59..62 не затронуты. Adapter нельзя
использовать одновременно с обычным владельцем GPIO58 или raw reset module.

gpiochip_request_own_desc(... GPIO_ACTIVE_LOW,GPIOD_OUT_HIGH) утверждаетreset
(физический0), через5ms gpiod_set_value_cansleep(desc,0) снимаетreset
(физический1), далее2ms ожидание. API logical0, cfg200/io3. Автовозврат15s,
нормальный module_exit возвращаетcfg1/io0 и удаляетgpio_chip.

Uptime24726–24737: RX/TX обаAttached, аппаратныеID240217010d00/230217010d00.
Прямые sdw_read_no_pm снова дали3401=00,3402=00,3403=0d,3404=01,34b0=0a
(WCD9385 variant5). Ошибок нового теста в сохранённом журнале нет.
Cleanup24737: GPIOcfg1/io0,disable_complete1,stock soundwire_qcom восстановлен,
Westonactive,taint4096,тотжеbootc781c4d4-cd36-4a07-b734-40b059ac3e52.

Питание В ЭТОМ тесте по-прежнему raw RPMh tcl_audio_power_test (20s), не
regulator framework. BOB framework проверялся отдельно ранее. Не утверждать,
что aggregate WCD driver уже работал или общий framework power/reset цикл
готов. ALSA card/PCM/звук ещё не проверены. Перезагрузок/аудиобаги не было.

Следующая конкретная интеграция: adapter GPIO без собственного descriptor
предоставляет reset descriptor общему WCD9385 consumer; отдельные supply
providers дляLDO15_A/BOB_C с проверенными запросами, IO/RXTX питание учитывает
остающуюся гипотезуLDO10_A. Исключить старый автоconsumerBOB, чтобы он не снимал
питание нового теста. Удерживать macro/master clocks до завершения bind;
после удаления aggregate сначала освободить callbacks/IRQ/device links,
затем GPIO/supplies. Старый tcl_gpiod_reset сам занимает единственныйdesc,
поэтому без изменения lifecycle НЕ годится в качестве provider aggregate.


## 2026-09-09 — общий WCD9385 ASoC component зарегистрирован; Oops при cleanup

ВАЖНО: текущая загрузка c781c4d4-cd36-4a07-b734-40b059ac3e52 ПОСЛЕ KERNEL OOPS,
taint4224 (ранее4096). Аппаратные тесты прекратить до согласованной перезагрузки.
SSH/Weston оставались доступны. Не выгружать/перепривязывать далее драйверы.

### Конфигурация объединённого теста

Overlay10: отдельные tcl-audio-ldo-test (создан толькоldo15!), GPIO58 provider,
aggregate qcom,wcd9385-codec и regulator-fixed tcl-io-existing-test-only.
LDO10_A НЕ создан/НЕ получил votes; IO/RXTX представлены условным неуправляемым
источником1.8V, что сохраняет прежнее питание. Это временная диагностическая
модель, не подтверждение реальной разводки или постоянного power-management.
BOB_C — ранее созданный provider узелphandlee8,точно3300mV. LDO15_A1800mV.
Старый BOB автоconsumer подавлялся driver_override=tcl-disabled на время теста.
GPIOprovider только58, MMIOcallback безsleep (WCD использует gpiod_set_value).
Приfree descriptor восстановлениеcfg1/io0. Новые resource providers изолированы
от обычных PMICbankdrivers. Штатный TLMM иreserved59..62 не трогались.

### Два теста

Первый (aggregate-result.txt): общий драйвер смог получитьGPIO ивключитьоба
реальныхрегулятора, но of_parse_phandle RX вернулNULL. Причина вoverlay.c:
add_changeset_property игнорирует name/phandle/linux,phandle для существующих
узлов. Лейбл __overlay__ не назначает phandle старомуcodec node. Cleanupуспешен.

Второй (aggregate-oops-full.txt содержитполныйdmesg,lsmod,журнал): диагностический
вариант wcd938x.c на tcl,book14/узлеtcl-wcd9385 приотсутствииphandle ищет реальные
RX/TX nodes поабсолютнымпутям. Это обход runtimeoverlay, не требуется в полном
bootDT с корректными phandle. Вuptime25408 обаslave bound, driverwcd938x_codec
привязан, soc@0:tcl-wcd9385 появился в debugfs/asoc/components.
ALSAcard отсутствует: это регистрациякомпонента, ещё не запускPCM/звук.
Есть предупреждения runtime PM: child SDW активируется при неактивном logical
master sdw-master2/3. Физическиеplatformmaster иmacro clocks держалисьon.
Это остаётся отдельным вопросом корректностиPM, не скрывать предупреждения.

Watchdog25439: codec detached, LDO15/BOB enable0 ret0, GPIOcfg1/io0 restored.
Далее скриптвыгрузилкодек/resources/candidateSWR ивернулstockSWR.
В25443 процессmodprobe получилOops вirq_find_matching_fwspec+0x7c;
stackof_irq_get -> qcom_swrm_probe. Taint4224. ВосстановлениеSWR НЕуспешно.

### Наиболее вероятная причина

wcd938x_irq_init создаёт irq_domain_create_linear(NULL,1,&wcd_domain_ops,NULL)
иirq_create_mapping, но sourcewcd938x.c не содержитirq_domain_remove или
irq_dispose_mapping. unbind также ихнеосвобождает. После выгрузки диагностического
wcd module вобщемспискедомeновостаётсяуказательнаops извыгруженнойпамяти.
irq_find_matching_fwspec проходитэтотсписок иразыменовываетh->ops->select/match;
это соответствует местуfault иunmappedmoduleaddressffff80007a84c280.
Это сильнаяобоснованнаягипотеза, не доказаннаяповторнымтестомисправления.
Падениеподmutexirq_domain_mutex моглооставитьзамокзанятым; не пытаться лечить
повторнымmodprobe/kill илиновымиIRQallocations. Нужнаперезагрузка.

### До следующего аппаратного теста

1. Исправитьlifecycle IRQ-domain: снять slave_irq ссылки сRX/TX с правильной
   синхронизацией SoundWire callbacks, удалитьregmap IRQchip ДОdisposingparent
   mapping, затемirq_domain_remove; покрытьbind-error/remove/rebind пути.
   Одного удалениядомена вmodule_exit недостаточно.
2. ПолныйbootDT долженсодержатьcodec RX/TX phandlesсмоментасозданияузлов.
3. Уточнить logicalSoundWiremaster runtimePM и lifecycle component_bind/unbind.
4. Не использовать старый cleanupскриптсвыгрузкойWCD безисправленияIRQcleanup.
5. Послечистойзагрузки восстановитьпроверенныеoverlay/SWR/powerресурсы,
   зарегистрироватьASoC card/маршруты итихийPCMтест. Нынешнююзагрузкунетрогать.

Всеисходники/ko/DT/testscripts сохранены рядом. Bugzillaпоаудио необновлялась;
пользовательпросилсначалапроверитьзвук. Перезагрузка пока не выполнена.


# WCD9385 IRQ lifetime fix — 2026-09-09

Исправление написано и cross-built, на оборудовании пока НЕ проверено.
Текущий ноут после Oops: bootc781c4d4-cd36-4a07-b734-40b059ac3e52,taint4224.
Никаких SSH-модификаций/новых аппаратных тестов/перезагрузок в этом шаге не было.

## Изменения

* wcd938x_irq_init проверяет создание IRQ mapping. При неудаче mapping или
  regmap IRQchip удаляет созданные mapping/domain в обратном порядке.
* wcd938x_irq_exit вызывается при unbind и при ошибке регистрации ASoC component.
  Сначала очищает slave_irq RX/TX под sdw_dev_lock каждогоslave, дожидаясь
  завершения текущего SoundWire interrupt_callback. Затем удаляет managed
  regmap IRQchip, parent mapping, domain; обнуляет поля.
* wcd9380_interrupt_callback в wcd938x-sdw.c возвращает0 приNULLslave_irq.
  Нужен вместе с основным патчем: иначе после очистки ссылки callback вызовет
  общий helper с NULLdomain, что не является корректной обработкой отвязки.
* Публикация slave_irq также под sdw_dev_lock. SoundWire bus.c вызывает
  interrupt_callback подэтимmutex. При удаленииIRQchip lock не удерживается,
  чтобы не ждатьIRQhandler под его собственнымlock.

Это исправляет найденный отсутствующий cleanup, а не только откладывает выгрузку.
Не гарантирует отсутствие других ошибок драйвера: PM предупреждения, аудиокарта,
реальная разводкаIO и воспроизведение остаются непроверенными.

## Артефакты и проверки

`generic/wcd938x-irq-lifetime.patch` — исправление двух исходников чистого6.18.34.
`generic/` — исходники и tcl_wcd_irq_fixed.ko / tcl_wcd_sdw_irq_fixed.ko.
`runtime-path/` — та же пара с отдельным временным fallback точныхDTпутей TCL;
этот fallback нужен только пока старыеRX/TX overlay узлы не имеютphandle.
`runtime-path-fallback.patch` отдельно показываетэтодиагностическоеизменение.
Названияkoвдвухкаталогахсовпадают, обязательноуказыватькаталог/сверятьSHA256SUMS.

Обе пары успешно собраны ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1
против /tmp/tcl-audio2-linux, безошибок/предупрежденийкомпилятора.
checkpatch.pl --no-tree --no-signoff:0errors,0warnings,112lines.
git apply --check genericpatch противисходногодерева:успех.
Основное деревоисходниковнеизменено, оригинальныеproductionмодулинеперезаписаны.
Это compile/static validation; runtime гонки и error injection не тестировались.

## Проверка после согласованной чистой загрузки

1. Восстановить нужные macro/SWR/codec nodes и стабильные clocks; проверитьtaintbaseline.
2. Использовать исправленные ОБА модуля: aggregate и SoundWire codec. Старый
   wcd938x-sdw безNULLguard с новым aggregate НЕ использовать.
3. Проверитьaggregatebind, списокASoCcomponents ичислоIRQdomains/IRQmapping
   до/послеunbind. Сначалаbind/unbind безmoduleunload; сборлогаавтосохранения.
4. Послеуспеха проверитьвыгрузкукодека и повторнуюрегистрациюSoundWire,
   на которой былOops. ПроверитьотсутствиеновыхOops/WARN/taintизменений.
5. Покрыть отказы mapping/regmapirq/component registration доступным fault
   injection либоотдельнымтестовымядром; этипути пока проверены только по коду.
6. Затем вернуться кALSAcard/маршрутам и тихомуPCM. Не заявлятьзвукготовым.

Bugzilla не обновлялась до проверкизвука поуказаниюпользователя.


2026-09-09: User explicitly approved reboot ("да") after async approval request.
Before reboot verified ssh.socket enabled+active (ssh.service itself disabled is
socket activation, not missing SSH), tcl-radio enabled, tcl-weston enabled.
Staged runtime-path fixed modules and restore-audio-buses.sh in
/var/tmp/tcl-audio2/tcl-irq-recovery; both SHA256SUMS OK remotely.
Executed sync && systemctl reboot. Await clean boot/SSH return; do not infer
successful reboot until boot_id changes and kernel taint is checked.
Bootfiles unchanged. Recovery script syntax checked, not yet hardware executed.


## 2026-09-09 — IRQ cleanup: аппаратная проверка после перезагрузки

Согласованная перезагрузка завершилась. Новый boot_id:
`d82e1f9e-de49-4a4d-9c36-edc5c1266009`, ядро `6.18.34-tcl-audio2`.
RX/TX восстановлены; используются оба исправленных модуля WCD.

| Проверка | Результат |
|---|---|
| Первый bind, автоматическая отвязка через 30 с, выгрузка aggregate, возврат stock SoundWire | Успех, прежнего Oops нет |
| Повторный bind через sysfs в первом runner | Не выполнен: драйвер имеет `suppress_bind_attrs = true`; это ошибка тестовой процедуры |
| Отдельный исправленный runner: два bind → detach → module unload | Оба цикла успешны, компонент `soc@0:tcl-wcd9385` зарегистрирован в каждом |
| Возврат stock SoundWire после двух циклов | Успех; Weston active, SSH работает, taint 4096 не изменился |
| Звук / ALSA-карта | Не проверен / карты пока нет |

Отвязка в повторном runner выполняется существующим watchdog через
`device_release_driver`; повторная регистрация — обычной загрузкой модуля.
Никаких изменений `suppress_bind_attrs` в драйвере не делалось.
В сумме подтверждены три успешных жизненных цикла исправленного aggregate.
Прежний Oops в `irq_find_matching_fwspec` после выгрузки не воспроизводится.
Это не проверка всех гонок и error paths. debugfs IRQ domains на этом ядре
недоступен: прямой подсчёт доменов не выполнен.

В журналах остались сообщения runtime PM о спящем логическом `sdw-master`
при привязке дочернего кодека. Также udev пытается загрузить штатный SDW
модуль, получая duplicate symbol: он не загружается, владельцем символов
остаётся исправленный модуль. Не выдавать журнал за полностью чистый.

Следующий этап: ALSA-карта с DSP FE/BE links. В живой системе присутствуют
Q6ASM, Q6AFE, routing, RX_CODEC_DMA_RX_0..7 и TX_CODEC_DMA_TX_0..5, RX/TX macro
и SDW DAI. Наличие DAI не доказывает поддержку конкретного порта прошивкой.
В локальном исходном дереве пример цепочки есть в `sm8250-mtp.dts`:
Q6AFE RX → WCD938x + SoundWire + RX macro; FE Q6ASM и platform Q6routing.
`sm8250.c` содержит SoundWire stream callbacks; нельзя считать драйвер
`sc7180.c` автоматически подходящим только по названию SoC.

Логи и runner: `audio2/irq-lifetime-fix/recovery/irq-fixed-*.txt`.
Bugzilla не обновлялась: ждём проверки звука согласно указанию пользователя.


### Дополнение: проверка логических SoundWire masters

В том же boot выполнены ещё два успешных цикла (uptime 897–907 с), всего пять.
Перед bind RX/TX кодека у обоих `sdw-master-*` временно установлено
`power/control=on` и проверено `runtime_status=active`. Предупреждение
`runtime PM trying to activate child ... parent ... is not active` исчезло
в обоих циклах. Новых Oops/BUG/WARNING в интервале теста нет; taint 4096,
Weston active, штатный SoundWire восстановлен, control возвращён в auto.

Источник предупреждения локализован в `wcd-common.c:wcd_sdw_component_bind`:
`pm_runtime_set_active(dev)` вызывается без проверки ошибки и без гарантии,
что логический родитель активен. Физический контроллер и логический master —
разные устройства PM. Следующее исправление должно корректно брать/отпускать
PM reference родителя и обрабатывать ошибки, а не постоянно запрещать sleep.
Пока это только подтверждение причины и временный контроль условий теста;
код общего WCD PM не изменён. Не смешивать с уже исправленным IRQ cleanup.

Артефакты: `irq-fixed-master-pm-{start,result,dmesg,interval}.txt`,
`test-fixed-master-pm.sh`. Карта ALSA ещё не зарегистрирована, PCM не запускался.


## 2026-09-09 — первая регистрация ALSA-карты WCD9385

Без перезагрузки, boot `d82e1f9e-de49-4a4d-9c36-edc5c1266009`, uptime 1398 с.
Собран диагностический модуль `tcl_audio_card.ko` против 6.18.34-tcl-audio2,
ARCH=arm64, CROSS_COMPILE=aarch64-linux-gnu-, W=1: сборка без предупреждений.
Он использует callbacks из `sound/soc/qcom/sm8250.c`, но разрешает DAI по
точным существующим DT-путям, без добавления phandle к живым overlay-узлам.
Это временная карта для исследования, не окончательный DT или штатная поддержка TCL.

| Звено | Разрешённый DAI |
|---|---|
| Q6ASM service@7/dais, id 0 | MultiMedia1 |
| Q6AFE service@4/dais, id 113 | RX_CODEC_DMA_RX_0 |
| tcl-wcd9385, id 0 | wcd938x-sdw-rx |
| SoundWire 62610000, id 0 | SDW Pin0 |
| RX macro 62600000, id 0 | rx_macro_rx1 |
| Platform | Q6 routing service@8/routing |

`/proc/asound/cards`: card 0 `TCL WCD9385 Test`, driver `tcl-audio-test`.
`/proc/asound/pcm`: `00-00: MultiMedia1 (*) : playback 1 : capture 1`.
Capture виден из-за двунаправленного FE; TX backend не добавлен и запись
не подтверждена. Регистрация компонента WCD теперь дошла до codec probe,
появились регуляторы WCD9385 и RX macro; полный `amixer contents` сохранён.
Первый тест PCM не запускал. Были DAPM unknown pin Headphone/Mic Jack:
в следующей версии добавлены эти widgets и RX→WCD routes из sm8250-mtp.dts.
Точная разводка внешнего усилителя динамиков пока не установлена.

После снятия карты, кодека и временных ресурсов восстановлен stock SoundWire;
Weston active, taint 4096, SSH работает. Журналы и обе версии исходника:
`audio2/card-live/`. Bugzilla пока не обновлялась — слышимый звук не проверен.


### PCM, тихий тон и проверка выходного тракта

В том же boot PCM-тест тишиной завершился с exit 0: 48 кГц, S16_LE, stereo,
состояние RUNNING, hw_ptr=43680 через ~1 с, period=6240, buffer=24960.
Первый тест тишины не включал отдельные SoundWire HPHL/HPHR/CLSH switches,
поэтому не считать его доказательством передачи полезных данных до аналогового выхода.

Затем включены порты HPHL/HPHR/CLSH и проигран тон 440 Гц: четыре импульса
за 6 с с плавными краями 40 мс. Файл -20 dBFS peak, RX digital -6 dB,
HPH analog -12 dB (ALSA value 12). Aplay завершился с exit 0, PCM RUNNING.
Пользователь явно сообщил: «звука не слышал». Это не успешная проверка слышимого звука.

Отдельная запись DAPM на потоке тишины с включёнными HPH/CLSH портами:
Q6 RX mixer On; RX_MCLK, RX AIF, RX INT0/1, HPHL_OUT/HPHR_OUT On;
WCD IN1_HPHL/IN2_HPHR, RDAC1/2, HPHL/HPHR PGA и выходы HPHL/HPHR On.
Это программное состояние DAPM, не измерение аналогового сигнала.
GPIO46/47 при этом output low, func0, 2mA, no pull, без consumer.

OEM INF AUDD_CLS_7180: speaker analog PA GroupID_DeviceID=0x45,
GPIOUID1/2 (= ACPI GPIO46/47), STAGE=2 «config after enabling device»;
INITIALVALUE=0. Усилитель в Linux ещё не моделируется. Подготовлен отдельный
5-секундный эксперимент: запросить только 46/47 через GPIO API, проверить
исходные output low, затем high после PCM RUNNING и вернуть low по таймеру
и при выгрузке. Полярность high=enable — проверяемая гипотеза, не установленный
факт из INF. GPIO58..62 не затрагиваются этим модулем. Перезагрузка не нужна.

Duplicate symbol в этих тестах — отказ загрузки штатного SDW codec поверх
уже загруженного исправленного; это не одновременная работа двух драйверов.
Исправленные модули не установлены вместо production modules. Все результаты
и исходники лежат в `audio2/card-live/`; публикация в Bugzilla пока отложена.


### Подтверждён слышимый звук; запущен фильм

Пользователь после теста с GPIO46/47: «Я слышал звук!!!».
В отличие от предыдущего неслышимого тона, добавлено только включение PA
после PCM RUNNING. Оба GPIO переведены high в uptime 3388.2146 с, возвращены
low таймером в 3393.252 с. Это подтверждённый слышимый тест через данный
тракт, но не независимая проверка каждого динамика/канала.
Карточка/кодек сняты, stock SoundWire восстановлен, taint 4096, Weston active.

По просьбе пользователя запущен полный Big Buck Bunny (9:56) с начала:
`/run/initramfs/usb/tcl-venus1/big_buck_bunny_1080p_h264.mov`.
Unit `tcl-movie-audio-v2.service`, скрипт `/var/tmp/tcl-audio2/play-movie.sh`.
mpv fullscreen gpu-next/OpenGL/Wayland, hwdec=no (прежний визуально проверенный
вариант), ALSA hw:Test,0, 48000 Hz s16 stereo, volume=15, HPH -12 dB,
RX digital -6 dB. В uptime ~3590 с PCM RUNNING; счётчик фильма прошёл 27 с,
A-V около 0.000 с. Усилитель включён. Полный просмотр ещё не завершён.
Первый запуск не состоялся из-за недопустимого `--volume-max=30`; исправлено
на 100, фактическая громкость осталась 15. Не копировать прежний ошибочный
параметр из старого bbb-sound-test.service.

PA module теперь принимает hold_seconds (default 5, max 1200); фильм использует
900. Codec watchdog default 30, max 1800; фильм использует 1000. Unit ограничен
850 с, после mpv EXIT скрипт снимает PA, карту, кодек и временные ресурсы,
возвращает stock SoundWire и auto PM. Во время фильма ничего не выгружать.
Новый фильм ещё не оценён пользователем; звук подтверждён именно коротким тоном.

TODO постоянной реализации: DT карта и phandles, GPIO46/47 как DAPM PA supply
с корректным sequencing, проверка stereo и уровней, runtime PM и IRQ cleanup,
убрать диагностический IO regulator и проверить реальное питание, автозагрузка
после ребута. Текущая работа не сохраняется автоматически после перезагрузки.

2026-09-09: по прямой просьбе пользователя громкость работающего mpv изменена с 15% на 50% через IPC; get_property volume вернул 50.0, error=success. Аппаратные уровни HPH -12 dB и RX -6 dB не менялись.
Отчёт о первом подтверждённом звуке в #19084 отправлен один раз с work_time=45; Lavtomate confirmation b9d3b0b4-176c-4199-b6d4-d45e917ffe91 pending. Не повторять bug_add_comment; проверить confirmation_check.

2026-09-09: по просьбе пользователя повышены аппаратные уровни работающей карты Test: HPHL/HPHR Volume=20 (0 dB, ранее -12 dB), RX_RX0/RX_RX1 Digital Volume=84 (0 dB, ранее -6 dB). Все четыре cset успешны и вернули заданные значения. Громкость mpv ранее установлена 50%; в этой операции не менялась. Это runtime-настройка; play-movie.sh пока задаёт прежние начальные уровни.


# Q6ASM: потеря звука после повторного prepare, 2026-09-09

Пользователь подтвердил качество фильма: «А вообще было хорошо и громко».
mpv воспроизводил AV до 8:12, затем prepare failed; рядом переход к 8:07.
Нельзя приписывать ошибку изменению громкости. Инициатор STOP (seek/XRUN)
не определён однозначно, лог показывает последующие перемотки назад.

В uptime 4085 с: Buffer already allocated, OPEN_WRITE 0x10db3 → DSP 0x9
(ADSP_EALREADY), unmap → error 0xa, audio_client освобождён; последующие
map → error 1 / -22. mpv отключил AO и продолжил только видео.
PA включён в 3590 с на 900 с: в момент ошибки его таймер не истёк.

Кандидат в q6asm_dai_prepare: закрывать старую DSP-сессию и снимать mapping
при state != IDLE, а не только RUNNING; после очистки устанавливать IDLE.
trigger(STOP) переводит в STOPPED и посылает EOS, но не CLOSE, поэтому
прежняя проверка пропускает необходимый cleanup. Патч минимальный; все error
paths, асинхронный EOS и capture/compress ещё требуют отдельного анализа.
Собран W=1 без предупреждений, отдельный tcl_q6asm_dai_fixed.ko. Штатное дерево
не изменялось. База — текущий локальный 6.18.34 с прежними диагностическими
правками, не объявлять полный исходник чистым upstream.

Аппаратный тест в прежнем boot НЕ достиг повторного prepare: уже первый
snd_pcm_set_params получил -12 из-за DSP Memory_map_regions failed.
Это согласуется с оставшимся после аварии состоянием DSP, но не доказывает
конкретную утечку handle. Патч НЕ валидирован и не доказан неработающим.
Тест reprepare.py делает 3 цикла write/drop/prepare на одном PCM handle.
Тестовый q6asm module выгружен, штатный q6asm_dai восстановлен, ALSA-карты
нет, PA low, stock SoundWire, Weston active, taint 4096.
Следующая проверка требует чистого состояния DSP; перезагрузка ещё НЕ разрешена.

Bugzilla: отчёт о звуке #19084 pending confirmation
b9d3b0b4-176c-4199-b6d4-d45e917ffe91 (work_time 45).
Создание отдельной задачи Q6ASM pending confirmation
ac0fa84e-f16e-423b-a83a-5b8f774f5ed5. Не дублировать вызов bug_create.
После создания проставить blocks=[19084], добавить время и результат проверки.

Фильм остановлен автоматически лимитом tcl-movie-audio-v2 (850 с); cleanup
успешен. Лимит сработал ПОСЛЕ потери звука и не является её причиной.


## 2026-09-09 — проверка на чистой загрузке

Пользователь разрешил перезагрузку. Новый boot_id
`faf038a3-aea7-46c4-92d4-3266a7ebbef4`, ядро 6.18.34-tcl-audio2, taint 4096.
SSH и Weston восстановились. После восстановления overlays исправленный Q6ASM
прошёл три цикла write/drop/prepare на одном PCM handle: все возвраты 0,
writei по 4800 frames, затем close 0. Прежних ADSP_EALREADY/map failures
в интервале нет. Это аппаратное подтверждение данного перехода, а не всех
error paths/capture/compress. Штатный модуль затем восстановлен cleanup.

По просьбе пользователя запущен фильм с тем же исправлением, mpv 50%, HPH/RX
0 dB. Первая попытка прекращена runner после 10 с ожидания PCM, mpv ещё не
открыл поток. Во второй попытке окно 60 с и verbose log; результат старта
пока проверяется. Unit tcl-movie-q6asm-fixed-v2. Не называть первый запуск
успешным воспроизведением. Логи сохранены на целевой системе.

### Ошибки загрузки по просьбе пользователя

systemctl --failed: 0 units. Полный dmesg и journal warning сохранены до
runtime аудиотестов в boot-faf038a3-*.txt.

- FAT sdg1: volume not properly unmounted. Это требует offline fsck FAT;
  сейчас USB содержит рабочую корневую систему, fsck с исправлением не запускать.
- dummy regulators qfprom/HID/DSI/GPU: незавершённые supply DT, не автоматически
  доказательство отсутствия питания. Native Weston работает.
- sys_imageblit/sys_fillrect: framebuffer not in virtual address space —
  отдельная проблема fbdev-консоли, не ошибка Wayland. Нужен отдельный разбор.
- q6afe Unknown cmd 0x100f6: по исходнику это DEVOTE_REQUEST response, которого
  нет в switch APR_BASIC_RSP_RESULT. Без сообщения status error нельзя трактовать
  как отказ прошивки; это прежде всего необработанный ответ в callback.
- VA qcom,dmic-sample-rate missing: драйвер использует default DIV_2; запись
  микрофона ещё не проверена.
- ath10k invalid MAC → random, invalid channel frequency 0: отдельные недостатки
  board/config/scan info; Wi-Fi и SSH работают. Не исправлены в этом ходе.
- systemd world-inaccessible unit files: предупреждение о permissions, не отказ;
  содержимое могло включать настройки доступа, chmod вслепую не делался.
- ptrace_scope отсутствует: проверить CONFIG_SECURITY_YAMA при следующей сборке.
- unused clocks/domains/regulators удержаны диагностическими boot parameters;
  OOT taint ожидаем от модулей; No ITS сам по себе не означает сбой загрузки.


## Подтверждение фильма, перемотки и RAM, 2026-09-09

Вторая попытка запуска фильма на исправленном Q6ASM успешна. Выполнена
команда mpv seek -5 relative+exact на позиции 53.791667 с; через 3 с позиция
51.666667, aid=1, audio-out-params 48000/stereo/s16, PCM RUNNING.
Прежний отказ prepare/ADSP_EALREADY не появился. Полного покрытия ошибок нет.

Пользователь сообщил остановки, затем дребезг. Обнаружены частая буферизация
и I/O pressure full avg10 ~35.55%, свободно 6765 MiB RAM. Mpv volume оказался
100% (кто/как изменил, не установлено; мы запускали 50%). Звук приглушён,
фильм приостановлен. Копирование 725106140 байт с USB в /dev/shm/tcl-bbb.mov
заняло 22.865 с (~31.7 MB/s); SHA256 источника и RAM-копии совпали:
`dc2146a2b1172def56730143ad80cd1825b7fad15f1fc9c23a4e7d01a741ac11`.

Через IPC заменён только файл, продолжение с 399.625 с, volume=50,
mute=false, pause=false; аппаратные HPH/RX остались 0 dB. Audio-out-params
48000/stereo/s16, PCM RUNNING, paused-for-cache=false. К 7:15 A-V 0.000 с,
I/O full avg10 снизился до 0.54%. Пользователь подтвердил:
«звук появился и не тормозит».

Это подтверждает рабочее воспроизведение из RAM и влияние I/O, но не
однозначно устанавливает причину дребезга: одновременно выполнены повторное
открытие файла/PCM и снижение volume 100→50. На USB повторялись сообщения
Q6ASM `command[0x10dab] not expecting rsp` (WRITE_V2); при смене файла ещё
0x10bdb. Их статус payload не логируется, дальнейшая диагностика отдельно.
Не объявлять полностью исправленным весь Q6ASM по одному успешному сценарию.

Текущий фильм оставлен работать в unit tcl-movie-q6asm-fixed-v2. Его скрипт
после завершения снимет экспериментальный аудиотракт. RAM-копия не постоянна.
Boot DT/штатные модули не заменены; звук после перезагрузки требует восстановления
runtime overlays и загрузки исправленных модулей. Постоянную интеграцию ещё делать.

Отчёт #19084 и создание подзадачи Q6ASM всё ещё pending в Lavtomate
(confirmation b9d3b0b4-176c-4199-b6d4-d45e917ffe91 и
ac0fa84e-f16e-423b-a83a-5b8f774f5ed5). Публикация не подтверждена.


2026-09-09: пользователь попросил повторить фильм с начала. Прежний unit уже
завершился и снял ALSA-карту. Запущен tcl-movie-ram-replay.service через
play-movie-ram.sh, источник /dev/shm/tcl-bbb.mov с начала, исправленный Q6ASM,
mpv 50%, HPH/RX 0 dB. PCM RUNNING, PID3900, uptime1238.96; видео прошло9с,
AV около0.000. RuntimeMaxSec850, PA900, codecwatchdog1000; после окончания
скрипт вернёт временные модули/питание в исходное состояние.

Пользователь предложил запись микрофоном для контроля дребезга. USB camera
13d3:784b имеет только интерфейсы класса14 Video Control/Streaming, Audio
interface не обнаружен. Временная карта показывает pcmC0D0c из-за двунаправленного
Q6ASM FE, однако в machine card только FE и RX playback BE, нет TX capture BE.
Поэтому рабочая запись не подтверждена и не включалась. Следующий шаг —
добавить TX Q6AFE/SoundWire/WCD/macro link и установить разводку микрофонов
(аналоговые WCD или DMIC VA/TX), проверить полученные ненулевые samples.
Не считать формальное устройство capture уже работающим микрофоном.
Не разбирать текущую карту во время повторного просмотра фильма.


## Скорость флешки и кэш mpv: уточнение по замечанию пользователя

Пользователь справедливо указал, что 31.7 MB/s при копировании в RAM могло
включать page cache Linux, и большой запас средней скорости не объясняет
застревания. Отдельный read-only тест O_DIRECT на файле фильма (mmap aligned
buffer, os.readv, page cache обходится): 64 MiB за 2.868 с =23.399 MB/s.
32 случайных чтения по128 KiB:21.414 MB/s, медиана6.071 мс, максимум6.617 мс.
Никакой записи на USB, drop_caches или raw-device access не делалось.
USB Kingston DT101 G2 работает через usb-storage/hub на480M (USB2),
это скорость шины, не полезного чтения. Средний поток фильма725106140/596
=1.217 MB/s (~9.73 Mbit/s), пики не измерены.

Фактические IPC-настройки mpv: cache=auto, demuxer-readahead-secs=1.0,
demuxer-max-bytes=157286400 (150 MiB), cache-pause-wait=1.0.
На позиции385.54с cache-duration=1.0, fw-bytes568112 (~0.54MiB), idle=true,
underrun=false. Cache-secs=3600000 не означает реально заполненный большой
кэш в этом режиме. Поэтому «слишком медленная флешка» НЕ доказано;
задержки I/O наблюдались, но малый readahead — отдельная сильная гипотеза.
Следующий контроль: файл на USB с cache=yes и readahead30–60s, сравнить
буферизацию и I/O при неизменных громкости/ядре/декодере. Текущий просмотр
из RAM не прерывался ради этого сравнения. Тест ещё не выполнен.

Top второй двухсекундный срез: CPU87.4% idle, iowait0%; mpv83.7% одного
ядра (~0.84 core из8), Weston3%, доступно6068MiB RAM. Дефицита CPU/RAM
в этот момент нет. CPU decoding software, вывод GPU. Снимки сохранены.


## 2026-09-09 — контроль mpv cache60 с реальным чтением USB

Запущен tcl-movie-usb-cache60.service, с начала того же MOV на FAT USB,
Q6ASM fixed, software decode/GPU output, mpv50%, HPH/RX0dB. Добавлены:
`--cache=yes --cache-secs=60 --demuxer-readahead-secs=60
--demuxer-max-bytes=256MiB --cache-pause-wait=3`.
Кэш действительно заполняется на60с (а не прежнюю1с); перед контрольным
сбросом fw-bytes ~57MB, включая backward total~158MB.

mincore показал177028/177028 страниц файла resident (100%). Выполнен только
posix_fadvise(DONTNEED) для файла фильма: после0/177028 (0%). Глобальный
page cache не сбрасывался, mpv demux buffer не трогали, запись на USB не
производилась. Это подтверждает замечание пользователя о влиянии кэша Linux.

В следующие30.032с (7 срезов с шагом5с):
- time-pos вырос на30.041667с;
- cache-duration59.958–60.042с;
- paused-for-cache=false и underrun=false во всех срезах;
- dropped frames1→1 (один уже был до интервала, новых не добавилось);
- счётчик чтения USB вырос на28004352байт (~26.7MiB); этот счётчик общий
  для флешки, включая root image, не строго process-local/movie-only.
Следовательно, запас mpv пополнялся при реальных чтениях USB; остановок
в измеренном интервале нет. Это не полный A/B на идентичном холодном
состоянии для обоих режимов и не доказательство отсутствия редких зависаний.
Полный новый просмотр/оценка звука пользователем ещё не завершены.

Настройкиcache60 выбраны для следующего USB-запуска в play-movie-fixed.sh;
прежний вариант сохранён как play-movie-fixed-before-cache60.sh.
RAM-сценарий остаётся отдельным. Это изменение диагностического runner,
не системной конфигурации mpv и не автозапуска после перезагрузки.
Текущий фильм продолжает играть с USB, не останавливать для других аудиотестов.

Артефакты: movie-usb-cache60-{monitor,cold-monitor}.jsonl,
movie-usb-cache60-pagecache.txt, scripts в том же каталоге.

### mpv: включение декодера и статистики (2026-09-09)

На TCL проверен mpv 0.41.0 / FFmpeg 8.0.1; список декодеров содержит h264_v4l2m2m. Для H.264 через Venus используется явный выбор `--vd=lavc:h264_v4l2m2m`; одного `--hwdec=auto` недостаточно для гарантии этого пути. При последней проверке зарегистрированы только /dev/video0 и /dev/video1 (камера), узел Venus отсутствует: перед аппаратным воспроизведением требуется восстановить Venus. Текущий тест фильма использует программное декодирование (`--hwdec=no`) и GPU-вывод OpenGL.

Статистика mpv: `i` — кратковременно, `Shift+i` — включить/выключить постоянный показ; при открытой статистике `1` — общая, `2` — время отрисовки, `3` — кэш. Источник: https://mpv.io/manual/stable/#stats . В рамках ответа воспроизведение и драйверы не переключались.

### Щелчок при закрытии mpv (2026-09-09)

Пользователь слышит щелчок при закрытии плеера. В play-movie-usb-cache60.sh подтверждён порядок: завершение mpv/закрытие PCM, затем cleanup выгружает tcl_speaker_pa; при TERM cleanup также сначала завершает mpv. Усилитель остаётся включённым на переходе аудиотракта — вероятная, но акустически не доказанная причина. Требуется управление PA в согласованной последовательности PCM/DAPM (выключение до остановки тракта, включение после стабилизации), включая EOF и ручное закрытие. Изменение одного cleanup не решит EOF: к этому времени PCM уже закрыт. Пока исправление не выполнено; повторное громкое испытание не запускалось.

2026-09-09: GNOME Web 49.2 / WebKitGTK 2.52.6 установлен через epm, свободно552MiB. Первый запуск tcl-browser.service (непривилегированный пользователь, Wayland, Wikipedia) показал окно, но пользователь развернул его и приложение завершилось: Error71 Protocol error dispatching to Wayland display в18:26:54. Weston осталсяactive, новых сообщенийядра нет. По просьбе пользователя повторно запущен tcl-browser-debug.service с WAYLAND_DEBUG=client; лог /var/tmp/tcl-browser-wayland.log. Точная причина ошибки пока не установлена. В журнале также отсутствует Secret Service (сохранение паролей не настроено).

### SIM и GPS/GNSS (2026-09-09)

Официальные характеристики TCL BOOK 14 GO указывают лоток 2-in-1 Nano SIM + TF/microSD и диапазоны 3G/4G B220G: https://www.tcl.com/eu/en/laptops/tcl-book-14-go/specifications . Для нашего экземпляра сохранённый inventory.json Windows показывает Snapdragon X15 LTE Modem Mobile Broadband Device (service mbb, Status OK, error0) и Qualcomm Location (ACPI QCOM0872, WUDFRd, Status OK, error0). Сохранён OEM qcgnss7180.inf + qcgnss.dll; modemr.jsn содержит gps/gps_service. Это подтверждает предусмотренную поддержку GNSS, но получение координат/спутников не проверялось. LTE подключение и SIM в Linux также не проверялись.

### Firefox установка (2026-09-09, в работе)

По запросу пользователя подключён официальный Mozilla APT arm64: /etc/apt/sources.list.d/mozilla.sources, ключ /etc/apt/keyrings/packages.mozilla.org.asc. Fingerprint35BAA0B33E9EB396F59CA838C0BA5CE6DC6315A3 проверен локально по https://support.mozilla.org/en-US/kb/install-firefox-linux . Pin1000 ограничен firefox и firefox-l10n-*. epm update выполнен. Кандидат155.0.1~build1, firefox + firefox-l10n-ru:78MBdownload/297MBinstalled. ВНИМАНИЕ: `apt-get --assume-no --no-install-recommends install ...`, предназначенный для оценки размера, фактически начал установку; --assume-no не является dry-run. Для последующих расчётов использовать только `apt-get -s`. Задача установки авторизована пользователем, но этот этап фактически прошёл напрямую apt, а не через epm.

GNSS: пока QMI LOC/PDS в qrtr-lookup отсутствуют; загружена qcmpss7180_nm.mbn, MPSSrunning. Возможна имитация NMEA через gpsd для проверки приложений; она не проверяет физический приёмник/антенну и не заменяет диагностику QMI LOC. Симуляция не запускалась.

Firefox155.0.1~build1 arm64 и firefox-l10n-ru установлены. tcl-firefox.service запущен от tcl-browser через dbus-run-session, MOZ_ENABLE_WAYLAND=1, WAYLAND_DISPLAY=/run/tcl-weston/wayland-tcl, URLhttps://www.wikipedia.org. Проверка: unitactive, firefox-binPID10729, ошибок в начальном журнале нет; визуальная проверка разворачивания ещё не выполнена. `epm clean` удалил240MiBкэша пакетов, свободно492MiB. GNOME Web сохранён; автозапуск Firefox не настроен.

### GNSS: найдено различие прошивок (2026-09-09)

# GNSS: выбор прошивки MPSS

2026-09-09. Исследование без перезагрузки и без остановки MPSS/Wi-Fi.

| Проверка | Результат |
|---|---|
| Активная прошивка | qcmpss7180_nm.mbn, SHA256 совпадает с OEM |
| Активный вариант | rennell.gennm.prodQ, 5057288 байт |
| Windows Hardware ID | ACPI VEN_QCOM DEV_081E SUBSYS_CLS07180 |
| Секция OEM INF для этого ID | SUBSYS_Device_AMSS_MSM_Ext → SUBSYSReg_AMSS_MSM |
| Файл в секции MSM | qcmpss7180.mbn, 61815992 байт |
| Полный вариант | rennell.gen.prodQ |
| Память NM | [0x86000000,0x88000000), 32 MiB |
| Память MSM | [0x86000000,0x8e400000), 132 MiB |
| QMI LOC/PDS в QRTR | Не объявлены |
| Строки QMI_LOC/gps/gps_service | Есть в полной прошивке; отсутствуют в NM при таком поиске |

`firmware-comparison.json` содержит SHA256, идентификаторы и все PT_LOAD сегменты. INF декодирован из UTF-16; полная версия сохранена рядом. Поиск строк сам по себе не доказывает отсутствие исполняемого кода, но вместе с INF и отсутствием служб указывает на неверный для GNSS вариант прошивки.

## Проверка памяти

Текущий DT резервирует MPSS32MiB. Полные PT_LOAD требуют132MiB и помещаются в OEMрезерв, целиком ниже первой SystemRAM0x94600000. Venus[0x85b00000,0x86000000), ADSP[0x90b00000,0x93300000), WLAN[0x93900000,0x93b00000) не пересекаются с полным MPSS.

Есть пересечение с унаследованным `reserved-memory/memory@8b700000` размер64KiB — ipa_fw_mem из общего sc7180.dtsi. В текущем DT нет phandle у этого узла; IPA@1e40000 disabled и не имеет memory-region. Для эксперимента GNSS оставить IPA disabled и убрать неиспользуемый отдельный резерв из будущего DT, так как он входит в полный резерв MPSS. Для последующего LTE назначение памяти IPA надо изучить отдельно; автоматически включать IPA нельзя.


Подробный план и ограничения: peripherals/gnss/README.md. Активная прошивка, DT и GRUB не изменены.

### Дополнительные датчики (2026-09-09)

В inventory.json Windows найдены Aw9610x Sar Sensor (ACPI AWDZ9610X, WUDFRd, Status OK) и ACPI Lid (PNP0C0D). Акселерометр/гироскоп/компас/освещённость по именам устройств не обнаружены; это не доказывает физическое отсутствие. DSDT SAR1: I2C5 address0x12,400kHz, GPIO0x22=34, interruptEdgeBoth/PullUp. DSM содержит строкуAW96105 и OEMконфигурацию0x37cбайт. Сохранены aw9610x.inf/AW9610X.dll, qsarconfig7180/qsarmgr. СемействоAW9610x — ёмкостные датчики приближения дляSAR, не IMU/ориентация; источникпроизводителя https://www.awinic.com/en/evbInfo/AW93105DNR/340 . В локальномLinux6.18.34 есть drivers/iio/proximity/aw96103.c; совместимостьAW96105 надо проверить поmatchtable. В текущемLinux IIOустройств нет. Драйвер/DTдатчика в этой сессии не включались.

2026-09-09: созданы отдельные баги GPS https://bugs.etersoft.ru/19500 и датчиков https://bugs.etersoft.ru/19501. Связи blocks=[19084] для обеих подтверждены Lavtomate. Комментарииучёта времени: GPS25мин confirmation8ccc9ce7-e84c-44fd-87da-af75f03f0874 (последний статусpending); датчики10мин confirmation7eb387b1-3cf9-4579-a5ea-407b58829abd pending. Не создавать повторно.

Для звука Firefox подготовлены browser/enable-browser-audio.sh и browser/browser-pulse.pa: проверенный audio2тракт, PulseAudioкакtcl-browser, статический48kHzstereoS16LEsink,50%,безidle-suspend. В cleanupPAвыключается до остановкиPulse/PCM. Включение ещё не проверено; установкаlibqmi-utils держитdpkg, PulseAudioожидает еёзавершения. Ограничение теста: PA900сек, codecwatchdog1000сек, plannedRuntimeMaxSec850.

2026-09-09: по повторному запросу пользователя проверена существующая ACPIбага19492 (дубльне создавался). Вней уже есть подробные комментарии181236/181239/181247; отправлено актуальное дополнение о звуке, GPS19500/датчиках19501. Текст acpi-addendum-20260909.md; confirmation46007d47-9d70-4aac-ad7d-c61a05f36e1d pending,10мин.

### Firefox: звук подтверждён (2026-09-09)

Пользователь: «так уже работает». PulseAudio17 запущен как tcl-browser из /run/tcl-pulse-stage (распаковка уже скачанных подписанных пакетов; обычная установка через epm продолжается медленно на USB). tcl-browser-audio.service active, RuntimeMaxSec850, KillMode=mixed; источник enable-browser-audio-ram.sh, конфигурация browser-pulse.pa. ALSA hw:Test,0: S16LE stereo48kHz, PA GPIO46/47 включён послеRUNNING, sinkvolume50%, HPH/RX0dB. pactl показывает application.name=Firefox, Corked:no, Mute:no, tcl_speakers RUNNING. Firefox подхватил появившийся аудиосервер без перезапуска. Старые OpenCubeb failures относятся к времени до запускаPulse. Taint4096 не изменился.

При управляемом cleanup PA выключается раньше Pulse/PCM; отсутствие щелчка при остановке ещё не подтверждено. Предел PA900сек/codecwatchdog1000сек, общий850сек; это временный тест, не постоянный автозапуск звука. После таймера аудиотракт будет восстановлен в исходное состояние. Сценарий запуска из установленных пакетов тоже подготовлен enable-browser-audio.sh, но ещё не проверен. Не выдавать RAMвариант за завершённую постоянную интеграцию.

QMI1.38.0 установленчерезepm; read-only `qmicli -d qrtr://0 --loc-get-operation-mode` завершился `couldn't create client for the loc service: QMI protocol error (3): Internal`. Это не ответ LOCобошибкеGNSSдвижка — клиент службы не создан.

2026-09-09: проверка нехватки места при медленной установкеPulseAudio. root loop0Btrfs3GiB: свободно285.59MiB; Data2.48GiB/used2.20GiB; MetadataDUP256MiBлогических/used48.36MiB(18.89%); globalreserve5.50MiB/used0; unallocated1MiB. USBFAT7.5GiB:свободно3.2GiB. ENOSPC/Btrfs/I/Oerrorsвdmesgне найдены. PSI IOfullavg60=36.50%,some47.66%; dpkgранееwait_log_commit/write_all_supers. Нехваткаметаданныхне подтверждена,местоограничено,ноустановкапродвигается(Settingup). Не расширяли/не балансировали ФС.

2026-09-09: пользователь сообщил об исчезновениизвука. Подтверждён штатный RuntimeMaxSec850: unitresulttimeout, PAвыключен, cleanupRESTORED, taint4096; не новый аудиосбой. Запущен tcl-browser-audio-installed через установленный PulseAudio17 и enable-browser-audio.sh; PCM RUNNING, unitactive. Ограничение850сек пока сохранено. Работа без таймеров требует отдельной доводки жизненного цикла; не считать постоянным звуком.

2026-09-09 статус: ACPIдополнение опубликовано #19492 comment181334 (confirmation46007d47 approved). Firefoxactive, tcl-browser-audio-installedactive, Pulse tcl_speakersIDLE/безsink-inputs (браузер сейчас не воспроизводит звук). apt13978/dpkg15852 ещё завершают настройкуPulseAudio, rootсвободно281MiB, taint4096.

2026-09-09: установкаPulseAudio/pulseaudio-utils завершилась, оба dpkgstatus install ok installed; блокировкиapt/dpkg освободились. По отложенному поручениюпользователя запущен epm -y remove epiphany-browser epiphany-browser-data; предварительнаясимуляция показала удаление только этихдвухпакетов. epm выбрал dpkg --purge, лог /var/tmp/tcl-epiphany-remove.log. Покаудалениевпроцессе; общиебиблиотеки/автозависимостине удалялись. Firefoxactive.

2026-09-09: epm -y remove epiphany-browser epiphany-browser-data завершился с кодом0, включая триггеры GLib/hicolor. Оба пакета not-installed. Общие зависимости и профиль пользователя не удалялись; Firefox сохранён.

### Уточнение назначения SAR-датчика и требований к драйверам

Пожелание пользователя: собственные аппаратные драйверы допустимы, но должны предоставлять стандартные интерфейсы подсистем Linux.

AW96105 — ёмкостный датчик приближения для SAR: близость тела используется системой управления радиопередатчиком для выбора мощности. В Windows присутствует Aw9610x Sar Sensor (Status OK), в ACPI SAR1 указаны I2C5/0x12, GPIO34 и строка AW96105. Физическое расположение микросхемы и чувствительных электродов на TCL пока неизвестно; расположение около антенн/под корпусом — предположение, не результат осмотра. Наличие записи Windows не заменяет проверку живых измерений. Не считать датчиком присутствия перед экраном или датчиком крышки. Предпочтительный интерфейс Linux — существующая подсистема IIO и драйвер aw96103 с поддержкой AW96105; связь событий с ограничением мощности требует отдельной проверки.

Источники: https://doc.awinic.com/doc/202305/78ba5923-be3a-4a1f-8bbc-7b140959f19f.pdf ; описание автора драйвера https://lkml.indiana.edu/2409.0/04835.html .

2026-09-09: AW96105 OEM-конфигурация извлечена из SAR1._DSM, 892 байта/138 записей, checksum и границы проверены. ACPI I2C5=MMIO890000=Linux i2c-4 (подтверждено SSH). Отчёт и бинарник: .claude/docs/tcl-b220g-data/peripherals/sensors/README.md. Датчик не активирован; питание vcc требует выяснения. Звук tcl-audio active, RuntimeMaxUSec=infinity, PA/watchdog hold_seconds=0.

2026-09-09: AW96105 реально запущен через штатный IIO-драйвер aw96103 (внешняя сборка без изменений), OEM firmware из ACPI. iio:device0 aw96105_sensor, меняющиеся raw0..4, raw5 отключён. GPIO34/IRQ168 EdgeBoth/PullUp через групповой pinctrl. vcc пока dummy, фактическая линия неизвестна; диагностический клиент без выгрузки, автозапуска нет. Полный отчёт/исходники/модули: peripherals/sensors/README.md. Звук и Weston active, taint4096, перезагрузки не было.

### ACPI1 готов к аппаратному тесту (2026-09-09)

Собрано ядро6.18.34-tcl-acpi1 из отдельной копииaudio2, включён ACPI/I2C_HID_ACPI/I2C OpRegion, UFS отключён. Автономный initramfs с BusyBox+реальным blkid; chrootпроверкаblkid и sh-n пройдены. Комплект /var/ftp/tmp/lav/tcl/acpi1 и .claude/docs/tcl-b220g-data/acpi-boot/acpi1; таблица сравнения comparison.md. На USB /tcl-acpi1 установлены Image/initramfs/grub/маркер, все SHA256 совпали. EFI заменён после проверки+backup: новый37e9f7546e6a8ee96fcd34cebe2635116a0346a63adc1a3421e110cd2abdedd0, BOOTAA64-before-acpi1.bak9bf487c31f44164c022a74c4c0d1228233929cef77e4c196024341898ae08147. Default осталсяtcl-audio2-va, новыйIDtcl-acpi1. Перезагрузки НЕ было. Для ACPI используетсяforce+nospcrбезdevicetree; USBсохранениеусловное, безsavedавторебута нет.

Бага19501: результатIIO опубликованcomment181342 (confirmationda628c98approved). ACPI19492 новыйотчёт40мин отправлен, confirmation2dea6df1-f14a-4e31-aec0-cd8c3531d3d3 pending; не дублировать.

Виртуализация: текущаяконфигурацияKVM=y, namespaces/cgroups=y, /dev/kvmнет; dmesgCPU All CPU(s) started at EL1 и HYP mode not available. Архитектурные ядра Cortex-A76/A55 поддерживаютEL2 (Arm https://documentation-service.arm.com/static/63739bde5854836459183244), но доступLinuxне предоставлен цепочкойзагрузки. ID_AA64PFR0вsysfsнепрочитан, невыдаватьза аппаратноеподтверждениерегистром.

2026-09-09: пользователь разрешил reboot для ACPI1. Установлен проверенный EFI d45aca3ab5678fbc142aaa600e9435ea6300e3f67baac2b9778864bcaef8ec6f с loadenv и одноразовым next_entry=tcl-acpi1 (внешний /tcl-acpi1/grubenv, очищение перед запуском, defaultDT при ошибкеsave_env). systemctl reboot завершилсякодом0; исходныйbootfaf038a3-aea7-46c4-92d4-3266a7ebbef4. Ожидается аппаратный результат. Новаяперезагрузка без необходимости не разрешалась.

ACPI1: отчёт подготовки опубликован в19492 comment181346 (confirmation2dea6df1 approved). После разрешённой перезагрузки SSH пока не вернулся; это не доказательство зависания, вRAMтестеSSHнет. Результатзагрузки не подтверждён, требуется экран либо возвратDTиUSBлоги.

ACPI1 первый тест: фото23-48-57 и расшифровка сохранены acpi-boot/acpi1/photos. Kernel6.18.34-tcl-acpi1, CPU0-7, EFI VGA, BusyBox shell; /sys/firmware/acpi/tables и /sys/bus/acpi/devices отсутствуют, USBsaved=no. Клавиатура не работает по словам пользователя. Нельзя утверждать, что ранняя CPUинициализация не использовалаACPI: возможен позднийdisable_acpi. force+nospcr вкоде независимы. Подготовлен /tmp/tcl-acpi1-screenlog-initramfs.cpio.gz с автоматическимпоказомраннихлогов, НЕустановлен. init-first-test сохраняет исходныйсценарий, init теперь кандидат следующейтестовойверсии. Нужен физический возвратвDTдлязаписи; автоперезагрузканепроизошлаиз-заsaved=no.

ВозвратDTподтверждён: bootd93992f0-801e-4d77-b1e0-75847ca70ed4, kernel6.18.34-tcl-audio2, next_entryпустой. Westonactive, tcl-audiofailedвprepare; журналсохранён отдельно, не исправлено. ПодготовленаиустанавливаетсяACPI1screenlogv2, журнал4MiB, убраныinitcall_debug/ignore_loglevel/keep_bootcon, loglevel6. Этообновлениедиагностики, неподдержкаUSBACPI. Дополнительнаяперезагрузканевыполнялась.

2026-09-10: по согласию пользователя выполнена повторная перезагрузка ACPI1 screenlogv2. До reboot kernel6.18.34-tcl-audio2/bootd93992f0-801e-4d77-b1e0-75847ca70ed4; всеSHA256комплектаOK, установленныйEFIa69436c2e98a928d3919be906cf8697e95fd436c8cd8913ccc7419140883c685. Одноразовыйgrubenvnext_entry=tcl-acpi1установленпослеhashcheck, sync/systemctlreboot. Ожидаетсяэкранраннегожурнала; успешныйACPIещёнеподтверждён.

2026-09-10 screenlog-v2: получены8фотоиз~/Загрузки/TelegramDesktopналавбуке. ACPIраннийCPU/GIC/timerдоказан. Причинапозднегоотказа AE_BAD_PARAMETER During Region initialization(tbxfload52):CONFIG_PCI=n,безусловныйPCI_CONFIGвdefaultspacesпривыключенномcaseобработчика. Подготовлен6строчныйпатчусловногомассива/размера3или4, аналог2021ссылкивacpi-boot/pci-disabled/README.md. ОбъектысобраныW1,nmразмер3; полнаясборкаImageв/tmp/tcl-acpi1-linuxидётsession47293,лог/tmp/tcl-acpi-no-pci-image.log. ИсходныйDT-treeнеизменён,ядронаплатенеобновлено. Созданиеновойбагиconfirmation23ab1e63pending,отчёт19492confirmation01c84004pending. Фотографииитекстысохраненыacpi1/photos/screenlog-v2.

ПолнаясборкаImageзавершиласьуспешно, SHA256f889a28591a98583f58a8637593d2f065074ef74fe67e0d3de18aa5c1b3a2e1c. Предупреждениятолькопрежниеunusedvariablesвacer-aspire1-ec.c. Исправленноеполноеядроподготовлено,нонаTCLещёнеустановлено/непроверено. Отдельнаябага создана:19506.

2026-09-10: исправленный Image и комплект скопированы в /var/ftp/tmp/lav/tcl/acpi-pci-disabled/, SHA256 проверен: f889a28591a98583f58a8637593d2f065074ef74fe67e0d3de18aa5c1b3a2e1c. Бага 19506 создана; связь blocks=[19492,19084] ожидает подтверждения Lavtomate 6c272602-c213-4fc4-9113-39c9b9087e8d. Комментарий о полной сборке (10 минут) ожидает подтверждения 7d9e3d9c-8943-4811-8a58-71cbc608e9d0. Не повторять операции. Для установки требуется возврат TCL в рабочий DT-пункт TCL AUDIO2 VA; аппаратная проверка нового ядра ещё не выполнена.

### 2026-09-10: исправленное ядро ACPI установлено

Возврат по SSH подтверждён: kernel `6.18.34-tcl-audio2`, boot ID `9365beef-bc6b-41ac-88c0-02cceed5edbd`. Исправленный Image установлен в `/run/initramfs/usb/tcl-acpi1/Image`; SHA256 `f889a28591a98583f58a8637593d2f065074ef74fe67e0d3de18aa5c1b3a2e1c`. Предыдущий Image сохранён как `Image-before-pci-fix`, прежний манифест — `SHA256SUMS-before-pci-fix`. Новый SHA256SUMS на USB сформирован, все пять файлов успешно проверены после sync. EFI не менялся (a69436c2e98a928d3919be906cf8697e95fd436c8cd8913ccc7419140883c685), рабочий DT-пункт не менялся. Перезагрузка не выполнена, требуется согласование следующего аппаратного теста.

2026-09-10: пользователь разрешил перезагрузку для проверки исправления ACPICA (19506). Проверены Image f889a28591a98583f58a8637593d2f065074ef74fe67e0d3de18aa5c1b3a2e1c, весь SHA256SUMS, установленный EFI a69436c2e98a928d3919be906cf8697e95fd436c8cd8913ccc7419140883c685 и одноразовый grubenv 6f483ea97ad452c2f657e0a6c33c070a47541bee3c1014a762c52aab7cd0efc3. grubenv.next перемещён в grubenv, sync выполнен, systemctl reboot завершился кодом 0. Исходный boot ID 9365beef-bc6b-41ac-88c0-02cceed5edbd. Ожидается аппаратный результат на экране; успешная загрузка исправленного ядра ещё не подтверждена.

### 2026-09-10: чёрный экран после исправления ACPICA

Пользователь сообщил о чёрном экране после разрешённой загрузки Image f889a28591a98583f58a8637593d2f065074ef74fe67e0d3de18aa5c1b3a2e1c. SSH 192.168.8.177 timeout; RAM-образ не содержит сетевого доступа, поэтому это не подтверждает зависание. Прохождение прежнего AE_BAD_PARAMETER на аппаратуре пока не доказано. По коду bus.c после acpi_load_tables следуют acpi_enable_subsystem и acpi_initialize_objects, затем дальнейшая инициализация; новое выполнение AML — гипотеза, а не установленная причина. CONFIG_ACPI_VIDEO=n, DRM_MSM=m, модулей MSM в RAM-тесте не загружаем; автоматический переход на родной DRM не подтверждён. Подготовлен только кандидат entry-trace-candidate.cfg с keep_bootcon/initcall_debug/ignore_loglevel/loglevel=8 для локализации последней операции. На USB не установлен, EFI не изменён; следующая перезагрузка не отправлялась. Требуется возврат в рабочий TCL AUDIO2 VA для чтения возможных USB-логов и установки диагностики.

### 2026-09-10: установлен диагностический GRUB после чёрного экрана

Рабочая Ubuntu доступна, kernel6.18.34-tcl-audio2, boot7a852d3d-b401-4301-8145-e0e3fbbb9620. Каталог tcl-acpi1/logs отсутствует; grubenv next_entry пустой. Подготовлен grub-trace.cfg: только ACPI entry изменён добавлением keep_bootcon/initcall_debug/ignore_loglevel и loglevel8. Остальные пункты побайтно сохранены. grub-script-check успешен, EFI собран согласованными Debian GRUB2.12 tools/modules; проверено точное вхождение config в EFI. Новый EFI SHA25659491a713189522d55e81cbf2c821c201ba52417898a85dd62bec1e5dcaea4c2 установлен с резервной копией EFI/BOOT/BOOTAA64-before-pci-trace.bak (a69436c2e98a928d3919be906cf8697e95fd436c8cd8913ccc7419140883c685). Kernel и initramfs не менялись. Default остаётся tcl-audio2-va, однократный ACPI не взведён. Перезагрузка не выполнена. Следующая проверка требует согласия пользователя; вывод с keep_bootcon может замедлить загрузку.

2026-09-10: после просьбы пользователя продолжить отправлена однократная перезагрузка в ACPI с trace. До reboot все SHA256SUMS, Image f889a285…, EFI59491a71… и grubenv6f483ea9… проверены успешно; next_entry установлен, sync выполнен. systemctl reboot код0, исходный boot7a852d3d-b401-4301-8145-e0e3fbbb9620. Ожидается экранный результат. Предыдущий вызов bug_add_comment19506 и последующий FTP-copy были прерваны (functions.exec aborted); статус публикации неизвестен, автоматически комментарий не повторять.

### 2026-09-10: проверка USB из Windows после trace-теста

Windows SSH etersoft@192.168.8.143 доступен (whoami b220g\etersoft). Get-Volume: D: TCLDIAG FAT32, свободно3230633984байт. Проверено D:\tcl-acpi1: каталог logs отсутствует (ACPI_LOGS_ABSENT). Таким образом, последний trace-тест также не оставил ожидаемых USB-логов; причина чёрного экрана неизвестна. Наличие скрипта сохранения не обеспечило запись. Windows/загрузчик в этой проверке не изменялись, перезагрузки не было. Следующий аппаратный тест нельзя основывать лишь на снимках экрана и неподтверждённом USB; требуется подготовить и проверить способ получения диагностики.

2026-09-10: выполнен статический аудит AML, отчёт acpi-audit/aml-static/README.md. DSDT checksum0,189125байт,324метода,_INIнет. _STA IPA/RVRM вызывают BREV с прямыми чтениями GPIO67/68 через SystemMemory; адреса14/66/67/68 совпадают с Linux. Это кандидат анализа, не установленная причина чёрного экрана. Iasl обратная компиляция8errors/32warnings/311remarks; свежая декомпиляция подтверждает исходные особенности. Локальный acpiexec некорректно строит/читает синтетические таблицы даже для минимального control.asl; его результат нельзя считать валидацией TCL. Требуется отдельное расследование инструмента с воспроизводителем. Отчёт19492(20мин) ожидает подтверждения022e5802-96b3-414f-be0f-da750aba377d, не повторять. Никаких аппаратных тестов/перезагрузок в этом аудите.

2026-09-10: Linux TCL доступен на192.168.8.177, kernel6.18.34-tcl-audio2, boot1589e0d5-7a52-4d0b-8405-d8b498bdbbc2. Windows192.168.8.143 timeout; ошибка агента — проверял только старый Windows-адрес, не проверивLinux. Пользователь уточнил8.177. Локальная ACPICA20260408 собрана изcommit57a2afa78dbe3580e19307aedd19f32388512094; контроль и OEMDSDT загружаются. AE_NOT_FOUND GPU0.AVS0 вTZ7._TZD воспроизводится офлайн. 11_STA сboard-ID завершились всимуляции fill0/1, это не реальныеGPIO. Отчёт дополнен acpi-audit/aml-static/README.md; новаябагаacpiexec confirmation3520d5ca-a6e1-46c7-9014-fe9ac877ffae pending, не повторять.

2026-09-10:
## Все _STA и чтение живых таблиц

Все47методов _STA проверены в симуляции fill0, пятью группами из-за ограничения acpiexec1023байта на batch-команду. 46вернули Integer, BAT0._STA завершился AE_NOT_EXIST: нет GenericSerialBus handler для ECBF/IC3R. Это ожидаемое отсутствие I2C-обработчика в изолированной среде и существенная зависимость аппаратного порта, не доказательство причины чёрного экрана. Полная таблица all-sta-results.md; журналы all-sta-batch0..4.log.

В рабочем Linux boot1589e0d5-7a52-4d0b-8405-d8b498bdbbc2 EFI сообщает ACPI20=0xffffd000, область0xfff22000–0xffffdfff зарезервирована внутри SystemRAM. Ограниченное чтение36байт RSDP через /dev/mem O_RDONLY запрещено EPERM. Ни таблицы, ни MMIO прочитаны не были. read-live-acpi.py останавливается на выходе за проверенный ACPI-диапазон и исключает содержимое MSDM.

Пользователь согласен при случае снять защиту для подробного исследования. TODO: отдельный диагностический вариант рабочего DT-ядра с CONFIG_DEVMEM=y, CONFIG_STRICT_DEVMEM=n; сохранить рабочий пункт, Wi-Fi/SSH и логирование. Нынешнее ядро не изменено, сборка/перезагрузка ради этого не выполнялись. По lib/devmem_is_allowed.c запрет page_is_ram независим от iomem_is_exclusive: одного iomem=relaxed недостаточно. Разрешение относится к исследованию; не к произвольным записям MMIO.

Бага инструмента создана:19510. Связь blocks19492 ожидает подтверждения be2fbce8-f3e7-4bfe-8353-e3e0d23ebe5d.


2026-09-10: архив acpi-aml-static на /var/ftp/tmp/lav/tcl обновлён. Отчёт19492(15мин) confirmationc28c803c-0997-4aef-b2e8-6bbbb0d660ba pending; время/контроль19510(10мин) confirmation2faf0532-012d-424e-b48a-d0b236fa977f pending. Не дублировать.

2026-09-10: уточнение аудита батареи и72методов.
## Порядок зависимостей Linux и ещё72метода (2026-09-10)

Важное уточнение к прямому вызову BAT0._STA в симуляторе: ядро уже учитывает этот случай. drivers/acpi/bus.c:105–109 в acpi_bus_get_status возвращает status0 без вызова AML для battery с dep_unmet. В scan.c acpi_scan_dep_init выполняется перед acpi_scan_init_status; GPIO/I2C dependencies подсчитываются даже без honor_deps. PEP0 сCIDPNP0D80 игнорируется существующим правилом Linux. При отсутствии GIO0/I2C3 батарея должна ждать зависимости, а не обязательно исполнять проблемный _STA. Не переносить AE_NOT_EXIST из прямой симуляции на причинное объяснение загрузки.

В i2c-core-base.c порядок: i2c_acpi_install_space_handler затем i2c_acpi_register_devices; последний вызывает acpi_dev_clear_dependencies. gpiolib-acpi-core.c также снимает зависимости после регистрации регионов. Возврат i2c_acpi_install_space_handler в i2c_register_adapter не проверяется: потенциальный пограничный случай при ошибке установки, но такой аппаратный отказ не подтверждён.

acpi_get_object_info (nsxfname.c) не исполняет _SUB с2015года и _STA с2018года. Поэтому наличие прямых чтений в PEP0._SUB не означает их неизбежное выполнение через стандартное получение идентификаторов Linux. Отдельные _STA и _CRS остаются другими путями.

В симуляции fill0 выполнены все72объявленных метода _CRS/_SUB/_DEP: каждый вернул объект. Список resource-evaluation.json и resources-batch0..8.log. Это проверка выполнения, а не соответствия возвращённых ресурсов железу и не полный перебор условий. Повторяются только ранее зафиксированные GPU0.AVS0 при загрузке namespace и4allocations инструмента на выходе. Статические объекты Name(_DEP/...) в число72методов не входят.

Диагностическое ядро для будущего чтения таблиц: взять рабочий DT audio2, выключить только STRICT_DEVMEM и зависимую IO_STRICT_DEVMEM (если включена), сохранить DEVMEM=y, конфигурацию USB/сети и весь рабочий initramfs. При новом LOCALVERSION обязательно обеспечить соответствующие модули и проверить привязанные к версии сценарии звука/графики; простая смена суффикса сломает существующие проверки. Безопаснее подготовить отдельный Image/пункт с документированным hash, проверив совместимость модулей, прежде чем выбирать вариант. Ничего из этого пока не собрано/не установлено, перезагрузка не выполнялась.


Комментарий19492 об уточнении battery deps и72методах (10мин): confirmation155e81ab-422b-4af0-894b-e73e9f8b971e pending, не повторять.

# TCL MEMDIAG — рабочая DT Ubuntu с чтением ACPI-памяти

2026-09-10. Установлено, ещё не загружалось.

Основа — точная копия рабочего /tmp/tcl-audio2-linux, исходный Image3cfc0fcb6d48d4816423ad794a045bf783f049bc12311f5c66df213f179aa71e. Отдельное дерево /tmp/tcl-memdiag-linux. Изменено CONFIG_STRICT_DEVMEM=n; автоматически исчез CONFIG_EXCLUSIVE_SYSTEM_RAM=y. Остальные различия config.diff — недоступные выключенные опции. Сгенерированный autoconf.h отличается только этими двумя define.

Сборка `make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j16 Image` успешна. Kernel release остался6.18.34-tcl-audio2: установленным модулям и скриптам не нужен новый суффикс. Utsrelease.h и Module.symvers совпадают с рабочим деревом (SHA256symvers6b8a49b0453fc8b07ecfdf7644bc662f6536abc593554fcb65f02aadbaa7d448). Это статические проверки совместимости, не аппаратная проверка нового ядра.

Image SHA256:78c251e585d082d221b17429769b4ad861e9aeb23ef64e5249029610ca059ef5.
EFI SHA256:51200ae5f014a18b2e8ac1cb5ba9b6d559f3d936090ff1850b931c17874eb7f8.

Новый пункт `TCL MEMDIAG: working Ubuntu + Wi-Fi SSH + ACPI memory dump`, idtcl-memdiag. Ядро/tcl-memdiag/Image, DT/tcl-audio2/va-probe.dtb, initramfs/tcl-audio2/initramfs.cpio.gz; добавлен только tcl.memdiag=1. Само ACPI в этом ядре по-прежнему выключено: цель прочитать таблицы из RAM, а не менять способ инициализации оборудования. Ожидается прежняя рабочая сеть, потому что используются прежние DT/rootfs/initramfs.

EFI собран согласованными GRUB2.12 tool/modules. grub-script-check пройден, вхождение полного config вEFI проверено. Все прежние menuentry сохранены побайтно. Defaulttcl-audio2-va; одноразовый next_entry теперь допускаетtcl-acpi1 илиtcl-memdiag, очищается до старта. Для нового теста next_entry пока не установлен. Резервная копия EFI/BOOT/BOOTAA64-before-memdiag.bak содержит предыдущий59491a71….

Установлен /usr/local/lib/tcl-memdiag/{collect.sh,read-live-acpi.py}, tcl-memdiag.service enabled с ConditionKernelCommandLine=tcl.memdiag=1 и проверкой mountpoint. В текущем ядре systemctl start пропускает службу, ConditionResult=no.

Прямой пробный запуск collect.sh на нынешнем ядре успешно сохранил STARTED, dmesg-before/after, cmdline, iomem, EFI pointers, reader-error, RESULTreader_failed=1 иCOMPLETE на USB UUID405E-8AB3. Причина отказа ожидаемая: STRICT_DEVMEM запрещает чтение RSDP. Логи в preflight-logs/. SSH после теста active.

Результаты будущей загрузки: /run/initramfs/usb/tcl-memdiag/logs/<boot-id>/. При успехе tables.json, при отказе reader-error.txt иRESULT. Скрипт сначала сохраняет и sync журнала, затем пытается прочитать только подтверждённый зарезервированный диапазон0xfff22000..0xffffdfff через O_RDONLY. Проверяет сигнатуры/длины/checksum, читает XSDT иDSDTпоFADT. MSDMpayload исключён. При смещении таблиц/изменении карты останавливается, не читает произвольную память. Автоматической перезагрузки нет.

Все файлы на USB проверены SHA256послеsync. Текущийboot1589e0d5-7a52-4d0b-8405-d8b498bdbbc2,IP192.168.8.177. Перезагрузка не выполнялась; требуется согласие пользователя на испытание.


MEMDIAG отчёт19492(20мин) confirmationef323d92-d1de-4322-9a74-67d7cba08705 pending, не повторять.

2026-09-10: по явному согласию пользователя отправлен reboot MEMDIAG. ВсеSHA256комплекта/EFIпроверены; одноразовыйgrubenv652573105534abfd79f67b5fa02eefc77a3c828b1ae7ecf1a6f6de4d3ee0f56e установлен+sync. Исходныйboot1589e0d5-7a52-4d0b-8405-d8b498bdbbc2. SSHразорванудалённымхостомпослесистемногоreboot(exit255); успехследующейзагрузкиожидается, не подтверждён.


## Успешная аппаратная проверка MEMDIAG

2026-09-10: после разрешённой перезагрузки boot4e6555b5-375b-4855-8672-59a720177a4c. В cmdline /tcl-memdiag/Image иtcl.memdiag=1. Wi-Fi192.168.8.177,SSHactive,Westonactive. tcl-memdiag.service Resultsuccess; RESULTsuccess/COMPLETEнаUSB, reader-errorпустой. Логи полностью скопированы вlive-logs/. Одноразовыйnext_entryпустой.

Прочитана XSDT0xffffc000 (140байт,13указателей), черезFADTDSDT. Все прочитанные checksum0, MSDMpayloadнечитался. SSDTвживойXSDTнет. DSDTпобайтноравнаWindows78b42f6e…; FACP/CSRT/DBG2/GTDT/IORT/APIC/MCFG/PPTT/SPCR/TPM2/FPDTтакжепобайтносовпадают. BGRTотличаетсятолькостатусом(offset38:0→1)исвязаннойchecksum(offset9:91→90); адрес/размерструктуры/координатыизображениянеизменны. См.live-logs/comparison.md.

ОтсутствиеGPU0.AVS0подтвержденодляживогонаборатаблицтекущейзагрузки,нодинамическиедобавлениявиныхрежимахнеисключены. Отдельнаязадачаотправленанасоздание:confirmation1fa40b13-dbdf-400b-8423-ffa3a3fc5d1c pending,послесозданиясвязать19492/19084изаписатьвремя. ПричиначёрногоэкранаACPIвсёещёнеустановлена.


MEMDIAG успешный аппаратный результат19492(15мин): confirmation6d5e526d-f234-4921-b2df-c60d76533e12 pending, не повторять. Архивmemdiagобновлёнживымдампом.

2026-09-10 SMMU-аудит: создана19515 (confirmationebbabbd2approved). IORT OEMrev7180 отсутствуетвqcom_acpi_platlist(только8180);genericSMMUпутьнеполучаетqcomcfg_probe/write_s2cr. ДваSMMUтребуютразныхimpl;SCMестьтолькоOFmatch. ПотеряdisplayDMAприreset—гипотеза,неподтверждённаяпричиначёрногоэкрана. Отчётacpi-audit/smmu/README.md. AVS0задача19512создана;blocks19492/19084pending6d94ea2d-4886-47f5-8357-67e57d86cf7f; дополнениеIORTAVS0(5мин)pending66fdacdc-001c-4def-895a-94cf0cb839ad. АппаратныеоперацииSMMU/перезагрузкиневыполнялись.

19515 blocks19492/19084 подтверждены66aefeda-6b14-49f6-95dc-40c4e8c1f2f0; комментарий20мин pendinge96f06f9-51c2-4b1a-a471-78a0d285c151, не повторять.

# Черновой ACPI SCM порт TCL — не готов к аппаратному тесту

2026-09-10. DSDT SCM0 HIDQCOM080B, без_CRS/_CCA; родитель_SB также без_CCA (scm-cca.log).

- qcom_scm.c:2333 probe вызывает devm_of_icc_get. interconnect/core.c:562 безOFnode возвращаетENODEV. Это воспроизводимый по коду блокер простого ACPI-match.
- Подготовлен scm-acpi-draft.patch: добавить linux/acpi.h, ACPI_IDQCOM080B и acpi_match_table; ICCзапрашиватьтолькоприOFnode. Только эти изменения.
- Применён исключительно к /tmp/tcl-acpi1-linux/drivers/firmware/qcom/qcom_scm.c. Исходник before сохранён. Objectbuild `make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1 drivers/firmware/qcom/qcom_scm.o` успешен.
- ПолныйImage с этим патчем НЕсобирался; последняяACPI1ImageнаUSBостаётсяпрежней. Рабочиеaudio2/memdiagдеревьяНЕизменены. Нельзя путать исходныйACPI1кодпослечерновика с ранее собраннымImage.

## Нерешённая DMA-зависимость

ACPI_CCA_REQUIRED=y, acpi_dma_supported требуетcca_seen. Отсутствие_CCAуSCM0ипредковведёткdma_dummy_opsчерезplatform_dma_configure/acpi_dma_configure_id. qcom_tzmemвыделяетпамятьdma_alloc_coherent;initialpoolsize0можетотложитьошибкудопервойреальнойаллокации. Поэтому успешныйprobe/availableнеозначаетготовностьSCMбуферов. Не назначатьcoherentбезсверкисDT/SCMсемантикой.

## Прочие этапы probe

find_dload_addressбезOFphandleвыходитбезадреса;clocksoptional;reserved-memory-ENODEVдопускается;GENERICtzmemневызываетSHMBRIDGEenable;IRQoptional-ENXIOдопускается. После__scmпубликацииидут__get_convention, set_download_mode, disable_sdiприdownload_mode0, затемQSEECOM/QTEEинициализацияпоконфигурации. АппаратныхSMC/MMIOоперацийсчерновикомнебыло.

GitHubпоискчерезgh: QCOM080Bвqcom_scm.cиopenbsd/srcневыдалсовпадений. Этоограниченныйпоиск,неутверждениеоботсутствиивсехготовыхрешений.


SCMзадача19517создана55a73b17approved, blocks19515/19492подтверждены8cfa5c74. Комментарий20минpending8c7fabb5-272b-4b34-ac26-bd38c2247ce3,неповторять. Архивacpi-scmобновлён.


## DMA и черновик v2 (2026-09-10)

Живой DT MEMDIAG проверен по /sys/firmware/devicetree/base: SCM, firmware и root не содержат dma-coherent/dma-noncoherent/dma-ranges/iommus. CONFIG_ARCH_DMA_DEFAULT_COHERENT не включён; of_dma_is_coherent возвращает dma_default_coherent=false. Следовательно, программная DMA-семантика рабочего SCM — noncoherent. Это подтверждение выбора Linux, а не отдельное измерение свойств TrustZone.

Подготовлена дополнительная SSDT scm-cca0.asl: External SCM0 DeviceObj, Scope(SCM0), Name(_CCA,Zero). OEM DSDTнеизменена. iasl20250807:0errors/0warnings/0remarks. AML70байт,SHA256d63bfb4959a75431c1416a34a99aa702888a436fed435304e4eafbdb5d044b25. В acpiexec20260408 OEMDSDT+SSDTуспешнозагружены; evaluateSCM0._CCA возвращает0. Эта таблица НЕустановлена наTCL.

_CCAдолженприсутствоватьдоACPIсозданияplatformdevice: acpi_platform.c172 задаётdma_mask32биттолькоприacpi_dma_supported,иначе0. Нельзя просто подгрузить свойство посленеудачногоprobeиобъявитьDMAисправленным. БудущийACPIкомплектдолжензагрузитьSSDTраньшесканирования,черезпроверенныймеханизмраннихACPIтаблицinitramfs.

Актуальный scm-acpi-draft-v2.patch включает предыдущиеID/ICCизменения и:
- раннийотказ с понятнойошибкойприотсутствии_CCA;
- initial_size=PAGE_SIZEдляACPIпулаTZmem, чтобыпроверитьреальнуюаллокациюдопубликации__scm (DTinitial_size0сохранён);
- acpi_dev_clear_dependencies послеуспешногоprobe.
ОбъектW=1скомпилировануспешно. GENERICTZmemвэтомядреиспользуетdma_alloc_coherent,неSHMBRIDGE;приnoncoherentdeviceDMAAPIсамобеспечиваетсоответствующеепредставлениепамяти. Наличия_CCAнедостаточнодлядоказательстваработоспособностиSMCбуферов.

Изменёнтолькоqcom_scm.cв/tmp/tcl-acpi1-linux. ПолныйImagev2нестроился,наTCLнеустанавливался. Требуетсяаппаратнаяпроверкааллокации+безопасногозапроса,проверкапобочныхдействийSCMprobe,затемSMMUотдельно. НикакихSMC/MMIOвызововсновымкодом/перезагрузокнебыло.


19517 DMA/v2отчёт15минpending6c8788d2-cee2-442a-a411-9ba87a48743f. Архивacpi-scmобновлён;неповторятькомментарий.


## 2026-09-10 — полный реестр устройств и DT

По просьбе пользователя снят живой DT boot 4e6555b5-375b-4855-8672-59a720177a4c. Каталог `dt-inventory-20260910/`: основной перечень device-summary.md, полная Markdown-таблица 738 узлов full-inventory.md, properties.json, live-tree.tar, live.dts и привязки драйверов. Адреса, IRQ, GPIO/pinctrl, питание, clocks, DMA и остальные свойства сохранены; phandle разрешены до путей. Отключённые блоки и сведения только из ACPI/Windows отделены от рабочего DT.

DMA-проба рабочего SCM завершилась: coherent=0, mask/coherent_mask=0xffffffff, выделено 4096 байт DMA 0x998ae000, CPU pattern OK, память освобождена. Модуль выгружен, SSH/Weston active. SMC не выполнялся; это не проверка ACPI SCM.

2026-09-10: основная таблица устройств опубликована в #19084, comment181422; полная таблица 738 узлов — attachment7799, оба подтверждены.
