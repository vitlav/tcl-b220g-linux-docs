# Согласованная диагностическая сборка 6.18.34-tcl-kms1

Дата: 2026-09-08. Статус: Image + modules и отдельный handoff module успешно собраны (exit 0).

Это новый комплект kernel + modules. Он не предназначен для подмешивания в работающий qc7. Установок на TCL/USB и пакетов в систему не выполнялось.

## Входы

Дерево `/tmp/tcl-qc7-build/linux-6.18.34` реконструировано ранее по qc7, отчёт `/tmp/tcl-qc7-build-preparation.md`. Два hunks travmurav были применены с явно описанным fuzz1; остальные fuzz0. Полные SHA и patch logs сохранены.

Toolchain: cross GCC15.3.1-alt4, binutils2.46; это отличается от оригинального qc7 GCC14.2/binutils2.44, но для нового ядра и его модулей используется согласованно. Исходный `config.original`, подготовленный `config.before-kms1`, финальный `config.kms1` и `logs/kms1-config.diff` сохранены.

Перед запуском: /tmp74ГБ свободно, RAM19ГБ available, CPU32. Выбрана умеренная параллельность -j8. QFPROM=y, QCOM_APCS_IPC=y, BTRFS_FS=y, ATH10K_SNOC=m, DRM_MSM=m, MSM_DSI_10NM_PHY=y, FB_EFI=y, SIMPLEDRM=n. Изменение относительно подготовленного конфига — только LOCALVERSION: -stb-qc7 → -tcl-kms1.

## Команды

```sh
cd /tmp/tcl-qc7-build/linux-6.18.34
cp .config /tmp/tcl-qc7-build/config.before-kms1
scripts/config --set-str LOCALVERSION -tcl-kms1
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olddefconfig
cp .config /tmp/tcl-qc7-build/config.kms1
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j8 Image modules
```

olddefconfig завершился успешно. Проверенный kernel.release: `6.18.34-tcl-kms1`.
Логи: `logs/kms1-olddefconfig.log`, `logs/kms1-image-modules.log`.

После успешной полной сборки планируется обычная сборка external handoff module с настоящим Module.symvers. MODPOST warnings/отсутствующие экспорты подавляться не будут.

## Результат полной сборки

`make Image modules` exit 0. Созданы ARM64 Image, настоящий Module.symvers, System.map и 948 in-tree `.ko`.
Всего 4 предупреждения unused variables ec/ret в `drivers/platform/arm64/acer-aspire1-ec.c:472–487` из исходного qc7-патча. Ошибок нет; предупреждения не подавлялись, код не исправлялся. DTBs не собирались.

В чистый `/tmp/tcl-kms1-handoff` скопирован проверенный handoff source/Makefile. Затем:

```sh
make -C /tmp/tcl-qc7-build/linux-6.18.34 \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- \
  M=/tmp/tcl-kms1-handoff W=1 modules
```

Exit 0; обычный MODPOST выполнен с настоящим Module.symvers от только что собранного ядра. Предупреждений/неопределённых экспортов нет. Лог `logs/kms1-handoff-module.log`.

Оба проверенных модуля (`msm.ko` и `tcl_lt8911_handoff.ko`) имеют:

```text
vermagic=6.18.34-tcl-kms1 SMP preempt mod_unload aarch64
```

У handoff `depends=` пуст: используемые им core DRM/I2C API в этой конфигурации встроены; это не означает, что дисплей работает без MSM. Полный modinfo: `logs/kms1-modinfo.txt`. На builder modinfo не в PATH, использован `/sbin/modinfo`; пакеты не устанавливались.

SHA256 артефактов: `kms1-SHA256SUMS`:

```text
463ad3972650ac6d7e21d532b98325c149ba738f498850a523a5715411d5a383  /tmp/tcl-qc7-build/linux-6.18.34/arch/arm64/boot/Image
207e529aa7d2568161675e7295a3598aa64f7b68edc8c677b0efeeb1ce1f2db9  /tmp/tcl-qc7-build/linux-6.18.34/Module.symvers
c5e20ed6ca4e8db3a6d63a7e4ab44e90cd24fc9218b8e54b379965d1c28e5c69  /tmp/tcl-qc7-build/linux-6.18.34/System.map
6cc65cd2bb2e93a427d39a1106e0984f4af7d2885fc78c9c82b0f56f5780cc21  /tmp/tcl-qc7-build/config.kms1
166e078007888e6712398255cfe1cab5f88d8d8e32ec3b85ff96fe98038b3642  /tmp/tcl-kms1-handoff/tcl_lt8911_handoff.ko

```

## Практическая граница

Комплект согласован внутри новой сборки по kernelrelease/config/toolchain и проверен MODPOST. Он пока не испытан на TCL. Для загрузки нужны отдельные каталог modules, initramfs с проверенными зависимостями USB/Wi-Fi, DTB и сохранённый рабочий GRUB пункт. Ничего из этого сборочный агент не устанавливал. Существующие qc7 модули и бинарник не заменялись.

## Staging и архив модулей

`make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=/tmp/tcl-kms1-artifacts/stage INSTALL_MOD_STRIP=1 DEPMOD=/sbin/depmod modules_install` exit0. Использован cross strip (Kbuild STRIP от CROSS_COMPILE, --strip-debug). Handoff ko скопирован в extra и обработан явно `aarch64-linux-gnu-strip --strip-debug`.

build/source symlinks удалены только из staging перед архивом, ссылки на /tmp в архив не попали. `/sbin/depmod -b /tmp/tcl-kms1-artifacts/stage 6.18.34-tcl-kms1` exit0.

13 offline проверок `/sbin/modprobe -d STAGE -S 6.18.34-tcl-kms1 --show-depends MODULE` прошли: msm, tcl_lt8911_handoff, ath10k_snoc, phy_qcom_qusb2, dwc3, dwc3_qcom, xhci_hcd, xhci_plat_hcd, usb_storage, nvmem_qfprom, qcom_hwspinlock, qcom_apcs_ipc_mailbox, ufs_qcom. Полный вывод `logs/kms1-offline-modprobe.log`; builtin поставщики распознаны как builtin, отдельные modules разрешены из stage. Реальная загрузка не выполнялась.

Таблица сравнения критичных CONFIG с исходным рабочим qc7: `kms1-critical-config.md`. Все перечисленные USB/Wi-Fi/UFS/mailbox/hwspinlock/QFPROM/provider значения совпадают.

Архив `/tmp/tcl-kms1-artifacts/modules.tar.gz`: 949 ko, 12248510 байт.
SHA256: `5004e5ab8a157e6fa9cef2bff9cd8d83d16927ab8bffea327960330e2b0a593d`.
Проверена читаемость tar, все пути начинаются с lib/modules/6.18.34-tcl-kms1, symlinks/hardlinks отсутствуют. На TCL и USB ничего не установлено.
