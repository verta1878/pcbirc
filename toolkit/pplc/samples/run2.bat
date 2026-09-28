@ECHO OFF
CLS
ECHO   This batch file will run a PPE from the DOS command line.
ECHO   You may need to replace the PATH to your pcboard.exe as well.
ECHO   Example
ECHO   RUN2 myppe
ECHO   Notice that no extension is neccisary
ECHO   c:\pcb\pcboard /PPE;%1.ppe
ECHO   The /PPE switch can be used in events or as an install PPE. Remember
ECHO   that *no* user information is initialized when using the /PPE switch.
ECHO   IF you need user information in your PPE use the /RUNPPE switch
pause

c:\pcb\pcboard /PPE:%1.ppe
