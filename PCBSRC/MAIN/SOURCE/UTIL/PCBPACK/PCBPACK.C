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

/***************************************************************************
 *
 * The following global "#define" statements may be used to enable various
 * features of PCBPack.  Most of these features have a command line switch
 * associated with them so they will be optional for the user.
 *
 *   ALLOCMODE    - Enable packing of messages in "message by message" mode
 *                  where the entire body of a message is read into memory
 *                  and then checked against a duplicate list (if requested)
 *                  and checked for file attachments or carbon copy lists.
 *                  If memory is not available for the text to be loaded,
 *                  then PCBPack falls back to "block by block" mode.
 *
 *   ARCHIVE      - Enable the ability to archive certain messages found in
 *                  the message base.  The criteria
 *
 *   BLOCKMODE    - Enable packing of messages in "block by block" mode where
 *                  PCBPack will read and write each block of a message
 *                  individually instead of on a "message by message" basis.
 *
 *   CHECKDUPS    - Enable checking of duplicate messages.  A 32-bit CRC is
 *                  calculated for each message and if any CRC values match
 *                  the latter is assumed to be a duplicate and is removed.
 *
 *   CLEANUP      - Enable the ability for PCBPack to "clean up" after a
 *                  crash by removing any of the "temporary" files that might
 *                  be left over.
 *
 *   DEBUGMODE    - Enable the use of the /DEBUG command line switch which
 *                  allows display of extra output to aid in debugging.
 *
 *   MAILWAITING  - Enables the ability for PCBPack to update a users mail
 *                  waiting flags so they will be notified of new mail upon
 *                  logging into the system.
 *
 *   DEBUG        - Enable the use of the MemCheck libraries.
 *
 *   NEWINDEX     - Enable PCBPack to use the version 15.0 compatible index
 *                  files.  These files are much different than the old
 *                  index files so to make PCBPack backward/forward
 *                  compatible this option exists.
 *                  (this option isn't implemented at this time)
 *
 *   ONLINE       - Enable PCBPack to be used in an "online" mode.  It will
 *                  interactively ask for criteria to use while packing.
 *                  This option will most likely NOT be available in the
 *
 *   PACKATTACH   - Enable removal of any file attachments that a message may
 *                  contain.
 *
 *   PACKGAPS     - Enable renumbering of messages where some program (either
 *                  a third party utility or otherwise) has created a large
 *                  "gap" of missing message numbers.  Such a "gap" would
 *                  cause an extremely large IDX file to be created and this
 *                  would solve such a problem.
 *
 *   REFNUM       - Enable checking and processing of reference numbers.
 *
 *   RENUMBER     - Enable renumbering of messages.  It is recommended that
 *                  this option NOT be used due to problems it causes with
 *                  Last Message Read pointers.
 *
 *   REPORT       - Enable PCBPack reporting features.  With this option
 *                  enabled, the operator can specify what type of report
 *                  should be generated (if any) when the MSGS files are
 *                  packed.
 *
 *   USECONFIG    - Enable PCBPack to read a configuration file containing
 *                  parameters to be used while packing MSGS files.
 *
 *   NODATE        - Disregards the date and time validation normally used to
 *                   determine if a message is valid or not.
 ***************************************************************************/

#ifdef __BORLANDC__
  #include <alloc.h>
#else
  #include <malloc.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <screen.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>
#include <system.h>
#include <country.h>
#include <dosfunc.h>
#include <misc.h>
#include "pcb.h"
#include <scrnio.h>
#include <scrnio.ext>
#include "pcbpack.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef __OS2__
  #include <semafore.hpp>
  CTimeoutSemaphore TimerSemaphore;
  void WatchKbd(void *Ignore);
#endif

char            LineSeparator           = 0xE3;
bool            AbortPack               = FALSE;
bool            PressedEscape           = FALSE;
#ifdef  EXECNET
bool            BackTrack               = FALSE;
#endif
bool            CallerLog               = TRUE;
bool            ConfFixUp               = FALSE;
bool            DisplayHelp             = FALSE;
bool            ForcePack               = FALSE;    /*** /FORCE obsolete ***/
bool            IndexOnly               = FALSE;
int             KeepUnreceived          = 0;
bool            NoSizeCheck             = FALSE;    /*** /NOSIZE obsolete **/
bool            OldIndex                = FALSE;
bool            PurgeReceived           = FALSE;
bool            IgnoreDate              = FALSE;
int             PurgeReceivedAfterDays  = 0;
bool            QuietPack               = FALSE;
bool            RemoveAll               = FALSE;
bool            RemoveBackup            = FALSE;
bool            RemoveDuplicates        = FALSE;
bool            UpdateOnly              = FALSE;
bool            ToUpperCase             = FALSE;
bool            IgnorePackoutDate       = FALSE;
bool            Compress                = FALSE;
char            Areas[100]              = "";
char            CapFile[MAX_FILENAME]   = "";
char            LogFile[MAX_FILENAME]   = "";
char            RptFile[MAX_FILENAME]   = "";
char            Today[MAX_DATELEN];
char            *FixLog = NULL;
char            ExtHdrFunctions[EXTHDR_TOTAL][EXTFUNCLEN+1] = {
    {"TO     "}, {"FROM   "}, {"SUBJECT"}, {"ATTACH "}, {"LIST   "},
    {"ROUTE  "}, {"ORIGIN "}, {"REQRR  "}, {"ACKRR  "}, {"ACKNAME"},
    {"PACKOUT"}
};
unsigned int    CRCDays     = 0;
unsigned int    Days        = 0;
unsigned int    DebugLevel  = 0;
unsigned int    MaxMessages = 0;
unsigned int    MinMessages = 0;
unsigned int    Timeout     = PACK_TIMEOUT;
unsigned int    MinimumPackThreshold = 0;
DOSFILE         Caller,
                Capture;

#ifdef  ARCHIVE
bool            Archive                 = FALSE;
#endif
#ifdef  BLOCKMODE
bool            BlockMode               = FALSE;
#endif
#ifdef  CLEANUP
bool            CleanUp                 = FALSE;
#endif
#ifdef  DEBUGMODE
DOSFILE         debugfile;
#endif
#ifdef  PACKGAPS
unsigned        MsgGap                  = 16384;
#endif
#ifdef  RENUMBER
long            Renumber                = 0;
#endif
#ifdef  REPORT
long            Report                  = 0;
#endif
char            *DatFile                = "PCBOARD.DAT";
pcbdattype      PcbData;
#ifdef  USECONFIG
char            *ConfigName             = "PCBPACK.CFG";
#endif
long            RangeLowMsg;
long            RangeHighMsg;
bool            PackRange               = FALSE;

