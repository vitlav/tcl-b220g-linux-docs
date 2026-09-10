# Qualcomm WLAN firmware services

[Спецификация WLAN](../../../docs/hardware/wifi.md#firmware-и-интерфейсы-управления) описывает роль MPSS, QRTR/QMI, RMTFS и tqftpserv. [Конфигурация Ubuntu](../../../docs/ubuntu.md#firmware-и-запуск-wi-fi) задаёт порядок служб.

[firmware-sha256.txt](firmware-sha256.txt) — контрольные суммы 16 файлов OEM TCL из Windows DriverStore: MPSS, WLAN DSP и варианты board data. Наличие файла в наборе не подтверждает его выбор при запуске. Бинарные firmware в Git не включены.

Сборочные файлы этого каталога относятся к отдельному диагностическому initramfs. Его пользовательский qrtr-ns не является требованием Ubuntu-конфигурации, использующей nameserver ядра.
