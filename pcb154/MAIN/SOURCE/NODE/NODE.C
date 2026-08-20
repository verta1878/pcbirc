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


/********************************************************************
*
*  Function:  getnodecommand()
*
*  Desc    :  Displays the usernet.dat file, asks the user for a number which
*             it places in *Str.
*
*  Returns :  Returns -1 if the string was empty, otherwise 0.
*/

static int _NEAR_ LIBENTRY getnodecommand(char *Str, int PcbTextNum) {
  displayusernet(0);
  Str[0] = 0;
  inputfield(Str,PcbTextNum,5,NEWLINE|LFBEFORE|FIELDLEN,HLP_CHAT,mask_numbers);
  return(Str[0] == 0 ? -1 : 0);
}


#pragma warn -par
void LIBENTRY displayusernet(int NumTokens) {
  bool        ShowExtended;
  bool        AlreadyShown;
  int         Count;
  int         TextNum;
  char        *Flags;
  char        *p;
  DOSFILE     File;
  nodetype    Node;
  pcbtexttype Buf;
  char        Str[100];

  if (UserNetFile <= 0 || ! PcbData.Network)
    return;

  if ((Flags = (char *) bmalloc(USERNETFLAGSIZE)) == NULL)
    return;

  if (dosfopen(PcbData.NetFile,OPEN_READ|OPEN_DENYNONE,&File) == -1) {
    bfree(Flags);
    return;
  }

  ShowExtended = FALSE;
  if (Status.CurSecLevel >= PcbData.SysopSec[SEC_11]) {
    if (NumTokens != 0) {
      p = getnexttoken();
      if (*p == 'X')
        ShowExtended = TRUE;
    }
  }

  startdisplay(FORCECOUNTLINES);
  displaypcbtext(TXT_USERNETHEADER,NEWLINE|LFBEFORE);
  displaypcbtext(TXT_USERNETUNDERLINE,NEWLINE);
  printcolor(PCB_CYAN);

  dossetbuf(&File,4096);
  dosrewind(&File);
  dosfseek(&File,USERNETFLAGS,SEEK_SET);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  dosfread(Flags,USERNETFLAGSIZE,&File);  /*lint !e534 */

  for (Count = 0; Count < PCB_MAXNODES; Count++) {
    if (! isset(Flags,Count))
      continue;

    dosfseek(&File,((long) Count * sizeof(nodetype)) + USERNETSTART,SEEK_SET);

    #ifdef DEBUG
      mc_check_buffers();
    #endif

    if (dosfread(&Node,sizeof(nodetype),&File) != sizeof(nodetype))
      break;

    switch (Node.Status) {
      case 0              : continue;  /* don't do anything if node is down */
      case HANDLEMAIL     : TextNum = TXT_HANDLINGMAIL;     break;
      case RUNONCONNECT   : /* use NOCALLER until PCBTEXT is updated */
      case NOCALLER       : TextNum = TXT_NOCALLER;         break;
      case AVAILABLE      : TextNum = TXT_AVAILABLE;        break;
      case INADOOR        : TextNum = TXT_INADOOR;          break;
      case OUTINDOS       : TextNum = TXT_OUTTODOS;         break;
      case CHATWITHSYSOP  : TextNum = TXT_CHATWITHSYSOP;    break;
      case ANSWERSCRIPT   : TextNum = TXT_ANSWERSCRIPT;     break;
      case ENTERMESSAGE   : TextNum = TXT_ENTERMESSAGE;     break;
      case GROUPCHAT      : TextNum = TXT_GROUPCHAT;        break;
      case LOGOFFPENDING  : TextNum = TXT_LOGOFFPENDING;    break;
      case PAGINGSYSOP    : TextNum = TXT_PAGINGSYSOP;      break;
      case RECYCLEBBS     : TextNum = TXT_RECYCLEBBS;       break;
      case FILEVIEW       :
      case TRANSFER       : TextNum = TXT_TRANSFER;         break;
      case NODEMESSAGE    : TextNum = TXT_RCVDMESSAGE;      break;
      case UNAVAILABLE    : TextNum = TXT_UNAVAILABLE;      break;
      case DROPDOSDELAYED : TextNum = TXT_DROPDOSDELAYED;   break;
      case DROPDOSNOW     : TextNum = TXT_DROPDOSNOW;       break;
      case LOGINTOSYSTEM  : TextNum = TXT_LOGINTOSYSTEM;    break;
      case RUNNINGEVENT   : TextNum = TXT_RUNNINGEVENT;     break;
      default             : continue;
    }

    getpcbtext(TextNum,&Buf);
    sprintf(Str,"%4d   %-23.23s ",Count+1,Buf.Str);
    print(Str);

    AlreadyShown = FALSE;
    if (Node.Operation[0] != 0 &&
        (Node.Status == INADOOR   ||
         Node.Status == RUNNINGEVENT ||
         Node.Status == RUNONCONNECT ||
         Node.Status == NOCALLER)) {
      Node.Operation[sizeof(Node.Operation)-1] = 0;
      println(Node.Operation);
      AlreadyShown = TRUE;
    } else if ((Node.Status != NOCALLER && Node.Status != RUNONCONNECT) && Node.Name[0] != 0) {
      if (Node.Name[0] == 0)
        Str[0] = 0;
      else if (PcbData.IncludeCity && Node.City[0] != 0)
        sprintf(Str,"%s (%s)",Node.Name,Node.City);
      else
        strcpy(Str,Node.Name);
      Str[48] = 0;
      println(Str);
    } else newline();

    if (ShowExtended && Node.Operation[0] != 0 && ! AlreadyShown) {
      memset(Str,' ',31); Str[31] = 0;
      print(Str);
      Node.Operation[sizeof(Node.Operation)-1] = 0;
      println(Node.Operation);
    }
  }

  checkdisplaystatus();
  dosfclose(&File);
  bfree(Flags);
}
#pragma warn +par


