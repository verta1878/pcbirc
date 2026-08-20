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


#include <string.h>
#include <pcb.h>
#include <misc.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "unique.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMFIELDS 10

static char Search[33];
static char Replace[33];
static bool MainFiles;
static bool ConfFiles;
static bool BltFiles;
static bool ScriptFiles;
static bool DirFiles;
static bool DlPathFiles;
static bool CmdFiles;
static bool DoorFiles;


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vUPSTR,SRCHREPLACE+0,ALLFILE,3, 5,32,"Search for  ",Search ,NOCLEARFLD,NULL);
  addquest(P, 1,vUPSTR,SRCHREPLACE+1,ALLFILE,3, 6,32,"Replace with",Replace,NOCLEARFLD,NULL);

  addquest(P, 2,vBOOL ,SRCHREPLACE+2,YESNO  ,3, 8, 1,"Update basic file locations",&MainFiles  ,CLEAR,NULL);
  addquest(P, 3,vBOOL ,SRCHREPLACE+3,YESNO  ,3, 9, 1,"Update conference locations",&ConfFiles  ,CLEAR,NULL);
  addquest(P, 4,vBOOL ,SRCHREPLACE+4,YESNO  ,3,10, 1,"Update BLT.LST    contents ",&BltFiles   ,CLEAR,NULL);
  addquest(P, 5,vBOOL ,SRCHREPLACE+5,YESNO  ,3,11, 1,"Update SCRIPT.LST contents ",&ScriptFiles,CLEAR,NULL);
  addquest(P, 6,vBOOL ,SRCHREPLACE+6,YESNO  ,3,12, 1,"Update DIR.LST    contents ",&DirFiles   ,CLEAR,NULL);
  addquest(P, 7,vBOOL ,SRCHREPLACE+7,YESNO  ,3,13, 1,"Update DLPATH.LST contents ",&DlPathFiles,CLEAR,NULL);
  addquest(P, 8,vBOOL ,SRCHREPLACE+8,YESNO  ,3,14, 1,"Update CMD.LST    contents ",&CmdFiles   ,CLEAR,NULL);
  addquest(P, 9,vBOOL ,SRCHREPLACE+9,YESNO  ,3,15, 1,"Update DOORS.LST  contents ",&DoorFiles  ,CLEAR,NULL);
}


/********************************************************************
*
*  Function:  subst()
*
*  Desc    :  This is just a front-end for the substitute() function call that
*             is found in the MISC library.  The only reason for the front-end
*             is that it reduces the number of far-calls, and it also assumes
*             the two Search and Replace constants.  The net effect is that
*             the inline code that calls subst() is shorter than the code that
*             would have been used to call substitute().
*
*  Returns :  TRUE if a change was made.
*/

static bool near pascal subst(char *Str, int Len) {
  return(substitute(Str,Search,Replace,Len));
}


/********************************************************************
*
*  Function:  updatepcbfiles()
*
*  Desc    :  This function updates the filenames and paths that are stored
*             inside the PCBOARD.DAT file.
*
*  Returns :  TRUE if any changes were made.
*/

