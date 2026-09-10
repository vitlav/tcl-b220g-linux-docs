$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$r = [ordered]@{}
$r.System = Get-CimInstance Win32_ComputerSystem | Select-Object Manufacturer,Model,SystemType,TotalPhysicalMemory
$r.Board = Get-CimInstance Win32_BaseBoard | Select-Object Manufacturer,Product,Version
$r.BIOS = Get-CimInstance Win32_BIOS | Select-Object Manufacturer,SMBIOSBIOSVersion,ReleaseDate
$r.OS = Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber,OSArchitecture
$r.CPU = Get-CimInstance Win32_Processor | Select-Object Name,Architecture,NumberOfCores,NumberOfLogicalProcessors
$r.Disks = Get-Disk | Select-Object Number,FriendlyName,BusType,Size,PartitionStyle,IsBoot,IsSystem
$r.Partitions = Get-Partition | Select-Object DiskNumber,PartitionNumber,DriveLetter,Offset,Size,GptType,IsBoot,IsSystem,IsHidden
$r.Devices = Get-CimInstance Win32_PnPEntity | Select-Object Name,PNPDeviceID,HardwareID,Service,Status,ConfigManagerErrorCode
$r.Drivers = Get-CimInstance Win32_PnPSignedDriver | Select-Object DeviceName,DeviceID,InfName,DriverVersion,DriverProviderName
try { $r.SecureBoot = Confirm-SecureBootUEFI } catch { $r.SecureBootError = $_.Exception.Message }
try { $r.BitLocker = Get-BitLockerVolume | Select-Object MountPoint,VolumeStatus,ProtectionStatus,EncryptionPercentage } catch { $r.BitLockerError = $_.Exception.Message }
$r.BootEntries = bcdedit /enum firmware | Out-String
$r.Recovery = reagentc /info | Out-String
$r.Sleep = powercfg /a | Out-String
$r | ConvertTo-Json -Depth 8 -Compress
