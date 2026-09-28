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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <system.h>
#include <misc.h>
#include <country.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

FldType *SaveFldPtr;

bool ScreenInputChanged;


/********************************************************************
*
*  Function:  addquest()
*
*  Adds a question (or field) to InpFld[] which is an array of fields
*/

void LIBENTRY addquest(FldType   *Fields,   /* Pointer to Fields Array          */
                     int       FieldNum,  /* Number of Field on Screen        */
                     vartype   VarType,   /* Type of field                    */
                     int       HelpN,     /* Help Number                      */
                     char      *InpMask,  /* Pointer to the input mask to use */
                     int       Xl,        /* X coordinate                     */
                     int       Yl,        /* Y coordinate                     */
                     int       Fl,        /* Length of input field            */
                     char      *Q,        /* Question to place on screen      */
                     void      *Fp,       /* Pointer to the field variable    */
                     cleartype AutoClear, /* Turn ON/OFF auto clear of field  */
                     int       (*Sub)(int)/* Pointer to a function (if any)   */
                     ) {

  FldType *q;

  q = &Fields[FieldNum];

  if ((q->Answer = (char *) malloc((Fl+1) + (strlen(Q)+1))) == NULL) {
    errorexit("addquest","memory error",__FILE__,__LINE__);
  }
  q->Question = &q->Answer[Fl+1]; /*Point Question to loc AFTER end of Answer*/

  strcpy(q->Question,Q);
  q->InputType = VarType;
  q->Mask      = InpMask;
  q->HelpNum   = HelpN;
  q->Xloc      = (char) Xl;
  q->Yloc      = (char) Yl;
  q->Flen      = (char) Fl;
  q->Fptr      = Fp;
  q->SubFunc   = Sub;
  q->AutoClear = AutoClear;
}

void LIBENTRY initquest(FldType *Fields, int NumFields) {
  int Count;

  for (Count = 0; Count < NumFields; Count++, Fields++)
    if ((Fields->Answer = (char *) malloc(Fields->Flen+1)) == NULL)
      errorexit("initquest","memory error",__FILE__,__LINE__);
}

/********************************************************************
*
*  Function:  initvar()
*
*  Initializes the Answer[] field with a string produced by the data pointed
*  to by *p that is of type VarType.
*/

void _NEAR_ LIBENTRY initvar(vartype VarType, /*  Type of field                       */
                             void    *p,      /*  Pointer to VAR                      */
                             int     Len,     /*  Length of field on screen           */
                             char    *Answer  /*  pass-back VAR converted to a string */
                             ) {
  char Temp[80];

  if (p != NULL) {
    switch (VarType) {
      case vBOOL    : sprintf(Answer,"%c",(*(char *)p ? 'Y' : 'N')); return;
      case vCHAR    : sprintf(Answer,"%c", *(char *)p);              return;
      case vSTR     :
      case vUPSTR   : sprintf(Answer,"%-*.*s",Len,Len,(char*)p);    return;
      case vBYTE    : sprintf(Temp,"%d" ,*(char     *)p);  break;
      case vINT     : sprintf(Temp,"%d" ,*(int      *)p);  break;
      case vHEXBYTE : sprintf(Temp,"%0*X",Len,*(char     *)p); break;
      case vHEXINT  : sprintf(Temp,"%0*X",Len,*(int      *)p); break;
      case vUNSIGNED: sprintf(Temp,"%u" ,*(unsigned *)p);  break;
      case vLONG    : sprintf(Temp,"%ld",*(long *)p);      break;
      case vUNLONG  : sprintf(Temp,"%lu",*(long *)p);      break;
#ifdef USEFLOAT
      case vFLOAT   : dcomma(Temp,*(float  *)p);
                      if (strlen(Temp) > Len)
                        substitute(Temp,Country.ThousandSep,"",sizeof(Temp));
                      break;
      case vFLOATD  : dcomma(Temp,*(double *)p);
                      if (strlen(Temp) > Len)
                        substitute(Temp,Country.ThousandSep,"",sizeof(Temp));
                      break;
#endif
#ifdef USEDATE
      case vDATE    : strcpy(Answer,juliantodate(*(int *)p));
                      countrydate(Answer);
                      return;
#endif
      default  : errorexit("(initvar) VarType Error","",__FILE__,__LINE__);
    }

    sprintf(Answer,"%*.*s",Len,Len,Temp);
  }
}


/********************************************************************
*
*  Function:  makescreen()
*
*  Does the initial painting of the screen before actual input can be performed
*/

