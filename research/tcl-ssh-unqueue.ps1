$ErrorActionPreference='Stop'
$s=New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -ExecutionTimeLimit (New-TimeSpan -Minutes 2)
Set-ScheduledTask -TaskName 'TCL-SSH-Service-Switch' -Settings $s | Out-Null
Start-ScheduledTask -TaskName 'TCL-SSH-Service-Switch'
