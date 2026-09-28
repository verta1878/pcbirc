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


/******************************************************************************/
/*                                                                            */
/*                                  MENU.CPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                          PCBoard 15.1 Menu System                          */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

#include    <ctype.h>
#include    <stdarg.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "pcbmisc.hpp"

#include    "stack.hpp"

#include    "scrmisc.hpp"

#include    "menu.h"

#ifdef DEBUG
#include    <memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

#define MNU_PN_SIZE     (64+1)

#define TYP_MENU     0                  /* MENU FILE NAME */
#define TYP_SCR      1                  /* NUM */
#define TYP_BLT      2                  /* NUM */
#define TYP_FILE     3                  /* FILE NAME */
#define TYP_DOOR     4                  /* NUM/NAME */
#define TYP_CONF     5                  /* NUM/NAME */
#define TYP_DIR      6                  /* NUM/U/P */
#define TYP_STFS     7                  /* STRING */
#define TYP_STFF     8                  /* FILENAME */
#define TYP_XPERT    9                  /* ON/OFF/TOGGLE */
#define TYP_GBYE    10                  /* MAIN BOARD G COMMAND */
#define TYP_BYE     11                  /* MAIN BOARD BYE COMMAND */
#define TYP_QUIT    12                  /* QUIT THIS MENU */
#define TYP_EXIT    13                  /* QUIT ALL MENUS */
#define TYP_PPE     14                  /* PPE FILENAME */
#define TYP_SSNR    15                  /* STRING */
#define TYP_SFNR    16                  /* FILENAME */
#define TYP_DIS     17                  /* DISABLED TYPE */
#define TYP_STFQ    18                  /* STUFF STRING QUIETLY */
#define TYP_SQNR    19                  /* STUFF QUIETLY W/NO RETURN */
#define TYP_DORX    20                  /* DOOR & EXIT MNU */
#define TYP_PPEX    21                  /* PPE & EXIT MNU */
#define TYP_CMD     22                  /* COMMAND W/O CONTROL LOSS */
#define TYP_CLST    23                  /* CMD.LST W/O CONTROL LOSS */

// 4128003140839385 07/96 Stan's Credit Card Info

#ifdef LIB

#define TXT_INVOPT      0

#define TXT_EXPERTON    350
#define TXT_EXPERTOFF   351

#else

#define TXT_INVOPT  TXT_INVALIDMENUOPTION

#endif

#ifdef LIB

#define UsersData   UserSys

#endif

#define fexist(f)           ((fileexist(f) & 0x18) == 0)

/******************************************************************************/

// Types

typedef unsigned char byte;

struct sMENUITEM
{
    char    cmd  [8+1];
    int     sec;
    byte    typ;
    char    inf  [MNU_PN_SIZE+1];
};

/******************************************************************************/

// Constants

#ifdef LIB

char   menu_mask[] = { 3, 0, '\x21', '\x7E' };

char * menuText [] =
{
    "Invalid menu option type encountered (@OPTEXT@)"
};

#else

char * menu_mask   = mask_command;

#endif

/******************************************************************************/

// Variables

cISTACK<char> menuStack(DD_NO,TRUE);

char      * menuTitle     = NULL;
char      * menuDispFile  = NULL;
char      * menuHelpFile  = NULL;
bool        menuDispForce = FALSE;
bool        menuHotKeys   = FALSE;
bool        menuPassThru  = 0;
int         menuPromptCnt = 0;
char      * menuPrompt    = NULL;
int         menuOptCnt    = 0;
sMENUITEM * menuOpts      = NULL;

char        LST_NAME[12+1];

char        mnuListExist  = FALSE;
char        mnuExit       = FALSE;
char        displayMNU    = FALSE;

#ifdef LIB

cSCRIPT   * scriptPtr;
int         DebugLevel;

#endif

/******************************************************************************/

