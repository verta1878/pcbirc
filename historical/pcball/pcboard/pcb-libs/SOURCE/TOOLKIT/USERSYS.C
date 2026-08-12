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
#include <stdlib.h>
#include <string.h>
#include <misc.h>
#include "pcbtools.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif

#ifdef __cplusplus
  #define max(a,b)    (((a) > (b)) ? (a) : (b))
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define movestr(dest,srce,size) memcpy(dest,srce,size); dest[size] = 0;

#define CNF_USR      0
#define CNF_REG      ConfByteLen
#define CNF_EXP      ConfByteLen * 2
#define CNF_CON      ConfByteLen * 3
#define CNF_MFL      ConfByteLen * 4
#define CNF_NET      ConfByteLen * 5
#define CNF_JOINED   0
#define CNF_SCANNED  ConfByteLen

void       *TPAstatic        = NULL;
void       *TPAdynamic       = NULL;
long _FAR_ *MsgReadPtr       = NULL;
char _FAR_ *ConfReg          = NULL;
char _FAR_ *ConfFlags        = NULL;
char        SysName[66];


static void _NEAR_ LIBENTRY setcurseclevel(int BaseLevel) {
  Status.CurSecLevel = (char) (BaseLevel + Status.ConfAddSec);
}


static void _NEAR_ LIBENTRY getfirstname(char *FirstName, char *FullName) {
  int  Len;
  char *p;

  if ((p = strchr(FullName,' ')) != NULL) {
    Len = (int) (p-FullName);
    movestr(FirstName,FullName,Len);
  } else
    strcpy(FirstName,FullName);

  proper(FirstName);
}

