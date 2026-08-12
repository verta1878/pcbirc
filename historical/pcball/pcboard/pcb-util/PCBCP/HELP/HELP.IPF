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
.* Help.ipf - Information Tag Language file for the Help menu   *
.*              help panels.                                    *
.*                                                              *
.*==============================================================*/

.* .*--------------------------------------------------------------*\
.* .*  Main Help menu                                              *
.* .*      res = PANEL_HELP                                        *
.* .*--------------------------------------------------------------*/
.* :h1 res=2910 name=PANEL_HELP.Help Menu
.* :i1 id=Help.Help Menu
.* :p.
.*
.* .*--------------------------------------------------------------*\
.* .*  Help menu Save As command help panel                        *
.* .*      res = PANEL_HELPINDEX                                   *
.* .*--------------------------------------------------------------*/
.* :h1 res=2920 name=PANEL_HELPINDEX.Index
.* :i2 refid=Help.Index
.* :p.
.*
.* .*--------------------------------------------------------------*\
.* .*  Help menu General help help panel                           *
.* .*      res = PANEL_HELPGENERAL                                 *
.* .*--------------------------------------------------------------*/
.* :h1 res=2930 name=PANEL_HELPGENERAL.General help
.* :i2 refid=Help.General
.* :p.
.*
.* .*--------------------------------------------------------------*\
.* .*  Help menu Using help help panel                             *
.* .*      res = PANEL_HELPUSINGHELP                               *
.* .*--------------------------------------------------------------*/
.* :h1 res=2940 name=PANEL_HELPUSINGHELP.Using Help
.* :i2 refid=Help.Using
.* :p.
.*
.*
.* .*--------------------------------------------------------------*\
.* .*  Help menu Save command help panel                           *
.* .*      res = PANEL_HELPKEYS                                    *
.* .*--------------------------------------------------------------*/
.* :h1 res=2950 name=PANEL_HELPKEYS.Keys
.* :i2 refid=Help.Keys
.* :p.
.*
.* .*--------------------------------------------------------------*\
.* .*  Help menu Save As command help panel                        *
.* .*      res = PANEL_HELPTUTORIAL                                *
.* .*--------------------------------------------------------------*/
.* :h1 res=2960 name=PANEL_HELPTUTORIAL.Tutorial
.* :i2 refid=Help.Tutorial
.* :p.
.*
.* .*--------------------------------------------------------------*\
.* .*  Help menu Product information help panel                    *
.* .*      res = PANEL_HELPPRODUCTINFO                             *
.* .*--------------------------------------------------------------*/
.* :h1 res=2970 name=PANEL_HELPPRODUCTINFO.Product information
.* :i2 refid=Help.Product information
.* :p.



:h1 res=3000 name=PANEL_TUTORIAL.PCBoard Control Panel Tutorial
:i1 id=Tutorial.Tutorial
:p.The following topics are arranged in an order that may help to better
understand the :hp1.PCBoard Control Panel:ehp1. by studying it from start to
finish.  You may select each topic in order, or skip around to read the topics
that interest you the most.
:sl compact.
:li.:link reftype=hd res=2100.What is it?:elink.
:li.:link reftype=hd res=2220.First things first - Open Usernet.:elink.
:li.:link reftype=hd res=2230.Setting Node Limits:elink.
:li.:link reftype=hd res=2240.Setting Up Nodes:elink.
:li.:link reftype=hd res=3020.Sharing a Single PCBoard Configuration:elink.
:li.:link reftype=hd res=3010.Automatic Node Startup:elink.
:li.:link reftype=hd res=2630.Setting Up Display Area:elink.
:li.:link reftype=hd res=2643.Automatic Problem Detection/Correction:elink.
:li.:link reftype=hd res=2640.Defining Automatic Warnings:elink.
:li.:link reftype=hd res=2655.Understanding the PCBCP Log File:elink.
:li.:link reftype=hd res=2310.Editing Current Node Status:elink.
:li.:link reftype=hd res=2340.Sending a Message to a Node:elink.
:li.:link reftype=hd res=2370.Finding the Process ID:elink.
:li.:link reftype=hd res=2410.Controlling Nodes:elink.
:li.:link reftype=hd res=2530.Setting the Screen Update Frequency:elink.
:li.:link reftype=hd res=2680.Saving Your Settings:elink.
:li.:link reftype=hd res=3023.Error Codes:elink.
:esl.


