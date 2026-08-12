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


#ifdef __OS2__
  #define NORESTORE
#else
  /* to disable screen restore, change to:  #define NORESTORE */
  #undef NORESTORE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <dos.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <system.h>
#include <misc.h>

#ifndef __OS2__
//#include <conio.h>
extern "C" int _Cdecl kbhit(void);
#endif

#ifdef DEBUG
  #include <memcheck.h>
#endif

#define NUMFIELDS 5
#define min(a,b)    (((a) < (b)) ? (a) : (b))

enum { CLR_TEXT, CLR_BAR, CLR_HEAD, CLR_TITLE };
char MoniColors[2][CLR_TITLE+1] = {{0x1E,0x71,0x1B,0x4E},{0x07,0x70,0x0F,0x0F}};

#define NumEKeys 1
int  NumExitKeys = NumEKeys;   /* maximum number of exit keys                */
char ExitKeyNum[NumEKeys];     /* actual values are defined in their modules */
int  ExitKeyFlag[NumEKeys];


#define TRUE        1
#define FALSE       0
#define WAITPERIOD  3   /* number of seconds in between reads */

#pragma pack(1)
typedef struct {
  unsigned short Version;     /* PCBoard version number */
  unsigned short NumOfNodes;  /* Number of nodes supported in the USERNET.DAT */
  unsigned short SizeOfRec;   /* The size of each node record */
} usernethdrtype;
#pragma pack()

#pragma pack(1)
typedef union {
  long DateTime;
  struct {
    uint Time;            /* number of seconds past midnight divided by 2 */
    uint Date;            /* julian date */
  } Split;
} updttype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char     Status;        /* node status */
  bool     MailWaiting;   /* true if msg posted */
  unsigned short Pager;   /* node number of pager */
  char     Name[26];      /* caller's name */
  char     City[25];      /* caller's city */
  char     Operation[49]; /* current operational text */
  char     Message[80];   /* broadcast message text */
  char     Channel;       /* channel number of pager */
  updttype LastUpdate;    /* hour:min:sec of last update on this record */
} nodetype;
#pragma pack()

#define USERNETFLAGSIZE ((UserNetHeader.NumOfNodes+7)/8)
#define USERNETSTART    (sizeof(usernethdrtype) + (USERNETFLAGSIZE*2))
#define USERNETFLAGS    (sizeof(usernethdrtype) + USERNETFLAGSIZE)

int            UserNetFile;
usernethdrtype UserNetHeader;

int      Abort;
int      LineNum;       /* Line 2 is the first line on the screen */
int      FirstNode;     /* 0-based (i.e. Node1 would be 0 */
int      MaxNodes;
int      MaxWindow;
bool     ReadFile;     /* TRUE=need to read USERNET.XXX file */
bool     Refresh;      /* TRUE=need to refresh/display the ENTIRE screen */
char     ComSpec[66];
unsigned Reads;

#ifdef __OS2__
#include "semafore.hpp"
static CUpdateSemaphore DisplaySemaphore;
#endif


/********************************************************************
*
*  Function:  shell()
*
*  Desc    :  Shells out to NODE.BAT passing two parameters like this:
*
*                NODE.BAT NODEx x
*
*             Where 'x' is the node number.
*/

static void _NEAR_ pascal shell(void) {
  savescrntype SaveWindow;
  char Str[80];
  int  Node;

  savescreen(&SaveWindow);
  #ifdef __OS2__
    DisplaySemaphore.disabletimer();
  #endif
  Node = (LineNum-2) + (FirstNode+1);
  cls();
  sprintf(Str,"Executing SHELL for Node %d",Node);
  fastcenter(0,Str,0x0F);
  gotoxy(0,2);
  sprintf(Str,"/c node.bat node%d %d",Node,Node);
  spawnl(P_WAIT,ComSpec,ComSpec,Str,NULL);
  restorescreen(&SaveWindow);
  Refresh = TRUE;
  #ifdef __OS2__
    DisplaySemaphore.enabletimer(3000);
  #endif
}


