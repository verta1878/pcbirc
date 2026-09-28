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


#ifdef PCBSTATS

#include "project.h"
#pragma hdrstop


#ifdef __WATCOMC__
  #include <io.h>
  #include <borland.h>
#endif

extern struct ffblk DTA;  /* declared in exist.c */

enum {SYSTEM,LOCALNODE,TOTAL};

unsigned static LastTime;

static void _NEAR_ LIBENTRY clearstats(int File, bool Keep) {
  bool     Save;
  int      X;
  long     Y;
  stattype Stats;

  doslseek(File,0,SEEK_SET);
  memset(&Stats,0,sizeof(stattype));

  for (X = 0, Y = 0; X < NUMNODES; X++, Y += sizeof(stattype)) {
    if (Keep) {
      readcheck(File,&Stats,sizeof(stattype));  /*lint !e534 */
      Save = Stats.LocalStats;
      memset(&Stats,0,sizeof(stattype));
      Stats.LocalStats = Save;
      doslseek(File,Y,SEEK_SET);
    }
    writecheck(File,&Stats,sizeof(stattype));    /*lint !e534 */
  }
}


static void _NEAR_ LIBENTRY setlasttime(void) {
  /* record the current time stamp on the pcbstats.dat file so that */
  /* we can later on avoid reading the file if it hasn't changed    */
  if (fileexist(PcbData.StatsFile) != 255)
    LastTime = DTA.ff_ftime;
}


void LIBENTRY updatestats(int NewMsgs, int NewUpld, int NewDnld, statupdatetype Type) {
  bool     UpdateSystem;
  int      File;
  stattype Stats[TOTAL];

  if (fileexist(PcbData.StatsFile) == 255 || DTA.ff_fsize != (long) sizeof(stattype)*NUMNODES) {
    unlink(PcbData.StatsFile);
    memset(Stats,0,sizeof(Stats));
    if ((File = doscreatecheck(PcbData.StatsFile,OPEN_RDWR|OPEN_DENYRDWR,OPEN_NORMAL)) == -1)
      return;
    clearstats(File,FALSE);
  } else {
    if ((File = dosopencheck(PcbData.StatsFile,OPEN_RDWR|OPEN_DENYWRIT)) == -1)
      return;
    readcheck(File,&Stats[SYSTEM],sizeof(stattype));  /*lint !e534 */
    if (PcbData.Network) {
      doslseek(File,(long) PcbData.NodeNum*sizeof(stattype),SEEK_SET);
      readcheck(File,&Stats[LOCALNODE],sizeof(stattype));  /*lint !e534 */
    }
  }

  UpdateSystem = TRUE;

  switch (Type) {
    case LOGOFF    : if (Asy.Online == LOCAL && PcbData.ExcludeLocals) {
                       dosclose(File);
                       return;
                     }
                     memset(Stats[SYSTEM].LastCaller,0,sizeof(Stats[SYSTEM].LastCaller));
                     if (Status.CurConf.AllowAliases && UsersData.Alias[0] != 0 && ! PcbData.ShowAlias) {
                       // set up to get the real name
                       Status.CurConf.AllowAliases = FALSE;
                       getdisplaynames();
                       sprintf(Stats[SYSTEM].LastCaller,"%s (%s)",Status.DisplayName,usercity());
                       // put things back the way they were
                       Status.CurConf.AllowAliases = TRUE;
                       getdisplaynames();
                     } else
                       sprintf(Stats[SYSTEM].LastCaller,"%s (%s)",Status.DisplayName,usercity());
                     timestr2(Stats[SYSTEM].Time);
                     Stats[SYSTEM].NewCalls++;
                     memcpy(Stats[LOCALNODE].LastCaller,Stats[SYSTEM].LastCaller,sizeof(Stats[SYSTEM].LastCaller));
                     memcpy(Stats[LOCALNODE].Time,Stats[SYSTEM].Time,sizeof(Stats[SYSTEM].Time));
                     Stats[LOCALNODE].NewCalls++;
                     /* fall thru to update any new numbers */
    case UPDATE    : if (Asy.Online == LOCAL && PcbData.ExcludeLocals) {
                       dosclose(File);
                       return;
                     }
                     Stats[SYSTEM].NewMsgs += NewMsgs;
                     Stats[SYSTEM].TotalUp += NewUpld;
                     Stats[SYSTEM].TotalDn += NewDnld;
                     Stats[LOCALNODE].NewMsgs += NewMsgs;
                     Stats[LOCALNODE].TotalUp += NewUpld;
                     Stats[LOCALNODE].TotalDn += NewDnld;
                     break;
    case RESETSTATS: if (! PcbData.Network || ! Status.LocalStats) {
                       clearstats(File,TRUE);
                       dosclose(File);
                       return;
                     } else {
                       memset(Stats,0,sizeof(Stats));
                       /* Fall thru */
                     }
    case SETTYPE   : UpdateSystem = FALSE;  /*lint !e616  only update local stats */
                     Stats[LOCALNODE].LocalStats = Status.LocalStats;
                     break;
  }

  if (UpdateSystem) {
    doslseek(File,0,SEEK_SET);
    writecheck(File,&Stats[SYSTEM],sizeof(stattype)); /*lint !e534 */
  }

  if (PcbData.Network) {
    doslseek(File,(long) PcbData.NodeNum*sizeof(stattype),SEEK_SET);
    writecheck(File,&Stats[LOCALNODE],sizeof(stattype));   /*lint !e534 */
  }
  dosclose(File);
  setlasttime();
}


