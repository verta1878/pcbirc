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
#include <dosfunc.h>
#include <misc.h>
#include <newdata.h>

#ifdef DEBUG
#include <memcheck.h>
#endif

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

static DOSFILE UserNet;
static long NumNodes;
static long UserNetFlagSize;
static long UserNetStart;


unsigned pascal foundinusernet(char *Name) {
  nodetype Node;
  int      Count;

  // if no network, then the user can't be found in the usernet!
  if (! PcbData.Network)
    return(0);

  stripright(Name,' ');
  dosfseek(&UserNet,UserNetStart,SEEK_SET);

  for (Count = 1; Count <= NumNodes; Count++) {
    if (dosfread(&Node,sizeof(nodetype),&UserNet) == -1) {
      dosfclose(&UserNet);
      return(0);
    }
    if (Node.Status == 0)
      continue;
    if (Name[0] == Node.Name[0]) {
      if (strcmp(&Name[1],&Node.Name[1]) == 0)
        return(Count);
    }
    // check for alias by checking to see if the status is "Logging into System"
    // and the Operation field is set to the user name
    if (Node.Status == 'O' && Name[0] == Node.Operation[0]) {
      if (strcmp(&Name[1],&Node.Operation[1]) == 0)
        return(Count);
    }
  }
  return(0);
}


int pascal openusernet(void) {
  usernethdrtype Header;

  // if no network then exit, but don't return an error because we don't want
  // to abort the operation
  if (! PcbData.Network)
    return(0);

  if (dosfopen(PcbData.NetFile,OPEN_RDWR|OPEN_DENYNONE,&UserNet) == -1)
    return(-1);

  if (dosfread(&Header,sizeof(usernethdrtype),&UserNet) == -1) {
    dosfclose(&UserNet);
    return(-1);
  }

  if (Header.Version == 150 && Header.SizeOfRec == sizeof(nodetype)) {
    NumNodes = Header.NumOfNodes;
    UserNetFlagSize = ((NumNodes+7)/8);
    UserNetStart    = (sizeof(usernethdrtype) + (UserNetFlagSize*2));
  } else {
    dosfclose(&UserNet);
    return(-1);
  }

  dossetbuf(&UserNet,8192);
  return(0);
}


void pascal closeusernet(void) {
  if (! PcbData.Network)
    return;
  dosfclose(&UserNet);
}
