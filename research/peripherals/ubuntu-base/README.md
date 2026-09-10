# Ubuntu ARM64 для TCL B220G

Подготовка: 2026-09-08. Это Ubuntu Base с проверенным ядром Qualcomm и DTB TCL. Загрузка самой Ubuntu/systemd подтверждена. Сеть и SSH проверены после ручного исправления прав DHCP; повторной загрузки после исправления пока не было. Стандартное ядро Ubuntu здесь пока не проверялось.

## Исправление DHCP, версия v2

После первой загрузки пользователь сообщил, что система видит диск и Wi-Fi, но IPv4-адреса нет. В предзагрузочном архиве обнаружены ошибочные права600 root:root на файлах `/etc/systemd/network/*.network` и `.link`. Networkd работает от systemd-network и не может прочитать закрытые root-файлы. Каталог network имеет755; профиль WPA присутствует и имеет правильные600. Метаданные без содержимого профиля сохранены в `network-permissions-before.txt`.

В `configure-root.sh` добавлены `umask 022` и явный chmod644 для трёх файлов networkd. В отдельной локальной копии rootfs исправлены только эти права; WPA-профиль сохранил600. После размонтирования btrfs check --readonly без ошибок. SHA256 исправленного rootfs: `5a7c8f191c5bbde431fae69316f419637c8758bd7e230aaaea1c3698f8d565f5`.

Использовать архив **tcl-ubuntu-usb-20260908-v2.tar.gz.gpg**. Исходный архив без v2 оставлен для истории и содержит ошибку прав DHCP. Старый USB-SHA256SUMS относится к первой версии; исправленная версия проверяется по USB-SHA256SUMS-v2. При проверке v2 успешно выполнена потоковая расшифровка/MDC,169 файлов и5 эталонных SHA. У168 файлов содержимое не менялось, изменён только rootfs.img.

После ручного исправления SSH подтверждён на192.168.8.177, boot15a44890-c95f-4081-8cab-3d71212e63bb. PID1=systemd, is-system-running=running, failed units=0. Networkd, wpa_supplicant@wlan0, tcl-radio, rmtfs, tqftpserv и ssh.socket имеют enabled и active; ssh.service active. Журнал прямо подтверждает Permission denied до исправления и получение DHCPv4-адреса после него. WPA-профиль не меняли.

Root — /dev/loop0 Btrfs rw на USB /dev/sdg1; исправленные права находятся в постоянном rootfs, выполнен sync. Все шесть внутренних UFS-дисков и их разделы RO=1. Работают DNS через WLAN и чтение EC. Это проверка после ручных команд; автономный DHCP/SSH после следующей перезагрузки ещё не проверен. Лог: logs/15a44890-c95f-4081-8cab-3d71212e63bb/status.txt.

Исправление на уже загруженной Ubuntu без перезагрузки:

```sh
chmod 644 /etc/systemd/network/*.network /etc/systemd/network/*.link
systemctl restart systemd-networkd
systemctl start ssh.socket
```

Не менять права `/etc/wpa_supplicant/wpa_supplicant-wlan0.conf`: он содержит ключ сети и должен оставаться600. Отдельный dhcpcd не требуется, DHCP настроен в networkd. OpenSSH использует socket activation, поэтому отсутствие отдельного процесса sshd до первого подключения само по себе не означает сбой сервера.

## Основа и размещение

- Ubuntu Base 26.04.1 LTS ARM64: https://cdimage.ubuntu.com/ubuntu-base/releases/26.04/release/
- Исходный `ubuntu-base-26.04.1-base-arm64.tar.gz`: SHA256 `5a1906794ced63a71a8119c3f211ef5f0bbe0a243001b4bbd41fdf80c5b219fd`, совпал с официальным SHA256SUMS.
- Готовое ядро `6.18.34-stb-qc7+`: https://github.com/hexdump0815/linux-mainline-qcom-kernel/releases/tag/6.18.34-stb-qc7%2B . Ядро не компилировалось и не менялось при подготовке Ubuntu.
- DTB: `sc7180-tcl-cpufreq.dtb`, SHA256 `f88a6854d0e1085b32b5f92cbca01e391b61248d512bcf00003ee080a83f7713`.
- На существующей FAT32 Kingston создан `/tcl-ubuntu/rootfs.img`, 3 ГиБ, Btrfs UUID `b433988c-6473-450b-9e9a-b07598eac2a3`, label `TCLUBUNTU`. Ext4 в данном ядре отсутствует; Btrfs и loop встроены. Размер меньше ограничения FAT32 на один файл. Разделы USB и внутреннего UFS не изменялись.
- После установки Btrfs использует около 191 МБ физических данных/метаданных; доступно около 2.3 ГиБ. Применено `compress=zstd:1`. Это консольная система без рабочего стола.

