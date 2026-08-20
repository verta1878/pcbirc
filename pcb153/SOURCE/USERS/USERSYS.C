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

#ifdef __WATCOMC__
  #include <io.h>
#endif

#include <stddef.h>

#define RELEASENUM 1530        /* v15.2 syshdr release */

#pragma pack(1)
typedef struct {
  unsigned short Version;            /* PCBoard version number (i.e. 145)            */
  long           RecNo;              /* Record number from USER's file               */
  unsigned short SizeOfRec;          /* Size of "fixed" user record                  */
  unsigned short NumOfAreas;         /* Number of conference areas (Main=1 - 65535)  */
  unsigned short NumOfBitFields;     /* Number of Bit Map fields for conferences     */
  unsigned short SizeOfBitFields;    /* Size of each Bit Map field                   */
  char           AppName[15];        /* Name of the Third Party Application (if any) */
  unsigned short AppVersion;         /* Version number for the application (if any)  */
  unsigned short AppSizeOfRec;       /* Size of a "fixed length" record (if any)     */
  unsigned short AppSizeOfConfRec;   /* Size of each conference record (if any)      */
  long           AppRecOffset;       /* Offset of AppRec into USERS.INF record       */
  bool           Updated;            /* TRUE if users.sys was updated                */
} syshdrtype;
#pragma pack()

#define BUFSIZE 1024

static int _NEAR_ LIBENTRY copyuserinfbytes(DOSFILE *Out, long NumBytes) {
  char Buf[BUFSIZE];

  while (NumBytes > BUFSIZE) {
    if (dosfread(Buf,BUFSIZE,&DosUsersInfFile) != BUFSIZE || dosfwrite(Buf,BUFSIZE,Out) == -1)
      return(-1);
    NumBytes -= BUFSIZE;
  }

  // the last copy is less than or equal to BUFSIZE bytes
  if (dosfread(Buf,(int) NumBytes,&DosUsersInfFile) != (int) NumBytes || dosfwrite(Buf,(int) NumBytes,Out) == -1)
    return(-1);

  return(0);
}


static int _NEAR_ LIBENTRY copybackinfbytes(DOSFILE *In, long NumBytes) {
  char Buf[BUFSIZE];

  while (NumBytes > BUFSIZE) {
    if (dosfread(Buf,BUFSIZE,In) != BUFSIZE || dosfwrite(Buf,BUFSIZE,&DosUsersInfFile) == -1)
      return(-1);
    NumBytes -= BUFSIZE;
  }

  // the last copy is less than or equal to BUFSIZE bytes
  if (dosfread(Buf,(int) NumBytes,In) != (int) NumBytes || dosfwrite(Buf,(int) NumBytes,&DosUsersInfFile) == -1)
    return(-1);

  return(0);
}


int LIBENTRY gettpaheader(char *KeyWord, apptype *AppHdr) {
  unsigned X;

  if (KeyWord[0] != 0) {
    validateinfoffset(sizeof(hdrtype),"get tpa header"); // validate & seek to offset
    for (X = 0; X < Hdr.NumOfApps; X++) {
      if (dosfread(AppHdr,sizeof(apptype),&DosUsersInfFile) != sizeof(apptype))
        return(-1);
      if (strcmp(AppHdr->KeyWord,KeyWord) == 0)
        return(0);
    }
  }
  return(-1);
}


