# AUDIO2 — подготовка, ещё не установлено

База /tmp/tcl-ice1-linux скопирована отдельно в /tmp/tcl-audio2-linux.
Release 6.18.34-tcl-audio2. Старые Image/модули/GRUB-пункты сохраняются.

- 0001: backport upstream GLINK 5a5a48e788e02 (bug19481).
- 0002: передача явной q6afe active_channels_mask (bug19478).
- 0003: кандидат APR IDR restore при повторном probe (bug19479), не upstream.
- 0004: дополнительные dev_dbg для ответов/записи Q6ASM, выключены по умолчанию.
- .config включает DYNAMIC_DEBUG/FTRACE/FUNCTION_GRAPH/KPROBES/KPROBE_EVENTS/FTRACE_SYSCALLS/OF_OVERLAY. Трассировка выключена по умолчанию. LOCKDEP оставлен для отдельной сборки.
- DT: прежний ADSP DT плюс уже проверенный live dai@0 Q6ASM, без изменения кодека/GPIO.
- init/collect/shutdown адаптированы из ICE1 только для release и log paths; collect дополнен APR/ADSP/ALSA.
- Initramfs GPU firmware и BusyBox извлечены побайтно из рабочего ICE1; OEM firmware не публиковать.

Сборка: make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olddefconfig; make -j12 Image modules. Кросскомпилятор прежний GCC15.3.1 ALT.

На текущей ICE1 аудиоканал удалён, unbind PID3248 завис в device_del; Wi-Fi/SSH работают. Не повторять unbind, не выгружать общий PAS. Для следующего теста потребуется загрузка подготовленного ядра после проверок.

Сборка завершена exit0. 949 модулей проверены по vermagic и побайтно в initramfs; scripts sh -n PASS; критические config y PASS; DT272nodes PASS. Локальный host test тела q6afe для масок3,5,0 и повторного использования PASS (не аппаратная проверка). Only four old unused-variable warnings Acer EC. Package /tmp/tcl-audio2-package.tar.gz SHA256 2c8a4b6103a8ea754ea3c072c193988ae12e76b207e2ca71cdcbcf51e9327b7b.


## Аппаратные проверки 2026-09-09

Boot 2109ce1f-a976-4457-b4f1-35f313948d45, kernel6.18.34-tcl-audio2, systemd running. Одна штатная перезагрузка вернула доступ после прежнего GLINK D-state. Wi-Fi автоматически на5220MHz/5GHz, Weston GL active, msmdrmfb, modem+ADSP running, все четыре APR сервиса. UFS sda..sdf readonly1, USBsdg rw.

1. Явная channel_mask3 теперь проходит hw_params; прежней AFE cmd100ef error2 нет. Q6ASM open10db3, mediafmt10d98, run10daa отвечают status0. PCMwrite по-прежнему EIO.
2. rmmod card/q6asm_dai/q6asm; новая загрузка q6asm с дополнительным WRITE_DONE dev_dbg; q6asm_dai+card mask0. APR service7 нормально отвечает, map-timeout отсутствует. При period12000frames, buffer48000: RUNNING, appl_ptr48000, hw_ptr0. Первый WRITE48000bytes адрес1:fff80000, map handleb0a06c88 принят send52/52. WRITE_DONE приходит через1.13s непосредственно перед ответом CLOSE, words fff80000,1,b0a06c88,0. По времени похоже на возврат при закрытии после таймаута, не доказательство нормального воспроизведения.
3. Закрыта карта, unbind только APR rpmsg-канала вернулся в ту же секунду. Канал и APR4сервиса автоматически восстановились через~2s. D-state не обнаружен, DSP не перезапускали.
4. После восстановления снова cardmask3, open/media/run status0, тот же поздний PCM EIO, таймаута map и ошибкиAFE нет. Карта оставлена подключённой, PCM закрыт; диагностическая печать q6asm выключена.

ЗВУКА НЕТ. Карта dummy-codec; физический кодек/выход/усилитель не настроены. Гипотеза: неработающий последующий аппаратный тракт/тактирование, пока не доказано. Linuxdummy ALSA не подтверждает физический WCD938x/ES83xx.

Пользователь попросил дождаться проверки настоящего звука перед новым отчётом в багу. Новые runtime-результаты пока сохраняются локально, баги не закрыты. Для визуально-звукового теста выбран прежний Big Buck Bunny в mpv с software decode и низкой громкостью; пока не запущен, так как PCMнеработает. Проверить громкость до включения.


