# DPU assignment diagnostic candidate

Three-file patch, built locally against the exact working6.18.34-tcl-kms1 tree with unchanged config/toolchain. Kbuild M=drivers/gpu/drm/msm modules passed MODPOST. No compiler warnings in build.log. checkpatch found only missing submission description/signoff; this is an unsigned local diagnostic patch, not an upstream submission. No user signoff invented.

Legacy lookup is retained. On a miss, helper checks DPU's assigned CRTC (established before vblank_on, cleared after vblank_off), logs one recovery message and returns that encoder. New getter uses READ_ONCE with WRITE_ONCE at assignment. It deliberately does NOT take enc_spinlock because vblank IRQ calls into CRTC while already holding this lock. This diagnostic tests the hypothesis and provides a recovery path; not yet a finalized upstream fix.

All stage files identical to original except msm.ko. Initramfs contains949 modules and3 GPU firmware files; all compared byte-for-byte against stage/firmware. Same thermal DT, Image and init/shutdown/collector scripts. New default tcl-dpu-assignment; old tcl-kms1-thermal retained with original initramfs, which loads original MSM before root. Root module file is replaced for consistency with new boot; rollback root file /var/tmp/msm-before-dpu.ko if reverting permanently. USB EFI backup BOOTAA64-before-dpu.bak. No live module unloading.

Hashes: msm.ko2305a5d3b8c568d2277c53b006b83cf240c6e48602f850ad14d86ba4abae59f4; initramfs c289074835178882b1733da533c8296370267c4105da857d5a58a0c843845ba2; EFI96dadb1cdbd9452869fc07b994943d0085702235c16791ac2a0342843b207038.

Hardware validation pending. Require full33-scene fullscreen run, journal recovery message if legacy lookup actually misses, no unhandled encoder errors/lockup, intact desktop, temperatures and benchmark result. Absence of error without diagnostic hit does not prove the race was exercised.

## Hardware validation completed

Boot80d53c86-78c1-4257-b68b-1310cdb831fc, native console/Wi-Fi/SSH/Weston autostart successful. Root module checksum matches candidate. Full33-scene glmark2 fullscreen+annotation run exit0, score347, duration5min37s. At uptime217.176541 the diagnostic logged legacy lookup miss recovered via assigned CRTC. No `no encoder found for crtc` messages in the saved log. This directly demonstrates the fallback handled the condition on hardware; exact writer/callback interleaving still was not traced. Code remains a local diagnostic recovery, not upstream-reviewed patch.

Sampled GPU max60.0°C. Score lower than prior381; runs were not frequency-controlled. Mean sampled big-cluster frequency1034240kHz versus1602880kHz previously; GPU mean651.9MHz versus594.1MHz. Thus a9% aggregate-score difference cannot be attributed to this patch from these two runs. Matched CPU scheduling/frequency conditions needed for performance comparison; do not claim no regression or a proven regression.

Refresh verified during600-frame Vulkan animation: encoder vsync delta250 over4.16s after initial startup sample, ~60.1Hz with SSH/sysfs sample timing uncertainty. Mode calculation142520000/(2080*1142)=59.9993Hz. Underrun counter0. Panel refresh is60Hz even when glmark2 renders hundreds of FPS.

Saved results/ includes CSV, telemetry, before/after dmesg, DRM state, desktop log and refresh samples. comparison.json contains measured frequencies/temperature and refresh calculation. Old thermal boot with original initramfs remains; root-original MSM backup /var/tmp/msm-before-dpu.ko.
