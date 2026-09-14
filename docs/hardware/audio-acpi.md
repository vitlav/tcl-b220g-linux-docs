# ACPI-аудиотракт TCL B220G

**Подтверждено на оборудовании:** ядро `7.2.4-tcl-acpi-display1+`, WCD9385 на SoundWire RX/TX, PCM playback через ADSP и слышимый звук встроенных динамиков. Проверена также полноэкранная картина Big Buck Bunny через Weston/mpv. Эта проверка описывает ACPI-путь отдельно от DT-конфигурации в [основном разделе аудио](audio.md); постоянная загрузочная интеграция пока не завершена.

## Рабочий playback-профиль

Обычная диагностическая карта называлась `TCL ACPI WCD9385 Capture Test`; ALSA card ID — `Test`, PCM — `MultiMedia1`, 48 kHz, stereo, S16_LE. Тракт проходит через Q6ASM, Q6 routing, Q6AFE `RX_CODEC_DMA_RX_0`, LPASS RX macro, SoundWire RX и WCD9385 HPHL/HPHR.

Перед воспроизведением оба SoundWire codec slaves должны сообщать `Attached`. В проверенном ручном профиле использовались следующие ALSA controls:

| Control | Проверенное значение | Значение шкалы |
|---|---:|---|
| `HPHL Volume`, `HPHR Volume` | 20 из 24 | 0 dB (шаг 1.5 dB от −30 dB) |
| `RX_RX0 Digital Volume`, `RX_RX1 Digital Volume` | 78 из 124 | −6 dB (шаг 1 dB от −84 dB) |
| `RX_MACRO RX0 MUX`, `RX_MACRO RX1 MUX` | `AIF1_PB` | Вход RX macro |
| `RX INT0_1 MIX1 INP0`, `RX INT1_1 MIX1 INP0` | `RX0`, `RX1` | Стерео RX-маршрут |
| `RX INT0_1 INTERP`, `RX INT1_1 INTERP` | соответствующий `RX INT*_1 MIX1` | RX interpolator |
| `RX INT0 DEM MUX`, `RX INT1 DEM MUX` | `CLSH_DSM_OUT` | Class-H/CLSH output |
| `RX HPH Mode` | `CLS_H_LP` | HPH operating mode |
| `HPHL Switch`, `HPHR Switch`, `CLSH Switch` | `on` | Unmute/enable |
| `HPHL_RDAC Switch`, `HPHR_RDAC Switch` | `on` | Enable headphone DACs |
| `RX_CODEC_DMA_RX_0 Audio Mixer MultiMedia1` | `on` | Connect MultiMedia1 to RX backend |

После каждой записи control нужно прочитать его обратно (`amixer -c 0 cget name='HPHL Switch'` и т. п.). Успешный exit code `amixer cset` без readback не считается подтверждением настройки.

GPIO46/47 — подтверждённое включение внешнего speaker PA: `high` активирует усилитель, `low` выключает. Слышимость подтверждена пользователем во время тона и фильма. Точные характеристики PA и электрический уровень аналогового выхода отдельно не измерялись.

## Power, reset и runtime PM

APCC из ACPI `AUDD` подтверждает участие `LDO15_A` (1.8 V) и `BOB_C` (3.3 V). Тестовая загрузка также использовала guarded GPIO58 reset и стандартный SoundWire runtime-PM suspend/resume для повторной enumeration. После корректного цикла оба codec slaves сообщали `Attached` и карта регистрировалась.

Это пока экспериментальная последовательность: ACPI coordinator не связывает WCD9385 с полным набором `vdd-rxtx`, `vdd-io`, `vdd-buck`, `vdd-mic-bias`; реальное соответствие линий питания к codec не установлено. В тестах применялись ограниченные APCC/GPIO58 leases. Они не являются постоянным power-management драйвером и не должны заменять точную схему регуляторов. GPIO58 нельзя описывать как окончательно установленный reset signal до завершения сверки OEM/ACPI.

`alsactl store 0` сохраняет текущую карту в `/var/lib/alsa/asound.state`; проверенные значения записаны для ALSA card ID `Test`. Пользователь наблюдал нулевой/заглушённый микшер в `alsamixer`, тогда как `amixer -c 0 cget` на активной карте показывал HPH 20/24 и switches `on`. Повторный `alsactl restore 0` завершился с кодом 0; HPH 20, RX 78 и все HPH/CLSH/RDAC switches прочитаны обратно с ожидаемыми значениями. Остаются предупреждения об отсутствующем UCM-файле и read-only impedance/HPH-type controls. Стандартное udev-правило также восстанавливает state при позднем событии `controlC*`, поэтому ранний запуск `alsa-restore.service` сам по себе не объясняет прежние нули/заглушение. Причина отображения в `alsamixer` и restore после холодного старта остаются открытыми.

## Загрузочная интеграция ACPI audio