static bool near pascal updatepcbfiles(void) {
  bool Changed = FALSE;
  Changed |= subst(PcbData.HlpLoc,sizeof(PcbData.HlpLoc));
  Changed |= subst(PcbData.SecLoc,sizeof(PcbData.SecLoc));
  Changed |= subst(PcbData.ChtLoc,sizeof(PcbData.ChtLoc));
  Changed |= subst(PcbData.TxtLoc,sizeof(PcbData.TxtLoc));
  Changed |= subst(PcbData.NdxLoc,sizeof(PcbData.NdxLoc));
  Changed |= subst(PcbData.TmpLoc,sizeof(PcbData.TmpLoc));

  Changed |= subst(PcbData.UsrFile,sizeof(PcbData.UsrFile));
  Changed |= subst(PcbData.InfFile,sizeof(PcbData.InfFile));
  Changed |= subst(PcbData.ClrFile,sizeof(PcbData.ClrFile));
  Changed |= subst(PcbData.CnfFile,sizeof(PcbData.CnfFile));
  Changed |= subst(PcbData.PwdFile,sizeof(PcbData.PwdFile));
  Changed |= subst(PcbData.FscFile,sizeof(PcbData.FscFile));
  Changed |= subst(PcbData.UscFile,sizeof(PcbData.UscFile));
  Changed |= subst(PcbData.TcnFile,sizeof(PcbData.TcnFile));
  Changed |= subst(PcbData.WlcFile,sizeof(PcbData.WlcFile));
  Changed |= subst(PcbData.NewFile,sizeof(PcbData.NewFile));
  Changed |= subst(PcbData.ClsFile,sizeof(PcbData.ClsFile));
  Changed |= subst(PcbData.WrnFile,sizeof(PcbData.WrnFile));
  Changed |= subst(PcbData.ExpFile,sizeof(PcbData.ExpFile));
  Changed |= subst(PcbData.NetFile,sizeof(PcbData.NetFile));
  Changed |= subst(PcbData.RegFile,sizeof(PcbData.RegFile));
  Changed |= subst(PcbData.AnsFile,sizeof(PcbData.AnsFile));
  Changed |= subst(PcbData.TrnFile,sizeof(PcbData.TrnFile));
  Changed |= subst(PcbData.DldFile,sizeof(PcbData.DldFile));
  Changed |= subst(PcbData.CnfMenu,sizeof(PcbData.CnfMenu));

  Changed |= subst(PcbData.LogOffScr,sizeof(PcbData.LogOffScr));
  Changed |= subst(PcbData.LogOffAns,sizeof(PcbData.LogOffAns));
  Changed |= subst(PcbData.MultiLang,sizeof(PcbData.MultiLang));
  Changed |= subst(PcbData.GroupChat,sizeof(PcbData.GroupChat));
  Changed |= subst(PcbData.ColorFile,sizeof(PcbData.ColorFile));
  Changed |= subst(PcbData.ViewBatch,sizeof(PcbData.ViewBatch));
  Changed |= subst(PcbData.NetCopy,sizeof(PcbData.NetCopy));
  Changed |= subst(PcbData.ChatFile,sizeof(PcbData.ChatFile));
  Changed |= subst(PcbData.StatsFile,sizeof(PcbData.StatsFile));
  Changed |= subst(PcbData.ChatMenu,sizeof(PcbData.ChatMenu));
  Changed |= subst(PcbData.NoAnsi,sizeof(PcbData.NoAnsi));
  Changed |= subst(PcbData.SwapPath,sizeof(PcbData.SwapPath));
  Changed |= subst(PcbData.EventDatFile,sizeof(PcbData.EventDatFile));
  Changed |= subst(PcbData.EventFiles,sizeof(PcbData.EventFiles));
  Changed |= subst(PcbData.CmdLst,sizeof(PcbData.CmdLst));
  Changed |= subst(PcbData.AllFilesList,sizeof(PcbData.AllFilesList));
  Changed |= subst(PcbData.FidoLoc,sizeof(PcbData.FidoLoc));
  Changed |= subst(PcbData.FidoConfig,sizeof(PcbData.FidoConfig));
  Changed |= subst(PcbData.FidoQueue,sizeof(PcbData.FidoQueue));
  Changed |= subst(PcbData.LogOnScr,sizeof(PcbData.LogOnScr));
  Changed |= subst(PcbData.LogOnAns,sizeof(PcbData.LogOnAns));
  Changed |= subst(PcbData.FileTcan,sizeof(PcbData.FileTcan));
  Changed |= subst(PcbData.SlowDriveBat,sizeof(PcbData.SlowDriveBat));
  Changed |= subst(PcbData.CmdLoc,sizeof(PcbData.CmdLoc));
  Changed |= subst(PcbData.HolidaysFile,sizeof(PcbData.HolidaysFile));
  Changed |= subst(PcbData.AccountConfig,sizeof(PcbData.AccountConfig));
  Changed |= subst(PcbData.AccountInfo,sizeof(PcbData.AccountInfo));
  Changed |= subst(PcbData.AccountWarn,sizeof(PcbData.AccountWarn));
  Changed |= subst(PcbData.AccountTrack,sizeof(PcbData.AccountTrack));
  Changed |= subst(PcbData.AccountLogoff,sizeof(PcbData.AccountLogoff));
  Changed |= subst(PcbData.uucpPath,sizeof(PcbData.uucpPath));
  Changed |= subst(PcbData.uucpSpoolPath,sizeof(PcbData.uucpSpoolPath));
  Changed |= subst(PcbData.uucpLogPath,sizeof(PcbData.uucpLogPath));
  Changed |= subst(PcbData.uucpModFile,sizeof(PcbData.uucpModFile));
  Changed |= subst(PcbData.CompBatFile,sizeof(PcbData.CompBatFile));
  Changed |= subst(PcbData.DeCompBatFile,sizeof(PcbData.DeCompBatFile));

  return(Changed);
}


