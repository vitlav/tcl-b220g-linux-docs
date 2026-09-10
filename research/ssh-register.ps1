$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
try {
$exe='C:\Program Files\OpenSSH\sshd.exe'
& $exe -t
if($LASTEXITCODE -ne 0) { throw 'sshd configuration validation failed' }
$acl=Get-Acl $exe
$acl | Select-Object Owner,AccessToString | ConvertTo-Json -Compress
$svc=Get-Service sshd -ErrorAction SilentlyContinue
if($svc) { throw 'Service now exists; inspect before changing' }
New-Service -Name sshd -DisplayName 'OpenSSH SSH Server' -BinaryPathName ('"'+$exe+'"') -Description 'OpenSSH SSH Server' -StartupType Automatic | Out-Null
& sc.exe privs sshd SeAssignPrimaryTokenPrivilege/SeTcbPrivilege/SeBackupPrivilege/SeRestorePrivilege/SeImpersonatePrivilege
if($LASTEXITCODE -ne 0) { throw 'Setting service privileges failed' }
Get-CimInstance Win32_Service -Filter "Name='sshd'" | Select-Object Name,State,StartMode,StartName,PathName | ConvertTo-Json -Compress
} catch { [Console]::Error.WriteLine($_.Exception.Message); exit 1 }
