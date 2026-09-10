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

## Успешная аппаратная проверка MEMDIAG

2026-09-10: после разрешённой перезагрузки boot4e6555b5-375b-4855-8672-59a720177a4c. В cmdline /tcl-memdiag/Image иtcl.memdiag=1. Wi-Fi192.168.8.177,SSHactive,Westonactive. tcl-memdiag.service Resultsuccess; RESULTsuccess/COMPLETEнаUSB, reader-errorпустой. Логи полностью скопированы вlive-logs/. Одноразовыйnext_entryпустой.

Прочитана XSDT0xffffc000 (140байт,13указателей), черезFADTDSDT. Все прочитанные checksum0, MSDMpayloadнечитался. SSDTвживойXSDTнет. DSDTпобайтноравнаWindows78b42f6e…; FACP/CSRT/DBG2/GTDT/IORT/APIC/MCFG/PPTT/SPCR/TPM2/FPDTтакжепобайтносовпадают. BGRTотличаетсятолькостатусом(offset38:0→1)исвязаннойchecksum(offset9:91→90); адрес/размерструктуры/координатыизображениянеизменны. См.live-logs/comparison.md.

ОтсутствиеGPU0.AVS0подтвержденодляживогонаборатаблицтекущейзагрузки,нодинамическиедобавлениявиныхрежимахнеисключены. Отдельнаязадачаотправленанасоздание:confirmation1fa40b13-dbdf-400b-8423-ffa3a3fc5d1c pending,послесозданиясвязать19492/19084изаписатьвремя. ПричиначёрногоэкранаACPIвсёещёнеустановлена.