2026-09-09 VA overlay experiment: shortened boot DT lacked AON/VA nodes and q6afe clock provider phandle. Initial status-only overlay returned EINVAL without changes. V2 replaced unused clock-provider via changeset to create phandle and added AON/VA; failed due __symbols__ absent in live DT, reverted0. V3 without -@ (__local_fixups__ retained) applied, but OF dynamic platform notifier assigned new provider parent to platform root, not APR device. Actual sysfs /sys/devices/platform/62400000.remoteproc:glink-edge:apr:service@4:clock-controller confirmed. q6afe_vote_lpass_core_hw expects dev_get_drvdata(dev->parent); NULL afe caused Oops mutex_lock+0x14 at address0x20. This was our runtime overlay construction error, not evidence of failed hardware or a standalone q6afe bug. Both DSPs/Wi-Fi/SSH remained up; global kernel taint4224. Do not continue clock operations in that tainted boot. Evidence va-overlay-experiment/dmesg-va-v3.log and parent-evidence.

Prepared boot DT va-probe.dtb: full original SC7180 AON/VA definitions, referenced q6afe clock phandle; correct device parent will come from q6afe devm_of_platform_populate at boot. Same audio2 kernel/initrd. Static audit PASS. New default tcl-audio2-va, oldaudio2 retained, EFI9bf487c31f44164c022a74c4c0d1228233929cef77e4c196024341898ae08147, DTf795878b99d960c253b2b2cd994cbe29e15b7f2aed2e9da169c63cb807de1dd0. Installed backupBOOTAA64-before-audio2-va.bak. Second necessary reboot issued session38395; waiting boot verification.

Windows qcauddev code VA14007e52c etc confirms SWR RX62610000/TX62630000/WSA62650000. Register translation function VA14000b4xx with base625ff000 for page1000 confirms RXmacro62600000; base6276d000 +3000 confirms VA62770000. Exact analog codec still not proven.

User requests: wait for actual sound check before NEW bug reports; BBB movie with low volume after sound works. No film yet, no nonzeroPCM, no amplifierGPIOwrites. Pending prior bug comments are from before this restriction; do not duplicate them.

После установки VA boot DT два SSH подключения пока timeout. Требуется отличить застрявшее shutdown после Oops от новой загрузки. Пользователю задан вопрос об экране; ответа пока нет. Фильм не запущен; bbb-sound-test.service — только черновик, volume5/max20, software video decode, явный ALSAdevice. После физической настройки выхода проверить ALSAгромкость отдельно, программный предел mpv не заменяет настройку усилителя.


## VA boot подтверждён после ручного выключения/включения

2026-09-09 boot ecc340af-a306-4585-b154-186000cdcc05, 6.18.34-tcl-audio2, systemd running. Пользователь видел чёрный экран после предыдущего штатного reboot из Oops; CapsLock реагировал, SysRqB нет. После аппаратного выключения и включения загрузился выбранный VA DT. Точный этап предыдущей остановки не установлен.

- AON62780000 → lpass-gfm-clk, VA62770000 → va_macro успешно bound.
- q6afe clock provider теперь дочерний APR service4 (проверено readlink), прежнего NULL-pointer Oops нет. Taint4096 только внешний handoff/диагностические модули, без DIE128.
- Wi-Fi/SSH/Weston и modem/ADSP автоматически работают.
- В журнале Unknown cmd0x100f6: это ответ на AFE_CMD_REMOTE_LPASS_CORE_HW_DEVOTE_REQUEST. Callback не имеет case для этой fire-and-forget команды; строка сама по себе не означает отказ DSP (ненулевой status печатался бы отдельно). Не считать первопричиной отсутствия звука.
- КартаTCLADSPDiagnost снова создана, явнаяmask3 проходитhwparams, PCM writeEIO сохраняется. PCM закрыт, маршрутMultiMedia1 включён. Ненулевых аудиоотсчётов и переключенийGPIO усилителя не было.
- Полный scontrols сохранён: Volume/Gain/Master/Headphone/Speaker регуляторов нет. Это всё ещё dummy-карта с маршрутамиDSP, не физический аудиокодек. Фильм не запущен; низкая громкость5/max20 только в черновике сервиса.
- Результаты va-boot/, архив /tmp/tcl-audio2-va-boot.tar.gz. В багу новый отчёт не отправлять до проверки реального звука, по указанию пользователя.

