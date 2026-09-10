## 2026-09-09 — общий WCD9385 ASoC component зарегистрирован; Oops при cleanup

ВАЖНО: текущая загрузка c781c4d4-cd36-4a07-b734-40b059ac3e52 ПОСЛЕ KERNEL OOPS,
taint4224 (ранее4096). Аппаратные тесты прекратить до согласованной перезагрузки.
SSH/Weston оставались доступны. Не выгружать/перепривязывать далее драйверы.

### Конфигурация объединённого теста

Overlay10: отдельные tcl-audio-ldo-test (создан толькоldo15!), GPIO58 provider,
aggregate qcom,wcd9385-codec и regulator-fixed tcl-io-existing-test-only.
LDO10_A НЕ создан/НЕ получил votes; IO/RXTX представлены условным неуправляемым
источником1.8V, что сохраняет прежнее питание. Это временная диагностическая
модель, не подтверждение реальной разводки или постоянного power-management.
BOB_C — ранее созданный provider узелphandlee8,точно3300mV. LDO15_A1800mV.
Старый BOB автоconsumer подавлялся driver_override=tcl-disabled на время теста.
GPIOprovider только58, MMIOcallback безsleep (WCD использует gpiod_set_value).
Приfree descriptor восстановлениеcfg1/io0. Новые resource providers изолированы
от обычных PMICbankdrivers. Штатный TLMM иreserved59..62 не трогались.

### Два теста

Первый (aggregate-result.txt): общий драйвер смог получитьGPIO ивключитьоба
реальныхрегулятора, но of_parse_phandle RX вернулNULL. Причина вoverlay.c:
add_changeset_property игнорирует name/phandle/linux,phandle для существующих
узлов. Лейбл __overlay__ не назначает phandle старомуcodec node. Cleanupуспешен.

Второй (aggregate-oops-full.txt содержитполныйdmesg,lsmod,журнал): диагностический
вариант wcd938x.c на tcl,book14/узлеtcl-wcd9385 приотсутствииphandle ищет реальные
RX/TX nodes поабсолютнымпутям. Это обход runtimeoverlay, не требуется в полном
bootDT с корректными phandle. Вuptime25408 обаslave bound, driverwcd938x_codec
привязан, soc@0:tcl-wcd9385 появился в debugfs/asoc/components.
ALSAcard отсутствует: это регистрациякомпонента, ещё не запускPCM/звук.
Есть предупреждения runtime PM: child SDW активируется при неактивном logical
master sdw-master2/3. Физическиеplatformmaster иmacro clocks держалисьon.
Это остаётся отдельным вопросом корректностиPM, не скрывать предупреждения.

Watchdog25439: codec detached, LDO15/BOB enable0 ret0, GPIOcfg1/io0 restored.
Далее скриптвыгрузилкодек/resources/candidateSWR ивернулstockSWR.
В25443 процессmodprobe получилOops вirq_find_matching_fwspec+0x7c;
stackof_irq_get -> qcom_swrm_probe. Taint4224. ВосстановлениеSWR НЕуспешно.

### Наиболее вероятная причина

wcd938x_irq_init создаёт irq_domain_create_linear(NULL,1,&wcd_domain_ops,NULL)
иirq_create_mapping, но sourcewcd938x.c не содержитirq_domain_remove или
irq_dispose_mapping. unbind также ихнеосвобождает. После выгрузки диагностического
wcd module вобщемспискедомeновостаётсяуказательнаops извыгруженнойпамяти.
irq_find_matching_fwspec проходитэтотсписок иразыменовываетh->ops->select/match;
это соответствует местуfault иunmappedmoduleaddressffff80007a84c280.
Это сильнаяобоснованнаягипотеза, не доказаннаяповторнымтестомисправления.
Падениеподmutexirq_domain_mutex моглооставитьзамокзанятым; не пытаться лечить
повторнымmodprobe/kill илиновымиIRQallocations. Нужнаперезагрузка.

### До следующего аппаратного теста

1. Исправитьlifecycle IRQ-domain: снять slave_irq ссылки сRX/TX с правильной
   синхронизацией SoundWire callbacks, удалитьregmap IRQchip ДОdisposingparent
   mapping, затемirq_domain_remove; покрытьbind-error/remove/rebind пути.
   Одного удалениядомена вmodule_exit недостаточно.
2. ПолныйbootDT долженсодержатьcodec RX/TX phandlesсмоментасозданияузлов.
3. Уточнить logicalSoundWiremaster runtimePM и lifecycle component_bind/unbind.
4. Не использовать старый cleanupскриптсвыгрузкойWCD безисправленияIRQcleanup.
5. Послечистойзагрузки восстановитьпроверенныеoverlay/SWR/powerресурсы,
   зарегистрироватьASoC card/маршруты итихийPCMтест. Нынешнююзагрузкунетрогать.

Всеисходники/ko/DT/testscripts сохранены рядом. Bugzillaпоаудио необновлялась;
пользовательпросилсначалапроверитьзвук. Перезагрузка пока не выполнена.
