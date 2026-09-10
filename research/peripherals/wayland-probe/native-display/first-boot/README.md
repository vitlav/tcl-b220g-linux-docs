# Первый native DRM на TCL B220G

2026-09-08. Kernel6.18.34-tcl-kms1, boot eb351fb4-4b70-454a-8425-1047c5c4c9aa. Пользователь загрузил предпоследний пункт (default tcl-kms1-deferred), сообщил о чёрном экране. SSH вернулся: на uptime66.5s Ubuntu running, Wi-Fi5220MHz автоматически, tcl-radio/ssh.socket/collector active. Caps Lock реагирует на нажатие; первоначальная фраза «мигает» уточнена пользователем, самопроизвольное мигание не наблюдалось.

MSM/handoff отсутствовали в /proc/modules до явного теста; runtime blacklist отработал. EFI fb ещё зарегистрирован, но изображение чёрное. Причина исчезновения firmware scanout пока не доказана; DISPCC builtin probe программирует PLL и clocks, поэтому отсутствие загруженного MSM не гарантирует сохранения картинки UEFI.

На uptime112.76s выполнен modprobe msm через systemd-run oneshot (TimeoutStartSec30), result success. DSI/PHY bound, refgen отсутствует → dummy regulator. На uptime222.67s явно загружен handoff. ID/timing gate прошёл (иначе probe не продолжился бы). Зарегистрированы DRM card0, eDP-1, Writeback-1, renderD128; no GPU device found, как ожидается при disabled GPU. На uptime222.74s fb0 стал msmdrmfb.

**Пользователь подтвердил появление изображения.** `modetest -M msm -c -p` без modeset: connector34/eDP-1 connected, encoder33, CRTC65/FB67, один активный режим1920×1080 60.00Hz,142520kHz,+H/+V. connected/link-statusGood у данного фиксированного handoff не является независимой проверкой физического eDP link. Появление renderD128 не доказывает GPU acceleration.

## Ошибка v1 и исправление v2

Исходный handoff ошибочно использовал devm_kzalloc для контейнера bridge. drm_bridge_add в этом дереве требует devm_drm_bridge_alloc, инициализирующий container/kref. В журнале DRM bridge corrupted/not allocated, refcount addition on0/use-after-free warning, затем saturation/leaking memory. Это ошибка нашей заготовки, пропущенная compile-only/API review; не трактовать работающую картинку как корректность lifecycle.

V1 сохранён в ../handoff/first-test-v1.c. Основной исходник исправлен на devm_drm_bridge_alloc; v2 W=1/MODPOST exit0, SHA3b9a0e24090f76527a86142f858e98ca70b9b2bb72d3444ceddbbfb4f0340401. Исправление пока **не загружалось на TCL**. Не выполнять unload/rebind старого объекта с насыщенным refcount ради горячей замены; отсутствие warning v2 проверить при следующей чистой загрузке.

Другие сообщения: sys_imageblit/sys_fillrect framebuffer not in virtual address space — msm_fbdev не выставляет FBINFO_VIRTFB; sys_imageblit после предупреждения всё равно выполняет рисование. Это отдельный fbdev вопрос, не исправление bridge. Также сохранены refgen dummy, прежние firmware/auxbridge сообщения и loop0 DISCARD unsupported. Rootfs, Wi-Fi и SSH продолжают работать; panic не было. DPMS/suspend/cold-start моста не проверены.

Полные исходные снимки и журнал находятся в подкаталоге boot ID. /var/log/tcl-kms1 и /tcl-kms1/logs на USB реально содержат ранние логи и snapshots, collector не был потерян. Новые режимы, GPU или Wayland в этом тесте не запускались.
