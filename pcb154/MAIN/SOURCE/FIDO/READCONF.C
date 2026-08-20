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


#include <pcbtools.h>
#include <string.h>

#ifdef DEBUG
#include <memcheck.h>
#endif

pcbconftype     ConfRec;
DOSFILE         Pcbfile;

struct
{
  bool ForceEcho;          /* turn off echo question, force all msgs to echo */
  bool ReadOnly;           /* do not allow ANY msgs to be entered in conf    */
  bool NoPrivateMsgs;      /* do not allow PRIVATE msgs to be entered        */
  char RetReceiptLevel;    /* level required to request return receipts      */
  bool RecordOrigin;       /* Record ORIGIN in messages                      */
  bool PromptForRouting;   /* Prompt user for ROUTING information            */
  bool AllowAliases;       /* Allow aliases to be used                       */
  bool ShowIntroOnRA;      /* Show the Conf INTRO in the middle of R A scan  */
  char ReqLevelToEnter;    /* Security Level required to enter messages      */
  char Password[13];       /* password reqd to join if private               */
  char Intro[32];          /* name/location of conference INTRO file         */
  char AttachLoc[32];      /* location for file attachment storage           */
  char RegFlags[4];        /* RXS flags for automatic conf registration      */
  char AttachLevel;        /* level required to attach files to messages     */
  char CarbonLimit;        /* max number of names in carbon list             */
  char CmdLst[32];         /* name/location of CMD.LST use instead of default*/
  bool OldIndex;           /* maintain old MSGS indexes?                     */
  bool LongToNames;        /* allow long TO: names to be entered             */
  char CarbonLevel;        /* level required to enter @LIST@ messages        */
}addrec;

pcbconftype GetConference(unsigned int ConfNum)
{
    int RecSize;
    char string[80];

    strcpy(string,PcbData.CnfFile);
    strcat(string,".@@@");
    if(dosfopen(string,OPEN_READ|OPEN_DENYNONE,&Pcbfile)==-1)
      {
        dosfclose(&Pcbfile);
        return ConfRec;
      }
    dosfread(&RecSize,sizeof(int),&Pcbfile);
    dosfseek(&Pcbfile,(ConfNum*RecSize)+sizeof(int),SEEK_SET);
    if(dosfread(&ConfRec,RecSize,&Pcbfile)==-1)
      {
        dosfclose(&Pcbfile);
        return ConfRec;
      }
    dosfclose(&Pcbfile);


    strcpy(string,PcbData.CnfFile);
    strcat(string,".ADD");
    if(dosfopen(string,OPEN_READ|OPEN_DENYNONE,&Pcbfile)==-1)
      {
        dosfclose(&Pcbfile);
        return ConfRec;
      }
    dosfseek(&Pcbfile,(ConfNum*256),SEEK_SET);
    if(dosfread(&addrec,sizeof(addrec),&Pcbfile)==-1)
      {
        dosfclose(&Pcbfile);
        return ConfRec;
      }
    memcpy(&ConfRec.ForceEcho,&addrec,sizeof(addrec));
    dosfclose(&Pcbfile);
    return ConfRec;
}
