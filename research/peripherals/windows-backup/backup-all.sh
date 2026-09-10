#!/bin/bash
set -euo pipefail
umask 077
root=/var/ftp/tmp/lav/tcl/windows-ufs-20260908
scripts=/srv/lav/Projects/git-eter/etersoft-admin-essential/.claude/docs/tcl-b220g-data/peripherals/windows-backup
export BORG_BASE_DIR=/tmp/tcl-windows-backup/borg-state
repo=$root/repository
test -f "$repo/config"
# First preserve the small boot/service LUNs; then capture the main Windows LUN.
for lun in 1 2 3 4 5 0; do
 archive=windows-20260908-lun$lun
 if [ -f "$root/logs/lun$lun-verified.json" ]; then
  echo "Already verified LUN$lun; skipping"
  continue
 fi
 echo "Starting LUN$lun at $(date -u --iso-8601=seconds)"
 if ! borg list --short "$repo" | grep -Fxq "$archive"; then
  borg create --stats --json --compression zstd,3 --checkpoint-interval 300 \
   --stdin-name "lun$lun.img" --content-from-command "$repo::$archive" -- \
   python3 "$scripts/relay.py" "$lun" "$root" > "$root/logs/lun$lun-create.json"
 fi
 test -s "$root/logs/lun$lun-received.json"
 echo "Verifying restored stream of LUN$lun"
 borg extract --stdout "$repo::$archive" "lun$lun.img" | \
  python3 "$scripts/verify-stream.py" "$root/logs/lun$lun-received.json" > "$root/logs/lun$lun-verified.json.new"
 mv "$root/logs/lun$lun-verified.json.new" "$root/logs/lun$lun-verified.json"
 echo "LUN$lun complete and read-back verified at $(date -u --iso-8601=seconds)"
done
borg check --repository-only "$repo"
borg list --json "$repo" > "$root/archives.json"
date -u --iso-8601=seconds > "$root/COMPLETE"
echo 'All six UFS LUNs archived and restored-stream SHA256 verified.'