colorpalette    ColorList[2] = {
    { 0x07, 0x0F, 0x0F, 0x70, 0x70, 0x0E, 0x30, 0x30, 0x30, 0x0F },
    { 0x07, 0x0F, 0x0C, 0x3F, 0x3E, 0x0F, 0x1F, 0x1E, 0x1C, 0x09 }
};
colorpalette            *ScreenColor;

#ifdef  REPORT
extern  msgbasestattype ReportInfo;
#endif

extern unsigned         _stklen = 0x4000;

void    closecnames(void);


static bool _NEAR_ LIBENTRY getrangeparams(char *Params) {
  char *p;

  if ((p = strchr(Params,'-')) == NULL)
    return(FALSE);

  *p = 0;
  RangeLowMsg  = atol(Params);
  RangeHighMsg = atol(p+1);
  *p = '-';

  if (RangeHighMsg < RangeLowMsg ||
      RangeLowMsg  <= 0          ||
      RangeHighMsg <= 0)
    return(FALSE);

  return(TRUE);
}


/* This function matches what the user typed (Arg) against built-in          */
/* command-line parameter values (Param) and returns TRUE if they match.     */
/*                                                                           */
/* Abbreviations are made possible through two methods:                      */
/*                                                                           */
/* 1) By comparing just the first X number of characters, where "X" is the   */
/*    number of characters typed by the user.  In other words, if the user   */
/*    types "AR" or "ARE" or "AREA", all of these will match "AREA".         */
/* 2) Or, because some parameters have conflicts in the first few characters */
/*    a second, optional, abbreviation may be supplied.  If Abbrev != NULL   */
/*    then it is compared against what the user typed.                       */

static int _NEAR_ LIBENTRY match(char *Arg, char *Param, int ArgLength, char *Abbrev) {
  if (memcmp(Arg,Param,ArgLength) == 0)
    return(TRUE);
  if (Abbrev != NULL && memcmp(Arg,Abbrev,ArgLength) == 0)
    return(TRUE);
  return(FALSE);
}


/***************************************************************************
 *** Program Entry Point ***************************************************/
