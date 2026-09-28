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
.* Option.ipf - Information Tag Language file for the Option    *
.*              menu help panels.                               *
.*                                                              *
.*==============================================================*/

.*--------------------------------------------------------------*\
.*  Main Option menu                                            *
.*      res = PANEL_OPTION                                      *
.*--------------------------------------------------------------*/
:h1 res=2610 name=PANEL_OPTION.Option Menu
:i1 id=Option.Option Menu
:p.The Option menu consists of several submenus which allow you to customize
the way the PCBoard Control Panel operates.  The following options appear in
the Options menu:
:parml tsize=20 break=none.
:pt.:hp2.Font:ehp2.
:pd.This option allows you to select a font that is used to display the node
status information in the :hp1.PCBCP Node List:ehp1..
:pt.:hp2.Display:ehp2.
:pd.This option sets the starting node number to be shown in the :hp1.PCBCP Node List:ehp1. display,
and also sets whether you want to display the node list using a fixed font or
a scalable font.  If using a scalable font, then you also specify the number of
nodes shown in the list.
:pt.:hp2.Warnings:ehp2.
:pd.This option allows you to set both visible and audible alarms and the
amount of time PCBCP should wait before alerting you to a possible hung node.
You can also set whether PCBCP should automatically restart a hung node.
:pt.:hp2.Startup:ehp2.
:pd.This option enables or disables the automatic running of nodes at startup
Both the :hp1.Run on Startup:ehp1. as well as :hp1.Run on Connect:ehp1. nodes
can be selectively enabled or disabled.
:pt.:hp2.Mouse:ehp2.
:pd.This option lets you configure the mouse so that when double clicking on a
node in the :hp1.PCBCP Node List:ehp1. it will either edit the node status, or
run/switch-to the node.
:pt.:hp2.Save Settings:ehp2.
:pd.This option is used to save the changes in the settings.  It also saves
the node limits specified in :hp2.File | Node Limits:ehp2..
:eparml.


.*--------------------------------------------------------------*\
.*  Option menu Font option                                     *
.*      res = PANEL_OPTIONFONT                                  *
.*--------------------------------------------------------------*/
:h1 res=2620 name=PANEL_OPTIONFONT.Font Option
:i2 refid=Option.Font Option
:p.The :hp2.Options | Font:ehp2. selection brings up a dialog box which allows you to select a font
that is used to display the node status information in the :hp1.PCBCP Node
List:ehp1. window.
:note.See also :hp2.Options | Display:ehp2. for the option which selects
whether the font should be used as a scalable font or as a fixed size font.


.*--------------------------------------------------------------*\
.*  Option menu Display option                                  *
.*      res = PANEL_OPTIONDISPLAY                               *
.*--------------------------------------------------------------*/
:h1 res=2630 name=PANEL_OPTIONDISPLAY.Display Options
:i2 refid=Option.Display Option
:link reftype=hd res=2631 vpx=left vpy=top vpcx=100% vpcy=75% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=2639 vpx=left vpy=bottom vpcx=100% vpcy=25%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=2631 nosearch noprint.Display Options General Help
:p.The :hp2.Options | Display:ehp2. selection brings up a dialog box which allows you to set which
node is the first node (by default) to be shown on the screen when the
:hp1.PCBoard Control Panel:ehp1. is started.
:p.You can also control how fonts are handled, selecting either a Fixed Size
font or a Scalable font.
:p.For specific help, select a topic below.

:h1 hide res=2639 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=2632.First Node on Screen:elink.
:li.:link reftype=hd res=2633.Scalable Font:elink.
:li.:link reftype=hd res=2634.Number of Lines on Screen:elink.
:li.:link reftype=hd res=2635.Fixed Font:elink.
:esl.

:h1 res=2632 nosearch noprint.First Node on Screen at Startup
:p.For most systems, you will want to display all nodes on the system starting
with node 1 at the top of the node list window.  Some systems, however, may be
configured such that you'll want to start the node list display at a higher
node number.  To do this you have two choices:
:ol.
:li.You can set the :hp1.First Node to Monitor:ehp1. in the :hp2.File | Node
Limits:ehp2. dialog, or
:li.You can set the :hp1.First Node on Screen at Startup:ehp1. in the
:hp2.Options | Display:ehp2. dialog.
:eol.
:p.The :hp1.First Node to Monitor:ehp1.  option prevents you from monitoring
any node lower than that first node.  So if you set it to 10, then nodes 1-9
could not be monitored.
:p.On the other hand, the :hp1.First Node on Screen at Startup:ehp1. option
simply sets a default starting node number.  If you set it to 10, then when
you start the PCBoard Control Panel it will begin with showing node 10 at the
top of the node list window, but you can still scroll up to see nodes 1-9.

