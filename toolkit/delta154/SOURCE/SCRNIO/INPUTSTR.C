/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#include <stdio.h>
#include <string.h>
#include <screen.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: inputstr()
*
*  - Prints the question on the screen
*  - Sets AllCaps to desired setting then calls inputall()
*/

void LIBENTRY inputstr(int X,int Y,int Limit,char *Q,char *Old,char *Ans,char *Mask,int Attribute,int HelpNum) {
  if (strlen(Q) > 0) {
    fastprint(X,Y,Q,Colors[QUESTION]);
    Input.OrgX = (char) (X + strlen(Q) + 1);
    fastprint(Input.OrgX,Y,"?",Colors[ANSWER]);
    Input.OrgX += (char) 2;
  } else {
    Input.OrgX = (char) X;
  }
  Input.AllCaps   = (bool) (Attribute & INPUT_CAPS);
  Input.AllowWrap = (bool) (Attribute & INPUT_WRAP);
  Input.CurX      = Input.OrgX;
  Input.OrgY      = (char) Y;
  Input.ScrLimit  = (char) Limit;
  Input.AnsLimit  = Limit;
  Input.Old       = Old;
  Input.Ans       = Ans;
  Input.Mask      = Mask;
  Input.Keyed     = (bool) ((Attribute & INPUT_CLEAR) != 0 ? FALSE : TRUE);
  Input.HelpNum   = HelpNum;
  inputall();
}