int main(int argc, char **argv) {
    bool                LoadedConfig;
    int                 ArgLength;
    int                 i;
    unsigned            Conference = 0;
    char                *Arg,
                        *Env,
                        *Param;
    DOSFILE             rfp;
    char                Str[100];

#ifdef  DEBUG
    mc_startcheck(erf_standard);
#endif

    initscrnio();

    #ifndef __OS2__
      installhandlers();
      atexit(uninstallhandlers);
    #endif

    ScreenColor = &ColorList[Scrn_Mode];

    ShowClock = NOCLOCK;
    UpdateKbdStatus = FALSE;

#ifdef  USECONFIG
    for (i = 1; i < argc; i++) {
        strupr(argv[i]);
        if (argv[i][0] == '@') {
            ConfigName = argv[i] + 1;
        } else if (memcmp(argv[i],"/FILE:",6) == 0)
            DatFile = argv[i] + 6;
    }

    LoadedConfig = FALSE;
    if (ConfigName[0] != 0 && fileexist(ConfigName) != 255)
        LoadedConfig = LoadConfig(ConfigName);
#endif

    readdatfile();

    LineSeparator = (char) (PcbData.Foreign ? 0x0D : 0xE3);

#ifdef DEMO
    if (PcbData.NumAreas > 2 || PcbData.NumConf > 1) {
      PcbData.NumAreas = 2;
      PcbData.NumConf = 1;
    }
#endif

    if (argc == 1 && ! LoadedConfig)
        DisplayHelp = TRUE;

    for (i = 1; i < argc; i++) {
        switch (argv[i][0]) {
            case '?' :          /*** HELP **********************************/
                    DisplayHelp = TRUE;
                    i = argc;
                break;
            case '-' :
                break;
            case '/' :
                    Arg = argv[i] + 1;
                    if ((Param = strchr(Arg, ':')) != NULL)
                        *Param++ = 0;
                    ArgLength = strlen(Arg);
                    if (match(Arg, "AREA", ArgLength, NULL)) {
                        if (Param != NULL)
                            maxstrcpy(Areas, Param, sizeof(Areas));
#ifdef  ARCHIVE
                    } else if (match(Arg, "ARCHIVE", ArgLength, NULL)) {
                        Archive = TRUE;
#endif
#ifdef  EXECNET
                    } else if (match(Arg, "BACKTRACK", ArgLength, NULL)) {
                        BackTrack = TRUE;
#endif
#ifdef  BLOCKMODE
                    } else if (match(Arg, "BLOCK", ArgLength, NULL)) {
                        BlockMode = TRUE;
#endif
                    } else if (match(Arg, "CAP", ArgLength, NULL)) {
                        if (Param != NULL)
                            strcpy(CapFile, Param);
#ifdef  CLEANUP
                    } else if (match(Arg, "CLEANUP", ArgLength, NULL)) {
                        CleanUp = TRUE;
#endif
                    } else if (match(Arg, "COMPRESS", ArgLength, NULL)) {
                        Compress = TRUE;

                    } else if (match(Arg, "CRC", ArgLength, NULL)) {
                        if (Param)
                            CRCDays = (unsigned int)atol(Param);
                    } else if (match(Arg, "DATE", ArgLength, "DT")) {
                           if (Param != NULL) {
                               if ((Days = datetojulian(Param)) > 0)
                                   Days = datetojulian(datestr(Today)) - Days;
/*** 'Days' is unsigned so it *can't* be less than 0 ***********************
                               if (Days < 0)
                                   Days = 0;
 ***************************************************************************/                           }
                    } else if (match(Arg, "DAYS", ArgLength, "DY")) {
                        if (Param != NULL)
                           Days = atoi(Param);
#ifdef  DEBUGMODE
                    } else if (match(Arg, "DEBUG", ArgLength, NULL)) {
                        if (Param)
                            DebugLevel = atoi(Param);
                        else
                            DebugLevel = 1;
                        if (DebugLevel) {
                            if (dosfopen("DEBUG.OUT", OPEN_WRIT|OPEN_DENYNONE, &debugfile) == -1)
                                DebugLevel = 0;
                        }
#endif
                    } else if (match(Arg, "FAST", ArgLength, NULL)) {
                        QuietPack = TRUE;
                    } else if (match(Arg, "FIX", ArgLength, "ROY")) {
                        ConfFixUp = TRUE;
                        if (Param != NULL) {
                            FixLog = Param;
                        }
                    } else if (match(Arg, "FORCE", ArgLength, NULL)) {
                        ForcePack = TRUE;
#ifdef  PACKGAPS
                    } else if (match(Arg, "GAP", ArgLength, NULL)) {
                       if (Param)
                           MsgGap = atoi(Param);
#endif
                    } else if (match(Arg, "HELP", ArgLength, NULL)) {
                        DisplayHelp = TRUE;
                        i = argc;
                    } else if (match(Arg, "IGNORE", ArgLength, NULL)) {
                        IgnorePackoutDate = TRUE;
                    } else if (match(Arg, "INDEX", ArgLength, NULL)) {
                        IndexOnly = TRUE;
                    } else if (match(Arg, "KEEP", ArgLength, NULL)) {
                        if (Param != NULL) {
                          // find out how long to keep the private+unread msgs
                          KeepUnreceived = atoi(Param);
                        } else {
                          // never remove private+unread msgs
                          KeepUnreceived = -1;
                        }
                    } else if (match(Arg, "KILLALL", ArgLength, NULL)) {
                        RemoveAll = TRUE;
                    } else if (match(Arg, "KILLBAK", ArgLength, "KB")) {
                        RemoveBackup = TRUE;
                    } else if (match(Arg, "KILLDUPS", ArgLength, "KD")) {
                        RemoveDuplicates = TRUE;
                    } else if (match(Arg, "MAXMSGS", ArgLength, NULL)) {
                        if (Param != NULL)
                            MaxMessages = atoi(Param);
                    } else if (match(Arg, "MINMSGS", ArgLength, "MM")) {
                        if (Param != NULL)
                           MinMessages = atoi(Param);
                    } else if (match(Arg, "NOCALLER", ArgLength, "NC")) {
                           CallerLog = FALSE;
                    } else if (match(Arg, "NODATE", ArgLength, NULL)) {
                           IgnoreDate = TRUE;
                    } else if (match(Arg, "NOSIZE", ArgLength, NULL)) {
                        NoSizeCheck = TRUE;
                    } else if (match(Arg, "OLDINDEX", ArgLength, NULL)) {
                        OldIndex = TRUE;
                    } else if (match(Arg, "PURGE", ArgLength, NULL)) {
                        PurgeReceived = TRUE;
                        if (Param != NULL) {
                            PurgeReceivedAfterDays = atoi(Param);
                        } else {
                            PurgeReceivedAfterDays = 0;
                        }
                    } else if (match(Arg, "QUIET", ArgLength, NULL)) {
                        QuietPack = TRUE;
                    } else if (match(Arg, "RANGE", ArgLength, NULL)) {
                      if (Param != NULL)
                        PackRange = getrangeparams(Param);
#ifdef  RENUMBER
                    } else if (match(Arg, "RENUMBER", ArgLength, NULL)) {
                        if (Param != NULL)
                            Renumber = atol(Param);
#endif
                    } else if (match(Arg, "REPAIR", ArgLength, NULL)) {
                        ConfFixUp = TRUE;
                        if (Param != NULL) {
                            FixLog = Param;
                        }
#ifdef  REPORT
                    } else if (match(Arg, "REPORT", ArgLength, NULL)) {
                        if (Param != NULL) {
                            strcpy(RptFile, Param);
                            Report = TRUE;
                        }
#endif
                    } else if (match(Arg, "TIMEOUT", ArgLength, NULL)) {
                        if (Param != NULL)
                            Timeout = (unsigned)((atol(Param) == 0) ? PACK_TIMEOUT : atol(Param));
                    } else if (match(Arg, "THRESHOLD", ArgLength, NULL)) {
                        if (Param != NULL)
                            MinimumPackThreshold = (unsigned)(atol(Param));
                    } else if (match(Arg, "UPCASE", ArgLength, "UC")) {
                        ToUpperCase = TRUE;
                    } else if (match(Arg, "UPDATE", ArgLength, NULL)) {
                        UpdateOnly = TRUE;
                        CallerLog = FALSE;
                    }
                                break;
            case '@' :          /*** should be done already ***/
                break;
            default :
                break;
        }
    }

    setcursor(CUR_BLANK);
    cls();
    #ifdef __OS2__
      setscreenupdateinterval(QuietPack ? 500 : 200);
    #endif


    /*** Display command line parameters ***********************************/
    if (DisplayHelp) {
        ShowHelp();
        #ifdef __OS2__
          updatelinesnow();
        #endif
        goto exitPack;
    }

    fastprint(0,  0, "  PCBPack v15.3                                             Press ESC to Abort  ", ScreenColor->Status);
    fastprint(0, 24, "                                                                                ", ScreenColor->Status);

    if (CallerLog) {
        openlog();
    }
    if (CapFile[0] != 0) {
        if (dosfopen(CapFile, OPEN_RDWR|OPEN_APPEND|OPEN_DENYWRIT, &Capture) == -1) {
            strcpy(CapFile, "");
        }
    }

    if ((Env = getenv("PCB")) != NULL) {
        if (strstr(strupr(Env), "/OLDINDEX") != NULL) {
            OldIndex = TRUE;
        }
    }

#ifdef  DEMO
# ifdef RENUMBER
    if (Renumber) {
        DisplayLine(TRUE, ScreenColor->Intense, "/RENUMBER is not available in the DEMO.");
        goto exitPack;
    }
# endif
#endif

#ifdef  WARN
    /*** Warn user of obsolete parameters **********************************/
    if (ForcePack) {
        DisplayLine(TRUE, ScreenColor->Intense, "*** Obsolete parameter : /FORCE ***");
    }
    if (NoSizeCheck) {
        DisplayLine(TRUE, ScreenColor->Intense, "*** Obsolete parameter : /NOSIZE ***");
    }
#endif

    loadcnames(FALSE);

#ifdef  REPORT
    if (Report) {
        if (dosfopen(RptFile, OPEN_WRIT|OPEN_CREATE|OPEN_DENYNONE, &rfp) == -1)
            return FALSE;
        sprintf(Str, "Conference           %29.29sLow       High      Active\r\n","");
        dosfputs(Str, &rfp);
        sprintf(Str, "=====================%29.29s========  ========  ========\r\n","");
        dosfputs(Str, &rfp);
        dosfclose(&rfp);
    }
#endif

    #ifdef __OS2__
      TimerSemaphore.open("\\SEM32\TIMER");
      #if defined(__BORLANDC__)
        _beginthread(WatchKbd,4096,NULL);
      #elif defined(__WATCOMC__)
        _beginthread(WatchKbd,malloc(12*1024),12*1024,NULL);
      #endif
    #endif

    #if __TURBOC__ == 0x401
      // WARNING:  Under TC 3.0, the 'huge pointer arithmetic', on which
      // the zsort() function is based, has a BUG in the compiler code.
      // Therefore, the zsort() function should not be used under TC 3.0.
      //
      // To avoid this, the code in this section checks for 3.0 and,
      // when encountered, it will simply ensure that CRCDays is set to 0
      // so that when packconf() is called it will not attempt to use the
      // zsort() function.
      //
      // The code below will beep and show a message, then wait for 5 seconds
      // to ensure that you can see the message.  If possible, use BC 3.1 or
      // later to compile PCBPACK.  If not, then avoid using the /CRC switch.
      if (CRCDays != 0) {
        CRCDays = 0;
        DisplayLine(FALSE, ScreenColor->Warning, "CRC:## parameter not supported under Turbo C 3.0");
        beep();
        mydelay(500);
      }
    #endif

    while (Conference < PcbData.NumAreas && !AbortPack) {
        if (ValidConf(Conference)) {
            fastprint(0, 24, "                                                                                ", ScreenColor->Status);
#ifdef  REPORT
            if (Report) {
                GenerateReport(Conference);
            }
#endif
            if (UpdateOnly) {
                UpdateIndex(Conference++);
                continue;
            } else if (IndexOnly) {
                CreateIndex(Conference++);
                continue;
            }
            /*** this was moved up here for the /FIX parameter to work *****
             *** properly **************************************************/
            if (ConfFixUp)
                FixConf(Conference);
            if (PackConference(Conference) != TRUE) {
                if (AbortPack == TRUE) {
                    char    Str[80];

                    /*** report error **************************************/
                    if (CallerLog) {
                        sprintf(Str, "*** User Abort - Conference (%u) ***", Conference);
                        writelog(Str, SPACERIGHT);
                    }
                    DisplayLine(TRUE, ScreenColor->Intense, "*** User Abort - Conference (%u) ***", Conference);
                    DisplayLine(TRUE, ScreenColor->Intense, "");
                    break;
                }
            }
        }
        Conference++;
    }

#ifdef  REPORT
    if (Report) {
        if (dosfopen(RptFile, OPEN_RDWR|OPEN_APPEND|OPEN_DENYNONE, &rfp) == -1)
            return FALSE;
        sprintf(Str, "%70.70s========\r\n","");
        dosfputs(Str, &rfp);
        sprintf(Str, "%68.68s%10ld\r\n","",ReportInfo.NumActiveMsgs);
        dosfputs(Str, &rfp);
        dosfclose(&rfp);
    }
#endif

    closecnames();

exitPack:
    if (! DisplayHelp)
      DisplayLine(TRUE, ScreenColor->Normal, "");
    gotoxy(0, 23);
    fastprint(0, 24, "                                                                                ", ScreenColor->Normal);
    setcursor(CUR_NORMAL);
    if (CapFile[0] != 0)
        dosfclose(&Capture);
    if (CallerLog) {
        writelog("**************************************************************", LEFTJUSTIFY);
        closecallerlog();
    }

    #ifdef __OS2__
      updatelinesnow();
      mydelay(30);
    #endif

    #ifdef DEBUGMODE
      if (DebugLevel)
        dosfclose(&debugfile);
    #endif

    #ifdef DEBUG
      mc_endcheck();
    #endif

    if (PressedEscape)
      return(PCK_ESCAPE);
    if (AbortPack)
      return(PCK_ABORT);
    return(PCK_OK);
}


