/*ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ INSTALL.CMD - a REXX script to install sio2k and associated files      บ
  บ Author:  Ray Gwinn 09/01/01                                            บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Several routines in this script were taken from REXX Tips & Tricks by  บ
  บ Bernd Schemmer, but I cannot remember where I downloaded it from.      บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ The script RGCUBE.CMD (used by this script) is a hack of CUBE.CMD      บ
  บ which I found on LEO at http://archiv.leo.org/pub/comp/os/os2/leo/rexx/บ
  บ I modified CUBE.CMD so that it would not comment out a line that was   บ
  บ already a comment and renamed it to RGCUBE.CMD to avoid confusion.     บ
  บ                                                                        บ
  บ The author of CUBE.CMD is not identified well, but may be Didier LAFON.บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Feel free to use any part(s) of this script in your own projects.      บ
  บ However, please give credit to ALL those mentioned above for their     บ
  บ contribution(s) to your project.                                       บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/

'@echo off'

signal on halt name UserAbort

call RxFuncAdd "SysLoadFuncs", "RexxUtil", "SysLoadFuncs"
call SysLoadFuncs

say ""

debug = 0

InstallOkay()
CopyFiles()
call FixConfigSys
say "    Installation Done"

exit

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Routine to create a list of files to copy to the sio2k directory      บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
Manifest: procedure

	files = ''
	files = files || 'logger.exe '
	files = files || 'modes.exe '
	files = files || 'pci.exe '
	files = files || 'vmodem.exe '
	files = files || 'pmlm.exe '
	files = files || 'viewpmlm.exe '
	files = files || 'help.me '
	files = files || 'read.me '
	files = files || 'esp.sys '
	files = files || 'sio2k.sys '
	files = files || 'uart.sys '
	files = files || 'vmodem.sys '
	files = files || 'vsio2k.sys '
	files = files || 'vx00.sys '
	files = files || 'sample.cfg '
	files = files || 'vmodem.ico '
	files = files || 'pci.inc '
	files = files || 'design.txt '
	files = files || 'faq.txt '
	files = files || 'history.txt '
	files = files || 'install.txt '
	files = files || 'logger.txt '
	files = files || 'modes.txt '
	files = files || 'pmlm.txt '
	files = files || 'techtalk.txt '
	files = files || 'vmodem.txt '

	return files

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Routine to build a list of files required to do an install            บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
RequiredFiles: procedure

	files = ''
	files = files || 'rgcube.cmd '
	files = files || 'sio2k.sys '
	files = files || 'uart.sys '
	files = files || 'vsio2k.sys '
	files = files || 'modes.exe '
	files = files || 'pci.inc '

	return files

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Routine to insure we can do an install                                บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
InstallOkay: procedure

	files = RequiredFiles()

	missing = (files = '')
	if files \= '' then do until files = ''
		parse var files file files

		if FileExist(file) = 0 then do
			say "    The file " || file || " is missing"
			missing = 1
		end
	end

	if missing then do
		say ''
		say '    Note: You MUST change to the disk and directory containing the'
		say '    sio2k distribution files and execute Install.cmd from there.'
		say ''
		say '    Installation was NOT completed'

		exit
	end

	return ''

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บSimple procedure to concatenate strings togather to form a comment     บ
  บcommand line for RgCUBE.CMD and to execute the command                 บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
CommentCommand: procedure
 parse arg string,FileName

	command = 'call RgCUBE {commentline "' || string || '" with'
	command = command || '"rem" (*ID} ' || FileName
	command = command || '>nul'
	command

	return ''

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บSimple procedure to concatenate strings togather to form an ADDLINE    บ
  บcommand line for RgCUBE.CMD and to execute the command                 บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
AddLineCommand: procedure
 parse arg line,driver,FileName

	command = 'call RgCUBE {AddLine "' || line || '" (*ID IFNOT "\sio2k\'
	command = command || driver || '"} '
	command = command || FileName || '>nul'

	command
	return ''

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บFix up config.sys to use sio2k                                         บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
FixConfigSys: procedure expose debug bootdrive

	if (debug) then BootDrive = '';

	say "    Commenting out com.sys and sio.sys if they exist in config.sys"

	ConfigName = bootdrive || '\config.sys'
	BackupName = bootdrive || '\config.s2k'

	SaveCommand = 'copy ' || ConfigName || ' ' || BackupName || ' /v >nul'
	SaveCommand

	CommentCommand('COM.SYS',ConfigName)
	CommentCommand('VCOM.SYS',ConfigName)
	CommentCommand('SIO.SYS',ConfigName)
	CommentCommand('VSIO.SYS',ConfigName)

	say "    Adding commands to config.sys to load the sio2k drivers"

	line = 'rem device=' || bootdrive || '\sio2k\esp.sys logfile='
	line = line || bootdrive || '\sio2k\sio2k.log NoPause'
	AddLineCommand(line, 'esp.sys',ConfigName)

	line = 'rem device=' || bootdrive || '\sio2k\vmodem.sys logfile='
	line = line || bootdrive || '\sio2k\sio2k.log NoPause nPorts=2'
	AddLineCommand(line, 'vmodem.sys',ConfigName)

	line = 'device=' || bootdrive || '\sio2k\uart.sys logfile='
	line = line || bootdrive || '\sio2k\sio2k.log'
	AddLineCommand(line, 'uart.sys',ConfigName)

	line = 'device=' || bootdrive || '\sio2k\sio2k.sys logfile='
	line = line || bootdrive || '\sio2k\sio2k.log'
	AddLineCommand(line, 'sio2k.sys',ConfigName)

	line = 'device=' || bootdrive || '\sio2k\vsio2k.sys logfile='
	line = line || bootdrive || '\sio2k\vsio2k.log vIrqList(3,4)'
	AddLineCommand(line, 'vsio2k.sys',ConfigName)

	line = 'rem device=' || bootdrive || '\sio2k\vx00.sys'
	AddLineCommand(line, 'vx00.sys',ConfigName)

	return ''

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บCopy the distribution files to the sio2k home directory                บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
CopyFiles: procedure expose debug bootdrive

	Destination = GetBootDrive() || '\sio2k'

	if DirExist( Destination ) = "" then do
		say "    Creating directory " || Destination
		'md ' || Destination || '>nul'
	end

	say "    Copying files to " || Destination

	files = Manifest()
	if files \= '' then do until files = ''
		parse var files file files

		if FileExist(file) \= 0 then 'copy' file Destination '/v > nul'
	end
return ''

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Routine to get the boot (installiation) drive                         บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
GetBootDrive: procedure expose BootDrive debug

	/* install a local error handler */
	signal on syntax name BootDriveNotFound
	BootDrive = SysBootDrive()

