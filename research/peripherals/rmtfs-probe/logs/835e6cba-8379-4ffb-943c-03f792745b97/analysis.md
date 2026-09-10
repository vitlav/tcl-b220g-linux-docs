# Результат RMTFS: пройден прежний watchdog, WLAN QMI появился

Boot835e6cba-8379-4ffb-943c-03f792745b97, полный каталог сохранён локально с SHA256. MPSS up64.264117, после90с наблюдения running/recoverydisabled; прежнего fatal/crash в сохранённом dmesg нет. Это подтверждает устойчивость в окне теста, не длительный тест.

RMTFSservice14 зарегистрирован доMPSS. Модем открыл modem_fs1/modem_fs2/modem_fsg, запросил2MiB по80600000, прочитал через80600200/80600400, включая4094сектора modem_fs2. Ответы0:0. Корректная памятьTCL и данныеRMTFS позволили пройти прежний этап зависания. Роли отдельно исправленногоадреса/службы/данных этим комбинированным тестом не разделены.

WLANservice69 появился. На64.845405 ath10k получил chip_id0x320/family0x4001/board_id0xff/soc0x400c0000; firmware WLAN.HL.3.2.2.c10-00748-QCAHLSWMTPLZ-1. На65.057785 — failed to fetch board-2.bin or board.bin from ath10k/WCN3990/hw1.0. PHY пока нет. В кодеath10k board_info_invalid даёт sentinel0xff; нельзя автоматически назначитьb67 только по аналогииQC710. Следующая задача — обоснованныйOEMboardfile/путь Linux.

TFTP теперь получает запросы wlanmdsp.mbn: сначала два неподдерживаемыхvendorпути, потом /readonly/firmware/image/wlanmdsp.mbn. Есть stat failed и END OF TRANSFER, но также запросrsize3832108; WLANQMIдошёлдоcapabilities. Не объявлять всеTFTPзапросыуспешными или всёнеудачей по одномуstat: нужно сопоставить translate.c и обработку probe/размера/передачи. /readwrite/server_check.txt записанвRAM.

Android RNDIS зарегистрирован на118.251423; usb0=10.254.135.41/24, DHCPserver/router10.254.135.36. Пользователь подтвердил сеть иNAT. Внешнийинтернетпоэтимлогамнепроверен; SSHсервервinitramfsпокаотсутствует. WindowssshdнедоступенкогдаработаетLinux. Для входа нуженDropbear/OpenSSHвLinux; для доступачерезNAT — исходящийобратныйтуннельнаразрешённыйдоступныйсервер. Ничегоэтоговэтомэтапенеустанавливали.

На вопросType-C: обычныйкабельнеобеспечиваетhost-hostсетьсампосебе. НуженgadgetнаоднойсторонеилиThunderbolt/USB4сетьнаобоих. KernelDWC3dual-role/GADGETесть, но маршрутизацияпортаTCL/рольиодновременнаяработафлешкинепроверены. Не переключать контроллер с диагностической флешкой вслепую. Источники: https://www.kernel.org/doc/html/latest/driver-api/usb/gadget.html и https://docs.kernel.org/6.16/admin-guide/thunderbolt.html . Предпочтительна уже подтверждённаяUSBсетьтелефона сSSHтуннелем.
