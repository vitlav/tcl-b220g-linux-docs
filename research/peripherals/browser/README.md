# GNOME Web на TCL B220G

2026-09-09: для проверки браузера в текущем Weston выбрана установка Ubuntu arm64 epiphany-browser 49.2-3ubuntu1 через epm. Firefox в этом репозитории — пакет установки snap. Расчёт apt без Recommends: 59 новых пакетов (включая acl), 50.9 MB загрузки, 213 MB на диске.

Установка: `APT_CONFIG=/var/tmp/tcl-browser-apt.conf epm -y install epiphany-browser acl`, временный конфиг содержит `APT::Install-Recommends "false";`. Лог: `/var/tmp/tcl-browser-install.log`.

Создан пользователь tcl-browser (группы video,render); runtime /run/tcl-browser, mode 0700. ACL: только проход в /run/tcl-weston и rw к wayland-tcl для этого пользователя. Weston остаётся в существующем диагностическом root-сеансе. ACL и runtime относятся к текущей загрузке; автозапуск браузера не настроен.

2026-09-09: GNOME Web 49.2 / WebKitGTK 2.52.6 установлен через epm, свободно552MiB. Первый запуск tcl-browser.service (непривилегированный пользователь, Wayland, Wikipedia) показал окно, но пользователь развернул его и приложение завершилось: Error71 Protocol error dispatching to Wayland display в18:26:54. Weston осталсяactive, новых сообщенийядра нет. По просьбе пользователя повторно запущен tcl-browser-debug.service с WAYLAND_DEBUG=client; лог /var/tmp/tcl-browser-wayland.log. Точная причина ошибки пока не установлена. В журнале также отсутствует Secret Service (сохранение паролей не настроено).

### Firefox установка (2026-09-09, в работе)

По запросу пользователя подключён официальный Mozilla APT arm64: /etc/apt/sources.list.d/mozilla.sources, ключ /etc/apt/keyrings/packages.mozilla.org.asc. Fingerprint35BAA0B33E9EB396F59CA838C0BA5CE6DC6315A3 проверен локально по https://support.mozilla.org/en-US/kb/install-firefox-linux . Pin1000 ограничен firefox и firefox-l10n-*. epm update выполнен. Кандидат155.0.1~build1, firefox + firefox-l10n-ru:78MBdownload/297MBinstalled. ВНИМАНИЕ: `apt-get --assume-no --no-install-recommends install ...`, предназначенный для оценки размера, фактически начал установку; --assume-no не является dry-run. Для последующих расчётов использовать только `apt-get -s`. Задача установки авторизована пользователем, но этот этап фактически прошёл напрямую apt, а не через epm.

GNSS: пока QMI LOC/PDS в qrtr-lookup отсутствуют; загружена qcmpss7180_nm.mbn, MPSSrunning. Возможна имитация NMEA через gpsd для проверки приложений; она не проверяет физический приёмник/антенну и не заменяет диагностику QMI LOC. Симуляция не запускалась.

Firefox155.0.1~build1 arm64 и firefox-l10n-ru установлены. tcl-firefox.service запущен от tcl-browser через dbus-run-session, MOZ_ENABLE_WAYLAND=1, WAYLAND_DISPLAY=/run/tcl-weston/wayland-tcl, URLhttps://www.wikipedia.org. Проверка: unitactive, firefox-binPID10729, ошибок в начальном журнале нет; визуальная проверка разворачивания ещё не выполнена. `epm clean` удалил240MiBкэша пакетов, свободно492MiB. GNOME Web сохранён; автозапуск Firefox не настроен.

### Firefox: звук подтверждён (2026-09-09)

Пользователь: «так уже работает». PulseAudio17 запущен как tcl-browser из /run/tcl-pulse-stage (распаковка уже скачанных подписанных пакетов; обычная установка через epm продолжается медленно на USB). tcl-browser-audio.service active, RuntimeMaxSec850, KillMode=mixed; источник enable-browser-audio-ram.sh, конфигурация browser-pulse.pa. ALSA hw:Test,0: S16LE stereo48kHz, PA GPIO46/47 включён послеRUNNING, sinkvolume50%, HPH/RX0dB. pactl показывает application.name=Firefox, Corked:no, Mute:no, tcl_speakers RUNNING. Firefox подхватил появившийся аудиосервер без перезапуска. Старые OpenCubeb failures относятся к времени до запускаPulse. Taint4096 не изменился.

При управляемом cleanup PA выключается раньше Pulse/PCM; отсутствие щелчка при остановке ещё не подтверждено. Предел PA900сек/codecwatchdog1000сек, общий850сек; это временный тест, не постоянный автозапуск звука. После таймера аудиотракт будет восстановлен в исходное состояние. Сценарий запуска из установленных пакетов тоже подготовлен enable-browser-audio.sh, но ещё не проверен. Не выдавать RAMвариант за завершённую постоянную интеграцию.

QMI1.38.0 установленчерезepm; read-only `qmicli -d qrtr://0 --loc-get-operation-mode` завершился `couldn't create client for the loc service: QMI protocol error (3): Internal`. Это не ответ LOCобошибкеGNSSдвижка — клиент службы не создан.

2026-09-09: epm -y remove epiphany-browser epiphany-browser-data завершился с кодом0, включая триггеры GLib/hicolor. Оба пакета not-installed. Общие зависимости и профиль пользователя не удалялись; Firefox сохранён.
