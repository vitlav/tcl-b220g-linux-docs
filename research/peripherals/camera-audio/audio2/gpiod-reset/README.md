## 2026-09-09 — GPIO descriptor API для сброса проверен

Временный OOT gpio_chip представляет исключительно проверенный GPIO58 WEST,
MMIO0353a000..13. Это диагностический adapter, НЕ штатный TLMM driver.
Machine tcl,book14 и presence1/cfg1/io0 проверяются до записи. DT reservation
и TLMM valid_mask не меняются. GPIO59..62 не затронуты. Adapter нельзя
использовать одновременно с обычным владельцем GPIO58 или raw reset module.

gpiochip_request_own_desc(... GPIO_ACTIVE_LOW,GPIOD_OUT_HIGH) утверждаетreset
(физический0), через5ms gpiod_set_value_cansleep(desc,0) снимаетreset
(физический1), далее2ms ожидание. API logical0, cfg200/io3. Автовозврат15s,
нормальный module_exit возвращаетcfg1/io0 и удаляетgpio_chip.

Uptime24726–24737: RX/TX обаAttached, аппаратныеID240217010d00/230217010d00.
Прямые sdw_read_no_pm снова дали3401=00,3402=00,3403=0d,3404=01,34b0=0a
(WCD9385 variant5). Ошибок нового теста в сохранённом журнале нет.
Cleanup24737: GPIOcfg1/io0,disable_complete1,stock soundwire_qcom восстановлен,
Westonactive,taint4096,тотжеbootc781c4d4-cd36-4a07-b734-40b059ac3e52.

Питание В ЭТОМ тесте по-прежнему raw RPMh tcl_audio_power_test (20s), не
regulator framework. BOB framework проверялся отдельно ранее. Не утверждать,
что aggregate WCD driver уже работал или общий framework power/reset цикл
готов. ALSA card/PCM/звук ещё не проверены. Перезагрузок/аудиобаги не было.

Следующая конкретная интеграция: adapter GPIO без собственного descriptor
предоставляет reset descriptor общему WCD9385 consumer; отдельные supply
providers дляLDO15_A/BOB_C с проверенными запросами, IO/RXTX питание учитывает
остающуюся гипотезуLDO10_A. Исключить старый автоconsumerBOB, чтобы он не снимал
питание нового теста. Удерживать macro/master clocks до завершения bind;
после удаления aggregate сначала освободить callbacks/IRQ/device links,
затем GPIO/supplies. Старый tcl_gpiod_reset сам занимает единственныйdesc,
поэтому без изменения lifecycle НЕ годится в качестве provider aggregate.