Следующий шаг: RXmacro62600000/SWR_RX62610000 (адреса подтверждены Windows кодом), корректные clocks/pinctrl/physical codec. В текущем bootDT phandle q6afecc и AON имеются, VA fsgen provider может не иметь phandle, так как на него пока нет ссылок. Не заменять APR clockprovider через обычный runtime-overlay повторно: OF notifier использует platform-parent lookup и теряет APR-parent. Новые SoC platform nodes этой проблемы не имеют. Проверить полный фрагмент и fsgen-ссылку до применения; не добавлять перезагрузки ради мелких изменений.


2026-09-09 parallel BBB test user requested despite unfinished audio. Initial mpv launch failed: --volume-max=20 invalid (mpv requires higher minimum); corrected to100, initialvolume5 actually verified by IPC. Unit tcl-bbb-sound-test in /run/systemd/system, software H264, gpu-next/GL/Wayland, ALSAhw:TCLADSPDiagnost,0. Video/audio opened, fullscreen showed black; IPC time-pos0.083333 unchanged over2s, pausefalse, volume5, currentaoalsa. Sent IPC set_property aid no: success, time-pos advanced0.125. USER CONFIRMED video started moving. Thus black fullscreen in this test was mpv playback blocked with broken ALSA path, not evidence of DRM display failure. User asked cancel to restore screen; systemctl stop sent before their confirmation of motion. Do not restart audio playback automatically. Max100 means no amplification above nominal, not20% cap. No audible sound, no new bug report per user instruction.


## 2026-09-09: RX macro and SoundWire identification, no reboot

Boot ecc340af-a306-4585-b154-186000cdcc05, kernel 6.18.34-tcl-audio2.
Runtime SoC overlay added RX macro at 0x62600000, compatible sm8250-lpass-rx-macro. Clocks q6afe phandle0xda IDs57/58/102/103, attribute1; VA fsgen supplied through clkdev alias because VA node has no phandle. APR provider unchanged. rx_macro driver bound; component visible in ALSA. LPASS codec version reported v1.0. RX output clock lpass-rx-mclk=9600000 Hz. Overlay module persistent until reboot; no forced removal.

Separate module enabled RX clock, read identification registers at 0x62610000, disabled clock again. No direct SoundWire register writes, no IRQ registration and no amplifier GPIO writes.

| Register | Value | Interpretation |
|---|---|---|
| HW_VERSION 0x000 | 0x01050001 | SoundWire 1.5.1 |
| COMP_PARAMS 0x100 | 0x02684026 | DOUT capacity6, DIN capacity1 (Linux masks bits4:0,9:5) |
| MASTER_ID 0x104 | 0x00000002 | controller ID2 |
| CFG 0x004 | 0 | controller not enabled |
| STATUS 0x014 | 0 | frame generator not enabled |

Taint4096 unchanged (OOT only), Weston active and SSH working after both probes. No audible sound yet. Next: identify IRQ ordering from OEM resources and actual port assignments; SM8250 template has5DOUT/0DIN and cannot be assumed exact from controller capacity alone. Windows disassembly immediate0x147..149 at tracing call sites are diagnostic IDs, NOT evidence of IRQ mapping. ACPI resource values remain evidence of IRQ set only. New bug publishing remains paused until sound test.
Sources in rx-live/. Boot files unchanged: runtime RX will disappear on reboot and must later be integrated into a verified boot DT.


## 2026-09-09 SoundWire probe and LPI experiment

Same boot ecc340af-a306-4585-b154-186000cdcc05. Found matching downstream resources in https://github.com/LineageOS/android_kernel_xiaomi_sm6150/blob/lineage-22.2/arch/arm64/boot/dts/qcom/atoll-audio-overlay.dtsi : RX62610000 ID2 SPI297, TX62630000 SPI296, WSA62650000 SPI295. Matches TCL ACPI329/328/327 after GIC offset32. Five RX output ports are used despite hardware capacity6. Applied mainline SoundWire1.5.1 RX overlay using actual RX phandle0xdd, SPI297 edge, SM8250 five-port timing template. Driver bound, sdw-master-2-0 created, actual GIC329 interrupt received. No slave detected, bus clash reported. This is not proof of codec failure.

Full qualcomm-registers debugfs dump reads FIFO registers and generated read FIFO underflow errors itself. Do not repeat whole-register dump as passive diagnostic.

