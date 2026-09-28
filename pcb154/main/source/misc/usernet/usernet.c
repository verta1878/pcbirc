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


#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <misc.h>
#include <screen.h>
#include <dosfunc.h>

enum {OFF = 0, ON = 1};

typedef enum {
  NOCALLER=' ',AVAILABLE='A',OUTINDOS='B',CHATWITHSYSOP='C',INADOOR='D',
  ENTERMESSAGE='E',GROUPCHAT='G',HANDLEMAIL='H',LOGOFFPENDING='L',
  NODEMESSAGE='M',RUNNINGEVENT='N',LOGINTOSYSTEM='O',PAGINGSYSOP='P',
  RECYCLEBBS='R',ANSWERSCRIPT='S',TRANSFER='T',UNAVAILABLE='U',
  DROPDOSDELAYED='W',DROPDOSNOW='X', OFFLINE = 0
} netstatustype;

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
  uint     Pager;         /* node number of pager */
  char     Name[26];      /* caller's name */
  char     City[25];      /* caller's city */
  char     Operation[49]; /* current operational text */
  char     Message[80];   /* broadcast message text */
  char     Channel;       /* channel number of pager */
  updttype LastUpdate;    /* hour:min:sec of last update on this record */
} nodetype;
#pragma pack()

#define USERNETFLAGSIZE ((Header.NumOfNodes+7)/8)
#define USERNETSTART    (sizeof(usernethdrtype) + (USERNETFLAGSIZE*2))
#define USERNETFLAGS    (sizeof(usernethdrtype) + USERNETFLAGSIZE)

usernethdrtype Header;

void pascal instruct(void) {
static char *Str =
       "\r\n"
       "Usage:  USERNET filename nodenum [status [name [city [text]]]]\r\n\n"

       "where:  filename  = the full name and path of USERNET.XXX\r\n\t"
               "nodenum   = the node number to modify (or type ALL for all nodes)\r\n\t"
               "status    = the status of the node (see below)\r\n\t"
               "name      = '*' for no change, or the caller's name (max 25 bytes)\r\n\t"
               "city      = '*' for no change, or the caller's city (max 24 bytes)\r\n\t"
               "text      = '*' for no change, or any text (max 49 bytes)\r\n\n"
       "NOTE: if multiple words are entered for name or city you must enclose them\r\n"
       "within quotation marks (e.g. \"JOHN DOE\" \"NOWHERE, USA\").\r\n\n"

       "status values:\r\n"
       "A=Available for chat  H=Handling Mail   R=Recycle BBS        Y=No caller online\r\n"
       "B=Out of code in DOS  L=Logoff pending  S=Answering Script   Z=Node is offline\r\n"
       "C=Chat with Sysop     M=Message         T=Transferring File  Z##=Off after ##\r\n"
       "D=In a DOOR           N=Running Event   U=Unavail for chat\r\n"
       "E=Entering a message  O=Logging in      W=Wait/Drop DOS\r\n"
       "G=Group Chat          P=Paging Sysop    X=Drop to DOS now\r\n\n"

       "With 'Y' and 'Z' - no name or city parameters are needed\r\n"
       "With 'D' - any text given is automatically combined with the user name\r\n"
       "Z120 would show node offline after 120 minutes of inactivity\r\n";

  say(Str);
// doswrite(1,Str,strlen(Str));
}


void pascal setattentionflags(int UserNetFile, unsigned NodeNum) {
  int  Offset;
  int  BitMask;
  char Flag;

  BitMask = 1 << (NodeNum & 7);
  Offset  = sizeof(usernethdrtype) + (NodeNum / 8);

  doslseek(UserNetFile,Offset,SEEK_SET);
  if (doslockcheck(UserNetFile,Offset,sizeof(char)) != -1) {
    readcheck(UserNetFile,&Flag,sizeof(char));
    Flag |= (char) BitMask;
    doslseek(UserNetFile,Offset,SEEK_SET);
    writecheck(UserNetFile,&Flag,sizeof(char));
    unlock(UserNetFile,Offset,sizeof(char));
  }
}

static void _NEAR_ pascal setnetstatusflag(int UserNetFile, int NodeNum, bool On) {
  int  Offset;
  int  BitMask;
  char Flag;

  BitMask = 1 << (NodeNum & 7);
  Offset  = USERNETFLAGS + (NodeNum / 8);

  doslseek(UserNetFile,Offset,SEEK_SET);
  if (doslockcheck(UserNetFile,Offset,sizeof(char)) != -1) {
    readcheck(UserNetFile,&Flag,sizeof(char));
    if (On)
      Flag |= (char) BitMask;
    else
      Flag &= (char) ~BitMask;
    doslseek(UserNetFile,Offset,SEEK_SET);
    writecheck(UserNetFile,&Flag,sizeof(char));
    unlock(UserNetFile,Offset,sizeof(char));
  }
}




