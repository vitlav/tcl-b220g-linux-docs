# Compile-only проверка TCL LT8911EXB handoff

2026-09-08. Область: только компиляция одного `.o` на подготовленном qc7. На TCL действий не было. Исходник не изменялся.

## Команда и результат

Исходник и Makefile скопированы из `.claude/docs/tcl-b220g-data/peripherals/wayland-probe/native-display/handoff/` в `/tmp/tcl-handoff-compile/`.

```sh
make -C /tmp/tcl-qc7-build/linux-6.18.34 \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- \
  M=/tmp/tcl-handoff-compile W=1 tcl_lt8911_handoff.o
```

Exit 0, предупреждений нет. Полный лог: `build.log`. Получен ELF 64-bit LSB relocatable, ARM aarch64, not stripped.

SHA256 исходника: `279a657586cca17a49e9d7561556ff8f174609fe42eb47a3e1e72ba48f817fcc`.
SHA256 объекта: `d914cc8f66b0677041033170d6924b50ce37ed36f398506232b2f8df52950afc`.

`.ko`, MODPOST, kernel Image и пакеты не собирались. Kernelrelease не менялся, отсутствующий Module.symvers не подменялся. Подробные ограничения дерева: `/tmp/tcl-qc7-build-preparation.md`. Успех компиляции подтверждает совместимость использованных объявлений C/API в этом дереве; **не доказывает ABI-совместимость с работающим ядром и не разрешает загрузку**.

## Проверка API и lifecycle

- Сигнатуры bridge attach(mode encoder flags), mode_valid, get_modes, atomic state helpers и i2c probe соответствуют 6.18.34: проверены компилятором W=1.
- `of_graph_get_remote_node()` reference освобождается после поиска DSI host; при отсутствии host возвращается EPROBE_DEFER.
- Порядок регистрации bridge → DSI device → attach осмыслен: MSM dsi_host_attach вызывает component_add, который может сразу привести к поиску bridge через DT graph. Bridge уже зарегистрирован.
- Обратный devres cleanup: detach → DSI unregister → bridge remove → освобождение ctx. В проверенном MSM detach вызывает component_del, поэтому unregister bridge не предшествует component unbind. Очевидного use-after-free в этом порядке не найдено.
- Нулевой info.node у зарегистрированного DSI device сам по себе не является ошибкой: in-tree lt8912b использует тот же подход, MSM ищет следующий bridge через собственный DT port 1 endpoint 0. Должен быть корректный двусторонний DT graph.
- MSM использует DRM_BRIDGE_ATTACH_NO_CONNECTOR; отказ tcl_attach от самостоятельного legacy connector совместим с текущим MSM manager.
- Ошибки чтения/несовпадения регистров прерывают probe; попытка финального bank81 выполняется и при ошибке. Ошибка финального выбора не игнорируется. Это выбор конечного банка, не восстановление неизвестного исходного.
- Все I2C writes исходника — только byte-data FF; проверка I2C_FUNC_SMBUS_BYTE_DATA разрешает штатную эмуляцию SMBus через I2C адаптером.

## Ограничения диагностической заготовки

1. Gate проверяет ID и выбранные timing registers один раз до DSI attach. Он не проверяет успешный link training/PLL lock и не выполняется повторно после MSM reprogramming; его успех не доказывает работоспособность экрана после modeset.
2. Нет enable/disable, suspend/resume и восстановления LT8911EXB. Это сознательно ограниченный handoff; runtime suspend, DPMS blank/unblank и повторные modeset не следует считать поддержанными.
3. Native MSM перезапускает DSI/PHY и может удалить EFI framebuffer независимо от отсутствия PLL/reset writes в этом модуле. Не считать тест пассивным чтением.
4. Последовательность bank select + read не защищена от сторонних принудительных i2c-tools/I2C_RDWR обращений. На время probe исключить параллельные userspace bank probes; обычное kernel ownership само по себе не предотвращает принудительные raw обращения.
5. Регуляторы/PHY supplies, DSC/DSI flags, размер панели и полярности не подтверждены данной компиляцией. Continuous clock остаётся явно указанным экспериментальным выбором.
6. Для аппаратного теста всё ещё нужны Module.symvers/согласованная сборка, проверка DT и отдельный рабочий путь SSH/возврата. Этот отчёт не является результатом аппаратного испытания.

Исправления исходника: нет; diff отсутствует.
