$ErrorActionPreference='Stop'
try {
$disks=@(Get-Disk | Where-Object { $_.BusType -eq 'USB' -and $_.SerialNumber.Trim() -eq '001CC0EC34E4FBB085C323F2' -and $_.Size -eq 8010194944 -and -not $_.IsBoot -and -not $_.IsSystem })
if($disks.Count -ne 1) { throw 'USB identity mismatch' }
$vols=@(Get-Partition -DiskNumber $disks[0].Number | Get-Volume | Where-Object FileSystemLabel -eq 'TCLDIAG')
if($vols.Count -ne 1 -or $vols[0].FileSystem -ne 'FAT32') { throw 'USB volume mismatch' }
$root=('{0}:\' -f $vols[0].DriveLetter)
if((Get-Content (Join-Path $root 'tcl-diag\TCL-RAM-TEST') -Raw).Trim() -ne 'TCL B220G experimental RAM-only kernel diagnostic') { throw 'Marker mismatch' }
$boot=Join-Path $root 'EFI\BOOT\BOOTAA64.EFI'
$expected='3ABBA529D5E6F2668BC3EED21AEBB38EC01B094970E631540C370AC9CA3528C2'
$source=Join-Path $env:USERPROFILE 'BOOTAA64-fixed.EFI'
if((Get-FileHash $source).Hash -ne $expected) { throw 'Source mismatch' }
if((Get-FileHash (Join-Path $root 'tcl-wifi-persistent\initramfs.cpio.gz')).Hash -ne '51C58B0F865F5A0B9D562B5B274DE74D77AE370A138EC53FD88C1227FBED7556') { throw 'Initramfs mismatch' }
foreach($name in @('tcl-diag\Image','tcl-rmtfs-probe\sc7180-tcl-rmtfs.dtb','EFI\BOOT\BOOTAA64-before-persistent.bak')) { if(-not(Test-Path (Join-Path $root $name))) { throw ('Missing '+$name) } }
$old=(Get-FileHash $boot).Hash
if($old -ne '3B6B6046BB44E78FA492ABDE8E9A3E0ED7E2C7AB35B64689DC1945E90CAC4886' -and $old -ne $expected) { throw 'Unexpected installed EFI' }
Copy-Item $source ($boot+'.new') -Force
if((Get-FileHash ($boot+'.new')).Hash -ne $expected) { throw 'Staged EFI mismatch' }
Move-Item ($boot+'.new') $boot -Force
if((Get-FileHash $boot).Hash -ne $expected) { throw 'Written EFI mismatch' }
[ordered]@{Root=$root;EFI=(Get-FileHash $boot).Hash;InitramfsVerified=$true;BackupPreserved=$true} | ConvertTo-Json -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
