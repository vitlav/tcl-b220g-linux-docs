# Загрузка, диагностика и восстановление доступа

## Рабочий путь

UEFI → GRUB ARM64 → Image + DTB + initramfs → Ubuntu на USB. Windows находится на внутреннем UFS.

В рабочем комплекте конфигурация GRUB **встроена в EFI**, отдельный `/EFI/BOOT/grub.cfg` не является источником меню. Версия grub-mkstandalone должна совпадать с версией модулей GRUB.

Проверенный инструмент сборки — Debian GRUB 2.12 с соответствующим каталогом `arm64-efi`. Перед установкой проверяются синтаксис меню, наличие файлов каждого пункта, SHA256, резервная копия EFI и совместимость ядра/модулей/DTB/initramfs. Старый рабочий пункт сохраняется. One-shot next_entry очищается до перехода к эксперименту.

## MEMDIAG, текущий срез

| Компонент | SHA256 |
|---|---|
| MEMDIAG Image | `78c251e585d082d221b17429769b4ad861e9aeb23ef64e5249029610ca059ef5` |
| Рабочий audio2 Image, база | `3cfc0fcb6d48d4816423ad794a045bf783f049bc12311f5c66df213f179aa71e` |
| audio2 va-probe.dtb | `f795878b99d960c253b2b2cd994cbe29e15b7f2aed2e9da169c63cb807de1dd0` |
| audio2 initramfs.cpio.gz | `3c196074a0602c528840711587fefbb3a39b196a1f81affd2ec8641959de071c` |
| EFI с MEMDIAG | `51200ae5f014a18b2e8ac1cb5ba9b6d559f3d936090ff1850b931c17874eb7f8` |

MEMDIAG сохраняет DT и отключает STRICT_DEVMEM для ограниченного чтения ACPI RAM. Это диагностическая конфигурация, не рекомендация для постоянного ядра. Сборщик читает только проверенный reserved-диапазон и не читает MSDM payload.

## Логи до рискованного действия

Проверять запись на физическую USB заранее, а не только наличие кода logger. Текущий MEMDIAG пишет STARTED, исходные сведения, результат reader, RESULT и COMPLETE в `/tcl-memdiag/logs/<boot-id>/` на USB. Успех подтверждается фактическими файлами и sync. Wi-Fi/SSH должны быть подготовлены именно в запускаемом образе.

ACPI USB и сеть пока не подтверждены. Наличие shell или меню GRUB не обеспечивает сохранение логов после передачи управления ядру.

Подробности: [MEMDIAG](../research/acpi-audit/memdiag/README.md), [конфигурация ACPI1](../research/acpi-boot/acpi1/README.md), [Wi-Fi и SSH](../research/peripherals/wifi-ssh/README.md), [архив Windows](../research/peripherals/windows-backup/README.md).