:h1 res=2633 nosearch noprint.Scalable Font
:p.The :hp1.Scalable Font:ehp1. option offers an advantage in that you can set
up the PCBoard Control Panel to always monitor the same number of nodes, but
the :hp1.size:ehp1. of the information shown on screen will vary according to
the size of the window used to display it.  So, for instance, you could keep
PCBCP in a tiny window in the corner of your screen to monitor activity from
a distance, but then when you enlarge the window, the information on screen
will increase in size at the same time to make it more readable.
:note.You must set the :link reftype=hd res=2634.Number of Lines on
Screen:elink. at the same time.
:note.When the :hp1.Scalable Font:ehp1. option is used, the :hp1.Size:ehp1.
field in the :hp2.Options | Font:ehp2. dialog is ignored.

:h1 res=2634 nosearch noprint.Number of Lines on Screen
:p.The :hp1.Number of Lines on Screen:ehp1. option is used together with the
:hp1.Scalable Font:ehp1. option.  It is used to determine how many nodes should
be displayed on the screen when using the scalable font.
:p.What happens is that if you set the :hp1.Number of Lines on Screen:ehp1. to
10, then PCBCP will scale the size of the font such that exactly 10 nodes
appear in the node list window.  If you have a large window, then the font used
will be large and the 10 nodes will be very readable.  If you shrink the window
down, then the font will be reduced in size so that the 10 nodes remain in the
list but harder to read.

:h1 res=2635 nosearch noprint.Fixed Font
:p.When the :hp1.Fixed Font:ehp1. option is used, the actual size of the font
is determined by your selection for the Font Size in the :hp2.Options | Font:ehp2.
dialog.
:p.In this case, increasing the size of the window also affects the number of
nodes that appear on the screen (up to the node limits set in :hp2.File | Node
Limits:ehp2. dialog).

.*--------------------------------------------------------------*\
.*  Option menu Warnings option                                 *
.*      res = PANEL_OPTIONWARNING                               *
.*--------------------------------------------------------------*/
:h1 res=2640 name=PANEL_OPTIONWARNING.Warnings Option
:i2 refid=Option.Warnings Option
:link reftype=hd res=2641 vpx=left vpy=top vpcx=100% vpcy=75% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=2659 vpx=left vpy=bottom vpcx=100% vpcy=25%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=2641 nosearch noprint.Display Warnings General Help
:p.The :hp2.Options | Warnings:ehp2. selection allows you to control how PCBCP warns you about problems
on the system.  You define a time limit for inactivity at which point PCBCP
lets you know that it appears that a node may be hung.
:p.You can also set PCBCP to automatically shut down and restart a node that
appears to be hung, and how long after the warnings begin that it should do
this.
:p.There is also an option to turn off the Verify Status Change pop-up window
which appears whenever you apply a status change to nodes.
:p.Also, an option to enable a Page Alarm to sound when a caller is paging you,
or automatically bring the node to the foreground.
:p.For specific help, select a topic below.

:h1 hide res=2659 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=2642.Warning Options:elink.
:li.:link reftype=hd res=2651.Restart Options:elink.
:li.:link reftype=hd res=2652.Apply Status Warning Option:elink.
:li.:link reftype=hd res=2653.Page Alarm Options:elink.
:esl.

