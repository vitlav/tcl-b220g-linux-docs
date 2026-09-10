# Материалы по подсистемам

| Подсистема | Что искать | Первичные материалы |
|---|---|---|
| CPU / частота / криптография | cpufreq, аппаратные crypto-инструкции, измерения | [CPU freq](../research/peripherals/cpufreq-probe/README.md), [CPU crypto](../research/peripherals/cpu-crypto/README.md) |
| Wi-Fi | ath10k, OEM firmware, RMTFS, DHCP, постоянный доступ | [Спецификация Wi-Fi](hardware/wifi.md), [службы Ubuntu](ubuntu.md#firmware-и-запуск-wi-fi) |
| UFS / ICE | PHY, LUN, inline encryption, проверка чтения | [Спецификация UFS / ICE](hardware/storage.md), [охват резервной копии](../research/peripherals/windows-backup/README.md) |
| Экран / GPU | DPU, DSI, LT8911EXB handoff, Mesa/Weston, тесты | [Спецификация дисплея / GPU](hardware/display.md) |
| Видео | Venus отдельно от GPU-вывода; CPU сравнения | [Decode](../research/peripherals/video-decode/README.md), [mpv CPU](../research/peripherals/video-decode/venus1/mpv-cpu/README.md), [A/B playback](../research/peripherals/video-decode/venus1/playback-cpu/README.md) |
| Звук / камера | WCD9385, ADSP, SoundWire, питание, повторный запуск | [Спецификация звука](hardware/audio.md), [материалы камеры](../research/peripherals/camera-audio/README.md) |
| EC / батарея | ACPI I²C-протокол, проверенные поля, зарядка | [Спецификация EC](hardware/ec.md) |
| SAR / крышка | AW96105, IIO, EC lid | [Спецификация датчиков](hardware/sensors.md) |
| GNSS / LTE | NM и полный MPSS, резерв RAM, LOC/PDS | [GNSS](../research/peripherals/gnss/README.md) |
| Браузер / звук | Firefox, Wayland, PulseAudio | [Браузеры](../research/peripherals/browser/README.md) |

Материалы Windows используются для сопоставления адресов, схем питания и firmware. Драйверы Windows и прошивки не включены в Git; они перечислены в архивном манифесте. Отсутствие устройства в текущем DT не отменяет данные Windows: в частности, сохранены сведения Snapdragon X15 LTE и Qualcomm Location, но Linux GNSS fix и LTE-сеанс не подтверждены.
