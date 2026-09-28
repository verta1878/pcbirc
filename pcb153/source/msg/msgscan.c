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

#include "messages.h"

#define IDXBUFSIZE  128
#define MAXMSGS     256
#define MAXCOLUMNS  12

typedef struct {
  bool AllConf;
  bool WaitConf;
  bool SelectConf;
  bool Since;
  bool Forward;
  bool Quick;
  bool SkipZero;
} msgscantype;

typedef struct {
  long Number;
  char Read;
} msgptrtype;


char static Columns[MAXCOLUMNS] = { 0, 0, 31, 21, 15, 12, 10, 9, 7, 7, 6, 5 };

static void _NEAR_ LIBENTRY printmsgnumbers(msgptrtype *MsgNumbers, long HighNum) {
  int  Count;
  int  Width;
  int  NumColumns;
  msgptrtype *p;
  char Str[15];

  lascii(Str,HighNum);
  Width = strlen(Str)+2;
  if (Width > MAXCOLUMNS)
    return;

  NumColumns = Columns[Width];

  for (p = MsgNumbers, Count = NumColumns; p->Number != 0; p++, Count--) {
    lascii(Str,p->Number);
    if (Count == 0) {
      newline();
      print("                ");
      Count = NumColumns;
    }
    padstr(Str,' ',Width);
    Str[Width-2] = p->Read;
    print(Str);
    if (Display.AbortPrintout)
      return;
  }
  newline();
}


static void _NEAR_ LIBENTRY showconfname(msgscantype *Scan) {
  int  X;
  char Str[80];

  if (Scan->Quick) {
    printdefcolor();
    sprintf(Str,"%5u %s",Status.Conference,Status.CurConf.Name);
    print(Str);
    X = 66 - strlen(Str);
    if (X > 0) {
      printcolor(0x08);
      for (; X > 0; X--)
        print(X & 1 ? "." : " ");
    }
  } else {
    makeconfstr(Str,Status.Conference);
    displaypcbtext(TXT_SCANNING,LFBEFORE);
    println(Str);
  }
}


static void _NEAR_ LIBENTRY shownumber(unsigned Num) {
  char Str[10];

  printcolor(Num == 0 ? 0x07 : 0x0F);
  sprintf(Str,"%6u",Num);
  print(Str);
}


