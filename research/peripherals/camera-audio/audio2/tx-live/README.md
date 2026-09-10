## 2026-09-09 — TX и RX аппаратно Attached одновременно

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52, kernel6.18.34-tcl-audio2.
TX macro62620000 подтверждён qcauddev7180 VA14000b524 и atoll-audio-overlay.dtsi.
Добавлен overlay5 с qcom,sm8250-lpass-tx-macro, clocks q6afe57/58/102/103,
fsgen alias от VA62770000 по образцу уже рабочегоRX. Driver tx_macro bound,
phandle e3. Overlay7 добавляет TX SoundWire62630000, SPI296 edge, clocke3,
LPI gpio0clk/gpio1,2,14data (10mA,slew3,nopull/hold), пятьTXпортов поSM8250,
codec@0,3 compatible sdw20217010d00, tx-port-mapping2/3/4/5.
Первая попытка overlay6 отклонена -EINVAL: live tree без __symbols__, dtc -@
экспортировал символы локальных labels. Пересборка без -@ сохранила
__local_fixups__, убрала __symbols__; overlay7 применён успешно.

| Канал | Контроллер | HW ENUM address | Статус в четырёх выборках |
| --- | --- | --- | --- |
| RX | 62610000, master2 | 240217010d00 | Attached, device_number1, driver wcd9380-codec |
| TX | 62630000, master3 | 230217010d00 | Attached, device_number1, driver wcd9380-codec |

Uptime23580–23593s: candidatev5 обслуживает оба контроллера, сначалаreset-resume
с аппаратнымstatus0, затем общие питание/reset. ОбаCOMP_STATUS1, slave_status4.
TX version01050001, ID1=01170223, ID2=0000000d; UID3 теперь подтверждён железом.
ПослеcleanupGPIOcfg1/io0,disable_complete1, исходныйsoundwire_qcom восстановлен,
Westonactive,taint4096. Никаких reboot/PCM/публикации аудиобаги.
Runtimeoverlays5/7 остаются до reboot, SoundWire baseline теперь4 entries;
старые скрипты с ожиданием1/2 entries неприменимы. Новый test-codec-rxtx.sh
держит VA/RX/TX и обаSWR включёнными, снимает питание/reset автоматически.

Следующее: aggregate WCD938x driver требует reset GPIO и supplies доcomponent binding.
GPIO58 остаётся reserved в TLMM — обычный devm_gpiod_get пока неприменим.
Не снимать вслепую reservation58..62: только58 проверен. Питание пока тестовыми
RPMh votes, ещё не regulator-framework. Нужна корректная интеграция этих ресурсов,
затем определение ревизии кодека, ALSA card, маршруты и тихая проверка звука.
TX/RX enumeration ещё не доказывает работоспособность PCM/аналогового выхода.
