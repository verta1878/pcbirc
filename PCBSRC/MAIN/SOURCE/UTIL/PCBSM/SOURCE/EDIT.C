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
#include <dos.h>
#include <math.h>
#include <alloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <country.h>
#include <misc.h>
#include <help.h>
#include <account.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define RECORDDELETIONS

enum {ALTF=FLAG12+1,ALTS,ALTR,ALTT,ALTB,ALTP,F2,F3,ALTD,ALTA,ALTL,ALTE,ALTO,ALTJ,ALTC};
enum {SHORTFORM=0,FIDOFORM,LONGFORM,MSGSFORM,ALIASFORM,ADDRESSFORM,PASSWORDFORM,VERIFYFORM,STATSFORM,NOTESFORM,ACCOUNTFORM,QWKNETFORM,NOMOREFORMS};

#define  CNFSCRNLEN 15
#define  CNFSCRNTOP 7

#define  ABORTADD   0
#define  OKAYTOADD  1
#define  DUPLICATE  2

#define  NUMQWKNETFIELDS    4
#define  NUMACCOUNTFIELDS  17
#define  NUMNOTESFIELDS     5
#define  NUMSTATSFIELDS    19
#define  NUMPASSWORDFIELDS  6
#define  NUMADDRESSFIELDS   6
#define  NUMALIASFIELDS     1
#define  NUMVERIFYFIELDS    1
#define  NUMSHORTFIELDS     6
#define  NUMLONGFIELDS     34
#define  NUMFIDOFIELDS      9

#ifdef __cplusplus
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef __WATCOMC__
extern hdrtype Header;
extern apptype AppHeader;
extern addresstypez Address;
extern addresstype OldAddress;
extern passwordtypez Password;
extern notestypez Notes;
extern callerstattype CallerStats;
#endif
bool NeedToReindex;
char LastFind;

char static SaveSrchName[26];
char static SrchName[26];
int  static CheckInf;
int  static SrchNum;
long static SrchRec;
bool static MsgClear;
bool static ShortDesc;
char static FSEDefault;
bool static WideEditor;
bool static ScrollMsgs;
bool static LongHeaders;
bool static ChatStatus;
long static LastMsgRead[CNFSCRNLEN*2];
char static Flags[CNFSCRNLEN*2][6];
char static OriginalName[26];
char static OriginalAlias[26];
char static AU[] = {2, 'A', 'U'};
bool static DataChanged;
bool static IsFidoUser;

double static Balance;

static int nodecimal(int Before);
static int checkforusername(int Before);
static int checkforaliasname(int Before);
static int calculatebalance(int Before);


static FldType LongForm[NUMLONGFIELDS] = {
  {vUPSTR   ,ALLCHAR ,UEDIT+ 0, 3, 3,25,CLEAR     ,"Name       "     ,"", UsersData.Name            ,checkforusername},
  {vSTR     ,ALLCHAR ,UEDIT+ 1, 3, 4,24,CLEAR     ,"City       "     ,"", UsersData.City            ,NULL},
  {vSTR     ,ALLPHONE,UEDIT+ 2, 3, 5,13,CLEAR     ,"B/D Phone  "     ,"", UsersData.BusDataPhone    ,NULL},
  {vSTR     ,ALLPHONE,UEDIT+ 3, 3, 6,13,CLEAR     ,"H/V Phone  "     ,"", UsersData.HomeVoicePhone  ,NULL},
  {vUPSTR   ,ALLCHAR ,UEDIT+ 4, 3, 7,12,CLEAR     ,"Password   "     ,"", UsersData.Password        ,NULL},
  {vBYTE    ,ALLNUM  ,UEDIT+ 5, 3, 8, 3,CLEAR     ,"Security   "     ,"",&UsersData.SecurityLevel   ,NULL},
  {vBOOL    ,YESNO   ,UEDIT+ 6, 3, 9, 1,NOCLEARFLD,"Expert     "     ,"",&UsersData.ExpertMode      ,NULL},
  {vCHAR    ,ALLPROTO,UEDIT+ 7, 3,10, 1,NOCLEARFLD,"Protocol   "     ,"",&UsersData.Protocol        ,NULL},
  {vBYTE    ,ALLNUM  ,UEDIT+ 8, 3,11, 2,CLEAR     ,"Page Len   "     ,"",&UsersData.PageLen         ,NULL},
  {vDATE    ,ALLDATE ,UEDIT+ 9, 3,12, 8,CLEAR     ,"Reg Ex Date"     ,"",&UsersData.RegExpDate      ,NULL},
  {vBYTE    ,ALLNUM  ,UEDIT+10, 3,13, 3,CLEAR     ,"Expired Sec"     ,"",&UsersData.ExpSecurityLevel,NULL},
  {vBOOL    ,YESNO   ,UEDIT+11, 3,14, 1,NOCLEARFLD,"Msg Clear  "     ,"",&MsgClear                  ,NULL},
  {vBOOL    ,YESNO   ,UEDIT+30, 3,15, 1,NOCLEARFLD,"Scroll Msgs"     ,"",&ScrollMsgs                ,NULL},
  {vBOOL    ,YESNO   ,UEDIT+79, 3,16, 1,NOCLEARFLD,"Short Desc "     ,"",&ShortDesc                 ,NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+12, 3,17, 5,CLEAR     ,"Last in    "     ,"",&UsersData.LastConference  ,NULL},
  {vBOOL    ,YESNO   ,UEDIT+13, 3,18, 1,NOCLEARFLD,"Delete User"     ,"",&UsersData.DeleteFlag      ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+14, 3,20,30,CLEAR     ,"Comment1   "     ,"", UsersData.UserComment     ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+15, 3,21,30,CLEAR     ,"Comment2   "     ,"", UsersData.SysopComment    ,NULL},

  {vDATE    ,ALLDATE ,UEDIT+16,45, 3, 8,CLEAR     ,"Last DIR Listing","",&UsersData.DateLastDirRead ,NULL},
  {vDATE    ,ALLDATE ,UEDIT+17,45, 4, 8,CLEAR     ,"Last Date On    ","",&UsersData.LastDateOn      ,NULL},
  {vSTR     ,ALLTIME ,UEDIT+18,45, 5, 5,CLEAR     ,"Last Time On    ","", UsersData.LastTimeOn      ,NULL},
  {vINT     ,ALLNUM  ,UEDIT+19,45, 6, 5,CLEAR     ,"Elapsed Time On ","",&UsersData.ElapsedTimeOn   ,NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+20,45, 7, 5,CLEAR     ,"Number Times On ","",&UsersData.NumTimesOn      ,NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+21,45, 8, 5,CLEAR     ,"Number Uploads  ","",&UsersData.NumUploads      ,NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+22,45, 9, 5,CLEAR     ,"Number Downloads","",&UsersData.NumDownloads    ,NULL},
  {vLONG    ,ALLNUM  ,UEDIT+23,45,10,10,CLEAR     ,"Daily Download  ","",&UsersData.DailyDnldBytes  ,NULL},
  {vFLOATD  ,ALLNUM  ,UEDIT+24,45,11,12,CLEAR     ,"Total Upload    ","",&UsersData.TotUpldBytes    ,nodecimal},
  {vFLOATD  ,ALLNUM  ,UEDIT+25,45,12,12,CLEAR     ,"Total Download  ","",&UsersData.TotDnldBytes    ,nodecimal},
  {vUNLONG  ,ALLNUM  ,UEDIT+26,45,13,10,CLEAR     ,"Messages Read   ","",&UsersRec.MsgsRead         ,NULL},
  {vUNLONG  ,ALLNUM  ,UEDIT+27,45,14,10,CLEAR     ,"Messages Left   ","",&UsersRec.MsgsLeft         ,NULL},
  {vCHAR    ,ANY     ,UEDIT+28,45,15, 1,NOCLEARFLD,"Full Scrn Editor","",&FSEDefault                ,NULL},
  {vBOOL    ,YESNO   ,UEDIT+29,45,16, 1,NOCLEARFLD,"79-Column Editor","",&WideEditor                ,NULL},
  {vBOOL    ,YESNO   ,UEDIT+31,45,17, 1,NOCLEARFLD,"Long Headers    ","",&LongHeaders               ,NULL},
  {vCHAR    ,AU      ,UEDIT+32,45,18, 1,NOCLEARFLD,"Chat Status     ","",&ChatStatus                ,NULL}
};

