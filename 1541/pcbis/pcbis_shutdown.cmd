/* pcbis_shutdown.cmd — Stop PCBoard BBS (OS/2) */
/* Part of pcbrevival (GPL v3.0) */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

say 'pcbis_shutdown: stopping PCBoard...'

/* Find and kill PCBOARD.EXE */
/* OS/2 doesn't have taskkill — use PSTAT or KILL */
'@pstat /c | rxqueue'
do while queued() > 0
    parse pull line
    if pos('PCBOARD', translate(line)) > 0 then do
        parse var line pid .
        if datatype(pid,'W') then do
            say 'Killing PID' pid
            '@kill' pid
        end
    end
end

say 'PCBoard stopped.'
