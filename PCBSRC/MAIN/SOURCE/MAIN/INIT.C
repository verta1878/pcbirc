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

// bool Sound;   // remove this switch after FOSSIL test is done


#include "system.h"
#include "account.h"
#include "messages.h"
#include "menu.h"
#include "event.h"
#include "serial.h"

#ifdef __OS2__
  #define  INCL_DOSPROCESS
  #define  INCL_DOSMISC
  #include <os2.h>
  #include <kbd.hpp>
  #include "sem.hpp"
#endif

#ifdef FIDO
  #include "defines.h"
  #include "structs.h"
//#include <prototyp.h>
  #include "tossmisc.h"
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <io.h>
  #include <borland.h>
  extern int _argc;
  extern char **_argv;
#endif

char *DatFile;
pcbdattype _FARDATA_ PcbData;

#ifdef __OS2__
  CTimeoutSemaphore WaitKeySem;
  CSemaphore        KbdTimer;
#else
  uint far *KbdStatus;
#endif

colortype static ScreenColors[2] = {{0x07,0x70,0x70,0x07,0x00,0x07,0x70,0x07,0x07,0x07,0x07}, {0x1F,0x70,0x70,0x4F,0x10,0x17,0x30,0x1E,0x4E,0x1E,0x1C}};
colortype *ScrnColors;

countrytype SystemCountry;

char pcbcolors[8] = { 0,12,10,14,9,13,11,15 };

char mask_command[]    = {15,'!',0,'#','+','-',0,'.',':',0,'<',']','_','|','á','~'};
char mask_blts[]       = {11,'A','D','G','H','L','N','R','S',0,'0','9'};
char mask_dirs[]       = {13,'A','D','H','L','N','P','S','U','V','-',0,'0','9'};
char mask_alphanum[]   = { 3,0,' ','~'};
char mask_systemyesno[]= { 2,'Y','N'};
char mask_yesno[]      = { 0 };
char mask_graphics[]   = { 5,'N','Q','R','S',141};
char mask_scan[]       = { 9,'A','C','L','Q','S','W','Z','+','-'};
char mask_more[]       = { 3,'N','S','G'};
char mask_editor[]     = { 1,'U'};
char mask_any[]        = { 1,'A'};
char mask_join[]       = {14,'$','#','!','&','-','+','.',' ',0,'0','9',0,'A','z'};
char mask_morexfer[]   = {15,'!',0,'#','*',' ','-','.',0,'0',':',0,'?','z','á','~'};
char mask_filename[]   = {14,'!',0,'#',')','-','.',0,'0',':',0,'@','z','á','~'};
char mask_wildname[]   = {16,'!',0,'#','*',' ',')','-','.',0,'0',':',0,'?','z','á','~'};
char mask_atoz[]       = { 3,0,'A','Z'};
char mask_numbers[]    = { 3,0,'0','9'};
char mask_channel[]    = { 5,0,'0','9', 'L','Q'};
char mask_cnfnumbers[] = { 7,0,'0','9','-','D','Q','S'};
char mask_nodeview[]   = { 6,'A','N','S',0,'0','9'};
char mask_viewlog[]    = { 4,'V','P','S','D'};
char mask_message[]    = { 6,0,' ','~', 0,128,254};
char mask_cr[]         = { 2,'C','R'};
char mask_crs[]        = { 3,'C','R','S'};
char mask_crsu[]       = { 4,'C','R','S','U'};
char mask_gnrsd[]      = { 5,'G','N','R','S','D'};
char mask_editmsg[]    = {17,0,'0','9','A','C','D','E','F','H','I','K','L','N','Q','S','U','V'};
char mask_fnprst[]     = { 6,'F','N','P','R','S','T'};
char mask_efnprst[]    = { 7,'E','F','N','P','R','S','T'};
char mask_msgread[]    = { 6,0,'!','9',0,'A','z'};
char mask_node[]       = { 3,'A','G','U'};
char mask_pv[]         = { 2,'P','V'};
char mask_usrmaint[]   = {14,0,'0','9','A','C','D','F','L','P','Q','S','U','-','+'};
char mask_conf[]       = { 2,' ','X'};
char mask_phone1[]     = { 6,0,'0','9','(',')','-'};
char mask_phone2[]     = { 7,0,'0','9','(',')','-',' '};
char mask_msgrange[]   = { 7,0,'0','9','+','-','S','L'};
char mask_editlist[]   = { 3,'A','R','L'};
char mask_referlist[]  = { 4,'Q',0,'0','9'};
char mask_byeabort[]   = { 2,'A','G'};
char mask_name[]       = {11,0,'A','Z',0,'a','z',' ','.',',','-','\''};
char mask_crsxn[]      = { 6,'C','L','R','S','X','N'};

char YesChar;
char NoChar;
char SystemYesChar;
char SystemNoChar;
bool AddTabs;
char ComSpec[66];
#ifdef __OS2__
char DosComSpec[66];
char DosSettingsFile[128];
#endif
char TestBat[66];
char QwkBat[66];
char CompressBat[66];
char DszLog[66];
bool CtrlBreak;
bool UseAnsi;
bool HasRip;
bool SkipRipDetect;
bool NoKeyWait;
bool LocalOn;
bool FrontEnd;
bool NovellSearch;
bool OldIndex;
long ConnectSpeedOverride;
bool NoEnv;
char LineSeparator;
bool DisableAnswer;
bool NoCmdLst;
bool NoPPE;
bool NoMNU;
bool NoLanguage;
bool NoGraphics;
bool NoSecurity;
bool NoConfReg;
bool ISDN;
bool MixedCase;
#ifdef BIGNDX
bool BigNdx;
#endif
#ifdef FIDO
bool FidoMailHour = FALSE;   // don't initialize via init() because this variable needs to maintain it's value across a recycle
bool EnableMSG = FALSE;
bool NoEmsi = FALSE;
bool NoWazoo = FALSE;
bool NoFTS1 = FALSE;
bool EnableDLO = FALSE;
bool NoEchoFlag = FALSE;
#endif
bool IgnoreViewSec;
char ReadOnlyDrives[27];
bool ProtectAlias;
bool AnswerOnStartup;

static bool NoRip;

#ifdef __OS2__
  int  ComHandle = 0;
#else
  char NO16550;
  char Allow16650;
  char FORCE16550A;
#endif

char VerifyCDLoss = 1;

static char MaxNumScrnLines;
static char StartNumScrnLines;
static bool DisableFido;

extern struct ffblk DTA;  /* declared in exist.c */

#ifndef PCB_DEMO
static int  NumNodes;
static int  Version;
#endif

int KEY;

int LIBENTRY version (int x, int y) {
    int z = x + y;
    z++;
    return 0;
}

#ifdef KBD3
#ifdef __OS2__
  #define _INTERRUPT_
  #define _USERENTRY_ _USERENTRY
#else
  #define _INTERRUPT_ interrupt
  #define _USERENTRY_ _Cdecl
#endif

static void _USERENTRY_ explain(void) {
  int   Date1;
  int   Date2;
  char *p;
  char  Temp1[16];
  char  Temp2[16];
  char  Path1[66];
  char  Path2[66];
  char  Temp[66];
  char  FullPath1[66];
  char  FullPath2[66];
  char  Str[256];

  strcpy(Path1,_argv[0]);
  strcpy(Path2,"PCBOARD.SER");

  if (srchpath(Path2) == -1) {
    maxstrcpy(Temp,_argv[0],sizeof(Temp));
    if ((p = strrchr(Temp,'\\')) != NULL) {
      strcpy(p+1,Path2);   // path=serfilename, append it to path of .exe file
      strcpy(Path2,Temp);  // then copy it into path since we use path below
    }
    if (fileexist(Path2) == 255)
      return;
  }

  fileexist(Path1);          //lint !e534
  Date1 = DTA.ff_fdate;
  fullyqualifiedname(FullPath1,Path1,sizeof(FullPath1));

  fileexist(Path2);          //lint !e534
  Date2 = DTA.ff_fdate;
  fullyqualifiedname(FullPath2,Path2,sizeof(FullPath2));

  sprintf(Str,"Executable : %s (date = %s)\r\n"
              "Serial file: %s (date = %s)\r\n",
          FullPath1,dtoc(Date1,Temp1),
          FullPath2,dtoc(Date2,Temp2));
  say(Str);
  mydelay(300);
}

