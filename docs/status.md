# Проверенное состояние и следующие задачи

Состояние поддержки: 2026-09-15. Таблица ниже сохраняет DT-эталон; отдельные ACPI-результаты явно подписаны. Полное сопоставление DT/ACPI и критерии готовности: [план поддержки](roadmap.md).

| Подсистема | Доказано | Что остаётся |
|---|---|---|
| [USB / ввод](hardware/usb-input.md) | USB rootfs, tethering, UVC; клавиатура и I²C HID | SuperSpeed по портам, role switching, wakeup и жесты |
| [CPU](hardware/cpu.md) | 8 CPU, cpufreq-hw, рабочая Linux DT-загрузка | EL2 не предоставлен firmware: Linux стартует в EL1, /dev/kvm нет |
| [UFS](hardware/storage.md) | Накопитель доступен, архив всех 6 LUN проверен по SHA256 | Реальное восстановление Windows из архива не испытывалось |
| [Wi-Fi](hardware/wifi.md) | ath10k SNOC, 5 ГГц, автоматические Wi-Fi/SSH после загрузки подтверждены | Не считать настройку перенесённой в каждый новый initramfs без проверки |
| [Графика](hardware/display.md) | MSM/DPU + DSI + LT8911EXB handoff, Adreno/Wayland проверены | Ранний KMS проверен; остаются cold-start моста, подсветка и suspend/resume |
| [Видео](hardware/video.md) | H.264 через Venus проверен; software decode — визуальный контроль | Ошибки unrestricted decode, артефакты/остановка mpv; прочие кодеки не испытаны |
| [Звук](hardware/audio.md) | WCD9385 variant=5; слышимый DT- и ACPI-playback, ACPI film audio | Штатное power/profile/PA управление, устранение треска при stop, проверка после cold boot |
| [Камера](hardware/camera.md) | UVC 13d3:784b, живое изображение MJPEG 720p подтверждено пользователем | Измеренные FPS, полный набор режимов, controls и suspend/resume |
| [Микрофон](hardware/audio.md) | ACPI capture PCM работает через временный MultiMedia2 FE | Реальный акустический сигнал и физическая разводка не подтверждены |
| [EC/батарея](hardware/ec.md) | Чтение состояния, подтверждена зарядка | Стандартный драйвер power_supply/lid |
| [SAR AW96105](hardware/sensors.md) | Временный IIO-тест дал меняющиеся raw-значения | Питание vcc, постоянный DT/драйвер, назначение электродов |
| [GNSS/LTE](hardware/modem.md) | Windows-устройства и OEM firmware найдены | Linux LOC/GPS fix, правильная прошивка/резерв памяти |
| ACPI | Ubuntu 26.04.1 LTS, USB/rootfs, Wi-Fi/SSH, MSM/Wayland/Adreno, слышимый звук | Воспроизводимость полного комплекта, питание и повторные циклы, UFS/видео/capture — отдельная квалификация |

Текущая ACPI-система использует `7.2.4-tcl-acpi-display1+`. Её функциональные результаты не означают, что весь новый исходный комплект уже пересобран и повторно проверен: [серия и манифест](../patches/kernel/acpi/linux-7.2.4/README.md).

[Подробные критерии завершения поддержки](validation-gaps.md) отделяют текущие неисправности от ещё не проверенных возможностей.
