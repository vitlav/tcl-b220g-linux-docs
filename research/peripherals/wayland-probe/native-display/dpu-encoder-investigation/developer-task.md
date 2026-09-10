# DPU vblank/scanout lookup: investigate transient missing encoder

## Observed

TCL B220G, SC7180 DPU + Adreno618 + diagnostic LT8911 handoff bridge v2; kernel6.18.34-tcl-kms1, Weston14.0.2 GL, Mesa26.0.8. Boot ab49eb3a-f0bc-47ce-94e6-b6662673243d.

Full default glmark2 run completed all33 scenes, but dmesg logged missing encoder for CRTC0 at uptime169.436741,206.420499,353.655481 (vblank counter),372.505702 (scanout position). Main command:

```sh
XDG_RUNTIME_DIR=/run/tcl-weston WAYLAND_DISPLAY=wayland-tcl \
glmark2-wayland --fullscreen --annotate --results-file /var/log/tcl-gpu/full/results.csv
```

Original logs/CSV are ../kms1-gpu/full-benchmark/results/. No observed GPU reset/hang. At idle later no additional errors; active DRM state has CRTC0 encoder_mask=1, eDP-1 attached, native1920x1080. GPU returned180MHz/31.8°C.

## Code evidence and hypothesis

In drivers/gpu/drm/msm/disp/dpu1/dpu_crtc.c:54, get_encoder_from_crtc searches encoder->crtc. Callers at188 and270 return0/false after logging if no match. drm_atomic_helper_update_legacy_modeset_state (drivers/gpu/drm/drm_atomic_helper.c:1396) first clears encoder->crtc and then assigns the new pointer. Its documentation explicitly restricts access to these unsynchronized legacy pointers to atomic_commit_tail paths.

Hypothesis: vblank/scanout callback overlaps the transient NULL assignment in legacy-state update. Exact overlap NOT traced/proven. Do not label it fixed or apply an unreviewed pointer substitution. Review callback context, IRQ locking, CRTC lifetime and stable encoder associations before selecting a fix.

Same helper and callbacks remain in downloaded official v6.18.49 and mainline/master source on2026-09-08. sources.json pins downloaded file hashes/URLs; master reference itself is moving. This is not proof no related fix exists elsewhere.

## Negative reproduction result

probe.sh runs A/B/A:12 one-second fullscreen build scenes each, default context recreation / --reuse-context / default recreation. All three processes completed successfully; error count4→4 in each phase. No new errors. Therefore context/window recreation alone has NOT been established as sufficient trigger, and --reuse-context is NOT a verified fix.

Current kernel has CONFIG_FTRACE unset, so function tracing requires a separately prepared diagnostic kernel. No current kernel, driver code, DT or boot default was changed by this investigation; no reboot performed.

## Work for DRM developer

1. Reproduce with the recorded full run and preserve event timing; distinguish scene boundaries, atomic commit and IRQ callbacks.
2. Instrument the NULL lookup together with atomic legacy-state transition and its calling context, using a separate diagnostic build with tracing if needed.
3. Check whether state encoder_mask or a protected cached association is valid in both callbacks; review disabling and reassignment races, not only the visible error log.
4. Validate multiple full scene runs and normal idle/desktop transitions, with vblank/presentation integrity checks. Do not accept removal/downgrading of the message as a fix.

No outreach sent to upstream. Local Etersoft bug is the work log.