/********************************************************************
*
*  Function:  checknodestatus()
*
*  Desc    :  Takes a string (NodeStr) as an argument - verifies that it is
*             a valid node number and then checks to see if the node is online
*
*  Returns :  0xFFFF if NodeStr is invalid, the NodeNum is invalid or the node
*             is not online - otherwise it returns the Node Number
*/

static unsigned _NEAR_ LIBENTRY checknodestatus(char *NodeStr, nodetype *Buf) {
  unsigned  NodeNum;

  if (! alldigits(NodeStr))
    return(0xFFFF);

  NodeNum = atoi(NodeStr);
  if (NodeNum == 0 || NodeNum > PCB_MAXNODES || NodeNum == PcbData.NodeNum) {
    displaypcbtext(TXT_INVALIDENTRY,NEWLINE|LFBEFORE);
    return(0xFFFF);
  }

  doslseek(UserNetFile,((long) (NodeNum-1) * sizeof(nodetype))+USERNETSTART,SEEK_SET);
  if (readcheck(UserNetFile,Buf,sizeof(nodetype)) == (unsigned) -1)
    return(0xFFFF);
  return(NodeNum);
}


unsigned LIBENTRY foundinusernet(char *Name, char *Alias) {
  int       Count;
  int       NodeNum;
  char     *NodeName;
  char      Flags[USERNETFLAGSIZE];
  DOSFILE   File;
  nodetype  Node;

  if (! PcbData.Network || UserNetFile <= 0)
    return(0);

  File.handle = UserNetFile;
  if (dosfopen("",OPEN_READ|OPEN_DENYNONE|OPEN_ALREADY,&File) == -1)
    return(0);

  NodeNum = PcbData.NodeNum - 1;
  dossetbuf(&File,4096);
  dosrewind(&File);
  dosfseek(&File,USERNETFLAGS,SEEK_SET);

  dosfread(Flags,USERNETFLAGSIZE,&File);    /*lint !e534 */

  for (Count = 0; Count < PCB_MAXNODES; Count++) {
    if (Count == NodeNum || ! isset(Flags,Count))
      continue;

    dosfseek(&File,((long) Count * sizeof(nodetype)) + USERNETSTART,SEEK_SET);

    if (dosfread(&Node,sizeof(nodetype),&File) != sizeof(nodetype))
      goto notfound;

    NodeName = (Node.Status == LOGINTOSYSTEM ? Node.Operation : Node.Name);

    if (strcmp(Name,NodeName) == 0)
      goto found;

    if (Alias != NULL && Alias[0] != 0 && strcmp(Alias,NodeName) == 0)
      goto found;
  }

notfound:
  dosfclose(&File);
  return(0);

found:
  dosfclose(&File);
  return(Count+1);
}


