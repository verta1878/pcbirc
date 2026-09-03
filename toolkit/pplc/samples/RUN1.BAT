@ECHO OFF
CLS
ECHO   This batch file will run a PPE from the DOS command line.
ECHO   You must enter your first and last name along with your password
ECHO   by replacing the text in the appropriate fields below.
ECHO   You may need to replace the PATH to your pcboard.exe as well.
ECHO   Example
ECHO   RUN1 myppe
ECHO   Notice that no extension is neccisary
ECHO   c:\pcb\pcboard /runppe;YOUR_FIRST_NAME_HERE;YOUR_LAST_NAME_HERE;PWRD:YOUR_PASSWORD_HERE;PPE:%1.ppe
ECHO   Remember that the /RUNPPE switch can be used when user information
ECHO   is needed from within the PPE.
ECHO   You may need to combine the /RUNPPE with other commandline switches
ECHO   such as /localon.
pause

c:\pcb\pcboard /RUNPPE;FIRST_NAME;LAST_NAME;PWRD:PASSWORD;PPE:%1.ppe
