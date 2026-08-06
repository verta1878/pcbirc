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
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <help.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef __cplusplus
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
  unsigned SizeOfRec;
  unsigned SizeOfConfRec;
} sizetype;

typedef struct {
  char *Name;
  int  Version;
  int  RecSize;
  int  ConfSize;
  bool *Enabled;
} psatype;

enum {ALIAS,ADDRESS,PASSWORD,VERIFY,STATS,NOTES,ACCOUNT,QWKNET,PERSONAL,BANK,TOTALPSAS};


#ifdef PCBSM
static psatype PSAs[TOTALPSAS] = {
  {"PCBALIAS"   ,10, 25,0,&AliasSupport},
  {"PCBADDRESS" ,10,160,0,&AddressSupport},
  {"PCBPASSWORD",10, 42,0,&PasswordSupport},
  {"PCBVERIFY"  ,10, 25,0,&VerifySupport},
  {"PCBSTATS"   ,10, 30,0,&StatsSupport},
  {"PCBNOTES"   ,10,300,0,&NotesSupport},
  {"PCBACCOUNT" ,10,137,0,&AccountSupport},
  {"PCBQWKNET"  ,10, 30,1,&QwkSupport},
  /* 15.4: Personal PSA (Gender/Birthdate/Email/Web) — 1+6+60+60 = 127 bytes */
  {"PCBPERSONAL",10,127,0,&PersonalSupport},
  /* 15.4: Time/Byte Bank — 12 longs = 48 bytes */
  {"PCBBANK"    ,10, 48,0,&BankSupport},
};

static bool PSAName[15];
static bool UpdatingPSAs = FALSE;
static bool AddingPSAs = FALSE;
static bool Removed = FALSE;
#endif


long pascal calcconfsize(unsigned NumAreas) {
  ConfByteLen = (NumAreas >> 3) + ((NumAreas & 0x07) != 0 ? 1 : 0);
  if (ConfByteLen < 5)
    ConfByteLen = 5;
  ExtConfLen = ConfByteLen - 5;

  if (NumAreas > 40)
    NumAreas -= 40;
  else
    NumAreas = 0;

  return(((long) sizeof(long) * NumAreas) + (long)ExtConfLen*3 + ConfByteLen + ConfByteLen);
}


#ifdef PCBSM
int pascal copyinfbytes(DOSFILE *Dest, DOSFILE *Srce, char *Buffer, long Bytes) {
  if (Bytes == 0)
    return(0);

  while (Bytes > INFBUFSIZE) {
    if (dosfread(Buffer,INFBUFSIZE,Srce) != INFBUFSIZE)
      return(-1);
    if (dosfwrite(Buffer,INFBUFSIZE,Dest) == -1)
      return(-1);
    Bytes -= INFBUFSIZE;
  }

  if (Bytes > 0) {
    if (dosfread(Buffer,(int) Bytes,Srce) != (int) Bytes)
      return(-1);
    if (dosfwrite(Buffer,(int) Bytes,Dest) == -1)
      return(-1);
  }
  return(0);
}


int pascal writezeroes(DOSFILE *Dest, char *Buffer, long Bytes) {
  if (Bytes == 0)
    return(0);

  if (Bytes > INFBUFSIZE) {
    memset(Buffer,0,INFBUFSIZE);
    do {
      if (dosfwrite(Buffer,INFBUFSIZE,Dest) == -1)
        return(-1);
      Bytes -= INFBUFSIZE;
    } while (Bytes > INFBUFSIZE);
  } else
    memset(Buffer,0,(int) Bytes);

  if (dosfwrite(Buffer,(int) Bytes,Dest) == -1)
    return(-1);
  return(0);
}


static int near pascal copydifferentsizes(DOSFILE *Dest, DOSFILE *Srce, char *Buffer, long SrceBytes, long DestBytes) {
  int InBytes;
  int OutBytes;

  if (SrceBytes == DestBytes)
    return(copyinfbytes(Dest,Srce,Buffer,SrceBytes));

  while (1) {
    InBytes = (int) min(INFBUFSIZE,SrceBytes);
    if (InBytes != 0) {
      if (dosfread(Buffer,InBytes,Srce) != InBytes)
        return(-1);
      SrceBytes -= InBytes;
    }
    OutBytes = (int) min(INFBUFSIZE,DestBytes);
    if (OutBytes != 0) {
      if (OutBytes > InBytes) {
        if (dosfwrite(Buffer,InBytes,Dest) == -1)
          return(-1);
        DestBytes -= InBytes;
        return(writezeroes(Dest,Buffer,DestBytes));
      } else {
        if (dosfwrite(Buffer,OutBytes,Dest) == -1)
          return(-1);
      }
      DestBytes -= OutBytes;
    } else {
      if (InBytes == 0)
        break;
    }
  }

  return(0);
}


static void near pascal updateoffsets(DOSFILE *Dest, DOSFILE *Srce, unsigned NumApps, long Offset) {
  apptype  App;
  unsigned X;

  dosfseek(Srce,sizeof(hdrtype),SEEK_SET);
  dosfseek(Dest,sizeof(hdrtype),SEEK_SET);

  for (X = 0; X < NumApps; X++) {
    if (dosfread(&App,sizeof(apptype),Srce) != sizeof(apptype))
      break;
    App.Offset = Offset;
    dosfwrite(&App,sizeof(apptype),Dest);   //lint !e534
    Offset += App.SizeOfRec + ((long) App.SizeOfConfRec * PcbData.NumAreas);
  }
}


static int near pascal copyapplication(DOSFILE *Dest,
                                DOSFILE *Srce,
                                char    *Buffer,
                                unsigned OldRecSize,
                                unsigned NewRecSize,
                                unsigned OldConfRecSize,
                                unsigned NewConfRecSize,
                                unsigned OldNumConf) {
  unsigned X;

  if (copydifferentsizes(Dest,Srce,Buffer,OldRecSize,NewRecSize) == -1)
    return(-1);

  if (OldNumConf <= PcbData.NumAreas) {
    for (X = 0; X < OldNumConf; X++)
      if (copydifferentsizes(Dest,Srce,Buffer,OldConfRecSize,NewConfRecSize) == -1)
        return(-1);
    if (writezeroes(Dest,Buffer,(long) NewConfRecSize*(PcbData.NumAreas-OldNumConf)) == -1)
      return(-1);
  } else {
    for (X = 0; X < PcbData.NumAreas; X++)
      if (copydifferentsizes(Dest,Srce,Buffer,OldConfRecSize,NewConfRecSize) == -1)
        return(-1);
    for (; X < OldNumConf; X++)
      if (copydifferentsizes(Dest,Srce,Buffer,OldConfRecSize,0) == -1)
        return(-1);
  }
  return(0);
}


