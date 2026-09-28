/* pcbis_shutdown.cmd — stop pcboard bbs (os/2) */
/* part of pcbrevival (gpl v3.0) */

call rxfuncadd 'sysloadfuncs', 'rexxutil', 'sysloadfuncs'
call sysloadfuncs

say 'pcbis_shutdown: stopping pcboard...'

/* find and kill pcboard.exe */
/* os/2 doesn't have taskkill — use pstat or kill */
'@pstat /c | rxqueue'
do while queued() > 0
    parse pull line
    if pos('pcboard', translate(line)) > 0 then do
        parse var line pid .
        if datatype(pid,'w') then do
            say 'killing pid' pid
            '@kill' pid
        end
    end
end

say 'pcboard stopped.'