#ifdef FIDO
bool LIBENTRY okaytohandlemail(void) {
  bool     Okay;
  int      Count;
  int      NodeNum;
  char     *Flags;
  DOSFILE  File;
  nodetype Node;

  if (! PcbData.Network || UserNetFile <= 0)
    return(TRUE);

  if ((Flags = (char *) bmalloc(USERNETFLAGSIZE)) == NULL)
    return(TRUE);

  File.handle = UserNetFile;
  if (dosfopen("",OPEN_READ|OPEN_DENYNONE|OPEN_ALREADY,&File) == -1) {
    bfree(Flags);
    return(TRUE);
  }

  NodeNum = PcbData.NodeNum - 1;
  dossetbuf(&File,8192);
  dosrewind(&File);
  dosfseek(&File,USERNETFLAGS,SEEK_SET);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  dosfread(Flags,USERNETFLAGSIZE,&File);    /*lint !e534 */

  for (Count = 0, Okay = TRUE; Count < PCB_MAXNODES; Count++) {
    if (Count == NodeNum || ! isset(Flags,Count))
      continue;

    dosfseek(&File,((long) Count * sizeof(nodetype)) + USERNETSTART,SEEK_SET);

    #ifdef DEBUG
      mc_check_buffers();
    #endif

    if (dosfread(&Node,sizeof(nodetype),&File) != sizeof(nodetype))
      break;

    if (Node.Status == HANDLEMAIL) {
      Okay = FALSE;
      break;
    }
  }

  bfree(Flags);
  dosfclose(&File);

  if (Okay) {
    writeusernetstatus(HANDLEMAIL,NULL);
    return(TRUE);
  }

  return(FALSE);
}
#endif  /* FIDO */


void LIBENTRY logoffcommand(int NumTokens) {
  unsigned NodeNum;
  char     *p;
  char     Str[60];
  nodetype Buf;

  if (NumTokens)
    p = getnexttoken();
  else {
    p = Str;
    *p = 255;  /* it will ignore this and then ask for input */
  }

  while (1) {
    if ((NodeNum = checknodestatus(p,&Buf)) != 0xFFFF) {
      Buf.Status = LOGOFFPENDING;
      updateusernetrecord(NodeNum,&Buf);
      sprintf(Str,"Forced node %d to %s",NodeNum,"logoff");
      writelog(Str,SPACERIGHT);
      return;
    }

    if (getnodecommand(Str,TXT_NODENUMTOLOGOFF) == -1)
      break;
    p = Str;
  }
}


void LIBENTRY recyclecommand(int NumTokens) {
  unsigned NodeNum;
  char     *p;
  char     Str[60];
  nodetype Buf;

  if (NumTokens)
    p = getnexttoken();
  else {
    p = Str;
    *p = 255;  /* it will ignore this and then ask for input */
  }

  while (1) {
    if ((NodeNum = checknodestatus(p,&Buf)) != 0xFFFF) {
      Buf.Status = RECYCLEBBS;
      updateusernetrecord(NodeNum,&Buf);
      sprintf(Str,"Forced node %d to %s",NodeNum,"recycle");
      writelog(Str,SPACERIGHT);
      return;
    }

    if (getnodecommand(Str,TXT_RECYCLETHRUDOS) == -1)
      break;
    p = Str;
  }
}


void LIBENTRY broadcast(int NumTokens) {
  int      Start;
  int      End;
  int      NodeNum;
  char     *p;
  char     Str[80];
  nodetype Buf;

  if (UserNetFile <= 0)
    return;

  if (NumTokens < 2)
    return;

  p = getnexttoken();
  if (strcmp(p,"ALL") == 0) {
    Start = 1;
    End = PCB_MAXNODES;
  } else {
    if ((NodeNum = atoi(p)) < 1 || NodeNum > PCB_MAXNODES)
      return;
    Start = End = NodeNum;
    if ((p = strchr(p,'-')) != NULL)
      if ((End = atoi(p+1)) < 1 || NodeNum > PCB_MAXNODES - 1)
        return;
  }

  for (Str[0] = 0, NumTokens--; NumTokens && strlen(Str) < sizeof(Buf.Message)-1; NumTokens--) {
    p = getnexttoken();
    if (strlen(p) > sizeof(Buf.Message)-1)
      p[sizeof(Buf.Message)-1] = 0;
    addtext(Str,p,sizeof(Str));
  }

  Str[sizeof(Buf.Message)-1] = 0;
  stripright(Str,' ');

  if ((p = (char *) bmalloc(USERNETFLAGSIZE)) == NULL)
    return;

  doslseek(UserNetFile,USERNETFLAGS,SEEK_SET);
  if (readcheck(UserNetFile,p,USERNETFLAGSIZE) == (unsigned) -1) {
    bfree(p);
    return;
  }

  for (NodeNum = Start; NodeNum <= End; NodeNum++) {
    if (isset(p,NodeNum-1)) {
      readusernetrecord(NodeNum,&Buf);
      if (Buf.Status == AVAILABLE ||
          Buf.Status == ENTERMESSAGE ||
          Buf.Status == GROUPCHAT ||
          Buf.Status == PAGINGSYSOP ||
          Buf.Status == ANSWERSCRIPT ||
          Buf.Status == TRANSFER ||
          Buf.Status == UNAVAILABLE) {
        Buf.Status = NODEMESSAGE;
        strcpy(Buf.Message,Str);
        updateusernetrecord(NodeNum,&Buf);
      }
    }
  }

  bfree(p);
}