void LIBENTRY getstatstype(void) {
  int      File;
  stattype Stats;

  if (fileexist(PcbData.StatsFile) == 255 || DTA.ff_fsize != (long) sizeof(stattype)*NUMNODES) {
    updatestats(0,0,0,SETTYPE);  /* let updatestats() create the file */
    return;
  }

  if ((File = dosopencheck(PcbData.StatsFile,OPEN_READ|OPEN_DENYNONE)) != -1) {
    doslseek(File,(long) PcbData.NodeNum*sizeof(stattype),SEEK_SET);
    readcheck(File,&Stats,sizeof(stattype));     /*lint !e534 */
    dosclose(File);
    Status.LocalStats = Stats.LocalStats;
  }
}


bool LIBENTRY checkstats(stattype *Stats, bool Refresh) {
  int File;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  if (fileexist(PcbData.StatsFile) != 255) {
    /* make sure it's the right size before continuing */
    if (DTA.ff_fsize != (long) sizeof(stattype)*NUMNODES)
      return(FALSE);
    if (Refresh) {
      /* when refreshing, retry the file if not successful the first time */
      if ((File = dosopencheck(PcbData.StatsFile,OPEN_READ|OPEN_DENYNONE)) == -1)
        return(FALSE);
    } else if (DTA.ff_ftime == LastTime) {
      /* times are the same, no need to re-read the file */
      return(FALSE);
    } else {
      /* we're not refreshing so we don't care if we can't get to it */
      if ((File = dosopen(PcbData.StatsFile,OPEN_READ|OPEN_DENYNONE POS2ERROR)) == -1)
        return(FALSE);
    }
    if (Status.LocalStats && PcbData.Network)
      doslseek(File,(long) PcbData.NodeNum*sizeof(stattype),SEEK_SET);
    readcheck(File,Stats,sizeof(stattype));      /*lint !e534 */
    dosclose(File);
    setlasttime();
    return(TRUE);
  }
  return(FALSE);
}


#pragma warn -par
void LIBENTRY lastcaller(char *CallerName, bool Local) {
  int      File;
  stattype Stats;

  if ((File = dosopencheck(PcbData.StatsFile,OPEN_READ|OPEN_DENYNONE)) != -1) {
    if (Local && PcbData.Network)
      doslseek(File,(long) PcbData.NodeNum*sizeof(stattype),SEEK_SET);
    readcheck(File,&Stats,sizeof(stattype));     /*lint !e534 */
    dosclose(File);
    strcpy(CallerName,Stats.LastCaller);
  }
}
#pragma warn +par
#endif
