# Logging v2

User reports normal FIX TEST reached BusyBox, but /diagnose says blkid not found and USB saved=no. This is a diagnostic packaging error, not proof of USB hardware failure. Exact applet availability is unverified; all runtime blkid calls removed.

New logger matches USB ancestor serial001CC0EC34E4FBB085C323F2, mounts each candidate partition read-only and verifies exact TCL-RAM-TEST marker content, then remounts writable only that match. Reports saved=yes only after copy, sync and unmount succeed. Original logging-v1 artifacts retained. Snapshot no longer invokes blkid; input count corrected to count event character devices rather than ls output rows.

No new kernel/DT or Wi-Fi changes. Deployed on Kingston disk6 D:, both new files verified by read-back hash; hardware test of automatic logging pending. Current boot logs can be copied manually before reboot.
