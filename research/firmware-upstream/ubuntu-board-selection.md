# Выбор board data Ubuntu для TCL

**Проверено чтением работающего TCL:** QMI сообщает chip_id `0x320`, chip_family `0x4001`, board_id `0xff`, soc_id `0x400c0000`. Оба свойства DT `qcom,calibration-variant` и `qcom,ath10k-calibration-variant` отсутствуют. Пользовательский firmware search path пуст; среди board-файлов в установленном каталоге найден только `ath10k/WCN3990/hw1.0/board.bin`.

Журнал WLAN сообщает `WLAN.HL.3.2.2.c10-00748-QCAHLSWMTPLZ-1`, timestamp `2021-05-07 12:53`. Эта версия относится к работающему OEM DSP, а не к Ubuntu WCN3990/SDM845 DSP.

## Контейнер и алгоритм

Исследован `board-2.bin.zst` из wireless `20260319.git217ca6e4-0ubuntu1.2`: после декомпрессии 867116 байт, SHA256 `867e1010787764020653812167d93f5952cbbea05f576209d953d8c9322f18aa`. [35 записей контейнера](ubuntu-board-records.json) разобраны тем же [TLV-парсером](parse-board.py), что и upstream. Padding не включён в хэши payload.

В исходниках Linux 6.18.34 `ath10k_qmi_fetch_board_file()` переносит QMI board/chip IDs, проверяет DT variant и вызывает `ath10k_core_fetch_board_file()`. QMI IDs в имени форматируются через `%x`, то есть строки `ff` и `320` — шестнадцатеричные значения.

| Порядок поиска | Имя | Наличие в DEB board-2 |
|---|---|---|
| Полное имя с variant | `bus=snoc,qmi-board-id=ff,qmi-chip-id=320` | Нет; variant отсутствует |
| Без variant | `bus=snoc,qmi-board-id=ff,qmi-chip-id=320` | Нет; совпадает с первым запросом |
| Без variant и chip ID | `bus=snoc,qmi-board-id=ff` | Есть |

Выбранная по этому алгоритму резервная запись содержит 26328 байт с SHA256 `e937bf4779ea30d588c3c9dab82913cd9e7fd31faaa45bde76b9fca876c0ad3c`. Это точное совпадение с установленным board.bin и сохранёнными OEM bdwlan.bin/bdwlanu.bin. [Результат сопоставления](ubuntu-board-selection.json).

Запись ECS_QC710 также присутствует, но имеет board ID `67` и variant `ECS_QC710`, поэтому к наблюдаемому TCL с `ff` она не относится. Менять variant для её выбора не требуется.

## Граница подтверждения

Это вывод по исходникам драйвера, фактическим QMI/DT данным и содержимому DEB. Пакет не установлен, ath10k не перезапускался, загрузка API2-контейнера на оборудовании не проверена. Подтверждено совпадение предполагаемого payload, а не завершённый тест нового способа его загрузки.

Текущий ath10k собран без debug/debugfs/tracing; обычный журнал не показывает точное имя выбранного board-файла. Наличие только API1 файла согласуется с рабочей конфигурацией, но в этой проверке не выполнялось чтение буфера, уже переданного DSP.

Пакетный WLAN DSP по-прежнему отличается от OEM TCL. Совпадение board data не является доказательством совместимости другой DSP firmware или полного замещения OEM набора.
