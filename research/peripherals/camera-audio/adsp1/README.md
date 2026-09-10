# TCL: проверка звука и черновик ADSP, 2026-09-09

## Состояние

Живое ядро6.18.34-tcl-ice1: /proc/asound/cards, aplay -l, arecord -l — no soundcards. Единственный remoteproc0 — работающий modem. remoteproc@62400000, smp2p-lpass, lpass@62d87000 disabled. QDSP6/PAS модули собраны; CONFIG_OF_OVERLAY выключен. Звук не проверен проигрыванием: PCM-устройств нет. Изменений живого DT, GPIO, прошивки, ядра и загрузчика не делалось; перезагрузки не было.

## Подтверждения из OEM

- qcauddev7180.inf, профиль AUDDReg_CLS_7180, выбранный SUBSYS27822202: GPIOUID0/1/2 соответствуют индексам0/1/2 в ACPI AUDD._CRS, т.е. GPIO58/46/47. Обе ветки BSID возвращают здесь одинаковые ресурсы.
- Профиль speaker analog PA использует GPIOUID1 и2, stage2: следовательно GPIO46 и47 участвуют в включении усилителя. Полярность и полный порядок включения из этих строк не установлены. Ничего не переключалось.
- Строка ES_CODEC/AudioDeviceDriver_8100_For_Emdoor_QL328 в qcauddev7180.sys не устанавливает модель чипа. ES8316/ES8326/Realtek не подтверждены. Realtek-параметры QSP нельзя применять к CLS.
- qcsubsys_ext_adsp7180.inf, строки212/216: alignment0x100000, reservation0x2800000 (40MiB).
- Родной qcadsp7180.mbn: 13463676 bytes, SHA256 24d665684966cab76eb0ca58536cd96f9e978267f52ffef1dd300d8e3dfe2860. ELF32, PT_LOAD отмечены relocatable; физический диапазон заголовков 0x90b00000–0x93300000 exclusive, span0x2800000, ровно40MiB. Это адреса ELF, не измеренное runtime размещение Windows.
- Диапазон0x90b00000–0x93300000 не пересекает именованные reserved-memory текущего DT; находится в ранее установленном PIL reserved pool. Контроль всех ограничений TrustZone/firmware при запуске ещё не выполнен.

## Подготовлено, НЕ установлено

adsp-probe.dts/dtb основаны на текущем Venus1 DT, добавляют carveout40MiB по0x90b00000, включают smp2p-lpass и ADSP PAS, задают отдельный OEM firmware path. Это только черновик транспорта ADSP, звуковой карты и codec routes в нём нет. Статический аудит271достижимого узла: отсутствуют disabled suppliers и ошибки phandle в проверяемом графе. Аудит расширен для qcom,smem-states; целочисленные qcom,smem IDs smp2p не интерпретируются как phandle. Это не полная DT-schema-валидация и не доказательство аппаратной работоспособности.

Следующее: установить точный codec/bus и последовательность питания из OEM бинарника/данных, проверить пригодность firmware/PAS и зарегистрировать APR/Q6AFE. Затем подготовить описанную звуковую карту и тихое воспроизведение. Простого modprobe или установки аудиосервера недостаточно. Для штатного включения отсутствующих активных узлов понадобится следующая диагностическая загрузка с новым DT, но этот черновик ещё не готов как функциональный audio boot.

Источники: сохранённые DSDT.dsl и OEM INF/SYS/MBN; локальные исходники6.18.34 arch/arm64/boot/dts/qcom/sc7180.dtsi, drivers/remoteproc/qcom_q6v5_pas.c, sound/soc/qcom/sc7180.c. Поиск аналогов не дал проверенного описания звука именно TCL/Emdoor QL328.

DT overlays: support exists in our source6.18.34, but CONFIG_OF_OVERLAY=n; OF_DYNAMIC=y and CONFIGFS_FS=y. Enabling OF_OVERLAY requires rebuilt kernel (bool), not modprobe. Upstream docs describe in-kernel of_overlay_fdt_apply API; current drivers/of has no userspace configfs overlay interface. Need suitable loader as well. ADSP reserved-memory handling is early __init in of_reserved_mem.c; overlays do not retroactively reserve its40MiB. Plan initial reservation in boot DT. https://docs.kernel.org/devicetree/overlay-notes.html . Audio findings Bug19084 comment181158.

2026-09-09 ADSP1 boot prepared: EFI/grub adds default tcl-adsp1; same ICE1 kernel/initrd, DT with40MiB ADSP carveout. Firmware staged only /var/tmp/tcl-adsp1/firmware-staged, intentionally absent from /lib/firmware until post-SSH manual start. Modem uses same PAS driver, so driver-wide blacklist/unload must NOT be used. PAS ADSP auto_boot=true, missing firmware expected to leave it offline; then supply firmware and start targetADSP only. Boot logger enabled condition tcl.adsp=probe, saves root+USB6minutes. Old EFI backupBOOTAA64-before-adsp1.bak. Installation checksums passed; no codec or PA GPIO changes.

