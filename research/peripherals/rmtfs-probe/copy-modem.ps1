$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
$out=Join-Path $HOME 'tcl-rmtfs-export'
$gpt=Get-Content (Join-Path $out 'gpt.json') -Raw | ConvertFrom-Json
$d=@($gpt | Where-Object Disk -eq 5)
if($d.Count -ne 1 -or $d[0].Sector -ne 4096){throw 'Unexpected disk5 metadata'}
$expected=@{modemst1=@(262144,2097152,'modem_fs1');modemst2=@(2359296,2097152,'modem_fs2');fsg=@(4456448,2097152,'modem_fsg');fsc=@(6553600,131072,'modem_fsc')}
$f=[IO.FileStream]::new('\\.\PhysicalDrive5',[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::ReadWrite)
try {
 $header=[byte[]]::new(131072);if($f.Read($header,0,$header.Length) -ne $header.Length){throw 'Short GPT read'}
 $sha=[Security.Cryptography.SHA256]::Create()
 $hash={param($b) [BitConverter]::ToString($sha.ComputeHash($b)).Replace('-','')}
 if((&$hash $header) -ne (&$hash ([IO.File]::ReadAllBytes((Join-Path $out 'disk5-gpt.bin'))))){throw 'GPT changed'}
 $result=@()
 foreach($name in $expected.Keys) {
  $v=$expected[$name];$p=@($d[0].Parts | Where-Object Name -eq $name)
  if($p.Count -ne 1 -or $p[0].Offset -ne $v[0] -or $p[0].Size -ne $v[1]){throw ('Partition mismatch '+$name)}
  $a=[byte[]]::new($v[1]);$b=[byte[]]::new($v[1])
  $null=$f.Seek($v[0],[IO.SeekOrigin]::Begin);if($f.Read($a,0,$a.Length) -ne $a.Length){throw 'Short read'}
  $null=$f.Seek($v[0],[IO.SeekOrigin]::Begin);if($f.Read($b,0,$b.Length) -ne $b.Length){throw 'Short second read'}
  $ha=&$hash $a;$hb=&$hash $b
  if($ha -ne $hb){throw ('Partition changing during read: '+$name)}
  [IO.File]::WriteAllBytes((Join-Path $out $v[2]),$a)
  $result += [ordered]@{Partition=$name;File=$v[2];Offset=$v[0];Size=$v[1];SHA256=$ha;DoubleReadMatch=$true}
 }
 $result | ConvertTo-Json | Set-Content -Encoding UTF8 (Join-Path $out 'modem-files.json')
 $result | ConvertTo-Json
} finally {$f.Dispose()}
