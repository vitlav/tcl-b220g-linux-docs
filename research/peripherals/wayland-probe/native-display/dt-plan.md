# TCL: план дисплейного DT, не загрузочный overlay

Рабочий DT: sc7180-tcl-i2c10.dtb, SHA1929faac529880bcf7bd87d83ce30cccf0bf28c074141b61657af81fb44cc71c.
2026-09-08 проверены прямые ссылки дисплея в декомпиляции, полный транзитивный аудит ещё не завершён.

Предлагаемый тракт: DPU → DSI@ae94000 + PHY@ae94400 → LT8911EXB(I2C a90000/0x29) → встроенная eDP панель. Это внешний мост, не штатный SC7180 DP-controller@ae90000; включать его для этой панели не требуется.

| Узел | Сейчас | Действие перед тестом |
|---|---|---|
| interconnect@1740000 (mmss-noc),phandle a0 | disabled | Поставщик mdp0-mem у MDSS; нужен вместе с дисплеем, проверить BCM/RPMh цепочку |
| clock-controller@af00000 (dispcc),a3 | disabled | Clocks и MDSS GDSC; проверить supply/parent цепочку |
| display-subsystem@ae00000 | disabled | Включать после готовности graph/bridge и suppliers |
| display-controller@ae01000 | Родитель disabled | Существующая связь DPU→DSI имеется |
| dsi@ae94000 | disabled | Выход port1 endpoint пуст, связать с мостом; уточнить supplies |
| phy@ae94400 | disabled | Проверить analog supply по схеме/рабочему родственному DT, не угадывать |
| LT8911EXB I2C child | Отсутствует | Новый binding и driver, не активировать преждевременно |
| GPU/GMU | disabled | Оставить выключенными на первом KMS тесте |

Другие прямые MDSS suppliers: GCC100000, config-noc1500000, mc-virt1638000, gem-noc9680000, SMMU15000000, RPMh clocks/power18200000. В этих узлах direct status=disabled не найден; это не доказательство live bind или полной готовности. Нужно проверить их живое состояние и рекурсивные зависимости.

Нельзя ограничиться status=okay для MDSS/DSI: обнаружен также выключенный MMSS NoC, без которого возможен deferred probe. Окончательный DTB не создан и не установлен. Сначала драйвер, binding, power sequence и supplier audit; потом отдельный тестовый пункт с сохранением рабочего EFI.

## Проверка на работающем TCL

На boot e7269aed-4e6c-42d4-a2c7-fefdad4573cd: GCC100000 привязан к gcc-sc7180; config-noc1500000,mc-virt1638000,gem-noc9680000 — к qnoc-sc7180; SMMU15000000 — к arm-smmu; RSC18200000 — к rpmh. devices_deferred пуст. Узлы MMSS NoC1740000,DISPCCaf00000,MDSSae00000 не созданы (disabled в DT). msm.ko имеется; modinfo dispcc_sc7180 сообщает builtin. Чтение статуса, без modprobe/bind/GPIO/смены режима.