:h1 hide res=2642 nosearch noprint.Warning Options
:link reftype=hd res=2643 vpx=left vpy=top vpcx=100% vpcy=75% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=2658 vpx=left vpy=bottom vpcx=100% vpcy=25%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=2643 nosearch noprint.Warnings General Help
:p.The :hp1.PCBoard Control Panel:ehp1. can act as a :hp1.Watch Dog:ehp1. for
your system.  It can watch over the PCBoard operations and detect when it
appears that node is :hp1.hung:ehp1. and warn you of this occurance.  This
lets you have the peace of mind of knowing that the system is up and running
and only having to worry about if when an alarm goes off to get your attention.
:p.Because of this, you can safely run your nodes in the background without
monitoring the nodes for signs of trouble.  Or, because PCBCP does not have to
be run on the same machine the nodes are running on, you can even run the nodes
on another machine some distance away, perhaps locked in a closet somewhere but
accessible via a network, and still have PCBCP warn you of any signs of
trouble.
:p.PCBCP detects signs of trouble by watching the USERNET.XXX file which
PCBoard periodically updates.  Press here for :link reftype=hd res=2656.More
Information:elink. about how PCBCP detects a hung node.
:p.The warning options allow you to defined what action PCBCP should take.
Whether it should simply highlight the node, in the nodelist window, by showing
the node information in red, or sound an alarm.  In addition, PCBCP can be set
to monitor nodes which are not currently shown inside the node list window.
:p.The Number of Minutes options allow you to control how long PCBCP waits
before deciding that a node may be inactive (or hung).
:p.For specific help, select a topic below.

:h1 hide res=2658 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=2644.Highlight Nodes On Screen:elink.
:li.:link reftype=hd res=2645.Sound Alarm:elink.
:li.:link reftype=hd res=2646.Monitor All Nodes:elink.
:li.:link reftype=hd res=2647.Number of Minutes of Inactivity:elink.
:li.:link reftype=hd res=2648.Number of Minutes for Events:elink.
:li.:link reftype=hd res=2649.Number of Minutes for Doors:elink.
:li.:link reftype=hd res=2650.Number of Minutes for Transfers:elink.
:esl.


:h1 res=2644.Highlight Nodes on Screen
:i2 refid=Option.Highlight Nodes on Screen
:p.When this option is selected, PCBCP will monitor for inactive (or hung)
nodes and will warn you about them by highlighting the node inside the node
list window by displaying the node information in red.
:artwork name='warn.bmp'.
:p.While this may get your attention if you are looking at the screen, there
may be times when you will step away from your computer, or perhaps have PCBCP
minimized so that you cannot see it.  In this case, it can be helpful to turn
on the :link reftype=hd res=2645.Sound Alarm:elink option.
:note.Press here for :link reftype=hd res=2656.More Information:elink. about
how PCBCP detects a hung node.

:h1 res=2645.Sound Alarm
:i2 refid=Option.Sound Alarm
:p.When this option is selected, PCBCP will monitor for inactive (or hung)
nodes and will warn you about them by sounding an alarm through the PC's
built-in speaker.
:p.The alarm can be useful for getting your attention if you will be away from
the computer, or if you will be running PCBCP in a minimized state.
:note.With the alarm enabled, it is a good idea to also enable the option that
will :link reftype=hd res=2644.Highlight Nodes on Screen:elink so that you can
quickly determine which node(s) are the source of the problem.
:note.Press here for :link reftype=hd res=2656.More Information:elink. about
how PCBCP detects a hung node.

:h1 res=2646.Monitor All Nodes
:i2 refid=Option.Monitor All Nodes
:p.One of biggest goals in the design of the :hp1.PCBoard Control Panel:ehp1.
is to ensure that it uses a few CPU cycles as possible during its operation so
that you won't be afraid of its impact on overall system performance.
:p.To help ensure low CPU utilization, PCBCP normally scans only a sub-portion
of the usernet file, only that which would be shown on screen.  This
optimization is based on the assumption that, if you aren't interested enough
to be watching the node in the node list window, then you probably don't want
any cpu cycles dedicated to reading the information either.
:p.However, when you have PCBCP set up as a :hp1.Watch Dog:ehp1. to monitor
your nodes for inactivity, it may be useful to monitor nodes which are not
being shown in the node list window.  For example, you may run a 25 node
system, but only monitoring nodes 1-10 on the screen.  With the :hp2.Monitor
All Nodes:ehp2. option enabled, problems on nodes 11-25 can still be detected.
Leaving this option disabled would cause any problems on those nodes to be
ignored by PCBCP.
:note.Press here for :link reftype=hd res=2656.More Information:elink. about
how PCBCP detects a hung node.

