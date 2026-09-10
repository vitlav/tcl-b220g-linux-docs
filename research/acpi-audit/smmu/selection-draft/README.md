# SC7180 ACPI: отдельный выбор Apps и Adreno SMMU

2026-09-10. **Изолированный черновик, object-build и host-проверки. Не установлен на TCL; полный Image с патчем не собирался.**

## Изменение

`sc7180-acpi-smmu-selection.patch` добавляет отдельную ACPI-ветку только для IORT `QCOM  / QCOMEDK2 / OEM revision 0x7180`. Существующие DT-match и ACPI-платформы 0x8180 не изменены.

| Проверяемые ресурсы | Выбор |
|---|---|
| SMMUv2, ARM_MMU500, MMIO 0x15000000, размер 0x100000 | qcom_smmu_500_impl0_data |
| SMMUv2, GENERIC_SMMU, MMIO 0x05040000, размер 0x10000 | qcom_adreno_smmu_v2_impl |
| Другое сочетание на распознанной SC7180 IORT | -ENODEV с диагностикой; generic reset не выбирается этой веткой |

Специальная `sc7180_acpi_adreno_data` кладёт GPU implementation в поле `.impl`, потому что существующий `qcom_smmu_create()` выбирает `.adreno_impl` только по OF compatible. Другие поля этой структуры нулевые, как в DT qcom_smmu_v2_data для SC7180. Это узкий экспериментальный способ избежать изменения всех существующих вызовов helper.

Проверка размера использует `smmu->numpage`: **на этой стадии** поле ещё содержит размер MMIO-ресурса в байтах. Порядок подтверждён в `arm_smmu_device_probe()`: модель/версия → ioremap → ioaddr и numpage=resource_size → arm_smmu_impl_init. IORT `arm_smmu_init_resources()` формирует размер прямо из `span`. При переносе на другое ядро этот порядок необходимо перепроверить.

Обе ветки по-прежнему проходят `qcom_smmu_create()` и откладываются с EPROBE_DEFER, пока SCM не готов. SCM ACPI v2 и ранняя SSDT — отдельные необходимые части эксперимента. Этот патч их не включает.

## Проверки

| Проверка | Результат |
|---|---|
| ARM64 object-build, ACPI1 CONFIG_ACPI=y, W=1 | PASS, без предупреждений |
| ARM64 object-build, MEMDIAG CONFIG_ACPI=n, W=1 | PASS, без предупреждений |
| Символы SC7180 ACPI в объекте без ACPI | Отсутствуют; в ACPI-объекте присутствуют |
| patch --dry-run против исходного файла | PASS |
| Два SMMU из снятой живой IORT | Apps → 1, Adreno → 2 (метки host harness) |
| Перепутанные модели Apps/Adreno | -ENODEV |
| Неверные размеры MMIO для каждого SMMU | -ENODEV |
| Неизвестный адрес / SMMUv1 вместо v2 | -ENODEV |
| Исходники ACPI1 и MEMDIAG после проверки | Побайтно не изменены |

SHA256 исходного arm-smmu-qcom.c: `4ea6fd76bcc1c30dae42ca4a9bfa887cb18337133a328a542174cae05796226e`.
SHA256 кандидата: `922875dffdb4714eab1a6acce4e71416effd34d336189b08ad2127431ee21e87`.
SHA256 живой IORT: `aec7a46d8518744956d3776fee085de8ac3f23e6ab44e1ae04611c76261637b9`.

Host harness компилирует извлечённые **фактические функции** кандидата `sc7180_acpi_smmu_create()` и ядра `acpi_smmu_get_data()`. Значения ресурсов берёт из бинарной IORT после проверки checksum и OEM ID/revision. qcom_smmu_create в harness заменён на выдачу метки выбранной структуры: аппаратные операции, готовность SCM и поведение generic probe этим не проверяются. Platform-match проверен аудитом кода; матрица harness проверяет селектор ресурсов внутри уже выбранной платформы.

## Воспроизведение

Патч применять только к отдельной копии исходников. Для object-build можно скопировать изменённый arm-smmu-qcom.c и локальные заголовки из drivers/iommu/arm/arm-smmu в отдельный каталог, добавить Makefile с `obj-m += arm-smmu-qcom.o`, затем:

```sh
make -C /path/to/configured-kernel M=/path/to/isolated-object \
  ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1 arm-smmu-qcom.o
python3 check-selection.py --kernel /path/to/configured-kernel \
  --candidate /path/to/isolated-object/arm-smmu-qcom.c \
  --iort /path/to/07-IORT.dat
```

Это не сборка загружаемого модуля: modpost и линковка полного ядра здесь не выполнялись. Использован репозиторный aarch64 GCC 15.3.1.

## Что ещё не решено

- Политика доменов клиентов и ACTLR по-прежнему выбирается по OF; см. [client-policy.md](../client-policy.md).
- Правильная реализация SMMU не создаёт отсутствующий GPU0.AVS0 и не добавляет ACPI-драйверы USB/графики.
- EPROBE_DEFER до готовности SCM нельзя считать доказанным сохранением экрана.
- Qualcomm cfg_probe/reset действительно выполняют MMIO и могут менять потоки DMA. Это не read-only probe.
- Причина прежнего чёрного экрана всё ещё не подтверждена последними строками ядра.
- До следующего аппаратного теста требуется диагностируемый загрузочный комплект и отдельное согласование перезагрузки.

Архив кандидата, объектов и логов: `/var/ftp/tmp/lav/tcl/acpi-smmu/selection-draft/`. Исторические рабочие Image/DTB/EFI не менялись.

## Файлы патчей

- [sc7180-acpi-smmu-selection.patch](../../../../patches/kernel/acpi/sc7180-acpi-smmu-selection.patch) — Раздельный выбор Apps MMU500 и Adreno SMMUv2 по IORT/ресурсам. Черновик; object-build ACPI=y/n и host-матрица; не установлен.