Atoll LPI confirms GPIO3=RXclock and GPIO4/5=RXdata, func1, pin base627c0000, slew physical6295a000. Mainline SC7280 mux assignments match these three pins. Initial test hog on pinctrl itself failed because driver registers groups after devm_pinctrl_register; no groups available during hog application. Removed failed overlay3 and applied LPI plus separate consumer. Pins muxed, but slew configuration Oops in lpi_config_set+0x1ec. ROOT CAUSE OUR DT: mainline driver adds LPI_SLEW_RATE_CTL_REG=0xa000, while test supplied already-offset base6295a000 with length1000. Correct resource must be base62950000 length10000. Translation fault is beyond mapping; not evidence of unsupported peripheral. Corrected DT source saved, UNTESTED. No boot files changed.

Taint now4224 (DIE128+OOT4096); SSH still works. Stop hardware mutations pending user-authorized reboot. Existing boot default tcl-audio2-va does not include temporary RX/SWR/LPI overlays, so reboot clears them. Do not reload failed modules or force-remove in this boot. Full Oops and all source variants in swr-lpi-live/. No audible sound; no new Bugzilla publishing until sound test. Next clean boot: RX overlay, corrected LPI controller+separate consumer FIRST, then SoundWire; inspect enumeration before amplifier or playback.


## 2026-09-09 clean reboot and fixed LPI success

User authorized reboot. systemctl reboot completed; SSH/Weston returned automatically. New boot c781c4d4-cd36-4a07-b734-40b059ac3e52, kernel6.18.34-tcl-audio2, taint4096 only. Applied RX overlay id1, then corrected LPI overlay id2 with separate consumer and slew resource62950000/10000, then SoundWire overlay id3. RX pinctrl client active, GPIO3/4/5 mux and configuration applied without Oops. SoundWire master2 registered without previous bus clash, no slaves enumerated. No audio played, no PA GPIO writes. Boot files unchanged; these are runtime experiments.

Critical next constraint: OEM AUDD GPIOUID0 maps ACPI GIO0 pin58; PA uses46/47. GPIO58 is excluded by existing gpio-reserved-ranges=<58 5>, which previously cured reproducible TLMM boot hangs. Do NOT blindly remove exclusion or directly read/write reserved GPIO MMIO. Exact failing pin within58..62 was never isolated. GPIO58 role as codec reset remains hypothesis, not established fact. GIO0 ACPI resource03400000 size00c00000; Linux TLMM tile mapping must be compared with qcgpio.sys. Local disassembly saved /tmp/tcl-audio-research/qcgpio-disassembly.txt. Simple opcode search finds no SMC/HVC, which does not exclude an indirect firmware interface.

Current regulator list does not establish codec supplies. Atoll L10A/L15A/BOB assignments must NOT be copied blindly: TCL ldo10 visible WiFi3.3V, whereas Atoll codec wants1.8V. Need OEM board-specific power mapping. GPIO46/47 remain out-low. Whole SoundWire debugfs register dump avoided (FIFO read side effects). No new bugs published pending real sound test.


## 2026-09-09 codec-presence and Windows GPIO addressing

Clean boot c781c4d4-cd36-4a07-b734-40b059ac3e52 remains, taint4096, Weston active. Selected read-only SoundWire registers collected via module using pm_runtime_resume_and_get on bound controller; FIFO registers excluded. Result at uptime536.452168:

| Register | Value | Meaning |
|---|---|---|
| CFG 0x004 | 0x00000003 | controller enabled, pulse IRQ |
| COMP_STATUS 0x014 | 0x00000001 | frame generator enabled |
| IRQ_STATUS 0x200 | 0x00004000 | raw status; not decoded as a new failure |
| ENUM_CFG 0x500 | 0x00000001 | auto enumeration enabled |
| BUS_CTRL 0x1044 | 0 | snapshot |
| MCP_STATUS 0x104c | 0 | bank0 |
| SLAVE_STATUS 0x1090 | 0 | no attached slave status |
| ID1/ID2 slot1 0x538/0x53c | 0 / 0 | no enumerated identity |

Linux qcom_swrm_enumerate calls sdw_slave_add(bus,&id,NULL) for unlisted devices. Therefore missing codec DT alone does not explain absent slave sysfs entry: hardware slave-status is zero with active frame generator. This narrows next investigation to physical codec power/reset/link, but does not identify the faulty member.

