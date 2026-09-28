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
#include <pcbtools.h>
#include <misc.h>

#ifdef MEMCHECK
#include <memcheck.h>
#endif

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

typedef enum {NOCALLER=' ',AVAILABLE='A',OUTINDOS='B',CHATWITHSYSOP='C',INADOOR='D',ENTERMESSAGE='E',FILEVIEW='F',GROUPCHAT='G',HANDLEMAIL='H',LOGOFFPENDING='L',NODEMESSAGE='M',
              RUNNINGEVENT='N',LOGINTOSYSTEM='O',PAGINGSYSOP='P',RECYCLEBBS='R',ANSWERSCRIPT='S',TRANSFER='T',UNAVAILABLE='U',DROPDOSDELAYED='W',DROPDOSNOW='X'} netstatustype;

#define USERNETFLAGSIZE ((UserNetHeader.NumOfNodes+7)/8)
#define USERNETSTART    (sizeof(usernethdrtype) + (USERNETFLAGSIZE*2))
#define USERNETFLAGS    (sizeof(usernethdrtype) + USERNETFLAGSIZE)

static int  UserNetFile = 0;
static usernethdrtype UserNetHeader;

static void near pascal readusernetrecord(unsigned NodeNum, nodetype *Buf) {
  long Offset;

  if (UserNetFile > 0 && NodeNum >= 1 && NodeNum <= UserNetHeader.NumOfNodes) {
    Offset = ((long)(NodeNum-1) * sizeof(nodetype)) + USERNETSTART;
    doslseek(UserNetFile,Offset,SEEK_SET);
    readcheck(UserNetFile,Buf,sizeof(nodetype));
  } else memset(Buf,0,sizeof(nodetype));
}


static int near pascal writeusernetrecord(unsigned NodeNum, nodetype *Buf) {
  if (UserNetFile > 0 && NodeNum >= 1 && NodeNum <= UserNetHeader.NumOfNodes) {
    doslseek(UserNetFile,((long)(NodeNum-1) * sizeof(nodetype)) + USERNETSTART,SEEK_SET);
    if (NodeNum == PcbData.NodeNum) {
      Buf->LastUpdate.Split.Date = getjuliandate();
      Buf->LastUpdate.Split.Time = (int) (exacttime() / 2);
    }
    return(writecheck(UserNetFile,Buf,sizeof(nodetype)));
  }
  return(-1);
}


static void near pascal writeusernetstatus(int NetStatus, char *Operation) {
  nodetype Buf;

  if (! PcbData.Network)
    return;

  readusernetrecord(PcbData.NodeNum,&Buf);

  // don't override any of the special settings
  if (Buf.Status != LOGOFFPENDING &&
      Buf.Status != RECYCLEBBS &&
      Buf.Status != DROPDOSDELAYED &&
      Buf.Status != DROPDOSNOW &&
      Buf.Status != HANDLEMAIL)
    Buf.Status = NetStatus;

  maxstrcpy(Buf.Operation,Operation,sizeof(Buf.Operation));
  writeusernetrecord(PcbData.NodeNum,&Buf);
}



static char * near pascal downloadtime(long FileSize, char *Str) {
  int  Minutes;
  int  Tenths;
  int  CPS;
  long CalcSecs;
  long Speed;

  Speed = Asy.CarrierSpeed;
  if (Asy.ModemSpeed > Asy.CarrierSpeed && Asy.ErrorCorrected && ! Status.TimeAdjustedForEvent)
    Speed = (Speed * 6) / 5;   /* 20% boost in speed for MNP (unless an EVENT is coming!) */

  CPS = (int) (Speed / 10);

  /* calculate the number of seconds estimated for the file transfer */
  /* and then add 10 seconds so that at least one TENTH of a minute  */
  /* will be required for the transfer protocol                      */
  CalcSecs = (((FileSize / CPS) * 107) / 100) + 10;
  Minutes  = (int) (CalcSecs / 60);
  Tenths   = ((int) CalcSecs - (Minutes*60)) / 6;

  if (Str != NULL) {
    if (Tenths == 0)
      sprintf(Str," %d",Minutes);
    else
      sprintf(Str," %d.%d",Minutes,Tenths);
  }

  return(Str);
}


void pascal showfileinusernet(char Mode, char *Name, long FileSize) {
  char *p;
  char Time[30];
  char Size[30];
  char Operation[80];

  for (p = Name; *p != 0; p++)      /* skip over drive letter and path */
    if (*p == '\\' || *p == ':')
      Name = p + 1;

  sprintf(Operation,"(%c) %s (ZM) %s  - %s",
          Mode,
          Name,
          comma(Size,FileSize),
          downloadtime(FileSize,Time));
  writeusernetstatus('T',Operation);
}


int pascal openusernet(void) {
  char *p;

  if (! PcbData.Network) {
    UserNetFile = 0;
    return(-1);
  }

  if ((p = strstr(PcbData.NetFile,"USERNET.DAT")) != NULL)
    memcpy(p+8,"XXX",3);

  if ((UserNetFile = dosopencheck(PcbData.NetFile,OPEN_RDWR|OPEN_DENYNONE)) != -1) {
    if (readcheck(UserNetFile,&UserNetHeader,sizeof(usernethdrtype)) == -1 || UserNetHeader.Version != 150) {
      dosclose(UserNetFile);
      UserNetFile = 0;
      return(-1);
    }
  }
  return(0);
}


void pascal closeusernet(void) {
  if (UserNetFile > 0) {
    dosclose(UserNetFile);
    UserNetFile = 0;
  }
}
