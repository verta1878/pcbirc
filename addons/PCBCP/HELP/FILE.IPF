.*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
.* The source code in this module is proprietary software belonging to       */
.* Clark Development Company and is part of the PCBoard source code library. */
.* You are granted the right to use this source code for the building of any */
.* of the PCBoard products you have licensed.  Any other usage is forbidden  */
.* without prior written consent from Clark Development Company, Inc.        */
.*                                                                           */
.* Be sure to read the source code license agreement before utilizing any    */
.* of the source code found herein.                                          */
.*                                                                           */
.* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
.*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


.*==============================================================*\
.*                                                              *
.* File.ipf - Information Tag Language file for the File menu   *
.*            help panels.                                      *
.*                                                              *
.*==============================================================*/

.*--------------------------------------------------------------*\
.*  Main file menu                                              *
.*      res = PANEL_FILE                                        *
.*--------------------------------------------------------------*/
:h1 res=2210 name=PANEL_FILE.File Menu
:i1 id=File.File Menu
:p.The File menu contains a command for opening the :hp2.USERNET:ehp2. file,
and also for setting up the :hp2.Node Limits:ehp2. and :hp2.Node Settings:ehp2..
The following commands appear in the File menu:
:parml tsize=25 break=none.
:pt.:hp2.:link reftype=hd res=2220.Open Usernet:elink.:ehp2.
:pd.Opens the :hp2.USERNET:ehp2. file
:pt.:hp2.:link reftype=hd res=2230.Node Limits:elink.:ehp2.
:pd.Sets the First and Last Node Numbers
:pt.:hp2.:link reftype=hd res=2240.Node Settings:elink.:ehp2.
:pd.Sets Node Startup Information
:pt.:hp2.:link reftype=hd res=2250.Exit:elink.:ehp2.
:pd.Ends the application
:eparml.

.*--------------------------------------------------------------*\
.*  File menu Open Usernet command help panel                   *
.*      res = PANEL_FILEOPEN                                    *
.*--------------------------------------------------------------*/
:h1 res=2220 name=PANEL_FILEOPEN.Open Usernet
:i2 refid=File.Open Usernet
:p.Before the :hp1.PCBoard Control Panel:ehp1. can set up any nodes, or
monitor any activity, you first have to open the :hp2.USERNET:ehp2. file.
:hp1.PCBCP:ehp1. reads this file to determine how many nodes you have when
setting up nodes, and this is also the file it reads when monitoring
system activity.  To open the file manually:
:ul.
:li.Select the :hp2.File | Open Usernet:ehp2. menu option.
:li.Then either type in the name of the file to open, or use the dialog boxes
to select the drive/directory and filename to open.
:eul.
:p.To start :hp1.PCBCP:ehp1. so that it automatically opens the usernet file,
pass the name of the usernet file on the command line.  Example:
:xmp.
PCBCP C&colon.\PCB\MAIN\USERNET.XXX
:exmp.
:note.You can also specify a different INI file for PCBCP to use by using the
/INI= command line parameter.  Example:
:xmp.
PCBCP C&colon.\PCB\MAIN\USERNET.XXX /INI=C&colon.\PCB\PCBCP.INI
:exmp.

.*--------------------------------------------------------------*\
.*  File menu Node Limits command help panel                    *
.*      res = PANEL_FILELIMIT                                   *
.*--------------------------------------------------------------*/
:h1 res=2230 name=PANEL_FILELIMIT.Node Limits
:i2 refid=File.Node Limits
:p.By default, the :hp1.PCBoard Control Panel:ehp1. sets the node limits
to cover the entire range of possible nodes on your system.  For example,
if you are using the 100-node version of :hp1.PCBoard:ehp1., the first node will be 1
and the last node will be 100.  You can increase the :hp1.Control Panel's:ehp1.
efficiency by reducing the range to just those nodes that you will be
configuring or monitoring.  To do this,
:ol.
:li.Select the :hp7.File:ehp7. menu and choose the :hp7.Node Limits:ehp7. command
:li.Then use the spin buttons to set the First and Last nodes to monitor/configure
:li.Then select the :hp7.Options:ehp7. menu and choose the :hp7.Save Settings:ehp7. command to save
:eol.