Windows qcgpio.sys static analysis: functionVA140002430 probes three tile candidates at resourcebase+tileoffset+(pin<<12)+0x10, selects bit0 and caches tileoffset atVA1400092a0. TableVA140007158 has0x100000,0x500000,0x900000. With ACPI GIO0 base0x03400000 these yield0x03500000/0x03900000/0x03d00000, exactly Linux tile bases. GPIO routines e.g.VA1400024a8 directly read/write MMIO using this cache. No evidence of a simple GPIO58 renumbering or special secure path was found; this is not proof of its absence everywhere. GPIO58 is WEST in Linux and thus predicted control0x0353a000, NOT accessed. Reserved58..62 unchanged; prior boot hang was localized only to group, not exact pin. Codec reset role/polarity remains unproven. No reboot or GPIO writes this turn.

Evidence/source in codec-presence/. OEM power rail mapping remains unresolved; do not copy Atoll supply names/voltages to TCL. No sound and no new Bugzilla report.


# Windows audio GPIO control: partial static reconstruction

2026-09-09. Offline analysis of preserved qcauddev7180.sys, no device writes.

Registry reader VA14004f858 allocates 40-byte records. Confirmed fields by string references and store destinations:

| Offset | Field | Evidence VA |
|---|---|---|
| +0x00 | GPIOUID | 14004f9f4..14004fa04 |
| +0x04 | ACPI resource INDEX | 14004fa58..14004fa60 |
| +0x08 | INITIALVALUE | 14004fab8..14004fac0 |
| +0x10 | I/O target handle, initially NULL | used14005d4c0, initialized14004fb20 |
| +0x18 | active/valid marker, initialized1 | 14004fb08 |
| +0x1c | reference counter, initialized0 | increment14005d780, decrement14005d898 |
| +0x20 | cached/override state, exact initialization unresolved | 14005f2cc |

AUDD global table pointer VA1400237b0 and countVA1400242a8; MBHC has separate table. Lookup14005d380 takes group0=AUDD/group1=MBHC and searches GPIOUID, not INDEX. Read helper14005d3f8 sends I/O control0x480000 with output1byte. Write helper14005d560 sends0x480004 with input1byte through140047260. GPIO control14005d5e8 selects UID and writes byte1 for requested state1, byte0 for state0, with reference counting for AUDD. Thus the observed path is ordinary GPIO I/O target requests; it does not itself imply a direct secure-monitor operation.

Value dispatch around14005f26c supports requested0/1, requested2 loads INITIALVALUE, and requested-1 toggles cached state (xor1). Therefore INITIALVALUE is a default to restore, NOT an active-low flag. This corrects a possible misleading inference from INF alone. Actual GPIOUID0 caller and reset timing remain unresolved. No conclusion that GPIO58 can safely be accessed under Linux. Reserved GPIO58..62 stay excluded.

Firmware PMIC resource binaries29a524/2e257c contain LPASS register labels but string search did not identify a named external-codec power client. This negative search does not prove missing supplies or firmware support. Do not infer voltages from Atoll rails.

Next static work: trace UID0 use through the codec OS callback table and reconstruct GPIO target open from ACPI resource INDEX. Hardware remains at working RX/LPI/SoundWire master with no slave responses. No sound test or bug publication.

## 2026-09-09 — OEM codec GPIO58 reset request sequence recovered

Static analysis of qcauddev7180.sys SHA256 `923326aa1538fc699e52a11d6c5f594ea086d1abc31b76740005626deff6fe3e`; evidence in `windows-gpio-flow/codec-reset-chain.asm`, `codec-reset-pe-evidence.txt`, `qcauddev-imports.txt`.

| Stage | Confirmed addresses / values |
|---|---|
| Callback registration | 14003624c installs 14003d590 into init structure; 140051e6c reads offset0x58; API command0x601 reaches140064f88 and stores pointer at140022148 |
| GPIO backend | 14006d990(selector,state), static table14001c2a0 stride12; selectors0..7 have backend5; produces callback outer opcode4 |
| Board mapping | 14003d800 dispatcher: selector7 -> AUDD group0 UID0; selector0 -> MBHC group1 UID0; selectors1..4 -> AUDD correspondingUID |
| Board resource | OEM INF AUDD UID0 INDEX0 INITIALVALUE0; DSDT AUDD first GpioIo is GIO0 pin0x3a=58 in both BSID branches |
| Reset request | 14006c768 calls setter(7,0); 14006c770 delay(5); 14006c77c setter(7,1); 14006c784 delay(2) |
| Timing units | delay14000b030 ->1400088a8 multiplies input by -10000 and calls IAT140018188, independently resolved through PE import table as KeDelayExecutionThread: requested relative intervals5ms and2ms |
| Caller | 14006c9dc calls reset routine14006c6b0 in initialization branch w20==1, after two successful140081b48 calls, before14006c4f0 calls with0and1 |
| Disable path | 14006be9c requests setter(7,0) |

