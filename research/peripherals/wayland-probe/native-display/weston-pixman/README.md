# Wayland на родном MSM DRM: подтверждено пользователем

2026-09-08. Пользователь: «всё работает!!» после запуска рабочего стола, терминала и weston-simple-shm. Kernel6.18.34-tcl-kms1, boot2256e057-bbfd-4c35-9264-e150d76a2321. После чистой перезагрузки SSH/Wi-Fi автоматически доступны на uptime32.5s. V2 installedKO SHA cfff58ea5393414a835a741e589204c4bf2e58ae7d0868ad7777b03edf35ab0b,initramfsSHA38ab5885c44d7a33828b552520441bd87a91da1472254aeabc8bf19f24fb416a.

MSM и bridgev2 загружены явно на uptime49.16s. Refcount/container WARN v1 не повторились; kerneltaint4096 означает только out-of-tree модуль, без WARNbit. fbdev сообщения FBINFO_VIRTFB и refgen dummy остаются отдельно. Драйвер handoff всё ещё использует настройки UEFI в мосте, cold-start/suspend/DPMS не реализованы и не проверены.

Weston14.0.2-5 установлен штатным Ubuntu пакетом через epm --auto install weston. Использованы DRM backend, Pixman renderer, /dev/dri/card0, eDP-1/crtc65,1920×1080@60Hz. Embedded seatd backend builtin создал VT-bound seat0 и предоставил session control. Libinput обнаружил встроенные I2C клавиатуру и тачпад. GPUdisabled; это программный рендеринг Wayland с аппаратным выводом через DPU, не ускоренный OpenGL/Vulkan.

Доказательства в logs/BOOTID/wayland/: weston.log содержит Using Pixman renderer, DRM supports atomic modesetting, Output eDP-1 enabled. DRM clients показывает weston master=y; systemd states compositor/terminal/simple-shm active/success. Пользователь визуально подтвердил работу. ps снимок во время анимации: weston7.2%CPU,~29MiBRSS, simple-shm3.8%CPU; это средняя загрузка по ps, не benchmark/FPS measurement.

## Воспроизведение диагностического сеанса

На чистой загрузке сначала убедиться в v2 и загрузить:
```sh
modprobe --all msm tcl_lt8911_handoff
```
Проверить dmesg на отсутствие refcount WARN. Скопировать weston.ini в /var/tmp/tcl-weston.ini; start.sh содержит фактически проверенную команду запуска transient systemd unit с приватным /run/tcl-weston. Это root diagnostic session, не настроенный display manager/login и не автозапуск после reboot.

Приложения:
```sh
systemd-run --unit=tcl-weston-terminal \
 --setenv=XDG_RUNTIME_DIR=/run/tcl-weston --setenv=WAYLAND_DISPLAY=wayland-tcl \
 /usr/bin/weston-terminal
systemd-run --unit=tcl-weston-shm \
 --setenv=XDG_RUNTIME_DIR=/run/tcl-weston --setenv=WAYLAND_DISPLAY=wayland-tcl \
 /usr/bin/weston-simple-shm
```

Работающий сеанс оставлен пользователю. Default GRUB по-прежнему KMS1-deferred: после reboot Wi-Fi/SSH стартуют сами, DRM/Weston пока запускаются явно. Старый I2C10 пункт и EFI backup сохранены. Следующие отдельные задачи: постоянный пользовательский графический вход, GPU acceleration, cold-start bridge/power management; не считать их решёнными этим тестом.
