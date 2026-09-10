$ErrorActionPreference='Stop'
Copy-Item (Join-Path $PSScriptRoot 'tcl-ssh-switch.ps1') 'C:\ProgramData\ssh\tcl-ssh-switch.ps1'
if(Get-ScheduledTask -TaskName 'TCL-SSH-Service-Switch' -ErrorAction SilentlyContinue) { throw 'Task exists' }
$a=New-ScheduledTaskAction -Execute 'C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe' -Argument '-NoProfile -NonInteractive -ExecutionPolicy Bypass -File C:\ProgramData\ssh\tcl-ssh-switch.ps1'
$p=New-ScheduledTaskPrincipal -UserId 'SYSTEM' -LogonType ServiceAccount -RunLevel Highest
Register-ScheduledTask -TaskName 'TCL-SSH-Service-Switch' -Action $a -Principal $p | Out-Null
Start-ScheduledTask -TaskName 'TCL-SSH-Service-Switch'