static FldType ShortForm[NUMSHORTFIELDS] = {
  {vBYTE   ,ALLNUM  ,UEDIT+ 5, 3, 8, 3,CLEAR     ,"Security   "     ,"",&UsersData.SecurityLevel   ,NULL},
  {vDATE   ,ALLDATE ,UEDIT+ 9, 3,10, 8,CLEAR     ,"Reg Ex Date"     ,"",&UsersData.RegExpDate      ,NULL},
  {vBYTE   ,ALLNUM  ,UEDIT+10, 3,11, 3,CLEAR     ,"Expired Sec"     ,"",&UsersData.ExpSecurityLevel,NULL},
  {vBOOL   ,YESNO   ,UEDIT+13, 3,13, 1,NOCLEARFLD,"Delete User"     ,"",&UsersData.DeleteFlag      ,NULL},
  {vSTR    ,ALLCHAR ,UEDIT+14, 3,16,30,CLEAR     ,"Comment1   "     ,"", UsersData.UserComment     ,NULL},
  {vSTR    ,ALLCHAR ,UEDIT+15, 3,17,30,CLEAR     ,"Comment2   "     ,"", UsersData.SysopComment    ,NULL}
};

static FldType FidoForm[NUMFIDOFIELDS] = {
  {vUPSTR  ,ALLCHAR ,UEDIT+74, 3, 6,12,CLEAR     ,"Session       " ,"", UsersData.Password        ,NULL},
  {vSTR    ,ALLCHAR ,UEDIT+75, 3, 7,12,CLEAR     ,"AreaFix       " ,"", UsersData.UserComment     ,NULL},
  {vSTR    ,ALLCHAR ,UEDIT+76, 3, 8, 8,CLEAR     ,"Packet        " ,"", UsersData.SysopComment    ,NULL},
  {vSTR    ,ALLPHONE,UEDIT+77, 3, 9,13,CLEAR     ,"Phone Override" ,"", UsersData.BusDataPhone    ,NULL},
  {vBYTE   ,ALLNUM  ,UEDIT+ 5, 3,10, 3,CLEAR     ,"Security      " ,"",&UsersData.SecurityLevel   ,NULL},
  {vBOOL   ,YESNO   ,UEDIT+78, 3,11, 1,NOCLEARFLD,"UpLink        " ,"",&UsersData.ExpertMode      ,NULL},
  {vDATE   ,ALLDATE ,0       , 3,12, 8,CLEAR     ,"Last Date On  " ,"",&UsersData.LastDateOn      ,NULL},
  {vSTR    ,ALLTIME ,0       , 3,13, 5,CLEAR     ,"Last Time On  " ,"", UsersData.LastTimeOn      ,NULL},
  {vBOOL   ,YESNO   ,UEDIT+13, 3,14, 1,NOCLEARFLD,"Delete User   " ,"",&UsersData.DeleteFlag      ,NULL},
};

static FldType AliasForm[NUMALIASFIELDS] = {
  {vUPSTR   ,ALLCHAR ,UEDIT+35, 3, 5,25,CLEAR     ,"Alias Name "     ,"", Alias                    ,checkforaliasname}
};

static FldType VerifyForm[NUMVERIFYFIELDS] = {
  {vUPSTR   ,ALLCHAR ,UEDIT+41, 3, 5,25,CLEAR     ,"Verification Information"     ,"", Verify      ,NULL}
};

static FldType AddressForm[NUMADDRESSFIELDS] = {
  {vSTR     ,ALLCHAR ,UEDIT+36, 3, 5,50,CLEAR     ,"Address #1 "     ,"", Address.Street[0]        ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+36, 3, 6,50,CLEAR     ,"Address #2 "     ,"", Address.Street[1]        ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+36, 3, 7,25,CLEAR     ,"City       "     ,"", Address.City             ,NULL},
  {vUPSTR   ,ALLCHAR ,UEDIT+36, 3, 8,10,CLEAR     ,"State      "     ,"", Address.State            ,NULL},
  {vUPSTR   ,ALLCHAR ,UEDIT+36, 3, 9,10,CLEAR     ,"Zip Code   "     ,"", Address.Zip              ,NULL},
  {vUPSTR   ,ALLCHAR ,UEDIT+36, 3,10,15,CLEAR     ,"Country    "     ,"", Address.Country          ,NULL}
};

static FldType PasswordForm[NUMPASSWORDFIELDS] = {
  {vUPSTR   ,ALLCHAR ,UEDIT+37, 3, 5,12,CLEAR     ,"Previous Password 1" ,"", Password.Previous[0] ,NULL},
  {vUPSTR   ,ALLCHAR ,UEDIT+37, 3, 6,12,CLEAR     ,"Previous Password 2" ,"", Password.Previous[1] ,NULL},
  {vUPSTR   ,ALLCHAR ,UEDIT+37, 3, 7,12,CLEAR     ,"Previous Password 3" ,"", Password.Previous[2] ,NULL},
  {vDATE    ,ALLDATE ,UEDIT+38, 3, 9, 8,CLEAR     ,"Last Change Date   " ,"",&Password.LastChange  ,NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+39, 3,10, 5,CLEAR     ,"# of Times Changed " ,"",&Password.TimesChanged,NULL},
  {vDATE    ,ALLDATE ,UEDIT+40, 3,12, 8,CLEAR     ,"Expiration Date    " ,"",&Password.ExpireDate  ,NULL}
};

static FldType StatsForm[NUMSTATSFIELDS] = {
  {vDATE    ,ALLDATE ,UEDIT+42, 3, 5, 8,CLEAR     ,"First Date On"          ,"",&CallerStats.FirstDateOn    , NULL},
  {vDATE    ,ALLDATE ,UEDIT+17, 3, 6, 8,CLEAR     ,"Last Date On "          ,"",&UsersData.LastDateOn       , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+20, 3, 7, 8,CLEAR     ,"Num Times On "          ,"",&UsersData.NumTimesOn       , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+43, 3, 9, 5,CLEAR     ,"# Times Paged Sysop   " ,"",&CallerStats.NumSysopPages  , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+44, 3,10, 5,CLEAR     ,"# Times Group Chat    " ,"",&CallerStats.NumGroupChats  , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+45, 3,11, 5,CLEAR     ,"# Comments to Sysop   " ,"",&CallerStats.NumComments    , NULL},
  {vUNLONG  ,ALLNUM  ,UEDIT+27, 3,12,10,CLEAR     ,"# Messages Left       " ,"",&UsersRec.MsgsLeft          , NULL},
  {vUNLONG  ,ALLNUM  ,UEDIT+26, 3,13,10,CLEAR     ,"# Messages Read       " ,"",&UsersRec.MsgsRead          , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+46, 3,15, 5,CLEAR     ,"# Security Violations " ,"",&CallerStats.NumSecViol     , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+47, 3,16, 5,CLEAR     ,"# Un-Reg Conf Attempts" ,"",&CallerStats.NumNotReg      , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+48, 3,17, 5,CLEAR     ,"# Password Failures   " ,"",&CallerStats.NumPwrdErrors  , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+49, 3,18, 5,CLEAR     ,"# Dnld Limit Reached  " ,"",&CallerStats.NumReachDnldLim, NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+50, 3,19, 5,CLEAR     ,"# Dnld File Not Found " ,"",&CallerStats.NumFileNotFound, NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+51, 3,20, 5,CLEAR     ,"# Upld Verify Failed  " ,"",&CallerStats.NumVerifyErrors, NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+52,45, 9, 5,CLEAR     ,"# Times On at   300 "   ,"",&CallerStats.Num300         , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+52,45,10, 5,CLEAR     ,"# Times On at  1200 "   ,"",&CallerStats.Num1200        , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+52,45,11, 5,CLEAR     ,"# Times On at  2400 "   ,"",&CallerStats.Num2400        , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+52,45,12, 5,CLEAR     ,"# Times On at  9600 "   ,"",&CallerStats.Num9600        , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+52,45,13, 5,CLEAR     ,"# Times On at 14400+"   ,"",&CallerStats.Num14400       , NULL}
};

static FldType NotesForm[NUMNOTESFIELDS] = {
  {vSTR     ,ALLCHAR ,UEDIT+53, 3, 5,60,CLEAR     ,"Line 1"                 ,"", Notes.Line[0]              ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+53, 3, 6,60,CLEAR     ,"Line 2"                 ,"", Notes.Line[1]              ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+53, 3, 7,60,CLEAR     ,"Line 3"                 ,"", Notes.Line[2]              ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+53, 3, 8,60,CLEAR     ,"Line 4"                 ,"", Notes.Line[3]              ,NULL},
  {vSTR     ,ALLCHAR ,UEDIT+53, 3, 9,60,CLEAR     ,"Line 5"                 ,"", Notes.Line[4]              ,NULL}
};

