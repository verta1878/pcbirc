@echo off
rem ============================================================================
rem  pcbirc repo restructure — run from repo root
rem  Makes the repo match hexadecimal's 2026-08-20 session structure.
rem
rem  Output: RESTRUCTURE.LOG in repo root
rem
rem  BACK UP FIRST. This renames and moves directories.
rem ============================================================================

if not exist LICENSE (
    echo ERROR: Run this from the pcbirc repo root.
    echo        LICENSE file not found — wrong directory.
    pause
    goto :EOF
)

set LOGFILE=RESTRUCTURE.LOG
echo pcbirc repo restructure > %LOGFILE%
echo Date: %DATE% %TIME% >> %LOGFILE%
echo. >> %LOGFILE%
set ERRORS=0

rem --- Rename MAIN -> pcb153 ---
if exist MAIN (
    if exist pcb153 (
        echo ERROR: pcb153 already exists, cannot rename MAIN >> %LOGFILE%
        set /a ERRORS+=1
    ) else (
        ren MAIN pcb153
        if exist pcb153 (
            echo OK: MAIN -> pcb153 >> %LOGFILE%
        ) else (
            echo FAIL: Could not rename MAIN to pcb153 >> %LOGFILE%
            set /a ERRORS+=1
        )
    )
) else (
    if exist pcb153 (
        echo SKIP: pcb153 already exists, MAIN not found >> %LOGFILE%
    ) else (
        echo ERROR: Neither MAIN nor pcb153 found >> %LOGFILE%
        set /a ERRORS+=1
    )
)

rem --- Rename PCBSRC -> pcb154 ---
if exist PCBSRC (
    if exist pcb154 (
        echo ERROR: pcb154 already exists, cannot rename PCBSRC >> %LOGFILE%
        set /a ERRORS+=1
    ) else (
        ren PCBSRC pcb154
        if exist pcb154 (
            echo OK: PCBSRC -> pcb154 >> %LOGFILE%
        ) else (
            echo FAIL: Could not rename PCBSRC to pcb154 >> %LOGFILE%
            set /a ERRORS+=1
        )
    )
) else (
    if exist pcb154 (
        echo SKIP: pcb154 already exists, PCBSRC not found >> %LOGFILE%
    ) else (
        echo ERROR: Neither PCBSRC nor pcb154 found >> %LOGFILE%
        set /a ERRORS+=1
    )
)

rem --- Rename LIBS -> pcbcbase ---
if exist LIBS (
    if exist pcbcbase (
        echo ERROR: pcbcbase already exists, cannot rename LIBS >> %LOGFILE%
        set /a ERRORS+=1
    ) else (
        ren LIBS pcbcbase
        if exist pcbcbase (
            echo OK: LIBS -> pcbcbase >> %LOGFILE%
        ) else (
            echo FAIL: Could not rename LIBS to pcbcbase >> %LOGFILE%
            set /a ERRORS+=1
        )
    )
) else (
    if exist pcbcbase (
        echo SKIP: pcbcbase already exists, LIBS not found >> %LOGFILE%
    ) else (
        echo ERROR: Neither LIBS nor pcbcbase found >> %LOGFILE%
        set /a ERRORS+=1
    )
)

rem --- Move LIB contents into toolkit\ ---
if exist LIB (
    if not exist toolkit mkdir toolkit
    if exist LIB\H (
        if not exist toolkit\H (
            move LIB\H toolkit\H >> %LOGFILE% 2>&1
            if exist toolkit\H (
                echo OK: LIB\H -> toolkit\H >> %LOGFILE%
            ) else (
                echo FAIL: Could not move LIB\H >> %LOGFILE%
                set /a ERRORS+=1
            )
        ) else (
            echo SKIP: toolkit\H already exists >> %LOGFILE%
        )
    )
    if exist LIB\SOURCE (
        if not exist toolkit\SOURCE (
            move LIB\SOURCE toolkit\SOURCE >> %LOGFILE% 2>&1
            if exist toolkit\SOURCE (
                echo OK: LIB\SOURCE -> toolkit\SOURCE >> %LOGFILE%
            ) else (
                echo FAIL: Could not move LIB\SOURCE >> %LOGFILE%
                set /a ERRORS+=1
            )
        ) else (
            echo SKIP: toolkit\SOURCE already exists >> %LOGFILE%
        )
    )
    if exist LIB\BUILD.BAT (
        if not exist toolkit\BUILD.BAT (
            move LIB\BUILD.BAT toolkit\BUILD.BAT >> %LOGFILE% 2>&1
            echo OK: LIB\BUILD.BAT -> toolkit\BUILD.BAT >> %LOGFILE%
        ) else (
            echo SKIP: toolkit\BUILD.BAT already exists >> %LOGFILE%
        )
    )
    rmdir LIB 2>nul
    if not exist LIB (
        echo OK: Removed empty LIB\ >> %LOGFILE%
    ) else (
        echo WARN: LIB\ not empty after move, check contents >> %LOGFILE%
        dir /b LIB >> %LOGFILE% 2>&1
    )
) else (
    if exist toolkit\H (
        echo SKIP: LIB not found, toolkit\H already exists >> %LOGFILE%
    ) else (
        echo ERROR: LIB not found and toolkit\H missing >> %LOGFILE%
        set /a ERRORS+=1
    )
)

