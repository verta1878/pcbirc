@echo off
REM ============================================================
REM  BLDPCBCOMM - Open Watcom V2, 32-bit flat model
REM  pcbcomm serial library: 19 modules -> PCBCOMM_WF.LIB
REM  delta154 / pcb1541
REM
REM  Source: PCB1541\PCBCOMM\SRC\
REM  Headers: PCB1541\PCBCOMM\INC\ + TOOLKIT\DELTA154\H\
REM  Output: OUT\DELTA154\SDK\OW2FLAT\OBJ\ + OUT\DELTA154\SDK\OW2FLAT\
REM
REM  Requires: WATCOM environment variable set (e.g. SET WATCOM=C:\WATCOM)
REM  Built by: hexadecimal — pcbirc crew, November 2026
REM ============================================================
echo.
echo Building PCBCOMM (Open Watcom V2, 32-bit flat)
echo delta154 pcbcomm serial library - 19 modules
echo.

REM --- Check WATCOM is set ---
if "%WATCOM%"=="" goto nowatcom
set PATH=%WATCOM%\BINW;%PATH%
set INCLUDE=%WATCOM%\H

REM --- Paths ---
set SRCDIR=PCB1541\PCBCOMM\SRC
set INCDIR=PCB1541\PCBCOMM\INC
set TKDIR=TOOLKIT\DELTA154\H
set OBJDIR=OUT\DELTA154\SDK\OW2FLAT\OBJ
set OUTDIR=OUT\DELTA154\SDK\OW2FLAT

REM --- Compiler flags ---
REM  -oxs    = optimize for space + time
REM  -bt=dos = DOS target
REM  -mf     = flat 32-bit memory model
REM  -dPCBCOMM_V1 = pcbcomm v1 API
REM  -dPCB1541    = enable 15.41 extended backends
set CFLAGS=-oxs -bt=dos -mf -i%WATCOM%\H -i%INCDIR% -i%TKDIR% -dPCBCOMM_V1 -dPCB1541

REM --- Create output dirs ---
if not exist OUT md OUT
if not exist OUT\DELTA154 md OUT\DELTA154
if not exist OUT\DELTA154\SDK md OUT\DELTA154\SDK
if not exist OUT\DELTA154\SDK\OW2FLAT md OUT\DELTA154\SDK\OW2FLAT
if not exist %OBJDIR% md %OBJDIR%

echo.
echo === Compiling 19 modules ===
echo.

echo   [ 1/19] PCBCOMM (core)
wcc386 %CFLAGS% -fo=%OBJDIR%\pcbcomm.obj %SRCDIR%\pcbcomm.c
if errorlevel 1 goto fail

echo   [ 2/19] IRQ (interrupt management)
wcc386 %CFLAGS% -fo=%OBJDIR%\irq.obj %SRCDIR%\irq.c
if errorlevel 1 goto fail

echo   [ 3/19] UART (chip detection)
wcc386 %CFLAGS% -fo=%OBJDIR%\uart.obj %SRCDIR%\uart.c
if errorlevel 1 goto fail

echo   [ 4/19] UART_BACKEND (8250/16550)
wcc386 %CFLAGS% -fo=%OBJDIR%\uart_backend.obj %SRCDIR%\uart_backend.c
if errorlevel 1 goto fail

echo   [ 5/19] INT14 (BIOS INT 14h + keyboard/screen)
wcc386 %CFLAGS% -fo=%OBJDIR%\int14.obj %SRCDIR%\int14.c
if errorlevel 1 goto fail

echo   [ 6/19] FOSSIL (FOSSIL driver)
wcc386 %CFLAGS% -fo=%OBJDIR%\fossil.obj %SRCDIR%\fossil.c
if errorlevel 1 goto fail

echo   [ 7/19] COMMDRBL (ser_rs232_* SDK layer)
wcc386 %CFLAGS% -fo=%OBJDIR%\commdrbl.obj %SRCDIR%\commdrbl.c
if errorlevel 1 goto fail

echo   [ 8/19] LIBSBL (SBL linkable shim)
wcc386 %CFLAGS% -fo=%OBJDIR%\libsbl.obj %SRCDIR%\libsbl.c
if errorlevel 1 goto fail

echo   [ 9/19] CARD_POOL (per-card state)
wcc386 %CFLAGS% -fo=%OBJDIR%\card_pool.obj %SRCDIR%\card_pool.c
if errorlevel 1 goto fail

echo   [10/19] BOCA_BACKEND (Boca 1610)
wcc386 %CFLAGS% -fo=%OBJDIR%\boca_backend.obj %SRCDIR%\boca_backend.c
if errorlevel 1 goto fail

