# Автоматическая native-консоль при загрузке

Проверено 2026-09-08, boot `0b07456b-9f44-48c9-a92e-8ea948e2441b`. Пользователь подтвердил: «да, быстро» — длительного чёрного экрана больше нет.

Новый основной пункт GRUB `tcl-kms1-early` передаёт `tcl.kms=early` и использует `/tcl-kms1/initramfs-early.cpio.gz`. В initramfs явно загружаются MSM и handoff v2 до поиска USB/root image и systemd. Ядро, DT, остальные модули и firmware прежние. Ранние отметки и /proc/fb пишутся в `/run/kms1-log/early-kms.txt`, затем сохраняются на USB и rootfs.

Журнал ядра: MSM initialized на 1.509s, fb0 msmdrmfb на 1.566s; /proc/fb после modprobe уже показывает native framebuffer. Между uptime в early-kms.txt (1.16→1.31s) и ранними printk-метками есть смещение, причина отдельно не проверялась; для сравнения используется последовательность dmesg. Раньше ручной старт происходил на 49s и 222s в разных тестах. Wi-Fi и SSH вернулись автоматически, проверены на uptime35.94s. Refcount warning не повторился.

EFI SHA256 `2fd1dc6746c0660e40447e1a31ad30a58705820b4c723d0dbb108623e62b1af0`; initramfs SHA256 `0a360fbe68a1de564bd1587e337f8099d56c346d3a8da0c4547802ce5153d7a7`. Backup предыдущего EFI: `EFI/BOOT/BOOTAA64-before-early.bak`, SHA `9617e47d43a1531b055af7acf5a783f77dae8f078a6c678cf95a453e2928159b`.

Прежние deferred/control/I2C10 пункты сохранены. Они используют свои прежние initramfs, отложенный старт остаётся доступен для диагностики. Новый boot kit проверен: 949 модулей соответствуют v2 staging, исходники init/shutdown/collect сверены, BusyBox sh -n на TCL и GRUB 2.12 script-check/ARM64 check прошли.

Weston/Pixman и терминал после проверки восстановлены, оба service active/success. **Автоматически при загрузке пока появляется консоль, а не Weston**: графический display manager/user login отдельно ещё не настроен. GPU пока disabled, 3D не проверено. Мост остаётся firmware handoff, а не полным cold-start драйвером.
