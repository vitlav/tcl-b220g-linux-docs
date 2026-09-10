# Первый тест UFS TCL

Подготовлено 2026-09-07. База — проверенный EC/Wi-Fi DTB. Ядро 6.18.34-stb-qc7+ и initramfs wifi-persistent остаются прежними.

| Изменение | Основание |
|---|---|
| ufshc@1d84000 и phy@1d87000 status=okay | SoC definitions и TCL ACPI MMIO/IRQ |
| vcc → L19A, 2.96 В | TCL PEP vote и аналог Galaxy Book Go |
| vccq2 → L12A, 1.8 В | TCL PEP vote и аналог Galaxy Book Go |
| PHY vdda-phy → существующий L4A 0.88 В | TCL PEP и SC7180/SM7150 PHY |
| PHY vdda-pll → существующий L3C 1.2 В | TCL PEP и SC7180/SM7150 PHY |
| Удалена ссылка qcom,ice | ICE отключён; ссылка вызывает deferred probe, отсутствие допускается драйвером |

Соответствие четырёх supply names — обоснованная проверяемая гипотеза, не измеренная схема. Новым L12/L19 задан начальный HPM, как у действующих регуляторов. Чужие vcc-max-microamp не перенесены: ufshcd_populate_vreg v6.18 допускает отсутствие свойства и max_uA=0. Токи PHY задаёт драйвер (SM7150: 62900/18300 мкА), не чужой DTS.

Не добавлен неподтверждённый reset-gpios. Сохранены стандартные SoC resets, clocks, IOMMU, interconnects. ICE — блок SoC crypto@1d90000; работа без него не меняет шифрование существующих данных и не расшифровывает BitLocker. Ключи Windows не извлекались.

Diff DTB сохранён. Проверены синтаксис GRUB, прежний инструмент Debian 2.12 с matching modules, SHA256 при передаче и повторном чтении с USB. Пункт TEST: internal UFS + battery + Wi-Fi + SSH выбран default=tcl-ufs, таймер 5 секунд. Предыдущие пункты и EFI/BOOT/BOOTAA64-before-ufs.bak сохранены.

EFI SHA256 133d0eae4c4475910cfaf4f8d3db989ea3d0485c1f290cafe9020e0149864e2c.
DTB SHA256 84b5858185f35a2c6922da5befc47c912d793b655f45e44d11494f63d689d856.
DTB: /tcl-ufs-probe/sc7180-tcl-ufs.dtb. Initramfs: /tcl-wifi-persistent/initramfs.cpio.gz.

Тест ещё не загружался. После загрузки проверить SSH, dmesg UFS/SCSI, devices_deferred, список LUN и их model/size/sysfs path. Номера sdX не считать постоянными. Перед чтением GPT выставить blockdev --setro только для проверенных UFS LUN. На Windows-разделы ничего не писать и не монтировать их rw. Логгер USB выбирает Kingston по serial и marker, поэтому не принимает UFS за флешку. В initramfs нет автоматического монтирования внутренних дисков.

Источники v6.18 получены через gh и сохранены рядом: drivers/ufs/host/ufshcd-pltfrm.c, ufs-qcom.c, drivers/soc/qcom/ice.c. https://github.com/torvalds/linux/tree/v6.18/drivers/ufs/host .

### UFS заработал: подтверждённое чтение всех шести LUN

Boot 4bed92e8-2cfe-4a9e-99cd-b26cdd21a3e0, прямой Wi-Fi SSH доступен на uptime 53 секунды. Контроллер 1d84000.ufshc успешно прошёл probe на 13.25 секунды; начальный -EPROBE_DEFER сменился успехом после появления зависимостей. Модель SAMSUNG KM8F9001JM-B813, revision0700.

| Linux в этом boot | UFS LUN / Windows PhysicalDrive | Размер байт | GPT-разделов |
|---|---:|---:|---:|
| sda | 0 | 251331084288 | 4 |
| sdb | 1 | 8388608 | 2 |
| sdc | 2 | 8388608 | 2 |
| sdd | 3 | 134217728 | 2 |
| sde | 4 | 4294967296 | 35 |
| sdf | 5 | 134217728 | 4 |

Kingston теперь sdg, а не sda: определять устройства по sysfs-пути/serial, не по букве. Размер сектора UFS 4096 байт; /sys/block/*/size выражен в 512-байтных секторах. Все UFS-диски и разделы по sysfs-пути 1d84000.ufshc переведены blockdev --setro и проверены --getro=1. Это настройка текущей загрузки, не постоянная защита накопителя.

Прочитаны первые 128 КиБ каждого LUN. Для каждого проверены CRC32 primary GPT header и partition-entry array; все корректны. Все шесть буферов побайтно совпадают с disk0-gpt.bin ... disk5-gpt.bin, ранее снятыми из Windows. JSON сравнения, raw GPT и логи сохранены в peripherals/ufs-probe/logs/4bed92e8-2cfe-4a9e-99cd-b26cdd21a3e0/ с SHA256SUMS.

Windows-разделы не монтировались, команды записи на UFS не выполнялись. Проверена инициализация и чтение GPT; скорость, длительная устойчивость, чтение всего диска и работа файловых систем ещё не проверены. Предупреждения об отсутствии vdd-hba/vccq и max-microamp ожидаемы по изученному коду, probe завершился успешно. ICE остаётся отключён, содержимое зашифрованных разделов не расшифровывалось.
