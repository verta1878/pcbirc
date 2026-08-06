#include "pcbsm_externs.h"
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
#include <string.h>
#include <alloc.h>
#include <misc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifndef DEMO

#define BUFSIZE  32768U

typedef struct {
  char Status;            /* D=already deleted, F=flagged, T=truncated  */
  long RecNo;             /* User Record Number                         */
  char Name[26];          /* User Name                                  */
  long ReplaceRecNo;      /* Replacement User Record Number             */
  char ReplaceName[26];   /* Replacement User Name                      */
  char Cr;
  char Lf;
} stattype;


#ifdef __WATCOMC__
extern hdrtype Header;
extern apptype AppHeader;
extern addresstypez Address;
extern addresstype OldAddress;
#endif
static DOSFILE  PackFile;
static char     Reason[51];


static long near pascal totaluserrecords(int Handle) {
  return(doslseek(Handle,0,SEEK_END) / sizeof(URead));
}

static long near pascal totalinfrecords(int Handle) {
  return((doslseek(Handle,0,SEEK_END) - InfHeaderSize) / Header.TotalRecSize);
}

static long near pascal positioninfrecord(int Handle, long RecNo) {
  return(doslseek(Handle,(RecNo-1)*Header.TotalRecSize+InfHeaderSize,SEEK_SET));
}

static long near pascal positionuserrecord(int Handle, long RecNo) {
  return(doslseek(Handle,(RecNo-1) * sizeof(URead),SEEK_SET));
}

static int pascal lockrecord(int handle, long offset, long length) {
  // only lock the record if networking is enabled
  if (PcbData.Network)
    return(doslockcheck(handle,offset,length));
  return(0);
}

static void pascal unlockrecord(int handle, long offset, long length) {
  // only unlock the record if networking is enabled
  if (PcbData.Network)
    unlock(handle,offset,length);
}


static int near pascal copyinfrecord(int OutHandle, int InHandle, long NewRecNo, long OldRecNo, char *Buf) {
  long Bytes;

  positioninfrecord(InHandle,OldRecNo);   //lint !e534
  positioninfrecord(OutHandle,NewRecNo);  //lint !e534

  for (Bytes = Header.TotalRecSize; Bytes > BUFSIZE; Bytes -= BUFSIZE) {
    if (readcheck(InHandle,Buf,BUFSIZE) == (unsigned) -1)
      return(-1);
    if (writecheck(OutHandle,Buf,BUFSIZE) == (unsigned) -1)
      return(-1);
  }

  if (Bytes > 0) {
    if (readcheck(InHandle,Buf,(int) Bytes) == (unsigned) -1)
      return(-1);
    if (writecheck(OutHandle,Buf,(int) Bytes) == (unsigned) -1)
      return(-1);
  }
  return(0);
}


/********************************************************************
*
*  Function:  replaceinfrecord()
*
*  Desc    :  A bad or invalid USERS.INF record has been identified and is
*             specified as RecNo.  This function will attempt to replace it
*             using a record from the end of the USERS.INF file so that the
*             file can then be shortened.
*/

static long LastDeleteInfRec;