:h1 res=3010 name=PANEL_AUTOSTART.Automatic Node Startup
:i1 id=Startup.Automatic Node Startup
:p.Steps to enabling auto-node startup:
:ol.
:li.Set up one or more nodes via :link reftype=hd res=2240.:hp2.File | Node Settings:ehp2.:elink..
:li.Set the node to :link reftype=hd res=4152.:hp2.Run on Startup:ehp2.:elink..
:li.Enable :link reftype=hd res=2660.:hp2.Startup Settings:ehp2.:elink..
:eol.


:h1 res=3020.Sharing the PCBoard Configuration
:i2 refid=Startup.Multi-Node Setup
:p.A capability which seems almost :link reftype=hd res=3021.hidden:elink. in
the PCBoard manual, is the ability to share a single configuration file among
multiple nodes.  Now, with the introduction of the PCBoard Control Panel, it
seems a perfect opportunity to expose and promote this capability.
:p.The disadvantage to using multiple configuration files is that, the more
nodes you have, the more time you would spend updating each of the node
configuration files whenever you make a change.  So the advantage of a shared
configuration file is the time savings you reap by only having to make a change
once and have all of the nodes pick up the same change at the same time.
:note.Of course, there are times when separate configuration files can be
important or even necessary.  Press here for :link reftype=hd res=3022.More
Information:elink..
:p.The thing that brings the sharing of the PCBOARD.DAT file to the foreground
with PCBCP's introduction, is that PCBCP can be configured to launch all of
your nodes at startup.  By sharing configuration files, the actual setup of
the nodes in the :hp2.File | Node Settings:ehp2. is greatly simplified!
:p.The following outlines the steps required to set up your nodes, both in
PCBCP, as well as the .CMD files that will be used to tie everything together:
:ol.
:li.In the :hp2.File | Node Settings:ehp2. screen, you can configure one or
more PCBoard nodes.  On that screen, you define the path and filename to
execute, the working directory, any optional parameters, and optionally a port
number.  An example setup might look like this:
:xmp.
  Path and Filename:     :hp2.C&colon.\PCB\STARTOS2.CMD:ehp2.
  Working Directory:     :hp2.C&colon.\PCB:ehp2.
  Optional Parameters:
  Optional Port Number:  :hp2.2:ehp2.
:exmp.
With the above settings, PCBCP will execute STARTOS2.CMD in the C&colon.\PCB
directory and it will pass port number 2 to the program via a
:hp2.%PCBPORT%:ehp2. environment variable.  The node number is always passed
to the program via a :hp2.%PCBNODE%:ehp2. environment variable, but the port
number is only passed if it is non-zero.
:li.Next, you need to create the STARTOS2.CMD file that PCBCP will run.  An
example of the STARTOS2.CMD file might look like this:
:xmp.
  @echo off
  rem set any common variables here such as dszlog
  :hp2.set dszlog=pcbdsz.log:ehp2.
  rem set the port and node parameters to override pcboard.dat
  :hp2.set pcb=/PORT%PCBPORT% /OS /NODE&colon.%PCBNODE%:ehp2.
  rem now run board.cmd
  :hp2.board.cmd:ehp2.
:exmp.
:li.As noted above, PCBCP will run STARTOS2.CMD, which in turn will then run
the BOARD.CMD file.  An example of the BOARD.CMD might look like this:
:xmp.
  @echo off
  :hp2.&colon.top:ehp2.
  :hp2.c&colon.:ehp2.
  rem change to the node subdirectory (i.e. C&colon.\PCB\1 for node 1)
  :hp2.cd\pcb\%PCBNODE%:ehp2.
  :hp2.if exist remote.cmd REN remote.cmd remote.sys:ehp2.
  :hp2.if exist endpcb     DEL endpcb:ehp2.
  rem run PCBoard while sharing a single copy of pcboard.dat
  :hp2.pcboard2 /file&colon.c&colon.\pcb\pcboard.dat:ehp2.
  :hp2.if exist remote.cmd CALL remote.cmd:ehp2.
  :hp2.if exist event.cmd  CALL event.cmd:ehp2.
  :hp2.if exist event.bat  CALL event.bat:ehp2.
  :hp2.if NOT exist endpcb GOTO top:ehp2.
  :hp2.&colon.end:ehp2.