/***************************************************************************
 *** Check Conference for Command Line Validity ****************************/
int LIBENTRY ValidConf(unsigned Conf) {
    char    *semicolon,
            *hyphen,
            *str;
    char    ConfList[100];
    long    LowConf = -1,
            HighConf = -1;

    maxstrcpy(ConfList, Areas, sizeof(ConfList));
    str = ConfList;
    if (strstr(strupr(ConfList), "ALL") != NULL) {
        return TRUE;
    }
    while (str[0] != 0) {
        if ((semicolon = strchr(str, ';')) != NULL) {
            *semicolon = 0;
        }
        LowConf = atol(str);
        if ((hyphen = strchr(str, '-')) != NULL) {
            *hyphen = 0;
            HighConf = atol(hyphen + 1);
        } else {
            HighConf = LowConf;
        }
        if (LowConf > HighConf) {
            SWAP(LowConf, HighConf);
        }
#ifdef  DEBUGMODE
        if (DebugLevel >= 1000) {
            DebugDisplayLine(ScreenColor->Intense, "LowConf  = %u", (unsigned)LowConf);
            DebugDisplayLine(ScreenColor->Intense, "HighConf = %u", (unsigned)HighConf);
            DebugDisplayLine(ScreenColor->Intense, "");
        }
#endif
#ifdef  DEMO
        if (Conf > 1) {
            DisplayLine(FALSE, ScreenColor->Intense, "Only the Main Board and Conference One");
            DisplayLine(FALSE, ScreenColor->Intense, "");
            return FALSE;
        }
#endif
        if (Conf >= LowConf && Conf <= HighConf)
            return TRUE;
        if (semicolon != NULL)
            str = semicolon + 1;
        else
            break;
    }

    return FALSE;
}


/***************************************************************************
 ***************************************************************************/
void LIBENTRY DisplayGraph(unsigned long Processed, unsigned long Total)
{
    unsigned    Percent;
    register int i;

    if (Total) {
        Percent = (unsigned)((Processed * 50) / Total);
        if (Percent > 50)
          return;
    } else
        Percent = 50;

    for (i = 0; i < Percent; i++) {
        fastputch(LINE_3 + (COLUMN_28) + (i << 1),'�',ScreenColor->Status);
    }
}


/***************************************************************************
 ***************************************************************************/
int LIBENTRY DisplayConference(unsigned ConfNum, pcbconftype *Conf) {
    char        Str[256];

    if (!QuietPack) {
        fastprint(0, 1,  "旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커", ScreenColor->Win);
        fastprint(0, 2,  "� Conference:                                                                  �", ScreenColor->Win);
        fastprint(0, 3,  "�     Number:               같같같같같같같같같같같같같같같같같같같같같같같같같 �", ScreenColor->Win);
        fastprint(0, 4,  "읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸", ScreenColor->Win);
        fastprint(0, 5,  "旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커", ScreenColor->Win);
        fastprint(0, 6,  "�  Number:                             납                                      �", ScreenColor->Win);
        fastprint(0, 7,  "�      To:                             납                                      �", ScreenColor->Win);
        fastprint(0, 8,  "�    From:                             납                                      �", ScreenColor->Win);
        fastprint(0, 9,  "� Subject:                             납                                      �", ScreenColor->Win);
        fastprint(0, 10, "읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸", ScreenColor->Win);
        fastprint(14, 2, Conf->Name, ScreenColor->WinHighlight);
        sprintf(Str, "%u", ConfNum);
        fastprint(14, 3, Str, ScreenColor->WinHighlight);
    }
    if (CallerLog) {
        sprintf(Str, "*** Packing (%u) %s ***", ConfNum, Conf->Name);
        writelog(Str, SPACERIGHT);
    }
    DisplayLine(TRUE, ScreenColor->Intense, "*** Packing Conference (%u) -- %s ***", ConfNum, Conf->Name);
    #ifndef __OS2__
    DisplayLine(TRUE, ScreenColor->Intense, "");
    DisplayLine(TRUE, ScreenColor->Intense, "Memory available: %s bytes", comma(Str,farcoreleft()));
    #endif

    return 0;
}


