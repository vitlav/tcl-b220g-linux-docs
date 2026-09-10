import pathlib,tarfile,struct,json,hashlib,re
D=pathlib.Path('.claude/docs/tcl-b220g-data/dt-inventory-20260910'); nodes={}
with tarfile.open(D/'live-tree.tar') as t:
 for m in t:
  name='/' + m.name.removeprefix('./').strip('.') .strip('/')
  if m.isdir(): nodes.setdefault(name,{})
  elif m.isfile():
   parent,key=name.rsplit('/',1); nodes.setdefault(parent or '/',{})[key]=t.extractfile(m).read()
def cells(b): return list(struct.unpack('>'+str(len(b)//4)+'I',b)) if len(b)%4==0 else []
def strings(b):
 return bool(b) and b[0]!=0 and b.endswith(b'\0') and all(c==0 or 32<=c<127 for c in b) and any(c!=0 for c in b)
def val(b):
 if not b:return 'true'
 if strings(b):return ', '.join(b.rstrip(b'\0').decode().split('\0'))
 if len(b)%4==0:return '<'+' '.join(hex(x) for x in cells(b))+'>'
 return '['+b.hex(' ')+']'
def parent(p):return p.rsplit('/',1)[0] or '/'
def integer(p,k,default):return cells(nodes.get(p,{}).get(k,b''))[0] if nodes.get(p,{}).get(k) else default
ph={cells(v[k])[0]:p for p,v in nodes.items() for k in ('phandle',) if k in v}
refs={'clocks':'#clock-cells','assigned-clocks':'#clock-cells','assigned-clock-parents':'#clock-cells','resets':'#reset-cells','power-domains':'#power-domain-cells','iommus':'#iommu-cells','interconnects':'#interconnect-cells','phys':'#phy-cells','dmas':'#dma-cells','mboxes':'#mbox-cells','io-channels':'#io-channel-cells','thermal-sensors':'#thermal-sensor-cells','interrupts-extended':'#interrupt-cells','sound-dai':'#sound-dai-cells','pwms':'#pwm-cells'}
single={'interrupt-parent','memory-region','operating-points-v2','next-level-cache','remote-endpoint','nvmem-cells','qcom,rpmh-rsc','qcom,bcm-voters'}
def render(p,k,b):
 s=val(b); a=cells(b)
 if k in refs or k.endswith('-gpios') or k=='gpios':
  count=refs.get(k,'#gpio-cells'); i=0; out=[]
  while i<len(a):
   target=ph.get(a[i]); i+=1
   if not target:out.append('unresolved/null '+hex(a[i-1]));break
   n=integer(target,count,0); args=a[i:i+n];i+=n
   out.append(target+(' ('+', '.join(str(x)+'/'+hex(x) for x in args)+')' if args else ''))
  return s+' → '+'; '.join(out)
 if k.endswith('-supply') or k in single or re.fullmatch(r'pinctrl-\d+',k):
  return s+' → '+'; '.join(ph.get(x,'cell '+hex(x)) for x in a)
 if k.endswith('-microvolt') or k.endswith('-microamp') or k in ('clock-frequency','assigned-clock-rates','drive-strength','bus-width','data-lanes','hid-descr-addr','opp-level'):
  return s+' = '+', '.join(str(x) for x in a)
 if k=='opp-hz' and len(b)==8:return s+' = '+str(int.from_bytes(b,'big'))+' Hz'
 if k=='reg':
  par=parent(p);ac=integer(par,'#address-cells',2);sc=integer(par,'#size-cells',1); n=ac+sc
  if n and len(a)%n==0:
   out=[]
   for i in range(0,len(a),n):
    addr=int.from_bytes(b[i*4:(i+ac)*4],'big'); size=int.from_bytes(b[(i+ac)*4:(i+n)*4],'big')
    out.append(hex(addr)+(f' + {hex(size)} ({size} bytes)' if sc else ' (bus address/ID)'))
   return s+' → '+'; '.join(out)
 if k=='interrupts':
  cur=p
  while 'interrupt-parent' not in nodes.get(cur,{}) and cur!='/':cur=parent(cur)
  ip=ph.get(integer(cur,'interrupt-parent',-1)); out=[]
  if ip:
   n=integer(ip,'#interrupt-cells',0)
   if n and len(a)%n==0:
    for i in range(0,len(a),n):
     v=a[i:i+n]
     if 'arm,gic-v3' in val(nodes[ip].get('compatible',b'')) and n==3:
      out.append(('SPI' if v[0]==0 else 'PPI' if v[0]==1 else str(v[0]))+' '+str(v[1])+' flags='+hex(v[2])+(f' (INTID {v[1]+(32 if v[0]==0 else 16)})' if v[0] in (0,1) else ''))
     else:out.append(str(v))
   return s+' → '+ip+' '+ '; '.join(out)
 return s
def status(p):
 cur=p
 while True:
  s=val(nodes.get(cur,{}).get('status',b'okay\0'))
  if s not in ('okay','ok'):return s+(' (ancestor '+cur+')' if cur!=p else '')
  if cur=='/':return 'okay'
  cur=parent(cur)
B=json.loads((D/'live-bindings.json').read_text()); bindings={}
for r in B['bindings']:
 if r['driver']:bindings.setdefault(r['of_node'],set()).add(r['driver'])
def esc(s):return s.replace('|','&#124;').replace('\n',' ')
def props(p,keys):return '<br>'.join('`'+esc(k)+'`: '+esc(render(p,k,nodes[p][k])) for k in keys) or '—'
head=f'''# TCL B220G: полный реестр живого Device Tree — 2026-09-10

Источник: `/sys/firmware/devicetree/base`, boot `{B['boot_id']}`, ядро `{B['kernel']}`. Дерево снято после загрузки, поэтому включает применённые изменения OF. Привязки драйверов сняты отдельно в той же загрузке. Исходный архив SHA256 `{hashlib.sha256((D/'live-tree.tar').read_bytes()).hexdigest()}`.

Это полный реестр **описания DT**, а не доказательство наличия каждого блока на плате. `okay` означает разрешённый узел; привязанный драйвер не гарантирует работоспособности всех функций. `disabled` и отключённые предки указаны явно. Отсутствующее свойство не заменено предположением. Устройства, перечисляемые USB, SoundWire или firmware динамически, могут не иметь отдельного DT-узла.

Адрес `reg` интерпретирован по `#address-cells/#size-cells` родителя: для MMIO показаны адрес и размер, для I²C — адрес ведомого. Адреса остаются в адресном пространстве родительской шины; преобразования `ranges` сохранены в полном перечне. SPI/PPI — номера DT, INTID — архитектурный номер GIC, **не Linux IRQ**. Флаги IRQ: 1 rising, 2 falling, 4 level-high, 8 level-low. GPIO specifier — линия/флаги провайдера (обычно 0 active-high, 1 active-low); смысл прочих числовых specifier определяется binding соответствующего контроллера. Напряжения — в µV, частоты clocks/OPP — в Hz, drive-strength — в mA; это заданные ограничения, не измерения. Phandle разрешён до полного пути. Pinctrl, OPP, регуляторы, endpoint и reserved-memory вынесены отдельными строками, чтобы не терять зависимости.

'''
lines=[head,'## Все узлы с аппаратным описанием и служебные параметры\n','| Узел / compatible | Статус DT; драйвер сейчас | Адреса / IRQ | GPIO, pinctrl, линии | Питание / тактирование | Остальные свойства |','|---|---|---|---|---|---|']
count=0
for p,v in sorted(nodes.items()):
 if not v:continue
 if p.startswith('/__symbols__') or p.startswith('/__fixups__'):continue
 keys=sorted(set(v)-{'name','phandle','linux,phandle','compatible','status'})
 a=[k for k in keys if k in ('reg','reg-names','ranges','dma-ranges','interrupts','interrupt-parent','interrupts-extended','interrupt-names')]
 g=[k for k in keys if k not in a and (any(x in k for x in ('gpio','pinctrl','pins','bias-','drive-','data-lanes','function')))]
 power=[k for k in keys if k not in a+g and any(x in k for x in ('supply','regulator','clock','power-domain','opp-','operating-points','resets','reset-names'))]
 other=[k for k in keys if k not in a+g+power]
 lines.append('| `'+p+'`<br>'+esc(val(v.get('compatible',b'')) if 'compatible'in v else 'узел параметров/ресурсов')+' | '+esc(status(p))+'; '+', '.join(sorted(bindings.get(p,{'нет привязки в снимке'})))+' | '+' | '.join(props(p,x) for x in (a,g,power,other))+' |');count+=1
(D/'full-inventory.md').write_text('\n'.join(lines)+'\n')
(D/'properties.json').write_text(json.dumps({p:{k:render(p,k,b) for k,b in sorted(v.items())} for p,v in sorted(nodes.items())},ensure_ascii=False,indent=2)+'\n')
print('nodes',len(nodes),'table rows',count,'bytes',(D/'full-inventory.md').stat().st_size)