// Function Prototypes

 int  LIBENTRY doCommand    ( bool   tryCmdLst,
                            char * cmd,
                            int    tokens,
                            int  & tries,
                            char * tokBuf,
                            int    tokBufSize,
                            char * unTokCmds );

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

#ifdef LIB

#pragma argsused
int LIBENTRY doCommand(bool tryCmdLst, char * cmd, int tokens, int & tries,
    char * tokBuf, int tokBufSize, char * unTokCmds)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY doScript(char * p, char * a, int c)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY bltmenu(int NumTokens)
{
}

#endif

    /*--------------------------------------------------------------------*/

#if 0

#pragma argsused
void LIBENTRY joinconfcommand(int NumTokens)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY doquestionnaire(int Qnum)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY showblt(int BltNum)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY dirmenu(int NumTokens)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY stuffpplbuffer(char * Str)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY stuffkbdwithfile(char * FileName)
{
}

    /*--------------------------------------------------------------------*/

void LIBENTRY byecommand(void)
{
    goodbye(TRUE);
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY loguseroff(logtype Type)
{
    goodbye(TRUE);
}

#endif

    /*--------------------------------------------------------------------*/

void menuErr(int textNum, bool disp, displaytype flags, char * format, ...)
{
    char    fmtBuf[80+1];
    va_list arglist;

    va_start(arglist,format);
    vsprintf(fmtBuf,format,arglist);
    va_end(arglist);

    strcpy(Status.DisplayText,fmtBuf);

    if (disp)
    {
#ifdef LIB
        if (flags & LFBEFORE) newline();
        printxlated(menuText[textNum]);
#else
        displaypcbtext(textNum,flags);
#endif
        if (!(flags & AUTO))
        {
            newline();
            moreprompt(PRESSENTER);
        }
#ifdef LIB
        else if (flags & NEWLINE)
        {
            newline();
        }
        if (flags & LFAFTER) newline();
#endif
    }

#ifdef LIB
    if (flags & LOGIT) writelog(menuText[textNum],SPACERIGHT);
#else
    if (!disp && (flags & LOGIT)) logsystext(textNum,SPACERIGHT);
#endif
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doSCR(char * s)
{
    doquestionnaire(atoi(s));
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doBLT(char * s)
{
//    int i = tokenize(s);
//
//    while (i-- > 0) showblt(atoi(getnexttoken()));
    bltmenu(tokenize(s));
}

    /*--------------------------------------------------------------------*/

void LIBENTRY bldLST(bool wait = TRUE)
{
    // Create LST_NAME
    DOSFILE fs;
    if (dosfopen(LST_NAME,OPEN_WRIT|OPEN_CREATE|OPEN_DENYRDWR,&fs) == 0)
    {
        dosfputs(wait ? "1" : "0",&fs);
        dosfputs("\r\n",&fs);

        dosfputs(menuDispForce ? "1" : "0",&fs);
        dosfputs("\r\n",&fs);

        cISTACK<char> tmpStack(DD_YES,TRUE);
        while (!menuStack.isEmpty()) tmpStack.push(menuStack.pop());

        while (!tmpStack.isEmpty())
        {
            char * tmp = tmpStack.pop();
            dosfputs(tmp,&fs);
            dosfputs("\r\n",&fs);
            menuStack.push(tmp);
        }

        dosfclose(&fs);

        mnuListExist = TRUE;
    }
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doDOOR(char * s, int retFlag)
{
    if (retFlag) bldLST(FALSE);

    opendoor(tokenize(s));

    // If we ever get here, door is not accessible...
    if (retFlag)
    {
        unlink(LST_NAME);
        mnuListExist = FALSE;
    }
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doPPE(char * s, int retFlag)
{
    if (retFlag) bldLST(FALSE);

    char file[256+1];
    char args[256+1];
    int  argcnt = 0;

    strcpy(file,s);
    *args = '\0';
    if (strchr(file,' ') != NULL)
    {
        strcpy(args,strchr(file,' '));
        *strchr(file,' ') = '\0';
        stripleft(args,' ');
        stripright(args,' ');
        argcnt = tokenizestr(args);
    }

    substenvinfile(file,sizeof(file));

    if ((doScript(file,NULL,argcnt) < 0) && retFlag)
    {
        unlink(LST_NAME);
        mnuListExist = FALSE;
    }
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doCMD(bool tryCmdList, char * cmdLine)
{
    char tokLine [ 256 + 1 ];

    strcpy(tokLine,cmdLine);

    int toks = tokenize(tokLine);
    if (toks > 0)
    {
        int tries = 0;
        char * cmd = getnexttoken();
        doCommand(!tryCmdList,cmd,toks,tries,tokLine,sizeof(tokLine),cmdLine);
    }
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doCONF(char * s)
{
    joinconfcommand(tokenize(s));
    displayMNU = TRUE;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doDIR(char * s)
{
    dirmenu(tokenize(s));
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doSTFS(char * s)
{
    bldLST();

    stuffbufferstr(s);
    stuffbufferstr("^M");
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doSTFF(char * f)
{
    bldLST();

    stuffkbdwithfile(f);
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doSTFQ(char * s)
{
    bldLST();

    stuffpplbuffer(s);
    stuffpplbuffer("^M");
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doSSNR(char * s)
{
    stuffbufferstr(s);
    stuffbufferstr("^M");

    unlink(LST_NAME);
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doSFNR(char * f)
{
    stuffkbdwithfile(f);

    unlink(LST_NAME);
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doSQNR(char * s)
{
    stuffpplbuffer(s);
    stuffpplbuffer("^M");

    unlink(LST_NAME);
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doGBYE(void)
{
    byecommand();
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doBYE(void)
{
    loguseroff(NLOGOFF);
}

    /*--------------------------------------------------------------------*/

char * LIBENTRY nextField(char * p)
{
    char * n = NULL;

    if (p == NULL) return n;

    n = strchr(p,',');
    if (n != NULL) *(n++) = '\0';

    return n;
}

    /*--------------------------------------------------------------------*/

// char * LIBENTRY pcbstrdup(char * s);

    /*--------------------------------------------------------------------*/

void LIBENTRY localClearMenu(void)
{
    if (menuTitle    != NULL) bfree(menuTitle),    menuTitle    = NULL;
    if (menuDispFile != NULL) bfree(menuDispFile), menuDispFile = NULL;
    if (menuHelpFile != NULL) bfree(menuHelpFile), menuHelpFile = NULL;
    menuPromptCnt = 0;
    if (menuPrompt   != NULL) bfree(menuPrompt),   menuPrompt   = NULL;
    menuOptCnt    = 0;
    if (menuOpts     != NULL) bfree(menuOpts),     menuOpts     = NULL;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY clearMenu(void)
{
    menuStack.flush();
    localClearMenu();
#ifndef LIB
    if (Status.Logoff != DOOR)
#endif
    {
        unlink(LST_NAME);
        mnuListExist = FALSE;
        mnuExit      = FALSE;
    }
}

    /*--------------------------------------------------------------------*/

char * LIBENTRY readStr(DOSFILE * fs, bool & re, bool & me)
{
    char * p = NULL;

    if (re || me) return p;

    char buf[256+1];
    re = (dosfgets(buf,256+1,fs) == -1);

    int len = strlen(buf);
    if ((len > 0) && !re)
    {
        p = (char *) bmalloc(len+1);
        me = (p == NULL);
        if (!me) memcpy(p,buf,len+1);
    }

    return p;
}

    /*--------------------------------------------------------------------*/

int LIBENTRY readInt(DOSFILE * fs, bool & re, bool & me)
{
    int i = 0;

    if (re || me) return i;

    char * p = readStr(fs,re,me);
    if (p != NULL)
    {
        i = atoi(p);
        bfree(p);
    }

    return i;
}

    /*--------------------------------------------------------------------*/

extern int DebugLevel;

void debugLog(int level, char * fmt, ...)
{
    if (DebugLevel >= level)
    {
        char buf[80+1];
        va_list args;

        va_start(args,fmt);
        vsprintf(buf,fmt,args);
        va_end(args);

        writelog(buf,SPACERIGHT);
    }
}

    /*--------------------------------------------------------------------*/

int LIBENTRY readMenu(char * f)
{
    debugLog(2,"Reading MNU:  %s",f);

    // Clear out any existing information
    localClearMenu();

    // If problem opening file, return with error
    DOSFILE fs;
    if (dosfopen(f,OPEN_READ|OPEN_DENYWRIT,&fs) == -1) return -1;

    bool readErr = FALSE;
    bool memErr  = FALSE;

    menuTitle     = readStr(&fs,readErr,memErr);
    menuDispFile  = readStr(&fs,readErr,memErr);
    menuHelpFile  = readStr(&fs,readErr,memErr);

    char * p;
    int    i;

    p = nextField(menuDispFile);
    menuDispForce = ((p && atoi(p)) ? TRUE : FALSE);

    p = nextField(p);
    menuHotKeys   = ((p && atoi(p)) ? TRUE : FALSE);

    p = nextField(p);
    menuPassThru  = (p ? atoi(p) : 0);

    menuPromptCnt = readInt(&fs,readErr,memErr);

    for (i = 0; (i < menuPromptCnt) && !readErr && !memErr; ++i)
    {
        char * p = readStr(&fs,readErr,memErr);
        if (p == NULL) continue;

        if (i == 0)
        {
            menuPrompt = p;
            continue;
        }

        char * t = strchr(p,',');
        if (t != NULL) *(t++) = '\0';

        if ((t != NULL) && (stricmp(p,Status.MultiLangExt) == 0))
        {
            bfree(menuPrompt);
            menuPrompt = pcbstrdup(t);
            if (menuPrompt == NULL) memErr = TRUE;
        }

        bfree(p);
    }

    menuOptCnt = readInt(&fs,readErr,memErr);
    if ((menuOptCnt > 0) && !readErr && !memErr)
    {
        menuOpts = (sMENUITEM *) bmalloc(sizeof(sMENUITEM)*menuOptCnt);
        memErr = (menuOpts == NULL);
    }

    for (i = 0; (i < menuOptCnt) && !readErr && !memErr; ++i)
    {
        memset(&menuOpts[i],0,sizeof(sMENUITEM));

        char * p = readStr(&fs,readErr,memErr);
        if (p == NULL) continue;

        debugLog(2," Option #%d:  %s",i,p);

        char * op = p;
        char * sp = nextField(op);
        char * fp = nextField(sp);
        char * ip = nextField(fp);

        if ((op != NULL) && (sp != NULL) && (fp != NULL) && (ip != NULL))
        {
            strcpy(menuOpts[i].cmd,strupr(op));
            menuOpts[i].sec = atoi(sp);
            menuOpts[i].typ = atoi(fp);
            strcpy(menuOpts[i].inf,ip);
        }

        debugLog(2," Option #%d:  %s - %d - %d - %s",i,
            menuOpts[i].cmd,menuOpts[i].sec,menuOpts[i].typ,menuOpts[i].inf);

        bfree(p);
    }

    // If problem closing file, return with error
    if ((dosfclose(&fs) == -1) && !readErr) return -1;

    return 0;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY processPARAMS(char * buf, int & nt)
{
    char tmpBuf[512+1];
    strcpy(tmpBuf,buf);

    char * ptr = strstr(buf,"@PARAMS@");
    if (ptr != NULL)
    {
        // Insert user parameters
        *ptr = '\0';
        while (nt > 0)
        {
            addchar(buf,' ');
            strcat(buf,getnexttoken());
            --nt;
        }

        // Append other inf parameters
        addchar(buf,' ');
        strcat(buf,tmpBuf+int(ptr-buf)+8);

        // replace all ';' with ' '
        change(buf,';',' ');

        // strip leading and trailing ' '
        stripleft(buf,' ');
        stripright(buf,' ');

        // Start at beg of buf
        ptr = buf;

        // While not at end of string . . .
        while (*ptr)
        {
            // Make a temp ptr to next char
            char * tmp = ptr+1;

            // If cur and next chars are spaces . . .
            if ((*ptr == ' ') && (*tmp == ' '))
                strcpy(ptr,tmp);    // Remove one space
            else
                ++ptr;              // Otherwise inc to next char
        }
    }
}

    /*--------------------------------------------------------------------*/

#ifdef LIB

       bool NoMNU; // Used to temporarily disable MNU execution

#else

extern bool NoMNU; // Used to temporarily disable MNU execution

#endif

bool doMenuSub(char * f, DOSFILE * fs, int nt = 0)
{
    if (NoMNU)
    {
        print("MNUs disabled:  ");
        println(f);
        return 0;
    }

    // Initialize local variables
    bool exitAllFlag = FALSE;
    bool exitFlag    = FALSE;
    int  i;

    // Save filename in a local buffer for possible later use
    char fn[MNU_PN_SIZE+1];
    strcpy(fn,f);
    strupr(fn);

#ifndef LIB
    writeusernetstatus(UNAVAILABLE,NULL);
#endif

    // Push this filename
    menuStack.push(fn);

    // Read the menu file, returning with error if necessary
    if (readMenu(fn) == -1) goto EXIT;

    // If there are more menus listed, gotta do them too
    if ((fs != NULL) && (fs->handle > 0))
    {
        char nfn[MNU_PN_SIZE+1];
        if (dosfgets(nfn,MNU_PN_SIZE+1,fs) == 0)
        {
            exitAllFlag = exitFlag = doMenuSub(nfn,fs);
            if (!exitAllFlag && (readMenu(fn) == -1)) exitFlag = TRUE;
            if (exitFlag) goto EXIT;
        }
        else
        {
            dosfclose(fs);
            unlink(LST_NAME);
            debugLog(2,"Closed & Deleted:  %s",LST_NAME);
        }
    }

    // Display the menu if defined and not in expert mode and tokens were not passed
    if ((!UsersData.ExpertMode || menuDispForce) && (menuDispFile != NULL) &&
        (nt == 0))
    {
        startdisplay(NOCHANGE);
        displayfile(menuDispFile,
            displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
    }

    // if menuDispFile == NULL figure out something to display

    do {

        char cmdbuf[40+1];
        char cmd[8+1];

        *cmdbuf = *cmd = '\0';

        strcpy(Status.DisplayText,menuTitle ? menuTitle : "");
        if (nt == 0)
        {
            if (menuHotKeys)
            {
                if (menuPrompt == NULL)
                {
#ifdef LIB
                    printxlated("@X0E@OPTEXT@ Menu Command ('MENU' for options)");
#else
                    displaypcbtext(TXT_MENUCOMMAND,DEFAULTS);
#endif
                }
                else
                {
                    printcolor(PCB_YELLOW);
                    bool suppress = lastChar(menuPrompt,'_');
                    if (suppress) menuPrompt[strlen(menuPrompt)-1] = '\0';
                    dispString(menuPrompt,0);
                    if (suppress) strcat(menuPrompt,"_");
                }

                print(" ");

                Display.NumLinesPrinted = 0;

                cmdbuf[0] = '\0';
                cmdbuf[1] = '\0';

                while (*cmdbuf == '\0')
                {
                    int k = cinkey();
                    if ((k == '\x0D') || ((k >= '\x21') && (k <= '\x7E')))
                        *cmdbuf = k;
                }

                println(cmdbuf);

                *cmdbuf = toupper(*cmdbuf);
                if (*cmdbuf == '\x0D') *cmdbuf = '\0';
            }
            else
            {
                if (menuPrompt == NULL)
                {
#ifdef LIB
                    inputfieldstr(cmdbuf,"@OPTEXT@ Menu Command ('MENU' for options)",
                        PCB_YELLOW,40,
#else
                    inputfield(cmdbuf,TXT_MENUCOMMAND,40,
#endif
                        displaytype(UPCASE|NEWLINE|STACKED|HIGHASCII),
                        NOHELP,menu_mask);
                }
                else
                {
                    printcolor(PCB_YELLOW);
                    bool suppress = lastChar(menuPrompt,'_');
                    if (suppress) menuPrompt[strlen(menuPrompt)-1] = '\0';
                    dispString(menuPrompt,0);
                    if (suppress) strcat(menuPrompt,"_");
                    inputfieldstr(cmdbuf,suppress ? "_" : "",curcolor(),40,
                        displaytype(UPCASE|NEWLINE|STACKED|HIGHASCII),
                        NOHELP,menu_mask);
                }
            }

            nt = tokenizestr(cmdbuf);
        }

        startdisplay(FORCECOUNTLINES);

        if (nt > 0)
        {
            --nt;
            memcpy(cmd,getnexttoken(),8);
            cmd[8] = '\0';
        }

        // Built in commands ( * means only if not used in menu )
        //     MENU *MEN *ME *Q *X *G *BYE *HELP *HEL *HE *H

        // User typed MENU
        if ((memcmp(cmd,"MENU",5) == 0) && (menuDispFile != NULL))
        {
            debugLog(2,"User selected MENU");
            startdisplay(NOCHANGE);
            displayfile(menuDispFile,
                displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
            continue;
        }
        // User wants help
        else if (memcmp(cmd,"?",2) == 0)
        {
            debugLog(2,"User selected HELP");
            startdisplay(NOCHANGE);

            if (menuHelpFile != NULL)
                displayfile(menuHelpFile,
                    displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
//            else
//                ;//help not available

            // Clear out any pending tokens
            nt = 0;

            // Set up to force a redisplay
            Display.NumLinesPrinted = 2;
        }

        // Search for EXACT match
        for (i = 0; i < menuOptCnt; ++i)
        {
            if ((strcmp(menuOpts[i].cmd,cmd) == 0) &&
                (Status.CurSecLevel >= menuOpts[i].sec))
            {
                debugLog(2,"Found Option #%d:  %s = %s",i,cmd,menuOpts[i].cmd);
                break;
            }
        }

        // Search for PARTIAL match
        if (!menuPassThru && (i == menuOptCnt) && (strlen(cmd) > 0))
            for (i = 0; i < menuOptCnt; ++i)
            {
                if ((memcmp(menuOpts[i].cmd,cmd,strlen(cmd)) == 0) &&
                    (Status.CurSecLevel >= menuOpts[i].sec))
                {
                    debugLog(2,"Found Option #%d:  %s = LEFT(%s,%d)",i,
                        cmd,menuOpts[i].cmd,strlen(cmd));
                    break;
                }
            }

        // Either process option or search for optional internal command
        if (i < menuOptCnt)
        {
            debugLog(2,"Processing option #%d:  %s - %d - %s",i,menuOpts[i].cmd,
                menuOpts[i].typ,menuOpts[i].inf);

            char buf[249+64+1];
            strcpy(buf,menuOpts[i].inf);
            processPARAMS(buf,nt);

            switch (menuOpts[i].typ)
            {
                case TYP_MENU:
                    substenvinfile(buf,sizeof(buf));
                    mnuExit = exitAllFlag = exitFlag = doMenuSub(buf,NULL,nt);
                    if (!exitAllFlag && (readMenu(fn) == -1)) exitFlag = TRUE;
                    break;

                case TYP_PPE:
                    doPPE(buf,TRUE);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_PPEX:
                    doPPE(buf,FALSE);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_CMD:
                    doCMD(FALSE,buf);
                    break;

                case TYP_CLST:
                    doCMD(TRUE,buf);
                    break;

                case TYP_SCR:
                    doSCR(buf);
                    break;

                case TYP_BLT:
                    doBLT(buf);
                    break;

                case TYP_FILE:
                    startdisplay(NOCHANGE);
                    displayfile(buf,
                        displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
                    break;

                case TYP_DOOR:
                    doDOOR(buf,TRUE);
                    break;

                case TYP_DORX:
                    doDOOR(buf,FALSE);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_CONF:
                    doCONF(buf);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_DIR:
                    doDIR(buf);
                    break;

                case TYP_STFS:
                    doSTFS(buf);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_STFF:
                    doSTFF(buf);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_STFQ:
                    doSTFQ(buf);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_SSNR:
                    doSSNR(buf);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_SFNR:
                    doSFNR(buf);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_SQNR:
                    doSQNR(buf);
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_DIS:
                    break;

                case TYP_XPERT:
                    strupr(buf);
                    if (strstr(buf,"OF") != NULL)
                        strcpy(buf,"0");
                    else if (strstr(buf,"ON") != NULL)
                        strcpy(buf,"1");
                    else if (strstr(buf,"TO") != NULL)
                        strcpy(buf,"-1");
                    switch (atoi(buf))
                    {
                        case  1:
                            UsersData.ExpertMode = TRUE;
                            break;

                        case  0:
                            UsersData.ExpertMode = FALSE;
                            break;

                        case -1:
                        default:
                            UsersData.ExpertMode = !UsersData.ExpertMode;
                            break;
                    }
                    displaypcbtext(UsersData.ExpertMode ?
                        TXT_EXPERTON : TXT_EXPERTOFF,
                        LOGIT|LFBEFORE|NEWLINE|LFAFTER);
                    break;

                case TYP_GBYE:
                    doGBYE();
                    break;

                case TYP_BYE:
                    doBYE();
                    exitAllFlag = exitFlag = TRUE;
                    break;

                case TYP_QUIT:
                    exitFlag = TRUE;
                    break;

                case TYP_EXIT:
                    mnuExit = exitAllFlag = exitFlag = TRUE;
                    break;

                default:
                    menuErr(TXT_INVOPT,FALSE,LOGIT,"%s - %d",fn,menuOpts[i].typ);
                    break;
            }

            // Set up to force a redisplay
            Display.NumLinesPrinted = 2;
        }
        else if (((memcmp(cmd,"MEN",4) == 0) || (memcmp(cmd,"ME",3) == 0)) &&
            (menuDispFile != NULL))
        {
            debugLog(2,"Default MENU - %s",cmd);
            startdisplay(NOCHANGE);
            displayfile(menuDispFile,
                displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
            continue;
        }
        else if ((memcmp(cmd,"HELP",5) == 0) || (memcmp(cmd,"HEL",4) == 0) ||
            (memcmp(cmd,"HE",3) == 0) || (memcmp(cmd,"H",2) == 0))
        {
            debugLog(2,"Default HELP - %s",cmd);
            startdisplay(NOCHANGE);

            if (menuHelpFile != NULL)
                displayfile(menuHelpFile,
                    displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
//            else
//                ;//help not available

            // Clear out any pending tokens
            nt = 0;

            // Set up to force a redisplay
            Display.NumLinesPrinted = 2;
        }
        else if (memcmp(cmd,"Q",2) == 0)
        {
            debugLog(2,"Default QUIT - %s",cmd);
            exitFlag = TRUE;
        }
        else if (memcmp(cmd,"X",2) == 0)
        {
            debugLog(2,"Default EXPERT - %s",cmd);
            UsersData.ExpertMode = !UsersData.ExpertMode;
            displaypcbtext(UsersData.ExpertMode ?
                TXT_EXPERTON : TXT_EXPERTOFF,
                LOGIT|LFBEFORE|NEWLINE|LFAFTER);
        }
        else if (memcmp(cmd,"G",2) == 0)
        {
            debugLog(2,"Default GOODBYE - %s",cmd);
            doGBYE();
        }
        else if (memcmp(cmd,"BYE",4) == 0)
        {
            debugLog(2,"Default BYE - %s",cmd);
            doBYE();
            exitAllFlag = exitFlag = TRUE;
        }
        else if (menuPassThru)
        {
            char buf[249+64+1];
            strcpy(buf,cmd);
            strcat(buf," @PARAMS@");
            processPARAMS(buf,nt);
            switch (menuPassThru)
            {
                case 2:
                    doSTFQ(buf);
                    break;

                default:
                    doSTFS(buf);
                    break;
            }
            exitAllFlag = exitFlag = TRUE;
        }

        if (!exitFlag && (!UsersData.ExpertMode || menuDispForce) &&
            (menuDispFile != NULL) && (nt == 0))
        {
            if ((Display.NumLinesPrinted > 1) &&
                ((i == menuOptCnt) ||
                ((menuOpts[i].typ != TYP_MENU) &&
                (menuOpts[i].typ != TYP_GBYE))))
            {
                moreprompt(PRESSENTER);
                startdisplay(NOCHANGE);
                displayfile(menuDispFile,
                    displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
            }
            else if ((i < menuOptCnt) && (menuOpts[i].typ == TYP_MENU))
            {
                startdisplay(NOCHANGE);
                displayfile(menuDispFile,
                    displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
            }
        }

    } while (!exitFlag);

EXIT:
    localClearMenu();
    menuStack.pop();
#ifndef LIB
    usernetavailable();
#endif
    return exitAllFlag;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY chkMenuList(void)
{
    sprintf(LST_NAME,"MNU%05u.$$$",PcbData.NodeNum);
    mnuListExist = fexist(LST_NAME);
}

    /*--------------------------------------------------------------------*/

int LIBENTRY doMenuList(void)
{
    displayMNU   = FALSE;
    mnuListExist = FALSE;
    mnuExit      = FALSE;

    if (scriptPtr != NULL) return -2;
    if (!menuStack.isEmpty()) return -1;

    sprintf(LST_NAME,"MNU%05u.$$$",PcbData.NodeNum);

    // If the file doesn't exist, exit
    if (!fexist(LST_NAME)) return 0;

    // If problem opening file, return
    DOSFILE fs;
    if (dosfopen(LST_NAME,OPEN_READ|OPEN_DENYWRIT,&fs) == -1)
    {
        unlink(LST_NAME);
        return 0;
    }

    // Read in first line to get things started and do the menu
    char fn[MNU_PN_SIZE+1];
    if (dosfgets(fn,MNU_PN_SIZE+1,&fs) == 0)
    {
        int waitFlag = atoi(fn);

        dosfgets(fn,MNU_PN_SIZE+1,&fs);
        menuDispForce = atoi(fn);

        if (waitFlag && (!UsersData.ExpertMode || menuDispForce))
            moreprompt(PRESSENTER);

        if (dosfgets(fn,MNU_PN_SIZE+1,&fs) == 0)
            doMenuSub(fn,&fs);
    }

    // Close and delete the file
    if (fs.handle > 0)
    {
        mnuListExist = FALSE;
        dosfclose(&fs);
        unlink(LST_NAME);
    }

    if (!mnuListExist) Display.NumLinesPrinted = 1;

    return 0;
}

    /*--------------------------------------------------------------------*/

int LIBENTRY doMenu(char * f, int nt)
{
    displayMNU = FALSE;
    mnuExit    = FALSE;

    if (scriptPtr != NULL) return -2;
    if (!menuStack.isEmpty()) return -1;

    sprintf(LST_NAME,"MNU%05u.$$$",PcbData.NodeNum);
    if (!fexist(LST_NAME))
        doMenuSub(f,NULL,nt);
    else
        return doMenuList();

    if (!mnuListExist) Display.NumLinesPrinted = 1;

    return 0;
}

/******************************************************************************/

