# ACPI: что установлено и что мешает загрузке

Рабочая система использует DT. OEM ACPI — отдельная исследовательская ветка; переключение на ACPI после запуска ядра не переносит уже созданные устройства автоматически.

## Таблицы проверены на живом ноутбуке

MEMDIAG прочитал XSDT `0xffffc000` (13 указателей) и DSDT через FADT, проверил длины и checksum. DSDT 189125 байт, SHA256 `78b42f6edc265bffdb3ad1d82c0e63bd98792ab42db9238571f893eeeb06b19f`, побайтно совпадает с Windows. Другие прочитанные основные таблицы также совпадают; BGRT отличается статусом изображения и соответствующей checksum. В текущей XSDT SSDT нет. MSDM payload не читался.

## Независимые проблемы

| Проблема | Установленный факт | Ограничение вывода |
|---|---|---|
| ACPI при PCI=n | Default address space PCI_CONFIG регистрировался при выключенном обработчике; подготовлен условный compile guard | После исправления ACPI всё ещё не даёт рабочую систему |
| GPU0.AVS0 | Ссылка присутствует в DSDT TZ7._TZD и IORT, определения в текущих таблицах нет | Это не доказанная причина чёрного экрана |
| SCM QCOM080B | Нет ACPI match в исходной SCM реализации; unconditional OF interconnect даёт ENODEV | Простой match не решает DMA и побочные вызовы probe |
| SCM DMA | В OEM SCM0 нет _CCA; на ARM64 это ведёт к dma_dummy_ops | Требуется ранняя таблица/корректная конфигурация до создания device |
| SMMU | IORT OEM revision 0x7180 отсутствует в Qualcomm ACPI match list; Apps и Adreno требуют разных impl | Потеря display DMA при generic reset — гипотеза, последняя строка зависания не получена |
| BAT0 GSBus | Для батареи требуется EC/I²C handler и зависимости | Ошибка прямого вызова _STA в симуляторе не доказывает преждевременный вызов Linux |

SCM текущего рабочего DT: `coherent=0`, обе DMA-маски `0xffffffff`; одноразовая проба успешно выделила и освободила 4096 байт. SMC не вызывался. Это доказательство Linux DMA-конфигурации, не проверка ACPI/TrustZone передачи.

Подготовлена SSDT с `_SB.SCM0._CCA = 0`; компиляция и acpiexec успешны. Она **не установлена**. SCM v2 с проверкой DMA включён в полностью собранное ядро ACPI2 вместе с SMMU selection draft; аппаратного теста ещё не было. Ранний несжатый CPIO подготовлен и проверен ядровым earlycpio-парсером; объединённый загрузочный образ и аппаратный тест ещё предстоят. [Сборка, проверки и SHA256](../research/acpi-audit/scm/early-ssdt/README.md).

[Общий аудит](../research/acpi-audit/README.md) · [AML](../research/acpi-audit/aml-static/README.md) · [SCM](../research/acpi-audit/scm/README.md) · [SMMU](../research/acpi-audit/smmu/README.md) · [PCI=n](../research/acpi-boot/pci-disabled/README.md).

Дополнительно проверены OF-зависимости выбора Adreno impl и политики клиентских доменов: [отчёт по SMMU](../research/acpi-audit/smmu/client-policy.md). Это отдельные части ACPI-порта; глобальный identity domain не используется.

[Черновик выбора Apps/Adreno SMMU](../research/acpi-audit/smmu/selection-draft/README.md): изолированная сборка объекта ACPI=y/n, два случая из живой IORT и шесть отрицательных проверок. Включён в полный Image ACPI2; не установлен и не проверен на оборудовании.

## Прямые ссылки на изменения ядра

- [ACPI default address spaces при PCI=n](../patches/kernel/acpi/acpica-default-spaces-without-pci.patch).
- [SCM ACPI v2: match, DMA и зависимости](../patches/kernel/acpi/scm-acpi-draft-v2.patch); предыдущий v1 заменён этим вариантом.
- [SC7180: раздельный выбор Apps и Adreno SMMU](../patches/kernel/acpi/sc7180-acpi-smmu-selection.patch).

Последние два патча остаются черновиками с проверкой сборки; совместный загрузочный комплект ещё не проверен аппаратно.

## Полная сборка и новые препятствия USB

[ACPI2: сборка и манифест](../research/acpi-boot/acpi2/README.md): `6.18.34-tcl-acpi2`, Image и 948 согласованных модулей. Объединённый initramfs ещё не подготовлен.

[Проверка IORT mappings](../research/acpi-audit/iort-mappings/README.md): все 16 NamedComponent имеют flags=0, поэтому обычный Linux-путь без input ID не выбирает SID. На настоящих функциях ядра проверены первые отображения всех 16 узлов: с явным ID перевод успешен. Это не аппаратный DMA-тест. Задача [19529](https://bugs.etersoft.ru/19529).

[Ресурсы USB в ACPI и DT](../research/acpi-audit/iort-mappings/usb-resources.md): память у URS0, IRQ у дочернего USB0; требуется корректный firmware parent, явный DMA input ID и управление PHY/питанием. Задача [19530](https://bugs.etersoft.ru/19530). До решения доступа к диагностике следующая ACPI-загрузка не подготовлена.