static void near pascal openerror(void) {
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Msg1    = "ERROR!  All nodes need to be down before proceeding!";
  MsgData.Line1   = 18;
  MsgData.Color1  = Colors[HEADING];
  showmessage();
}


void pascal createuserinfo(void) {
  char     Str[80];
  char     Temp1[16];
  char     Temp2[16];
  rectype  Rec;
  DOSFILE  InfOut;
  DOSFILE  UsrIn;
  DOSFILE  UsrOut;
  long     RecNo;
  long     End;
  long     ZeroLen;
  char     *p;
  bool     Answer;

  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,"Create User Info File");

  fastprint(3,6,"This procedure creates a USERS.INF file which holds information about"  ,Colors[DISPLAY]);
  fastprint(3,7,"callers beyond what the v14.0 - v14.2 software requires; including last",Colors[DISPLAY]);
  fastprint(3,8,"message read pointers beyond conference #39.  It is required for using" ,Colors[DISPLAY]);
  fastprint(3,9,"PCBoard v14.5 or v15.x and is transparent to previous releases."        ,Colors[DISPLAY]);

  if (dosfopen(PcbData.UsrFile,OPEN_READ|OPEN_DENYREAD,&UsrIn) == -1) {
    openerror();
    return;
  }

  if (dosfopen(PcbData.UsrFile,OPEN_WRIT|OPEN_DENYWRIT,&UsrOut) == -1) {
    dosfclose(&UsrIn);
    openerror();
    return;
  }

  End = numrecs(UsrIn.handle,sizeof(URead));
  doslseek(UsrIn.handle,0,SEEK_SET);

  sprintf(Str,"Estimated size of USERS.INF file (for %s records): %s",comma(Temp1,End),comma(Temp2,(End*(sizeof(rectype)+calcconfsize(PcbData.NumAreas)))+sizeof(hdrtype)));
  fastprint(3,11,Str,Colors[DISPLAY]);

  if (fileexist(PcbData.InfFile) != 255)
    fastprint(3,13,"WARNING: The USERS.INF file already exists!",Colors[HEADING]);

  setcursor(CUR_NORMAL);
  Answer = FALSE;
  inputnum(3,15,1,"Do you want to create the USERS.INF file now",&Answer,vBOOL,0);
  setcursor(CUR_BLANK);
  if (! Answer || KeyFlags == ESC) {
    dosfclose(&UsrIn);
    dosfclose(&UsrOut);
    return;
  }

  if ((p = (char *) mallochk(INFBUFSIZE)) == NULL) {
    dosclose(UsersFile);
    return;
  }

  if (dosfopen(PcbData.InfFile,OPEN_WRIT|OPEN_CREATE|OPEN_DENYRDWR,&InfOut) == -1) {
    free(p);
    dosfclose(&UsrOut);
    dosfclose(&UsrIn);
    openerror();
    return;
  }

  // turn them all OFF since we're about to create NEW FILE!
  AliasSupport = VerifySupport = AddressSupport = PasswordSupport =
  StatsSupport = NotesSupport = AccountSupport = QwkSupport = FALSE;
  setpsaorder();

  dossetbuf(&InfOut,4096);  /* try for bigger buffers */
  dossetbuf(&UsrOut,8192);
  dossetbuf(&UsrIn,4096);

  Header.Version      = 145;
  Header.NumOfConf    = (PcbData.NumConf > 39 ? PcbData.NumConf - 39 : 0);
  Header.SizeOfRec    = sizeof(rectype);
  Header.SizeOfConf   = calcconfsize(PcbData.NumAreas);
  Header.NumOfApps    = 0;
  Header.TotalRecSize = (long) sizeof(rectype)+(2L*ConfByteLen)+(3L*ExtConfLen)+(ExtConfLen > 0 ? ((long) PcbData.NumAreas-40)*sizeof(long) : 0);

  ZeroLen = Header.TotalRecSize - sizeof(rectype);

  dosfwrite(&Header,sizeof(hdrtype),&InfOut);    //lint !e534

  memset(&Rec,0,sizeof(rectype));
  sprintf(Str,"There are %s records to be processed in the USERS file...",comma(Temp1,End));
  fastprint(3,18,Str,Colors[DISPLAY]);
  fastprint(3,20,"Processing #     1:",Color[DISPLAY]);

  for (RecNo = 1; RecNo <= End; RecNo++) {
    if (dosfread(&UsersRead,sizeof(URead),&UsrIn) != sizeof(URead))
      break;

    sprintf(Str,"%6ld",RecNo);
    fastprint(15,20,Str,Color[DISPLAY]);
    memcpy(Rec.Name,UsersRead.Name,sizeof(Rec.Name));
    fastprint(23,20,Rec.Name,Color[DISPLAY]);

    if (dosfwrite(&Rec,sizeof(rectype),&InfOut) == -1)
      break;
    if (writezeroes(&InfOut,p,ZeroLen) == -1)
      break;

    UsersRead.RecNum = RecNo;
    UsersRead.PackedFlags.HasMail = FALSE;
    UsersRead.LastConf2 = UsersRead.LastConference;
    if (dosfwrite(&UsersRead,sizeof(URead),&UsrOut) == -1)
      break;
  }

  free(p);
  dosfclose(&UsrOut);
  dosfclose(&UsrIn);
  dosfclose(&InfOut);
}


