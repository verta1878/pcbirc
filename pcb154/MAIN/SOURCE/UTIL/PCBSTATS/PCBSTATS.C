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


#include <dir.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include <scrnio.h>
#include <system.h>
#include <dosfunc.h>
#include <misc.h>

typedef enum { UPDATE,LOGOFF,RESETNODE,RESETALL,MAKELOCAL } statupdatetype;
enum {SYSTEM,NODE,TOTAL};

typedef struct {
  char LastCaller[54];
  char Time[6];
  long NewMsgs;
  long NewCalls;
  long TotalUp;
  long TotalDn;
  bool LocalStats;
} stattype;


char FileName[256];
char Name[256];
char City[256];
int  Node;
int  NumNodes;

extern struct ffblk DTA;  /* declared in exist.c */


int near pascal openfile(void) {
  int  File;
  long Size;

  if (fileexist(FileName) == 255) {
    printf("unable to locate filename specified by /FILE:%s\n",FileName);
    return(-1);
  }

  Size = DTA.ff_fsize;
  NumNodes = (int) (Size/sizeof(stattype));

//printf("File Size = %ld, NumNodes = %d\n",Size,NumNodes);

  if (Node < 0 || Node > NumNodes) {
    printf("/NODE:%d  specifies a node number that is not in the PCBSTATS.DAT file\n",Node);
    return(-1);
  }

  if ((File = dosopencheck(FileName,OPEN_RDWR|OPEN_DENYWRIT)) == -1) {
    printf("unable to open filename specified by /FILE:%s\n",FileName);
    return(-1);
  }
  return(File);
}


void near pascal showstats(int Node) {
  stattype Stats[TOTAL];
  int File;

  if ((File = openfile()) == -1)
    return;

  if (Node == 0) {
    readcheck(File,&Stats[SYSTEM],sizeof(stattype));
    printf("SYSTEM  -> ");
    printf("Calls: %5ld  Messages: %5ld  Downloads: %5ld  Uploads: %5ld\n",Stats[SYSTEM].NewCalls,Stats[SYSTEM].NewMsgs,Stats[SYSTEM].TotalDn,Stats[SYSTEM].TotalUp);
  } else if (Node == -1) {
    readcheck(File,&Stats[SYSTEM],sizeof(stattype));
    printf("SYSTEM  -> ");
    printf("Calls: %5ld  Messages: %5ld  Downloads: %5ld  Uploads: %5ld\n",Stats[SYSTEM].NewCalls,Stats[SYSTEM].NewMsgs,Stats[SYSTEM].TotalDn,Stats[SYSTEM].TotalUp);
    for (Node = 1; Node < NumNodes; Node++) {
      readcheck(File,&Stats[NODE],sizeof(stattype));
      printf("Node %2d -> ",Node);
      printf("Calls: %5ld  Messages: %5ld  Downloads: %5ld  Uploads: %5ld\n",Stats[NODE].NewCalls,Stats[NODE].NewMsgs,Stats[NODE].TotalDn,Stats[NODE].TotalUp);
    }
  } else {
    doslseek(File,Node*sizeof(stattype),SEEK_SET);
    readcheck(File,&Stats[NODE],sizeof(stattype));
    printf("Node %2d -> ",Node);
    printf("Calls: %5ld  Messages: %5ld  Downloads: %5ld  Uploads: %5ld\n",Stats[NODE].NewCalls,Stats[NODE].NewMsgs,Stats[NODE].TotalDn,Stats[NODE].TotalUp);
  }

  dosclose(File);
}

void near pascal clearstats(int File, bool KeepCaller) {
  stattype Stats;
  char     LastCaller[54];
  char     Time[6];
  int      X;
  int      Y;
  bool     Save;

  doslseek(File,0,SEEK_SET);

  for (X = Y = 0; X < NumNodes; X++, Y += sizeof(stattype)) {
    readcheck(File,&Stats,sizeof(stattype));
    Save = Stats.LocalStats;
    strcpy(LastCaller,Stats.LastCaller);
    strcpy(Time,Stats.Time);
    memset(&Stats,0,sizeof(stattype));
    Stats.LocalStats = Save;
    if (KeepCaller) {
      strcpy(Stats.LastCaller,LastCaller);
      strcpy(Stats.Time,Time);
    }
    doslseek(File,Y,SEEK_SET);
    writecheck(File,&Stats,sizeof(stattype));
  }
}