void _NEAR_ pascal setattentionflags(int UserNetFile, unsigned NodeNum) {
  int  Offset;
  char BitMask;
  char Flag;

  BitMask = (char) (1 << (NodeNum & 7));
  Offset  = sizeof(usernethdrtype) + (NodeNum / 8);

  doslseek(UserNetFile,Offset,SEEK_SET);
  if (doslockcheck(UserNetFile,Offset,sizeof(char)) != -1) {
    if (readcheck(UserNetFile,&Flag,sizeof(char)) != (unsigned) -1) {
      Flag |= BitMask;
      doslseek(UserNetFile,Offset,SEEK_SET);
      writecheck(UserNetFile,&Flag,sizeof(char));
    }
    unlock(UserNetFile,Offset,sizeof(char));
  }
}


static void _NEAR_ pascal setnetstatusflag(int UserNetFile, int NodeNum, bool On) {
  int  Offset;
  char BitMask;
  char Flag;

  BitMask = (char) (1 << (NodeNum & 7));
  Offset  = USERNETFLAGS + (NodeNum / 8);

  doslseek(UserNetFile,Offset,SEEK_SET);
  if (doslockcheck(UserNetFile,Offset,sizeof(char)) != -1) {
    if (readcheck(UserNetFile,&Flag,sizeof(char)) != (unsigned) -1) {
      if (On)
        Flag |= BitMask;
      else
        Flag &= (char) ~BitMask;
      doslseek(UserNetFile,Offset,SEEK_SET);
      writecheck(UserNetFile,&Flag,sizeof(char));
    }
    unlock(UserNetFile,Offset,sizeof(char));
  }
}


/********************************************************************
*
*  Function:  getinput()
*
*  Desc    :  Displays information about the highlighted node and then
*             allows the user to change information for that node - to
*             log the node off, drop it to DOS, etc.
*/

char     Status[2];
nodetype NodeBuf;
char     mask[] = {10, ' ', 0, 'A','E', 0,'F','H', 0,'L','Z' };

FldType Fields[NUMFIELDS] = {
  {vUPSTR,ALLCHAR ,0,2, 4,25,NOCLEARFLD,"Name     " ,"",NodeBuf.Name     ,NULL},
  {vUPSTR,ALLCHAR ,0,2, 5,24,NOCLEARFLD,"City     " ,"",NodeBuf.City     ,NULL},
  {vUPSTR,ALLCHAR ,0,2, 6,48,CLEAR     ,"Operation" ,"",NodeBuf.Operation,NULL},
  {vSTR  ,ALLCHAR ,0,2, 7,64,CLEAR     ,"Message  " ,"",NodeBuf.Message  ,NULL},
  {vUPSTR,mask    ,0,2,19, 1,CLEAR     ,"Status   " ,"",Status           ,NULL},
};