:exmp.
:note.
For the above to work, the BOARD.CMD file is going to need to be in your path.
For example, if BOARD.CMD is in C&colon.\PCB, you'll want to add C&colon.\PCB
to your SET PATH= statement in the CONFIG.SYS file (under OS/2 the CONFIG.SYS
file holds the PATH statement for OS/2 sessions).
:note.
The above example assumes your subdirectory structure looks like this:
:xmp.
  :hp2.C&colon.\PCB:ehp2.         main directory
  :hp2.C&colon.\PCB\1:ehp2.       node 1's subdirectory
  :hp2.C&colon.\PCB\2:ehp2.       node 2's subdirectory
  :hp2.C&colon.\PCB\3:ehp2.       node 3's subdirectory, etc.
:exmp.
:p.If your subdirectory structure inserts the word NODE into the name, as in
:hp2.C&colon.\PCB\NODE1:ehp2., then you need to modify the 5th line of the
BOARD.CMD file to say :hp2.cd\pcb\node%NODE%:ehp2. (note the addition of the
word :hp1.node:ehp1.).
:p.The combination of just these two CMD files, and the fact that PCBCP
passes the node and port numbers to the program it runs, means that you can set
up all of your nodes without having to create any more CMD files, and without
having to create any further PCBOARD.DAT files, etc.  Instead, all of your
nodes will share a single copy of STARTOS2.CMD, and a single copy of BOARD.CMD,
and a single copy of PCBOARD.DAT.
:p.This works because the CMD files will automatically change to the proper
node subdirectory on startup based on the %PCBNODE% variable.  And each node
will use the proper comm port based on the port you set in PCBCP.  And all of
the nodes will share the same PCBOARD.DAT file by using the /FILE:  command
line parameter on the PCBOARD2 command line.
:li.All that's left now is to simply COPY the first node settings to each of
your other nodes!  Once you have one node up and running, you can set up 10
more in a matter of seconds by using the COPY_TO button in the File | Node
Settings screen.  Using COPY_TO you copy an existing setup to a "range" of
other nodes and then just go to each node and set the appropriate port number
for each.
:eol.



:h1 hide res=3021 nosearch noprint.Where to Find Information?
:p.Hidden?  ;-)
:p.It's actually right near the front of the manual where the /FILE:  command
line parameter is discussed.  But it's often overlooked as the manual has not
gone out of its way to point out the benefits of sharing a single PCBoard
configuration.

:h1 hide res=3022 nosearch noprint.Using Separate Configurations
:p.The whole issue of sharing configuration files revolves around using the
:hp2.SET PCB=:ehp2. environment variable to pass enough configuration
information that a node can be set up and run as a unique node, even while
sharing the PCBOARD.DAT file.  For example, by using SET PCB=/NODE&colon.1
/COM1&colon.  you can override the node number and comm port number that is
stored in the PCBOARD.DAT file.  In similar fashion, then, each node can be set
up to have a unique node number and comm port while all of them share the same
PCBOARD.DAT file.
:p.However, sometimes it can be important, or even required, that separate
configuration files be used when setting up multiple PCBoard nodes.
:p.The following outlines some of the reasons why you might use separate
configuration files:
:ul.
:li.In some systems it may be desirable to have different capabilities on each
node.  For example, you may want to provide special access on one set of nodes,
while providing public access to another set of nodes.  In this case, you may
want to set up different security levels, different display files or menus,
different security level requirements, etc.  To accomodate any of these, you'll
need separate PCBOARD.DAT files because these settings are only stored in the
PCBOARD.DAT file (there are no SET PCB= environment equivalents which can be
used to override the contents of the PCBOARD.DAT file).
:li.Some programs, written by :hp1.Third Party Authors:ehp1., are unable to use
the environment variables (such as the PCB= environment variable) to override
the PCBOARD.DAT file settings.  For example, some doors will read the comm port
setting right out of the PCBOARD.DAT file and will have no way to override that
setting.  In this case, an error would occur if two nodes tried to access the
same comm port at the same time, so the PCBOARD.DAT file cannot be shared.
:note.One method for dealing with this issue, even while sharing the
PCBOARD.DAT file, is to have all of your PCBoard nodes share the PCBOARD.DAT
file, but copy a separate PCBOARD.DAT file into each node's subdirectory where
you then set the comm port.  Often the third party application will ignore most
of the other settings in the PCBOARD.DAT file and, therefore, even if your
separate PCBOARD.DAT files get out-of-synch, they won't be used by PCBoard and
the settings that are out-of-synch won't be used by the third party
application, so everyone is happy and you still save time.
:eul.


