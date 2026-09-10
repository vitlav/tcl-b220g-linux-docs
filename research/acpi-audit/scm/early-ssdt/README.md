# Ранняя SSDT для SCM DMA — локальная проверка

2026-09-10. Подготовлено и проверено **без перезагрузки и установки на TCL**.

## Что делает таблица

`scm-cca0.asl` добавляет только `Name (_CCA, Zero)` в существующий `\_SB.SCM0`. OEM DSDT не заменяется. Таблица содержит один объект, ноль устройств, ноль OperationRegion и ноль методов. Она не читает/пишет MMIO и не вызывает SMC.

В работающем DT SCM использует `coherent=0`, DMA mask и coherent mask `0xffffffff`. Предыдущая аппаратная проба выделила и освободила страницу, CPU pattern OK; SMC не выполнялся. Это обосновывает программную DMA-семантику `_CCA=0`, но не доказывает работу ACPI SCM с TrustZone.

## Сборка

Из каталога этого документа:

```sh
python3 build-early-cpio.py --output /tmp/tcl-scm-early-build
python3 check-kernel-parser.py --kernel /path/to/linux-6.18.34 \
  --archive /tmp/tcl-scm-early-build/scm-cca0-early.cpio \
  --aml /tmp/tcl-scm-early-build/scm-cca0.aml
cpio -it < /tmp/tcl-scm-early-build/scm-cca0-early.cpio
```

Нужны Python 3, iasl, C-компилятор и выбранные исходники ядра. Скрипты не устанавливают файлы на загрузочный носитель. iasl использован версии 20250807; при другой версии возможен другой hash AML из-за поля compiler revision.

| Артефакт | Размер | SHA256 |
|---|---:|---|
| scm-cca0.aml | 70 | `d63bfb4959a75431c1416a34a99aa702888a436fed435304e4eafbdb5d044b25` |
| scm-cca0-early.cpio | 1024 | `e56100971e1114dab15a308c8f6063a23d32433916105a48fa2cf16c9b649e3c` |

В архиве путь **без `./`**: `kernel/firmware/acpi/scm-cca0.aml`. Формат newc, несжатый, метаданные фиксированы, выравнивание 4 байта и дополнение до 512 байт.

## Проверка реальным парсером

Host harness компилирует неизменённый `lib/earlycpio.c` выбранного ядра, подставляя только минимальные userspace-заголовки. Проверяет имя, длину и побайтовое совпадение извлечённого AML, отсутствие второй таблицы.

| Вариант | Результат |
|---|---|
| Только несжатый ранний архив | Найдена SSDT, 70 байт |
| Ранний архив перед gzip | Найдена SSDT, 70 байт |
| Только gzip | Таблица не найдена |
| gzip перед ранним архивом | Таблица не найдена |

Gzip в этих тестах — синтетическая контрольная нагрузка, **не загрузочный initramfs**. Готовый объединённый образ Ubuntu/ACPI не создан. Порядок будущей сборки: несжатый early CPIO, затем существующий сжатый initramfs без изменения его содержимого. Нельзя сжать всё вместе.

## Порядок в Linux 6.18.34

1. `arch/arm64/kernel/setup.c`: `acpi_table_upgrade()` вызывается перед `acpi_boot_table_init()`.
2. `drivers/acpi/tables.c`: проверяются сигнатура, длина и checksum; ACPI таблицы копируются в зарезервированную память.
3. `acpi_table_initrd_override()` заменяет только совпадающие signature/OEM ID/OEM table ID с большей OEM revision. Наша SSDT с отдельной идентичностью TCLLAB/SCMCCA0 не совпадает с текущими OEM-таблицами.
4. `acpi_table_initrd_scan()` устанавливает дополнительные таблицы через `acpi_install_physical_table()`; это вызывается из `acpi_table_init_complete()`.
5. Позже `drivers/acpi/bus.c` вызывает `acpi_load_tables()` и сканирование ACPI-устройств. `_CCA` доступен до создания platform device и выбора DMA ops.

В ACPI1 конфигурации `CONFIG_ACPI_TABLE_UPGRADE=y`, `CONFIG_ACPI_CCA_REQUIRED=y`, встроенный initramfs пуст, `CONFIG_SECURITY_LOCKDOWN_LSM` выключен. В другом ядре следует перепроверить конфигурацию и lockdown: код явно может отклонить подмену/добавление таблиц.

## AML-проверка и границы результата

ACPICA acpiexec 20260408 загрузил OEM DSDT + SSDT; `evaluate \_SB.SCM0._CCA` вернул Integer 0. Сохранилась ранее известная ошибка ссылки GPU0.AVS0 (#19512); четыре cache allocation warning воспроизводились и в контрольном тесте ACPICA, не относятся к этой SSDT.

Утилита может вернуть код 0 при ошибке AML evaluation, поэтому проверен именно текст результата, а не только exit status. Первая команда имела лишний обратный слеш в пути; после исправления аргумента проверка успешно повторена.

SCM v2 остаётся черновиком, только объект скомпилирован. Даже успешная ранняя DMA-аллокация не проверяет firmware visibility. Probe SCM также определяет SMC convention, меняет download/SDI и вызывает QTEE probe — его нельзя считать чисто регистрационным.

Дополнительный OF-only fallback принудительного ARM64 SCM convention для SC7180 найден в `__get_convention()`. В текущем сохранённом dmesg `smc arm 64` **без `(forced)`**, то есть на этой firmware fallback не использовался. Не переносить quirk на все QCOM080B без идентификации платформы.

Бинарный комплект и полные логи: `/var/ftp/tmp/lav/tcl/acpi-scm/early-ssdt/`. В Git сохранены ASL, скрипты, manifest и результаты. Исторический манифест исходных 2429 файлов не изменяется.
