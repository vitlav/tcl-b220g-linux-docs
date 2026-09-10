## 2026-09-09 — BOB3300 через regulator framework проверен

Использован изолированный OOT вариант qcom-rpmh-regulator.c из6.18.34-tcl-audio2.
Match только tcl,book14-bob-test, имяdriver tcl-bob-test, только resource bobc1.
Проверяет machine tcl,book14 и CMD DBbobc1=40400. Таблица напряжения — единственная
точка3300000uV, не общий диапазон с неподтверждённым шагом. Остальные PMIC
драйверы не заменялись. Это диагностический provider, не готовый upstreampatch.

Overlay8 добавил отдельный bank tcl-audio-bob-test под18200000.rsc, pmic-id=c,
и consumer. Первоначальный consumerprobe вернул-EPERM наset_mode: DT не разрешал
смену режима. До enable не дошёл. Overlay9 добавил allowed-modes2/initial-mode2
(AUTO вDT => REGULATOR_MODE_NORMAL => hardware6). Перепривязан только тестовый
bank; существующие regulators-0/-1 не перепривязывались.

| Uptime | Операция | Результат |
| --- | --- | --- |
|24402.525| RPMh40408=6 |ret0|
|24402.526| RPMh40400=3300 |ret0|
|24402.526| RPMh40404=1 |ret0|
|24402.526| regulator_get_voltage / get_mode |3300000 / NORMAL(2)|
|24407.684| автоматический regulator_disable, RPMh40404=0 |ret0, enabled0|

Таким образом обычный consumer API работает с точнымOEM3300mV, включая
refcount/enable/disable; проблема представляла собой Linux selector table,
а не отказRPMh. get_voltage показывает запрошенное/кэшированное значение,
не измеренное физическое напряжение. Данные подтверждают только проверенную
точку3300mV; не публиковать обобщение о всём диапазоне PM6150L.

После теста consumer и provider unbound, tcl_bob_provider выгружен.
Overlay8/9 и модули tcl_bob_test/tcl_bob_mode безexit остаются доreboot,
но потребителя и provider driver binding нет. Не повторять insmod этих
overlayмодулей; для нового теста использовать существующие DT nodes и явную
привязку после проверки конфликта владельцев. Не смешивать rawRPMh powertest
с активным regulator consumer. Подтверждениеcleanup вresult.txt.
Westonactive,taint4096, тот жеbootc781c4d4-cd36-4a07-b734-40b059ac3e52.
Безreboot/PCM/аудиобаги. Следующее — совместный цикл питания LDO15/BOB через
framework и GPIO58 для aggregate WCD9385, затем звуковая карта/маршруты.
