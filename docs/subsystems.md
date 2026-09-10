# Спецификации подсистем

| Подсистема | Содержание | Спецификация |
|---|---|---|
| Инфраструктура SoC | IRQ, GPIO, clocks, питание и DMA | [Спецификация платформы](hardware/platform.md) |
| USB / ввод | DWC3, PHY, I²C HID, клавиатура и тачпад | [Спецификация USB и ввода](hardware/usb-input.md) |
| CPU / частота / криптография | cpufreq, аппаратные crypto-инструкции, измерения | [Спецификация CPU](hardware/cpu.md) |
| Wi-Fi | ath10k, OEM firmware, RMTFS, DHCP, постоянный доступ | [Спецификация Wi-Fi](hardware/wifi.md), [службы Ubuntu](ubuntu.md#firmware-и-запуск-wi-fi) |
| UFS / ICE | PHY, LUN, inline encryption, проверка чтения | [Спецификация UFS / ICE](hardware/storage.md), [охват резервной копии](../research/peripherals/windows-backup/README.md) |
| Экран / GPU | DPU, DSI, LT8911EXB handoff, Mesa/Weston, тесты | [Спецификация дисплея / GPU](hardware/display.md) |
| Видео | Venus отдельно от GPU-вывода; CPU сравнения | [Спецификация Venus](hardware/video.md) |
| Звук / камера | WCD9385, ADSP, SoundWire, питание, повторный запуск | [Спецификация звука](hardware/audio.md), [камера UVC](hardware/camera.md) |
| EC / батарея | ACPI I²C-протокол, проверенные поля, зарядка | [Спецификация EC](hardware/ec.md) |
| SAR / крышка | AW96105, IIO, EC lid | [Спецификация датчиков](hardware/sensors.md) |
| GNSS / LTE | NM и полный MPSS, резерв RAM, LOC/PDS | [Спецификация MPSS / GNSS](hardware/modem.md) |
| Браузер / звук | Firefox, Wayland, PulseAudio | [Браузеры](../research/peripherals/browser/README.md) |

Материалы Windows используются для сопоставления адресов, схем питания и firmware. Драйверы Windows и прошивки не включены в Git; они перечислены в архивном манифесте. Отсутствие устройства в текущем DT не отменяет данные Windows: в частности, сохранены сведения Snapdragon X15 LTE и Qualcomm Location, но Linux GNSS fix и LTE-сеанс не подтверждены.
