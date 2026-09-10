$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
try {
$stage=Join-Path $env:USERPROFILE 'tcl-kernel-diag-stage'
if(Test-Path $stage) { throw 'Staging directory exists; inspect before resuming' }
Expand-Archive -LiteralPath (Join-Path $PSScriptRoot 'tcl-kernel-diag.zip') -DestinationPath $stage
$manifest=Get-Content -Raw (Join-Path $stage 'manifest.json') | ConvertFrom-Json
foreach($f in $manifest) { $src=Join-Path $stage $f.Name; if((Get-Item $src).Length -ne $f.Size -or (Get-FileHash $src).Hash -ne $f.SHA256) { throw 'Staging checksum mismatch' } }
$disks=@(Get-Disk | Where-Object { $_.BusType -eq 'USB' -and $_.SerialNumber.Trim() -eq '001CC0EC34E4FBB085C323F2' -and $_.Size -eq 8010194944 -and -not $_.IsBoot -and -not $_.IsSystem })
if($disks.Count -ne 1) { throw 'USB identity mismatch' }
$vols=@(Get-Partition -DiskNumber $disks[0].Number | Get-Volume | Where-Object FileSystemLabel -eq 'TCLDIAG')
if($vols.Count -ne 1 -or $vols[0].FileSystem -ne 'FAT32') { throw 'USB volume mismatch' }
$root=('{0}:\' -f $vols[0].DriveLetter)
$boot=Join-Path $root 'EFI\BOOT\BOOTAA64.EFI'
if((Get-FileHash $boot).Hash -ne 'E444B7E1AD78D87D81F78A4EAEC9027EE14AE846ED84DADB3986770C582DE1F2') { throw 'Unexpected previous EFI; inspect before replacing' }
Copy-Item $boot (Join-Path $root 'EFI\BOOT\BOOTAA64-grub-only.bak')
$target=Join-Path $root 'tcl-diag'
New-Item -ItemType Directory -Path $target -Force | Out-Null
foreach($f in $manifest) { if($f.Name -ne 'BOOTAA64.EFI') { Copy-Item (Join-Path $stage $f.Name) (Join-Path $target $f.Name) -Force; if((Get-FileHash (Join-Path $target $f.Name)).Hash -ne $f.SHA256) { throw 'USB data checksum mismatch' } } }
Copy-Item (Join-Path $stage 'BOOTAA64.EFI') $boot -Force
if((Get-FileHash $boot).Hash -ne ($manifest | Where-Object Name -eq 'BOOTAA64.EFI').SHA256) { throw 'USB EFI checksum mismatch' }
[IO.File]::WriteAllText((Join-Path $root 'README.txt'),[IO.File]::ReadAllText((Join-Path $stage 'README.md')))
[ordered]@{Disk=$disks[0].Number;Root=$root;FilesVerified=$manifest.Count;EFIHash=(Get-FileHash $boot).Hash;Backup='EFI/BOOT/BOOTAA64-grub-only.bak'} | ConvertTo-Json -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
