@echo off
rem ============================================================================
rem  pcbirc cleanup -- v1.6 reorg + pcbic v1.0.0 restructure
rem
rem  Removes stale directories from prior reorgs after their new-location
rem  contents have been verified present:
rem
rem     drivers/firmware/     -> pcb1541/install/dist/target/COMMDRV/
rem     drivers/config/       -> pcb1541/install/dist/target/COMMDRV/
rem     drivers/scripts/      -> pcb1541/install/dist/target/COMMDRV/
rem     extracted/            -> pcb1541/install/dist/target/ (fanned out)
rem     archivers/            -> pcb1541/install/archivers/
rem     pcb1541/pcbic/        -> pcb1541/pcbic12/ (pcbic v1.0.0 restructure)
rem                              also removes pcb1541/pcbic/Pcbic12.zip
rem
rem  Safe to run multiple times (idempotent). Logs to CLEANUP.LOG.
rem  Run from repo root.
rem ============================================================================

if not exist LICENSE (
    echo ERROR: Run this from the pcbirc repo root ^(where LICENSE lives^).
    pause
    goto :EOF
)

echo. >> CLEANUP.LOG
echo ============================================================================ >> CLEANUP.LOG
echo pcbirc cleanup run: %DATE% %TIME% >> CLEANUP.LOG
echo ============================================================================ >> CLEANUP.LOG
echo. >> CLEANUP.LOG

set REMOVED=0
set KEPT=0

rem --- Idempotency check: if none of the stale dirs exist, we're done.
set DIRTY=0
if exist drivers\firmware              set DIRTY=1
if exist drivers\config                set DIRTY=1
if exist drivers\scripts               set DIRTY=1
if exist extracted                      set DIRTY=1
if exist archivers                      set DIRTY=1
if exist pcb1541\pcbic                  set DIRTY=1

if "%DIRTY%"=="0" (
    echo ALREADY CLEAN -- no stale directories found. >> CLEANUP.LOG
    echo.
    echo  ALREADY CLEAN -- no stale directories present.
    echo  Nothing to do. Log appended to CLEANUP.LOG.
    echo.
    pause
    goto :EOF
)

rem ----------------------------------------------------------------------------
rem  drivers/firmware -> now under pcb1541/install/dist/target/COMMDRV/
rem ----------------------------------------------------------------------------
if exist drivers\firmware (
    if exist pcb1541\install\dist\target\COMMDRV\XABIOS.BIN (
        echo REMOVING drivers\firmware\ ^(now at pcb1541/install/dist/target/COMMDRV/^) >> CLEANUP.LOG
        rmdir /S /Q drivers\firmware
        echo   drivers\firmware\        REMOVED
        set /a REMOVED+=1
    ) else (
        echo KEEPING drivers\firmware\ -- target not populated, cannot verify safe delete >> CLEANUP.LOG
        echo   drivers\firmware\        KEPT ^(target not populated -- apply v1.6.3 first^)
        set /a KEPT+=1
    )
)

rem ----------------------------------------------------------------------------
rem  drivers/config -> now under pcb1541/install/dist/target/COMMDRV/
rem ----------------------------------------------------------------------------
if exist drivers\config (
    if exist pcb1541\install\dist\target\COMMDRV\ARNETSP4.DAT (
        echo REMOVING drivers\config\ ^(now at pcb1541/install/dist/target/COMMDRV/^) >> CLEANUP.LOG
        rmdir /S /Q drivers\config
        echo   drivers\config\          REMOVED
        set /a REMOVED+=1
    ) else (
        echo KEEPING drivers\config\ -- target not populated >> CLEANUP.LOG
        echo   drivers\config\          KEPT ^(target not populated -- apply v1.6.3 first^)
        set /a KEPT+=1
    )
)

rem ----------------------------------------------------------------------------
rem  drivers/scripts -> now under pcb1541/install/dist/target/COMMDRV/
rem ----------------------------------------------------------------------------
if exist drivers\scripts (
    if exist pcb1541\install\dist\target\COMMDRV\MONITOR.BAT (
        echo REMOVING drivers\scripts\ ^(now at pcb1541/install/dist/target/COMMDRV/^) >> CLEANUP.LOG
        rmdir /S /Q drivers\scripts
        echo   drivers\scripts\         REMOVED
        set /a REMOVED+=1
    ) else (
        echo KEEPING drivers\scripts\ -- target not populated >> CLEANUP.LOG
        echo   drivers\scripts\         KEPT ^(target not populated -- apply v1.6.3 first^)
        set /a KEPT+=1
    )
)