static void near pascal replaceinfrecord(int InfIn, int InfOut, long RecNo, char *Buf) {
  long    Total;
  long    Pos;
  long    UserRecNo;
  long    UserRecPos;
  long    NumUserRecs;
  char    Alias[26];
  rectype Inf;
  char    Str[128];
  URead   Rec;

  NumUserRecs = totaluserrecords(UsersFile);

  Total = totalinfrecords(InfIn);
  if (LastDeleteInfRec == 0)
    LastDeleteInfRec = Total;


  // The LastDeleteInfRec is used to keep track of the last record that we
  // deleted or replaced.  This is done on the *assumption* that if we have
  // already decided that a record near the end cannot be moved, then there
  // is no sense RE-considering it again, so we'll just skip over it and look
  // for the next possible replacement.

  if (LastDeleteInfRec < Total)
    Total = LastDeleteInfRec;

  /* start with last USERS.INF record and read backwards from there */

  for (; Total > RecNo; Total--) {
    Pos = positioninfrecord(InfIn,Total);
    if (readcheck(InfIn,&Inf,sizeof(rectype)) != (unsigned) -1) {
      movestr(UsersData.Name,Inf.Name,25);

      /* we can't move FIDO users because we cannot detect if they are */
      /* online - and we can't move users who are online right now, so */
      /* make sure the user is not online right now */
      if (! PcbData.Network || (memcmp(UsersData.Name,"~FIDO~",6) != 0 && ! foundinusernet(UsersData.Name))) {

        // if alias support is enabled, then we need to check the usernet
        // file for the alias -- but only if networking is enabled and there
        // is a chance that the user is online
        if (AliasSupport && PcbData.Network) {
          doslseek(InfIn,Pos+AliasOffset,SEEK_SET);
          if (readcheck(InfIn,Alias,25) != 25)
            continue;
          if (Alias[0] > ' ') {
            Alias[25] = 0;
            if (foundinusernet(Alias))
              continue;
          }
        }

        /* see if we can find the user in the index files */
        if ((UserRecNo = finduser(UsersData.Name)) != -1 && UserRecNo <= NumUserRecs) {

          /* the user name IS in the index file, let's continue */
          UserRecPos = positionuserrecord(UsersFile,UserRecNo);

          /* lock the USERS record specified by the index */
          if (lockrecord(UsersFile,UserRecPos,sizeof(URead)) != -1) {

            /* lock the USERS.INF record we're on (scanning back from end) */
            if (lockrecord(InfIn,Pos,Header.TotalRecSize) != -1) {

              /* read the USERS record into memory */
              if (readcheck(UsersFile,&Rec,sizeof(URead)) != (unsigned) -1) {

                /* copy the USERS.INF record pointed to by the current end */
                /* pointer (Total) into the record that we are replacing   */
                /* the bad/invalid record indicated by RecNo.              */

                if (copyinfrecord(InfOut,InfIn,RecNo,Total,Buf) != -1) {

                  // record the number of the record that we last considered
                  // as a valid record for movement
                  LastDeleteInfRec = Total;

                  // Update the USERS record which contains the pointer to
                  // the USERS.INF record that we just moved.

                  Rec.RecNum = RecNo;
                  doslseek(UsersFile,UserRecPos,SEEK_SET);
                  if (writecheck(UsersFile,&Rec,sizeof(URead)) != (unsigned) -1) {
                    sprintf(Str,"%-25.25s moved %ld to %ld",Rec.Name,Total,RecNo);
                    fastprint(5,19,Str,Colors[QUESTION]);
                    scrollup(5,10,74,19,Colors[OUTBOX]);
                    if (Rem.Printer) {
                      dosfputs(Str,&prn);     //lint !e534
                      dosfputs("\r\n",&prn);  //lint !e534
                    }
                    unlockrecord(InfIn,Pos,Header.TotalRecSize);
                    unlockrecord(UsersFile,UserRecPos,sizeof(URead));

                    /* if the record we just moved still happens to be at   */
                    /* the end of the USERS.INF file then truncate the file */

                    if (totalinfrecords(InfIn) == Total) {
                      /* truncate USERS.INF file */
                      dostrunc(InfOut,Pos);           //lint !e534
                      if (Rem.Printer) {
                        sprintf(Str,"(USERS.INF) Record %ld truncated\r\n",Total);
                        dosfputs(Str,&prn);           //lint !e534
                      }
                    } else {
                      // it was not at the end of the file, so let's fill
                      // the name with spaces so that next time it will get
                      // packed out
                      doslseek(InfOut,Pos,SEEK_SET);
                      memset(&Rec,0,sizeof(Rec));
                      writecheck(InfOut,&Rec,sizeof(Rec));
                      if (Rem.Printer) {
                        sprintf(Str,"Record %ld blanked for later removal\r\n",Total);
                        dosfputs(Str,&prn);           //lint !e534
                      }
                    }
                    break;  /* break out of the for() loop now! */
                  }
                }
              }
              unlockrecord(InfIn,Pos,Header.TotalRecSize);
            }
            unlockrecord(UsersFile,UserRecPos,sizeof(URead));
          }
        } else {

          /* Corresponding user record NOT found! Check to see if we can */
          /* remove this record from the users.inf file immediately!     */

          if (totalinfrecords(InfIn) == Total) {
            /* truncate USERS.INF file */
            dostrunc(InfOut,Pos);                     //lint !e534
            if (Rem.Printer) {
              sprintf(Str,"(USERS.INF) Record %ld truncated\r\n",Total);
              dosfputs(Str,&prn);                     //lint !e534
            }
            // don't decrement Total here, the for() loop will do it for us
          }
        }
      }
    }
  }

/*
  if (NumUserRecs != totaluserrecords(UsersFile)) {
    sprintf(Str,"SIZE CHANGED:  was %ld, now %ld - packing %ld, %s\r\n",
            NumUserRecs,
            totaluserrecords(UsersFile),
            RecNo,
            Buf);
     dosfputs(Str,&prn);
  }
*/
}