/* CRYPTO: ERROR!  The PCBOARD.SER file is corrupted. */
static char _FARDATA_ CorruptStr[] = {
  0x80, 0x51, 0xA4, 0xA4, 0xB4, 0x0D, 0xBC, 0x05, 0xB6, 0x88, 0xFE, 0x91, 0x94,
  0x99, 0x9D, 0xA4, 0xD4, 0xA1, 0x90, 0x8F, 0xBA, 0x15, 0x98, 0x10, 0x54, 0xC2,
  0x02, 0x04, 0xD0, 0x44, 0x18, 0x58, 0xF4, 0x35, 0x86, 0x47, 0x66, 0xAD, 0x03,
  0xD9, 0x41, 0x3F, 0x41
};

static void _INTERRUPT_ corrupted(...) {
  decrypt(CorruptStr,43);
  atexit(explain);
  errorexittodos(CorruptStr);
}

/* CRYPTO: The PCBOARD.SER file is missing!" */
static char _FARDATA_ MissingStr[] = {
  0x62, 0x5E, 0xA8, 0x1E, 0xB8, 0xCA, 0xF1, 0x22, 0xB9, 0x08, 0xFB, 0xEC, 0x93,
  0x46, 0x96, 0x64, 0xCF, 0xFE, 0xD6, 0xA5, 0x44, 0xA3, 0x04, 0x45, 0x6D, 0x28,
  0x1E, 0x5B, 0x77, 0x35, 0x56, 0x52, 0x56
};

static void _INTERRUPT_ missing(...) {
  decrypt(MissingStr,33);
  errorexittodos(MissingStr);
}

/* CRYPTO: Invalid PCBOARD.SER or wrong OS version of PCBoard! */
static char _FARDATA_ WrongOSStr[] = {
  0xED, 0xF6, 0x21, 0xD8, 0x48, 0xB4, 0x51, 0xBC, 0x41, 0x68, 0x08, 0x80, 0x9A,
  0x8A, 0x7E, 0xC8, 0xD7, 0x6A, 0xC7, 0x43, 0x1B, 0xD8, 0xF5, 0x98, 0x3C, 0x25,
  0x5B, 0x4B, 0x48, 0x83, 0x04, 0x02, 0xAA, 0xCE, 0x63, 0x03, 0xDE, 0xA6, 0x3C,
  0xA0, 0xE2, 0x6C, 0xF6, 0x64, 0x72, 0xE2, 0x84, 0xF4, 0xC8, 0x7A, 0xC8
};

static void _INTERRUPT_ wrongos(...) {
  decrypt(WrongOSStr,36);
  atexit(explain);
  errorexittodos(WrongOSStr);
}

/* CRYPTO: The PCBOARD.SER file does not match the PCBoard version in use. */
static char _FARDATA_ MismatchStr[] = {
  0x62, 0x5E, 0xA8, 0x1E, 0xB8, 0xCA, 0xF1, 0x22, 0xB9, 0x08, 0xFB, 0xEC, 0x93,
  0x46, 0x96, 0x64, 0xCF, 0xFE, 0xD6, 0xA5, 0x94, 0xA3, 0x5E, 0x7D, 0x92, 0xFC,
  0x5F, 0x11, 0xB7, 0x51, 0xD6, 0x3C, 0xCE, 0xE1, 0xC3, 0xE5, 0x2B, 0x35, 0x0B,
  0x50, 0x2A, 0xF8, 0x67, 0x10, 0xA5, 0xF4, 0xBC, 0xFC, 0xDA, 0xAB, 0x3E, 0x4D,
  0xA8, 0xBB, 0xA5, 0x7F, 0x79, 0xAD, 0x2B, 0xAA, 0x85, 0xC6, 0x05, 0xCD
};

static void _INTERRUPT_ mismatch(...) {
  decrypt(MismatchStr,64);
  atexit(explain);
  errorexittodos(MismatchStr);
}
#endif

static char _FARDATA_ PromptStr[128]; // stores the new PROMPT= environment variable
                                      // this must be static or putenv() will fail

short static NodeNum;


int  DebugLevel;
/*   0 = no debug entries in the callers log            */
/*   1 = record comm port settings / shells / ext prots */
/*   2 = record all failed attempts at opening a file   */
/*   3 = record all successfully opened files           */
/*   4 = record all file closings                       */
/*   5 = record information during group chat           */
/*   6 = record memory alloc/free (define DEBUGMEM too) */


#ifdef FIDO
static void _NEAR_ LIBENTRY dofidoswitches(char *Str) {
  char *p;

  strupr(Str);
  PcbData.EnableFido = TRUE;

  if ((p = strstr(Str,"/IMPORT:")) != NULL) {
    PcbData.FidoPktFreq = atoi(p+8);
    PcbData.FidoProcessIn = TRUE;
  }

  if ((p = strstr(Str,"/EXPORT:")) != NULL) {
    PcbData.FidoExportFreq = atoi(p+8);
    PcbData.FidoProcessOut = TRUE;
  }

  if ((p = strstr(Str,"/DIAL:")) != NULL) {
    PcbData.FidoMailFreq = atoi(p+6);
    PcbData.FidoDialOut = TRUE;
  }

  if (strstr(Str,"/MSG") != NULL)
    EnableMSG = TRUE;

  if (strstr(Str,"/NOEMSI") != NULL)
    NoEmsi = TRUE;

  if (strstr(Str,"/NOWAZOO") != NULL)
    NoWazoo = TRUE;

  if (strstr(Str,"/NOFTS1") != NULL)
    NoFTS1 = TRUE;

  if (strstr(Str,"/DLO") != NULL)
    EnableDLO = TRUE;

  if (strstr(Str,"/NOECHOFLAG") != NULL)
    NoEchoFlag = TRUE;
}
#endif


static void _NEAR_ LIBENTRY doswitches(char *Str) {
  char *p;

  strupr(Str);
  stripall(Str,'"');

//// remove this switch after FOSSIL tests are done
//if ((p = strstr(Str,"/SOUND:")) != NULL)
//  Sound = atoi(p+7);
//else
//  Sound = 1;

  if ((p = strstr(Str,"/NODE:")) != NULL)
    NodeNum = (short) atoi(p+6);

  if (strstr(Str,"/NMT") != NULL)
    Novell = TRUE;
  if (strstr(Str,"/NOGIVEUP") != NULL)
    DisableGiveup = TRUE;
  if (strstr(Str,"/TABS") != NULL)
    AddTabs = TRUE;

  if ((p = strstr(Str,"/DEBUG")) != NULL) {
    DebugLevel = 1;
    if (p[6] == ':')
      DebugLevel = atoi(p+7);
  }

  if ((p = strstr(Str,"/CONNECT:")) != NULL)
    ConnectSpeedOverride = atol(p+9);

  if (strstr(Str,"/ISDN") != NULL)
    ISDN = TRUE;

  if (strstr(Str,"/MIXED") != NULL)
    MixedCase = TRUE;

  #ifdef BIGNDX
    if (strstr(Str,"/BIGNDX") != NULL)
      BigNdx = TRUE;
  #endif

  if (strstr(Str,"/COLOR") != NULL)
    Scrn_Mode = VID_COLOR;
  if (strstr(Str,"/MONO") != NULL)
    Scrn_Mode = VID_MONO;

  if (strstr(Str,"/NOVELL") != NULL)
    NovellSearch = TRUE;

  if (strstr(Str,"/OLDINDEX") != NULL)
    OldIndex = TRUE;

  if (strstr(Str,"/NOATA") != NULL)
    DisableAnswer = TRUE;

  if (strstr(Str,"/NOCMD") != NULL)
    NoCmdLst = TRUE;

  if (strstr(Str,"/NOPPE") != NULL)
    NoPPE = TRUE;

  if (strstr(Str,"/NOLANG") != NULL)
    NoLanguage = TRUE;

  if (strstr(Str,"/NOGRAPH") != NULL)
    NoGraphics = TRUE;

  if (strstr(Str,"/NOSEC") != NULL)
    NoSecurity = TRUE;

  if (strstr(Str,"/NOCONF") != NULL)
    NoConfReg = TRUE;

  if (strstr(Str,"/NOMNU") != NULL)
    NoMNU = TRUE;

  #ifdef COMM
    #ifndef __OS2__
      if (strstr(Str,"/16650") != NULL)
        Allow16650 = TRUE;
    #endif

    if (strstr(Str,"/NORIP") != NULL)
      NoRip = TRUE;

    if (strstr(Str,"/NOKEY") != NULL)
      NoKeyWait = TRUE;
  #endif

  if (strstr(Str,"/NOPROTECT") != NULL)
    ProtectAlias = FALSE;

  if ((p = strstr(Str,"/CDROM:")) != NULL)
    maxstrcpy(ReadOnlyDrives,p+7,sizeof(ReadOnlyDrives));

  #ifdef FIDO
    if (strstr(Str,"/NOFIDO") != NULL)
      DisableFido = TRUE;
  #endif

  if (strstr(Str,"/NOVIEWSEC") != NULL)
    IgnoreViewSec = TRUE;

  if ((p = strstr(Str,"/INIT:")) != NULL) {
    p += 6;
    if (strlen(p) < sizeof(PcbData.ModemInit)) {
      strcpy(PcbData.ModemInit,p);
      PcbData.ModemInit2[0] = 0;
    } else {
      char *q;
      char  Save;

      Save = p[sizeof(PcbData.ModemInit)];
      p[sizeof(PcbData.ModemInit)] = 0;

      while (1) {
        if ((q = strrchr(p,'A')) == NULL)
          break;
        if (*(q+1) == 'T')
          break;
      }
      p[sizeof(PcbData.ModemInit)] = Save;
      *q = 0;
      strcpy(PcbData.ModemInit,p);
      *q = 'A';
      maxstrcpy(PcbData.ModemInit2,q,sizeof(PcbData.ModemInit2));
    }
  }
}


