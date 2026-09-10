# TCL KMS1: GPU candidate

Kernel unchanged: 6.18.34-tcl-kms1, handoff v2. New default entry tcl-kms1-gpu; old tcl-kms1-early retained. GPU, GMU, GPU SMMU and GPUCC enabled. Existing display/power/USB/Wi-Fi settings retained. The bridge endpoint phandle number changes during compilation; its resolved target remains the same.

## Firmware and memory

A618 kernel catalog requests qcom/a630_sqe.fw and qcom/a630_gmu.bin. Official linux-firmware files are pinned in provenance.json; LICENSE.qcom and NOTICE.qcom downloaded from LICENSES/ (root-level license request was 404).

OEM qcdxkmsuc7180.mbn from this TCL's Windows driver is installed as qcom/sc7180/tcl/qcdxkmsuc7180.mbn. It is not included in this public source directory. Initramfs contains this OEM binary and must be treated accordingly when publishing.

The OEM ELF contains one relocatable LOAD segment; qcom_mdt_get_size computes 4096 bytes. Reserved region is 0x80840000 + 0x2000, inside firmware-reserved 64 KiB region 0x80840000–0x8084ffff. Evidence: grub-lsefimmap-photo-verified.md and early boot 0b07456b dmesg. Static fit is not proof of secure authentication or functional GPU.

## Checks

DT supplier audit includes qcom,gmu: 77 nodes, no missing references or disabled suppliers. CONFIG_SC_GPUCC_7180=y, CONFIG_NVMEM_QCOM_QFPROM=y, CONFIG_ARM_SMMU=y, CONFIG_QCOM_SCM=y, CONFIG_QCOM_MDT_LOADER=y, CONFIG_DRM_MSM=m.

Initramfs contains 949 modules and three GPU firmware files. Byte comparison against stage, source scripts and BusyBox passed. Original root/USB detection, UFS write protection and log collection remain. GRUB script and ARM64 EFI checks passed using coherent Debian GRUB 2.12 tools/modules.

Before enabling GPU, vulkaninfo reports Turnip GPU-ID initialization failure and only CPU llvmpipe. This establishes the baseline. Installed through epm: vulkan-tools 1.4.341.0+dfsg1-1, glmark2-wayland and glmark2-data 2023.01+dfsg-3. Existing mesa-vulkan-drivers 26.0.8-1ubuntu0.3.

## Build

Run dtc on gpu.dts with its relative include tree. Run build-initramfs.py with BusyBox, module directory, firmware root, output gzip. Kernel/module compilation is not needed. Firmware is required in both initramfs (early DRM) and Ubuntu root (later opens).

## Pending hardware validation

Boot new DT, save dmesg/deferred devices and Vulkan result, then test Wayland GL/Vulkan. Require Adreno/Turnip device; CPU software rendering does not qualify. Old early-console entry is the rollback path.

### Аппаратный GPU и Vulkan заработали

Boot ID 7bf9601b-61ad-465c-a4e3-ad6b8447ab05. Новый GPU DT загрузился, ранний framebuffer msmdrmfb и автоматический SSH/Wi-Fi сохранились. Vulkaninfo с принудительным freedreno_icd.json определяет Turnip Adreno (TM) 618, integrated GPU, vendor 0x5143, device 0x6010800, API 1.3.335, Mesa 26.0.8-1ubuntu0.3. До теста был только CPU llvmpipe.

Weston 14.0.2 запущен с --renderer=gl: GL_VENDOR freedreno, GL_RENDERER FD618, OpenGL ES 3.2. Wayland vkcube --wsi wayland --c 600 с VK_DRIVER_FILES=/usr/share/vulkan/icd.d/freedreno_icd.json выбрал Turnip Adreno 618 и завершился Result=success/ExecMainStatus=0. glmark2-wayland --size 800x600 --benchmark build:duration=10.0 определил FD618 / OpenGL 4.6 Compatibility и завершился успешно (3169 FPS, score3168). Это короткая проверка одной сцены, частично одновременно с vkcube, а НЕ полный сравнительный benchmark.

Поиск ошибок GPU hang/fault в dmesg после короткого теста их не выявил. Сохраняются сообщения dummy vdd/vddcx и sync_state pending GMU; это не мешало текущим тестам, управление питанием/длительная стабильность ещё не проверены. Полные логи в native-display/kms1-gpu/hardware-results, на TCL /var/log/tcl-gpu и USB /tcl-kms1/gpu-results. Для пользователя запущен vkcube-demo на 18000 кадров и терминал. Weston GL и демонстрация — transient units, постоянный графический автозапуск пока не настроен.

Подготовительный отчёт: Bug19084 comment181088 (20 минут).
