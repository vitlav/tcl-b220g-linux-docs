# WCD9385 IRQ lifetime fix — 2026-09-09

Исправление написано и cross-built, на оборудовании пока НЕ проверено.
Текущий ноут после Oops: bootc781c4d4-cd36-4a07-b734-40b059ac3e52,taint4224.
Никаких SSH-модификаций/новых аппаратных тестов/перезагрузок в этом шаге не было.

## Изменения

* wcd938x_irq_init проверяет создание IRQ mapping. При неудаче mapping или
  regmap IRQchip удаляет созданные mapping/domain в обратном порядке.
* wcd938x_irq_exit вызывается при unbind и при ошибке регистрации ASoC component.
  Сначала очищает slave_irq RX/TX под sdw_dev_lock каждогоslave, дожидаясь
  завершения текущего SoundWire interrupt_callback. Затем удаляет managed
  regmap IRQchip, parent mapping, domain; обнуляет поля.
* wcd9380_interrupt_callback в wcd938x-sdw.c возвращает0 приNULLslave_irq.
  Нужен вместе с основным патчем: иначе после очистки ссылки callback вызовет
  общий helper с NULLdomain, что не является корректной обработкой отвязки.
* Публикация slave_irq также под sdw_dev_lock. SoundWire bus.c вызывает
  interrupt_callback подэтимmutex. При удаленииIRQchip lock не удерживается,
  чтобы не ждатьIRQhandler под его собственнымlock.

Это исправляет найденный отсутствующий cleanup, а не только откладывает выгрузку.
Не гарантирует отсутствие других ошибок драйвера: PM предупреждения, аудиокарта,
реальная разводкаIO и воспроизведение остаются непроверенными.

## Артефакты и проверки

`generic/wcd938x-irq-lifetime.patch` — исправление двух исходников чистого6.18.34.
`generic/` — исходники и tcl_wcd_irq_fixed.ko / tcl_wcd_sdw_irq_fixed.ko.
`runtime-path/` — та же пара с отдельным временным fallback точныхDTпутей TCL;
этот fallback нужен только пока старыеRX/TX overlay узлы не имеютphandle.
`runtime-path-fallback.patch` отдельно показываетэтодиагностическоеизменение.
Названияkoвдвухкаталогахсовпадают, обязательноуказыватькаталог/сверятьSHA256SUMS.

Обе пары успешно собраны ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1
против /tmp/tcl-audio2-linux, безошибок/предупрежденийкомпилятора.
checkpatch.pl --no-tree --no-signoff:0errors,0warnings,112lines.
git apply --check genericpatch противисходногодерева:успех.
Основное деревоисходниковнеизменено, оригинальныеproductionмодулинеперезаписаны.
Это compile/static validation; runtime гонки и error injection не тестировались.

## Проверка после согласованной чистой загрузки

1. Восстановить нужные macro/SWR/codec nodes и стабильные clocks; проверитьtaintbaseline.
2. Использовать исправленные ОБА модуля: aggregate и SoundWire codec. Старый
   wcd938x-sdw безNULLguard с новым aggregate НЕ использовать.
3. Проверитьaggregatebind, списокASoCcomponents ичислоIRQdomains/IRQmapping
   до/послеunbind. Сначалаbind/unbind безmoduleunload; сборлогаавтосохранения.
4. Послеуспеха проверитьвыгрузкукодека и повторнуюрегистрациюSoundWire,
   на которой былOops. ПроверитьотсутствиеновыхOops/WARN/taintизменений.
5. Покрыть отказы mapping/regmapirq/component registration доступным fault
   injection либоотдельнымтестовымядром; этипути пока проверены только по коду.
6. Затем вернуться кALSAcard/маршрутам и тихомуPCM. Не заявлятьзвукготовым.

Bugzilla не обновлялась до проверкизвука поуказаниюпользователя.

## Файлы патчей

- [wcd938x-irq-lifetime.patch](../../../../../patches/kernel/audio/wcd938x-irq-lifetime.patch) — WCD938x: cleanup IRQ domain/mapping и защита SoundWire callback. Локальное исправление; два файла применяются совместно; см. историю аппаратных проверок.
- [runtime-path-fallback.patch](../../../../../patches/kernel/audio/runtime-path-fallback.patch) — WCD938x: экспериментальный runtime path fallback. Вспомогательный эксперимент, не часть общего IRQ lifetime fix.