static char * _NEAR_ LIBENTRY checkcommandline(char *Search) {
  int  X;
  char *p;

  for (X = 1; X < _argc; X++) {
    strupr(_argv[X]);
    if ((p = strstr(_argv[X],Search)) != NULL)
      return(p);
  }
  return(NULL);
}


#ifdef COMM
static void _NEAR_ LIBENTRY portoverride(char *Str) {
  char *p;
  int  Num;

  if ((p = strstr(Str,"/COM")) != NULL && p[5] == ':') {
    Num = atoi(&p[4]);
    sprintf(PcbData.ModemPort,"COM%d:",Num);
    switch(Num) {
      case 1 : PcbData.IrqNum = 4; PcbData.BaseAddress = 0x3F8; break;
      case 2 : PcbData.IrqNum = 3; PcbData.BaseAddress = 0x2F8; break;
      default: if ((p = strstr(Str,"/IRQ:")) != NULL)
                 PcbData.IrqNum = atoi(p+5);
               if ((p = strstr(Str,"/BASE:")) != NULL)
                 PcbData.BaseAddress = hextoint(p+6);
    }
  } else  {
    #ifdef MULTIPORT
      if (strstr(Str,"/OS") != NULL)
        PcbData.OS2Driver = TRUE;

      if ((p = strstr(Str,"/PORT")) != NULL) {
        sprintf(PcbData.ModemPort,"PORT%d",atoi(p + (p[5] == ':' ? 6 : 5)));
        if (p[strlen(PcbData.ModemPort)+1] == 'F')
          addchar(PcbData.ModemPort,'F');
        PcbData.IrqNum = 0;
        PcbData.BaseAddress = 0;
      }
    #endif
  }

  if ((p = strstr(Str,"/BAUD:")) != NULL)
    PcbData.ModemSpeed = atol(p+6);

  if ((p = strstr(Str,"/PAD")) != NULL)
    PcbData.Packet = TRUE;

  #ifdef __OS2__
    if ((p = strstr(Str,"/HANDLE:")) != NULL)
      ComHandle = atoi(p+8);
  #endif

  #ifndef __OS2__
    // keep this as an environment setting even though it is also in the
    // pcboard.dat file
    if (strstr(Str,"/NO16550") != NULL)
      NO16550 = TRUE;
  #endif
}
#endif

#ifdef __OS2__
void LIBENTRY setdefaultpriority(void) {
  // set to default priority by setting regular class, priority 0
  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0,0);  //lint !e534
}

void LIBENTRY adjustpriority(int PriorityDelta) {
  if (PriorityDelta != 0) {
    // first return to the default priority
    setdefaultpriority();
    // now adjust the priority based on the delta that was passed
    DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,PriorityDelta,0);  //lint !e534
  }
}
#else
#define setdefaultpriority()
#define adjustpriority()
#endif


static void _NEAR_ LIBENTRY initializecontrols(void) {
  pcbtexttype Buf;

  YesChar = 'Y';
  NoChar  = 'N';

  initbmalloc();
  #ifndef PCB_DEMO
//  setcollect(SERIALCRC);
  #endif
  memset(&UsersData,0,sizeof(UsersData));
  memset(&UsersRead,0,sizeof(UsersRead));
  memset(&Asy,0,sizeof(Asy));
  memset(&Status,0,sizeof(Status));
  memset(&Display,0,sizeof(Display));
  memset(&Control,0,sizeof(Control));

  installhandlers();

  initkbdbuffers();
  readdatfile();

  #ifdef __OS2__
    // force pre-loaded pcbtext file in the OS/2 version
    PcbData.FastText = TRUE;
    adjustpriority(PcbData.PriorityNormal);
  #endif

  if (AnswerOnStartup) {
    PcbData.AnswerRing    = TRUE;  // record that we'll be answering immediately
    PcbData.ExitToDos     = TRUE;  // force drop to dos when caller hangs up
    PcbData.NoCarrierExit = TRUE;  // force recycle if no connection is established
  }

  #ifndef PCB_DEMO
    Status.FoundCrc = 0;
  #endif

  LineSeparator = (char) (PcbData.Foreign ? 0x0D : 0xE3);

  if (PcbData.EnableAccounting)
    readaccountrates();           //lint !e534

  #ifdef PCB_DEMO
    PcbData.NumConf  = min(PcbData.NumConf,1);
    PcbData.NumAreas = min(PcbData.NumAreas,2);
  #endif

  if (PcbData.Network) {
    // make sure the USERNET file is defined, otherwise we cannot continue
    if (PcbData.NetFile[0] == 0)
      errorexittodos("Error!  USERNET.XXX filename is blank.");

    // make sure the usernet file exists
    createusernet();

    // then test the SHARE capabilities of the system
    if (! shareloaded())
      if (! testforshareloaded(PcbData.NetFile))
        errorexittodos("ERROR!  System is set up for SHARE access but SHARE is not loaded/working");

    // check for /NODE:## value
    if (NodeNum >= 1 && NodeNum <= PCB_MAXNODES)
      PcbData.NodeNum = NodeNum;

    // make sure node number is in range
    if (PcbData.NodeNum < 1 || PcbData.NodeNum > PCB_MAXNODES)
      NodeNum = 1;

    // check for FLOAT setting, if set, check to see if we're already up,
    // if so get the node number from pcboard.sys, if not, check the usernet
    // file for the first free node number
    if (PcbData.Float) {
      NodeNum = PcbData.NodeNum;
      if ((PcbData.NodeNum = readpcbsysfornodenum()) == 0) {
        if ((PcbData.NodeNum = firstfreenode(NodeNum)) == 0) {
          // we need to open the PCBTEXT file in order to tell the
          // user that the nodes are busy
          PcbData.FastText = FALSE;
          readpcbtextfile("",PROGRAM);     //lint !e534
          getpcbtext(TXT_NODESBUSY,&Buf);
          errorexittodos(Buf.Str);
        }
      }
    }
  } else // else, not set up for network, set node numbers to 0
    PcbData.NodeNum = NodeNum = 0;

  sprintf(Status.Version,"PCBoard (R) v15.4/M %d",PcbData.NodeNum);   /* 15.4: bumped from v15.3/M */
  sprintf(Status.NodeStr," - Node %d",PcbData.NodeNum);

  /* 15.4 Beta release marker â€” identifies this as the 15.4 source port.
   * Clark's original had a 60-day timer that nagged sysops to update.
   * Our version logs the build date/time for traceability instead.
   */
  {
    static char BetaVersion[] = "15.4";
    static char BetaFlag[]    = "Beta";
    static char BetaMsg[]     = "This Beta Released on " __DATE__ " " __TIME__ "!";
    (void) BetaVersion;  /* ensure string is in binary for comparison */
    (void) BetaFlag;
    (void) BetaMsg;
  }

  #ifndef PCB_DEMO
    Status.FoundCrc = 0;
  #endif

  openlog();                      /* open the log now to record all openings */
  loadcnames(FALSE);
  readprotfile();

  #ifndef PCB_DEMO
    Status.FoundCrc = 0;
  #endif

  usersalloc();
  openusersfile();      //lint !e534
  openusersinffile();   //lint !e534

  Status.UseAlias = TRUE;

  #ifndef PCB_DEMO
    Status.FoundCrc = 0;
  #endif
}


