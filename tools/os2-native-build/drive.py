import subprocess,os,sys
R='/home/claude/fpc264irc'; S=R+'/src'; OUT='/home/claude/os2build/units'
names=open('/tmp/claude-0/-home-claude/7afa3437-1229-5e0a-b7ae-a94fa44e17c8/scratchpad/os2names.txt').read().split()
src=[l.strip()[2:] for l in open('/tmp/claude-0/-home-claude/7afa3437-1229-5e0a-b7ae-a94fa44e17c8/scratchpad/allsrc.txt')]
by={}
for p in src: by.setdefault(os.path.splitext(os.path.basename(p))[0].lower(),[]).append(p)
override={'unixcp':'/home/claude/os2build/patched/unixcp.pas','fpwidestring':'/home/claude/os2build/patched/fpwidestring.pp',
 'crc':'packages/hash/src/crc.pas','resource':'packages/fv/src/resource.pas','dialogs':'packages/fv/src/dialogs.pas',
 'menus':'packages/fv/src/menus.pas','msgbox':'packages/fv/src/msgbox.pas','tabs':'packages/fv/src/tabs.pas',
 'regexpr':'packages/regexpr/src/regexpr.pas','rexxsaa':'os2bindings/rexxsaa.pas','cpu':'rtl/i386/cpu.pp',
 'graph':'packages/graph/src/os2/graph.pp','dynlibs':'rtl/inc/dynlibs.pas','matrix':'rtl/inc/matrix.pp'}
bad=('/win','/linux','/unix','/go32','/amiga','/morphos','/darwin','/bsd','/msdos','/wince','/netware','/beos','/haiku','/tests/','/test/','/examples/','/emx/','lazarus/')
plan={}
for n in names:
    if n=='system': continue
    if n in override: p=override[n]
    else:
        c=by[n]; p=([x for x in c if '/os2/' in x] or [x for x in c if not any(b in '/'+x for b in bad)] or c)[0]
    plan[n]=p if p.startswith('/') else S+'/'+p
INC=['-Fi'+S+d for d in ('/rtl/inc','/rtl/i386','/rtl/objpas','/rtl/objpas/sysutils','/rtl/objpas/classes','/rtl/os2','/rtl/objpas/unicode','/packages/fv/src')]
BASE=['-Tos2','-Pi386','-s','-O2','-Ur','-n','-FU'+OUT,'-Fu'+OUT]+INC
todo={n:f for n,f in plan.items() if not os.path.exists(f'{OUT}/{n}.ppu')}; p=0
while todo:
    p+=1; done=[]
    for n,f in sorted(todo.items()):
        d=os.path.dirname(f)
        r=subprocess.run([R+'/bin/ppc386']+BASE+['-Fi'+d,'-Fi'+d+'/os2','-Fi'+d+'/inc','-Fi'+d+'/../inc',os.path.basename(f)],cwd=d,capture_output=True,text=True)
        open(f'log/{n}.log','w').write(r.stdout+r.stderr)
        if r.returncode==0 and os.path.exists(f'{OUT}/{n}.ppu'): done.append(n)
    for n in done: del todo[n]
    print('pass',p,'built',len(done),'left',len(todo),flush=True)
    if not done: break
print('FAILED:',sorted(todo))
