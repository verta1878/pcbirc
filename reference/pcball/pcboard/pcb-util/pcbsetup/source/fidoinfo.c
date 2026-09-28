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


#ifdef FIDO

#include <string.h>
#include <dir.h>

#include <dosfunc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <validate.h>
#include <pcb.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "defines.h"
#include "structs.h"
#include "fidocfg.h"
#include "fidoinfo.h"
#include "fconfig.h"

#define     FIDO_INDEX_FILE         PcbData.FidoIndex

#ifdef DEBUG
#include <memcheck.h>
#endif

extern far     pcbdattype      PcbData;
extern      char            ALLTEXT[];
extern      char            ALLFILE[];

extern far ARCHIVERS        archiver_info;          /* archiver information     */
extern far DIRECTORIES      directory_info;         /* directory information    */
long                        final_offset;           /* offset in .CFG file      */


void pascal validate(char *dir)
{
  switch(checkexistence(dir,VALIDATE_DIR,NULL,FALSE))
    {
      case  ERRNOTDIR:
                              beep();
                              memset(&MsgData,0,sizeof(MsgData));
                              MsgData.Save    = TRUE;
                              MsgData.AutoBox = TRUE;
                              MsgData.Line1   = 16;
                              MsgData.Msg1    = "Found a file with the name of the directory.";
                              MsgData.Color1  = Colors[HEADING];
                              MsgData.Line2   = 18;
                              MsgData.Msg2    = dir;
                              MsgData.Color2  = Colors[HEADING];
                              MsgData.Line3   = 17;
                              MsgData.Msg3    = "Please delete the file or change the name of the directory.";
                              MsgData.Color3  = Colors[HEADING];
                              showmessage();
                              break;

      case  ERRDISKINVALID:
                              beep();
                              memset(&MsgData,0,sizeof(MsgData));
                              MsgData.Save    = TRUE;
                              MsgData.AutoBox = TRUE;
                              MsgData.Line1   = 16;
                              MsgData.Msg1    = "The drive specified is invalid.";
                              MsgData.Color1  = Colors[HEADING];
                              MsgData.Line2   = 18;
                              MsgData.Msg2    = dir;
                              MsgData.Color2  = Colors[HEADING];
                              MsgData.Line3   = 17;
                              MsgData.Msg3    = "Please change the drive letter to a valid disk.";
                              MsgData.Color3  = Colors[HEADING];
                              showmessage();
                              break;

      case  ERRPATHINVALID:
                              beep();
                              memset(&MsgData,0,sizeof(MsgData));
                              MsgData.Save    = TRUE;
                              MsgData.AutoBox = TRUE;
                              MsgData.Line1   = 16;
                              MsgData.Msg1    = "The path specified is invalid.";
                              MsgData.Color1  = Colors[HEADING];
                              MsgData.Line2   = 18;
                              MsgData.Msg2    = dir;
                              MsgData.Color2  = Colors[HEADING];
                              MsgData.Line3   = 17;
                              MsgData.Msg3    = "Please change the path to a valid path.";
                              MsgData.Color3  = Colors[HEADING];
                              showmessage();
                              break;
    }
}



/*****************************************************************************/
/* Initialize the screen fields.                                             */

static void pascal near initinfofields(FldType *P) {
  addquest(P, 0,vUPSTR,FIDODIRCFG+0,ALLFILE, 3, 9,30,Questions[FLDFIDOLOC+ 0],directory_info.incoming_packets,CLEAR,NULL);
  addquest(P, 1,vUPSTR,FIDODIRCFG+1,ALLFILE, 3,10,30,Questions[FLDFIDOLOC+ 1],directory_info.outgoing_packets,CLEAR,NULL);
  addquest(P, 2,vUPSTR,FIDODIRCFG+2,ALLFILE, 3,11,30,Questions[FLDFIDOLOC+ 2],directory_info.bad_packets     ,CLEAR,NULL);
  addquest(P, 3,vUPSTR,FIDODIRCFG+3,ALLFILE, 3,12,30,Questions[FLDFIDOLOC+ 3],directory_info.nodelist_path   ,CLEAR,NULL);
  addquest(P, 4,vUPSTR,FIDODIRCFG+4,ALLFILE, 3,13,30,Questions[FLDFIDOLOC+ 4],directory_info.work_directory  ,CLEAR,NULL);
  addquest(P, 5,vUPSTR,FIDODIRCFG+5,ALLFILE, 3,14,30,Questions[FLDFIDOLOC+ 5],directory_info.outgoing_msg    ,CLEAR,NULL);
  addquest(P, 6,vUPSTR,FIDODIRCFG+6,ALLFILE, 3,15,30,Questions[FLDFIDOLOC+ 6],directory_info.passthrough     ,CLEAR,NULL);
  addquest(P, 7,vUPSTR,FIDODIRCFG+7,ALLFILE, 3,16,30,Questions[FLDFIDOLOC+ 7],directory_info.securemail      ,CLEAR,NULL);
  addquest(P, 8,vUPSTR,FIDODIRCFG+8,ALLFILE, 3,17,30,Questions[FLDFIDOLOC+ 8],directory_info.messages      ,CLEAR,NULL);
}

/*****************************************************************************/
/* This is the entry point to this module.  It activates editing the Fido    */
/* directory and archive information.                                        */

void pascal edit_fido_dirs(void)
{
  FldType         *InfoFields;
  savescrntype     screenbuff;

  //if(fileexist(CONFIG_FILE)==255)
  //  make_fido_config();

  if((InfoFields=(FldType *)mallochk(NUMFIDOLOC*sizeof(FldType)))==NULL)
    return;

  KeyFlags = 0;
  savescreen(&screenbuff);
  if(read_fido_config(0))
    {
      clscolor(Colors[OUTBOX]);
      box(0,0,79,24,Colors[OUTBOX],DOUBLE);
      setcursor(CUR_BLANK);
      initinfofields(InfoFields);
      readscrn(InfoFields,NUMFIDOLOC-1,0,"Fido Directory Configuration","",1,NOCLEARFLD);
      validate(directory_info.incoming_packets);
      validate(directory_info.outgoing_packets);
      validate(directory_info.bad_packets);
      validate(directory_info.work_directory);
      validate(directory_info.nodelist_path);
      validate(directory_info.outgoing_msg);
      validate(directory_info.passthrough);
      validate(directory_info.securemail);
      validate(directory_info.messages);
      addbackslash(directory_info.incoming_packets,sizeof(directory_info.incoming_packets)-1);
      addbackslash(directory_info.outgoing_packets,sizeof(directory_info.outgoing_packets)-1);
      addbackslash(directory_info.bad_packets,sizeof(directory_info.bad_packets)-1);
      addbackslash(directory_info.work_directory,sizeof(directory_info.work_directory)-1);
      addbackslash(directory_info.nodelist_path,sizeof(directory_info.nodelist_path)-1);
      addbackslash(directory_info.outgoing_msg,sizeof(directory_info.outgoing_msg)-1);
      addbackslash(directory_info.passthrough,sizeof(directory_info.passthrough)-1);
      addbackslash(directory_info.securemail,sizeof(directory_info.securemail)-1);
      addbackslash(directory_info.messages,sizeof(directory_info.messages)-1);
      write_fido_config();
      restorescreen(&screenbuff);
      setcursor(CUR_NORMAL);
    }
  freescrn(InfoFields,NUMFIDOLOC-1);
  KeyFlags = ESC;
}


#endif