/***************************************************************************
 ***************************************************************************/
void LIBENTRY DisplayMessage(msgheadertype *Header)
{
    char    Str[80];

    lascii(Str,bassngltolong(Header->MsgNumber));
    fastprint(11, 6, Str, ScreenColor->WinHighlight);

    Str[25] = 0;
    memcpy(Str, Header->ToField, 25);   fastprint(11, 7, Str, ScreenColor->WinHighlight);
    memcpy(Str, Header->FromField, 25); fastprint(11, 8, Str, ScreenColor->WinHighlight);
    memcpy(Str, Header->SubjField, 25); fastprint(11, 9, Str, ScreenColor->WinHighlight);
}


/***************************************************************************
 ***************************************************************************/
void LIBENTRY DisplayStatus(unsigned Status, msgheadertype *Header)
{
    static  char *MsgStatusStr[2] = {"Removed","Kept"};
    char    Str[80];
    char    MsgNumStr[20];

    lascii(MsgNumStr,bassngltolong(Header->MsgNumber));
    fastprint(11, 6, MsgNumStr, ScreenColor->WinHighlight);

    Str[25] = 0;
    memcpy(Str, Header->ToField, 25);   fastprint(11, 7, Str, ScreenColor->WinHighlight);
    memcpy(Str, Header->FromField, 25); fastprint(11, 8, Str, ScreenColor->WinHighlight);
    memcpy(Str, Header->SubjField, 25); fastprint(11, 9, Str, ScreenColor->WinHighlight);

    sprintf(Str, "Message %6.6s %s", MsgNumStr, MsgStatusStr[Status]);
    scrollup(42, 6, 64, 9, ScreenColor->Win);
    fastprint(42, 9, Str,(Status == MSG_KEPT ? ScreenColor->WinHighlight : ScreenColor->WinPurge));
}


/***************************************************************************
 ***************************************************************************/

int MaxLen = 1;

void DisplayLine(bool CaptureOutput, char Color, char *Format, ...) {
  int     Len;
  va_list vars;
  char    OutStr[65];
  char    Str[255];

  va_start(vars, Format);
  vsprintf(Str, Format, vars);
  va_end(vars);
  Len = strlen(Str);

  // minus 1 because column offsets start at 0.
  if (Len > 0)
    Len--;

  // MaxLen controls the width of the scrollup() call.  The lower MaxLen is,
  // the less information has to be scrolled up, and the faster the scrollup()
  // function runs.  Without knowing the maximum it will ever be, the only
  // value we could hardcode would be 79 (for a complete width), but that
  // wastes CPU cycles, so we'll let the system dynamically set the width that
  // needs to by scrolled by checking for the longest string to be displayed.
  if (Len > MaxLen) {
    MaxLen = Len;
    if (MaxLen >= 79) {
      MaxLen = 79;
      Str[79] = 0;
    }
  }

  scrollup(0, (UpdateOnly || IndexOnly || QuietPack) ? 1 : 11, MaxLen, 23, Color);
  fastprint(0, 23, Str, Color);

  if (CaptureOutput && CapFile[0] != 0) {
    sprintf(OutStr, "        %-54.54s", Str);
    OutStr[62] = '\r';
    OutStr[63] = '\n';
    OutStr[64] = 0;
    if (dosfputs(OutStr, &Capture) == -1) {
      dosfclose(&Capture);
      CapFile[0] = 0;
    }
  }
}


#ifdef  DEBUGMODE
/***************************************************************************
 ***************************************************************************/
void    DebugDisplayLine(char Color, char *Format, ...)
{
    char    Str[255];
    va_list vars;

    va_start(vars, Format);
    vsprintf(Str, Format, vars);
    va_end(vars);

    scrollup(0, (UpdateOnly || IndexOnly) ? 1 : 11, 79, 23, Color);

    if (strlen(Str) > 79)
      Str[79] = 0;

    fastprint(0, 23, Str, Color);
    if (DebugLevel) {
        dosfputs(Str, &debugfile);
        dosfputs("\r\n", &debugfile);
    }
}
#endif


/***************************************************************************
NOTE:  Header is an input variable, oMsgNum and oDate are OUTPUT variables.
This was done so that the message number and date only had to be determined
ONCE per message ... at the time that the header is validated.  This saves
the overhead of having to call bassngltolong() and datetojulian() *AGAIN* on
values that have already been converted.
 ***************************************************************************/
int LIBENTRY ValidHeader(msgheadertype *Header, long *oMsgNum, unsigned *oDate) {
  char Save;
  // first check the EASY ones... the ones that are not "compute intensive"

  // is the active flag a valid flag?
  if ((Header->ActiveFlag != MSG_ACTIVE) && (Header->ActiveFlag != MSG_INACTIVE))
      return FALSE;

  // are the to and from names valid?  at least, do they start with a printable
  // character?
  if ((Header->ToField[0] < ' ') || (Header->FromField[0] < ' '))
      return FALSE;

  // are the number of blocks at least 2?
  if ((Header->NumBlocks <= 1))
      return FALSE;

  // now do the compute-intensive stuff

  // get the message number and find out if it is a valid message number
  *oMsgNum = bassngltolong(Header->MsgNumber);
  if (*oMsgNum <= 0 || *oMsgNum > MAX_MSGNUM || *oMsgNum < MIN_MSGNUM)
      return FALSE;

  // get the date and find out if it is a valid date
  Save = Header->Time[0];
  Header->Time[0] = 0;
  *oDate = datetojulian(Header->Date);
  Header->Time[0] = Save;

  if (! IgnoreDate)  {
    if (*oDate == 0)
        return FALSE;
  }

  return TRUE;
}


/***************************************************************************
 *** Create an NDX and/or IDX file *****************************************/
