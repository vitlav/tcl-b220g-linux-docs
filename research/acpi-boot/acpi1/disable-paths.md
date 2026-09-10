# Где ACPI может отключиться после раннего старта

Проверено по Linux6.18.34-tcl-acpi1. parse_acpi(force) и parse_acpi(nospcr) независимы. В acpi_boot_table_init при force даже ошибка table_init/FADT не вызывает disable_acpi. Это НЕ предотвращает поздний disable_acpi.

- acpi_early_init: acpi_reallocate_root_table или acpi_initialize_subsystem могут завершиться ошибкой; ветка error0 отключает ACPI.
- acpi_subsystem_init: ошибка acpi_enable_subsystem вызывает disable_acpi.
- acpi_init: ошибка acpi_bus_init освобождает acpi_kobj и отключает ACPI. Там могут отказать acpi_load_tables, enable_subsystem, initialize_objects, установка уведомлений и регистрация шины.

Именно эти сообщения надо получить из начала dmesg. Отсутствие /sys/firmware/acpi/tables в конце загрузки не различает эти пути. Новая экранная диагностика сохраняет EFI systab, список корневых DT-узлов и автоматически показывает отфильтрованный ранний журнал по18строк/20секунд. Она подготовлена локально, но не установлена, поскольку ноутбук недоступен поSSH и в RAMтесте нетклавиатуры/USB. Нужен возврат в DT физической перезагрузкой.
