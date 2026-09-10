
## 2026-09-09 — команда просмотра батареи в Ubuntu audio2

На текущей загрузке c781c4d4-cd36-4a07-b734-40b059ac3e52 каталог /sys/class/power_supply пуст, /proc/acpi/battery отсутствует. Доступна установленная команда `sudo /usr/local/sbin/battery-status`; обновление `sudo watch -n 30 /usr/local/sbin/battery-status`. Она проверяет шину 888000.i2c/i2c-2 и отсутствие драйвера 2-0007, затем читает 32-байтный блок EC через i2ctransfer с таймаутом. Процент — percent_field.

Фактическое чтение uptime12507.75: ac0, discharging, percent_field63, remaining_mAh3402, voltage_mV7763, current_raw569. Это снимок на момент проверки, а не постоянное значение. Стандартный power_supply-драйвер остаётся TODO; пользователь просил команду, новые интерфейсы не добавлялись.