static long _NEAR_ LIBENTRY scanconference(msgscantype *Scan) {
  bool            Yours;
  bool            NeedToShowConf;
  long            X;
  long            End;
  int             Move;
  int             CurPos;                 // should CurPos and NewPos be longs?
  int             NewPos;
  unsigned        NumVisible;
//unsigned        NumGeneric;
  unsigned        NumToYou;
  unsigned        NumFromYou;
  msgstattype     MsgStatus;
  readaccesstype  Access;
  msgptrtype      *ToPtr;
  msgptrtype      *FromPtr;
  newindextype    *IdxPtr;
  long            LastMsgRead;
  char            Str[80];
  msgextendtype   ExtHeader;
  msgbasetype     MsgBase;
  newindextype    IdxBuffer[IDXBUFSIZE];
  msgptrtype      ToMsgs[MAXMSGS];
  msgptrtype      FromMsgs[MAXMSGS];

  checkstatus();
  if (Display.AbortPrintout)
    return(0);

  if (OldIndex || Status.CurConf.OldIndex)
    updatemsgindex();

  if (openmessagebase(Status.Conference,&Status.CurConf,&MsgBase,READONLY) == -1)
    return(0);

  memset(ToMsgs,0,sizeof(ToMsgs));
  memset(FromMsgs,0,sizeof(FromMsgs));

  NeedToShowConf = TRUE;

  #if defined(PCBCOMM) && defined(COMM)
  if (Status.TerseMode) {
  } else {
  #endif

  // Show the conference name now, unless SkipZero is used.
  // SkipZero doesn't show the name until the scan is complete because we
  // don't know if there are any messages yet.
  if (! Scan->SkipZero) {
    showconfname(Scan);
    NeedToShowConf = FALSE;
  }
  #if defined(PCBCOMM) && defined(COMM)
  }
  #endif

  NumToYou   = 0;
  NumFromYou = 0;
  NumVisible = 0;
//NumGeneric = 0;
  ToPtr      = ToMsgs;
  FromPtr    = FromMsgs;
  CurPos     = -1;

  LastMsgRead = fixlmrpointer(Status.Conference,MsgBase.Stats.LowMsgNum,MsgBase.Stats.HighMsgNum);

  if (Scan->Forward) {
    Move = 1;
    End  = (MsgBase.Stats.HighMsgNum-MsgBase.Stats.LowMsgNum)+1;
    X    = (Scan->Since ? LastMsgRead-MsgBase.Stats.LowMsgNum+1 : 0);
  } else {
    Move = -1;
    X    = (MsgBase.Stats.HighMsgNum-MsgBase.Stats.LowMsgNum);
    End  = (Scan->Since ? LastMsgRead-MsgBase.Stats.LowMsgNum : -1);
  }

  showactivity(ACTBEGIN);

  for (; X != End; X += Move, IdxPtr += Move) {
    showactivity(ACTSHOW);
    if (Display.AbortPrintout)
      goto exit;

    NewPos = (int) (((long) X*sizeof(newindextype)) / sizeof(IdxBuffer));
    if (NewPos != CurPos) {
      IdxPtr = &IdxBuffer[(int)(X % IDXBUFSIZE)];
      CurPos = NewPos;
      doslseek(MsgBase.Index,(long)CurPos*sizeof(IdxBuffer),SEEK_SET);
      if (readcheck(MsgBase.Index,IdxBuffer,sizeof(IdxBuffer)) == (unsigned) -1)
        break;
    }

    if (IdxPtr->Offset > 0) {     //lint !e644  IdxPtr is initialzed by NewPos != CurPos
      MsgStatus = getmsgstatus(IdxPtr->Status);
      #pragma warn -stv
      Access    = readstatus(IdxPtr,MsgStatus);
      #pragma warn +stv

      if (Access.OkayToRead) {
        if (Access.Msg.ToYou) {
//        if (Access.Msg.Generic)
//          NumGeneric++;

          if (Access.Msg.List) {
            Yours = FALSE;

            // jump to the message, plus skip over the header, go for the body
            dosfseek(&MsgBase.Msgs,IdxPtr->Offset+sizeof(msgheadertype),SEEK_SET);

            while (1) {
              if (dosfread(&ExtHeader,sizeof(msgextendtype),&MsgBase.Msgs) != sizeof(msgextendtype))
                break;

              if (ExtHeader.Ident != EXTHDRID)
                break;

              if (carbonedtoyou((char*) &ExtHeader) != NULL) {
                Yours = TRUE;
                break;
              }
            }
            if (! Yours)
              continue;   // return to top of for() loop
          }
          if ((! Scan->Quick) && ToPtr < &ToMsgs[MAXMSGS-1]) {
            ToPtr->Number = IdxPtr->Num;
            ToPtr->Read   = (MsgStatus.Read ? '+' : ' ');
          }
          ToPtr++;
          NumToYou++;
        }
        if (Access.Msg.FromYou) {
          if ((! Scan->Quick) && FromPtr < &FromMsgs[MAXMSGS-1]) {
            FromPtr->Number = IdxPtr->Num;
            FromPtr->Read   = (MsgStatus.Read ? '+' : ' ');
          }
          FromPtr++;
          NumFromYou++;
        }

        NumVisible++;
        checkstatus();

        // if this is the first message found, and if we're in SkipZero mode,
        // then display the conference name now
        if (NeedToShowConf) {
          // if we are only showing msgs in conferences with personal mail
          // then don't show the conference name unless the ToPtr is changed
          // to indicate that we actually have mail waiting
          if (Scan->WaitConf && ToPtr == ToMsgs)
            continue;

          showactivity(ACTEND);
          if (! Display.AbortPrintout) {
            showconfname(Scan);
            NeedToShowConf = FALSE;
          }
          showactivity(ACTBEGIN);
        }
      }
    }
  }

exit:
  closemessagebase(&MsgBase);
  checkdisplaystatus();
  showactivity(ACTEND);

//NumToYou   = (unsigned) (ToPtr - ToMsgs);
//NumFromYou = (unsigned) (FromPtr - FromMsgs);

  #if defined(PCBCOMM) && defined(COMM)
  if (Status.TerseMode) {
    sprintf(Str,"[%-13.13s%5u%5u%5u%5u]",
            Status.CurConf.Name,
            Status.Conference,
            NumFromYou,
            NumToYou,
            NumVisible);
    tersemodechecksum(Str,33);
  } else {
  #endif
    if (Scan->SkipZero) {
      if (NumVisible == 0 || (Scan->WaitConf && NumToYou == 0))
        return(0);
    }

    if (NeedToShowConf)     // in case scan was aborted, do we still need to
      showconfname(Scan);   // show the conference name?

    // if any NON-generic messages were found, then set the mail flag
//  if (NumToYou > NumGeneric)
//    setbit(&ConfReg[MFL],Status.Conference);

    if (Scan->Quick) {
      printcolor(PCB_WHITE);
      shownumber(NumToYou);
      shownumber(NumVisible);
      newline();
    } else {
      displaypcbtext(TXT_MSGSFORYOU,DEFAULTS);
      printdefcolor();
      if (NumToYou != 0)
        printmsgnumbers(ToMsgs,MsgBase.Stats.HighMsgNum);
      else
        displaypcbtext(TXT_NONE,NEWLINE);

      displaypcbtext(TXT_MSGSFROMYOU,DEFAULTS);
      printdefcolor();
      if (NumFromYou != 0)
        printmsgnumbers(FromMsgs,MsgBase.Stats.HighMsgNum);
      else
        displaypcbtext(TXT_NONE,NEWLINE);

      displaypcbtext(TXT_TOTALMSGSFOUND,DEFAULTS);
      printcolor(PCB_RED);
      Str[0] = ' ';
      lascii(&Str[1],NumVisible);
      println(Str);
    }
  #if defined(PCBCOMM) && defined(COMM)
  }
  #endif
  return(NumVisible);
}