static void near pascal packinffile(void) {
  int      InfIn;
  int      InfOut;
  long     Counter;
  long     Total;
  long     RecNo;
  URead    Rec;
  char    *Buf;
  rectype  Inf;
  char     Str[80];

  clsbox(5,8,40,8,Colors[DISPLAY]);
  fastprint(5,8,"Purifying USERS.INF File",Colors[DISPLAY]);
  scrollup(5,10,74,19,Colors[OUTBOX]);

  if ((InfIn = dosopencheck(PcbData.InfFile,OPEN_READ|OPEN_DENYNONE)) == -1)
    return;

  if ((InfOut = dosopencheck(PcbData.InfFile,OPEN_WRIT|OPEN_DENYNONE)) == -1) {
    dosclose(InfIn);
    return;
  }

  PrintHeadStr = "USERS.INF Purification History";
  if (Rem.Printer) {
    if (printheading(FALSE) == -1) {
      dosclose(InfOut);
      dosclose(InfIn);
      return;
    }
  }


  if ((Buf = (char *) malloc(BUFSIZE)) == NULL) {
    dosclose(InfOut);
    dosclose(InfIn);
    return;
  }

  Total = totalinfrecords(InfIn);
  for (Counter = 1; Counter <= Total; Counter++) {
    sprintf(Str,"Processing%7ld of%7ld",Counter,Total);
    fastprint(5,19,Str,Colors[QUESTION]);
    positioninfrecord(InfIn,Counter);  //lint !e534
    if (readcheck(InfIn,&Inf,sizeof(rectype)) == sizeof(rectype)) {

      /* If the USERS.INF record is blank, or a corresponding USERS record  */
      /* does not exist then replace the USERS.INF record with one from the */
      /* end of the file and truncate the file to shorten (if possible).    */

      if (Inf.Name[0] == 0 || ((RecNo = finduser(Inf.Name)) == -1)) {
        if (Inf.Name[0] == 0) {
          // if the name was blanked, then we'll just be removing it
          if (Rem.Printer) {
            sprintf(Str,"Record %ld was blanked from previous action",Counter);
            dosfputs(Str,&prn);        //lint !e534
            dosfputs("\r\n",&prn);     //lint !e534
          }
        } else {
          // otherwise, we couldn't find this user in the index files so we'll
          // remove him now since the record is probably a duplicate.
          Inf.Name[sizeof(Inf.Name)] = 0;
          stripright(Inf.Name,' ');
          sprintf(Str,"Record %ld specifies unknown user (%s)",Counter,Inf.Name);
          fastprint(5,19,Str,Colors[QUESTION]);
          scrollup(5,10,74,19,Colors[OUTBOX]);
          if (Rem.Printer) {
            dosfputs(Str,&prn);        //lint !e534
            dosfputs("\r\n",&prn);     //lint !e534
          }
        }
        replaceinfrecord(InfIn,InfOut,Counter,Buf);
      } else {

        /* USERS.INF appears okay, let's make sure it matches the name in */
        /* the USERS record.  If it does not, the replace the USERS.INF   */
        /* record with one from the end of the file and truncate the file */
        /* to shorten it (if possible).                                   */

        positionuserrecord(UsersFile,RecNo);  //lint !e534
        if (readcheck(UsersFile,&Rec,sizeof(URead)) != (unsigned) -1 && memcmp(Rec.Name,Inf.Name,25) != 0) {
          /* Name mismatch between USERS and USERS.INF records */

          Inf.Name[sizeof(Inf.Name)] = 0;
          Rec.Name[sizeof(Rec.Name)] = 0;
          stripright(Inf.Name,' ');
          stripright(Rec.Name,' ');
          sprintf(Str,"Record %ld mismatch (%s/%s)",Counter,Inf.Name,Rec.Name);
          fastprint(5,19,Str,Colors[QUESTION]);
          scrollup(5,10,74,19,Colors[OUTBOX]);
          if (Rem.Printer) {
            dosfputs(Str,&prn);             //lint !e534
            dosfputs("\r\n",&prn);          //lint !e534
          }
          replaceinfrecord(InfIn,InfOut,Counter,Buf);
        }
      }
    }
    Total = totalinfrecords(InfIn);
  }

  free(Buf);
  dosclose(InfIn);
  dosclose(InfOut);
}


