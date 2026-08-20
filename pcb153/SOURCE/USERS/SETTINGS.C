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


#include "project.h"
#pragma hdrstop


void LIBENTRY setpagelength(int NumTokens) {
  char Str[5];
  int  Num;
  char *p;

  if (NumTokens) {
    p = getnexttoken();
  } else {
    Str[0] = ' ';
    p = &Str[1];
    displaypcbtext(TXT_CURPAGELEN,LFBEFORE);
    ascii(p,UsersData.PageLen);
    println(Str);
    inputfield(p,TXT_ENTERPAGELENGTH,2,FIELDLEN|NEWLINE,HLP_P,mask_numbers);
  }

  if (*p >= '0' && *p <= '9') {
    Num = atoi(p);
    if (Num != UsersData.PageLen) {
      UsersData.PageLen = (char) Num;
      checkpagelen();
      startdisplay(NOCHANGE);  /* call this now in case the length is set to 0 */
      ascii(Status.DisplayText,Num);
      displaypcbtext(TXT_PAGELENGTHSETTO,NEWLINE|LOGIT);
    }
  }
}


void LIBENTRY protfile(char Letter, bool BatchOnly) {
  char     Temp[10];
  char     X;
  int      Count;
  int      Offset;
  char     *p;
  prottype *q;

  displaycmdfile("PREPROT");

  if (displaycmdfile("PROT") == 0)
    return;

  strcpy(Temp,"   (x) ");
  printcolor(PCB_CYAN);

  if (BatchOnly && ! isprotbatch(Letter))
    Letter = 'N';

  for (Count = 0, p = &ValidProtocols[1]; Count < ValidProtocols[0]; p++, Count++) {
    X = *p;
    Offset = protoffset(X);
    q = &Protocols[Offset];
    if (q->Desc[0] == 0)
      continue;
    if (BatchOnly && ! q->Batch)
      continue;
    if (q->ErrCorrReq && ! Asy.ErrorCorrected)
      continue;
    if (X == Letter) {
      Temp[0] = '=';
      Temp[1] = '>';
    } else {
      Temp[0] = ' ';
      Temp[1] = ' ';
    }
    Temp[4] = X;
    print(Temp);
    println(q->Desc);
  }
}


void LIBENTRY setprotocol(int NumTokens) {
  char Valid[40];
  char Str[10];
  char Letter;
  char *p;
  int  Offset;

  if (Asy.ErrorCorrected)
    strcpy(Valid,ValidProtocols);
  else {
    /* remove protocols that require error corrected sessions */
    for (Valid[0] = 0, p = ValidProtocols; *p; p++)
      if (! Protocols[protoffset(*p)].ErrCorrReq)
        addchar(Valid,*p);
  }

  Letter = UsersData.Protocol;
  if  (! Asy.ErrorCorrected && (Letter == 'F' || Letter == 'G'))
    Letter = 'N';

  while (1) {
    if (NumTokens) {
      p = getnexttoken();
      NumTokens = 0;
      if (! isprotvalid(*p))
        continue;
    } else {
      p = Str;
      *p = Letter;
      *(p+1) = 0;
      newline();
      Display.NumLinesPrinted = 0;  /* start counting now */
      Display.Break = FALSE;        /* don't let them break out */
      protfile(Letter,FALSE);
      inputfield(p,TXT_DESIREDPROTOCOL,1,FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,Valid);
      Display.Break = TRUE;         /* restore the BREAK setting */
      if (*p == UsersData.Protocol || *p == 0)
        return;
    }

    if ((Offset = protoffset(*p)) != -1) {
      if (Protocols[Offset].Desc[0] != 0) {
        UsersData.Protocol = *p;
        strcpy(Status.DisplayText,Protocols[Offset].Desc);
        Status.AppendText = TRUE;
        displaypcbtext(TXT_DEFAULTPROTOCOL,NEWLINE|LFBEFORE| (Status.UserRecNo != 0 ? LOGIT : DEFAULTS));
        break;
      }
    }
  }
}
