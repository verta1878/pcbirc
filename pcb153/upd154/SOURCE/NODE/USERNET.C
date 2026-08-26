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

#include <io.h>

#ifdef __OS2__
  #include <kbd.hpp>
  #include <threads.h>
#endif

static char LastStatus;

bool InChat              = FALSE;
int  UserNetFile         = 0;


static void _NEAR_ LIBENTRY userneterror(char *Msg) {
  char Str[80];

  sprintf(Str,"%s: %s error",PcbData.NetFile,Msg);
  errorexittodos(Str);
}


static usernethdrtype UserNetHeader;

static void _NEAR_ LIBENTRY updatebitflags(int Offset, ubyte BitMask, bool On) {
  ubyte Flag;

  // optimization (6/5/96): see if the flag is already at the desired setting
  // and, if so, avoid the whole process of locking/writing/unlocking
  doslseek(UserNetFile,Offset,SEEK_SET);
  if (readcheck(UserNetFile,&Flag,sizeof(ubyte)) == sizeof(ubyte)) {
    if (On) {
      // we want the flag to be ON, see if it is already ON
      if ((Flag & BitMask) != 0)
        return;    // no change necessary, return now
    } else {
      // we want the flag to be OFF, see if it is already OFF
      if ((Flag & BitMask) == 0)
        return;    // no change necessary, return now
    }
  }

  // a change is necessary, to ensure we're updating the byte correctly,
  // we must go thru the formal process of locking the byte and re-reading it
  // before updating it and writing it back out to disk
  doslseek(UserNetFile,Offset,SEEK_SET);
  if (doslockcheck(UserNetFile,Offset,sizeof(ubyte)) != -1) {
    if (readcheck(UserNetFile,&Flag,sizeof(ubyte)) == sizeof(ubyte)) {
      doslseek(UserNetFile,Offset,SEEK_SET);
      if (On)
        Flag |= (ubyte) BitMask;
      else
        Flag &= (ubyte) (~BitMask);
      writecheck(UserNetFile,&Flag,sizeof(ubyte));     /*lint !e534 */
    }
    unlock(UserNetFile,Offset,sizeof(ubyte));
  }
}


void LIBENTRY setattentionflags(unsigned NodeNum, bool On) {
  int   Offset;
  ubyte BitMask;

  NodeNum--;     /* fix number to an offset from 0 instead of 1 */
  BitMask = 1 << (NodeNum & 7);
  Offset  = sizeof(usernethdrtype) + (NodeNum / 8);
  updatebitflags(Offset,BitMask,On);
}


static void _NEAR_ LIBENTRY setnetstatusflag(bool On) {
  unsigned NodeNum;
  int      Offset;
  ubyte    BitMask;

  NodeNum=PcbData.NodeNum-1; /* fix number to an offset from 0 instead of 1 */
  BitMask = 1 << (NodeNum & 7);
  Offset  = USERNETFLAGS + (NodeNum / 8);
  updatebitflags(Offset,BitMask,On);
}


static void _NEAR_ LIBENTRY writeusernettimestamp(void) {
  long distance;
  updttype LastUpdate;

  #ifdef __OS2__
    // in some situations, we don't want to update the usernet file so that
    // PCBCP can detect that we've gotten hung up somewhere
    if (Status.DisableUserNetUpdate)
      return;
  #endif

  #ifdef offsetof
    int UpdateOffset = (int) offsetof(nodetype,LastUpdate);
  #else
    nodetype Buf;
    int UpdateOffset = (int) ((char *) &Buf.LastUpdate - (char*) &Buf.Status);
  #endif

  if (UserNetFile > 0) {
    distance = ((long)(PcbData.NodeNum-1) * sizeof(nodetype))
             + UpdateOffset
             + USERNETSTART;
    doslseek(UserNetFile,distance,SEEK_SET);
    LastUpdate.Split.Date = getjuliandate();
    LastUpdate.Split.Time = (int) (exacttime() / 2);
    writecheck(UserNetFile,&LastUpdate,sizeof(LastUpdate));     //lint !e534
  }
}   //lint !e550


