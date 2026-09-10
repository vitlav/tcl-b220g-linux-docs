# Wi-Fi preparation findings

Current successful TCL DT has Wi-Fi disabled. Kernel has ATH10K_SNOC=m, QMI helpers and QRTR. Windows exports already include wlanmdsp.mbn and bdwlan/bdwlanu board variants. No matching board file selected yet; do not pick one at random.

TCL DSDT AMSS.QWLN CRS confirms MMIO0x18800000/0x800000 and MSA0x93900000/0x200000. The latter matches original TCL DTS and upstream Acer SC7180 reference and lies inside the verified reserved region0x85b00000–0x945fffff. DT reference Wi-Fi supplies L9a,L1c,L2c,L10c,L11c require checking against TCL PEP voltage votes before enabling.

Upstream Acer enables remoteproc_mpss with qcmpss7180_nm.mbn. ath10k SNOC without wifi-firmware child chooses use_tz; its presence alone does not prove QMI service availability. Need establish firmware boot path/MPSS dependencies and board ID, then add Wi-Fi tools/authentication. No Wi-Fi test image prepared, no interface observed under Linux yet.

## 2026-09-07: код ядра и поиск аналогов после services-теста

Проверены upstream v6.18 (через gh): ath10k/snoc.c, ath10k/qmi.c, qcom_q6v5.c, qcom_pd_mapper.c, net/qrtr/ns.c и af_qrtr.c; qrtr/src/addr.c версииv1.1. Это upstream-база, не доказательство идентичности всем downstream-патчам экспериментального 6.18.34-stb-qc7+.

Цепочка: ath10k_snoc_probe -> ath10k_qmi_init -> qmi_add_lookup(WLFW). Probe0 подтверждает создание клиента, но не готовность Wi-Fi. При появлении сервиса: регистрация уведомлений -> host capabilities -> MSA memory info/permissions/ready -> capability (chip/board/fw IDs). После MSA_READY ядро получает board-файл через ath10k_core_fetch_board_file и передаёт его через QMI BDF download. FW_READY приводит к ath10k_core_register. Поэтому отсутствие phy/boardID согласуется с остановкой до этих событий; конечный QRTR-снимок не исключает кратковременное появление службы раньше.

Уточнение предыдущего объяснения: board data в этом ath10k передаёт ядро по QMI; это не обязательный TFTP-запрос. tqftpserv обслуживает запросы дополнительных файлов от DSP, в текущем тесте RRQ/WRQ отсутствуют. wlanmdsp специфичен для платформы, универсальная замена WCN3990 из другой SoC неверна: commit https://kernel.googlesource.com/pub/scm/linux/kernel/git/srini/linux-firmware/+/a0142c57045701b7557c3060af5c4246c420e4d8 .

qcom_q6v5.c q6v5_fatal_interrupt читает текст причины из SMEM и вызывает rproc_report_crash; dog_hal_common.c — исходник закрытой firmware, не Linux-драйвера. Ядро не содержит причину ожидания внутри DSP.

**QRTR адрес:** af_qrtr.c задаёт qrtr_local_nid=1. qrtr v1.1 addr.c делает getsockname и сразу возвращается, если адрес уже1. Следовательно прежняя гипотеза, что qrtr-ns -f1 изменил адрес и тем вызвал сбой, не подтверждена. EADDRINUSE/dormant ожидаем при встроенном NS; второй демон лишний, но не установленная причина watchdog. Проверка kill -0 не проверяет работу NS.

**PD mapper:** CONFIG_QCOM_PD_MAPPER=y; root compatible содержит qcom,sc7180; sc7180_domains включает msm/modem/root_pd и msm/modem/wlan_pd (wlan/fw). Лог probe qcom_common.pd-mapper.0=0 на64.840606 до MPSSup64.912522, конечная служба64 также видна. Нельзя говорить, что mapper отсутствует. Найден patch Loic Poulain от2026-09-01 с точно таким watchdog через40с на Agatti/Arduino UNO Q; исправление softdep для порядка загрузки модульного mapper не переносится автоматически на нашу builtin-конфигурацию. Источник https://www.mail-archive.com/linux-kernel%40vger.kernel.org/msg2653217.html (индекс первичной рассылки также https://lists.openwall.net/linux-kernel/2026/09/01/2243 ).

