# TCL B220G: Wi-Fi firmware services test

Новый пункт: `NEXT: Wi-Fi firmware services + auto reboot`.
DT и ядро совпадают с предыдущим Wi-Fi probe: исправленный TLMM GPIO58–62, USB/HID, MPSS и WCN3990. Новых изменений аппаратного описания нет.

Initramfs содержит оригинальные OEM qcmpss7180_nm.mbn, wlanmdsp.mbn и все 14 bdwlan/bdwlanu-вариантов из сохранённого Windows DriverStore. board-2.bin не подбирался. Файлы расположены вместе в /lib/firmware/qcom/sc7180/tcl; tqftpserv 1.1 переводит /readonly/firmware/image/ относительно firmware remoteproc. Запросы /readwrite/ попадают в RAM /tmp/tqftpserv.

Перед запуском MPSS запускаются qrtr-ns -f 1 и tqftpserv через stdbuf -oL -eL. Оба процесса должны оставаться запущенными. Для выбранного MPSS выставляется и проверяется recovery=disabled, затем одна попытка старта и 90 секунд наблюдения. QRTR проверяется qrtr-lookup, ошибочное чтение /proc/net/qrtr удалено. Причина watchdog пока не установлена: тест проверяет гипотезу нехватки firmware/service, не обещает соединение Wi-Fi.

Автологирование каждые 30 секунд сохраняет report.txt, dmesg.txt, wifi.log, tqftpserv.log, qrtr-ns.log, autoreboot.log и доступные input event файлы. Запись разрешается только USB с точным серийным номером Kingston и проверенным маркером. После завершения теста отсчёт 30 секунд; команда cancel-reboot отменяет перезагрузку. Затем отдельный финальный цикл записи, sync и успешный umount. Только подтверждение этого цикла разрешает reboot -f. При зависшем тесте или неподтверждённой записи автоматической перезагрузки нет. Загрузка после reboot определяется UEFI и может снова открыть меню USB.

Проверки: sh -n; ELF AArch64 и полное замыкание DT_NEEDED/интерпретатора; CPIO без повторных имён, все firmware побайтово; mock-проверки автоперезагрузки: успех, отмена, ошибка сохранения, незавершённый тест. Исполнение ARM64-служб проверится только на ноутбуке. Regulatory.db пока отсутствует; это известный отдельный пробел прежнего теста.

Готовые пакеты взяты из официального Debian deb.debian.org, распакованы без установки на хост; исходники служб не изменялись. stdbuf нужен для немедленного сохранения printf-журнала tqftpserv.

## Пакеты

- Package: tqftpserv; Version: 1.1-4; SHA256 `e594609d54ae5cf96afa597c9b26da9261f02f35c1bd20970754452c47049399`.
- Package: qrtr-tools; Version: 1.1-2+b1; SHA256 `dc3fff08ab2c88e6b4116f6c94dda0152013d23a3fe199ebeb6821f52581c8e2`.
- Package: libqrtr1; Version: 1.1-2+b1; SHA256 `f6a6d0a25baf9e77435deab10e16ed5a4dcc36b3a7a9ecf57f7454943a773e89`.
- Package: libc6; Version: 2.41-12+deb13u4; SHA256 `8784eda966b189c777a384dac5ce009e8fc9b52d006926c5a013e7fa8aa688cc`.
- Package: libzstd1; Version: 1.5.7+dfsg-1; SHA256 `924540bd59fdbfa77a0604360efdaca54411a43daf11c7e002a3c64791b67448`.
- Package: coreutils; Version: 9.7-3; SHA256 `99b2531e8d16e4f2d3f239ae3cb57913ec1f43764d2d6cd5c69154cbb1e57f3e`.

## Firmware SHA256

````text
12c738f0393672ccfa2acf0d5ee5d80f4d7e868cb1238f003fd563362e009e6e  firmware/qcom/sc7180/tcl/bdwlan.b36
93c6e9f50ec6d3dd3acc88921911dbc68a80aa7a581df28062cba59ca3afb6a2  firmware/qcom/sc7180/tcl/bdwlan.b37
e10ef5e66070bd21f005d309a81afe5aee2a6affd11b33c97ab264c02d7119f2  firmware/qcom/sc7180/tcl/bdwlan.b46
58725816a65596dbdf20d1fecdf5b62cfc7ab51c35999daa337c62afa72dfbc3  firmware/qcom/sc7180/tcl/bdwlan.b47
cef488c8ecf2da501299ef150128aa4d8d208e5faa76bea2c7f82e14c251c128  firmware/qcom/sc7180/tcl/bdwlan.b48
8da568415c32b4946f123a2dc2cda75dc077218516353ca452cd109bca3fd586  firmware/qcom/sc7180/tcl/bdwlan.b59
c3f0406864e479752072253c9562c88d780eaebcb61573c110d7c9c44acac1ed  firmware/qcom/sc7180/tcl/bdwlan.b5b
da2e615dea087b66889d09ab627e091710a35ce05167b0bd95cf4926196a62e1  firmware/qcom/sc7180/tcl/bdwlan.b67
cd3ffca25b10d79c154830b2037e5b144da73472d42b0375ef8ff1e4e598e1df  firmware/qcom/sc7180/tcl/bdwlan.b71
e937bf4779ea30d588c3c9dab82913cd9e7fd31faaa45bde76b9fca876c0ad3c  firmware/qcom/sc7180/tcl/bdwlan.bin
c7ceb9dbf330587755aeb3116d3757399b2d75aadf1ed6d38d00c85c7b6f505a  firmware/qcom/sc7180/tcl/bdwlanu.b59
7c54c92b39d34c26d53c783d79ab0d81dedbf785e08ed600adae610d8e3814e9  firmware/qcom/sc7180/tcl/bdwlanu.b5b
da2e615dea087b66889d09ab627e091710a35ce05167b0bd95cf4926196a62e1  firmware/qcom/sc7180/tcl/bdwlanu.b67
e937bf4779ea30d588c3c9dab82913cd9e7fd31faaa45bde76b9fca876c0ad3c  firmware/qcom/sc7180/tcl/bdwlanu.bin
9ea0c34b4bcd380aec638135ba0751067a46ae988d207e959d33883430c36ab3  firmware/qcom/sc7180/tcl/qcmpss7180_nm.mbn
b3213d834d11e602c2422690edf316f8fa786ab2d1a34df60b9f4bf83801fa82  firmware/qcom/sc7180/tcl/wlanmdsp.mbn
````
