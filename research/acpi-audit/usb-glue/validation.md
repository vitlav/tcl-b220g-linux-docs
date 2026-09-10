# USB host через ACPI: подтверждённая конфигурация

Проверено 2026-09-10 на TCL B220G, Linux 6.18.34-tcl-acpi6, UEFI/ACPI, диагностический initramfs в RAM. Ubuntu rootfs в этом тесте не запускалась.

| Элемент | Подтверждение |
|---|---|
| SCM QCOM080B | probe returned 0; convention smc arm 64 |
| Apps и Adreno SMMU | оба probe returned 0 |
| QCOM0897 / DWC3 / xHCI | все probe returned 0 |
| DMA | driver guard требует IORT SID0x540 и domain до запуска; USB передача данных состоялась |
| USB2 hub | high-speed, four ports detected |
| Накопитель | USB Mass Storage через usb-storage |
| Запись | два снимка диагностики, SHA256 после remount read-only |
| Получение журналов | оба снимка скопированы с флешки в рабочей системе; все SHA256 совпали |

Основные строки журнала:

```text
qcom_scm: convention: smc arm 64
probe of QCOM080B:00 returned 0 after 4000 usecs
probe of arm-smmu.0.auto returned 0 after 42511 usecs
probe of arm-smmu.1.auto returned 0 after 38175 usecs
xhci-hcd xhci-hcd.3.auto: irq 150, io mem 0x0a600000
probe of xhci-hcd.3.auto returned 0 after 502922 usecs
probe of dwc3.2.auto returned 0 after 534875 usecs
probe of QCOM0897:00 returned 0 after 567571 usecs
hub 1-1:1.0: USB hub found
hub 1-1:1.0: 4 ports detected
usb-storage 1-1.2:1.0: USB Mass Storage device detected
```

IRQ150 — динамический номер Linux, аппаратный USB core INTID165. Устройства high-speed подтверждают USB2; строка контроллера о SuperSpeed не является измерением USB3. Длительность probe приведена как исходное свидетельство, не benchmark.

PHY/clocks/power пока используют firmware-состояние. Не проверены холодная инициализация PHY, wakeup, suspend/resume, role switching, SuperSpeed по портам, USB tethering и UVC capture в ACPI. В журнале есть FAT dirty-volume warning; успешная проверка файлов не заменяет проверку всей файловой системы.

Конфигурация требует ранней SCM `_CCA=0`, ACPI-aware SCM reserved-memory path, Qualcomm SMMU selection, IORT mapping для URS0 и специализированного USB glue. Базовое ядро Ubuntu без адаптации этим тестом не проверялось.
