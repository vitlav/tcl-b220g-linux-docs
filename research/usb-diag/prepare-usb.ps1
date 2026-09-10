$ErrorActionPreference='Stop'
try {
$matches=@(Get-Disk | Where-Object { $_.BusType -eq 'USB' -and $_.SerialNumber.Trim() -eq '001CC0EC34E4FBB085C323F2' -and $_.Size -eq 8010194944 })
if ($matches.Count -ne 1) { throw 'USB identity mismatch' }
$d=$matches[0]
if ($d.IsBoot -or $d.IsSystem -or $d.Number -eq 0) { throw 'Refusing system disk' }
if ($d.FriendlyName -ne 'Kingston DT 101 G2') { throw 'Model mismatch' }
$bytes=[IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'BOOTAA64.EFI'))
$stage=Join-Path $env:TEMP 'tcl-BOOTAA64.EFI'
[IO.File]::WriteAllBytes($stage,$bytes)
if ((Get-FileHash -Algorithm SHA256 $stage).Hash -ne 'E444B7E1AD78D87D81F78A4EAEC9027EE14AE846ED84DADB3986770C582DE1F2') { throw 'Staging hash mismatch' }
Clear-Disk -Number $d.Number -RemoveData -RemoveOEM -Confirm:$false
if ((Get-Disk -Number $d.Number).PartitionStyle -eq 'RAW') { Initialize-Disk -Number $d.Number -PartitionStyle GPT | Out-Null }
if ((Get-Disk -Number $d.Number).PartitionStyle -ne 'GPT') { throw 'Unexpected partition style' }
$p=New-Partition -DiskNumber $d.Number -UseMaximumSize -AssignDriveLetter
$p | Format-Volume -FileSystem FAT32 -NewFileSystemLabel TCLDIAG -Confirm:$false | Out-Null
$root=('{0}:\' -f $p.DriveLetter)
New-Item -ItemType Directory -Path (Join-Path $root 'EFI\BOOT') -Force | Out-Null
$dest=Join-Path $root 'EFI\BOOT\BOOTAA64.EFI'
Copy-Item -LiteralPath $stage -Destination $dest
if ((Get-FileHash -Algorithm SHA256 $dest).Hash -ne 'E444B7E1AD78D87D81F78A4EAEC9027EE14AE846ED84DADB3986770C582DE1F2') { throw 'USB hash mismatch' }
[IO.File]::WriteAllText((Join-Path $root 'README.txt'),'TCL B220G UEFI diagnostic USB. GRUB only; Linux kernel is not included. Internal UFS is not modified. Select video information or memory map; press c for GRUB console. Reboot and remove USB to return to Windows.'+[Environment]::NewLine)
[ordered]@{DiskNumber=$d.Number;Serial=$d.SerialNumber;Root=$root;SHA256=(Get-FileHash $dest).Hash;Volume=(Get-Volume -DriveLetter $p.DriveLetter | Select-Object DriveLetter,FileSystem,FileSystemLabel,Size,SizeRemaining)} | ConvertTo-Json -Depth 4 -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
