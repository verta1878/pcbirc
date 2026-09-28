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
*  Function: inputline()
*
*  - Sets AllCaps to desired setting then calls inputall()
*
*  Returns the current cursor position
*/

void LIBENTRY inputextend(int X,int Y,int ScrLimit,int AnsLimit,char *Buf,char *Mask,inputattrtype Attribute,int HelpNum) {
  Input.CurX      = (char) X;
  Input.OrgX      = (char) X;
  Input.OrgY      = (char) Y;
  Input.AllCaps   = (bool) (Attribute & INPUT_CAPS);
  Input.AllowWrap = FALSE;
  Input.ScrLimit  = (char) ScrLimit;
  Input.AnsLimit  = AnsLimit;
  Input.Old       = Buf;
  Input.Ans       = Buf;
  Input.Mask      = Mask;
  Input.Keyed     = (bool) ((Attribute & INPUT_CLEAR) != 0 ? FALSE : TRUE);
  Input.HelpNum   = HelpNum;
  inputall();
}