void LIBENTRY messagescan(int NumTokens) {
  unsigned short ConfNum;
  unsigned       X;
  char          *p;
  long           MsgsFound;
  msgscantype    Scan;
  char           Str[10];

  memset(&Scan,FALSE,sizeof(Scan));
  Scan.Quick = PcbData.QuickScan;

  if (NumTokens == 0) {
    Str[0] = 0;
    inputfield(Str,TXT_MSGSCANPROMPT,8,YESNO|UPCASE|STACKED|NEWLINE|LFBEFORE,HLP_Y,mask_scan);
    if (Str[0] == 0)
      return;
    NumTokens = tokenize(Str);
  }

  for (;  NumTokens; NumTokens--) {
    p = getnexttoken();
    if (*(p+1) == 0) {
      switch (*p) {
        case 'A': Scan.SelectConf = TRUE;
                  Scan.AllConf    = TRUE;
                  break;
        case '*':
        case 'S': Scan.Since   = TRUE;
                  Scan.Forward = TRUE;
                  break;
        case 'C': Scan.SelectConf = FALSE;
                  Scan.AllConf    = FALSE;
                  break;
        case 'Q': Scan.Quick   = TRUE;
                  break;
        case 'L': Scan.Quick   = FALSE;
                  break;
        case '+': Scan.Forward = TRUE;
                  break;
        case '-': Scan.Forward = FALSE;
                  break;
        case 'W': Scan.WaitConf = TRUE;
                  Scan.AllConf  = TRUE;
                  Scan.Since    = TRUE;
                  Scan.Forward  = TRUE;
                  /* fall thru */
        case 'Z': Scan.SkipZero = TRUE;
                  break;
      }
    } else {
      switch (*p) {
        case 'A': if (strcmp(p,"ALL") == 0) {
                    Scan.AllConf    = TRUE;
                    Scan.SelectConf = FALSE;
                  }
                  break;
        case 'C': switch (*(p+1)) {
                    case '-': Scan.AllConf = FALSE; Scan.SelectConf = FALSE; Scan.Forward = FALSE;  break;
                    case '+': Scan.AllConf = FALSE; Scan.SelectConf = FALSE; Scan.Forward = TRUE;   break;
                  }
                  break;
      }
    }
  }

  displaypcbtext(TXT_ABORTKEYS,NEWLINE|LFBEFORE);
  startdisplay(NOCHANGE);

  if (Scan.Quick) {
    displaypcbtext(TXT_SCANHEADER1,NEWLINE|LFBEFORE|NOTBLANK);
    displaypcbtext(TXT_SCANHEADER2,NEWLINE|NOTBLANK);
    displaypcbtext(TXT_SCANHEADER3,NEWLINE|NOTBLANK);
  }

  MsgsFound = 0;

  if (Scan.AllConf) {
    ConfNum = Status.Conference;
    for (X = 0; X < PcbData.NumAreas && ! Display.WasAborted; X++) {
      if (X != ConfNum) {
        if (Status.CurSecLevel < PcbData.UserLevels[SEC_J])
          continue;
        if (Scan.SelectConf && ! isset(&ConfReg[USR],X))
          continue;
        if (! isregistered(ConfReg,X))
          continue;
      }

      if (Scan.WaitConf && ! isset(&ConfReg[MFL],X))
        continue;

      getconfrecord(X,&Status.CurConf);
      if (Status.CurConf.Name[0] != 0 && Status.CurConf.MsgFile[0] != 0) {
        Status.Conference = (unsigned short) X;
        confdetails(NOUPDATE);
        MsgsFound += scanconference(&Scan);
        setbit(&ConfFlags[SCANNED],X);
      }
      if (Display.AbortPrintout)
        break;
    }
    Status.Conference = ConfNum;
    getconfrecord(Status.Conference,&Status.CurConf);
    confdetails(NOUPDATE);
  } else {
    MsgsFound = scanconference(&Scan);
    setbit(&ConfFlags[SCANNED],Status.Conference);
  }

  if (Scan.SkipZero && MsgsFound == 0)
    displaypcbtext(TXT_NOMAILFOUND,NEWLINE);
}