static FldType AccountForm[NUMACCOUNTFIELDS] = {
  {vFLOATD,ALLNUM  ,UEDIT+54, 3, 5,12,CLEAR,"Beginning Balance      " ,"",&Account.StartingBalance      , calculatebalance},
  {vBYTE  ,ALLNUM  ,UEDIT+55, 3, 7, 3,CLEAR,"Security (When Empty)  " ,"",&Account.DropSecLevel         , NULL            },
  {vFLOATD,ALLNUM  ,UEDIT+56, 3,10,12,CLEAR,"Calls Made             " ,"",&Account.DebitCall            , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+57, 3,11,12,CLEAR,"Spending Time Online   " ,"",&Account.DebitTime            , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+58, 3,12,12,CLEAR,"Reading Messages       " ,"",&Account.DebitMsgRead         , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+59, 3,13,12,CLEAR,"Capturing Messages     " ,"",&Account.DebitMsgReadCapture  , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+60, 3,14,12,CLEAR,"Writing Messages       " ,"",&Account.DebitMsgWrite        , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+61, 3,15,12,CLEAR,"Writing Echoed Messages" ,"",&Account.DebitMsgWriteEchoed  , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+62, 3,16,12,CLEAR,"Writing Priv. Messages " ,"",&Account.DebitMsgWritePrivate , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+63, 3,17,12,CLEAR,"Downloading Files      " ,"",&Account.DebitDownloadFile    , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+64, 3,18,12,CLEAR,"Downloading Bytes      " ,"",&Account.DebitDownloadBytes   , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+65, 3,19,12,CLEAR,"Using Group Chat       " ,"",&Account.DebitGroupChat       , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+66, 3,20,12,CLEAR,"Using 3rd Party App    " ,"",&Account.DebitTPU             , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+67, 3,21,12,CLEAR,"Other Charges          " ,"",&Account.DebitSpecial         , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+68,43,10,12,CLEAR,"Uploading Files     "    ,"",&Account.CreditUploadFile     , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+69,43,11,12,CLEAR,"Uploading Bytes     "    ,"",&Account.CreditUploadBytes    , calculatebalance},
  {vFLOATD,ALLNUM  ,UEDIT+70,43,12,12,CLEAR,"Other Earnings      "    ,"",&Account.CreditSpecial        , calculatebalance}
};

static FldType QwkNetForm[NUMQWKNETFIELDS] = {
  {vUNSIGNED,ALLNUM  ,UEDIT+71, 3, 6, 5,CLEAR,"Maximum Messages in QWK Packet  (0=Setup Defaults)  ","",&QwkConfig.MaxMsgs            , NULL},
  {vUNSIGNED,ALLNUM  ,UEDIT+72, 3, 7, 5,CLEAR,"Maximum Messages Per Conference (0=Setup Defaults)  ","",&QwkConfig.MaxMsgsPerConf     , NULL},
  {vLONG    ,ALLNUM  ,UEDIT+73, 3, 9,10,CLEAR,"Download Attach Limit for Personal Messages (0=None)","",&QwkConfig.PersonalAttachLimit, NULL},
  {vLONG    ,ALLNUM  ,UEDIT+73, 3,10,10,CLEAR,"Download Attach Limit for Public Messages   (0=None)","",&QwkConfig.PublicAttachLimit  , NULL},
};


static void near pascal showname(void) {
  clsbox(17,3,42,3,Colors[DISPLAY]);
  fastprint(17,3,UsersData.Name,Colors[DISPLAY]);
}


// this function is used to truncate any digits to the right of the decimal
// point so that the byte counter fields do not show anything less than whole
// byte values
static int nodecimal(int Before) {
  double TempDnld;
  double TempUpld;

  if (Before)
    return(0);

  TempDnld = UsersData.TotDnldBytes;
  TempUpld = UsersData.TotUpldBytes;
  UsersData.TotDnldBytes = floor(TempDnld);
  UsersData.TotUpldBytes = floor(TempUpld);
  return(2);
/*
  if (UsersData.TotDnldBytes != TempDnld ||
      UsersData.TotUpldBytes != TempUpld)
    return(2);
  return(0);
*/
}


static int checkforusername(int Before) {
  if (Before)
    return(0);

  if (KeyFlags >= FLAG1)
    return(2);

  if (strcmp(OriginalName,UsersData.Name) == 0)  /* are they still equal? */
    return(0);

  if (finduser(UsersData.Name) == -1)
    return(0);

  beep();
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Msg1    = "ERROR!  Name already exists in the index";
  MsgData.Line1   = 18;
  MsgData.Color1  = Colors[HEADING];
  showmessage();
  strcpy(UsersData.Name,OriginalName);
  KeyFlags = NOTHING;
  return(2);
}


static int checkforaliasname(int Before) {
  if (Before)
    return(0);

  if (KeyFlags >= FLAG1)
    return(2);

  stripright(Alias,' ');
  if (strcmp(OriginalAlias,Alias) == 0)  /* are they still equal? */
    return(0);

  if (finduser(Alias) == -1)
    return(0);

  beep();
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Msg1    = "ERROR!  Name already exists in the index";
  MsgData.Line1   = 18;
  MsgData.Color1  = Colors[HEADING];
  showmessage();
  strcpy(Alias,OriginalAlias);
  KeyFlags = NOTHING;
  return(2);
}


#define bigger(x,y) if (x < y) x = y;

static void near pascal fixnum(double *x) {
  char *p = (char *) x;

  // the following pattern has been spotted a few times, the code below
  // detects it and resets it to 0.
  if (memcmp(p,"\0\0\0\0\0\0\xF8\xFF",8) == 0) {
    *x = 0;
    DataChanged = TRUE;
  }

  if (*x > 10000000000.0) {
    *x = 0;
    DataChanged = TRUE;
  }
}


static int calculatebalance(int Before) {
  double Temp;
  char   TempStr[40];
  char   Str[40];

  if (Before)
    return(0);

  fixnum(&Account.DebitTime);
  fixnum(&Account.DebitCall);
  fixnum(&Account.DebitGroupChat);
  fixnum(&Account.DebitTPU);
  fixnum(&Account.DebitSpecial);
  fixnum(&Account.DebitMsgRead);
  fixnum(&Account.DebitMsgReadCapture);
  fixnum(&Account.DebitMsgWrite);
  fixnum(&Account.DebitMsgWriteEchoed);
  fixnum(&Account.DebitMsgWritePrivate);
  fixnum(&Account.DebitDownloadFile);
  fixnum(&Account.DebitDownloadBytes);
  fixnum(&Account.CreditUploadFile);
  fixnum(&Account.CreditUploadBytes);

  if (PcbData.Concurrent) {
    Temp = Account.DebitTime;
    bigger(Temp,Account.DebitCall);
    bigger(Temp,Account.DebitGroupChat);
    bigger(Temp,Account.DebitTPU);
    bigger(Temp,Account.DebitSpecial);
    bigger(Temp,Account.DebitMsgRead);
    bigger(Temp,Account.DebitMsgReadCapture);
    bigger(Temp,Account.DebitMsgWrite);
    bigger(Temp,Account.DebitMsgWriteEchoed);
    bigger(Temp,Account.DebitMsgWritePrivate);
    bigger(Temp,Account.DebitDownloadFile);
    bigger(Temp,Account.DebitDownloadBytes);
    Temp = Temp
         -  Account.CreditUploadFile
         -  Account.CreditUploadBytes
         -  Account.CreditSpecial;
  } else {
    Temp =  Account.DebitTime
         +  Account.DebitCall
         +  Account.DebitMsgRead
         +  Account.DebitMsgReadCapture
         +  Account.DebitMsgWrite
         +  Account.DebitMsgWriteEchoed
         +  Account.DebitMsgWritePrivate
         +  Account.DebitDownloadFile
         +  Account.DebitDownloadBytes
         +  Account.DebitGroupChat
         +  Account.DebitTPU
         +  Account.DebitSpecial
         -  Account.CreditUploadFile
         -  Account.CreditUploadBytes
         -  Account.CreditSpecial;
  }

  Balance = Account.StartingBalance - Temp;

  dcomma(TempStr,Balance);
  if (strlen(TempStr) > 12)
    substitute(TempStr,Country.ThousandSep,"",sizeof(TempStr));

  sprintf(Str,"%12.12s",TempStr);
  fastprint(29,6,Str,Colors[(Balance > 0 ? DISPLAY : HEADING)]);
  return(2);
}