**Рабочий аналог QC710:** публикация Val Packett от2026-01-11 описывает SC7180/QSIP7180P, Windows package qcwlan7180.inf_arm64_38b56292be899426 и bdwlan.b67. Наш SHA256 полностью совпал: da2e615dea087b66889d09ab627e091710a35ce05167b0bd95cf4926196a62e1. В рабочем аналоге qmi-board-id=67/qmi-chip-id=320/variant=ECS_QC710. Это кандидат, но не доказательство ID TCL; variant другой платы в наш DT не добавлен. Источник https://www.mail-archive.com/ath10k%40lists.infradead.org/msg17504.html .

**RMTFS отдельное направление:** текущий DT явно отключает rmtfs_mem. В публикации об SC7180 описана зависимость networking от корректного rmtfs/SCM: https://lkml.rescloud.iu.edu/2102.2/06743.html . Это про иной сбой SCM, не готовое объяснение нашего watchdog. До включения нужны подтверждённые memory reservation и файлы modem storage именно TCL. Сейчас не включали и UFS не трогали.

CONFIG_ATH10K_DEBUG, ATH10K_DEBUGFS и DYNAMIC_DEBUG выключены: включение debug_mask не обеспечит отсутствующий в бинарнике диагностический код. Пока подготовлен сбор ранней последовательности через существующий qrtr-lookup и sysrq-w; вопрос отладочной сборки при необходимости передаётся сборщику пакетов.

## 2026-09-07: приоритет следующего шага — RMTFS TCL

Новые локальные подтверждения: OEM qcremotefs7180.inf привязан к ACPI QCOM0817; строки qcremotefs7180.sys содержат /boot/modem_fsg, /boot/modem_fsc и пути разделов диска. Наличие драйвера не доказывает, что конкретный запуск MPSS ожидает этот сервис.

DSDT.dsl: RMTB=0x80600000, RMTX=0x00200000 (строки128–129), RFMB/RFMS/RFAB/RFAS=0. RFS0._CRS подставляет эти значения в ресурсы QCOM0817. Это отличается от типового rmtfs_mem94600000 в sc7180.dtsi. Диапазон80600000–807fffff уже был виден как EFI reserved2MiB. Поэтому нельзя просто включить типовой узел94600000; требуется описание памяти TCL и проверка прав SCM/VMID. В этом этапе DT/USB не менялись.

Сверены modemr.jsn и modemuw.jsn OEM: root_pd/wlan_pd instance180, службы gps/gps_service, tms/pdr_enabled, kernel/elf_loader, wlan/fw совпадают с таблицей SC7180 kernel PD mapper. tms/servreg ядро добавляет каждому домену автоматически в qcom_pdm_add_domain. Простая замена таблицы mapper не имеет найденного обоснования.

Официальный репозиторий RMTFS https://github.com/andersson/rmtfs проверен через gh. README отсутствует, изучены rmtfs.c/sharedmem.c, локальные копии сохранены. rmtfs.c поддерживает -o storage_root, -r avoid writing to storage, -P partitions; точные semantics хранения нужно сверить по storage.c перед запуском. Предпочтительный тест — копии необходимых modem storage в RAM с корректной общей памятью; внутренние разделы Linux пока не подключать для записи. Сначала собрать read-only карту разделов и убедиться, какие файлы/разделы TCL реально использует; не создавать пустые данные взамен заводских вслепую.

Текущий kernel: KPROBES/FTRACE/ATH10K_DEBUG/DYNAMIC_DEBUG выключены. Полный захват внутренних kernel QMI через эти механизмы нельзя включить одной командой; нужен подходящий готовый отладочный kernel или задача сборщику пакетов. Обычный qrtr-lookup показывает объявления, не трафик запросов. Приоритет: проверка RMTFS по конкретным OEM/ACPI данным, затем отладочное ядро если причина не выявлена. UART полезен для раннего boot, но Linux уже сохраняет логи, поэтому сейчас не обязательное условие.
