# Карта исходников и конфигураций ядра

Этот указатель дополняет [каталог патчей](../patches/kernel/README.md): часть изменений сохранена отдельными исходниками модулей, а не diff. Наличие файла в репозитории не означает, что он входит в каждое собранное ядро или пригоден для автоматического применения.

## Конфигурации

| Конфигурация | Назначение | Проверка |
|---|---|---|
| [MEMDIAG](../research/acpi-audit/memdiag/kernel.config) | Рабочая DT-система, release `6.18.34-tcl-audio2`; STRICT_DEVMEM отключён | Загрузка и чтение ACPI-таблиц подтверждены; это не ACPI-инициализация устройств |
| [ACPI1](../research/acpi-boot/acpi1/kernel.config) | Отдельная ACPI-конфигурация | Не является рабочей заменой DT-комплекта |
| [ACPI2](../research/acpi-boot/acpi2/kernel.config) | SCM v2 и выбор Apps/Adreno SMMU | Полная сборка Image и 948 модулей; аппаратно не проверена |
| [ICE1](../research/peripherals/ice-research/ice1/kernel.config) | Конфигурация исследования UFS ICE | Не заменяет конфигурацию текущего аудио/графического комплекта |

Контрольные суммы приведены для файлов конфигурации в Git, не для Image:

| Файл | SHA256 |
|---|---|
| `acpi-audit/memdiag/kernel.config` | `943ef791c2c37e20ca086b4c2b5b9ca3534f40c1a3911e40b6e6381169d15adc` |
| `acpi-boot/acpi1/kernel.config` | `e2eccc4ca78b44c23d95a6b2fc2f5693265e49cb3e725e9f1cb24f8c5606b142` |
| `acpi-boot/acpi2/kernel.config` | `55540cb5eabc19bbf387c51c9a484d19f6095c1fc4efabf462feca2daddc89a3` |
| `peripherals/ice-research/ice1/kernel.config` | `64ff22f70a710f6b1e48bc940bcd7784f51b742905451ab98aa8aa770d4dd6a9` |

ACPI2 также содержит [манифест сборки](../research/acpi-boot/acpi2/manifest.json). Одинаковый kernel release сам по себе не подтверждает идентичность Image, конфигурации или внешних модулей.

## Device Tree

| Файл | Назначение и ограничения |
|---|---|
| [hardware/live.dts](hardware/live.dts) | Снимок живого дерева: источник сведений об активной конфигурации и runtime-узлах; не замена исходному boot DTB |
| [va-probe.dts](../research/peripherals/camera-audio/audio2/va-probe.dts) | Сохранённое описание AUDIO2/VA; дополнительные аудиосвязи создавались модулями |
| [audio2.dts](../research/peripherals/camera-audio/audio2/audio2.dts) | Ранний frontend-only draft; include `adsp-probe.dts` отсутствует в том же каталоге, поэтому самодостаточная сборка файла не обеспечена |
| [adsp-probe.dts](../research/peripherals/camera-audio/adsp1/adsp-probe.dts) | Черновик ADSP transport, исходная зависимость ранней диагностики; не готовая звуковая карта |
| [kms1.dts](../research/peripherals/wayland-probe/native-display/kms1-dt/kms1.dts) | Отдельная конфигурация KMS; не подмена полного AUDIO2 DT |

Для сборки надо восстановить точную цепочку include и преобразований выбранного DT, затем сопоставить итоговый DTB с загрузочным комплектом. Механическое исправление include в старом черновике не превращает его в текущую конфигурацию.

## Отдельные исходники модулей

| Компонент | Исходник | Назначение / предел применимости |
|---|---|---|
| LT8911EXB | [tcl_lt8911_handoff.c](../research/peripherals/wayland-probe/native-display/handoff/tcl_lt8911_handoff.c) | Проверенный handoff bridge; самостоятельная инициализация моста отсутствует |
| ASoC machine card | [tcl_audio_card.c](../research/peripherals/camera-audio/audio2/card-live/tcl_audio_card.c) | Проверенный playback frontend/backend; полноценного capture backend нет |
| WCD aggregate | [tcl_wcd_aggregate.c](../research/peripherals/camera-audio/audio2/aggregate/tcl-wcd-aggregate/tcl_wcd_aggregate.c) | Диагностическое создание связей кодека вместо завершённого DT |
| RX codec overlay | [tcl_wcd_rx_overlay.c](../research/peripherals/camera-audio/audio2/gpio58-reset-test/rx-codec-dt/tcl_wcd_rx_overlay.c) | Runtime-описание RX кодека |
| TX macro overlay | [tcl_tx_overlay.c](../research/peripherals/camera-audio/audio2/tx-live/tcl-audio2-tx/tcl_tx_overlay.c) | Runtime-описание TX macro |
| SoundWire TX overlay | [tcl_swr_tx_overlay.c](../research/peripherals/camera-audio/audio2/tx-live/tcl-swr-tx/tcl_swr_tx_overlay.c) | Runtime-описание шины TX |
| Speaker PA | [tcl_speaker_pa.c](../research/peripherals/browser/persistent/tcl_speaker_pa.c) | Диагностическое управление усилителем; не штатный DAPM driver |
| WCD watchdog | [tcl_wcd_watchdog.c](../research/peripherals/browser/persistent/tcl_wcd_watchdog.c) | Вспомогательный контроль временного аудиотракта |
| SAR | [aw96103.c](../research/peripherals/sensors/aw96103.c), [tcl_sar_client.c](../research/peripherals/sensors/tcl_sar_client.c) | Драйвер семейства и клиент TCL; измерения AW96105 подтверждены, питание/IRQ/единицы требуют уточнения |

Это основные точки входа, не список модулей для последовательного `insmod`. Порядок probe, владение GPIO/regulator, зависимости и cleanup должны быть согласованы с [аудиотрактом](hardware/audio.md). Альтернативные копии в соседних каталогах не следует смешивать.

## Требования к сборке внешних модулей

Используются `ARCH=arm64` и `CROSS_COMPILE=aarch64-linux-gnu-`; происхождение компилятора описано в [разделе ядра](kernel-and-ubuntu.md). Нужны точные исходники и подготовленное build tree целевого ядра с его конфигурацией, generated headers и `Module.symvers`. Одна команда `modules_prepare` не восстанавливает таблицу CRC для CONFIG_MODVERSIONS.

Сохранённые Makefile не все переносимы: например, machine card задаёт абсолютный include-path к `sound/soc/qcom` исследовательского дерева. Перед независимой сборкой его надо связать с выбранной базой. Файлы `*.mod.c` — сгенерированные результаты kbuild, а не самостоятельные исходники драйверов.

Для воспроизводимой поставки ещё нужны единая база/серия, переносимые правила сборки этих модулей и DT, проверка чистой сборки и манифест согласованных Image/DTB/initramfs/modules. Этот указатель не заявляет, что такая поставка уже готова.
