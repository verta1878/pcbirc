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


// BC includes
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// PCBoard includes
#include <system.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <country.h>
#include <misc.h>

#ifdef DEBUG
#include "c:\memcheck\memcheck.h"
#endif

/********************************************************************
*
*  Function: inputnum()
*
*  NOTE:  use MUST define USEFLOAT if you plan to do floating point inputs
*
*  - Determines type of variable that Old is points to
*  - Converts *Old to a string in Ans[]
*  - Prints the question on the screen
*  - Calls inputall
*  - Converts Ans[] back to *Old depending on variable type
*/

void pascal inputnum(int X,int Y,int Limit,char *Q,void *Old,vartype VarType,int HelpNum) {
  char   *C;
  int    *I;
  long   *L;
#ifdef USEFLOAT
  float  *F;
  double *D;
  char    Temp[80];
#endif
  char    Ans[80];

/*  point them all to *Old  */
#pragma warn -sus
  C = (char *) Old;
  I = (int *)  Old;
  L = (long *) Old;
#pragma warn .sus

#ifdef USEFLOAT
  F = Old;
  D = Old;
#endif

  /* turn off the INSERT status prior to allowing one of these types of */
  /* input fields to be processed since they might try to push a value  */
  /* off the end of the input field (i.e. "   12", typing "1" turns the */
  /* value into "1   1" which is wrong.                                 */
  setkbdstatus(getkbdstatus() & ~INSERT);
//  if (*KbdStatus & INSERT)
//    *KbdStatus -= INSERT;

  switch(VarType) {
    case vBOOL    : Ans[0] = (*C ? 'Y' : 'N');      /* convert bool to str */
                   Ans[1] = 0; break;
    case vCHAR    : Ans[0] = *C; Ans[1]=0; break;   /* convert char  to str */
    case vBYTE    : sprintf(Ans,"%d" ,*C); break;   /* convert byte  to str */
    case vINT     : sprintf(Ans,"%d" ,*I); break;   /* convert int   to str */
    case vUNSIGNED: sprintf(Ans,"%u" ,*I); break;   /* convert int   to str */
    case vLONG    : sprintf(Ans,"%ld",*L); break;   /* convert long  to str */
    case vUNLONG  : sprintf(Ans,"%lu",*L); break;   /* convert long  to str */
    case vHEXINT  : sprintf(Ans,"%X" ,*I); break;   /* convert int   to hex */
#ifdef USEFLOAT
    case vFLOAT   : dcomma(Ans,*F);        break;
    case vFLOATD  : dcomma(Ans,*D);        break;
#endif
   default  : errorexit("(inputnum) VarType Error","",__FILE__,__LINE__);
  }

  if (strlen(Q) > 0) {
    fastprint(X,Y,Q,Colors[QUESTION]);
    Input.OrgX = X + strlen(Q)+1;
    fastprint(Input.OrgX,Y,"?",Colors[ANSWER]);
    Input.OrgX += 2;
  } else {
    Input.OrgX = X;
  }

  Input.CurX      = Input.OrgX;
  Input.OrgY      = Y;
  Input.ScrLimit  = Limit;
  Input.AnsLimit  = Limit;
  Input.Old       = Ans;
  Input.Ans       = Ans;
  Input.HelpNum   = HelpNum;
  Input.AllowWrap = FALSE;

  switch (VarType) {
    case vBOOL:   Input.AllCaps = TRUE;
                 Input.Mask    = YESNO;
                 Input.Keyed   = FALSE;
                 inputall();
                 if (Ans[0] == 0 || Ans[0] == ' ')
                   Ans[0] = 'N';
                 break;
    case vCHAR:   Input.AllCaps = TRUE;
                 Input.Mask    = ALLCHAR;
                 Input.Keyed   = FALSE;
                 inputall();
                 if (Ans[0] == 0)
                   Ans[0] = ' ';
                 break;
    case vHEXINT: Input.Mask    = ALLHEX;
                 Input.Keyed   = FALSE;
                 inputall();
                 break;
    default :    Input.Mask    = ALLNUM;
                 Input.Keyed   = FALSE;
                 inputall();
                 break;
  }

  stripall(Ans,Country.ThousandSep[0]);

  switch(VarType) {
    case vBOOL    : *C = (Ans[0]=='Y' ? 1 : 0);         /* convert str to bool  */
                   sprintf(Ans,"%c",Ans[0]);
                   break;
    case vCHAR    : *C = Ans[0];                        /* convert str to char  */
                   sprintf(Ans,"%c",Ans[0]);
                   break;
    case vBYTE    : *C = atoi(Ans);                     /* convert str to byte  */
                   sprintf(Ans,"%*d",Limit,*C);
                   break;
    case vINT     : *I = atoi(Ans);                     /* convert str to int   */
                   sprintf(Ans,"%*d",Limit,*I);
                   break;
    case vUNSIGNED: *I = (unsigned) atol(Ans);          /* convert str to int   */
                   sprintf(Ans,"%*u",Limit,*I);
                   break;
    case vHEXINT  : *I = hextoint(Ans);                 /* convert str to int   */
                   sprintf(Ans,"%*X",Limit,*I);
                   break;
    case vLONG    : *L = atol(Ans);                     /* convert str to long  */
                   sprintf(Ans,"%*ld",Limit,*L);
                   break;
    case vUNLONG  : *L = (unsigned) atol(Ans);          /* convert str to long  */
                   sprintf(Ans,"%*lu",Limit,*L);
                   break;
#ifdef USEFLOAT
    case vFLOAT   : strcpy(Temp,Ans);
                   substitute(Temp,Country.FractionSep,".",sizeof(Temp));
                   *F = atof(Temp);
                   dcomma(Temp,*F);
                   sprintf(Ans,"%*.*s",Limit,Limit,Temp);
                   break;

    case vFLOATD  : strcpy(Temp,Ans);
                   substitute(Temp,Country.FractionSep,".",sizeof(Temp));
                   *D = atof(Temp);
                   dcomma(Temp,*D);
                   sprintf(Ans,"%*.*s",Limit,Limit,Temp);
                   break;
#endif
  }
  fastprint(Input.OrgX,Y,Ans,Colors[ANSWER]);
}
