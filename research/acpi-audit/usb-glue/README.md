# ACPI USB: основа для адаптации Qualcomm glue

Этап текущей работы — ACPI-загрузка с сохранением диагностики на USB. Wi-Fi/SSH, затем родная графика и остальные подсистемы следуют после него; рабочий DT-комплект остаётся резервом.

## Upstream-основа

[Коммит 41717b88](https://github.com/torvalds/linux/commit/41717b88abf1cacd953e9ea2ace2f62eaf763c48) удалил минимальную ACPI-поддержку из dwc3-qcom в марте 2024 года. Обоснование указывает на отсутствие пользователей/обнаруженных регрессий и упрощение дальнейшей переработки драйвера, а не на доказанную невозможность ACPI USB.

[Последняя версия перед удалением](https://github.com/torvalds/linux/blob/12fc84e8c4288cc8ed5f14a35e077130c2cfece2/drivers/usb/dwc3/dwc3-qcom.c) содержит ACPI URS-путь: память берётся у родительского platform device, IRQ — у дочернего USB device. Программно задаётся dr_mode=host, создаётся DWC3 child и сохраняется родительская цепочка. Это подходит как исходная схема для [ресурсов TCL](../iort-mappings/usb-resources.md), но не готовая реализация для него.

Выбрана версия после исправлений teardown/probe-deferral: 51392a1879ff, 9feefbf57d92 и 9cf87666fc6e. Поэтому простой перенос более раннего варианта из Linux 6.6 нежелателен.

## Проверка совместимости сборки

Исторический исходник собран отдельным модулем против дерева `6.18.34-tcl-acpi2`, `ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1`, с include каталога drivers/usb/dwc3. Единственное необходимое изменение C API для этой проверки — [platform_driver.remove_new → remove](platform-remove-api.patch). Object compile, modpost и линковка модуля прошли без предупреждений. Это проверка сборки; никакой аппаратной загрузки модуля не выполнялось.

## TCL ACPI-only прототип

[Исходник](tcl_usb_acpi.c), [изменения относительно указанной upstream-базы](tcl-acpi-only.patch), [сборочный манифест](prototype-manifest.json). Это отдельный диагностический модуль, не патч для механического применения к текущему dwc3-qcom.c.

Сделано и проверено сборкой:

- Единственный ACPI alias — QCOM0897; OF aliases удалены, probe дополнительно отвергает OF device.
- Имя platform driver — `tcl-dwc3-acpi`, без конфликта с `dwc3-qcom`.
- Перед reset/clocks/MMIO проверяются родитель `\_SB.URS0`, память `0x0a600000/0x000fffff`, уже вычисленные ACPI признаки _CCA=0, fwspec с одним SID0x540 и наличие IOMMU domain.
- DWC3 child получает `linux,sysdev_is_parent`, чтобы DMA core использовал ACPI-родителя URS0.
- Исправлены inclusive end у выделяемых диапазонов core и QSCRATCH: `start + size - 1`.
- Адаптирован platform remove API. W=1 compile, modpost и link завершены; ELF modinfo содержит только ожидаемый ACPI alias и release ACPI2.

Контроль _CCA использует уже вычисленные поля ACPI device; AML заново не исполняется. Проверка домена не заменяет аппаратный DMA-тест и пока не проверяет все свойства контроллера IOMMU.

Пример локальной сборки при подготовленном ядре:

```sh
make -C /путь/к/linux M="$PWD" ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1 modules
```

## DMA до probe и оставшиеся требования

`platform_dma_configure()` вызывает ACPI DMA configuration, затем `iommu_device_use_default_domain()` до входа в probe. Настройка нового IORT mapping внутри USB probe была бы слишком поздней для этого порядка владения доменом. Соответствующие default-domain helpers также не являются экспортируемым API внешнего модуля.

Поэтому прототип **не исправляет DMA из probe**: без ожидаемого fwspec/domain он отказывает до аппаратных операций. Следующее изменение должно обеспечить input ID0x80030000 в раннем ACPI/IORT пути и сохранить URS0 как firmware parent. В ACPI2 это изменение ещё не внесено.

| Требование | Состояние |
|---|---|
| HID / изоляция от DT / имя драйвера | Реализовано, compile/modinfo проверены |
| Родитель URS0 / дочерний USB0 | Историческая схема сохранена; поведение нового ядра аппаратно не проверено |
| IORT input ID0x80030000 → SID0x540 | Требуется изменение до probe; проверка SID/domain в прототипе уже есть |
| SCM/SMMU | Изменения ACPI2 собраны; аппаратно не проверены |
| PHY / clocks / power / wake IRQ | Исторические SDM845 параметры ещё требуют полной сверки с TCL; cold-start не доказан |
| USB storage / журнал на носителе | Не проверены; STARTED/RESULT/COMPLETE пока не получены |
| Deferred probe / remove / suspend | Требуют проверки адаптированной связки, успешная линковка их не доказывает |

Модуль не устанавливался и не загружался на TCL. Готового загрузочного эксперимента пока нет. Работа остаётся на первом этапе: ACPI и USB-диагностика, затем Wi-Fi/SSH, затем родная графика.