/********************************************************************
*
*  Function:  updatecnames()
*
*  Desc    :  The cnames record itself is read in by the scancnames() function
*             then updatecnames() is called to change any of the fields that
*             are found directly inside of the cnames record.
*
*  Returns :  TRUE if any changes were made.
*/

static bool near pascal updatecnames(unsigned ConfNum, pcbconftype *Rec) {
  bool Changed = FALSE;

  Changed |= subst(Rec->MsgFile   ,sizeof(Rec->MsgFile   ));
  Changed |= subst(Rec->UserMenu  ,sizeof(Rec->UserMenu  ));
  Changed |= subst(Rec->SysopMenu ,sizeof(Rec->SysopMenu ));
  Changed |= subst(Rec->NewsFile  ,sizeof(Rec->NewsFile  ));
  Changed |= subst(Rec->UpldDir   ,sizeof(Rec->UpldDir   ));
  Changed |= subst(Rec->PubUpldLoc,sizeof(Rec->PubUpldLoc));
  Changed |= subst(Rec->PrivDir   ,sizeof(Rec->PrivDir   ));
  Changed |= subst(Rec->PrvUpldLoc,sizeof(Rec->PrvUpldLoc));
  Changed |= subst(Rec->DrsMenu   ,sizeof(Rec->DrsMenu   ));
  Changed |= subst(Rec->DrsFile   ,sizeof(Rec->DrsFile   ));
  Changed |= subst(Rec->BltMenu   ,sizeof(Rec->BltMenu   ));
  Changed |= subst(Rec->BltNameLoc,sizeof(Rec->BltNameLoc));
  Changed |= subst(Rec->ScrMenu   ,sizeof(Rec->ScrMenu   ));
  Changed |= subst(Rec->ScrNameLoc,sizeof(Rec->ScrNameLoc));
  Changed |= subst(Rec->DirMenu   ,sizeof(Rec->DirMenu   ));
  Changed |= subst(Rec->DirNameLoc,sizeof(Rec->DirNameLoc));
  Changed |= subst(Rec->PthNameLoc,sizeof(Rec->PthNameLoc));
  Changed |= subst(Rec->Intro     ,sizeof(Rec->Intro     ));
  Changed |= subst(Rec->AttachLoc ,sizeof(Rec->AttachLoc ));
  Changed |= subst(Rec->CmdLst    ,sizeof(Rec->CmdLst    ));

  if (Changed)
    putconfrecord(ConfNum,Rec);

  return(Changed);
}


/********************************************************************
*
*  Function:  updatebltlst()
*
*  Desc    :  The updatebltlst() function is a callback function for the
*             updatelstfiles() function.  It is used for updating both
*             the BLT.LST and SCRIPT.LST files because they have similar
*             structures.
*
*  Returns :  TRUE if any changes were made.
*/

static bool near pascal updatebltlst(DOSFILE *In, DOSFILE *Out) {
  bool Changed;
  char NamePath[31];

  Changed = FALSE;
  NamePath[sizeof(NamePath)-1] = 0;

  while (dosfread(NamePath,sizeof(NamePath)-1,In) == sizeof(NamePath)-1) {
    Changed |= subst(NamePath,sizeof(NamePath));
    dosfwrite(NamePath,sizeof(NamePath)-1,Out);
  }
  return(Changed);
}


/********************************************************************
*
*  Function:  updatecmdlst()
*
*  Desc    :  The updatecmdlst() function is a callback function for the
*             updatelstfiles() function.  It is used for updating both
*             the CMD.LST files.
*
*  Returns :  TRUE if any changes were made.
*/

