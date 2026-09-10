# TCL B220G: куда сообщить результаты

Проверено 2026-09-08 через `gh`. Сообщения на GitHub не отправлены: пользователь просил найти и запомнить площадки для будущего отчёта.

| Приоритет | Обсуждение / участники | Что сообщить |
|---|---|---|
| 1 | [MinimumLaw/TCL-B220G-Linux #1](https://github.com/MinimumLaw/TCL-B220G-Linux/issues/1), MinimumLaw, AliceCyber, fixinit75 | Отчёт именно по B220G: Ubuntu, рабочий DTS, UFS, Wi-Fi, cpufreq; ссылка на воспроизводимый комплект и ограничения графики. |
| 2 | [velvet-os/imagebuilder #136](https://github.com/velvet-os/imagebuilder/issues/136), AliceCyber, TravMurav, hexdump0815 | Дополнение статуса Snapdragon 7c WoA для TCL, изменения относительно раннего DTS AliceCyber и сведения для интеграции в образ. Старый адрес hexdump0815/imagebuilder перенаправляется сюда. |
| 3 | [hexdump0815/linux-mainline-qcom-kernel](https://github.com/hexdump0815/linux-mainline-qcom-kernel) | Подготовленные изменения DTS/конфигурации для используемой ветки ядра; отдельной TCL issue поиском не обнаружено. Сначала согласовать направление в #136. |
| Дополнительно | [rainbyte/firmware-tcl-book14-go](https://github.com/rainbyte/firmware-tcl-book14-go) | Сопоставление прошивок и ревизий. README говорит, что бинарники извлечены из CX 27000W, предположительно ребрендинга TCL. Совместимость с нашим B220G не проверена. |

## Точные точки обсуждения

- [AliceCyber, 2023-05-21: DTS и список работающих устройств](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1556219317): EFIFB, клавиатура, USB, Bluetooth; не работают touchpad, GPU/мост и UFS; Wi-Fi/WWAN не исследованы. Это исторический статус, не описание нашего экземпляра.
- [AliceCyber, 2023-05-18: LT8911EXB](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1553782007): идентификация по UART и ссылка на внешний драйвер. Наш разбор Windows/UEFI уточняет последовательности и обмен данными, но не является первым обнаружением модели мостика.
- [TravMurav: ограничения внешнего драйвера и использование efifb](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1554131693): драйвер не интегрирован в DRM; предложено начинать с EFI framebuffer. Также приглашение в `#aarch64-laptops` на OFTC.
- [AliceCyber: сбой UFS и тактирования](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1557691230), [ответ TravMurav](https://github.com/velvet-os/imagebuilder/issues/136#issuecomment-1558529478). Есть журнал ранних экспериментов, полезный для сравнения с нашим рабочим UFS.
- [MinimumLaw, 2025-05-12: две нерешённые задачи](https://github.com/MinimumLaw/TCL-B220G-Linux/issues/1#issuecomment-2870881676): мост LT8911EXB и UFS PHY/clock. Наш результат по UFS прямо отвечает на вторую задачу; консоль через efifb не означает готового DRM-драйвера.
- [fixinit75: зависание после systemd-udevd](https://github.com/MinimumLaw/TCL-B220G-Linux/issues/1#issuecomment-2851310656): симптом для сопоставления с нашими ранними зависаниями. Причину именно чужого зависания не установили.

В #1 упоминался `foric27/kernel_tcl_b220g` и со слов автора — работа периферии, но опубликованного DTS в обсуждении не было. На 2026-09-08 `gh repo view` возвращает Could not resolve to a Repository; удаление или приватность не установлены. Не считать подтверждённым воспроизводимым решением.

## Содержание будущего отчёта

1. Модель/ревизия, Ubuntu userspace и точная версия ядра `6.18.34-stb-qc7+`: это сборка Qualcomm community, **не Ubuntu linux-generic**.
2. Рабочий DTS и изменения: сохранение необходимых GPIO/reserved-memory, UFS/ICE, Wi-Fi firmware/RMTFS, cpufreq. Приложить точные файлы и команды из основного журнала.
3. Подтверждённые результаты: SSH через встроенный Wi-Fi, в том числе 5 ГГц; чтение шести UFS LUN; консоль EFI framebuffer. Отдельно указать, какие настройки автозапуска ещё не проверены перезагрузкой.
4. Графика: LT8911EXB, GPIO reset/enable, EDID и взаимодействие DisplayDxe/Windows; пока без заявления о работающем DRM/GPU.
5. Ссылка на [Etersoft #19084](https://bugs.etersoft.ru/show_bug.cgi?id=19084) и воспроизводимые материалы. Не публиковать приватный UFS-архив, ключи, Wi-Fi credentials и персональные данные. Право на распространение OEM-бинарников отдельно не установлено.

Основной журнал: [tcl-b220g-linux.md](../history/lab-notebook.md).
