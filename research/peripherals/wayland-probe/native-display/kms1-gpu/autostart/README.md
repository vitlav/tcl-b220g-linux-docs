# Diagnostic desktop autostart

System service runs the existing root diagnostic Weston session with GL renderer. This is not a general-purpose user login/display manager. No terminal or benchmark is started automatically. GPU devfreq and card0 conditions skip old GPU-disabled/control boots. Early initramfs already provides these nodes before systemd; service does not load modules or modify DT.

Install unit to /etc/systemd/system/tcl-weston.service and config to /etc/xdg/weston/tcl-weston.ini; systemctl daemon-reload and enable/start tcl-weston. serv was not available on the TCL at preparation time. Conflicts stops the previous transient Weston compositor when starting the persistent one. No restart loop is configured.

Rollback: systemctl disable --now tcl-weston. The underlying native console and independent SSH/Wi-Fi remain available. Boot-time operation requires verification on a future necessary reboot.
