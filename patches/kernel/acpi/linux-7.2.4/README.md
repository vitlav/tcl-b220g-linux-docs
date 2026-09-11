# ACPI: Linux 7.2.4 для TCL B220G

Самостоятельный набор из **16 патчей** к официальному Linux stable **v7.2.4**, базовый коммит `5015d0d945b3d3f2b038d2667880d5762f7d9437`. Проверено на ноутбуке 11 сентября 2026 года: загрузка через ACPI без DT, USB с записью и проверкой логов, встроенный Wi-Fi 5 ГГц и SSH. I2C HID-клавиатура зарегистрирована; физические события клавиш отдельно не проверены.

Патчи qc7, DT и другие ACPI-патчи из соседних каталогов перед этой серией применять не нужно. Это полный исходный набор проверенной базовой ACPI-сборки; поддержка нативной графики, видеодекодирования и звука через ACPI ещё не включена. Рабочий DT-комплект ведётся отдельно.

## Файлы и воспроизведение

- [series](series) — точный порядок применения.
- [commits.txt](commits.txt) — полные SHA и названия настоящих Git-коммитов.
- [manifest.json](manifest.json) — база, итоговый коммит, SHA256 патчей и проверенного комплекта.
- [kernel.config](kernel.config) — фактическая конфигурация сборки; последний конфигурационный патч также добавляет `tcl_acpi_defconfig`.
- [Проверка воспроизведения](patch-replay.txt) — применение всей серии к чистому тегу даёт в точности дерево проверенной ветки.

В чистом checkout официального тега `v7.2.4`:

```sh
git switch -c tcl-acpi
patch_dir=/path/to/tcl-b220g-linux-docs/patches/kernel/acpi/linux-7.2.4
# Выполнить в отдельном shell: любая ошибка должна остановить применение.
(
    set -eu
    while IFS= read -r patch_name; do
        git am "$patch_dir/$patch_name"
    done < "$patch_dir/series"
)
```

После успешного применения всей серии:

```sh
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- tcl_acpi_defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j16 Image modules
```

Фактическая версия проверенной сборки — `7.2.4-tcl-acpi-mainline1+`. Суффикс `+` создаётся правилами версии при сборке из Git; Image, модули и initramfs должны быть от одной сборки. Версия сама по себе не является хешем содержимого: точные хеши комплекта указаны в манифесте. `git am` сохраняет авторство и сообщения, но может изменить SHA коммитов из-за метаданных коммиттера.

## Что проверено

| Подсистема | Подтверждённый результат |
|---|---|
| Ядро | Исправленный Image опознан по совпадению kernel notes с ELF сборки; 8 CPU |
| ACPI | Живой Device Tree отсутствует; SCM и SMMU инициализированы |
| USB | Флешка доступна, диагностические снимки записаны; SHA256 проверены после повторного монтирования |
| Wi-Fi | WCN3990, связь 5220 МГц, DHCP, SSH; 3 из 3 ping шлюза успешны |
| Модем | MPSS remoteproc в состоянии running; транспорт для Wi-Fi работает |
| Клавиатура | QTEC0001 / 0CF2:9020, I2C5, FIFO16; HID-устройство зарегистрировано |
| Экран | EFI framebuffer; `/dev/dri` пока отсутствует |

Полная ARM64-сборка Image/modules и modules_install прошла. Затронутые объекты собраны с W=1. Регрессионные проверки I2C и SCM/reset прошли. Проверены vermagic всех 953 файлов модулей; depmod с System.map не выявил неразрешённых символов. Через QEMU выполнены 14 проверок ARM64 userspace, включая BusyBox, разрешение зависимостей модулей, WPA/iw, SSH и blkid.

## Условия загрузки

Использован диагностический initramfs с OEM firmware, RMTFS/TFTP, настройками Wi-Fi, DHCP, SSH и сохранением снимков на USB. Патчи ядра не заменяют эти файлы и службы. Приватный initramfs с ключами и паролями в Git не публикуется. Ubuntu rootfs для этой проверки не использовался.

Командная строка проверенного пункта:

```text
acpi=force acpi=nospcr console=tty0 earlycon=efifb efi=novamap,noruntime,debug clk_ignore_unused pd_ignore_unused regulator_ignore_unused maxcpus=8 cpuidle.off=1 nokaslr loglevel=8 ignore_loglevel keep_bootcon panic=0 rdinit=/init initcall_debug usbcore.autosuspend=-1
```

Диагностические параметры и сохранение состояния clocks/power/PHY firmware ещё требуются; этот результат не подтверждает suspend, энергосбережение, полноценную Ubuntu через ACPI или весь мультимедийный тракт. Поддержка остальных I2C-контроллеров и touchpad не заявлена.
