# Ядро, кросс-компиляция и Ubuntu

Используется собственная сборка Linux `6.18.34-tcl-audio2` с изменениями DT, экспериментальными драйверами и отдельными исправлениями аудио/графики. Пользовательское окружение — Ubuntu. Нельзя называть такой комплект «чистым ядром Ubuntu».

Ядро собиралось на x86_64 с `ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-`; GCC 15.3.1 установлен из репозитория. Полный Image MEMDIAG успешно собран. Исходная база `/tmp/tcl-audio2-linux`, копия MEMDIAG `/tmp/tcl-memdiag-linux`; эти пути — места исходной сессии, не переносимые зависимости нового репозитория.

MEMDIAG отличается отключением CONFIG_STRICT_DEVMEM и производного CONFIG_EXCLUSIVE_SYSTEM_RAM; Module.symvers совпадает с базой. Release намеренно оставлен прежним для совместимости существующих модулей и сценариев. Поэтому `uname -r` недостаточно: нужны SHA256 Image и kernel.config.

В отдельном `/tmp/tcl-acpi1-linux` применяется черновик SCM ACPI v2. Собран лишь объект qcom_scm.o с W=1; **полный Image с этим черновиком не строился и не установлен**. Не путать исходники с последней ACPI1 Image на USB.

## Изменения образа Ubuntu

Первичный подробный отчёт: [ubuntu-base/README.md](../research/peripherals/ubuntu-base/README.md). Там сохранены этапы создания rootfs, GRUB/initramfs, установки модулей и firmware, настройки Wi-Fi, SSH, DHCP и systemd. Скрипты рядом являются историческими версиями, не единым проверенным установщиком.

Пакеты устанавливать через epm. Для воспроизводимого нового образа надо зафиксировать пакетные версии, точный commit ядра, применённые патчи, .config, DTB и hashes firmware. Манифест этого репозитория фиксирует накопленные файлы, но не подменяет полностью воспроизводимую сборку с нуля.

## Конфигурация следующего ядра

- Сохранить рабочие UFS/PHY, USB, ath10k SNOC, remoteproc, RPMh, SCM, SMMU, DRM/MSM, ADSP и ASoC компоненты.
- Проверить CONFIG_OF_OVERLAY и CONFIG_OF_DYNAMIC для выбранного механизма диагностических наложений; наличие этих опций не означает наличие configfs-интерфейса overlays.
- Включение ACPI требует отдельно решить SCM DMA, Qualcomm SMMU и доступ к USB/сети; нельзя просто перенести флаг из другого ноутбука.
- Свести проверенные аудиопатчи и GPIO/питание в нормальные драйверы и DT, исключив зависимость от временных тестовых клиентов.

## Каталог патчей и история

[Патчи ядра](../patches/kernel/README.md) собраны отдельно с назначением, статусом и SHA256. В исходных рабочих деревьях Linux `.git` не было: отдельных локальных kernel-коммитов нет. Есть файлы патчей и коммиты их сохранения в этом репозитории. Полная последовательная серия для чистой базы ещё не сформирована.

Основные точки входа:

- [GLINK backport](../patches/kernel/audio/0001-glink-destroy-backport.patch) и [Q6AFE channel mask](../patches/kernel/audio/0002-q6afe-active-mask.patch).
- [WCD938x IRQ lifetime](../patches/kernel/audio/wcd938x-irq-lifetime.patch) и [Q6ASM повторный prepare](../patches/kernel/audio/q6asm-prepare-stopped.patch).
- [DPU encoder assignment diagnostic](../patches/kernel/graphics/dpu-assignment-diagnostic.patch).
- [ACPI-патчи и ограничения](acpi.md#прямые-ссылки-на-изменения-ядра).

Для применимости и порядка проверять статус в каталоге и первичный отчёт; эти ссылки не обозначают единую последовательную серию.