## ADSP1 hardware result: PASS transport, sound still absent

Boot3f72d7f5-839f-4deb-9d1a-afd403ce9cce. Wi-Fi/SSH returned, ADSP remoteproc1 offline until firmware supplied. OEM firmware hash24d665684966cab76eb0ca58536cd96f9e978267f52ffef1dd300d8e3dfe2860 verified then manual start succeeded. Modem remains running. Q6CORE/AFE/ASM/ADM drivers bound on aprbus; AFE DAIs/clocks/routing registered. One-shot module query confirmed API versions3=5,4=7,7=2,8=4 (ret0), then unloaded. readiness API1 alone not proof due unsupported-command fallback. Concrete versions provide live command/response evidence.

ALSA still no soundcard. Q6ASM frontend dai probe failed No dais found in DT (-22), expected absent board PCM child definitions. Exact physical codec/routes unresolved. Generic AFE DAI list is not board hardware enumeration. FASTRPC no reserved DMA memory remains separate issue. Logger initially failed before remoteproc existed; fixed empty-glob handling before firmware start. Correct APR sysfs bus is aprbus. Firmware now exists on root, so next boot may autostart once available; not yet reboot-tested.

Runtime archive /tmp/tcl-adsp1-runtime.tar.gz. Module sources/binary in query/. One diagnostic reboot, no amplifier GPIO or internal UFS writes. Next prepare PCM/machine/codec mapping; do not claim audible sound.

Next-step adsp-pcm-draft.dts/dtb adds one generic Q6ASM frontend dai@0 (reg0), resolving missing child definition structurally. Static audit272nodesPASS. NOT installed, no sound card/backend/codec defined, no claim of PCM registration/playback. Do not reboot solely for this draft; combine with verified remaining audio configuration.


## 2026-09-09: live PCM и отдельная ошибка q6afe

Без перезагрузки модуль `live-q6asm/tcl_q6asm_live.ko` через OF changeset добавил dai@0 (reg=0) в Q6ASM; повторный bind q6asm-dai успешен. CONFIG_OF_OVERLAY=n этому не мешает: используется OF_DYNAMIC, не интерфейс DTBO. Модуль намеренно без module_exit, не выгружать принудительно: дерево используется потребителями.

`diagnostic-card/tcl_adsp_card.ko` создал TCL-ADSP-Diagnostic с dummy codec, MultiMedia1 → RX_CODEC_DMA_RX_0 (0xb030). Это искусственная диагностическая карта, физический кодек и усилитель ещё не настроены.

- Явная channel_mask=3: DSP отвергает AFE_PORT_CMD_SET_PARAM_V2 (0x100ef), error=2, hw_params -EINVAL.
- channel_mask=0: драйвер вычисляет 3, hw_params проходят; позднее aplay завершается write EIO. ADSP остаётся running. Звука пока нет.
- В q6afe_cdc_dma_port_prepare отсутствует перенос ненулевой входной маски. Отдельная бага https://bugs.etersoft.ru/19478 блокирует https://bugs.etersoft.ru/19084 (связь установлена). Подробности q6afe-channel-mask-bug.md. Исправленный модуль ещё не установлен.
- Текущая диагностическая карта оставлена с channel_mask=0; PCM закрыт, маршрут MultiMedia1 включён. GPIO усилителя не переключались.
- Логи на TCL: /var/tmp/tcl-adsp1/pcm-zero-test.log и pcm-zero-default-mask.log.

OEM ACDB: Speaker_cal device 0x45 = SPEAKER_OUT (совпадает с CLS INF), DEVICE_SPEC_INFO содержит путь 0x11103 и порт 0xb030. Codec_cal путь 0x11103 назван HEADSET_SPEAKER_STEREO. Строка CDCNAME WCD938x.1.0 присутствует также в QSP: это ещё не доказательство физического WCD938x на TCL. Остальные поля не расшифрованы окончательно.


2026-09-09 PCM trace follow-up: CONFIG_DYNAMIC_DEBUG/KPROBES/FTRACE disabled. Built temporary q6asm with rate-limited callback/write logs, saved pcm-trace/. Replacing q6asm after removing card and q6asm_dai exposed APR lifecycle bug19479: service7 absent from IDR after reload, map opcode10d92 times out. This is a new failure, not original PCM write EIO.

Attempted parent APR audio rpmsg unbind (not PAS/modem). It blocked in kernel: bash PID3248 Ds, device_del → device_unregister → rpmsg_unregister_device → qcom_glink_destroy_ept → rpmsg_dev_remove → unbind_store. APR devices and ALSA card already removed; ADSP remains running, Wi-Fi/SSH works. Do not claim audio service restored. Remote exec session55338 still pending; script cannot advance to bind while unbind blocked. Avoid another unbind, force unloading PAS or kill -9. Need recovery/reboot planning after preserving logs; no reboot performed this turn.

Prepared q6afe-active-mask.patch for19478, not yet installed/tested. Next kernel checklist: ../../../../kernel-next-TODO.md (actual repository path .claude/docs/tcl-b220g-data/kernel-next-TODO.md).