static void near pascal removeinfrecord(URead *Rec) {
  long    Offset;
  rectype Inf;

  if (Rec->RecNum >= 1 && Rec->RecNum <= totalinfrecords(DosUsersInfFile.handle)) {
    Offset = positioninfrecord(DosUsersInfFile.handle,Rec->RecNum);
    if (readcheck(DosUsersInfFile.handle,&Inf,sizeof(rectype)) != (unsigned) -1) {
      if (memcmp(Rec->Name,Inf.Name,25) == 0) {
        memset(&Inf,0,sizeof(rectype));
        doslseek(DosUsersInfFile.handle,Offset,SEEK_SET);
        writecheck(DosUsersInfFile.handle,&Inf,sizeof(rectype));  //lint !e534
      }
    }
  }
}


static void near pascal updatepackfile(long Counter, stattype *Pack, char Status, long ReplaceNum, char *ReplaceName) {
  long CurPos;

  CurPos = doslseek(PackFile.handle,0,SEEK_CUR);
  doslseek(PackFile.handle,Counter*sizeof(stattype),SEEK_SET);
  Pack->Status = Status;
  if (ReplaceNum != 0) {
    Pack->ReplaceRecNo = ReplaceNum;
    strcpy(Pack->ReplaceName,ReplaceName);
  }
  writecheck(PackFile.handle,Pack,sizeof(stattype));  //lint !e534
  doslseek(PackFile.handle,CurPos,SEEK_SET);
}


static void near pascal truncateusers(void) {
  long     Counter;
  stattype Pack;

  fastprint(5,19,"Truncating Users File (Deleted Records at End of File)",Colors[HEADING]);
  scrollup(5,10,74,19,Colors[OUTBOX]);
  dosflush(&PackFile);
  Counter = dosfseek(&PackFile,0,SEEK_END) / sizeof(stattype);
  for (Counter--; Counter >= 0; Counter--) {
    doslseek(PackFile.handle,Counter * sizeof(stattype),SEEK_SET);
    if (readcheck(PackFile.handle,&Pack,sizeof(stattype)) != (unsigned) -1) {
      /* is last record deleted also the last record in the users file? */
      if (Pack.RecNo == totaluserrecords(UsersFile)) {
        /* yes, remove the record from the users file */
        positionuserrecord(UsersFile,Pack.RecNo);     //lint !e534
        dostrunc(UsersFile,-1);                       //lint !e534
        /* also set the status to (T)runcated */
        updatepackfile(Counter,&Pack,'T',0,NULL);
      } else {
        /* if not last record in users file abort truncation process */
        return;
      }
    }
  }
}


