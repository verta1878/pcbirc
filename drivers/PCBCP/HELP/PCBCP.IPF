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
.* Main.ipf - Information Tag Language file for the main        *
.*              sample application's help manager               *
.*                                                              *
.*==============================================================*/
:userdoc.

.*--------------------------------------------------------------*\
.*  Main window extended help panel                             *
.*      res = PANEL_MAIN                                        *
.*--------------------------------------------------------------*/
:h1 res=2100 name=PANEL_MAIN.About PCBoard Control Panel
:i1 id=aboutMain.About PCBoard Control Panel
:p.The :hp1.PCBoard Control Panel:ehp1. is a utility that can be used to monitor
and control PCBoard nodes.  In its simplest form, it can monitor what each
of your nodes are doing.  Or, as a full Control Panel, it can be set up to
automatically launch your nodes for you, monitor them for problems, close
them down and restart them automatically.
:p.Press here for a :link reftype=hd res=3000.Tutorial:elink..

.*-- Import the File menu help file --*
.im file.ipf

.*-- Import the Edit menu help file --*
.im edit.ipf

.*-- Import the Action menu help file --*
.im action.ipf

.*-- Import the Update menu help file --*
.im update.ipf

.*-- Import the Option menu help file --*
.im option.ipf

.*-- Import the Help menu help file --*
.im help.ipf

.*-- Import the Application Menu help file --*
.im menu.ipf

.*-- Import the Dialog boxes help file --*
.im dlg.ipf

:euserdoc.
