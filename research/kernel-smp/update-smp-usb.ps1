$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
try {
$src=Join-Path $PSScriptRoot 'TCL-SMP-BOOTAA64.EFI'
if((Get-FileHash $src).Hash -ne '0A575CA6269C6DD4AFA278155F892E32DD57F2AFFB69CA7492044A2EE31A140F') { throw 'New EFI checksum mismatch' }
$disks=@(Get-Disk | Where-Object { $_.BusType -eq 'USB' -and $_.SerialNumber.Trim() -eq '001CC0EC34E4FBB085C323F2' -and $_.Size -eq 8010194944 -and -not $_.IsBoot -and -not $_.IsSystem })
if($disks.Count -ne 1) { throw 'USB identity mismatch' }
$vols=@(Get-Partition -DiskNumber $disks[0].Number | Get-Volume | Where-Object FileSystemLabel -eq 'TCLDIAG')
if($vols.Count -ne 1 -or $vols[0].FileSystem -ne 'FAT32') { throw 'USB volume mismatch' }
$root=('{0}:\' -f $vols[0].DriveLetter)
$boot=Join-Path $root 'EFI\BOOT\BOOTAA64.EFI'
if((Get-FileHash $boot).Hash -ne '0F4C1F01CCEF95C3701B80EC02F458343F5A3A9EA8999F8DE4B6877F117C898D') { throw 'Unexpected previous EFI' }
$manifest=ConvertFrom-Json '[{"Name":"Image","Size":25541120,"SHA256":"04F22CC028D2230DC18787776666C60EADA9105AC37E60AD0E0BF03835187D8B"},{"Name":"sc7180-tcl-minimal-efi.dtb","Size":1595,"SHA256":"657EB19CD9FAB8D3ADF81E9E7E33E3F99F2311F32CF41B9418B60C11EBD8DF5A"},{"Name":"initramfs.cpio.gz","Size":1016445,"SHA256":"A33A49C0F2F2B3152D06DBBA0EC742B94052F0AB52B6BAF5F6DAECE90E437CFE"},{"Name":"TCL-RAM-TEST","Size":50,"SHA256":"B1505A1B5BFB0DA9EF5E31B754730E80A914D97D1CC233A4131ECE676C78EF14"},{"Name":"README.md","Size":2469,"SHA256":"AB8A7D069295145D8FEEEECB01E268DCBDCA3F8A6A4A701FF613C7F6C6BABA1E"}]'
foreach($f in $manifest) { $path=Join-Path $root ('tcl-diag\'+$f.Name); if((Get-Item $path).Length -ne $f.Size -or (Get-FileHash $path).Hash -ne $f.SHA256) { throw ('Baseline payload mismatch: '+$f.Name) } }
Copy-Item $boot (Join-Path $root 'EFI\BOOT\BOOTAA64-onecpu.bak')
Copy-Item $src $boot -Force
if((Get-FileHash $boot).Hash -ne '0A575CA6269C6DD4AFA278155F892E32DD57F2AFFB69CA7492044A2EE31A140F') { throw 'Written EFI checksum mismatch' }
[IO.File]::WriteAllText((Join-Path $root 'SMP-TEST.txt'),'Added manual eight CPUs menu entry. Only maxcpus=1 changed to maxcpus=8; same kernel, DTB and initramfs. Expected CPU online: 0-7. One CPU entry retained.'+[Environment]::NewLine)
[ordered]@{Disk=$disks[0].Number;Root=$root;EFIHash=(Get-FileHash $boot).Hash;BaselineFilesVerified=$manifest.Count;Backup='EFI/BOOT/BOOTAA64-onecpu.bak'} | ConvertTo-Json -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
