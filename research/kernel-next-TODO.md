# TCL B220G: TODO следующей сборки ядра

Обновлено 2026-09-09. База: 6.18.34-tcl-ice1. Новое ядро пока не собрано и не установлено.

## Конфигурация для диагностики

| Опция | Требуемое значение | Зачем |
|---|---|---|
| CONFIG_DYNAMIC_DEBUG | y | Включать dev_dbg/pr_debug отдельных драйверов без пересборки модулей |
| CONFIG_DEBUG_FS | y (сохранить) | debugfs и dynamic_debug/control |
| CONFIG_FTRACE | y | Общая поддержка трассировки |
| CONFIG_FUNCTION_TRACER | y | Трассировка вызовов функций |
| CONFIG_FUNCTION_GRAPH_TRACER | y | Вложенные вызовы и длительность |
| CONFIG_KPROBES | y | Динамические точки наблюдения |
| CONFIG_KPROBE_EVENTS | y | Доступ к kprobes через tracefs |
| CONFIG_FTRACE_SYSCALLS | y | События системных вызовов |
| CONFIG_STACKTRACE, CONFIG_KALLSYMS | y (сохранить) | Читаемые стеки и символы |
| CONFIG_OF_DYNAMIC | y (сохранить) | Изменения дерева устройств внутри ядра |
| CONFIG_OF_OVERLAY | y | API применения DT overlays |
| CONFIG_CONFIGFS_FS | y (сохранить) | Для загрузчика overlays, если выбранный загрузчик использует configfs |

Уточнить зависимости через Kconfig/olddefconfig именно выбранной версии; проверить итоговый .config, а не только запрошенный fragment. По умолчанию трассировку и отладочную печать оставить выключенными; включать на время отдельных тестов.

CONFIG_OF_OVERLAY сам не создаёт пользовательский интерфейс загрузки DTBO: отдельно выбрать и проверить загрузчик. Зарезервировать память ADSP в загрузочном DT, не рассчитывать на поздний overlay для reserved-memory.

Для отдельного диагностического варианта рассмотреть CONFIG_PROVE_LOCKING/LOCKDEP и DEBUG_ATOMIC_SLEEP: помогают разбирать блокировки, но добавляют накладные расходы.

## Исправления и регрессии

- Исправить q6afe_cdc_dma_port_prepare: потеря ненулевой active_channels_mask, бага https://bugs.etersoft.ru/19478 (блокирует https://bugs.etersoft.ru/19084). Патч и проверка явной/автоматической маски обязательны.
- Проверить повторный bind APR-сервисов: после замены q6asm наблюдалось APR: service is not registered (7). В apr_device_remove есть idr_remove, в apr_device_probe нет симметричной регистрации. Отдельная бага https://bugs.etersoft.ru/19479.
- Разобрать зависание unbind GLINK: https://bugs.etersoft.ru/19481. Стек и исходники указывают на повторное удаление rpmsg_device из destroy_ept.
- Сохранить рабочие Wi-Fi, UFS/ICE, CPUfreq, питание, DPU/LT8911, GPU, Venus и UVC. Проверить наличие соответствующих модулей и firmware в root/initramfs.
- Перенести проверенный dai@0 Q6ASM в загрузочный DT вместо временного live-модуля. Диагностическая dummy-карта не заменяет физический кодек.
- Сохранить предыдущий рабочий GRUB-пункт; новый пункт сделать по умолчанию после проверки файлов. Сохранить логгер в root и на USB, автономный Wi-Fi/SSH.