:h1 hide res=3023.Error codes
:i1 id=Errors.Error codes
:p.If an error occurs while trying to start a node, PCBCP will log the error
code to the PCBCP.LOG file.  Below is an example of what this looks like:
:xmp.
   09-18-95 13&colon.43&colon.39 - Error 164 launching 15
   09-18-95 13&colon.43&colon.39 - /c C&colon.\PCB\STARTOS2.CMD
   09-18-95 13&colon.43&colon.39 - C&colon.\PCB
:exmp.
:p.The three lines above are communicating several pieces of information:
:ol.
:li.The error number is 164.
:li.The node it was launching was node number 15.
:li.The program it was attempting to run was STARTOS2.CMD found in C&colon.\PCB.
:li.The default directory on startup was C&colon.\PCB.
:eol.
:p.A list of a few of the possible error codes follows (click on the error
code for additional information):
:parml tsize=12 break=none.
:pt.:link reftype=hd res=3030.:hp8.Error 2:ehp8.:elink.
:pd.File Not Found
:pt.:link reftype=hd res=3031.:hp8.Error 3:ehp8.:elink.
:pd.Path Not Found
:pt.:link reftype=hd res=3032.:hp8.Error 4:ehp8.:elink.
:pd.Too Many Open Files
:pt.:link reftype=hd res=3033.:hp8.Error 15:ehp8.:elink.
:pd.Invalid Drive
:pt.:link reftype=hd res=3034.:hp8.Error 164:ehp8.:elink.
:pd.Max Threads Reached
:eparml.

:h1 hide res=3030 noprint nosearch.Error 2: File Not Found
:p.The program specified in the :hp1.Path and Filename to Execute:ehp1. field does not exist.

:h1 hide res=3031 noprint nosearch.Error 3: Path Not Found
:p.Either the path to the program (specified in the :hp1.Path and Filename to
Execute:ehp1. field) or the :hp1.Working Directory:ehp1. in which it was to run
does not exist (or both) and needs to be created.

:h1 hide res=3032 noprint nosearch.Error 4: Too Many Open Files
:p.Normally this error is only seen by DOS applications because OS/2 does not
set a limit for the number of file handles that can be opened.
:p.However, the error could also occur if attempting to open files on a server
where the number of file handles available may still be exceeded.

:h1 hide res=3033 noprint nosearch.Error 15: Invalid Drive
:p.The drive specified in either the :hp1.Path and Filename to Execute:ehp1.
field or the :hp1.Working Directory:ehp1. field is invalid.

:h1 hide res=3034 noprint nosearch.Error 164: Max Threads Reached
:p.You need to the increase THREADS= setting in config.sys.  PCBoard uses
between 6 and 10 threads per node.  Plus there are additional threads used
inside of the OS/2 system itself.
:p.You can probably figure on a maximum of about 10-12 nodes with THREADS=256
but you can experiment with it from there.
:note.Because of the consumption of resources, you should probably not set the
THREADS= parameter too high without first determining that it needs to be
increased.  For example, a setting of THREADS=512 is probably overkill for most
systems and wastes precious memory.