.*--------------------------------------------------------------*\
.*  File menu Node Limits dialog help panel                     *
.*      res = HID_NODELIMIT_DLG_HELP_PANEL                      *
.*--------------------------------------------------------------*/
:h1 res=4130 name=HID_NODELIMIT_DLG_HELP_PANEL.Node Limits
:i2 refid=File.Node Limits Dialog
:p.By default, the :hp1.PCBoard Control Panel:ehp1. sets the node limits
to cover the entire range of possible nodes on your system.  For example,
if you are using the 100-node version of :hp1.PCBoard:ehp1., the first node will be 1
and the last node will be 100.  You can increase the :hp1.Control Panel's:ehp1.
efficiency by reducing the range to just those nodes that you will be
configuring or monitoring.
:p.The effects of these changes are two-fold:
:ol.
:li.The :hp2.File | Node Settings:ehp2. dialog will load faster if it only has
to load the nodes you normal configure and control.
:li.When PCBCP is scanning the usernet file for updates, reducing the number of
nodes to scan will reduce the amount of information to read from disk, and will
also reduce the number of cpu cycles spent processing the information.
:eol.
:p.Another reason to change these limits is that, on a machine by machine
basis, you may only want PCBCP to see and control the nodes that will be run
on that machine.  For example, if you run 16 nodes per machine, and have 3
separate machines, then you may want to set the node limits to 1-16 for the
first machine, 17-32 for the second, and 33-48 for the third.  In this manner,
each machine runs PCBCP to monitor only those nodes which run on it.
:p.Finally, if you run your own personal workstation to monitor the entire
system, you may then want to set your node limits to 1-48 so that you can
monitor all of the nodes on all 3 machines.

.*--------------------------------------------------------------*\
.*  File menu Node Settings dialog help panel                   *
.*      res = HID_NOTEBOOK_NODE_DLG_HELP_PANEL                  *
.*--------------------------------------------------------------*/
:h1 res=4140 name=HID_NOTEBOOK_NODE_DLG_HELP_PANEL.Node Settings
:i2 refid=File.Node Settings Dialog
:link reftype=hd res=4148 vpx=left vpy=top vpcx=100% vpcy=50% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=4149 vpx=left vpy=bottom vpcx=100% vpcy=50%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=4148 nosearch noprint.Node Settings General Help
:p.The :hp2.Node Settings:ehp2. are used to configure each individual node.
You define the path and name of the program to execute, and the directory in
which it should run, as well as any optional parameters that you may want to
pass on the command line.
:p.For specific help, select a topic below.

:h1 hide res=4149 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=4141.Path and Filename to Execute:elink.
:li.:link reftype=hd res=4142.Working Directory:elink.
:li.:link reftype=hd res=4143.Optional Parameters:elink.
:li.:link reftype=hd res=4144.Optional Port Number:elink.
:li.:link reftype=hd res=4150.Run Settings:elink.
:li.:link reftype=hd res=4160.Copy From:elink.
:li.:link reftype=hd res=4170.Copy To:elink.
:esl.

:h1 hide res=4141 nosearch noprint.Path and Filename to Execute
:p.Enter a fully qualified path and the name of the file to execute.  This
could be :hp2.BOARD.CMD:ehp2. or you may wish to use a single shared command
file such as :hp2.STARTOS2.CMD:ehp2..  Examples:
:dl compact tsize=30.
:dt.:hp2.C&colon.\PCB\1\BOARD.CMD:ehp2.
:dd.Node-specific .CMD file
:dt.:hp2.C&colon.\PCB\STARTOS2.CMD:ehp2.
:dd.Shared .CMD file
:edl.
:p.Press here for a :link reftype=hd res=4147.Complete Example:elink. of the
:hp2.Node Settings:ehp2. and :hp2.Run Settings screens:ehp2..
:p.Press here for :link reftype=hd res=3020.More Information:elink. about
sharing a single startup command file.

