# LPASS power management: дополнительные исправления ACPI

Каталог применяется **после основной ACPI-серии** и разделяет аппаратно проверенное управление LPI clocks от сохранённых кандидатов qc7. В `series` включён только проверенный LPI runtime-PM patch. Q6AFE hardware-vote firmware ноутбука отвергает как неизвестную команду, поэтому Q6AFE и VA macro patches сохранены для анализа происхождения, но в рабочую TCL-серию не входят. DT-комплект не меняется.

## Состав

| Файл | Назначение и происхождение |
|---|---|
| [0001](0001-lpass-va-macro-disable-clocks-in-suspend.patch) | Оригинал Nikita Travkin `283c8d3f8579`: баланс macro/dcodec clocks в VA macro runtime suspend/resume. Сохранён без изменений и не входит в `series`: VA probe на штатной TCL ADSP firmware упирается в неподдерживаемый hardware-vote. |
| [0002](0002-q6afe-track-dsp-vote-handle.patch) | Логически отдельная адаптация Nikita Travkin `2637dc2cbba0`: сохранить DSP client handle и передать его при unvote. Не входит в `series`, поскольку TCL firmware отвергает сам vote opcode до выдачи handle. |
| [0003](0003-lpi-acpi-runtime-clock-management.patch) | Аппаратно проверенный TCL ACPI LPI runtime-PM: получить core/audio clocks, балансировать их стандартными callbacks, проверять ошибку и откатывать частичное включение. Это единственный patch в `series`. |
| [0004](0004-q6afe-compact-vote-response.patch) | Логически отдельная поддержка компактного 32-битного vote-response. Не входит в `series`: текущая TCL firmware вместо такого ответа возвращает `APR_BASIC_RSP_RESULT`, status `0x16` (`Unknown cmd`) для opcode `0x100f4`. |
| [modules/tcl_lpi_provider.c](modules/tcl_lpi_provider.c) | Обязательная замена одноимённого внешнего модуля при проверке 0003: зарегистрировать проверяемые clkdev aliases до устройства, убрать временное включение clocks вокруг регистрации. Удаление устройства предшествует освобождению aliases и ссылок. |

[series](series) содержит только аппаратно проверенный LPI patch. Внешний provider собирается из указанной здесь версии вместо файла основной серии. Не смешивать новый LPI driver со старым provider: отсутствие clocks должно приводить к ошибке probe, а не к доступу к обесточенным регистрам. [Манифест](manifest.json) фиксирует базу, итоговое дерево и SHA-256 файлов. Оригиналы доступны в [архиве qc7](../../../../../research/qc7-provenance/travmurav-original-commits.mbox).

## Стандартные интерфейсы

Путь устройства: `/sys/bus/platform/devices/tcl-lpi-provider.0/power/`.

| Интерфейс | Смысл |
|---|---|
| `control=auto` | Разрешить runtime suspend после завершения обращений; установленная драйвером задержка — 100 мс. |
| `control=on` | Удерживать устройство активным через PM core. Обязательно проверять результат записи и `runtime_status`. |
| `runtime_status` | Состояние runtime PM. Само по себе не доказывает выключения аппаратных clocks. |
| `runtime_active_time`, `runtime_suspended_time` | Время в состояниях runtime PM, мс. |
| `autosuspend_delay_ms` | Стандартная настройка задержки autosuspend. |

Соответствие clocks сохраняет ранее использовавшиеся источники платы: `core` → `LPASS_CLK_ID_RX_CORE_MCLK`, `audio` → `LPASS_CLK_ID_RX_CORE_NPL_MCLK`. CCF владеет общими ссылками: при наличии других потребителей общий clock не обязан выключаться. Для диагностики сверять отдельные `clk_prepare_count`/`clk_enable_count` в debugfs и их **приращения**, не читать подряд регистры устройств. Debugfs — диагностика, не пользовательский ABI управления питанием.