/*
void pascal judnewell(void) {
  char     Str[80];
  char     Temp1[16];
  char     Temp2[16];
  DOSFILE  Out;
  rectype  Rec;
  bool     Answer;
  char     *p;
  long     Loc;
  long     RecNum;
  long     End;
  long     ZeroLen;
  long     NumInfRecs;

  if (openusersfile() == -1)
    return;

  End = numrecs(UsersFile,sizeof(URead));
  doslseek(UsersFile,0,SEEK_SET);

  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,"Jud Newell'ers Stuff");

  fastprint(3,6,"HI JUD - this bud's for you....",Colors[DISPLAY]);

  sprintf(Str,"Estimated size of USERS.INF file (for %s records): %s",comma(Temp1,End),comma(Temp2,(End*(sizeof(rectype)+calcconfsize(PcbData.NumAreas)))+sizeof(hdrtype)));
  fastprint(3,11,Str,Colors[DISPLAY]);

  setcursor(CUR_NORMAL);
  Answer = FALSE;
  inputnum(3,15,1,"Do you want to extend the USERS.INF file now",&Answer,BOOL,0);
  setcursor(CUR_BLANK);
  if (! Answer || KeyFlags == ESC) {
    dosclose(UsersFile);
    return;
  }

  if ((p = mallochk(INFBUFSIZE)) == NULL) {
    dosclose(UsersFile);
    return;
  }

  if (dosfopen(PcbData.InfFile,OPEN_WRIT|OPEN_DENYNONE,&Out) == -1) {
    free(p);
    dosclose(UsersFile);
    return;
  }

  dossetbuf(&Out,16384);  /* try for a bigger buffer */

  Header.Version      = 145;
  Header.NumOfConf    = (PcbData.NumConf > 39 ? PcbData.NumConf - 39 : 0);
  Header.SizeOfRec    = sizeof(rectype);
  Header.SizeOfConf   = calcconfsize(PcbData.NumAreas);
  Header.NumOfApps    = 0;
  Header.TotalRecSize = sizeof(rectype)+(2*ConfByteLen)+(3*ExtConfLen)+(ExtConfLen > 0 ? (PcbData.NumAreas-40)*sizeof(long) : 0);
  NumInfRecs          = (dosfseek(&Out,0,SEEK_END) - sizeof(hdrtype)) / Header.TotalRecSize;

  if (NumInfRecs >= End)
    goto exit;

  ZeroLen = Header.TotalRecSize - sizeof(rectype);

  Loc = doslseek(UsersFile,NumInfRecs*sizeof(URead),SEEK_SET);

  memset(&Rec,0,sizeof(rectype));
  sprintf(Str,"There are %s records to be processed in the USERS file...",comma(Temp1,End));
  fastprint(3,18,Str,Colors[DISPLAY]);
  fastprint(3,20,"Processing #     1:",Color[DISPLAY]);

  for (RecNum = NumInfRecs+1; RecNum <= End; RecNum++, Loc += sizeof(URead)) {
    if (readcheck(UsersFile,&UsersRead,sizeof(URead)) == -1)
      break;

    sprintf(Str,"%6ld",RecNum);
    fastprint(15,20,Str,Color[DISPLAY]);
    memcpy(Rec.Name,UsersRead.Name,sizeof(Rec.Name));
    fastprint(23,20,Rec.Name,Color[DISPLAY]);

    if (dosfwrite(&Rec,sizeof(rectype),&Out) == -1)
      break;
    if (writezeroes(&Out,p,ZeroLen) == -1)
      break;

    UsersRead.RecNum = RecNum;
    UsersRead.PackedFlags.HasMail = FALSE;
    UsersRead.LastConf2 = UsersRead.LastConference;
    doslseek(UsersFile,Loc,SEEK_SET);
    if (writecheck(UsersFile,&UsersRead,sizeof(URead)) == -1)
      break;
  }

exit:
  free(p);
  dosfclose(&Out);
  dosclose(UsersFile);
}
*/