:h1 hide res=4142 nosearch noprint.Working Directory
:p.Enter the drive and directory which you want to be made the :hp1.current
directory:ehp1. at the time that the program you specified in
:link reftype=hd res=4141.Path and Filename to Execute:elink. is started.
Examples:
:sl compact.
:li.:hp2.C&colon.\PCB:ehp2.
:li.:hp2.C&colon.\PCB\NODE1:ehp2.
:esl.
:P.PRESS HERE FOR A :LINK REFTYPE=HD RES=4147.COMPLETE EXAMPLE:ELINK. OF THE
:HP2.NODE SETTINGS:EHP2. AND :HP2.RUN SETTINGS SCREENS:EHP2..

:h1 hide res=4143 nosearch noprint.Optional Parameters
:p.Any optional parameters you put in this field are passed on the command line
to the program you specified in :link reftype=hd res=4141.Path and Filename to Execute:elink..
:p.These parameters would be for options you want to define yourself.  There
are no specific optional parameters that PCBoard or the Control Panel are
expecting you to use.

:h1 hide res=4144 nosearch noprint.Optional Port Number
:p.If you set the :hp2.Optional Port Number:ehp2. to a non-zero value, the
value you select will be passed via an environment variable called :hp2.PCBPORT=:ehp2..
:p.You would then pass that value to PCBoard by setting up the PCB= environment
variable using a line such as this:
:xmp.SET PCB=/PORT%PCBPORT% /OS:exmp.
:p.PCBCP also passes the node number via the PCBNODE= environment variable.  So
a more complete example would include the node number as well in a line such as this:
:xmp.SET PCB=/PORT%PCBPORT% /OS /NODE&colon.%PCBNODE%:exmp.
:p.Press here for a :link reftype=hd res=4147.Complete Example:elink. of the
:hp2.Node Settings:ehp2. and :hp2.Run Settings screens:ehp2., including samples
of the STARTOS2.CMD and BOARD.CMD files.

:h1 hide res=4147 nosearch noprint.Complete Example
:p.The screen shots below serve to illustrate a complete example of a node
setup when using a single shared startup command file and a shared pcboard.dat
configuration file.
:p.First the :hp2.Node Settings:ehp2. screen.  This screen shows a single STARTOS2.CMD
file being defined and that the node will start in the C&colon.\PCB directory.
It also shows that the node will be using COM2.
:artwork name='nodeset.bmp'.
:p.Now the :hp2.Run Settings:ehp2. screen.  This screen shows that the node
will be initially launched in a minimized state, and that it will be launched
as soon as PCBCP is started.  (assuming you have :link reftype=hd res=2662.Options | Run on Startup:elink. enabled).
:artwork name='runset.bmp'.
:p.The following illustrates what you might put into the STARTOS2.CMD file:
:table cols='78'.:row.:c.:lines.
@echo off
rem Set any common variables here.
rem %PCBNODE% and %PCBPORT% are set by the Control Panel
set dszlog=pcbdsz.log
set node=%PCBNODE%
set port=%PCBPORT%

rem Set the PCB= environment variable
set pcb=/NODE&colon.%node% /PORT%port% /OS

rem Create the PCBDAT= variable for BOARD.CMD to use
set pcbdat=c&colon.\pcb\pcboard.dat

rem Customize the prompt to show which node we're on
prompt [NODE %node% - OS/2] $p$g

rem change to the node directory and delete the pcboard.sys file so that if
rem the system is being started up after a reboot it will not assume that a
rem caller is already online
rem NOTE:  This assumes the node directory is C&colon.\PCB\##.  If you use
rem        C&colon.\PCB\NODE## then change it to use cd\pcb\node%node%
rem NOTE:  If you set the alarm and page bell options, then REMOVE the
rem        line that deletes the pcboard.sys file.
c:
cd \pcb\%node%
del pcboard.sys