#ifndef PCB_DEMO
static void _NEAR_ LIBENTRY createusersysfile(char *Str) {
  int  NumTokens;
  char *p;
  char Pwrd[20];
  char User[80];

  initializecontrols();
  NumTokens = tokenize(Str)-1;    /* minus the /USERSYS part */
  getnexttoken();                 /*lint !e534 eat the /USERSYS part */

  for (User[0] = 0, Pwrd[0] = 0; NumTokens; NumTokens--) {
    p = getnexttoken();
    if (memcmp(p,"PWRD:",5) == 0) {
      maxstrcpy(Pwrd,p+5,sizeof(Pwrd));
    } else if (memcmp(p,"TPA:",4) == 0) {
      maxstrcpy(Status.DoorName,p+4,sizeof(Status.DoorName));
    } else {
      strcat(User,p);
      addchar(User,' ');
    }
  }

  stripright(User,' ');
  if ((Status.UserRecNo = finduser(User)) != -1) {
    Status.JulianLogonDate = getjuliandate();
    getuserrecord(FALSE,FALSE);
    if (strcmp(Pwrd,UsersData.Password) == 0)
      makeusersys();
  }
  exit(0);
}


static void _NEAR_ LIBENTRY setscreen(void) {
  if (LocalOn) {
    Display.NumScrnLines = (char) (MaxNumScrnLines-1);
    Status.StatusLine1   = (char) (MaxNumScrnLines-1);
  } else {
    Display.NumScrnLines = (char) (MaxNumScrnLines-2);
    Status.StatusLine1   = (char) (MaxNumScrnLines-2);
  }
  Status.StatusLine2   = (char) (MaxNumScrnLines-1);
  Display.PageLen      = (char) (Display.NumScrnLines-2);
  setlimits(Display.NumScrnLines);
}


#ifdef PCB152
static void _NEAR_ LIBENTRY showstronconsole(char *Str) {
  #ifdef __OS2__
    puts(Str);
  #else
    doswrite(0,Str,strlen(Str));   //lint !e534
    doswrite(0,"\r\n",2);          //lint !e534
  #endif
}


static void _NEAR_ LIBENTRY runppe(char *Str) {
  int  NumTokens;
  char *p;
  char Pwrd[20];
  char User[80];
  char PPE[128];
  char Params[128];

  initializecontrols();

  User[0] =
  Pwrd[0] =
  PPE[0]  = 0;

  if (memcmp(Str,"/PPE:",5) ==0) {
    maxstrcpy(PPE,Str+5,sizeof(PPE));
    change(PPE,';',' ');
  } else {
    NumTokens = tokenize(Str)-1;    /* minus the /RUNPPE part */
    getnexttoken();                 /*lint !e534 eat the /RUNPPE part */

    for (; NumTokens; NumTokens--) {
      p = getnexttoken();
      if (memcmp(p,"PWRD:",5) == 0) {
        maxstrcpy(Pwrd,p+5,sizeof(Pwrd));
      } else if (memcmp(p,"PPE:",4) == 0) {
        maxstrcpy(PPE,p+4,sizeof(PPE));
      } else if (PPE[0] != 0) {
        addchar(PPE,' ');
        maxstrcpy(PPE+strlen(PPE),p,sizeof(PPE)-strlen(PPE));
      } else {
        strcat(User,p);
        addchar(User,' ');
      }
    }
  }

  if (PPE[0] == 0) {
    showstronconsole("PPE Name is missing");
    exit(99);
  }

  Params[0] = 0;
  if ((p = strchr(PPE,' ')) != NULL) {
    // terminate the PPE name because we found parameters */
    *p = 0;
    maxstrcpy(Params,p+1,sizeof(Params));
  }

  // if the PPE name doesn't have a dot in it, then assume it needs to have
  // the .PPE extension added to it.
  if (strchr(PPE,'.') == NULL)
    strcat(PPE,".PPE");

  // Now that the PPE name is fixed, we can put the parameters back together
  // with the PPE name.
  if (Params[0] != 0) {
    addchar(PPE,' ');
    strcat(PPE,Params);
  }

  stripright(User,' ');
  if (User[0] != 0) {
    if ((Status.UserRecNo = finduser(User)) == -1) {
      showstronconsole("User Name not found");
      exit(99);
    }

    Status.JulianLogonDate = getjuliandate();
    getuserrecord(FALSE,FALSE);
    if (strcmp(Pwrd,UsersData.Password) != 0) {
      showstronconsole("Error in Password");
      exit(99);
    }
  }

  Control.WatchKbdClock     = FALSE;
  Control.WatchSessionClock = FALSE;

  if (! PcbData.MultiLingual || getsystemlanguage() == -1) {
    PcbData.MultiLingual = FALSE;  // in case pcbml.dat could not be read
    readpcbtextfile("",PROGRAM);   //lint !e534
  }

  mask_systemyesno[1] = SystemYesChar = YesChar;
  mask_systemyesno[2] = SystemNoChar  = NoChar;

  LocalOn               = TRUE;
  UseAnsi               = TRUE;
  HasRip                = FALSE;
  SkipRipDetect         = NoRip;
  Control.Screen        = TRUE;
  Control.GraphicsMode  = TRUE;
  Display.ShowOnScreen  = TRUE;
  Display.AbortPrintout = FALSE;
  Display.Break         = TRUE;
  setscreen();
  ScrnColors = &ScreenColors[Scrn_Mode];

  if (Scrn_Rtrc && ! PcbData.EliminateSnow)  /* turn retrace checking back   */
    Scrn_Rtrc = FALSE;                       /* OFF if they don't want it on */

  initdisplay();             /* make sure everything is set up to default on */

  Display.DefaultColor = PcbData.NewDefaultColor;

  startdisplay(FORCECOUNTLINES);
  addtime(1440L * 60L,NEWTIME);

  Asy.Online = LOCAL;
  #ifdef COMM
    Asy.ComPortNumber = 0;
  #endif
  Asy.CarrierSpeed = Asy.ConnectSpeed = Asy.ModemSpeed = PcbData.ModemSpeed;

  getsysopname();
  turndisplayon(FALSE);
  openlog();

  cls();
  agotoxy(0,0);
  asetcolor(7);
  if (Asy.Online == LOCAL)
    turnonscrollback();

  getdisplaynames();    // in case we're in an alias conference

  openusernet();        //lint !e534
  usernetavailable();

  #ifdef __OS2__
    // Control.NetTimer is used with mydelay() so based it on 1/100th of a second
    Control.NetTimer = PcbData.ChatDelay * (ONESECOND/10);
    createusernetthread();
  #else
    Control.NetTimer = (((int) PcbData.ChatDelay) * 182) / 10;
  #endif

  newline();
  redisplaystatusline();

  if (User[0] != 0 && Status.ActStatus != ACT_DISABLED) {
    calculatebalance();
    UsersData.Account.StartThisSession = Status.Balance;
  }

  Status.ConfJoinTime = dosgetlongtime();

  runscriptwithparams(PPE);       //lint !e534
  clsbox(0,24,79,24,0x07);
  #ifdef __OS2__
    updatelinesnow();
  #endif

  #ifdef __OS2__
    puts("");
  #else
    doswrite(0,"\r\n",2);           //lint !e534
  #endif

  Asy.Online = OFFLINE;
  Status.UserRecNo = 0;
// creating a pcboard.sys without having read it is bad because it hasn't read
// in the alarm, page bell settings, etc, so creating a new one wipes out those
// settings.  but reading in a pcboard.sys is probably bad too because it will
// pull in information about a user that may be online, so see if we can just
// avoid the problem by not creating the pcboard.sys file.  DWT 08/05/95
//makepcboardsys();
  #ifdef __OS2__
    destroyusernetthread();
  #endif
  closeusernet();
  mydelay(25);
  exit(0);
}
#endif  /* ifdef PCB152 */
#endif  /* ifndef DEMO  */

#ifdef KBD3
#ifdef __OS2__
  void (LIBENTRY *serialcheck)(void);
#else
  typedef void interrupt (far * _CType intfunctype)(...);
  intfunctype int3;
#endif

