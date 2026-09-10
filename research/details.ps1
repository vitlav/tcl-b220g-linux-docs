$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$r = [ordered]@{}
Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; public class FirmwareRead { [DllImport("kernel32.dll", SetLastError=true)] public static extern uint GetSystemFirmwareTable(uint p, uint id, byte[] b, uint n); }'
$id=[BitConverter]::ToUInt32([Text.Encoding]::ASCII.GetBytes('DSDT'),0)
$n=[FirmwareRead]::GetSystemFirmwareTable(0x41435049,$id,$null,0)
if ($n -gt 0) { $b=New-Object byte[] $n; $got=[FirmwareRead]::GetSystemFirmwareTable(0x41435049,$id,$b,$n); if ($got -ne $n) { throw 'DSDT read failed' }; $r.DSDT=[Convert]::ToBase64String($b) } else { $r.DSDTError=[Runtime.InteropServices.Marshal]::GetLastWin32Error() }
$r.RegistryACPI = @(Get-ChildItem 'Registry::HKEY_LOCAL_MACHINE\HARDWARE\ACPI' -Recurse -ErrorAction SilentlyContinue | Select-Object Name)
$r.EDID = @(Get-ChildItem 'HKLM:\SYSTEM\CurrentControlSet\Enum\DISPLAY' -Recurse -ErrorAction SilentlyContinue | ForEach-Object { $e=Get-ItemProperty $_.PSPath -Name EDID -ErrorAction SilentlyContinue; if ($e) { [pscustomobject]@{Path=$_.Name;Base64=[Convert]::ToBase64String($e.EDID)} } })
$r.DeviceProperties = @(Get-PnpDevice -PresentOnly | Where-Object InstanceId -Match '^ACPI\\(QTEC|EDP|QCOM24A5|QCOM0811|QCOM0818|QCOM0871)' | ForEach-Object { $dev=$_; [pscustomobject]@{Name=$dev.FriendlyName;Id=$dev.InstanceId;Properties=@(Get-PnpDeviceProperty -InstanceId $dev.InstanceId | Where-Object KeyName -Match 'Location|Parent|BusNumber|Address|BiosDeviceName|HardwareIds|DriverInfPath' | Select-Object KeyName,Data)} })
$r | ConvertTo-Json -Depth 10 -Compress
