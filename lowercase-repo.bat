@echo off
REM lowercase-repo.bat v2 — rename all files and directories to lowercase
REM Run from the repo root on Windows (NTFS).
REM Uses "for /d /r" to walk dirs, "dir /l /b" + "ren" to rename.
REM Skips .git tree. Logs to lowercase-repo.log.
REM
REM v1 — file renames worked, dir renames buggy (listed children not self)
REM v2 — fixed dir rename: pipe parent listing through findstr to match
REM
REM hexadecimal — PCBoard, Cyclades lane — the crew 4free

setlocal enabledelayedexpansion

set "VER=v2"
set "ROOT=%CD%"
set "LOGFILE=%ROOT%\lowercase-repo.log"
echo === lowercase-repo.bat %VER% started %DATE% %TIME% === > "%LOGFILE%"
echo ROOT=%ROOT% >> "%LOGFILE%"

REM === STEP 1: Rename FILES directory by directory (skip .git) ===
echo === Step 1: Renaming files ===
echo === Step 1: Renaming files === >> "%LOGFILE%"
set FCOUNT=0
set FSKIP=0

REM First do the root directory
echo DEBUG: processing root >> "%LOGFILE%"
for /f "tokens=*" %%f in ('dir /l /b /a-d 2^>nul') do (
    for /f "tokens=*" %%g in ('dir /b /a-d "%%f" 2^>nul') do (
        if not "%%g"=="%%f" (
            echo DEBUG: ren file "%%g" -^> "%%f" >> "%LOGFILE%"
            ren "%%g" "%%f" >nul 2>nul
            if not errorlevel 1 (
                echo OK   file %%g -^> %%f >> "%LOGFILE%"
                set /a FCOUNT+=1
            ) else (
                echo FAIL file %%g -^> %%f >> "%LOGFILE%"
            )
        ) else (
            set /a FSKIP+=1
        )
    )
)

REM Now walk each subdirectory
for /d /r "%ROOT%" %%D in (*) do (
    echo %%D | find /i ".git" >nul
    if errorlevel 1 (
        echo DEBUG: dir %%D >> "%LOGFILE%"
        for /f "tokens=*" %%f in ('dir /l /b /a-d "%%D\" 2^>nul') do (
            for /f "tokens=*" %%g in ('dir /b /a-d "%%D\%%f" 2^>nul') do (
                if not "%%g"=="%%f" (
                    echo DEBUG: ren file "%%g" -^> "%%f" in %%D >> "%LOGFILE%"
                    ren "%%D\%%g" "%%f" >nul 2>nul
                    if not errorlevel 1 (
                        echo OK   file %%g -^> %%f >> "%LOGFILE%"
                        set /a FCOUNT+=1
                    ) else (
                        echo FAIL file %%g -^> %%f >> "%LOGFILE%"
                    )
                ) else (
                    set /a FSKIP+=1
                )
            )
        )
    )
)

echo Files: %FCOUNT% renamed, %FSKIP% already lowercase
echo Files: %FCOUNT% renamed, %FSKIP% already lowercase >> "%LOGFILE%"

REM === STEP 2: Rename DIRECTORIES deepest first (skip .git) ===
REM FIX v2: list the PARENT dir and pipe through findstr to match
REM the specific directory name. v1 bug: dir /ad "parent\name"
REM listed children INSIDE name, not name itself.
echo === Step 2: Renaming directories ===
echo === Step 2: Renaming directories === >> "%LOGFILE%"
set DCOUNT=0
set DSKIP=0

REM 5 passes to catch nested dirs after parent renames
for /L %%P in (1,1,5) do (
    echo DEBUG: dir pass %%P >> "%LOGFILE%"
    for /d /r "%ROOT%" %%D in (*) do (
        echo %%D | find /i ".git" >nul
        if errorlevel 1 (
            set "ORIGNAME=%%~nxD"
            set "PARENT=%%~dpD"
            for /f "tokens=*" %%L in ('dir /l /b /ad "!PARENT!" 2^>nul ^| findstr /i /x "!ORIGNAME!"') do (
                if not "!ORIGNAME!"=="%%L" (
                    echo DEBUG: ren dir "!ORIGNAME!" -^> "%%L" >> "%LOGFILE%"
                    ren "%%D" "%%L" >nul 2>nul
                    if not errorlevel 1 (
                        echo OK   dir  !ORIGNAME! -^> %%L >> "%LOGFILE%"
                        set /a DCOUNT+=1
                    ) else (
                        echo FAIL dir  !ORIGNAME! -^> %%L >> "%LOGFILE%"
                    )
                ) else (
                    set /a DSKIP+=1
                )
            )
        )
    )
)

echo Dirs: %DCOUNT% renamed, %DSKIP% already lowercase
echo Dirs: %DCOUNT% renamed, %DSKIP% already lowercase >> "%LOGFILE%"
echo === lowercase-repo.bat %VER% done %DATE% %TIME% === >> "%LOGFILE%"
echo.
echo Done (%VER%). Check %LOGFILE% for details.
pause