/********************************************************************
*
*  Function: initcnffields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initcnffields(FldType *p, unsigned StartConf) {
  pcbconftype Conf;
  char        Q[40];
  unsigned    Line;
  unsigned    X;
  unsigned    Y;
  unsigned    Z;

  for (X = 0, Y = 0, Z = StartConf, Line = 0; X < CNFSCRNLEN; X++, Z++, Line++) {
    if (Z > PcbData.NumConf)
      return;
    Flags[X][0] = 0;
    getconfrecord(Z,&Conf);
    sprintf(Q,"%5u %-13.13s",Z,Conf.Name);
    if (isset(&ConfReg[REG],Z)) {
      addchar(Flags[X],'R');
      if (isset(&ConfReg[EXP],Z)) addchar(Flags[X],'X');
    } else if (isset(&ConfReg[EXP],Z))
      addchar(Flags[X],'L');
    if (isset(&ConfReg[USR],Z)) addchar(Flags[X],'S');
    if (isset(&ConfReg[CON],Z)) addchar(Flags[X],'C');
    if (QwkSupport && QwkConfFlags[Z]) addchar(Flags[X],'N');
    if (LastMsgRead[X] < 0)
      LastMsgRead[X] = 0;
    addquest(p,Y++,vUPSTR ,UEDIT+33,RXSCN , 2,Line+CNFSCRNTOP,5,Q ,Flags[X]       ,CLEAR,NULL);
    addquest(p,Y++,vUNLONG,UEDIT+34,ALLNUM,28,Line+CNFSCRNTOP,8,"",&LastMsgRead[X],CLEAR,NULL);
  }

  #ifndef DEMO
    for (Line = 0; X < CNFSCRNLEN*2; X++, Z++, Line++) {
      if (Z > PcbData.NumConf)
        return;
      Flags[X][0] = 0;
      getconfrecord(Z,&Conf);
      sprintf(Q,"%5u %-13.13s",Z,Conf.Name);
      if (isset(&ConfReg[REG],Z)) {
        addchar(Flags[X],'R');
        if (isset(&ConfReg[EXP],Z)) addchar(Flags[X],'X');
      } else if (isset(&ConfReg[EXP],Z))
        addchar(Flags[X],'L');
      if (isset(&ConfReg[USR],Z)) addchar(Flags[X],'S');
      if (isset(&ConfReg[CON],Z)) addchar(Flags[X],'C');
      if (QwkSupport && QwkConfFlags[Z]) addchar(Flags[X],'N');
      if (LastMsgRead[X] < 0)
        LastMsgRead[X] = 0;
      addquest(p,Y++,vUPSTR ,UEDIT+33,RXSCN  ,41,Line+CNFSCRNTOP,5,Q ,Flags[X]       ,CLEAR,NULL);
      addquest(p,Y++,vUNLONG,UEDIT+34,ALLNUM ,67,Line+CNFSCRNTOP,8,"",&LastMsgRead[X],CLEAR,NULL);
    }
  #endif
}


/********************************************************************
*
*  Function: searchforrec()
*
*  Desc    :
*
*  Calls   :
*
*  Returns : Returns the record number where the string was found, or 0 if the
*            string was not found.
*/