This supersedes the previous unresolved UID0 caller/timing note. The code requests low -> wait5ms -> high -> wait2ms, strongly supporting active-low codec reset. These are software-requested intervals, not measured electrical timing. GPIO writes go through the previously reconstructed IOCTL0x480004 path with reference-count/override gating; static calls alone do not prove that every invocation produces both electrical edges. INITIALVALUE remains a default value, not a polarity flag.

Live read-only verification: same boot c781c4d4-cd36-4a07-b734-40b059ac3e52, uptime21min, taint4096, Weston active, SoundWire only sdw-master-2-0, no newly logged kernel Oops. Existing selected register snapshot still shows no slave response. No GPIO58..62 access, voltage change, bootfile change, reboot, PCM test, or Bugzilla publication performed.

Remaining blocker: establish safe Linux access/ownership and exact tile for GPIO58, plus board-specific codec supplies. A prior boot hang was isolated only to group58..62, not to pin58 individually. Windows dynamically probes GPIO tiles; Linux SC7180 chooses a static tile. Do not infer a safe register write or remove the whole reservation from this reset-sequence evidence. Also decode the two140081b48 prerequisite calls and clock-vote request preceding the pulse before designing a hardware test.

## TCL OEM audio power prerequisites — 2026-09-09

Confirmed by preserved board ACPI and static OEM driver analysis, then read-only CMD DB probe on live Linux. No power votes or GPIO writes performed.

## OEM board tables

DSDT PEP0.APMD returns APCC in BOTH BSID branches; APCD is not selected by that method. APCC AUDD contains24 components. Do not use the pin-empty APCD component5/6 definitions as the active board configuration.

| AUDD component | APCC active PSTATE0 resource | Linux correspondence |
|---|---|---|
| 1 | PPP_RESOURCE_ID_BUCK_BOOST1_C, raw voltage0x324b00=3300000 | PM6150L id c, resource bobc1, node bob |
| 2 | PPP_RESOURCE_ID_LDO15_A, raw voltage0x1b7740=1800000 | PM6150 id a, resource ldoa15, node ldo15 |
| 5 | TLMMGPIO entries0,1,2,14 | TX LPI pin group; raw fields retained in APCC.dsl.txt |
| 6 | TLMMGPIO entries3,4,5 | RX LPI pin group, matches current Linux mux ownership |
| 3,4 | no explicit resource package | Empty table does not prove these components have no firmware-side effects |

APCC PSTATE1 clears enable in regulator tuples. Do not assume raw PMIC mode numbers map to Linux enum values. No audio LDO10_A vote appears in this APCC component list; do not copy Atoll LDO10 supply based on name alone.

## Driver path

Prerequisites140081b48 called with indices0and1 before reset construct separate bus contexts, synchronization objects and callbacks;14007ead0 then ORs masks0x3f and0x3f00 into register6295a000. It issues outer callback7 with payload(enable1,selector1) for index0, and(enable1,selector0) for index1. Callback14003d468 maps these to AUDD component5 and6. Helper14005f410 goes via140054d88 to IAT140018218=PoFxActivateComponent, then140018250=PoFxIssueComponentPerfStateChange. Import identities independently checked against saved PE IAT order. These are pin-group power-state requests, not external reset GPIO writes.

Immediately preceding the reset pulse,14006ac40 sends outer callback1 through14006a0c8. Handler14003d2e0 maps four change fields to components4,3,2,1 (offsets12,0,4,8), using values1=activate,-1=deactivate,0=no change. Thus the callback path reaches the board's LDO15/BOB components; exact which change fields are nonzero depends on prior runtime state. Do not claim every reset call necessarily enables both rails anew.

## Live comparison and CMD DB probe