void pascal updateuserinfo(void) {
  bool      NeedToUpdate;
  bool      Answer;
  unsigned  X;
  unsigned  OldNumConf;
  int       OldByteLen;
  int       OldExtLen;
  char     *p;
  sizetype *Sizes;
  long      Num;
  long      RecNo;
  long      PcbRecSize;
  long      TotalRecs;
  long      OldStaticSize;
  long      NewStaticSize;
  long      TpaStaticSize;
  long      OldDynamicSize;
  long      NewDynamicSize;
  long      TpaDynamicSize;
  char      Temp1[16];
  char      Temp2[16];
  hdrtype   NewHeader;
  DOSFILE   In;
  DOSFILE   Out;
  char      Str[80];

  Sizes = NULL;

  if ((p = (char *) mallochk(INFBUFSIZE)) == NULL)
    return;

  #ifdef JUDNEWELL
    if (copyfile(PcbData.InfFile,"V:USERS.IBK",FALSE) != 0) {
      free(p);
      return;
    }
    if (dosfopen("V:USERS.IBK",OPEN_READ|OPEN_DENYRDWR,&In) == -1) {
      openerror();
      free(p);
      return;
    }
  #else
    if (dosfopen(PcbData.InfFile,OPEN_READ|OPEN_DENYRDWR,&In) == -1) {
      openerror();
      free(p);
      return;
    }
  #endif

  if ((UsersFile = dosopencheck(PcbData.UsrFile,OPEN_RDWR|OPEN_DENYRDWR)) == -1) {
    openerror();
    goto exit;
  }

  TotalRecs = numrecs(UsersFile,sizeof(URead));
  dosclose(UsersFile);

  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,"Update User Info File");
  fastprint(3,4,"Current User Info file allocations are as follows:" ,Colors[DISPLAY]);

  if (dosfread(&Header,sizeof(hdrtype),&In) != sizeof(hdrtype))
    goto exit;

  OldStaticSize  = Header.SizeOfRec;
  OldDynamicSize = Header.SizeOfConf;
  TpaStaticSize  = 0;
  TpaDynamicSize = 0;

  NewHeader.Version      = 145;
  NewHeader.NumOfConf    = (PcbData.NumConf > 39 ? PcbData.NumConf - 39 : 0);
  NewHeader.SizeOfRec    = sizeof(rectype);
  NewHeader.SizeOfConf   = calcconfsize(PcbData.NumAreas);
  NewHeader.NumOfApps    = Header.NumOfApps;
  NewHeader.TotalRecSize = sizeof(rectype)+((long) 2*ConfByteLen)+((long) 3*ExtConfLen)+(ExtConfLen > 0 ? ((long) PcbData.NumAreas-40)*sizeof(long) : 0);
  PcbRecSize             = NewHeader.TotalRecSize;

  if (Header.NumOfApps != 0) {
    if ((Sizes = (sizetype *) malloc(Header.NumOfApps * sizeof(sizetype))) == NULL) {
      dosfclose(&Out);
      dosfclose(&In);
      free(p);
      return;
    }
    for (X = 0; X < Header.NumOfApps; X++) {
      if (dosfread(&AppHeader,sizeof(apptype),&In) != sizeof(apptype))
        goto exit;
      Sizes[X].SizeOfRec      = AppHeader.SizeOfRec;
      Sizes[X].SizeOfConfRec  = AppHeader.SizeOfConfRec;
      TpaStaticSize          += AppHeader.SizeOfRec;
      TpaDynamicSize         += AppHeader.SizeOfConfRec;
      NewHeader.TotalRecSize += AppHeader.SizeOfRec + ((long) AppHeader.SizeOfConfRec * PcbData.NumAreas);
    }
  }

  /* we calculate the old number of conferences by sensing how much space  */
  /* was allocated inside the USERS.INF file for third party dynamic usage */
  /* I think that if none was allocated then we don't need to know the     */
  /* number so set it to zero to avoid a divide by zero error              */
  if (TpaDynamicSize == 0)
    OldNumConf = 0;
  else
    OldNumConf = (unsigned) ((Header.TotalRecSize - OldStaticSize - OldDynamicSize - TpaStaticSize) / TpaDynamicSize);

  OldStaticSize  += TpaStaticSize;
  OldDynamicSize += TpaDynamicSize * OldNumConf;
  NewStaticSize   = sizeof(rectype) + TpaStaticSize;
  NewDynamicSize  = calcconfsize(PcbData.NumAreas) + (TpaDynamicSize * PcbData.NumAreas);

  NeedToUpdate = FALSE;

  sprintf(Str,"Total Conferences Areas:%15s",comma(Temp1,PcbData.NumAreas));
  fastprint(3,6,Str,Colors[DISPLAY]);
  Num = (PcbData.NumConf > 39 ? PcbData.NumConf - 39 : 0);
  sprintf(Str,"Number of EXTENDED Conferences:%8s",comma(Temp1,Header.NumOfConf));
  fastprint(3,7,Str,Colors[DISPLAY]);
  sprintf(Str,"(needed:%9s)",comma(Temp1,Num));
  fastprint(45,7,Str,Colors[Num != Header.NumOfConf ? HEADING : DISPLAY]);
  NeedToUpdate = (Num != Header.NumOfConf);

  sprintf(Str,"Total static user allocation:%10s",comma(Temp1,OldStaticSize));
  fastprint(3,8,Str,Colors[DISPLAY]);
  sprintf(Str,"(needed:%9s)",comma(Temp1,NewStaticSize));
  fastprint(45,8,Str,Colors[NewStaticSize != OldStaticSize ? HEADING : DISPLAY]);
  NeedToUpdate |= (NewStaticSize != OldStaticSize);  //lint !e514 usage of boolean is okay

  sprintf(Str,"Total dynamic allocations:%13s",comma(Temp1,OldDynamicSize));
  fastprint(3,9,Str,Colors[DISPLAY]);
  sprintf(Str,"(needed:%9s)",comma(Temp1,NewDynamicSize));
  fastprint(45,9,Str,Colors[NewDynamicSize != OldDynamicSize ? HEADING : DISPLAY]);
  NeedToUpdate |= (NewDynamicSize != OldDynamicSize); //lint !e514 usage of boolean is okay

  sprintf(Str,"For %s records the resultant filesize will be: %s",comma(Temp1,TotalRecs),comma(Temp2,sizeof(hdrtype) + ((long) Header.NumOfApps * sizeof(apptype)) + (long) TotalRecs * ((long) NewStaticSize + NewDynamicSize)));
  fastprint(3,11,Str,Colors[DISPLAY]);

  if (NeedToUpdate)
    fastprint(3,13,"WARNING:  One or more of the above allocations requires updating",Colors[HEADING]);
  else
    fastprint(3,13,"None of the above allocations require upgrading at this time...",Colors[DISPLAY]);

  setcursor(CUR_NORMAL);
  Answer = FALSE;
  inputnum(3,15,1,"Do you want to update the USERS.INF file",&Answer,vBOOL,0);
  setcursor(CUR_BLANK);
  if (! Answer || KeyFlags == ESC) {
    dosfclose(&In);
    if (Sizes != NULL)
      free(Sizes);
    free(p);
    return;
  }

  if (NewHeader.TotalRecSize < Header.TotalRecSize) {
    setcursor(CUR_NORMAL);
    Answer = FALSE;
    fastprint(3,17,"WARNING: shrinking the file may cause existing message pointers to be lost!",Colors[HEADING]);
    inputnum(3,18,1,"Do you want to continue",&Answer,vBOOL,0);
    setcursor(CUR_BLANK);
    if (! Answer || KeyFlags == ESC) {
      dosfclose(&In);
      free(p);
      return;
    }
  }

  #ifdef JUDNEWELL
    if (dosfopen(PcbData.InfFile,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1) {
      dosfclose(&In);
      free(p);
      return;
    }
  #else
    if (dosfopen(TempInfFileName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1) {
      dosfclose(&In);
      free(p);
      return;
    }
  #endif

  dossetbuf(&In,8192);   /* try for a larger buffer */
  dossetbuf(&Out,8192);  /* try for a larger buffer */

  if (Header.NumOfConf == 0) {
    OldByteLen = 5;
    OldExtLen = 0;
  } else {
    OldByteLen = ((Header.NumOfConf+40) >> 3) + (((Header.NumOfConf+40) & 0x07) != 0 ? 1 : 0);
    OldExtLen = OldByteLen - 5;
  }

  dosfwrite(&NewHeader,sizeof(hdrtype),&Out);    //lint !e534
  dosfseek(&In,sizeof(hdrtype),SEEK_SET);
  for (X = 0; X < Header.NumOfApps; X++) {
    if (copyinfbytes(&Out,&In,(char *) &AppHeader,sizeof(apptype)) == -1)
      goto exit;
  }

  memset(&UsersRec,0,sizeof(rectype));
  sprintf(Str,"There are %s records to be processed in the USERS.INF file...",comma(Temp1,TotalRecs));
  fastprint(3,20,Str,Colors[DISPLAY]);
  fastprint(3,22,"Processing #     1:",Color[DISPLAY]);

  for (RecNo = 1; RecNo <= TotalRecs; RecNo++) {
    /* read in the OLD SIZED user record into a ZERO padded record */
    if (dosfread(&UsersRec,Header.SizeOfRec,&In) != Header.SizeOfRec)
      goto exit;

    /* show it on the screen */
    sprintf(Str,"%6ld",RecNo);
    fastprint(15,22,Str,Color[DISPLAY]);
    memcpy(Str,UsersRec.Name,sizeof(UsersRec.Name));
    Str[sizeof(UsersRec.Name)] = 0;
    fastprint(23,22,Str,Color[DISPLAY]);

    /* and write it out to disk */
    if (dosfwrite(&UsersRec,sizeof(rectype),&Out) == -1)
      goto exit;

    /* copy the "Has Mail" flags */
    if (copydifferentsizes(&Out,&In,p,OldByteLen,ConfByteLen) == -1)
      goto exit;

    /* copy the "Conference Sysop" flags */
    if (copydifferentsizes(&Out,&In,p,OldByteLen,ConfByteLen) == -1)
      goto exit;

    /* copy the "registered" flags */
    if (copydifferentsizes(&Out,&In,p,OldExtLen,ExtConfLen) == -1)
      goto exit;

    /* copy the "expired" flags */
    if (copydifferentsizes(&Out,&In,p,OldExtLen,ExtConfLen) == -1)
      goto exit;

    /* copy the "scan" flags */
    if (copydifferentsizes(&Out,&In,p,OldExtLen,ExtConfLen) == -1)
      goto exit;

    /* copy the last message read pointers */
    if (copydifferentsizes(&Out,&In,p,(long) Header.NumOfConf*sizeof(long),(long) NewHeader.NumOfConf*sizeof(long)) == -1)
      goto exit;

    /* now copy any APPLICATION record information */
    for (X = 0; X < Header.NumOfApps; X++) {
      if (copyapplication(&Out,&In,p,Sizes[X].SizeOfRec,Sizes[X].SizeOfRec,Sizes[X].SizeOfConfRec,Sizes[X].SizeOfConfRec,OldNumConf) == -1)
        goto exit;
    }
  }

  if (NewHeader.NumOfApps != 0)
    updateoffsets(&Out,&In,Header.NumOfApps,PcbRecSize);

  if (Sizes != NULL)
    free(Sizes);
  dosfclose(&Out);
  dosfclose(&In);
  free(p);
  #ifdef JUDNEWELL
    unlink("V:USERS.IBK");
  #else
    unlink(BackInfFileName);
    rename(PcbData.InfFile,BackInfFileName);     //lint !e534
    rename(TempInfFileName,PcbData.InfFile);     //lint !e534
  #endif
  return;

exit:
  if ((In.status & EOFBIT) != 0) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Msg1    = "ERROR! USERS.INF is Incomplete. Pack the USERS File to Fix.";
    MsgData.Line1   = 18;
    MsgData.Color1  = Colors[HEADING];
    showmessage();
  }

  dosfclose(&Out);
  dosfclose(&In);
  free(p);
  #ifdef JUDNEWELL
    unlink("V:USERS.IBK");
  #else
    unlink(TempInfFileName);
  #endif
}