void near pascal makelocal(int File) {
  stattype Stats;
  int      X;
  int      Y;

  doslseek(File,0,SEEK_SET);

  for (X = Y = 0; X < NumNodes; X++, Y += sizeof(stattype)) {
    readcheck(File,&Stats,sizeof(stattype));
    Stats.LocalStats = TRUE;
    doslseek(File,Y,SEEK_SET);
    writecheck(File,&Stats,sizeof(stattype));
  }
}


void pascal updatestats(long NewMsgs, long NewUpld, long NewDnld, statupdatetype Type, bool KeepCaller) {
  stattype Stats[TOTAL];
  int      File;
  bool     LocalStats;
  bool     UpdateSystem;

  if ((File = openfile()) == -1)
    return;

  readcheck(File,&Stats[SYSTEM],sizeof(stattype));
  if (Node != 0) {
    doslseek(File,Node*sizeof(stattype),SEEK_SET);
    readcheck(File,&Stats[NODE],sizeof(stattype));
  }

  UpdateSystem = TRUE;

  switch (Type) {
    case UPDATE    : Stats[SYSTEM].NewMsgs += NewMsgs;
                     Stats[SYSTEM].TotalUp += NewUpld;
                     Stats[SYSTEM].TotalDn += NewDnld;
                     Stats[NODE].NewMsgs += NewMsgs;
                     Stats[NODE].TotalUp += NewUpld;
                     Stats[NODE].TotalDn += NewDnld;
                     break;
    case LOGOFF    : memset(Stats[SYSTEM].LastCaller,0,sizeof(Stats[SYSTEM].LastCaller));
                     sprintf(Stats[SYSTEM].LastCaller,"%s (%s)",Name,City);
                     timestr2(Stats[SYSTEM].Time);
                     Stats[SYSTEM].NewCalls++;
                     memcpy(Stats[NODE].LastCaller,Stats[SYSTEM].LastCaller,sizeof(Stats[SYSTEM].LastCaller));
                     memcpy(Stats[NODE].Time,Stats[SYSTEM].Time,sizeof(Stats[SYSTEM].Time));
                     Stats[NODE].NewCalls++;
                     break;
    case RESETALL  : clearstats(File,KeepCaller);
                     dosclose(File);
                     return;
    case MAKELOCAL : makelocal(File);
                     dosclose(File);
                     return;
    case RESETNODE : if (Node != 0)
                       UpdateSystem = FALSE;
                     LocalStats = Stats[NODE].LocalStats;
                     memset(Stats,0,sizeof(Stats));
                     Stats[NODE].LocalStats = LocalStats;
                     break;
  }

  if (UpdateSystem) {
    doslseek(File,0,SEEK_SET);
    writecheck(File,&Stats[SYSTEM],sizeof(stattype));
  }

  if (Node != 0) {
    doslseek(File,Node*sizeof(stattype),SEEK_SET);
    writecheck(File,&Stats[NODE],sizeof(stattype));
  }
  dosclose(File);
}


void pascal showoptions(void) {
  printf("The PCBSTATS program updates your PCBSTATS.DAT file outside of PCBoard\n"
         "so that third party software can keep the call waiting screen statistics\n"
         "up to date without having to know the format of the PCBSTATS.DAT file.\n"
         "To use the program type PCBSTATS followed by the parameters:\n"
         "\n"
         "    /FILE:filename  = mandatory - the full name & location of PCBSTATS.DAT\n"
         "    /NODE:nn        = mandatory - node number (or 0 if /S or /D software)\n"
         "\n"
         "Switches used to update statistics\n"
         "    /RESETNODE      = optional  - resets the statistics for the node specified\n"
         "    /RESETALL       = optional  - resets the statistics for all nodes\n"
         "    /MAKELOCAL      = optional  - sets all nodes to LOCAL stats\n"
         "    /KEEPCALLER     = optional  - used with RESETNODE or RESETALL\n"
         "    /MSGS:nnn       = optional  - number of new messages\n"
         "    /UP:nnn         = optional  - number of files uploaded\n"
         "    /DOWN:nnn       = optional  - number of files downloaded\n"
         "    /NAME:name      = optional  - used ONLY on logoff\n"
         "    /CITY:city      = optional  - used ONLY on logoff\n"
         "\n"
         "Switches used to view statistics\n"
         "    /SHOWSYS        = used to display only SYSTEM WIDE statistics\n"
         "    /SHOWNODE       = used to display only NODE SPECIFIC statistics\n"
         "    /SHOWALL        = used to display all statistics at once\n"
         "\n"
         "\n"
         "press enter for examples...");

  bgetkey(0);

  printf("\n"
         "\n"
         "Examples:  Show that on Node 1, John Doe logged off\n"
         "\n"
         "      PCBSTATS /FILE:PCBSTATS.DAT /NODE:1 /NAME:JOHN DOE /CITY:SLC, UT\n"
         "\n"
         "Show that on Node 1, the current user uploaded 1 file, downloaded 2\n"
         "\n"
         "      PCBSTATS /FILE:PCBSTATS.DAT /NODE:1 /UP:1 /DOWN:2\n"
         "\n");
}


