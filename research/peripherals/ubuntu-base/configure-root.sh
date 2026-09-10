#!/bin/bash
# Run inside the prepared Ubuntu chroot; secrets are copied separately.
set -euo pipefail
umask 022
export PATH=/usr/sbin:/usr/bin:/sbin:/bin
mkdir -p /etc/systemd/system/{rmtfs.service.d,tqftpserv.service.d,wpa_supplicant@wlan0.service.d,getty@tty1.service.d,ssh.socket.d} /etc/systemd/network /etc/modprobe.d /etc/ssh/sshd_config.d /usr/local/sbin /etc/systemd/journald.conf.d
printf 'tcl-ubuntu\n' > /etc/hostname
printf '127.0.0.1 localhost\n127.0.1.1 tcl-ubuntu\n::1 localhost ip6-localhost ip6-loopback\n' > /etc/hosts
printf 'LANG=C.UTF-8\n' > /etc/default/locale
printf '# USB loop root is mounted by the TCL initramfs. No internal disk mounts.\n' > /etc/fstab
cat > /etc/modprobe.d/tcl-radio.conf <<'EOF'
# Remoteproc must wait for RMTFS and tqftpserv; explicit modprobe is in tcl-radio.
blacklist qcom_q6v5_pas
blacklist ath10k_snoc
EOF
cat > /etc/systemd/system/rmtfs.service.d/tcl.conf <<'EOF'
[Service]
ExecStart=
ExecStart=/usr/bin/stdbuf -oL -eL /usr/bin/rmtfs -r -v -o /rmtfs-storage
Restart=no
EOF
cat > /etc/systemd/system/tqftpserv.service.d/tcl.conf <<'EOF'
[Service]
ExecStartPre=/usr/bin/mkdir -p /tmp/tqftpserv
ExecStart=
ExecStart=/usr/bin/stdbuf -oL -eL /usr/bin/tqftpserv
Restart=on-failure
RestartSec=5
EOF
cat > /etc/systemd/system/tcl-radio.service <<'EOF'
[Unit]
Description=Start TCL modem after firmware services are ready
Requires=rmtfs.service tqftpserv.service
After=rmtfs.service tqftpserv.service systemd-udev-trigger.service
Before=wpa_supplicant@wlan0.service

[Service]
Type=oneshot
ExecStart=/usr/local/sbin/tcl-radio-start
RemainAfterExit=yes
TimeoutStartSec=120

[Install]
WantedBy=multi-user.target
EOF
cat > /usr/local/sbin/tcl-radio-start <<'EOF'
#!/bin/bash
set -euo pipefail
mem=/sys/class/rmtfs/qcom_rmtfs_mem1
test -c /dev/qcom_rmtfs_mem1
addr=$(cat "$mem/phys_addr"); size=$(cat "$mem/size")
(( 16#${addr#0x} == 0x80600000 && 16#${size#0x} == 0x200000 ))
ready=no
for ((i=0; i<15; i++)); do
 if timeout 3 qrtr-lookup | awk '$1 == 14 {found=1} END {exit !found}'; then ready=yes; break; fi
 sleep 1
done
test "$ready" = yes
modprobe ath10k_snoc
modprobe qcom_q6v5_pas
found=no
for ((i=0; i<20; i++)); do
 for rp in /sys/class/remoteproc/remoteproc*; do
  test -f "$rp/firmware" || continue
  test "$(cat "$rp/firmware")" = qcom/sc7180/tcl/qcmpss7180_nm.mbn || continue
  found=yes
  echo disabled > "$rp/recovery"
  test "$(cat "$rp/state")" != offline || echo start > "$rp/state"
 done
 test "$found" = no || break
 sleep 1
done
test "$found" = yes
for ((i=0; i<60; i++)); do
 test ! -d /sys/class/net/wlan0 || exit 0
 sleep 1
done
echo 'WLAN interface did not appear' >&2
exit 1
EOF
chmod 755 /usr/local/sbin/tcl-radio-start
cat > /etc/systemd/network/10-tcl-wifi.link <<'EOF'
[Match]
OriginalName=wlan*

[Link]
Name=wlan0
MACAddressPolicy=none
MACAddress=46:14:79:6e:ba:d1
EOF
cat > /etc/systemd/network/20-tcl-wifi.network <<'EOF'
[Match]
Name=wlan0

[Network]
DHCP=yes
IPv6AcceptRA=yes

[DHCPv4]
ClientIdentifier=mac
RouteMetric=100
EOF
cat > /etc/systemd/network/30-usb-ethernet.network <<'EOF'
[Match]
Driver=rndis_host cdc_ether cdc_ncm r8152 asix ax88179_178a

[Network]
DHCP=yes

[DHCPv4]
ClientIdentifier=mac
RouteMetric=500
EOF
chmod 644 /etc/systemd/network/10-tcl-wifi.link /etc/systemd/network/20-tcl-wifi.network /etc/systemd/network/30-usb-ethernet.network
cat > /etc/systemd/system/wpa_supplicant@wlan0.service.d/tcl.conf <<'EOF'
[Unit]
Requires=tcl-radio.service
After=tcl-radio.service

[Service]
Restart=on-failure
RestartSec=5
EOF
cat > /etc/ssh/sshd_config.d/10-tcl.conf <<'EOF'
PermitRootLogin prohibit-password
PasswordAuthentication no
KbdInteractiveAuthentication no
HostKey /etc/ssh/ssh_host_ed25519_key
EOF
cat > /etc/systemd/system/ssh.socket.d/tcl.conf <<'EOF'
[Socket]
ListenStream=
ListenStream=0.0.0.0:22
EOF
cat > /etc/systemd/system/getty@tty1.service.d/autologin.conf <<'EOF'
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin root --noclear %I $TERM
EOF
cat > /etc/systemd/journald.conf.d/tcl.conf <<'EOF'
[Journal]
Storage=persistent
SystemMaxUse=64M
RuntimeMaxUse=16M
EOF
mkdir -p /var/log/journal
systemctl mask qrtr-ns.service
systemctl disable wpa_supplicant.service systemd-networkd-wait-online.service
systemctl enable rmtfs.service tqftpserv.service tcl-radio.service wpa_supplicant@wlan0.service systemd-networkd.service systemd-resolved.service systemd-timesyncd.service ssh.socket getty@tty1.service
systemctl disable systemd-networkd-wait-online.service
systemctl set-default multi-user.target
mkdir -p /etc/udev/rules.d /var/lib/systemd/timesync
cat > /etc/udev/rules.d/99-tcl-ufs-readonly.rules <<'EOF'
ACTION=="add|change", SUBSYSTEM=="block", KERNELS=="1d84000.ufshc", RUN+="/usr/sbin/blockdev --setro /dev/%k"
EOF
touch /var/lib/systemd/timesync/clock
if [ -f /usr/sbin/policy-rc.d ]; then unlink /usr/sbin/policy-rc.d; fi
depmod -a 6.18.34-stb-qc7+
mkdir -p /run/sshd
/usr/sbin/sshd -t
systemd-analyze verify tcl-radio.service rmtfs.service tqftpserv.service wpa_supplicant@wlan0.service
dpkg --audit
