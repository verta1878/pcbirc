/* pcbis_startup.cmd — start pcboard bbs (os/2) */
/* part of pcbrevival (gpl v3.0) */
/* rexx script for os/2 warp */

call rxfuncadd 'sysloadfuncs', 'rexxutil', 'sysloadfuncs'
call sysloadfuncs

pcbroot = value('pcbis_root',,'os2environment')
if pcbroot = '' then pcbroot = 'c:\pcboard'

say 'pcbis_startup: beginning'
say 'pcboard root: ' pcbroot

/* check prerequisites */
if stream(pcbroot'\pcboard.exe','c','query exists') = '' then do
    say 'error: pcboard.exe not found in' pcbroot
    say '       run pcbis_initv.cmd first.'
    exit 1
end

/* start pcboard */
say 'starting pcboard...'
'@start /min /n' pcbroot'\pcboard.exe /n:1'

say 'pcbis_startup: complete'
say ''
say 'pcboard is running.'
say '  stop: pcbis_shutdown.cmd'