int LIBENTRY readusersysfile(openstatus OpenStatus) {
  DOSFILE  File;
  unsigned Today;
  long     LongInt;
  long _FAR_ *p;
  char     *tmp;
  unsigned X;

  strcpy(SysName,PcbDir);
  strcat(SysName,"USERS.SYS");

  if (fileexist(SysName) == 255 || dosfopen(SysName,OPEN_RDWR|OPEN_DENYNONE,&File) == -1) {
    println("Please leave a (C)omment to the sysop that the door has problems with the");
    println("USERS.SYS file.");
    return(-1);
  } else {
    if (dosfread(&UserSysHdr,sizeof(UserSysHdr),&File) == -1)
      goto error;
    memset(&UserSys,0,sizeof(userrectype));
    if (dosfread(&UserSys,min(sizeof(UserSys),UserSysHdr.SizeOfRec),&File) == -1)
      goto error;
    if (sizeof(UserSys) != UserSysHdr.SizeOfRec)
      dosfseek(&File,sizeof(UserSysHdr)+UserSysHdr.SizeOfRec,SEEK_SET);
  }

  if (UserSysHdr.Version < 1450) {
    println("Wrong version users.sys");
    dosfclose(&File);
    return(-1);
  }

  if (OpenStatus & LMRS) {
    if ((MsgReadPtr = (long _FAR_ *) fbmalloc(UserSysHdr.NumOfAreas * sizeof(long))) == NULL) {
      println("Unable to allocate memory for LMRs");
      dosfclose(&File);
      return(-1);
    }
    for (X = 0, p = MsgReadPtr; X < UserSysHdr.NumOfAreas; X++, p++) {
      if (dosfread(&LongInt,sizeof(long),&File) == -1)
        goto error;
      *p = LongInt;
    }
  } else {
    MsgReadPtr = NULL;
    dosfseek(&File,UserSysHdr.NumOfAreas * sizeof(long),SEEK_CUR);
  }

  ConfByteLen = (UserSysHdr.NumOfAreas >> 3) + ((UserSysHdr.NumOfAreas & 0x07) != 0 ? 1 : 0);
  if (ConfByteLen < 5)
    ConfByteLen = 5;

  if (OpenStatus & CONFFLAGS) {
    if ((ConfReg = (char _FAR_ *) fbmalloc(ConfByteLen * 6)) == NULL || (ConfFlags = (char _FAR_ *) fbmalloc(ConfByteLen * 2)) == NULL || (tmp = (char *) bmalloc(ConfByteLen)) == NULL) {
      println("Unable to allocate memory for flags");
      dosfclose(&File);
      return(-1);
    }

    /* read extended REGISTERED flags */
    if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
      goto error;
    fmemcpy(&ConfReg[CNF_REG],tmp,ConfByteLen);

    /* read extended EXPIRED flags */
    if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
      goto error;
    fmemcpy(&ConfReg[CNF_EXP],tmp,ConfByteLen);

    /* read extended USER SCAN flags */
    if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
      goto error;
    fmemcpy(&ConfReg[CNF_USR],tmp,ConfByteLen);

    /* read the CONFERENCE SYSOP flags */
    if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
      goto error;
    fmemcpy(&ConfReg[CNF_CON],tmp,ConfByteLen);

    /* read the CONFERENCE MAIL flags */
    if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
      goto error;
    fmemcpy(&ConfReg[CNF_MFL],tmp,ConfByteLen);

    /* read the CONFERENCE JOINED flags */
    if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
      goto error;
    fmemcpy(&ConfFlags[CNF_JOINED],tmp,ConfByteLen);

    /* read the CONFERENCE SCANNED flags */
    if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
      goto error;
    fmemcpy(&ConfFlags[CNF_SCANNED],tmp,ConfByteLen);

    /* read the NET STATUS flags */
    if (UserSysHdr.NumOfBitFields > 7) {
      if (dosfread(tmp,ConfByteLen,&File) != ConfByteLen)
        goto error;
      fmemcpy(&ConfReg[CNF_NET],tmp,ConfByteLen);
    }

    bfree(tmp);
  } else {
    ConfReg = ConfFlags = NULL;
    dosfseek(&File,ConfByteLen * UserSysHdr.NumOfBitFields,SEEK_CUR);
  }

  if (OpenStatus & TPA) {
    if (UserSysHdr.AppName[0] == 0 || (UserSysHdr.AppSizeOfRec == 0 && UserSysHdr.AppSizeOfConfRec == 0)) {
      println("TPA not present");
      dosfclose(&File);
      return(-1);
    }

    if (UserSysHdr.AppSizeOfRec != 0) {
      if ((TPAstatic = bmalloc(UserSysHdr.AppSizeOfRec)) == NULL) {
        println("Unable to allocate memory for TPA");
        dosfclose(&File);
        return(-1);
      }
      if (dosfread(TPAstatic,UserSysHdr.AppSizeOfRec,&File) == -1)
        goto error;
    }

    if (UserSysHdr.AppSizeOfConfRec != 0) {
      if ((TPAdynamic = bmalloc(UserSysHdr.AppSizeOfConfRec * PcbData.NumAreas)) == NULL) {
        println("Unable to allocate memory for TPA");
        dosfclose(&File);
        return(-1);
      }
      if (dosfread(TPAdynamic,UserSysHdr.AppSizeOfConfRec * PcbData.NumAreas,&File) == -1)
        goto error;
    }
  } else {
    TPAstatic = TPAdynamic = NULL;
  }


  dosfclose(&File);
  strcpy(Status.LastDateOnStr,juliantodate(UserSys.LastDateOn));

  Today = getjuliandate();
  if (PcbData.SubscriptMode && UserSys.RegExpDate != 0 && Today > UserSys.RegExpDate) { /* expired? */
    setcurseclevel(UserSys.ExpSecurityLevel);
  } else {
    setcurseclevel(UserSys.SecurityLevel);
  }

  if (UserSys.AliasSupport && Status.UseAlias) {
    getfirstname(Status.FirstName,UserSys.Alias);
    strcpy(Status.DisplayName,UserSys.Alias);
  } else if (Status.UserRecNo == 1) {
    if (PcbData.UseRealName) {
      getfirstname(Status.FirstName,UserSys.Name);
      strcpy(Status.DisplayName,UserSys.Name);
    } else {
      getfirstname(Status.FirstName,PcbData.Sysop);
      strcpy(Status.DisplayName,"SYSOP");
    }
  } else {
    getfirstname(Status.FirstName,UserSys.Name);
    strcpy(Status.DisplayName,UserSys.Name);
  }

  return(0);

error:
  println("Error reading users.sys file");
  dosfclose(&File);
  return(-1);
}