void getinput(void) {
  int          Node;
  int          Hour;
  int          Min;
  int          Sec;
  unsigned     Today;
  long         Time;
  long         ElapsedTime;
  updttype     LastUpdate;
  char         Str[80];
  nodetype     SaveBuf;
  savescrntype Screen;

  Node = (LineNum-2) + FirstNode;
  doslseek(UserNetFile,(long) Node*sizeof(nodetype)+USERNETSTART,SEEK_SET);
  if (readcheck(UserNetFile,&NodeBuf,sizeof(nodetype)) == (unsigned) -1)
    return;

  #ifdef __OS2__
    DisplaySemaphore.disabletimer();
  #endif

  savescreen(&Screen);
  boxcls(0,0,79,24,Colors[OUTBOX],DOUBLE);

  #ifdef __OS2__
    setscreenupdateinterval(1000);
    enableshowstatus();
    enablekbdmonitor();
  #endif

  LastUpdate.DateTime = NodeBuf.LastUpdate.DateTime;
  if (LastUpdate.Split.Date <= 1)
    Time = LastUpdate.DateTime;
  else
    Time  = (long) LastUpdate.Split.Time * 2;

  Hour  = (int) (Time / 3600);
  Time -= (Hour * 3600L);
  Min = (int) (Time / 60);
  Sec = (int) (Time - (Min * 60));

  sprintf(Str,"Record Last Updated: %02d:%02d:%02d",Hour,Min,Sec);
  fastprint(40,4,Str,Colors[DISPLAY]);
  strcpy(Str,"Elapsed Time:");
  fastprint(40,5,Str,Colors[DISPLAY]);

  if (LastUpdate.Split.Date <= 2) {
    Time = LastUpdate.DateTime;
    ElapsedTime = exacttime() - Time;
    if (ElapsedTime < 0)
      ElapsedTime += (24 * 60 * 60);
  } else {
    Time  = (long) LastUpdate.Split.Time * 2;
    Today = getjuliandate();
    if (LastUpdate.Split.Date == Today)
      ElapsedTime = exacttime() - Time;
    else if (Today > LastUpdate.Split.Date) {
      ElapsedTime = exacttime() - Time + ((Today - LastUpdate.Split.Date) * (24 * 60 * 60));
    } else
      ElapsedTime = 0;
  }

  Hour = (int) (ElapsedTime / 3600);
  ElapsedTime -= (Hour * 3600L);
  Min = (int) (ElapsedTime / 60);
  Sec = (int) (ElapsedTime - (Min * 60));

  if (Hour != 0)
    sprintf(Str,"%2dh %2dm %2ds",Hour,Min,Sec);
  else if (Min != 0)
    sprintf(Str,"%2d min %2d sec",Min,Sec);
  else
    sprintf(Str,"%d seconds",Sec);

  fastprint(55,5,Str,Colors[DISPLAY]);

  fastprint( 2, 9,"Possible Status Bytes:"                                              ,Colors[DISPLAY]);
  fastprint( 5,10,"A=Available            L=Logoff pending          T=Transfer file"    ,Colors[DISPLAY]);
  fastprint( 5,11,"B=Out to DOS           M=Message                 U=Unavailable"      ,Colors[DISPLAY]);
  fastprint( 5,12,"C=Chat w/Sysop         N=Running Event           V=No Caller online" ,Colors[DISPLAY]);
  fastprint( 5,13,"D=In a DOOR            O=Logging in              W=Wait/Drop to DOS" ,Colors[DISPLAY]);
  fastprint( 5,14,"E=Enter msg            P=Paging Sysop            X=Drop to DOS now"  ,Colors[DISPLAY]);
  fastprint( 5,15,"F=File View            Q=Run on Connect          Y=No caller (clear)",Colors[DISPLAY]);
  fastprint( 5,16,"G=Group chat           R=DOS Recycle Pending     Z=Node is offline"  ,Colors[DISPLAY]);
  fastprint( 5,17,"H=Handling Mail        S=Answering Script"                           ,Colors[DISPLAY]);
  fastcenter(21,"Press ESC to abort or PGDN to apply changes",Colors[DISPLAY]);

  Status[0] = NodeBuf.Status;
  Status[1] = 0;
  SaveBuf   = NodeBuf;

  initquest(Fields,NUMFIELDS);
  readscrn(Fields,NUMFIELDS-1,4,"Edit User Net Status","",1,NOCLEARFLD);
  freeanswers(Fields,NUMFIELDS);

  NodeBuf.Status = Status[0];
  restorescreen(&Screen);

  if (KeyFlags != ESC) {
    switch (NodeBuf.Status) {
      case ' ': NodeBuf.Status = SaveBuf.Status;
                break;
      case 'Y': memset(&NodeBuf,0,sizeof(NodeBuf));
                /* fall thru */
      case 'V': memset(&NodeBuf,0,sizeof(NodeBuf));
                NodeBuf.Status = ' ';
                break;
      case 'Z': memset(&NodeBuf,0,sizeof(NodeBuf));
                break;
    }

    stripright(NodeBuf.Name,' ');
    stripright(NodeBuf.City,' ');
    stripright(NodeBuf.Operation,' ');
    stripright(NodeBuf.Message,' ');

    /* if they changed the message but forgot to change the status, */
    /* then change the status for them...                           */

    if (NodeBuf.Status == SaveBuf.Status && strcmp(NodeBuf.Message,SaveBuf.Message) != 0)
      NodeBuf.Status = 'M';
    if (NodeBuf.Status != 'M')
      NodeBuf.Message[0] = 0;

    if (memcmp(&SaveBuf,&NodeBuf,sizeof(nodetype)) != 0) {
      NodeBuf.LastUpdate.Split.Date = getjuliandate();
      NodeBuf.LastUpdate.Split.Time = (uint) (exacttime() / 2);
      doslseek(UserNetFile,(long) Node*sizeof(nodetype)+USERNETSTART,SEEK_SET);
      if (writecheck(UserNetFile,&NodeBuf,sizeof(nodetype)) != (unsigned) -1) {
        if (NodeBuf.Status != SaveBuf.Status) {
          if (NodeBuf.Status == 0)
            setnetstatusflag(UserNetFile,Node,FALSE);
          if (SaveBuf.Status == 0)
            setnetstatusflag(UserNetFile,Node,TRUE);
          switch (NodeBuf.Status) {
            case 'M':
            case 'R':
            case 'W':
            case 'X': setattentionflags(UserNetFile,Node);
                      break;
          }
        }
      }
    }
  }
  Refresh = TRUE;
  #ifdef __OS2__
    disablekbdmonitor();
    disableshowstatus();
    setscreenupdateinterval(-1);
    DisplaySemaphore.enabletimer(3000);
  #endif
}


