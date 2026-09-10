# Черновой ACPI SCM порт TCL — не готов к аппаратному тесту

2026-09-10. DSDT SCM0 HIDQCOM080B, без_CRS/_CCA; родитель_SB также без_CCA (scm-cca.log).

- qcom_scm.c:2333 probe вызывает devm_of_icc_get. interconnect/core.c:562 безOFnode возвращаетENODEV. Это воспроизводимый по коду блокер простого ACPI-match.
- Подготовлен scm-acpi-draft.patch: добавить linux/acpi.h, ACPI_IDQCOM080B и acpi_match_table; ICCзапрашиватьтолькоприOFnode. Только эти изменения.
- Применён исключительно к /tmp/tcl-acpi1-linux/drivers/firmware/qcom/qcom_scm.c. Исходник before сохранён. Objectbuild `make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- W=1 drivers/firmware/qcom/qcom_scm.o` успешен.
- ПолныйImage с этим патчем НЕсобирался; последняяACPI1ImageнаUSBостаётсяпрежней. Рабочиеaudio2/memdiagдеревьяНЕизменены. Нельзя путать исходныйACPI1кодпослечерновика с ранее собраннымImage.

## Нерешённая DMA-зависимость

ACPI_CCA_REQUIRED=y, acpi_dma_supported требуетcca_seen. Отсутствие_CCAуSCM0ипредковведёткdma_dummy_opsчерезplatform_dma_configure/acpi_dma_configure_id. qcom_tzmemвыделяетпамятьdma_alloc_coherent;initialpoolsize0можетотложитьошибкудопервойреальнойаллокации. Поэтому успешныйprobe/availableнеозначаетготовностьSCMбуферов. Не назначатьcoherentбезсверкисDT/SCMсемантикой.

## Прочие этапы probe

find_dload_addressбезOFphandleвыходитбезадреса;clocksoptional;reserved-memory-ENODEVдопускается;GENERICtzmemневызываетSHMBRIDGEenable;IRQoptional-ENXIOдопускается. После__scmпубликацииидут__get_convention, set_download_mode, disable_sdiприdownload_mode0, затемQSEECOM/QTEEинициализацияпоконфигурации. АппаратныхSMC/MMIOоперацийсчерновикомнебыло.

GitHubпоискчерезgh: QCOM080Bвqcom_scm.cиopenbsd/srcневыдалсовпадений. Этоограниченныйпоиск,неутверждениеоботсутствиивсехготовыхрешений.

## DMA и черновик v2 (2026-09-10)

Живой DT MEMDIAG проверен по /sys/firmware/devicetree/base: SCM, firmware и root не содержат dma-coherent/dma-noncoherent/dma-ranges/iommus. CONFIG_ARCH_DMA_DEFAULT_COHERENT не включён; of_dma_is_coherent возвращает dma_default_coherent=false. Следовательно, программная DMA-семантика рабочего SCM — noncoherent. Это подтверждение выбора Linux, а не отдельное измерение свойств TrustZone.

Подготовлена дополнительная SSDT scm-cca0.asl: External SCM0 DeviceObj, Scope(SCM0), Name(_CCA,Zero). OEM DSDTнеизменена. iasl20250807:0errors/0warnings/0remarks. AML70байт,SHA256d63bfb4959a75431c1416a34a99aa702888a436fed435304e4eafbdb5d044b25. В acpiexec20260408 OEMDSDT+SSDTуспешнозагружены; evaluateSCM0._CCA возвращает0. Эта таблица НЕустановлена наTCL.

_CCAдолженприсутствоватьдоACPIсозданияplatformdevice: acpi_platform.c172 задаётdma_mask32биттолькоприacpi_dma_supported,иначе0. Нельзя просто подгрузить свойство посленеудачногоprobeиобъявитьDMAисправленным. БудущийACPIкомплектдолжензагрузитьSSDTраньшесканирования,черезпроверенныймеханизмраннихACPIтаблицinitramfs.

Актуальный scm-acpi-draft-v2.patch включает предыдущиеID/ICCизменения и:
- раннийотказ с понятнойошибкойприотсутствии_CCA;
- initial_size=PAGE_SIZEдляACPIпулаTZmem, чтобыпроверитьреальнуюаллокациюдопубликации__scm (DTinitial_size0сохранён);
- acpi_dev_clear_dependencies послеуспешногоprobe.
ОбъектW=1скомпилировануспешно. GENERICTZmemвэтомядреиспользуетdma_alloc_coherent,неSHMBRIDGE;приnoncoherentdeviceDMAAPIсамобеспечиваетсоответствующеепредставлениепамяти. Наличия_CCAнедостаточнодлядоказательстваработоспособностиSMCбуферов.

Изменёнтолькоqcom_scm.cв/tmp/tcl-acpi1-linux. ПолныйImagev2нестроился,наTCLнеустанавливался. Требуетсяаппаратнаяпроверкааллокации+безопасногозапроса,проверкапобочныхдействийSCMprobe,затемSMMUотдельно. НикакихSMC/MMIOвызововсновымкодом/перезагрузокнебыло.


## Ранняя загрузка таблицы проверена локально

2026-09-10: [early-ssdt/README.md](early-ssdt/README.md) — воспроизводимая сборка несжатого CPIO, проверка настоящим earlycpio.c, положительные и отрицательные сценарии размещения, повторная AML evaluation. На ноутбук комплект не установлен.