:h1 res=2647.Number of Minutes of Inactivity
:i2 refid=Option.Number of Minutes of Inactivity
:p.As a review for how PCBCP detects a hung node, press here for :link
reftype=hd res=2656.More Information:elink..
:p.The :hp2.Number of Minutes of Inactivity:ehp2. option is a default option
used in all cases except those covered by the other three options which monitor
for inactivity when running a door, running an event, or performing a file
transfer.
:p.This option sets the length of time during which the usernet record can go
without being updated before PCBCP will warn you that the node appears to be
inactive (or hung).
:p.It is generally a good idea to keep this number low, while increasing the
values for the other three options.
:p.See also:
:sl compact.
:li.:link reftype=hd res=2648.Number of Minutes for Events:elink.
:li.:link reftype=hd res=2649.Number of Minutes for Doors:elink.
:li.:link reftype=hd res=2650.Number of Minutes for Transfers:elink.
:esl.

:h1 res=2648.Number of Minutes for Events
:i2 refid=Option.Number of Minutes for Events
:p.As a review for how PCBCP detects a hung node, press here for :link
reftype=hd res=2656.More Information:elink..
:p.The :hp2.Number of Minutes for Events:ehp2. option is used to determine how
long a node's status in the usernet file can indicate that the node is running
an event before PCBCP will warn you that the event appears to be hung.
:p.While it's important to know about an event that is stuck, you don't want
PCBCP to warn you if it's just taking a little longer than usual.  So you
should generally set this option for the length of time that would cover your
longest event, and perhaps add a few minutes as a :hp1.fudge factor:ehp1. in
case the event happens to be running a little slow.
:p.See also:
:sl compact.
:li.:link reftype=hd res=2647.Number of Minutes of Inactivity:elink.
:li.:link reftype=hd res=2649.Number of Minutes for Doors:elink.
:li.:link reftype=hd res=2650.Number of Minutes for Transfers:elink.
:esl.

:h1 res=2649.Number of Minutes for Doors
:i2 refid=Option.Number of Minutes for Doors
:p.As a review for how PCBCP detects a hung node, press here for :link
reftype=hd res=2656.More Information:elink..
:p.The :hp2.Number of Minutes for Doors:ehp2. option is used to determine how
long a node's status in the usernet file can indicate that the node is
currently running a DOOR application before PCBCP will warn you that the DOOR
appears to be hung.
:p.Depending on the kinds of DOOR applications you have on your system, your
callers may customarily spend a significant amount of time inside of a Door.
You should take this into consideration when setting the time limit for PCBCP's
detection of a possible hung DOOR application.
:p.See also:
:sl compact.
:li.:link reftype=hd res=2647.Number of Minutes of Inactivity:elink.
:li.:link reftype=hd res=2648.Number of Minutes for Events:elink.
:li.:link reftype=hd res=2650.Number of Minutes for Transfers:elink.
:esl.

:h1 res=2650.Number of Minutes for Transfers
:i2 refid=Option.Number of Minutes for Transfers
:p.As a review for how PCBCP detects a hung node, press here for :link
reftype=hd res=2656.More Information:elink..
:p.The :hp2.Number of Minutes for Transfers:ehp2. option is used to determine
how long a node's status in the usernet file can indicate that the node is
currently transfering a file before PCBCP will warn you that the file transfer
appears to be hung.
:p.During a batch transfer, PCBoard's internal protocols as well as its
external Zmodem modules will update the usernet file in between each file of
the batch transfer.  Other external protocols do not do this and so you should
take this into consideration when determining how long a period of time is
acceptable to allow a node to be in the file transfer state before PCBCP warns
you of a possible problem.
:p.See also:
:sl compact.
:li.:link reftype=hd res=2647.Number of Minutes of Inactivity:elink.
:li.:link reftype=hd res=2648.Number of Minutes for Events:elink.
:li.:link reftype=hd res=2649.Number of Minutes for Doors:elink.
:esl.

