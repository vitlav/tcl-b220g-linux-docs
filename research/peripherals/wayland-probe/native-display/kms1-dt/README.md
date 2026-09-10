# KMS1: DT-кандидат для проверки, не установлен

2026-09-08. База: рабочий sc7180-tcl-i2c10.dtb SHA256 1929faac529880bcf7bd87d83ce30cccf0bf28c074141b61657af81fb44cc71c. base.dtsi получен из этой базы; сравнение сортированной декомпиляции побайтно подтвердило её соответствие сохранённому рабочему DTB.

Изменения в changes.diff:
- Включены MMSS NoC1740000, DISPCCaf00000, MDSSae00000, DSIae94000 и PHYae94400.
- Добавлены vdda-supply=PM6150L C3 (существующие 1200000uV) и vdds-supply=PM6150 A4 (существующие 880000uV). Напряжения, режимы и constraints регуляторов не меняются. Это согласуется с OEM ресурсом disp_edp, требованиями драйверов и назначениями upstream Acer Aspire1 SC7180; физическая схема TCL отсутствует, поэтому это обоснованный тестовый выбор, а не измеренная карта цепей.
- Добавлен I2C10 bridge@29 с экспериментальным compatible и двусторонней связью DSI port1 → bridge port0, четыре data-lanes.
- USB/UFS/Wi-Fi, GPU/GMU, native DP и остальная база без изменений.

Команды:
```sh
dtc -I dts -O dtb -o kms1.dtb kms1.dts
python3 audit.py kms1.dtb > suppliers.json
```

dtc exit0. В базе320 строк предупреждений, в кандидате323: три дополнительных строки — `also defined at` для дополненных узлов; новых категорий предупреждений не появилось. Исходные предупреждения декомпилированного DT не скрывались. Это не dt-schema validation: для экспериментального bridge binding ещё нет.

Аудит audit.py разбирает бинарный FDT, проверяет phandle/specifier по #*-cells и рекурсивно проходит clocks, resets, power-domains, interconnects, iommus, phys, dmas, mboxes, interrupts-extended, io-channels, interrupt-parent, OPP, BCM voters, nvmem, memory-region, supplies и pinctrl плюс родителей. Начальные узлы — активные поддеревья MDSS, DISPCC, MMSSNoC, I2C10. Результат:57 узлов, отсутствующих ссылок и disabled suppliers в проверенной цепочке нет. Graph remote-endpoint проверен через итоговую декомпиляцию/dtc; это не проверка всех возможных vendor-specific зависимостей, наличия драйверов или успешного hardware probe.

## Питание и ограничения handoff

В живой системе C3 уже используется USB PHY (use1,1200mV), A4 — USB2/USB3 PHY (use2,880mV); новые supply references добавляют потребителей, а не заменяют USB связи. regulator_summary сообщает состояние framework, не измерение тестером. OEM C8/1.8V не описан в текущем DT и пока не добавлен: назначение/удержание моста требует отдельной проверки.

Рабочая cmdline содержит clk_ignore_unused и pd_ignore_unused, но не regulator_ignore_unused. Последний параметр поддерживается данным ядром (drivers/regulator/core.c) и может использоваться как диагностическое удержание зарегистрированных unused rails; он не восстанавливает неизвестное питание и не обеспечивает suspend/resume.

Для первого испытания планируется загрузка нового согласованного kernel+modules комплекта с отложенной загрузкой MSM/bridge после установления SSH, сохранением логов на rootfs и USB. Требуется проверить фактическую реализацию блокировки автозагрузки в initramfs/rootfs; одного обещания или наличия modprobe.blacklist недостаточно. Рабочий EFI/DTB не заменены, новый пункт не установлен.
