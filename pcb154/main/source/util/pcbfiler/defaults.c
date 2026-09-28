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


#include <dos.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <help.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMFIELDS1 16
#define NUMFIELDS2 20

/*
static long MaxScanLines;

int jumpmaxscanlines(int Before) {
  if (! Before) {
    if (MaxScanLines < 100 || MaxScanLines > 65000L) {
      beep();
      showhelp(DEFAULTS+14);
      return(-1);
    }
  }
  return(0);
}

int jumpmaxlines(int Before) {
  if (! Before) {
    if (Work.MaxDirLines < 1 || Work.MaxDirLines > 32000) {
      beep();
      showhelp(DEFAULTS+0);
      return(-1);
    }
  }
  return(0);
}
*/

static FldType DefFields1[NUMFIELDS1] = {
 { vBOOL   ,YESNO  ,DEFAULTS+ 0, 2, 3, 1,NOCLEARFLD,"Default to 50-line mode in DIR File Editor"                              ,"",&Work.BigScreen      ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+ 1, 2, 4, 1,NOCLEARFLD,"Default to showing Primary Lines Only in DIR File Editor"                ,"",&Work.ShowParentsOnly,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+ 2, 2, 5, 1,NOCLEARFLD,"Include files found on DISK that are not listed in the DIR file"         ,"",&Work.ReadAll        ,NULL },
 { vCHAR   ,YNAQ   ,DEFAULTS+ 3, 2, 6, 1,NOCLEARFLD,"Verify file existence (Y=attached directory, N=No, A=All download paths)","",&Work.VerifyExist    ,NULL },
 { vCHAR   ,YNAQ   ,DEFAULTS+ 4, 2, 8, 1,NOCLEARFLD,"Perform operation (Move/Rename/Delete) on marked files"                  ,"",&Work.Perform        ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+ 5, 2, 9, 1,NOCLEARFLD,"Check for duplicate files in each text DIR listing processed"            ,"",&Work.CheckDupes     ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+ 6, 2,11, 1,NOCLEARFLD,"Perform DIR processing in NON-STOP mode (no pause when screen full)"     ,"",&Work.NonStop        ,NULL },
 { vCHAR   ,YNA    ,DEFAULTS+ 7, 2,12, 1,NOCLEARFLD,"Delete tagged files from disk (Y=Yes, N=No, A=Always i.e. don't ask)"    ,"",&Work.EraseFiles     ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+ 8, 2,14, 1,NOCLEARFLD,"Update FILE DATE with date found on disk file"                           ,"",&Work.UseDiskDate    ,NULL },
 { vCHAR   ,TONAR  ,DEFAULTS+ 9, 2,15, 1,NOCLEARFLD,"Set FILE DATE when moving files (T=Today,O=Oldest,R=Recent,N=No,A=Ask)"  ,"",&Work.SetFileDate    ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+10, 2,16, 1,NOCLEARFLD,"Create 0-Byte file when moving OFF-LINE or creating DELETED entries"     ,"",&Work.ZeroByte       ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+11, 2,17, 1,NOCLEARFLD,"Remove `Uploaded by:' lines when files are moved"                        ,"",&Work.RemoveUpldBy   ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+12, 2,18, 1,NOCLEARFLD,"Remove `Files:/Oldest:/Newest:' lines when files are moved"              ,"",&Work.RemoveDates    ,NULL },
 { vBOOL   ,YESNO  ,DEFAULTS+13, 2,20, 1,NOCLEARFLD,"Default to Expert Mode inside DIR File Editor"                           ,"",&Work.ExpertMode     ,NULL },
 { vBYTE   ,ALLNUM ,DEFAULTS+14, 2,21, 2,CLEAR     ,"Default indentation for vertical bar on Secondary Lines"                 ,"",&Work.Indent         ,NULL },
 { vCHAR   ,YNA    ,0          , 2,22, 1,NOCLEARFLD,"Overwrite Files (A=Ask First, Y=Assume Yes, N=Assume No)"                ,"",&Work.OverWrite      ,NULL }
};