/********************************************************************
*
*  Function:  kbdhandler()
*
*  Desc    :  Checks for a key pressed and whether to abort the display
*             or change the nodes being displayed in the window.
*/

void pascal kbdhandler(void) {
  char Key;
  char Ext;

  /* Check for Keyboard CHAR and act Accordingly  */

    #ifdef __OS2__
      bkeytype bKey;

      top:
      bKey.W = bgetkey(0);
      if (bKey.B.LoB == 0 || bKey.B.LoB == 0xE0) {
        Ext = TRUE;
        Key = bKey.B.HiB;
      } else {
        Ext = FALSE;
        Key = bKey.B.LoB;
      }
    #else
      Key = timedinkey(&Ext,WAITPERIOD);
    #endif

  /* ReadFile is used to indicate two things:
       1) That the keyboard timer expired, and
       2) That the disk needs to be re-read to see if there are any updates

     Refresh is used to indicate that we MUST update the ENTIRE screen, but not
     necessarily that the disk needs to be re-read.  So both ReadFile
     and Refresh must be set to true if we want to *force* a disk read and
     a screen update
  */

  Refresh = FALSE;  /* default to updating only what has changed */

  if (Key == 0) {
    ReadFile = TRUE;  /* cause it to re-read the file to see what changed */
  } else {
    ReadFile = FALSE; /* don't need to re-read, just perform requested action */
    if (! Ext) {
      switch (Key) {
        case 13: shell();
                 ReadFile = TRUE;
                 break;
        case 32: getinput();
                 ReadFile = TRUE;
                 break;
        case 27: Abort = 1;
                 return;
      }
    } else {
      switch (Key) {
        case 72: if (LineNum > 2)                  /* Up */
                   LineNum--;
                 else {
                   if (FirstNode > 0)
                     FirstNode--;
                   ReadFile = TRUE;
                   Refresh  = TRUE;
                 }
                 break;
        case 80: if (LineNum <= MaxWindow+1)         /* Down */
                   LineNum++;
                 else {
                   if (FirstNode < MaxNodes-MaxWindow-1)
                     FirstNode++;
                   ReadFile = TRUE;
                   Refresh  = TRUE;
                 }
                 break;
        case 73: FirstNode -= MaxWindow+1;           /* PgUp */
                 if (FirstNode < 0)
                   FirstNode = 0;
                 ReadFile   = TRUE;
                 Refresh    = TRUE;
                 break;
        case 81: FirstNode += MaxWindow+1;           /* PgDn */
                 if (FirstNode>MaxNodes-MaxWindow-1)
                   FirstNode=MaxNodes-MaxWindow-1;
                 ReadFile   = TRUE;
                 Refresh    = TRUE;
                 break;
      }
    }
  }

  #ifdef __OS2__
    // command the display thread to update the screen
    DisplaySemaphore.postevent();
    // under OS/2 this function doesn't return until ESC is pressed
    goto top;
  #endif
}



