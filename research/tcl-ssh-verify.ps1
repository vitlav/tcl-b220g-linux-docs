$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
$svc=Get-CimInstance Win32_Service -Filter "Name='sshd'"
$listeners=@(Get-NetTCPConnection -LocalPort 22 -State Listen | Select-Object -ExpandProperty OwningProcess -Unique)
if($svc.State -ne 'Running' -or $svc.StartMode -ne 'Auto' -or $listeners.Count -ne 1 -or $listeners[0] -ne $svc.ProcessId) { throw 'Service/listener mismatch' }
$task=Get-ScheduledTaskInfo -TaskName 'TCL-SSH-Service-Switch'
if($task.LastTaskResult -ne 0) { throw 'Switch task failed' }
Unregister-ScheduledTask -TaskName 'TCL-SSH-Service-Switch' -Confirm:$false
[ordered]@{Service=($svc | Select-Object Name,State,StartMode,StartName,PathName,ProcessId);ListenerPID=$listeners[0];TaskRemoved=$true;SwitchResult=[IO.File]::ReadAllText('C:\ProgramData\ssh\tcl-ssh-switch-result.json')} | ConvertTo-Json -Depth 4 -Compress