int LIBENTRY writeusernetrecord(unsigned NodeNum, nodetype *Buf) {
  if (UserNetFile > 0 && NodeNum >= 1 && NodeNum <= PCB_MAXNODES) {
    doslseek(UserNetFile,((long)(NodeNum-1) * sizeof(nodetype)) + USERNETSTART,SEEK_SET);
    if (NodeNum == PcbData.NodeNum) {
      Buf->LastUpdate.Split.Date = getjuliandate();
      Buf->LastUpdate.Split.Time = (int) (exacttime() / 2);
    }
    return(writecheck(UserNetFile,Buf,sizeof(nodetype)));
  }
  return(-1);
}


void LIBENTRY updateusernetrecord(unsigned NodeNum, nodetype *Buf) {
  if (writeusernetrecord(NodeNum,Buf) != -1)
    setattentionflags(NodeNum,TRUE);
}


void LIBENTRY readusernetrecord(unsigned NodeNum, nodetype *Buf) {
  long Offset;

  if (UserNetFile > 0 && NodeNum >= 1 && NodeNum <= PCB_MAXNODES) {
    Offset = ((long)(NodeNum-1) * sizeof(nodetype)) + USERNETSTART;
    doslseek(UserNetFile,Offset,SEEK_SET);
    if (readcheck(UserNetFile,Buf,sizeof(nodetype)) != sizeof(nodetype))
      memset(Buf,0,sizeof(nodetype));
  } else
    memset(Buf,0,sizeof(nodetype));
}


bool LIBENTRY checkforusernetmatch(unsigned NodeNum, char *Name) {
  nodetype Buf;

  readusernetrecord(NodeNum,&Buf);
  padstr(Buf.Name,' ',25);
  return((bool) (memcmp(Buf.Name,Name,sizeof(UsersRead.Name)) == 0));
}


