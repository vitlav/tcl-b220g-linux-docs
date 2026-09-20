# ACPI: Linux 7.2.4 для TCL B220G

Здесь опубликована воспроизводимая серия из **70 логических патчей** к Linux stable **v7.2.4**. Базовый commit — `5015d0d945b3d3f2b038d2667880d5762f7d9437`, итоговое дерево — `885e8419bbeebe4f34b47d7e692311e301004167`.

Это точное дерево аппаратно проверенного ядра `7.2.4-tcl-acpi-repro1+`. Оно загрузило установленную ALT Linux с UFS, Wi-Fi, встроенной клавиатурой и тачпадом, USB-камерой, eDP/DPU, Adreno и pstore/ramoops. Согласованный внешний LPI provider поднял оба WCD9385 SoundWire slave; пользователь подтвердил стереозвук. При проверке не было Oops, trace, refcount warning, runtime-PM warning или synchronous abort.

## Происхождение и границы

Серия строится непосредственно на Linux stable v7.2.4. Предварительно применять qc7 или DT-патчи не требуется. Перенесённые решения qc7 сохранены с исходным авторством; их происхождение разобрано в [отдельной таблице](../../../research/qc7-provenance/acpi-attribution.md).

70 файлов — логическое представление конечного результата, а не журнал экспериментов. Последовательные правки одного механизма объединены, если промежуточные состояния не несут самостоятельной функции. Независимые изменения разных подсистем оставлены раздельно. Каждый patch содержит описание причины, механизма и области действия.

## Воспроизведение

[series](patches/series) задаёт порядок, [commits.txt](commits.txt) фиксирует SHA логических коммитов, [manifest.json](manifest.json) содержит базу, целевое дерево, контрольные суммы и результат аппаратной проверки. Применение всех патчей к чистой базе повторно проверено в отдельном checkout: получен ровно tree `885e8419bbeebe4f34b47d7e692311e301004167`.

```sh
patch_dir=/path/to/tcl-b220g-linux-docs/patches/kernel/acpi
git switch -c tcl-acpi 5015d0d945b3d3f2b038d2667880d5762f7d9437
while IFS= read -r patch_name; do
    git am "$patch_dir/patches/$patch_name"
done < "$patch_dir/patches/series"
test "$(git rev-parse HEAD^{tree})" = 885e8419bbeebe4f34b47d7e692311e301004167
cp "$patch_dir/kernel.config" .config
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olddefconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j16 Image modules
```

Используйте приложенную фактическую конфигурацию. Image, встроенные модули, [внешние аудиомодули](modules/README.md) и initramfs должны быть собраны для одного release. Системная настройка находится в [ACPI root overlay](../../../system/acpi/root-overlay/README.md).

## Что остаётся проверить

- повторные cold boot и system suspend/resume;
- захват микрофона, гарнитуру и датчики;
- длительные циклы аудио, GPU и видеодекодирования;
- устранение щелчка первого включения усилителя;
- штатную интеграцию board-specific питания и software-node устройств.

Q6AFE hardware vote/devote остаётся известным протокольным ограничением: firmware отвечала status `0x16` (`Unknown cmd`) на ранний vote и поздний devote. Патчи сохранены в точной серии, поскольку они входят в проверенное дерево и нужны для его побайтного воспроизведения; это не означает, что firmware lifecycle исправлен. Подробности и обязательная проверка слышимого результата приведены в [описании ACPI-аудио](../../../docs/hardware/audio-acpi.md).
