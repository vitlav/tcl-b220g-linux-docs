# Handoff v2: корректное выделение DRM bridge

2026-09-08. Сборка v2 успешно завершена; установка и аппаратная проверка v2 не выполнялись. Работающий v1 и исходники ядра не менялись.

## Исправление и признанный пропуск review

В v1 использован devm_kzalloc и ручная установка funcs. В нашем ядре drm_bridge_add() требует devm_drm_bridge_alloc(), проверяет container и затем drm_bridge_get() увеличивает refcount. Нулевая память kzalloc не инициализирует kref: в v1 вызов попадает на нулевой refcount, вызывая предупреждения и saturation. Предыдущий lifecycle review проверил обратный devres порядок, но пропустил этот обязательный invariant. Успешная компиляция W=1 и MODPOST такую ошибку не обнаруживают.

Изменение v2 строго одно: devm_drm_bridge_alloc(dev, struct tcl_handoff, bridge, &tcl_bridge_funcs) и обработка IS_ERR/PTR_ERR вместо devm_kzalloc/null. Полный diff `v1-to-v2.diff`. Мною исходник дополнительно не редактировался.

## Сборка

Исходник и Makefile скопированы в отдельный `/tmp/tcl-kms1-handoff-v2`.

```sh
make -C /tmp/tcl-qc7-build/linux-6.18.34 \
 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- \
 M=/tmp/tcl-kms1-handoff-v2 W=1 modules
```

Exit0, предупреждений нет, обычный MODPOST с настоящим Module.symvers от согласованного kms1. Лог `build.log`. Kernelrelease и Module.symvers не изменялись. Модуль не stripped (дополнительных преобразований не поручено).

```text
3c39c8f00546bffb7dbd6cc1b0c46db382d6631d0f2eef15298c85ac82cb8446  tcl_lt8911_handoff.c
3b9a0e24090f76527a86142f858e98ca70b9b2bb72d3444ceddbbfb4f0340401  tcl_lt8911_handoff.ko

```

Полный `modinfo.txt`: vermagic `6.18.34-tcl-kms1 SMP preempt mod_unload aarch64`, depends пуст. Новый экспорт __devm_drm_bridge_alloc разрешён штатным MODPOST.

## Пересмотр lifecycle по реализации ядра

- __devm_drm_bridge_alloc (drivers/gpu/drm/drm_bridge.c:259) выделяет container через kzalloc, назначает container/funcs, выполняет kref_init(1) и регистрирует devm action drm_bridge_put. При ошибке регистрации action вызов or_reset освобождает initial reference.
- devm_drm_bridge_add → drm_bridge_add добавляет отдельную registry reference через drm_bridge_get; его devm action drm_bridge_remove снимает registry reference. При ошибке регистрации action remove выполняется немедленно, остаётся alloc reference до devres cleanup.
- drm_bridge_attach берёт собственную reference и освобождает её на error paths либо при detach. Atomic state helpers в заготовке соответствуют private-state lifecycle attach/detach. Самостоятельно вызывать drm_bridge_detach из этого I2C драйвера не нужно: это часть encoder cleanup.
- Порядок devres теперь: DSI detach/component_del → unregister DSI device → remove bridge из global list и put registry ref → put initial allocation ref. Финальный container освобождается только при последнем kref_put, не ранним devm_kfree; это исправляет также ошибочный lifetime подхода v1.
- ctx содержит только drm_bridge, нет указателей на отдельно devm-выделенные данные. Отложенное освобождение container не оставляет таких dangling данных. funcs находится в модуле; hot-unbind/module unload не проверены аппаратно и не объявляются поддержанными данным обзором.
- OF host reference по-прежнему освобождается, bridge добавляется до component-triggering DSI attach. Gate, I2C bank handling и timings не изменены.

Границы прежние: только firmware handoff, нет cold start/suspend/link recovery. Нужна последующая аппаратная проверка v2 без DRM/refcount warnings; успех v1 с изображением не подтверждает исправление v2. Ничего на TCL/USB и в staging/archive v1 не изменялось.
