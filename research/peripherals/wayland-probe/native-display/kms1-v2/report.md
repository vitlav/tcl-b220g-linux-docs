# Boot artifacts KMS1 handoff v2

2026-09-08. Подготовлены отдельные `/tmp/tcl-kms1-v2-artifacts/modules.tar.gz` и `initramfs.cpio.gz`. Сборка исходников не выполнялась, v1 artifacts не изменялись, подключений к TCL не было.

## Изменения

Копия `/tmp/tcl-kms1-artifacts/stage` создана в отдельном stage. Только `extra/tcl_lt8911_handoff.ko` заменён готовым `/tmp/tcl-kms1-handoff-v2/tcl_lt8911_handoff.ko`, затем `aarch64-linux-gnu-strip --strip-unneeded` и `/sbin/depmod -b STAGE 6.18.34-tcl-kms1` (exit0). После сравнения всех 963 файлов stage с v1 единственное различие — этот ko; metadata depmod побайтно совпала.

Initramfs создан неизменённым `kms1-boot/build-initramfs.py` с `/tmp/tcl-kms1-init/bin/busybox` и v2 stage/lib/modules. Modules tar содержит только lib/modules/6.18.34-tcl-kms1, нет build/source symlinks.

## Проверки

- Полный разбор newc после gzip: все 963 module/metadata файла побайтно совпали со stage, точное равенство наборов файлов. Внутри 949 ko.
- Все regular module paths принадлежат только release 6.18.34-tcl-kms1, старых release нет.
- Единственный handoff ko соответствует stripped v2; ни одна другая ko не изменилась относительно v1.
- init/shutdown/collect побайтно совпали с исходниками и предыдущим v1 initramfs; executable mode100755. BusyBox также совпал с указанным файлом и v1 initramfs.
- Modules tar полностью прочитан; набор и содержимое всех файлов совпали со stage/cpio; symlinks/hardlinks отсутствуют.
- Offline modprobe --show-depends для msm, tcl_lt8911_handoff, ath10k_snoc: exit0. Модуль vermagic `6.18.34-tcl-kms1 SMP preempt mod_unload aarch64`, depends пуст.

| Артефакт | Размер, байт | SHA256 |
|---|---:|---|
| modules.tar.gz | 12250923 | ee7fa426b46e41d80af700910e259403ba5ce3756c0caccba92ab725407499ea |
| initramfs.cpio.gz | 13087776 | 38ab5885c44d7a33828b552520441bd87a91da1472254aeabc8bf19f24fb416a |
| handoff v2 ko после strip | 9720 | cfff58ea5393414a835a741e589204c4bf2e58ae7d0868ad7777b03edf35ab0b |

Исходный unstripped v2 SHA был 3b9a0e24090f76527a86142f858e98ca70b9b2bb72d3444ceddbbfb4f0340401. Изменение SHA после strip ожидаемо; в tar и cpio лежит stripped вариант cfff58….

Машиночитаемые результаты: validation.json, SHA256SUMS; дополнительные modinfo.txt и offline-modprobe.log. Это проверка упаковки; аппаратный v2 тест ещё не выполнен.
