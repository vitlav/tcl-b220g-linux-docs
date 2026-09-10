$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
$disks=@(Get-Disk | Select-Object Number,FriendlyName,SerialNumber,BusType,Size,LogicalSectorSize,PartitionStyle,IsBoot,IsSystem)
$parts=@(Get-Partition | Select-Object DiskNumber,PartitionNumber,Offset,Size,GptType,Guid,DriveLetter,Type)
$pnp=@(Get-PnpDevice | Where-Object { $_.InstanceId -like 'ACPI\QCOM0817*' } | Select-Object Status,FriendlyName,InstanceId)
[ordered]@{Disks=$disks;Partitions=$parts;RemoteFS=$pnp} | ConvertTo-Json -Depth 5 | Set-Content -Encoding UTF8 rmtfs-inventory.json
Get-Content -Raw rmtfs-inventory.json
