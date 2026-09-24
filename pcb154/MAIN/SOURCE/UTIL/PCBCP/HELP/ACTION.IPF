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

.*--------------------------------------------------------------*\
.*  Main Action menu                                            *
.*      res = PANEL_ACTION                                      *
.*--------------------------------------------------------------*/
:h1 res=2410 name=PANEL_ACTION.Action Menu
:i1 id=Action.Action Menu
:p.The :hp2.Action:ehp2. menu consists of action items that can be applied to one or more
PCBoard nodes.  The following commands appear in the Action menu:
:parml tsize=30 break=none.
:pt.:hp2.:link reftype=hd res=2420.Nodes On Screen:elink.:ehp2.
:pd.This option has a submenu which applies an action to all nodes that are
currently displayed on the screen, in the PCBCP node list.
:pt.:hp2.:link reftype=hd res=2430.All Nodes:elink.:ehp2.
:pd.This option has a submenu which applies an action to all nodes on the
system, including nodes which are not currently displayed in the :hp1.PCBCP node list:ehp1..
:pt.:hp2.:link reftype=hd res=2440.Run/Switch-to Nodes:elink.:ehp2.
:pd.After highlighting one or more nodes in the :hp1.PCBCP node list:ehp1.,
this option will either start the node (if currently inactive), or switch the
node to the foreground.
:pt.:hp2.:link reftype=hd res=2450.Restart Inactive Nodes:elink.:ehp2.
:pd.This option applies to all nodes that are configured in the :hp1.File | Node Settings:ehp1.
screen.  If any of these nodes are currently inactive, PCBCP will run it now.
:pt.:hp2.:link reftype=hd res=2451.Hide Active Nodes:elink.:ehp2.
:pd.This option will minimize (hide) all of the visable windows (PCBoard nodes)
which PCBCP is currently controlling.
:pt.:hp2.:link reftype=hd res=2452.Show Active Nodes:elink.:ehp2.
:pd.This option will make visable (show) all of the minimized windows (PCBoard
nodes) which PCBCP is currently controlling.
:pt.:hp2.:link reftype=hd res=2453.Arrange Visible Nodes:elink.:ehp2.
:pd.This option will rearrange the window placements of the visible nodes so
that the start at the top left hand corner of the screen.
:eparml.

.*--------------------------------------------------------------*\
.*  Action menu Nodes On Screen command help panel              *
.*      res = PANEL_ACTIONONSCREEN                              *
.*--------------------------------------------------------------*/
:h1 res=2420 name=PANEL_ACTIONONSCREEN.Nodes on Screen
:i2 refid=Action.Nodes on Screen
:p.The :hp2.Actions | Nodes on Screen:ehp2. option automatically selects all of the
nodes that are currently on the screen (i.e. in the :hp1.PCBCP Node List:ehp1.)
and then provides a submenu of actions to apply to all of those nodes at once.
:p.Click on
:link
reftype=hd res=2460.Actions List
:elink.
to get a list of the actions that can be applied to a node.


.*--------------------------------------------------------------*\
.*  Action menu All Nodes command help panel                    *
.*      res = PANEL_ACTIONALLNODES                              *
.*--------------------------------------------------------------*/
:h1 res=2430 name=PANEL_ACTIONALLNODES.All Nodes
:i2 refid=Action.All Nodes
:p.The :hp2.Actions | All Nodes:ehp2. option automatically selects all of the nodes
for the system (within the limits you have defined in the :hp2.File | Node Limits:ehp2.
dialog) and then provides a submenu of actions to apply to all of those
nodes at once.
:p.Click on
:link
reftype=hd res=2460.Actions List
:elink.
to get a list of the actions that can be applied to a node.


.*--------------------------------------------------------------*\
.*  Action menu Run/Switch-to Nodes command help panel          *
.*      res = PANEL_ACTIONRUNSWITCH                             *
.*--------------------------------------------------------------*/
:h1 res=2440 name=PANEL_ACTIONRUNSWITCH.Run/Switch-to Nodes
:i2 refid=Action.Run/Switch-to Nodes
:p.After selecting one or more nodes from the :hp1.PCBCP Node List:ehp1., the
:hp2.Actions | Run/Switch-to Nodes:ehp2. option will automatically switch to (i.e. bring
to the foreground) any of those nodes which were already running.  And, it will
start up any of the selected nodes which were not already running.


.*--------------------------------------------------------------*\
.*  Action menu Restart Inactive Nodes command help panel       *
.*      res = PANEL_ACTIONRESTART                               *
.*--------------------------------------------------------------*/
:h1 res=2450 name=PANEL_ACTIONRESTART.Restart Inactive Nodes
:i2 refid=Action.Restart Inactive Nodes
:p.The :hp2.Actions | Restart Inactive Nodes:ehp2. option automatically restarts all of
the nodes that have been defined in the :hp2.File | Node Settings:ehp2. screen
which are currently inactive.
:p.This option can be useful if you have disabled the :hp2.Options | Run on Startup:ehp2. option,
or if you have subsequently dropped all of your nodes and now need to start
them all back up again.


