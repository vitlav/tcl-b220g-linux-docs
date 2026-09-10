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
