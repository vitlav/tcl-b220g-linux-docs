# Подготовка исходников qc7 для внешнего модуля

Дата: 2026-09-08. Результат: подготовительные цели успешно выполнены, но дерево **не подтверждено как ABI-совместимое с работающим ядром**. Никаких установок, загрузки модулей, перезагрузок или операций на TCL не было. Всё создано в `/tmp`.

## Область применения правил

Прочитаны `/srv/lav/Projects/Claude/alt-packaging-agents/.claude/agents/build.md`, AGENTS.md, config.md, docs/{commands,gear-rules,spec-format,changelog,workflow-new-package,common-mistakes,rules}.md и шаблон C/C++. Это подготовка Kbuild внешнего модуля по отдельному поручению, не импорт нового RPM/gear: rpmgs/rpmbb/rpmbsh, сборка RPM и отправка girar не выполнялись.

## Источники и команды

Источник: `/tmp/tcl-qc7-source/`, рецепт `readme.qc7`. Архив 154486284 байт; `xz -t linux-6.18.34.tar.xz` завершился с кодом 0. Это проверка целостности сжатого потока, не проверка подписи upstream. SHA256 всех входов: `/tmp/tcl-qc7-build/source-sha256.txt`.

```sh
mkdir -p /tmp/tcl-qc7-build/logs
tar -xJf /tmp/tcl-qc7-source/linux-6.18.34.tar.xz -C /tmp/tcl-qc7-build
cd /tmp/tcl-qc7-build/linux-6.18.34
patch --batch --forward --fuzz=0 -p1 < /tmp/tcl-qc7-source/misc.qc7/patches/v6.18/travmurav-changes-v6.12.patch
```

Строгая первая попытка вернула 1: два rejects. Они сохранены в `logs/` и в дереве; это исторические rejects первой попытки, не оставшиеся неприменённые hunks. После чтения контекста выполнены dry-run, затем явное применение только этих двух rejects с `patch --batch --forward --fuzz=1 -p0 -i PATH.rej`.

| Патч / hunk | Результат |
|---|---|
| travmurav, dpu_plane.c | fuzz 1, offset +315: функция стала dpu_plane_init_common; нужная замена 0xff на possible_crtcs остаётся применимой |
| travmurav, pinctrl-sc7280-lpass-lpi.c | fuzz 1, offset +3: крайний контекст remove_new заменён upstream на remove; добавлены PM ops |
| Остальные hunks travmurav | fuzz 0, некоторые offsets; полный лог `logs/1-travmurav-changes-v6.12.patch.log` |
| add-new-dts-files-to-makefile | fuzz 0, exit 0 |
| add-gamma-lut-support | fuzz 0, offsets +1/+4, exit 0 |
| fix-version | fuzz 0, offset +4, exit 0 |

Порядок рецепта сохранён: travmurav; копирование `galaxy-book-go.dts` в `arch/arm64/boot/dts/qcom/sc7180-samsung-galaxy-book-go.dts`; Makefile patch; gamma patch; fix-version. Default patch автора допускает fuzz 2; здесь допущены только описанные два fuzz 1. Это реконструкция по опубликованному рецепту, не доказательство тождества исходникам бинарного qc7.

Вместо повторного merge давно меняющихся фрагментов использован предоставленный финальный config qc7. Он сохранён побайтно в `config.original` и передан в `.config`.

```sh
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olddefconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules_prepare
```

Обе цели exit 0. Полные stdout/stderr: `logs/olddefconfig.log`, `logs/modules_prepare.log`. Финальная конфигурация `config.prepared`; diff с оригиналом `logs/config-after-olddefconfig.diff`. Kernel Image, DTBs, полное ядро и модули не собирались; modules_prepare создал необходимые host tools, generated headers и служебные объекты/vDSO.

## Несовпадения и блокеры

1. Оригинал: GCC `(Debian 14.2.0-19) 14.2.0`, GNU assembler/linker 2.44. Здесь: cross GCC `(15.3.1-alt4) 15.3.1 20260804`, binutils 2.46. olddefconfig изменил compiler feature symbols; обнаружил Rust 1.98/LLVM22 вместо отсутствовавшего Rust; включил CONFIG_GCC_PLUGINS=y. Эти отличия не скрывались, .config после prepare не выдаётся за оригинальную.
2. `include/config/kernel.release` = `6.18.34-stb-qc7`, тогда как работающий бинарник `6.18.34-stb-qc7+`. Архивное дерево без git metadata не воспроизводит SCM suffix. Суффикс не подставлялся вручную ради прохождения vermagic; нужно документировать происхождение и осмысленно настроить локальную версию в отдельной сборке.
3. `Module.symvers` отсутствует: modules_prepare его не создаёт. CONFIG_MODVERSIONS=n, но полноценный MODPOST всё равно требует таблицу экспортов/namespace для проверки зависимостей. Нужен оригинальный build/header artifact с таблицей символов либо отдельная полная воспроизводимая сборка. Не использовать KBUILD_MODPOST_WARN для сокрытия отсутствующей таблицы.
4. CONFIG_MODULE_SIG=n и CONFIG_RANDSTRUCT_NONE=y в исходном config; это не является доказательством совместимости.
5. До сборки загружаемого внешнего LT8911EXB нужны сверка реального vermagic/экспортов, подходящий toolchain, проверка структуры API и отдельное ревью драйвера. Подготовленное дерево уже пригодно для чтения API и предварительной compile-only проверки, но **загрузка результата на ноутбук пока не обоснована**.

## Следующий конкретный шаг

Найти опубликованные автором headers/build artifacts именно `6.18.34-stb-qc7+` (Module.symvers, generated headers, compiler provenance) либо подготовить согласованный воспроизводимый kernel+module комплект. Отдельно оценить, затрагивают ли изменения compiler config ABI используемых драйвером структур; одинаковой строки vermagic недостаточно.