rem --- Create drivers\ if missing ---
if not exist drivers (
    mkdir drivers
    echo OK: Created drivers\ >> %LOGFILE%
) else (
    echo SKIP: drivers\ already exists >> %LOGFILE%
)

rem --- Copy PCBCP to 1541\pcbcp ---
if not exist 1541\pcbcp (
    if exist reference\pcball\pcboard\pcb-util\PCBCP (
        xcopy /E /I /Q reference\pcball\pcboard\pcb-util\PCBCP 1541\pcbcp >> %LOGFILE% 2>&1
        if exist 1541\pcbcp\SOURCE (
            echo OK: Copied PCBCP OS/2 source to 1541\pcbcp\ >> %LOGFILE%
        ) else (
            echo FAIL: PCBCP copy failed >> %LOGFILE%
            set /a ERRORS+=1
        )
    ) else (
        echo SKIP: reference\pcball\pcboard\pcb-util\PCBCP not found >> %LOGFILE%
    )
) else (
    echo SKIP: 1541\pcbcp already exists >> %LOGFILE%
)

rem --- Copy Pcbic12.zip to 1541\ ---
if not exist 1541\Pcbic12.zip (
    if exist toolkit\devtools\Pcbic12.zip (
        copy toolkit\devtools\Pcbic12.zip 1541\Pcbic12.zip >> %LOGFILE% 2>&1
        if exist 1541\Pcbic12.zip (
            echo OK: Copied Pcbic12.zip to 1541\ >> %LOGFILE%
        ) else (
            echo FAIL: Pcbic12.zip copy failed >> %LOGFILE%
            set /a ERRORS+=1
        )
    ) else (
        echo SKIP: toolkit\devtools\Pcbic12.zip not found >> %LOGFILE%
    )
) else (
    echo SKIP: 1541\Pcbic12.zip already exists >> %LOGFILE%
)

rem --- Verify final structure ---
echo. >> %LOGFILE%
echo === VERIFICATION === >> %LOGFILE%
set MISSING=0
for %%D in (pcb153 pcb154 toolkit toolkit\H toolkit\SOURCE pcbcbase 1541 drivers docs todo) do (
    if exist %%D (
        echo FOUND: %%D >> %LOGFILE%
    ) else (
        echo MISSING: %%D >> %LOGFILE%
        set /a MISSING+=1
    )
)

echo. >> %LOGFILE%
echo === SUMMARY === >> %LOGFILE%
echo Errors: %ERRORS% >> %LOGFILE%
echo Missing directories: %MISSING% >> %LOGFILE%
echo. >> %LOGFILE%
echo Final structure: >> %LOGFILE%
echo   pcb153\        15.3 source (Borland C++ 3.1) >> %LOGFILE%
echo   pcb154\        15.4 source (OpenWatcom 16-bit, no extender) >> %LOGFILE%
echo   toolkit\       Clark's shared toolkit library (H\, SOURCE\) >> %LOGFILE%
echo   pcbcbase\      third-party: CODEBASE + BC31 prebuilt >> %LOGFILE%
echo   1541\          our 15.41 work >> %LOGFILE%
echo   drivers\       FOSSIL etc >> %LOGFILE%
echo   docs\          DOCDEV >> %LOGFILE%
echo   reference\     archives >> %LOGFILE%
echo   todo\          docs to review >> %LOGFILE%
echo. >> %LOGFILE%

rem --- Display result ---
echo.
echo  Restructure complete. %ERRORS% error(s).
echo  See %LOGFILE% for details.
echo.
if %ERRORS% GTR 0 (
    echo  ** ERRORS FOUND — check %LOGFILE% **
    echo.
)
type %LOGFILE%
