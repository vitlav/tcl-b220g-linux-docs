# CPUFreq hardware test

База — проверенный UFS/EC/Wi-Fi DTB. Включён cpufreq@18323000, CPU0–5 связаны с freq-domain0, CPU6–7 с freq-domain1 через clocks и qcom,freq-domain. Изменения сохранены в dtb.diff. Поддержка ARM_QCOM_CPUFREQ_HW встроена в ядро6.18.34-stb-qc7+.

По исходнику qcom-cpufreq-hw v6.18 отсутствие DT OPP table допускается: частоты/напряжения берутся из аппаратной LUT, ICC scaling отключён. CPU interconnects и OPP tables на этом этапе не восстанавливались. CPU power-domains и глубокие idle states остаются отключены, cpuidle.off=1 сохранён. Это отдельный тест частот, не завершённая настройка энергосбережения.

GRUB запрашивает cpufreq.default_governor=schedutil (параметр проверен в cpufreq.c v6.18). Реальный выбранный governor и доступные частоты нужно проверить после загрузки. Новый пункт TEST: CPU frequency scaling + UFS + Wi-Fi выбран по умолчанию. Прежний UFS-пункт и BOOTAA64-before-cpufreq.bak сохранены.

EFI SHA256 ddf237a46c16f6921937f3fd9d83b92a4a63e90cbddb6b240360ff9c2d266485; DTB f88a6854d0e1085b32b5f92cbca01e391b61248d512bcf00003ee080a83f7713. Запись и повторное чтение проверены. Ядро/initramfs прежние. После запуска проверить policy0/policy6, driver, governor, related_cpus и LUT, затем короткую нагрузку на каждый домен. Не считать scaling_cur_freq независимым измерителем фактической частоты.

### CPUFreq работает после автоматической перезагрузки

Boot 2b0f6945-4e68-425a-8b0d-ebbf7de2af76, Wi-Fi SSH проверен на uptime50. Драйвер qcom-cpufreq-hw зарегистрирован, обе policy используют schedutil. CPU0–5:300000–1804800 кГц; CPU6–7:652800–2400000 кГц. scaling_min/max совпадают с cpuinfo_min/max.

Короткие независимые нагрузки taskset CPU0/CPU6 по5сек: policy0 поднялась до1804800, policy6 до2400000; после нагрузки значения снизились. Статистика time_in_state показывает основную долю времени на768000/825600 кГц. Это подтверждает изменение состояний драйвером; scaling_cur_freq и time_in_state не являются независимым физическим частотомером. Доступные минимумы300000/652800 в этом окне статистики не использовались.

MPSS running, Wi-Fi/SSH, UFS и i2c-2 сохранены. Read-only UFS после новой загрузки установлен повторно; автоматической постоянной защиты пока нет. Глубокий сон по-прежнему отключён, ICC scaling не включён. Экономия мощности количественно не измерена. Логи, ограничения policy и SHA256 сохранены в peripherals/cpufreq-probe/logs/2b0f6945-4e68-425a-8b0d-ebbf7de2af76/.
