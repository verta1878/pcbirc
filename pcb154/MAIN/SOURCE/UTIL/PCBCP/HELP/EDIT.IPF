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
.* Edit.ipf - Information Tag Language file for the Edit menu   *
.*              help panels.                                    *
.*                                                              *
.*==============================================================*/

.*--------------------------------------------------------------*\
.*  Main Edit menu                                              *
.*      res = PANEL_EDIT                                        *
.*--------------------------------------------------------------*/
:h1 res=2310 name=PANEL_EDIT.Edit Option
:i1 id=Edit.Edit Option
:p.The :hp2.Edit:ehp2. selection allows you to edit the current status of a node.
:p.To bring up the :hp2.Node Status:ehp2. dialog box, you first select a node by
:hp1.single clicking:ehp1. on the node status line, then select the
:hp7.Edit:ehp7. option from the main menu.
:p.Another method for bringing up the :hp2.Node Status:ehp2. dialog box is to
move your mouse to the node status line, then press :hp1.Mouse Button 2:ehp1..
This brings up a pop-up menu, from which you can select the :hp7.Edit Node Status:ehp7.
menu option.


.*--------------------------------------------------------------*\
.*  Edit menu Edit Node Status dialog help panel                *
.*      res = PANEL_EDITNODE                                    *
.*--------------------------------------------------------------*/
:h1 res=2320 name=PANEL_EDITNODE.Edit Node Status
:i2 refid=Edit.Edit Node Status
:link reftype=hd res=2321 vpx=left vpy=top vpcx=100% vpcy=50% scroll=vertical
rules=border titlebar=none child auto.
:link reftype=hd res=2322 vpx=left vpy=bottom vpcx=100% vpcy=50%
scroll=vertical rules=none titlebar=none child auto.

:h1 hide res=2321 nosearch noprint.Edit Node Status General Help
:p.The :hp2.Edit Node Status:ehp2. dialog is used to view and/or modify the
current node status.  The dialog box provides you with information about the
current node (name, city, operation text, message, node status), shows the
last update, and provides several Quick Action Buttons for modifying the
current node status.
:p.Select a topic below for further details.

:h1 hide res=2322 nosearch noprint.List of Fields
:sl compact.
:li.:link reftype=hd res=2330.Viewing/Changing Node Status:elink.
:li.:link reftype=hd res=2340.Sending a Message:elink.
:li.:link reftype=hd res=2350.Changing Node Status:elink.
:li.:link reftype=hd res=2360.Quick Action Buttons:elink.
:li.:link reftype=hd res=2370.Process Id:elink.
:esl.

:h1 hide res=2330.Viewing/Changing Node Status
:i2 refid=Edit.Viewing/Changing Node Status
:p.When viewing the :hp2.Node Status:ehp2., the text fields on the screen
reflect the current information found inside of the USERNET.XXX file as set
by PCBoard during normal operations.
:p.For example, you can view the Name and City information that corresponds
with the user that is online.
:p.If the user is currently performing a file transfer, or is out in a door, or
if the node is currently running an event, you may see information inside the
Operational Text field which reflects the current operating being performed.
For example, if a file is being downloaded, the operation text will indicate
the name of the file being downloaded.  If an event is being executed, then
the operation text will indicate the number and name of the event.
:p.The message text will normally be blank unless a message has been sent to
the caller and the caller's node has not yet picked it up.
:p.PCBoard maintains a separate Status Field for each node which reflects the
current status of the node (i.e. indicates what the node is currently doing).
The Status field shown in the dialog box will indicate the current node's
status.
:note.PCBCP will continue monitoring changes in the usernet file while you are
looking at the Edit Node Status screen.  If the node status changes, you will
see these changes appear on the screen unless you have modified one or more of
the fields in the dialog box.
:note.Any changes you make in the Edit Node Status dialog will be undone if
you select :hp1.Cancel:ehp1. to close the dialog box.  Changes are not actually
written to the usernet file until the :hp1.OK:ehp1. button is pressed.