static long near pascal searchforrec(char FindType, bool JumpToStart) {
  bool         YesNo;
  bool         Found;
  long         Counter;
  long         Total;
  DOSFILE      File;
  savescrntype Screen;

  Total = numrecs(UsersFile,sizeof(URead));
  Found = FALSE;

  if (dosfopen(PcbData.UsrFile,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(0);

  dossetbuf(&File,16384);

  if (JumpToStart) {
    Counter = 0;
    CheckInf = 0;
    if (AliasSupport)
      CheckInf |= 1;
    if (VerifySupport)
      CheckInf |= 2;
    if (AddressSupport)
      CheckInf |= 4;
    if (PasswordSupport)
      CheckInf |= 8;
    if (NotesSupport)
      CheckInf |= 16;
  } else {
    Counter = RecNum;
    dosfseek(&File,Counter * sizeof(UsersRead),SEEK_SET);
  }

  if (CheckInf && FindType == SEARCHSTR && JumpToStart) {
    savescreen(&Screen);
    boxcls(3,18,75,22,Colors[MENUBOX],SINGLE);
    YesNo = FALSE;
    inputnum(16,20,1,"Search inside of PSAs (slows down search)",&YesNo,vBOOL,0);
    restorescreen(&Screen);
    if (! YesNo)
      CheckInf = FALSE;
  }

  while (Counter < Total) {
    Counter++;
    if (dosfread(&UsersRead,sizeof(URead),&File) == -1)
      break;

    if (PcbData.Encrypt) {
      decrypt(UsersRead.City,62); /* 62 bytes = Rec->City thru Rec->HomeVoicePhone */
      decrypt(UsersRead.UserComment,60); /* 60 bytes = both comment fields */
    }

    switch(FindType) {
      case SEARCHSTR : convertreadtostr();
                       if (strstr(CatchAll,SrchName) != NULL) {
                         Found = TRUE;
                         goto done;
                       }
                       if (CheckInf) {
                         if (readusersinffile() != -1) {
                           if (CheckInf & 1) {
                             if (strstr(Alias,SrchName) != NULL) {
                               Found = TRUE;
                               goto done;
                             }
                           }
                           if (CheckInf & 2) {
                             if (strstr((char *)Verify,SrchName) != NULL) {
                               Found = TRUE;
                               goto done;
                             }
                           }
                           if (CheckInf & 4) {
                             AddrRec.Country[sizeof(AddrRec.Country)-1] = 0;
                             strupr((char *) &AddrRec);
                             if (strstr((char*) &AddrRec,SrchName) != NULL) {
                               Found = TRUE;
                               goto done;
                             }
                           }
                           if (CheckInf & 8) {
                             PwrdRec.LastChange = 0;
                             if (strstr((char*) &PwrdRec,SrchName) != NULL) {
                               Found = TRUE;
                               goto done;
                             }
                           }
                           if (CheckInf & 16) {
                             NotesRec.Line[4][sizeof(NotesRec.Line[4])-1] = 0;
                             strupr((char *) &NotesRec);
                             if (strstr((char*) &NotesRec,SrchName) != NULL) {
                               Found = TRUE;
                               goto done;
                             }
                           }
                         }
                       }
                       break;
      case SEARCHSEC : if (UsersRead.SecurityLevel == SrchNum) {
                         Found = TRUE;
                         goto done;
                       }
                       break;
      case SEARCHEXP : if (UsersRead.ExpSecurityLevel == SrchNum) {
                         Found = TRUE;
                         goto done;
                       }
                       break;
      case SEARCHDEL : if (UsersRead.DeleteFlag == 'Y') {
                         Found = TRUE;
                         goto done;
                       }
                       break;
    }
    if (userabort())
      break;
  }

done:
  dosfclose(&File);
  if (! Found)
    return(0);
  return(Counter);
}


/********************************************************************
*
*  Function: findspecific()
*
*  Desc    :
*
*  Calls   :
*/

static bool near pascal findspecific(char FindType) {
  char FoundStr[26];
  int  TempKeyFlags;
  long Temp;
  bool Continue;
  savescrntype ScreenBuf;

  TempKeyFlags = KeyFlags;
  savescreen(&ScreenBuf);
  boxcls(3,18,75,22,Colors[MENUBOX],SINGLE);

  switch (FindType) {
    case SEARCHNAM : inputstr(16,20,25,"User Name to Find",SrchName,SrchName,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,FINDNAME);
                     Continue = SrchName[0] != 0;
                     SaveSrchName[0] = 0;
                     break;
    case SEARCHSTR : if (SaveSrchName[0] != 0)
                       strcpy(SrchName,SaveSrchName);
                     inputstr(16,20,25,"ASCII Data to Find",SrchName,SrchName,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,FINDSTR);
                     stripboth(SrchName,' ');
                     strupr(SrchName);
                     Continue = SrchName[0] != 0;
                     break;
    case SEARCHSEC : inputnum(25,20,3,"Security Level to Find",&SrchNum,vINT,0);
                     Continue = TRUE;
                     break;
    case SEARCHEXP : inputnum(21,20,3,"Expired Security Level to Find",&SrchNum,vINT,0);
                     Continue = TRUE;
                     break;
    case SEARCHDEL : fastprint(24,20,"Searching for Deleted Users...",Colors[DISPLAY]);
                     Continue = TRUE;
                     break;
    default        : return(FALSE);
  }

  Temp = -1;

  if (Continue && KeyFlags != ESC) {
    LastFind = FindType;

    if (FindType == SEARCHNAM) {
      Temp = finduser(SrchName);
      #ifdef DEMO
        if (Temp > 10)
          Temp = -1;
      #endif
      if (Temp == -1) {
        SrchRec = 0;
        boxcls(3,16,75,22,Colors[MENUBOX],SINGLE);
        stripright(SrchName,' ');
        fastcenter(18,SrchName,Colors[DISPLAY]);
        Continue = TRUE;
        inputnum(10,20,1,"Name not found... continue with a `sound alike' search",&Continue,vBOOL,0);
        if (Continue && KeyFlags != ESC) {
          while (TRUE) {
            Temp = findusersoundex(SrchName,FoundStr,&SrchRec);
            #ifdef DEMO
              if (Temp > 10)
                Temp = -1;
            #endif
            if (Temp <= 0)
              break;
            clsbox(4,17,74,21,Colors[MENUBOX]);
            fastcenter(18,FoundStr,Colors[DISPLAY]);
            Continue = FALSE;
            inputnum(13,20,1,"Found a sound alike name -- continue with search",&Continue,vBOOL,0);
            if (! Continue || KeyFlags == ESC)
              break;
          }
        }
      }
    } else {
      strcpy(SaveSrchName,SrchName);
      stripboth(SrchName,'"');
      Temp = searchforrec(FindType,TRUE);
    }

    #ifdef DEMO
      if (Temp > 10)
        Temp = -1;
    #endif

    if (Temp > 0)
      RecNum = Temp;
  }

  restorescreen(&ScreenBuf);
  KeyFlags = TempKeyFlags;
  return(FindType == SEARCHNAM && Temp != 0 && RecNum == Temp);
}


/********************************************************************
*
*  Function: findwhichever()
*
*  Desc    : Repeats the last search performed by the user.
*
*  Calls   : finduser(), searchforstr()
*/

static bool near pascal findwhichever(void) {
  long Temp;

  Temp = -1;
  if (LastFind) {
    if (LastFind == SEARCHNAM)
      Temp = finduser(SrchName);
    else
      Temp = searchforrec(LastFind,FALSE);

    #ifdef DEMO
      if (Temp > 10)
        Temp = -1;
    #endif

    if (Temp > 0)
      RecNum = Temp;
    else
      beep();
  }

  return(LastFind == SEARCHNAM && Temp != 0 && RecNum == Temp);
}


/********************************************************************
*
*  Function: checkneedarray()
*
*  Desc    : Takes as input the first letters of both the original user name
*            (Second) and the new user name (First).  If the name has changed
*            then a new user index will need to be created.  If the new name
*            begins with the same letter as the old name then only one index
*            must be generated.  If the letters are different then two indexes
*            will have to be generated.
*
*  Calls   : strcpy(), strcat(), fastprint()
*
*  Returns : TRUE if need to regenerate index, FALSE otherwise
*/

static int near pascal checkneedarray(char First, char Second, char *Msg) {
  if (First != 0) {
    strcpy(Msg,"NOTE: the name changed, ");

    if (First != 1) {
      if (First < 'A')
        First = 'A';
      else if (First > 'Z')
        First = 'Z';

      NeedToIndex[First  - 'A'] = TRUE;
    }

    if (First != Second) {
      if (Second < 'A')
        Second = 'A';
      else if (Second > 'Z')
        Second = 'Z';

      NeedToIndex[Second - 'A'] = TRUE;
      if (First != 1) {
        strcat(Msg,"both the x and x indexes");
        Msg[33] = First; Msg[39] = Second;
      } else {
        strcat(Msg,"the x index");
        Msg[28] = Second;
      }
    } else {
      strcat(Msg,"the x index");
      Msg[28] = First;
    }
    strcat(Msg," must be regenerated");
    return(TRUE);
  }
  return(FALSE);
}


/********************************************************************
*
*  Function: verifywrite()
*
*  Desc    : At the time this function is called it has already been determined
*            that the user has changed the record in some way.  This function
*            simply asks if the user wants to save the record to the user file.
*
*/

static void near pascal verifywrite(const char First, const char Second) {
  char Temp[70];
  bool NameChanged;
  int  TempKeyFlags;

  TempKeyFlags = KeyFlags;

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.Save      = TRUE;
  MsgData.X1        =  3;
  MsgData.Y1        = (First == 0 ? 16 : 14);
  MsgData.X2        = 77;
  MsgData.Y2        = 22;
  MsgData.Quest     = "Save Record to Disk";
  MsgData.QuestLine = 20;
  MsgData.Answer[0] = 'Y';
  MsgData.Answer[1] = 0;
  MsgData.Mask      = YESNO;
  MsgData.Msg2      = "The information for this record has changed.";
  MsgData.Line2     = 18;
  MsgData.Color2    = Colors[QUESTION];

  if ((NameChanged = checkneedarray(First,Second,Temp)) == TRUE) {
    MsgData.Msg1   = Temp;
    MsgData.Line1  = 16;
    MsgData.Color1 = Colors[HEADING];
  } else if (AliasSupport) {
    if (strcmp(OriginalAlias,Alias) != 0) {
      if (OriginalAlias[0] == 0)
        OriginalAlias[0] = 1;
      if ((NameChanged = checkneedarray(OriginalAlias[0],Alias[0],Temp)) == TRUE) {
        MsgData.Y1     = 14;
        MsgData.Msg1   = Temp;
        MsgData.Line1  = 16;
        MsgData.Color1 = Colors[HEADING];
      }
    }
  }
  showmessage();

  if (MsgData.Answer[0] != 'N' && KeyFlags != ESC)
    if (writeusersfile(RecNum,TRUE,FALSE) != -1)
      if (NameChanged)
        NeedToReindex = TRUE;

  KeyFlags = TempKeyFlags;
}


/********************************************************************
*
*  Function: jumprec()
*
*  Desc    :
*
*/

static long near pascal jumprec(long RecNo) {
  long Num;
  int  TempKeyFlags;
  savescrntype ScreenBuf;

  TempKeyFlags = KeyFlags;
  savescreen(&ScreenBuf);
  boxcls(19,18,58,22,Colors[MENUBOX],SINGLE);
  Num = RecNo;
  inputnum(21,20,6,"Jump to which Record Number",&Num,vLONG,0);
  if (KeyFlags == ESC || Num < 1 || Num > numrecs(UsersFile,sizeof(URead)))
    Num = RecNo;
  restorescreen(&ScreenBuf);
  KeyFlags = TempKeyFlags;
  return(Num);
}


/********************************************************************
*
*  Function: verifyadd()
*
*  Desc    : At the time this function is called, the user has already pressed
*            the ALT-A key to add a record (which merely presents a blank
*            record on the screen for editing) and the user has now exited from
*            that record.  This function merely verifies if the user wants to
*            save the record to disk.
*
*  Returns : DUPLICATE if name was duplicated
*            ABORTADD  if user answered no
*            OKAYTOADD if user answered yes
*/

static int near pascal verifyadd(long *TotRec) {
  int  TempKeyFlags;
  char Dummy1;
  char Dummy2;

  stripboth(UsersData.Name,' ');
  if (UsersData.Name[0] != 0) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Save    = TRUE;
    TempKeyFlags    = KeyFlags;

    if (checkforusername(0) == 0) {
      MsgData.QuestLine = 20;
      MsgData.Quest     = "Add Record to Disk";
      MsgData.Answer[0] = 'Y';
      MsgData.Answer[1] = 0;
      MsgData.Mask      = YESNO;
      TempKeyFlags      = KeyFlags;
      showmessage();
/*    KeyFlags = TempKeyFlags; */
      if (MsgData.Answer[0] == 'Y' && KeyFlags != ESC) {
        UsersData.PackedFlags.MsgClear      = MsgClear;
        UsersData.PackedFlags.HasMail       = FALSE;
        UsersData.PackedFlags.DontAskFSE    = (FSEDefault != 'A');
        UsersData.PackedFlags.FSEDefault    = (FSEDefault == 'Y');
        UsersData.PackedFlags.ScrollMsgBody = ScrollMsgs;
        UsersData.PackedFlags.ShortHeader   = ! LongHeaders;
        UsersData.PackedFlags.WideEditor    = WideEditor;
        UsersData.Flags.SingleLines         = ShortDesc;
        UsersData.Flags.UnAvailable         = (ChatStatus == 'U');
        convertdatatoread(&Dummy1,&Dummy2);  //lint !e534
        #ifdef DEMO
          if (numrecs(UsersFile,sizeof(URead)) >= 10)
            return(ABORTADD);
        #endif
        *TotRec = adduserrecord();
        if (adduser(UsersData.Name,*TotRec) == -1) {
          NeedToReindex = TRUE;
          NeedToIndex[UsersData.Name[0] - 'A'] = TRUE;
        }
      }
      return(OKAYTOADD);
    }
    KeyFlags = TempKeyFlags;
    return(DUPLICATE);
  }
  return(ABORTADD);
}


/********************************************************************
*
*  Function: verifydelete()
*
*  Desc    : This function is called when the user presses ALT-D to delete a
*            user record.  This function merely asks if the user really wants
*            to delete the record and if so it sets the Delete Flag to YES.
*            NOTE that to actually remove the record the user base must be
*            packed.
*
*/

#ifdef RECORDDELETIONS
static void near pascal logdelete(void) {
  char    *p;
  DOSFILE  File;
  char     Date[9];
  char     Time[9];
  char     FileName[66];
  char     Text[256];

  sprintf(Text,"%s %s Node %d, deleted record %ld: %-25.25s\r\n",datestr(Date),timestr2(Time),PcbData.NodeNum,RecNum,UsersRead.Name);

  strcpy(FileName,PcbData.NetFile);
  if ((p = strrchr(FileName,'\\')) == NULL)
    p = FileName;
  else
    p++;
  strcpy(p,"DELRECS.TXT");

  if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYWRIT|OPEN_APPEND,&File) == -1)
    return;

  dosfputs(Text,&File);
  dosfclose(&File);
}
#endif


