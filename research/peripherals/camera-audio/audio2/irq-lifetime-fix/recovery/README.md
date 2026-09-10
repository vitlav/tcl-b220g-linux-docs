# IRQ fix recovery staging

Not executed on hardware yet. Await user approval before reboot.
The current boot c781c4d4-cd36-4a07-b734-40b059ac3e52 suffered Oops.

The two ko files are the runtime-path variant with IRQ cleanup and callback guard.
They must be used together; do not mix with the old SoundWire codec module.
No boot files or system module files are overwritten by this staging directory.

restore-audio-buses.sh restores only the previously tested RX/LPI/SWR/TX overlays
from /var/tmp/tcl-audio2 after a clean boot; it leaves supplies disabled and does
not load the aggregate codec. It has not yet been executed in a new boot.

After that, verify node phandles, restore the BOB/resource/aggregate descriptions
without activating their old automatic test consumers, install both fixed codec
modules, and perform bind/unbind before attempting module unload. Inspect IRQ
mappings/domains, taint and full dmesg at each step. Stop on Oops/WARN.
The old aggregate test scripts load unfixed modules and MUST NOT be rerun as-is.