void LIBENTRY scanmail(char *Command, bool LoginScan) {
  int      NumTokens;
  unsigned X;

  if (*Command != NoChar) {
    strcat(Command," S");  /* default to "SINCE" for scan */

    if (PcbData.ScanAll && LoginScan)   /* are they just logging on now? */
      strcat(Command," A");

    NumTokens = tokenize(Command);
    messagescan(NumTokens);
  }

  /* if it's set to SCAN ALL conferences by default - then regardless of  */
  /* whether or not they did in fact scan the conference - let's turn the */
  /* 'conference SCANNED flag' on so that we don't ask them again if they */
  /* want to scan for messages when the join another conference in their  */
  /* current scan list                                                    */

  if (PcbData.ScanAll)
    for (X = 0; X < PcbData.NumAreas; X++)
      if (isset(&ConfReg[USR],X))
        setbit(&ConfFlags[SCANNED],X);

}


#if defined(PCBCOMM) && defined(COMM)
void LIBENTRY tersemodescanmail(void) {
  msgscantype Scan;

  Scan.SelectConf = FALSE;
  Scan.AllConf    = FALSE;
  Scan.Since      = TRUE;
  Scan.Forward    = TRUE;
  Scan.Quick      = TRUE;

  scanconference(&Scan);     //lint !e534
  setbit(&ConfFlags[SCANNED],Status.Conference);
}
#endif