rem ----------------------------------------------------------------------------
rem  extracted/ -> fanned out into pcb1541/install/dist/target/ subdirs
rem ----------------------------------------------------------------------------
if exist extracted (
    if exist pcb1541\install\dist\target\PPL\HELLO1.PPS (
        if exist pcb1541\install\dist\target\PCBMAIL\PCBMAIL.EXE (
            echo REMOVING extracted\ ^(fanned out into pcb1541/install/dist/target/^) >> CLEANUP.LOG
            rmdir /S /Q extracted
            echo   extracted\               REMOVED
            set /a REMOVED+=1
        ) else (
            echo KEEPING extracted\ -- target/PCBMAIL not populated >> CLEANUP.LOG
            echo   extracted\               KEPT ^(target not fully populated -- apply v1.6.3 first^)
            set /a KEPT+=1
        )
    ) else (
        echo KEEPING extracted\ -- target/PPL not populated >> CLEANUP.LOG
        echo   extracted\               KEPT ^(target not fully populated -- apply v1.6.3 first^)
        set /a KEPT+=1
    )
)

rem ----------------------------------------------------------------------------
rem  archivers/ -> now under pcb1541/install/archivers/
rem ----------------------------------------------------------------------------
if exist archivers (
    if exist pcb1541\install\archivers\redx\redx.c (
        echo REMOVING archivers\ ^(now at pcb1541/install/archivers/^) >> CLEANUP.LOG
        rmdir /S /Q archivers
        echo   archivers\               REMOVED
        set /a REMOVED+=1
    ) else (
        echo KEEPING archivers\ -- new location not populated >> CLEANUP.LOG
        echo   archivers\               KEPT ^(new location not populated -- apply v1.6.4 first^)
        set /a KEPT+=1
    )
)

echo. >> CLEANUP.LOG
echo Summary: %REMOVED% removed, %KEPT% kept ^(waiting on new-location files^). >> CLEANUP.LOG
echo. >> CLEANUP.LOG

rem ----------------------------------------------------------------------------
rem  pcb1541/pcbic/ -> pcbic v1.0.0 restructure to pcb1541/pcbic12/
rem  Safety check: verify at least one canonical file exists in pcbic12/bin/
rem  before removing the old pcbic/ tree (which also contains Pcbic12.zip).
rem ----------------------------------------------------------------------------
if exist pcb1541\pcbic (
    if exist pcb1541\pcbic12\bin\Pcbic.exe (
        if exist pcb1541\pcbic12\bin\RUNINET.PPE (
            echo REMOVING pcb1541\pcbic\ ^(restructured to pcb1541/pcbic12/^) >> CLEANUP.LOG
            echo   ^(also removes the encrypted Pcbic12.zip that lived inside^) >> CLEANUP.LOG
            rmdir /S /Q pcb1541\pcbic
            echo   pcb1541\pcbic\          REMOVED ^(with Pcbic12.zip inside^)
            set /a REMOVED+=1
        ) else (
            echo KEEPING pcb1541\pcbic\ -- pcbic12/bin/RUNINET.PPE missing, cannot verify safe delete >> CLEANUP.LOG
            echo   pcb1541\pcbic\          KEPT ^(pcbic12/ not fully populated -- apply pcbic v1.0.0 release first^)
            set /a KEPT+=1
        )
    ) else (
        echo KEEPING pcb1541\pcbic\ -- pcbic12/bin/Pcbic.exe missing, cannot verify safe delete >> CLEANUP.LOG
        echo   pcb1541\pcbic\          KEPT ^(pcbic12/ not populated -- apply pcbic v1.0.0 release first^)
        set /a KEPT+=1
    )
)

echo.
echo  Cleanup done: %REMOVED% removed, %KEPT% kept.
echo  Log appended to CLEANUP.LOG.
echo.
echo  Next steps:
echo    1. If any "KEPT" above, apply the corresponding release zip
echo       (v1.6.3 target/, v1.6.4 archivers/, pcbic v1.0.0 pcbic12/)
echo       then re-run this.
echo    2. Commit + push via GitHub Desktop:
echo         git add -A
echo         git commit -m "cleanup: remove stale pcb1541/pcbic/ and v1.6 reorg dirs"
echo.
pause
