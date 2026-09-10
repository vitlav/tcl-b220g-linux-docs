# TCL ACPI1 — первый автономный ACPI boot

Подготовка 2026-09-09, задача19492. Аппаратного запуска ещё не было.

Ядро 6.18.34-tcl-acpi1 собрано из копии рабочего /tmp/tcl-audio2-linux в /tmp/tcl-acpi1-linux. Исходный код дополнительно не изменён; изменения Kconfig в config.diff, окончательная конфигурация kernel.config. Компилятор /bin/aarch64-linux-gnu-gcc из ALT, 15.3.1. Команда: make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j12 Image. Драйвер UFS отключён, встроены ACPI/GTDT/IORT/I2C OpRegion/AC/BATTERY/BUTTON/I2C_HID_ACPI и IKCONFIG. Сборка имеет предупреждения unused variables в уже существующем acer-aspire1-ec.c; ошибок сборки нет.

GRUB использует OEM ACPI: acpi=force и отдельный acpi=nospcr, команды devicetree в новом пункте нет. EFI stub может создать служебный FDT для handoff — это не описание платформы через DT. OEM ACPI не подменяются. efi=novamap,noruntime и cpuidle.off пока оставлены как диагностическая база; это не итоговая конфигурация питания.

Initramfs целиком автономный: статический ARM64 BusyBox, /init, без Ubuntu rootfs и модулей от DT-ядра. Результат выводится на экран. /run/acpi-report содержит логи, таблицы (без MSDM), список ACPI/platform/I2C/USB устройств, IRQ и CPU. Сохранение допускается только на USB-раздел UUID405E-8AB3 с маркером /tcl-acpi1/TCL-ACPI1; путь /tcl-acpi1/logs/<boot-id>/. Пока USB ACPI не подтверждён, гарантировать сохранение нельзя. При saved=no автоматической перезагрузки нет, чтобы можно было сфотографировать экран; RAM-логи исчезнут после перезагрузки. При saved=yes возврат к DT через120секунд.

Меню целиком сохранено: исходный файл grub-va.cfg найден дословно внутри рабочего BOOTAA64.EFI, SHA2569bf487c31f44164c022a74c4c0d1228233929cef77e4c196024341898ae08147. Добавлена только запись tcl-acpi1; исходный default=tcl-audio2-va сохранён до согласования теста. Генератор и arm64-efi модули из одного комплекта Debian GRUB2.12, как в проверенном рецепте ubuntu-base/build-boot.sh. Не смешивать с системным GRUB2.14.

Не обещать в первом ACPI опыте клавиатуру, сеть, USB-root или native DRM: пробелы перечислены в comparison.md. UEFI framebuffer — канал наблюдения, его доступность после ACPI boot тоже ещё требует проверки. Перезагрузка — после согласования пользователя.

Виртуализация в текущем DT-ядре: CONFIG_KVM=y, но dmesg сообщает HYP mode not available, /dev/kvm отсутствует. Namespaces/cgroups включены. ACPI сам по себе доступ к EL2 не создаёт; KVM оставлен в тестовом ядре для сравнения журнала.

Проверка BusyBox на TCL: applet blkid отсутствует. В образ добавлен настоящий /sbin/blkid вместе с libblkid.so.1/libc.so.6/ld-linux-aarch64.so.1 из работающей Ubuntu. Не полагаться на наличие applet в static BusyBox. CPIO проверен собственным разбором заголовков и наличием всех файлов, ARM64 Image magic и встраивание полного grub.cfg проверены.

Уточнение по виртуализации: журнал текущего DT boot содержит All CPU(s) started at EL1. Поддержка EL2 у архитектурных ядер Cortex-A76/A55 описана Arm: https://documentation-service.arm.com/static/63739bde5854836459183244 . Прямое чтение ID_AA64PFR0 через sysfs пока недоступно; не заявлять, что аппаратный регистр прочитан.

Установка завершена: контрольные суммы файлов USB и установленного EFI проверены. Backup BOOTAA64-before-acpi1.bak сохранён и проверен. Новый пункт доступен, default остаётся tcl-audio2-va. Аппаратного запуска не было. Отчёт отправлен в19492, confirmation2dea6df1-f14a-4e31-aec0-cd8c3531d3d3 pending.

Пользователь разрешил аппаратный тест. Подготовлен одноразовый выбор через внешний /tcl-acpi1/grubenv: next_entry=tcl-acpi1 очищается save_env до выбора пункта, при ошибке сохранения остаётся DT default. EFI пересобран тем же GRUB2.12 с модулем loadenv, SHA256d45aca3ab5678fbc142aaa600e9435ea6300e3f67baac2b9778864bcaef8ec6f. Перезагрузка отправляется после sync и проверки файла.

Экранная диагностика v2: initramfs SHA256900b585a61a7badad0d47f7891c1e83f768466b279c5943c234f7ea7f5d8d0f8, EFIa69436c2e98a928d3919be906cf8697e95fd436c8cd8913ccc7419140883c685. Добавлены EFI systab/DT-root и циклический вывод ранних ACPI-строк18строк/20секунд. log_buf_len=4M вместо конфигурационного128KiB. По замечанию пользователя удалены initcall_debug/ignore_loglevel/keep_bootcon, loglevel=6, чтобы сократить шум и задержкиEFIвывода. Провереныblkidвchroot, sh-nиналичиеgrep/sed/sleep/wc. ПервыйinitramfsсохранённаUSBкакinitramfs-first-test.cpio.gz.
