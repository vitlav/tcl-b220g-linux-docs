# 2026-09-09 — первый аппаратный ответ WCD938x после GPIO58 reset

Активная задача — ЗВУК. ACPI19492 завели и отложили по явному уточнению пользователя; больше не продолжать его автоматически.

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52,6.18.34-tcl-audio2. Успешные единичные чтения GPIO58:0353a010=1;0353a000cfg=1;0353a004io=0. Это mux0,OE0,pull-down,drive0. GPIO58 WEST доступен в этой загрузке. GPIO59..62 NORTH не проверялись, reservation в DT сохранён.

OEM последовательность low5ms/high2ms проверена коротким модулем с guards machine=tcl,book14,presence1,cfg1,io0; записьlatch0,cfg200,5ms,latch2,2ms. Послеrelease cfg200/io3. Автовосстановление15s, питание20s; сценарий снимает reset/power раньше. Использованы ранее проверенные LDO15_A1800mV/mode7 и BOB_C3300mV/mode6, SoundWire reset-resume candidatev2.

Первый тест15103–15115s: четыре чтенияCOMP_STATUS1,slave_status4,ID1=01170224,ID2=0000000d. По qcom byte-order addr240217010d00: version2,unique4,mfg0217,part010d,class0. Соответствует драйверуWCD938x, не различает9380/9385. Slot1 ATTACHED. До reset при тех же запросах питания/работающемframegen ответы были0.

ИСПРАВЛЕНО 2026-09-09: sysfs slave ПОЯВЛЯЛСЯ во время теста; прежний вывод по списку после cleanup был ошибочным. Звук не воспроизводился. Все измененияGPIO восстановленыcfg1/io0, votesdisable_complete1,stock soundwire_qcom восстановлен,Westonactive,taint4096. Следующийv3 тест15958–15967s дал тот же аппаратныйответ. v3добавляетenumerate вreset-resume; послеGPIOrelease resume НЕ был вызван, поэтому hook не был проверен. Не утверждать, что явноеenumerate не помогло. ПроверитьIRQ/PM.

GPIO58 теперь адресно проверен; это не разрешение доступа ко всей58..62. Багу по звуку пока не обновлять до проверки слышимого результата. Логи первого теста и исходники рядом; второй лог сохранён рядом. На ноуте файлcandidate.ko сейчасv3,но загружен исходныйsoundwire_qcom. Перезагрузок не было.


2026-09-09 — IRQ и регистрация RX-кодека проверены полными логами.
В codec-irq-result.txt четыре выборки содержат sdw:2:0:0217:010d:00:4.
IRQ raw2/raw4, ENUM slot1 addr240217010d00: пропуска IRQ нет.
В codec-binding-result.txt (uptime22647–22660) устройство существует, но без driver,
status UNATTACHED, device_number N/A. Причина отказа привязки в
kernel drivers/soundwire/bus_type.c: sdw_drv_probe возвращает -ENODEV без fwnode
(при !CONFIG_ACPI также требует of_node). qcom dynamic discovery вызывает
sdw_slave_add(..., NULL), а dev_num новому slave остаётся0; DT pre-registration
должна обеспечить штатные match, dev_num и binding. Не добавлять polling/повторный
reset ради обхода отсутствующего DT. Новый RX-only overlay содержит compatible
sdw20217010d00, reg<0 4>, rx-port-mapping<1 2 3 4 5> по binding WCD938x.
Это ещё не проверка звука и не доказательство модели9380/9385. ACPI отложен.


## 2026-09-09 — RX WCD938x Attached: проверено без перезагрузки

| Проверка | Результат |
| --- | --- |
| RX DT overlay | Применён id4, /soc@0/soundwire@62610000/codec@0,4; остаётся до перезагрузки |
| DT + candidate v4, uptime22954–22966 | Driver wcd9380-codec привязан, но framegen8 и UNATTACHED |
| Причина второго отрицательного теста | v4 проверял list_empty(bus.slaves); DT создал slave заранее, поэтому reset-resume больше не выбирался |
| Candidate v5 | После sdw_bus_clk_stop свежий get_device_status; при ret0/-ENODATA и аппаратном slave_status0 выбирается существующий reset-resume |
| v5, uptime23080–23092 | Четыре выборки: driver wcd9380-codec, status Attached, device_number1; COMP_STATUS1, slave_status4, ID240217010d00 |
| Завершение | GPIO cfg1/io0, питание disable_complete1, stock soundwire_qcom восстановлен, Westonactive, taint4096, тот же boot |

Логи codec-dt-binding-result.txt и codec-dt-attached-result.txt; точные исходники и ko
в rx-codec-dt/ и unattached-candidate-v5/. На ноуте файл candidate.ko теперь v5,
но загружен исходный драйвер. После cleanup DT-устройство остаётся, поскольку overlay
сохранён; питание снято, нельзя утверждать, что сейчас оно Attached. Базовый список
SoundWire теперь содержит master и один DT slave (2 entries), что учтено новым скриптом.
Это подтверждает RX enumeration и привязку штатного WCD938x SoundWire driver,
но не воспроизведение, не ALSA card и не различие WCD9380/9385.

Следующее: TX macro/контроллер62630000 (OEM SPI296/global328), pin/clock dependencies,
TX codec UID3 сверить с аппаратным ответом. Затем aggregate WCD938x с управлением
питанием/reset через драйверы и sound-card/маршруты; только после этого тихая PCM-проверка.
RX mapping1..5 взят из binding, перенос аудиоданных пока не проверен.
Багу по звуку не обновляли: ждём проверки слышимого результата. ACPI отложен.