## Все изменения относительно Ubuntu Base

| Область | Изменение |
|---|---|
| Пакеты | Через подписанные репозитории Ubuntu установлены systemd-sysv, udev, dbus, OpenSSH server/client/SFTP, sudo, iproute2, iputils-ping, wpasupplicant, iw, ca-certificates, btrfs-progs, rmtfs, tqftpserv, qrtr-tools, kmod, i2c-tools, less, nano, bash-completion, locales, tzdata, systemd-resolved и systemd-timesyncd; выполнен apt upgrade. Полный состав в validation-and-packages.txt. |
| Установка | Пакеты установлены native ARM64 в chroot работающего TCL; на время установки policy-rc.d запрещал запуск служб, затем удалён. Временные proc/sys/dev и DNS подготовлены для chroot; они не включены в rootfs. |
| Ядро | В GRUB используется прежний Image, в rootfs скопированы соответствующие /lib/modules/6.18.34-stb-qc7+ и выполнен depmod. Обычный linux-generic Ubuntu не устанавливался. |
| Device Tree | Сохранены зарезервированные GPIO58–62, EFI framebuffer, USB/HID, EC, исправленный RMTFS, UFS без ICE, CPUFreq. GPU/MDSS/GMU и глубокий cpuidle остаются отключёнными. |
| Ранняя загрузка | Собственный initramfs из готового статического BusyBox и модулей; находит Kingston по serial и маркеру, монтирует Btrfs loop-образ, передаёт proc/sys/dev/run и запускает /sbin/init Ubuntu. При ошибке остаётся локальная shell. |
| Завершение | /run/initramfs/shutdown возвращает управление в RAM, размонтирует rootfs, отсоединяет loop и размонтирует FAT32 перед reboot/poweroff. Интерфейс: https://github.com/systemd/systemd/blob/main/docs/INITRD_INTERFACE.md . |
| GRUB | Добавлен default=tcl-ubuntu. Все прежние пункты сохранены, предыдущий EFI сохранён как EFI/BOOT/BOOTAA64-before-ubuntu.bak. Использованы согласованные Debian GRUB 2.12 executable/modules. Убраны initcall_debug, ignore_loglevel и keep_bootcon, loglevel=6; аппаратные ограничения и schedutil сохранены. |
| Firmware | Перенесён проверенный набор /lib/firmware: OEM MPSS, wlanmdsp и board-варианты TCL, ath10k firmware, regulatory.db. Новые случайные board-файлы не подбирались. |
| RMTFS | Штатный пакет Ubuntu 1.1.1-2, override ExecStart: rmtfs -r -v -o /rmtfs-storage. Используются индивидуальные копии modemst1/modemst2/fsg/fsc с RAM shadow writes; не реальные разделы. Убраны штатные -P и -s. |
| TFTP Qualcomm | Штатный tqftpserv 1.1.1-1, журнал через stdbuf, временные запросы /tmp/tqftpserv. Это QRTR TFTP-служба для модема, не сетевой TFTP для загрузки ОС. |
| Порядок модема | qcom_q6v5_pas и ath10k_snoc исключены из автоматической загрузки udev через blacklist. tcl-radio.service проверяет память 0x80600000/2 МиБ и QRTR service14, затем явно загружает модули и запускает MPSS с recovery=disabled. |
| QRTR | qrtr-ns.service masked: используется уже проверенный nameserver ядра. |
| Wi-Fi | systemd-networkd + wpa_supplicant@wlan0. Фиксированное имя wlan0 и прежний локально администрируемый MAC 46:14:79:6e:ba:d1. DHCP ClientIdentifier=mac, metric100. Профиль eterwifi перенесён с правами600. Пароль в открытый комплект не включён. |
| USB-сеть | Дополнительный networkd-профиль для распространённых USB Ethernet/tethering-драйверов, DHCP metric500. Телефон не требуется для основного WLAN. |
| SSH | OpenSSH 10.2p1; сохранены доверенные authorized_keys и прежний закрытый hostkey для непрерывности проверки сервера. Root только по ключу, парольный/keyboard-interactive вход отключён. ssh.socket слушает IPv4:22; IPv6 в ядре модульный и на этапе проверки не загружен. |
| Консоль | Hostname tcl-ubuntu, C.UTF-8, multi-user.target, root autologin на локальной tty1. SSH остаётся только по ключам. |
| DNS/время | systemd-resolved и timesyncd включены. Записано время в /var/lib/systemd/timesync/clock, поскольку диагностика стартовала с 1970 года. networkd-wait-online отключён. |
| Журнал | journald persistent, SystemMaxUse=64M, RuntimeMaxUse=16M. |
| UFS | В initramfs blockdev --setro по пути контроллера 1d84000.ufshc; дополнительное udev-правило для новых block-устройств этого контроллера. fstab не содержит внутренних разделов. |
| Батарея | /usr/local/sbin/battery-status читает ранее проверенный EC I2C2@0x07. В Ubuntu i2ctransfer нужен -a для адреса0x07; -f не используется. Kernel power_supply-драйвера пока нет. |