Существующие `pm_runtime_resume_and_get()` вокруг GPIO/pinmux операций получают ошибку `clk_bulk_prepare_enable()` и не выполняют MMIO при неуспешном включении. Bulk helper освобождает уже включённые clocks при отказе следующего. При удалении драйвера активные ссылки освобождаются после остановки runtime PM.

## Выполненные проверки

- Независимое применение всей дополнительной серии во временном индексе воспроизводит итоговое дерево из манифеста.
- ARM64 cross-build с `W=1`: Q6AFE и оба LPI модуля, внешний provider; VA macro проверен сборкой объекта.
- Host harness вызывает извлечённые из исходника функции Q6AFE с подменённым APR transport: успешные последовательные handles, NULL output, unvote payload/token, send error, timeout, DSP error, короткий ответ, освобождение mutex. Исходная версия не проходит проверку возвращённого handle, адаптированная проходит.
- Host harness вызывает новые LPI callbacks и реальные bulk helpers ядра с подменёнными низкоуровневыми clocks: 1000 сбалансированных циклов, отказ prepare/enable первого и второго clock, полный откат. Проверено сохранение вызова прежнего pm_clk пути; это не аппаратное тестирование DT.
- ASan/UBSan для host-проверок; LeakSanitizer отключён из-за ограничений запуска под ptrace. Эти проверки не моделируют DSP, IRQ concurrency или реальную электрическую схему.

Аппаратная проверка LPI на TCL B220G выполнена после восстановления root-файловой системы. Новые `pinctrl_lpass_lpi`, `pinctrl_sc7280_lpass_lpi` и `tcl_lpi_provider` загружены без перезагрузки; provider успешно привязался и настроил SoundWire pinmux. Через `/sys/bus/platform/devices/tcl-lpi-provider.0/power/control` переход `on` показал active clocks, `auto` вернул `runtime_status=suspended` и нулевые enable counts у RX MCLK/NPL. Короткий `speaker-test` 880 Гц прошёл на HPHL/HPHR=12 из 24 и RX digital=62 из 124; DAPM включил и выключил усилители, после теста LPI снова suspended. Во время проверки сеть, графика и загрузочный комплект не менялись.

Отдельная аппаратная проверка Q6AFE дала точный отрицательный результат: VA macro запросил блок 3 командой `AFE_CMD_REMOTE_LPASS_CORE_HW_VOTE_REQUEST` (`0x100f4`), ADSP ответил обычным `APR_BASIC_RSP_RESULT` со status `0x16`, драйвер расшифровал его как `Unknown cmd`, а VA probe завершился `-ETIMEDOUT`. Это не компактный успешный ответ и не исправляется patch 0004. RX/TX macro и SoundWire после появления собственных clocks успешно перепривязались, ALSA playback работает; VA в текущем speaker path не требуется.

## Аппаратная приёмка и оставшиеся требования

1. На исправной файловой системе сохранить рабочие модули и mixer state; исключить активный PCM. Проверить возможность обычного освобождения модулей; принудительную выгрузку не использовать.
2. Проверить probe LPI и точные aliases, несколько циклов `power/control=on/auto`, состояния и приращения CCF, отсутствие новых ошибок ядра.
3. Проверить слышимый PCM, остановку и повторный старт; сверить обе громкости и switches чтением после записи. Убедиться в отсутствии утечки clock references и регрессии SoundWire после idle.
4. После безопасной замены Q6AFE проверить реальные vote/unvote циклы и отсутствие ошибок DSP. Адаптация не решает корреляцию запоздалого ответа после timeout, восстановление handles после DSP reset и подтверждение асинхронного unvote.
5. Согласовать runtime PM VA/RX/TX, SoundWire и ADSP, затем проверять system suspend/resume. Этот набор **не объявляет полноценную поддержку сна ноутбука**.
6. Перенести удержание аудиопитания из board power-hold модулей в корректные regulator consumers/DAPM/runtime PM. Управление батареей и EC должно предоставляться через power_supply, thermal и другие соответствующие интерфейсы ядра, а не произвольные записи MMIO из userspace; эти драйверы не реализуются данным набором.