int LIBENTRY gettparec(apptype *AppHdr, long RecNum, void *Rec) {
  long Offset;

  if (AppHdr->Name[0] != 0 && AppHdr->SizeOfRec != 0 && Rec != NULL) {
    Offset = ((RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize + AppHdr->Offset;
    validateinfoffset(Offset,"get tpa rec"); // validate & seek to offset
    if (dosfread(Rec,AppHdr->SizeOfRec,&DosUsersInfFile) != AppHdr->SizeOfRec)
      return(-1);
    return(0);
  }
  return(-1);
}


int LIBENTRY gettpaconfrec(apptype *AppHdr, long RecNum, unsigned ConfNum, void *Rec) {
  long Offset;

  if (AppHdr->Name[0] != 0 && AppHdr->SizeOfConfRec != 0 && Rec != NULL) {
    Offset = ((RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize + AppHdr->Offset + AppHdr->SizeOfRec + ((long) ConfNum*AppHdr->SizeOfConfRec);
    validateinfoffset(Offset,"get tpa conf rec"); // validate & seek to offset
    if (dosfread(Rec,AppHdr->SizeOfConfRec,&DosUsersInfFile) != AppHdr->SizeOfConfRec)
      return(-1);
    return(0);
  }
  return(-1);
}


int LIBENTRY puttparec(apptype *AppHdr, long RecNum, void *Rec) {
  long Offset;

  if (AppHdr->Name[0] != 0 && AppHdr->SizeOfRec != 0 && Rec != NULL) {
    Offset = ((RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize + AppHdr->Offset;
    validateinfoffset(Offset,"put tpa rec"); // validate & seek to offset
    if (dosfwrite(Rec,AppHdr->SizeOfRec,&DosUsersInfFile) != -1 && dosflush(&DosUsersInfFile) != -1)
      return(0);
  }
  return(-1);
}


int LIBENTRY puttpaconfrec(apptype *AppHdr, long RecNum, unsigned ConfNum, void *Rec) {
  long Offset;

  if (AppHdr->Name[0] != 0 && AppHdr->SizeOfConfRec != 0 && Rec != NULL) {
    Offset = ((RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize + AppHdr->Offset + AppHdr->SizeOfRec + ((long) ConfNum*AppHdr->SizeOfConfRec);
    validateinfoffset(Offset,"put tpa conf rec"); // validate & seek to offset
    return(dosfwrite(Rec,AppHdr->SizeOfConfRec,&DosUsersInfFile));
  }
  return(-1);
}


void LIBENTRY makeusersys(void) {
  bool        Completed;
  unsigned    X;
  char       *p;
  long        Offset;
  DOSFILE     Out;
  apptype     AppHdr;
  syshdrtype  SysHdr;

  Completed = FALSE;
  memset(&SysHdr,0,sizeof(syshdrtype));

  if (gettpaheader(Status.DoorName,&AppHdr) != -1) {
    strcpy(SysHdr.AppName,AppHdr.Name);
    SysHdr.AppVersion       = AppHdr.Version;
    SysHdr.AppSizeOfRec     = AppHdr.SizeOfRec;
    SysHdr.AppSizeOfConfRec = AppHdr.SizeOfConfRec;
    SysHdr.AppRecOffset     = AppHdr.Offset;
  }

  if ((p = (char *) checkmalloc(ConfByteLen,"USER SYS OUT")) == NULL)
    return;

  if (dosfopen("USERS.SYS",OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1)
    goto exit2;

  UsersData.AliasSupport    = AliasSupport;
  UsersData.AddressSupport  = AddressSupport;
  UsersData.PasswordSupport = PasswordSupport;
  UsersData.VerifySupport   = VerifySupport;
  UsersData.StatsSupport    = StatsSupport;
  UsersData.NotesSupport    = NotesSupport;
  UsersData.AccountSupport  = AccountSupport;
  UsersData.QwkSupport      = QwkSupport;

  SysHdr.Version         = RELEASENUM;
  SysHdr.RecNo           = Status.UserRecNo;
  SysHdr.SizeOfRec       = sizeof(UData);
  SysHdr.NumOfAreas      = PcbData.NumAreas;
  SysHdr.SizeOfBitFields = (short) ConfByteLen;

  // for compatibility with v14.5a doors, if Status.MakeUserSys is set to 2
  // then it means that the door can only handle v14.5a-sized users.sys
  // files so reduce the size of the record (in the header) and use that
  // information to write the correct amount of data from UsersData out to disk

  if (Status.MakeUserSys == 2) {
    SysHdr.Version   = 1450;  // version 14.5 style users.sys
    #ifdef offsetof
      SysHdr.SizeOfRec = (int) offsetof(UData,AliasSupport);
    #else
      SysHdr.SizeOfRec = (int) (&UsersData.AliasSupport - UsersData.Name);
    #endif
    SysHdr.NumOfBitFields  = 7;   /* REG, EXP, USR, CON, MFL, JOINED, SCANNED */
  } else if (Status.MakeUserSys == 3) {
    SysHdr.Version   = 1500;  // version 15.0 style users.sys
    #ifdef offsetof
      SysHdr.SizeOfRec = (int) offsetof(UData,AccountSupport);
    #else
      SysHdr.SizeOfRec = (int) (&UsersData.AccountSupport - UsersData.Name);
    #endif
    SysHdr.NumOfBitFields  = 7;   /* REG, EXP, USR, CON, MFL, JOINED, SCANNED */
  } else if (Status.MakeUserSys == 4) {
    SysHdr.Version = 1520;  // version 15.2 style users.sys
    #ifdef offsetof
      SysHdr.SizeOfRec = (int) offsetof(UData,TotDnldBytes);
    #else
      SysHdr.SizeOfRec = (int) (&UsersData.TotDnldBytes - UsersData.Name);
    #endif
    // include the NET STATUS flags *only* if necessary
    SysHdr.NumOfBitFields = (short) (QwkSupport ? 8 : 7);   /* REG, EXP, USR, CON, MFL, JOINED, SCANNED, NET */
  } else {
    // include the NET STATUS flags *only* if necessary
    SysHdr.NumOfBitFields = (short) (QwkSupport ? 8 : 7);   /* REG, EXP, USR, CON, MFL, JOINED, SCANNED, NET */
  }

  if (dosfwrite(&SysHdr,sizeof(syshdrtype),&Out) == -1)
    goto exit;

  if (dosfwrite(&UsersData,SysHdr.SizeOfRec,&Out) == -1)
    goto exit;

  /* write extended LAST MSG READ pointers */
  for (X = 0; X < PcbData.NumAreas; X++) {
    fmemcpy(p,&MsgReadPtr[X],sizeof(long));
    if (dosfwrite(p,sizeof(long),&Out) == -1)
      goto exit;
  }

  /* write extended REGISTERED flags */
  fmemcpy(p,&ConfReg[REG],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&Out) == -1)
    goto exit;

  /* write extended EXPIRED flags */
  fmemcpy(p,&ConfReg[EXP],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&Out) == -1)
    goto exit;

  /* write extended USER SCAN flags */
  fmemcpy(p,&ConfReg[USR],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&Out) == -1)
    goto exit;

  /* write the CONFERENCE SYSOP flags */
  fmemcpy(p,&ConfReg[CON],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&Out) == -1)
    goto exit;

  /* write the CONFERENCE MAIL flags */
  fmemcpy(p,&ConfReg[MFL],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&Out) == -1)
    goto exit;

  /* write the CONFERENCE JOINED flags */
  fmemcpy(p,&ConfFlags[JOINED],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&Out) == -1)
    goto exit;

  /* write the CONFERENCE SCANNED flags */
  fmemcpy(p,&ConfFlags[SCANNED],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&Out) == -1)
    goto exit;

  if (SysHdr.NumOfBitFields > 7) {
    /* write the NET STATUS flags */
    fmemcpy(p,&ConfReg[NET],ConfByteLen);
    if (dosfwrite(p,ConfByteLen,&Out) == -1)
      goto exit;
  }

  /* now get the APPLICATION information */
  if (SysHdr.AppName[0] != 0) {
    Offset = ((UsersData.RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize + SysHdr.AppRecOffset;
    validateinfoffset(Offset,"put app info"); // validate & seek to offset
    if (SysHdr.AppSizeOfRec != 0)
      if (copyuserinfbytes(&Out,SysHdr.AppSizeOfRec) == -1)
        goto exit;

    /* now get the APPLICATION CONFERENCE information */
    if (SysHdr.AppSizeOfConfRec != 0)
      if (copyuserinfbytes(&Out,(long) SysHdr.AppSizeOfConfRec * PcbData.NumAreas) == -1)
        goto exit;
  }



  Completed = TRUE;

exit:
  dosfclose(&Out);
exit2:
  bfree(p);

  if (! Completed)
    unlink("USERS.SYS");
}


void LIBENTRY readusersys(bool UserLoggedOff) {
  #ifdef PCBSTATS
    bool       UpdateStats;
    unsigned   NumUploads;
    unsigned   NumDownloads;
    unsigned   OldUploads;
    unsigned   OldDownloads;
    unsigned long NumMsgsLeft;
    unsigned long OldMsgsLeft;
  #endif
  unsigned long OldulTotDnldBytes;
  unsigned long OldulTotUpldBytes;
  double     OldTotDnldBytes;
  double     OldTotUpldBytes;
  unsigned   X;
  char       *p;
  long       Num;
  long       Offset;
  DOSFILE    In;
  char       SaveName[26];
  syshdrtype SysHdr;
  char       Str[80];

  memset(&SysHdr,0,sizeof(SysHdr));

  #ifdef PCBSTATS
    UpdateStats = FALSE;
    NumMsgsLeft = NumUploads = NumDownloads = 0;
  #endif

  if ((p = (char *) checkmalloc(ConfByteLen,"USER SYS IN")) == NULL)
    goto exit2;

  if (dosfopen("USERS.SYS",OPEN_READ|OPEN_DENYNONE,&In) == -1)
    goto exit2;

  if (dosfread(&SysHdr,sizeof(syshdrtype),&In) != sizeof(syshdrtype))
    goto exit;

  // because the user COULD drop out to dos with one version of the
  // USERS.SYS and reload with another - let's check the version number
  // and get out of here if it's different from what we expect to find
  //
  // NOTE:  we're going to allow both 14.5a and 15.0 style users.sys files
  // the former is smaller as it does not have the PSA data in it, the
  // 15.0 style does.
  //
  // Armed with that information we'll also verify that the sizes of the
  // records are correct before reading them in.

  if (SysHdr.Version == RELEASENUM) {
    if (SysHdr.SizeOfRec != sizeof(UData))
      goto exit;
  } else if (SysHdr.Version == 1450) {
    #ifdef offsetof
      if (SysHdr.SizeOfRec != (int) offsetof(UData,AliasSupport))
        goto exit;
    #else
      if (SysHdr.SizeOfRec != (int) (&UsersData.AliasSupport - UsersData.Name))
        goto exit;
    #endif
  } else if (SysHdr.Version == 1500) {
    #ifdef offsetof
      if (SysHdr.SizeOfRec != (int) offsetof(UData,AccountSupport))
        goto exit;
    #else
      if (SysHdr.SizeOfRec != (int) (&UsersData.AccountSupport - UsersData.Name))
        goto exit;
    #endif
  } else if (SysHdr.Version == 1520) {
    #ifdef offsetof
      if (SysHdr.SizeOfRec != (int) offsetof(UData,TotDnldBytes))
        goto exit;
    #else
      if (SysHdr.SizeOfRec != (int) (&UsersData.TotDnldBytes - UsersData.Name))
        goto exit;
    #endif
  } else
    goto exit;

  /* check SPECIFICALLY for the number 1 in the Sys.Hdr.Updated field and   */
  /* ignore it if it's anything but the number 1 - this is in case the file */
  /* is improperly formatted or something...                                */

  if (SysHdr.Updated != 1) {
    /* get name/city in case of logoff */
    if (dosfread(&UsersData,SysHdr.SizeOfRec,&In) != SysHdr.SizeOfRec || UsersData.Name[0] == 0) {
      UserLoggedOff = FALSE;
      Status.UserRecNo = SysHdr.RecNo = 0;
    }
    goto exit;
  }

  /* check to see that either a user is NOT logged on - in which case the   */
  /* Status.UserRecNo value is 0 - or if a user IS logged on that the value */
  /* for SysHdr.RecNo matches - if not, set it to 0 and get out quick       */

  if (Status.UserRecNo == 0)
    Status.UserRecNo = SysHdr.RecNo;
  else if (Status.UserRecNo != SysHdr.RecNo) {
    sprintf(Str,"USERS ptr changed:  PCB.SYS=%ld, USERS.SYS=%ld",Status.UserRecNo,SysHdr.RecNo);
    writelog(Str,SPACERIGHT);
    SysHdr.RecNo = 0;
    UserLoggedOff = FALSE;
    #ifdef PCBSTATS
      UpdateStats = FALSE;
    #endif
    goto exit;
  }


  Status.UserRecNo = SysHdr.RecNo;
/*if (UsersRead.Name[0] == 0)*/
    getuserrecord(FALSE,FALSE);

  #ifdef PCBSTATS
    OldUploads        = UsersRead.NumUploads;
    OldDownloads      = UsersRead.NumDownloads;
    OldMsgsLeft       = UsersData.MsgsLeft;
	OldulTotDnldBytes = UsersData.ulTotDnldBytes;
	OldulTotUpldBytes = UsersData.ulTotUpldBytes;
	OldTotDnldBytes   = UsersData.TotDnldBytes;
	OldTotUpldBytes   = UsersData.TotUpldBytes;
  #endif

  /* save the OLD users.inf pointer, read the users.sys file, then compare */
  /* the two values - if they are different write an error into the log    */

  Num = UsersData.RecNum;
  strcpy(SaveName,UsersData.Name);
  if (dosfread(&UsersData,SysHdr.SizeOfRec,&In) != SysHdr.SizeOfRec) {
    UserLoggedOff = FALSE;
    Status.UserRecNo = SysHdr.RecNo = 0;
    goto exit;
  }

  if (Num != UsersData.RecNum || strcmp(SaveName,UsersData.Name) != 0 || UsersData.Name[0] == 0) {
    dosfclose(&In);
    if (Num != UsersData.RecNum) {
      sprintf(Str,"USERS.INF ptr changed:  SYS=%ld, USER=%ld",UsersData.RecNum,Num);
      writelog(Str,SPACERIGHT);
    }
    if (strcmp(SaveName,UsersData.Name) != 0 || UsersData.Name[0] == 0) {
      sprintf(Str,"USERS.SYS name mismatch: %s",UsersData.Name);
      writelog(Str,SPACERIGHT);
    }
    UserLoggedOff = FALSE;
    #ifdef PCBSTATS
      UpdateStats = FALSE;
    #endif
    Status.UserRecNo = SysHdr.RecNo = 0;
    goto exit2;
  }


  // For backward compatibility, the values that were read from disk were
  // stored into "Old" variables up above *before* the users.sys file was
  // read in from disk.  The logic below then tests to see if the NEW STYLE
  // values (stored in a double) have changed.  If they have NOT changed, but
  // the OLD style values *have* changed, then we assumed that the application
  // has updated the old style values and we overwrite the new style values.

  if (OldTotDnldBytes == UsersData.TotDnldBytes) {
    if (OldulTotDnldBytes != UsersData.ulTotDnldBytes)
      UsersData.TotDnldBytes = UsersData.ulTotDnldBytes;
  }

  if (OldTotUpldBytes == UsersData.TotUpldBytes) {
    if (OldulTotUpldBytes != UsersData.ulTotUpldBytes)
      UsersData.TotUpldBytes = UsersData.ulTotUpldBytes;
  }


  /* read extended LAST MSG READ pointers */
  for (X = 0; X < PcbData.NumAreas; X++) {
    if (dosfread(&Num,sizeof(long),&In) != sizeof(long))
      goto exit;
    MsgReadPtr[X] = (Num < 0 ? 0 : Num);
  }

  /* read extended REGISTERED flags */
  if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
    goto exit;
  fmemcpy(&ConfReg[REG],p,ConfByteLen);

  /* read extended EXPIRED flags */
  if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
    goto exit;
  fmemcpy(&ConfReg[EXP],p,ConfByteLen);

  /* read extended USER SCAN flags */
  if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
    goto exit;
  fmemcpy(&ConfReg[USR],p,ConfByteLen);

  /* read the CONFERENCE SYSOP flags */
  if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
    goto exit;
  fmemcpy(&ConfReg[CON],p,ConfByteLen);

  /* read the CONFERENCE MAIL flags */
  if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
    goto exit;
  fmemcpy(&ConfReg[MFL],p,ConfByteLen);

  /* read the CONFERENCE JOINED flags */
  if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
    goto exit;
  fmemcpy(&ConfFlags[JOINED],p,ConfByteLen);

  /* read the CONFERENCE SCANNED flags */
  if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
    goto exit;
  fmemcpy(&ConfFlags[SCANNED],p,ConfByteLen);

  if (QwkSupport && SysHdr.NumOfBitFields > 7) {
    /* read extended REGISTERED flags */
    if (dosfread(p,ConfByteLen,&In) != ConfByteLen)
      goto exit;
    fmemcpy(&ConfReg[NET],p,ConfByteLen);
  }

  /* now get the APPLICATION information */
  if (SysHdr.AppName[0] != 0 && (SysHdr.AppSizeOfRec != 0 || SysHdr.AppSizeOfConfRec != 0)) {
    Offset = ((UsersData.RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize + SysHdr.AppRecOffset;
    validateinfoffset(Offset,"get app info"); // validate & seek to offset
    if (SysHdr.AppSizeOfRec != 0)
      if (copybackinfbytes(&In,SysHdr.AppSizeOfRec) == -1)
        goto exit;

    /* now get the APPLICATION CONFERENCE information */
    if (SysHdr.AppSizeOfConfRec != 0)
      if (copybackinfbytes(&In,(long) SysHdr.AppSizeOfConfRec * PcbData.NumAreas) == -1)
        goto exit;
  }

  /* check for an error in writing to disk */
  if (dosflush(&DosUsersInfFile) == -1)
    goto exit;

  #ifdef PCBSTATS
    if (UsersData.MsgsLeft > OldMsgsLeft) {
      NumMsgsLeft = UsersData.MsgsLeft - OldMsgsLeft;
      UpdateStats = TRUE;
    }

    if (UsersData.NumUploads > OldUploads) {
      NumUploads = UsersData.NumUploads - OldUploads;
      UpdateStats = TRUE;
    }

    if (UsersData.NumDownloads > OldDownloads) {
      NumDownloads = UsersData.NumDownloads - OldDownloads;
      UpdateStats = TRUE;
    }
  #endif

  convertdatatoread(&UsersData,&UsersRead);
  if (Status.UserRecNo != 0) {
    putuserrecord(IGNOREDIRTY);
    /* we encrypted the data in order to write the record out */
    /* so now decrypt it again so that we can continue online */
    decryptusersrec(&UsersRead);
  }

exit:
  dosfclose(&In);
exit2:
  if (p != NULL)
    bfree(p);
  unlink("USERS.SYS");

  if (UserLoggedOff && SysHdr.RecNo != 0) {
    Status.UserRecNo = SysHdr.RecNo;  /* we need the record # to see if it's the sysop */
    getdisplaynames();
    if (SysHdr.Updated != 1) /* if the header didn't say the record was updated */
      Status.UserRecNo = 0;  /* then get rid of the record number so that we don't update it */
  }

  #ifdef PCBSTATS
    if (UpdateStats || UserLoggedOff)
      updatestats((int) NumMsgsLeft,NumUploads,NumDownloads,(UserLoggedOff ? LOGOFF : UPDATE));
  #endif
}
