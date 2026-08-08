/* pcbis_startup.cmd — Start PCBoard BBS (OS/2) */
/* Part of pcbrevival (GPL v3.0) */
/* REXX script for OS/2 Warp */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

pcbroot = value('PCBIS_ROOT',,'OS2ENVIRONMENT')
if pcbroot = '' then pcbroot = 'C:\PCBOARD'

say 'pcbis_startup: beginning'
say 'PCBoard root: ' pcbroot

/* Check prerequisites */
if stream(pcbroot'\PCBOARD.EXE','C','QUERY EXISTS') = '' then do
    say 'ERROR: PCBOARD.EXE not found in' pcbroot
    say '       Run pcbis_initv.cmd first.'
    exit 1
end

/* Start PCBoard */
say 'Starting PCBoard...'
'@start /min /n' pcbroot'\PCBOARD.EXE /N:1'

say 'pcbis_startup: complete'
say ''
say 'PCBoard is running.'
say '  Stop: pcbis_shutdown.cmd'
