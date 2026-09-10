# Инфраструктура SC7180: IRQ, GPIO, clocks, питание и DMA

Раздел описывает общие поставщики ресурсов рабочего Device Tree. Полный перечень свойств и связей сохранён в [реестре DT](full-inventory.md). Привязка драйвера и работа потребителей подтверждают используемую конфигурацию, но не все режимы каждого контроллера.

## Прерывания и таймер

| Блок | Адрес / размер | Назначение |
|---|---|---|
| GICv3 distributor | 0x17a00000 / 0x10000 | Основной контроллер IRQ |
| GICv3 redistributor | 0x17a60000 / 0x100000 | Персональные интерфейсы CPU |
| PDC | 0xb220000 / 0x30000 | Wakeup routing, родитель GIC |
| TLMM IRQ | GIC SPI208, level-high | GPIO interrupt controller |
| ARM timer | PPI1/2/3/0, level-low в DT | INTID17/18/19/16 |

Номер GIC SPI преобразуется в INTID добавлением32, PPI — добавлением16. Linux IRQ в /proc/interrupts назначается отдельно. Числовые ячейки дочерних доменов MDSS, PDC и GPIO нельзя автоматически считать GIC SPI.

PDC `qcom,pdc-ranges` содержит тройки `(0,480,94)`, `(94,609,31)`, `(125,63,1)`. Они сохранены из DT; соответствие конкретного wakeup-события устройства требует проверки маршрута через оба контроллера.

В OEM GTDT порядок INTID таймера совпадает с DT, но полярность описана иначе (high против low). MADT maintenance IRQ24 отличается от значения25 в сравниваемом mainline-описании. Эти расхождения не устранены и не объясняют автоматически ошибки ACPI-загрузки. [ACPI](../acpi.md).

## TLMM и pinctrl

| Tile | MMIO / размер |
|---|---|
| west | 0x03500000 / 0x300000 |
| north | 0x03900000 / 0x300000 |
| south | 0x03d00000 / 0x300000 |

Compatible qcom,sc7180-pinctrl, gpio-ranges охватывает120линий. Wakeup parent — PDC. GPIO58–62 исключены через gpio-reserved-ranges в исходном рабочем DT.

Windows-драйвер выбирает tile из трёх кандидатов относительно ACPI GIO0 base0x03400000; получаются те же базы. Это сопоставление адресации, не разрешение на общий MMIO-опрос. Полная карта владения secure firmware не установлена.

Опрос направления всех GPIO без reservation сопровождался остановкой загрузки; точный проблемный pin не локализован. GPIO58 отдельно проверен для reset WCD9385. Постоянное изменение доступности GPIO требует корректного boot DT/gpiochip valid_mask и не обосновывает снятие всего диапазона.

Функция pinmux, pull, drive-strength и output level — разные параметры. Значения 2 mA и напряжения в таблицах являются заданными ограничениями, а не электрическими измерениями.

## Clocks и питание

| Provider | Ресурс / интерфейс |
|---|---|
| GCC | MMIO0x100000/0x1f0000; clocks, resets и power domains |
| RPMh RSC | drv-0 0x18200000, drv-1 0x18210000, drv-2 0x18220000; размер каждого0x10000 |
| RPMh IRQ | GIC SPI3/4/5, level-high |
| RPMh config | drv-id2, tcs-offset0xd00; точные TCS cells в реестре |
| PM6150 | regulators-0, pmic-id a |
| PM6150L | regulators-1, pmic-id c |
| GPUCC / DISPCC / VIDEOCC | 0x5090000 / 0xaf00000 / 0xab00000; зависимости по подсистемам |
| SCM | firmware/scm, compatible qcom,scm-sc7180 / qcom,scm |

Одинаковый ID clock/reset/domain имеет смысл только вместе с provider. Аналогично ldo10 банкаA и ldo10 банкаC — разные регуляторы. Supply-ссылка показывает программную связь потребителя с provider; физическую разводку следует подтверждать отдельно.

RPMh-регуляторы работают через firmware resource requests. SPMI PMIC arbiter описан в DT, но отключён; это не означает отсутствия управления питанием через RPMh. Зарядка батареи управляется отдельно EC, [протокол](ec.md).

SCM обеспечивает secure services, используемые PAS/remoteproc, SMMU и ICE. Для рабочего DT проверены noncoherent DMA и 32-битная маска SCM; перенос этих зависимостей в ACPI требует [отдельных изменений](../acpi.md#scm-и-dma).

## Два разных SMMU

| Контроллер | Ресурс | DT semantics |
|---|---|---|
| Apps MMU-500 | 0x15000000 / 0x100000 | qcom,sc7180-smmu-500 / arm,mmu-500; 2 IOMMU cells; dma-coherent |
| Adreno SMMUv2 | 0x5040000 / 0x10000 | qcom,sc7180-smmu-v2 / qcom,adreno-smmu; 1 IOMMU cell |

Adreno SMMU clocks — GCC38/35, power domain GPUCC cx0. Два global IRQ SPI229/231 level-high и context IRQ SPI364–371 rising. Полный список Apps IRQ приведён в реестре.

| Клиент | IOMMU provider | Specifier DT |
|---|---|---|
| GPU | Adreno | 0 |
| GMU | Adreno | 5 |
| UFS | Apps | 0xa0,0 |
| USB | Apps | 0x540,0 |
| WLAN | Apps | 0xc0,1 |
| MDSS | Apps | 0x800,2 |
| Venus | Apps | 0xc00,0x60 |

Вторая ячейка Apps specifier не является универсальным дополнительным SID; трактуется binding/драйвером. `dma-coherent` у самого SMMU не означает, что все клиенты, включая SCM, имеют ту же DMA-семантику.

ACPI IORT описывает оба контроллера, но выбор Qualcomm implementation и отображений клиентов требует отдельной поддержки. Успешная DT-загрузка не подтверждает корректность generic reset или identity domain при ACPI.

## Границы интеграции

Драйвер устройства зависит не только от своего MMIO: ему могут требоваться clocks, resets, power domains, interconnect, pinctrl, firmware и IOMMU. Deferred probe при отсутствии поставщика не равен аппаратной неисправности; исчезновение deferred также не доказывает все функции устройства.

Глобальная перепривязка TLMM, PMIC banks или SMMU затрагивает активные потребители и не является операцией только чтения. Для постоянной поддержки нужны согласованные описания и управление жизненным циклом ресурсов в драйверах.
