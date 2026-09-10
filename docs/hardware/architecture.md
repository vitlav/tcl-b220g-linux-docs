# Функциональная схема

Это схема известных связей, а не электрическая принципиальная схема. Сплошные линии обозначают связь, подтверждённую рабочим DT или предыдущим аппаратным тестом; пунктир — отдельное исследование, данные ACPI/Windows или неполное описание. Питание не показано как точная разводка платы: используйте таблицу регуляторов.

```mermaid
flowchart TB
    UEFI["UEFI / Windows firmware"] --> GRUB["GRUB ARM64"]
    GRUB --> DT["Рабочая загрузка: DT + Linux 6.18.34"]
    GRUB -.-> ACPI["OEM ACPI: экспериментальная загрузка"]
    DT --> CPU["SC7180 / Kryo 468 / 8 CPU"]
    CPU --> GIC["GICv3 + PDC: IRQ / wakeup"]
    CPU --> TLMM["TLMM: GPIO / pinmux"]
    CPU --> SCM["SCM / SMC / TrustZone"]
    SCM --> FW["Защищённая firmware / PAS"]
    CPU --> RPMH["RPMh / AOSS: clocks, domains, regulators"]
    RPMH --> PMIC["PM6150 / PM6150L"]
    CPU --> ASMMU["Apps SMMU-500 @15000000"]
    ASMMU --> DPU["MDSS / DPU"]
    DPU --> DSI["DSI + DSI PHY"]
    DSI --> BRIDGE["LT8911EXB: firmware handoff"]
    BRIDGE --> PANEL["Встроенная матрица 1920×1080"]
    CPU --> GSMMU["Adreno SMMU @5040000"]
    GSMMU --> GPU["Adreno 618 + GMU"]
    GPU --> WAYLAND["Mesa / Weston / Wayland"]
    WAYLAND --> DPU
    CPU --> VENUS["Venus: видеодекодирование"]
    VENUS --> VIDEO["V4L2 / FFmpeg / mpv"]
    VIDEO --> WAYLAND
    CPU --> UFS["UFS host + PHY"]
    UFS --> DISK["Внутренний UFS: Windows, 6 LUN"]
    UFS --- ICE["ICE: inline crypto"]
    CPU --> USB["DWC3 + USB2 / USB3 PHY"]
    USB --> FLASH["USB: Ubuntu / загрузка / логи"]
    USB --> CAMERA["USB-камера"]
    USB --> PHONE["Внешний Android USB tethering"]
    CPU --> WIFI["WCN3990 / ath10k SNOC"]
    FW --> MPSS["MPSS remoteproc + OEM firmware"]
    MPSS --> WIFI
    MPSS --- RMTFS["RMTFS / shared memory"]
    MPSS -.-> GNSS["GNSS / LTE: отдельные эксперименты"]
    CPU --> I2CHID["GENI I²C @890000"]
    I2CHID --> KBD["Клавиатура @05; IRQ GPIO33; enable GPIO32"]
    I2CHID --> TP["Тачпад @2c; IRQ GPIO94; enable GPIO25"]
    I2CHID -.-> SAR["AW96105 @12; IRQ GPIO34"]
    CPU --> I2CEC["GENI I²C @888000"]
    I2CEC -.-> EC["EC @07"]
    EC -.-> BAT["Батарея / адаптер / крышка"]
    CPU --> I2CDISP["GENI I²C @a90000"]
    I2CDISP --> BRIDGE
    FW --> ADSP["ADSP / APR / Q6ASM / Q6AFE"]
    ADSP --> RX["LPASS RX macro / SoundWire RX"]
    RX --> WCD["WCD9385"]
    WCD --> SPEAKERS["Проверенный выход звука / динамики"]
    ADSP -.-> TX["TX / VA / микрофон: неполная проверка"]
    TX -.-> WCD
    CPU --> TSENS["TSENS: температура SoC"]
```

SCM не заменяет драйверы периферии. Apps SMMU и Adreno SMMU требуют разных Qualcomm implementation. Связь MPSS с Wi-Fi означает зависимость инициализации/firmware, а не прохождение всего сетевого трафика через модем. Линия звука не утверждает прямое электрическое подключение динамиков к выводам WCD без промежуточного тракта.

Численные параметры: [основная таблица](device-summary.md), [все 738 узлов](full-inventory.md), [живой DTS](live.dts).
