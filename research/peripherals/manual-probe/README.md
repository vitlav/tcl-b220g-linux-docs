# Manual platform probe diagnostic

Motivation: user reports both CORE no-qfprom and no-smmu hang. Exact last lines of those runs have not been photo-verified. Neither single-node exclusion fixed the symptom.

This test returns to the original USB/HID DT and unchanged Image. Adds `initcall_blacklist=deferred_probe_initcall` to the initcall-debug command line. The custom initramfs starts platform probes through /sys/bus/platform/drivers_probe after printing each device name. Linux v6.18 init/main.c, drivers/base/dd.c and bus.c verified: blacklist supported with KALLSYMS; drivers_probe uses device_attach with asynchronous probing disallowed for that call. Newly created child devices may still probe asynchronously.

Expected: TCL MANUAL PROBE: REACHED USERSPACE, then after10seconds BEGIN/END pairs. Three rounds retry unbound platform devices as suppliers appear. A watcher every20seconds prints the last operation and /proc/<worker>/stack. If there is a system-wide firmware lockup, watcher may not run. Last BEGIN identifies the requested device, but nested child probes are possible; stack is stronger evidence.

Automatic deferred probe remains disabled, so missing devices or success here do not establish a correct normal boot. No modules loaded, no network setup, no disk mounts/writes. Same static ARM64 BusyBox. User should photograph the last BEGIN and stack, or the completed result and unbound devices. Old menu entries retained. Prepared locally; no successful hardware test yet.

Deployed to Kingston disk6 D:, both new files verified by read-back SHA256. See usb-update-result.json.