.*--------------------------------------------------------------*\
.*  Action menu Hide Active Nodes command help panel            *
.*      res = PANEL_ACTIONHIDENODES                             *
.*--------------------------------------------------------------*/
:h1 res=2451 name=PANEL_ACTIONHIDENODES.Hide Active Nodes
:i2 refid=Action.Hide Active Nodes
:p.The :hp2.Actions | Hide Active Nodes:ehp2. option will locate all of the
windows which PCBCP has open and visible and minimize them all at once.
:p.You can restore all of the windows at once by using the
:link reftype=hd res=2452.Show Active Nodes:elink. option.


.*--------------------------------------------------------------*\
.*  Action menu Show Active Nodes command help panel            *
.*      res = PANEL_ACTIONSHOWNODES                             *
.*--------------------------------------------------------------*/
:h1 res=2452 name=PANEL_ACTIONSHOWNODES.Show Active Nodes
:i2 refid=Action.Show Active Nodes
:p.The :hp2.Actions | Show Active Nodes:ehp2. option will locate all of the
open windows which PCBCP has minimized and make them all visible at once.
:p.You can hide all of the windows at once by using the
:link reftype=hd res=2451.Hide Active Nodes:elink. option.


.*--------------------------------------------------------------*\
.*  Action menu Arrange Visible Nodes command help panel        *
.*      res = PANEL_ACTIONARRANGENODES                          *
.*--------------------------------------------------------------*/
:h1 res=2453 name=PANEL_ACTIONARRANGENODES.Arrange Visible Nodes
:i2 refid=Action.Arrange Visible Nodes
:p.The :hp2.Actions | Arrange Visible Nodes:ehp2. option will take each of the
visible node windows that are under PCBCP's control and rearrange them on the
screen starting at the top left corner of the screen and progressing down and
to the right so that the title bar for each node is visible.


.*--------------------------------------------------------------*\
.*  The following is a simple list of Action items which is     *
.*  linked-to by various other panels                           *
.*--------------------------------------------------------------*/
:h2 res=2460 name=PANEL_ACTIONLIST.Actions List
:p.:hp2.Click on the Action item below for further information.:ehp2.
:sl.
:li.:link reftype=hd res=2461.Drop to DOS Now:elink.
:li.:link reftype=hd res=2462.Drop to DOS at Logoff:elink.
:li.:link reftype=hd res=2463.Log User Off:elink.
:li.:link reftype=hd res=2464.Recycle Node:elink.
:li.:link reftype=hd res=2465.Clear Node:elink.
:esl.

.*--------------------------------------------------------------*\
.*  This panel defines the phrase "Drop to DOS Now"             *
.*--------------------------------------------------------------*/
:h2 res=2461 name=PANEL_ACTIONDROPDOSNOW.Drop to DOS Now
:i2 refid=Action.Drop to DOS Now
:p.The :hp2.Drop to DOS Now:ehp2. action causes a node to immediately exit
without regard for what the caller is doing.
:note.
The phrase :hp1."Drop to DOS":ehp1. is a holdover from PCBoard's roots as a DOS
application.  The functionality implied is that PCBoard is to exit and return
to the operating system.  Under DOS this meant that PCBoard would exit out to
a command prompt.  Under OS/2, the operating system may instead close the
window instead of landing at a command prompt.
:note.
If a node is at the call waiting screen, the node will exit right away.  If a
caller is logged in and performing an internal PCBoard function, the node will
log the caller off and exit right away.  But if the caller is performing an
external file transfer, or out in a door or some other external process, then
the logoff will not occur until the caller returns back to PCBoard, at which
point he will be logged of and PCBoard will then recycle and exit.
:note.
There is a system defined :link reftype=hd res=2466.Waiting Period:elink.
before the action takes place.
:p.See also:
:link
reftype=hd res=2462.Drop to DOS at Logoff
:elink.

.*--------------------------------------------------------------*\
.*  This panel defines the phrase "Drop to DOS at Logoff"       *
.*--------------------------------------------------------------*/
:h2 res=2462 name=PANEL_ACTIONDROPDOS.Drop to DOS at Logoff
:i2 refid=Action.Drop to DOS at Logoff
:p.The :hp2.Drop to DOS at Logoff:ehp2. action causes a node to wait until the
caller that is currently online logs off, and then it will exit.  If no caller
is online, then it will exit immediately.
:note.
The phrase :hp1."Drop to DOS":ehp1. is a holdover from PCBoard's roots as a DOS
application.  The functionality implied is that PCBoard is to exit and return
to the operating system.  Under DOS this meant that PCBoard would exit out to
a command prompt.  Under OS/2, the operating system may instead close the
window instead of landing at a command prompt.
:note.
There is a system defined :link reftype=hd res=2466.Waiting Period:elink.
before the action takes place.
:p.See also:
:link
reftype=hd res=2461.Drop to DOS Now
:elink.

