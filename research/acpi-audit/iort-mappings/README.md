# IORT NamedComponent: необходим явный входной ID

2026-09-10, Linux 6.18.34, живая IORT QCOM/QCOMEDK2 revision 0x7180. Таблица побайтно совпадает с Windows. Аппаратные операции не выполнялись.

## Найденное различие

Все 16 NamedComponent используют mappings с flags=0, без SINGLE_MAPPING. В `iort_nc_iommu_map()` (обычная настройка ACPI platform device без input_id) вызывается `iort_node_get_id()`, который возвращает начальный SID/родителя только при SINGLE_MAPPING. Поэтому обычный путь не получает iommu_fwspec лишь из наличия подходящего output SID в OEM-таблице.

Альтернативный путь `iort_nc_iommu_map_id()` с явным входным ID использует диапазоны input_base/id_count и может правильно вычислить SID. **Flags=0 не означает само по себе нарушение спецификации**: это несовместимость с выбранным Linux-путём без входного ID.

| Узел IORT | Input ID | Выходной SID | Результат host-проверки |
|---|---|---|---|
| `\_SB.URS0` | 0x80030000 | 0x540 | Без ID parent=NULL; с явным ID перевод успешен |
| `\_SB.USB0` | 0x80030000 | 0x540 | То же; такого корневого USB0 не следует путать с DSDT `\_SB.URS0.USB0` |
| `\_SB.UFS0` | 0x81030000 | 0xa0 | Без ID parent=NULL; с явным ID перевод успешен |
| Прочие 13 NamedComponent | См. mappings.json | См. mappings.json | Проверены первые mapping каждого узла |

У GPU0 — 27 mappings, среди них несколько диапазонов; у других компонентов тоже встречается id_count>0. Нельзя просто поставить SINGLE_MAPPING всем записям или свести все устройства к одному SID.

`acpi_dma_configure_id()` игнорирует ошибки IOMMU кроме EPROBE_DEFER и продолжает `arch_setup_dma_ops()`. Следовательно, успешный probe не доказывает корректную связь DMA с SMMU. Фактическое последствие на железе зависит от состояния потоков и DMA-режима; здесь оно не проверялось.

## Проверка

`check-mappings.py` извлекает неизменённые функции `iort_node_get_id()` и `iort_id_map()` из выбранного ядра и компилирует host harness. Структуры IORT заданы с packed-layout из actbl2.h; проверены checksum, длина и границы таблицы. Проверены первые отображения всех 16 узлов; flags=0 проверены для всех mappings. Полный проход регистрации устройств и IOMMU API в harness не исполняется.

```sh
python3 check-mappings.py --kernel /path/to/linux-6.18.34 --iort /path/to/07-IORT.dat
```

Результаты — check-results.json, полная расшифровка — mappings.json. IORT SHA256: `aec7a46d8518744956d3776fee085de8ac3f23e6ab44e1ae04611c76261637b9`.

Дальнейшая задача: выбрать явные input IDs для подтверждённых клиентов и правильного firmware parent или обосновать узкую коррекцию таблицы. Ни общий IORT-парсер, ни OEM IORT в этом этапе не менялись. Связано с задачами ACPI #19492 и SMMU #19515; отдельная задача [#19529](https://bugs.etersoft.ru/19529).