void _NEAR_ LIBENTRY makescreen(FldType   *Ptr,
                                int       LastField,
                                char      *H1,
                                char      *H2,
                                cleartype Clr) {

  int  X;
  int  LocX;
  int  LocY;

  FldType *q;

  setcursor(CUR_BLANK);
  if (Clr == CLEAR)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  generalscreen(H1,H2);

  for (X = 0; X <= LastField; X++) {
    q = &Ptr[X];
    initvar(q->InputType, q->Fptr, q->Flen, q->Answer);
    LocX = q->Xloc;
    LocY = q->Yloc;

    fastprint(q->Xloc,LocY,q->Question,Colors[QUESTION]);
    LocX += strlen(q->Question) + 1;

    if (q->Fptr != NULL) {
      fastprint(LocX,LocY,":",Colors[DISPLAY]);
      fastprint(LocX+2,LocY,q->Answer,Colors[ANSWER]);
    }
  }
  setcursor(CUR_NORMAL);
  showkeystatus();
}

/********************************************************************
*
*  Function:  transfer()
*
*  Transfers data back from a string Prev[] to the pointer *p according to the
*  variable type VarType.
*/

void _NEAR_ LIBENTRY transfer(FldType *Ptr, int LastField) {
  FldType *q;
  void    *p;
  int     X;
  #ifdef USEFLOAT
    char Temp[80];
  #endif


  for (X = 0; X <= LastField; X++) {
    q = &Ptr[X];
    p = q->Fptr;

    if (q->Fptr != NULL) {
      switch (q->InputType) {
        case vSTR     :
        case vUPSTR   : maxstrcpy((char *)p,q->Answer,q->Flen+1);
                        removespaces((char *)p,q->Flen);
                        break;
        case vCHAR    : *(char     *)p = q->Answer[0];    break;
        case vBOOL    : *(bool     *)p = (bool) (q->Answer[0] == 'Y' ? TRUE : FALSE); break;
        case vBYTE    : *(char     *)p = (char) atoi(q->Answer); break;
        case vINT     : *(int      *)p = atoi(q->Answer); break;
        case vHEXBYTE : *(char     *)p = hextoint(q->Answer); break;
        case vHEXINT  : *(int      *)p = hextoint(q->Answer); break;
        case vUNSIGNED: *(unsigned *)p = (unsigned) atol(q->Answer); break;
        case vLONG    : *(long     *)p = atol(q->Answer); break;
        case vUNLONG  : *(long     *)p = (unsigned long) atol(q->Answer); break;
#ifdef USEFLOAT
        case vFLOAT   : strcpy(Temp,q->Answer);
                        stripall(Temp,Country.ThousandSep[0]);
                        substitute(Temp,Country.FractionSep,".",sizeof(Temp));
                        *(float    *)p = atof(Temp);
                        break;
        case vFLOATD  : strcpy(Temp,q->Answer);
                        stripall(Temp,Country.ThousandSep[0]);
                        substitute(Temp,Country.FractionSep,".",sizeof(Temp));
                        *(double   *)p = atof(Temp);
                        break;
#endif
#ifdef USEDATE
        case vDATE : uncountrydate(q->Answer);
                     *(int *)p = datetojulian(q->Answer);
                     countrydate(q->Answer);
                     break;
#endif
      }
    }
  }
}


/********************************************************************
 *
 *  Function:  updatefield()
 *
 *  Reprints the value of a field on the screen to indicate to the user that
 *  the input has been accepted.
 */

void _NEAR_ LIBENTRY updatefield(int X, FldType *q) {
  char   Temp[80];
  unsigned long pl;
#ifdef USEFLOAT
  double pv;
  char   Temp2[80];
#endif
#ifdef USEDATE
  int    pi;
#endif

  switch (q->InputType) {
    case vINT     :
    case vLONG    : pl = atol(q->Answer);
                    sprintf(Temp,"%*ld",q->Flen,pl);
                    break;
    case vHEXBYTE :
    case vHEXINT  : pl = hextoint(q->Answer);
                    sprintf(Temp,"%0*lX",q->Flen,pl);
                    break;
    case vBYTE    :
    case vUNSIGNED:
    case vUNLONG  : pl = atol(q->Answer);
                    sprintf(Temp,"%*lu",q->Flen,pl);
                    break;
#ifdef USEFLOAT
    case vFLOAT   :
    case vFLOATD  : strcpy(Temp,q->Answer);
                    stripall(Temp,Country.ThousandSep[0]);
                    substitute(Temp,Country.FractionSep,".",sizeof(Temp));
                    pv = atof(Temp);
                    dcomma(Temp2,pv);
                    if (strlen(Temp2) > q->Flen)
                      substitute(Temp2,Country.ThousandSep,"",sizeof(Temp2));
                    sprintf(Temp,"%*.*s",q->Flen,q->Flen,Temp2);
                    break;
#endif

#ifdef USEDATE
    case vDATE    : formatdate(q->Answer);
                    uncountrydate(q->Answer);
                    pi = datetojulian(q->Answer);
                    strcpy(Temp,juliantodate(pi));
                    countrydate(Temp);
                    strcpy(q->Answer,Temp);
                    break;
#endif

    default      : maxstrcpy(Temp,q->Answer,sizeof(Temp));
  }
  fastprint(X,q->Yloc,Temp,Colors[ANSWER]);
  maxstrcpy(q->Answer,Temp,q->Flen+1);
}