unsigned short LIBENTRY firstfreenode(unsigned StartNum) {
  unsigned short Count;
  long     Offset;
  DOSFILE  File;
  nodetype Node;

  if (PcbData.Network && UserNetFile > 0 && StartNum <= PCB_MAXNODES) {
    if (dosfopen(PcbData.NetFile,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
      dosfseek(&File,((long) sizeof(nodetype)*(StartNum-1))+USERNETSTART,SEEK_SET);
      for (Count = (short) StartNum; Count <= PCB_MAXNODES; Count++) {

        #ifdef DEBUG
          mc_check_buffers();
        #endif

        if (dosfread(&Node,sizeof(nodetype),&File) != sizeof(nodetype)) {
          dosfclose(&File);
          return(0);
        }
        if (Node.Status == 0) {
          Offset = ((long)sizeof(nodetype) * (Count-1)) + USERNETSTART;
          doslseek(UserNetFile,Offset,SEEK_SET);
          if (doslockcheck(UserNetFile,Offset,sizeof(nodetype)) == -1)
            continue;
          if (readcheck(UserNetFile,&Node,sizeof(nodetype)) == (unsigned) -1 || Node.Status != 0) {
            unlock(UserNetFile,Offset,sizeof(nodetype));
            continue;
          }
          PcbData.NodeNum = Count;
          clearusernet();
          unlock(UserNetFile,Offset,sizeof(nodetype));
          dosfclose(&File);
          return(Count);
        }
      }
      dosfclose(&File);
    }
  }
  return(0);
}


/********************************************************************
*
*  Function:  scanusernet()
*
*  Desc    :  Scans the USERNET.DAT file to see if someone wants to chat with
*             us or if we're supposed to do an auto-logoff or drop to DOS...
*
*  Returns :  TRUE if someone wants to chat, FALSE otherwise.
*/

#ifdef __OS2__
  static CSemaphore UserNetSemaphore;
  static bool       KillUserNetThread;
  static bool       NeedAttention;
#endif

static bool _NEAR_ LIBENTRY seeifneedattention(void) {
  char Flag;
  int  NodeNum;
  int  BitMask;
  int  Offset;

  NodeNum = PcbData.NodeNum-1;   // fix number to an offset from 0 instead of 1
  BitMask = 1 << (NodeNum & 7);
  Offset  = sizeof(usernethdrtype) + (NodeNum / 8);

  doslseek(UserNetFile,Offset,SEEK_SET);
  if (readcheck(UserNetFile,&Flag,sizeof(char)) != sizeof(char))
    return(FALSE);

  #ifndef __OS2__
    writeusernettimestamp();
  #endif

  if ((Flag & BitMask) == 0)
    return(FALSE);

  #ifdef __OS2__
    NeedAttention = TRUE;
    releasekbdblock(0);
  #endif
  return(TRUE);
}


bool LIBENTRY scanusernet(void) {
  bool     Logoff;
  bool     MsgDisplayed;
  bool     Updated;
  int      NodeNum;
  nodetype PagerBuf;
  nodetype Buf;

  if (! PcbData.Network || UserNetFile <= 0)
    return(FALSE);

  #ifndef __OS2__
    static int Count;

    showusernetscan(TRUE);
    if (++Count > 15) {
      writeusernettimestamp();
      Count = 0;
    }
  #endif

  #ifdef __OS2__
    // Perform the seeifneedattention() function call down below *ONLY* if
    // the NeedAttention flag has not already been set.  This way we avoid
    // re-checking the attention flag if it's already known to be set.
    if (! NeedAttention)
    // if ! need attention, execute next if() statement below
  #endif

  if (! seeifneedattention()) {
    #ifndef __OS2__
      showusernetscan(FALSE);
    #endif
    return(FALSE);
  }

  NodeNum=PcbData.NodeNum-1; /* fix number to an offset from 0 instead of 1 */

  /* if we get this far then our BIT FLAG has been set */

  setattentionflags(PcbData.NodeNum,FALSE);  /* turn it off */

  doslseek(UserNetFile,USERNETSTART + (long) NodeNum*sizeof(nodetype),SEEK_SET);
  if (readcheck(UserNetFile,&Buf,sizeof(nodetype)) == (unsigned) -1) {
    #ifndef __OS2__
      showusernetscan(FALSE);
    #endif
    return(FALSE);
  }

  #ifdef __OS2__
    NeedAttention = FALSE;
  #endif

  MsgDisplayed = FALSE;
  Logoff       = FALSE;
  Updated      = FALSE;

  switch(Buf.Status) {
    case LOGOFFPENDING : Logoff = TRUE;
                         writelog("USERNET forced logoff",SPACERIGHT);
                         break;
    case DROPDOSNOW    : Logoff = TRUE;            // fall thru!
    case DROPDOSDELAYED: Status.SysopFlag = 'X';
                         makepcboardsys();
                         writelog("USERNET forced drop to DOS",SPACERIGHT);
                         break;
    case RECYCLEBBS    : Status.SysopFlag = 'R';
                         makepcboardsys();
                         break;
    case NODEMESSAGE   : if (Asy.Online != OFFLINE && ! Status.FileXfer && ! Status.LoggingUserOff) {
                           bell();
                           freshline();
                           newline();
                           printcolor(PCB_WHITE);
                           println(Buf.Message);
                           Buf.Message[0] = 0;
                           Buf.Status = LastStatus;
                           MsgDisplayed = TRUE;
                           Updated = TRUE;
                         }
                         break;
  }

  if (Buf.MailWaiting) {
    Buf.MailWaiting = FALSE;
    UsersData.PackedFlags.HasMail = TRUE;
    Status.CheckForMail = TRUE;
    Updated = TRUE;
    if (Status.CmdPrompt)
      MsgDisplayed = TRUE;
  }

  if (Buf.Pager != 0 && ! Status.FileXfer && ! Status.LoggingUserOff) {
    readusernetrecord(Buf.Pager,&PagerBuf);
    if (PagerBuf.Name[0] != 0) {
      #ifdef PCBCOMM2
      if (Status.TerseMode) {
        sprintf(Status.DisplayText,"%s:%d",PagerBuf.Name,Buf.Channel);
        strcpy(Status.DisplayText,PagerBuf.Name);
        displaypcbtext(TXT_WANTSTOCHAT,NEWLINE|LFBEFORE);
      } else {
      #endif
        bell();
        freshline();
        newline();
        printcolor(PCB_RED);
        print(PagerBuf.Name);
        ascii(Status.DisplayText,Buf.Channel);
        displaypcbtext(TXT_WANTSTOCHAT,NEWLINE);
      #ifdef PCBCOMM2
      }
      #endif
      if (! InChat) {
        ascii(Status.DisplayText,Buf.Channel);
        displaypcbtext(TXT_TORESPONDTOCHAT,NEWLINE);
      }
      CalledIntoChannel = Buf.Channel;
      Buf.Pager = 0;
      Updated = TRUE;
      MsgDisplayed = TRUE;
    }
  }

  if (Updated)
    writeusernetrecord(PcbData.NodeNum,&Buf); /*lint !e534 */

  #ifndef __OS2__
    showusernetscan(FALSE);
  #endif

  if (Logoff && ! Status.LoggingUserOff) {
    if (! Control.Screen)
      turndisplayon(FALSE);
    if (Asy.Online == OFFLINE)
      recycle();
    else {
      displaypcbtext(TXT_AUTOLOGOFF,NEWLINE|LFBEFORE);
      Status.AutoLogoff = TRUE;
      loguseroff(NLOGOFF);
    }
  }

  return(MsgDisplayed);
}


#ifdef __OS2__
#pragma argsused
static void THREADFUNC usernetthread(void *Ignore) {
  int Count;

  Count = 0;
  while (TRUE) {
    UserNetSemaphore.waitforevent(Control.NetTimer);
    UserNetSemaphore.reset();
    // if the KillUserNetThread variable has been set to TRUE then exit
    // the function now -- note:  use this approach instead of the
    // killthread() approach because the borland debugger gets stuck on it
    // (on just *this* thread) for some stupid reason... don't know if it is
    // a debugger problem or if there's really something different about the
    // thread that requires not using killthread()
    if (KillUserNetThread)
      return;
    if (! NeedAttention)     // if the in-memory flag isn't already set,
      seeifneedattention();  //lint !e534   then go check the on-disk flag to see if it should be
    if (++Count > 15) {
      writeusernettimestamp();
      Count = 0;
    }
  }
}


bool LIBENTRY needtoscanusernet(void) {
  return(NeedAttention);
}

static unsigned long UserNetThreadId;

void LIBENTRY createusernetthread(void) {
  if (! PcbData.Network || UserNetFile <= 0)
    return;

  KillUserNetThread = FALSE;
  NeedAttention     = FALSE;
  if (UserNetSemaphore.createunique("UNET"))
    UserNetThreadId = startthread(usernetthread,8*1024,NULL);
}


void LIBENTRY destroyusernetthread(void) {
  if (! PcbData.Network || UserNetFile <= 0 || UserNetThreadId == 0)
    return;

  KillUserNetThread = TRUE;
  UserNetSemaphore.postevent();
  waitthread(UserNetThreadId);
  UserNetSemaphore.close();
  NeedAttention = FALSE;
}
#endif  /* ifdef __OS2__ */


void LIBENTRY writeusernetstatus(char NetStatus, char *Operation) {
  char     Temp[80];
  nodetype Buf;

  if (! PcbData.Network)
    return;

  if (NetStatus == AVAILABLE)
    Status.Available = TRUE;
  else if (NetStatus == UNAVAILABLE)
    Status.Available = FALSE;

  memset(&Buf,0,sizeof(nodetype));
  Buf.Status = LastStatus = NetStatus;

  maxstrcpy(Buf.Name,Status.DisplayName,sizeof(Buf.Name));

  if (Status.CurConf.AllowAliases && UsersData.Alias[0] != 0 && Status.AllowAlias && Status.UseAlias) {
    if (PcbData.ShowAlias)
      Buf.City[0] = 0;
    else {
      // we're not allowed to SHOW the alias name in the USERNET file
      // so go get the caller's real name thru the getdisplaynames() function
      Status.CurConf.AllowAliases = FALSE;
      getdisplaynames();
      maxstrcpy(Buf.Name,Status.DisplayName,sizeof(Buf.Name));
      // now put things back the way we found them!
      Status.CurConf.AllowAliases = TRUE;
      getdisplaynames();
      maxstrcpy(Buf.City,usercity(),sizeof(Buf.City));
    }
  } else
    maxstrcpy(Buf.City,usercity(),sizeof(Buf.City));

  if (Operation != NULL) {
    if (NetStatus == INADOOR) {
      sprintf(Temp,"%s - %s",Buf.Name,Operation);
      maxstrcpy(Buf.Operation,Temp,sizeof(Buf.Operation));
    } else
      maxstrcpy(Buf.Operation,Operation,sizeof(Buf.Operation));
  }

  scanusernet();                            /*lint !e534 */
  writeusernetrecord(PcbData.NodeNum,&Buf); /*lint !e534 */
}


void LIBENTRY hidealiasinusernet(void) {
  nodetype Buf;

  if (! PcbData.Network)
    return;

  memset(&Buf,0,sizeof(nodetype));
  Buf.Status = LOGINTOSYSTEM;

  if (Status.CurConf.AllowAliases) {
    Status.CurConf.AllowAliases = FALSE;
    getdisplaynames();
    Status.CurConf.AllowAliases = TRUE;
    maxstrcpy(Buf.Operation,Status.DisplayName,sizeof(Buf.Operation));
    getdisplaynames();
  } else
    maxstrcpy(Buf.Operation,Status.DisplayName,sizeof(Buf.Operation));

  scanusernet();                             /*lint !e534 */
  writeusernetrecord(PcbData.NodeNum,&Buf);  /*lint !e534 */
  Status.HideAliasStartTime = dosgetlongtime();
}


static char _NEAR_ LIBENTRY availableifpossible(void) {
  if (Status.InChat)
    return(CHATWITHSYSOP);
  if (InChat)
    return(GROUPCHAT);
  if (Status.EnteringMessage)
    return(ENTERMESSAGE);
  if (UsersData.Flags.UnAvailable || Status.CurSecLevel < PcbData.UserLevels[SEC_CHAT])
    return(UNAVAILABLE);

  return(AVAILABLE);
}


bool LIBENTRY canbeavailable(void) {
  return((bool) (availableifpossible() == AVAILABLE));
}


void LIBENTRY usernetavailable(void) {
  writeusernetstatus(availableifpossible(),NULL);
  Status.HideAliasStartTime = 0;
}


void LIBENTRY clearusernet(void) {
  nodetype Buf;

  if (UserNetFile > 0) {
    memset(&Buf,0,sizeof(nodetype));
    Buf.Status = NOCALLER;
    Buf.LastUpdate.Split.Date = getjuliandate();
    Buf.LastUpdate.Split.Time = (int) (exacttime() / 2);
    writeusernetrecord(PcbData.NodeNum,&Buf);  /*lint !e534 */
    setnetstatusflag(TRUE);
    setattentionflags(PcbData.NodeNum,FALSE);
    InChat = FALSE;
  }
}


void LIBENTRY closeusernet(void) {
  nodetype Buf;

  if (UserNetFile > 0) {
    if (Status.Logoff != REMOTEDOS && Status.Logoff != DOOR && Status.Logoff != RUNEVENT) {
      memset(&Buf,0,sizeof(nodetype));
      writeusernetrecord(PcbData.NodeNum,&Buf);  /*lint !e534 */
      setnetstatusflag(FALSE);
    }
    dosclose(UserNetFile);
    UserNetFile = 0;
  }
}


static void _NEAR_ LIBENTRY initusernet(void) {
  int      X;
  char     *Flags;
  DOSFILE  File;
  nodetype Buf;

  if (dosfopen(PcbData.NetFile,OPEN_RDWR|OPEN_DENYNONE,&File) == -1)
    userneterror("open");

  if ((Flags = (char *) bmalloc(USERNETFLAGSIZE)) == NULL) {
    userneterror("memory");
    return;  // keep PC/Lint happy
  }

  UserNetHeader.Version    = 150;
  UserNetHeader.NumOfNodes = PCB_MAXNODES;
  UserNetHeader.SizeOfRec  = sizeof(nodetype);
  memset(Flags,0,USERNETFLAGSIZE);

  dossetbuf(&File,8192);

  if (dosfwrite(&UserNetHeader,sizeof(usernethdrtype),&File) == -1 ||
      dosfwrite(Flags,USERNETFLAGSIZE,&File) == -1 ||  /* attention flags */
      dosfwrite(Flags,USERNETFLAGSIZE,&File) == -1)    /* up status flags */
    userneterror("write");

  memset(&Buf,0,sizeof(nodetype));
  for (X = 0; X < PCB_MAXNODES; X++)
    if (dosfwrite(&Buf,sizeof(nodetype),&File) == -1)
      userneterror("write");

  bfree(Flags);
  dosfclose(&File);
}


void LIBENTRY createusernet(void) {
  if (openusernet() == -1) {
    dosclose(UserNetFile);
    UserNetFile = -1;
  }

  if (UserNetFile == -1 && PcbData.NetFile[0] != 0) {
    if ((UserNetFile = doscreatecheck(PcbData.NetFile,OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL)) == -1)
      userneterror("create");

    initusernet();
  }
}


int LIBENTRY openusernet(void) {
  char *p;
  char  Str[66];

  if (! PcbData.Network) {
    UserNetFile = -1;
    return(-1);
  }

  if (UserNetFile > 0) {
    /* already open so return now */
    return(0);
  }

  if ((p = strstr(PcbData.NetFile,"USERNET.DAT")) != NULL)
    memcpy(p+8,"XXX",3);

  if ((UserNetFile = dosopencheck(PcbData.NetFile,OPEN_RDWR|OPEN_DENYNONE)) != -1) {
    if (readcheck(UserNetFile,&UserNetHeader,sizeof(usernethdrtype)) == (unsigned) -1)
      userneterror("read");

    if (UserNetHeader.Version != 150 || UserNetHeader.SizeOfRec < sizeof(nodetype) || UserNetHeader.NumOfNodes != PCB_MAXNODES) {
      if (UserNetHeader.NumOfNodes != PCB_MAXNODES) {
        sprintf(Str,"Init USERNET.XXX (old %d new %d)",UserNetHeader.NumOfNodes,PCB_MAXNODES);
        writedebugrecord(Str);
      }
      initusernet();
    }
  }
  return(0);
}


void LIBENTRY setmsgflaginusernet(char *Name) {
  unsigned NodeNum;
  nodetype Buf;

  if (strcmp(Name,UsersData.Name) == 0 || strcmp(Name,UsersData.Alias) == 0 || strcmp(Name,Status.DisplayName) == 0)
    NodeNum = PcbData.NodeNum;
  else if ((NodeNum = foundinusernet(Name,NULL)) == 0)
    return;

  readusernetrecord(NodeNum,&Buf);
  if (Buf.Status != 0) {
    Buf.MailWaiting = TRUE;
    updateusernetrecord(NodeNum,&Buf);
  }
}
