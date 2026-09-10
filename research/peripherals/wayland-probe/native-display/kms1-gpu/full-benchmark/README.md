# Fullscreen performance baseline

Command on TCL (Ubuntu26.04.1, kernel6.18.34-tcl-kms1, Mesa26.0.8, Weston14.0.2):

```sh
XDG_RUNTIME_DIR=/run/tcl-weston WAYLAND_DISPLAY=wayland-tcl  glmark2-wayland --fullscreen --annotate --results-file /var/log/tcl-gpu/full/results.csv
```

Default full scene list, no concurrent benchmark. Log confirms 1920x1080 fullscreen, GL_RENDERER FD618, OpenGL4.6 Compatibility. FPS is annotated on screen as requested. This baseline includes annotation and compositor costs; compare only matching resolution/options/software. Display refresh remains60Hz; reported rendering FPS is not panel refresh rate. No forced-vsync option specified; use identical default swap mode for comparisons.

monitor.sh records all TSENS zones and GPU/CPU frequencies every5s into telemetry.csv. Temperatures in millidegrees Celsius, GPU frequency Hz, CPU scaling_cur_freq kHz. Performance monitor has small sampling overhead. It does not prove absence of all transient events between samples. Environment and kernel logs recorded separately.

Four-scene 1280x720 score3005 and initial 800x600 single-scene score3168 are NOT comparable full benchmark scores. Wait for final complete result of this run before quoting its aggregate.

## Completed baseline

Boot ab49eb3a-f0bc-47ce-94e6-b6662673243d, all 33 standard scenes successful, process exit0; duration5min37s. Fullscreen1920×1080 with annotation, score381. Examples: terrain55FPS, refract129FPS, jellyfish432FPS, shadow421FPS. CSV and logs in results/.

Sampled maximum GPU temperatures: gpuss0 54.9°C, gpuss1 57.2°C; maximum among CPU-named zones47.5°C. Sampling interval5s may miss brief peaks. GPU observed267–800MHz during recording; 180MHz and runtime suspend were separately observed at idle before this full test. Checked cooling states were0; logged temperatures are far below passive GPU threshold95°C. Deliberate thermal-throttling/critical-shutdown tests not performed.

No GPU hang/fault found in saved log, but four DPU messages report missing encoder for CRTC0 during the test (vblank counter/scanout position). These remain an open issue; successful completion is not a claim of a completely clean driver log.

thermal-frequency.png/.svg plots recorded temperatures and frequency. telemetry-summary.json contains per-zone minima/maxima. All results include annotation and telemetry overhead and must be compared with the same command/configuration.
