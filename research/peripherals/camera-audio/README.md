# Камера и звук TCL, 2026-09-09

Камера: пользователь подтвердил живое изображение на экране. UVC13d3:784b, MJPEG1280x720@30. Пакеты через epm: v4l-utils1.32.0, FFmpeg8.0.1, alsa-utils1.2.15.2 (Ubuntu). Установка успешна. Временный unit tcl-camera-preview.service, без автозапуска и без записи/звука.

```sh
XDG_RUNTIME_DIR=/run/tcl-weston WAYLAND_DISPLAY=wayland-tcl SDL_VIDEODRIVER=wayland ffplay -hide_banner -loglevel warning -nostats -window_title TCL-Camera -an -f v4l2 -input_format mjpeg -video_size 1280x720 -framerate 30 -i /dev/video0
```

Закрытие окна: q/Escape. Управление unit на текущей Ubuntu (serv отсутствует): systemctl stop tcl-camera-preview. ffplay сообщает неускоренную конверсию yuv422p→rgba и предупреждение MJPEG APP fields, но изображение пользователь видит. Файл с изображением не создавался.

Звук: /proc/asound/cards, aplay -l, arecord -l не видят звуковых карт. В DT lpass@62d87000 и smp2p-lpass disabled, звуковая карта не описана. В kernel имеются модули LPASS_SC7180, SND_SOC_SC7180, ряд codec drivers. Наличие модулей недостаточно: нужно установить реальный codec/усилитель/маршруты/питание из OEM/ACPI и подготовить корректный machine DT. Поиск по именам сохранённых OEM inf пока не дал идентификации codec. Не подставлять codec от похожего ноутбука. Звук/микрофон функционально НЕ работают и не проверены воспроизведением/записью. Следующий шаг — разбор OEM описаний аппаратуры, затем probe ALSA, playback на малой громкости; запись микрофона согласовать. Перезагрузки не было.

## Снимки экрана для самопроверки

2026-09-09: подтверждён полный захват Weston1920x1080 без перезапуска. Прямой weston-screenshooter из SSH возвращает unauthorized; это политика Weston, не отсутствие поддержки. Штатный путь Super+S запускает разрешённого клиента. Из SSH отправлена ровно последовательность KEY_LEFTMETA125 down, KEY_S31 down/up, KEY_LEFTMETA up через /dev/input/by-path/platform-890000.i2c-event-kbd (struct input_event llHHi, SYN_REPORT после каждого). Физическую клавиатуру перед повторением сверять по имени/пути; не посылать случайные сочетания.

Weston working directory по умолчанию /, файл создан /wayland-screenshot-2026-09-09_02-31-53.png, доступ затем600. Приватная локальная копия /tmp/tcl-graphics-private/camera-preview-selfcheck.png. Просмотрена инструментом view_image: рабочий стол1920x1080, окноTCL-Camera с реальным изображением; кадр тёмный/шумный, изображение не пустое. Снимок камеры не приложен к публичной баге/FTP. При работе с camera preview дальнейшие снимки также приватные по умолчанию.

2026-09-09 Venus1 installed, NOT yet hardware tested: defaulttcl-venus1 uses same6.18.34-tcl-ice1 Image/initrd, newDT enablingVIDEOCC/Venus plus5MiB no-map0x85b00000. OEM qcpilext confirms poolbase0x85b00000 and Venus reservation5MiB, exact runtime placement still inference. DT audit250nodes PASS. Firmware VIDEO.VE.5.4-00064-PROD-1 upstream48d27ba...,sha256db2a2efe...; old EFI backupBOOTAA64-before-venus1.bak. Driver autoload blacklisted; manual probe after SSH and log collector. Teststream300frames1080p30 ready; compare software/hardware required. Package/install passed. Reboot planned.
Audio correction: matchingTCL SUBSYS27822202 selects AUDD_CLS_7180. Realtek settings in qcauddev_ext are QSP-only forQSIP7180, not evidence of TCL codec. CLS analog speakerPA usesGPIOUID1/2, ExtSpeaker/HeadsetCodec0; binaryPDB pathES_CODEC/Emdoor_QL328 hints customization but proves no chip model. Audio DT not changed.

Подробная повторная проверка звука и черновик ADSP: adsp1/README.md. ALSA по-прежнему без карт. OEM ADSP40MiB подтверждён INF+ELF, GPIO46/47 связаны с PA; точный codec неизвестен. DT draft собран, статический аудит271узла PASS, не установлен, не загружен.

## ADSP1 hardware result: PASS transport, sound still absent

Boot3f72d7f5-839f-4deb-9d1a-afd403ce9cce. Wi-Fi/SSH returned, ADSP remoteproc1 offline until firmware supplied. OEM firmware hash24d665684966cab76eb0ca58536cd96f9e978267f52ffef1dd300d8e3dfe2860 verified then manual start succeeded. Modem remains running. Q6CORE/AFE/ASM/ADM drivers bound on aprbus; AFE DAIs/clocks/routing registered. One-shot module query confirmed API versions3=5,4=7,7=2,8=4 (ret0), then unloaded. readiness API1 alone not proof due unsupported-command fallback. Concrete versions provide live command/response evidence.

ALSA still no soundcard. Q6ASM frontend dai probe failed No dais found in DT (-22), expected absent board PCM child definitions. Exact physical codec/routes unresolved. Generic AFE DAI list is not board hardware enumeration. FASTRPC no reserved DMA memory remains separate issue. Logger initially failed before remoteproc existed; fixed empty-glob handling before firmware start. Correct APR sysfs bus is aprbus. Firmware now exists on root, so next boot may autostart once available; not yet reboot-tested.

Runtime archive /tmp/tcl-adsp1-runtime.tar.gz. Module sources/binary in query/. One diagnostic reboot, no amplifier GPIO or internal UFS writes. Next prepare PCM/machine/codec mapping; do not claim audible sound.
