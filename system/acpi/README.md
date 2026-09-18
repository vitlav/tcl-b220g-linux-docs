# Системные настройки ACPI

[Ранняя SSDT](early-ssdt/README.md) добавляет обязательные свойства ACPI и
полученный из Qualcomm DPP заводской WLAN MAC до перечисления устройств.

[root-overlay](root-overlay/README.md) содержит службы и скрипты запуска аудио. Они требуют [ACPI-ядра](../../patches/kernel/acpi/README.md), [внешних модулей](../../patches/kernel/acpi/modules/README.md), firmware и сохранённого mixer state. Это аудиочасть системной настройки, а не полный установочный образ с Wi-Fi, DHCP и SSH.

Файлы root-overlay устанавливаются с сохранением путей. Секреты Wi-Fi и SSH в репозиторий не входят. Системные зависимости и дальнейшая интеграция описаны в [плане поддержки](../../docs/roadmap.md).