static bool int3set = FALSE;

#ifdef __OS2__
static void LIBENTRY donothing(void) {
}

static void LIBENTRY check4(void) {
  char Type1 = 'B';

//setcollect(SERIALOS1);
//Type1 = collect();

  if (Type1 != 'O' && Type1 != 'B')
    wrongos();
  serialcheck = donothing;
}

static void LIBENTRY check3(void) {
  if (Status.ForceScreenOff == 0xF0)
    missing();
  serialcheck = check4;
}

static void LIBENTRY check2(void) {
  if (version(NumNodes,Version))
    mismatch();
  serialcheck = check3;
}

static void LIBENTRY check1(void) {
  if (Status.FoundCrc != 0)    // is CRC bad?  if so set out to fall out
    corrupted();
  serialcheck = check2;
}
#endif  // ifdef __OS2__

void LIBENTRY setint3(void) {
  if (! int3set) {
    #ifdef __OS2__
      serialcheck = check1;
    #else
      int3 = getvect(0x03);
      setvect(0x03,getvect(0x16));
    #endif
    int3set = TRUE;
  }
}

void LIBENTRY restoreint3(void) {
  if (int3set) {
    #ifdef __OS2__
      serialcheck = donothing;
    #else
      setvect(0x03,int3);
    #endif
    int3set = FALSE;
  }
}
#endif  //ifdef KBD3


