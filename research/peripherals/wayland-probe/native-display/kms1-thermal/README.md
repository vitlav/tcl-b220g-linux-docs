# SC7180 thermal bring-up

Both TSENS controllers were disabled and all thermal zones absent in the reduced TCL DT. qcom_tsens.ko exists in the running kernel module set and GPU initramfs. CPU_THERMAL/DEVFREQ_THERMAL/ENERGY_MODEL are enabled. No kernel rebuild required.

thermal.dts imports 25 zones from the exact kernel source sc7180.dtsi (provenance.txt), retaining sensor indices, passive/critical trip temperatures, sustainable power and CPU/GPU cooling maps. Only label references are converted to absolute paths and THERMAL_NO_LIMIT to 0xffffffff. Two existing TSENS nodes are enabled. This is the SoC reference thermal policy, not a measured TCL chassis/skin-temperature policy.

234-node DT supplier audit includes thermal-sensors, cooling-device and trip references; no missing or disabled suppliers. New default tcl-kms1-thermal; original GPU and early-console entries retained. Same Image and GPU initramfs. DT SHA e0a20054fa5ea5f9156811c113065bf1285861d229513604bc8d02300f7685bc; EFI d7b9957e50505b973355a820040211118fe2f74333af8952006e2f999efc2900. USB copies verified with cmp; backup BOOTAA64-before-thermal.bak. Reboot requested for live verification.

A persistent diagnostic root Weston unit has also been enabled, with conditions on GPU devfreq and DRM card. Initial transition from transient unit failed due to socket still locked: Conflicts alone did not order the stop. After ordering against old units added, service started successfully. Future thermal boot tests its boot-time autostart.

Two-minute GL stability test passed (four 30-second scenes, 1280x720, score3005). GPU automatically reached 800 MHz under load and returned to 180 MHz / runtime suspended after desktop stopped. No forced frequency or voltage changes were made. Fullscreen standard benchmark with telemetry requested by user after enabling sensors.

## Hardware confirmation

Boot ab49eb3a-f0bc-47ce-94e6-b6662673243d: all 25 zones report plausible 33.7–35.0 °C after startup. Cooling devices cpufreq-cpu0 (max state9), cpufreq-cpu6 (max13), devfreq-5000000.gpu (max6) registered, current states0. power_allocator governor active; GPU sustainable_power is estimated, as logged by kernel. Temperature-triggered throttling/critical shutdown not exercised deliberately.

Persistent Weston autostart succeeded on this boot (service active). Full standard fullscreen glmark2 with --annotate and CSV launched; telemetry every5s. See ../kms1-gpu/full-benchmark/ for procedure/results. Raw sensor indices cpu8/cpu9 name thermal locations in the SoC reference, not extra logical CPU cores (machine remains 8-core).
