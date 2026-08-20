@echo off
rem ============================================================================
rem  pcb1541 cleanup — run from repo root
rem  Cleans 1541\ directory and renames it to pcb1541\
rem
rem  Requires: repo-restructure.bat already run (1541\ must exist)
rem  Output: PCB1541-CLEANUP.LOG in repo root
rem ============================================================================

if not exist LICENSE (
    echo ERROR: Run this from the pcbirc repo root.
    pause
    goto :EOF
)

if not exist 1541 (
    if exist pcb1541 (
        echo SKIP: pcb1541 already exists, 1541 not found.
        echo       This script may have already been run.
        pause
        goto :EOF
    )
    echo ERROR: 1541\ not found.
    pause
    goto :EOF
)

set LOGFILE=PCB1541-CLEANUP.LOG
echo pcb1541 cleanup > %LOGFILE%
echo Date: %DATE% %TIME% >> %LOGFILE%
echo. >> %LOGFILE%
set ERRORS=0

rem --- Delete ELF binaries (Linux compiled, don't belong) ---
if exist 1541\pcbfido_linux (
    del 1541\pcbfido_linux
    echo OK: Deleted ELF binary pcbfido_linux >> %LOGFILE%
) else (
    echo SKIP: pcbfido_linux not found >> %LOGFILE%
)

if exist 1541\pcbtic (
    if not exist 1541\pcbtic\*.* (
        del 1541\pcbtic
        echo OK: Deleted ELF binary pcbtic >> %LOGFILE%
    ) else (
        echo SKIP: pcbtic is a directory, not ELF >> %LOGFILE%
    )
)

if exist 1541\upgrade1541 (
    if not exist 1541\upgrade1541\*.* (
        del 1541\upgrade1541
        echo OK: Deleted ELF binary upgrade1541 >> %LOGFILE%
    ) else (
        echo SKIP: upgrade1541 is a directory >> %LOGFILE%
    )
)

rem --- Delete pcbdrawbak (reference apps, not our code) ---
if exist 1541\pcbdrawbak (
    rmdir /S /Q 1541\pcbdrawbak
    if not exist 1541\pcbdrawbak (
        echo OK: Deleted pcbdrawbak\ >> %LOGFILE%
    ) else (
        echo FAIL: Could not delete pcbdrawbak\ >> %LOGFILE%
        set /a ERRORS+=1
    )
) else (
    echo SKIP: pcbdrawbak not found >> %LOGFILE%
)

rem --- Delete upgrade1541.c (duplicate of upd1541.c) ---
if exist 1541\upgrade1541.c (
    del 1541\upgrade1541.c
    echo OK: Deleted upgrade1541.c (duplicate, kept upd1541.c) >> %LOGFILE%
) else (
    echo SKIP: upgrade1541.c not found >> %LOGFILE%
)

rem --- Delete PCBKIT_L.LIB duplicates (kept in pcbcbase\PREBUILT\BC31\) ---
if exist 1541\..\PCBKIT_L.LIB (
    del PCBKIT_L.LIB
    echo OK: Deleted root PCBKIT_L.LIB duplicate >> %LOGFILE%
)
if exist 1541\..\pcb154\PCBKIT_L.LIB (
    del pcb154\PCBKIT_L.LIB
    echo OK: Deleted pcb154\PCBKIT_L.LIB duplicate >> %LOGFILE%
)

rem --- Delete pcb154\LIB\&1 artifact ---
if exist "pcb154\LIB\&1" (
    del "pcb154\LIB\&1"
    echo OK: Deleted pcb154\LIB\&1 artifact >> %LOGFILE%
)

rem --- Create directories for loose source files ---
if not exist 1541\pcbfido mkdir 1541\pcbfido
if not exist 1541\pcbtic mkdir 1541\pcbtic
if not exist 1541\upd1541 mkdir 1541\upd1541

rem --- Move loose source files into their directories ---
if exist 1541\pcbfido.c (
    move 1541\pcbfido.c 1541\pcbfido\ >> %LOGFILE% 2>&1
    echo OK: pcbfido.c -> pcbfido\ >> %LOGFILE%
)
if exist 1541\pcbfcfg.c (
    move 1541\pcbfcfg.c 1541\pcbfido\ >> %LOGFILE% 2>&1
    echo OK: pcbfcfg.c -> pcbfido\ >> %LOGFILE%
)
if exist 1541\pcbtic.c (
    move 1541\pcbtic.c 1541\pcbtic\ >> %LOGFILE% 2>&1
    echo OK: pcbtic.c -> pcbtic\ >> %LOGFILE%
)
if exist 1541\upd1541.c (
    move 1541\upd1541.c 1541\upd1541\ >> %LOGFILE% 2>&1
    echo OK: upd1541.c -> upd1541\ >> %LOGFILE%
)
if exist 1541\bso_clark_style.c (
    move 1541\bso_clark_style.c 1541\pcbbinkp\ >> %LOGFILE% 2>&1
    echo OK: bso_clark_style.c -> pcbbinkp\ >> %LOGFILE%
)
if exist 1541\utrayit.c (
    move 1541\utrayit.c 1541\pcbdraw\ >> %LOGFILE% 2>&1
    echo OK: utrayit.c -> pcbdraw\ >> %LOGFILE%
)
if exist 1541\utrayit.h (
    move 1541\utrayit.h 1541\pcbdraw\ >> %LOGFILE% 2>&1
    echo OK: utrayit.h -> pcbdraw\ >> %LOGFILE%
)
if exist 1541\pcbis_nodedata.pas (
    move 1541\pcbis_nodedata.pas 1541\pcbis\ >> %LOGFILE% 2>&1
    echo OK: pcbis_nodedata.pas -> pcbis\ >> %LOGFILE%
)

rem --- Move reference zips to reference\ ---
if not exist reference mkdir reference
for %%Z in (QFRONT.zip UUPC.zip btxe-source.zip portal-of-power-src.zip) do (
    if exist 1541\%%Z (
        move 1541\%%Z reference\ >> %LOGFILE% 2>&1
        echo OK: %%Z -> reference\ >> %LOGFILE%
    )
)

rem --- Move stale root files ---
if exist README.TXT (
    if exist pcb154 (
        move README.TXT pcb154\ >> %LOGFILE% 2>&1
        echo OK: README.TXT -> pcb154\ >> %LOGFILE%
    )
)
if exist CLEANIT.BAT (
    if exist pcb154 (
        move CLEANIT.BAT pcb154\ >> %LOGFILE% 2>&1
        echo OK: CLEANIT.BAT -> pcb154\ >> %LOGFILE%
    )
)
if exist pcb153src0014.zip (
    move pcb153src0014.zip reference\ >> %LOGFILE% 2>&1
    echo OK: pcb153src0014.zip -> reference\ >> %LOGFILE%
)

rem --- Delete .err files from root (compile test artifacts) ---
if exist *.err (
    del *.err
    echo OK: Deleted .err files from root >> %LOGFILE%
) else (
    echo SKIP: No .err files at root >> %LOGFILE%
)

rem --- Rename 1541 -> pcb1541 ---
if exist pcb1541 (
    echo ERROR: pcb1541 already exists, cannot rename 1541 >> %LOGFILE%
    set /a ERRORS+=1
) else (
    ren 1541 pcb1541
    if exist pcb1541 (
        echo OK: 1541 -> pcb1541 >> %LOGFILE%
    ) else (
        echo FAIL: Could not rename 1541 to pcb1541 >> %LOGFILE%
        set /a ERRORS+=1
    )
)

rem --- Verify ---
echo. >> %LOGFILE%
echo === VERIFICATION === >> %LOGFILE%
for %%D in (pcb1541 pcb1541\pcbfido pcb1541\pcbtic pcb1541\upd1541 pcb1541\pcbdraw pcb1541\pcbbinkp pcb1541\pcbis pcb1541\pcbmail pcb1541\pcbcp pcb1541\qfront pcb1541\wip) do (
    if exist %%D (
        echo FOUND: %%D >> %LOGFILE%
    ) else (
        echo MISSING: %%D >> %LOGFILE%
        set /a ERRORS+=1
    )
)

echo. >> %LOGFILE%
echo === SHOULD NOT EXIST === >> %LOGFILE%
for %%F in (1541 pcb1541\pcbdrawbak pcb1541\pcbfido_linux pcb1541\upgrade1541.c pcb1541\pcbfido.c pcb1541\pcbtic.c pcb1541\upd1541.c pcb1541\bso_clark_style.c pcb1541\utrayit.c pcb1541\pcbis_nodedata.pas PCBKIT_L.LIB) do (
    if exist %%F (
        echo STILL EXISTS: %%F >> %LOGFILE%
        set /a ERRORS+=1
    ) else (
        echo GONE: %%F >> %LOGFILE%
    )
)

echo. >> %LOGFILE%
echo === SUMMARY === >> %LOGFILE%
echo Errors: %ERRORS% >> %LOGFILE%

echo.
echo  pcb1541 cleanup complete. %ERRORS% error(s).
echo  See %LOGFILE% for details.
echo.
if %ERRORS% GTR 0 (
    echo  ** ERRORS FOUND — check %LOGFILE% **
    echo.
)
type %LOGFILE%