int LIBENTRY CreateIndex(unsigned Conf) {
    char static         Zero[4] = { 0x00, 0x02, 0x00, 0x00 };
    bool                MakeOldIndex = OldIndex;
    unsigned            MsgDate;
/*  char                *pth; */
    long                LastNum = 0L;
    long                MsgNum  = 0L;
    long                Offset  = 0L;
    DOSFILE             imfp;
    DOSFILE             onfp;
    DOSFILE             oifp;
    msgbasediskstattype MsgStatsDisk;
    msgbasestattype     MsgStats;
    newindextype        Index;
    char                idxTemp[MAX_FILENAME];
    char                idxFile[MAX_FILENAME];
    char                idxBack[MAX_FILENAME];
    char                msgFile[MAX_FILENAME];
    char                ndxTemp[MAX_FILENAME];
    char                ndxFile[MAX_FILENAME];
    char                ndxBack[MAX_FILENAME];
    msgheadertype       Header;
    char                str[80];
    pcbconftype         ConfRec;

    getconfrecord(Conf, &ConfRec);

    if (OldIndex == FALSE) {
        MakeOldIndex = ConfRec.OldIndex;
    }

#ifdef  DEBUGMODE
    if (DebugLevel >= 25) {
        DebugDisplayLine(ScreenColor->Intense, "*** Conference : %-15s : %-5u ***", ConfRec.Name, Conf);
        DebugDisplayLine(ScreenColor->Intense, "");
    }
#endif

    if (ConfRec.MsgFile[0] == 0 || ConfRec.Name[0] == 0) {
#ifdef  DEBUGMODE
        if (DebugLevel >= 25) {
            DebugDisplayLine(ScreenColor->Intense, "No MSGS file to pack.");
            DebugDisplayLine(ScreenColor->Intense, "");
        }
#endif
        return FALSE;
    }

/*** LDZ: Added 3/8/93 for Andy Keeves due to his CNAMES file containing ***
 *** "extraneous" information (NotInUse) in the Name field. ****************/
    if (fileexist(ConfRec.MsgFile) == 255) {
        return FALSE;
    }

    /*** MSGS file *********************************************************/
    strcpy(msgFile, ConfRec.MsgFile);

    /*** IDX file **********************************************************/
    strcpy(idxFile, msgFile);
    AddExtension(idxFile, EXT_IDX);

    strcpy(idxBack, idxFile);
    AddExtension(idxBack, EXT_IDXBAK);

    strcpy(idxTemp, idxFile);
    AddExtension(idxTemp,EXT_IDXTEMP);
/*
    if ((pth = strrchr(idxTemp, '\\')) != NULL)
        *(pth + 1) = 0;
    strcat(idxTemp, "MIDX");
    strcat(idxTemp, TMP_FILENAME);
    mktemp(idxTemp);
*/

    if (MakeOldIndex) {
        /*** NDX file ******************************************************/
        strcpy(ndxFile, msgFile);
        AddExtension(ndxFile, EXT_NDX);

        strcpy(ndxBack, ndxFile);
        AddExtension(ndxBack, EXT_NDXBAK);

        strcpy(ndxTemp, ndxFile);
        AddExtension(ndxTemp,EXT_NDXTEMP);
/*
        if ((pth = strrchr(ndxTemp, '\\')) != NULL)
            *(pth + 1) = 0;
        strcat(ndxTemp, "MNDX");
        strcat(ndxTemp, TMP_FILENAME);
        mktemp(ndxTemp);
*/
    }

#ifdef  DEBUGMODE
    if (DebugLevel) {
        DebugDisplayLine(ScreenColor->Intense, "MSGS      = [%s]", msgFile);

        DebugDisplayLine(ScreenColor->Intense, "MSGS.IDX  = [%s]", idxFile);
        DebugDisplayLine(ScreenColor->Intense, "MSGS.IBK  = [%s]", idxBack);
        DebugDisplayLine(ScreenColor->Intense, "IDX Temp  = [%s]", idxTemp);

        DebugDisplayLine(ScreenColor->Intense, "");
    }
#endif

    if (dosfopen(msgFile, OPEN_READ|OPEN_DENYNONE, &imfp) == -1)
        return FALSE;

    if (dosfopen(idxTemp, OPEN_WRIT|OPEN_DENYNONE, &oifp) == -1) {
        dosfclose(&imfp);
        return FALSE;
    }

    if (MakeOldIndex)
        if (dosfopen(ndxTemp, OPEN_WRIT|OPEN_DENYNONE, &onfp) == -1) {
            MakeOldIndex = FALSE;
        } else {
            register int i;

            for (i = 0; i < (ConfRec.MsgBlocks * 1024); i++)
                dosfwrite(Zero, sizeof(bassngl), &onfp);
            dosrewind(&onfp);
            dosfseek(&onfp, 0L, SEEK_SET);
        }

    dossetbuf(&oifp,16384);
    dossetbuf(&imfp,16384);
    if (MakeOldIndex)
        dossetbuf(&onfp, 8192);

    if (dosfread(&MsgStatsDisk, sizeof(msgbasediskstattype), &imfp) != sizeof(msgbasediskstattype)) {
        dosfclose(&imfp);
        dosfclose(&oifp);

        if (MakeOldIndex)
            dosfclose(&onfp);
        return FALSE;
    }

    memset(&Index, 0, sizeof(Index));

    MsgStats.LowMsgNum      = bassngltolong(MsgStatsDisk.LowMsgNum);
    MsgStats.HighMsgNum     = bassngltolong(MsgStatsDisk.HighMsgNum);
    MsgStats.NumActiveMsgs  = bassngltolong(MsgStatsDisk.HighMsgNum);

    Offset = sizeof(msgheadertype);
    LastNum = MsgStats.LowMsgNum - 1;

    dosfseek(&imfp, sizeof(msgheadertype), SEEK_SET);

    sprintf(str, "Indexing (%u) -- %s", Conf, ConfRec.Name);
    fastcenter(24, str, ScreenColor->Status);
#ifdef  DEBUGMODE
    if (DebugLevel >= 25) {
        DebugDisplayLine(ScreenColor->Intense, str);
        DebugDisplayLine(ScreenColor->Intense, "");
    }
#endif

    while (dosfread(&Header, sizeof(msgheadertype), &imfp) == sizeof(msgheadertype)) {
        // check for a valid header, plus get the current message number & date
        if (ValidHeader(&Header,&MsgNum,&MsgDate)) {

            if (MsgNum <= LastNum) {
                Offset += sizeof(msgheadertype);
                continue;
            }


            /*** Press ESC to abort ****************************************/
            if (Escape()) {
                AbortPack = TRUE;
                goto Abort;
            }

            if (MsgNum > LastNum + 1) {
                /*** write blank index records *****/

                memset(&Index, 0, sizeof(Index));

                for (; LastNum < MsgNum - 1; LastNum++) {

                    if (dosfwrite(&Index, sizeof(Index), &oifp) == -1) {
                        AbortPack = TRUE;
                        goto Abort;
                    }

                    if (MakeOldIndex) {
                        if (dosfwrite(Zero, sizeof(Zero), &onfp) == -1) {
                            dosfclose(&onfp);
                            MakeOldIndex = FALSE;
                        }
                    }
                    if (Escape()) {
                        AbortPack = TRUE;
                        goto Abort;
                    }
                }
            }

            LastNum         = MsgNum;
            Index.Num       = MsgNum;
            Index.Offset    = (Header.ActiveFlag == MSG_ACTIVE) ? Offset : -Offset;
            Index.Status    = Header.Status;
            Index.Date      = (short) MsgDate;

// don't force to uppercase because the SYSTEM's table for uppercase may not
// be the same table that was used by the caller while the caller was online
//          {
//            char ToName[26];
//            memcpy(ToName,Header.ToField,25);  ToName[25] = 0;
//            strupr(ToName);
//            memcpy(Index.To,ToName,25);
//          }
            memcpy(Index.To,Header.ToField,25);

            memcpy(Index.From,      Header.FromField,       25);
            if (dosfwrite(&Index, sizeof(Index), &oifp) == -1) {
                AbortPack = TRUE;
                goto Abort;
            }

            if (MakeOldIndex) {
                bassngl     OldOffset;
                long        CalcOffset;

                CalcOffset = (Offset >> 7) + 1;
                if (Header.ActiveFlag == MSG_INACTIVE)
                  CalcOffset = -CalcOffset;

                longtobassngl(OldOffset,CalcOffset);
                if (dosfwrite(OldOffset, sizeof(bassngl), &onfp) == -1) {
                    dosfclose(&onfp);
                    MakeOldIndex = FALSE;
                }
            }

            Offset += sizeof(msgheadertype) * Header.NumBlocks;
            dosfseek(&imfp, Offset, SEEK_SET);

            DisplayLine(FALSE, (Header.ActiveFlag == MSG_ACTIVE) ? ScreenColor->Intense :
                            ScreenColor->Warning, "Message #%9ld", MsgNum);

        } else {
            Offset += sizeof(msgheadertype);
        }
    }

    memset(&Index, 0, sizeof(Index));

    while (LastNum < MsgStats.HighMsgNum) {

        if (dosfwrite(&Index, sizeof(Index), &oifp) == -1)
            break;

        if (MakeOldIndex) {
            if (dosfwrite(Zero, sizeof(bassngl), &onfp) == -1) {
                dosfclose(&onfp);
                MakeOldIndex = FALSE;
            }
        }
        ++LastNum;
#ifdef  DEBUGMODE
        if (DebugLevel >= 50) {
            DebugDisplayLine(ScreenColor->Intense, "LastNum = %ld", LastNum);
            DebugDisplayLine(ScreenColor->Intense, "");
        }
#endif
    }

Abort:
    DisplayLine(FALSE, ScreenColor->Intense, "");
    DisplayLine(FALSE, ScreenColor->Warning, "Renaming temporary files...  Please wait...");

    if (MakeOldIndex && dosfclose(&onfp) == -1)
      AbortPack = TRUE;
    if (dosfclose(&oifp) == -1)
      AbortPack = TRUE;
    if (dosfclose(&imfp) == -1)
      AbortPack = TRUE;

    if (!AbortPack) {
      if ((removefile(idxBack)) == -1 ||
          (MakeOldIndex && removefile(ndxBack) == -1))
        return(FALSE);
    }

    if (AbortPack) {
      if ((removefile(idxTemp) == -1) ||
          (MakeOldIndex && removefile(ndxTemp) == -1))
        return(FALSE);
    } else {
      if (RemoveBackup) {
        if ((removefile(idxFile) == -1) ||
            (MakeOldIndex && removefile(ndxFile) == -1))
          return(FALSE);
      } else {
        if ((renamefile(idxFile,idxBack) == -1) ||
            (MakeOldIndex && renamefile(ndxFile,ndxBack) == -1))
          return(FALSE);
      }
      if ((renamefile(idxTemp,idxFile) == -1) ||
          (MakeOldIndex && renamefile(ndxTemp,ndxFile) == -1))
        return(FALSE);
    }

    DisplayLine(FALSE, ScreenColor->Intense, "Proceeding.");
    DisplayLine(FALSE, ScreenColor->Intense, "");

    return ((AbortPack) ? FALSE : TRUE);
}


