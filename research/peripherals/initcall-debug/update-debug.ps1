$ErrorActionPreference='Stop'
try {
$disks=@(Get-Disk | Where-Object { $_.BusType -eq 'USB' -and $_.SerialNumber.Trim() -eq '001CC0EC34E4FBB085C323F2' -and $_.Size -eq 8010194944 -and -not $_.IsBoot -and -not $_.IsSystem })
if($disks.Count -ne 1) { throw 'USB identity mismatch' }
$vols=@(Get-Partition -DiskNumber $disks[0].Number | Get-Volume | Where-Object FileSystemLabel -eq 'TCLDIAG')
if($vols.Count -ne 1 -or $vols[0].FileSystem -ne 'FAT32') { throw 'USB volume mismatch' }
$root=('{0}:\' -f $vols[0].DriveLetter)
$boot=Join-Path $root 'EFI\BOOT\BOOTAA64.EFI'
if((Get-FileHash $boot).Hash -ne '8C3DF2BE1B7500A7BE30996751B809449F8A21E5EF2332F44D991036743E0707') { throw 'Unexpected previous EFI' }
$manifest=ConvertFrom-Json '[{"Name": "sc7180-tcl-usb-hid.dtb", "Size": 67364, "SHA256": "1D6A4743632D75549E0FB2DD3C3FECA3F3AF6EAC9D52D89025BF3DCFF7241EAD"}, {"Name": "initramfs.cpio.gz", "Size": 15132835, "SHA256": "F5973E676BA86E6BA36DEFED1944145B5470BF0B217077892905A5A785724973"}]'
foreach($f in $manifest) {
 $path=Join-Path $root ('tcl-peripherals\'+$f.Name)
 if((Get-FileHash $path).Hash -ne $f.SHA256) { throw 'Existing payload mismatch' }
}
$src=Join-Path $PSScriptRoot 'BOOTAA64.EFI'
if((Get-FileHash $src).Hash -ne 'C900B48D4B871CE7ACFCB4E00C809415D304046F1EF20AE17D32CA590706D1DE') { throw 'Source EFI mismatch' }
$backup=Join-Path $root 'EFI\BOOT\BOOTAA64-peripheral-v1.bak'
if(Test-Path $backup) { throw 'Backup already exists' }
Copy-Item $boot $backup
Copy-Item $src $boot -Force
if((Get-FileHash $boot).Hash -ne 'C900B48D4B871CE7ACFCB4E00C809415D304046F1EF20AE17D32CA590706D1DE') { throw 'Written EFI mismatch' }
[ordered]@{Disk=$disks[0].Number;Root=$root;EFIHash=(Get-FileHash $boot).Hash;PayloadVerified=$manifest.Count;Backup='EFI/BOOT/BOOTAA64-peripheral-v1.bak'} | ConvertTo-Json -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