/********************************************************************
*
*  Function:  readscrn()
*
*  Controls the making of the screen, the initialization of variables, the
*  ordering of the input according to the requests of the user and the
*  subsequent transfering of the screen/input data back to the proper
*  variables.
*/

int LIBENTRY readscrn(FldType   *Ptr,
                    int       LastField,
                    int       StartField,
                    char      *H1,
                    char      *H2,
                    int       MoveFactor,
                    cleartype Clr) {

  int X;
  int Code;
  int SaveFlags;
  int SaveOrgX;
  FldType *q;
  char Temp[80];
  bool Changed;

  Changed = FALSE;
  makescreen(Ptr,LastField,H1,H2,Clr);

  X = StartField;
  while (X <= LastField) {
    q = &Ptr[X];
    if (q->SubFunc != NULL) {
      SaveFldPtr = q;
      Code = q->SubFunc(TRUE);   /* call to sub indicating BEFORE */
      if (Code == 1)
        makescreen(Ptr,LastField,H1,H2,Clr);
    }

    strcpy(Temp,q->Answer);

    Input.AllCaps   = (bool) (q->InputType == vUPSTR || q->InputType == vBOOL || q->InputType == vCHAR);
    Input.OrgX      = (char) (q->Xloc + strlen(q->Question) + 3);
    Input.CurX      = Input.OrgX;
    Input.OrgY      = q->Yloc;
    Input.ScrLimit  = q->Flen;
    Input.AnsLimit  = q->Flen;
    Input.Old       = q->Answer;
    Input.Ans       = q->Answer;
    Input.Mask      = q->Mask;
    Input.Keyed     = (bool) (! q->AutoClear);
    Input.HelpNum   = q->HelpNum;

    /* if set to AutoClear, then turn off the INSERT mode */
    if (q->AutoClear)
      setkbdstatus((uint) (getkbdstatus() & ~INSERT));

    inputall();

    stripright(Temp,' ');
    Changed |= (bool) (strcmp(Temp,q->Answer) != 0);

    if (q->SubFunc != NULL) {
      transfer(Ptr,LastField);
      SaveFldPtr = q;
      SaveFlags  = KeyFlags;
      SaveOrgX   = Input.OrgX;
      Code = q->SubFunc(FALSE); /* call to sub indicating AFTER */
      switch (Code) {
        case -1: KeyFlags = NOTHING;   break;
        case  2: makescreen(Ptr,LastField,H1,H2,Clr);
                 SaveFlags = NOTHING;
                 break;
        case  1: makescreen(Ptr,LastField,H1,H2,Clr);     /* fall thru */
        case  0: KeyFlags = SaveFlags; break;
      }
      if (SaveFlags >= FLAG1)
        KeyFlags = NOTHING;
      Input.OrgX = (char) SaveOrgX;
    }
    updatefield(Input.OrgX,q);

    switch (KeyFlags) {
      case NOTHING   : /* do nothing */ break;
      case SHIFTTAB  :
      case UP        : X -= MoveFactor;
                       if (X < 0)
                         X = LastField;
                       break;
      case RET       :
      case TAB       :
      case DN        : X += MoveFactor;
                       if (X > LastField)
                         X = 0;
                       break;
      default        : StartField = X;
                       X = LastField + 1;
                       break;
    }
  }

  ScreenInputChanged = Changed;
  transfer(Ptr,LastField);
  return(StartField);
}

/********************************************************************
*
*  Function:  freescrn()
*
*  Frees up all memory allocated to the screen
*/

void LIBENTRY freescrn(FldType Fields[], int LastField) {
  FldType *q;

  for (q = Fields; q <= &Fields[LastField]; q++)
    free(q->Answer);

  free(Fields);
}

void LIBENTRY freeanswers(FldType *Fields, int NumFields) {
  int Count;

  for (Count = 0; Count < NumFields; Count++, Fields++)
    free(Fields->Answer);
}
