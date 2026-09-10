# Armbian как база для TCL / native Wayland

Проверено через gh 2026-09-08. Пользователь предложил более новое ядро Armbian; simpledrm отложен.

Кандидат — семейство **uefi-arm64**, не spacemit. config/sources/families/include/uefi_common.inc задаёт current6.18, edge7.2, bleedingedge7.3. Это настройки сборочного дерева на момент чтения, не подтверждение доступности/работоспособности готового пакета на TCL. Наша текущая версия6.18.34 также относится к6.18, поэтому current не обязательно новее по версии minor.

В linux-uefi-arm64-current.config присутствуют ARCH_QCOM=y, ATH10K_SNOC=m, SCSI_UFS_QCOM=m, QCOM_Q6V5_MSS=m, QCOM_RMTFS_MEM=m, ARM_QCOM_CPUFREQ_HW=m, DRM_MSM=m, BTRFS_FS=m. Это обнадёживает, но полная конфигурация зависимостей ещё не проверена. Для rootfs на Btrfs loop/USB модули нужно включить в новый initramfs. Существующий initramfs с модулями6.18.34-stb-qc7+ использовать с другим ядром нельзя без пересборки состава.

Сохранить проверенный TCL DTB как исходный материал, затем проверить bindings/совместимость с выбранной версией. Сохранить OEM firmware/WPA/RMTFS, параметры boot и RO UFS. Начать проверку нового ядра при выключенных MDSS/GPU, проверить USB/HID/UFS/Wi-Fi/cpufreq, только затем добавлять дисплейный драйвер. Старый пункт загрузки оставить рабочим. Копировать Armbian image целиком/переразмечать диск не нужно.

## Найденный LT8911EXB драйвер

В Armbian есть патчи для SpacemiT MuseBook:
https://github.com/armbian/build/blob/9ebabb47b083586028dc6e7d960f638fe8203af9/patch/kernel/archive/spacemit-6.18/021-drm-spacemit-lt8911exb-pick-up-the-panel-backlight-f.patch

Реальный DRM panel + MIPI DSI драйвер:
https://github.com/chainsx/linux-spacemit/blob/041303ed328da5910c6f0b1150788d0c7ed8fd7c/drivers/gpu/drm/spacemit/lt8911exb.c

Он использует drm_panel, mipi_dsi_attach, GPIO, delayed work. Это более пригодная основа, чем прежний standalone I2C driver, но не готовая поддержка TCL. В нём задан VIDEO_SYNC_PULSE, а у TCL OEM XML traffic1 соответствует NonBurst_VSEvent. Нельзя переносить режим и GPIO/reset последовательности вслепую. Найденный Armbian backlight patch также описывает проблему порядка регистрации PWM/backlight; это свидетельство необходимости проверки lifecycle, не универсальный фикс TCL.

Репозиторий luhenry/linux-scw-em-rv1 содержит файл под drivers/gpu/drm/bridge/lontium-lt8911exb.c, однако поиск drm_bridge/drm_panel/mipi_dsi в самом файле ничего не нашёл. Название каталога не доказывает DRM интеграцию; как основной кандидат его не выбирать.

SpacemiT Armbian — ARCH=riscv64, для TCL ARM64 готовое ядро не подходит. Переносить нужно драйвер/необходимые патчи в ARM64-базу.

Источники:
- https://github.com/armbian/build/blob/main/config/sources/families/include/uefi_common.inc
- https://github.com/armbian/build/blob/main/config/kernel/linux-uefi-arm64-current.config
- https://github.com/armbian/build/blob/main/config/sources/families/spacemit.conf

Следующее: выбрать конкретный ARM64 kernel source/tag и проверить SC7180 dependencies + изменения относительно qc7; подготовить сборочное задание специалисту по пакетам. Сборка/установка нового ядра ещё не начаты.

## 2026-09-08: проверен готовый Armbian 7.1.8 ARM64

Официальный apt.armbian.com, resolute/main/binary-arm64, Release от 2026-09-07. Пакет linux-image-edge-arm64 26.8.3, kernel release 7.1.8-edge-arm64, ARM64, 87763228 байт. SHA256 08413acb920ef695c4042643dfa1d7f51eae562364e41bce640b1191ab7482b1 совпал с Packages.gz. Подпись Release на этом шаге не проверялась; до установки требуется проверка доверия репозиторию. Доступный current в этом индексе — 6.18.44, edge — 7.1.8; версии сборочных конфигов не равны опубликованным пакетам.

Пакет распакован только на builder: /tmp/tcl-armbian-candidate/edge.deb и edge/. Полная конфигурация и сравнение с рабочим qc7 сохранены в wayland-probe/armbian-7.1.8/. Основные SC7180 UFS/ICE, USB PHY, Wi-Fi/modem/RMTFS, cpufreq, I2C и MSM DRM/DSI опции включены. Страница памяти 4K. Все пути файлов в modules.dep существуют. Это проверка состава, не тест загрузки или доказательство совместимости DT/драйверов.

USB DT использует qcom,sc7180-qmp-usb3-dp-phy (его поддерживает phy_qcom_qmp_combo), поэтому выключенный PHY_QCOM_QMP_USB_LEGACY сам по себе не блокирует этот PHY. Старый двухуровневый DWC3 DT требует отдельной проверки с dwc3-qcom-legacy. PINCTRL_SC7180 и многие зависимости в новом пакете модульные: initramfs должен содержать полный набор для USB rootfs/loop/Btrfs и удалённого доступа.

Важное поведение preinst: при /boot на FAT удаляет /boot/System.map*, config*, vmlinuz*, Image и uImage. Postinst запускает kernel hooks и обновляет Image/symlinks. Поэтому первый тест готовить из распакованных файлов в отдельных путях, без выполнения этих скриптов на рабочем TCL. Пакет не устанавливался; GRUB, запущенное ядро и TCL не менялись, перезагрузки не было.

Задание сборщику подготовлено (packaging-task.md). Для DEB требуется Armbian workflow; ALT hasher/girar не заменяет его. Сначала проверка готового ядра, затем при необходимости сборка с портом LT8911EXB. Следующее: проверка подписи репозитория, DT compatibility и состава нового initramfs; после этого отдельный boot entry. Native DRM/Wayland ещё не получены.
