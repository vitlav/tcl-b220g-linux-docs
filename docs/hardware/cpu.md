# CPU, частоты, температура и криптография

Qualcomm SC7180 / Snapdragon 7c предоставляет восемь CPU. В проверенной DT-конфигурации CPU0–5 относятся к частотному домену0, CPU6–7 — к домену1. **Аппаратно подтверждены** изменение частот под нагрузкой, TSENS-телеметрия и ускорение AES-GCM инструкциями CPU.

## CPUFreq

| Параметр | Домен0 | Домен1 |
|---|---|---|
| CPU | 0–5 | 6–7 |
| MMIO | 0x18323000 / 0x1400 | 0x18325800 / 0x1400 |
| CPUFreq policy | policy0 | policy6 |
| Доступный диапазон | 300000–1804800 kHz | 652800–2400000 kHz |
| Драйвер / governor | qcom-cpufreq-hw / schedutil | qcom-cpufreq-hw / schedutil |

Compatible: `qcom,sc7180-cpufreq-hw`, `qcom,cpufreq-hw`. Clocks xo от RPMh clock controller ID0 и alternate от GCC ID1. CPU связаны через clocks и qcom,freq-domain. [Полные ресурсы DT](full-inventory.md).

Частоты и напряжения берутся из аппаратной LUT. Изученный драйвер допускает отсутствие DT OPP table; в этой конфигурации CPU interconnect scaling не включён. Короткие нагрузки на CPU0 и CPU6 подняли состояния до 1804800 и 2400000 kHz соответственно; после нагрузки значения снизились.

`scaling_cur_freq` и `time_in_state` показывают состояние драйвера и не являются независимыми физическими частотомерами. Доступные минимумы не обязательно используются в каждом коротком окне измерения.

Проверка без изменения настроек:

```sh
for policy in /sys/devices/system/cpu/cpufreq/policy*; do
    echo "$policy"
    cat "$policy/related_cpus" "$policy/scaling_driver" "$policy/scaling_governor"
    cat "$policy/cpuinfo_min_freq" "$policy/scaling_cur_freq" "$policy/cpuinfo_max_freq"
done
```

Все значения частот в этой команде — kHz. Отсутствие строки MHz в /proc/cpuinfo не означает отсутствие CPUFreq.

## Температурные датчики

| TSENS | MMIO windows | GIC SPI / тип |
|---|---|---|
| 0, 15 sensors | 0xc263000/0x1ff; 0xc222000/0x1ff | 506 uplow, 508 critical; level-high |
| 1, 10 sensors | 0xc265000/0x1ff; 0xc223000/0x1ff | 507 uplow, 509 critical; level-high |

Compatible `qcom,sc7180-tsens`, `qcom,tsens-v2`; драйвер qcom-tsens. Размеры окон приведены как в DT, без округления. Все 25 thermal zones выдавали значения; после запуска наблюдалось 33.7–35.0 °C.

Зарегистрированы cooling devices cpufreq-cpu0 (max state9), cpufreq-cpu6 (max13), GPU devfreq (max6). Используется power_allocator. Thermal zones, trip points и cooling maps перенесены из SoC reference sc7180.dtsi; это не измеренная thermal policy корпуса TCL. GPU sustainable_power оценочный, что отмечает драйвер.

В полном графическом тесте максимум из выборок GPU составил 57.2 °C, CPU-named zones — 47.5 °C; шаг опроса 5 s мог пропустить краткие пики. Температурный throttling и critical shutdown специально не испытывались. Имена thermal zones cpu8/cpu9 — обозначения мест датчиков, а не дополнительные CPU.

## Простой и виртуализация

PSCI описан как arm,psci-1.0, method smc. Глубокие idle states в проверяемой конфигурации ограничены, использовался `cpuidle.off=1`; завершённая поддержка suspend и измеренная экономия мощности не подтверждены. Отсутствие cpuidle-драйвера не устанавливает отсутствие базового WFI.

В конфигурации проверенного ядра KVM включён, но журнал сообщает `All CPU(s) started at EL1` и `HYP mode not available`; /dev/kvm отсутствует. Это ограничение предоставленного загрузочной цепочкой режима. Наличие архитектурного EL2 у CPU не означает его доступность запущенному Linux. Прямое подтверждение ID_AA64PFR0 в этих проверках не получено. Рабочий запуск гостевой системы с KVM не выполнен.

## Криптографические инструкции

В /proc/cpuinfo присутствуют aes, pmull, sha1, sha2, asimd. В kernel Crypto API зарегистрированы aes-ce, xts-aes-ce, gcm-aes-ce и NEON-реализации. Это отдельный от [UFS ICE](storage.md) путь.

OpenSSL 3.5.5 ARM64 автоматически определил OPENSSL_armcap=0xbd. Тест AES-256-GCM: CPU6, блок 16384 байта, 3 s на запуск, порядок A/B/A. Частоты не фиксировались, в фоне работала камера.

| Режим | MB/s, 10^6 bytes |
|---|---:|
| Авто, A1 | 1825.84934 |
| Без AES/PMULL, 0x99 | 103.92371 |
| Авто, A2 | 1832.53948 |

Разница около 17.6 раза относится к библиотечному тесту в памяти. Команды для указанной сборки OpenSSL:

```sh
taskset -c 6 openssl speed -elapsed -seconds 3 -bytes 16384 -evp aes-256-gcm
env OPENSSL_armcap=0x99 taskset -c 6 openssl speed -elapsed -seconds 3 -bytes 16384 -evp aes-256-gcm
taskset -c 6 openssl speed -elapsed -seconds 3 -bytes 16384 -evp aes-256-gcm
```

0x99 — измеренное значение 0xbd без AES0x04 и PMULL0x20; override ограничен одним процессом. Для другой версии библиотеки следует сверить назначение битов, а не считать эту маску универсальной.

OpenSSH 10.2p1 в проверенной Ubuntu связан с libcrypto.so.3. Этот факт и тест OpenSSL не измеряют пропускную способность SSH, согласованный cipher конкретной сессии или VPN. OpenVPN на TCL в этой проверке не запускался. Экономия энергии и предел сетевой скорости по этим цифрам не установлены.