rem Finally, transfer control to the BOARD.CMD file
board.cmd
:elines.:etable.
:p.The following illustrates what you might put into the BOARD.CMD file:
:table cols='50'.:row.:c.:lines.
@echo off
&colon.top
c:
cd\pcb\%PCBNODE%
if exist remote.cmd REN remote.cmd remote.sys
if exist endpcb     DEL endpcb
pcboard2 /file&colon.%pcbdat%
if exist remote.cmd CALL remote.cmd
if exist event.cmd  CALL event.cmd
if exist event.bat  CALL event.bat
if NOT exist endpcb GOTO top
:elines.:etable.

:note.In this example, you'll want to put :hp2.C&colon.\PCB:ehp2. into the
PATH= settings of both your OS/2 environment (set in config.sys) as well as
your DOS environment (set in autoexec.bat).

.*--------------------------------------------------------------*\
.*  File menu Node Settings dialog help panel                   *
.*      res = HID_NOTEBOOK_RUN_DLG_HELP_PANEL                   *
.*--------------------------------------------------------------*/
:h1 res=4150 name=HID_NOTEBOOK_RUN_DLG_HELP_PANEL.Run Settings
:i2 refid=File.Run Settings Dialog
:link reftype=hd res=4158 vpx=left vpy=top vpcx=100% vpcy=66% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=4159 vpx=left vpy=bottom vpcx=100% vpcy=34%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=4158 nosearch noprint.Run Settings General Help
:p.The :hp2.Node Settings:ehp2. are used to configure each individual node.
You define the path and name of the program to execute, and the directory in
which it should run, as well as any optional parameters that you may want to
pass on the command line.
:p.For specific help, select a topic below.

:h1 hide res=4159 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=4151.Session Type:elink.
:li.:link reftype=hd res=4152.Auto-Run Settings:elink.
:li.:link reftype=hd res=4140.Node Settings:elink.
:li.:link reftype=hd res=4160.Copy From:elink.
:li.:link reftype=hd res=4170.Copy To:elink.
:esl.

:h1 hide res=4151 nosearch noprint.Session Type
:p.Select one of the following Session Types in which to run the program you
specified in the :link reftype=hd res=4141.Path and Filename to Execute:elink.
field:
:dl compact tsize=30.
:dt.:hp2.Windowed:ehp2.
:dd.The program starts inside of a visible window.
:dt.:hp2.Windowed (Minimized):ehp2.
:dd.The program starts in a windowed session, but is immediately minimized.
:dt.:hp2.Full Screen:ehp2.
:dd.The program starts in a full-screen session, but is started in the background.
:edl.

:h1 hide res=4152 nosearch noprint.Auto-Run Settings
:p.PCBCP can be set to automatically launch your nodes, either on startup or,
if you are using VMODEM, at the time of an incoming call.  Select one of the
following automatic startup types:
:dl compact tsize=25.
:dt.:hp2.Disabled:ehp2.
:dd.Disable automatic running of this particular node.
:dt.:hp2.Run at Startup:ehp2.
:dd.Automatically launch this node when PCBCP is started.
:dt.:hp2.Run upon Connection:ehp2.
:dd.Automatically launch this node when a call comes in.
:edl.
:note.These settings are specific to the node which is being configured.  There
is also a :hp1.master-switch:ehp1. in the :hp2.Options | Startup:ehp2. screen.
Select one of the items below for further details:
:sl compact.
:li.:link reftype=hd res=2662.Enable/Disable Auto-Run on Startup:elink.
:li.:link reftype=hd res=2663.Enable/Disable Auto-Run on Connect:elink.
:esl.


