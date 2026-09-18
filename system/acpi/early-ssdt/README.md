# Ранняя SSDT, заводской WLAN MAC и board properties

OEM DSDT не передаёт Linux заводской адрес WCN3990. Без свойства firmware
`ath10k` сообщает `invalid MAC address; choosing random`, поэтому адрес и DHCP
lease меняются после загрузки.

Заводские данные находятся в GPT-разделе `DPP`, в Qualcomm RWFS-файле
`WLAN.PROVISION`. Формат каталога и размещение файла совпадают с открытой
реализацией `qcom_dpp_read_file()` из
[TravMurav/dtbloader](https://github.com/TravMurav/dtbloader/blob/main/src/qcom.c).
Файл WLAN имеет размер 27 байт: после трёх служебных байтов следуют шесть
байтов MAC. На проверенном экземпляре это `c8:2a:f1:7f:78:60`; тот же адрес
показывает Windows для физического Qualcomm Wi-Fi adapter.

`tools/qcom_dpp.py` читает DPP без записи, проверяет обе сигнатуры RWFS,
границы каталога и файла, точный размер/заголовок WLAN-записи и допустимость
глобального unicast MAC. Затем он подставляет адрес в `mac-address` существующей
QWLN `_DSD`, компилирует AML и создаёт несжатый early CPIO:

```sh
python3 tools/qcom_dpp.py /dev/disk/by-partlabel/DPP \
  --template system/acpi/early-ssdt/tcl-input.asl.in \
  --output-dir /var/tmp/tcl-acpi
```

Для сборки нужен `iasl`. Полученный `tcl-input-early.cpio` должен предшествовать
обычному initramfs в команде загрузчика. Ядро загружает
`kernel/firmware/acpi/tcl-input.aml` до перечисления устройств. Таблица также
сохраняет уже необходимые `_CCA`, GPIO, I2C5 и IC11 свойства; загружать рядом
старую таблицу, которая повторно определяет эти `_DSD`, нельзя.

Текущий шаблон дополнительно описывает проверенные по рабочему DT ресурсы
шины EC: I2C3 `0x888000`, 400 kHz и SC7180 pinmux GPIO15/16
`qup02_i2c`. Точный GPIO whitelist включает GPIO31 для OEM ACPI battery event
`_E1F` и GPIO117 для display HPD `_E75`. Для этого варианта требуется ядро с
соответствующей TCL ACPI-адаптацией GENI/pinctrl; старое ядро отклонит
расширенный whitelist. Аппаратная проверка BAT0/ADP1 после этой интеграции ещё
не выполнена.

Mainline `ath10k` вызывает `device_get_mac_address()` до регистрации mac80211,
поэтому `mac-address` становится permanent address без сетевой службы и без
подмены уже созданного `wlan0`. После аппаратной проверки должны одновременно
выполняться условия:

1. предупреждение `invalid MAC address; choosing random` отсутствует;
2. `/sys/class/net/wlan0/address` равно адресу из DPP;
3. `iw phy phy0 info` и журнал ассоциации показывают тот же адрес;
4. сеть, 5 ГГц, DHCP и SSH запускаются штатно после холодной загрузки.

Downstream Qualcomm имеет отдельное QMI-сообщение WLFW MAC `0x0033` и посылает
его перед probe WLAN-драйвера. Mainline ath10k описывает формат этого сообщения,
но не вызывает его. Сначала проверяется стандартный firmware-property/WMI путь;
QMI следует добавлять отдельным патчем только при подтверждённой необходимости.