static FldType DefFields2[NUMFIELDS2] = {
 { vSTR    ,CONFNUMS,DEFAULTS+15, 2, 4,40,NOCLEARFLD,"Include Conferences in File List","", Work.ConfList   ,NULL },
 { vSTR    ,CONFNUMS,DEFAULTS+16, 2, 5,40,NOCLEARFLD,"Include for Missing/Dupe Scan   ","", Work.ScanList   ,NULL },

 { vUPSTR  ,ALLFILE ,DEFAULTS+17, 2, 7,50,NOCLEARFLD,"Default Rules File"              ,"", Work.RulesName  ,NULL },
 { vUPSTR  ,ALLFILE ,DEFAULTS+18, 2, 8,50,NOCLEARFLD,"Default Moves File"              ,"", Work.ProcName   ,NULL },

 { vUPSTR  ,ALLFILE ,DEFAULTS+19, 2,10,30,NOCLEARFLD,"Off-line directory"              ,"", Work.OffLine    ,NULL },
 { vUPSTR  ,ALLFILE ,DEFAULTS+20, 2,11,30,NOCLEARFLD,"Backup directory  "              ,"", Work.BackupPath ,NULL },
 { vUPSTR  ,ALLFILE ,DEFAULTS+21, 2,14,30,NOCLEARFLD,"Original Directory"              ,"", Work.SubstOld   ,NULL },
 { vUPSTR  ,ALLFILE ,DEFAULTS+22, 2,15,30,NOCLEARFLD,"New Directory     "              ,"", Work.SubstNew   ,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+23, 2,18, 3,NOCLEARFLD,"File Extension"                  ,"", Work.View[0].Ext,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+24,28,18,30,NOCLEARFLD,"DOS Command"                     ,"", Work.View[0].Cmd,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+23, 2,19, 3,NOCLEARFLD,"File Extension"                  ,"", Work.View[1].Ext,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+24,28,19,30,NOCLEARFLD,"DOS Command"                     ,"", Work.View[1].Cmd,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+23, 2,20, 3,NOCLEARFLD,"File Extension"                  ,"", Work.View[2].Ext,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+24,28,20,30,NOCLEARFLD,"DOS Command"                     ,"", Work.View[2].Cmd,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+23, 2,21, 3,NOCLEARFLD,"File Extension"                  ,"", Work.View[3].Ext,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+24,28,21,30,NOCLEARFLD,"DOS Command"                     ,"", Work.View[3].Cmd,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+23, 2,22, 3,NOCLEARFLD,"File Extension"                  ,"", Work.View[4].Ext,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+24,28,22,30,NOCLEARFLD,"DOS Command"                     ,"", Work.View[4].Cmd,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+23, 2,23, 3,NOCLEARFLD,"File Extension"                  ,"", Work.View[5].Ext,NULL },
 { vUPSTR  ,ALLCHAR ,DEFAULTS+24,28,23,30,NOCLEARFLD,"DOS Command"                     ,"", Work.View[5].Cmd,NULL }
};


void  savedefaults(void) {
  bool Save;
  int  DefFile;

  if (memcmp(&Work,&Defaults,sizeof(DefType)) != 0) {
    switch(editexit("Exit PCBFiler Defaults Screen")) {
      case 'Y': Save = TRUE;  KeyFlags = ESC;     break;
      case 'N': Save = FALSE; KeyFlags = NOTHING; break;
      case 'A': Save = FALSE; KeyFlags = ESC;     break;
      default : return;
    }

    if (Save) {
      Defaults = Work;
      #ifdef DEMO
        Defaults.MaxDirLines   = min(Defaults.MaxDirLines,100);
        Defaults.MaxScanLines  = min(Defaults.MaxScanLines,500);
        Work.MaxDirLines  = Defaults.MaxDirLines;
        Work.MaxScanLines = Defaults.MaxScanLines;
      #endif

      if (fileexist(DefName) == 255)
        DefFile = doscreatecheck(DefName,OPEN_WRIT,OPEN_NORMAL);
      else
        DefFile = dosopencheck(DefName,OPEN_WRIT);

      if (DefFile != -1) {
        writecheck(DefFile,&Defaults,sizeof(DefType));   //lint !e534
        dosclose(DefFile);
      }
    }
  }
}


void  getdefaults1(void) {
  initquest(DefFields1,NUMFIELDS1);
  clsbox(1,1,78,23,Colors[OUTBOX]);
  while (KeyFlags != ESC) {
    readscrn(DefFields1,NUMFIELDS1-1,0,"Defaults Page 1","",1,NOCLEARFLD); //lint !e534
    savedefaults();
  }
  freeanswers(DefFields1,NUMFIELDS1);
  KeyFlags = NOTHING;
}


void  getdefaults2(void) {
  int Counter;

  initquest(DefFields2,NUMFIELDS2);
  clsbox(1,1,78,23,Colors[OUTBOX]);
  fastprint(37, 3,"(ex. 1-10,15,17,20-99,105)",Colors[DISPLAY]);
  fastprint( 2,13,"Replacement directory for RAM disk usage (leave Original blank if unused)",Colors[DISPLAY]);
  fastprint( 2,17,"File VIEWERS - filename extensions and commands for executing viewers",Colors[DISPLAY]);
  while (KeyFlags != ESC) {
    readscrn(DefFields2,NUMFIELDS2-1,0,"Defaults Page 2","",1,NOCLEARFLD);
    stripright(Work.OffLine,' ');
    stripright(Work.BackupPath,' ');
    stripright(Work.SubstOld,' ');
    stripright(Work.SubstNew,' ');
    stripright(Work.ConfList,' ');
    stripright(Work.ScanList,' ');
    addbackslash(Work.BackupPath,sizeof(Work.BackupPath)-1);
    addbackslash(Work.OffLine,sizeof(Work.OffLine)-1);
    SubstLen = strlen(Work.SubstOld);
    if (SubstLen != 0) {
      addbackslash(Work.SubstOld,sizeof(Work.SubstOld)-1);
      addbackslash(Work.SubstNew,sizeof(Work.SubstNew)-1);
    }
    for (Counter = 0; Counter < 6; Counter++) {
      stripright(Work.View[Counter].Ext,' ');
      stripright(Work.View[Counter].Cmd,' ');
    }
    savedefaults();
  }
  freeanswers(DefFields2,NUMFIELDS2);
  KeyFlags = NOTHING;
}