static bool near pascal updatecmdlst(DOSFILE *In, DOSFILE *Out) {
  bool    Changed;
  cmdtype Rec;

  Changed = FALSE;
  while (dosfread(&Rec,sizeof(Rec),In) == sizeof(Rec)) {
    Changed |= subst(Rec.File,sizeof(Rec.File));
    dosfwrite(&Rec,sizeof(Rec),Out);
  }
  return(Changed);
}


/********************************************************************
*
*  Function:  updatedirlst()
*
*  Desc    :  The updatedirlst() function is a callback function for the
*             updatelstfiles() function.  It is used for updating the DIR.LST
*             files.
*
*  Returns :  TRUE if any changes were made.
*/

static bool near pascal updatedirlst(DOSFILE *In, DOSFILE *Out) {
  bool         Changed;
  char         Temp[80];
  DirListType2 Rec;

  Changed = FALSE;

  while (dosfread(&Rec,sizeof(Rec),In) == sizeof(Rec)) {
    memcpy(Temp,Rec.DirPath,sizeof(Rec.DirPath));
    Temp[sizeof(Rec.DirPath)-1] = 0;
    if (subst(Temp,sizeof(Temp))) {
      padstr(Temp,' ',sizeof(Rec.DirPath));
      memcpy(Rec.DirPath,Temp,sizeof(Rec.DirPath));
      Changed = TRUE;
    }

    memcpy(Temp,Rec.DskPath,sizeof(Rec.DskPath));
    Temp[sizeof(Rec.DskPath)-1] = 0;
    if (subst(Temp,sizeof(Temp))) {
      padstr(Temp,' ',sizeof(Rec.DskPath));
      memcpy(Rec.DskPath,Temp,sizeof(Rec.DskPath));
      Changed = TRUE;
    }

    memcpy(Temp,Rec.DirDesc,sizeof(Rec.DirDesc));
    Temp[sizeof(Rec.DirDesc)-1] = 0;
    if (subst(Temp,sizeof(Temp))) {
      padstr(Temp,' ',sizeof(Rec.DirDesc));
      memcpy(Rec.DirDesc,Temp,sizeof(Rec.DirDesc));
      Changed = TRUE;
    }

    dosfwrite(&Rec,sizeof(Rec),Out);
  }

  return(Changed);
}


/********************************************************************
*
*  Function:  updatetextlst()
*
*  Desc    :  The updatetextlst() function is a callback function for the
*             updatelstfiles() function.  It is used for updating both the
*             DOORS.LST and DLPATH.LST files because of their text format.
*
*  Returns :  TRUE if any changes were made.
*/

static bool near pascal updatetextlst(DOSFILE *In, DOSFILE *Out) {
  bool Changed;
  char Buf[1024];

  Changed = FALSE;
  while (dosfgets(Buf,sizeof(Buf),In) != -1) {
    Changed |= subst(Buf,sizeof(Buf));
    dosfputs(Buf,Out);
    dosfputs("\r\n",Out);
  }
  return(Changed);
}


/********************************************************************
*
*  Function:  updatelstfiles()
*
*  Desc    :  This function is used to scan the contents of each of the
*             different kinds of .LST files (BLT.LST, DOORS.LST, etc).  It
*             uses a callback function, passed in as update(), to access each
*             of the different kinds of .LST files in the appropriate manner.
*
*  Returns :  TRUE if changes are made.
*/

static bool near pascal updatelstfiles(char *FileName, uniqueunlimited & FileNameList, bool (near pascal update(DOSFILE *In, DOSFILE *Out))) {
  bool    Changed;
  DOSFILE In;
  DOSFILE Out;

  // if the filename is blank, or if the name is already found in the list,
  // then return early without attempting to update the file
  if (FileName[0] == 0 || FileNameList.foundinlist(FileName))
    return(FALSE);

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    return(FALSE);

  if (dosfopen(FileName,OPEN_WRIT|OPEN_DENYNONE,&Out) == -1) {
    dosfclose(&In);
    return(FALSE);
  }

  dossetbuf(&Out,16384);
  dossetbuf(&In,8192);

  // call the appropriate update function for the type of .LST file that is
  // being updated
  Changed = update(&In,&Out);

  if (Changed) {
    dosflush(&Out);
    dosftrunc(&Out,-1);  /* truncate the file at this point */
  }

  dosfclose(&In);
  dosfclose(&Out);
  return(Changed);
}


