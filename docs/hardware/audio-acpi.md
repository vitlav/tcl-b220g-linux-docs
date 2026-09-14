# ACPI-аудио TCL B220G

Состояние проверки на 14 сентября 2026 года. Все сведения относятся к отдельному ACPI-запуску на Linux `7.2.4-tcl-acpi-display1+`; DT-аудиотракт описан отдельно в [спецификации DT-аудио](audio.md).

## Подтверждённый тракт воспроизведения

На оборудовании зарегистрирована ASoC-карта `TCL B220G ACPI Audio` (ALSA ID `Audio`). Воспроизведение идёт через Q6ASM `MultiMedia1`, Q6 routing, Q6AFE backend `RX_CODEC_DMA_RX_0`, LPASS RX macro, SoundWire RX и кодек WCD9385. PCM-параметры backend: stereo, 48 kHz, `S16_LE`.

Пользователь услышал тестовый 440 Hz тон через встроенные динамики. Также на ACPI-системе проверен запуск Big Buck Bunny через mpv/Wayland: mpv вывел `AO: ALSA 48000Hz stereo`, PCM переходил в `RUNNING`, а DAPM включал GPIO46/47 во время потока. Последний запуск фильма здесь подтверждает работу программного аудиовыхода и видеокомпозиции; отдельного подтверждения слышимости именно этого прогона и акустического отсутствия щелчка при остановке нет.

| ALSA control | Сохранённое проверенное значение | Интерпретация |
|---|---:|---|
| `HPHL Volume`, `HPHR Volume` | 20/24 | Выходная громкость HPH |
| `RX_RX0 Digital Volume`, `RX_RX1 Digital Volume` | 78/124 | RX digital gain |
| `RX_MACRO RX0 MUX`, `RX_MACRO RX1 MUX` | `AIF1_PB` | RX macro input |
| `RX INT0_1 MIX1 INP0`, `RX INT1_1 MIX1 INP0` | `RX0`, `RX1` | Стереомаршрут |
| `RX INT0_1 INTERP`, `RX INT1_1 INTERP` | соответствующий `RX INT*_1 MIX1` | RX interpolator |
| `RX INT0 DEM MUX`, `RX INT1 DEM MUX` | `CLSH_DSM_OUT` | Class-H output |
| `RX HPH Mode` | `CLS_H_LP` | HPH operating mode |
| `HPHL Switch`, `HPHR Switch`, `CLSH Switch` | `on` | Включение аналогового тракта |
| `HPHL_RDAC Switch`, `HPHR_RDAC Switch` | `on` | DAC channels |
| `RX_CODEC_DMA_RX_0 Audio Mixer MultiMedia1` | `on` | Связь frontend с RX backend |

После изменения mixer control проверяйте его чтение через `amixer -c 0 cget name='CONTROL'`. Сохранённый профиль находится в `/var/lib/alsa/asound.state`; `/usr/sbin/alsactl restore 0` и восстановление по udev-событию карты `Audio` проверены в текущей системе. Значения impedance и HPH type доступны только для чтения. Предупреждение об отсутствующем UCM профиле не препятствует прямому ALSA playback.

## Speaker PA и остановка потока

Внешний усилитель встроенных динамиков включается GPIO46/47: оба уровня `high` при активном playback и `low` после закрытия PCM. Это проверено по DAPM и GPIO. Постоянный внешний модуль карты управляет PA через стандартный DAPM speaker widget: включает линии после power-up speaker path и выключает перед power-down. Измерения электрического уровня выхода усилителя не выполнялись.

Для снижения риска stop-pop программная последовательность проверена на коротком mpv-фрагменте: плавно уменьшить mpv volume до нуля, отключить `Internal Speaker Switch` пока PCM ещё `RUNNING`, затем закрыть PCM. GPIO46/47 перешли в low до закрытия потока. Программный порядок подтверждён; акустическое отсутствие щелчка при остановке ещё не подтверждено пользователем.

## Внешний модуль карты

Исходник карты и сборочная инструкция находятся в [`patches/kernel/acpi/audio-module-7.2.4/`](../../patches/kernel/acpi/audio-module-7.2.4/). Это внешний модуль ASoC, собранный для точного ядра `7.2.4-tcl-acpi-display1+`; он не является патчем к ванильному ядру. Модуль установлен в `/lib/modules/7.2.4-tcl-acpi-display1+/extra/tcl-audio/tcl_acpi_card.ko`, `depmod` выполнен, загрузка через `modprobe tcl_acpi_card` проверена. SHA-256 установленного модуля: `0a26d084d6dc7c03fdba7a6602a0dc139aa6455553f14c4266bbf09a9533ff63`.

Сборка для текущего целевого ядра:

```sh
make KERNEL_SRC=/path/to/matching/linux-7.2.4 \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1
```

Перед сборкой исходники ядра, `.config` и `Module.symvers` должны соответствовать установленному на ноутбуке ядру.

## Автозапуск ACPI-аудиоподсистемы

Установлены и включены две отдельные службы:

1. `tcl-acpi-audio-prepare.service` ждёт привязки нужных Qualcomm providers и загружает Q6/LPASS/SoundWire/WCD модули.
2. `tcl-acpi-audio-start.service` запускается после подготовки, переводит ADSP в `running`, ждёт появления APR `q6adm`, загружает `tcl_acpi_card` и проверяет регистрацию ALSA-карты.

Обновлённый стартовый сценарий и unit-файлы сохранены в `patches/kernel/acpi/audio-module-7.2.4/root-overlay/`. Автозапуск ADSP/q6adm и регистрация карты проверены на работающей системе и повторным запуском службы без reboot. ALSA state восстанавливается штатной ALSA/udev интеграцией.

## Ограничение постоянного запуска

Автозапуск ADSP, регистрация карты и сохранение mixer-профиля работают. Однако это пока не означает, что динамики готовы сразу после холодной загрузки: WCD9385 зависит от питания и reset до enumeration по SoundWire. Временные тестовые APCC votes для `LDO15_A`/`BOB_C`, guarded GPIO58 reset и runtime-PM последовательность позволили получить слышимый тон в текущем boot, но после снятия этих ограниченных leases оба SoundWire slave вернулись в `UNATTACHED`.

Постоянная схема питания WCD9385 и роль reset-линии ещё не подтверждены, поэтому временные тестовые leases не включены в boot. Для полного холодного старта остаётся установить OEM-подтверждённое соответствие питания и reset стандартным regulator/GPIO consumers, автоматизировать attach, затем проверить слышимый звук после reboot без предварительного запуска Windows. До такой проверки статус — **звук подтверждён в текущем сеансе; полный автоматический cold-boot audio не подтверждён**.

## Capture

Физический микрофон и его routing пока не подтверждены. Временная проверка Q6ASM capture не получила ожидаемый акустический сигнал, поэтому микрофон нельзя считать работающим. Нужны проверенные OEM analog/DMIC port mapping и повторная запись с измеримым тестовым сигналом.