echo   [11/19] CYCLOM_BACKEND (Cyclades Cyclom-Y)
wcc386 %CFLAGS% -fo=%OBJDIR%\cyclom_backend.obj %SRCDIR%\cyclom_backend.c
if errorlevel 1 goto fail

echo   [12/19] DIGI_PCXE_BACKEND (DigiBoard PC/Xe)
wcc386 %CFLAGS% -fo=%OBJDIR%\digi_pcxe_backend.obj %SRCDIR%\digi_pcxe_backend.c
if errorlevel 1 goto fail

echo   [13/19] DIGI_ACCEL_BACKEND (DigiBoard Accel)
wcc386 %CFLAGS% -fo=%OBJDIR%\digi_accel_backend.obj %SRCDIR%\digi_accel_backend.c
if errorlevel 1 goto fail

echo   [14/19] DIGI_FEP (Digi FEP shared)
wcc386 %CFLAGS% -fo=%OBJDIR%\digi_fep.obj %SRCDIR%\digi_fep.c
if errorlevel 1 goto fail

echo   [15/19] ROCKET_BACKEND (Comtrol RocketPort)
wcc386 %CFLAGS% -fo=%OBJDIR%\rocket_backend.obj %SRCDIR%\rocket_backend.c
if errorlevel 1 goto fail

echo   [16/19] EASYIO_BACKEND (Stallion EasyIO)
wcc386 %CFLAGS% -fo=%OBJDIR%\easyio_backend.obj %SRCDIR%\easyio_backend.c
if errorlevel 1 goto fail

echo   [17/19] ARNET_BACKEND (Arnet SmartPort)
wcc386 %CFLAGS% -fo=%OBJDIR%\arnet_backend.obj %SRCDIR%\arnet_backend.c
if errorlevel 1 goto fail

echo   [18/19] HUB6_BACKEND (Intel HUB-6)
wcc386 %CFLAGS% -fo=%OBJDIR%\hub6_backend.obj %SRCDIR%\hub6_backend.c
if errorlevel 1 goto fail

echo   [19/19] GTEK_BACKEND (GTEK BBS-550/8Fx)
wcc386 %CFLAGS% -fo=%OBJDIR%\gtek_backend.obj %SRCDIR%\gtek_backend.c
if errorlevel 1 goto fail

echo.
echo === All 19 modules compiled. Building library ===
echo.

REM --- Build PCBCOMM_WF.LIB ---
if exist %OUTDIR%\PCBCOMM_WF.LIB del %OUTDIR%\PCBCOMM_WF.LIB
wlib -n %OUTDIR%\PCBCOMM_WF.LIB ^
  +%OBJDIR%\pcbcomm.obj ^
  +%OBJDIR%\irq.obj ^
  +%OBJDIR%\uart.obj ^
  +%OBJDIR%\uart_backend.obj ^
  +%OBJDIR%\int14.obj ^
  +%OBJDIR%\fossil.obj ^
  +%OBJDIR%\commdrbl.obj ^
  +%OBJDIR%\libsbl.obj ^
  +%OBJDIR%\card_pool.obj ^
  +%OBJDIR%\boca_backend.obj ^
  +%OBJDIR%\cyclom_backend.obj ^
  +%OBJDIR%\digi_pcxe_backend.obj ^
  +%OBJDIR%\digi_accel_backend.obj ^
  +%OBJDIR%\digi_fep.obj ^
  +%OBJDIR%\rocket_backend.obj ^
  +%OBJDIR%\easyio_backend.obj ^
  +%OBJDIR%\arnet_backend.obj ^
  +%OBJDIR%\hub6_backend.obj ^
  +%OBJDIR%\gtek_backend.obj
if errorlevel 1 goto fail

REM --- Copy SDK-layer OBJs alongside library ---
copy %OBJDIR%\commdrbl.obj %OUTDIR%\commdrbl.obj >nul
copy %OBJDIR%\libsbl.obj %OUTDIR%\libsbl.obj >nul
copy %OBJDIR%\fossil.obj %OUTDIR%\fossil.obj >nul

echo.
echo === PCBCOMM_WF.LIB built successfully ===
echo Output: %OUTDIR%\PCBCOMM_WF.LIB
echo.
goto end

:nowatcom
echo ERROR: WATCOM environment variable not set.
echo Set it to your Open Watcom installation directory.
echo Example: SET WATCOM=C:\WATCOM
goto end

:fail
echo.
echo *** BUILD FAILED ***
echo.

:end