void LIBENTRY writeusersysfile(void) {
  DOSFILE  File;
  long     LongInt;
  long _FAR_ *p;
  char     *tmp;
  unsigned X;

  if (dosfopen(SysName,OPEN_RDWR|OPEN_DENYNONE,&File) != -1) {
    UserSysHdr.Updated = TRUE;
    dosfwrite(&UserSysHdr,sizeof(UserSysHdr),&File);
    dosfwrite(&UserSys,min(sizeof(UserSys),UserSysHdr.SizeOfRec),&File);

    if (sizeof(UserSys) != UserSysHdr.SizeOfRec) {
      dosflush(&File);
      dosfseek(&File,sizeof(UserSysHdr)+UserSysHdr.SizeOfRec,SEEK_SET);
    }

    if (MsgReadPtr != NULL) {
      for (X = 0, p = MsgReadPtr; X < UserSysHdr.NumOfAreas; X++, p++) {
        LongInt = *p;
        if (dosfwrite(&LongInt,sizeof(long),&File) == -1)
          goto error;
      }
      fbfree(MsgReadPtr);
      MsgReadPtr = NULL;
    } else {
      dosflush(&File);
      dosfseek(&File,UserSysHdr.NumOfAreas * sizeof(long),SEEK_CUR);
    }

    if (ConfReg != NULL && (tmp = (char *) bmalloc(ConfByteLen)) != NULL) {
      /* write extended REGISTERED flags */
      fmemcpy(tmp,&ConfReg[CNF_REG],ConfByteLen);
      if (dosfwrite(tmp,ConfByteLen,&File) == -1)
        goto error;

      /* write extended EXPIRED flags */
      fmemcpy(tmp,&ConfReg[CNF_EXP],ConfByteLen);
      if (dosfwrite(tmp,ConfByteLen,&File) == -1)
        goto error;

      /* write extended USER SCAN flags */
      fmemcpy(tmp,&ConfReg[CNF_USR],ConfByteLen);
      if (dosfwrite(tmp,ConfByteLen,&File) == -1)
        goto error;

      /* write the CONFERENCE SYSOP flags */
      fmemcpy(tmp,&ConfReg[CNF_CON],ConfByteLen);
      if (dosfwrite(tmp,ConfByteLen,&File) == -1)
        goto error;

      /* write the CONFERENCE MAIL flags */
      fmemcpy(tmp,&ConfReg[CNF_MFL],ConfByteLen);
      if (dosfwrite(tmp,ConfByteLen,&File) == -1)
        goto error;

      /* write the CONFERENCE JOINED flags */
      fmemcpy(tmp,&ConfFlags[CNF_JOINED],ConfByteLen);
      if (dosfwrite(tmp,ConfByteLen,&File) == -1)
        goto error;

      /* write the CONFERENCE SCANNED flags */
      fmemcpy(tmp,&ConfFlags[CNF_SCANNED],ConfByteLen);
      if (dosfwrite(tmp,ConfByteLen,&File) == -1)
        goto error;

      /* write NET STATUS flags */
      if (UserSysHdr.NumOfBitFields > 7) {
        fmemcpy(tmp,&ConfReg[CNF_NET],ConfByteLen);
        if (dosfwrite(tmp,ConfByteLen,&File) == -1)
          goto error;
      }


      bfree(tmp);
      fbfree(ConfReg);
      fbfree(ConfFlags);
      ConfReg = ConfFlags = NULL;
    } else {
      dosflush(&File);
      dosfseek(&File,ConfByteLen * UserSysHdr.NumOfBitFields,SEEK_CUR);
    }

    if (TPAstatic != NULL || TPAdynamic != NULL) {
      if (TPAstatic != NULL)
        if (dosfwrite(TPAstatic,UserSysHdr.AppSizeOfRec,&File) == -1)
          goto error;

      if (TPAdynamic != NULL)
        if (dosfwrite(TPAdynamic,UserSysHdr.AppSizeOfConfRec * PcbData.NumAreas,&File) == -1)
          goto error;

      bfree(TPAstatic);
      bfree(TPAdynamic);
    }
    dosfclose(&File);
  }
  return;

error:
  println("Error updating users.sys");
}