Установленный `tcl-acpi-audio-prepare.service` загружает audio providers и ACPI coordinator, но оставляет ADSP остановленным; старт DSP — отдельный этап. Существующий `tcl-audio.service` относится к DT-пути и запускается только при `uname -r = 6.18.34-tcl-audio2`, поэтому на ACPI-ядре `7.2.4-tcl-acpi-display1+` он пропускается. Сейчас ADSP запускается отдельно, затем вручную регистрируется диагностическая карта. Для постоянной ACPI-конфигурации нужен свой порядок: запустить ADSP, дождаться APR `q6adm` service, загрузить штатную ASoC-карту с DAPM PA, восстановить ALSA profile и проверить SoundWire attach. Не включать DT-ограниченную unit без адаптации.

## Проверка playback

На ACPI-ядре запущен Big Buck Bunny длительностью 9:56 через Weston/Wayland и mpv: fullscreen, GPU/GL вывод, `--hwdec=no`, ALSA `hw:0,0`, mpv volume 50%. PCM `MultiMedia1` был `RUNNING`; GPIO46/47 включались только после подтверждения активного PCM. Пользователь подтвердил, что фильм слышен. Этот тест подтверждает аудиовыход и программный видеодекодер с GPU-композицией, но не аппаратное декодирование видео.

При остановке фильма пользователь слышал треск. Временный тестовый модуль `tcl_acpi_capture_card_pa_dapm_test.ko` проверил стандартное управление GPIO46/47 через DAPM speaker widget: `POST_PMU` поднимает PA, `PRE_PMD` опускает его. В отдельном 24-секундном PCM тесте GPIO были high при `PCM RUNNING` и вернулись в low при закрытии потока.

Упорядоченная остановка проверена на коротком mpv-фрагменте Big Buck Bunny при подключённых WCD9385 и действующих ограниченных APCC/GPIO58 leases. После передачи mpv IPC команд снижения volume от 50 до 0, `Internal Speaker Switch=off` перевёл DAPM в `Off` и GPIO46/47 в low, пока PCM оставался `RUNNING`; после выдержки 200 мс команда `quit` закрыла PCM. После теста switch восстановлен в `on`, leases штатно сняты, все шесть SoundWire runtime-PM узлов вернулись в `auto/suspended`, а кодеки — в `UNATTACHED`. Boot ID не изменился.

Это подтверждает программный порядок fade → PA off при открытом PCM → закрытие PCM → снятие reset/APCC, но не подтверждает отсутствие щелчка на слух. DAPM/GPIO реализация пока остаётся временной тестовой картой; перенос в постоянную ASoC-карту и акустическая проверка остановки фильма ещё нужны.

## Capture и статус микрофона

Отдельный временный Q6ASM DAI `MultiMedia2` позволил параллельно открыть playback `MultiMedia1` и capture `MultiMedia2`; оба ALSA процесса завершились с кодом 0, capture создал полный восьмисекундный WAV. В записи был только слабый шум в канале 0, канал 1 оказался нулевым, ожидаемый 440 Hz сигнал не обнаружен. Поэтому работоспособность физического микрофона, его расположение и акустическая связь с динамиками пока не подтверждены.

Постоянный coordinator сейчас не создаёт этот дополнительный frontend. Перед включением capture в штатную карту нужно определить реальные analog/DMIC входы и их port mapping, затем проверить каналы тестовым сигналом и голосом.

## План завершения ACPI audio

1. **Постоянное питание и attach.** Сверить ACPI `AUDD`/PEP, Windows OEM power tables и WCD938x regulator names; установить реальную связь каждой rail. Заменить временные APCC/GPIO58 helpers штатными regulator/GPIO owners и обеспечить повторяемый `Attached` после холодной загрузки.
2. **Профиль и уровни.** Выбрать одну ALSA card identity, убрать неоднозначность stale state entries, оформить UCM/default route или systemd restore. После cold boot проверить сохранённые HPH/RX gains и mute readback через тот же card ID.
3. **Постоянная ASoC-карта и boot.** Добавить RX и TX backends и отдельный capture frontend в ACPI coordinator/software-node hierarchy; убрать временный runtime rebind `q6asm-dai`. Создать ACPI-specific startup, который запускает ADSP, ждёт APR `q6adm`, загружает карту и восстанавливает профиль. Существующий `tcl-audio.service` ограничен DT-ядром. Проверить повторные start/stop/seek/XRUN recovery и запуск после cold boot.
4. **Постоянный click-free stop.** Перенести уже проверенный временный DAPM/GPIO consumer в штатную ACPI ASoC-карту и связать fade/mute, PA off при открытом PCM, закрытие PCM и освобождение reset/rails. Повторить pause/seek/stop фильма и подтвердить на слух отсутствие треска. Программный порядок сейчас проверен; акустический результат и постоянная интеграция ещё не завершены.
5. **Физический capture.** Сопоставить ADC/DMIC и SoundWire TX ports с OEM/Windows данными, определить установленный микрофон, получить ожидаемый сигнал в обоих каналах, проверить уровни и отсутствие обратной связи.
6. **Полная проверка.** После reboot без предварительного запуска Windows повторить автоматический boot, Wi-Fi/SSH, аудиоподготовку, весь 9:56 фильм, pause/seek/stop и проверить `dmesg` на codec/SoundWire/DSP errors. Зафиксировать, что осталось включено после завершения и что корректно уходит в runtime suspend.
