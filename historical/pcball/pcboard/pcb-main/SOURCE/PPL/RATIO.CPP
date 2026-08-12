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

#include <math.h>

#if defined(__OS2__) || defined(PCB153)
  #define qint double
#else
  #include "qint.hpp"
#endif

#pragma warn -lvc

long LIBENTRY totalbytesleft(void) {
  bool   Unlimited;
  long   CurrentBytes;
  long   ByteLimit;
  long   TempL;
  qint   TempQ;

  CurrentBytes = numbyteslessignore();

  if (Status.BytesRemaining == -1) {
    ByteLimit = 0x7FFFFFFFL;
    Unlimited = TRUE;
  } else {
    ByteLimit = Status.BytesRemaining - CurrentBytes;
    Unlimited = FALSE;
  }

  if (Status.TotalKByteLimit > 0) {
    TempL = Status.TotalKByteLimit * 1024 - UsersData.TotDnldBytes - CurrentBytes;
    if (TempL < ByteLimit) {
      ByteLimit = TempL;
      Unlimited = FALSE;
    }
  }

  if (Status.ByteRatio > 0) {
    // Equation transformation
    //
    // Ratio = (Dn * 10) / (Up + Credits)
    //
    // (Up + Credits) * Ratio
    // ----------------------  = Dn
    //       10
    //
    // The "Dn" limit, minus what's been downloaded so far, equals what's left

    TempQ  = UsersData.TotUpldBytes;
    TempQ += ((long) Status.KByteRatioCredits * 1024L);
    TempQ *= Status.ByteRatio;
    TempQ /= 10;

    TempQ -= (UsersData.TotDnldBytes + CurrentBytes);
    if (TempQ < ByteLimit) {
      ByteLimit = (TempQ < 0 ? (qint) 0L : TempQ);
      Unlimited = FALSE;
    }
  }

  if (Unlimited)
    return(-1);

  if (ByteLimit < 0)
    return(0);

  return(ByteLimit);
}


bool LIBENTRY checkratio(char *Name, unsigned long Up, unsigned long Dn, long New, long CheckRatio, int RatioTxt, int ExceededText) {
  int           Len;
  qint          Ratio;
  qint          BytesLeft;
  char          Str[40];

  if (CheckRatio == 0)
    return(FALSE);

  if (Up == 0)
    Up = 1;

  // Equation transformation
  //
  // Ratio = ((Dn + New) * 10) / Up
  //
  // Up * Ratio
  // ---------- - Dn = Bytes Left
  //   10
  //
  BytesLeft  = Up;
  BytesLeft *= CheckRatio;
  BytesLeft /= 10;
  BytesLeft -= Dn;

  if (BytesLeft < New) {
    displaypcbtext(RatioTxt,NEWLINE|LFBEFORE);

    Ratio = Dn;
    Ratio *= 10;
    Ratio /= Up;

    Len = strlen(decimal(Str,Ratio));
    sprintf(Status.DisplayText,"%*s:1",Len,decimal(Str,CheckRatio));

    displaypcbtext(TXT_RATIOLIMIT,NEWLINE);
    strcpy(Status.DisplayText,Name);
    displaypcbtext(ExceededText,NEWLINE|LFAFTER|LOGIT);
    return(TRUE);
  }
  return(FALSE);
}


bool LIBENTRY checkratio2(char *Name, double Up, double Dn, long New, long CheckRatio, int RatioTxt, int ExceededText) {
  int           Len;
  double        Ratio;
  double        BytesLeft;
  char          Str[40];

  if (CheckRatio == 0)
    return(FALSE);

  if (Up == 0)
    Up = 1;

  // Equation transformation
  //
  // Ratio = ((Dn + New) * 10) / Up
  //
  // Up * Ratio
  // ---------- - Dn = Bytes Left
  //   10
  //
  BytesLeft  = Up;
  BytesLeft *= CheckRatio;
  BytesLeft /= 10;
  BytesLeft -= Dn;

  if (BytesLeft < New) {
    displaypcbtext(RatioTxt,NEWLINE|LFBEFORE);

    Ratio = Dn;
    Ratio *= 10;
    Ratio /= Up;

    sprintf(Str,"%f",Ratio);
    Len = strlen(Str);
    sprintf(Status.DisplayText,"%f:1",Len,CheckRatio);

    displaypcbtext(TXT_RATIOLIMIT,NEWLINE);
    strcpy(Status.DisplayText,Name);
    displaypcbtext(ExceededText,NEWLINE|LFAFTER|LOGIT);
    return(TRUE);
  }
  return(FALSE);
}


static void _NEAR_ LIBENTRY ratio(char *Str, double P1, double P2) {
  if (P2 == 0)
    sprintf(Str,"%0.1f",P1);
  else
    sprintf(Str,"%0.1f",P1/P2);
}


void LIBENTRY fileratio(char *Str) {
  unsigned Up = UsersData.NumUploads; // + Status.FileRatioCredits;
  unsigned Dn = UsersData.NumDownloads;

  if (Dn > Up) {
    ratio(Str,Dn,Up);
    strcat(Str,(Up == 0 ? ":0" : ":1"));
  } else if (Dn == Up)
    strcpy(Str,(Up == 0 ? "0:0" : "1:1"));
  else {
    strcpy(Str,(Dn == 0 ? "0:" : "1:"));
    ratio(&Str[2],Up,Dn);
  }
}


void LIBENTRY byteratio(char *Str) {
  double Up = UsersData.TotUpldBytes; // + (Status.KByteRatioCredits * 1024L);
  double Dn = UsersData.TotDnldBytes;

  if (Dn > Up) {
    ratio(Str,Dn,Up);
    strcat(Str,(Up == 0 ? ":0" : ":1"));
  } else if (Dn == Up)
    strcpy(Str,(Up == 0 ? "0:0" : "1:1"));
  else {
    strcpy(Str,(Dn == 0 ? "0:" : "1:"));
    ratio(&Str[2],Up,Dn);
  }
}


unsigned LIBENTRY averagecps(long Bytes, trntype TranType) {
  int      X;
  qint     Temp;
  qint     Total;
  spectype File;

  for (X = 0, Total = 0; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    if (File.Success == TranType) {
      Temp   = File.CPS;
      Temp  *= File.Size;
      Total += Temp;
    }
  }
  return((unsigned) (Total / (qint) Bytes));
}


#pragma warn +lvc