/********************************************************************
*
*  Function:  scancnames()
*
*  Desc    :  This function scans the cnames files.  It is used so that the
*             scan is performed only ONCE even though there are many various
*             aspects of the information that may need to be updated.  For
*             instance, rather than having to re-scan the cnames files when
*             processing DOORS.LST, scancnames() reads the records once and
*             then calls the subprocessing functions that are necessary.
*
*  Returns :  TRUE if changes are made.
*/

static bool near pascal scancnames(uniqueunlimited & FileNameList) {
  bool        Changed = FALSE;
  unsigned    ConfNum;
  pcbconftype Rec;

  for (ConfNum = 0; ConfNum <= PcbData.NumConf; ConfNum++) {
    getconfrecord(ConfNum,&Rec);

    // scan the individual fields within the cnames structure
    if (ConfFiles)
      Changed |= updatecnames(ConfNum,&Rec);

    // check inside of the BLT.LST files
    if (BltFiles)
      Changed |= updatelstfiles(Rec.BltNameLoc,FileNameList,updatebltlst);

    // check inside of the SCRIPT.LST files
    if (ScriptFiles)
      Changed |= updatelstfiles(Rec.ScrNameLoc,FileNameList,updatebltlst);  // uses updatebltlst() to update script.lst files

    // check inside of the DIR.LST files
    if (DirFiles)
      Changed |= updatelstfiles(Rec.DirNameLoc,FileNameList,updatedirlst);

    // check inside of the DLPATH.LST files
    if (DlPathFiles)
      Changed |= updatelstfiles(Rec.PthNameLoc,FileNameList,updatetextlst);

    // check inside of the DOORS.LST files
    if (DoorFiles)
      Changed |= updatelstfiles(Rec.DrsFile,FileNameList,updatetextlst);

    // check inside of CMD.LST files
    if (CmdFiles)
      Changed |= updatelstfiles(Rec.CmdLst,FileNameList,updatecmdlst);
  }

  return(Changed);
}


/********************************************************************
*
*  Function:  perform()
*
*  Desc    :  This function is the main overseer of the search and replace
*             functionality.
*
*  Returns :  TRUE if any changes were made.
*/

static bool near pascal perform(void) {
  bool            MadeChanges = FALSE;
  uniqueunlimited FileNameList(40);

  if (MainFiles)
    MadeChanges |= updatepcbfiles();

  if (CmdFiles)
    MadeChanges |= updatelstfiles(PcbData.CmdLst,FileNameList,updatecmdlst);

  if (ConfFiles || BltFiles || ScriptFiles || DirFiles || DlPathFiles || CmdFiles || DoorFiles)
    MadeChanges |= scancnames(FileNameList);

  return(MadeChanges);
}


/********************************************************************
*
*  Function:  searchandreplace()
*
*  Desc    :  This is the user interface for the search and replace function.
*/

void pascal searchandreplace(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMFIELDS * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  fastprint( 3,18,"NOTE:  Changes to basic file locations are not made until you" ,Colors[HELPKEY]);
  fastprint(10,19,       "exit PCBSetup and SAVE.  However, all other changes are",Colors[HELPKEY]);
  fastprint(10,20,       "made immediately."                                      ,Colors[HELPKEY]);

  fastcenter(22," Press PGDN to perform operation, or press ESC to abort ",Colors[DESC]);

  readscrn(Fields,NUMFIELDS-1,0,"File Locations Search and Replace","",1,NOCLEARFLD);
  freescrn(Fields,NUMFIELDS-1);

  if (KeyFlags == PGDN) {
    if (Search[0] != 0) {
      if (perform()) {
        memset(&MsgData,0,sizeof(MsgData));
        MsgData.Msg1  = "Changes have been made.";
      } else {
        memset(&MsgData,0,sizeof(MsgData));
        MsgData.Msg1  = "No changes have been made.";
      }
      MsgData.Save    = TRUE;
      MsgData.AutoBox = TRUE;
      MsgData.Line1   = 18;
      MsgData.Color1  = Colors[HEADING];
      beep();
      showmessage();
    }
    KeyFlags = NOTHING;
  }
}