.*--------------------------------------------------------------*\
.*  File menu Node Settings dialog help panel                   *
.*      res = HID_NOTEBOOK_COPYFROM_HELP_PANEL                  *
.*--------------------------------------------------------------*/
:h1 res=4160 name=HID_NOTEBOOK_COPYFROM_HELP_PANEL.Copy From
:i2 refid=File.Copy From Dialog
:p.The :hp2.Copy From:ehp2. option lets you change the current node settings
to match those of a node you have previous configured.
:p.For example, if you have configured node 1, and you are now setting up node
2, you could select Copy From and then select Node 1 and then after the
operation is completed, you could make any changes to the configuration which
may be necessary (such as setting a different comm port number).
:p.See also: :link reftype=hd res=4170.Copy To:elink.


.*--------------------------------------------------------------*\
.*  File menu Node Settings dialog help panel                   *
.*      res = HID_NOTEBOOK_COPYTO_HELP_PANEL                    *
.*--------------------------------------------------------------*/
:h1 res=4170 name=HID_NOTEBOOK_COPYTO_HELP_PANEL.Copy To
:i2 refid=File.Copy To Dialog
:p.The :hp2.Copy To:ehp2. option is the fastest method of setting up a large
number of nodes.  Essentially, you would set one node up, and then copy those
settings to a whole range of nodes.
:p.For example, if you have already configured node 1, and you are now setting
up nodes 2-10, you would go to the node 1 notebook page and select Copy To.
Then select Node 2 as the :hp2.First Node Number:ehp2. and Node 10 as the
:hp2.Last Node Number:ehp2.
:p.After the operation is completed, you may want to make any changes to the
configuration which may be necessary to finish the configuration of the
individual nodes (such as setting a different comm port number).
:p.See also: :link reftype=hd res=4160.Copy From:elink.


.*--------------------------------------------------------------*\
.*  File menu Node Settings command help panel                  *
.*      res = PANEL_FILENODE                                    *
.*--------------------------------------------------------------*/
:h1 res=2240 name=PANEL_FILENODE.Node Settings
:i2 refid=File.Node Settings
:p.The :hp1.PCBoard Control Panel:ehp1. can be used to load, run and
control your PCBoard nodes.  The :hp2.File | Node Settings:ehp2. dialog lets you define
the various node parameters for starting the node, such as the name of the
program to run, any optional parameters, and also startup settings such as
windowed, full screen, etc.
:p.For more information, see :link reftype=hd res=4140.Node Settings Dialog:elink..

.*--------------------------------------------------------------*\
.*  File menu Exit command help panel                           *
.*      res = PANEL_FILEEXIT                                    *
.*--------------------------------------------------------------*/
:h1 res=2250 name=PANEL_FILEEXIT.Exit
:i2 refid=File.Exit
:p.You can exit the :hp1.PCBoard Control Panel:ehp1. by doing the following:
:ul.
:li.Select the :hp7.File:ehp7. menu and choose the :hp7.Exit:ehp7. command.
:eul.
:p.Other methods for exiting include the following:
:ul.
:li.Press either :hp7.Alt-X:ehp7. or :hp7.Alt-F4:ehp7.
:li.Select the :hp7.System Menu:ehp7. by clicking on the top left corner, then select the :hp7.Close:ehp7. command
:li.:hp1.Double click:ehp1. on the :hp7.System Menu:ehp7. at the top left corner
:eul.
:p.:hp8.NOTE: If the Control Panel has any PCBoard Windows open, it will close
them on exit.  Prior to closing, the Control Panel will ask you if you want
to proceed.:ehp8.

.***********************************************************************
.*Help for Open Dialog box   (D)
:h1 hide res=4110 scroll=none titlebar=yes group=1 id=A9003.Help for Open
.***********************************************************************
:link reftype=hd res=20500 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=20600 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=20500 nosearch noprint.Help for Open
:ol compact.
:li.Select the down arrow to the right of the :hp2.Drive:ehp2. list to display
all the drives on your system.
:li.Select a drive from the :hp2.Drive:ehp2. list.
:li.Select a directory from the :hp2.Directory:ehp2. list.
:li.Select a file name from the :hp2.File:ehp2. list or type in a file name
and select the :hp2.Open:ehp2. pushbutton to display
the file you want to edit.
:eol.
:p.For specific help, select a topic below.

