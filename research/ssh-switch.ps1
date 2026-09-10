$ErrorActionPreference='Stop'
try {
$exe='C:\Program Files\OpenSSH\sshd.exe'
if((Get-Service sshd).Status -ne 'Stopped') { throw 'Expected stopped service' }
$ids=@(Get-NetTCPConnection -LocalPort 22 -State Listen | Select-Object -ExpandProperty OwningProcess -Unique)
if($ids.Count -ne 1) { throw 'Unexpected listeners' }
$p=Get-CimInstance Win32_Process -Filter ('ProcessId='+$ids[0])
if($p.ExecutablePath -ne $exe) { throw 'Unexpected listening process' }
& $exe -t
if($LASTEXITCODE -ne 0) { throw 'Invalid configuration' }
Stop-Process -Id $ids[0] -Force -Confirm:$false -ErrorAction Stop
Start-Service sshd
(Get-Service sshd).WaitForStatus('Running',[TimeSpan]::FromSeconds(20))
Get-CimInstance Win32_Service -Filter "Name='sshd'" | Select-Object Name,State,StartMode,StartName,PathName,ProcessId | ConvertTo-Json -Compress | Set-Content -Encoding UTF8 'C:\ProgramData\ssh\tcl-ssh-switch-result.json'
} catch { $_.Exception.Message | Set-Content -Encoding UTF8 'C:\ProgramData\ssh\tcl-ssh-switch-error.txt'; exit 1 }
