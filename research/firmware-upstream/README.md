# Сопоставление с upstream linux-firmware

Проверен commit `eeccccbe83daf22e1931e3557ba05b2c02427e4e` (2026-09-10) [linux-firmware](https://gitlab.com/kernel-firmware/linux-firmware), файлы получены из [зеркала kernel.org](https://kernel.googlesource.com/pub/scm/linux/kernel/git/firmware/linux-firmware/+/eeccccbe83daf22e1931e3557ba05b2c02427e4e/). Пакеты не устанавливались, файлы на TCL не менялись. Сравнение выполнено по SHA256 бинарников, а не по совпадению имён.

## Наличие нужных компонентов

| Компонент TCL | Есть в проверенном upstream? | Степень совпадения |
|---|---|---|
| A630 SQE / GMU | Да | Оба файла побайтно совпадают |
| Venus 5.4 | Да | Побайтно совпадает |
| ath10k firmware-5.bin | Да | Побайтно совпадает |
| OEM board.bin | Да, как payload внутри board-2.bin | Два точных совпадения; выбор контейнерной записи TCL не проверен |
| OEM bdwlan.b67 | Да, payload ECS_QC710 | Точное совпадение; не основание менять variant TCL |
| OEM wlanmdsp.mbn | Есть WCN3990 файл, но другой | Хэш отличается; совместимость не проверена |
| qcadsp7180.mbn | По этому имени в WHENCE не найден | Аналог под другим именем не исключён |
| qcdxkmsuc7180.mbn | По этому имени в WHENCE не найден | Не путать с совпадающими SQE/GMU |
| qcmpss7180(_nm).mbn | По этим именам в WHENCE не найден | Полноценная замена OEM MPSS не установлена |
| regulatory.db / подпись | В WHENCE не перечислены | Происхождение установленного набора исследуется отдельно |

## Точные совпадения

| Upstream-файл | Байт | С чем совпадает |
|---|---:|---|
| `qcom/a630_sqe.fw` | 34188 | Файл работающей TCL-системы |
| `qcom/a630_gmu.bin` | 32768 | Файл работающей TCL-системы |
| `ath10k/WCN3990/hw1.0/firmware-5.bin` | 60 | Файл работающей TCL-системы |
| `qcom/venus-5.4/venus.mbn` | 922376 | Сохранённый проверенный Venus-комплект |

[Суммы upstream-файлов](results.json), [установленный поднабор](../../artifacts/firmware-runtime-manifest.json), [архивный поднабор](../../artifacts/firmware-manifest.json). Для этих четырёх бинарников найден точный публичный источник; это не устанавливает, откуда их первоначально скопировали при подготовке ноутбука.

## Board data WCN3990

`board-2.bin` содержит 36 записей. Разобраны вложенные TLV с проверкой границ и выравнивания на 4 байта; хэши вычислены по data payload без заголовков и padding. Формат сопоставлен с `drivers/net/wireless/ath/ath10k/hw.h` и `core.c` Linux 6.18.34. [Результат всех записей](board-records.json), [парсер](parse-board.py).

| Ключ upstream board-2 | Совпадающий OEM TCL файл | SHA256 payload |
|---|---|---|
| `bus=snoc,qmi-board-id=67` | bdwlan.bin / bdwlanu.bin / установленный board.bin | `e937bf4779ea30d588c3c9dab82913cd9e7fd31faaa45bde76b9fca876c0ad3c` |
| `bus=snoc,qmi-board-id=ff` | bdwlan.bin / bdwlanu.bin / установленный board.bin | `e937bf4779ea30d588c3c9dab82913cd9e7fd31faaa45bde76b9fca876c0ad3c` |
| `bus=snoc,qmi-board-id=67,qmi-chip-id=320,variant=ECS_QC710` | bdwlan.b67 / bdwlanu.b67 | `da2e615dea087b66889d09ab627e091710a35ce05167b0bd95cf4926196a62e1` |

Ключей с названием TCL нет. Совпадение data payload не доказывает, что драйвер TCL выберет нужную запись из этого контейнера. Нужна отдельная проверка фактического board/chip/variant и алгоритма fallback. Подменять variant на ECS_QC710 только из-за совпадения одного файла не обосновано.

Для повторения разбора сохраните upstream board-2.bin по относительному пути `ath10k/WCN3990/hw1.0/board-2.bin` в выбранном каталоге и выполните `python3 parse-board.py <каталог>`. Парсер записывает рядом board-records.json; на ноутбуке его запускать не требуется.

## Отличающийся WLAN DSP и происхождение

Upstream `ath10k/WCN3990/hw1.0/wlanmdsp.mbn`: 3725044 байт, SHA256 `92e1501254e6de78c0f2e2cf091507d488b608d07e53acd14813a82744823ec2`. WHENCE связывает его с `qcom/sdm845/wlanmdsp.mbn` и версией WLAN.HL.2.0-01387-QCAHLSWMTPLZ-1. OEM TCL имеет SHA256 `b3213d834d11e602c2422690edf316f8fa786ab2d1a34df60b9f4bf83801fa82`; файлы различаются. Совместимость upstream DSP с TCL не проверена.

В WHENCE этого коммита нет имён `qcadsp7180`, `qcdxkmsuc7180`, `qcmpss7180` или упоминания TCL/SC7180. Это результат поиска в перечне, не доказательство отсутствия побайтного аналога под любым другим именем. OEM ADSP, GPU secure firmware и MPSS остаются отдельной частью комплекта.

WHENCE указывает redistributable-лицензирование: для Venus — LICENSE.qcom/NOTICE.qcom, для рассматриваемой группы ath10k — LICENSE.QualcommAtheros_ath10k. Условия следует брать из того же upstream-коммита. Наличие публичных аналогов не меняет автоматически лицензию остальных OEM-файлов.