static void near pascal verifydelete(void) {
  int  TempKeyFlags;
  bool Okay;
  savescrntype ScreenBuf;

  TempKeyFlags = KeyFlags;
  savescreen(&ScreenBuf);
  boxcls(23,18,54,22,Colors[MENUBOX],SINGLE);
  Okay = FALSE;
  inputnum(28,20,1,"Delete this Record",&Okay,vBOOL,UFDELETE);
  if (KeyFlags != ESC && Okay) {
    UsersRead.SecurityLevel = 0;
    UsersRead.DeleteFlag = 'Y';
    writeusersfile(RecNum,TRUE,FALSE); //lint !e534
    #ifdef RECORDDELETIONS
      logdelete();
    #endif
  }
  restorescreen(&ScreenBuf);
  KeyFlags = TempKeyFlags;
}


/********************************************************************
*
*  Function: cleardata()
*
*  Desc    : Fills the UsersData record with 0's and then pre-initializes all
*            "default" fields in preparation for an Add-User screen.
*/

static void near pascal cleardata(void) {
  pcbconftype Conf;
  char        DStr[11];
  unsigned    X;

  memset(&UsersData,0,sizeof(UData));

  datestr(DStr);
  UsersData.LastDateOn      = datetojulian(DStr);
  UsersData.DateLastDirRead = datetojulian(DStr);
  UsersData.RegExpDate      = 0;

  UsersData.Protocol = ' ';
  UsersData.PageLen  = 23;

  strcpy(UsersData.LastTimeOn,"00:00");

  UsersRec.MsgsRead = 0;
  UsersRec.MsgsLeft = 0;

  fmemset(MsgReadPtr,0,(PcbData.NumAreas)*sizeof(long));
  fmemset(ConfReg,0,ConfByteLen*5);

  memset(Alias,0,sizeof(Alias));
  memset(Verify,0,sizeof(Verify));
  memset(&Address,0,sizeof(addresstypez));
  memset(&Password,0,sizeof(passwordtypez));
  memset(&CallerStats,0,sizeof(callerstattype));
  memset(&Notes,0,sizeof(notestypez));
  memset(&Account,0,sizeof(accounttype));
  memset(&QwkConfig,0,sizeof(qwkconfigtype));
  memset(QwkConfFlags,0,PcbData.NumAreas*sizeof(char));

  UsersData.SecurityLevel    = PcbData.UserLevels[SEC_REG];
  UsersData.ExpSecurityLevel = PcbData.DefExpiredLevel;
  Account.StartingBalance    = AccountRates.NewUserBalance;
  Account.DropSecLevel       = PcbData.DefExpiredLevel;

  UsersData.Flags.SingleLines = FALSE;

  /* auto-register the caller in all PUBLIC conferences that have NO */
  /* SECURITY requirement as long as the sysop specifies AutoRegConf */

  if (PcbData.AutoRegConf) {
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&Conf);
      if (Conf.PublicConf && Conf.ReqSecLevel <= 0) {
        if (Conf.RegFlags[0] != 0) {
          if (strchr(Conf.RegFlags,'R') != NULL)
            setbit(&ConfReg[REG],X);
          if (strchr(Conf.RegFlags,'X') != NULL)
            setbit(&ConfReg[EXP],X);
          if (strchr(Conf.RegFlags,'S') != NULL)
            setbit(&ConfReg[USR],X);
        }
      }
    }
  }
}


/********************************************************************
*
*  Function: makeeditscrn()
*
*  Desc    : Sets up the edit screen based on the screen number passed to it
*
*/

static void near pascal makeeditscrn(char Scrn) {
  clscolor(Colors[OUTBOX]);
  generalscreen("","");
  fastprint(3, 3,"Name        :",Colors[DISPLAY]);
  fastprint(1,23,"  ESC=Exit    PgDn=Forw    PgUp=Back    Ctrl-PgDn=Forw20    Ctrl-PgUp=Back20  ",Colors[DESC]);

  switch (Scrn) {
    case SHORTFORM:    fastprint( 3, 4,"City        :",Colors[DISPLAY]);
                       fastprint( 3, 5,"B/D Phone   :",Colors[DISPLAY]);
                       fastprint( 3, 6,"H/V Phone   :",Colors[DISPLAY]);
                       fastprint(46, 3,"Alt-A  Add a new user"            ,Colors[DISPLAY]);
                       fastprint(46, 4,"Alt-F  Find a user name"          ,Colors[DISPLAY]);
                       fastprint(46, 5,"Alt-S  Search for any text"       ,Colors[DISPLAY]);
                       fastprint(46, 6,"Alt-L  Locate Security Level"     ,Colors[DISPLAY]);
                       fastprint(46, 7,"Alt-E  Locate Exp. Sec Level"     ,Colors[DISPLAY]);
                       fastprint(46, 8,"Alt-O  Locate Deleted Users"      ,Colors[DISPLAY]);
                       fastprint(46, 9,"Alt-R  Repeat last search"        ,Colors[DISPLAY]);
                       fastprint(46,10,"Alt-J  Jump to Record Number"     ,Colors[DISPLAY]);
                       fastprint(46,11,"Alt-T  Jump to the Top record"    ,Colors[DISPLAY]);
                       fastprint(46,12,"Alt-B  Jump to the Bottom record" ,Colors[DISPLAY]);
                       fastprint(46,13,"Alt-P  Print the current record"  ,Colors[DISPLAY]);
                       fastprint(46,14,"Alt-D  Delete the current record" ,Colors[DISPLAY]);
                       fastprint(15,21," Press  F2/F3  to rotate between different views ",Colors[DISPLAY]);
                       break;
    case MSGSFORM:     fastprint( 3,CNFSCRNTOP-2, "Num   Conference     Flags  Last Msg   Num   Conference     Flags  Last Msg",Colors[DISPLAY]);
                       fastprint( 2,CNFSCRNTOP-1,"컴컴� 컴컴컴컴컴컴�   컴컴�  컴컴컴컴  컴컴� 컴컴컴컴컴컴�   컴컴�  컴컴컴컴",Colors[DISPLAY]);
                       break;
    case FIDOFORM:     fastprint( 3, 5,"Passwords",Colors[DISPLAY]);
                       break;
    case QWKNETFORM:
    case NOTESFORM:
    case STATSFORM:
    case VERIFYFORM:
    case PASSWORDFORM:
    case ADDRESSFORM:
    case ALIASFORM:    break;
    case ACCOUNTFORM:  fastprint( 3, 6,"Calculated Balance"  ,Colors[QUESTION]);
                       fastprint(27, 6,":"                   ,Colors[DISPLAY]);
                       fastprint( 3, 9,"Accumulated Charges" ,Colors[DISPLAY]);
                       fastprint(43, 9,"Accumulated Earnings",Colors[DISPLAY]);
                       fastprint(43,15,"ALT-C: Clear Accumulated Values",Colors[DISPLAY]);
                       break;
  }
}


static int near pascal getconfinfo(unsigned StartMsgs) {
  FldType  *Conf;
  unsigned Start;
  unsigned Stop;
  unsigned X;
  unsigned Y;
  unsigned NumFields;

  Start = 0;

top:
  if ((Conf = (FldType *) mallochk((CNFSCRNLEN*4) * sizeof(FldType))) == NULL)
    return(0);
  Stop = min(PcbData.NumAreas,Start+(CNFSCRNLEN * 2));
  NumFields = (Stop-Start)*2;
  farmemcpy(LastMsgRead,&MsgReadPtr[Start],(Stop-Start)*sizeof(long));
  initcnffields(Conf,Start);
  showname();
  if (StartMsgs > NumFields - 1)
    StartMsgs = NumFields - 1;
  StartMsgs = readscrn(Conf,NumFields-1,StartMsgs,"Edit User Record (Conferences)","",1,NOCLEARFLD);  //lint !e534
  DataChanged |= ScreenInputChanged;
  freescrn(Conf,NumFields-1);

  farmemcpy(&MsgReadPtr[Start],LastMsgRead,(Stop-Start)*sizeof(long));
  for (X = 0, Y = Start; Y < Stop; X++, Y++) {
    if (strchr(Flags[X],'L') != NULL) {
      unsetbit(&ConfReg[REG],Y);
      setbit(&ConfReg[EXP],Y);
    } else {
      if (strchr(Flags[X],'R') != NULL) setbit(&ConfReg[REG],Y); else unsetbit(&ConfReg[REG],Y);
      if (strchr(Flags[X],'X') != NULL) setbit(&ConfReg[EXP],Y); else unsetbit(&ConfReg[EXP],Y);
    }
    if (strchr(Flags[X],'S') != NULL) setbit(&ConfReg[USR],Y); else unsetbit(&ConfReg[USR],Y);
    if (strchr(Flags[X],'C') != NULL) setbit(&ConfReg[CON],Y); else unsetbit(&ConfReg[CON],Y);
    if (QwkSupport)
      QwkConfFlags[Y] = (strchr(Flags[X],'N') != NULL ? 1 : 0);
  }

  #ifndef DEMO
    if (KeyFlags == PGDN && Start+(CNFSCRNLEN*2) <= PcbData.NumConf) {
      Start += CNFSCRNLEN*2;
      clsbox(2,CNFSCRNTOP,77,CNFSCRNTOP+CNFSCRNLEN,Colors[QUESTION]);
      goto top;
    }
    if (KeyFlags == PGUP && Start != 0) {
      Start -= CNFSCRNLEN*2;
      clsbox(2,CNFSCRNTOP,77,CNFSCRNTOP+CNFSCRNLEN,Colors[QUESTION]);
      goto top;
    }
  #endif

  return(StartMsgs);
}