static void near pascal replacerecords(void) {
  int      InfFile;
  long     Counter;
  long     DelRecNum;
  long     DelRecOffset;
  long     RepRecNum;
  long     RepRecOffset;
  long     Total;
  long     Pos;
  long     LastDeleteRec;
  long     NumPackRecords;
  stattype Pack;
  char     Alias[26];
  char     Str[80];
  URead    DelRec;

  clsbox(5,8,40,8,Colors[DISPLAY]);
  fastprint(5,8,"Replacing Deleted Records",Colors[DISPLAY]);
  scrollup(5,10,74,19,Colors[OUTBOX]);
  scrollup(5,10,74,19,Colors[OUTBOX]);

  NumPackRecords = dosfseek(&PackFile,0,SEEK_END) / sizeof(stattype);
  dosrewind(&PackFile);

  // if alias support is enabled, and networking is too, then there is a chance
  // that the user may be online using an alias so we will need to open the
  // users.inf file in order to pick up the alias information
  if (AliasSupport && PcbData.Network) {
    if ((InfFile = dosopencheck(PcbData.InfFile,OPEN_READ|OPEN_DENYNONE)) == -1)
      return;
  }

  LastDeleteRec = totaluserrecords(UsersFile);

  for (Counter = 0; dosfread(&Pack,sizeof(stattype),&PackFile) == sizeof(stattype); Counter++) {
    Total        = totaluserrecords(UsersFile);
    DelRecNum    = Pack.RecNo;
    DelRecOffset = (Pack.RecNo-1) * sizeof(UsersRead);

    sprintf(Str,"Processing%7ld of%7ld",Counter+1,NumPackRecords);
    fastprint(5,19,Str,Colors[QUESTION]);

    // has the record already been removed from the file?
    // if so, this is signified by the record number being beyond the end
    // of the users file

    if (DelRecNum > Total) {
      /* yes, indicate record was truncated already */
      updatepackfile(Counter,&Pack,'T',0,NULL);
      continue;
    }

    if (lockrecord(UsersFile,DelRecOffset,sizeof(UsersRead)) != -1) {

     /* Verify that the record number we're about to replace is still    */
     /* flagged for deletion.  The reason being, if the record has been  */
     /* truncated and a _NEW_ record subsequently added in its location, */
     /* then the record should not now be replaced!  Instead, update the */
     /* stats to indicate that the record must have been truncated.      */

      doslseek(UsersFile,DelRecOffset,SEEK_SET);
      if (readcheck(UsersFile,&DelRec,sizeof(URead)) != (unsigned) -1 && DelRec.DeleteFlag == 'Y') {

        // Set up a loop that searches backwards from the end of the users
        // file searching for a suitable replacement record for the deleted
        // record   -  NOTE:  LastDeleteRec is a variable which is used to
        // help speed up the process.  It assumes that if we have skipped over
        // a bunch of previous records (because perhaps they cannot be removed)
        // then there is no sense RE-considering them again, so we start the
        // search at the LastDeleteRec instead of at the very end of the file.

        RepRecNum = (LastDeleteRec < Total ? LastDeleteRec : Total);

        for (; RepRecNum >= DelRecNum; RepRecNum--) {
          RepRecOffset = (RepRecNum-1) * sizeof(UsersRead);
          doslseek(UsersFile,RepRecOffset,SEEK_SET);
          if (readcheck(UsersFile,&UsersRead,sizeof(URead)) == (unsigned) -1)
            continue;  // repeat the inner for() loop

          movestr(UsersData.Name,UsersRead.Name,25);

          // don't move FIDO users, and don't move users that are currently online
          if (PcbData.Network && (memcmp(UsersData.Name,"~FIDO~",6) == 0 || foundinusernet(UsersData.Name)))
            continue;  // repeat the inner for() loop

          // check to see if the user is online using an alias - of course,
          // a user can't be online unless networking is enabled
          if (AliasSupport && PcbData.Network) {
            Pos = positioninfrecord(InfFile,UsersRead.RecNum);
            doslseek(InfFile,Pos+AliasOffset,SEEK_SET);
            if (readcheck(InfFile,Alias,25) != 25)
              continue; // repeat the inner for() loop
            if (Alias[0] > ' ') {
              Alias[25] = 0;
              if (foundinusernet(Alias))
                continue; // repeat the inner for() loop
            }
          }

          // remember the number of the last record that we considered to be
          // valid for removal or replacement
          LastDeleteRec = RepRecNum;

          /* is the replacement record already flagged for deletion? */

          if (UsersRead.DeleteFlag == 'Y') {
            /* yes, so don't bother copying it to the deleted record! */
            /* is it also the LAST record in the users file?          */
            if (RepRecNum == totaluserrecords(UsersFile)) {
              /* yes, remove the record from the users file */
              dostrunc(UsersFile,RepRecOffset);  //lint !e534
              if (Rem.Printer) {
                sprintf(Str,"(USERS) Record %ld truncated",RepRecNum);
                dosfputs(Str,&prn);     //lint !e534
                dosfputs("\r\n",&prn);  //lint !e534
              }
              removeinfrecord(&DelRec);
              updatepackfile(Counter,&Pack,'T',RepRecNum,UsersData.Name);
              /* don't decrement Total here, for() loop will do it */
              continue; // repeat the inner for() loop
            }
          } else {
            /* no, go ahead and copy the replacement */
            if (lockrecord(UsersFile,RepRecOffset,sizeof(UsersRead)) != -1) {
              doslseek(UsersFile,DelRecOffset,SEEK_SET);
              writecheck(UsersFile,&UsersRead,sizeof(UsersRead)); //lint !e534
              updateindex(UsersRead.Name,DelRecNum);
              removeinfrecord(&DelRec);
              updatepackfile(Counter,&Pack,'R',RepRecNum,UsersData.Name);
              /* is it the last record in the users file? */
              if (RepRecNum == totaluserrecords(UsersFile)) {
                /* yes, remove the record from the users file */
                dostrunc(UsersFile,RepRecOffset);  //lint !e534
                if (Rem.Printer) {
                  sprintf(Str,"(USERS) Record %ld truncated",RepRecNum);
                  dosfputs(Str,&prn);     //lint !e534
                  dosfputs("\r\n",&prn);  //lint !e534
                }
//              Was Total--  .. but should it even be in here?
//              RepRecNum--;     /* we now have one less record in the file */
              } else {
                /* if not last record in users file, empty record, set delete flag */
                memset(&UsersRead,0,sizeof(UsersRead));
                memcpy(UsersRead.Name,"DELETED RECORD - IGNORE  ",25);
                UsersRead.DeleteFlag = 'Y';
                doslseek(UsersFile,RepRecOffset,SEEK_SET);
                doswrite(UsersFile,&UsersRead,sizeof(UsersRead)); //lint !e534
                if (Rem.Printer) {
                  sprintf(Str,"(USERS) Record %ld cannot be truncated, marked as DELETED",DelRecNum);
                  dosfputs(Str,&prn);     //lint !e534
                  dosfputs("\r\n",&prn);  //lint !e534
                }
              }
              unlockrecord(UsersFile,RepRecOffset,sizeof(UsersRead));
              // the record has been successfully moved so break out of the *
              // for() loop that searches for a replacement record
              break;
            }
          }
        }
      } else {
        /* The record is not readable or is not flagged for deletion.     */
        /* Chances are, a new user has signed on in the interim and his   */
        /* record is brand new and should not be replaced.  This probably */
        /* indicates that the record we wanted to replace was already     */
        /* truncated out of the file by earlier processing so indicate    */
        /* truncation in the pack file.                                   */
        updatepackfile(Counter,&Pack,'T',0,NULL);
      }

      unlockrecord(UsersFile,DelRecOffset,sizeof(UsersRead));
    }
  }

  // we only open the users.inf file if both alias support is enabled as well
  // as networking, so close it now if it's appropriate to do so
  if (AliasSupport && PcbData.Network)
    dosclose(InfFile);
}