#ifdef __OS2__
static void LIBENTRY getdoscommand(void) {
  static  char *SysFile = "x:\\CONFIG.SYS";
  int     Drive;
  DOSFILE In;
  char    Str[512];

  DosQuerySysInfo(QSV_BOOT_DRIVE,QSV_BOOT_DRIVE,&Drive,sizeof(Drive));    //lint !e534
  SysFile[0] = Drive + 'A' - 1;

  if (dosfopen(SysFile,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    goto notfound;

  while (dosfgets(Str,sizeof(Str),&In) != -1) {
    if (memicmp(Str,"SHELL=",6) == 0) {
      tokenize(Str);    //lint !e534
      maxstrcpy(DosComSpec,getnexttoken()+6,sizeof(DosComSpec));
      break;
    }
  }

  dosfclose(&In);
  if (fileexist(DosComSpec) != 255)
    return;

notfound:
  maxstrcpy(DosComSpec,"COMMAND.COM",sizeof(DosComSpec));
  srchpath(DosComSpec);      //lint !e534
}
#endif


/********************************************************************
*
*  Function:  findbat()
*
*  Desc    :  Searches for either a .BAT file by checking the current directory
*             first, then searches the path.
*
*             NOTE:  For OS/2, it searches first for a .CMD file and, if
*             not found, it then searches for a .BAT file.
*
*  Returns :  0  if found, with the FoundPath filled in
*             -1 if not found (and the FoundPath is blank)
*/

int LIBENTRY findbat(char *FoundPath, char *SearchName) {
  char *p;

  strcpy(FoundPath,SearchName);
  p = FoundPath + strlen(FoundPath);

  #ifdef __OS2__
    // search first for a .CMD file if this is the OS/2 version
    // but don't return (yet) if the file isn't found
    strcpy(p,".CMD");
    if (srchpath(FoundPath) != -1 && DTA.ff_fsize != 0)
      return(0);
  #endif

  // now search for a .BAT file and return a -1 and empty the filename if
  // the file cannot be found
  strcpy(p,".BAT");
  if (srchpath(FoundPath) == -1 || DTA.ff_fsize == 0) {
    FoundPath[0] = 0;
    return(-1);
  }
  return(0);
}


void LIBENTRY initnontransient(void) {
  char *p;

  registerdosopencb();
  registerdosdupcb();
  registerdoscreatecb();
  registerdosreadcb();
  registerdoswritecb();
  registerdosclosecb();

  #ifdef KBD3
    setint3();
  #endif

  getmode();
  setfont(BIGFONT);
  setlimits(23);

  #ifndef __OS2__
    KbdStatus  = (uint far *) MK_FP(0x40,0x17);
  #endif

  Scrn_24Hour           = TRUE;
  Scrn_DateSeparator    = '-';

  getcountryspecs(0,0);  /* get default country information */
  SystemCountry = Country;

  #ifdef COMM
    LocalOn = FALSE;
  #else
    LocalOn = TRUE;
  #endif

  #ifndef PCB_DEMO
//  openserialfile();
  #endif

  AddTabs              = FALSE;
  FrontEnd             = FALSE;
  NovellSearch         = FALSE;
  OldIndex             = FALSE;
  DisableAnswer        = FALSE;
  NoCmdLst             = FALSE;
  NoPPE                = FALSE;
  NoMNU                = FALSE;
  NoLanguage           = FALSE;
  NoGraphics           = FALSE;
  NoSecurity           = FALSE;
  NoConfReg            = FALSE;
  Novell               = FALSE;
  DisableGiveup        = FALSE;
  DebugLevel           = 0;
  MaxNumScrnLines      = 25;
  ConnectSpeedOverride = 0;
  StartNumScrnLines    = getrows();
  ReadOnlyDrives[0]    = 0;
  ISDN                 = FALSE;
  MixedCase            = FALSE;
  NoRip                = FALSE;
  NoKeyWait            = FALSE;
  ProtectAlias         = TRUE;
  DisableFido          = FALSE;
  IgnoreViewSec        = FALSE;
  AnswerOnStartup      = FALSE;
  #if defined(COMM) && ! defined(__OS2__)
    Allow16650         = FALSE;
  #endif

  #ifdef __OS2__
    ComHandle          = 0;
  #endif

  #ifdef BIGNDX
    BigNdx             = FALSE;
  #endif

  #ifdef __OS2__
    kbdstartthread();   //lint !e534
  #endif

  if (StartNumScrnLines == 0)
    StartNumScrnLines = 25;

  if ((p = checkcommandline("/FILE:")) != NULL)
    DatFile = p+6;
  else
    DatFile = "PCBOARD.DAT";

  if (checkcommandline("/LOCALON") != NULL)
    LocalOn = TRUE;

  if ((p = getenv("PCB")) != NULL)
    doswitches(p);

  if ((p = getenv("PCBANSWER")) != NULL) {
    strupr(p);
    if (*p == 'Y')
      AnswerOnStartup = TRUE;
  }

  if ((p = getenv("COMSPEC")) != NULL)
    maxstrcpy(ComSpec,p,sizeof(ComSpec));
  else {
    #ifdef __OS2__
      strcpy(ComSpec,"CMD.EXE");
    #else
      strcpy(ComSpec,"COMMAND.COM");
    #endif
  }

  #ifdef __OS2__
    getdoscommand();

    // this is a "default" settings file for DOS windows under OS/2
    // inside shell.c it will search for a settings file that is specific to
    // the program being executed and if not found, it will use this default
    // one instead unless this one does not exist in which case no file is used
    maxstrcpy(DosSettingsFile,"PCBDOS.OS2",sizeof(DosSettingsFile));
    if (srchpath(DosSettingsFile) == -1)
      DosSettingsFile[0] = 0;  // blank out if the file does not exist
  #endif

  #ifndef PCB_DEMO
//  readserialfile();

    if ((p = checkcommandline("/READSYS")) != NULL) {
      initializecontrols();
      if (fileexist("USERS.SYS") != 255) {
        getuserrecord(TRUE,TRUE);
        unlink("USERS.SYS");
      }
      exit(0);
    }

    #ifdef PCB152
      if ((p = checkcommandline("/RUNPPE")) != NULL || (p = checkcommandline("/PPE:")) != NULL)
        runppe(p);
    #endif

    if ((p = checkcommandline("/USERSYS")) != NULL)
      createusersysfile(p);
  #endif  /* ifndef DEMO */

  NoEnv = (bool) (checkcommandline("/NOENV") != NULL);

  if (! NoEnv) {
    if (getenv("INPCB") != NULL)
      errorexittodos("PCBoard is already loaded!  Type 'EXIT' to return");

    putenv("INPCB=Y");

    if ((p = getenv("PROMPT")) != NULL) {
      if (strlen(p) < 128-40) {
        buildstr(PromptStr,"PROMPT=Type EXIT to return to PCBoard$_",p,NULL);
        putenv(PromptStr);
      }
    }
  }

  #ifndef PCB_DEMO
//  decryptall();
  #endif

  findbat(TestBat,"PCBTEST");          //lint !e534
  findbat(QwkBat,"PCBQWK");            //lint !e534
  findbat(CompressBat,"PCBCMPRS");     //lint !e534

  if ((p = getenv("DSZLOG")) != NULL) {
    maxstrcpy(DszLog,p,sizeof(DszLog));
    strupr(DszLog);
  } else {
    putenv("DSZLOG=PCBDSZ.LOG");
    strcpy(DszLog,"PCBDSZ.LOG");
  }

  ScrnColors = &ScreenColors[Scrn_Mode];
  #ifndef PCB_DEMO
//  decryptwithoutcrc();
  #endif
}


static void _NEAR_ LIBENTRY seteventup(bool UserOnline) {
  unsigned Num;
  char     *p;

  #ifndef PCB_DEMO
    NumNodes = 0;
  #endif

  Status.EventMinute   = 0;
  Status.SuspendMinute = 0;

  if ((p = checkcommandline("/EVENT:")) != NULL) {
    #ifndef PCB_DEMO
      NumNodes = 0;
    #endif

    if ((Num = atoi(p+7)) > 0) {
      PcbData.EventActive = TRUE;      /* we're going to force an event  */
                                       /* to occur in /EVENT:### minutes */
      if (UserOnline) {
        // set the event time to the callers LOGON TIME plus the number of
        // minutes specified on the command line
        Status.EventMinute = (short) (Status.LogonMinute + Num);
        // have we gone past MIDNIGHT?  If so, adjust the event minute
        if (currentminute() < (unsigned) Status.LogonMinute) {  //lint !e571
          Status.EventMinute -= (short) 1440;
          // an event time of 00:00 MIDNIGHT cannot be detected because it is
          // equal to zero, so we'll "fudge" a little and make it 12:01
          if (Status.EventMinute == 0)
            Status.EventMinute = 1;
        }
      } else
        Status.EventMinute = (short) (currentminute() + Num);

      if (Status.EventMinute > 1440)
        Status.EventMinute -= (short) 1440;

      // in order for the event times to work, we need both a START and an END
      // event time.  The Status.EventMinute calculated above is our START
      // time.  To ensure that the event *will* run, we'll now define an END
      // event time that should cover a period of time that the caller is
      // likely to remain online (via doors or transfers, etc).  This END event
      // time determines when the event is "no longer valid" and should be
      // ignored.  Down below, we will set the END event time to 60 minutes
      // after the START event.  This means that /EVENT:#### is only valid up
      // to a value of 1380.  Anything higher will result in an immediate
      // reduction in a caller's time limit because it would appear that the
      // event time is already upon us.  Anyway, if 60 turns out to be too
      // large of a value it *can* be adjusted, there is no magic in that
      // number, it was only picked to provide a certain level of assurance
      // that the event would run by widening the START/END range.

      Status.EndEventMinute = (short) (Status.EventMinute + 60);
      if (Status.EndEventMinute > 1440)
        Status.EndEventMinute -= (short) 1440;

      Status.SuspendMinute = (short) (Status.EventMinute - PcbData.EventSuspend);
      if (Status.SuspendMinute < 0)    /* did we cross midnight going backwards? */
        Status.SuspendMinute += (short) 1440;  /* if so add 60*24 minutes to the number  */

      if (UserOnline)
        addtime(0,CREDTIME);
    }
  } else {
    #ifndef PCB_DEMO
      NumNodes = 0;
    #endif

    if (PcbData.EventActive)
      checkforevent();        //lint !e534
  }
}


void LIBENTRY setkbdtimer(int Minutes) {
  // calculate and store the keyboard timer value
  //
  // if the value is exactly 1 minute, then add 10 seconds to it - this
  // will avoid the problem of having the kbd timer beep every time a
  // key is pressed (i.e. the "60th second beep" is now 10 seconds away)

  if (Minutes == 0) {
    Control.KbdTimer = 0;
    return;
  }

  #ifdef __OS2__
    Minutes--;  // under OS/2, the timer runs up to the first bell which occurs
                // at the 60-seconds-to-go mark, so subtract one minute.

    // NOTE:  divide by 10 because dosgetlongtime() uses 1/100ths of a second

    Control.KbdTimer = Minutes * (ONEMINUTE/10);

    if (Minutes == 0)                      // if the timeout was only 1 minute
      Control.KbdTimer += (TENSECONDS/10); // then add 10 seconds
  #else
    Control.KbdTimer = (unsigned) ((long) Minutes * 182 * 6);
    if (Minutes == 1)                 // if timeout is 1 minute, then
      Control.KbdTimer += TENSECONDS; //   add 10 seconds to make it 70 seconds
  #endif
}


static void _NEAR_ LIBENTRY removeeventbat(void) {
  bool        Found;
  char        Date[9];
  char        Time[9];
  char        Str[80];
  pcbtexttype Buf;

  Found = FALSE;
  #ifdef __OS2__
    if (fileexist("EVENT.CMD") != 255) {
      Found = TRUE;
      unlink("EVENT.CMD");
    }
  #endif

  if (fileexist("EVENT.BAT") != 255) {
    Found = TRUE;
    unlink("EVENT.BAT");
  }

  if (! Found)
    return;

  getpcbtext(TXT_EVENTFINISHED,&Buf);
  sprintf(Str,"%s%s %s",Buf.Str,datestr(Date),timestr1(Time));

  if (PcbData.Network)
    sprintf(&Str[strlen(Str)]," Node %d",PcbData.NodeNum);

  writelog(Str,SPACERIGHT);
  writelog(LogDivider,LEFTJUSTIFY);
}


static void _NEAR_ LIBENTRY readcolors(void) {
  int File;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  if (PcbData.ColorFile[0] != 0 && (File = dosopen(PcbData.ColorFile,OPEN_READ|OPEN_DENYNONE POS2ERROR)) != -1) {
    doslseek(File,123,SEEK_SET);
    dosread(File,Status.DirColors,sizeof(Status.DirColors) POS2ERROR);   //lint !e534
    if (doslseek(File,0,SEEK_END) < 500)
      Status.DirColors[NEWF] = Status.DirColors[FDSC];
    dosclose(File);
    Status.ColorizeDirFiles = TRUE;
  }
}

void LIBENTRY init(void) {
  #ifdef COMM
  long static Speeds[10] = {300,0,1200,2400,4800,9600,19200,38400L,57600L,115200L};
  long        Bps;
  char        *p;
  #endif
  char        Save;
  pcbconftype Conf;

  Save = Scrn_Mode;      /* for /COLOR or /MONO overrides, remember mode */
  #ifndef __OS2__
    getmode();           /* refresh screen pointers */
  #endif
  Scrn_Mode = Save;      /* restore the color mode  */

  checkmultitaskers();

  getcountryspecs(0,0);  /* get default country information */
  SystemCountry = Country;

  cls();
  agotoxy(0,0);
  asetcolor(7);
  doesccodes();

  #ifndef PCB_DEMO
    // 0xF0 means that the pcboard.ser file is missing
    if (Status.ForceScreenOff == 0xF0) {
      Save = 0x23;
    }
  #endif

  initializecontrols();

  #ifdef FIDO
    if ((p = getenv("PCBFIDO")) != NULL)
      dofidoswitches(p);
  #endif

  UseAnsi              = FALSE;
  HasRip               = FALSE;
  SkipRipDetect        = NoRip;
  Control.Screen       = TRUE;
  Display.ShowOnScreen = TRUE;
  CalledIntoChannel    = 0;
  setscreen();

  Status.EnteringMessage = FALSE;
  Status.FileAttach      = FALSE;

  #ifdef FIDO
    if (DisableFido)
      PcbData.EnableFido     = FALSE;
    Status.AllowHumanCallers = ! FidoMailHour;
    Status.AllowFileRequests = FALSE;
    Status.FidoNextEvent     = 0;
    Status.FidoEndEvent      = 0;
  #endif


  if (Scrn_Rtrc && ! PcbData.EliminateSnow)  /* turn retrace checking back   */
    Scrn_Rtrc = FALSE;                       /* OFF if they don't want it on */

  initdisplay();             /* make sure everything is set up to default on */

  AutoGoodBye   = FALSE;

  YesChar = 'Y';
  NoChar  = 'N';

  // getsystemlanguage will read in pcbtext IF it is set for multilingual
  // if not set to multilingual, or if an error occurs getting the pcbml.dat
  // information, then it will have to read the pcbtext file separately.

  if (! PcbData.MultiLingual || getsystemlanguage() == -1) {
    PcbData.MultiLingual = FALSE;  // in case pcbml.dat could not be read
    readpcbtextfile("",PROGRAM);   //lint !e534
  }

  mask_systemyesno[1] = SystemYesChar = YesChar;
  mask_systemyesno[2] = SystemNoChar  = NoChar;

  openprinter();

  #ifndef PCB_DEMO
//  checkcrc();
  #endif

  if (PcbData.Network) {
    openusernet();           //lint !e534
    initchatfile();
  }

  #ifndef __OS2__
    if (DebugLevel >= 3) {
      sprintf(Status.DisplayText,"PSP Segment: %04X",_psp);
      writedebugrecord(Status.DisplayText);
    }
  #endif

  setpcbenv(NULL,TRUE);  /* set up the PCB environment variables */
  #ifdef __OS2__
  {
    DOSFILE File;

    // create a PCBDOS.BAT file in the current directory which can be called
    // to obtain the current PCBxxx= environment variables.
    if (dosfopen(".\\PCBDOS.BAT",OPEN_RDWR|OPEN_CREATE|OPEN_DENYWRIT,&File) != -1) {
      dosfputs("@echo off\r\n",&File);
      dosfputs("rem This batch file is created by PCBoard for OS/2 to run DOS programs while\r\n",&File);
      dosfputs("rem passing the various PCBoard environment variables to the VDM.\r\n",&File);
      setpcbenv(&File,FALSE);
      dosfputs("pcbtitle.com\r\n",&File);
      dosfputs("call %1 %2 %3 %4 %5 %6 %7 %8 %9\r\n",&File);
      dosfclose(&File);
    }
  }
  #endif

  #if defined(PCBSTATS)
    if (PcbData.Network)
      getstatstype();
  #endif

  setkbdtimer(PcbData.KbdTimeout);
  Display.DefaultColor = PcbData.NewDefaultColor;

  #ifdef __OS2__
    Control.NetTimer = PcbData.ChatDelay * ONESECOND;
    createusernetthread();
  #else
    Control.NetTimer = (((int) PcbData.ChatDelay) * 182) / 10;
  #endif

  Display.Break            = TRUE;
  Asy.Online               = OFFLINE;
  Asy.DataBits             = 8;
  Asy.HstMode              = FALSE;
  Status.SysopFlag         = ' ';
  Status.PackFlag          = ' ';
  Status.ErrorLevel        = EXIT_ERROR; // default to meaning an ERROR occured
  Status.LoggingIn         = NOTNOW;

  #ifndef PCB_DEMO              //  SERIALNODES and seteventup() must
//  setcollect(SERIALNODES);    //  remain in this order
  #endif                        //
                                //
  seteventup(FALSE);            //


  #ifdef KBD3
    #ifdef __OS2__
      serialcheck();
    #else
      if (Status.FoundCrc != 0)   // is CRC bad?  if so set out to fall out
        setvect(0x03,corrupted);
    #endif
  #endif

  #ifdef COMM
    #ifndef __OS2__
    NO16550      = PcbData.No16550;
    FORCE16550A  = PcbData.Force16550A;
    #endif
    VerifyCDLoss = PcbData.VerifyCDLoss;

    if (! LocalOn) {
      if ((p = getenv("PCB")) != NULL)
        portoverride(p);

      #ifdef __OS2__
        PcbData.OS2Driver = TRUE;
      #endif

      if (memcmp(PcbData.ModemPort,"COM",3) == 0) {
        Asy.ComPortNumber = PcbData.ModemPort[3] - '0';
        if (Asy.ComPortNumber > 8)
          Asy.ComPortNumber = 0;
      } else {
        #ifdef MULTIPORT
          if (memcmp(PcbData.ModemPort,"PORT",4) == 0)
            Asy.ComPortNumber = atoi(&PcbData.ModemPort[4]);
          else
            Asy.ComPortNumber = 0;
        #else
          Asy.ComPortNumber = 0;
        #endif
      }

      if (Asy.ComPortNumber != 0 && (p = checkcommandline("/C:")) != NULL) {
        FrontEnd = TRUE;
        if (PcbData.ModemSpeed > 9600)
          PcbData.LockSpeed = TRUE;

        Bps = atol(p+3);
        if (Bps <= 9) {
          switch ((int) Bps) {
            case      0:
            case      2:
            case      3:
            case      4:
            case      5:
            case      6:
            case      7:
            case      8:
            case      9: Asy.CarrierSpeed = Speeds[(int) Bps];
                         break;
            default: errorexittodos("Invalid use of /C: switch");
          }
        } else {
          /* ASSUME that they are providing a VALID speed *ugh* but there */
          /* are just TOO MANY possibilities to predict them all, like    */
          /* 21600, 24400, 28800 and more coming every day!               */
          Asy.CarrierSpeed = Bps;
        }

        if ((p = checkcommandline("/CSTR:")) != NULL) {
          maxstrcpy(Asy.ConnectString,p+6,sizeof(Asy.ConnectString));
          change(Asy.ConnectString,'_',' ');  /* change underlines to spaces */
        } else
          Asy.ConnectString[0] = 0;

        if (checkcommandline("/ERR") != NULL || errorcorrected(Asy.ConnectString))
          Asy.ErrorCorrected = TRUE;
        if (! (Asy.ErrorCorrected || PcbData.LockSpeed))
          Asy.ModemSpeed = Asy.ConnectSpeed = Asy.CarrierSpeed;
        else if (PcbData.LockSpeed || PcbData.ModemSpeed > 9600) {
          Asy.ModemSpeed = PcbData.ModemSpeed;
          Asy.ConnectSpeed = (Asy.ErrorCorrected && Asy.CarrierSpeed >= 9600 ? Asy.ModemSpeed : Asy.CarrierSpeed);
        } else
          Asy.ModemSpeed = Asy.ConnectSpeed = Asy.CarrierSpeed;

        if ((p = checkcommandline("/DTE:")) != NULL) {
          Asy.ModemSpeed = atol(p+5);
          Asy.ConnectSpeed = (Asy.ErrorCorrected && Asy.CarrierSpeed >= 9600 ? Asy.ModemSpeed : Asy.CarrierSpeed);
        }

      }

      if ((p = checkcommandline("/CID:")) != NULL) {
        maxstrcpy(Asy.CallerIDString,p+5,sizeof(Asy.CallerIDString));
        change(Asy.CallerIDString,'_',' ');  /* change underlines to spaces */
      }

      #if defined(__OS2__) && defined(MULTIPORT)
        if (PcbData.OS2Driver && istelnetviavmodem())
          Asy.Telnet = TRUE;
      #endif
    }
  #endif

  #ifndef PCB_DEMO
    Version = 0;
  #endif

  getconfrecord(0,&Conf);
  strcpy(Status.MainMsgFile,Conf.MsgFile);

  #ifndef PCB_DEMO
    Version = 0;
  #endif

  getsysopname();


  #ifdef KBD3
    #ifdef __OS2__
      serialcheck();
    #else
      if (version(NumNodes,Version))
        setvect(0x03,mismatch);
    #endif
  #elif defined(PCB_DEMO)
    version();
  #else
    version(NumNodes,Version);
  #endif

  #ifdef __OS2__
    WaitKeySem.createunique("WKEY");
    if (! WaitKeySem.opened())
      errorexittodos("Unable to create WKEY semaphore");
    kbdregistersemaphore(WaitKeySem);  //lint !e534
    KbdTimer.createunique("HBEAT");
    if (! KbdTimer.opened())
      errorexittodos("Unable to create HBEAT semaphore");
    createheartbeatthread();
  #else
    CtrlBreak = getcbrk();
    setcbrk(0);                //lint !e534
  #endif

  setkbdstatus(getkbdstatus() & ~SCROLL);

  readcolors();

  #ifdef KBD3
    #ifdef __OS2__
      serialcheck();
    #else
      if (Save == 0x23)       // was the serial file not found/opened/read?
        setvect(0x03,missing);
    #endif
  #endif

  #ifdef FIDO
    if (checkcommandline("/TOSS") != NULL || checkcommandline("/IMPORT") != NULL) {
      readpcboardsys();   // needed because when resetuser() is called it makes a new pcboard.sys
      toss_messages();    //lint !e534
      exit(0);
    }

    if (checkcommandline("/SCAN") != NULL || checkcommandline("/EXPORT") != NULL) {
      readpcboardsys();   // needed because when resetuser() is called it makes a new pcboard.sys
      copy_pcb_messages();  //lint !e534
      exit(0);
    }
  #endif

  removeeventbat();

  #ifdef KBD3
    #ifdef __OS2__
      serialcheck();
    #else
    {
      char Type1 = 'B';

//    setcollect(SERIALOS1);
//    Type1 = collect();
      if (Type1 != 'D' && Type1 != 'B')
        setvect(0x03,wrongos);
    }
    #endif
  #endif
}


void LIBENTRY stuffkbdforcaller(void) {
  char *p;

  if (fileexist("PCBDOOR.TXT") != 255) {
    startdisplay(FORCECOUNTLINES);
    displayfile("PCBDOOR.TXT",NOALTERNATE);
    unlink("PCBDOOR.TXT");
  }

  stuffkbdwithfile("PCBSTUFF.KBD");

  if ((p = checkcommandline("/KEY:")) != NULL) {
    *p = '!';  /* avoid finding this command on re-load of the software! */
    for (p += 5; *p; p++) {
      if (*p == '@' && *(p+1) != 0) {
        stuffkbdwithfile(p+1);
        return;
      }
      if (*p == '^' && *(p+1) >= 'A' && *(p+1) <= '[') {
        p++;
        *p -= '@';
      }
      stuffbuffer(*p);
    }
  }
}


void LIBENTRY backfromdos(void) {
  unsigned TimeOut;
  unsigned TimeIn;
  int      Minutes;
  char     Password[13];
  char     DoorName[20];

  timestr2(Status.DisplayText);
  logsystext(TXT_BACKFROMDOS,SPACERIGHT);
  redisplaystatusline();

  if (Status.ActStatus != ACT_DISABLED) {
    checkaccountbalance();
    Status.ConfJoinTime = dosgetlongtime();
    if ((TimeOut = searchlogfordoor(DoorName)) != 0xFFFF) {
      scandoorsforcharges(DoorName);
      TimeIn = currentminute();
      if (TimeOut > TimeIn)
        TimeIn += 1440;     /* 24 hours x 60 minutes */
      Minutes = (TimeIn - TimeOut);
      if (Minutes != 0)
        recordusage("DOOR USAGE MIN",DoorName,Status.DoorChargePerMin,Minutes,&UsersData.Account.DebitTPU);
    }
  }

  /* we're online, but are we calling in from remote? and if so, is     */
  /* there someone still there or have they lost carrier at this point? */

  #ifdef COMM
    Asy.IgnoreCDLoss = FALSE;
    if (Asy.Online == REMOTE && ! online()) {
      Asy.LostCarrier = TRUE;
      loguseroff(ALOGOFF);
    }
  #endif

  if (! PcbData.DisablePassword && UsersData.Password[0] != 0) {
    displaypcbtext(TXT_ENTERLOGONPWRD,NEWLINE|LFBEFORE);
    turnkbdtimeron();
    Password[0] = 0;
    if (! passwordokay(UsersData.Password,Password,TXT_YOURPASSWORD,TXT_WRONGPWRDENTERED,4)) {
      displaypcbtext(TXT_DENIEDPWRDFAILED,NEWLINE|LFBEFORE|BELL|LOGIT);
      Status.AutoLogoff = TRUE;
      loguseroff(ALOGOFF);
    }
  }

  stuffkbdforcaller();
  #ifdef PCBCOMM
    if (Status.TerseMode)
      sendstr("[t   ",12);
  #endif
  turnkbdtimeron();

  #ifdef __OS2__
    updatelinesnow();
  #endif
}


calltype LIBENTRY reload(void) {
  /* fix it so that a "time limit exceeded" doesn't occur      */
  /* inside of readpcboardsys() - which can happen if the code */
  /* tries to use the displaypcbtext() function while bringing */
  /* up a user who had dropped out to DOS but came back with   */
  /* a "no security level match in PWRD file" error.           */

  #ifndef __OS2__
    settimer(5,FIVEMINUTES);
  #endif

  setkbdtimer(PcbData.KbdTimeout);
  Control.WatchKbdClock     = FALSE;
  Control.WatchSessionClock = FALSE;

  /* the CDokay variable is used when the /NOCDVERIFY switch is on   */
  /* a TRUE value means that carrier has NEVER BEEN LOST and that if */
  /* the value ever goes to zero the carrier must have been lost at  */
  /* some point (regardless of its current state! and that the code  */
  /* should recycle as soon as it checks the cdstillup() status      */
  #ifdef COMM
    CDokay = 128;
  #endif

  /* open the callers log in case any errors occur while reading the */
  /* pcboard.sys file so that we can log them to the callers log     */
  openlog();

  readpcboardsys();

  if (LocalOn) {
    setscreen();
    if (Status.ForceScreenOff)
      turndisplayon(FALSE);
  }

  unlink("DOOR.SYS");
  chkMenuList();

  /* first check to see if we're coming back from DOS or a DOOR by checking  */
  /* our user record number.  If it's not zero then a user is already online */

  if (Status.UserRecNo != 0) {
    seteventup(TRUE);
    Status.CallerNumber = numcallers();
    Status.User = (Status.UserRecNo == 1 ? SYSOP : USER);
    #ifdef COMM
      if (! LocalOn)
        openmodem(HIDE);
    #endif
    agotoxy(0,0);
    if (Asy.Online == LOCAL)
      turnonscrollback();

    getdisplaynames();    // in case we're in an alias conference
    usernetavailable();
    Control.WatchKbdClock     = TRUE;
    Control.WatchSessionClock = TRUE;

    reloadfilelist();

    /* did we go to DOS to pack the users file or message base? */

    if (Status.PackFlag != ' ') {
      Status.PackFlag = ' ';
      #ifdef COMM
        if (Asy.Online == REMOTE)  {
          if (online()) {
            CDokay = 128;
            SkipRipDetect = TRUE;
            checkforrip();
          }
        }
      #endif
      newline();
      redisplaystatusline();
      #if defined(COMM) && defined(PCBCOMM)
        if (Status.TerseMode)
          sendstr("[t   ",12);
      #endif
      turnkbdtimeron();
      return(CALLWAIT_ONLINE);
    }

    if (fileexist(PcbData.SwapPath) != 255) {
      if (DebugLevel >= 1)
        writelog("SWAP FILE FOUND AT CODE LOAD",SPACERIGHT);
      unlink(PcbData.SwapPath);
    }

    Asy.IgnoreCDLoss = TRUE;
    SkipRipDetect = TRUE;
    checkforrip();
    backfromdos();
    return(CALLWAIT_ONLINE);
  }

  if (PcbData.Network)
    clearusernet();

  deallocatefilelist(THROWAWAY);      /* free up FileList memory */
  deallocatefiledesc();               /* free up FileDesc memory */
  clearMenu();                        /* clear menu state flags */

  /* noone is online yet - are we using the /LOCALON switch? */

  if (LocalOn) {
    #ifndef COMM
      callwait();
    #endif
    if (PcbData.EventActive) {
      checkforevent();            //lint !e534
      if (timeforevent() > 0)
        performevent();
    }

    #ifndef __OS2__
      settimer(7,Control.NetTimer);
    #endif

    settimer(5,FIVEMINUTES);  /* initial setting of the session timer */
    redisplaystatusline();
    stuffkbdforcaller();
    turnkbdtimeron();
    return(CALLWAIT_LOCALUSER);
  }


  if (fileexist("PCBSTUFF.KBD") != 255)
    unlink("PCBSTUFF.KBD");

  /* perhaps a user is online and we've been "front-ended"? */

  #ifdef COMM
    if (FrontEnd) {
      openmodem(HIDE);
      if (! online()) {
        Asy.LostCarrier = TRUE;
        loguseroff(ALOGOFF);
        Status.ErrorLevel = EXIT_GOODBYE;
        return(CALLWAIT_EXIT);
      }

      CDokay = 128;

      #ifndef __OS2__
        settimer(7,Control.NetTimer);
      #endif

      redisplaystatusline();
      settimer(5,FIVEMINUTES);  /* initial setting of the session timer */
      turnkbdtimeron();
      return(CALLWAIT_CALLER);
    }
  #endif

  /* if we got this far then we're not logging a caller in */

  closecallerlog();   // close the callers log, it'll be re-opened when needed

  Control.WatchKbdClock     = FALSE;
  Control.WatchSessionClock = FALSE;
  return(callwait());
}
