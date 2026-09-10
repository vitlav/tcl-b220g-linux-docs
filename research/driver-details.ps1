$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$r = [ordered]@{}
$r.Services = @('EDPBridge','QcPep','qci2c','storufs','QCDX') | ForEach-Object { $s=Get-ItemProperty ('HKLM:\SYSTEM\CurrentControlSet\Services\'+$_) -ErrorAction SilentlyContinue; if($s){[pscustomobject]@{Name=$_;ImagePath=$s.ImagePath;Start=$s.Start}} }
$r.BridgeINF = Get-Content C:\Windows\INF\oem48.inf -Raw
$r.PepDrivers = Get-CimInstance Win32_PnPSignedDriver | Where-Object DeviceName -Match 'PEP|Power Engine|Platform Extension' | Select-Object DeviceName,DeviceID,InfName,DriverVersion
$r.BridgeFiles = @(Get-ChildItem C:\Windows\System32\DriverStore\FileRepository -Directory -Filter '*edp*' | ForEach-Object { Get-ChildItem $_.FullName -File -Recurse | Select-Object FullName,Length })
$r.FirmwareFiles = @(Get-ChildItem C:\Windows\System32\DriverStore\FileRepository -File -Recurse -Include *.mbn,*.mdt,*.b00,*.elf | Select-Object FullName,Length)
$r | ConvertTo-Json -Depth 6 -Compress