:h1 hide res=20600 nosearch noprint.List of Fields
:dl compact tsize=20.
:dt.:link reftype=hd res=20501 group=20.File name:elink.
:dd.:link reftype=hd res=20502 group=21.Type of file:elink.
:dt.:link reftype=hd res=20503 group=22.Drive:elink.
:dd.:link reftype=hd res=20504 group=23.File:elink.
:dt.:link reftype=hd res=20505 group=24.Directory:elink.
:dd.:link reftype=hd res=20506 group=25.Open pushbutton:elink.
:edl.
.***********************************************************************

.*Help for File name  (F)

:h1 hide res=20501 scroll=none titlebar=yes group=26 nosearch noprint.Help for File name
:link reftype=hd res=20550 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=20650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=20550 nosearch.Help for File name
:p.Type the name of the file you want to open in the
:hp2.File name:ehp2. field
and select the :hp2.Open:ehp2. pushbutton.
:p.For more help, select a topic below.

:h1 hide res=20650 nosearch noprint.List of Fields
:dl compact tsize=20.
:dt.:link reftype=hd res=20501 group=27.File name:elink.
:dd.:link reftype=hd res=20502 group=28.Type of file:elink.
:dt.:link reftype=hd res=20503 group=29.Drive:elink.
:dd.:link reftype=hd res=20504 group=30.File:elink.
:dt.:link reftype=hd res=20505 group=31.Directory:elink.
:dd.:link reftype=hd res=20506 group=32.Open pushbutton:elink.
:dt.
:dd.:link reftype=hd res=4110  group=33.General help:elink.
:edl.
.************************************************************************

.*Help for Type of file  (F)

:h1 hide res=20502 scroll=none titlebar=yes group=38 nosearch noprint.Help for Type of file
:link reftype=hd res=20552 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=20650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=20552 group=39 nosearch.Help for Type of file
:p.Select the down arrow to the right of the :hp2.Type of file:ehp2. field to
display the available file types.  The sample has set this field for
all file types.
:p.For more help, select a topic below.
.***********************************************************************

.*Help for Drive  (F)

:h1 hide res=20503 scroll=none titlebar=yes group=11 nosearch noprint.Help for Drive
:link reftype=hd res=20553 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=20650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=20553 group=40 nosearch.Help for Drive
:p.The :hp2.Drive:ehp2. list displays the drives on your system.  Select the
drive that contains the files you want to edit.
:p.For more help, select a topic below.
.***************************************************************************

.*Help for File  (F)

:h1 hide res=20504 scroll=none titlebar=yes group=11 nosearch noprint.Help for File
:link reftype=hd res=20554 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=20650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=20554 group=42 nosearch.Help for File
:p.The :hp2.File:ehp2. list displays all the files in the directory
you selected from the :hp2.Directory:ehp2. list.  Select the file you want
to open.
:p.For more help, select a topic below.
.***************************************************************************

.*Help for Directory  (F)

:h1 hide res=20505 scroll=none titlebar=yes group=11 nosearch noprint.Help for Directory
:link reftype=hd res=20555 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=20650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=20555 group=43 nosearch.Help for Directory
:p.The :hp2.Directory:ehp2. list displays the directories on the selected
drive.  Select a directory to display the list of files from that directory
in the :hp2.File:ehp2. list box.
:p.For more help, select a topic below.
.*************************************************************************

.*Help for Open pushbutton  (PU)

:h1 hide res=20506 scroll=none titlebar=yes group=17 nosearch noprint.Help for Open
:link reftype=hd res=20556 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=20650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=20556 group=44 nosearch.Help for Open
:p.Select the :hp2.Open:ehp2. pushbutton to display
the file you want to edit.
:p.For more help, select a topic below.
.*************************************************************************
