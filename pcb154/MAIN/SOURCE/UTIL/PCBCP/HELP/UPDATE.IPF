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
.* Update.ipf - Information Tag Language file for the Update    *
.*              menu help panels.                               *
.*                                                              *
.*==============================================================*/

.*--------------------------------------------------------------*\
.*  Main Update menu                                            *
.*      res = PANEL_UPDATE                                      *
.*--------------------------------------------------------------*/
:h1 res=2510 name=PANEL_UPDATE.Update Menu
:i1 id=Update.Update Menu
:p.The :hp2.Update:ehp2. menu has an option to update the :hp1.PCBCP Node List:ehp1.
immediately, or set the frequency of automatic updates.


.*--------------------------------------------------------------*\
.*  Update menu Update Now command help panel                   *
.*      res = PANEL_UPDATENOW                                   *
.*--------------------------------------------------------------*/
:h1 res=2520 name=PANEL_UPDATENOW.Update Now
:i2 refid=Update.Update Now
:p.Selecting the :hp2.Update | Update Now:ehp2. option will refresh the :hp1.PCBCP Node
List:ehp1. immediately.  Alternatively, you can update the screen using the
keyboard by pressing the :hp2.SPACE BAR:ehp2..


.*--------------------------------------------------------------*\
.*  Update menu Set Interval command help panel                 *
.*      res = PANEL_UPDATEINTERVAL                              *
.*--------------------------------------------------------------*/
:h1 res=2530.Update Interval
:i2 refid=Update.Update Interval
:p.Selecting the :hp2.Update | Set Inteveral:ehp2. option will allow you to set
the frequency with which automatic screen updates occur for the :hp1.PCBCP Node List:ehp1.
window.