BootDriveNotFound:
  if symbol( 'BootDrive' ) <> 'VAR' then do
		BootDrive = SysSearchPath('PATH', 'OS2.INI')
		if BootDrive = '' then
			BootDrive = substr( value('RUNWORKPLACE',,'OS2ENVIRONMENT'),1,2)

		if BootDrive = '' then
			BootDrive = substr( value( 'COMSPEC' ,, 'OS2ENVIRONMENT' ), 1,2 )

		if BootDrive = '' then
      	BootDrive = 'C'
	end

	BootLetter = left(BootDrive, 1)
	BootDrive = BootLetter || ':'
	if (debug) then BootDrive = '.';

	return BootDrive

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Get typed input from the user                                         บ
  บ                                                                       บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
Input: procedure
	parse arg prompt
 
	call CharOut, "1b"x || "[7l"
	call charOut, prompt || " "
	result = lineIn()
	call CharOut, "1b"x || "[7h"

	return result



/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Taken from the install.cmd script of REXX tips and tricks             บ
  บ                                                                       บ
  บ Error handler (called if the DLL REXXUTIL is not found)               บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
RexxUtilNotFound:
	say ''
	say 'Error: REXXUTIL.DLL not found!'
exit 255

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Taken from the install.cmd script of REXX tips and tricks             บ
  บ                                                                       บ
  บ Error handler (called if the user presses CTRL-BREAK)                 บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
UserAbort:
	say ''
	say ' Installation aborted by the user.'
exit 254

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ function: Check if a directory exists                                 บ
  บ                                                                       บ
  บ call:     DirExist( testDir )                                         บ
  บ                                                                       บ
  บ where:    testDir - name of the directory to test                     บ
  บ                                                                       บ
  บ returns:  full name of the directory or "" if the directory           บ
  บ           don't exist                                                 บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
DirExist: PROCEDURE
	parse arg testDir .

	thisRC = ""
	testDir = strip( testDir )
	if testDir = "" then signal DirDoesNotExist

	if right( testDir, 1 ) = '\' then testDir = dbrright( testDir,1 )
	testDir = testDir || '\.'

	SIGNAL ON NOTREADY NAME DirDoesNotExist

	call stream testDir || "\*", "D"
	curDir = directory()
	curDir1 = directory( fileSpec( "drive", testDir ) )
	thisRC = directory( testDir )
	call directory curDir1
	call directory curDir

	DirDoesNotExist:
	return thisRC

/*ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ Returns true if the passed file exists, otherwise false is returned   บ
  ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ*/
FileExist: procedure
	parse arg fileName
 
	if stream( fileName, "c", "QUERY EXISTS" ) <> "" then return 1
	return 0