Скрипты подготовки и точные конфигурации: configure-root.sh, init, shutdown, build-initramfs.py, battery-status, install-boot.sh, grub.cfg. Перенос секретных файлов выполнялся отдельно, они не зашиты в эти скрипты.

## Проверки до загрузки

- APT, ARM64 bash и утилиты успешно выполнялись на действующем ядре.
- Настоящее SSH-подключение по существующему ключу к OpenSSH внутри chroot на тестовом порту2222 успешно; сервер показал Ubuntu26.04.1 и ядро6.18.34. Тестовый listener затем остановлен.
- sshd -t, dpkg --audit, systemd-analyze verify пользовательских radio/Wi-Fi units прошли. Предупреждение об IPv6 относится к отсутствующему в текущей RAM-системе загруженному модулю.
- battery-status: AC=1, remaining5400мА·ч, voltage8699мВ, percent100, raw current0. Флаг charging при токе0 не означает фактический ненулевой ток.
- После размонтирования btrfs check --readonly: no error found. Этот запуск не проверял контрольные суммы всего содержимого файлов.
- EFI/initramfs проверены SHA256 после размонтирования и повторного монтирования USB read-only. Полные суммы: USB-SHA256SUMS.
- Полный зашифрованный архив расшифрован потоково без записи закрытого содержимого на FTP: проверены 169 файлов, GnuPG MDC и совпадение всех пяти эталонных SHA256 (rootfs, initramfs, Image, DTB, EFI).

## Комплект /var/ftp/tmp/lav/tcl

Полный файловый архив USB: `tcl-ubuntu-usb-20260908-v2.tar.gz.gpg`. Он содержит rootfs.img, все прежние диагностические файлы и резервные EFI. Это архив файлов FAT32, не raw-образ таблицы разделов.

Архив зашифрован AES256 с проверкой целостности: внутри есть закрытые SSH-ключи, Wi-Fi-профиль и индивидуальные данные модема. Пароль хранится в записи `tcl-ubuntu-usb-20260908` общего хранилища администраторов. Администратор получает его штатно: `ssh rooter@server pass show tcl-ubuntu-usb-20260908`. Не передавать пароль в аргументах команд и не публиковать расшифрованный архив.

Для восстановления на исходную Kingston с FAT32: проверить SHA256SUMS, выполнить `gpg --decrypt tcl-ubuntu-usb-20260908-v2.tar.gz.gpg | tar -xz -C /ПУТЬ/К/СМОНТИРОВАННОЙ/FAT32`, затем sync и безопасное размонтирование. Перед распаковкой обязательно проверить устройство назначения. На другом USB serial в init нужно заменить и пересобрать initramfs; размер устройства не меньше8ГБ. EFI/BOOT/BOOTAA64.EFI используется как ARM64 removable bootloader.

Сами initramfs Ubuntu, Image, DTB, EFI, GRUB config и скрипты не содержат пароля или индивидуальных modem storage. Не расшифровывать полный архив в FTP-каталог.

Для пересборки загрузчика на x86_64 Linux распаковать `grub-2.12-build-tools.tar.gz` рядом со скриптами, затем выполнить `sh build-boot.sh /путь/к/rootfs/lib/modules /каталог/результата`. Скрипт использует согласованные GRUB2.12 executable/modules. Не смешивать их с системным GRUB другой версии. `sc7180-tcl-cpufreq.dts` — текстовая декомпиляция опубликованного DTB для изучения.

## Ограничения

Графический рабочий стол, ускорение GPU, подсветка, power_supply, тепловое управление и глубокий сон не добавлены. Используется EFI framebuffer. Стандартное ядро Ubuntu, автономный DHCP/SSH после исправления и shutdown loop-root требуют отдельной проверки. Загрузка systemd и работа сети после ручного исправления уже подтверждены; это не доказательство последующего полностью автоматического старта.
