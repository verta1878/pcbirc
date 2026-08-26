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

#include <io.h>

#ifdef __BORLANDC__
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#else
  #include <malloc.h>
#endif

#ifdef __OS2__
  #include <semafore.hpp>
#endif

static int _NEAR_ LIBENTRY lockslowdrive(char *LockName, char *LockDrives, char Drive) {
  char Temp;
  int  LockHandle;
  int  LoopCount;
  char *p;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  LockHandle = 0;

  strcpy(LockName,PcbData.NetFile);
  if ((p = strrchr(LockName,'\\')) == NULL)
    p = LockName;
  else
    p++;
  sprintf(p,"SLOWDRIV.%s",LockDrives);

  for (LoopCount = 0; LoopCount < 3; LoopCount++) {
    for (settimer(3,ONESECOND); ! timerexpired(3); ) {

      // If the lock file does NOT exist, then create it now.  This gives us
      // ownership of the drive and lets us access it.

      // if the lock file DOES exist, then check to see if the contents of
      // the file are the same as the drive letter we are trying to access
      // right now.  If it is, then we can "borrow" the drive letter while
      // this drive is locked and both processes can access it.  When we
      // finish we will attempt to close and delete the lock file the same as
      // if we had created it.  This is because the original owner may have
      // already closed their handle.

      if (fileexist(LockName) == 255) {
        if ((LockHandle = doscreate(LockName,OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL POS2ERROR)) != -1) {
          doswrite(LockHandle,&Drive,sizeof(char) POS2ERROR); //lint !e534
          doscommit(LockHandle);
          return(LockHandle);
        }
      } else {
        if ((LockHandle = dosopen(LockName,OPEN_RDWR|OPEN_DENYNONE POS2ERROR)) != -1) {
          dosread(LockHandle,&Temp,sizeof(char) POS2ERROR);  //lint !e534
          // if the drive letters are the same, then return the locked file
          if (Temp == Drive)
            return(LockHandle);
          // otherwise, close the handle and we'll try again later.
          close(LockHandle);
        }
      }

      showactivity(ACTSHOW);

      #ifdef __OS2__
        mydelay(25);
      #else
        giveup();
      #endif
    }

    unlink(LockName);  // it's been 10 seconds, try deleting the file
  }

  return(0);
}


static void _NEAR_ LIBENTRY unlockslowdrive(char *LockName, int LockHandle) {
  dosclose(LockHandle);

  // We may not be the only one accessing the file so the unlink() call below
  // may fail.  We'll give it a couple of seconds to be successfully removed
  // and if we can't do it, we'll assume the "other" process will succeed.

  for (settimer(3,TWOSECONDS); ! timerexpired(3); ) {
    unlink(LockName);
    if (fileexist(LockName) == 255)
      return;
    #ifdef __OS2__
      mydelay(25);
    #else
      giveup();
    #endif
  }
}