:h1 hide res=2651 nosearch noprint.Restart Options
:p.The :hp2.Restart After Warnings:ehp2. option is like a :hp1.Watch Dog:ehp1.
feature in that it can be used to automatically shut down a node which is hung
and restart it automatically.
:p.This capability is not based on "ring detection" but is based on PCBCP's
ability to detect a hung node based on inactivity in the usernet file.  As a
review for how PCBCP detects a hung node, press here for :link reftype=hd
res=2656.More Information:elink..
:p.To enable this feature, click on the :hp2.Restart Hung Nodes:ehp2. option.
:p.You also need to set the :hp2.Minutes After Warning:ehp2. which determines
how long after an inactivity warning occurs the node shoud be shut down and
restarted.
:p.As a general recommendation, this value should be set to about 3 minutes
which gives you time to hear the warning and investigate the cause of the
problem.  If you will be away from the computer, then a shorter time limit
can be set in order to restart the hung node more quickly.
:p.Whenever PCBCP restarts a node, it updates a log file called PCBCP.LOG
with progress information.  Press here for :link reftype=hd res=2655.More
Information:elink. about the log file.

:h1 hide res=2652 nosearch noprint.Apply Status Warning Option
:p.The :hp2.Verify Status Changes:ehp2. option is used to enable or disable
the pop-up verification window which appears when you attempt to apply a
status change to one or more nodes.  The pop-up window appears and indicates
in the window the node, or nodes, to which the status will be applied and gives
you a chance to cancel the request or continue.

:h1 hide res=2653 nosearch noprint.Page Alarm Options
:p.When a caller is paging the sysop, normally you'd hear PCBoard's page alarm
(the once every 2-second beep that PCBoard emits).  But if you are at a remote
location (in another room, for example) using PCBCP to monitor the system, you
may not hear PCBoard's page alarm.
:p.To help alert you to callers paging the sysop, you can enable the :hp2.Page
Alarm:ehp2.  feature which will then sound an alarm whenever PCBCP detects that
someone is paging the sysop.
:note.If you are running PCBCP on the same machine where the nodes are located,
then it is best to leave the Page Alarm disabled to avoid hearing both alarms
at the same time.
:p.The :hp2.Bring Node to Foreground:ehp2. option is only useful if PCBCP is
controlling the node.  If it is, then when a caller pages you, you can have
PCBCP automatically bring the node to the foreground so that you can jump into
chat with the caller.
:note.This option can be useful if you don't frequently access the machine or
are paged infrequently by callers because it removes the necessity of locating
the node that is paging you and then manually bringing it to the foreground.
However, if you plan to work in the foreground a lot, or expect to receive
frequent pages, this option may actually be bothersome in that it will abruptly
bring the node to the foreground interrupting your work.

:h1 hide res=2655 nosearch noprint.PCBCP Log File
:p.The :hp1.PCBoard Control Panel:ehp1. maintains a log file, called PCBCP.LOG,
which contains status information about some of PCBCP's processing.  The two
items that PCBCP tracks in this file are:
:ol.
:li.The starting of a node via the :link reftype=hd res=4152.Run upon Connection:elink.
wherein a node is started automatically by PCBCP when an incoming connection is
detected.  In this case, the information in the PCBCP.LOG file looks
something like this:
:xmp.
08-30-95 11&colon.24&colon.01 - Launched Node 10 (Run on Connect)
:exmp.
:p.This information is used to indicate the time at which a Run on Connect node
was started and can be used to give you some feedback as to how often your
Run on Connect nodes are being accessed.
:li.PCBCP also keeps a log of any hung nodes which have to be automatically
restarted.  These log entries include progress information such as the
following:
:xmp.
08-30-95 12&colon.19&colon.56 - Attempting to close Node 1 (Inactive)
08-30-95 12&colon.19&colon.56 - Node 1 Status was&colon.  JOSEPH PETRIES - Available for Chat
08-30-95 12&colon.19&colon.57 - Node 1 stopped.  Attempting to restart now.
08-30-95 12&colon.19&colon.59 - Node 1 is now running.
:exmp.
:p.The above example shows a successful detection of a hung node, together with
the status of the node at the time it was stopped, and a log entry indicating
that the node was successfully restarted.
:p.There are two more log entries which might appear, and if so, are used to
indicate that an error has occurred in attempting to shut the node down or
restart it after having shut it down.  They are:
:xmp.
08-30-95 12&colon.20&colon.30 - Node 1 still active (did not exit).
08-30-95 12&colon.20&colon.35 - Node 1 is still down (did not restart).
:exmp.
:eol.
:note.The PCBCP.LOG file is created in PCBCP's :hp1.current directory:ehp1..
So if you use a program object to run PCBCP, the location you specify in the
:hp2.Working Directory:ehp2. will be the location which will be used by PCBCP
when creating the PCBCP.LOG file.
:note.If the nodes do not start up, you should examine the PCBCP.LOG file for
error codes.  Press here for a :link reftype=hd res=3023.list of error codes:elink..