Current DT regulator banks are pm6150 id a and pm6150l id c. Neither ldo15 nor bob registered in regulator_summary. This means Linux has no consumer votes for those providers in this configuration; it does NOT prove that firmware has electrically disabled the rails.

Read-only module compiled against6.18.34-tcl-audio2, loaded and normally unloaded on boot c781c4d4-cd36-4a07-b734-40b059ac3e52. Resources ldoa15=0x42200,bobc1=0x40400 are present, type4(VRM), auxiliary data4bytes. ldoa10=0x41d00 and ldoc10=0x41900 confirm A/C are different resources. No RPMh request issued. Taint remains4096. Code and exact output saved here.

Current mux ownership confirms RX gpio3/4/5 assigned to rx-pin-test. Pinconf debugfs displays empty strings, so it cannot independently verify every electrical setting. SoundWire still only master; no sound confirmed.

## Voltage constraint before hardware test

Kernel qcom-rpmh-regulator.c pmic5_pldo_lv table:1504000+8000*n;1800000 is representable (n37). pmic5_bob:3000000+32000*n;3300000 is NOT representable. Neighbours3288000/3320000. Exact min=max3300000 would reject voltage selection. Do not silently round this to3320000 or expand the allowed voltage range without establishing codec tolerance or OEM PMIC rounding behavior. CMD DB presence alone does not establish such tolerance.

Next: identify OEM BOB voltage rounding/range and remaining codec supply path; prepare regulator providers with confirmed constraints. Resolve GPIO58 ownership/tile independently before reset experiment. Reserved58..62 stay excluded. No reboot, bootfile edits, or Bugzilla publication.

## Exact OEM audio voltage test — 2026-09-09

## Correction: software voltage table is not proof of hardware quantization

0x00324b00 equals3300000 decimal microvolts exactly. Binary representation does not cause a different voltage. Linux pmic5_bob lists3000000+32000*n, which excludes3300000; this restriction is in the Linux voltage-selector table. It does not establish the PMIC request resolution.

OEM PmicDxe SHA e28eb7f6a6e3c6d41a627e478e7a124ab55057a590952084801920dfe3c02401 confirms BOB initialization at RVAad34 by pm_bob_driver.c string references. RVAae38..ae64 reads a subtype-dependent upper limit: two-byte limit times1000, or (af6c..af80) one-byte limit times32000. Crucially both paths converge ataf84: RangeMin=0 (afac), RangeMax=decoded limit (afcc), VStep=1000 (afa0,afdc). Thus32000 in this function scales the upper-limit encoding, not the request step. No firmware was executed.

Related Qualcomm source fetched via gh at pinned commit20bb8ae36c93fc16bbadda0e0a83f930c0c8a271 agrees: pm_bob_driver.c sets VStep1000; pm_pwr_alg.c converts requested microvolts to Vset by integer division. Source URLs in sources.txt. This supports issuing the original3300mV request rather than rounding it to3320mV. It does not measure the physical output or its accuracy.

## Controlled runtime tests

Fixed-purpose module uses the already-bound RPMh regulator bank devices and verifies resource addresses and absence of competing ldo15/bob DT providers. Exact active-only OEM requests:

| Resource | RPMh base | Voltage request (mV) | Mode | Enable |
|---|---|---:|---:|---:|
| ldoa15 | 0x42200 | 1800 | 7 (LDO HPM) | 1 |
| bobc1 | 0x40400 | 3300 | 6 (BOB AUTO) | 1 |

Requests bypass the Linux regulator selector table for this bounded diagnostic only. They are not a production regulator driver or persistent DT solution. Delayed work automatically sends enable0 for both resources after20seconds; script and normal module exit also perform cleanup. Voltage/mode request values remain cached; only enable votes are cleared. Other firmware clients may have their own votes, so do not describe this as measured physical power-off or full restoration of all register state. No sleep/wake votes, reserved GPIO, internal disk, or boot configuration changed.

Boot c781c4d4-cd36-4a07-b734-40b059ac3e52. First test uptime2602s: all six requests ret0; after20s both disable requests ret0 and disable_complete1. No SoundWire slaves. Before test COMP_STATUS1; during at2605s COMP_STATUS8 (frame-generator bit0 clear). First sample coincides with3s autosuspend, so repeated with power/control=on, trap restoresauto.

