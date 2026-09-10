$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
try {
$root=Join-Path $env:USERPROFILE 'tcl-driver-export-20260906'
if(Test-Path $root) { throw 'Export directory already exists; inspect it before resuming' }
New-Item -ItemType Directory -Path $root | Out-Null
$oem=New-Item -ItemType Directory -Path (Join-Path $root 'OEM')
$drivers=Export-WindowsDriver -Online -Destination $oem.FullName
$drivers | Select-Object Driver,OriginalFileName,ProviderName,ClassName,Date,Version | ConvertTo-Json -Depth 4 | Set-Content -Encoding UTF8 (Join-Path $root 'oem-packages.json')
$inbox=New-Item -ItemType Directory -Path (Join-Path $root 'Inbox')
$names=@('storufs.sys','hidi2c.sys','hidclass.sys','hidparse.sys','kbdhid.sys','kbdclass.sys','mouhid.sys','mouclass.sys','SpbCx.sys','Wdf01000.sys','ACPI.sys','EDPBridge.sys')
$extra=@()
foreach($name in $names) {
$src=Join-Path $env:windir ('System32\drivers\'+$name)
if(Test-Path $src) { Copy-Item -LiteralPath $src -Destination $inbox.FullName; $extra += [pscustomobject]@{Source=$src;Name=$name;Found=$true} } else { $extra += [pscustomobject]@{Source=$src;Name=$name;Found=$false} }
}
$extra | ConvertTo-Json | Set-Content -Encoding UTF8 (Join-Path $root 'inbox-sources.json')
Get-CimInstance Win32_PnPSignedDriver | Select-Object DeviceName,DeviceID,InfName,DriverVersion,DriverProviderName | ConvertTo-Json -Depth 4 | Set-Content -Encoding UTF8 (Join-Path $root 'pnp-drivers.json')
Get-CimInstance Win32_SystemDriver | Select-Object Name,DisplayName,PathName,State,StartMode | ConvertTo-Json -Depth 4 | Set-Content -Encoding UTF8 (Join-Path $root 'system-drivers.json')
$files=@(Get-ChildItem $root -Recurse -File | ForEach-Object { [pscustomobject]@{Path=$_.FullName.Substring($root.Length+1).Replace('\','/');Size=$_.Length;SHA256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash} })
$files | ConvertTo-Json -Depth 4 | Set-Content -Encoding UTF8 (Join-Path $root 'manifest.json')
$archive=$root+'.tar'
& tar.exe -cf $archive -C $root .
if($LASTEXITCODE -ne 0) { throw 'tar failed' }
[ordered]@{Root=$root;Archive=$archive;Files=$files.Count;Bytes=($files | Measure-Object Size -Sum).Sum;ArchiveSize=(Get-Item $archive).Length;ArchiveSHA256=(Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash} | ConvertTo-Json -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