static void near pascal printreport(void) {
  char     Str[80];
  stattype Pack;

  if ((dosfseek(&PackFile,0,SEEK_END) / sizeof(stattype)) == 0)
    return;

  dosrewind(&PackFile);

  PrintHeadStr = "USERS File Packing History";
  if (printheading(FALSE) == -1)
    return;

  while (dosfread(&Pack,sizeof(stattype),&PackFile) == sizeof(stattype)) {
    sprintf(Str,"%5ld %-26.26s",Pack.RecNo,Pack.Name);
    dosfputs(Str,&prn);      //lint !e534
    switch(Pack.Status) {
      case 'T': strcpy(Str,"truncated\r\n");
                break;
      case 'R': sprintf(Str,"replaced with %5ld %s\r\n",Pack.ReplaceRecNo,Pack.ReplaceName);
                break;
      case 'D': strcpy(Str,"already marked for deletion but not removed\r\n");
                break;
      case 'F': strcpy(Str,"flagged for deletion but not removed\r\n");
                break;
    }
    if (dosfputs(Str,&prn) == -1)
      return;
  }

  if (dosfputs("\r\n",&prn) != -1)
    dosflush(&prn);
}


/********************************************************************
*
*  Function: packonline()
*
*  Desc    : Packs the users and users.inf files while users are online
*
*  Returns : -1 if the operation was aborted otherwise 0
*/