static bool near pascal findpsa(char *Str) {
  psatype *p;

  for (p = PSAs; p < &PSAs[TOTALPSAS]; p++)
    if (strcmp(p->Name,Str) == 0)
      return(TRUE);

  return(FALSE);
}


static void near pascal pause(void) {
  char Ch;

  fastprintmove(3,20,"Press any key to continue...",Colors[HELPKEY]);
  Ch = inkey(&Ch,CLOCK);
}

void pascal listtpas(void) {
  unsigned X;
  int      NumLines;
  DOSFILE  In;
  apptype  App;
  hdrtype  TmpHeader;
  char     Str[80];

  clscolor(Colors[DISPLAY]);
  generalscreen(MainHead1,"List Installed Third Party Applications");
  memset(Str,'�',78);
  Str[78] = 0;
  fastprint(1, 4,Str,Colors[OUTBOX]);
  fastprint(1,21,Str,Colors[OUTBOX]);

  if (dosfopen(PcbData.InfFile,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    return;

  if (dosfread(&TmpHeader,sizeof(hdrtype),&In) != sizeof(hdrtype) || TmpHeader.NumOfApps < 1) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Msg1    = "No TPAs or PSAs Found";
    MsgData.Line1   = 18;
    MsgData.Color1  = Colors[HEADING];
    showmessage();
    dosfclose(&In);
    return;
  }

  gotoxy(3,20);
  setcursor(CUR_NORMAL);

  for (X = 0, NumLines = 0; X < TmpHeader.NumOfApps; X++, NumLines++) {
    if (dosfread(&App,sizeof(apptype),&In) != sizeof(apptype))
      goto exit;
    sprintf(Str,"%s: %-14.14s Ver: %-4u Static: %-4u Dynamic: %-4u %s %s",
            (findpsa(App.Name) ? "PSA" : "TPA"),App.Name,App.Version,App.SizeOfRec,App.SizeOfConfRec,(findpsa(App.Name) ? "" : "Key:"),App.KeyWord);
    fastprint(3,20,Str,Colors[DISPLAY]);
    scrollup(3,5,77,20,Colors[DISPLAY]);
    if (NumLines >= 14) {
      pause();
      clsbox(3,20,77,20,Colors[DISPLAY]);
      NumLines = 0;
    }
  }

exit:
  pause();
  dosfclose(&In);
}


#define NUMTPAFIELDS 4

static char ALLINT[] = {0,'0','9'};

static FldType TPA[NUMTPAFIELDS] = {
  {vUNSIGNED,ALLINT ,0, 3, 9, 5,CLEAR  ,"Version     ","",&AppHeader.Version      ,NULL},
  {vUNSIGNED,ALLINT ,0, 3,10, 5,CLEAR  ,"Static Size ","",&AppHeader.SizeOfRec    ,NULL},
  {vUNSIGNED,ALLINT ,0, 3,11, 5,CLEAR  ,"Dynamic Size","",&AppHeader.SizeOfConfRec,NULL},
  {vUPSTR   ,ALLCHAR,0, 3,12, 8,CLEAR  ,"Keyword     ","", AppHeader.KeyWord      ,NULL},
};

void pascal updatetpa(void) {
  bool      Found;
  bool      YesNo;
  unsigned  X;
  unsigned  NewAppNum;
  char     *p;
  char     *Token;
  sizetype *Sizes;
  long      RecNo;
  long      TotalRecs;
  long      Offset;
  char      Temp[15];
  char      Name[15];
  DOSFILE   In;
  DOSFILE   Out;
  hdrtype   NewHeader;
  apptype   App;
  char      Str[80];

  /* find out if users.inf exists and if the conference allocations are  */
  /* correct - if not, abort out without making changes to the TPA stuff */
  if (openusersinffile(PcbData.InfFile) == -1)
    return;

  dosfclose(&DosUsersInfFile);  /* we just opened it, close it now */

  memset(&In,0,sizeof(DOSFILE));
  memset(&Out,0,sizeof(DOSFILE));

  Sizes = NULL;
  if ((p = (char *) mallochk(INFBUFSIZE)) == NULL)
    return;

  if (dosfopen(PcbData.InfFile,OPEN_READ|OPEN_DENYRDWR,&In) == -1) {
    openerror();
    free(p);
    return;
  }

  if (dosfread(&NewHeader,sizeof(hdrtype),&In) != sizeof(hdrtype)) {
    openerror();
    goto exit;
  }

  if ((UsersFile = dosopencheck(PcbData.UsrFile,OPEN_RDWR|OPEN_DENYRDWR)) == -1) {
    openerror();
    goto exit;
  }

  TotalRecs = numrecs(UsersFile,sizeof(URead));
  dosclose(UsersFile);

  clscolor(Colors[OUTBOX]);

  #ifdef DEMO
    generalscreen(MainHead1,"Add/Update PCBoard Supported Allocations");
  #else
    generalscreen(MainHead1,(UpdatingPSAs ? "Add/Update PCBoard Supported Allocations" : "Add/Update Third Party Application"));
  #endif

  if (BatchMode) {
    if ((Token = parsepaths(NULL)) == NULL)
      goto exit;
    strcpy(Name,Token);
    sprintf(Str,": %s",Name);
    if (UpdatingPSAs) {
      fastprint( 3,5,"PCBoard Supported Allocation (PSA) Name",Colors[QUESTION]);
      fastprint(43,5,Str,Colors[ANSWER]);
    } else {
      fastprint( 3,5,"Enter the name of the Third Party Application",Colors[QUESTION]);
      fastprint(49,5,Str,Colors[ANSWER]);
    }
  } else {
    Name[0] = 0;
    setcursor(CUR_NORMAL);
    inputstr(3,5,14,"Enter the name of the Third Party Application",Name,Name,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,0);
    setcursor(CUR_BLANK);
    stripright(Name,' ');
    if (Name[0] == 0 || KeyFlags == ESC) {
      KeyFlags = ESC;
      goto exit;
    }
  }

  NewAppNum = 0xFFFF;
  Found = FALSE;
  if (NewHeader.NumOfApps != 0) {
    if ((Sizes = (sizetype *) malloc(NewHeader.NumOfApps * sizeof(sizetype))) == NULL)
      goto exit;

    for (X = 0; X < NewHeader.NumOfApps; X++) {
      if (dosfread(&App,sizeof(apptype),&In) != sizeof(apptype))
        goto exit;
      Sizes[X].SizeOfRec     = App.SizeOfRec;
      Sizes[X].SizeOfConfRec = App.SizeOfConfRec;
      if (stricmp(Name,App.Name) == 0) {
        Found = TRUE;
        NewHeader.TotalRecSize -= App.SizeOfRec + ((long) PcbData.NumAreas * App.SizeOfConfRec);
        NewAppNum = X;
        AppHeader = App;
      }
    }
  }

  if (Found) {
    fastprint(3,7,"Application already exists...",Colors[HEADING]);
    if (! UpdatingPSAs)
      fastprint(33,7,"enter any changes below:",Colors[HEADING]);
  } else {
    NewAppNum = NewHeader.NumOfApps;
    NewHeader.NumOfApps++;
    memset(&AppHeader,0,sizeof(apptype));
    strcpy(AppHeader.Name,Name);
    #ifdef DEMO
      fastprint(3,7,"PSA not installed",Colors[DISPLAY]);
    #else
      fastprint(3,7,(UpdatingPSAs ? "PSA not installed" : "Application name not found"),Colors[DISPLAY]);
    #endif
    if (! UpdatingPSAs)
      fastprint(29,7,", enter new information below:",Colors[DISPLAY]);
  }


  if (BatchMode) {
    if ((Token = parsepaths(NULL)) == NULL)
      goto manual;
    AppHeader.Version = atoi(Token);
    if ((Token = parsepaths(NULL)) == NULL)
      goto manual;
    AppHeader.SizeOfRec = atoi(Token);
    if ((Token = parsepaths(NULL)) == NULL)
      goto manual;
    AppHeader.SizeOfConfRec = atoi(Token);
    if ((Token = parsepaths(NULL)) == NULL)
      goto manual;
    maxstrcpy(AppHeader.KeyWord,Token,sizeof(AppHeader.KeyWord));
  }

  // even in batch mode, let's ask the question and wait for the sysop to
  // press the PGDN key to begin ... unless it's a PSA, then just display
  // the values and wait for confirmation to begin

manual:
  if (UpdatingPSAs) {
    sprintf(Str,"Version     : %d",AppHeader.Version);
    fastprint(3, 9,Str,Colors[QUESTION]);
    sprintf(Str,"Static Size : %d",AppHeader.SizeOfRec);
    fastprint(3,10,Str,Colors[QUESTION]);
    sprintf(Str,"Dynamic Size: %d",AppHeader.SizeOfConfRec);
    fastprint(3,11,Str,Colors[QUESTION]);
    setatt(17,9,25,11,Colors[ANSWER]);
    YesNo = FALSE;
    inputnum(3,16,1,"Proceed with Add/Update of PSA",&YesNo,vBOOL,0);
    if (KeyFlags == ESC || ! YesNo) {
      KeyFlags = ESC;
      goto exit;
    }
  } else {
    fastcenter(22,"Press PGDN to begin or ESC to exit",Colors[DISPLAY]);
    initquest(TPA,NUMTPAFIELDS);
    readscrn(TPA,NUMTPAFIELDS-1,0,"","",1,NOCLEARFLD);  //lint !e534
    freeanswers(TPA,NUMTPAFIELDS);
    if (KeyFlags == ESC)
      goto exit;
  }

  stripright(AppHeader.Name,' ');
  stripright(AppHeader.KeyWord,' ');

  NewHeader.TotalRecSize += AppHeader.SizeOfRec + ((long) PcbData.NumAreas * AppHeader.SizeOfConfRec);

  if (dosfopen(TempInfFileName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1)
    goto exit;

  dossetbuf(&Out,8192);  /* try for a bigger buffer */
  dossetbuf(&In ,8192);  /* try for a bigger buffer */

  dosfseek(&In,sizeof(hdrtype),SEEK_SET);
  if (dosfwrite(&NewHeader,sizeof(hdrtype),&Out) == -1)
    goto exit;

  Offset = NewHeader.SizeOfRec + NewHeader.SizeOfConf;
  for (X = 0; X < NewHeader.NumOfApps; X++) {
    if (X == NewAppNum) {
      if (Found) { /* we gotta read it if it was found but we won't use it */
        if (dosfread(&App,sizeof(apptype),&In) != sizeof(apptype))
          goto exit;
      }
      AppHeader.Offset = Offset;
      App = AppHeader;
    } else {
      if (dosfread(&App,sizeof(apptype),&In) != sizeof(apptype))
        goto exit;
      App.Offset = Offset;
    }
    Offset += App.SizeOfRec + ((long) App.SizeOfConfRec * PcbData.NumAreas);
    if (dosfwrite(&App,sizeof(apptype),&Out) == -1)
      goto exit;
  }

  memset(&UsersRec,0,sizeof(rectype));
  clsbox(3,20,77,22,Colors[DISPLAY]);
  sprintf(Str,"There are %s records to be processed in the USERS.INF file...",comma(Temp,TotalRecs));
  fastprint(3,20,Str,Colors[DISPLAY]);
  fastprintmove(3,22,"Processing #     1",Color[DISPLAY]);

  for (RecNo = 1; RecNo <= TotalRecs; RecNo++) {
    /* show it on the screen */
    sprintf(Str,"%6ld",RecNo);
    fastprint(15,22,Str,Color[DISPLAY]);

    /* copy the pcboard record */
    if (copyinfbytes(&Out,&In,p,NewHeader.SizeOfRec+NewHeader.SizeOfConf) == -1)
      goto exit;

    /* now copy the APPLICATION record information */
    for (X = 0; X < NewHeader.NumOfApps; X++) {
      if (X == NewAppNum) {
        if (Found) {
          if (copyapplication(&Out,&In,p,Sizes[X].SizeOfRec,AppHeader.SizeOfRec,Sizes[X].SizeOfConfRec,AppHeader.SizeOfConfRec,PcbData.NumAreas) == -1)
            goto exit;
        } else {
          if (copyapplication(&Out,&In,p,0,AppHeader.SizeOfRec,0,AppHeader.SizeOfConfRec,0) == -1)
            goto exit;
        }
      } else {
        if (copyapplication(&Out,&In,p,Sizes[X].SizeOfRec,Sizes[X].SizeOfRec,Sizes[X].SizeOfConfRec,Sizes[X].SizeOfConfRec,PcbData.NumAreas) == -1)
          goto exit;
      }
    }
  }

  if (Sizes != NULL)
    free(Sizes);
  dosfclose(&Out);
  dosfclose(&In);
  free(p);
  unlink(BackInfFileName);
  rename(PcbData.InfFile,BackInfFileName);  //lint !e534
  rename(TempInfFileName,PcbData.InfFile);  //lint !e534
  return;


exit:
  if (KeyFlags != ESC && (In.status & EOFBIT) != 0) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Msg1    = "ERROR! USERS.INF is Incomplete. Pack the USERS File to Fix.";
    MsgData.Line1   = 18;
    MsgData.Color1  = Colors[HEADING];
    showmessage();
  }

  if (Sizes != NULL)
    free(Sizes);
  dosfclose(&Out);
  dosfclose(&In);
  free(p);
  KeyFlags = NOTHING;
}


void pascal removetpa(void) {
  bool      Found;
  bool      Answer;
  long      RecNo;
  long      TotalRecs;
  long      Offset;
  char     *p;
  unsigned  X;
  unsigned  AppNum;
  sizetype *Sizes;
  char      Temp[15];
  char      Name[15];
  hdrtype   NewHeader;
  apptype   App;
  DOSFILE   In;
  DOSFILE   Out;
  char      Str[80];

  Removed = FALSE;

  /* find out if users.inf exists and if the conference allocations are  */
  /* correct - if not, abort out without making changes to the TPA stuff */
  if (openusersinffile(PcbData.InfFile) == -1)
    return;

  dosfclose(&DosUsersInfFile);  /* we just opened it, close it now */

  memset(&In,0,sizeof(DOSFILE));
  memset(&Out,0,sizeof(DOSFILE));

  Sizes = NULL;
  if ((p = (char *) mallochk(INFBUFSIZE)) == NULL)
    return;

  if (dosfopen(PcbData.InfFile,OPEN_READ|OPEN_DENYRDWR,&In) == -1) {
    openerror();
    free(p);
    return;
  }

  if (dosfread(&NewHeader,sizeof(hdrtype),&In) != sizeof(hdrtype)) {
    openerror();
    goto exit;
  }

  if ((UsersFile = dosopencheck(PcbData.UsrFile,OPEN_RDWR|OPEN_DENYRDWR)) == -1) {
    openerror();
    goto exit;
  }

  TotalRecs = numrecs(UsersFile,sizeof(URead));
  dosclose(UsersFile);

  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,(UpdatingPSAs ? "Remove PCBoard Supported Allocation" : "Remove Third Party Application"));

  setcursor(CUR_NORMAL);

  if (UpdatingPSAs) {
    fastprint( 3,5,"Removing PCBoard Supported Allocation (PSA):",Colors[QUESTION]);
    fastprint(48,5,PSAName,Colors[ANSWER]);
    strcpy(Name,PSAName);
  } else {
    Name[0] = 0;
    inputstr(3,5,14,"Enter the name of the Third Party Application",Name,Name,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,0);
  }

  setcursor(CUR_BLANK);
  stripright(Name,' ');
  if (Name[0] == 0 || KeyFlags == ESC)
    goto exit;

  AppNum = 0xFFFF;
  Found = FALSE;
  if (NewHeader.NumOfApps != 0) {
    if ((Sizes = (sizetype *) malloc(NewHeader.NumOfApps * sizeof(sizetype))) == NULL)
      goto exit;

    for (X = 0; X < NewHeader.NumOfApps; X++) {
      if (dosfread(&App,sizeof(apptype),&In) != sizeof(apptype))
        goto exit;
      Sizes[X].SizeOfRec     = App.SizeOfRec;
      Sizes[X].SizeOfConfRec = App.SizeOfConfRec;
      if (stricmp(Name,App.Name) == 0) {
        Found = TRUE;
        NewHeader.TotalRecSize -= App.SizeOfRec + ((long) PcbData.NumAreas * App.SizeOfConfRec);
        AppNum = X;
      }
    }
  }

  if (! Found) {
    fastprint(3,7,(UpdatingPSAs ? "PSA does not exist!" : "Application does not exist!"),Colors[HEADING]);
    mydelay(300);
    goto exit;
  }

  for (X = 0; X < TOTALPSAS; X++) {
    if (stricmp(Name,PSAs[X].Name) == 0) {
      *PSAs[X].Enabled = FALSE;
      break;
    }
  }

  setcursor(CUR_NORMAL);
  Answer = FALSE;
  inputnum(3,7,1,"Do you want to continue",&Answer,vBOOL,0);
  setcursor(CUR_BLANK);
  if (! Answer || KeyFlags == ESC)
    goto exit;

  if (dosfopen(TempInfFileName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1)
    goto exit;

  dosfseek(&In,sizeof(hdrtype),SEEK_SET);
  dossetbuf(&Out,16384);  /* try for a bigger buffer */
  NewHeader.NumOfApps--;

  if (dosfwrite(&NewHeader,sizeof(hdrtype),&Out) == -1)
    goto exit;

  Offset = NewHeader.SizeOfRec + NewHeader.SizeOfConf;
  for (X = 0; X < NewHeader.NumOfApps+1; X++) {
    if (dosfread(&App,sizeof(apptype),&In) != sizeof(apptype))
      goto exit;
    if (X != AppNum) {
      App.Offset = Offset;
      Offset += App.SizeOfRec + ((long) App.SizeOfConfRec * PcbData.NumAreas);
      if (dosfwrite(&App,sizeof(apptype),&Out) == -1)
        goto exit;
    }
  }

  memset(&UsersRec,0,sizeof(rectype));
  clsbox(3,20,77,22,Colors[DISPLAY]);
  sprintf(Str,"There are %s records to be processed in the USERS.INF file...",comma(Temp,TotalRecs));
  fastprint(3,20,Str,Colors[DISPLAY]);
  fastprintmove(3,22,"Processing #     1",Color[DISPLAY]);

  for (RecNo = 1; RecNo <= TotalRecs; RecNo++) {
    /* show it on the screen */
    sprintf(Str,"%6ld",RecNo);
    fastprint(15,22,Str,Color[DISPLAY]);

    /* copy the pcboard record */
    if (copyinfbytes(&Out,&In,p,NewHeader.SizeOfRec+NewHeader.SizeOfConf) == -1)
      goto exit;

    /* now copy the APPLICATION record information */
    for (X = 0; X < NewHeader.NumOfApps+1; X++) {
      if (X == AppNum) {
        if (copyapplication(&Out,&In,p,Sizes[X].SizeOfRec,0,Sizes[X].SizeOfConfRec,0,PcbData.NumAreas) == -1)
          goto exit;
      } else {
        if (copyapplication(&Out,&In,p,Sizes[X].SizeOfRec,Sizes[X].SizeOfRec,Sizes[X].SizeOfConfRec,Sizes[X].SizeOfConfRec,PcbData.NumAreas) == -1)
          goto exit;
      }
    }
  }

  if (Sizes != NULL)
    free(Sizes);
  dosfclose(&Out);
  dosfclose(&In);
  free(p);
  unlink(BackInfFileName);
  rename(PcbData.InfFile,BackInfFileName);  //lint !e534
  rename(TempInfFileName,PcbData.InfFile);  //lint !e534
  Removed = TRUE;
  return;


exit:
  if (Sizes != NULL)
    free(Sizes);
  dosfclose(&Out);
  dosfclose(&In);
  free(p);
  KeyFlags = NOTHING;
}


void pascal addpcboardtpa(void) {
  Removed    = FALSE;
  AddingPSAs = TRUE;
  menusel(8,PSAMENU,SMALL,0,TRUE);
  AddingPSAs = FALSE;
  setpsaorder();
}


void pascal removepcboardtpa(void) {
  Removed    = FALSE;
  AddingPSAs = FALSE;
  menusel(8,PSAMENU,SMALL,0,TRUE);
  setpsaorder();
}


static void near pascal updatepsa(psatype *p) {
  char CommandLine[40];

  UpdatingPSAs = TRUE;

  if (AddingPSAs) {
    sprintf(CommandLine,"/ADDTPA;%s;%d;%d;%d;",p->Name,p->Version,p->RecSize,p->ConfSize);
    parsepaths(CommandLine);  //lint !e534
    BatchMode = TRUE;
    updatetpa();
    BatchMode = FALSE;
  } else {
    strcpy(PSAName,p->Name);
    removetpa();
  }

  KeyFlags = ESC;
  UpdatingPSAs = FALSE;
  setpsaorder();
}

void pascal aliastpa(void) {
  updatepsa(&PSAs[ALIAS]);
  if (Removed)
    createallindexes();
}

void pascal addresstpa(void) {
  updatepsa(&PSAs[ADDRESS]);
}

void pascal passwordtpa(void) {
  updatepsa(&PSAs[PASSWORD]);
}

void pascal verifytpa(void) {
  updatepsa(&PSAs[VERIFY]);
}

void pascal statstpa(void) {
  updatepsa(&PSAs[STATS]);
}

void pascal notestpa(void) {
  updatepsa(&PSAs[NOTES]);
}

void pascal accounttpa(void) {
  updatepsa(&PSAs[ACCOUNT]);
}

void pascal qwknettpa(void) {
  updatepsa(&PSAs[QWKNET]);
}

/* 15.4: install/remove the Personal PSA (Gender/Birthdate/Email/Web) */
void pascal personaltpa(void) {
  updatepsa(&PSAs[PERSONAL]);
}

void pascal banktpa(void) {
  updatepsa(&PSAs[BANK]);
}
#endif
