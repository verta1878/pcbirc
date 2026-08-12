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
  ENTERMESSAGE='E',FILEVIEW='F',GROUPCHAT='G',LOGOFFPENDING='L',
  NODEMESSAGE='M',RUNNINGEVENT='N',LOGINTOSYSTEM='O',PAGINGSYSOP='P',
  RECYCLEBBS='R',ANSWERSCRIPT='S',TRANSFER='T',UNAVAILABLE='U',
  DROPDOSDELAYED='W',DROPDOSNOW='X'
} netstatustype;

typedef struct {
  unsigned Version;     /* PCBoard version number */
  unsigned NumOfNodes;  /* Number of nodes supported in the USERNET.DAT */
  unsigned SizeOfRec;   /* The size of each node record */
} usernethdrtype;

typedef struct {
  char     Status;        /* node status */
  bool     MailWaiting;   /* true if msg posted */
  unsigned Pager;         /* node number of pager */
  char     Name[26];      /* caller's name */
  char     City[25];      /* caller's city */
  char     Operation[49]; /* current operational text */
  char     Message[80];   /* broadcast message text */
  char     Channel;       /* channel number of pager */
  long     LastUpdate;    /* hour:min:sec of last update on this record */
} nodetype;

#define USERNETFLAGSIZE ((Header.NumOfNodes+7)/8)
#define USERNETSTART    (sizeof(usernethdrtype) + (USERNETFLAGSIZE*2))
#define USERNETFLAGS    (sizeof(usernethdrtype) + USERNETFLAGSIZE)

usernethdrtype Header;

void pascal say(char *Str) {
  doswrite(0,Str,strlen(Str));
}

void pascal instruct(void) {
  say("\r\n"
      "This program tests 1 or more nodes in the USERNET.XXX file to see if the node\r\n"
      "is up and has someone online.  If ALL of the nodes specified are offline then\r\n"
      "an errorlevel of 0 is returned.  If one or more of the nodes specified has a\r\n"
      "caller online then an errorlevel of 1 is returned.\r\n"
      "\r\n"
      "usage:  OFFLINE usernet.xxx [down] node1 [node2 [node3 [..]]]\r\n"
      "\r\n"
      "example:   OFFLINE c:\\pcb\\main\\usernet.xxx 1 5 7 9\r\n"
      "\r\n"
      "If the DOWN keyword is used, then OFFLINE returns errorlevel 0 only if all\r\n"
      "specified nodes are completely down.  If any node is up at the call waiting\r\n"
      "screen then an errorlevel of 1 will be returned.\r\n"
      );
}


int main(int argc, char **argv) {
  nodetype Node;
  char FileName[66];
  int  NodeNum;
  int  UserNetFile;
  int  X;
  char *Buf;
  bool ReqDown;

  if (argc < 3) {
    instruct();
    return(1);
  }

  maxstrcpy(FileName,argv[1],sizeof(FileName));
  setdelay();

  if ((UserNetFile = dosopencheck(FileName,OPEN_RDWR|OPEN_DENYNONE)) == -1) {
    say("Unable to open file\r\n");
    return(1);
  }

  if (readcheck(UserNetFile,&Header,sizeof(usernethdrtype)) == -1) {
    say("Unable to read header\r\n");
    return(1);
  }

  if (Header.Version != 150 || Header.SizeOfRec < sizeof(nodetype)) {
    say("Invalid header information\r\n");
    return(1);
  }

  if ((Buf = (char*)malloc(USERNETFLAGSIZE)) == NULL) {
    say("can't allocate buffer for usernet flags\r\n");
    return(1);
  }

  doslseek(UserNetFile,USERNETFLAGS,SEEK_SET);
  if (readcheck(UserNetFile,Buf,USERNETFLAGSIZE) == -1) {
    say("can't read usernet flags\r\n");
    return(1);
  }

  for (X = 2, ReqDown = FALSE; X < argc; X++) {
    if (memicmp(argv[X],"DOWN",4) == 0)
      ReqDown = TRUE;
  }

  if (ReqDown)
    say("Requiring nodes to be at DOS\r\n");
  else
    say("Requiring callers to be offline\r\n");

  for (X = 2; X < argc; X++) {
    NodeNum = atoi(argv[X]);
    if (NodeNum >= 1 && NodeNum <= Header.NumOfNodes) {
      NodeNum--;
      if (isset(Buf,NodeNum)) {
        doslseek(UserNetFile,(NodeNum * sizeof(nodetype)) + USERNETSTART,SEEK_SET);
        if (readcheck(UserNetFile,&Node,sizeof(nodetype)) == -1) {
          say("can't read usernet record\r\n");
          return(1);
        }
        if (ReqDown) {
          if (Node.Status != 0 && Node.Status != RUNNINGEVENT) {
            say("Node ");
            say(argv[X]);
            say(" is still up\r\n");
            return(1);
          }
        }
        if (Node.Status != NOCALLER && Node.Status != 0 && Node.Status != RUNNINGEVENT) {
          say("A caller is online\r\n");
          return(1);
        }
      }
    }
  }

  dosclose(UserNetFile);
  say("All specified nodes down\r\n");
  return(0);
}
