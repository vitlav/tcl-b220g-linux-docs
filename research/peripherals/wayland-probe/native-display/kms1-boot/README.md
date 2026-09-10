# KMS1 boot kit: результат первого теста

Комплект установлен и успешно загрузился на TCL (boot eb351fb4-4b70-454a-8425-1047c5c4c9aa). Wi-Fi/SSH стартовали автоматически; до явного modprobe MSM/handoff экран оставался чёрным. После загрузки обоих модулей пользователь подтвердил изображение, DRM/msmdrmfb1920×1080@60Hz.

**Этот initramfs и архив modules.tar.gz содержат handoff v1 с обнаруженной ошибкой инициализации bridge refcount.** Не считать их окончательным комплектом для постоянной эксплуатации. Исправленный v2 source/module проверен компиляцией отдельно, но ещё не интегрирован и не испытан на TCL; см. ../handoff/v2-report.md и ../first-boot/README.md. Для следующего чистого теста требуется обновить модуль в rootfs и initramfs согласованно, не смешивать отчёты v1/v2.

Текущий default GRUB: tcl-kms1-deferred (предпоследний пункт), последний tcl-kms1-control со старым DT. Рабочий старый пункт tcl-i2c10 сохранён. Backup EFI: EFI/BOOT/BOOTAA64-before-kms1.bak, SHA c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52.

Логи реально пишутся в /var/log/tcl-kms1/BOOTID и на USB /tcl-kms1/logs/BOOTID. До появления USB доступны только /run/kms1-log (RAM), при poweroff они теряются. Runtime blacklist в /run/modprobe.d блокирует автозагрузку MSM/handoff; явный modprobe разрешён. После чистой загрузки автоматически DRM пока не включается.