Second test uptime2753s: runtime_statusactive before/during; COMP_STATUS8 already BEFORE supply enable and remains8 DURING. Therefore autosuspend alone does not explain the current register state, and the first change cannot confidently be attributed to rail power. All requests/disable ret0 again. Both modules normally unloaded. Taint4096 unchanged; Westonactive; SSHaccessible. No sound confirmed.

Important limitation: a negative slave result with frame-generator bit0 clear is not a decisive power/reset test. Next investigate the SoundWire active-but-no-frame status and missing clock/reset dependencies before testing GPIO58. Do not blindly rebind the driver: its remove() only deletes bus and disables clock, with no explicit runtime-PM teardown in this source. Reserved58..62 remain untouched. No Bugzilla publication pending sound validation.

## 2026-09-09 — SoundWire resume and wider search

Live trace: v1.5.1 hardware confirmed; full VA/RX/SWR clock chain active still leaves COMP_STATUS8 and no slaves. Two clock-stop calls return -ENODATA; subsequent frame-generator waits vary between success and failure. Causality not established. Isolated reset/re-enumeration candidate compiled, NOT loaded. No reboot, sound, or Bugzilla publication.

Broader search found a same-driver Qualcomm RFC about ignored enumeration timeout (2026-06-23), downstream SM6150 reset-vs-clock-stop paths, and AMD PM recovery discussion. No exact TCL fix found. Sources, applicability table, traces and candidate: `.claude/docs/tcl-b220g-data/peripherals/camera-audio/audio2/swr-resume-trace/README.md`. PM-core correction: device_unbind_cleanup calls pm_runtime_reinit; lack of explicit cleanup in qcom remove alone is not proof normal replacement is impossible.

## 2026-09-09 — runtime candidate tested; frame generation recovered, codec absent

Same boot c781c4d4-cd36-4a07-b734-40b059ac3e52. Normal module replacement, no reboot or bootfile changes. Original soundwire_qcom restored after both tests; temporary supplies unloaded and power controls returned to auto. Weston active, taint4096 unchanged.

| Test | Observed result | Limitation |
|---|---|---|
| Candidate v1, uptime7594–7616s | Three reset-resume cycles; six selected snapshots all COMP_STATUS1, no slaves. | Normal replacement exposed Unbalanced pm_runtime_enable on entry and restoration. No sound. |
| Candidate v2, uptime9143–9178s | Two reset-resume cycles; four snapshots COMP_STATUS1. Explicit PM disable for inherited state and removal prevents new imbalance warnings. | Test-only PM replacement handling; not a production driver patch. |
| v2 plus 20s OEM supply requests | LDO15_A1800mV/mode7 and BOB_C3300mV/mode6 all RPMh requests ret0. Three snapshots while enabled and one after disable all COMP_STATUS1, slaves0. Automatic disable_complete1. | Requests acknowledged, physical voltages not measured; reset GPIO untouched. |

The reset recovery consistently restores frame-generator bit0 in these bounded tests. This is stronger than the previous compiled-only hypothesis, but not a complete audio fix or proof that absent ClockStopNow ACK is the underlying cause. Initial probe before the first reset still logs a failed frame wait. No codec enumeration, sound playback or sound confirmation.

### Correction to PM-core interpretation

Our preceding statement about generic cleanup was incomplete. drivers/base/power/runtime.c:pm_runtime_reinit returns immediately when pm_runtime_enabled(dev). Thus qcom remove lacking pm_runtime_disable really does leave runtime PM enabled across driver unbind; the next probe logs Unbalanced pm_runtime_enable. This warning is not WARN_ON and taint stayed4096. Candidate v2 disables inherited PM at probe and disables PM at remove. Both scripts force VA/RX/SWR power/control=on for replacement; the removal code is tested under that condition only, not generalized to arbitrary suspended unbind. Do not repeat v1 for future tests.

Sources and final v2 module/script/patch saved in validated-runtime-test/. Original initial candidate files in parent are historical compiled-only artifacts. resume-candidate-result.txt corresponds to v1 with diagnostic messages; resume-supplies-result.txt corresponds to v2. The exact v1 instrumented binary was overwritten in /tmp; historical parent binary predates those messages and must not be described as byte-identical to v1 tested. v2 archived module is the tested binary.

Next investigate safe access to codec reset GPIO58 and remaining physical-link prerequisites. Existing OEM static evidence requests low5ms/high2ms; it does not prove Linux can access the pin. Reserved58..62 remain untouched because their reservation previously cured boot hang. No Bugzilla publication before sound validation.
