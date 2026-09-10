$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
try {
$disks=@(Get-Disk | Where-Object { $_.BusType -eq 'USB' -and $_.SerialNumber.Trim() -eq '001CC0EC34E4FBB085C323F2' -and $_.Size -eq 8010194944 -and -not $_.IsBoot -and -not $_.IsSystem })
if($disks.Count -ne 1) { throw 'USB identity mismatch' }
$vols=@(Get-Partition -DiskNumber $disks[0].Number | Get-Volume | Where-Object FileSystemLabel -eq 'TCLDIAG')
if($vols.Count -ne 1 -or $vols[0].FileSystem -ne 'FAT32') { throw 'USB volume mismatch' }
$root=('{0}:\' -f $vols[0].DriveLetter)
$boot=Join-Path $root 'EFI\BOOT\BOOTAA64.EFI'
if((Get-FileHash $boot).Hash -ne 'FFDD7D5BE6D435FC4808E326239E1938377537576015A992F2ECF034E91F2A91') { throw 'Unexpected previous EFI' }
$manifest=Get-Content (Join-Path $PSScriptRoot 'manifest.json') -Raw | ConvertFrom-Json
foreach($f in $manifest) {
 $path=Join-Path $PSScriptRoot $f.Name
 if((Get-Item $path).Length -ne $f.Size -or (Get-FileHash $path).Hash -ne $f.SHA256) { throw ('Source mismatch: '+$f.Name) }
}
$payload=Join-Path $root 'tcl-tlmm-reserved'
New-Item -ItemType Directory -Force $payload | Out-Null
foreach($f in $manifest | Where-Object Name -ne 'BOOTAA64.EFI') {
 $target=Join-Path $payload $f.Name
 Copy-Item (Join-Path $PSScriptRoot $f.Name) $target -Force
 if((Get-FileHash $target).Hash -ne $f.SHA256) { throw ('USB payload mismatch: '+$f.Name) }
}
$backup=Join-Path $root 'EFI\BOOT\BOOTAA64-manual-probe.bak'
if(Test-Path $backup) { throw 'Backup already exists; review required' }
Copy-Item $boot $backup
Copy-Item (Join-Path $PSScriptRoot 'BOOTAA64.EFI') $boot -Force
$expected=($manifest | Where-Object Name -eq 'BOOTAA64.EFI').SHA256
if((Get-FileHash $boot).Hash -ne $expected) { throw 'EFI write mismatch' }
[ordered]@{Disk=$disks[0].Number;Root=$root;EFIHash=(Get-FileHash $boot).Hash;FilesVerified=$manifest.Count;Backup='EFI/BOOT/BOOTAA64-manual-probe.bak'} | ConvertTo-Json -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
