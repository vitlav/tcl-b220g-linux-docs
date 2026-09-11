# Патчи ядра для TCL B220G

**Актуальный самостоятельный ACPI-набор:** [Linux 7.2.4 — 16 патчей, конфигурация и проверенные результаты](acpi/linux-7.2.4/README.md). Применяется к чистому v7.2.4 без qc7 и без других патчей из этого каталога. DT-комплект ведётся отдельно.

Здесь собраны сохранённые изменения Linux из исследования. Это **каталог, не единая применяемая серия**: есть альтернативные версии и диагностические патчи. Нельзя автоматически применить все файлы подряд. Основная исследованная база — Linux 6.18.34 с локальными изменениями; точный контекст каждого патча указан в первичном отчёте.

## Происхождение и применимость

Полная последовательная серия для чистой upstream-базы пока не сформирована. GLINK-патч — backport upstream `5a5a48e788e02`; остальные изменения имеют назначение и степень проверки, указанные ниже. Сохранённые заголовки патчей определяют авторство и происхождение.

## Перечень

| Файл | Назначение | Статус / ограничения | Первичный отчёт |
|---|---|---|---|
| [acpi/scm-acpi-draft-v2.patch](acpi/scm-acpi-draft-v2.patch) | SCM ACPI v2: match, DMA guard, начальный TZMEM pool, зависимости | Собран в полном ACPI2 Image; требует ранней _CCA; аппаратно не проверен | [Отчёт](../../research/acpi-audit/scm/README.md) |
| [acpi/scm-acpi-draft.patch](acpi/scm-acpi-draft.patch) | SCM ACPI v1: match QCOM080B, условный OF ICC | Предыдущий вариант, заменён v2; не применять вместе | [Отчёт](../../research/acpi-audit/scm/README.md) |
| [acpi/sc7180-acpi-smmu-selection.patch](acpi/sc7180-acpi-smmu-selection.patch) | Раздельный выбор Apps MMU500 и Adreno SMMUv2 по IORT/ресурсам | Полный ACPI2 Image; object-build ACPI=y/n и host-матрица; аппаратно не проверен | [Отчёт](../../research/acpi-audit/smmu/selection-draft/README.md) |
| [acpi/acpica-default-spaces-without-pci.patch](acpi/acpica-default-spaces-without-pci.patch) | Исключить PCI_CONFIG из default address spaces при PCI=n | Собран в ACPI1; вся ACPI-загрузка ещё не работает | [Отчёт](../../research/acpi-boot/pci-disabled/README.md) |
| [audio/adsp1-pcm-trace.patch](audio/adsp1-pcm-trace.patch) | Q6ASM: ранняя трассировка PCM | Историческая диагностика ADSP1 | [Отчёт](../../research/peripherals/camera-audio/adsp1/README.md) |
| [audio/adsp1-q6afe-active-mask.patch](audio/adsp1-q6afe-active-mask.patch) | Ранняя версия передачи явной маски каналов Q6AFE | Исторический вариант; сравнить с AUDIO2 0002, не применять оба | [Отчёт](../../research/peripherals/camera-audio/adsp1/README.md) |
| [audio/0001-glink-destroy-backport.patch](audio/0001-glink-destroy-backport.patch) | GLINK: backport upstream 5a5a48e788e02 | В составе AUDIO2; отдельный upstream commit существует | [Отчёт](../../research/peripherals/camera-audio/audio2/README.md) |
| [audio/0002-q6afe-active-mask.patch](audio/0002-q6afe-active-mask.patch) | Q6AFE: передача active_channels_mask | В AUDIO2; hw_params с маской 3 прошёл аппаратно | [Отчёт](../../research/peripherals/camera-audio/audio2/README.md) |
| [audio/0003-apr-reprobe-candidate.patch](audio/0003-apr-reprobe-candidate.patch) | APR: восстановление IDR для повторного probe | Локальный кандидат AUDIO2; reprobe проверялся, не upstream | [Отчёт](../../research/peripherals/camera-audio/audio2/README.md) |
| [audio/0004-q6asm-debug-messages.patch](audio/0004-q6asm-debug-messages.patch) | Q6ASM: диагностические dev_dbg | Диагностика AUDIO2, не самостоятельное исправление | [Отчёт](../../research/peripherals/camera-audio/audio2/README.md) |
| [audio/0005-q6asm-write-done-debug.patch](audio/0005-q6asm-write-done-debug.patch) | Q6ASM: подробности WRITE_DONE | Дополнительная диагностика; учитывать исходную версию q6asm.c | [Отчёт](../../research/peripherals/camera-audio/audio2/README.md) |
| [audio/wcd938x-irq-lifetime.patch](audio/wcd938x-irq-lifetime.patch) | WCD938x: cleanup IRQ domain/mapping и защита SoundWire callback | Локальное исправление; два файла применяются совместно; полное покрытие гонок не выполнено | [Отчёт](../../research/peripherals/camera-audio/audio2/irq-lifetime-fix/README.md) |
| [audio/runtime-path-fallback.patch](audio/runtime-path-fallback.patch) | WCD938x: экспериментальный runtime path fallback | Вспомогательный эксперимент, не часть общего IRQ lifetime fix | [Отчёт](../../research/peripherals/camera-audio/audio2/irq-lifetime-fix/README.md) |
| [audio/q6asm-prepare-stopped.patch](audio/q6asm-prepare-stopped.patch) | Q6ASM DAI: повторный prepare после STOPPED | Три write/drop/prepare цикла на одном PCM handle проверены; остальные пути требуют проверки | [Отчёт](../../research/peripherals/camera-audio/audio2/q6asm-reprepare/README.md) |
| [audio/empty-bus-resume-candidate.patch](audio/empty-bus-resume-candidate.patch) | SoundWire: reset/re-enumeration для пустой шины после ENODATA | Гипотеза; не считать доказанным исправлением причины | [Отчёт](../../research/peripherals/camera-audio/audio2/swr-resume-trace/README.md) |
| [audio/experimental-resume-and-pm.patch](audio/experimental-resume-and-pm.patch) | SoundWire: эксперимент с resume и PM | Альтернативный runtime-эксперимент; не складывать с empty-bus вариантом | [Отчёт](../../research/peripherals/camera-audio/audio2/swr-resume-trace/README.md) |
| [graphics/dpu-assignment-diagnostic.patch](graphics/dpu-assignment-diagnostic.patch) | DPU: fallback на назначенный CRTC при потере legacy lookup | Аппаратно сработал fallback; 33 сцены glmark2 завершились; локальная диагностика, не upstream | [Отчёт](../../research/peripherals/wayland-probe/native-display/dpu-encoder-investigation/fix/README.md) |

## Применимость и происхождение

`manifest.json` сопоставляет каждую копию с исходным файлом в `research/` и его SHA256. Копии побайтно одинаковы. При обновлении изменять обе копии и манифест; контекст и старые результаты сохранять в тематическом отчёте.

Для чистой последовательности `git format-patch` потребуется выбрать точную базу ядра, разделить рабочие исправления и эксперименты, проверить последовательное применение и полную сборку, затем создать настоящие коммиты в новом Git-дереве Linux. Эта работа пока не выполнена.

Сравнения DT/config `.diff` и сторонние конфигурации ALT/pmOS оставлены в `research/`: они не являются нашей единой серией исходников ядра. ASL/SSDT — отдельно в [раннем SCM-комплекте](../../research/acpi-audit/scm/early-ssdt/README.md). Новые драйверы, которые сохранялись полными `.c`, также находятся в `research/` и пока не оформлены как патчи добавления в Linux.

Проверка каталога: все 17 файлов проходят `git apply --numstat`. Это проверка синтаксиса diff, не доказательство последовательной применимости к одной базе. При подготовке восстановлены три срезанные при первоначальном импорте завершающие строки контекста из оригинального архива; обе копии и манифесты исправлены.