static int _NEAR_ LIBENTRY copysub(char *Srce, char *Dest, bool CheckForEOF, bool ShowActivity, bool CopyAttributes) {
  int       S;
  int       D;
  int       Error;
  int       NameLen;
  unsigned  Memory;
  unsigned  BytesRead;
  char     *Buffer;
  char     *Eof;
  char     *p;
  #ifdef __BORLANDC__
    struct  ftime TimeDate;
  #else
    uint    Time;
    uint    Date;
  #endif

  NameLen = 0;
  Error = 0;

  if ((S = dosopencheck(Srce,OPEN_READ|OPEN_DENYWRIT)) == -1)
    return(COPYFILE_ERROPENSRCE);

  #ifdef __BORLANDC__
    getftime(S,&TimeDate);
  #else
    _dos_getftime(S,&Date,&Time);
  #endif

  unlink(Dest);
  if ((D = doscreatecheck(Dest,OPEN_RDWR|OPEN_DENYRDWR,OPEN_NORMAL)) == -1) {
    dosclose(S);
    return(COPYFILE_ERROPENDEST);
  }

  #ifdef __OS2__
    Memory = 16384;
  #else
    if (coreleft() > 16384)        /* Don't let it try to use */
      Memory = 16384;              /* more than an 16K buffer */
    else {
      Memory = (int) coreleft();

      if (Memory > 2048) {        /* if memory is greater than 2048 bytes  */
        Memory /= 2048;           /* then round it to a multiple of 2048   */
        Memory *= 2048;           /* for more efficient copying            */
      }
    }
  #endif

  while (1) {
    if ((Buffer = (char *) bmalloc(Memory)) != NULL)   /* if memory alloc successful */
      break;                                           /* then proceed with copy     */

    if (Memory > 256)
      Memory >>= 1;
    else {
      Error = COPYFILE_ERRMEMALLOC;
      goto end2;
    }
  }

  if (ShowActivity) {
    showactivity(ACTEND);

    if ((p = strrchr(Dest,'\\')) != NULL)
      p++;
    else if ((p = strrchr(Dest,':')) != NULL)
      p++;
    else
      p = Dest;

    NameLen = strlen(p);
    print(p);
    showactivity(ACTBEGIN);
  }

  BytesRead = Memory;
  while (BytesRead == Memory) {
    if ((BytesRead = readcheck(S,Buffer,Memory)) == 0xffff) {
      Error = COPYFILE_ERRREADSRCE;
      goto end3;
    }

    if (ShowActivity)
      showactivity(ACTSHOW);

    if (CheckForEOF)
      if ((Eof = strnchr(Buffer,26,BytesRead)) != NULL)
        BytesRead = (int) (Eof - Buffer);  //lint !e613

    if (writecheck(D,Buffer,BytesRead) == (unsigned) -1) {
      Error = COPYFILE_ERRWRITDEST;
      goto end3;
    }
  }


end3:
  bfree(Buffer);

end2:
  dosclose(S);

  #ifdef __BORLANDC__
    setftime(D,&TimeDate);
  #else
    _dos_setftime(D,Date,Time);
  #endif

  dosclose(D);

  /* I'm not sure if this can be done under Watcom */
  #ifdef __BORLANDC__
    if (CopyAttributes)
      _chmod(Dest,1,_chmod(Srce,0));  /*lint !e534 set file mode for Dest equal to Srce */
  #endif

  if (Error == COPYFILE_ERRREADSRCE || Error == COPYFILE_ERRWRITDEST)
    unlink(Dest);

  if (ShowActivity) {
    showactivity(ACTSUSPEND);     /* restore status by ending current copy */
    backupdestructive(NameLen);   /* removing the filename */
    showactivity(ACTRESUME);      /* and resuming current show status */
  }

  return(Error);
}


int LIBENTRY pcbcopyfile(char *Srce, char *Dest, bool CheckForEOF, bool ShowActivity, char *LockDrives, bool CopyAttributes) {
  int  Error;
  int  LockHandle;
  char LockName[66];
  char Params[128];

  // source = destination!  ignore copy request */
  if (stricmp(Srce,Dest) == 0)
    return(0);

  #ifdef COMM
    bool SaveIgnoreState;

    SaveIgnoreState = Asy.IgnoreCDLoss;
    Asy.IgnoreCDLoss = TRUE;
  #endif

  if (PcbData.Network && LockDrives != NULL && LockDrives[0] > 0 && Srce[1] == ':') {
    // try to lock the drive
    LockHandle = lockslowdrive(LockName,LockDrives,Srce[0]);
    // if the lock failed, then return a status that will let us come back
    // and try the process again later
    if (LockHandle == 0)
      return(COPYFILE_DRIVEINUSE);
  } else
    LockHandle = 0;

  if (PcbData.NetCopy[0] != 0) {
    sprintf(Params,"%s %s",Srce,Dest);
    Error = performshell(PcbData.NetCopy,Params,SHELLVIACOMMAND,PcbData.PriorityShells,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,-1);
  } else {
    Error = copysub(Srce,Dest,CheckForEOF,ShowActivity,CopyAttributes);
  }

  if (LockHandle)
    unlockslowdrive(LockName,LockHandle);

  #ifdef COMM
    Asy.IgnoreCDLoss = SaveIgnoreState;
  #endif
  return(Error);
}


int LIBENTRY pcbmovefile(char *Srce, char *Dest) {
  int  Code;
  char Buf[80];

  strupr(Srce);   /* just in case the sysop wrote the path in lowercase  */
  strupr(Dest);   /* convert it to uppercase so drive letters compare ok */

  if (Srce[0] == Dest[0] && ((Srce[1] == ':' && Dest[1] == ':') || Srce[1] == '\\')) {
    Code = rename(Srce,Dest);
    if (Code) {
      Code = pcbcopyfile(Srce,Dest,FALSE,FALSE,NULL,TRUE);
      if (! Code)
        unlink(Srce);
    }
  } else {
    Code = pcbcopyfile(Srce,Dest,FALSE,FALSE,NULL,TRUE);
    if (! Code)
      unlink(Srce);
  }
  if (DebugLevel >= 3) {
    sprintf(Buf,"MOVE=%d %s",Code,Srce);
    writelog(Buf,SPACERIGHT);
    sprintf(Buf,"MOVE=%d %s",Code,Dest);
    writelog(Buf,SPACERIGHT);
  }
  return(Code);
}
