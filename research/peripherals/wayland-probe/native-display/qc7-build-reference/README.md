# Подготовка совместимой сборки LT8911EXB для qc7

2026-09-08. Драйвер не собран и не загружен. На TCL нет /lib/modules/6.18.34-stb-qc7+/{build,source} и подготовленных заголовков в /usr/src. vermagic msm: `6.18.34-stb-qc7+ SMP preempt mod_unload aarch64`. В config MODVERSIONS выключен. Это облегчает отдельную сборку, но не доказывает ABI-совместимость по одному vermagic.

Рецепт закреплён по commit b626cbd286665293e0cf9d72b97d1a980f1a9fe2:
https://github.com/hexdump0815/linux-mainline-qcom-kernel/tree/b626cbd286665293e0cf9d72b97d1a980f1a9fe2
Конфигурация config.qc7-6.18.34-stb-qc7+ SHA2568d89d4b6f3558818b468f8bd919fb283d6b503c3ae94ff4005c53891e3a93f49 побайтно совпадает с ранее сохранённым конфигом рабочего релиза. В release только один tar.gz46MB; отдельного assets headers нет (содержимое tar на этом шаге повторно не проверялось).

Рецепт использует stable6.18.34 плюс travmurav-changes-v6.12.patch из каталога v6.18, add-new-dts-files-to-makefile.patch, add-gamma-lut-support.patch, внешний fix-kernel-version/v6.12.5.patch и draft Galaxy Book DTS. Первые три патча сохранены, внешний version patch и draft ещё не закреплены. Это не готовое восстановленное дерево исходников.

В travmurav patch найдены исправления DISPCC rot_clk/GDSC при runtime suspend и назначения DPU planes. Gamma patch меняет DPU. Следовательно, текущее ядро не чистый stable и не Ubuntu generic; замена на vanilla6.18 с тем же config не эквивалентна.

## Задание сборщику (alt-packaging-agents)

1. Восстановить точный исходный stable tag6.18.34, перечисленные патчи и generated config; зафиксировать все ревизии/хеши, kernelrelease и отличия итогового config.
2. Подготовить дерево для внешнего модуля, проверить требуемые exported DRM/I2C/DSI symbols. Отключённый MODVERSIONS не отменяет проверку ABI; modules_prepare сам по себе не является успешной сборкой драйвера.
3. После реализации driver+binding из ../driver-review.md собрать отдельный LT8911EXB модуль для qc7. Не менять модуль msm или запущенное ядро без обоснованной необходимости.
4. Передать исходники, config, команды, полный build log, .ko/vermagic/dependencies, контрольные суммы и список неразрешённых вопросов. Не устанавливать/не загружать на TCL в процессе сборки.

## Питание пока не подтверждено

В сохранённом upstream6.18 Acer Aspire1 DTS mdss_dsi0.vdda=vreg_l3c_1p2, mdss_dsi0_phy.vdds=vreg_l4a_0p8. Это родственная плата, не электрическая схема TCL. До первого теста установить TCL источники по OEM firmware/таблицам либо иному прямому свидетельству; не добавлять guessed supplies в загрузочный DT. На TCL DT DSI/PHY supplies сейчас отсутствуют, узлы отключены.
