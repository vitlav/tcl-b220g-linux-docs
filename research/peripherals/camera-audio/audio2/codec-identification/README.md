## 2026-09-09 — WCD9385 идентифицирован прямым чтением TX

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52, uptime23858s.
Модуль tcl_wcd_id читает sdw_read_no_pm на TX slave только при Attached/dev_num1/driver,
во время удержания контроллеров, macro clocks, питания и снятого reset тестовым сценарием.
Regmap cache не использован, запись регистров кодека не выполнялась.

| Адрес | Значение | Назначение |
| --- | --- | --- |
| 3401 | 00 | CHIP_ID0 |
| 3402 | 00 | CHIP_ID1 |
| 3403 | 0d | CHIP_ID2 |
| 3404 | 01 | CHIP_ID3 |
| 34b0 | 0a | DIGITAL_EFUSE_REG_0 |

В wcd938x.c variant = field EFUSE bits4:1. (0x0a & 0x1e)>>1 =5,
CHIPID_WCD9385=5, CHIPID_WCD9380=0. Таким образом установлен именно WCD9385,
и подтверждён прямой транспорт чтения регистров черезTX, не только enumeration.
После теста оба RX/TX Attached в трёх выборках; cleanup GPIOcfg1/io0,
питаниеdisable_complete1, stock soundwire_qcom restored, Westonactive,taint4096.
ALSA cards отсутствуют, PCM не запускался, Bugzilla не обновлялась.

### Следующая интеграция: ресурсы общего драйвера

wcd938x_populate_dt_data требует GPIO reset и supplies vdd-rxtx/vdd-io/vdd-buck/vdd-mic-bias.
Atoll reference связывает первые два с L10A1.8V, buck сL15A1.8V, mic сBOB3.3V.
ИСПРАВЛЕНО: WiFi использует L10C3304mV, а не L10A; см. power-topology.md.
Не менять L10C WiFi; соответствие L10A кодеку требует отдельной проверки. TCL APCC явно
содержит LDO15_A1.8V и BOB_C3.3V, компонентов3/4 ресурсы пустые; питания IO/RXTX
требуют дальнейшего доказательства. Успешные чтения показывают, что необходимые
питания во время теста доступны, но не устанавливают их физическую разводку.

GPIO58 проверен, но reserved58..62 в boot DT. gpiochip_apply_reserved_ranges
формирует gpiodev->valid_mask при регистрации gpiochip; простое изменение DT
runtime overlay не восстановит валидность58. Не перепривязывать системный TLMM
и не менять внутреннюю valid_mask. Для штатного gpiod пути при следующей
согласованной загрузке менять reservation только на59..62 (<59 4>).
Это не требование немедленно перезагрузиться: сначала подготовить все ресурсы.
Не выдавать временные power/reset модули за постоянное управление питанием.


## 2026-09-09 — исправление идентичности LDO10 и план ресурсов

Предыдущий вывод «LDO10_A используется WiFi при3304mV» ОШИБОЧЕН:
regulator_summary показывает имя ldo10 без банка. Проверка sysfs regulator.7:
parent18200000.rsc:regulators-1, of_node regulators-1/ldo10, pmic-id=c.
Это LDO10_C, ресурсldoc10=41900. LDO10_A — другой PMIC, ldoa10=41d00;
он отсутствует среди зарегистрированных Linux regulator providers.

| Ресурс | Доказательства | Вывод |
| --- | --- | --- |
| LDO10_C | live sysfs regulators-1;3304000uV; WiFiconsumer | Не относится к предполагаемому питанию кодека |
| LDO10_A | CMD DB41d00; DSDTG6MD EXIT содержит1800000uV/enable1/mode7 | TCL firmware содержит запрос1.8V; это GPU/display table, не прямое доказательство разводки codec |
| LDO15_A | APCC AUDDcomp2; CMD DB42200; успешные тесты1800mV | Подтверждённое управляемое питание аудио |
| BOB_C | APCC AUDDcomp1; CMD DB40400; успешные тесты3300mV/mode6 | Подтверждённое управляемое питание аудио |
| GPIO58 | прямые чтения/сброс успешно; TLMM reservation58..62 | Для штатного gpiod в boot DT исключить только58 изreservation |

Atoll LDO10_A для vdd-rxtx/vdd-io теперь остаётся рабочей гипотезой, а не
опровергнутым вариантом. Связь G6MD с текущим runtime PEP и физическая разводка
до кодека отдельно не проверены. Нельзя выдавать наличие таблицы за выполненный vote.

Важные ограничения интеграции:
* rpmh_regulator_probe перечисляет child nodes один раз. Добавление ldo15/bob
  в уже bound PMIC node runtimeoverlay само по себе не создаёт providers.
* Не перепривязывать целиком PMIC banks: они обслуживают текущие WiFi/USB/display.
* BOB pmic5_bob Linux32mV selector не представляет3300mV точно; ранее проверенный
  OEM request3300mV прошёл напрямуюRPMh. Не менять напряжение на3320mV молча.
* gpiochip valid_mask вычисляется на регистрации; runtime DT изменениеreservation
  само по себе не даёт штатный доступ GPIO58.
* Стандартный WCD driver запрашивает4supplies. Не подменять отсутствующие providers
  dummy regulators с утверждением, что получено постоянное управление питанием.

Подготовлен power-topology.dtsi.txt: НЕ готов к загрузке, BOB и codec намеренно
неактивны, LDO10_A явно помечен как гипотеза. До нового загрузочного варианта
нужно решить точное представлениеBOB3300mV и согласованное управление GPIO58;
проверить полный DT, сохранить известный рабочий GRUB пункт. Перезагрузки не было.
