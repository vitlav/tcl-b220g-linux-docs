$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
$r=[ordered]@{}
$r.PhysicalMemoryResourceReg=(& reg.exe query 'HKLM\HARDWARE\RESOURCEMAP\System Resources\Physical Memory' | Out-String)
$r.MemoryDevices=Get-CimInstance Win32_PhysicalMemory | Select-Object Capacity,Speed,ConfiguredClockSpeed,DeviceLocator,Manufacturer,PartNumber
$r.Display=Get-CimInstance Win32_VideoController | Select-Object Name,CurrentHorizontalResolution,CurrentVerticalResolution,CurrentRefreshRate,CurrentBitsPerPixel,VideoModeDescription
$r | ConvertTo-Json -Depth 4 -Compress
