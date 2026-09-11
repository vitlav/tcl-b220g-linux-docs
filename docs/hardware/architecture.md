# Функциональная схема

Это схема известных связей, а не электрическая принципиальная схема. Сплошные линии обозначают связь, подтверждённую рабочим DT или предыдущим аппаратным тестом; пунктир — отдельное исследование, данные ACPI/Windows или неполное описание. Питание не показано как точная разводка платы: используйте таблицу регуляторов.

Схема разложена на вертикальные блоки по подсистемам. Повторяющиеся SC7180 и firmware обозначают одни и те же узлы; подробные ресурсы и степень проверки приведены в [спецификациях подсистем](../subsystems.md).

## Загрузка

```mermaid
flowchart TB
    UEFI["UEFI / Windows firmware"] --> GRUB["GRUB ARM64"]
    GRUB --> DT["Рабочая загрузка: DT<br/>Linux 6.18.34"]
    GRUB -.-> ACPI["OEM ACPI<br/>экспериментальная загрузка"]
    DT --> CPU["SC7180 / Kryo 468<br/>8 CPU"]
```

## Процессор, прерывания и питание

[Ресурсы общей инфраструктуры](platform.md).

```mermaid
flowchart TB
    CPU["SC7180 / Kryo 468<br/>8 CPU"] --> GIC["GICv3 + PDC<br/>IRQ / wakeup"]
    CPU --> TLMM["TLMM<br/>GPIO / pinmux"]
    CPU --> SCM["SCM / SMC<br/>TrustZone"]
    SCM --> FW["Защищённая firmware<br/>PAS"]
    CPU --> RPMH["RPMh / AOSS<br/>clocks, domains, regulators"]
    RPMH --> PMIC["PM6150 / PM6150L"]
    CPU --> TSENS["TSENS<br/>температура SoC"]
```

## Графика и видео

Полные пути ICC, такты и питание: [DT-дисплей](display.md) и [ACPI-дисплей](display-acpi.md). Физические связи SC7180 одинаковы; способы перечисления устройств и подтверждённая поддержка описаны раздельно.

```mermaid
flowchart TB
    CPU["SC7180 / Kryo 468<br/>8 CPU"] --> ASMMU["Apps SMMU-500<br/>@15000000"]
    ASMMU --> DPU["MDSS / DPU"]
    DPU --> DSI["DSI + DSI PHY"]
    DSI --> BRIDGE["LT8911EXB<br/>firmware handoff"]
    BRIDGE --> PANEL["Встроенная матрица<br/>1920×1080"]
    CPU --> GSMMU["Adreno SMMU<br/>@5040000"]
    GSMMU --> GPU["Adreno 618 + GMU"]
    GPU --> WAYLAND["Mesa / Weston / Wayland"]
    WAYLAND --> DPU
    CPU --> VENUS["Venus<br/>видеодекодирование"]
    VENUS --> VIDEO["V4L2 / FFmpeg / mpv"]
    VIDEO --> WAYLAND
    CPU --> I2CDISP["GENI I²C<br/>@a90000"]
    I2CDISP --> BRIDGE
```

## Внутренний накопитель

```mermaid
flowchart TB
    CPU["SC7180 / Kryo 468<br/>8 CPU"] --> UFS["UFS host + PHY"]
    UFS --> DISK["Внутренний UFS<br/>Windows / 6 LUN"]
    UFS --- ICE["ICE: inline crypto"]
```

## USB и внешние устройства

```mermaid
flowchart TB
    CPU["SC7180 / Kryo 468<br/>8 CPU"] --> USB["DWC3<br/>USB2 / USB3 PHY"]
    USB --> FLASH["USB-накопитель<br/>Ubuntu / загрузка / логи"]
    USB --> CAMERA["USB-камера"]
    USB --> PHONE["Внешний Android<br/>USB tethering"]
```

## Wi-Fi, модем и GNSS

```mermaid
flowchart TB
    CPU["SC7180 / Kryo 468<br/>8 CPU"] --> WIFI["WCN3990<br/>ath10k SNOC"]
    FW["Защищённая firmware<br/>PAS"] --> MPSS["MPSS remoteproc<br/>OEM firmware"]
    MPSS --> WIFI
    MPSS --- RMTFS["RMTFS / shared memory"]
    MPSS -.-> GNSS["GNSS / LTE<br/>отдельные эксперименты"]
```

## Клавиатура, тачпад, датчики и EC

```mermaid
flowchart TB
    CPU["SC7180 / Kryo 468<br/>8 CPU"] --> I2CHID["GENI I²C<br/>@890000"]
    I2CHID --> KBD["Клавиатура @05<br/>IRQ GPIO33<br/>enable GPIO32"]
    I2CHID --> TP["Тачпад @2c<br/>IRQ GPIO94<br/>enable GPIO25"]
    I2CHID -.-> SAR["AW96105 @12<br/>IRQ GPIO34"]
    CPU --> I2CEC["GENI I²C<br/>@888000"]
    I2CEC -.-> EC["EC @07"]
    EC -.-> BAT["Батарея / адаптер / крышка"]
```

## Звук

```mermaid
flowchart TB
    FW["Защищённая firmware<br/>PAS"] --> ADSP["ADSP / APR<br/>Q6ASM / Q6AFE"]
    ADSP --> RX["LPASS RX macro<br/>SoundWire RX"]
    RX --> WCD["WCD9385"]
    WCD --> SPEAKERS["Проверенный выход звука<br/>динамики"]
    ADSP -.-> TX["TX / VA / микрофон<br/>неполная проверка"]
    TX -.-> WCD
```

SCM не заменяет драйверы периферии. Apps SMMU и Adreno SMMU требуют разных Qualcomm implementation. Связь MPSS с Wi-Fi означает зависимость инициализации/firmware, а не прохождение всего сетевого трафика через модем. Линия звука не утверждает прямое электрическое подключение динамиков к выводам WCD без промежуточного тракта.

Численные параметры: [основная таблица](device-summary.md), [все 738 узлов](full-inventory.md), [живой DTS](live.dts).