static void near pascal getbitflags(void) {
  MsgClear = UsersData.PackedFlags.MsgClear;
  FSEDefault = (UsersData.PackedFlags.DontAskFSE ? (UsersData.PackedFlags.FSEDefault ? 'Y' : 'N') : 'A');
  ScrollMsgs  = UsersData.PackedFlags.ScrollMsgBody;
  LongHeaders = ! UsersData.PackedFlags.ShortHeader;
  WideEditor  = UsersData.PackedFlags.WideEditor;
  ShortDesc   = UsersData.Flags.SingleLines;
  ChatStatus  = (UsersData.Flags.UnAvailable ? 'U' : 'A');
}

static void near pascal setbitflags(void) {
  UsersData.PackedFlags.MsgClear      = MsgClear;
  UsersData.PackedFlags.DontAskFSE    = (FSEDefault != 'A');
  UsersData.PackedFlags.FSEDefault    = (FSEDefault == 'Y');
  UsersData.PackedFlags.ScrollMsgBody = ScrollMsgs;
  UsersData.PackedFlags.ShortHeader   = ! LongHeaders;
  UsersData.PackedFlags.WideEditor    = WideEditor;
  UsersData.Flags.SingleLines         = ShortDesc;
  UsersData.Flags.UnAvailable         = (ChatStatus == 'U');
}


/********************************************************************
*
*  Function: edit()
*
*  Desc    : Used to edit the Users File.  It first allocates memory for the
*            fields on screen and then displays and allows the user to edit
*            the user records.  Control of access to the users file, whether
*            moving forward/backward or searching for a user is done thru
*            cursor key and alt-key sequences.
*
*/