int pascal packonline(void) {
  bool     Deleted;
  int      InfFile;
  long     LastDeleted;
  long     Counter;
  long     Total;
  long     Pos;
  char     Temp[9];
  stattype Pack;
  char     Alias[26];
  char     Str[40];
  char     Divider[90];

  setcursor(CUR_BLANK);
  createallindexes();

  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,"Perform Online Pack");
  boxcls(3,9,75,20,Colors[OUTBOX],SINGLE);

  if (Rem.Printer) {
    if (printheading(TRUE) == -1)
      goto abort;
    memset(Divider,'-',78);
    Divider[78] = '\r';
    Divider[79] = '\n';
    Divider[80] = 0;
  }

  if (Rem.NumDays != 9999) {
    datestr(Temp);
    Rem.Since = datetojulian(Temp) - Rem.NumDays;
  }

  if (openusersfile() == -1)
    return(-1);

  if (dosfopen(PcbData.UsrFile,OPEN_READ|OPEN_DENYNONE,&DosUsersFile) == -1)
    goto abort;

  if (openusersinffile(PcbData.InfFile) == -1)
    goto abort;

  // we only need to check the aliases if both aliases and networking are
  // enabled
  if (AliasSupport && PcbData.Network) {
    // if alias support is installed then we need to keep the users.inf file
    // open while we scan for records to be deleted so that we can find out
    // the alias names of users to see if they are online
    InfFile = dosopencheck(PcbData.InfFile,OPEN_READ|OPEN_DENYNONE);
    if (InfFile == -1) {
      beep();
      if (! BatchMode) {
        memset(&MsgData,0,sizeof(MsgData));
        MsgData.AutoBox = TRUE;
        MsgData.Save    = TRUE;
        MsgData.Msg1    = "ERROR!  Unable to perform Online Pack";
        MsgData.Line1   = 18;
        MsgData.Color1  = Colors[HEADING];
        showmessage();
      }
      goto abort;
    }
  }

  if (dosfopen("PACKSTAT.$$$",OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&PackFile) == -1)
    goto abort;

  if (openusernet() == -1)
    goto abort;

  dossetbuf(&PackFile,8192);
  dossetbuf(&DosUsersFile,16384);

  LastDeleted = 0;
  Total       = totaluserrecords(UsersFile);

  if (doslseek(UsersFile,0,SEEK_END) != Total * sizeof(URead))
    dostrunc(UsersFile,Total*sizeof(URead));

  fastprint(5,18,"Scanning Users File",Colors[HEADING]);

  dosrewind(&DosUsersFile);
  for (Counter = 1; Counter <= Total; Counter++) {
    if (readpackinfo() == -1) {
      if (Rem.Printer)
        dosfputs("Error reading pack info\r\n",&prn);
      goto abort;
    }

    sprintf(Str,"Processing%7ld of%7ld",Counter,Total);
    fastprint(5,8,Str,Colors[DISPLAY]);

    clsbox(5,19,29,19,Colors[QUESTION]);
    fastprint(5,19,UsersData.Name,Colors[QUESTION]);
    Deleted = FALSE;

    if (Rem.Deleted && (UsersData.DeleteFlag || UsersData.SecurityLevel == 0)) {
      strcpy(Reason,"Deleted or Locked Out");
      Deleted = TRUE;
    } else if (memcmp(UsersData.Name,"~FIDO~",6) != 0) {
      if (UsersData.LastDateOn < Rem.Since) {
        strcpy(Reason,"Last On ");
        strcat(Reason,juliantodate(UsersData.LastDateOn));
        Deleted = TRUE;
      } else if (UsersData.RegExpDate != 0 && UsersData.RegExpDate < Rem.RegExp) {
        strcpy(Reason,"Expired ");
        strcat(Reason,juliantodate(UsersData.RegExpDate));
        Deleted = TRUE;
      }
    }

    if (Deleted) {
      if ((UsersRead.SecurityLevel >= Rem.KeepSec) && (UsersData.DeleteFlag != TRUE) || Counter == 1) {
        /* Kept because of security level or because he's the SYSOP */
        Deleted = FALSE;
      } else {
        if (UsersRead.SecurityLevel == 0 && Rem.KeepLockOut && ! UsersData.DeleteFlag) {
          /* Kept because user is Locked Out */
          Deleted = FALSE;
        } else {
          memset(&Pack,0,sizeof(stattype));
          if (! UsersData.DeleteFlag) {
            Pos = (Counter-1) * sizeof(UsersRead);
            if (lockrecord(UsersFile,Pos,sizeof(UsersRead)) != -1) {
              Pack.Status = 'F';
              doslseek(UsersFile,Pos,SEEK_SET);
              UsersRead.DeleteFlag = 'Y';
              UsersRead.SecurityLevel = 0;
              UsersRead.ExpSecurityLevel = 0;
              UsersRead.PackedFlags.Dirty = TRUE;
              doswrite(UsersFile,&UsersRead,sizeof(UsersRead)); //lint !e534
              unlockrecord(UsersFile,Pos,sizeof(UsersRead));
            } else
              Deleted = FALSE;
          } else {
            Pack.Status = 'D';
          }
        }
      }
    }

    if (Deleted) {
      Alias[0] = 0;

      // check to see if the user is online using an alias - of course, the
      // user can only be online if networking is enabled
      if (AliasSupport && PcbData.Network) {
        Pos = positioninfrecord(InfFile,UsersRead.RecNum);
        doslseek(InfFile,Pos+AliasOffset,SEEK_SET);
        if (readcheck(InfFile,Alias,25) != 25)
          continue;
        Alias[25] = 0;
        if (Alias[0] <= ' ')
          Alias[0] = 0;
      }

      if (foundinusernet(UsersData.Name) || (Alias[0] != 0 && foundinusernet(Alias))) {
        fastprint(30,19,"Deleted (Online):",Colors[HEADING]);
        fastprint(48,19,Reason,Colors[STATUS]);
        scrollup(5,10,74,19,Colors[OUTBOX]);
      } else {
        fastprint(30,19,"Flagged for Deletion:",Colors[HEADING]);
        fastprint(52,19,Reason,Colors[STATUS]);
        scrollup(5,10,74,19,Colors[OUTBOX]);
        strcpy(Pack.Name,UsersData.Name);
        Pack.RecNo = Counter;
        Pack.Cr = 13;
        Pack.Lf = 10;
        if (dosfwrite(&Pack,sizeof(stattype),&PackFile) == -1) {
          dosfputs("Error writing pack record\r\n",&prn);
          goto abort;
        }
        removeuserfromindex(UsersRead.Name);
        LastDeleted = Counter;
        if (Rem.Printer) {
          convertreadtodata();
          if (dosfputs(Divider,&prn) == -1)
            goto abort;
          LineNum++;
          if (printrec(TRUE) == -1)
            goto abort;
        }
      }
    }
  }

  clsbox(5,8,40,8,Colors[QUESTION]);
  dosfclose(&DosUsersFile);

  // if alias support and networking are enabled, then we opened the users.inf
  // file up above so we need to close it now
  if (AliasSupport && PcbData.Network)
    dosclose(InfFile);

  if (LastDeleted == Total)
    truncateusers();

  replacerecords();

  if (Rem.Printer)
    printreport();

  packinffile();
  dosclose(UsersFile);
  dosfclose(&DosUsersInfFile);
  dosfclose(&PackFile);

  closeusernet();
  unlink("PACKSTAT.$$$");
  return(0);

abort:
  closeusernet();
  if (UsersFile != -1)
    dosclose(UsersFile);
  dosfclose(&DosUsersFile);
  dosfclose(&DosUsersInfFile);
  dosfclose(&PackFile);
  return(-1);
}
#endif