/********************************************************************
*
*  Function:  display()
*
*  Desc    :  main processing loop
*
*/

#ifdef __OS2__
#pragma argsused
static void _Cdecl display(void *Ignore) {
#else
static void display(void) {
#endif
  bool     Paged;
  int      Count;
  int      PrevLine;
  int      LastLine;
  int      DispLine;
  int      NumBytes;
  char     *Msg;
  nodetype *p;
  nodetype *q;
  nodetype *NodeArray;
  nodetype *SaveArray;
  char     Str[80];
  char     NodeName[80];

  NumBytes = sizeof(nodetype) * (MaxWindow+1);

  if ((NodeArray = (nodetype *) malloc(NumBytes)) == NULL)
    return;

  if ((SaveArray = (nodetype *) malloc(NumBytes)) == NULL) {
    free(NodeArray);
    return;
  }

  memset(NodeArray,0,NumBytes);
  memset(SaveArray,0,NumBytes);

  PrevLine = 2;
  LineNum  = 2;
  Refresh  = TRUE;

  do {
    #ifdef __OS2__
      // We want to wait 3 seconds in between each screen update UNLESS the
      // keyboard thread commands us to update the screen right now.
      if (DisplaySemaphore.waitforevent() == SEM_TIMEDOUT) {
        // ReadFile needs to be set *only* if the 3 second timer expired
        // If the timer *didn't* expire, it's because the keyboard handler told
        // the thread to update the screen
        ReadFile = TRUE;
      }

      // if the monitor is not enabled then reset the semaphore and
      // then go right back to sleep again
      if (! DisplaySemaphore.isenabled()) {
        DisplaySemaphore.reset();
        continue;
      }
    #endif

    /* we only want to hit the disk if the keyboard caused the screen */
    /* context to change, or if the 3 second interval has expired     */

    if (ReadFile) {
      doslseek(UserNetFile,(long) FirstNode*sizeof(nodetype)+USERNETSTART,SEEK_SET);
      if (readcheck(UserNetFile,NodeArray,(MaxWindow+1) * sizeof(nodetype)) == (unsigned) -1) {
        Abort = TRUE;
        break;
      }
      sprintf(Str,"%u",++Reads);
      fastprintmove(70,1,Str,MoniColors[0][CLR_TEXT]);
    }

    /* turn off the highlight bar */
    setatt(1,PrevLine,78,PrevLine,MoniColors[0][CLR_TEXT]);

    Paged    = FALSE;
    DispLine = 2;  /* this is the first line on which node information is shown */

    if (FirstNode+MaxWindow+1 < MaxNodes+1)
      LastLine = FirstNode+MaxWindow+1;
    else
      LastLine = MaxNodes+1;

    /* Loop through and update node displays, but only if we read the file */
    if (ReadFile) {
      for (Count = FirstNode, p = NodeArray, q = SaveArray; Count < LastLine; Count++, p++, q++, DispLine++) {

        /* On demand, we must update the entire screen.  But if the timer  */
        /* simply expired, then we can skip over any records that have not */
        /* changed since the last screen update */
        if (! Refresh) {
          if (memcmp(p,q,sizeof(nodetype)) == 0)
            continue;
        }

        switch (p->Status) {
          case  0 : Msg = "(Inactive Node)";                       break;
          case ' ': Msg = "No Caller this Node";                   break;
          case 'A': Msg = "Available for CHAT";                    break;
          case 'B': Msg = "Out to DOS";                            break;
          case 'C': Msg = "Chatting with Sysop";                   break;
          case 'D': Msg = "Inside a DOOR";                         break;
          case 'E': Msg = "Entering a Message";                    break;
          case 'F': Msg = "Viewing a File";                        break;
          case 'G': Msg = "CHATTING with Group";                   break;
          case 'H': Msg = "Handling Mail";                         break;
          case 'L': Msg = "Auto Logoff Pending";                   break;
          case 'M': Msg = "Rcvd Broadcast Msg";                    break;
          case 'N': Msg = "Running Event";                         break;
          case 'O': Msg = "Logging Into System";                   break;
          case 'P': Msg = "Paging the Sysop";  Paged = TRUE;       break;
          case 'Q': Msg = "Run on Connect";                        break;
          case 'R': Msg = "DOS Recycle Pending";                   break;
          case 'S': Msg = "Answering Script";                      break;
          case 'T': Msg = "Transferring a File";                   break;
          case 'U': Msg = "Unavailable for CHAT";                  break;
          case 'W': Msg = "Drop to DOS at Logoff";                 break;
          case 'X': Msg = "Drop to DOS URGENT";                    break;
        }

        if (p->Operation[0] != 0 &&
            (p->Status == 'D' ||
             p->Status == 'N' ||
             p->Status == ' ')) {
          strcpy(NodeName,p->Operation);
        } else {
          if (p->Name[0] == 0)
            NodeName[0] = 0;
          else {
            strcpy(NodeName,p->Name);
            if (p->City[0] != 0) {
              strcat(NodeName," (");
              strcat(NodeName,p->City);
              strcat(NodeName,")");
            }
          }
        }

        sprintf(Str,"%-5d%-23.23s%-49.49s",Count+1,Msg,NodeName);
        fastprint(2,DispLine,Str,MoniColors[0][CLR_TEXT]);

        #ifndef __OS2__
          if (kbhit()) {
            Paged = FALSE;  /* don't let it beep every time a key is pressed   */
            break;          /* keypressed, quit the loop and update the screen */
          }
        #endif

        *q = *p;
      }
    }

    setatt(1,LineNum,78,LineNum,MoniColors[0][CLR_BAR]);
    PrevLine = LineNum;

    #ifdef __OS2__
      updatelinesnow();
      DisplaySemaphore.reset();
    #else
      kbdhandler();       /* Watch for key or timeout  */
    #endif

    if (Paged)
      mysound(1000,10);

  } while (! Abort);                  /* Are we Finsished Yet!     */

  free(SaveArray);
  free(NodeArray);
}


/********************************************************************
*
*  Function:  main()
*
*  Desc    :  Main Program Begins - Get Command Line Parms
*             argc  Number of Arguments
*             argv  Stores Filenames & # of Nodes
*/

void main(int argc, char *argv[]) {
  /* Define all Variable and String Types */
  #ifndef NORESTORE
    savescrntype SaveWindow;
    int      SaveX;
    int      SaveY;
  #endif
  #ifndef __OS2__
    char far *LinesOnScreenPtr;
  #endif
  int      NumLinesOnScreen;
  char     *p;

  #ifdef DEBUG
    mc_startcheck(erf_standard);
  #endif

/* Check for Command Line Errors       */

  if (argc<2) {
    say("Command Line Error!");
    ExitError:
    say("Command Line Example Below.\r\n\r\n"
         "pcbmoni <usernet.xxx> [nn [mm]]\r\n\r\n"
         "   <usernet.xxx> = complete drive+path+filename\r\n"
         "            [nn] = option number of nodes to monitor\r\n"
         "            [mm] = optional maximum lines on screen\r\n\r\n\r\n"
         "PCBMONI determines the number of nodes to monitor automatically.  You can\r\n"
         "use the [nn] parameter to override the default.\r\n\r\n"
         "PCBMONI will automatically use the entire screen if necessary (25, 43 or 50\r\n"
         "lines depending on screen mode).  You can override the default with the [mm]\r\n"
         "parameter, but you must supply both [nn] and [mm] to change the screen lines.");
    exit(1);
  }

/* Open Files for Read Access Shared   */

  if (fileexist(argv[1]) == 255 || (UserNetFile = dosopencheck(argv[1],OPEN_RDWR|OPEN_DENYNONE)) == -1) {
    say("Unable to open USERNET.XXX file!");
    goto ExitError;
  }
  readcheck(UserNetFile,&UserNetHeader,sizeof(usernethdrtype));
  if (UserNetHeader.Version != 150) {
    say("Wrong version of USERNET.XXX file!");
    goto ExitError;
  }

  initscrnio();

  p = getenv("PCB");
  if (p != NULL) {
    strupr(p);
    if (strstr(p,"NMT") == NULL)
      Novell = TRUE;
    if (strstr(p,"/COLOR") != NULL)
      Scrn_Mode = VID_COLOR;
    if (strstr(p,"/MONO") != NULL)
      Scrn_Mode = VID_MONO;
  }

  if (Scrn_Mode == VID_MONO)
    memcpy(MoniColors[0],MoniColors[1],sizeof(MoniColors)/2);

  checkmultitaskers();

  #ifndef __OS2__
    installhandlers();
  #endif

  #ifdef __OS2__
    NumLinesOnScreen = (getfont() == BIGFONT ? 24 : 49);
  #else
    LinesOnScreenPtr = (char far *) MK_FP(0x40,0x84);
    NumLinesOnScreen = *LinesOnScreenPtr;
    if (NumLinesOnScreen == 0)
      NumLinesOnScreen = 24;
  #endif


/* Initiailize all Variables Before Beginning */

  Abort     = 0;
  Reads     = 0;
  FirstNode = 0;
  ReadFile  = TRUE;

  MaxNodes = UserNetHeader.NumOfNodes;

  if (argc >= 3) {
    MaxNodes = atoi(argv[2]);
    if (MaxNodes > UserNetHeader.NumOfNodes)
      MaxNodes = UserNetHeader.NumOfNodes;
  }

  if (argc >= 4)
    MaxWindow = min(atoi(argv[3])-1,NumLinesOnScreen - 3);
/*  MaxWindow = min(atoi(argv[3]),NumLinesOnScreen - 3); */
  else
    MaxWindow = NumLinesOnScreen - 3;

  if (MaxNodes < 1 ) MaxNodes=1;

  if (MaxNodes-1 < MaxWindow) MaxWindow = MaxNodes-1;
/*if (MaxNodes < MaxWindow) MaxWindow = MaxNodes;*/

#ifndef NORESTORE
  SaveX = wherex();
  SaveY = wherey();
  savescreen(&SaveWindow);
#else
  cls();
#endif

  boxcls(0,0,79,(char)(3+MaxWindow),MoniColors[0][CLR_TEXT],DOUBLE);
  fastcenter(0,"[ PCBoard Node Monitoring Utility Program ]",MoniColors[0][CLR_TITLE]);

  if (MaxNodes-1 > MaxWindow)
    fastcenter(3+MaxWindow,"[ (\x18), (\x19), (Esc) to End ]",MoniColors[0][CLR_TITLE]);
  else
    fastcenter(3+MaxWindow,"[ (Esc) to End ]",MoniColors[0][CLR_TITLE]);

  fastprint(2,1,"#     Status                User                             Reads:",MoniColors[0][CLR_HEAD]);

  if ((p = getenv("COMSPEC")) != NULL)
    strcpy(ComSpec,p);
  else
    strcpy(ComSpec,"COMMAND.COM");

  #ifdef __OS2__
    setscreenupdateinterval(-1); // force update thread to update only on demand
    DisplaySemaphore.create(NULL);
    DisplaySemaphore.enabletimer(3000);
    _beginthread(display,4096,NULL);
    kbdhandler();
  #else
    display();
  #endif

  dosclose(UserNetFile);

#ifndef NORESTORE
  restorescreen(&SaveWindow);
  gotoxy(SaveX,SaveY);
#else
  gotoxy(0,MaxWindow+3);
#endif

#ifdef DEBUG
  mc_endcheck();
#endif
}

