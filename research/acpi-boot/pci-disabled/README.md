# ACPICA: отключение при CONFIG_ACPI=y, CONFIG_PCI=n

Фото восьми экранов повторного ACPI1 теста от2026-09-10 сохранены в ../acpi1/photos/screenlog-v2/. Ручные тексты рядом с JPG; для EFI memory map расшифрованы только существенные строки, остальные доступны в оригиналах. Не выдавать эти тексты за полный машинный dmesg.

Ранняя инициализация действительно использовала OEM ACPI: EFI ACPI20 RSDPffffd000, XSDTffffc000, FADT/DSDT/APIC/GTDT/IORT распознаны; PSCI probing from ACPI, v1.1/SMC, GICredistributorCPU0..7, virtual arch_timer19.20MHz. Далее:

```
[0.645806] ACPI Error: AE_BAD_PARAMETER, During Region initialization (20250807/tbxfload-52)
[0.645834] ACPI: Unable to load the System Description Tables
[0.665495] pnp: PnP ACPI: disabled
```

acpi_load_tables сначала вызывает acpi_ev_install_region_handlers и лишь затем acpi_tb_load_namespace. Поэтому ошибка происходит до загрузки OEM AML namespace. include/acpi/platform/aclinux.h определяет ACPI_PCI_CONFIGURED при CONFIG_PCI; в исследуемой конфигурации PCI=n. acpi_gbl_default_address_spaces безусловно включает PCI_CONFIG, но соответствующий case в acpi_ev_install_space_handler условный. При попытке регистрации срабатывает default→AE_BAD_PARAMETER. Затем acpi_bus_init возвращает ошибку, acpi_init удаляетkobj/отключаетACPI, из-за чего нет /sys/firmware/acpi/tables и /sys/bus/acpi/devices.

Исправление: условно включать PCI_CONFIG в массив; число элементов4приPCI,3безPCI. Не меняет OEM ACPI, не включает PCI host probe и не скрывает ошибки других обработчиков.

Аналогичный патч Weidong Cui/Xinyang Ge опубликован2021-02-09:
https://lists.openwall.net/linux-kernel/2021/02/09/160
Ответ сопровождающего:
https://marc.info/?l=linux-acpi&m=161481714525432&w=2
В локальном Linux6.18.34 этого исправления не было. Статус новейшего upstream не проверен.

Файлы .before — исходные файлы до патча; acpica-default-spaces-without-pci.patch — точное изменение. Применено в /tmp/tcl-acpi1-linux, исходный DT-tree /tmp/tcl-audio2-linux не менялся. Версия ядра остаётся6.18.34-tcl-acpi1, исправленный Image нужно различать поSHA256/номеру сборки. Рабочий и первый ACPI Image уже сохранены отдельно наUSB/FTP.

Проверка make W=1 drivers/acpi/acpica/evhandler.o drivers/acpi/acpica/evregion.o успешна. nm -S показывает acpi_gbl_default_address_spaces размер3байта приPCI=n. ПолнаясборкаImageзапущена; аппаратнойпроверкипока нет. ДополнительныйWARN /devices/cache observed послеACPIfailure, связьсочисткой/CPUcacheнеподтверждена.

Запрос создания отдельной баги: confirmation23ab1e63-db56-4e4b-a0ec-70ddf64281e3 pending. После создания поставитьblocks19492/19084 и учётвремени. Отчёт19492 confirmation01c84004-df11-4082-842e-53e71accfae7 pending; не дублировать.

ПолнаясборкаImageзавершиласьуспешно, SHA256f889a28591a98583f58a8637593d2f065074ef74fe67e0d3de18aa5c1b3a2e1c. Предупреждениятолькопрежниеunusedvariablesвacer-aspire1-ec.c. Исправленноеполноеядроподготовлено,нонаTCLещёнеустановлено/непроверено. Отдельнаябага создана:19506.

## Файлы патчей

- [acpica-default-spaces-without-pci.patch](../../../patches/kernel/acpi/acpica-default-spaces-without-pci.patch) — Исключить PCI_CONFIG из default address spaces при PCI=n. Собран в ACPI1; вся ACPI-загрузка ещё не работает.