:h1 hide res=2340.Sending a Message
:i2 refid=Edit.Sending a Message
:p.Sending a message to a caller that is online is as simple as typing text
into the Message field and then pressing the :hp1.OK:ehp1. button.
:p.If the caller is currently browsing the system, PCBoard will pick the
information up from the usernet file and display it on the caller's screen.
However, if the caller is currently busy (e.g. performing a file transfer),
PCBoard will not pick up the message until the caller finishes the current
operation.
:note.In addition to waiting for the caller to finish his current operation
before picking up the message text, there is also a delay which is set by
the sysop (called the :hp1.Node Chat Frequency:ehp1.) which is used to control
how frequently PCBoard examines the usernet file.  Press here for :link
reftype=hd res=2466.More Information:elink. about the delay associated with
the Node Chat Frequency setting.

:h1 hide res=2350.Changing Node Status
:i2 refid=Edit.Changing Node Status
:p.The :hp2.Node Status:ehp2. field uses a :hp1.List Box:ehp1. to display the
current node status and allow you to change the current node status.
:p.Three methods exist for changing the node status field:
:ol.
:li.By clicking on the arrow (to the right of the list box), a drop-down box
will appear on the screen showing all of the possible settings for the
node status.
:p.The list box will automatically highlight the current node status and you
can use the mouse or cursor keys to move up and down within the list box to
find the status you want to change it to.
:p.Once you find the status selection desired, you can either click on it with
the mouse, or if it is currently highlighted you can simply press enter.
:li.After highlighting the status field by clicking on it with the mouse, you
can simply press the first letter of the status desired and it will
automatically be shown in the status field without having to open up the list
box.
:p.If there is more than one status selection that starts with the same letter,
you can rotate through each of the selections by repeatedly pressing the first
letter of the status desired.  For example, if you type the letter 'A' several
times you will see :hp1.Available for Chat:ehp1., :hp1.Auto Logoff
Pending:ehp1. and :hp1.Answering Script:ehp1. appear in the status field.
:li.There are several :link reftype=hd res=2360.Quick Action Buttons:elink.
available which make it easier to perform some of the most common changes which
you might make to the node status field.
:eol.
:note.Node Status Changes are not actually written to the usernet file until
after you select the :hp1.OK:ehp1. button.

:h1 hide res=2360.Quick Action Buttons
:i2 refid=Edit.Quick Action Buttons
:p.The :hp2.Quick Action Buttons:ehp2. are used to made changes to the node
status easier and faster by avoiding using the :hp1.Node Status List Box:ehp1..
:p.There are five :hp2.Quick Action Buttons:ehp2. available.  The following is
a list of each, with more information available by clicking on the name of
the Quick Action Button below:
:parml tsize=30 break=none.
:pt.:hp2.:link reftype=hd res=2463.Force Logoff:elink.:ehp2.
:pd.This button will force a caller to be logged off the system right away
unless the caller is in a door or transferring a file.
:pt.:hp2.:link reftype=hd res=2462.Drop to DOS at Logoff:elink.:ehp2.
:pd.This button will cause the BBS to exit to the command prompt or close the
window in which it is running.  However, the BBS will wait until the caller
logs off before it exits or closes the window.
:pt.:hp2.:link reftype=hd res=2461.Drop to DOS - URGENT!:elink.:ehp2.
:pd.This button will cause the BBS to exit immediately, if possible.  If the
caller is currently in a door or performing a file transfer the command will
not be executed until the caller returns.
:pt.:hp2.:link reftype=hd res=2465.Clear Node:elink.:ehp2.
:pd.This button does not actually command the BBS to do anything, it simply
clears all of the fields in the Edit Node Status dialog.
:pt.:hp2.:link reftype=hd res=2464.Recycle:elink.:ehp2.
:pd.This button will force the node to recycle by exiting and cycling through
the board.bat/board.cmd file.
:eparml.
:note.Node Status Changes are not actually written to the usernet file until
after you select the :hp1.OK:ehp1. button.

:h1 hide res=2370.Process ID
:i2 refid=Edit.Finding the Process ID
:p.If PCBCP has launched a node and the window is currently active (visible
or not), there is a :hp2.Process ID:ehp2. that is associated with it.
:p.OS/2 assigns a unique Process ID very every session that is started and
utilitizes this ID for controlling the various sessions.
:p.If you are interested in finding out the Process ID that has been assigned
to a node, all you need to do is bring up the Edit Node Status dialog box and
then look at the title bar for the window.  The Process ID will be shown in
the title bar following the word "Pid".  An example is shown below:
:xmp.
Edit Node Status - Node 1 - Pid 1B
:exmp.
:p.The title bar is showing both the node number as well as the Pid in order to
reinforce the connection between the two.