void LIBENTRY dropdoscommand(int NumTokens) {
  unsigned NodeNum;
  char     *p;
  char     Str[60];
  nodetype Buf;

  if (NumTokens) {
    p = getnexttoken();
    NumTokens--;
  } else {
    p = Str;
    *p = 255;  /* it will ignore this and then ask for input */
  }

  while (1) {
    if ((NodeNum = checknodestatus(p,&Buf)) != 0xFFFF) {
      if (NumTokens)
        p = getnexttoken();
      else if ((Buf.Status != NOCALLER && Buf.Status != RUNONCONNECT) && Buf.Status != 0) {
        Str[0] = NoChar;
        Str[1] = 0;
        inputfield(Str,TXT_DROPNOW,1,YESNO|NEWLINE|LFBEFORE|FIELDLEN|GUIDE|UPCASE,NOHELP,mask_yesno);
        p = Str;
      }

      if (*p == YesChar)
        Buf.Status = DROPDOSNOW;
      else
        Buf.Status = DROPDOSDELAYED;

      updateusernetrecord(NodeNum,&Buf);
      sprintf(Str,"Forced node %d to %s",NodeNum,"drop to DOS");
      writelog(Str,SPACERIGHT);
      return;
    }

    NumTokens = 0;
    if (getnodecommand(Str,TXT_NODENUMTODROP) == -1)
      break;
    p = Str;
  }
}


void LIBENTRY chatcommand(int NumTokens) {
  int      Prompt;
  char     *p;
  char     Str[10];

  if (NumTokens)
    p = getnexttoken();
  else {
    p = Str;
    *p = 255;  /* it will ignore this and then ask for input */
  }

  while (1) {
    if (*(p+1) == 0) {
      switch (*p) {
        case 'A': writeusernetstatus(AVAILABLE,NULL);
                  displaypcbtext(TXT_AVAILABLE,LFBEFORE|NEWLINE);
                  UsersData.Flags.UnAvailable = FALSE;
                  return;
        case 'U': writeusernetstatus(UNAVAILABLE,NULL);
                  displaypcbtext(TXT_UNAVAILABLE,LFBEFORE|NEWLINE);
                  UsersData.Flags.UnAvailable = TRUE;
                  return;
        case 'H':
        case '?': displayhelpfile(HLP_CHAT);       break;
        case 'G': displayfile(PcbData.GroupChat,SECURITY|GRAPHICS|LANGUAGE);
                  newchat();
                  return;
      }
    }

    Str[0] = 0;
    Prompt = (Status.Available ? TXT_NODECHATUPROMPT : TXT_NODECHATAPROMPT);
    inputfield(Str,Prompt,1,NEWLINE|LFBEFORE|UPCASE|FIELDLEN,HLP_CHAT,mask_node);
    if (Str[0] == 0)
      break;

    p = Str;
  }
}


void LIBENTRY callcommand(int NumTokens) {
  char     CurStatus;
  unsigned NodeNum;
  char     Answer[2];
  char     *p;
  char     Str[10];
  nodetype PageeBuf;

  if (NumTokens)
    p = getnexttoken();
  else {
    p = Str;
    *p = 255;  /* it will ignore this and then ask for input */
  }

  while (1) {
    if ((NodeNum = checknodestatus(p,&PageeBuf)) != 0xFFFF) {
      CurStatus = PageeBuf.Status;
      if (CurStatus != AVAILABLE && CurStatus != GROUPCHAT) {
        displaypcbtext(TXT_NODENOTAVAILABLE,NEWLINE|LFBEFORE);
        if ((CurStatus != NOCALLER && CurStatus != RUNONCONNECT) && CurStatus != 0) {
          if (Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL]) {
            Answer[0] = NoChar;
            Answer[1] = 0;
            inputfield(Answer,TXT_CALLANYWAY,1,YESNO|FIELDLEN|UPCASE|NEWLINE,NOHELP,mask_yesno);
            if (Answer[0] == YesChar) {
              PageeBuf.Pager   = PcbData.NodeNum;
              PageeBuf.Channel = Channel;
              updateusernetrecord(NodeNum,&PageeBuf);
              return;
            }
          }
        }
      } else {
        PageeBuf.Pager   = PcbData.NodeNum;
        PageeBuf.Channel = Channel;
        updateusernetrecord(NodeNum,&PageeBuf);
        return;
      }
    }

    if (getnodecommand(Str,TXT_NODETOCALL) == -1)
      break;
    p = Str;
    NumTokens = 0;
  }
}