:h1 hide res=2656 nosearch noprint.PCBCP Hung Node Detection
:p.Based on the :link reftype=hd res=2530.Update Interval:elink. you set, PCBCP
scans the usernet.xxx file checking the status of each node and comparing the
current system time with the date stamp associated with each node in the file.
:p.PCBoard updates the usernet record for each node as various activities are
performed on the system.  PCBCP then uses this information to determine how
long it has been since the last update occurred.  Then it compares that length of
time to the :link reftype=hd res=2658.Warning Limits:elink. which you have set for
determining when it appears that a node may be hung.
:p.Because of the method used to detect a hung node, it is possible for PCBCP
to detect a problem and correct it (with the :link reftype=hd res=2651.Restart
Options:elink.) without waiting for a modem ring to go unanswered.
:p.If you plan to use PCBCP to monitor nodes running :hp2.PCBoard-for-DOS:ehp2.
then it is important to understand the differences in the way it updates the
usernet file as compared to how :hp2.PCBoard-for-OS/2:ehp2. performs the
updates.  Press here for :link reftype=hd res=2658.More Information:elink about
how the DOS version updates the usernet file.

:h1 hide res=2657 nosearch noprint.PCB for DOS vs OS/2
:p.PCBoard for DOS and PCBoard for OS/2 update the usernet file in a slightly
different, but important, manner which affects how you will want to set the
:link reftype=hd res=2642.Warning Options:elink. and :link reftype=hd
res=2651.Restart Options:elink..
:p.:hp2.PCBoard for OS/2:ehp2. updates the usernet file by using a separate
thread which is constantly running.  This thread updates the usernet record
approximately once every 90 seconds.  Therefore, a very low setting for the
:link reftype=hd res=2647.Number of Minutes of Inactivity:elink. is possible.
:p.:hp2.PCBoard for DOS:ehp2., on the other hand, does not use a separate
thread and therefore updates the usernet file only when the caller is not busy
doing something (such as entering a message, writing a file description, etc).
Because of this, a low setting for :hp2.Number of Minutes of Inactivity:ehp2.
is likely to cause a false warning to occur (i.e. the alarm could sound even
though the node is not hung, but is busy handling the caller's needs).  You
would, therefore, want to set a higher limit for the Number of Minutes of
Inactivity warning if you will be monitoring DOS nodes with PCBCP.

.*--------------------------------------------------------------*\
.*  Option menu Startup Settings option                         *
.*      res = PANEL_OPTIONSTARTUP                               *
.*--------------------------------------------------------------*/
:h1 res=2660 name=PANEL_OPTIONSTARTUP.Startup Settings Option
:i2 refid=Option.Startup Settings Option
:link reftype=hd res=2661 vpx=left vpy=top vpcx=100% vpcy=75% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=2669 vpx=left vpy=bottom vpcx=100% vpcy=25%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=2661 nosearch noprint.Startup Settings General Help
:p.The :hp2.Options | Startup Settings:ehp2. option brings up a dialog box
which allows you to enable or disable the automatic starting of nodes.
:p.For specific help, select a topic below.

:h1 hide res=2669 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=2662.Enable/Disable Auto-Run on Startup:elink.
:li.:link reftype=hd res=2663.Enable/Disable Auto-Run on Connect:elink.
:esl.

:h1 hide res=2662 nosearch noprint.Enable/Disable Auto-Run on Startup
:p.Enabling the :hp2.Auto-Run on Startup:ehp2. feature causes PCBCP to
automatically begin launching any nodes which are configured to :link
reftype=hd res=4152.Run at Startup:elink..
:p.This switch can be used as a :hp1.master switch:ehp1. to completely disable
the automatic launching of nodes at startup without having to go into the
:hp2.File | Node Settings:ehp2. dialog to reconfigure your nodes.

:h1 hide res=2663 nosearch noprint.Enable/Disable Auto-Run on Connect
:p.Enabling the :hp2.Auto-Run on Startup:ehp2. feature causes PCBCP to
automatically launch a new node to handle an incoming call when the node is
configured to use VMODEM and is set to :link reftype=hd res=4152.Run upon
Connection:elink..
:p.This switch can be used as a :hp1.master switch:ehp1. to completely disable
the automatic launching of nodes without having to go into the
:hp2.File | Node Settings:ehp2. dialog to reconfigure your nodes.

.*--------------------------------------------------------------*\
.*  Option menu Mouse option                                    *
.*      res = PANEL_OPTIONMOUSE                                 *
.*--------------------------------------------------------------*/
:h1 res=2670 name=PANEL_OPTIONMOUSE.Mouse Option
:i2 refid=Option.Mouse Option
:p.The :hp2.Options | Mouse:ehp2. selection brings up a dialog box which allows you to control the
behavior of Mouse Button 1.  When you double-click on Mouse Button 1 you can
have PCBCP either edit the Node Status of the node you clicked on, or you can
have it Run or Switch-to the Node window.


.*--------------------------------------------------------------*\
.*  Option menu Save option                                     *
.*      res = PANEL_OPTIONSAVE                                  *
.*--------------------------------------------------------------*/
:h1 res=2680 name=PANEL_OPTIONSAVE.Save Option
:i2 refid=Option.Save Option
:p.The :hp2.Options | Save:ehp2. selection saves all of the settings you have made via the
:hp2.Options:ehp2. menu and also the :hp2.File | Node Limits:ehp2. option.


.***********************************************************************
.*Help for Font Dialog box   (D)
:h1 hide res=4120 scroll=none titlebar=yes group=1 id=A9004.Help for Font
.***********************************************************************
:link reftype=hd res=21500 vpx=left vpy=top vpcx=100% vpcy=75% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=21600 vpx=left vpy=bottom vpcx=100% vpcy=25%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=21500 nosearch noprint.Help for Font
:ol compact.
:li.Select the down arrow to the right of the :hp2.Name:ehp2. list to display
all the fonts on your system.
:li.Select the :hp1.font style:ehp1. from the :hp2.Style:ehp2. list.
:li.Select the :hp1.font size:ehp1. from the :hp2.Size:ehp2. list.
:li.Press Ok to select the current displayed font, or Reset to return back to
the original font selection.  Cancel exits the dialog without making any
changes.
:eol.
:note.The :hp2.Options | Display:ehp2. dialog has an effect on the font selected.  If you
have chosen :hp1.Scalable Font:ehp1. then only proportional fonts will appear
in the Name list.
:p.For specific help, select a topic below.

:h1 hide res=21600 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=21501 group=50.Font name:elink.
:li.:link reftype=hd res=21502 group=51.Style:elink.
:li.:link reftype=hd res=21503 group=52.Size:elink.
:esl.

:h1 hide res=21501 nosearch noprint.Font name
:p.The font name list shows a list of all font types available.
:note.If you have selected :hp1.Scalable Font:ehp1. from the :hp2.Options | Display:ehp2.
dialog, the list of fonts will be reduced to only those fonts which can be
scaled.  These would be the fonts that are called :hp1.vector fonts:ehp1. as
opposed to :hp1.bitmap fonts:ehp1..  The reason is that the :hp1.bitmap
fonts:ehp1. cannot be scaled.
:note.If you select a :hp1.bitmap font:ehp1. first, then go to the
:hp2.Options | Display:ehp2. and select :hp1.Scalable Font:ehp1. the node list
screen will go blank.  This is because you are currently using a font which
cannot be scaled.  Either change back to a :hp1.Fixed Font:ehp1. or else select
a new font from the shortened font list that is now presented.

:h1 hide res=21502 nosearch noprint.Style
:p.You generally have three styles to choose from, depending on the font:
Bold, Italicized and Normal.
:p.In the node list window, the normal and bold styles are generally the
easiest to read.

:h1 hide res=21503 nosearch noprint.Size
:p.The :hp1.font size:ehp1. allows you to set the size of the font when using
the :hp1.Fixed Font:ehp1. option.
:note.If you select :hp1.Scalable Font:ehp1. from the :hp2.Options | Display:ehp2.
dialog, then the actual size of the font will depend on the size of the window
you have chosen.  The :hp1.font size:ehp1. option, in this case, has no effect.


