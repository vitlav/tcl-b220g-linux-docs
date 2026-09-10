from pathlib import Path
import re, os, subprocess
p=Path('/etc/wpa_supplicant/wpa_supplicant-wlan0.conf')
s=p.read_text()
blocks=list(re.finditer(r'(?m)^network=\{\n.*?^\}',s,re.S))
assert len(blocks)==1, 'Expected one original profile'
m=blocks[0]
b=m.group()
assert 'ssid="eterwifi"' in b
assert not re.search(r'(?m)^\s*(priority|freq_list|bssid)\s*=',b)
phy=subprocess.check_output(['iw','phy','phy0','info'],text=True)
freqs=sorted(set(int(f) for f in re.findall(r'\* (5\d{3})\.0 MHz[^\n]*',phy)))
assert 5220 in freqs
backup=p.with_name(p.name+'.before-prefer5')
assert not backup.exists(), 'Backup already exists'
os.umask(0o077)
backup.write_text(s)
def profile(extra):
    return b[:-1]+extra+'\n}'
fallback=profile(' priority=0\n id_str="eterwifi-fallback"')
preferred=profile(' priority=10\n id_str="eterwifi-5ghz"\n freq_list='+' '.join(map(str,freqs)))
new=s[:m.start()]+fallback+'\n\n'+preferred+s[m.end():]
tmp=p.with_name(p.name+'.new')
tmp.write_text(new)
os.chmod(tmp,0o600)
os.replace(tmp,p)
os.sync()
print('Saved two profiles: 5GHz priority=10; unrestricted fallback priority=0; mode=600. Running connection unchanged.')
