# TCL RMTFS RAM test

Пункт `NEXT: Wi-Fi RMTFS RAM test + auto reboot`.

Windows QCOM0817/RemoteFS statusOK. Считаны первые128KiB PhysicalDrive0..5: GPT CRC32 заголовков и массивов проверены локально. Скрытый PhysicalDrive5 содержит modemst1/modemst2/fsg/fsc. Данные каждого прочитаны дважды с совпадениемSHA256; это не атомарный снимок всего набора во время работающей Windows, но каждое парное чтение стабильно. Оригиналы открывались только FileAccess.Read. GPT и данные сохранены в tcl-rmtfs-export. Названия GPT с хвостом после первогоNUL надо трактовать доNUL (TrimEnd в исходном скрипте удаляет только хвостовые нули).

| Раздел | Offset | Размер | Файл initramfs |
|---|---:|---:|---|
| modemst2 | 2359296 | 2097152 | /rmtfs-storage/modem_fs2 |
| fsc | 6553600 | 131072 | /rmtfs-storage/modem_fsc |
| modemst1 | 262144 | 2097152 | /rmtfs-storage/modem_fs1 |
| fsg | 4456448 | 2097152 | /rmtfs-storage/modem_fsg |

DT: удалён типовой rmtfs_mem@94600000, добавлен memory@80600000/200000 no-map/client-id1. ACPI RFS0 и EFIreserved подтверждают диапазон. qcom,vmid не задан: Linux-драйвер допускает это и не делает SCM assign; сохраняем права firmware. Доступ модема к этому диапазону при Linux ещё должен подтвердить тест, не объявлять права проверенными. Запись в случайную память/SCM-переназначение прав не выполняется. UFS disabled; GPIO58–62 исключены как прежде.

RMTFS1.1-4 готовый Debian ARM64, libudev257.13-1~deb13u1 + libcap2.75-10+deb13u1+b1; остальные библиотеки из wifi-services. Точный upstream1.1 сохранён: -r загружает storage в shadow buffer и последующие записи делает в RAM; -o задаёт каталог. Запуск `stdbuf -oL -eL rmtfs -r -v -o /rmtfs-storage`. Без -P/-s. Перед запуском проверяются phys_addr/size устройства, а до MPSS — работа процесса и наличие QRTRservice14. При ошибке MPSS не стартует, логи сохраняются.

Одна попытка MPSS, recoverydisabled. QRTRtimeline и rmtfs.log сохраняются вместе с остальными журналами. После теста отсчёт30с и перезагрузка только после finalsave/sync/umount. cancel-reboot отменяет.

USB tethering: драйверы RNDIS/CDC-ECM/CDC-NCM уже есть; init теперь проверяет появление USBnet до45циклов с паузой5с, DHCP до3попыток на интерфейс. Логи network.log/modules.log сохранены. Android: включить USB-модем на телефоне с кабелем передачи данных. DHCP подтверждает выдачу адреса, но ещё не интернет и не доступ поSSH из нашейLAN: это другая сеть за телефоном. iPhone может потребовать pairing/userspace, не проверен. SSH-сервер в этом initramfs не добавлялся.

UbuntuARM64 рассматривается следующим сравнительным тестом: genericISO существует, но поддержка других Snapdragon не доказывает TCL; потребуется нашDT/firmware. Разделять Ubuntuuserspace на рабочемядре и проверку Ubuntuядра; только замена userspace не добавит отсутствующий код отладки в текущееядро.

Проверены shellsyntax, AArch64 ELF/DT_NEEDED/interpreter, GPT CRC32, DT memory/UFS/GPIO, CPIO и побайтовые копии modemstorage. Аппаратный запуск новых компонентов ожидается. Дампы modemstorage не прикладывать к публичной баге: для отчёта достаточно размеров/хешей/диагностических логов.

## Пакеты SHA256

- tcl-rmtfs-arm64.deb: 1a82935c55a7a48560ad69e1d1c9b19b14150ec1776ca1d997fb6c5a79fea655
- tcl-libudev-arm64.deb: 459b1c9c17cc3c586e90d13d2a991715d498c6eed6c046cabf321021167bc380
- tcl-libcap-arm64.deb: c9a77aa670c9ca163c624a00e55c4419df40b770fe516010dc495c04a16c80b0