.*--------------------------------------------------------------*\
.*  This panel defines the phrase "Log User Off"                *
.*--------------------------------------------------------------*/
:h2 res=2463 name=PANEL_ACTIONLOGOFF.Log User Off
:i2 refid=Action.Log User Off
:p.The :hp2.Log User Off:ehp2. action causes PCBoard to log the user off and
recycle.
:note.
If the caller is performing an internal PCBoard function, the action will occur
right away.  If the caller is performing an external file transfer, or out in a
door or some other external process, then the logoff will not occur until the
caller returns back to PCBoard.
:note.
There is a system defined :link reftype=hd res=2466.Waiting Period:elink.
before the action takes place.

.*--------------------------------------------------------------*\
.*  This panel defines the phrase "Recycle Node"                *
.*--------------------------------------------------------------*/
:h2 res=2464 name=PANEL_ACTIONRECYCLE.Recycle Node
:i2 refid=Action.Recycle Node
:p.The :hp2.Recycle Node:ehp2. action causes PCBoard to exit, at which point
the board.cmd (or board.bat for DOS) file continues execution.  PCBoard exits
with an errorlevel of 6, and does so without creating the ENDPCB file.  So the
batch file simply recycles back up to the top and restarts PCBoard.
:p.You might want to use this capability for keeping your nodes updated.  For
example, if a recycle occurs, you could have the board.cmd (or board.bat) file
check the server for files that have been updated and copy them down to a
local drive, if necessary.
:p.This action also causes configuration files to be re-read, which means
that if you have updated a shared configuration file, such as PCBOARD.DAT, then
you could set all of your nodes to recycle and pick up the new configuration.
:note.
If there is a caller online, PCBoard will wait until the caller logs off before
performing a recycle.  If the node is at the Call Waiting screen then the
action will take place right away.
:note.
There is a system defined :link reftype=hd res=2466.Waiting Period:elink.
before the action takes place.

.*--------------------------------------------------------------*\
.*  This panel defines the phrase "Clear Node"                  *
.*--------------------------------------------------------------*/
:h2 res=2465 name=PANEL_ACTIONCLEAR.Clear Node
:i2 refid=Action.Clear Node
:p.The :hp2.Clear Node:ehp2. action really doesn't affect PCBoard at all.
All it really does is clear the usernet record.
:p.You might want to use this option if you know that nodes have been shut
down (perhaps rebooted without properly exiting PCBoard).

.*--------------------------------------------------------------*\
.*  This panel defines the phrase "Waiting Period"              *
.*--------------------------------------------------------------*/
:h2 res=2466 name=PANEL_ACTIONWAITPERIOD.Waiting Period
:p.In :hp2.PCBSetup | Node Configuration:ehp2., there is an option to set the
:hp1.Node Chat Frequency:ehp1..  This setting basically controls how often
PCBoard checks the USERNET.XXX file to see if there is any processing that it
needs to perform.
:p.The USERNET.XXX file is used to pass information between nodes, such as node
chat requests and broadcast messages.  It is also used for passing commands
such as a :hp1.Drop to DOS:ehp1. request, a :hp1.Log User Off:ehp1. request,
a :hp1.Recycle:ehp1. request, etc.
:p.The sysop sets this value, and it typically has a range from 5 to 99 seconds.
So if you issue a command that is supposed to cause the node to exit, and the
Node Chat Frequency is set to 10 seconds, it may take anywhere from 0 to 10
seconds for the request to be acted upon.
:note.If you use the :hp1.Message:ehp1. field inside of the
:link reftype=hd res=2330.Edit Node Status:elink. dialog to broadcast a message
to the caller, the time delay before the caller sees the message is determined
by the Node Chat Frequency.  However, in addition to this delay, PCBoard will
avoid picking up the message text until the caller finishes the current
operation (i.e. if the caller is downloading a file, the caller won't see the
broadcast message until after the download is complete).
:note.
For DOS nodes, which do not have a separate thread dedicated to monitoring the
usernet file, the Node Chat Frequency is reduced at the Call Waiting screen to
reduce network traffic.  PCBoard reduces the Node Chat Frequency by a factor of
6.  So if you use a setting of 10 seconds, at the Call Waiting screen the
frequency will actually be 60 seconds.  Therefore, you should expect a longer
delay for response to commands such as forcing a node to :hp1.Drop to DOS:ehp1.
or :hp1.Recycle:ehp1. while at the Call Waiting screen.