void main(int argc, char **argv) {
  char  CmdLine[256];
  bool  Found;
  bool  ResetAll;
  bool  ResetNode;
  bool  MakeLocal;
  bool  KeepCaller;
  int   ArgPtr;
  long  NewMsgs;
  long  Uploads;
  long  Downloads;
  char  *p;
  char  *q;

  ResetAll = ResetNode = KeepCaller = FALSE;
  NewMsgs = Uploads = Downloads = 0;
  Found = FALSE;
  ArgPtr = 1;

  if (argc < 2) {
    showoptions();
    return;
  }

  for (CmdLine[0] = 0, ArgPtr = 1; ArgPtr < argc; ArgPtr++) {
    strupr(argv[ArgPtr]);
    strcat(CmdLine,argv[ArgPtr]);
    strcat(CmdLine," ");
  }

  if ((p = strstr(CmdLine,"/FILE:")) != NULL) {
    strcpy(FileName,p+6);
    if ((p = strchr(FileName,'/')) != NULL) {
      *p = 0;
      stripright(FileName,' ');
    }
  } else {
    showoptions();
    return;
  }

  if ((p = strstr(CmdLine,"/NODE:")) != NULL)
    Node = atoi(p+6);
  else {
    showoptions();
    return;
  }

  if ((p = strstr(CmdLine,"/MSGS:")) != NULL) {
    NewMsgs = atol(p+6);
    Found = TRUE;
  }

  if ((p = strstr(CmdLine,"/UP:")) != NULL) {
    Uploads = atol(p+4);
    Found = TRUE;
  }

  if ((p = strstr(CmdLine,"/DOWN:")) != NULL) {
    Downloads = atol(p+6);
    Found = TRUE;
  }

  if ((p = strstr(CmdLine,"/NAME:")) != NULL && (q = strstr(CmdLine,"/CITY:")) != NULL) {
    strcpy(Name,p+6);
    strcpy(City,q+6);
    if ((p = strchr(Name,'/')) != NULL)
      *p = 0;
    if ((q = strchr(City,'/')) != NULL)
      *q = 0;
    Name[25] = 0;
    City[25] = 0;
    stripright(Name,' ');
    stripright(City,' ');
    Found = TRUE;
  }

  if (strstr(CmdLine,"/RESETALL") != NULL) {
    ResetAll = TRUE;
    Found = TRUE;
  }

  if (strstr(CmdLine,"/RESETNODE") != NULL) {
    ResetNode = TRUE;
    Found = TRUE;
  }

  if (strstr(CmdLine,"/MAKELOCAL") != NULL) {
    MakeLocal = TRUE;
    Found = TRUE;
  }

  if (strstr(CmdLine,"/KEEPCALLER") != NULL)
    KeepCaller = TRUE;

  if (strstr(CmdLine,"/SHOWSYS") != NULL) {
    showstats(0);
    return;
  }

  if (strstr(CmdLine,"/SHOWNODE") != NULL) {
    showstats(Node);
    return;
  }

  if (strstr(CmdLine,"/SHOWALL") != NULL) {
    showstats(-1);
    return;
  }

  if (! Found) {
    showoptions();
    return;
  }

  if (ResetAll)
    updatestats(0,0,0,RESETALL,KeepCaller);
  else if (ResetNode)
    updatestats(0,0,0,RESETNODE,KeepCaller);
  else if (MakeLocal)
    updatestats(0,0,0,MAKELOCAL,KeepCaller);

  if (NewMsgs != 0 || Uploads != 0 || Downloads != 0)
    updatestats(NewMsgs,Uploads,Downloads,UPDATE,FALSE);
  if (Name[0] != 0)
    updatestats(0,0,0,LOGOFF,FALSE);
}