/***************************************************************************
 *** Check for ESC press ***************************************************/

#ifdef __OS2__
#pragma argsused
void WatchKbd(void *Ignore) {
  while (1)
    if (bgetkey(0) == 0x011B) {
      PressedEscape = TRUE;
      TimerSemaphore.stoptimer();
    }
}
#endif

int LIBENTRY Escape(void) {
  #ifdef __OS2__
    return(PressedEscape);
  #else
    if (bgetkey(1)) {
      if (bgetkey(0) == 0x011B) {
         PressedEscape = TRUE;
         return TRUE;
      }
    }
    return FALSE;
  #endif
}

/***************************************************************************
 *** Wait for One Second ***************************************************/

/* Wait one second, return TRUE if ESC is pressed, FALSE if the second delay
   runs to completion */

void LIBENTRY setpacktimer(long Seconds) {
  #ifdef __OS2__
    TimerSemaphore.settimer(Seconds);
  #else
    settimer(PACK_TIMER,Seconds);
  #endif
}

bool LIBENTRY packtimerexpired(int Pause) {
  #ifdef __OS2__
    return(TimerSemaphore.timerexpired(Pause));
  #else
    giveup();
    return(gettimer(PACK_TIMER) <= 0);
  #endif
}

int LIBENTRY waitonesecond(void) {
  #ifdef __OS2__
    TimerSemaphore.settimer(ONESECOND);
    TimerSemaphore.timerexpired(ONESECOND);
    if (Escape())
      return(TRUE);
  #else
    settimer(PACK_TIMER, ONESECOND);
    while (gettimer(PACK_TIMER) > 0) {
      giveup();
      if (Escape())
        return(TRUE);
    }
  #endif
  return(FALSE);
}


/***************************************************************************
 *** Load a configuration file *********************************************/