void main(int argc, char **argv) {
  static char Table[26] = {'A','B','C','D','E',0,'G','H',0,0,0,'L','M','N','O','P',0,'R','S','T','U',0,'W','X',' ','Z'};
  long        Offset;
  updttype    CurrentTime;
  int         NodeNum;
  int         Start;
  int         End;
  int         UserNetFile;
  int         Temp;
  long        CheckTime;
  char        Status;
  char        FileName[66];
  char        Str[128];
  nodetype    Node;

  if (argc < 3 || argc > 7) {
    instruct();
    return;
  }

  maxstrcpy(FileName,argv[1],sizeof(FileName));

  if (argc >= 4) {
    Temp = (argv[3][0] & 0x5F) - 'A';  /* uppercase it and subtract 'A' */
    if (Temp < 0 || Temp > 'Z'-'A') {
      instruct();
      return;
    }
    if (Temp == 'Z'-'A')
      Status = OFFLINE;
    else if ((Status = Table[Temp]) == 0) {
      instruct();
      return;
    }
  } else Status = ' ';

  CheckTime = 0;
  if (Status == OFFLINE) {
    CheckTime = atoi(&argv[3][1]) * 60L / 2;  // checktime = # of minutes expressed in half seconds
    CurrentTime.Split.Date = getjuliandate();
    CurrentTime.Split.Time = (short) (exacttime() / 2);
  }

  setdelay();

  if ((UserNetFile = dosopencheck(FileName,OPEN_RDWR|OPEN_DENYNONE)) == -1) {
    say("Unable to open file\r\n");
    return;
  }

  if (readcheck(UserNetFile,&Header,sizeof(usernethdrtype)) == (unsigned) -1) {
    say("Unable to read header\r\n");
    return;
  }

  if (Header.Version != 150 || Header.SizeOfRec < sizeof(nodetype)) {
    say("Invalid header information\r\n");
    return;
  }

  if (stricmp(argv[2],"ALL") == 0) {
    Start = 0;
    End = Header.NumOfNodes - 1;
  } else {
    if ((NodeNum = atoi(argv[2])) < 1 || NodeNum > Header.NumOfNodes) {
      say("Invalid node number\r\n");
      return;
    }
    Start = End = NodeNum - 1;    /* use an offset of 0 instead of 1 */
  }

  Offset = ((long) Start * sizeof(nodetype)) + USERNETSTART;

  if (argc > 4) {
    if (Status != NODEMESSAGE)
      strupr(argv[4]);
    if (argc > 5) {
      strupr(argv[5]);
      if (argc > 6)
        strupr(argv[6]);
    }
  }

  for (NodeNum = Start; NodeNum <= End; NodeNum++, Offset += sizeof(nodetype)) {
    doslseek(UserNetFile,Offset,SEEK_SET);
    readcheck(UserNetFile,&Node,sizeof(nodetype));

    if (Status == NODEMESSAGE) {
      if (argc > 4) {
        if (Node.Status == AVAILABLE ||
            Node.Status == ENTERMESSAGE ||
            Node.Status == GROUPCHAT ||
            Node.Status == PAGINGSYSOP ||
            Node.Status == ANSWERSCRIPT ||
            Node.Status == TRANSFER ||
            Node.Status == UNAVAILABLE) {
          Node.Status = NODEMESSAGE;
          maxstrcpy(Node.Message,argv[4],sizeof(Node.Message));
        }
      }
    } else if (Status == OFFLINE && CheckTime != 0) {
      doslseek(UserNetFile,Offset,SEEK_SET);
      readcheck(UserNetFile,&Node,sizeof(nodetype));

      // if the node is not shown as being down and the last update was more
      // than CheckTime ago, then change the node to offline

      if (Node.Status != OFFLINE && (Node.LastUpdate.DateTime < CurrentTime.DateTime - CheckTime))
        memset(&Node,0,sizeof(nodetype));
    } else {
      if (Status != OFFLINE && Status != 'M') {
        doslseek(UserNetFile,Offset,SEEK_SET);
        readcheck(UserNetFile,&Node,sizeof(nodetype));
        Node.Status = Status;

        if (argc > 4) {
          if (argv[4][0] != '*')
            maxstrcpy(Node.Name,argv[4],sizeof(Node.Name));
          if (argc > 5) {
            if (argv[5][0] != '*')
              maxstrcpy(Node.City,argv[5],sizeof(Node.City));
            if (argc > 6) {
              if (argv[6][0] != '*') {
                if (Status == 'D') {
                  strcpy(Str,Node.Name);
                  strcpy(&Str[strlen(Str)]," - ");
                  strcpy(&Str[strlen(Str)],argv[6]);
                  maxstrcpy(Node.Operation,Str,sizeof(Node.Operation));
                } else
                  maxstrcpy(Node.Operation,argv[6],sizeof(Node.Operation));
              }
            } else if (Status != 'D') {
              memset(Node.Operation,0,sizeof(Node.Operation));
            }
          }
        }
      } else if (Status == OFFLINE) {
        memset(&Node,0,sizeof(nodetype));
      }
    }

    if (Status != 0) {
      Node.LastUpdate.Split.Date = getjuliandate();
      Node.LastUpdate.Split.Time = (uint) (exacttime() / 2);
    }

    doslseek(UserNetFile,Offset,SEEK_SET);
    writecheck(UserNetFile,&Node,sizeof(nodetype));

    switch (Status) {
      case LOGOFFPENDING :
      case NODEMESSAGE   :
      case RECYCLEBBS    :
      case DROPDOSDELAYED:
      case DROPDOSNOW    : setattentionflags(UserNetFile,NodeNum);
    }

    if (Status == OFFLINE)
      setnetstatusflag(UserNetFile,NodeNum,OFF);
    else
      setnetstatusflag(UserNetFile,NodeNum,ON);
  }

  dosclose(UserNetFile);
}
