# Подготовленный загрузочный комплект Armbian 7.1.8

2026-09-08. Комплект установлен на TCL; пробная перезагрузка выполнена. Две попытки SSH после ожидания завершились timeout; запуск нового ядра и стадия остановки пока не подтверждены. Запрошены последние строки экрана.

InRelease проверен ключом с https://apt.armbian.com/armbian.key: VALIDSIG 8CFA83D13EB2181EEF5843E41EB30FAF236099FE. Доверие к ключу основано на HTTPS официального сайта, независимой сверки fingerprint не было. Вторая подпись 93D6889F9F0E78D5 не проверена: ключ отсутствует. SHA256 Packages.gz проверен по содержимому подписанного Release, SHA256 DEB — по Packages.gz.

Рабочий DT использует qcom,dwc3 fallback; modules.alias нового ядра сопоставляет его dwc3_qcom_legacy. USB3 PHY qcom,sc7180-qmp-usb3-dp-phy сопоставлен phy_qcom_qmp_combo. Это проверка matching, не полная проверка DT bindings и поведения драйверов. Рабочий DTB сохранён, MDSS/GPU выключены. SYSFB_SIMPLEFB/DRM_SIMPLEDRM в кандидате также выключены.

initramfs содержит полный набор модулей нового ядра (464 MiB на диске, архив около124 MiB), статический ARM64 BusyBox и адаптированный init. 28 модулей раннего запуска перечислены в early-modules.txt; их зависимости разрешены через modprobe --show-depends без загрузки в ядро builder. Бинарный cpio проверен: init/BusyBox и все файлы модулей совпадают с источниками, старых модулей нет. Подробности initramfs-validation.json.

Перед загрузкой необходимо разместить lib/modules/7.1.8-edge-arm64 в Ubuntu rootfs отдельным каталогом; новый init проверяет его наличие. Не выполнять preinst/postinst Armbian на TCL. Firmware и существующие сетевые настройки остаются в Ubuntu rootfs. Автозагрузка сети с новым ядром пока не проверена.

Файлы для USB: /tcl-armbian-7.1.8/Image и initramfs.cpio.gz. DTB: существующий /tcl-graphics-probe/sc7180-tcl-i2c10.dtb. Подготовленный BOOTAA64.EFI имеет default=tcl-armbian-718 и сохраняет все старые пункты. Построен согласованными Debian GRUB2.12 tools/modules, grub-script-check успешен. Установка EFI требует сохранения текущего BOOTAA64.EFI и сверки SHA до/после, сейчас не выполнялась.

После обнаружения флешки сохраняются /tcl-armbian-7.1.8/logs/early-modules.txt и early-dmesg.txt. До инициализации USB сохранить лог туда невозможно; при ошибке остаётся локальная shell. UFS RO проверяется по пути контроллера в init и существующему правилу Ubuntu. Автоматической перезагрузки в этот базовый тест не добавлено: он должен остаться доступен для SSH-диагностики.

## Установка и первая попытка

install.sh завершился exit0, все6648файлов проверены до установки, модули повторно проверены после перемещения. Установленные файлы соответствуют boot-SHA256SUMS. Старые модули сохранены, backup EFI/BOOT/BOOTAA64-before-armbian-718.bak SHA256 c9b35214dffe60860b7f235096258f39f344c3f9fe22ddfd4dba336410bddf52. На builder та же копия /tmp/tcl-i2c-boot-build/BOOTAA64.EFI. Для возврата выбрать в GRUB TCL: I2C10 controller probe - EFI display retained. Без вывода экрана нельзя считать причиной сеть либо ядро.
