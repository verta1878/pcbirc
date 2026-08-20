@Echo off
Cls

Rem *** START1.BAT batch file for node 1 ***

Set WCNODEID=1

:Start

@Echo off
Cls

Echo Loading QFront/WC, please wait...

C:
CD \QFRONT

QFront /CC:\QFRONT\NODE1

C:
CD \WILDCAT

Rem *** NOTES ON LOADING WILDCAT! ***

Rem When a human caller comes in or the sysop wants to log on
Rem locally, QFront creates a BBS-xxxx.BAT file with information
Rem such as baud rate, etc. in it.  The BBS-xxxx.BAT file simply
Rem loads the SPAWNBBS.BAT file which then loads Wildcat!
Rem See the SPAWNBBS.BAT file for more information.

if exist C:\WILDCAT\BBS-1.BAT goto LoadBBS
if errorlevel 20 goto makewild
if errorlevel 19 goto restart
if errorlevel 18 goto restart
if errorlevel 17 goto restart
if errorlevel 16 goto restart
if errorlevel 15 goto restart
if errorlevel 14 goto restart
if errorlevel 13 goto restart
if errorlevel 12 goto restart
if errorlevel 11 goto restart
if errorlevel 10 goto restart
if errorlevel 9 goto faxcall
if errorlevel 8 goto tossfido
if errorlevel 7 goto restart
if errorlevel 6 goto restart
if errorlevel 5 goto restart
if errorlevel 4 goto restart
if errorlevel 3 goto restart
if errorlevel 2 goto runevent
if errorlevel 1 goto error
if errorlevel 0 goto end

:LoadBBS
C:
CD \WILDCAT
BBS-1.BAT

:Makewild
C:
CD \WILDCAT
MAKEWILD
Goto Start

:RunEvent
QEVT-1.BAT
Goto Start

:TossFido
C:
CD \QFRONT
QScan /CC:\QFRONT\NODE1 /BOTH
Goto Start

:FaxCall
C:
CD \BGFAX
BGFAX /FCO C:\BGFAX 1 Q
Goto Start

:Restart
Goto Start

:Error
Goto Start

:End
