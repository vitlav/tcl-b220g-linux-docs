$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; public class FirmwareRead { [DllImport("kernel32.dll", SetLastError=true)] public static extern uint EnumSystemFirmwareTables(uint p, byte[] b, uint n); [DllImport("kernel32.dll", SetLastError=true)] public static extern uint GetSystemFirmwareTable(uint p, uint id, byte[] b, uint n); }'
$provider = [uint32]0x41435049
$size = [FirmwareRead]::EnumSystemFirmwareTables($provider,$null,0)
if ($size -eq 0) { throw 'ACPI enumeration returned zero bytes' }
$ids = New-Object byte[] $size
if ([FirmwareRead]::EnumSystemFirmwareTables($provider,$ids,$size) -ne $size) { throw 'ACPI enumeration changed size' }
$tables = @()
for ($i=0; $i -lt $size; $i+=4) { $name=[Text.Encoding]::ASCII.GetString($ids,$i,4); if ($name -eq 'MSDM') { continue }; $id=[BitConverter]::ToUInt32($ids,$i); $n=[FirmwareRead]::GetSystemFirmwareTable($provider,$id,$null,0); if ($n -eq 0) { throw "Empty ACPI table $name" }; $b=New-Object byte[] $n; if ([FirmwareRead]::GetSystemFirmwareTable($provider,$id,$b,$n) -ne $n) { throw "Failed reading $name" }; $tables += [pscustomobject]@{Name=$name;Length=$n;Base64=[Convert]::ToBase64String($b)} }
$tables | ConvertTo-Json -Compress
