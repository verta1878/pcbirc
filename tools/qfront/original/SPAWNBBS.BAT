@Echo off

Rem *** SPAWNBBS.BAT batch file for all nodes ***

:Top
C:
CD \WILDCAT

If Exist C:\WILDCAT\WCWORK\NODE%WCNODEID%\DOOR.BAT Goto ReturnDoor
If %1==LOCAL Goto LoadLocal
WILDCAT /B %1 %2

:Test
If Exist C:\WILDCAT\WCWORK\NODE%WCNODEID%\DOOR.BAT Goto RunDoor
If Errorlevel 1 goto Top
Goto End

:LoadLocal
WILDCAT /S
Goto Test

:RunDoor
C:
CD \WILDCAT\WCWORK\NODE%WCNODEID%
DOOR.BAT

:ReturnDoor
C:
CD \WILDCAT
WILDCAT
Goto Test

:End
C:
CD \WILDCAT
START%WCNODEID%