bool LIBENTRY LoadConfig(char *ConfigName) {
    char        *Arg,
                *Param;
    int         ArgLength;
    DOSFILE     ConfigFile;
    char static DatFileName[66];
    char        Str[250];

    if (dosfopen(ConfigName, OPEN_READ|OPEN_DENYNONE, &ConfigFile) == -1)
        return FALSE;
    while (dosfgets(Str, 250, &ConfigFile) == 0) {
        stripleft(Str, ' ');
        stripright(Str, ' ');
        strupr(Str);
        switch (Str[0]) {
            case '-' :
                break;
            case '/' :
                    Arg = Str + 1;
                    if ((Param = strchr(Arg, ':')) != NULL)
                        *Param++ = 0;
                    ArgLength = strlen(Arg);
                    if (match(Arg, "AREA", ArgLength, NULL)) {
                        if (Param != NULL)
                            maxstrcpy(Areas, Param, sizeof(Areas));
#ifdef  BLOCKMODE
                    } else if (match(Arg, "BLOCK", ArgLength, NULL)) {
                        BlockMode = TRUE;
#endif
                    } else if (match(Arg, "CAP", ArgLength, NULL)) {
                        if (Param != NULL)
                            strcpy(CapFile, Param);

                    } else if (match(Arg, "COMPRESS", ArgLength, NULL)) {
                        Compress = TRUE;

                    } else if (match(Arg, "CRC", ArgLength, NULL)) {
                        if (Param != NULL)
                            CRCDays = (unsigned int)atol(Param);
                    } else if (match(Arg, "DATE", ArgLength, "DT")) {
                       if (Param != NULL) {
                           if ((Days = datetojulian(Param)) > 0)
                               Days = datetojulian(datestr(Today)) - Days;
/*** 'Days' is unsigned so it *can't* be less than 0 ***********************
                           if (Days < 0)
                               Days = 0;
 ***************************************************************************/
                       }
                    } else if (match(Arg, "DAYS", ArgLength, "DY")) {
                        if (Param != NULL)
                            Days = atoi(Param);
#ifdef  DEBUGMODE
                    } else if (match(Arg, "DEBUG", ArgLength, NULL)) {
                        if (Param)
                            DebugLevel = atoi(Param);
                        else
                            DebugLevel = 1;
                        if (DebugLevel) {
                            if (dosfopen("DEBUG.OUT", OPEN_WRIT|OPEN_DENYNONE, &debugfile) == -1)
                                DebugLevel = 0;
                        }
#endif
                    } else if (match(Arg, "FAST", ArgLength, NULL)) {
                        QuietPack = TRUE;
                    } else if (match(Arg, "FILE", ArgLength, NULL)) {
                        if (Param != NULL) {
                          strcpy(DatFileName,Param);
                          DatFile = DatFileName;
                        }
                    } else if (match(Arg, "FIX", ArgLength, "ROY")) {
                        ConfFixUp = TRUE;
                        if (Param != NULL) {
                            FixLog = Param;
                        }
                    } else if (match(Arg, "FORCE", ArgLength, NULL)) {
                        ForcePack = TRUE;
#ifdef  PACKGAPS
                    } else if (match(Arg, "GAP", ArgLength, NULL)) {
                        if (Param)
                            MsgGap = atoi(Param);
#endif
                    } else if (match(Arg, "IGNORE", ArgLength, NULL)) {
                        IgnorePackoutDate = TRUE;
                    } else if (match(Arg, "INDEX", ArgLength, NULL)) {
                        IndexOnly = TRUE;
                    } else if (match(Arg, "KEEP", ArgLength, NULL)) {
                        if (Param != NULL) {
                          // find out how long to keep the private+unread msgs
                          KeepUnreceived = atoi(Param);
                        } else {
                          // never remove private+unread msgs
                          KeepUnreceived = -1;
                        }
                    } else if (match(Arg, "KILLBAK", ArgLength, "KB")) {
                        RemoveBackup = TRUE;
                    } else if (match(Arg, "KILLDUPS", ArgLength, "KD")) {
                        RemoveDuplicates = TRUE;
                    } else if (match(Arg, "MAXMSGS", ArgLength, NULL)) {
                        if (Param != NULL)
                            MaxMessages = atoi(Param);
                    } else if (match(Arg, "MINMSGS", ArgLength, "MM")) {
                        if (Param != NULL)
                            MinMessages = atoi(Param);
                    } else if (match(Arg, "NOCALLER", ArgLength, "NC")) {
                        CallerLog = FALSE;
                    } else if (match(Arg, "NODATE", ArgLength, NULL)) {
                        IgnoreDate = FALSE;
                    } else if (match(Arg, "NOSIZE", ArgLength, NULL)) {
                        NoSizeCheck = TRUE;
                    } else if (match(Arg, "OLDINDEX", ArgLength, NULL)) {
                        OldIndex = TRUE;
                    } else if (match(Arg, "PURGE", ArgLength, NULL)) {
                        PurgeReceived = TRUE;
                        if (Param != NULL) {
                            PurgeReceivedAfterDays = atoi(Param);
                        } else {
                            PurgeReceivedAfterDays = 0;
                        }
                    } else if (match(Arg, "QUIET", ArgLength, NULL)) {
                        QuietPack = TRUE;
                    } else if (match(Arg, "RANGE", ArgLength, NULL)) {
#ifdef  RENUMBER
                    } else if (match(Arg, "RENUMBER", ArgLength, NULL)) {
                        if (Param != NULL)
                            Renumber = atol(Param);
#endif
                    } else if (match(Arg, "REPAIR", ArgLength, NULL)) {
                        ConfFixUp = TRUE;
                        if (Param != NULL) {
                            FixLog = Param;
                        }
#ifdef  REPORT
                    } else if (match(Arg, "REPORT", ArgLength, NULL)) {
                        if (Param != NULL) {
                            strcpy(RptFile, Param);
                            Report = TRUE;
                        }
#endif
                    } else if (match(Arg, "TIMEOUT", ArgLength, NULL)) {
                        if (Param != NULL)
                            Timeout = (unsigned)((atol(Param) == 0) ? PACK_TIMEOUT : atol(Param));
                    } else if (match(Arg, "THRESHOLD", ArgLength, NULL)) {
                        if (Param != NULL)
                            MinimumPackThreshold = (unsigned)(atol(Param));
                    } else if (match(Arg, "UPCASE", ArgLength, "UC")) {
                        ToUpperCase = TRUE;
                    }
                break;
            default :
                break;
        }
    }
    if (dosfclose(&ConfigFile) == -1)
        return FALSE;

    return TRUE;
}


/***************************************************************************
 *** Compare two CRC values from the 'crctype' structure *******************/
int CompareCRC(const void *p, const void *q) {
    crctype *r = (crctype *) p;
    crctype *s = (crctype *) q;

    if (r->CRC < s->CRC)
        return -1;
    else if (r->CRC > s->CRC)
        return 1;

    return 0;
}


/***************************************************************************
 *** Compare two dates from the 'crctype' structure ************************/
int CompareDate(const void _HUGE_ *p, const void _HUGE_ *q) {
    crctype *r = (crctype *) p;
    crctype *s = (crctype *) q;

    if (r->Date < s->Date)
        return 1;
    else if (r->Date > s->Date)
        return -1;

    return 0;
}
