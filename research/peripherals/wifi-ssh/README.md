# TCL Wi-Fi boardfile + SSH

Новый пункт NEXT: Wi-Fi board file + SSH. Ядро и DT сохранены от успешного RMTFS-теста; initramfs дополнен.

## Wi-Fi

OEM bdwlan.bin из собственного DriverStore TCL скопирован побайтово в /lib/firmware/ath10k/WCN3990/hw1.0/board.bin как кандидат для штатного fallback. Board-2.bin не подставлялся; b67 и variantECS_QC710 не назначались. QMIboardff может означать отсутствующийboard_info, поэтому корректность именно этого кандидата ещё требует аппаратной проверки; APcredentials/association не настраивались.

Добавлен firmware-5.bin из Debian firmware-atheros20250410-2, только общий API-файл WCN3990/hw1.0. Чужой wlanmdsp.mbn не переносился: OEM TCL qcom/sc7180/tcl/wlanmdsp.mbn сохранён побайтово. Добавлены regulatory.db и upstreamподпись из wireless-regdb2026.05.30-1~deb13u1.

Разбор tqftpserv1.1: open_maybe_compressed сначала открывает существующий обычный файл; при отсутствии пробует .zst, где stat failed может возникнуть для одного кандидата поиска. translate_readonly перебирает пути firmware remoteproc. Поэтому stat failed сам по себе не означает полный отказ доставки. В предыдущем тесте service69/capabilities и запрос размера3832108 уже подтверждали продвижение; vendorпути не поддержаны, firmware переходит к /readonly/firmware/image. Исходники службы не изменялись.

## SSH

Готовый Dropbear2025.89-1~deb13u1: сервер /usr/sbin/dropbear, клиент dbclient (также /usr/bin/ssh), dropbearconvert/dropbearkey и полное дерево ELFзависимостей. Клиент имеет набор опций Dropbear, не полную совместимость с OpenSSHCLI. Сервер порт22, root только по ключам (-s), перенаправления входящих SSH-сессий запрещены (-j -k). Доступны открытые ключи текущего lav и lavbook; основные личные приватные ключи не копировались.

Отдельные диагностические host/tunnel ed25519 ключи созданы локально. Приватные копии в initramfs с0600, root/.ssh0700. На старте dropbearconvert переводит OpenSSHключи в форматDropbear в/run. /dev/pts смонтирован для интерактивной SSH-консоли. Логи ssh-setup.log/sshd.log/reverse-ssh.log сохраняются наUSB.

Обратный туннель: TCL -> lav@192.168.8.186, -R127.0.0.1:22220:127.0.0.1:22, проверка закреплённого ED25519hostkey, BatchMode и ExitOnForwardFailure. После появления маршрута до30попыток с5с паузой; каждое установление ограничено12с, успешный dbclient остаётся в фоне сkeepalive20. Автоматического переподключения после потери уже установленного туннеля нет: /bin/reverse-ssh можно повторить. Требуется достижимость192.168.8.186 с телефона; при мобильном интернете без маршрута вLAN этот адрес не сработает. Пользователь пока не уточнил источник раздачи, это ограничение явно сохраняется.

На lavbook добавлен отдельный ключ с restrict,port-forwarding,permitlisten127.0.0.1:22220,permitopen127.0.0.1:1,command/bin/false. Исходный authorized_keys сохранён в .ssh/authorized_keys.before-tcl-20260907. Ограниченный ключ проверен с текущего хоста OpenSSH: authsuccess и remoteforwardsuccess; тест прерванtimeout5(124) штатно. Это не тест ARM64Dropbear черезтелефон. Автоматически добавленный нашимSSHconfig ключ удалён из локального ssh-agent.

Ключ сервера TCL закреплён наlavbook в .ssh/tcl_known_hosts. Вход сlavbook после установления туннеля:

    ssh -o UserKnownHostsFile=~/.ssh/tcl_known_hosts -p22220 root@127.0.0.1

Остаётся прежняя автоперезагрузка после теста/логов. Для длительного SSH-доступа выполнить cancel-reboot вLinux (локально или поSSH). Прямой вход с доступного устройства в телефонной подсети — root@USB-IP, но NAT не обеспечивает вход из основнойLAN.

Проверки: shellsyntax; AArch64 ELF, всеDT_NEEDED/interpreter; CPIOбезповторов, SSHkeypermissions, файлыboard/firmware, проверка ограниченного ключа/forward наlavbook. Преобразование ключей и SSHсерверARM64 предстоит проверить наTCL. SHA256образов вSHA256SUMS. Набор содержит приватные диагностические ключи/копииmodemstorage; публиковать только очищенные логи/хеши, не образ.

## Firmware SHA256

- firmware/ath10k/WCN3990/hw1.0/board.bin: e937bf4779ea30d588c3c9dab82913cd9e7fd31faaa45bde76b9fca876c0ad3c
- firmware/ath10k/WCN3990/hw1.0/firmware-5.bin: fef6539e0127579536bc977be57a90d018b83f2931fedc3a8870fbe38d6c4127
- firmware/regulatory.db: 2fb33ca0074db573e05ef7dd50bb45b63c0ff98b7e852e1105ebad536fae8e6b
- firmware/regulatory.db.p7s: c941c08f51c93e46722293b85631604c3740d86c3de0c75f79aef50d2e919179
