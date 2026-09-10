$ErrorActionPreference='Stop'
[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false)
$out=Join-Path $HOME 'tcl-rmtfs-export'
New-Item -ItemType Directory -Force $out | Out-Null
$result=@()
foreach($disk in 0..5) {
 $f=$null
 try {
  $path='\\.\PhysicalDrive'+$disk
  $f=[IO.FileStream]::new($path,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::ReadWrite)
  $buf=[byte[]]::new(131072); $n=$f.Read($buf,0,$buf.Length)
  [IO.File]::WriteAllBytes((Join-Path $out ('disk'+$disk+'-gpt.bin')),$buf)
  $sector=0
  foreach($ss in 512,4096) { if([Text.Encoding]::ASCII.GetString($buf,$ss,8) -eq 'EFI PART') {$sector=$ss;break} }
  if(!$sector) {throw 'No primary GPT at LBA1 (512/4096)'}
  $entryLba=[BitConverter]::ToUInt64($buf,$sector+72)
  $count=[BitConverter]::ToUInt32($buf,$sector+80)
  $entrySize=[BitConverter]::ToUInt32($buf,$sector+84)
  if($count -gt 512 -or $entrySize -lt 128 -or $entrySize -gt 1024 -or ($entryLba*$sector+$count*$entrySize) -gt $n) {throw 'GPT entries outside bounded capture'}
  $parts=@()
  for($i=0;$i -lt $count;$i++) {
   $o=[int]($entryLba*$sector+$i*$entrySize)
   $guidBytes=[byte[]]::new(16);[Array]::Copy($buf,$o,$guidBytes,0,16)
   $type=[Guid]::new($guidBytes); if($type -eq [Guid]::Empty){continue}
   $start=[BitConverter]::ToUInt64($buf,$o+32);$end=[BitConverter]::ToUInt64($buf,$o+40)
   $name=[Text.Encoding]::Unicode.GetString($buf,$o+56,72).TrimEnd([char]0)
   $parts += [ordered]@{Index=$i+1;Name=$name;Type=$type.ToString();StartLBA=$start;EndLBA=$end;Offset=$start*$sector;Size=($end-$start+1)*$sector}
  }
  $result += [ordered]@{Disk=$disk;Sector=$sector;Parts=$parts}
 } catch { $result += [ordered]@{Disk=$disk;Error=$_.Exception.Message} }
 finally {if($f){$f.Dispose()}}
}
$result | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 (Join-Path $out 'gpt.json')
Get-Content -Raw (Join-Path $out 'gpt.json')
