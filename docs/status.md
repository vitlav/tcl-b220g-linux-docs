# Проверенное состояние и следующие задачи

Состояние поддержки: 2026-09-10. Аппаратные результаты относятся к Ubuntu с DT и модифицированным ядром 6.18.34; ACPI рассматривается отдельно.

| Подсистема | Доказано | Что остаётся |
|---|---|---|
| [USB / ввод](hardware/usb-input.md) | USB rootfs, tethering, UVC; клавиатура и I²C HID | SuperSpeed по портам, role switching, wakeup и жесты |
| [CPU](hardware/cpu.md) | 8 CPU, cpufreq-hw, рабочая Linux DT-загрузка | EL2 не предоставлен firmware: Linux стартует в EL1, /dev/kvm нет |
| [UFS](hardware/storage.md) | Накопитель доступен, архив всех 6 LUN проверен по SHA256 | Реальное восстановление Windows из архива не испытывалось |
| [Wi-Fi](hardware/wifi.md) | ath10k SNOC, 5 ГГц, автоматические Wi-Fi/SSH после загрузки подтверждены | Не считать настройку перенесённой в каждый новый initramfs без проверки |
| [Графика](hardware/display.md) | MSM/DPU + DSI + LT8911EXB handoff, Adreno/Wayland проверены | Ранний KMS проверен; остаются cold-start моста, подсветка и suspend/resume |
| [Видео](hardware/video.md) | H.264 через Venus проверен; software decode — визуальный контроль | Ошибки unrestricted decode, артефакты/остановка mpv; прочие кодеки не испытаны |
| [Звук](hardware/audio.md) | WCD9385 variant=5, RX/TX SoundWire, слышимый звук | Автоматическая подготовка после загрузки, щелчки и полное покрытие повторных запусков |
| [Камера](hardware/camera.md) | UVC 13d3:784b, живое изображение MJPEG 720p подтверждено пользователем | Измеренные FPS, полный набор режимов, controls и suspend/resume |
| [Микрофон](hardware/audio.md) | Части аудиотракта определены | Полноценная запись и физическая разводка не подтверждены |
| [EC/батарея](hardware/ec.md) | Чтение состояния, подтверждена зарядка | Стандартный драйвер power_supply/lid |
| [SAR AW96105](hardware/sensors.md) | Временный IIO-тест дал меняющиеся raw-значения | Питание vcc, постоянный DT/драйвер, назначение электродов |
| [GNSS/LTE](hardware/modem.md) | Windows-устройства и OEM firmware найдены | Linux LOC/GPS fix, правильная прошивка/резерв памяти |
| ACPI | Диагностический initramfs; SCM, оба SMMU и USB host; два снимка логов с проверкой SHA256 | Встроенные Wi-Fi/ввод, нативный дисплей, полноценная Ubuntu и управление питанием |

[USB через ACPI](hardware/usb-input.md) проверен на ядре `6.18.34-tcl-acpi6`. Журналы подтверждают запуск модема и появление QRTR службы WLAN firmware, но Wi-Fi-интерфейс не создан: назначение прав MSA завершается ошибкой -22. Автоматический Wi-Fi/SSH из DT не считается перенесённым в ACPI. [Проверка USB](../research/acpi-boot/acpi6-usb/README.md).

[Подробные критерии завершения поддержки](validation-gaps.md) отделяют текущие неисправности от ещё не проверенных возможностей.
