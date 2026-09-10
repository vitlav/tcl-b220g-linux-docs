

## 2026-09-09 — первая регистрация ALSA-карты WCD9385

Без перезагрузки, boot `d82e1f9e-de49-4a4d-9c36-edc5c1266009`, uptime 1398 с.
Собран диагностический модуль `tcl_audio_card.ko` против 6.18.34-tcl-audio2,
ARCH=arm64, CROSS_COMPILE=aarch64-linux-gnu-, W=1: сборка без предупреждений.
Он использует callbacks из `sound/soc/qcom/sm8250.c`, но разрешает DAI по
точным существующим DT-путям, без добавления phandle к живым overlay-узлам.
Это временная карта для исследования, не окончательный DT или штатная поддержка TCL.

| Звено | Разрешённый DAI |
|---|---|
| Q6ASM service@7/dais, id 0 | MultiMedia1 |
| Q6AFE service@4/dais, id 113 | RX_CODEC_DMA_RX_0 |
| tcl-wcd9385, id 0 | wcd938x-sdw-rx |
| SoundWire 62610000, id 0 | SDW Pin0 |
| RX macro 62600000, id 0 | rx_macro_rx1 |
| Platform | Q6 routing service@8/routing |

`/proc/asound/cards`: card 0 `TCL WCD9385 Test`, driver `tcl-audio-test`.
`/proc/asound/pcm`: `00-00: MultiMedia1 (*) : playback 1 : capture 1`.
Capture виден из-за двунаправленного FE; TX backend не добавлен и запись
не подтверждена. Регистрация компонента WCD теперь дошла до codec probe,
появились регуляторы WCD9385 и RX macro; полный `amixer contents` сохранён.
Первый тест PCM не запускал. Были DAPM unknown pin Headphone/Mic Jack:
в следующей версии добавлены эти widgets и RX→WCD routes из sm8250-mtp.dts.
Точная разводка внешнего усилителя динамиков пока не установлена.

После снятия карты, кодека и временных ресурсов восстановлен stock SoundWire;
Weston active, taint 4096, SSH работает. Журналы и обе версии исходника:
`audio2/card-live/`. Bugzilla пока не обновлялась — слышимый звук не проверен.


### PCM, тихий тон и проверка выходного тракта

В том же boot PCM-тест тишиной завершился с exit 0: 48 кГц, S16_LE, stereo,
состояние RUNNING, hw_ptr=43680 через ~1 с, period=6240, buffer=24960.
Первый тест тишины не включал отдельные SoundWire HPHL/HPHR/CLSH switches,
поэтому не считать его доказательством передачи полезных данных до аналогового выхода.

Затем включены порты HPHL/HPHR/CLSH и проигран тон 440 Гц: четыре импульса
за 6 с с плавными краями 40 мс. Файл -20 dBFS peak, RX digital -6 dB,
HPH analog -12 dB (ALSA value 12). Aplay завершился с exit 0, PCM RUNNING.
Пользователь явно сообщил: «звука не слышал». Это не успешная проверка слышимого звука.

Отдельная запись DAPM на потоке тишины с включёнными HPH/CLSH портами:
Q6 RX mixer On; RX_MCLK, RX AIF, RX INT0/1, HPHL_OUT/HPHR_OUT On;
WCD IN1_HPHL/IN2_HPHR, RDAC1/2, HPHL/HPHR PGA и выходы HPHL/HPHR On.
Это программное состояние DAPM, не измерение аналогового сигнала.
GPIO46/47 при этом output low, func0, 2mA, no pull, без consumer.

OEM INF AUDD_CLS_7180: speaker analog PA GroupID_DeviceID=0x45,
GPIOUID1/2 (= ACPI GPIO46/47), STAGE=2 «config after enabling device»;
INITIALVALUE=0. Усилитель в Linux ещё не моделируется. Подготовлен отдельный
5-секундный эксперимент: запросить только 46/47 через GPIO API, проверить
исходные output low, затем high после PCM RUNNING и вернуть low по таймеру
и при выгрузке. Полярность high=enable — проверяемая гипотеза, не установленный
факт из INF. GPIO58..62 не затрагиваются этим модулем. Перезагрузка не нужна.

Duplicate symbol в этих тестах — отказ загрузки штатного SDW codec поверх
уже загруженного исправленного; это не одновременная работа двух драйверов.
Исправленные модули не установлены вместо production modules. Все результаты
и исходники лежат в `audio2/card-live/`; публикация в Bugzilla пока отложена.


### Подтверждён слышимый звук; запущен фильм

Пользователь после теста с GPIO46/47: «Я слышал звук!!!».
В отличие от предыдущего неслышимого тона, добавлено только включение PA
после PCM RUNNING. Оба GPIO переведены high в uptime 3388.2146 с, возвращены
low таймером в 3393.252 с. Это подтверждённый слышимый тест через данный
тракт, но не независимая проверка каждого динамика/канала.
Карточка/кодек сняты, stock SoundWire восстановлен, taint 4096, Weston active.

По просьбе пользователя запущен полный Big Buck Bunny (9:56) с начала:
`/run/initramfs/usb/tcl-venus1/big_buck_bunny_1080p_h264.mov`.
Unit `tcl-movie-audio-v2.service`, скрипт `/var/tmp/tcl-audio2/play-movie.sh`.
mpv fullscreen gpu-next/OpenGL/Wayland, hwdec=no (прежний визуально проверенный
вариант), ALSA hw:Test,0, 48000 Hz s16 stereo, volume=15, HPH -12 dB,
RX digital -6 dB. В uptime ~3590 с PCM RUNNING; счётчик фильма прошёл 27 с,
A-V около 0.000 с. Усилитель включён. Полный просмотр ещё не завершён.
Первый запуск не состоялся из-за недопустимого `--volume-max=30`; исправлено
на 100, фактическая громкость осталась 15. Не копировать прежний ошибочный
параметр из старого bbb-sound-test.service.

PA module теперь принимает hold_seconds (default 5, max 1200); фильм использует
900. Codec watchdog default 30, max 1800; фильм использует 1000. Unit ограничен
850 с, после mpv EXIT скрипт снимает PA, карту, кодек и временные ресурсы,
возвращает stock SoundWire и auto PM. Во время фильма ничего не выгружать.
Новый фильм ещё не оценён пользователем; звук подтверждён именно коротким тоном.

TODO постоянной реализации: DT карта и phandles, GPIO46/47 как DAPM PA supply
с корректным sequencing, проверка stereo и уровней, runtime PM и IRQ cleanup,
убрать диагностический IO regulator и проверить реальное питание, автозагрузка
после ребута. Текущая работа не сохраняется автоматически после перезагрузки.