void pascal edituser(long UserNum) {
  char Scrn;
  bool NewUser;
  bool OldUpdateBoxStatus;
  bool CheckAlias;
  char First;
  char Second;
  int  AddResponse;
  int  StartShort;
  int  StartFido;
  int  StartLong;
  int  StartMsgs;
  long TotRec;
  char   SaveDropSec;
  double SaveNewBalance;
  char RecStr[24];

  if (openusersfile() == -1)
    return;

  if (openusersinffile(PcbData.InfFile) == -1) {
    dosclose(UsersFile);
    return;
  }

  initquest(LongForm    ,NUMLONGFIELDS);
  initquest(ShortForm   ,NUMSHORTFIELDS);
  initquest(FidoForm    ,NUMFIDOFIELDS);
  initquest(AliasForm   ,NUMALIASFIELDS);
  initquest(VerifyForm  ,NUMVERIFYFIELDS);
  initquest(AddressForm ,NUMADDRESSFIELDS);
  initquest(StatsForm   ,NUMSTATSFIELDS);
  initquest(AccountForm ,NUMACCOUNTFIELDS);
  initquest(QwkNetForm  ,NUMQWKNETFIELDS);
  initquest(NotesForm   ,NUMNOTESFIELDS);
  initquest(PasswordForm,NUMPASSWORDFIELDS);

  ExitKeyNum[ 0] = 33;  ExitKeyFlag[ 0] = ALTF;   /*  alt-f  */
  ExitKeyNum[ 1] = 31;  ExitKeyFlag[ 1] = ALTS;   /*  alt-s  */
  ExitKeyNum[ 2] = 19;  ExitKeyFlag[ 2] = ALTR;   /*  alt-r  */
  ExitKeyNum[ 3] = 20;  ExitKeyFlag[ 3] = ALTT;   /*  alt-t  */
  ExitKeyNum[ 4] = 48;  ExitKeyFlag[ 4] = ALTB;   /*  alt-b  */
  ExitKeyNum[ 5] = 25;  ExitKeyFlag[ 5] = ALTP;   /*  alt-p  */
  ExitKeyNum[ 6] = 60;  ExitKeyFlag[ 6] = F2;     /*  f2     */
  ExitKeyNum[ 7] = 32;  ExitKeyFlag[ 7] = ALTD;   /*  alt-d  */
  ExitKeyNum[ 8] = 30;  ExitKeyFlag[ 8] = ALTA;   /*  alt-a  */
  ExitKeyNum[ 9] = 38;  ExitKeyFlag[ 9] = ALTL;   /*  alt-l  */
  ExitKeyNum[10] = 18;  ExitKeyFlag[10] = ALTE;   /*  alt-e  */
  ExitKeyNum[11] = 24;  ExitKeyFlag[11] = ALTO;   /*  alt-o  */
  ExitKeyNum[12] = 36;  ExitKeyFlag[12] = ALTJ;   /*  alt-j  */
  ExitKeyNum[13] = 61;  ExitKeyFlag[13] = F3;     /*  f3     */
  ExitKeyNum[14] = 46;  ExitKeyFlag[14] = ALTC;   /*  alt-c  */

  SaveSrchName[0] = SrchName[0] = 0;
  NeedToReindex = FALSE;
  memset(NeedToIndex,FALSE,26);

  TotRec = numrecs(UsersFile,sizeof(URead));

  if (UserNum == 0 || UserNum > TotRec)
    RecNum = 1;
  else
    RecNum = UserNum;

  StartShort =
  StartFido  =
  StartLong  =
  StartMsgs  = 0;

  CheckAlias = FALSE;
  Scrn       = SHORTFORM;
  NewUser    = FALSE;
  makeeditscrn(Scrn);

  AddResponse = ABORTADD;
  KeyFlags    = NOTHING;

  while (RecNum >= 1) {
    if (KeyFlags != F2 && KeyFlags != F3) {
      if (! NewUser) {
        readusersfile(RecNum);  //lint !e534
        convertreadtodata();
        DataChanged = FALSE;

        if (AliasSupport && CheckAlias) {
          stripright(SrchName,' ');
          if (strcmp(SrchName,UsersData.Name) != 0 && strcmp(SrchName,Alias) == 0) {
            Scrn = ALIASFORM;
            makeeditscrn(Scrn);
          }
          CheckAlias = FALSE;
        }

        #ifdef DEMO
          if (UsersFileModified)
            writeusersfile(RecNum,FALSE,FALSE);
        #endif

        getbitflags();

        sprintf(RecStr,"Record %6ld of %6ld",RecNum,TotRec);
      } else {
        if (AddResponse != DUPLICATE) {
          cleardata();
          getbitflags();
        }
        strcpy(RecStr,"Adding a New Record");
        clsbox(1,23,78,23,Colors[DESC]);
        fastcenter(23,"press PGDN to save or ESC to exit",Colors[DESC]);
      }
    }

    scale(RecNum,TotRec-1);
    fastprint(56, 1,RecStr,Colors[DISPLAY]);
    OldUpdateBoxStatus = UpdateBox;
    UpdateBox = FALSE;
    stripright(Alias,' ');
    stripright((char *)Verify,' ');
    strcpy(OriginalAlias,Alias);

top:
    IsFidoUser = (memcmp(UsersData.Name,"~FIDO~",6) == 0);

    switch (Scrn) {
      case SHORTFORM:    if (IsFidoUser) {
                           KeyFlags = F2;
                           break;
                         }
                         showname();
                         clsbox(17,4,42,6,Colors[DISPLAY]);
                         fastprint(17, 4,UsersData.City          ,Colors[DISPLAY]);
                         fastprint(17, 5,UsersData.BusDataPhone  ,Colors[DISPLAY]);
                         fastprint(17, 6,UsersData.HomeVoicePhone,Colors[DISPLAY]);
                         StartShort = readscrn(ShortForm,NUMSHORTFIELDS-1,StartShort,"Edit User Record (Short Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case FIDOFORM:     if (! IsFidoUser) {
                           KeyFlags = F2;
                           break;
                         }
                         showname();
                         StartFido = readscrn(FidoForm,NUMFIDOFIELDS-1,StartFido,"Edit User Record (Fido Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case LONGFORM:     stripright(UsersData.Name,' ');
                         strcpy(OriginalName,UsersData.Name);
                         StartLong = readscrn(LongForm,NUMLONGFIELDS-1,StartLong,"Edit User Record (Long Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case MSGSFORM:     StartMsgs = getconfinfo(StartMsgs);
                         break;
      case ALIASFORM:    showname();
                         readscrn(AliasForm,NUMALIASFIELDS-1,0,"Edit User Record (Alias Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case VERIFYFORM:   showname();
                         readscrn(VerifyForm,NUMVERIFYFIELDS-1,0,"Edit User (Verification Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case ADDRESSFORM:  showname();
                         readscrn(AddressForm,NUMADDRESSFIELDS-1,0,"Edit User Record (Address Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case PASSWORDFORM: showname();
                         readscrn(PasswordForm,NUMPASSWORDFIELDS-1,0,"Edit User Record (Password Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case STATSFORM:    showname();
                         readscrn(StatsForm,NUMSTATSFIELDS-1,0,"Edit User Record (Statistics Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case NOTESFORM:    showname();
                         readscrn(NotesForm,NUMNOTESFIELDS-1,0,"Edit User Record (Caller Notes)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case ACCOUNTFORM:  showname();
                         calculatebalance(0);  //lint !e534
                         readscrn(AccountForm,NUMACCOUNTFIELDS-1,0,"Edit User Record (Account Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
      case QWKNETFORM:   showname();
                         readscrn(QwkNetForm,NUMQWKNETFIELDS-1,0,"Edit User Record (QWK/Net Form)","",1,NOCLEARFLD);  //lint !e534
                         DataChanged |= ScreenInputChanged;
                         break;
    }
    UpdateBox = OldUpdateBoxStatus;

    if (KeyFlags != F2 && KeyFlags != F3) {
      if (! NewUser) {
        setbitflags();
        if (convertdatatoread(&First,&Second) || DataChanged)
          verifywrite(First,Second);
      } else {
        /* if (KeyFlags != ESC) { */
          AddResponse = verifyadd(&TotRec);
          switch (AddResponse) {
            case OKAYTOADD : RecNum  = TotRec;  /* no "break" intentional */
            case ABORTADD  : NewUser = FALSE;
          }
        /* } else NewUser = FALSE; */
        KeyFlags = 0;
        fastprint(1,23,"  ESC=Exit    PgDn=Forw    PgUp=Back    Ctrl-PgDn=Forw20    Ctrl-PgUp=Back20  ",Colors[DESC]);
      }
    }

    switch (KeyFlags) {
      case ESC      : RecNum = -1; break;
      case DN       :
      case PGDN     : RecNum++;     if (RecNum > TotRec) RecNum = 1;      break;
      case PGUP     : RecNum--;     if (RecNum < 1)      RecNum = TotRec; break;
      case CTRLPGDN : RecNum += 20; if (RecNum > TotRec) RecNum = TotRec; break;
      case CTRLPGUP : RecNum -= 20; if (RecNum < 1)      RecNum = 1;      break;
      case ALTF     : CheckAlias = findspecific(SEARCHNAM);  break;
      case ALTS     :              findspecific(SEARCHSTR);  break;  //lint !e534
      case ALTR     : CheckAlias = findwhichever();          break;
      case ALTT     : RecNum = 1;               break;
      case ALTJ     : RecNum = jumprec(RecNum); break;
      case ALTB     : RecNum = numrecs(UsersFile,sizeof(URead)); break;
      case ALTP     : printrec(FALSE); dosflush(&prn); break;        //lint !e534
      case F2       : Scrn++;
                      if (Scrn == FIDOFORM && ! IsFidoUser)
                        Scrn++;
                      if (Scrn == ALIASFORM && (IsFidoUser || ! AliasSupport))
                        Scrn++;
                      if (Scrn == ADDRESSFORM && (IsFidoUser || ! AddressSupport))
                        Scrn++;
                      if (Scrn == PASSWORDFORM && (IsFidoUser || ! PasswordSupport))
                        Scrn++;
                      if (Scrn == VERIFYFORM && (IsFidoUser || ! VerifySupport))
                        Scrn++;
                      if (Scrn == STATSFORM && (IsFidoUser || ! StatsSupport))
                        Scrn++;
                      if (Scrn == NOTESFORM && ! NotesSupport)
                        Scrn++;
                      if (Scrn == ACCOUNTFORM && (IsFidoUser || ! AccountSupport))
                        Scrn++;
                      if (Scrn == QWKNETFORM && (IsFidoUser || ! QwkSupport))
                        Scrn++;
                      if (Scrn >= NOMOREFORMS)
                        Scrn = (IsFidoUser ? FIDOFORM : SHORTFORM);
                      makeeditscrn(Scrn);
                      break;
      reevaluate:
      case F3       : Scrn--;
                      if (Scrn >= NOMOREFORMS)
                        Scrn = QWKNETFORM;
                      if (Scrn == QWKNETFORM && (IsFidoUser || ! QwkSupport))
                        Scrn--;
                      if (Scrn == ACCOUNTFORM && (IsFidoUser || ! AccountSupport))
                        Scrn--;
                      if (Scrn == NOTESFORM && ! NotesSupport)
                        Scrn--;
                      if (Scrn == STATSFORM && (IsFidoUser || ! StatsSupport))
                        Scrn--;
                      if (Scrn == VERIFYFORM && (IsFidoUser || ! VerifySupport))
                        Scrn--;
                      if (Scrn == PASSWORDFORM && (IsFidoUser || ! PasswordSupport))
                        Scrn--;
                      if (Scrn == ADDRESSFORM && (IsFidoUser || ! AddressSupport))
                        Scrn--;
                      if (Scrn == ALIASFORM && (IsFidoUser || ! AliasSupport))
                        Scrn--;
                      if (Scrn == FIDOFORM && ! IsFidoUser)
                        Scrn--;
                      if (Scrn == SHORTFORM && IsFidoUser) {
                        Scrn = NOMOREFORMS;
                        goto reevaluate;
                      }

                      makeeditscrn(Scrn);
                      break;
      case ALTD     : verifydelete();  break;
      case ALTA     :
                      #ifdef DEMO
                        if (numrecs(UsersFile,sizeof(URead)) >= 10)
                          break;
                      #endif
                      NewUser = TRUE; Scrn = LONGFORM; StartLong = 0;
                      AddResponse = OKAYTOADD;
                      makeeditscrn(LONGFORM);
                      break;
      case ALTL     : findspecific(SEARCHSEC); break;      //lint !e534
      case ALTE     : findspecific(SEARCHEXP); break;      //lint !e534
      case ALTO     : findspecific(SEARCHDEL); break;      //lint !e534
      case ALTC     : if (Scrn == ACCOUNTFORM) {
                        SaveNewBalance = Account.StartingBalance;
                        SaveDropSec = Account.DropSecLevel;
                        memset(&Account,0,sizeof(Account));
                        Account.StartingBalance = SaveNewBalance;
                        Account.DropSecLevel = SaveDropSec;
                        DataChanged = TRUE;
                        goto top;
                      }
                      break;
    }
  }

  memset(ExitKeyNum,0,15);

  dosfclose(&DosUsersInfFile);
  dosclose(UsersFile);

  freeanswers(ShortForm   ,NUMSHORTFIELDS);
  freeanswers(FidoForm    ,NUMFIDOFIELDS);
  freeanswers(LongForm    ,NUMLONGFIELDS);
  freeanswers(AliasForm   ,NUMALIASFIELDS);
  freeanswers(VerifyForm  ,NUMVERIFYFIELDS);
  freeanswers(AddressForm ,NUMADDRESSFIELDS);
  freeanswers(StatsForm   ,NUMSTATSFIELDS);
  freeanswers(NotesForm   ,NUMNOTESFIELDS);
  freeanswers(PasswordForm,NUMPASSWORDFIELDS);
  freeanswers(AccountForm ,NUMACCOUNTFIELDS);
  freeanswers(QwkNetForm  ,NUMQWKNETFIELDS);

  if (NeedToReindex)
    createindexfiles();
}


void pascal edit(void) {
  edituser(1);
}


void pascal editusername(char *Name) {
  char *p;
  long  UserNum;
  char  TempName[26];

  maxstrcpy(TempName,Name,sizeof(TempName));
  // change semicolons to spaces
  for (p = TempName; (p = strchr(p,';')) != NULL; ) {
    *p = ' ';
  }
  UserNum = finduser(TempName);
  if (UserNum != -1)
    edituser(UserNum);
}
