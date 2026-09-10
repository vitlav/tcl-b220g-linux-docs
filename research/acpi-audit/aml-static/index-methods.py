from pathlib import Path
import re,json
s=Path('.claude/docs/tcl-b220g-data/DSDT.dsl').read_text()
t=re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"',lambda m: ''.join('\n' if c=='\n' else ' ' for c in m[0]),s,flags=re.S)
labels={}
for m in re.finditer(r'\b(Device|Scope|ThermalZone|Method)\s*\(\s*([\\\w.^]+)',t):
 labels[t.index('{',m.end())]=(m[1],m[2],m.start())
stack=[]; methods=[]
for m in re.finditer(r'[{}]',t):
 if m[0]=='{':stack.append((m.start(),labels.get(m.start())))
 else:
  start,lab=stack.pop()
  if lab and lab[0]=='Method':
   path=[]
   for _,parent in stack:
    if parent and parent[0] in ['Device','Scope','ThermalZone']:
     n=parent[1]
     if n.startswith('\\'):path=n[1:].split('.')
     else:path.append(n)
   path.append(lab[1]); body=s[lab[2]:m.end()]
   methods.append({'path':'\\'+'.'.join(path),'line':s.count('\n',0,lab[2])+1,'body':body})
Path('/tmp/tcl-aml-static/method-index.json').write_text(json.dumps(methods,indent=2)+'\n')
