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


struct pcbdattype {
  char Version[51];
  char Sysop[13];
  char Password[13];
  bool Graphics;
  char SysopSec[12];

  char MsgFile[9];
  char ClrFile[7];
  char ClrLoc[31];
  char WlcFile[8];
  char NewFile[8];
  char UsrFile[9];
  char CnfFile[13];
  char FscFile[13];
  char PwdFile[13];
  char TcnFile[13];
  char RemFile[9];
  char AnsFile[7];
  char RegFile[9];
  char DrsFile[9];
  char NwsFile[8];
  char NetFile[41];
  char DldFile[41];

  char UpldDrive[26];
  char GnrlDrive[26];
  char MainDrive[26];
  char HelpDrive[26];
  char UdirDrive[26];
  char ChatDrive[26];
  char DownDrive[255];

  char Seconds;
  char ModemInit[41];
  char ModemOff[31];
  char ModemPort[6];
  int  ModemSpeed;
  bool Modem300;
  char Modem300Start[6];
  char Modem300Stop[6];

  char UserLevels[30];

  char NumBull;
  char NumConf;
  char NumMainDirs;
  bool EnforceTime;
  bool MainPriv;
  bool DenyUpdate;
  bool ClosedBoard;
  bool NonGraphics;
  char PubConf[10];
  bool ExitToDos;
  bool EventActive;
  char EventTime[6];
  char MaxMsgLines;
  char DefaultIntensity[3];
  char DefaultColor;
  bool Network;
  char NodeNum;
  bool DisableDriveCheck;

  char FunctionKeys[10][58];
  char Comment[60];
  char BoardName[65];
  char ParallelPortNum;
  bool LastReadUpdate;
  char MessageBlocks;
  bool AllowEscCodes;
  bool AllowCCs;
  bool Validate;

  bool DisableCls;
  bool SlowModem;
  bool PerformReset;
  char UploadBufSize;
  bool DisableRegEdits;
  bool AnswerTrueRing;
  bool DisableKeyTimeout;
  bool IncludeUserCity;
  bool EliminateSnow;
  bool DisableFilter;
  bool DisableCTS;
  bool SlideEvent;
  int  StopFreeSpace;
  bool DisableQuickLogon;
  bool DisablePassword;
  char NetTimeout;
  bool MainMsgPrivate;
  bool SubscriptionMode;
  char UserPrompt[28];
};


struct pcbconftype {
  char ConfName[9];
  char DownPath[161];
  char ConfBull;
  char ConfDirs;
  bool PrivateUploads;
  bool AutoRejoin;
  char UploadPath[26];
  char ConfFiles[26];
  char ConfAddSec;
  char ConfAddTime;
  bool ViewMembers;
  bool MiniBBS;
  char MaxBlocks;
  bool KeepUploads;
  bool PrivateMsgs;
};

extern struct pcbdattype  PcbData;
extern struct pcbconftype PcbConf[9];
